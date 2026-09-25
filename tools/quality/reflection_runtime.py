#!/usr/bin/env python3
"""In-game gates for the map's reflection probes (R50-PARALLAX), on native Vulkan.

    python3 tools/quality/reflection_runtime.py references --fixture mirror-room
    python3 tools/quality/reflection_runtime.py mirror \\
        --map-build quality-results/reflection-maps/mirror-room --out quality-results/x
    python3 tools/quality/reflection_runtime.py walk \\
        --map-build quality-results/reflection-maps/two-rooms --out quality-results/y
    python3 tools/quality/reflection_runtime.py relight \\
        --map-build quality-results/reflection-maps/mirror-lamp --out quality-results/z

`references` renders the Cycles references of a reflection fixture's cameras
(quality/fixtures/reflection/<fixture>/references/: linear Combined EXRs and
object-index PNGs) with gi_reference's renderer and its gi-reference light
paths, which trace the mirror floor's reflection exactly.

`mirror` is the parallax gate. It boots the mirror-room map headless three
times and screenshots every fixture camera (all placed away from the room's
probe) in one boot each: blended and parallax-corrected (`mat_reflection_probes
1`), direction-only (`3`, the old single-probe lookup), and blended over a map
whose RPRB boxes were shifted by (1.0, 0.8) m (a deliberately wrong box). Per
camera the capture is decoded to linear light, scaled by the gain that best
fits the lightmapped walls to Cycles (the walls do not depend on probes, so
this removes exposure and display scale), and the mirror floor's mean
absolute error is taken relative to the floor's mean reference. The walls'
own error after that fit (lightmap noise, 8-bit capture, edge registration:
about 0.10 here) is what any capture of this view can reach, and the probes
reflect those walls, so each view's floor error is judged as a ratio to its
wall error. The gate passes when every blended view's ratio is at most
MIRROR_RATIO_LIMIT and each control's mean ratio is at least CONTROL_MARGIN
times the blend's and above the limit. `--reuse` rescores existing boots.

`walk` is the blend gate. It boots the two-rooms map in the weight view
(`mat_reflection_probes 5`: each probe's palette colour times its weight, by
rank, on the mirror floor) and in the nearest-capture weight view (`6`,
Source 1's switch) and screenshots every station of the fixture's walk
through the doorway. The floor's pixels come from an analytic ray cast of
the fixture's boxes; on them the red share R / (R + B) is the rank-0 probe's
weight. A blend is continuous: no step between neighbouring floor pixels may
exceed WALK_STEP_LIMIT, while the walk must see both probes take over. The
nearest-capture control must show a seam (a step of at least
WALK_SEAM_MINIMUM), so the gate is shown able to fail.

`relight-cost` measures the relight's GPU cost at 1920 x 1080 from the east
camera (-vkframestats timestamps), with the SDF producer settled: relit, as
baked, relit again, RELIGHT_COST_FRAMES each; the cost is the median of the
relit phases less the baked one, against RELIGHT_BUDGET_MS.

`relight` is the relight gate (R50-RELIGHT). The mirror-lamp map's bulb is
an unbaked light_dynamic, so its probes were baked without it; its Cycles
references are rendered with the bulb lit. Two boots screenshot every camera
after the SDF producer (r_indirect_producer sdf) has warmed up and converged
on the bulb: relit (`mat_reflection_relight 1`: each probe's relight bands
gain albedo times the bulb's shadowed direct light and the producer's change)
and as baked (`0`, the control). Each view is scored as `mirror` does, with
the floor pixels that mirror the bulb itself left out (the bulb is not in the
probes: they are the reference's pixels above BULB_MASK times its floor
median, grown by BULB_GROW pixels). The gate passes when every relit view's
floor/wall ratio is at most MIRROR_RATIO_LIMIT and the as-baked control's
mean ratio is at least CONTROL_MARGIN times the relit mean and above the
limit.

Why (RFC/0007-progress.md, R50-PARALLAX): a direction-only probe is right only
at its capture point, and switching probes shows a seam where the reflection
jumps (3kliksphilip, "Advanced Reflections in CS:GO... and for Source 2?",
2019; Lagarde and Zanuttini, SIGGRAPH 2012).
"""

import argparse
import hashlib
import json
import math
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import bsp2_reader  # noqa: E402
import gi_reference  # noqa: E402
import gi_runtime  # noqa: E402
import pbrt_map_toolchain  # noqa: E402
import reference_compare  # noqa: E402
import reflection_probe_set as rps  # noqa: E402

