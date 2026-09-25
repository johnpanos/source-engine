#!/usr/bin/env python3
"""Windows/Wine parity harness for RFC 0002 strict conformance suites.

The strict Hammer editor modules have no MFC/tier0/Windows dependency, so they
must behave identically on the Windows target and the Linux headless core. This
harness reuses the SAME suite definitions as the shared runner
(quality/conformance.manifest.json), builds each selected suite as a static
Windows PE with the MinGW cross-compiler, runs it under Wine, and checks the
outcome against the suite's declared expectation (the same expectation the native
gcc/clang run satisfies). A mismatch is a parity break.

This is intentionally a thin sibling of tools/quality/conformance.py: it does not
modify the shared runner. It exists so a profile that must be executed through a
wrapper (wine) and cross-compiled has a home without complicating the native gate.

Usage:
  tools/quality/parity_wine.py check [--rfc 0002] [--domain Q-EDITOR] \
      [--cxx x86_64-w64-mingw32-g++] [--wrapper wine] [--out -] \
      [--jobs N] [--launcher ccache] [--build-dir DIR]

With --jobs > 1 every suite is compiled first, concurrently; the Wine runs
then happen one at a time in manifest order. When the wrapper is Wine, one
persistent wineserver serves the whole run instead of a server per suite.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402


def repo_root():
    return os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def load_json(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def _flags(profile, suite, static):
    flags = ["-std=" + profile["cxx_std"]]
    flags += list(profile.get("base_flags", []))
    flags += list(suite.get("extra_flags", []))
    if static:
        # Self-contained PE so Wine needs no sibling MinGW runtime DLLs.
        flags += ["-static", "-static-libgcc", "-static-libstdc++"]
    return flags


def _includes(root, profile):
    includes = []
    for inc in profile.get("include_roots", []):
        includes += ["-I", os.path.join(root, inc)]
    return includes


def build_command(root, cxx, profile, suite, out_bin, static):
    sources = [os.path.join(root, s) for s in suite["sources"]]
    return [cxx, *_flags(profile, suite, static), *_includes(root, profile), *sources,
            "-o", out_bin]


def build_commands(root, cxx, profile, suite, out_bin, static, launcher=None):
    """The build as commands. With a compiler launcher (ccache) each source is
    compiled on its own and then linked, the shape a compiler cache accepts;
    without one it is the single build_command()."""
    if not launcher:
        return [build_command(root, cxx, profile, suite, out_bin, static)]
    flags = _flags(profile, suite, static)
    includes = _includes(root, profile)
    commands, objects = [], []
    for index, source in enumerate(suite["sources"]):
        obj = "%s.%d.o" % (out_bin, index)
        objects.append(obj)
        commands.append([launcher, cxx, *flags, *includes, "-c", os.path.join(root, source),
                         "-o", obj])
    commands.append([launcher, cxx, *flags, *objects, "-o", out_bin])
    return commands


def new_record(suite):
    return {"id": suite["id"], "expect": suite.get("expect", "pass"), "outcome": None,
            "matched": False, "exit_code": None, "detail": None, "repro": None}


def build_one(root, cxx, profile, suite, out_dir, launcher=None):
    """Compile one suite. Returns (record, exe): exe is None when the record is
    already final (missing source or compile error)."""
    record = new_record(suite)
    missing = [s for s in suite["sources"] if not os.path.exists(os.path.join(root, s))]
    if missing:
        record["outcome"] = "missing-source"
        record["detail"] = "missing: " + ", ".join(missing)
        return record, None

    out_bin = os.path.join(out_dir, suite["id"].replace("/", "_").replace(".", "_") + ".exe")
    commands = build_commands(root, cxx, profile, suite, out_bin, static=True, launcher=launcher)
    record["repro"] = " && ".join(" ".join(cmd) for cmd in commands)
    for cmd in commands:
        build = subprocess.run(cmd, capture_output=True, text=True, check=False)
        if build.returncode != 0:
            record["outcome"] = "compile-error"
            record["detail"] = (build.stderr or "").strip().splitlines()[-1:] or None
            return record, None
    return record, out_bin


def run_built(suite, record, out_bin, wrapper, timeout):
    """Run a compiled suite under the wrapper and classify it."""
    env = dict(os.environ)
    env["WINEDEBUG"] = "-all"
    run_cmd = ([*wrapper, out_bin] if wrapper else [out_bin])
    # Output goes to named files, not pipes: Wine's helper processes
    # (wineserver, services, winedevice) inherit the child's stdout and hold a
    # pipe open for seconds after the suite exits, and the run would wait for
    # them. The files must have a path; Wine cannot use an unlinked one.
    started = time.monotonic()
    with tempfile.NamedTemporaryFile(prefix="parity-", suffix=".out") as out, \
            tempfile.NamedTemporaryFile(prefix="parity-", suffix=".err") as err:
        try:
            run = subprocess.run(run_cmd, stdout=out, stderr=err, check=False,
                                 timeout=timeout, env=env)
        except subprocess.TimeoutExpired:
            record["outcome"] = "timeout"
            return record
        out.seek(0)
        # Universal newlines, as text=True applied: PE output ends lines in CRLF.
        stdout = out.read().decode("utf-8", "replace").replace("\r\n", "\n").replace("\r", "\n")
    record["duration_s"] = round(time.monotonic() - started, 3)
    # One result-protocol implementation: the native runner's classifier also
    # rejects a missing, duplicate, zero-check or failing checks-v1 record here.
    observed = conformance.classify_run(suite, run.returncode, stdout)
    record["exit_code"] = run.returncode
    record["outcome"] = observed["outcome"]
    record["checks"] = observed["checks"]
    tail = stdout.strip().splitlines()
    record["detail"] = observed["first_divergence"] or (tail[-1] if tail else None)
    record["matched"] = (record["outcome"] == record["expect"])
    return record


def run_one(root, cxx, wrapper, profile, suite, out_dir, timeout, launcher=None):
    record, out_bin = build_one(root, cxx, profile, suite, out_dir, launcher)
    if out_bin is None:
        return record
    return run_built(suite, record, out_bin, wrapper, timeout)


def start_wineserver(wrapper, linger):
    """Keep one wineserver alive for the run (it exits `linger` seconds after
    its last client), so each suite does not start and stop its own. Returns
    a description for the log, or None when the wrapper is not Wine."""
    if not wrapper or os.path.basename(wrapper[0]) not in ("wine", "wine64"):
        return None
    server = shutil.which("wineserver")
    if server is None:
        return None
    # A server already running for this prefix is reused; this call then
    # exits nonzero, which is not an error. The daemon inherits the standard
    # streams, so they must not be pipes this process would wait on.
    subprocess.run([server, "-p%d" % linger], stdin=subprocess.DEVNULL,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    return "%s -p%d" % (server, linger)


def cmd_check(args):
    root = repo_root()
    manifest = load_json(os.path.join(root, "quality/conformance.manifest.json"))
    profiles_dir = os.path.join(root, manifest.get("profiles_dir", "quality/profiles"))
    if args.jobs < 1:
        print("FATAL: --jobs must be at least 1", file=sys.stderr)
        return 2
    if args.launcher and shutil.which(args.launcher) is None:
        print("FATAL: compiler launcher %r is unavailable" % args.launcher, file=sys.stderr)
        return 2
    out_dir = args.build_dir or os.path.join(root, "build", "parity-wine")
    os.makedirs(out_dir, exist_ok=True)

    profile_cache = {}
    def profile_for(pid):
        if pid not in profile_cache:
            profile_cache[pid] = conformance.load_profile(profiles_dir, pid)
        return profile_cache[pid]

    # Mixed-dialect unit suites prove same-toolchain ABI combinations on their
    # native profile; they are not Windows parity suites.
    suites = [
        s for s in manifest["suites"]
        if s.get("units") is None
        and (args.rfc is None or s.get("rfc") == args.rfc)
        and (args.domain is None or s.get("domain") == args.domain)
    ]

    if not suites:
        # Zero discovery certifies nothing (RFC 0005 runner contract).
        print("FATAL: no suite matches rfc=%s domain=%s" % (args.rfc, args.domain),
              file=sys.stderr)
        return 2

    print("parity(wine): %d suite(s) selected; cxx=%s wrapper=%s jobs=%d%s"
          % (len(suites), args.cxx, args.wrapper or "(none)", args.jobs,
             " launcher=" + args.launcher if args.launcher else ""))
    wrapper = args.wrapper.split() if args.wrapper else []
    started = time.monotonic()
    built = {}
    if args.jobs > 1:
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
            futures = {pool.submit(build_one, root, args.cxx, profile_for(s["profile"]), s,
                                   out_dir, args.launcher): s["id"] for s in suites}
            for future in concurrent.futures.as_completed(futures):
                built[futures[future]] = future.result()
        print("parity(wine): built %d suite(s) on %d job(s) in %.1fs"
              % (len(built), args.jobs, time.monotonic() - started))
    server = start_wineserver(wrapper, args.wineserver_linger)
    if server:
        print("parity(wine): persistent server: %s" % server)
    results = []
    for suite in suites:
        profile = profile_for(suite["profile"])
        if suite["id"] in built:
            record, out_bin = built[suite["id"]]
            if out_bin is not None:
                record = run_built(suite, record, out_bin, wrapper, args.timeout)
        else:
            record = run_one(root, args.cxx, wrapper, profile, suite, out_dir, args.timeout,
                             args.launcher)
        results.append(record)
        flag = "ok  " if record["matched"] else record["outcome"].upper()
        print("  [%-4s] %-42s expect=%-6s got=%-14s" %
              (flag if record["matched"] else "FAIL", record["id"], record["expect"],
               record["outcome"]))
        if not record["matched"] and record.get("detail"):
            print("           %s" % (record["detail"],))

    matched = sum(1 for r in results if r["matched"])
    total = len(results)
    verdict = "PASS" if matched == total else "FAIL"
    print("\n%d suite(s): %d matched, %d mismatched -> %s (%.1fs)"
          % (total, matched, total - matched, verdict, time.monotonic() - started))

    if args.out and args.out != "-":
        evidence = {"harness": "parity_wine/v1", "cxx": args.cxx, "wrapper": args.wrapper,
                    "jobs": args.jobs, "launcher": args.launcher, "wineserver": server,
                    "matched": matched, "total": total, "results": results}
        with open(args.out, "w", encoding="utf-8") as handle:
            json.dump(evidence, handle, indent=2)
    return 0 if verdict == "PASS" else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    check = sub.add_parser("check", help="build+run suites as Windows PE under Wine")
    check.add_argument("--rfc", default="0002")
    check.add_argument("--domain", default="Q-EDITOR")
    check.add_argument("--cxx", default="x86_64-w64-mingw32-g++")
    check.add_argument("--wrapper", default="wine")
    check.add_argument("--timeout", type=int, default=120)
    check.add_argument("--out", default="-")
    check.add_argument("--jobs", type=int, default=1,
                       help="compile suites on N concurrent jobs first; runs stay serial")
    check.add_argument("--launcher", default=os.environ.get("CONFORMANCE_LAUNCHER") or None,
                       help="compiler launcher such as ccache (default: $CONFORMANCE_LAUNCHER)")
    check.add_argument("--build-dir", default=None,
                       help="directory for the PE binaries (default: build/parity-wine)")
    check.add_argument("--wineserver-linger", type=int, default=30,
                       help="seconds the shared wineserver stays up after its last client")
    check.set_defaults(func=cmd_check)
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
