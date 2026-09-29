#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Movers at rest cast no shadow on the static world (RFC 0011 moving-object occlusion).

RFC 0011's render.dynamic-occlusion.v1 lets moving objects block light in world
lightmaps, model lighting and the light set's GI consumers. The user's reports
(2026-09-28, sp_a2_core): "a very noticeable shadow against brushes/dynamic
props that are going to move later in the level", and "random shadows" around
the Wheatley receptacle. Retail shows neither: vrad bakes these movers' light
and Source's dynamic shadow model (ShadowCastType) gives brush entities no
shadow at all, and prop_dynamic's disableshadows none.

The cause was the occluder set: the client published every brush entity's solid
model box (func_brush, func_door, func_door_rotating, func_tank), shadows
disabled or not. Sunk in floors and pits, those boxes shadowed the floor around
them: the receptacle's frame (a 176x176x148 box whose top rises 52 units above
the floor once deployed) laid a large dark blotch on the floor, and the
stalemate button's platform box darkened its pedestal. The occluder set is now
the game's dynamic-shadow caster set (game/client/dynamic_occluders.cpp).

This harness boots Portal 2's sp_a2_core headless on native Vulkan
(portal_boot.py), brings each scene to the state of the reports, frames it with
a fixed noclip camera, and shoots it with occlusion on, on again (noise) and off
(r_dynamic_occlusion 0). Per scenario:

  boot                 the run finished with every shot and the occlusion report
  static               the scenario's regions do not change between the two on shots
  occlusion-active     r_dynamic_occlusion_report printed a published generation
  no-mover-shadow      in the scenario's regions, occlusion on is not darker than
                       off: mean luma darkening at most 0.5 levels and at most 1 %
                       of pixels darker by more than 4 levels (sRGB luma, 0-255)

Regions are fractions of the 1024x768 frame on receivers the old occluder set
darkened (receptacle: the floor below and left of the housing; stalemate: the
button pedestal's column and the arm models in the foreground), clear of models
that animate at rest.

Live negative control (receptacle): a copy of the housing model that casts
(prop_dynamic just_elevator.mdl, shadows not disabled) is set at the deployed
housing's pose, sunk in the pit as the old frame box was (render-to-texture
shadows off, so the off shot holds none). It is a legitimate occluder under the
new rule, so its occlusion must darken the regions and no-mover-shadow must
fail; its boxes must appear in the occlusion report (control-published). A
weighted cube or the same model standing on the open floor darkens nothing
there measurable: the defect needs a box sunk into the static world.

    portal2_mover_shadow.py suite --out <dir>     # checks-v1
    portal2_mover_shadow.py selftest              # the judge on synthetic shots

