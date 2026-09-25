#!/usr/bin/env python3
"""Measure a lightmap bake's Monte Carlo noise and gate on it.

The bake (pbrt_lightmap_bake.py --noise-pair) renders every light page as
two independent halves - half the samples each, seeds s and s + 1 - and
uses their mean, a full-sample estimate. For halves A and B of one texel,
(A - B)^2 / 4 is an unbiased estimate of the mean's variance.

Noise is judged the way it is seen: relative to the light, luminance only,
with a floor of FLOOR_FRACTION of the median light so near-black texels,
where no mottling is visible, do not dominate. A single texel's difference
is itself noisy, so the per-texel variance is averaged over a small
neighbourhood of covered texels (NEIGHBOURHOOD x NEIGHBOURHOOD) before the
percentile is taken. Noise falls as 1 / sqrt(samples), so a failing bake
reports the sample count that would meet the target.

    python3 lightmap_noise.py --first half-a.exr --second half-b.exr \\
        --coverage uv-coverage.exr --samples 1024 --target 0.01 --out noise.json
"""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))

SCHEMA = "lightmap-noise/v1"
FLOOR_FRACTION = 0.02
NEIGHBOURHOOD = 5
PERCENTILE = 99.0
LUMINANCE = np.array([0.2126, 0.7152, 0.0722])


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def relative_noise(first, second, coverage):
    """Per-texel relative standard deviation of the pair's mean, averaged
    over covered neighbours; NaN outside coverage."""
    if first.shape != second.shape or first.shape[:2] != coverage.shape:
        raise ValueError("pair halves and coverage must be the same size")
    if not coverage.any():
        raise ValueError("no covered texels to measure")
    a = first[..., :3].astype(np.float64) @ LUMINANCE
    b = second[..., :3].astype(np.float64) @ LUMINANCE
    if np.array_equal(a[coverage], b[coverage]):
        raise ValueError("the pair's halves are identical: the seeds were not varied")
    mean = (a + b) / 2
    variance = np.where(coverage, (a - b) ** 2 / 4, 0.0)
    from scipy import ndimage
    size = (NEIGHBOURHOOD, NEIGHBOURHOOD)
    weight = ndimage.uniform_filter(coverage.astype(np.float64), size, mode="constant")
    local_variance = ndimage.uniform_filter(variance, size, mode="constant") / np.maximum(weight,
                                                                                        1e-12)
    local_mean = ndimage.uniform_filter(np.where(coverage, mean, 0.0), size,
                                        mode="constant") / np.maximum(weight, 1e-12)
    floor = max(float(np.median(mean[coverage])) * FLOOR_FRACTION, 1e-9)
    noise = np.sqrt(np.maximum(local_variance, 0.0)) / np.maximum(local_mean, floor)
    return np.where(coverage, noise, np.nan), floor


def required_samples(samples, noise, target):
    """Samples at which `noise` (measured at `samples`) falls to `target`."""
    return int(math.ceil(samples * (noise / target) ** 2))


def statistics(first, second, coverage):
    noise, floor = relative_noise(first, second, coverage)
    return {"relative_noise": float(np.nanpercentile(noise, PERCENTILE)),
            "median_relative_noise": float(np.nanmedian(noise)), "floor": floor}


def measure(first, second, coverage, samples, target, denoiser=None):
    """Gate record for a pair of half bakes. With a lightmap_denoise.Denoiser
    the gate judges what is seen: each half is denoised per chart at one
    input scale, and the residual difference must meet the target (a clean
    result must not depend on which samples it came from); the raw noise is
    recorded beside it."""
    raw = statistics(first, second, coverage)
    judged = raw
    record = {"schema": SCHEMA, "percentile": PERCENTILE, "samples": samples, "target": target,
              "covered_texels": int(coverage.sum()), "raw": raw}
    if denoiser is not None:
        import lightmap_denoise
        mean = (first[..., :3].astype(np.float32) + second[..., :3]) / 2
        level = float(np.median(mean[coverage].mean(axis=1)))
        scale = 1.0 / level if level > 0 else 1.0
        halves = [lightmap_denoise.denoise_charts(half[..., :3].astype(np.float32), coverage,
                                                  denoiser, input_scale=scale)[0]
                  for half in (first, second)]
        judged = statistics(halves[0], halves[1], coverage)
        record["denoised"] = dict(judged, input_scale=scale)
    record.update({"judged": "denoised" if denoiser is not None else "raw",
                   "relative_noise": judged["relative_noise"],
                   "required_samples": required_samples(samples, judged["relative_noise"],
                                                        target),
                   "status": "pass" if judged["relative_noise"] <= target else "fail"})
    return record


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--first", type=Path, required=True, help="half-sample bake, seed s")
    parser.add_argument("--second", type=Path, required=True, help="half-sample bake, seed s + 1")
    parser.add_argument("--coverage", type=Path, required=True)
    parser.add_argument("--samples", type=int, required=True,
                        help="samples of the mean (both halves together)")
    parser.add_argument("--target", type=float, required=True,
                        help="largest relative noise allowed at the percentile")
    parser.add_argument("--after-denoise", action="store_true",
                        help="gate on the halves' difference after per-chart denoising")
    parser.add_argument("--oidn-library", default="libOpenImageDenoise.so.2")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    import imageio.v3 as iio
    first, second = iio.imread(args.first), iio.imread(args.second)
    coverage = iio.imread(args.coverage)[..., :3].min(axis=2) > 0.5
    denoiser = None
    if args.after_denoise:
        import lightmap_denoise
        denoiser = lightmap_denoise.Denoiser(lightmap_denoise.load_oidn(args.oidn_library))
    try:
        result = measure(first, second, coverage, args.samples, args.target, denoiser)
    finally:
        if denoiser is not None:
            denoiser.close()
    result.update({"first_sha256": sha256(args.first), "second_sha256": sha256(args.second),
                   "coverage_sha256": sha256(args.coverage)})
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print("LIGHTMAP_NOISE " + json.dumps({k: result[k] for k in (
        "status", "judged", "relative_noise", "target", "samples", "required_samples")},
        sort_keys=True))
    if result["status"] != "pass":
        raise SystemExit("lightmap noise %.4g exceeds %.4g at %d samples; about %d samples "
                         "would meet it" % (result["relative_noise"], args.target, args.samples,
                                            result["required_samples"]))


if __name__ == "__main__":
    main()
