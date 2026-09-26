#!/usr/bin/env python3
"""In-engine fixtures for spark-burst lights (RFC 0011, render.spark-light.v2).

`run` boots the Portal product headless on `spark_lab`
(tools/quality/spark_lab_map.py) with `fx_spark_lights_debug 2`, which logs
every live spark and every burst light the client considers each frame, then
judges the log. `check` judges an existing console log.

    python3 tools/quality/spark_light_scene.py run --out quality-results/spark-light/scene
    python3 tools/quality/spark_light_scene.py check <console.log> [--layout layout.json]

Oracles (each reads the logged sparks, not the light's own arithmetic):

* coverage: in every frame, a burst's light reaches (lies within its radius
  of) at least 90% of that burst's spark emission;
* liveness: every burst with a live, emitting spark asks for a light that
  frame;
* carried: every trail spark belongs to a burst that carries a light (the
  little sparks of FX_ElectricSpark share the big sparks' burst);
* selection: at most the budget is lit; each lit burst ranks at least as high
  as every unlit one (importance at the logged view, a lit-last-frame burst
  counted 1.25 times); and when the budget row fires, its lit bursts are the
  ones nearest the view;
* scenes: every scene of the map produced a burst, and the fall scene's
  sparks reached the floor.

The report also replays the first policy (only sparks within the base radius
of the source count, the light at their centroid with the base radius) on the
same logged sparks, as the in-engine negative control: its coverage must fail
in the fall scene.
"""

import argparse
import datetime
import json
import math
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
SCHEMA = "spark-light-scene/v1"
BUDGET = 4
KEEP_LIT_BIAS = 1.25
COVERAGE = 0.9
# Base radii of the map's bursts (fx_sparks.cpp: 64 + 64 x magnitude).
BASE_RADIUS = {"fall": 192.0, "wall": 192.0, "pcf": 160.0}
BUDGET_BASE_RADIUS = 128.0
# Camera positions (feet) and angles for --view: the fall scene from the side,
# its source at the top left and its landing zone in front.
VIEWS = {"fall": ((-480, 280, 0), (-12, 225, 0)),
         "wall": ((-256, 360, 0), (10, 270, 0))}
# Frames after a --view scene's SparkOnce at which it is captured (60 fps; the
# event reaches the client about 100 frames later in a headless run).
SHOT_FRAMES = (100, 115, 130, 150, 175, 205, 240)


def parse_log(text):
    """Frames from a fx_spark_lights_debug 2 log: frame -> {"view", "lights"
    (key -> record), "sparks" (key -> [(x, y, z, emission)])}."""
    frames = defaultdict(lambda: {"view": None, "budget": None, "lights": {},
                                  "sparks": defaultdict(list)})
    for line in text.splitlines():
        fields = line.split()
        if len(fields) < 3 or fields[0] != "sparkdbg":
            continue
        try:
            kind, frame = fields[1], int(fields[2])
            values = [float(v) for v in fields[3:]]
        except ValueError:
            continue
        record = frames[frame]
        if kind == "view" and len(values) == 4:
            record["view"] = tuple(values[:3])
            record["budget"] = int(values[3])
        elif kind == "light" and len(values) == 8:
            key = int(values[0])
            record["lights"][key] = {"lit": values[1] != 0, "origin": tuple(values[2:5]),
                                     "radius": values[5], "strength": values[6],
                                     "spread": values[7]}
        elif kind == "spark" and len(values) == 5:
            record["sparks"][int(values[0])].append(tuple(values[1:5]))
    return dict(sorted(frames.items()))


def distance(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(a, b)))


def coverage(sparks, origin, radius):
    total = sum(s[3] for s in sparks if s[3] > 0)
    inside = sum(s[3] for s in sparks if s[3] > 0 and distance(s[:3], origin) <= radius)
    return inside / total if total > 0 else 1.0


def importance(light, view):
    reach = light["radius"] ** 2
    return light["strength"] * reach / (reach + distance(light["origin"], view) ** 2)


def first_policy_light(sparks, source, base_radius):
    """The first policy's light: the centroid of the sparks within the base
    radius of the source; None when none are."""
    near = [s for s in sparks if s[3] > 0 and distance(s[:3], source) <= base_radius]
    total = sum(s[3] for s in near)
    if total <= 0:
        return None
    return tuple(sum(s[k] * s[3] for s in near) / total for k in range(3))


def classify_bursts(frames, sources):
    """Each burst key's scene: the source its first light sits at (a trail
    burst is lit where it starts) or, for a system, the nearest source to its
    first light."""
    scenes = {}
    for frame in frames.values():
        for key, light in frame["lights"].items():
            if key in scenes:
                continue
            name = min(sources, key=lambda n: distance(sources[n], light["origin"]))
            scenes[key] = name if distance(sources[name], light["origin"]) <= 96.0 else None
    return scenes


