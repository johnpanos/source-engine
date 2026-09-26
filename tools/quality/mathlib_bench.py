#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# mathlib conformance and microbenchmark driver (RFC 0005 Q-FOUNDATION).
#
# Builds the mathlib conformance suite (mathlibconformance) and benchmark
# (mathlibbench) from unittests/mathlibtest/ with the compiler, flags and
# defines a configured Waf tree recorded in its c4che, so the programs see
# mathlib exactly as that product profile compiles it: the Linux desktop tree
# (gcc, -march=core2), the Android arm64 tree (NDK clang), or any other.
# mathlib's sources come from mathlib/wscript, their one owner.
#
#   build   --tree DIR [--name N] [--extra-flags F] [--mathlib-rev REV]
#           compile into quality-results/mathlib/<name>/. --extra-flags adds
#           ISA flags (e.g. -march=x86-64-v3) to every unit; --mathlib-rev
#           builds mathlib and its public headers from a git revision, the
#           suite and benchmark from the working tree (for A/B)
#   check   --name N [--adb [SERIAL]] [--seeds K]
#           run the conformance suite K times with different seeds, locally
#           or on a device over adb (/data/local/tmp); fails unless every run
#           reports its checks-v1 record with zero failures
#   bench   N [N2 ...] [--adb [SERIAL]] [--cpu C] [--rounds R] [--filter S]
#           run --full benchmarks in interleaved rounds (N, N2, N, N2, ...),
#           report each kernel's median over rounds, ratios against the first
#           build, and whether the result checksums agree
#
# Evidence (build.json, check.json, bench-<names>.json) is written next to
# the programs. Dependency-free: Python 3 standard library only.
#
# ============================================================================

import argparse
import ast
import json
import os
import re
import shlex
import shutil
import statistics
import subprocess
import sys
import tarfile
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402  (source identity and compiler identity; one owner)

ROOT = conformance.repo_root()
OUT_ROOT = os.path.join(ROOT, "quality-results", "mathlib")
SUITE_SOURCES = [
    "unittests/mathlibtest/mathlib_conformance.cpp",
    "unittests/mathlibtest/mathlib_conformance_scalar.cpp",
    "unittests/mathlibtest/mathlib_conformance_transform.cpp",
    "unittests/mathlibtest/mathlib_conformance_geometry.cpp",
    "unittests/mathlibtest/mathlib_conformance_conversion.cpp",
    "unittests/mathlibtest/mathlib_conformance_simd.cpp",
    "unittests/mathlibtest/mathlib_conformance_vmatrix.cpp",
]
BENCH_SOURCES = ["unittests/mathlibtest/mathlib_bench.cpp"]
HARNESS_SOURCE = "unittests/mathlibtest/mathlib_conformance.cpp"
LINK_SURFACE = "unittests/mathlibtest/tier0_link_surface.cpp"
# Flags that belong to Waf's dependency tracking or to shared libraries.
DROP_FLAGS = {"-MMD", "-llog", "-pipe"}
DEVICE_DIR = "/data/local/tmp/mathlib"


def fail(msg):
    print("mathlib_bench: " + msg, file=sys.stderr)
    sys.exit(1)


def tree_env(tree):
    """The compile facts a Waf tree recorded (c4che/_cache.py)."""
    cache = os.path.join(tree, "c4che", "_cache.py")
    if not os.path.isfile(cache):
        fail("%s has no c4che/_cache.py; configure it first" % tree)
    env = {}
    for line in open(cache, encoding="utf-8"):
        m = re.match(r"^([A-Z_a-z0-9]+) = (.*)$", line)
        if m and m.group(1) in ("CXX", "CXXFLAGS", "DEFINES", "INCLUDES", "LINKFLAGS", "AR", "DEST_CPU", "DEST_OS"):
            env[m.group(1)] = ast.literal_eval(m.group(2))
    for key in ("CXX", "CXXFLAGS", "DEFINES"):
        if key not in env:
            fail("%s records no %s" % (cache, key))
    return env


def mathlib_sources(root):
    text = open(os.path.join(root, "mathlib", "wscript"), encoding="utf-8").read()
    # The list closes on a line of its own; comments inside it contain "]".
    m = re.search(r"source = \[(.*?)\n\s*\]\s*\n", text, re.S)
    if not m:
        fail("mathlib/wscript: no source list")
    return ["mathlib/" + s for s in re.findall(r"'([A-Za-z0-9_]+\.cpp)'", m.group(1))]


