#!/usr/bin/env python3
"""RFC 0011 G9 evidence: a bulb swinging on a rope (fixture `swing`), in game.

    python3 tools/quality/gi_swing.py frozen --out quality-results/rfc0011-g9/frozen
    python3 tools/quality/gi_swing.py swinging --out quality-results/rfc0011-g9/swinging
    python3 tools/quality/gi_swing.py frozen --producer rayquery --out ...

The map (quality-results/rfc0011-maps/swing, or --map-build) carries the lamp:
a physics prop `Lamp` on a rope constraint from the ceiling, released 40
degrees out so it swings, with an inverse-square `light_dynamic` riding on
it, and a `point_teleport` `Lamp_hold_<k>` per reference angle.

`frozen` (G9.3). For each state (rest, left, right) the lamp is held
(`ent_fire Lamp DisableMotion`, then its hold's Teleport); the producer runs
WARM_FRAMES and then its declared convergence, CHANGE_FRAMES, before the
capture:

  <producer>-<state>-diffuse   mat_indirect_view 3: all diffuse light (the bake,
                               the producer's change, the bulb's shadowed direct
                               light), judged per world region against Cycles
                               DiffDir + DiffInd of the state: must pass
  <producer>-<state>-indirect  mat_indirect_view 1 against DiffInd: must pass
  radiosity-<state>-indirect   radiosity claims no LightMotion, so it lacks the
                               bulb's bounce: must fail
  <producer>-right-no-shadows  the diffuse view with r_indirect_shadows 0: the
                               pillar's shadow on the east wall and floor is
                               lost, so it must fail

Every diffuse capture is also judged on its shadow: the east-wall and floor
pixels where Cycles' direct light is below SHADOW_FRACTION of the region's
(per pixel, Combined / albedo - DiffInd on the white Lambertian surfaces).
There the measured light must match the reference's within the tolerance;
without shadows it is several times too bright.

Models are measured, not gated: the probe sphere's light from the bulb is the
engine's dlight path (unshadowed), which the diffuse view does not show.

`swinging` (G9.4). Every run fixes the frame step (host_framerate 60), holds
the lamp at its left extreme, lets the producer warm up, releases it
(`EnableMotion`) and, RELEASE_FRAMES later, records MOVIE_FRAMES consecutive
frames of the shaded view with `startmovie`. With the step fixed the swing
replays identically, so frame k of every run shows the lamp at the same
place (to within a third of a frame's motion: the physics is not bit
identical between runs). Each world region's mean light per frame is a smooth
curve under the motion; its high-frequency term is the RMS of the curve's
second temporal difference over its mean. Baked (the same moving direct light
and shadows, no bounce) gives the motion's own term; a producer passes when
no region's term exceeds baked's by more than HF_LIMIT, which is what
flicker adds. A second baked run is the pairing control: it must also stay
within HF_LIMIT of the first.

`cost` (G9.5). At 1920 x 1080 with the lamp held at rest and the SDF
producer settled, the frame's GPU time (-vkframestats timestamps) with the
SDF shadow on, off and on again: the shadow's cost is the median of the
on phases less the off phase's, against SHADOW_BUDGET_MS.
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import frame_pacing  # noqa: E402
import gi_reference  # noqa: E402
import gi_runtime  # noqa: E402
import gi_temporal  # noqa: E402
import pbrt_map_toolchain  # noqa: E402
import reference_compare  # noqa: E402

MAPS = ROOT / "quality-results" / "rfc0011-maps"
FIXTURE = ROOT / "quality" / "fixtures" / "gi" / "swing" / "fixture.json"
WARM_FRAMES = 150
CHANGE_FRAMES = 64
# TracedProducer::Caps().responseTolerance.
RESPONSE_TOLERANCE = 0.1
MOVIE_FRAMES = 90
MOVIE_SKIP = 5
RELEASE_FRAMES = 30
# View 3's exposure: the bulb lights surfaces near it up to about 4.4
# (irradiance / pi), which an 8-bit screenshot would clip at 1.
DIFFUSE_SCALE = 0.125
# The high-frequency term a producer may add to the baked control's (a
# fraction of the burst's mean luminance, like gi_temporal's limits).
HF_LIMIT = 0.002
# RFC 0011 G9.5 (provisional): one light's SDF shadow at 1920 x 1080 on the
# desktop GPU.
SHADOW_BUDGET_MS = 1.0
COST_PHASE_FRAMES = 300
# G6.4 and G7.3's per-update budgets (quality/budgets/indirect-light-v1.json).
PRODUCER_BUDGET_MS = {"sdf": 2.0, "rayquery": 1.5}


def hold(state):
    index = json.loads(FIXTURE.read_text())["states"][state]["hold"]
    return "ent_fire Lamp DisableMotion; ent_fire Lamp_hold_%d Teleport" % index


def select(producer):
    return "r_indirect_report 1; r_indirect_producer %s" % producer


def capture(args, name, command, view, wait):
    target = Path(args.out) / name
    run = [sys.executable, HERE / "gi_runtime.py", "capture", "--fixture", "swing",
           "--map-build", Path(args.map_build or MAPS / "swing"), "--out", target,
           "--build", args.build, "--console-command", command, "--capture-wait",
           str(gi_runtime.PLACEMENT_FRAMES + wait), "--view", str(view)]
    if view == 3:
        run += ["--scale", str(DIFFUSE_SCALE)]
    booted = subprocess.run([str(part) for part in run], capture_output=True, text=True,
                            timeout=1800).returncode == 0
    return target, booted


def compare(target, state, light):
    command = [sys.executable, str(HERE / "gi_runtime.py"), "compare", "--fixture", "swing",
               "--state", state, "--capture", str(target), "--out", str(target / "gate.json"),
               "--absolute-fraction", str(RESPONSE_TOLERANCE), "--reference-light", light]
    if light == "diffuse":
        command.append("--gate-world")
    subprocess.run(command, capture_output=True, text=True, timeout=600)
    return json.loads((target / "gate.json").read_text()) \
        if (target / "gate.json").is_file() else {}


SHADOW_FRACTION = 0.1
WHITE_ALBEDO = 0.7
SHADOW_REGIONS = ("east_wall", "floor")


def shadow_check(target, state):
    """The shadow's measured and reference light (Cycles, per pixel) over the
    state's shadowed east-wall and floor pixels."""
    import imageio.v3 as iio
    fixture = gi_reference.load_fixture("swing")
    references = fixture["directory"] / "references"
    view, masks = gi_runtime.view_masks(fixture, state, "room")
    total = np.asarray(iio.imread(references / view["files"]["total"]["file"]),
                       dtype=np.float64)[..., :3].mean(axis=2)
    indirect = np.asarray(iio.imread(references / view["files"]["indirect"]["file"]),
                          dtype=np.float64)[..., :3].mean(axis=2)
    light = total / WHITE_ALBEDO
    direct = light - indirect
    shadow = np.zeros(direct.shape, dtype=bool)
    for region in SHADOW_REGIONS:
        mask = masks[region]
        shadow |= mask & (direct < SHADOW_FRACTION * direct[mask].mean())
    record = json.loads((Path(target) / "capture.json").read_text())
    shot = record["cameras"]["room"]["screenshot"]
    measured = gi_runtime.linear_capture(shot, fixture["film"], record["scale"]).mean(axis=2)
    if shadow.sum() < 50:
        return {"pixels": int(shadow.sum()), "status": "too-small"}
    expected, observed = float(light[shadow].mean()), float(measured[shadow].mean())
    ok = abs(observed - expected) <= RESPONSE_TOLERANCE * expected + \
        RESPONSE_TOLERANCE * float(light[masks["floor"]].mean())
    return {"pixels": int(shadow.sum()), "reference": expected, "measured": observed,
            "status": "pass" if ok else "fail"}