def evaluate(frames, layout, expected_budget=BUDGET, required=None):
    """Judges a parsed log; `required` names the scenes that must produce a
    burst (default every scene of the layout)."""
    sources = {name: tuple(origin) for name, origin in layout["sources"].items()}
    scenes = classify_bursts(frames, sources)
    failures = []
    counts = defaultdict(int)
    worst_coverage = 1.0
    first_policy = {"frames": 0, "failed": 0, "dark_while_sparking": 0, "worst_coverage": 1.0}
    fall_floor = False
    budget_checks = 0
    previous_lit = set()
    first_seen = {}
    for number, frame in frames.items():
        for key in frame["lights"]:
            first_seen.setdefault(key, number)

    def fail(message):
        if len(failures) < 40:
            failures.append(message)
        counts["failures"] += 1

    for number, frame in frames.items():
        lights, sparks = frame["lights"], frame["sparks"]
        counts["frames"] += 1
        # Carried: no trail spark without a burst light.
        if sparks.get(0):
            fail("frame %d: %d spark(s) belong to no burst light" % (number, len(sparks[0])))
        for key, burst in sparks.items():
            if key == 0:
                continue
            emitting = [s for s in burst if s[3] > 0]
            if not emitting:
                continue
            counts["burst_frames"] += 1
            # Liveness.
            if key not in lights:
                fail("frame %d: burst %d has %d emitting spark(s) and asks for no light"
                     % (number, key, len(emitting)))
                continue
            light = lights[key]
            # Coverage.
            share = coverage(burst, light["origin"], light["radius"])
            worst_coverage = min(worst_coverage, share)
            if share < COVERAGE:
                fail("frame %d: burst %d (%s) reaches %.0f%% of its sparks' emission"
                     % (number, key, scenes.get(key), 100 * share))
            # The first policy on the same sparks.
            scene = scenes.get(key)
            if scene in BASE_RADIUS:
                base = BASE_RADIUS[scene]
                first_policy["frames"] += 1
                old = first_policy_light(burst, sources[scene], base)
                if old is None:
                    first_policy["dark_while_sparking"] += 1
                    first_policy["failed"] += 1
                    first_policy["worst_coverage"] = 0.0
                else:
                    old_share = coverage(burst, old, base)
                    first_policy["worst_coverage"] = min(first_policy["worst_coverage"], old_share)
                    first_policy["failed"] += old_share < COVERAGE
            if scene == "fall" and min(s[2] for s in emitting) < 24.0:
                fall_floor = True

        # Selection, against the budget the frame ran with (logged; else the
        # expected one).
        budget = frame["budget"] if frame["budget"] is not None else expected_budget
        lit = {key for key, light in lights.items() if light["lit"]}
        if len(lit) > budget:
            fail("frame %d: %d bursts lit, more than the budget %d" % (number, len(lit), budget))
        view = frame["view"]
        if view is not None and lights:
            rank = {key: importance(light, view) * (KEEP_LIT_BIAS if key in previous_lit else 1.0)
                    for key, light in lights.items()}
            unlit = [key for key in lights if key not in lit and rank[key] > 0]
            if unlit and len(lit) < budget:
                fail("frame %d: %d burst(s) unlit with the budget not spent" % (number, len(unlit)))
            if lit and unlit:
                lowest = min(rank[key] for key in lit)
                highest = max(rank[key] for key in unlit)
                # Logged strengths carry five decimals.
                if highest > lowest * 1.001 + 1e-6:
                    fail("frame %d: an unlit burst outranks a lit one (%.5g > %.5g)"
                         % (number, highest, lowest))
            # The budget row's first frame: its bursts start together at full
            # strength and their base radius, so the lit ones are the nearest.
            row = [key for key in lights if first_seen.get(key) == number
                   and (scenes.get(key) or "").startswith("budget")]
            if len(row) == len(layout["budget_sources"]):
                budget_checks += 1
                by_distance = sorted(row, key=lambda k: distance(lights[k]["origin"], view))
                lit_row = [key for key in row if key in lit]
                if set(lit_row) != set(by_distance[:len(lit_row)]):
                    fail("frame %d: the budget row's lit bursts are not the nearest the view"
                         % number)
        previous_lit = lit

    seen = defaultdict(int)
    for key, scene in scenes.items():
        seen[scene] += 1
    required = list(sources) if required is None else required
    for name in required:
        if not seen.get(name):
            fail("scene %s produced no burst light" % name)
    if "fall" in required and not fall_floor:
        fail("the fall scene's sparks never reached the floor")
    if "budget0" in required and budget_checks == 0:
        fail("the budget row never fired all six bursts together")
    if counts["burst_frames"] == 0:
        fail("no burst frames logged (fx_spark_lights_debug 2?)")
    if first_policy["frames"] and first_policy["failed"] == 0:
        fail("the first policy passes coverage on these sparks: the scene does not "
             "discriminate")
    return {"status": "pass" if counts["failures"] == 0 else "fail",
            "frames": counts["frames"], "burst_frames": counts["burst_frames"],
            "bursts": {str(k): v for k, v in sorted(scenes.items())},
            "scenes": dict(seen), "worst_coverage": round(worst_coverage, 4),
            "budget_row_checks": budget_checks, "fall_reached_floor": fall_floor,
            "first_policy": first_policy, "failure_count": counts["failures"],
            "failures": failures}