def export_revision(rev, dest):
    """mathlib/, public/mathlib/ and common/sse2neon.h at a git revision."""
    archive = subprocess.run(["git", "-C", ROOT, "archive", "--format=tar", rev, "mathlib", "public/mathlib", "common/sse2neon.h"],
                             capture_output=True)
    if archive.returncode:
        fail("git archive %s failed: %s" % (rev, archive.stderr.decode(errors="replace")))
    tar_path = os.path.join(dest, "rev.tar")
    with open(tar_path, "wb") as f:
        f.write(archive.stdout)
    with tarfile.open(tar_path) as t:
        t.extractall(dest, filter="data")
    rev_id = subprocess.run(["git", "-C", ROOT, "rev-parse", rev], capture_output=True, text=True).stdout.strip()
    return rev_id


def compile_all(cxx, flags, jobs):
    """jobs: list of (source, object). Returns the first failure's log or None."""
    def one(job):
        src, obj = job
        r = subprocess.run(cxx + flags + ["-c", src, "-o", obj], capture_output=True, text=True, cwd=ROOT)
        return None if r.returncode == 0 else "%s\n%s" % (src, r.stderr[-4000:])
    with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        for err in pool.map(one, jobs):
            if err:
                return err
    return None


def cmd_build(args):
    tree = os.path.abspath(args.tree)
    env = tree_env(tree)
    name = args.name or os.path.basename(tree.rstrip("/"))
    out = os.path.join(OUT_ROOT, name)
    obj_dir = os.path.join(out, "obj")
    if os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(obj_dir)

    cxx = list(env["CXX"])
    if shutil.which(cxx[0]) is None and not os.path.isfile(cxx[0]):
        fail("compiler %s is missing" % cxx[0])
    flags = [f for f in env["CXXFLAGS"] if f not in DROP_FLAGS]
    flags += ["-D" + d for d in env["DEFINES"]]
    # The programs link no engine allocator; the link surface supplies tier0.
    flags += ["-DNO_MALLOC_OVERRIDE"]
    extra = shlex.split(args.extra_flags) if args.extra_flags else []
    flags += extra

    src_root = ROOT
    rev_id = None
    tmp = None
    if args.mathlib_rev:
        tmp = tempfile.mkdtemp(prefix="mathlib-rev-", dir=out)
        rev_id = export_revision(args.mathlib_rev, tmp)
        src_root = tmp
    # Revision headers first, then the working tree's.
    includes = []
    if tmp:
        includes += ["-I" + os.path.join(tmp, "public"), "-I" + os.path.join(tmp, "public", "mathlib"), "-I" + os.path.join(tmp, "common")]
    includes += ["-I" + os.path.join(ROOT, p) for p in ("public", "public/tier0", "public/mathlib", "common", "unittests/mathlibtest")]
    includes += ["-I" + p for p in env.get("INCLUDES", [])]

    lib_jobs = [(os.path.join(src_root, s), os.path.join(obj_dir, os.path.basename(s)[:-4] + ".o")) for s in mathlib_sources(src_root)]
    lib_flags = flags + ["-I" + os.path.join(src_root, "mathlib")] + includes
    err = compile_all(cxx, lib_flags, lib_jobs)
    if err:
        fail("mathlib compile failed:\n" + err)
    ar = env.get("AR", ["ar"])
    ar = [ar] if isinstance(ar, str) else list(ar)
    lib = os.path.join(out, "libmathlib.a")
    r = subprocess.run(ar + ["rcs", lib] + [o for _, o in lib_jobs], capture_output=True, text=True)
    if r.returncode:
        fail("archive failed: " + r.stderr)

    prog_flags = flags + includes
    link_flags = [f for f in env.get("LINKFLAGS", []) if f not in DROP_FLAGS and f != "-Wl,--no-undefined"]
    if env.get("DEST_OS") == "android":
        link_flags += ["-static-libstdc++"]
    programs = {}
    for prog, sources in (("mathlibconformance", SUITE_SOURCES), ("mathlibbench", BENCH_SOURCES)):
        jobs = [(os.path.join(ROOT, s), os.path.join(obj_dir, prog + "-" + os.path.basename(s)[:-4] + ".o")) for s in sources + [LINK_SURFACE]]
        # The harness unit tests results for NaN and infinity, so it keeps
        # IEEE semantics under the products' -ffast-math.
        harness = [j for j in jobs if j[0].endswith(HARNESS_SOURCE)]
        err = compile_all(cxx, prog_flags, [j for j in jobs if j not in harness]) or \
            compile_all(cxx, prog_flags + ["-fno-finite-math-only"], harness)
        if err:
            fail("%s compile failed:\n%s" % (prog, err))
        exe = os.path.join(out, prog)
        r = subprocess.run(cxx + flags + link_flags + [o for _, o in jobs] + [lib, "-o", exe, "-lm", "-pthread"], capture_output=True, text=True)
        if r.returncode:
            fail("%s link failed:\n%s" % (prog, r.stderr[-4000:]))
        programs[prog] = exe

    record = {
        "schema": "mathlib-build/v1",
        "name": name,
        "tree": os.path.relpath(tree, ROOT),
        "dest_os": env.get("DEST_OS"),
        "dest_cpu": env.get("DEST_CPU"),
        "compiler": conformance.compiler_identity(cxx[0]),
        "cxxflags": flags,
        "extra_flags": extra,
        "mathlib_rev": rev_id,
        "source": conformance.source_identity(ROOT),
        "built": time.strftime("%Y-%m-%dT%H:%M:%S"),
    }
    with open(os.path.join(out, "build.json"), "w") as f:
        json.dump(record, f, indent=1)
    if tmp:
        shutil.rmtree(tmp)
    shutil.rmtree(obj_dir)
    print("built %s (%s %s%s)" % (out, env.get("DEST_OS"), env.get("DEST_CPU"), ", mathlib " + rev_id[:10] if rev_id else ""))


