#!/usr/bin/env python3
"""RFC 0011 G6 evidence for the SDF-traced producer, in game.

    python3 tools/quality/gi_sdf.py door --out quality-results/rfc0011-g6/door
    python3 tools/quality/gi_sdf.py sun --out quality-results/rfc0011-g6/sun
    python3 tools/quality/gi_sdf.py door --producer rayquery --out quality-results/rfc0011-g7/door

`--producer rayquery` runs the same scenarios against the ray-query producer
(RFC 0011 G7.2, tolerances as tight as SDF's).

Each capture boots a fixture's map (quality-results/rfc0011-maps/<fixture>,
or --map-build; built with the gi-fixture profile, so it carries the SDFV)
headless on native Vulkan (gi_runtime.py capture), selects a producer, lets
it warm up for WARM_FRAMES, makes the scene change, and screenshots the
indirect-light view CHANGE_FRAMES later: the producer's declared 60 frames
(1 s) plus the input's latency. gi_runtime.py compare judges each capture
against the Cycles indirect-only reference of its state, world and model
regions gated, with the producer's declared response tolerance (0.1) as the
absolute allowance (a fraction of the reference level) beside the world
gate's relative 0.1. Every command writes gate.json; the exit status is
nonzero when any capture is not as expected.

`door` (G6.2): the door is a moving brush entity (`Door`, a func_brush open
at spawn), closed by `ent_fire Door Enable`. SDF open must match `open` and
SDF closed must match `closed`; radiosity and baked closed must NOT (they
claim no GeometryMotion): the scenario tells them apart. The closed room is
dark throughout (its reference is 0), so `closed` is judged at the `open`
state's light level. Those are indirect-light views; the shaded view (what a
player sees, the baked direct light included) must also go dark when the
door closes: every region at most SHADED_DARK of its open-door light, which
it does not with `r_indirect_occlusion 0` (the negative control).

`sun` (G6.3, "sun angle changes converge"): room-states' sun (its light
style from the radiosity transfer) is moved to the fixture's recorded sun-low
direction by `r_indirect_light_direction`. SDF must match `default` before
the move. After it, the capture CHANGE_FRAMES later must agree with one
SETTLE_FRAMES later still (settled), and every region must be closer to the
Cycles sun-low reference than the unchanged bake is; radiosity, which claims
no LightDirection, must not be. The regions' absolute error against sun-low
is recorded, not gated: this gate is convergence, not accuracy.
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import gi_runtime  # noqa: E402

MAPS = ROOT / "quality-results" / "rfc0011-maps"
# After the camera placement (gi_runtime.PLACEMENT_FRAMES), the producer is
# selected and runs WARM_FRAMES (past its warmupFrames, 72, and the switch's
# fade) before the scene changes; the screenshot follows CHANGE_FRAMES later.
WARM_FRAMES = 150
CHANGE_FRAMES = 64
SETTLE_FRAMES = 240  # the later capture a converged producer must agree with
# TracedProducer::Caps().responseTolerance: both traced producers declare it,
# so the ray-query producer's tolerance is exactly as tight as the SDF one's.
RESPONSE_TOLERANCE = 0.1
# A shaded closed-door region may keep at most this fraction of its shaded
# open-door light (Cycles: none; what remains is the door's own lit face
# bouncing and the capture's floor).
SHADED_DARK = 0.1


def run_captures(fixture, map_build, out, captures, level_state, args):
    """Each capture: (state, console command, expected to pass the state's
    gate (None: measured only)[, extra frames before the screenshot[, the
    view: 1 indirect light (default), 0 shaded]]). Returns (all as expected,
    per-capture results)."""
    results, passed = {}, True
    for name, (state, command, should_pass, *extra) in captures.items():
        settle = extra[0] if extra else 0
        view = extra[1] if len(extra) > 1 else 1
        target = out / name
        capture = [sys.executable, HERE / "gi_runtime.py", "capture", "--fixture", fixture,
                   "--map-build", map_build, "--out", target, "--build", args.build,
                   "--console-command", command, "--capture-wait",
                   str(gi_runtime.PLACEMENT_FRAMES + WARM_FRAMES + CHANGE_FRAMES + settle),
                   "--view", str(view)]
        if args.runtime:
            capture += ["--runtime", args.runtime]
        booted = subprocess.run([str(part) for part in capture], capture_output=True, text=True,
                                timeout=1200).returncode == 0
        compare = [sys.executable, str(HERE / "gi_runtime.py"), "compare", "--fixture", fixture,
                   "--state", state, "--capture", str(target), "--out",
                   str(target / "gate.json"), "--gate-models",
                   "--absolute-fraction", str(RESPONSE_TOLERANCE)]
        if level_state:
            compare += ["--level-state", level_state]
        compared = subprocess.run(compare, capture_output=True, text=True, timeout=600)
        gate = json.loads((target / "gate.json").read_text()) \
            if (target / "gate.json").is_file() else {}
        ok = booted and bool(gate) and (should_pass is None or
                                        (gate.get("status") == "pass") == should_pass)
        passed &= ok
        results[name] = {"state": state, "command": command, "booted": booted,
                         "settle_frames": settle, "regions": regions(gate),
                         "expected": {True: "pass", False: "fail", None: "measured"}[should_pass],
                         "compare": gate.get("status"), "status": "pass" if ok else "fail",
                         "producer_log": producer_log(target),
                         "compare_log": compared.stdout[-2000:]}
        print("%-17s %-8s expected %-4s compare %-4s -> %s" % (
            name, state, results[name]["expected"], gate.get("status"), results[name]["status"]))
    return passed, results


def regions(gate):
    """{region: (measured, reference)} luminance of a compare result."""
    return {region: (entry["measured_luminance"], entry["reference_luminance"])
            for view in gate.get("views", {}).values() for region, entry in view.items()
            if "measured_luminance" in entry}


def write(out, gate, passed, results, **extra):
    record = dict({"gate": gate, "status": "pass" if passed else "fail",
                   "warm_frames": WARM_FRAMES, "change_frames_before_capture": CHANGE_FRAMES,
                   "response_tolerance": RESPONSE_TOLERANCE, "captures": results}, **extra)
    out.mkdir(parents=True, exist_ok=True)
    (out / "gate.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("%s: %s" % (gate, record["status"]))
    return 0 if passed else 1


GATES = {"sdf": ("G6.2 door", "G6.3 sun"), "rayquery": ("G7.2 door", "G7.2 sun")}


def door(args):
    """G6.2 (and G7.2 with --producer rayquery). The closed room is dark
    throughout, so it is judged at the open state's light level."""
    producer = args.producer
    select = "r_indirect_report 1; r_indirect_producer %s; " % producer
    change = "wait %d; ent_fire Door Enable" % WARM_FRAMES
    captures = {
        producer + "-open": ("open", select + "wait %d" % WARM_FRAMES, True),
        producer + "-closed": ("closed", select + change, True),
        "radiosity-closed": ("closed", "r_indirect_report 1; r_indirect_producer radiosity; " +
                             change, False),
        "baked-closed": ("closed", "r_indirect_report 1; r_indirect_producer baked; " + change,
                         False),
        # What a player sees (shaded, direct light included): the closed room
        # must go dark, and does not without the moving-geometry occlusion.
        producer + "-shaded-open": ("open", select + "wait %d" % WARM_FRAMES, None, 0, 0),
        producer + "-shaded-closed": ("closed", select + change, None, 0, 0),
        producer + "-shaded-closed-no-occlusion": (
            "closed", "r_indirect_occlusion 0; " + select + change, None, 0, 0),
    }
    passed, results = run_captures("door", Path(args.map_build or MAPS / "door"),
                                   Path(args.out), captures, "open", args)
    opened = results[producer + "-shaded-open"]["regions"]
    dark = {name: {region: results[name]["regions"][region][0] <=
                   SHADED_DARK * opened[region][0] for region in opened}
            for name in (producer + "-shaded-closed", producer + "-shaded-closed-no-occlusion")}
    checks = {"shaded_closed_dark": all(dark[producer + "-shaded-closed"].values()),
              "without_occlusion_not_dark":
                  not all(dark[producer + "-shaded-closed-no-occlusion"].values())}
    for check, ok in checks.items():
        print("%-32s %s" % (check, "pass" if ok else "fail"))
    passed &= all(checks.values())
    return write(Path(args.out), GATES[producer][0], passed, results, build=args.build,
                 producer=producer, shaded_dark_fraction=SHADED_DARK, shaded_dark=dark,
                 checks=checks)


