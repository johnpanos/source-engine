#!/usr/bin/env python3
"""Android lane for the SDL3 window/input provider (RFC 0001 R14, R18).

Builds platform.window.sdl3's program (the SDL3 provider against the shared
platform.window.v1 suite, from that manifest row's sources) with the NDK pinned in
the Android product profile, for every ABI the profile declares, at its min_sdk,
linked against the SDL3 that build-android-apk.sh cross-built for that ABI.
`check` pushes the program and libSDL3.so for the attached device's ABI over adb
and runs it with SDL's offscreen video driver, so the provider's decoding,
normalization, surfaces, virtual gamepads and lifecycle run on the device's own
SDL; the activity-hosted window path is the APK's, judged by its own runs.

  tools/quality/android_window.py build             # cross-build only
  tools/quality/android_window.py check [--device SERIAL]

`check` exits 3 when no device is attached: the profile is then unverified,
never passed (AGENTS.md: missing required device tests leave it unverified).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import android_foundation as lane  # noqa: E402  (NDK, profile and adb helpers)

ROOT = lane.ROOT
ROW = "platform.window.sdl3"
REMOTE = "/data/local/tmp/r18_window"
RESULT = re.compile(r"^CONFORMANCE (\d+) (\d+)$", re.M)


def row_sources():
    with open(os.path.join(ROOT, "quality", "conformance.manifest.json"), encoding="utf-8") as f:
        manifest = json.load(f)
    suites = manifest["suites"] if isinstance(manifest, dict) else manifest
    return next(s for s in suites if s["id"] == ROW)["sources"]


def sdl_prefix(abi):
    """The newest completed cross-built dependency prefix kiln's android-ndk
    toolchain made for this ABI (dependencies/android-ndk/abi/<abi>-<key>/)."""
    import glob
    done = [d for d in glob.glob(os.path.join(ROOT, "dependencies", "android-ndk", "abi", abi + "-*"))
            if os.path.isfile(os.path.join(d, "complete"))]
    done.sort(key=os.path.getmtime)
    home = done[-1] if done else os.path.join(ROOT, "dependencies", "android-ndk", "abi", abi)
    return os.path.join(home, "deps", "prefix")


def build(out_dir):
    profile, ndk_dir = lane.load_profile()
    bindir = os.path.join(ndk_dir, "toolchains", "llvm", "prebuilt", "linux-x86_64", "bin")
    if not os.path.isdir(bindir):
        print("FATAL: pinned NDK not extracted at %s" % ndk_dir, file=sys.stderr)
        return None
    api = profile["android"]["min_sdk"]
    os.makedirs(out_dir, exist_ok=True)
    built = {}
    for abi, triple in sorted(profile["android"]["ndk_triple"].items()):
        prefix = sdl_prefix(abi)
        if not os.path.isfile(os.path.join(prefix, "lib", "libSDL3.so")):
            print("UNAVAILABLE build %s: no SDL3 at %s (run ./kiln build portal-android-native-vulkan --flavor %s)" % (abi, prefix, abi))
            continue
        cxx = os.path.join(bindir, "%s%d-clang++" % (triple, api))
        out = os.path.join(out_dir, "r18_window-%s" % abi)
        cmd = [cxx, "-std=c++20", "-O1", "-DNDEBUG", "-static-libstdc++",
               "-I", os.path.join(ROOT, "public"), "-I", os.path.join(ROOT, "public", "tier0"),
               "-I", os.path.join(ROOT, "public", "tier1"), "-I", os.path.join(prefix, "include"),
               *[os.path.join(ROOT, s) for s in row_sources()],
               "-L", os.path.join(prefix, "lib"), "-lSDL3", "-llog", "-o", out]
        result = lane.subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print("FAIL build %s:\n%s" % (abi, result.stderr[-4000:]))
            return None
        print("ok build %s (API %d): %s" % (abi, api, os.path.relpath(out, ROOT)))
        built[abi] = out
    return built or None


def check(args):
    built = build(args.out)
    if built is None:
        return 1
    attached = lane.devices()
    serial = args.device or (attached[0] if attached else None)
    if serial is None or serial not in attached:
        print("UNAVAILABLE: no Android device attached; the android profile stays unverified")
        return 3
    abi = lane.adb(serial, "shell", "getprop", "ro.product.cpu.abi").stdout.strip()
    if abi not in built:
        print("FAIL: device ABI %r is not one the profile declares (%s)" % (abi, ", ".join(built)))
        return 1
    lane.adb(serial, "shell", "mkdir", "-p", REMOTE)
    for local in (built[abi], os.path.join(sdl_prefix(abi), "lib", "libSDL3.so")):
        push = lane.adb(serial, "push", local, REMOTE + "/")
        if push.returncode != 0:
            print("FAIL push: %s" % push.stderr.strip())
            return 1
    program = "%s/%s" % (REMOTE, os.path.basename(built[abi]))
    run = lane.adb(serial, "shell", "cd %s && chmod 755 %s && SDL_VIDEODRIVER=offscreen "
                   "LD_LIBRARY_PATH=%s %s; echo EXIT=$?" % (REMOTE, program, REMOTE, program),
                   timeout=600)
    output = run.stdout
    sys.stdout.write(output)
    lane.adb(serial, "shell", "rm", "-rf", REMOTE)
    match = RESULT.search(output)
    exit_code = re.search(r"EXIT=(\d+)", output)
    ok = bool(match) and match.group(2) == "0" and int(match.group(1)) > 0 and \
        exit_code is not None and exit_code.group(1) == "0"
    print("android(%s, %s): %s" % (serial, abi, "PASS" if ok else "FAIL"))
    return 0 if ok else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("build", "check"):
        p = sub.add_parser(name)
        p.add_argument("--out", default="/tmp/claude-1000/r18-android-window")
        if name == "check":
            p.add_argument("--device")
    args = parser.parse_args()
    if args.command == "build":
        return 0 if build(args.out) else 1
    return check(args)


if __name__ == "__main__":
    sys.exit(main())
