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

from worldstage_directional_bake_compare import expected_directions
from worldstage_sh_l1_plan import load_plan


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


def check_extra_faces(authored, expected, identity):
    if (set(authored) != set(expected) or
            any(np.linalg.norm(np.asarray(direction) - expected[name][identity]) > 1e-4
                for name, direction in authored.items())):
        raise ValueError("supplemental Cycles direction differs from compiled BSP")


def fit(compare_path, geometry_comparison_path, paths, output, plan_path=None,
        extra_dir=None, bsp_path=None, manifest_path=None, negative_self_test=False):
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
    plan = None
    plan_hash = None
    aggregate_hash = None
    directions = BASIS
    negative_rejected = False
    if plan_path is not None:
        if extra_dir is None or bsp_path is None or manifest_path is None:
            raise ValueError("supplemental fit requires EXRs, compiled BSP, and Stage manifest")
        plan, plan_hash = load_plan(plan_path)
        if sha256(bsp_path) != comparison["bsp_sha256"]:
            raise ValueError("supplemental fit BSP differs from verified Stage")
        aggregate_path = extra_dir / "supplemental-bakes.json"
        aggregate = json.loads(aggregate_path.read_text())
        aggregate_hash = sha256(aggregate_path)
        chart_ids = set(json.loads(paths["flat"].with_suffix(".json").read_text())[
            "chart_max_linear"])
        if (aggregate["status"] != "pass" or aggregate["plan_sha256"] != plan_hash or
                aggregate["stage_sha256"] != comparison["stage_sha256"] or
                aggregate["manifest_sha256"] != sha256(manifest_path) or
                aggregate["width"] != comparison["width"] or
                aggregate["height"] != comparison["height"] or
                aggregate["samples"] != comparison["samples"] or
                set(aggregate["bakes"]) != {entry["id"] for entry in plan["additional"]}):
            raise ValueError("supplemental Cycles bake manifest does not match base bakes")
        expected = expected_directions(bsp_path, plan["additional"])
        for entry in plan["additional"]:
            identity = entry["id"]
            extra_path = extra_dir / (identity + ".exr")
            extra_evidence_path = extra_path.with_suffix(".json")
            extra_evidence = json.loads(extra_evidence_path.read_text())
            if (extra_evidence["status"] != "pass" or
                    extra_evidence["source_basis"] != identity or
                    extra_evidence["stage_sha256"] != comparison["stage_sha256"] or
                    extra_evidence["exr_sha256"] != sha256(extra_path) or
                    extra_evidence["width"] != comparison["width"] or
                    extra_evidence["height"] != comparison["height"] or
                    extra_evidence["samples"] != comparison["samples"] or
                    aggregate["bakes"][identity]["exr_sha256"] != sha256(extra_path) or
                    aggregate["bakes"][identity]["evidence_sha256"] !=
                    sha256(extra_evidence_path) or
                    set(extra_evidence["chart_max_linear"]) != chart_ids or
                    any(value <= 1e-5 for value in
                        extra_evidence["chart_max_linear"].values())):
                raise ValueError("supplemental Cycles direction differs from compiled BSP")
            check_extra_faces(extra_evidence["source_face_directions"], expected, identity)
            if negative_self_test and not negative_rejected:
                changed = dict(extra_evidence["source_face_directions"])
                changed[next(iter(changed))] = [0.0, 0.0, 0.0]
                try:
                    check_extra_faces(changed, expected, identity)
                except ValueError:
                    negative_rejected = True
                if not negative_rejected:
                    raise ValueError("seeded supplemental direction was accepted")
            paths[identity] = extra_path
        modes = modes + tuple(entry["id"] for entry in plan["additional"])
        directions = np.concatenate((BASIS, np.asarray([
            entry["normal"] for entry in plan["additional"]], dtype=np.float64)))
    images = []
    for mode in modes:
        path = paths[mode]
        expected_hash = (comparison["exr_sha256"][mode] if mode in comparison["exr_sha256"]
                         else json.loads(path.with_suffix(".json").read_text())["exr_sha256"])
        if sha256(path) != expected_hash:
            raise ValueError("Cycles " + mode + " EXR changed since basis verification")
        image = iio.imread(path)
        if (image.shape != (comparison["height"], comparison["width"], 3) or
                image.dtype != np.float32 or not np.isfinite(image).all() or
                image.min() < -1e-5):
            raise ValueError("Cycles " + mode + " has invalid linear irradiance")
        images.append(image)
    directional = np.stack(images, axis=2)
    design = np.column_stack((np.full(len(directions), Y00), directions * Y1))
    inverse = np.linalg.pinv(design)
    coefficients = np.einsum("ij,hwjc->hwic", inverse, directional)
    reconstructed = np.einsum("ij,hwjc->hwic", design, coefficients)
    absolute_error = np.abs(reconstructed - directional)
    worst = float(absolute_error.max())
    if plan is None and worst > 1e-4:
        raise ValueError("SH L1 fit cannot reconstruct its four bake directions")
    output.parent.mkdir(parents=True, exist_ok=True)
    np.savez_compressed(output, coefficients=coefficients.astype(np.float32))
    check = np.load(output)["coefficients"]
    if check.shape != coefficients.shape or not np.isfinite(check).all():
        raise ValueError("SH L1 preview artifact cannot be read back")
    evidence = {"status": "pass", "scope": "unencoded-sh-l1-least-squares-preview" if plan
                else "unencoded-sh-l1-preview",
                "style": style, "bsp_sha256": geometry["bsp_sha256"],
                "stage_sha256": comparison["stage_sha256"],
                "directional_comparison_sha256": sha256(compare_path),
                "geometry_comparison_sha256": sha256(geometry_comparison_path),
                "source_exr_sha256": {mode: sha256(paths[mode]) for mode in modes},
                "direction_plan_sha256": plan_hash,
                "supplemental_bake_manifest_sha256": aggregate_hash,
                "negative_wrong_extra_direction_rejected": negative_rejected,
                "fit_directions": {mode: list(map(float, directions[index]))
                                   for index, mode in enumerate(modes)},
                "npz_sha256": sha256(output), "width": comparison["width"],
                "height": comparison["height"], "samples": comparison["samples"],
                "coefficient_order": ["L0", "L1x", "L1y", "L1z"],
                "normalization": {"Y00": Y00, "Y1": Y1},
                "max_reconstruction_error": worst,
                "mean_reconstruction_error": float(absolute_error.mean()),
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
    parser.add_argument("--direction-plan", type=Path)
    parser.add_argument("--extra-dir", type=Path)
    parser.add_argument("--bsp", type=Path)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--negative-self-test", action="store_true")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if args.out.suffix != ".npz":
        parser.error("output must use .npz for this preview artifact")
    if args.negative_self_test and not args.direction_plan:
        parser.error("supplemental negative control requires --direction-plan")
    paths = {mode: getattr(args, mode) for mode in ("flat", "rnm0", "rnm1", "rnm2")}
    print(json.dumps(fit(args.comparison, args.geometry_comparison, paths, args.out,
                         args.direction_plan, args.extra_dir, args.bsp, args.manifest,
                         args.negative_self_test),
                     sort_keys=True))


if __name__ == "__main__":
    main()