def regions(gate):
    return {region: {"measured": entry["measured_luminance"],
                     "reference": entry["reference_luminance"],
                     "status": entry["status"]}
            for view in gate.get("views", {}).values() for region, entry in view.items()
            if "measured_luminance" in entry}


def frozen(args):
    producer = args.producer
    wait = WARM_FRAMES + CHANGE_FRAMES
    captures = {}
    for state in ("rest", "left", "right"):
        held = "%s; %s; wait %d" % (hold(state), select(producer), wait)
        captures["%s-%s-diffuse" % (producer, state)] = (state, held, 3, "diffuse", True)
        captures["%s-%s-indirect" % (producer, state)] = (state, held, 1, "indirect", True)
        captures["radiosity-%s-indirect" % state] = (
            state, "%s; %s; wait %d" % (hold(state), select("radiosity"), wait), 1, "indirect",
            False)
    captures["%s-right-no-shadows" % producer] = (
        "right", "r_indirect_shadows 0; %s; %s; wait %d" % (hold("right"), select(producer),
                                                           wait), 3, "diffuse", False)
    results, passed = {}, True
    for name, (state, command, view, light, should_pass) in captures.items():
        target, booted = capture(args, name, command, view, wait)
        gate = compare(target, state, light)
        shadow = shadow_check(target, state) if booted and light == "diffuse" else None
        judged = gate.get("status") == "pass" and (shadow is None or shadow["status"] == "pass")
        ok = booted and bool(gate) and judged == should_pass
        passed &= ok
        results[name] = {"state": state, "command": command, "view": view, "light": light,
                         "expected": "pass" if should_pass else "fail", "booted": booted,
                         "compare": gate.get("status"), "regions": regions(gate),
                         "shadow": shadow,
                         "failures": gate.get("failures", []),
                         "status": "pass" if ok else "fail"}
        print("%-26s %-6s view %d expected %-4s compare %-4s shadow %-4s -> %s" % (
            name, state, view, results[name]["expected"], gate.get("status"),
            shadow["status"] if shadow else "-", results[name]["status"]))
    return write(args, "G9.3 swing frozen", passed, captures=results, producer=producer,
                 warm_frames=WARM_FRAMES, change_frames=CHANGE_FRAMES,
                 response_tolerance=RESPONSE_TOLERANCE)


