#!/usr/bin/env python3
"""Android lane of the R103 Tier 0 gate (RFC 0001 "Tier 0 facade over the
foundation providers").

Checks one built arm64 libtier0.so the way tier0_facade.py checks a Linux one:
the export fixture on the host (with the NDK's llvm-nm), then on the device the
kept mod-fixture binary loaded into it and the behavior oracle. Both programs
are cross-built with the NDK pinned in the Android product profile.

  tools/quality/android_tier0.py check --tier0 <dir with libtier0.so> [--device SERIAL]

Exits 3 when no device is attached: the profile is then unverified, never
passed (AGENTS.md: missing required device tests leave it unverified).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PROFILE = os.path.join(ROOT, "quality", "product_profiles", "portal-android-native-vulkan.json")
FIXTURE = os.path.join(ROOT, "quality", "fixtures", "tier0-abi", "mod-fixture")
PLATFORM = "android-arm64-v8a"
ABI = "arm64-v8a"
REMOTE = "/data/local/tmp/r103_tier0"
RESULT = re.compile(r"^CONFORMANCE (\d+) (\d+)$", re.M)


def ndk_bin():
    with open(PROFILE, encoding="utf-8") as handle:
        profile = json.load(handle)
    ndk = profile["dependencies"]["ndk"]
    bindir = os.path.join(ROOT, "dependencies", "android", ndk["extracted_directory"],
                          "toolchains", "llvm", "prebuilt", "linux-x86_64", "bin")
    return bindir, profile["android"]["ndk_triple"][ABI], profile["android"]["min_sdk"]


def run(cmd, timeout=600, **kwargs):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, **kwargs)


def adb(serial, *args, timeout=300):
    return run(["adb"] + (["-s", serial] if serial else []) + list(args), timeout=timeout)


def devices():
    result = run(["adb", "devices"])
    return [line.split()[0] for line in result.stdout.splitlines()[1:]
            if line.strip().endswith("device")]


def parse(output):
    match = RESULT.search(output)
    if not match:
        return 1, 1
    return int(match.group(1)), int(match.group(2))


def check(args):
    bindir, triple, api = ndk_bin()
    if not os.path.isdir(bindir):
        print("FATAL: pinned NDK not extracted at %s" % bindir, file=sys.stderr)
        return 1
    lib = os.path.join(os.path.abspath(args.tier0), "libtier0.so")
    parts = []
    exports = run([sys.executable, os.path.join(ROOT, "tools/quality/tier0_abi.py"), "check", "--lib", lib,
                   "--platform", PLATFORM, "--nm", os.path.join(bindir, "llvm-nm")])
    parts.append(("exports", parse(exports.stdout), exports.stdout))

    attached = devices()
    serial = args.device or (attached[0] if attached else None)
    if serial is None or serial not in attached:
        print("UNAVAILABLE: no Android device attached; the android profile stays unverified")
        return 3
    abi = adb(serial, "shell", "getprop", "ro.product.cpu.abi").stdout.strip()
    if abi != ABI:
        print("FAIL: device ABI %r is not %s" % (abi, ABI))
        return 1

    cxx = os.path.join(bindir, "%s%d-clang++" % (triple, api))
    libcxx = os.path.join(os.path.dirname(bindir), "sysroot", "usr", "lib", "aarch64-linux-android",
                          "libc++_shared.so")
    with tempfile.TemporaryDirectory() as scratch:
        programs = {}
        for name in ("host", "behavior_oracle"):
            out = os.path.join(scratch, name)
            build = run([cxx, "-std=c++20", "-static-libstdc++", "-I", os.path.join(ROOT, "public"), "-I", ROOT,
                         os.path.join(FIXTURE, name + ".cpp"),
                         os.path.join(ROOT, "platform/posix/dynamic_library_provider.cpp"), "-ldl", "-o", out])
            if build.returncode != 0:
                print("FAIL build %s:\n%s" % (name, build.stderr[-4000:]))
                return 1
            programs[name] = out
        adb(serial, "shell", "rm -rf %s && mkdir -p %s" % (REMOTE, REMOTE))
        for path in (lib, libcxx, os.path.join(FIXTURE, PLATFORM, "libmod_fixture.so"), *programs.values()):
            push = adb(serial, "push", path, REMOTE + "/")
            if push.returncode != 0:
                print("FAIL push %s: %s" % (path, push.stderr.strip()))
                return 1
    env = "cd %s && chmod 755 host behavior_oracle && LD_LIBRARY_PATH=%s" % (REMOTE, REMOTE)
    host = adb(serial, "shell", "%s ./host %s/libtier0.so %s/libmod_fixture.so; echo EXIT=$?"
               % (env, REMOTE, REMOTE), timeout=600)
    parts.append(("mod-fixture", parse(host.stdout), host.stdout))
    oracle = adb(serial, "shell", "%s ./behavior_oracle %s/libtier0.so; echo EXIT=$?" % (env, REMOTE),
                 timeout=600)
    parts.append(("behavior-oracle", parse(oracle.stdout), oracle.stdout))
    adb(serial, "shell", "rm", "-rf", REMOTE)

    total_checks = total_failures = 0
    for name, (checks, failures), output in parts:
        exit_code = re.search(r"EXIT=(\d+)", output)
        if exit_code is not None and exit_code.group(1) != "0" and failures == 0:
            failures = 1
        total_checks += checks
        total_failures += failures
        print("%s %s: %d checks, %d failures" % ("FAIL" if failures else "ok", name, checks, failures))
        if failures:
            sys.stdout.write("".join("    " + l + "\n" for l in output.splitlines()[-15:]))
    print("android(%s, %s): %s" % (serial, abi, "PASS" if total_failures == 0 else "FAIL"))
    print("CONFORMANCE %d %d" % (total_checks, total_failures))
    return 0 if total_failures == 0 else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check"])
    parser.add_argument("--tier0", required=True, help="directory holding the built arm64 libtier0.so")
    parser.add_argument("--device", default=None)
    args = parser.parse_args()
    return check(args)


if __name__ == "__main__":
    sys.exit(main())
