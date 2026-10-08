#!/usr/bin/env python3
"""R103 gate for one built Tier 0 (RFC 0001 "Tier 0 facade over the foundation
providers"): the export fixture, the kept mod-fixture binary loaded into it,
and the OS-call ratchet, reported as one checks-v1 record.

  tools/quality/tier0_facade.py check --tier0 build/tier0 [--platform linux-x86_64]
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FIXTURE = os.path.join(ROOT, "quality", "fixtures", "tier0-abi", "mod-fixture")
RESULT = re.compile(r"^CONFORMANCE (\d+) (\d+)$", re.M)


def run(argv, env=None):
    result = subprocess.run(argv, capture_output=True, text=True, env=env)
    output = result.stdout + result.stderr
    match = RESULT.search(result.stdout)
    checks, failures = (int(match.group(1)), int(match.group(2))) if match else (1, 1)
    if result.returncode != 0 and failures == 0:
        failures = 1
    return checks, failures, output


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check"])
    parser.add_argument("--tier0", required=True, help="directory holding the built libtier0.so")
    parser.add_argument("--platform", default="linux-x86_64")
    args = parser.parse_args()
    tier0_dir = os.path.abspath(args.tier0)
    lib = os.path.join(tier0_dir, "libtier0.so")
    total_checks = total_failures = 0

    parts = []
    parts.append(("exports", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_abi.py"), "check",
                                  "--lib", lib, "--platform", args.platform])))
    parts.append(("exports.selftest", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_abi.py"),
                                           "selftest", "--lib", lib, "--platform", args.platform])))
    with tempfile.TemporaryDirectory() as scratch:
        host = os.path.join(scratch, "modhost")
        build = subprocess.run(["g++", "-std=c++20", "-I", os.path.join(ROOT, "public"), "-I", ROOT,
                                os.path.join(FIXTURE, "host.cpp"),
                                os.path.join(ROOT, "platform/posix/dynamic_library_provider.cpp"),
                                "-ldl", "-o", host], capture_output=True, text=True)
        if build.returncode != 0:
            parts.append(("mod-fixture", (1, 1, build.stderr)))
        else:
            env = dict(os.environ, LD_LIBRARY_PATH=tier0_dir)
            parts.append(("mod-fixture", run([host, lib, os.path.join(FIXTURE, args.platform,
                                                                      "libmod_fixture.so")], env)))
        oracle = os.path.join(scratch, "cmdline_oracle")
        build = subprocess.run(["g++", "-std=c++20", "-I", os.path.join(ROOT, "public"), "-I", ROOT,
                                os.path.join(FIXTURE, "cmdline_oracle.cpp"),
                                os.path.join(ROOT, "platform/posix/dynamic_library_provider.cpp"),
                                "-ldl", "-o", oracle], capture_output=True, text=True)
        if build.returncode != 0:
            parts.append(("cmdline-oracle", (1, 1, build.stderr)))
        else:
            env = dict(os.environ, LD_LIBRARY_PATH=tier0_dir)
            parts.append(("cmdline-oracle", run([oracle, lib], env)))
    parts.append(("ratchet", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_ratchet.py"), "check"])))
    parts.append(("ratchet.selftest", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_ratchet.py"),
                                           "selftest"])))
    for name, (checks, failures, output) in parts:
        total_checks += checks
        total_failures += failures
        print("%s %s: %d checks, %d failures" % ("FAIL" if failures else "ok", name, checks, failures))
        if failures:
            sys.stdout.write("".join("    " + l + "\n" for l in output.splitlines()[-15:]))
    print("CONFORMANCE %d %d" % (total_checks, total_failures))
    return 1 if total_failures else 0


if __name__ == "__main__":
    sys.exit(main())