SCHEMA = "reflection-runtime/v1"
FIXTURES = ROOT / "quality/fixtures/reflection"
MAPS = ROOT / "quality-results/reflection-maps"
CAPTURE_WIDTH, CAPTURE_HEIGHT = 1024, 768
# Frames a station takes: settle after the move, then the screenshot's own
# stall (gi_temporal.SHOT_FRAMES) and a gap (the engine drops a screenshot
# requested while it still writes the last).
SETTLE_FRAMES, GAP_FRAMES, SHOT_FRAMES = 90, 45, 60
# Mirror gate: floor error over wall error, per view. Measured 2026-09-25 on
# RADV Strix Halo: blended 1.4-1.8, direction-only 7.6-8.2, wrong box 4.4-6.0.
MIRROR_RATIO_LIMIT = 2.5
CONTROL_MARGIN = 1.5
# Pixels at or above this 8-bit level are clipped and left out of the fits.
CLIPPED = 250
WRONG_BOX_SHIFT_M = (1.0, 0.8, 0.0)
# Relight gate: the bulb's mirror image is left out of the floor's pixels; the
# producer warms up and converges on the bulb before the first screenshot
# (gi_portal.py's WARM_FRAMES + CHANGE_FRAMES).
BULB_MASK = 8.0
BULB_GROW = 2
RELIGHT_WARM_FRAMES = 150 + 64
RELIGHT_COST_FRAMES = 300
# Declared desktop budget for the relight at 1080p (world PBR only).
RELIGHT_BUDGET_MS = 0.5
# Walk gate: the largest red-share step between neighbouring floor pixels.
WALK_STEP_LIMIT = 0.25
WALK_SEAM_MINIMUM = 0.6
# A floor pixel needs this much weight-view signal (linear) to read a share.
WALK_SIGNAL = 0.02


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load_fixture(name):
    path = FIXTURES / name / "fixture.json"
    fixture = json.loads(path.read_text())
    if fixture.get("schema") != "gi-fixture/v1" or fixture.get("name") != name:
        raise ValueError("invalid reflection fixture " + str(path))
    fixture["directory"] = path.parent
    return fixture


def toolchain(args):
    profile, _ = pbrt_map_toolchain.load_profiles()
    values = pbrt_map_toolchain.load(getattr(args, "toolchain", None) or
                                     ROOT / profile["layout"]["toolchain_file"])
    return (Path(getattr(args, "build", None) or values["client_build"]),
            Path(getattr(args, "runtime", None) or values["runtime"]))


# ------------------------------------------------------------------ references

def cmd_references(args):
    import imageio.v3 as iio
    fixture = load_fixture(args.fixture)
    tools = gi_reference.Tools(args.toolchain)
    receipt, _, render_dir = gi_reference.render_state(
        tools, fixture, "default", args.work, args.samples, args.seed, args.device,
        "gi-reference")
    references = fixture["directory"] / "references"
    if references.exists():
        shutil.rmtree(references)
    references.mkdir()
    views = {}
    for camera, output in sorted(receipt["renders"].items()):
        passes = gi_reference.render_passes(render_dir / output["exr"])
        combined = references / (camera + ".exr")
        iio.imwrite(combined, passes["Combined"][..., :3].astype(np.float32))
        index = np.rint(passes["IndexOB"][..., 0]).astype(np.int32)
        if index.max() > 255:
            raise ValueError("more than 255 objects in a region index")
        index_path = references / (camera + ".index.png")
        Image.fromarray(index.astype(np.uint8)).save(index_path, optimize=False)
        views[camera] = {"combined": {"file": combined.name, "sha256": sha256(combined)},
                         "index": {"file": index_path.name, "sha256": sha256(index_path)}}
    record = {"schema": "reflection-references/v1", "fixture": fixture["name"],
              "fixture_reference_digest": gi_reference.reference_digest(fixture),
              "object_index": receipt["object_index"],
              "render": {key: receipt[key] for key in ("blender", "cycles_device", "samples",
                                                       "seed", "light_paths")},
              "views": views}
    (references / "references.json").write_text(json.dumps(record, indent=2, sort_keys=True) +
                                                "\n")
    print("[%s] %d reference views" % (fixture["name"], len(views)))
    return 0


# ------------------------------------------------------------------ capture