class Runner:
    """Runs a program locally or on an adb device."""

    def __init__(self, adb, serial, cpu=None):
        self.adb = adb
        self.serial = serial
        self.cpu = cpu
        self.pushed = set()

    def adb_cmd(self, *args):
        base = ["adb"] + (["-s", self.serial] if self.serial else [])
        return base + list(args)

    def run(self, name, prog, argv, timeout):
        path = os.path.join(OUT_ROOT, name, prog)
        if not os.path.isfile(path):
            fail("%s is not built; run build first" % path)
        if not self.adb:
            cmd = ([ "taskset", "-c", str(self.cpu) ] if self.cpu is not None else []) + [path] + argv
        else:
            remote = "%s/%s/%s" % (DEVICE_DIR, name, prog)
            if remote not in self.pushed:
                subprocess.run(self.adb_cmd("shell", "mkdir -p %s/%s" % (DEVICE_DIR, name)), check=True, capture_output=True)
                subprocess.run(self.adb_cmd("push", path, remote), check=True, capture_output=True)
                subprocess.run(self.adb_cmd("shell", "chmod 755 " + remote), check=True, capture_output=True)
                self.pushed.add(remote)
            pin = ["taskset", "%x" % (1 << self.cpu)] if self.cpu is not None else []
            cmd = self.adb_cmd("shell", " ".join(shlex.quote(a) for a in pin + [remote] + argv))
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        return r.returncode, r.stdout, r.stderr

    def fetch(self, name, remote_file, local_file):
        if self.adb:
            subprocess.run(self.adb_cmd("pull", remote_file, local_file), check=True, capture_output=True)


def cmd_check(args):
    runner = Runner(args.adb is not None, args.adb or None)
    runs = []
    ok = True
    for i in range(args.seeds):
        seed = str(0x5EED0001 + i)
        code, out, err = runner.run(args.name, "mathlibconformance", ["--seed", seed], args.timeout)
        records = re.findall(r"^CONFORMANCE (\d+) (\d+)$", out, re.M)
        fails = re.findall(r"^FAIL .*$", out, re.M)
        good = code == 0 and len(records) == 1 and int(records[0][0]) > 0 and records[0][1] == "0"
        ok = ok and good
        runs.append({"seed": seed, "exit": code, "record": records, "failures": fails[:40], "sections": re.findall(r"^section .*$", out, re.M)})
        print("seed %s: %s %s" % (seed, "pass" if good else "FAIL", records[0] if records else "(no record)"))
        for line in fails[:10]:
            print("  " + line)
    result = {"schema": "mathlib-check/v1", "name": args.name, "device": "adb" if args.adb is not None else "local", "runs": runs, "pass": ok}
    with open(os.path.join(OUT_ROOT, args.name, "check.json"), "w") as f:
        json.dump(result, f, indent=1)
    sys.exit(0 if ok else 1)


