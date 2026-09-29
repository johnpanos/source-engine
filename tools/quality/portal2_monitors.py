#!/usr/bin/env python3
"""Portal 2's Wheatley monitors show the live point_camera view.

Act 4's Wheatley monitors (the sp_a4_* maps) are a func_monitor brush in the
prefab's screen frame. The brush face uses dev/dev_tvmonitor1a, an
UnlitTwoTexture material whose $basetexture is _rt_Camera, and its target is
@wheatley_camera. That point_camera looks at a Wheatley model in a hidden room.
The server links the monitor to the camera (info_camera_link) and turns the
camera on while a linked monitor is in the player's PVS. The client renders
every active camera into _rt_Camera (CViewRender::DrawMonitors) before the
main view.

The user's report (2026-09-29): the monitors were black. The Portal 2 client
compiled DrawMonitors out, because USE_MONITORS was defined only for
HL2_CLIENT_DLL and CSTRIKE_DLL (game/client/viewrender.cpp, view.cpp). Portal
1 defines HL2_CLIENT_DLL and Portal 2 does not, so nothing drew _rt_Camera.

This harness boots each scenario's map headless on native Vulkan
(portal_boot.py). It deploys one monitor through its prefab's
relay_deploy_straight, places a noclip camera in front of the screen, and
takes two shots: one at the camera's authored FOV, and one after
`ChangeFOV` doubles it. The screen's placement comes from the BSP: the
func_monitor origin, its screen face's normal turned by the entity's yaw, and
a fixed viewing distance. Per scenario:

  facts.screen-material  the func_monitor's screen face uses a material whose
                         $basetexture is _rt_Camera (read from the VPK)
  facts.camera-target    the func_monitor targets a point_camera of the map
  boot                   the run finished with both shots
  screen-shows-camera    Wheatley's eye (blue: B >= 110, B - R >= 60, sRGB
                         0-255) covers at least 0.3 % of the screen region
                         in the authored-FOV shot (measured 1.5-1.6 %; 0 %
                         with the monitors off; the walls stay under B - R 20)
  view-is-live           doubling the camera's FOV shrinks the eye to at most
                         0.6 of its area (measured 0.25-0.26; a frozen screen
                         1.0): the screen shows this camera's view now, not a
                         stale texture

Live negative controls, each run on the first scenario:

  no-monitors  cl_drawmonitors 0 from startup: nothing renders _rt_Camera,
               the defect as reported; it must be rejected on
               screen-shows-camera
  frozen       cl_drawmonitors 0 after the first shot: _rt_Camera keeps its
               last frame; it must be rejected on view-is-live

    portal2_monitors.py suite --out <dir>     # checks-v1
    portal2_monitors.py selftest              # the judge on synthetic shots

Needs numpy and Pillow, a native Vulkan Portal 2 build (build-p2/;
SOURCE_PORTAL2_BUILD selects another tree) and a licensed Portal 2 install
(SOURCE_PORTAL2_STEAM_ROOT, by default the Steam library). A content runtime
is staged from the install under --out, or pass --p2-runtime. Keep --out
short.
"""

import argparse
import datetime
import json
import math
import os
from pathlib import Path
import re
import subprocess
import sys

import numpy
from PIL import Image

QUALITY = Path(__file__).resolve().parent
sys.path.insert(0, str(QUALITY))
import conformance_result  # noqa: E402
from legacy_bsp import FACE, LUMP_FACES, LegacyBsp  # noqa: E402

ROOT = QUALITY.parents[1]
PORTAL_BOOT = QUALITY / "portal_boot.py"
SCHEMA = "portal2-monitors-evidence/v1"
DEFAULT_STEAM_P2 = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"
MARK = "MONITORS"
SHOTS = ("authored", "wide")
WIDTH, HEIGHT = 1024, 768
STARTUP = ["mat_force_tonemap_scale 1", "host_framerate 0.016667", "r_drawviewmodel 0",
           "crosshair 0", "cl_drawhud 0"]