def station_commands(fixture, camera):
    commands, _ = reference_compare.camera_commands(gi_runtime.camera_scene(fixture, camera))
    return commands


def capture(fixture, content, cameras, mode, out, build, runtime, setup=(), warm=0):
    """Screenshots of `cameras` in one boot at `mat_reflection_probes mode`,
    after the `setup` commands and `warm` frames: {camera: path} (missing
    cameras are absent)."""
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    first = station_commands(fixture, cameras[0])
    aliases = []
    for k, camera in enumerate(cameras):
        place = [c for c in station_commands(fixture, camera)
                 if c.startswith(("cmd setpos", "cmd setang"))][:2]
        aliases.append('alias rr_shot%d "%s; wait %d; screenshot; wait %d; rr_shot%d"' % (
            k, "; ".join(place), SETTLE_FRAMES, GAP_FRAMES, k + 1))
    aliases.append('alias rr_shot%d "echo reflection_runtime done"' % len(cameras))
    line = "; ".join(["r_drawvgui 0", "mat_reflection_probes %d" % mode] + list(setup) + first +
                     (["wait %d" % warm] if warm else []) + ["rr_shot0"])
    frames = gi_runtime.PLACEMENT_FRAMES + warm + len(cameras) * (
        SETTLE_FRAMES + GAP_FRAMES + SHOT_FRAMES) + 60
    out.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        [sys.executable, HERE / "portal_boot.py", "--runtime", runtime, "--build", build,
         "--content-root", content, "--renderer", "native-vulkan", "--headless",
         "--map", manifest["map"], "--width", str(CAPTURE_WIDTH),
         "--height", str(CAPTURE_HEIGHT), "--capture-wait", str(frames)] +
        [item for command in aliases + [line] for item in ("--console-command", command)] +
        ["--out", out], cwd=ROOT, capture_output=True, text=True)
    evidence = json.loads((out / "evidence.json").read_text()) \
        if (out / "evidence.json").is_file() else {}
    shots = [shot["path"] for shot in evidence.get("screenshots", [])]
    # portal_boot's own last screenshot follows the chain.
    captured = dict(zip(cameras, shots[:len(cameras)])) if len(shots) > len(cameras) else {}
    return captured, {"boot": evidence.get("status"), "returncode": result.returncode,
                      "screenshots": len(shots), "mode": mode, "out": str(out)}


def wrong_box_content(content, map_name, out):
    """A copy of `content` whose map's RPRB boxes and influences are shifted
    by WRONG_BOX_SHIFT_M: the parallax gate's negative control."""
    target = out / "content"
    if target.exists():
        shutil.rmtree(target)
    shutil.copytree(content, target, symlinks=True)
    bsp = target / "maps" / (map_name + ".bsp")
    data = bsp.read_bytes()
    container = bsp2_reader.Bsp2File(data)
    lumps = []
    shifted = False
    for entry in container.entries:
        payload = container.lump(entry)
        if entry["fourcc"] == rps.MAGIC:
            layout = rps.layout_meters(rps.read(payload))
            shift = np.asarray(WRONG_BOX_SHIFT_M)
            probes = [dict(probe, **{key: probe[key] + shift for key in (
                "box_min", "box_max", "influence_min", "influence_max")}) for probe in
                layout["probes"]]
            # The shifted box must still hold its capture: a box that moved off
            # the room, not an invalid payload.
            for probe in probes:
                probe["box_min"] = np.minimum(probe["box_min"], probe["capture"])
            payload = rps.build(probes, layout["chains"], relight=layout["relight"])
            shifted = True
        lumps.append((entry["fourcc"], entry["version"], entry["flags"], entry["alignment"],
                      payload))
    if not shifted:
        raise ValueError("%s carries no RPRB lump" % bsp)
    bsp.unlink()
    bsp.write_bytes(bsp2_reader.write_bsp2(container.revision, lumps))
    return target


# ------------------------------------------------------------------ mirror

def reference_view(fixture, camera):
    references = fixture["directory"] / "references"
    record = json.loads((references / "references.json").read_text())
    if record["fixture_reference_digest"] != gi_reference.reference_digest(fixture):
        raise ValueError("%s references are stale: rerun `references`" % fixture["name"])
    import imageio.v3 as iio
    view = record["views"][camera]
    combined = np.asarray(iio.imread(references / view["combined"]["file"]),
                          dtype=np.float64)[..., :3]
    index = np.asarray(Image.open(references / view["index"]["file"]))
    masks = gi_reference.region_masks(index, record["object_index"], fixture["regions"][camera])
    return combined, masks


