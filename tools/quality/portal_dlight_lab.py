#!/usr/bin/env python3
"""Dynamic lights through portals: the `portal_dlight_lab` test map and its
in-game oracle (render.portal-lights.v1, RFC 0016 K7).

    python3 tools/quality/portal_dlight_lab.py build [--store DIR]
    python3 tools/quality/portal_dlight_lab.py check --out DIR [--seed noclip]
        [--build INSTALL] [--runtime RUNTIME] [--content-root DIR]

The map: two sealed 512-unit rooms, A (x 0..512) and B (x 1024..1536), whose
only connection is a linked, activated prop_portal pair: portal A on A's east
wall facing into A, portal B on B's west wall facing into B, both centered at
y 256, z 72. Room A has a dim baked light; room B a dim one in a far corner,
so its floor near the portal is dark. The map compiles with the pinned
legacy vbsp/vvis/vrad (tools/quality/vmf_map_build.py) and publishes to the
playable map store (./play portal_dlight_lab).

The check boots the installed product headless on the map with a test light
(`r_portal_dlight_test`, a cheat) 72 units in front of portal A, radius 400.
Its image lies behind portal B's wall at (952, 256, 72). Cameras look
straight down at floor points of room B, and straight at a wall point:

  in      floor (1150, 256): the path to the light passes through portal B's
          opening, so the light must brighten it;
  out     floor (1150, 420): within the light's reach of the image (255 units)
          but its path misses the opening, so it must stay dark (no leak);
  wall    wall (1024, 400, 72) beside portal B, facing into B: the image is
          behind it, so it must stay dark;
  direct  floor (400, 320) of room A, under the light: the light itself.

Each view is taken in four states: the light off, on, on with
r_portal_dlights 0, and on with portal A closed. The oracle, on the mean
of each view's central 32x32 pixels: `direct` brightens in every lit state;
`in` brightens only when on with portals open and the feature on; `out` and
`wall` never change. The check prints one checks-v1 record.

    check --seed noclip   the sensitivity run: the engine's
                          r_portal_dlights_seed_noclip lets images light
                          without the portal clip, and the no-leak checks
                          (`out`) must fail.
"""

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gyro_lab_map  # noqa: E402  (the Vmf writer)
import playable_maps  # noqa: E402
import vmf_map_build  # noqa: E402
from conformance_result import Checks  # noqa: E402

ROOT = HERE.parents[1]
MAIN = ROOT.parent / "source-engine"
NAME = "portal_dlight_lab"
WALL = "DEV/GRAYGRID"
FLOOR = "DEV/DEV_MEASUREGENERIC01B"
THICK = 16
ROOM_A = (0, 0, 0, 512, 512, 256)
ROOM_B = (1024, 0, 0, 1536, 512, 256)
PORTAL_Z = 72
LIGHT = (440, 256, 72)
LIGHT_RADIUS = 400
# views: name, setpos (noclip origin; the eye is 64 above), setang
VIEWS = (
    ("in", (1150, 256, 36), (89, 180, 0)),
    ("out", (1150, 420, 36), (89, 180, 0)),
    ("wall", (1200, 400, 8), (0, 180, 0)),
    ("direct", (400, 320, 36), (89, 180, 0)),
)
STATES = (
    ("off", "r_portal_dlight_test off"),
    ("on", "r_portal_dlight_test %d %d %d %d 255 240 210 1" % (LIGHT + (LIGHT_RADIUS,))),
    ("disabled", "r_portal_dlights 0"),
    ("closed", "r_portal_dlights 1; ent_fire portal_a SetActivatedState 0"),
)
LIT_DELTA = 8.0     # levels (0-255) a lit view must gain over "off"
DARK_DELTA = 2.0    # levels a dark view may move
SETTLE = 40
WAYPOINT_Z = 200  # above the portals' openings (z 18..126)
AFTER = 10


def vec(v):
    return " ".join("%g" % c for c in v)


