#!/usr/bin/env python3
"""In-game GI captures of the RFC 0011 fixtures, judged against Cycles.

    python3 tools/quality/gi_runtime.py capture --fixture furnace --camera inside \\
        --map-build quality-results/rfc0011-maps/furnace --out quality-results/x
    python3 tools/quality/gi_runtime.py compare --fixture furnace --state default \\
        --capture quality-results/x --out quality-results/x/gate.json
    python3 tools/quality/gi_runtime.py indirect-view --fixture furnace \\
        --map-build quality-results/rfc0011-maps/furnace --out quality-results/rfc0011-g0-view
    python3 tools/quality/gi_runtime.py oracles --fixture leak-rgb-rooms \\
        --capture quality-results/x --out quality-results/x/oracles.json

`capture` boots the fixture's map headless on native Vulkan (`portal_boot.py`),
places the camera at a fixture camera with the pipeline's own camera commands
(`reference_compare.camera_commands`; the fixtures' 4:3 90-degree film is
exactly Source's `fov 90`, so no crop is needed), turns on the indirect-light
debug view (`mat_indirect_view`, `mat_indirect_view_scale`) and screenshots.

`compare` decodes the sRGB capture to linear light, divides out the view's
exposure scale, averages it to the reference film (256 x 192) and compares
each fixture region (object-index masks from the reference, eroded) with the
Cycles indirect-only reference (DiffInd, the same E / pi unit as the LMAP
indirect layer and the view). World regions are gated by a relative
luminance tolerance; dynamic-model regions are measured and reported, and
gated only with --gate-models (RFC 0011 G1 makes them pass).

`oracles` judges a capture of the fixture's baked state by the fixture's own
relational oracles (`gi_oracles.py`): each region's mean indirect light from
the capture, and every declared oracle that reads only that state's indirect
light (leak zeros, mirror symmetry, chromaticity, dominance, uniformity),
with tolerances for an 8-bit capture. No Cycles value enters these checks,
only the capture's own regions (the masks come from the reference).

`indirect-view` is the G0 oracle run: a capture at scale 1 must pass the
world gate, and a seeded capture whose indirect light is doubled (scale 2,
declared as 1) must fail it, so the comparison is shown able to fail.
"""

import argparse
import hashlib
import json
import math
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import gi_oracles  # noqa: E402
import gi_reference  # noqa: E402
import pbrt_map_toolchain  # noqa: E402
import reference_compare  # noqa: E402

SCHEMA = "gi-runtime-compare/v1"
CAPTURE_SCHEMA = "gi-runtime-capture/v1"
# World indirect light (LMAP indirect layer) against Cycles DiffInd, per
# region: |measured - reference| <= WORLD_TOLERANCE * reference +
# ABSOLUTE_FRACTION * (the brightest region's reference in that view), so a
# region that should be dark is judged against the scene's light level rather
# than by a relative error near zero.
WORLD_TOLERANCE = 0.10
ABSOLUTE_FRACTION = 0.01
CAPTURE_WIDTH, CAPTURE_HEIGHT = 1024, 768
# Frames from the console commands to the screenshot: the camera commands
# settle in about 240, and the view must be on for several frames after.
CAPTURE_WAIT = 420
# The frames the camera placement and proof screenshot take from the start of
# the line (reference_compare.camera_commands' waits plus `wait 5`) before
# --console-command runs.
PLACEMENT_FRAMES = 245


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def srgb_decode(encoded):
    encoded = np.asarray(encoded, dtype=np.float64) / 255.0
    return np.where(encoded <= 0.04045, encoded / 12.92, ((encoded + 0.055) / 1.055) ** 2.4)


def camera_scene(fixture, camera):
    """The scene-model subset `reference_compare.camera_commands` reads."""
    film = fixture["film"]
    horizontal = math.radians(fixture["horizontal_fov_degrees"])
    vertical = 2 * math.atan(math.tan(horizontal / 2) * film["height"] / film["width"])
    return {"format": "usd", "film": dict(film),
            "camera": {"fov_degrees": math.degrees(vertical),
                       "pose": fixture["cameras"][camera]}}


