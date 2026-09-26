#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Portal testchmb_a_01 pedestal portal gun: each shot lands on a new wall.

In chamber 02 the unheld portal gun sits on a pedestal that turns a quarter
turn per cycle and fires a blue portal after each turn, so consecutive shots
land on the four walls in turn. The map does this with a func_door_rotating:
it opens 90 degrees carrying the gun, the gun is unparented (ClearParent), the
door closes back without it and the gun is reparented. The gun is a constrained
weapon (spawnflag 1), so leaving the hierarchy must not pull it back to its
spawn pose (CBaseCombatWeapon::SetParent); when it did, every shot landed on
the same wall.

The check boots the installed Portal product headless (portal_boot.py) with
`sv_portal_placement_log 1` for about 30 s of game time, once per physics
provider, and judges the logged blue-portal placements:

  - at least four pedestal shots were placed;
  - consecutive shots face a quarter turn apart;
  - the first four face four different walls, at different places.

  portal_pedestal_carousel.py --runtime run/runtime --build build --out DIR

Results are reported as checks-v1 (tools/quality/conformance_result.py).
"""

import argparse
import math
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from conformance_result import Checks  # noqa: E402

ROOT = HERE.parents[1]
MAP = "testchmb_a_01"
PHYSICS = ("vphysics", "vphysics_box3d")
SHOTS = 4
# Frames after the map loads (portal_boot pins fps_max 60): the pedestal fires
# at about 2, 9, 16 and 23 s of game time; `wait` counts command-buffer
# passes, which run a little ahead of frames.
CAPTURE_WAIT = 2700

PLACED = re.compile(
    r"portal_placed t=(?P<t>[-\d.]+) name=(?P<name>\S+) linkage=(?P<linkage>\d+) "
    r"portal2=(?P<portal2>[01]) origin=(?P<origin>[-\d. ]+?) angles=(?P<angles>[-\d. ]+)$")


def parse_placements(text):
    """Returns the logged placements, sorted by game time, without duplicates."""
    placements = {}
    for line in text.splitlines():
        match = PLACED.search(line.strip())
        if not match:
            continue
        origin = tuple(float(v) for v in match["origin"].split())
        angles = tuple(float(v) for v in match["angles"].split())
        if len(origin) != 3 or len(angles) != 3:
            continue
        key = (match["t"], match["name"])
        placements[key] = {"t": float(match["t"]), "name": match["name"],
                           "linkage": int(match["linkage"]),
                           "portal2": match["portal2"] == "1",
                           "origin": origin, "angles": angles}
    return sorted(placements.values(), key=lambda p: p["t"])


def facing(angles):
    """Unit forward vector of Source (pitch, yaw, roll) angles in degrees."""
    pitch, yaw = math.radians(angles[0]), math.radians(angles[1])
    return (math.cos(pitch) * math.cos(yaw), math.cos(pitch) * math.sin(yaw), -math.sin(pitch))


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def judge(checks, placements, label):
    """Counts this run's checks on the pedestal's blue-portal placements."""
    shots = [p for p in placements if not p["portal2"]]
    checks.check(len(shots) >= SHOTS, label + ".shots",
                 "%d blue placements, expected at least %d" % (len(shots), SHOTS))
    shots = shots[:SHOTS]
    faces = [facing(p["angles"]) for p in shots]
    for i in range(1, len(shots)):
        # A quarter turn: consecutive walls are perpendicular.
        d = dot(faces[i - 1], faces[i])
        checks.check(abs(d) < 0.2, "%s.quarter-turn.%d" % (label, i),
                     "shots at t=%.2f and t=%.2f face %s and %s (dot %.2f)"
                     % (shots[i - 1]["t"], shots[i]["t"], shots[i - 1]["angles"],
                        shots[i]["angles"], d))
    distinct = all(dot(faces[i], faces[j]) < 0.5
                   for i in range(len(faces)) for j in range(i + 1, len(faces)))
    checks.check(len(faces) == SHOTS and distinct, label + ".four-walls",
                 "facings %s" % [p["angles"] for p in shots])
    apart = all(math.dist(shots[i]["origin"], shots[j]["origin"]) > 64.0
                for i in range(len(shots)) for j in range(i + 1, len(shots)))
    checks.check(len(shots) == SHOTS and apart, label + ".different-places",
                 "origins %s" % [p["origin"] for p in shots])
    return shots


def boot(runtime, build, out, physics, renderer, timeout):
    """Runs the product once; returns (returncode, combined log text)."""
    command = [sys.executable, str(HERE / "portal_boot.py"), "--runtime", str(runtime),
               "--out", str(out), "--headless", "--map", MAP, "--physics", physics,
               "--capture-wait", str(CAPTURE_WAIT), "--timeout", str(timeout),
               "--startup-command=sv_portal_placement_log 1"]
    if build:
        command += ["--build", str(build)]
    if renderer:
        command += ["--renderer", renderer]
    result = subprocess.run(command, capture_output=True, text=True)
    Path(str(out) + ".log").write_text(result.stdout + result.stderr)
    # engine.log and console.log carry the same lines; parse_placements drops
    # the duplicates.
    text = ""
    for path in (out / "runtime/engine.log", out / "runtime/portal/console.log",
                 out / "stdout.log"):
        if path.is_file():
            text += path.read_text(errors="replace")
    return result.returncode, text


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                        help="staged Portal runtime with content (portal_boot.py --runtime)")
    parser.add_argument("--build", type=Path,
                        help="Waf output to overlay on the runtime (portal_boot.py --build)")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--physics", action="append", choices=PHYSICS,
                        help="physics provider (repeatable; default: both)")
    parser.add_argument("--renderer", default="native-vulkan")
    parser.add_argument("--timeout", type=float, default=180)
    args = parser.parse_args(argv)

    args.out.mkdir(parents=True, exist_ok=True)
    checks = Checks()
    for physics in args.physics or PHYSICS:
        out = args.out / physics
        if (out / "evidence.json").exists():
            parser.error("%s already holds a run; use a new --out" % out)
        code, text = boot(args.runtime, args.build, out, physics, args.renderer, args.timeout)
        checks.check(code == 0, physics + ".boot", "portal_boot.py exited %d (%s.log)" % (code, out))
        shots = judge(checks, parse_placements(text), physics)
        for shot in shots:
            print("%s t=%.2f origin=%s angles=%s" % (physics, shot["t"], shot["origin"],
                                                      shot["angles"]))
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