# The player's eye above its origin while standing (setpos places the origin).
EYE_HEIGHT = 64.0
# From the screen's plane to the eye: the prefab's 92x188 screen then fills
# the middle of a 1024x768 frame after the arm deploys it.
VIEW_DISTANCE = 190.0
# The screen in the frame at VIEW_DISTANCE: [x0, y0, x1, y1] fractions.
SCREEN_REGION = [0.30, 0.10, 0.70, 0.90]
THRESHOLDS = {"eye_blue_min": 110, "eye_blue_over_red_min": 60,
              "eye_fraction_min": 0.003, "wide_over_authored_max": 0.6}

SCENARIOS = [
    {"name": "tb_intro", "map": "sp_a4_tb_intro", "prefix": "monitor1-",
     "description": "the chamber's monitor (monitor1), which the user reported black"},
    {"name": "intro", "map": "sp_a4_intro", "prefix": "wheatley_monitor1-",
     "description": "the first of sp_a4_intro's five monitors (wheatley_monitor1)"},
]

CONTROLS = {
    "no-monitors": {
        "description": "cl_drawmonitors 0 from startup: nothing renders _rt_Camera (the "
                       "defect as reported)",
        "startup": ["cl_drawmonitors 0"], "before_wide": [],
        "must_fail": ["screen-shows-camera"],
    },
    "frozen": {
        "description": "cl_drawmonitors 0 after the authored-FOV shot: _rt_Camera keeps its "
                       "last frame, so the FOV change never reaches the screen",
        "startup": [], "before_wide": ["cl_drawmonitors 0"],
        "must_fail": ["view-is-live"],
    },
}


# ---------------------------------------------------------------------------
# Facts: the monitor, its camera and its screen, from the map and the VPKs.

def vmt_base_texture(text):
    match = re.search(r'"?\$basetexture"?\s+"?([^"\s]+)"?', text, re.IGNORECASE)
    return match.group(1) if match else None


def material_reader(steam_root):
    import source_content  # noqa: E402 (needs the Steam install only here)
    directories = []
    for name in ("portal2_dlc2", "portal2_dlc1", "portal2"):
        vpk = Path(steam_root) / name / "pak01_dir.vpk"
        if vpk.is_file():
            directories.append(source_content.VpkDirectory(str(vpk)))

    def read(material):
        relative = "materials/%s.vmt" % material.lower().replace("\\", "/")
        for directory in directories:
            data = directory.read(relative)
            if data is not None:
                return data.decode("latin-1")
        return None
    return read


def yaw_rotate(vector, yaw_degrees):
    yaw = math.radians(yaw_degrees)
    x, y, z = vector
    return [x * math.cos(yaw) - y * math.sin(yaw), x * math.sin(yaw) + y * math.cos(yaw), z]


def monitor_facts(bsp_path, prefix, read_material):
    """The scenario's func_monitor: its camera, screen material and a viewing
    placement in front of its screen face."""
    bsp = LegacyBsp.read(bsp_path)
    entities = bsp.entities()
    monitor = next((e for e in entities if e.get("classname") == "func_monitor"
                    and e.get("targetname") == prefix + "wheatley_monitor_screen"), None)
    if monitor is None:
        return {"found": False}
    cameras = [e for e in entities if e.get("classname") == "point_camera"
               and e.get("targetname") == monitor.get("target")]
    model = bsp.models()[int(monitor["model"].lstrip("*"))]
    faces = bsp.lump(LUMP_FACES, FACE.size)
    normals, _ = bsp.planes()
    texinfo, texdata = bsp.texinfo(), bsp.texdata()
    screens = []
    for index in range(model["firstface"], model["firstface"] + model["numfaces"]):
        record = FACE.unpack_from(faces, index * FACE.size)
        plane, texture = record[0], record[5]
        name = texdata[texinfo[texture]["texdata"]]["name"]
        vmt = read_material(name)
        screens.append({"material": name, "normal": normals[plane].tolist(),
                        "base_texture": vmt_base_texture(vmt) if vmt else None})
    angles = [float(v) for v in monitor.get("angles", "0 0 0").split()]
    camera_screens = [s for s in screens if (s["base_texture"] or "").lower() == "_rt_camera"]
    facts = {"found": True, "monitor": monitor.get("targetname"),
             "target": monitor.get("target"), "cameras": len(cameras),
             "camera_fov": float(cameras[0].get("FOV", 90)) if cameras else None,
             "screens": screens, "angles": angles, "origin": monitor.get("origin")}
    if camera_screens and abs(angles[0]) < 0.01 and abs(angles[2]) < 0.01:
        normal = yaw_rotate(camera_screens[0]["normal"], angles[1])
        center = [float(v) for v in monitor["origin"].split()]
        eye = [c + VIEW_DISTANCE * n for c, n in zip(center, normal)]
        facts["setpos"] = [round(eye[0], 2), round(eye[1], 2), round(eye[2] - EYE_HEIGHT, 2)]
        facts["setang"] = [0.0, round(math.degrees(math.atan2(-normal[1], -normal[0])), 2), 0.0]
    return facts