def parse_bench(out):
    kernels = {}
    for m in re.finditer(r"^(\S+)\s+([\d.]+) ns/op\s+min\s+([\d.]+)\s+check ([0-9a-f]+)$", out, re.M):
        kernels[m.group(1)] = {"ns": float(m.group(2)), "min": float(m.group(3)), "check": m.group(4)}
    return kernels


def cmd_bench(args):
    runner = Runner(args.adb is not None, args.adb or None, args.cpu)
    argv = ["--full", "--samples", str(args.samples), "--min-ms", str(args.min_ms)]
    if args.filter:
        argv += ["--filter", args.filter]
    rounds = {n: [] for n in args.names}
    for r in range(args.rounds):
        order = args.names if r % 2 == 0 else list(reversed(args.names))
        for n in order:
            code, out, err = runner.run(n, "mathlibbench", argv, args.timeout)
            if code:
                fail("%s bench failed (%d): %s" % (n, code, err[-2000:]))
            rounds[n].append(parse_bench(out))
        print("round %d/%d done" % (r + 1, args.rounds), file=sys.stderr)

    base = args.names[0]
    names = list(rounds[base][0].keys())
    table = []
    header = "%-40s" % "kernel" + "".join("%14s" % n[-14:] for n in args.names) + "".join("%9s" % ("x " + n[-6:]) for n in args.names[1:]) + "  same"
    print(header)
    for k in names:
        med = {n: statistics.median(rr[k]["ns"] for rr in rounds[n]) for n in args.names}
        checks = {n: rounds[n][0][k]["check"] for n in args.names}
        same = len(set(checks.values())) == 1
        row = {"kernel": k, "ns_per_op": med, "speedup_vs_" + base: {n: med[base] / med[n] for n in args.names[1:]}, "checks": checks, "checks_equal": same}
        table.append(row)
        print("%-40s" % k + "".join("%14.3f" % med[n] for n in args.names) + "".join("%9.2f" % (med[base] / med[n]) for n in args.names[1:]) + "  " + ("yes" if same else "no"))
    evidence = {"schema": "mathlib-bench/v1", "names": args.names, "device": "adb" if args.adb is not None else "local", "cpu": args.cpu, "rounds": args.rounds,
                "samples": args.samples, "min_ms": args.min_ms, "kernels": table,
                "builds": {n: json.load(open(os.path.join(OUT_ROOT, n, "build.json"))) for n in args.names}}
    path = os.path.join(OUT_ROOT, "bench-" + "-vs-".join(args.names) + ".json")
    with open(path, "w") as f:
        json.dump(evidence, f, indent=1)
    print("evidence: " + os.path.relpath(path, ROOT))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build")
    b.add_argument("--tree", required=True)
    b.add_argument("--name")
    b.add_argument("--extra-flags")
    b.add_argument("--mathlib-rev")
    c = sub.add_parser("check")
    c.add_argument("--name", required=True)
    c.add_argument("--adb", nargs="?", const="")
    c.add_argument("--seeds", type=int, default=3)
    c.add_argument("--timeout", type=int, default=300)
    r = sub.add_parser("bench")
    r.add_argument("names", nargs="+")
    r.add_argument("--adb", nargs="?", const="")
    r.add_argument("--rounds", type=int, default=5)
    r.add_argument("--samples", type=int, default=7)
    r.add_argument("--min-ms", type=float, default=2.0)
    r.add_argument("--filter")
    r.add_argument("--cpu", type=int, help="pin to this CPU (taskset)")
    r.add_argument("--timeout", type=int, default=900)
    args = p.parse_args()
    {"build": cmd_build, "check": cmd_check, "bench": cmd_bench}[args.cmd](args)


if __name__ == "__main__":
    main()