Needs numpy and Pillow, a native Vulkan Portal 2 build (build-p2/,
SOURCE_PORTAL2_BUILD selects another tree) and a licensed Portal 2 install
(SOURCE_PORTAL2_STEAM_ROOT, default the Steam library), from which a content
runtime is staged under --out (or pass --p2-runtime). Keep --out short.
"""

import argparse
import datetime
import json
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
from legacy_bsp import LegacyBsp  # noqa: E402

ROOT = QUALITY.parents[1]
PORTAL_BOOT = QUALITY / "portal_boot.py"
SCHEMA = "portal2-mover-shadow-evidence/v1"
DEFAULT_STEAM_P2 = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"
MARK = "MOVERSHADOW"
SHOTS = ("on", "on2", "off")
WIDTH, HEIGHT = 1024, 768
STARTUP = ["mat_force_tonemap_scale 1", "host_framerate 0.016667", "r_drawviewmodel 0",
           "crosshair 0"]
THRESHOLDS = {"mean_darkening_max": 0.5, "dark_level": 4.0, "dark_fraction_max": 0.01,
              "static_mean_max": 0.25}

# The map's movers that the old occluder set published (brush entities).
RECEPTACLE_BRUSHES = ("core_receptacle_frame", "core_receptacle_vertical_mover_1",
                      "core_receptacle_vertical_mover_3", "core_receptacle_pointer_1",
                      "iris_door_core_pit")
STALEMATE_BRUSHES = ("button_platform", "shield_trigger_target")

SCENARIOS = [
    {
        "name": "receptacle",
        "description": "the Wheatley receptacle deployed (deploy_core_receptacle_relay: the "
                       "vertical movers raise the housing and its func_brush frame, the iris "
                       "panels open) with the chamber's projected lights on, seen from the "
                       "floor's +x side",
        "brushes": RECEPTACLE_BRUSHES,
        "setup": ["ent_fire deploy_core_receptacle_relay trigger",
                  "ent_fire texturelight_wheatly_chamber turnon",
                  "ent_fire texturelight_glados_chamber turnon", "wait 600"],
        "camera": {"setpos": [640, 0, 64], "setang": [25, 180, 0]},
        # Floor below and left of the housing (the old frame box's shadow).
        "regions": {"floor-below": [0.17, 0.67, 0.66, 0.99],
                    "floor-left": [0.17, 0.46, 0.38, 0.66]},
        "controls": {
            "mover-prop": {
                "description": "a shadow-casting copy of the receptacle housing "
                               "(prop_dynamic just_elevator.mdl, shadows not disabled) set at "
                               "the deployed housing's pose (320 0 -68): a legitimate occluder "
                               "sunk in the pit as the old frame box was; render-to-texture "
                               "shadows off (r_shadows 0) so the off shot holds none",
                "setup": ["r_shadows 0",
                          "ent_create prop_dynamic model models/props_basement/just_elevator.mdl "
                          "targetname mover_shadow_control",
                          "wait 30", "ent_fire mover_shadow_control addoutput \"origin 320 0 -68\"",
                          "ent_fire mover_shadow_control addoutput \"angles 0 90 0\"", "wait 120"],
                "published_near": [320.0, 0.0, -68.0], "published_radius": 96.0,
                "must_fail": ["no-mover-shadow"],
            },
        },
    },
    {
        "name": "stalemate",
        "description": "the stalemate room's button on its pedestal (prop_button and "
                       "pedestal_base parented to the func_door button_platform), from the "
                       "room's +y end",
        "brushes": STALEMATE_BRUSHES,
        "setup": ["wait 300"],
        "camera": {"setpos": [60, -1100, 90], "setang": [35, -100, 0]},
        # The pedestal's column and the arm models in the foreground (model
        # lighting); the pedestal's ring under the button is left out: the
        # button's own boxes shade it.
        "regions": {"pedestal-column": [0.54, 0.41, 0.59, 0.49],
                    "arms": [0.03, 0.62, 0.12, 0.73]},
        "controls": {},
    },
]


# ---------------------------------------------------------------------------
# Facts: the scenario's movers are brush entities of the map (recorded facts).

def map_facts(runtime, scenario):
    bsp = LegacyBsp.read(runtime / "portal2/maps/sp_a2_core.bsp")
    entities = bsp.entities()
    found = {}
    for name in scenario["brushes"]:
        matches = [e for e in entities if e.get("targetname") == name]
        found[name] = sorted({e.get("classname") for e in matches})
    brush = all(found[name] and all(e.get("model", "").startswith("*")
                                    for e in entities if e.get("targetname") == name)
                for name in scenario["brushes"])
    return {"brush_movers": found, "all_brush_models": brush}


# ---------------------------------------------------------------------------
# The run: one boot, three shots.

def console_line(scenario, control):
    camera = scenario["camera"]
    steps = ["noclip"] + list(scenario["setup"])
    if control:
        steps += list(control["setup"])
    steps += ["cmd setpos %g %g %g" % tuple(camera["setpos"]),
              "cmd setang %g %g %g" % tuple(camera["setang"]), "wait 120",
              "echo %s report" % MARK, "r_dynamic_occlusion_report 1", "wait 5"]
    for label in SHOTS:
        if label == "off":
            steps += ["r_dynamic_occlusion 0"]
        steps += ["wait 120", "echo %s %s" % (MARK, label), "screenshot", "wait 10"]
    steps += ["r_dynamic_occlusion 1", "echo %s end" % MARK]
    return "; ".join(steps)


def boot(args, scenario, control, out):
    line = console_line(scenario, control)
    command = [sys.executable, str(PORTAL_BOOT), "--game", "portal2",
               "--runtime", str(p2_runtime(args)), "--build", str(args.p2_build),
               "--out", str(out), "--headless", "--map", "sp_a2_core",
               "--renderer", "native-vulkan", "--require-vulkan", "--physics", "vphysics_box3d",
               "--width", str(WIDTH), "--height", str(HEIGHT), "--capture-wait", "2600",
               "--timeout", str(args.timeout), "--console-command", line]
    for startup in STARTUP:
        command += ["--startup-command", startup]
    completed = subprocess.run(command, capture_output=True, text=True)
    screenshots = sorted((out / "runtime/portal2/screenshots").glob("*.tga"))
    console = out / "runtime/portal2/console.log"
    text = console.read_text(errors="replace") if console.is_file() else ""
    return {"returncode": completed.returncode, "console_line": line,
            "boot_tail": (completed.stdout[-600:] + completed.stderr[-600:]).strip(),
            "screenshots": [str(p) for p in screenshots],
            "markers": re.findall(r"^%s (\S+)" % MARK, text, re.MULTILINE),
            "report": parse_report(text)}


def p2_runtime(args):
    if args.p2_runtime is None:
        args.p2_runtime = args.out.resolve() / "p2content"
        if not args.p2_runtime.exists():
            import stage_portal2_runtime  # noqa: E402 (needs the Steam install only here)
            stage_portal2_runtime.stage_content(args.steam_root, args.p2_runtime)
    return args.p2_runtime


BOX_LINE = re.compile(r"^\s+box entity (-?\d+) part (\d+) v\d+ at (\S+) (\S+) (\S+) radius (\S+)")


def parse_report(text):
    """The first occlusion report after the report marker: {generation, boxes}."""
    after = text.split("%s report" % MARK, 1)
    if len(after) < 2:
        return None
    report = None
    for line in after[1].splitlines():
        match = re.match(r"^dynamic occlusion generation (\d+): (\d+) box", line)
        if match and report is None:
            report = {"generation": int(match.group(1)), "count": int(match.group(2)),
                      "boxes": []}
            continue
        if report is None:
            continue
        box = BOX_LINE.match(line)
        if box:
            report["boxes"].append({"entity": int(box.group(1)), "part": int(box.group(2)),
                                    "center": [float(v) for v in box.group(3, 4, 5)],
                                    "radius": float(box.group(6))})
        elif not line.startswith("  "):
            break
    return report


# ---------------------------------------------------------------------------
# The oracle: pure functions over shots and regions.

LUMA = numpy.array([0.2126, 0.7152, 0.0722])


def luma(image):
    return numpy.asarray(image, dtype=numpy.float64)[..., :3] @ LUMA


def region(values, box):
    height, width = values.shape[:2]
    x0, y0, x1, y1 = box
    return values[int(y0 * height):int(y1 * height), int(x0 * width):int(x1 * width)]


def judge(shots, scenario, report, thresholds=THRESHOLDS, control=None):
    """[(check, passed, detail)] for one run's shots {label: HxWx3 array}."""
    verdicts = []
    have = all(label in shots for label in SHOTS)
    verdicts.append(("boot", have and report is not None,
                     "shots %s, report %s" % (sorted(shots), report is not None)))
    if not have:
        return verdicts
    on, on2, off = (luma(shots[label]) for label in SHOTS)
    worst_static, worst_mean, worst_fraction, where = 0.0, 0.0, 0.0, None
    for name, box in scenario["regions"].items():
        a, b, c = region(on, box), region(on2, box), region(off, box)
        worst_static = max(worst_static, float(numpy.abs(b - a).mean()))
        darkening = c - a
        mean = float(darkening.mean())
        fraction = float((darkening > thresholds["dark_level"]).mean())
        if mean > worst_mean or fraction > worst_fraction:
            where = name
        worst_mean, worst_fraction = max(worst_mean, mean), max(worst_fraction, fraction)
    verdicts.append(("static", worst_static <= thresholds["static_mean_max"],
                     "regions change by %.3f levels between two on shots (limit %.2f)" % (
                         worst_static, thresholds["static_mean_max"])))
    verdicts.append(("occlusion-active", bool(report) and report.get("generation", 0) > 0,
                     "occlusion report %s" % (
                         "missing" if not report else "generation %d, %d box(es)" % (
                             report["generation"], report["count"]))))
    verdicts.append(("no-mover-shadow",
                     worst_mean <= thresholds["mean_darkening_max"]
                     and worst_fraction <= thresholds["dark_fraction_max"],
                     "occlusion darkens %s by %.2f levels on average, %.2f %% of pixels by more "
                     "than %g (limits %.2f, %.2f %%)" % (
                         where or "no region", worst_mean, 100.0 * worst_fraction,
                         thresholds["dark_level"], thresholds["mean_darkening_max"],
                         100.0 * thresholds["dark_fraction_max"])))
    if control and "published_near" in control:
        near = numpy.array(control["published_near"])
        boxes = (report or {}).get("boxes", [])
        close = [b for b in boxes
                 if numpy.linalg.norm(numpy.array(b["center"]) - near) <= control["published_radius"]]
        verdicts.append(("control-published", bool(close),
                         "%d published box(es) within %g of %s" % (
                             len(close), control["published_radius"], near.tolist())))
    return verdicts


