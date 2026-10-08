#!/usr/bin/env python3
"""Android lane for the RFC 0001 foundation providers (R26).

Builds unittests/platformtest/foundation_posix with the NDK pinned in the
Android product profile, for every ABI the profile declares, at its min_sdk.
`check` then runs the binary for the attached device's ABI over adb and
requires its checks-v1 record to pass, and requires the logcat provider's
three records at their priorities.

  tools/quality/android_foundation.py build             # cross-build only
  tools/quality/android_foundation.py check [--device SERIAL]

`check` exits 3 when no device is attached: the profile is then unverified,
never passed (AGENTS.md: missing required device tests leave it unverified).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PROFILE = os.path.join(ROOT, "quality", "product_profiles", "portal-android-native-vulkan.json")
SOURCES = ["unittests/platformtest/foundation_posix/test_foundation_posix.cpp"] + [
    "platform/posix/%s.cpp" % name for name in (
        "clock_providers", "thread_provider", "virtual_memory_provider",
        "process_environment_provider", "paths_provider", "diagnostics_provider")]
REMOTE = "/data/local/tmp/r26_foundation"
RESULT = re.compile(r"^CONFORMANCE (\d+) (\d+)$", re.M)


def load_profile():
    with open(PROFILE, encoding="utf-8") as handle:
        profile = json.load(handle)
    ndk = profile["dependencies"]["ndk"]
    ndk_dir = os.path.join(ROOT, "dependencies", "android", ndk["extracted_directory"])
    return profile, ndk_dir


def build(out_dir):
    profile, ndk_dir = load_profile()
    bindir = os.path.join(ndk_dir, "toolchains", "llvm", "prebuilt", "linux-x86_64", "bin")
    if not os.path.isdir(bindir):
        print("FATAL: pinned NDK not extracted at %s" % ndk_dir, file=sys.stderr)
        return None
    api = profile["android"]["min_sdk"]
    os.makedirs(out_dir, exist_ok=True)
    built = {}
    for abi, triple in sorted(profile["android"]["ndk_triple"].items()):
        cxx = os.path.join(bindir, "%s%d-clang++" % (triple, api))
        out = os.path.join(out_dir, "r26_foundation-%s" % abi)
        cmd = [cxx, "-std=c++20", "-Wall", "-Wextra", "-Werror", "-static-libstdc++",
               "-I", os.path.join(ROOT, "public"), "-I", ROOT,
               *[os.path.join(ROOT, s) for s in SOURCES], "-llog", "-o", out]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print("FAIL build %s:\n%s" % (abi, result.stderr[-4000:]))
            return None
        print("ok build %s (API %d): %s" % (abi, api, os.path.relpath(out, ROOT)))
        built[abi] = out
    return built


def adb(serial, *args, timeout=300):
    cmd = ["adb"] + (["-s", serial] if serial else []) + list(args)
    return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)


def devices():
    result = subprocess.run(["adb", "devices"], capture_output=True, text=True)
    return [line.split()[0] for line in result.stdout.splitlines()[1:]
            if line.strip().endswith("device")]


def check(args):
    built = build(args.out)
    if built is None:
        return 1
    attached = devices()
    serial = args.device or (attached[0] if attached else None)
    if serial is None or serial not in attached:
        print("UNAVAILABLE: no Android device attached; the android profile stays unverified")
        return 3
    abi = adb(serial, "shell", "getprop", "ro.product.cpu.abi").stdout.strip()
    if abi not in built:
        print("FAIL: device ABI %r is not one the profile declares (%s)" % (abi, ", ".join(built)))
        return 1
    adb(serial, "logcat", "-c")
    push = adb(serial, "push", built[abi], REMOTE)
    if push.returncode != 0:
        print("FAIL push: %s" % push.stderr.strip())
        return 1
    run = adb(serial, "shell", "chmod 755 %s && %s; echo EXIT=$?" % (REMOTE, REMOTE), timeout=600)
    output = run.stdout
    sys.stdout.write(output)
    adb(serial, "shell", "rm", "-f", REMOTE)
    match = RESULT.search(output)
    exit_code = re.search(r"EXIT=(\d+)", output)
    ok = bool(match) and match.group(2) == "0" and int(match.group(1)) > 0 and \
        exit_code is not None and exit_code.group(1) == "0"
    # The logcat provider's records, at their priorities, under tag r26.
    log = adb(serial, "logcat", "-d", "-s", "r26:V", "-v", "brief").stdout
    for priority, text in (("I", "r26 info"), ("W", "r26 warn"), ("E", "r26 error")):
        found = re.search(r"^%s/r26\s*\(\s*\d+\):\s*%s\s*$" % (priority, re.escape(text)), log, re.M)
        print("%s logcat %s/%s" % ("ok" if found else "FAIL", priority, text))
        ok = ok and bool(found)
    print("android(%s, %s): %s" % (serial, abi, "PASS" if ok else "FAIL"))
    return 0 if ok else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("build", "check"):
        p = sub.add_parser(name)
        p.add_argument("--out", default=os.path.join(ROOT, "build", "android-foundation"))
        if name == "check":
            p.add_argument("--device", default=None)
    args = parser.parse_args()
    if args.command == "build":
        return 0 if build(args.out) is not None else 1
    return check(args)


if __name__ == "__main__":
    sys.exit(main())