# ---------------------------------------------------------------------------
# The run: one boot, two shots.

def console_line(scenario, facts, control=None):
    prefix = scenario["prefix"]
    wide = 2.0 * facts["camera_fov"]
    steps = ["noclip", "ent_fire %srelay_enable_screen trigger" % prefix,
             "ent_fire %srelay_deploy_straight trigger" % prefix,
             "cmd setpos %g %g %g" % tuple(facts["setpos"]),
             "cmd setang %g %g %g" % tuple(facts["setang"]), "wait 360",
             "echo %s authored" % MARK, "screenshot", "wait 20"]
    steps += list((control or {}).get("before_wide", []))
    steps += ['ent_fire %s changefov "%g 0.05"' % (facts["target"], wide), "wait 120",
              "echo %s wide" % MARK, "screenshot", "wait 20", "echo %s end" % MARK]
    return "; ".join(steps)


def boot(args, scenario, facts, control, out):
    line = console_line(scenario, facts, control)
    command = [sys.executable, str(PORTAL_BOOT), "--game", "portal2",
               "--runtime", str(p2_runtime(args)), "--build", str(args.p2_build),
               "--out", str(out), "--headless", "--map", scenario["map"],
               "--renderer", "native-vulkan", "--require-vulkan", "--physics", "vphysics_box3d",
               "--width", str(WIDTH), "--height", str(HEIGHT), "--capture-wait", "1400",
               "--timeout", str(args.timeout), "--console-command", line]
    for startup in STARTUP + list((control or {}).get("startup", [])):
        command += ["--startup-command", startup]
    completed = subprocess.run(command, capture_output=True, text=True)
    screenshots = sorted((out / "runtime/portal2/screenshots").glob("*.tga"))
    console = out / "runtime/portal2/console.log"
    text = console.read_text(errors="replace") if console.is_file() else ""
    return {"returncode": completed.returncode, "console_line": line,
            "boot_tail": (completed.stdout[-600:] + completed.stderr[-600:]).strip(),
            "screenshots": [str(p) for p in screenshots],
            "markers": re.findall(r"^%s (\S+)" % MARK, text, re.MULTILINE)}


def p2_runtime(args):
    if args.p2_runtime is None:
        args.p2_runtime = args.out.resolve() / "p2content"
        if not args.p2_runtime.exists():
            import stage_portal2_runtime  # noqa: E402 (needs the Steam install only here)
            stage_portal2_runtime.stage_content(args.steam_root, args.p2_runtime)
    return args.p2_runtime


# ---------------------------------------------------------------------------
# The oracle: pure functions over shots.

def eye_fraction(image, box=SCREEN_REGION, thresholds=THRESHOLDS):
    """The share of the screen region that is Wheatley's bright blue eye."""
    pixels = numpy.asarray(image, dtype=numpy.int32)[..., :3]
    height, width = pixels.shape[:2]
    x0, y0, x1, y1 = box
    region = pixels[int(y0 * height):int(y1 * height), int(x0 * width):int(x1 * width)]
    red, blue = region[..., 0], region[..., 2]
    eye = (blue >= thresholds["eye_blue_min"]) & (blue - red >= thresholds["eye_blue_over_red_min"])
    return float(eye.mean())


