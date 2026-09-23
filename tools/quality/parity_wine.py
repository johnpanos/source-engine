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
      [--cxx x86_64-w64-mingw32-g++] [--wrapper wine] [--out -]
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import conformance  # noqa: E402


def repo_root():
    return os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def load_json(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def build_command(root, cxx, profile, suite, out_bin, static):
    flags = ["-std=" + profile["cxx_std"]]
    flags += list(profile.get("base_flags", []))
    flags += list(suite.get("extra_flags", []))
    if static:
        # Self-contained PE so Wine needs no sibling MinGW runtime DLLs.
        flags += ["-static", "-static-libgcc", "-static-libstdc++"]
    includes = []
    for inc in profile.get("include_roots", []):
        includes += ["-I", os.path.join(root, inc)]
    sources = [os.path.join(root, s) for s in suite["sources"]]
    return [cxx, *flags, *includes, *sources, "-o", out_bin]


def run_one(root, cxx, wrapper, profile, suite, out_dir, timeout):
    sid = suite["id"]
    expect = suite.get("expect", "pass")
    record = {"id": sid, "expect": expect, "outcome": None, "matched": False,
              "exit_code": None, "detail": None, "repro": None}

    missing = [s for s in suite["sources"] if not os.path.exists(os.path.join(root, s))]
    if missing:
        record["outcome"] = "missing-source"
        record["detail"] = "missing: " + ", ".join(missing)
        return record

    out_bin = os.path.join(out_dir, sid.replace("/", "_").replace(".", "_") + ".exe")
    cmd = build_command(root, cxx, profile, suite, out_bin, static=True)
    record["repro"] = " ".join(cmd)
    build = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if build.returncode != 0:
        record["outcome"] = "compile-error"
        record["detail"] = (build.stderr or "").strip().splitlines()[-1:] or None
        return record

    env = dict(os.environ)
    env["WINEDEBUG"] = "-all"
    run_cmd = ([*wrapper, out_bin] if wrapper else [out_bin])
    try:
        run = subprocess.run(run_cmd, capture_output=True, text=True, check=False,
                             timeout=timeout, env=env)
    except subprocess.TimeoutExpired:
        record["outcome"] = "timeout"
        return record
    # One result-protocol implementation: the native runner's classifier also
    # rejects a missing, duplicate, zero-check or failing checks-v1 record here.
    observed = conformance.classify_run(suite, run.returncode, run.stdout or "")
    record["exit_code"] = run.returncode
    record["outcome"] = observed["outcome"]
    record["checks"] = observed["checks"]
    tail = (run.stdout or "").strip().splitlines()
    record["detail"] = observed["first_divergence"] or (tail[-1] if tail else None)
    record["matched"] = (record["outcome"] == expect)
    return record


def cmd_check(args):
    root = repo_root()
    manifest = load_json(os.path.join(root, "quality/conformance.manifest.json"))
    profiles_dir = os.path.join(root, manifest.get("profiles_dir", "quality/profiles"))
    out_dir = os.path.join(root, "build", "parity-wine")
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

    print("parity(wine): %d suite(s) selected; cxx=%s wrapper=%s"
          % (len(suites), args.cxx, args.wrapper or "(none)"))
    results = []
    for suite in suites:
        profile = profile_for(suite["profile"])
        record = run_one(root, args.cxx, args.wrapper.split() if args.wrapper else [],
                         profile, suite, out_dir, args.timeout)
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
    print("\n%d suite(s): %d matched, %d mismatched -> %s"
          % (total, matched, total - matched, verdict))

    if args.out and args.out != "-":
        evidence = {"harness": "parity_wine/v1", "cxx": args.cxx, "wrapper": args.wrapper,
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
    check.set_defaults(func=cmd_check)
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
