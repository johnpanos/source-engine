#!/usr/bin/env python3
"""Compare a checked staircase2 USD/Cycles render with its supplied reference."""

import argparse
import hashlib
import json
from pathlib import Path

import imageio.v3 as iio
import numpy as np
from PIL import Image
from scipy.ndimage import gaussian_filter
from scipy.optimize import minimize


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def ssim(reference, candidate):
    # Conventional 11-pixel Gaussian-window SSIM over display luminance.
    weights = np.array([0.2126, 0.7152, 0.0722], dtype=np.float64)
    first = np.tensordot(reference, weights, axes=1)
    second = np.tensordot(candidate, weights, axes=1)
    mu_first = gaussian_filter(first, 1.5)
    mu_second = gaussian_filter(second, 1.5)
    var_first = gaussian_filter(first * first, 1.5) - mu_first * mu_first
    var_second = gaussian_filter(second * second, 1.5) - mu_second * mu_second
    covariance = gaussian_filter(first * second, 1.5) - mu_first * mu_second
    c1 = (0.01 * 255) ** 2
    c2 = (0.03 * 255) ** 2
    numerator = (2 * mu_first * mu_second + c1) * (2 * covariance + c2)
    denominator = (mu_first * mu_first + mu_second * mu_second + c1) * (
        var_first + var_second + c2)
    return float(np.mean(numerator / denominator))