def bulb_mask(reference, floor):
    """Floor pixels that mirror an emitter the probes lack: the reference's
    pixels above BULB_MASK times its floor median luminance, grown."""
    luminance = reference @ np.array((0.2126, 0.7152, 0.0722))
    bright = floor & (luminance > BULB_MASK * np.median(luminance[floor]))
    grown = bright.copy()
    for _ in range(BULB_GROW):
        grown[1:] |= grown[:-1].copy()
        grown[:-1] |= grown[1:].copy()
        grown[:, 1:] |= grown[:, :-1].copy()
        grown[:, :-1] |= grown[:, 1:].copy()
    return grown


def score_mirror(fixture, camera, shot, mask_bulb=False):
    """Relative floor error after fitting the walls' gain."""
    reference, masks = reference_view(fixture, camera)
    film = fixture["film"]
    measured = gi_runtime.linear_capture(shot, film, 1.0)
    raw = np.asarray(Image.open(shot).convert("RGB").resize(
        (film["width"], film["height"]), Image.BOX))
    unclipped = raw.max(axis=2) < CLIPPED
    walls = masks["walls"] & unclipped
    floor = masks["floor"] & unclipped
    masked = 0
    if mask_bulb:
        bulb = bulb_mask(reference, masks["floor"])
        masked = int((floor & bulb).sum())
        floor &= ~bulb
    if walls.sum() < 50 or floor.sum() < 50:
        return {"status": "fail", "failure": "too few wall or floor pixels",
                "wall_pixels": int(walls.sum()), "floor_pixels": int(floor.sum())}
    m, r = measured[walls], reference[walls]
    gain = float((m * r).sum() / max((m * m).sum(), 1e-12))
    wall_error = float(np.abs(gain * m - r).mean() / max(r.mean(), 1e-12))
    m, r = measured[floor], reference[floor]
    floor_error = float(np.abs(gain * m - r).mean() / max(r.mean(), 1e-12))
    return {"gain": gain, "wall_relative_error": wall_error,
            "floor_relative_error": floor_error,
            "floor_over_wall": floor_error / max(wall_error, 1e-9),
            "wall_pixels": int(walls.sum()), "floor_pixels": int(floor.sum()),
            "bulb_masked_pixels": masked}