def room(vmf, bounds):
    x0, y0, z0, x1, y1, z1 = bounds
    cx, cy, cz = (x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2
    hx, hy, hz = (x1 - x0) / 2 + THICK, (y1 - y0) / 2 + THICK, (z1 - z0) / 2 + THICK
    t = THICK / 2
    vmf.world += [
        vmf.box((cx, cy, z0 - t), (hx, hy, t), FLOOR),
        vmf.box((cx, cy, z1 + t), (hx, hy, t), WALL),
        vmf.box((x0 - t, cy, cz), (t, hy, hz), WALL),
        vmf.box((x1 + t, cy, cz), (t, hy, hz), WALL),
        vmf.box((cx, y0 - t, cz), (hx - THICK, t, hz), WALL),
        vmf.box((cx, y1 + t, cz), (hx - THICK, t, hz), WALL),
    ]


def light(vmf, origin, brightness):
    vmf.entity("light", {"origin": vec(origin), "_light": "255 250 240 %d" % brightness,
                         "_quadratic_attn": "0", "_linear_attn": "1", "_constant_attn": "0"})


def vmf_text():
    vmf = gyro_lab_map.Vmf()
    room(vmf, ROOM_A)
    room(vmf, ROOM_B)
    light(vmf, (64, 64, 230), 60)
    light(vmf, (1510, 20, 230), 25)
    vmf.entity("info_player_start", {"origin": "1400 256 8", "angles": "0 180 0"})
    for name, origin, angles, two in (("portal_a", (512, 256, PORTAL_Z), "0 180 0", "0"),
                                      ("portal_b", (1024, 256, PORTAL_Z), "0 0 0", "1")):
        vmf.entity("prop_portal", {"targetname": name, "origin": vec(origin), "angles": angles,
                                   "Activated": "1", "PortalTwo": two, "LinkageGroupID": "0"})
    return vmf.text()


def build(args):
    out = Path(args.out).resolve()
    work = out / "compile"
    work.mkdir(parents=True, exist_ok=True)
    vmf = work / (NAME + ".vmf")
    vmf.write_text(vmf_text())
    tools = Path(json.loads(Path(args.toolchain).read_text())["compile_tools"])
    record = vmf_map_build.build(vmf, out, tools, Path(args.runtime).resolve(), quality="full", name=NAME)
    if record["status"] != "pass":
        print("portal_dlight_lab: build %s (%s)" % (record["status"], out / "build.json"))
        return 1
    print("published " + playable_maps.describe(playable_maps.publish(record, store=Path(args.store))))
    return 0


def script():
    """Console lines for portal_boot's command cfg, as the view oracles
    script shots (tools/render/view_oracle.py console_script): every line of
    an exec'd cfg runs at once and a long wait chain on one line is not
    reliable, so each shot is an alias that ends by calling the next. Moves
    between the rooms go through a waypoint above the portals' openings (a
    move across an opening teleports the player), and the camera is placed
    again right before the frame."""
    shots = []
    for state, command in STATES:
        for index, (view, pos, ang) in enumerate(VIEWS):
            place = ["cmd setpos %s" % vec(pos), "cmd setang %s" % vec(ang)]
            steps = ([command, "wait 30"] if index == 0 else [])
            steps += ["cmd setpos %g %g %g" % (pos[0], pos[1], WAYPOINT_Z), "wait 5"] + place
            steps += ["wait %d" % SETTLE] + place + ["wait 3", "getpos",
                                                    "screenshot pdl_%s_%s" % (state, view), "wait %d" % AFTER]
            shots.append(steps)
    lines = ["noclip"]
    for index, steps in enumerate(shots):
        if index + 1 < len(shots):
            steps = steps + ["pdl_shot%d" % (index + 1)]
        lines.append('alias pdl_shot%d "%s"' % (index, "; ".join(steps)))
    lines.append("wait 60; pdl_shot0")
    return lines


def central_mean(path):
    from PIL import Image
    image = Image.open(path).convert("L")
    w, h = image.size
    box = image.crop((w // 2 - 16, h // 2 - 16, w // 2 + 16, h // 2 + 16))
    pixels = box.tobytes()
    return sum(pixels) / len(pixels)


def check(args):
    out = Path(args.out).resolve()
    frames = 200 + len(STATES) * (30 + len(VIEWS) * (SETTLE + AFTER + 10))
    command = [sys.executable, str(HERE / "portal_boot.py"), "--runtime", str(args.runtime),
               "--build", str(args.build), "--renderer", "native-vulkan", "--headless",
               "--map", NAME, "--width", "1280", "--height", "720",
               "--capture-wait", str(frames), "--startup-command", "sv_cheats 1",
               "--startup-command", "r_portal_dlights_report 1",
               # Fixed 15 ms frames, so each wait is game time the server
               # tick and the client's interpolation need (the view oracles'
               # setting); unthrottled headless frames outrun them.
               "--startup-command", "host_framerate 0.015",
               "--out", str(out)]
    if args.seed == "noclip":
        command += ["--startup-command", "r_portal_dlights_seed_noclip 1"]
    for line in script():
        command += ["--console-command", line]
    if args.content_root:
        command += ["--content-root", str(args.content_root)]
    result = subprocess.run(command, capture_output=True, text=True)
    checks = Checks()
    checks.check(result.returncode == 0, "boot.passes", (result.stdout + result.stderr)[-400:])
    directory = out / "runtime/portal/screenshots"
    shots = {(state, view): directory / ("pdl_%s_%s.tga" % (state, view))
             for state, _ in STATES for view, _, _ in VIEWS}
    missing = [str(path.name) for path in shots.values() if not path.exists()]
    checks.check(not missing, "boot.every-view-captured", "missing %s" % ", ".join(missing))
    if missing:
        return checks.report()
    means = {key: central_mean(path) for key, path in shots.items()}
    delta = {key: means[key] - means[("off", key[1])] for key in means}
    report = {"%s/%s" % key: round(value, 2) for key, value in sorted(delta.items())}
    print("INFO portal dlight lab: brightening over the light off, by state/view: %s" % json.dumps(report))
    (out / "portal_dlight_lab.json").write_text(json.dumps(
        {"means": {"%s/%s" % k: v for k, v in means.items()}, "delta": report}, indent=2) + "\n")
    for state in ("on", "disabled", "closed"):
        checks.check(delta[(state, "direct")] >= LIT_DELTA, "direct.%s-lights-room-a" % state,
                     "%.2f" % delta[(state, "direct")])
    checks.check(delta[("on", "in")] >= LIT_DELTA, "in.lit-through-the-portal", "%.2f" % delta[("on", "in")])
    for state in ("disabled", "closed"):
        checks.check(abs(delta[(state, "in")]) <= DARK_DELTA, "in.dark-when-%s" % state,
                     "%.2f" % delta[(state, "in")])
    for state in ("on", "disabled", "closed"):
        checks.check(abs(delta[(state, "out")]) <= DARK_DELTA, "out.no-leak-outside-the-opening-%s" % state,
                     "%.2f" % delta[(state, "out")])
        checks.check(abs(delta[(state, "wall")]) <= DARK_DELTA, "wall.dark-behind-the-exit-%s" % state,
                     "%.2f" % delta[(state, "wall")])
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    b = sub.add_parser("build")
    b.add_argument("--out", default=str(ROOT / "quality-results" / NAME))
    b.add_argument("--toolchain", default=str(MAIN / "build/toolchains/pbrt-map-toolchain.json"))
    b.add_argument("--runtime", default=str(MAIN / "run/runtime"))
    b.add_argument("--store", default=str(MAIN / "run/maps"), help="the playable map store (./play)")
    c = sub.add_parser("check")
    c.add_argument("--build", default=os.environ.get("PORTAL_DLIGHT_LAB_BUILD"),
                   help="the installed product (default $PORTAL_DLIGHT_LAB_BUILD)")
    c.add_argument("--runtime", default=str(MAIN / "run/runtime"))
    c.add_argument("--content-root", default=str(ROOT / "quality-results" / NAME / "content"),
                   help="where the built map lives (the build step's content package)")
    c.add_argument("--out", required=True)
    c.add_argument("--seed", choices=("noclip",),
                   help="sensitivity: images skip the portal clip; the no-leak checks must fail")
    args = parser.parse_args(argv)
    if args.command == "check" and not args.build:
        parser.error("check needs --build or PORTAL_DLIGHT_LAB_BUILD")
    return build(args) if args.command == "build" else check(args)


if __name__ == "__main__":
    sys.exit(main())