def judge(shots, thresholds=THRESHOLDS):
    """[(check, passed, detail)] for one run's shots {label: HxWx3 array}."""
    verdicts = []
    have = all(label in shots for label in SHOTS)
    verdicts.append(("boot", have, "shots %s" % sorted(shots)))
    if not have:
        return verdicts
    authored, wide = (eye_fraction(shots[label], thresholds=thresholds) for label in SHOTS)
    verdicts.append(("screen-shows-camera", authored >= thresholds["eye_fraction_min"],
                     "Wheatley's eye covers %.3f %% of the screen region (at least %.2f %%)" % (
                         100.0 * authored, 100.0 * thresholds["eye_fraction_min"])))
    ratio = wide / authored if authored > 0 else float("inf")
    verdicts.append(("view-is-live", authored > 0 and ratio <= thresholds["wide_over_authored_max"],
                     "doubling the FOV leaves the eye at %.2f of its area (at most %.2f)" % (
                         ratio, thresholds["wide_over_authored_max"])))
    return verdicts


def load_shots(run):
    return {label: numpy.asarray(Image.open(path).convert("RGB"))
            for label, path in zip(SHOTS, run["screenshots"])}


# ---------------------------------------------------------------------------
# Commands.

def cmd_suite(args):
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    checks = conformance_result.Checks()
    read_material = material_reader(args.steam_root)
    evidence = {"schema": SCHEMA, "thresholds": THRESHOLDS, "screen_region": SCREEN_REGION,
                "view_distance": VIEW_DISTANCE,
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "p2_build": str(args.p2_build.resolve()), "scenarios": []}
    first = True
    for index, scenario in enumerate(SCENARIOS):
        if args.scenario and scenario["name"] not in args.scenario:
            continue
        name = "%s.%s" % (scenario["map"], scenario["prefix"].rstrip("-"))
        facts = monitor_facts(p2_runtime(args) / ("portal2/maps/%s.bsp" % scenario["map"]),
                              scenario["prefix"], read_material)
        record = {"scenario": scenario["name"], "facts": facts, "runs": []}
        evidence["scenarios"].append(record)
        screen_ok = any((s["base_texture"] or "").lower() == "_rt_camera"
                        for s in facts.get("screens", []))
        checks.check(screen_ok, name + ".facts.screen-material",
                     "no _rt_Camera screen face: %s" % facts.get("screens"))
        checks.check(facts.get("cameras") == 1, name + ".facts.camera-target",
                     "func_monitor target %r names %s point_camera(s)" % (
                         facts.get("target"), facts.get("cameras")))
        if "setpos" not in facts:
            checks.check(False, name + ".boot", "no viewing placement: %s" % facts)
            continue
        run = boot(args, scenario, facts, None, out / ("s%d" % index))
        run["verdicts"] = judge(load_shots(run))
        record["runs"].append(run)
        for check, passed, detail in run["verdicts"]:
            checks.check(passed, "%s.%s" % (name, check), detail)
        if args.no_controls or not first:
            continue
        first = False
        for control_name, control in CONTROLS.items():
            run = boot(args, scenario, facts, control, out / ("s%d-%s" % (index, control_name)))
            verdicts = judge(load_shots(run))
            run["control"], run["verdicts"] = control_name, verdicts
            record["runs"].append(run)
            label = "%s.control.%s" % (name, control_name)
            failed = {check for check, passed, _ in verdicts if not passed}
            checks.check("boot" not in failed, label + ".boot",
                         next(d for c, _, d in verdicts if c == "boot"))
            for must in control["must_fail"]:
                checks.check(must in failed, "%s.rejects.%s" % (label, must),
                             "the oracle accepted the control on %s: %s" % (
                                 must, next((d for c, _, d in verdicts if c == must), "")))
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2, default=str) + "\n")
    print("evidence: %s" % (out / "evidence.json"))
    return checks.report()