def reference_display(linear, parameters):
    scaled = np.maximum(linear, 0.0) * np.exp(parameters[0])
    return np.clip(255.0 * (scaled / (1.0 + scaled)) ** np.exp(parameters[1]),
                   0.0, 255.0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("reference", "candidate", "evidence", "scene", "stage", "out"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--reference-exr", type=Path)
    parser.add_argument("--candidate-exr", type=Path)
    parser.add_argument("--display-candidate", type=Path)
    parser.add_argument("--max-mae", type=float)
    parser.add_argument("--min-ssim", type=float)
    parser.add_argument("--negative-self-test", action="store_true")
    args = parser.parse_args()
    evidence = json.loads(args.evidence.read_text())
    fixture_ocio = (Path(__file__).resolve().parents[2] /
                    "quality/fixtures/staircase2-ocio/config.ocio")
    if (evidence.get("status") != "pass" or evidence.get("fixture") != "staircase2" or
            evidence.get("renderer") != "Blender Cycles" or
            evidence.get("ocio_configuration_sha256") != sha256(fixture_ocio) or
            evidence.get("adapter_sha256") != sha256(
                Path(__file__).with_name("staircase2_usd_cycles.py")) or
            evidence.get("source_sha256") != sha256(args.scene) or
            evidence.get("stage_sha256") != sha256(args.stage) or
            evidence.get("image_sha256") != sha256(args.candidate) or
            evidence.get("reference_sha256") != sha256(args.reference) or
            evidence.get("mesh_count") != 19 or evidence.get("emitter_count") != 13 or
            len(evidence.get("material_assignments", {})) != 19 or
            evidence["material_assignments"].get("models/Mesh014.ply") != "Glass"):
        raise ValueError("candidate does not match its USD/Cycles/source receipt")
    if bool(args.reference_exr) != bool(args.candidate_exr):
        parser.error("both linear EXRs must be supplied together")
    if args.display_candidate and not args.reference_exr:
        parser.error("reference display rendering requires both linear EXRs")
    with Image.open(args.reference) as source:
        with Image.open(args.candidate) as rendered:
            if rendered.size != (evidence["size"], evidence["size"]):
                raise ValueError("candidate size differs from render receipt")
            original = np.asarray(source.convert("RGB").resize(
                rendered.size, Image.Resampling.LANCZOS), dtype=np.float64)
            image = np.asarray(rendered.convert("RGB"), dtype=np.float64)
    difference = np.abs(original - image)
    size = image.shape[0]
    regions = {
        "ceiling": (0.0, 0.0, 1.0, 0.26),
        "wall": (0.52, 0.0, 1.0, 0.55),
        "stairs": (0.32, 0.26, 1.0, 0.78),
        "floor": (0.0, 0.72, 1.0, 1.0),
        "glass": (0.34, 0.50, 0.53, 0.74),
    }
    region_mae = {}
    for name, (x0, y0, x1, y1) in regions.items():
        region_mae[name] = float(np.mean(difference[
            int(y0 * size):int(y1 * size), int(x0 * size):int(x1 * size)]))
    mae = float(np.mean(difference))
    structural = ssim(original, image)
    result = {
        "scope": "staircase2-usd-cycles-reference-parity",
        "scene_sha256": sha256(args.scene),
        "stage_sha256": sha256(args.stage),
        "reference_sha256": sha256(args.reference),
        "candidate_sha256": sha256(args.candidate),
        "render_evidence_sha256": sha256(args.evidence),
        "size": size,
        "standard_view_mean_absolute_rgb": mae,
        "standard_view_root_mean_square_rgb": float(np.sqrt(np.mean((original - image) ** 2))),
        "standard_view_luminance_ssim": structural,
        "standard_view_region_mean_absolute_rgb": region_mae,
        "target_max_mae": args.max_mae,
        "target_min_ssim": args.min_ssim,
    }
    if args.reference_exr:
        if (evidence.get("reference_exr_sha256") != sha256(args.reference_exr) or
                evidence.get("exr_sha256") != sha256(args.candidate_exr)):
            raise ValueError("linear EXRs differ from the render receipt")
        first = iio.imread(args.reference_exr)
        second = iio.imread(args.candidate_exr)
        if (first.shape != (size, size, 3) or second.shape != (size, size, 4) or
                not np.isfinite(first).all() or not np.isfinite(second).all() or
                not np.allclose(second[:, :, 3], 1.0)):
            raise ValueError("linear EXRs have invalid shape or values")
        second = second[:, :, :3]
        linear_difference = np.abs(first.astype(np.float64) - second.astype(np.float64))
        result["reference_exr_sha256"] = sha256(args.reference_exr)
        result["candidate_exr_sha256"] = sha256(args.candidate_exr)
        result["mean_absolute_linear_rgb"] = float(np.mean(linear_difference))
        result["reference_mean_linear_rgb"] = [float(v) for v in np.mean(first, axis=(0, 1))]
        result["candidate_mean_linear_rgb"] = [float(v) for v in np.mean(second, axis=(0, 1))]
        training = first[::4, ::4].astype(np.float64)
        training_png = original[::4, ::4]
        fit = minimize(lambda parameters: np.mean(np.abs(
            reference_display(training, parameters) - training_png)),
            [1.4, -0.2], method="Nelder-Mead",
            options={"xatol": 1e-7, "fatol": 1e-7, "maxiter": 1000})
        heldout_mae = float(np.mean(np.abs(reference_display(
            first[2::4, 2::4], fit.x) - original[2::4, 2::4])))
        if not fit.success or heldout_mae > 1.5:
            raise ValueError("reference EXR and PNG do not share the fitted display response")
        reference_view = np.rint(reference_display(second, fit.x)).astype(np.uint8)
        result["reference_display_fit"] = {
            "type": "Reinhard power from supplied EXR and PNG only",
            "gain": float(np.exp(fit.x[0])), "power": float(np.exp(fit.x[1])),
            "training_mae": float(fit.fun), "heldout_mae": heldout_mae,
        }
        result["reference_display_mean_absolute_rgb"] = float(np.mean(
            np.abs(reference_view.astype(np.float64) - original)))
        result["reference_display_luminance_ssim"] = ssim(original, reference_view)
        if args.display_candidate:
            args.display_candidate.parent.mkdir(parents=True, exist_ok=True)
            Image.fromarray(reference_view).save(args.display_candidate)
            result["reference_display_candidate_sha256"] = sha256(args.display_candidate)
    scored_mae = result.get("reference_display_mean_absolute_rgb", mae)
    scored_ssim = result.get("reference_display_luminance_ssim", structural)
    passed = ((args.max_mae is None or scored_mae <= args.max_mae) and
              (args.min_ssim is None or scored_ssim >= args.min_ssim))
    if args.negative_self_test:
        if args.max_mae is None or args.min_ssim is None:
            parser.error("negative self-test requires both comparison thresholds")
        black = np.zeros_like(original)
        black_mae = float(np.mean(np.abs(original - black)))
        black_ssim = ssim(original, black)
        if black_mae <= args.max_mae or black_ssim >= args.min_ssim:
            raise ValueError("negative black-frame control passed the parity thresholds")
        result["negative_black_frame"] = {
            "mean_absolute_rgb": black_mae, "luminance_ssim": black_ssim,
            "rejected": True,
        }
    result["status"] = "pass" if passed else "fail"
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))
    if not passed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