def load_shots(run):
    paths = run["screenshots"]
    return {label: numpy.asarray(Image.open(path).convert("RGB"))
            for label, path in zip(SHOTS, paths)}


# ---------------------------------------------------------------------------
# Commands.

def cmd_suite(args):
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    checks = conformance_result.Checks()
    evidence = {"schema": SCHEMA, "thresholds": THRESHOLDS,
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "p2_build": str(args.p2_build.resolve()), "scenarios": []}
    for index, scenario in enumerate(SCENARIOS):
        if args.scenario and scenario["name"] not in args.scenario:
            continue
        name = "sp_a2_core.%s" % scenario["name"]
        facts = map_facts(p2_runtime(args), scenario)
        record = {"scenario": scenario["name"], "facts": facts, "runs": []}
        evidence["scenarios"].append(record)
        checks.check(facts["all_brush_models"], name + ".facts.brush-movers",
                     "the scenario's movers are not all brush entities: %s" % facts["brush_movers"])
        run = boot(args, scenario, None, out / ("s%d" % index))
        verdicts = judge(load_shots(run), scenario, run["report"])
        run["verdicts"] = verdicts
        record["runs"].append(run)
        for check, passed, detail in verdicts:
            checks.check(passed, "%s.%s" % (name, check), detail)
        if args.no_controls:
            continue
        for control_name, control in scenario["controls"].items():
            run = boot(args, scenario, control, out / ("s%d-%s" % (index, control_name)))
            verdicts = judge(load_shots(run), scenario, run["report"], control=control)
            run["control"], run["verdicts"] = control_name, verdicts
            record["runs"].append(run)
            label = "%s.control.%s" % (name, control_name)
            failed = {check for check, passed, _ in verdicts if not passed}
            for check in ("boot", "static", "control-published"):
                detail = next((d for c, _, d in verdicts if c == check), "not judged")
                checks.check(check not in failed and any(c == check for c, _, _ in verdicts),
                             "%s.%s" % (label, check), detail)
            for must in control["must_fail"]:
                checks.check(must in failed, "%s.rejects.%s" % (label, must),
                             "the oracle accepted the control on %s: %s" % (
                                 must, next((d for c, _, d in verdicts if c == must), "")))
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2, default=str) + "\n")
    print("evidence: %s" % (out / "evidence.json"))
    return checks.report()