def synthetic(eye=0.0, width=128, height=96, background=(40, 44, 48)):
    """A frame whose screen region holds a centred blue eye covering `eye` of it."""
    frame = numpy.zeros((height, width, 3), numpy.uint8)
    frame[...] = background
    x0, y0, x1, y1 = SCREEN_REGION
    rx0, ry0, rx1, ry1 = int(x0 * width), int(y0 * height), int(x1 * width), int(y1 * height)
    area = (rx1 - rx0) * (ry1 - ry0)
    side = int(round(math.sqrt(eye * area)))
    cx, cy = (rx0 + rx1) // 2, (ry0 + ry1) // 2
    frame[cy - side // 2:cy - side // 2 + side, cx - side // 2:cx - side // 2 + side] = (20, 140, 250)
    return frame


def cmd_selftest(_args):
    checks = conformance_result.Checks()

    def verdict(verdicts, check):
        return next((passed for c, passed, _ in verdicts if c == check), None)

    live = judge({"authored": synthetic(0.03), "wide": synthetic(0.008)})
    checks.check(all(p for _, p, _ in live), "live-passes", "; ".join(d for _, p, d in live))
    black = judge({"authored": synthetic(0.0, background=(0, 0, 0)),
                   "wide": synthetic(0.0, background=(0, 0, 0))})
    checks.check(verdict(black, "screen-shows-camera") is False, "black-screen-rejected")
    checks.check(verdict(black, "view-is-live") is False, "black-screen-not-live")
    stale = judge({"authored": synthetic(0.03), "wide": synthetic(0.03)})
    checks.check(verdict(stale, "screen-shows-camera") is True
                 and verdict(stale, "view-is-live") is False, "stale-screen-rejected")
    grown = judge({"authored": synthetic(0.01), "wide": synthetic(0.03)})
    checks.check(verdict(grown, "view-is-live") is False, "narrowed-view-rejected")
    missing = judge({"authored": synthetic(0.03)})
    checks.check(verdict(missing, "boot") is False and len(missing) == 1, "missing-shot-rejected")
    # The walls' desaturated blue-grey is not the eye.
    wall = judge({"authored": synthetic(0.0, background=(90, 110, 140)),
                  "wide": synthetic(0.0, background=(90, 110, 140))})
    checks.check(verdict(wall, "screen-shows-camera") is False, "blue-grey-wall-not-eye")
    vmt = 'UnlitTwoTexture\n{\n$basetexture _rt_Camera\n"$texture2" "dev/dev_scanline"\n}\n'
    checks.check(vmt_base_texture(vmt) == "_rt_Camera", "vmt-base-texture")
    checks.check(vmt_base_texture('"UnlitGeneric" { "$basetexture" "dev/x" }') == "dev/x",
                 "vmt-base-texture.quoted")
    turned = yaw_rotate([0.0, 1.0, 0.0], 90.0)
    checks.check(abs(turned[0] + 1.0) < 1e-9 and abs(turned[1]) < 1e-9, "yaw-rotate")
    facts = {"setpos": [1880.62, 386.0, -320.9], "setang": [0.0, 0.0, 0.0],
             "target": "@wheatley_camera", "camera_fov": 30.0}
    line = console_line(SCENARIOS[0], facts, CONTROLS["frozen"])
    checks.check(line.count("screenshot") == len(SHOTS)
                 and line.index("cl_drawmonitors 0") < line.index("changefov")
                 and '"60 0.05"' in line
                 and line.index("relay_deploy_straight") < line.index("screenshot"),
                 "console-line", line)
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    commands = parser.add_subparsers(dest="command", required=True)
    suite = commands.add_parser("suite", help="boot, shoot and judge the monitors")
    suite.add_argument("--out", type=Path, required=True)
    suite.add_argument("--p2-build", type=Path,
                       default=Path(os.environ.get("SOURCE_PORTAL2_BUILD", ROOT / "build-p2")))
    suite.add_argument("--p2-runtime", type=Path)
    suite.add_argument("--steam-root", type=Path,
                       default=Path(os.environ.get("SOURCE_PORTAL2_STEAM_ROOT", DEFAULT_STEAM_P2)))
    suite.add_argument("--scenario", action="append")
    suite.add_argument("--no-controls", action="store_true")
    suite.add_argument("--timeout", type=int, default=600)
    commands.add_parser("selftest", help="the judge against synthetic shots")
    args = parser.parse_args(argv)
    return cmd_suite(args) if args.command == "suite" else cmd_selftest(args)


if __name__ == "__main__":
    sys.exit(main())
