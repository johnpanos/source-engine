#!/usr/bin/env python3
"""Run the SDL3 window/input suites on real Wayland and X11 drivers in private sessions.

Without --tree, the lane builds platform.window.sdl3's program from that manifest
row's sources and flags (the SDL3 window provider against the shared
platform.window.v1 suite). With --tree <profile tree>, it runs the product's SDL3
suites built in that tree instead: the launcher manager (sdl3_launcher_conformance),
the input system's gamepad slots (input_gamepad_slots_test, plus its seeded
slot-collapse provider, which must fail) and SDL3 voice capture
(sdl3_voice_record_test).

Each program runs with SDL_VIDEODRIVER=wayland inside kiln's "private" session and
with SDL_VIDEODRIVER=x11 inside "private-x11" (a private compositor with Xwayland
and its own D-Bus session), so no window reaches the user's display (RFC 0001
R14, R18). Each run's checks-v1 line is printed and one combined CONFORMANCE line
ends the output, so the manifest's runner counts every driver's checks.
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
VOICE_RESULT = re.compile(r"^SDL3 voice recording: (\d+) checks, pass$", re.M)

# (program relative to the tree's install directory, extra arguments, must pass)
PRODUCT_PROGRAMS = (
    ("sdl3_launcher_conformance", (), True),
    ("tests/input_gamepad_slots_test", (), True),
    ("tests/input_gamepad_slots_test", ("-seed-slot-collapse",), False),
    ("tests/sdl3_voice_record_test", (), True),
)


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


def run(out, argv, driver, display_name, environment, label):
    sys.path.insert(0, str(ROOT / "tools/kiln"))
    import sepipe_loader

    environment = dict(environment, SDL_VIDEODRIVER=driver)
    with sepipe_loader.Display(display_name, out / ("display-" + driver),
                               (1920, 1080, 60)) as display:
        command, environment = display.wrap([str(a) for a in argv], environment)
        environment["SDL_VIDEODRIVER"] = driver
        result = subprocess.run(command, cwd=ROOT, env=environment, capture_output=True,
                                text=True, timeout=600)
    log = out / ("%s-%s.log" % (label, driver))
    log.write_text(result.stdout + result.stderr)
    for line in result.stdout.splitlines():
        if line.startswith(("ok ", "FAIL", "  FAIL", "  recorded skip", "SDL ", "CONFORMANCE",
                            "SDL3 voice")):
            print("[%s %s] %s" % (driver, label, line))
    found = RESULT.findall(result.stdout)
    if not found:
        voice = VOICE_RESULT.findall(result.stdout)
        found = [(voice[0], "0")] if voice and result.returncode == 0 else []
    if len(found) != 1:
        print("[%s %s] exit %d, no result, log %s" % (driver, label, result.returncode, log))
        return None
    checks, failures = int(found[0][0]), int(found[0][1])
    if ( failures == 0 ) != ( result.returncode == 0 ):
        print("[%s %s] exit %d disagrees with its result, log %s" %
              (driver, label, result.returncode, log))
        return None
    return checks, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--out", type=Path, default=Path("/tmp/claude-1000/window-sdl3-lane"))
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--driver", action="append", choices=[d for d, _ in DRIVERS])
    parser.add_argument("--tree", type=Path,
                        help="a profile tree (out/<profile>/<flavor>) whose SDL3 suites to run")
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    if args.tree:
        install = (ROOT / args.tree / "install").resolve()
        environment = dict(os.environ, LD_LIBRARY_PATH=str(install / "bin"))
        programs = [(install / program, extra, must_pass, Path(program).name +
                     ("".join(extra) or ""))
                    for program, extra, must_pass in PRODUCT_PROGRAMS]
    else:
        environment = dict(os.environ)
        programs = [(build(args.out, args.cxx), (), True, "test_sdl3_window")]

    checks = failures = 0
    for driver, display_name in DRIVERS:
        if args.driver and driver not in args.driver:
            continue
        for program, extra, must_pass, label in programs:
            if not program.exists():
                print("[%s] %s is not built" % (driver, program))
                print("CONFORMANCE %d %d" % (checks, failures + 1))
                return 1
            counts = run(args.out, [program, *extra], driver, display_name, environment, label)
            if counts is None:
                print("CONFORMANCE %d %d" % (checks, failures + 1))
                return 1
            if must_pass:
                checks += counts[0]
                failures += counts[1]
            else:
                # A seeded provider: the suite must reject it.
                checks += 1
                if counts[1] == 0:
                    failures += 1
                    print("[%s %s] the seeded provider was not caught" % (driver, label))
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
