#!/usr/bin/env python3
"""R103 gate for one built Tier 0 (RFC 0001 "Tier 0 facade over the foundation
providers"): the export fixture, the kept mod-fixture binary loaded into it,
and the OS-call ratchet, reported as one checks-v1 record.

  tools/quality/tier0_facade.py check --tier0 build/tier0 [--platform linux-x86_64]
  tools/quality/tier0_facade.py check --tier0 out/dedicated-windows/dev/build/tier0 \
      --platform windows-x86_64 [--wineprefix DIR]

windows-x86_64 checks tier0.dll: the export table, then the kept MSVC mod DLL
(host_win32.cpp) and the Win32 behavior oracle (behavior_oracle_win32.cpp),
both built with MinGW and run under Wine.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
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


# The compiler that builds the host and the behavior oracle for each platform
# whose Tier 0 runs on this machine. Android has its own lane
# (tools/quality/android_tier0.py), which pushes the same programs to a device.
HOST_CXX = {
    "linux-x86_64": ["g++"],
    "linux-i386": ["g++", "-m32"],
}


def check_windows(tier0_dir, wineprefix):
    lib = os.path.join(tier0_dir, "tier0.dll")
    parts = []
    abi = os.path.join(ROOT, "tools/quality/tier0_abi.py")
    parts.append(("exports", run([sys.executable, abi, "check", "--lib", lib, "--platform", "windows-x86_64"])))
    parts.append(("exports.selftest", run([sys.executable, abi, "selftest", "--lib", lib,
                                           "--platform", "windows-x86_64"])))
    env = dict(os.environ, WINEDEBUG="-all")
    if wineprefix:
        env["WINEPREFIX"] = os.path.abspath(wineprefix)
    with tempfile.TemporaryDirectory() as scratch:
        shutil.copy(lib, scratch)
        shutil.copy(os.path.join(FIXTURE, "windows-x86_64", "mod_fixture.dll"), scratch)
        for name, label, argv in (("host_win32", "mod-fixture", ["tier0.dll", "mod_fixture.dll"]),
                                  ("behavior_oracle_win32", "behavior-oracle", ["tier0.dll"])):
            exe = os.path.join(scratch, name + ".exe")
            build = subprocess.run(["x86_64-w64-mingw32-g++", "-std=c++20", "-O2", "-static",
                                    os.path.join(FIXTURE, name + ".cpp"), "-o", exe],
                                   capture_output=True, text=True)
            if build.returncode != 0:
                parts.append((label, (1, 1, build.stderr)))
                continue
            result = subprocess.run(["wine", exe] + argv, capture_output=True, text=True, env=env, cwd=scratch,
                                    timeout=300)
            output = result.stdout
            match = RESULT.search(output)
            checks, failures = (int(match.group(1)), int(match.group(2))) if match else (1, 1)
            if result.returncode != 0 and failures == 0:
                failures = 1
            parts.append((label, (checks, failures, output + result.stderr[-2000:])))
    return parts


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check"])
    parser.add_argument("--tier0", required=True, help="directory holding the built libtier0.so")
    parser.add_argument("--platform", default="linux-x86_64", choices=sorted(HOST_CXX) + ["windows-x86_64"])
    parser.add_argument("--wineprefix", default=None, help="windows-x86_64: the Wine prefix to run in")
    args = parser.parse_args()
    tier0_dir = os.path.abspath(args.tier0)
    lib = os.path.join(tier0_dir, "libtier0.so")
    total_checks = total_failures = 0

    if args.platform == "windows-x86_64":
        parts = check_windows(tier0_dir, args.wineprefix)
        parts.append(("ratchet", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_ratchet.py"),
                                      "check"])))
        return report(parts)

    parts = []
    parts.append(("exports", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_abi.py"), "check",
                                  "--lib", lib, "--platform", args.platform])))
    parts.append(("exports.selftest", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_abi.py"),
                                           "selftest", "--lib", lib, "--platform", args.platform])))
    with tempfile.TemporaryDirectory() as scratch:
        host = os.path.join(scratch, "modhost")
        build = subprocess.run(HOST_CXX[args.platform] + ["-std=c++20", "-I", os.path.join(ROOT, "public"), "-I", ROOT,
                                os.path.join(FIXTURE, "host.cpp"),
                                os.path.join(ROOT, "platform/posix/dynamic_library_provider.cpp"),
                                "-ldl", "-o", host], capture_output=True, text=True)
        if build.returncode != 0:
            parts.append(("mod-fixture", (1, 1, build.stderr)))
        else:
            env = dict(os.environ, LD_LIBRARY_PATH=tier0_dir)
            parts.append(("mod-fixture", run([host, lib, os.path.join(FIXTURE, args.platform,
                                                                      "libmod_fixture.so")], env)))
        oracle = os.path.join(scratch, "behavior_oracle")
        build = subprocess.run(HOST_CXX[args.platform] + ["-std=c++20", "-I", os.path.join(ROOT, "public"), "-I", ROOT,
                                os.path.join(FIXTURE, "behavior_oracle.cpp"),
                                os.path.join(ROOT, "platform/posix/dynamic_library_provider.cpp"),
                                "-ldl", "-o", oracle], capture_output=True, text=True)
        if build.returncode != 0:
            parts.append(("behavior-oracle", (1, 1, build.stderr)))
        else:
            env = dict(os.environ, LD_LIBRARY_PATH=tier0_dir)
            parts.append(("behavior-oracle", run([oracle, lib], env)))
    parts.append(("ratchet", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_ratchet.py"), "check"])))
    parts.append(("ratchet.selftest", run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_ratchet.py"),
                                           "selftest"])))
    return report(parts)


def report(parts):
    total_checks = total_failures = 0
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
