#!/usr/bin/env python3
"""Check a Cycles direction excluded from four- and twelve-sample SH fits."""

import argparse
import hashlib
import json
from pathlib import Path

import imageio.v3 as iio
import numpy as np

from worldstage_directional_bake_compare import expected_directions
from worldstage_sh_l1_preview import Y00, Y1


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_directions(authored, expected):
    if set(authored) != set(expected):
        raise ValueError("held-out Cycles bake omitted a BSP world face")
    worst = max(float(np.linalg.norm(np.asarray(authored[name]) -
                                     expected[name]["heldout0"])) for name in expected)
    if worst > 1e-4:
        raise ValueError("held-out Cycles normal differs from compiled BSP")
    return worst


def chart_mask(manifest, shape):
    layout = manifest["lightmap_atlas"]
    height, width = shape
    if width % layout["width"] or height % layout["height"]:
        raise ValueError("held-out EXR differs from Stage atlas size")
    scale_x, scale_y = width // layout["width"], height // layout["height"]
    mask = np.zeros(shape, dtype=bool)
    for x, y, chart_width, chart_height in layout["charts"].values():
        top = height - (y + chart_height) * scale_y
        left = x * scale_x
        region = mask[top:top + chart_height * scale_y,
                      left:left + chart_width * scale_x]
        if region.shape != (chart_height * scale_y, chart_width * scale_x) or region.any():
            raise ValueError("Stage atlas has an escaped or overlapping chart")
        region[:] = True
    if not mask.any():
        raise ValueError("Stage atlas has no charts")
    return mask


def compare(bsp_path, stage_path, manifest_path, heldout_path, four_path,
            twelve_path, negative_self_test=False):
    stage_hash = sha256(stage_path)
    bsp_hash = sha256(bsp_path)
    heldout = json.loads(heldout_path.with_suffix(".json").read_text())
    direction = np.asarray(heldout["source_local_direction"], dtype=np.float64)
    if (heldout["status"] != "pass" or heldout["source_basis"] != "heldout0" or
            heldout["stage_sha256"] != stage_hash or
            heldout["exr_sha256"] != sha256(heldout_path) or
            direction.shape != (3,) or not np.isfinite(direction).all() or
            abs(np.linalg.norm(direction) - 1.0) > 1e-6 or direction[2] < 0):
        raise ValueError("held-out Cycles bake or normal is invalid")
    expected = expected_directions(bsp_path, [{"id": "heldout0", "normal": direction}])
    worst_direction = check_directions(heldout["source_face_directions"], expected)
    negative_rejected = False
    if negative_self_test:
        changed = dict(heldout["source_face_directions"])
        changed[next(iter(changed))] = [0.0, 0.0, 0.0]
        try:
            check_directions(changed, expected)
        except ValueError:
            negative_rejected = True
        if not negative_rejected:
            raise ValueError("seeded wrong held-out normal was accepted")
    target = iio.imread(heldout_path)[:, :, :3]
    if (target.shape != (heldout["height"], heldout["width"], 3) or
            target.dtype != np.float32 or not np.isfinite(target).all()):
        raise ValueError("held-out EXR dimensions or samples are invalid")
    manifest = json.loads(manifest_path.read_text())
    if manifest["stage_sha256"] not in stage_path.read_text():
        raise ValueError("material Stage differs from chart manifest")
    mask = chart_mask(manifest, target.shape[:2])
    basis = np.r_[Y00, direction * Y1]
    results = {}
    for name, path in (("four", four_path), ("twelve", twelve_path)):
        evidence = json.loads(path.with_suffix(".json").read_text())
        if (evidence["status"] != "pass" or evidence["bsp_sha256"] != bsp_hash or
                evidence["stage_sha256"] != stage_hash or
                evidence["npz_sha256"] != sha256(path) or
                evidence["width"] != heldout["width"] or
                evidence["height"] != heldout["height"] or
                evidence["samples"] != heldout["samples"] or
                (name == "four" and evidence["direction_plan_sha256"] is not None) or
                (name == "twelve" and evidence["direction_plan_sha256"] is None) or
                any(np.linalg.norm(np.asarray(item) - direction) < 1e-3
                    for item in evidence["fit_directions"].values())):
            raise ValueError("SH fit does not match held-out Cycles bake")
        coefficients = np.load(path)["coefficients"]
        if coefficients.shape != (*target.shape[:2], 4, 3):
            raise ValueError("SH fit coefficients have invalid dimensions")
        predicted = np.einsum("i,hwic->hwc", basis, coefficients)
        error = np.abs(predicted[mask] - target[mask])
        results[name] = {"mean_absolute": float(error.mean()),
                         "p95_absolute": float(np.percentile(error, 95)),
                         "max_absolute": float(error.max()),
                         "mean_relative_to_observed": float(error.mean() /
                                                            target[mask].mean())}
    improvement = 1.0 - results["twelve"]["mean_absolute"] / results["four"]["mean_absolute"]
    if improvement < 0.10:
        raise ValueError("supplemental SH directions did not improve held-out Cycles light")
    return {"status": "pass", "scope": "sh-l1-heldout-cycles-improvement",
            "bsp_sha256": bsp_hash, "stage_sha256": stage_hash,
            "heldout_exr_sha256": sha256(heldout_path),
            "four_fit_sha256": sha256(four_path), "twelve_fit_sha256": sha256(twelve_path),
            "heldout_direction": direction.tolist(), "worst_direction_error": worst_direction,
            "negative_wrong_direction_rejected": negative_rejected,
            "chart_pixels": int(mask.sum()), "four": results["four"],
            "twelve": results["twelve"], "mean_absolute_improvement_fraction": improvement}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--heldout", type=Path, required=True)
    parser.add_argument("--four", type=Path, required=True)
    parser.add_argument("--twelve", type=Path, required=True)
    parser.add_argument("--negative-self-test", action="store_true")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    evidence = compare(args.bsp, args.stage, args.manifest, args.heldout,
                       args.four, args.twelve, args.negative_self_test)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
