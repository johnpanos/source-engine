#!/usr/bin/env python3
"""Measure the four-direction SH L1 fit against analytic clamped cosine."""

import argparse
import json
import math
from pathlib import Path

import numpy as np

from worldstage_sh_l1_plan import load_plan
from worldstage_sh_l1_preview import BASIS, Y00, Y1


def hemisphere():
    directions = []
    for elevation in np.linspace(0.0, math.pi / 2.0, 33):
        for azimuth in np.linspace(0.0, 2.0 * math.pi, 64, endpoint=False):
            directions.append((math.cos(elevation) * math.cos(azimuth),
                               math.cos(elevation) * math.sin(azimuth),
                               math.sin(elevation)))
    return np.asarray(directions)


def evaluate(extra_directions=()):
    normals = hemisphere()
    design = np.column_stack((np.full(len(normals), Y00),
                              normals * Y1))
    directions = np.concatenate((BASIS, np.asarray(extra_directions,
                                                   dtype=np.float64).reshape((-1, 3))))
    sample_design = np.column_stack((np.full(len(directions), Y00), directions * Y1))
    inverse = np.linalg.pinv(sample_design)
    errors = []
    least_squares_errors = []
    worst_light = None
    for light in hemisphere()[::37]:
        sample = np.maximum(directions @ light, 0.0)
        approximation = design @ (inverse @ sample)
        exact = np.maximum(normals @ light, 0.0)
        error = np.abs(approximation - exact)
        errors.append(error)
        least_squares = design @ np.linalg.lstsq(design, exact, rcond=None)[0]
        least_squares_errors.append(np.abs(least_squares - exact))
        if worst_light is None or float(error.max()) > worst_light["max_error"]:
            worst_light = {"direction": light.tolist(), "max_error": float(error.max())}
    sky = np.ones(len(directions))
    sky_error = float(np.max(np.abs(design @ (inverse @ sky) - 1.0)))
    all_errors = np.concatenate(errors)
    best_l1_errors = np.concatenate(least_squares_errors)
    return {"fit_direction_count": len(directions),
            "max_direct_error": float(all_errors.max()),
            "p95_direct_error": float(np.percentile(all_errors, 95)),
            "mean_direct_error": float(all_errors.mean()),
            "dense_least_squares_l1": {
                "max_direct_error": float(best_l1_errors.max()),
                "p95_direct_error": float(np.percentile(best_l1_errors, 95)),
                "mean_direct_error": float(best_l1_errors.mean())},
            "uniform_sky_error": sky_error, "worst_light": worst_light,
            "light_count": len(errors), "normal_count": len(normals)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--max-direct-error", type=float, default=0.15)
    parser.add_argument("--direction-plan", type=Path)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    plan, plan_hash = load_plan(args.direction_plan) if args.direction_plan else (None, None)
    extra = [entry["normal"] for entry in plan["additional"]] if plan else ()
    result = evaluate(extra)
    result.update({"scope": "supplemental-sh-l1-analytic-preview" if plan else
                   "four-direction-sh-l1-analytic-preview",
                   "direction_plan_sha256": plan_hash,
                   "declared_max_direct_error": args.max_direct_error,
                   "status": "pass" if result["max_direct_error"] <= args.max_direct_error
                   else "fail"})
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))
    return 0 if result["status"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