SWING_REGIONS = ("floor", "red_wall", "back_wall", "east_wall")


def region_curves(frames, masks):
    """{region: its mean luminance per frame} of a (T, H, W) burst at 4x
    the reference film (the masks' resolution)."""
    count, height, width = frames.shape
    film = frames.reshape(count, height // 4, 4, width // 4, 4).mean(axis=(2, 4))
    return {region: film[:, masks[region]].mean(axis=1) for region in SWING_REGIONS}


def high_frequency(curve):
    """A region curve's RMS second temporal difference over its mean."""
    second = curve[2:] - 2.0 * curve[1:-1] + curve[:-2]
    return float(np.sqrt(np.mean(second ** 2))) / max(float(curve.mean()), 1e-9)


def swinging(args):
    fixture = gi_reference.load_fixture("swing")
    _, masks = gi_runtime.view_masks(fixture, "rest", "room")
    curves, results, passed = {}, {}, True
    runs = ["baked", "baked-again"] + args.producer_list
    for run in runs:
        producer = run.split("-")[0]
        # startmovie takes its rate from host_framerate (30 without one).
        command = ("host_framerate 60; %s; %s; wait %d; ent_fire Lamp EnableMotion; wait %d; "
                   "startmovie swing tga; wait %d; endmovie; host_framerate 0") % (
                       hold("left"), select(producer), WARM_FRAMES, RELEASE_FRAMES, MOVIE_FRAMES)
        target, booted = capture(args, run + "-swinging", command, 0,
                                 WARM_FRAMES + RELEASE_FRAMES + MOVIE_FRAMES + 30)
        # The first frames are written before the fixed step takes effect:
        # dropped.
        frames = sorted(Path(target).glob("*/runtime/portal/swing*.tga"))[MOVIE_SKIP:]
        results[run] = {"booted": booted, "command": command, "frames": len(frames)}
        if len(frames) >= 20:
            curves[run] = region_curves(gi_temporal.luminance_frames(frames), masks)
            results[run]["high_frequency"] = {region: high_frequency(curve)
                                              for region, curve in curves[run].items()}
    control = results["baked"].get("high_frequency")
    for run in runs[1:]:
        record = results[run]
        terms = record.get("high_frequency")
        record["added"] = {region: terms[region] - control[region] for region in terms} \
            if terms and control else None
        record["pass"] = bool(record["added"]) and max(record["added"].values()) <= HF_LIMIT
        passed &= record["pass"]
        print("%-12s frames %3d  added %s  -> %s" % (
            run, record["frames"],
            " ".join("%s %+.5f" % item for item in sorted((record["added"] or {}).items())),
            "pass" if record["pass"] else "fail"))
    return write(args, "G9.4 swing swinging", passed and control is not None, runs=results,
                 hf_limit=HF_LIMIT, movie_frames=MOVIE_FRAMES, release_frames=RELEASE_FRAMES,
                 warm_frames=WARM_FRAMES)


def cost(args):
    fixture = gi_reference.load_fixture("swing")
    profile, _ = pbrt_map_toolchain.load_profiles()
    toolchain = pbrt_map_toolchain.load(ROOT / profile["layout"]["toolchain_file"])
    map_build = Path(args.map_build or MAPS / "swing")
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    # A short relative name: the engine's whole command line holds 512
    # characters. It lands in the private runtime's working directory.
    stats_name = "g9-frame-stats.jsonl"
    prelude, _ = reference_compare.camera_commands(gi_runtime.camera_scene(fixture, "room"))
    # A cfg line holds 512 characters and `wait` holds only within its line:
    # the phases are an alias chain, one alias per line.
    # Then the lamp swings: the SDF producer, then the ray-query one (after
    # its warm-up), update every frame.
    marks = (("shadows_on", "r_indirect_shadows 1"), ("shadows_off", "r_indirect_shadows 0"),
             ("shadows_on_again", "r_indirect_shadows 1"),
             ("swinging_sdf", "ent_fire Lamp EnableMotion"),
             # The swing dies away within about 15 s: released again.
             ("swinging_rayquery", "r_indirect_producer rayquery; wait 150; "
              "%s; wait 2; ent_fire Lamp EnableMotion" % hold("left")))
    aliases = []
    for index, (mark, action) in enumerate(marks):
        after = "; g9p%d" % (index + 1) if index + 1 < len(marks) else ""
        aliases.append('alias g9p%d "%s; wait 30; vk_frame_mark %s; wait %d%s"'
                       % (index, action, mark, COST_PHASE_FRAMES, after))
    line = "; ".join(["r_drawvgui 0"] + prelude + [
        "%s; %s; wait %d; g9p0" % (hold("rest"), select("sdf"), WARM_FRAMES + CHANGE_FRAMES)])
    frames = WARM_FRAMES + CHANGE_FRAMES + 5 * (COST_PHASE_FRAMES + 30) + 150 + 60
    run = [sys.executable, HERE / "portal_boot.py", "--runtime", toolchain["runtime"],
           "--build", args.build, "--content-root", map_build / "content",
           "--renderer", "native-vulkan", "--headless", "--map", "gi_swing",
           "--width", "1920", "--height", "1080",
           "--capture-wait", str(gi_runtime.PLACEMENT_FRAMES + frames),
           "--engine-arg=-vkframestats", "--engine-arg=%s" % stats_name,
           "--out", out / "boot"]
    for command in aliases + [line]:
        run += ["--console-command", command]
    boot = subprocess.run([str(part) for part in run], capture_output=True, text=True,
                          timeout=1800)
    booted = boot.returncode == 0
    found = sorted((out / "boot").rglob(stats_name))
    stats = found[0] if found else out / stats_name
    if not stats.is_file():
        print(boot.stdout[-2000:] + boot.stderr[-2000:])
        return write(args, "G9.5 swing cost", False, booted=booted, error="no frame stats")
    _, records, _ = frame_pacing.read_stats(stats)
    gpu = {}
    for frame in records:
        if "gpu" in frame:
            gpu[frame["gpu"][0]] = frame["gpu"][1] / 1000.0
    phase, samples = None, {}
    for frame in records:
        for mark in filter(None, frame.get("mark", "").split(",")):
            phase = mark
        # Skip each phase's first 30 frames: the switch settles.
        if phase and frame["f"] in gpu:
            samples.setdefault(phase, []).append(gpu[frame["f"]])
    medians = {name: float(np.median(values[30:])) for name, values in samples.items()
               if len(values) > 60}
    shadow_ms = None
    if {"shadows_on", "shadows_off", "shadows_on_again"} <= medians.keys():
        shadow_ms = (medians["shadows_on"] + medians["shadows_on_again"]) / 2.0 - \
            medians["shadows_off"]
    # A swinging lamp's producer update, every frame: the frame over the
    # still, shadowed one.
    motion_ms = {producer: medians["swinging_" + producer] - medians["shadows_on_again"]
                 for producer in ("sdf", "rayquery")
                 if "swinging_" + producer in medians and "shadows_on_again" in medians}
    motion_ok = len(motion_ms) == 2 and all(
        motion_ms[producer] <= PRODUCER_BUDGET_MS[producer] for producer in motion_ms)
    passed = booted and shadow_ms is not None and shadow_ms <= SHADOW_BUDGET_MS and motion_ok
    print("producer update while swinging: %s ms (budgets %s)" % (
        ", ".join("%s %.3f" % item for item in sorted(motion_ms.items())), PRODUCER_BUDGET_MS))
    print("GPU frame ms: %s; SDF shadow %s ms (budget %.1f)" % (
        ", ".join("%s %.3f" % item for item in sorted(medians.items())),
        "%.3f" % shadow_ms if shadow_ms is not None else "-", SHADOW_BUDGET_MS))
    return write(args, "G9.5 swing cost", passed, gpu_frame_ms_median=medians,
                 shadow_ms=shadow_ms, shadow_budget_ms=SHADOW_BUDGET_MS, booted=booted,
                 producer_motion_ms=motion_ms, producer_budget_ms=PRODUCER_BUDGET_MS,
                 resolution=[1920, 1080], samples={k: len(v) for k, v in samples.items()})


def write(args, gate, passed, **record):
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    record = dict({"gate": gate, "status": "pass" if passed else "fail", "build": args.build},
                  **record)
    (out / "gate.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("%s: %s" % (gate, record["status"]))
    return 0 if passed else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("frozen", "swinging", "cost"):
        command = commands.add_parser(name)
        command.add_argument("--map-build", type=Path)
        command.add_argument("--out", required=True)
        command.add_argument("--build", default="build")
        if name == "frozen":
            command.add_argument("--producer", choices=("sdf", "rayquery"), default="sdf")
        else:
            command.add_argument("--producer", dest="producer_list", action="append",
                                 choices=("sdf", "rayquery", "radiosity"))
    args = parser.parse_args()
    if args.command == "swinging" and not args.producer_list:
        args.producer_list = ["sdf", "rayquery"]
    return {"frozen": frozen, "swinging": swinging, "cost": cost}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
