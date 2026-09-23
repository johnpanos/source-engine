#!/usr/bin/env python3
"""Fit unencoded SH L1 irradiance from four verified Cycles preview bakes.

The output is an inspection NPZ, not the RFC 0008 LMAP/KTX2 package. Axis
coefficients are in each face's Source tangent frame, with standard real SH
normalization: E(n) = Y00*L0 + Y1*(nx*L1x + ny*L1y + nz*L1z).
"""

import argparse
import hashlib
import json
from pathlib import Path

import imageio.v3 as iio
import numpy as np


Y00 = 0.28209479177387814
Y1 = 0.4886025119029199
BASIS = np.array([
    (0.0, 0.0, 1.0),
    (0.81649661064147949, 0.0, 0.57735025882720947),
    (-0.40824821591377258, 0.70710676908493042, 0.57735025882720947),
    (-0.40824821591377258, -0.70710676908493042, 0.57735025882720947),
], dtype=np.float64)
DESIGN = np.column_stack((np.full(4, Y00), BASIS * Y1))


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def fit(compare_path, geometry_comparison_path, paths, output):
    comparison = json.loads(compare_path.read_text())
    geometry = json.loads(geometry_comparison_path.read_text())
    if comparison["status"] != "pass" or comparison["scope"] != (
            "cycles-rnm-preview-directions") or comparison["worst_direction_error"] > 1e-4:
        raise ValueError("directional Cycles bakes lack verified Source bases")
    if (geometry["status"] != "pass" or
            geometry["bsp_sha256"] != comparison["bsp_sha256"]):
        raise ValueError("directional bakes differ from compiled World Stage geometry")
    style_ids = set(geometry["light_styles"].values())
    if len(style_ids) != 1:
        raise ValueError("preview fit requires exactly one compiled light style")
    style = next(iter(style_ids))
    modes = ("flat", "rnm0", "rnm1", "rnm2")
    images = []
    for mode in modes:
        path = paths[mode]
        if sha256(path) != comparison["exr_sha256"][mode]:
            raise ValueError("Cycles " + mode + " EXR changed since basis verification")
        image = iio.imread(path)
        if (image.shape != (comparison["height"], comparison["width"], 3) or
                image.dtype != np.float32 or not np.isfinite(image).all() or
                image.min() < -1e-5):
            raise ValueError("Cycles " + mode + " has invalid linear irradiance")
        images.append(image.astype(np.float64))
    directional = np.stack(images, axis=2)
    inverse = np.linalg.inv(DESIGN)
    coefficients = np.einsum("ij,hwjc->hwic", inverse, directional)
    reconstructed = np.einsum("ij,hwjc->hwic", DESIGN, coefficients)
    absolute_error = np.abs(reconstructed - directional)
    worst = float(absolute_error.max())
    if worst > 1e-4:
        raise ValueError("SH L1 fit cannot reconstruct its four bake directions")
    output.parent.mkdir(parents=True, exist_ok=True)
    np.savez_compressed(output, coefficients=coefficients.astype(np.float32))
    check = np.load(output)["coefficients"]
    if check.shape != coefficients.shape or not np.isfinite(check).all():
        raise ValueError("SH L1 preview artifact cannot be read back")
    evidence = {"status": "pass", "scope": "unencoded-sh-l1-preview",
                "style": style, "bsp_sha256": geometry["bsp_sha256"],
                "stage_sha256": comparison["stage_sha256"],
                "directional_comparison_sha256": sha256(compare_path),
                "geometry_comparison_sha256": sha256(geometry_comparison_path),
                "source_exr_sha256": comparison["exr_sha256"],
                "npz_sha256": sha256(output), "width": comparison["width"],
                "height": comparison["height"], "samples": comparison["samples"],
                "coefficient_order": ["L0", "L1x", "L1y", "L1z"],
                "normalization": {"Y00": Y00, "Y1": Y1},
                "max_reconstruction_error": worst,
                "max_abs_coefficient": float(np.max(np.abs(coefficients)))}
    output.with_suffix(".json").write_text(json.dumps(evidence, indent=2,
                                                        sort_keys=True) + "\n")
    return evidence


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--comparison", type=Path, required=True)
    parser.add_argument("--geometry-comparison", type=Path, required=True)
    for mode in ("flat", "rnm0", "rnm1", "rnm2"):
        parser.add_argument("--" + mode, type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if args.out.suffix != ".npz":
        parser.error("output must use .npz for this preview artifact")
    paths = {mode: getattr(args, mode) for mode in ("flat", "rnm0", "rnm1", "rnm2")}
    print(json.dumps(fit(args.comparison, args.geometry_comparison, paths, args.out),
                     sort_keys=True))


if __name__ == "__main__":
    main()