def cmd_mirror(args):
    fixture = load_fixture("mirror-room")
    build, runtime = toolchain(args)
    map_build = Path(args.map_build or MAPS / "mirror-room")
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    out = Path(args.out)
    cameras = sorted(fixture["cameras"])
    content = map_build / "content"
    runs = {"blended": (content, rps.MODE_BLEND),
            "direction-only": (content, rps.MODE_DIRECTION),
            "wrong-box": (wrong_box_content(content, manifest["map"], out / "wrong-box"),
                          rps.MODE_BLEND)}
    results = {}
    for name, (root, mode) in runs.items():
        if args.reuse and (out / name / "evidence.json").is_file():
            evidence = json.loads((out / name / "evidence.json").read_text())
            paths = [shot["path"] for shot in evidence.get("screenshots", [])]
            shots = dict(zip(cameras, paths[:len(cameras)])) if len(paths) > len(cameras) else {}
            boot = {"boot": evidence.get("status"), "reused": True, "mode": mode,
                    "screenshots": len(paths), "out": str(out / name)}
        else:
            shots, boot = capture(fixture, root, cameras, mode, out / name, build, runtime)
        views = {camera: score_mirror(fixture, camera, shots[camera]) if camera in shots else
                 {"status": "fail", "failure": "no screenshot"} for camera in cameras}
        ratios = [view["floor_over_wall"] for view in views.values() if "floor_over_wall" in view]
        errors = [view["floor_relative_error"] for view in views.values()
                  if "floor_relative_error" in view]
        complete = len(ratios) == len(cameras)
        results[name] = {"boot": boot, "views": views,
                         "mean_floor_relative_error": float(np.mean(errors)) if complete else None,
                         "mean_floor_over_wall": float(np.mean(ratios)) if complete else None,
                         "max_floor_over_wall": float(np.max(ratios)) if complete else None}
        print("%-15s floor error %s, floor/wall mean %s max %s  (boot %s, %d shots)" % (
            name, *("%.4f" % results[name][key] if results[name][key] is not None else "-"
                    for key in ("mean_floor_relative_error", "mean_floor_over_wall",
                                "max_floor_over_wall")),
            boot["boot"], boot["screenshots"]), flush=True)
    blended = results["blended"]
    failures = []
    if blended["max_floor_over_wall"] is None or \
            blended["max_floor_over_wall"] > MIRROR_RATIO_LIMIT:
        failures.append("blended floor/wall %s above %.2f" % (blended["max_floor_over_wall"],
                                                              MIRROR_RATIO_LIMIT))
    for control in ("direction-only", "wrong-box"):
        ratio = results[control]["mean_floor_over_wall"]
        if blended["mean_floor_over_wall"] is not None and (
                ratio is None or ratio < CONTROL_MARGIN * blended["mean_floor_over_wall"] or
                ratio <= MIRROR_RATIO_LIMIT):
            failures.append("%s control floor/wall %s is not above %.1f x the blend and the "
                            "limit" % (control, ratio, CONTROL_MARGIN))
    record = {"schema": SCHEMA, "gate": "mirror", "fixture": fixture["name"],
              "map_build": str(map_build.resolve()), "build": str(build),
              "map_bsp_sha256": sha256(content / "maps" / (manifest["map"] + ".bsp")),
              "limits": {"mirror_ratio_limit": MIRROR_RATIO_LIMIT,
                         "control_margin": CONTROL_MARGIN},
              "runs": results, "failures": failures,
              "status": "pass" if not failures else "fail"}
    (out / "mirror.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("mirror gate %s%s" % (record["status"], "".join("\n  " + f for f in failures)))
    return 0 if record["status"] == "pass" else 1


# ------------------------------------------------------------------ relight

def cmd_relight(args):
    fixture = load_fixture("mirror-lamp")
    build, runtime = toolchain(args)
    map_build = Path(args.map_build or MAPS / "mirror-lamp")
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    out = Path(args.out)
    cameras = sorted(fixture["cameras"])
    content = map_build / "content"
    producer = "r_indirect_producer %s" % args.producer
    runs = {"relit": ("mat_reflection_relight 1", producer),
            "as-baked": ("mat_reflection_relight 0", producer)}
    results = {}
    for name, setup in runs.items():
        if args.reuse and (out / name / "evidence.json").is_file():
            evidence = json.loads((out / name / "evidence.json").read_text())
            paths = [shot["path"] for shot in evidence.get("screenshots", [])]
            shots = dict(zip(cameras, paths[:len(cameras)])) if len(paths) > len(cameras) else {}
            boot = {"boot": evidence.get("status"), "reused": True, "setup": list(setup),
                    "screenshots": len(paths), "out": str(out / name)}
        else:
            shots, boot = capture(fixture, content, cameras, rps.MODE_BLEND, out / name, build,
                                  runtime, setup=setup, warm=RELIGHT_WARM_FRAMES)
            boot["setup"] = list(setup)
        logs = sorted((out / name).glob("**/engine.log"))
        lines = logs[0].read_text(errors="replace").splitlines() if logs else []
        boot["rprb_log"] = [line.strip() for line in lines if "RPRB" in line][-2:]
        views = {camera: score_mirror(fixture, camera, shots[camera], mask_bulb=True)
                 if camera in shots else {"status": "fail", "failure": "no screenshot"}
                 for camera in cameras}
        ratios = [view["floor_over_wall"] for view in views.values() if "floor_over_wall" in view]
        errors = [view["floor_relative_error"] for view in views.values()
                  if "floor_relative_error" in view]
        complete = len(ratios) == len(cameras)
        results[name] = {"boot": boot, "views": views,
                         "mean_floor_relative_error": float(np.mean(errors)) if complete else None,
                         "mean_floor_over_wall": float(np.mean(ratios)) if complete else None,
                         "max_floor_over_wall": float(np.max(ratios)) if complete else None}
        print("%-9s floor error %s, floor/wall mean %s max %s  (boot %s, %d shots)" % (
            name, *("%.4f" % results[name][key] if results[name][key] is not None else "-"
                    for key in ("mean_floor_relative_error", "mean_floor_over_wall",
                                "max_floor_over_wall")),
            boot["boot"], boot["screenshots"]), flush=True)
    relit, control = results["relit"], results["as-baked"]
    failures = []
    if not any("relightable" in line for line in relit["boot"]["rprb_log"]):
        failures.append("the map's RPRB lump is not relightable: %s" % relit["boot"]["rprb_log"])
    if relit["max_floor_over_wall"] is None or relit["max_floor_over_wall"] > MIRROR_RATIO_LIMIT:
        failures.append("relit floor/wall %s above %.2f" % (relit["max_floor_over_wall"],
                                                            MIRROR_RATIO_LIMIT))
    ratio = control["mean_floor_over_wall"]
    if relit["mean_floor_over_wall"] is not None and (
            ratio is None or ratio < CONTROL_MARGIN * relit["mean_floor_over_wall"] or
            ratio <= MIRROR_RATIO_LIMIT):
        failures.append("as-baked control floor/wall %s is not above %.1f x the relit and the "
                        "limit" % (ratio, CONTROL_MARGIN))
    record = {"schema": SCHEMA, "gate": "relight", "fixture": fixture["name"],
              "producer": args.producer,
              "map_build": str(map_build.resolve()), "build": str(build),
              "map_bsp_sha256": sha256(content / "maps" / (manifest["map"] + ".bsp")),
              "limits": {"mirror_ratio_limit": MIRROR_RATIO_LIMIT,
                         "control_margin": CONTROL_MARGIN, "bulb_mask": BULB_MASK,
                         "bulb_grow": BULB_GROW, "warm_frames": RELIGHT_WARM_FRAMES},
              "runs": results, "failures": failures,
              "status": "pass" if not failures else "fail"}
    (out / "relight.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("relight gate %s%s" % (record["status"], "".join("\n  " + f for f in failures)))
    return 0 if record["status"] == "pass" else 1


def cmd_relight_cost(args):
    import frame_pacing
    fixture = load_fixture("mirror-lamp")
    build, runtime = toolchain(args)
    map_build = Path(args.map_build or MAPS / "mirror-lamp")
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    # A short relative name: the engine's command line holds 512 characters.
    stats_name = "relight-frame-stats.jsonl"
    place = [c for c in station_commands(fixture, "east")
             if c.startswith(("cmd setpos", "cmd setang"))][:2]
    marks = (("relit", "mat_reflection_relight 1"), ("baked", "mat_reflection_relight 0"),
             ("relit_again", "mat_reflection_relight 1"))
    # One alias per cfg line: `wait` holds only within its line.
    aliases = []
    for index, (mark, action) in enumerate(marks):
        after = "; rrc%d" % (index + 1) if index + 1 < len(marks) else ""
        aliases.append('alias rrc%d "%s; wait 30; vk_frame_mark %s; wait %d%s"'
                       % (index, action, mark, RELIGHT_COST_FRAMES, after))
    line = "; ".join(["r_drawvgui 0", "mat_reflection_probes 1",
                      "r_indirect_producer %s" % args.producer] + list(args.setup or []) +
                     station_commands(fixture, "east") + place +
                     ["wait %d" % RELIGHT_WARM_FRAMES, "rrc0"])
    frames = RELIGHT_WARM_FRAMES + len(marks) * (RELIGHT_COST_FRAMES + 30) + 60
    run = [sys.executable, HERE / "portal_boot.py", "--runtime", runtime, "--build", build,
           "--content-root", map_build / "content", "--renderer", "native-vulkan", "--headless",
           "--map", manifest["map"], "--width", "1920", "--height", "1080",
           "--capture-wait", str(gi_runtime.PLACEMENT_FRAMES + frames),
           "--engine-arg=-vkframestats", "--engine-arg=%s" % stats_name, "--out", out / "boot"]
    for command in aliases + [line]:
        run += ["--console-command", command]
    boot = subprocess.run([str(part) for part in run], cwd=ROOT, capture_output=True,
                          text=True, timeout=1800)
    found = sorted((out / "boot").rglob(stats_name))
    record = {"schema": SCHEMA, "gate": "relight-cost", "producer": args.producer,
              "setup": list(args.setup or []),
              "build": str(build), "resolution": [1920, 1080],
              "budget_ms": RELIGHT_BUDGET_MS, "booted": boot.returncode == 0}
    if not found:
        record.update(status="fail", error="no frame stats")
    else:
        _, records, _ = frame_pacing.read_stats(found[0])
        gpu = {frame["gpu"][0]: frame["gpu"][1] / 1000.0 for frame in records if "gpu" in frame}
        phase, samples = None, {}
        for frame in records:
            for mark in filter(None, frame.get("mark", "").split(",")):
                phase = mark
            if phase and frame["f"] in gpu:
                samples.setdefault(phase, []).append(gpu[frame["f"]])
        medians = {name: float(np.median(values[30:])) for name, values in samples.items()
                   if len(values) > 60}
        cost = None
        if {"relit", "baked", "relit_again"} <= medians.keys():
            cost = (medians["relit"] + medians["relit_again"]) / 2.0 - medians["baked"]
        record.update(gpu_frame_ms_median=medians, relight_ms=cost,
                      samples={name: len(values) for name, values in samples.items()},
                      status="pass" if cost is not None and cost <= RELIGHT_BUDGET_MS and
                      record["booted"] else "fail")
        print("GPU frame ms: %s; relight %s ms (budget %.2f)" % (
            ", ".join("%s %.3f" % item for item in sorted(medians.items())),
            "%.3f" % cost if cost is not None else "-", RELIGHT_BUDGET_MS))
    (out / "relight-cost.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("relight cost %s" % record["status"])
    return 0 if record["status"] == "pass" else 1


# ------------------------------------------------------------------ walk

def two_rooms_scene(fixture):
    """The fixture's geometry as analytic boxes (walls, the doorway)."""
    a, b = (fixture["rooms_m"][key] for key in ("A", "B"))
    wall0, wall1 = a[1][0], b[0][0]
    door = ((wall0, 1.4, 0.0), (wall1, 2.6, 2.2))
    opening = ((wall0 - 0.1, 1.4, -1.0), (wall1 + 0.1, 2.6, 2.2))
    return rps.BoxScene(rooms=[a, b, door], openings=[opening])


def floor_pixels(scene, pose, width, height, horizontal_fov):
    """(mask of pixels whose first hit is the floor, their hit points)."""
    forward = np.asarray(pose["forward"], dtype=np.float64)
    right = np.cross(forward, (0.0, 0.0, 1.0))
    right /= np.linalg.norm(right)
    up = np.cross(right, forward)
    tan_x = math.tan(math.radians(horizontal_fov) / 2)
    tan_y = tan_x * height / width
    xs = (2 * (np.arange(width) + 0.5) / width - 1) * tan_x
    ys = (1 - 2 * (np.arange(height) + 0.5) / height) * tan_y
    x, y = np.meshgrid(xs, ys)
    directions = forward + x[..., None] * right + y[..., None] * up
    directions = (directions / np.linalg.norm(directions, axis=2, keepdims=True)).reshape(-1, 3)
    eye = np.asarray(pose["eye"], dtype=np.float64)
    distance, normal = scene(np.tile(eye, (len(directions), 1)), directions, 1.0e4)
    floor = np.isfinite(distance) & (normal[:, 2] > 0.5)
    return floor.reshape(height, width)


def score_walk_frame(shot, mask):
    from scipy.ndimage import binary_erosion
    image = gi_runtime.srgb_decode(np.asarray(Image.open(shot).convert("RGB")))
    if image.shape[:2] != mask.shape:
        raise ValueError("capture %s is not %dx%d" % (shot, mask.shape[1], mask.shape[0]))
    red, blue = image[..., 0], image[..., 2]
    signal = red + blue
    valid = binary_erosion(mask, iterations=3) & (signal > WALK_SIGNAL)
    share = np.where(valid, red / np.maximum(signal, 1e-9), np.nan)
    steps = []
    for axis in (0, 1):
        difference = np.abs(np.diff(share, axis=axis))
        steps.append(difference[np.isfinite(difference)])
    steps = np.concatenate(steps)
    shares = share[np.isfinite(share)]
    return {"floor_pixels": int(valid.sum()),
            "max_step": float(steps.max()) if len(steps) else None,
            "p999_step": float(np.percentile(steps, 99.9)) if len(steps) else None,
            "red_share_min": float(shares.min()) if len(shares) else None,
            "red_share_max": float(shares.max()) if len(shares) else None}


def cmd_walk(args):
    fixture = load_fixture("two-rooms")
    build, runtime = toolchain(args)
    map_build = Path(args.map_build or MAPS / "two-rooms")
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    out = Path(args.out)
    scene = two_rooms_scene(fixture)
    stations = fixture["walk"]
    masks = {camera: floor_pixels(scene, fixture["cameras"][camera], CAPTURE_WIDTH,
                                  CAPTURE_HEIGHT, fixture["horizontal_fov_degrees"])
             for camera in stations}
    results = {}
    for name, mode in (("blended", rps.MODE_BLEND | rps.MODE_WEIGHTS),
                       ("nearest", rps.MODE_NEAREST | rps.MODE_WEIGHTS)):
        shots, boot = capture(fixture, map_build / "content", stations, mode, out / name, build,
                              runtime)
        frames = {camera: score_walk_frame(shots[camera], masks[camera]) if camera in shots
                  else {"failure": "no screenshot"} for camera in stations}
        steps = [frame["max_step"] for frame in frames.values() if frame.get("max_step")
                 is not None]
        results[name] = {"boot": boot, "frames": frames,
                         "max_step": max(steps) if len(steps) == len(stations) else None,
                         "red_share_min": min((f["red_share_min"] for f in frames.values()
                                               if f.get("red_share_min") is not None),
                                              default=None),
                         "red_share_max": max((f["red_share_max"] for f in frames.values()
                                               if f.get("red_share_max") is not None),
                                              default=None)}
        print("%-8s max step %s  red share %s..%s  (boot %s, %d shots)" % (
            name, "%.3f" % results[name]["max_step"] if results[name]["max_step"] is not None
            else "-", results[name]["red_share_min"], results[name]["red_share_max"],
            boot["boot"], boot["screenshots"]), flush=True)
    blended, nearest = results["blended"], results["nearest"]
    failures = []
    if blended["max_step"] is None or blended["max_step"] > WALK_STEP_LIMIT:
        failures.append("blended weights step %s above %.2f" % (blended["max_step"],
                                                                 WALK_STEP_LIMIT))
    if blended["red_share_min"] is None or blended["red_share_min"] > 0.1 or \
            blended["red_share_max"] < 0.9:
        failures.append("the walk did not see both probes take over")
    if nearest["max_step"] is None or nearest["max_step"] < WALK_SEAM_MINIMUM:
        failures.append("nearest-capture control shows no seam (max step %s)" %
                        nearest["max_step"])
    record = {"schema": SCHEMA, "gate": "walk", "fixture": fixture["name"],
              "map_build": str(map_build.resolve()), "build": str(build),
              "map_bsp_sha256": sha256(map_build / "content" / "maps" /
                                       (manifest["map"] + ".bsp")),
              "limits": {"walk_step_limit": WALK_STEP_LIMIT,
                         "walk_seam_minimum": WALK_SEAM_MINIMUM},
              "runs": results, "failures": failures,
              "status": "pass" if not failures else "fail"}
    (out / "walk.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("walk gate %s%s" % (record["status"], "".join("\n  " + f for f in failures)))
    return 0 if record["status"] == "pass" else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    c = commands.add_parser("references", help="render a fixture's Cycles references")
    c.add_argument("--fixture", required=True, choices=("mirror-room", "mirror-lamp"))
    c.add_argument("--samples", type=int, default=1024)
    c.add_argument("--seed", type=int, default=0)
    c.add_argument("--device", default="gpu")
    c.add_argument("--work", type=Path, default=ROOT / "quality-results/reflection-references")
    c.add_argument("--toolchain", type=Path)
    for name, text in (("mirror", "the parallax gate on mirror-room"),
                       ("walk", "the blend gate on two-rooms"),
                       ("relight", "the relight gate on mirror-lamp"),
                       ("relight-cost", "the relight's GPU cost on mirror-lamp")):
        c = commands.add_parser(name, help=text)
        c.add_argument("--map-build", type=Path)
        c.add_argument("--build", type=Path, help="client build (default: the toolchain's)")
        c.add_argument("--runtime", type=Path)
        c.add_argument("--toolchain", type=Path)
        c.add_argument("--out", type=Path, required=True)
        c.add_argument("--reuse", action="store_true",
                       help="rescore the boots already in --out instead of booting")
        if name in ("relight", "relight-cost"):
            c.add_argument("--producer", default="sdf", choices=("sdf", "rayquery", "baked"),
                           help="the indirect producer whose change relights the probes")
        if name == "relight-cost":
            c.add_argument("--setup", action="append",
                           help="a console command before the phases (an attribution run)")
    args = parser.parse_args()
    return {"references": cmd_references, "mirror": cmd_mirror, "walk": cmd_walk,
            "relight": cmd_relight, "relight-cost": cmd_relight_cost}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
