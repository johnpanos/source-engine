#!/usr/bin/env python3
"""Pixel gates for PBRT-derived maps against their reference images.

`render` compares the pipeline's Cycles render of the exported USD stage with
the reference image supplied with the scene (for Bitterli scenes, a Tungsten
render). With both linear EXRs it also reports the linear exposure ratio and
applies the reference's own display response, fitted from its EXR/PNG pair,
to the candidate EXR, so the gate scores scene content rather than tone
mapping.

`runtime` compares a camera-matched native Vulkan frame with that Cycles
render. `camera_commands()` places the Source camera at the PBRT eye; the
capture is cropped to the reference's aspect and exact vertical FOV.

Both modes reject a black frame and a horizontally mirrored candidate, so a
gate that cannot fail, or a handedness regression, is caught.
"""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbrt_scene  # noqa: E402

SOURCE_UNITS_PER_METER = 39.37007874015748
EYE_HEIGHT = 64.0  # VEC_VIEW in game/shared/gamerules.cpp


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def ssim(reference, candidate):
    """11-tap Gaussian (sigma 1.5) SSIM over display luminance."""
    from scipy.ndimage import gaussian_filter
    weights = np.array([0.2126, 0.7152, 0.0722])
    a = np.tensordot(reference, weights, axes=1)
    b = np.tensordot(candidate, weights, axes=1)
    mu_a, mu_b = gaussian_filter(a, 1.5), gaussian_filter(b, 1.5)
    var_a = gaussian_filter(a * a, 1.5) - mu_a * mu_a
    var_b = gaussian_filter(b * b, 1.5) - mu_b * mu_b
    covariance = gaussian_filter(a * b, 1.5) - mu_a * mu_b
    c1, c2 = (0.01 * 255) ** 2, (0.03 * 255) ** 2
    return float(np.mean((2 * mu_a * mu_b + c1) * (2 * covariance + c2) /
                         ((mu_a * mu_a + mu_b * mu_b + c1) * (var_a + var_b + c2))))


def score(reference, candidate):
    return {"mean_absolute_rgb": float(np.mean(np.abs(reference - candidate))),
            "luminance_ssim": ssim(reference, candidate)}


def passes(metrics, gate):
    return ((gate.get("max_mae") is None or metrics["mean_absolute_rgb"] <= gate["max_mae"]) and
            (gate.get("min_ssim") is None or metrics["luminance_ssim"] >= gate["min_ssim"]))


def negative_controls(reference, candidate, gate):
    """The gate must reject a black frame and a mirrored candidate."""
    controls = {"black_frame": score(reference, np.zeros_like(reference)),
                "mirrored_candidate": score(reference, candidate[:, ::-1])}
    for name, metrics in controls.items():
        metrics["rejected"] = not passes(metrics, gate)
    return controls


def reference_display(linear, parameters):
    scaled = np.maximum(linear, 0.0) * np.exp(parameters[0])
    return np.clip(255.0 * (scaled / (1.0 + scaled)) ** np.exp(parameters[1]), 0.0, 255.0)


def fit_display(reference_linear, reference_png):
    """Fit the reference's Reinhard-power display response from its own pair."""
    from scipy.optimize import minimize
    training, target = reference_linear[::4, ::4], reference_png[::4, ::4]
    fit = minimize(lambda p: np.mean(np.abs(reference_display(training, p) - target)),
                   [0.0, -0.2], method="Nelder-Mead",
                   options={"xatol": 1e-7, "fatol": 1e-7, "maxiter": 2000})
    heldout = float(np.mean(np.abs(reference_display(reference_linear[2::4, 2::4], fit.x) -
                                   reference_png[2::4, 2::4])))
    return fit, heldout


def exposure_ratio(reference_linear, candidate_linear):
    """Median candidate/reference luminance over mid-tone reference pixels."""
    weights = np.array([0.2126, 0.7152, 0.0722])
    a = np.tensordot(reference_linear, weights, axes=1)
    b = np.tensordot(candidate_linear, weights, axes=1)
    low, high = np.percentile(a, 10), np.percentile(a, 90)
    mask = (a > max(low, 1e-4)) & (a < high)
    return float(np.median(b[mask] / a[mask]))


