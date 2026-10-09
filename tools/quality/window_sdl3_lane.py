#!/usr/bin/env python3
"""Run platform.window.sdl3 on real Wayland and X11 video drivers in private sessions.

The platform.window.sdl3 manifest row runs the SDL3 window provider against the
shared platform.window.v1 suite on SDL's offscreen driver. This lane builds the
same program from that row's sources and flags and runs it with
SDL_VIDEODRIVER=wayland inside kiln's "private" session and with
SDL_VIDEODRIVER=x11 inside "private-x11" (a private compositor with Xwayland and
its own D-Bus session), so no window reaches the user's display (RFC 0001 R14).

It prints each run's checks-v1 result line and finishes with one combined
CONFORMANCE line, so the manifest's runner counts every driver's checks.
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ROW = "platform.window.sdl3"
DRIVERS = (("wayland", "private"), ("x11", "private-x11"))
RESULT = re.compile(r"^CONFORMANCE (\d+) (\d+)$", re.M)


def manifest_row():
    manifest = json.loads((ROOT / "quality/conformance.manifest.json").read_text())
    suites = manifest["suites"] if isinstance(manifest, dict) else manifest
    for suite in suites:
        if suite["id"] == ROW:
            return suite
    raise SystemExit("window_sdl3_lane: manifest has no %s row" % ROW)


def build(out, cxx):
    row = manifest_row()
    exe = out / "test_sdl3_window"
    command = [cxx, "-std=c++20", "-O1", "-Ipublic", "-Ipublic/tier0", "-Ipublic/tier1"]
    command += row.get("extra_flags", []) + row["sources"] + row.get("link_flags", [])
    command += ["-o", str(exe)]
    subprocess.run(command, cwd=ROOT, check=True)
    return exe


def run(out, exe, driver, display_name):
    sys.path.insert(0, str(ROOT / "tools/kiln"))
    import sepipe_loader

    environment = dict(os.environ, SDL_VIDEODRIVER=driver)
    with sepipe_loader.Display(display_name, out / ("display-" + driver),
                               (1920, 1080, 60)) as display:
        command, environment = display.wrap([str(exe)], environment)
        environment["SDL_VIDEODRIVER"] = driver
        result = subprocess.run(command, cwd=ROOT, env=environment, capture_output=True,
                                text=True, timeout=600)
    log = out / ("test_sdl3_window-%s.log" % driver)
    log.write_text(result.stdout + result.stderr)
    found = RESULT.findall(result.stdout)
    for line in result.stdout.splitlines():
        if line.startswith(("ok ", "FAIL", "  FAIL", "  recorded skip", "SDL ")):
            print("[%s] %s" % (driver, line))
    if result.returncode != 0 or len(found) != 1:
        print("[%s] exit %d, log %s" % (driver, result.returncode, log))
        return None
    return int(found[0][0]), int(found[0][1])


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--out", type=Path, default=Path("/tmp/claude-1000/window-sdl3-lane"))
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--driver", action="append", choices=[d for d, _ in DRIVERS])
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    exe = build(args.out, args.cxx)
    checks = failures = 0
    for driver, display_name in DRIVERS:
        if args.driver and driver not in args.driver:
            continue
        counts = run(args.out, exe, driver, display_name)
        if counts is None:
            print("CONFORMANCE %d %d" % (checks, failures + 1))
            return 1
        checks += counts[0]
        failures += counts[1]
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