def sun(args):
    """G6.3 (and G7.2 with --producer rayquery). The sun (its light style
    from the radiosity transfer) is moved to the fixture's sun-low direction
    by the light-direction override."""
    producer = args.producer
    fixture = json.loads((ROOT / "quality/fixtures/gi/room-states/fixture.json").read_text())
    direction = fixture["states"]["sun-low"]["sun_direction"]
    map_build = Path(args.map_build or MAPS / "room-states")
    sources = json.loads((map_build / "lighting/radiosity/rtrn-bake.json").read_text())["sources"]
    style = next(source["style"] for source in sources if source["name"] == "Sun")
    select = "r_indirect_report 1; r_indirect_producer %s; " % producer
    change = "wait %d; r_indirect_light_direction %d %s" % (
        WARM_FRAMES, style, " ".join("%.6f" % v for v in direction))
    captures = {
        producer + "-default": ("default", select + "wait %d" % WARM_FRAMES, True),
        producer + "-sun-low": ("sun-low", select + change, None),
        producer + "-sun-low-settled": ("sun-low", select + change, None, SETTLE_FRAMES),
        "radiosity-sun-low": ("sun-low", "r_indirect_report 1; r_indirect_producer radiosity; " +
                              change, None),
    }
    passed, results = run_captures("room-states", map_build, Path(args.out), captures, None,
                                   args)
    # Settled: the capture CHANGE_FRAMES after the move agrees with the one
    # SETTLE_FRAMES later, per region, within the response tolerance.
    # Toward the reference: every region's error against the sun-low
    # reference is below the unchanged bake's (the default-state capture);
    # radiosity, which claims no LightDirection, must not get there.
    before = results[producer + "-default"]["regions"]
    early = results[producer + "-sun-low"]["regions"]
    late = results[producer + "-sun-low-settled"]["regions"]
    level = max(reference for _, reference in late.values())
    settled = {region: abs(early[region][0] - late[region][0]) <=
               RESPONSE_TOLERANCE * (late[region][0] + level) for region in late}

    def toward(moved):
        return {region: abs(moved[region][0] - moved[region][1]) <
                abs(before[region][0] - moved[region][1]) for region in moved}
    moved_toward = toward(early)
    radiosity_toward = toward(results["radiosity-sun-low"]["regions"])
    checks = {"settled": all(settled.values()),
              producer + "_toward_reference": all(moved_toward.values()),
              "radiosity_not_toward_reference": not all(radiosity_toward.values())}
    for check, ok in checks.items():
        print("%-32s %s" % (check, "pass" if ok else "fail"))
    passed &= all(checks.values())
    accuracy = {region: {"measured": value, "reference": reference,
                         "relative_error": abs(value - reference) / max(reference, 1e-9)}
                for region, (value, reference) in early.items()}
    return write(Path(args.out), GATES[producer][1], passed, results, build=args.build,
                 producer=producer, sun_style=style, sun_direction=direction, checks=checks,
                 settled=settled,
                 toward_reference={producer: moved_toward, "radiosity": radiosity_toward},
                 accuracy_measured=accuracy, settle_frames=SETTLE_FRAMES)


def producer_log(target):
    """The engine's indirect-light lines (producer switches, proxies)."""
    lines = []
    for log in sorted(Path(target).glob("*/runtime/engine.log")):
        for line in log.read_text(errors="replace").splitlines():
            if "indirect light:" in line and "indirect light: frame" not in line:
                lines.append(line.strip())
    return lines[-12:]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("door", "sun"):
        command = commands.add_parser(name)
        command.add_argument("--map-build", type=Path)
        command.add_argument("--out", required=True)
        command.add_argument("--build", default="build",
                             help="the client build tree carrying the producer")
        command.add_argument("--runtime", help="base runtime (a private copy is booted)")
        command.add_argument("--producer", choices=sorted(GATES), default="sdf",
                             help="the traced producer under test (rayquery: RFC 0011 G7)")
    args = parser.parse_args()
    return {"door": door, "sun": sun}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