def load_layout(path):
    layout = json.loads(Path(path).read_text())
    layout["budget_sources"] = [n for n in layout["sources"] if n.startswith("budget")]
    return layout


def command_check(args):
    frames = parse_log(Path(args.log).read_text(errors="replace"))
    report = evaluate(frames, load_layout(args.layout), expected_budget=args.budget)
    print(json.dumps({k: v for k, v in report.items() if k != "failures"}, indent=2))
    for failure in report["failures"]:
        print("FAIL: " + failure)
    return 0 if report["status"] == "pass" else 1


def command_run(args):
    out = args.out.resolve()
    if out.exists():
        raise SystemExit("%s exists; use a new output directory" % out)
    published = ROOT / "run/maps/spark_lab/published.json"
    layout_path = ROOT / "quality-results/spark-light/map/layout.json"
    if not published.is_file() or not layout_path.is_file():
        raise SystemExit("build the map first: python3 tools/quality/spark_lab_map.py")
    content = Path(json.loads(published.read_text())["content_root"])
    boot = out / "boot"
    commands = ["fx_spark_lights %d" % args.budget, "fx_spark_lights_debug 2", "r_drawviewmodel 0"]
    if args.view:
        # The scene fires from the console, so each screenshot is a known time
        # into its burst: with the budget, then with no spark light at all.
        # One line: a cfg runs separate lines at once, a line's waits in turn.
        position, angles = VIEWS[args.view]
        sequence = ["cmd setpos %g %g %g" % position, "cmd setang %g %g %g" % angles,
                    "ent_fire spark_cycle Disable", "wait 240"]
        for budget in (args.budget, 0):
            sequence += ["fx_spark_lights %d" % budget, "ent_fire spark_%s SparkOnce" % args.view]
            waited = 0
            for frame in SHOT_FRAMES:
                sequence += ["wait %d" % (frame - waited), "screenshot"]
                waited = frame
            sequence.append("wait %d" % max(0, 400 - waited))
        commands.append("; ".join(sequence))
    command = [sys.executable, str(HERE / "portal_boot.py"), "--runtime", str(args.runtime),
               "--build", str(args.build), "--content-root", str(content), "--out", str(boot),
               "--map", "spark_lab", "--renderer", args.renderer, "--physics", args.physics,
               "--capture-wait", str(args.frames or (900 if args.view else 1800)),
               "--timeout", str(args.timeout),
               "--width", "1280", "--height", "720", "--no-mouse"]
    if args.headless:
        command.append("--headless")
    for line in commands:
        command += ["--console-command", line]
    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
    result = subprocess.run(command, capture_output=True, text=True)
    (out).mkdir(parents=True, exist_ok=True)
    (out / "portal_boot.log").write_text(result.stdout + result.stderr)
    log = boot / "runtime/portal/console.log"
    evidence = {"schema": SCHEMA, "started_utc": started, "command": command,
                "boot_status": result.returncode, "console_log": str(log)}
    if not log.is_file():
        evidence["status"] = "fail"
        evidence["error"] = "no console log"
    else:
        report = evaluate(parse_log(log.read_text(errors="replace")), load_layout(layout_path),
                          expected_budget=args.budget, required=[args.view] if args.view else None)
        evidence.update(report)
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    print(json.dumps({k: v for k, v in evidence.items() if k not in ("failures", "command")},
                     indent=2))
    for failure in evidence.get("failures", []):
        print("FAIL: " + failure)
    return 0 if evidence.get("status") == "pass" else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    run = sub.add_parser("run", help="boot spark_lab and judge its log")
    run.add_argument("--out", type=Path, required=True)
    run.add_argument("--runtime", type=Path, default=ROOT / "run/runtime")
    run.add_argument("--build", type=Path, default=ROOT / "build")
    run.add_argument("--renderer", default="native-vulkan")
    run.add_argument("--physics", default="vphysics_box3d")
    run.add_argument("--frames", type=int, default=None,
                     help="frames logged after the map settles (60 fps: 30 s, three cycles)")
    run.add_argument("--timeout", type=float, default=300)
    run.add_argument("--budget", type=int, default=BUDGET)
    run.add_argument("--headless", action="store_true", default=True)
    run.add_argument("--view", choices=sorted(VIEWS),
                     help="fire one scene from the console (the map's timer off) and capture "
                          "its burst with the budget and with fx_spark_lights 0; only that "
                          "scene is required")
    check = sub.add_parser("check", help="judge an existing fx_spark_lights_debug 2 log")
    check.add_argument("log", type=Path)
    check.add_argument("--layout", type=Path,
                       default=ROOT / "quality-results/spark-light/map/layout.json")
    check.add_argument("--budget", type=int, default=BUDGET)
    args = parser.parse_args(argv)
    return command_run(args) if args.command == "run" else command_check(args)


if __name__ == "__main__":
    sys.exit(main())