def synthetic(scenario, darken=0.0, noise=0.0, shadow_fraction=1.0, base=60.0):
    """Shots {label: image}: a flat frame, the regions darkened in the on shots by
    `darken` levels over `shadow_fraction` of their rows, and `noise` added to on2."""
    frame = numpy.full((96, 128, 3), base)
    shots = {"off": frame.copy(), "on": frame.copy(), "on2": frame.copy()}
    for box in scenario["regions"].values():
        height, width = frame.shape[:2]
        x0, y0, x1, y1 = box
        rows = int((y1 - y0) * height * shadow_fraction)
        for label in ("on", "on2"):
            shots[label][int(y0 * height):int(y0 * height) + rows,
                         int(x0 * width):int(x1 * width)] -= darken
        shots["on2"][int(y0 * height):int(y1 * height), int(x0 * width):int(x1 * width)] += noise
    return {k: numpy.clip(v, 0, 255) for k, v in shots.items()}


def cmd_selftest(_args):
    checks = conformance_result.Checks()
    report = {"generation": 5, "count": 1,
              "boxes": [{"entity": 1, "part": 0, "center": [320, 0, -70], "radius": 94}]}

    def verdict(verdicts, check):
        return next((passed for c, passed, _ in verdicts if c == check), None)

    for scenario in SCENARIOS:
        name = scenario["name"]
        clean = judge(synthetic(scenario), scenario, report)
        checks.check(all(passed for _, passed, _ in clean), name + ".clean-passes",
                     "; ".join(d for _, p, d in clean if not p))
        # The defect: a hard shadow over a third of a region.
        dark = judge(synthetic(scenario, darken=20.0, shadow_fraction=0.34), scenario, report)
        checks.check(verdict(dark, "no-mover-shadow") is False, name + ".shadow-rejected")
        # A faint but wide shadow (mean over the limit, no pixel over 4 levels).
        faint = judge(synthetic(scenario, darken=3.0), scenario, report)
        checks.check(verdict(faint, "no-mover-shadow") is False, name + ".faint-shadow-rejected")
        # Brighter with occlusion on is not a mover shadow.
        brighter = judge(synthetic(scenario, darken=-10.0), scenario, report)
        checks.check(verdict(brighter, "no-mover-shadow") is True, name + ".brighter-not-shadow")
        noisy = judge(synthetic(scenario, noise=2.0), scenario, report)
        checks.check(verdict(noisy, "static") is False, name + ".animated-region-rejected")
        missing = judge({"on": synthetic(scenario)["on"]}, scenario, report)
        checks.check(verdict(missing, "boot") is False and len(missing) == 1,
                     name + ".missing-shots-rejected")
        silent = judge(synthetic(scenario), scenario, None)
        checks.check(verdict(silent, "boot") is False
                     and verdict(silent, "occlusion-active") is False,
                     name + ".missing-report-rejected")
    control = SCENARIOS[0]["controls"]["mover-prop"]
    seen = judge(synthetic(SCENARIOS[0]), SCENARIOS[0], report, control=control)
    checks.check(verdict(seen, "control-published") is True, "control.published-found")
    elsewhere = dict(report, boxes=[dict(report["boxes"][0], center=[0, 0, 0])])
    unseen = judge(synthetic(SCENARIOS[0]), SCENARIOS[0], elsewhere, control=control)
    checks.check(verdict(unseen, "control-published") is False, "control.unpublished-rejected")
    text = ("junk\n%s report\ndynamic occlusion generation 12: 2 box(es), 0 changed\n"
            "  model lights: 3 judged, 0 dimmed by a box\n"
            "  box entity 77 part 1 v3 at 1.0 -2.5 3.0 radius 4.5 reach 27\n"
            "%s on\n" % (MARK, MARK))
    parsed = parse_report(text)
    checks.check(parsed is not None and parsed["generation"] == 12 and parsed["count"] == 2
                 and parsed["boxes"] == [{"entity": 77, "part": 1, "center": [1.0, -2.5, 3.0],
                                          "radius": 4.5}], "parse-report", repr(parsed))
    checks.check(parse_report("no marker") is None, "parse-report.no-marker")
    line = console_line(SCENARIOS[0], control)
    checks.check(line.index("ent_create") < line.index("r_dynamic_occlusion 0")
                 and line.count("screenshot") == len(SHOTS), "console-line")
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    commands = parser.add_subparsers(dest="command", required=True)
    suite = commands.add_parser("suite", help="boot, shoot and judge sp_a2_core")
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