def load_rgb(path, size=None):
    with Image.open(path) as opened:
        image = opened.convert("RGB")
    if size and image.size != size:
        image = image.resize(size, Image.Resampling.LANCZOS)
    return np.asarray(image, dtype=np.float64)


def compare_render(args):
    gate = json.loads(args.gate) if args.gate else {}
    candidate = load_rgb(args.candidate)
    height, width = candidate.shape[:2]
    reference = load_rgb(args.reference, (width, height))
    result = {"scope": "pbrt-reference-render-comparison",
              "reference_sha256": sha256(args.reference),
              "candidate_sha256": sha256(args.candidate), "size": [width, height],
              "gate": gate, "standard_view": score(reference, candidate)}
    scored = result["standard_view"]
    if args.reference_exr and args.candidate_exr:
        import imageio.v3 as iio
        first = iio.imread(args.reference_exr)[:, :, :3].astype(np.float64)
        second = iio.imread(args.candidate_exr)[:, :, :3].astype(np.float64)
        if first.shape[:2] != (height, width) or second.shape[:2] != (height, width):
            raise ValueError("linear EXRs must match the candidate size")
        if not (np.isfinite(first).all() and np.isfinite(second).all()):
            raise ValueError("linear EXRs contain non-finite values")
        ratio = exposure_ratio(first, second)
        result["linear_exposure_ratio"] = ratio
        result["linear_exposure_stops"] = math.log2(ratio)
        fit, heldout = fit_display(first, reference)
        if not fit.success or heldout > 2.0:
            raise ValueError("reference EXR and PNG do not share a fitted display response")
        display = np.rint(reference_display(second, fit.x))
        result["reference_display_fit"] = {"gain": float(np.exp(fit.x[0])),
                                           "power": float(np.exp(fit.x[1])),
                                           "training_mae": float(fit.fun),
                                           "heldout_mae": heldout}
        result["reference_display"] = score(reference, display)
        scored = result["reference_display"]
        if args.display_candidate:
            Image.fromarray(display.astype(np.uint8)).save(args.display_candidate)
        candidate = display
    result["scored"] = "reference_display" if scored is not result["standard_view"] else "standard_view"
    passed = passes(scored, gate)
    if gate.get("max_exposure_stops") is not None and "linear_exposure_stops" in result:
        passed = passed and abs(result["linear_exposure_stops"]) <= gate["max_exposure_stops"]
    result["negative_controls"] = negative_controls(reference, candidate, gate)
    return result, passed


def camera_commands(scene):
    """Console commands that put the Source camera at the PBRT reference eye."""
    pose = pbrt_scene.camera_pose(scene)
    eye = [value * SOURCE_UNITS_PER_METER for value in pose["eye"]]
    forward = pose["forward"]
    yaw = math.degrees(math.atan2(forward[1], forward[0]))
    pitch = -math.degrees(math.asin(max(-1.0, min(1.0, forward[2]))))
    film = scene["film"]
    fov = math.radians(scene["camera"]["fov_degrees"])
    # PBRT's fov spans the shorter axis; Source's fov is a 4:3 horizontal fov.
    vertical = fov if film["width"] >= film["height"] else 2 * math.atan(
        math.tan(fov / 2) * film["height"] / film["width"])
    source_fov = round(math.degrees(2 * math.atan(math.tan(vertical / 2) * 4 / 3)))
    captured_vertical = 2 * math.atan(math.tan(math.radians(source_fov) / 2) * 3 / 4)
    place = ["cmd setpos %.3f %.3f %.3f" % (eye[0], eye[1], eye[2] - EYE_HEIGHT),
             "cmd setang %.4f %.4f 0" % (pitch, yaw)]
    # noclip must be active before the eye moves below standing height, or the
    # server unsticks the player back onto the floor; place twice under load.
    commands = (["r_worldmesh_draw 2", "cmd noclip", "cmd fov %d" % source_fov, "wait 60"] +
                place + ["wait 120"] + place + ["wait 60"])
    return commands, {"reference_vertical_fov_degrees": math.degrees(vertical),
                      "capture_vertical_fov_degrees": math.degrees(captured_vertical),
                      "source_fov": source_fov}