# Cycles total luminance below which no light reaches a view at all.
UNLIT_LUMINANCE = 1e-4


def dark_view(fixture, state, camera):
    """True when no light reaches `camera` in `state` (every region's Cycles
    total is below UNLIT_LUMINANCE), so no capture of it can show scene
    detail: the unlit room of thin-wall."""
    path = fixture["directory"] / "references" / "references.json"
    if not path.is_file():
        return False
    record = json.loads(path.read_text())
    means = [gi_reference.luminance(region["total"]) for view in record["views"].values()
             if view["camera"] == camera and view["state"] == state
             for region in view["regions"].values() if region.get("total")]
    return bool(means) and max(means) < UNLIT_LUMINANCE


def capture(args):
    fixture = gi_reference.load_fixture(args.fixture)
    profile, _ = pbrt_map_toolchain.load_profiles()
    toolchain = pbrt_map_toolchain.load(args.toolchain or
                                        ROOT / profile["layout"]["toolchain_file"])
    build = Path(args.build or toolchain["client_build"])
    runtime = Path(args.runtime or toolchain["runtime"])
    content = Path(args.map_build) / "content"
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    cameras = [args.camera] if args.camera else sorted(fixture["cameras"])
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    records = {}
    for camera in cameras:
        commands, fovs = reference_compare.camera_commands(camera_scene(fixture, camera))
        # r_drawvgui 0 hides every VGUI panel (a player's archived net_graph
        # included) without changing archived settings. A shaded frame is
        # screenshot first, so the boot proves a rendered scene even when the
        # scored indirect view is legitimately dark; portal_boot's final
        # screenshot is the scored one. A camera whose Cycles total light is
        # dark too (the unlit side of thin-wall) takes that proof frame from
        # a lit camera of the fixture, then moves (noclip stays on) to its own.
        dark = dark_view(fixture, fixture["baked_state"], camera)
        proof = next((other for other in sorted(fixture["cameras"])
                      if not dark_view(fixture, fixture["baked_state"], other)),
                     None) if dark else camera
        if proof is None:
            raise ValueError("%s: every camera is dark; nothing can prove a rendered frame"
                             % args.fixture)
        if proof == camera:
            prelude = commands
        else:
            prelude, _ = reference_compare.camera_commands(camera_scene(fixture, proof))
            commands = [c for c in commands if c != "cmd noclip"]
        view = ["mat_indirect_view %d" % args.view,
                "mat_indirect_view_scale %g" % args.scale] + list(args.console_command)
        steps = prelude + ["screenshot", "wait 5"] + (commands if proof != camera else []) + view
        # One line: `wait` applies only within a single script line.
        line = "; ".join(["r_drawvgui 0"] + steps)
        boot = out / camera
        result = subprocess.run(
            [sys.executable, HERE / "portal_boot.py", "--runtime", runtime, "--build", build,
             "--content-root", content, "--renderer", "native-vulkan", "--headless",
             "--map", manifest["map"], "--width", str(CAPTURE_WIDTH),
             "--height", str(CAPTURE_HEIGHT),
             "--capture-wait", str(getattr(args, "capture_wait", None) or CAPTURE_WAIT),
             "--console-command", line, "--out", boot],
            cwd=ROOT, capture_output=True, text=True)
        evidence = json.loads((boot / "evidence.json").read_text()) \
            if (boot / "evidence.json").is_file() else {}
        shots = evidence.get("screenshots", [])
        failures = list(evidence.get("failures") or [])
        if len(shots) != 2:
            failures.append("expected the shaded proof frame and the scored frame, got %d "
                            "screenshot(s)" % len(shots))
        records[camera] = {"boot": str(boot),
                           "status": "pass" if evidence.get("status") == "pass" and not failures
                           else "fail",
                           "failures": failures,
                           "proof_screenshot": shots[0]["path"] if shots else None,
                           "screenshot": shots[-1]["path"] if shots else None,
                           "screenshot_sha256": shots[-1]["sha256"] if shots else None,
                           "source_fov": fovs["source_fov"], "commands": line,
                           "expected_dark": dark, "proof_camera": proof,
                           "returncode": result.returncode}
        print("[%s/%s] boot %s" % (args.fixture, camera, evidence.get("status")), flush=True)
    record = {"schema": CAPTURE_SCHEMA, "fixture": args.fixture, "map": manifest["map"],
              "map_build": str(Path(args.map_build).resolve()), "build": str(build),
              "view": args.view, "scale": args.scale, "cameras": records,
              "map_bsp_sha256": sha256(content / "maps" / (manifest["map"] + ".bsp"))}
    (out / "capture.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    return record


def linear_capture(path, film, scale):
    """Linear light of a screenshot at the reference film size."""
    image = Image.open(path).convert("RGB")
    if image.size != (film["width"] * 4, film["height"] * 4):
        # Crop to the reference aspect (a 16:9 fallback surface), then scale.
        aspect = film["width"] / film["height"]
        width, height = image.size
        if width / height > aspect:
            crop = round(height * aspect)
            image = image.crop(((width - crop) // 2, 0, (width - crop) // 2 + crop, height))
        image = image.resize((film["width"] * 4, film["height"] * 4), Image.BILINEAR)
    linear = srgb_decode(np.asarray(image)) / scale
    return linear.reshape(film["height"], 4, film["width"], 4, 3).mean(axis=(1, 3))


# Reference light a capture is judged against: Cycles DiffInd (the indirect
# view), or DiffDir + DiffInd (all diffuse light: a model lit through the
# ambient cube alone, which carries the volume's total layer).
REFERENCE_LIGHT = {"indirect": ("indirect",), "diffuse": ("direct", "indirect")}


def region_light(region, light):
    return np.sum([np.asarray(region[key], dtype=np.float64)
                   for key in REFERENCE_LIGHT[light]], axis=0)


def reference_level(fixture, state, light="indirect"):
    """The brightest region's reference luminance over every camera of a
    state: the scale a region that should be dark is judged against."""
    record = json.loads((fixture["directory"] / "references" / "references.json").read_text())
    level = 0.0
    for key, view in record["views"].items():
        if view["state"] != state:
            continue
        for region in view["regions"].values():
            if region.get("indirect") and region["pixels"] >= 50:
                level = max(level, gi_reference.luminance(region_light(region, light)))
    return level


def view_masks(fixture, state, camera):
    """(reference view record, {region: mask}) for a fixture camera. A camera
    that renders another's view (the portal-view camera looking through the
    portal pair) takes that camera's reference and masks."""
    references = fixture["directory"] / "references"
    record = json.loads((references / "references.json").read_text())
    camera = fixture.get("reference_cameras", {}).get(camera, camera)
    view = record["views"]["%s.%s" % (state, camera)]
    index = np.asarray(Image.open(references / view["files"]["index"]["file"]))
    masks = gi_reference.region_masks(index, view["object_index"], fixture["regions"][camera],
                                      {"props": [{"name": name, "shapes": [mesh]} for name, mesh
                                                 in fixture_props(fixture).items()]})
    return view, masks


def compare_view(fixture, state, camera, capture_path, scale, gate_models, tolerance, level,
                 light="indirect", absolute_fraction=ABSOLUTE_FRACTION):
    references = fixture["directory"] / "references"
    view, masks = view_masks(fixture, state, camera)
    camera = fixture.get("reference_cameras", {}).get(camera, camera)
    import imageio.v3 as iio
    reference = np.asarray(iio.imread(references / view["files"]["indirect"]["file"]),
                           dtype=np.float64)[..., :3]
    measured = linear_capture(capture_path, fixture["film"], scale)
    scene_props = {region for region, entries in fixture["regions"][camera].items()
                   if any(entry in fixture_props(fixture) for entry in entries)}
    regions = {}
    allowance_floor = absolute_fraction * level
    for region, mask in masks.items():
        if mask.sum() < 50:
            regions[region] = {"pixels": int(mask.sum()), "status": "too-small"}
            continue
        # The indirect image's mean over the mask, or the recorded region
        # means (the same masks) for the other light.
        expected = reference[mask].mean(axis=0) if light == "indirect" else \
            region_light(view["regions"][region], light)
        observed = measured[mask].mean(axis=0)
        lum_expected = gi_reference.luminance(expected)
        lum_observed = gi_reference.luminance(observed)
        allowance = tolerance * lum_expected + allowance_floor
        # The error in units of the allowance: <= 1 passes.
        error = abs(lum_observed - lum_expected) / max(allowance, 1e-9)
        kind = "model" if region in scene_props else "world"
        # The world view shows the LMAP indirect layer: gated only against
        # indirect light.
        gated = (kind == "world" and light == "indirect") or (kind == "model" and gate_models)
        regions[region] = {"kind": kind, "pixels": int(mask.sum()),
                           "reference_rgb": expected.tolist(), "measured_rgb": observed.tolist(),
                           "reference_luminance": lum_expected,
                           "measured_luminance": lum_observed,
                           "relative_error": abs(lum_observed - lum_expected) /
                           max(lum_expected, 1e-9),
                           "allowance": allowance, "error_over_allowance": error,
                           "gated": gated,
                           "status": ("pass" if error <= 1.0 else "fail") if gated
                           else "measured"}
    return regions


def fixture_props(fixture):
    """{dynamic model: its stand-in mesh} as the fixture declares them."""
    return {model["name"]: model["stand_in_mesh"] for model in fixture["dynamic_models"]}


def compare(args):
    fixture = gi_reference.load_fixture(args.fixture)
    capture_record = json.loads((Path(args.capture) / "capture.json").read_text())
    views = {}
    failures = []
    light = getattr(args, "reference_light", "indirect")
    # A state that should be dark throughout (a closed door) is judged at the
    # light level of the state named by --level-state.
    level = reference_level(fixture, getattr(args, "level_state", None) or args.state, light)
    absolute_fraction = getattr(args, "absolute_fraction", None) or ABSOLUTE_FRACTION
    for camera, shot in sorted(capture_record["cameras"].items()):
        if shot["status"] != "pass" or not shot["screenshot"]:
            failures.append("%s: capture did not pass (%s)" % (camera, shot["failures"]))
            continue
        regions = compare_view(fixture, args.state, camera, shot["screenshot"],
                               args.declared_scale or capture_record["scale"], args.gate_models,
                               args.tolerance, level, light, absolute_fraction)
        views[camera] = regions
        for region, result in regions.items():
            if result["status"] == "fail":
                failures.append("%s/%s: measured %.4f, reference %.4f, allowance %.4f" %
                                (camera, region, result["measured_luminance"],
                                 result["reference_luminance"], result["allowance"]))
            if result["status"] == "too-small":
                failures.append("%s/%s: region has %d pixels" % (camera, region,
                                                                result["pixels"]))
    result = {"schema": SCHEMA, "fixture": args.fixture, "state": args.state,
              "capture": str(Path(args.capture).resolve()), "view": capture_record["view"],
              "capture_scale": capture_record["scale"],
              "declared_scale": args.declared_scale or capture_record["scale"],
              "tolerance": args.tolerance, "absolute_fraction": absolute_fraction,
              "reference_level": level,
              "level_state": getattr(args, "level_state", None) or args.state,
              "reference": "Cycles DiffInd (indirect diffuse light, E / pi)"
              if light == "indirect" else "Cycles DiffDir + DiffInd (diffuse light, E / pi)",
              "reference_light": light,
              "views": views, "failures": failures,
              "status": "fail" if failures else "pass"}
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    Path(args.out).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    for camera, regions in views.items():
        for region, entry in regions.items():
            if "relative_error" in entry:
                print("  %-8s %-12s %-6s ref %.4f got %.4f rel %.3f /allow %.2f %s" % (
                    camera, region, entry["kind"], entry["reference_luminance"],
                    entry["measured_luminance"], entry["relative_error"],
                    entry["error_over_allowance"], entry["status"]))
    print("GI runtime compare %s/%s: %s" % (args.fixture, args.state, result["status"]))
    return result


def capture_oracles(args):
    """The fixture's single-state oracles judged on a capture's indirect light."""
    fixture = gi_reference.load_fixture(args.fixture)
    capture_record = json.loads((Path(args.capture) / "capture.json").read_text())
    state = args.state or fixture["baked_state"]
    level = reference_level(fixture, state)
    absolute = args.absolute_fraction * level
    scale = args.declared_scale or capture_record["scale"]
    views, failures, measured = {}, [], {}
    for camera, shot in sorted(capture_record["cameras"].items()):
        if shot["status"] != "pass" or not shot["screenshot"]:
            failures.append("%s: capture did not pass (%s)" % (camera, shot["failures"]))
            continue
        _, masks = view_masks(fixture, state, camera)
        image = linear_capture(shot["screenshot"], fixture["film"], scale)
        regions = {}
        for region, mask in masks.items():
            regions[region] = {"pixels": int(mask.sum())}
            if mask.sum() >= 50:
                regions[region]["indirect"] = image[mask].mean(axis=0).tolist()
        views[(state, camera)] = regions
        measured[camera] = regions
    oracles = gi_oracles.for_capture(fixture, state, args.tolerance, absolute)
    results = gi_oracles.evaluate({"oracles": oracles}, views) if not failures else []
    for result in results:
        failures += ["%s observed %.5g expected %.5g" % (c["check"], c["observed"],
                                                        c["expected"])
                     for c in result["checks"] if not c["ok"]]
        if not result["control"]["rejected"]:
            failures.append("oracle %s did not reject its control (%s)" % (
                result["oracle"], result["control"]["description"]))
    record = {"schema": "gi-runtime-oracles/v1", "fixture": args.fixture, "state": state,
              "capture": str(Path(args.capture).resolve()), "declared_scale": scale,
              "relative_tolerance": args.tolerance, "zero_bound": absolute,
              "reference_level": level, "regions": measured,
              "oracles": results, "summary": gi_oracles.summary(results),
              "failures": failures,
              "status": "fail" if failures or not results else "pass"}
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    Path(args.out).write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    for result in results:
        print("  %-4s %-60s %3d checks" % ("ok" if result["ok"] else "FAIL", result["oracle"],
                                           len(result["checks"])))
    print("GI runtime oracles %s/%s: %s (%d oracles, %d checks, %d failures)" % (
        args.fixture, state, record["status"], len(results),
        record["summary"]["checks"], len(failures)))
    return record


def indirect_view(args):
    """G0 oracle: the view passes at scale 1; a doubled seeded view fails."""
    out = Path(args.out)
    fixture = gi_reference.load_fixture(args.fixture)
    if reference_level(fixture, args.state) < UNLIT_LUMINANCE:
        # A doubled zero is zero: the seeded control cannot fail, so the
        # oracle has no power here (probe-grid's single floor under the sky).
        raise SystemExit("indirect-view oracle: %s/%s has no reference indirect light; "
                         "the view cannot be judged" % (args.fixture, args.state))
    runs = {}
    for name, scale in (("view", 1.0), ("seeded-double", 2.0)):
        capture_args = argparse.Namespace(**vars(args))
        capture_args.scale, capture_args.out, capture_args.view = scale, out / name, 1
        capture(capture_args)
        compare_args = argparse.Namespace(fixture=args.fixture, state=args.state,
                                          capture=out / name, out=out / name / "gate.json",
                                          declared_scale=1.0, gate_models=args.gate_models,
                                          tolerance=args.tolerance)
        runs[name] = compare(compare_args)
    passed = runs["view"]["status"] == "pass" and runs["seeded-double"]["status"] == "fail"
    summary = {"schema": "gi-indirect-view-oracle/v1", "fixture": args.fixture,
               "state": args.state, "view_status": runs["view"]["status"],
               "seeded_double_status": runs["seeded-double"]["status"],
               "gate_models": args.gate_models,
               "status": "pass" if passed else "fail"}
    (out / "oracle.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")
    print("indirect-view oracle: %s" % summary["status"])
    return 0 if passed else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)

    def capture_options(p):
        p.add_argument("--fixture", required=True)
        p.add_argument("--camera")
        p.add_argument("--map-build", required=True, help="pbrt_map_build.py output directory")
        p.add_argument("--build", help="client build tree (default: the pipeline toolchain's)")
        p.add_argument("--runtime")
        p.add_argument("--toolchain", type=Path)
        p.add_argument("--console-command", action="append", default=[])
        p.add_argument("--out", required=True)
    c = commands.add_parser("capture")
    capture_options(c)
    c.add_argument("--view", type=int, default=1, choices=(0, 1, 2))
    c.add_argument("--capture-wait", type=int,
                   help="frames from the start of the console line to the scored screenshot "
                        "(default %d; the camera placement uses %d of them)" % (
                            CAPTURE_WAIT, PLACEMENT_FRAMES))
    c.add_argument("--scale", type=float, default=1.0)
    m = commands.add_parser("compare")
    m.add_argument("--fixture", required=True)
    m.add_argument("--state", required=True)
    m.add_argument("--capture", required=True)
    m.add_argument("--out", required=True)
    m.add_argument("--declared-scale", type=float,
                   help="exposure scale to divide out (default: the capture's)")
    m.add_argument("--gate-models", action="store_true")
    m.add_argument("--reference-light", choices=sorted(REFERENCE_LIGHT), default="indirect")
    m.add_argument("--tolerance", type=float, default=WORLD_TOLERANCE)
    m.add_argument("--absolute-fraction", type=float,
                   help="the dark-region allowance as a fraction of the reference level "
                        "(default %g; a producer's declared tolerance)" % ABSOLUTE_FRACTION)
    m.add_argument("--level-state",
                   help="the state whose brightest region sets the reference level "
                        "(default: the compared state; for a state that is dark throughout)")
    o = commands.add_parser("oracles")
    o.add_argument("--fixture", required=True)
    o.add_argument("--capture", required=True)
    o.add_argument("--out", required=True)
    o.add_argument("--state", help="the captured state (default: the fixture's baked state)")
    o.add_argument("--declared-scale", type=float,
                   help="exposure scale to divide out (default: the capture's)")
    o.add_argument("--tolerance", type=float, default=WORLD_TOLERANCE,
                   help="least relative tolerance of equal, chromaticity, value and uniform "
                        "oracles (default %g)" % WORLD_TOLERANCE)
    o.add_argument("--absolute-fraction", type=float, default=ABSOLUTE_FRACTION,
                   help="a zero oracle's bound as a fraction of the reference level "
                        "(default %g)" % ABSOLUTE_FRACTION)
    v = commands.add_parser("indirect-view")
    capture_options(v)
    v.add_argument("--state", default="default")
    v.add_argument("--tolerance", type=float, default=WORLD_TOLERANCE)
    v.add_argument("--gate-models", action="store_true",
                   help="judge dynamic-model regions too (RFC 0011 G1: the probe volume)")
    args = parser.parse_args()
    if args.command == "capture":
        record = capture(args)
        return 0 if all(c["status"] == "pass" for c in record["cameras"].values()) else 1
    if args.command == "compare":
        return 0 if compare(args)["status"] == "pass" else 1
    if args.command == "oracles":
        return 0 if capture_oracles(args)["status"] == "pass" else 1
    return indirect_view(args)


if __name__ == "__main__":
    sys.exit(main())