def matched_crop(image, aspect, reference_vfov, capture_vfov):
    """Crop a capture to the reference aspect and exact vertical FOV."""
    width, height = image.size
    scale = math.tan(math.radians(reference_vfov) / 2) / math.tan(math.radians(capture_vfov) / 2)
    crop_height = height * scale
    crop_width = crop_height * aspect
    if crop_width > width:
        raise ValueError("capture is narrower than the reference view")
    left, top = (width - crop_width) / 2, (height - crop_height) / 2
    box = tuple(round(v) for v in (left, top, left + crop_width, top + crop_height))
    return image.crop(box), list(box)


def compare_runtime(args):
    gate = json.loads(args.gate) if args.gate else {}
    scene = pbrt_scene.parse(args.scene)
    commands, fovs = camera_commands(scene)
    boot = json.loads(args.boot_evidence.read_text())
    if boot.get("schema") != "portal-boot-evidence/v1" or boot.get("status") != "pass":
        raise ValueError("runtime boot did not pass")
    command = boot.get("command", [])
    if "-renderer" not in command or command[command.index("-renderer") + 1] != "native-vulkan":
        raise ValueError("runtime capture did not use native Vulkan")
    config = args.boot_evidence.parent / "runtime/portal/cfg/portal_boot_commands.cfg"
    if config.read_text().splitlines() != commands:
        raise ValueError("runtime camera commands differ from the PBRT reference camera")
    console = (args.boot_evidence.parent / "runtime/portal/console.log").read_text(
        errors="replace")
    if console.count("noclip ON") != 1 or "noclip OFF" in console:
        raise ValueError("runtime camera requires noclip to stay on exactly once")
    shots = boot.get("screenshots", [])
    if len(shots) != 1 or sha256(shots[0]["path"]) != shots[0]["sha256"]:
        raise ValueError("runtime boot must record exactly one verified screenshot")
    reference = load_rgb(args.reference)
    height, width = reference.shape[:2]
    with Image.open(shots[0]["path"]) as opened:
        capture = opened.convert("RGB")
    cropped, box = matched_crop(capture, width / height, fovs["reference_vertical_fov_degrees"],
                                fovs["capture_vertical_fov_degrees"])
    candidate = np.asarray(cropped.resize((width, height), Image.Resampling.LANCZOS),
                           dtype=np.float64)
    if args.matched_frame:
        Image.fromarray(candidate.astype(np.uint8)).save(args.matched_frame)
    metrics = score(reference, candidate)
    result = {"scope": "pbrt-runtime-camera-comparison", "reference_sha256": sha256(args.reference),
              "boot_evidence_sha256": sha256(args.boot_evidence),
              "capture_sha256": shots[0]["sha256"], "capture_size": list(capture.size),
              "crop_box": box, "camera_commands": commands, **fovs, "gate": gate,
              "runtime_view": metrics, "scored": "runtime_view",
              "negative_controls": negative_controls(reference, candidate, gate)}
    return result, passes(metrics, gate)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    modes = parser.add_subparsers(dest="mode", required=True)
    render = modes.add_parser("render", help="Cycles render vs supplied reference image")
    render.add_argument("--reference", type=Path, required=True)
    render.add_argument("--candidate", type=Path, required=True)
    render.add_argument("--reference-exr", type=Path)
    render.add_argument("--candidate-exr", type=Path)
    render.add_argument("--display-candidate", type=Path)
    runtime = modes.add_parser("runtime", help="camera-matched game frame vs Cycles render")
    runtime.add_argument("--scene", type=Path, required=True)
    runtime.add_argument("--reference", type=Path, required=True)
    runtime.add_argument("--boot-evidence", type=Path, required=True)
    runtime.add_argument("--matched-frame", type=Path)
    for mode in (render, runtime):
        mode.add_argument("--gate", help='JSON thresholds, e.g. {"max_mae": 12, "min_ssim": 0.9}')
        mode.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    result, passed = compare_render(args) if args.mode == "render" else compare_runtime(args)
    gate = result["gate"]
    if gate and not all(control["rejected"] for control in result["negative_controls"].values()):
        raise SystemExit("gate thresholds accept a negative control: %s" %
                         json.dumps(result["negative_controls"]))
    result["status"] = "pass" if passed else "fail"
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps({"status": result["status"], "scored": result[result["scored"]],
                      **({"linear_exposure_stops": result["linear_exposure_stops"]}
                         if "linear_exposure_stops" in result else {})}, sort_keys=True))
    if not passed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
