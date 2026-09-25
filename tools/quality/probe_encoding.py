#!/usr/bin/env python3
"""RFC 0011 open decision 1: the probe irradiance encoding, by analytic error
and memory.

    python3 tools/quality/probe_encoding.py --out quality-results/rfc0011-decisions/encoding.json

For analytic lighting environments, each probe's diffuse light (irradiance /
pi, the lightmap's unit) is encoded three ways and reconstructed at 2048
normals against the exact integral:

  octahedral-rgba16f     the PRBV encoding (probe_volume.py): 6 x 6 interior
                         texels plus the 1-texel border, each the exact diffuse
                         light at its texel direction (what probe_volume_bake.py
                         bakes: a receiver per texel), half floats, bilinear
  octahedral-b10g11r11   the same texels in B10G11R11_UFLOAT (5 or 6 mantissa
                         bits, rounded to nearest)
  sh-l2                  the radiance projected on 9 real SH coefficients per
                         channel and convolved with the clamped cosine
                         (Ramamoorthi and Hanrahan, "An Efficient
                         Representation for Irradiance Environment Maps",
                         SIGGRAPH 2001), half floats

The error is the mean and the largest absolute error over the normals,
relative to the environment's mean diffuse light (the G1.4 oracles'
normalization: percent of a representative level). Memory is bytes per probe
for irradiance; the Chebyshev visibility PRBV needs (16 x 16 RG16F, 1 KB)
is the same for every candidate: SH L2 carries none (RFC 0011 "Probe volume
contract").

Environments: `sky` (an overcast upper hemisphere over a darker ground),
`sun` (the sky plus a 0.5-degree disk 35 degrees up, 1e4 times as bright),
`window` (a bright 40 x 30 degree patch at the horizon in a dim room) and
`bleed` (red on one side, blue on the other, a grey ceiling).
"""

import argparse
import json
import math
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import probe_volume  # noqa: E402

INTERIOR = probe_volume.IRRADIANCE_TILE - 2
NORMALS = 2048
SAMPLES = 400000


def fibonacci(count):
    i = np.arange(count) + 0.5
    z = 1.0 - 2.0 * i / count
    phi = math.pi * (1.0 + math.sqrt(5.0)) * i
    r = np.sqrt(np.maximum(0.0, 1.0 - z * z))
    return np.stack((r * np.cos(phi), r * np.sin(phi), z), axis=1)


SUN = np.array((math.cos(math.radians(35.0)), 0.0, math.sin(math.radians(35.0))))
SUN_COS = math.cos(math.radians(0.5))


def radiance(name, directions):
    """(N, 3) linear radiance of an environment along unit directions."""
    x, y, z = directions[:, 0], directions[:, 1], directions[:, 2]
    if name == "sky":
        value = np.where(z > 0, 1.0, 0.2)
        return np.repeat(value[:, None], 3, axis=1) * (0.9, 0.95, 1.0)
    if name == "sun":
        sky = radiance("sky", directions) * 0.1
        disk = (directions @ SUN) > SUN_COS
        return sky + disk[:, None] * np.array((1.0, 0.95, 0.85)) * 1.0e4
    if name == "window":
        azimuth = np.degrees(np.arctan2(y, x))
        elevation = np.degrees(np.arcsin(np.clip(z, -1, 1)))
        patch = (np.abs(azimuth) < 20.0) & (np.abs(elevation) < 15.0)
        return np.where(patch[:, None], 3.0, 0.05) * np.ones((1, 3))
    if name == "bleed":
        red = np.where(x > 0, 1.0, 0.0)[:, None] * (0.8, 0.1, 0.1)
        blue = np.where(x <= 0, 1.0, 0.0)[:, None] * (0.1, 0.1, 0.8)
        ceiling = np.where(z > 0.7, 1.0, 0.0)[:, None] * (0.5, 0.5, 0.5)
        return red + blue + ceiling
    raise ValueError(name)


def exact_diffuse(name, normals, samples):
    """Irradiance / pi at each normal, by uniform sphere quadrature (the sun
    disk analytically: its solid angle times its radiance and cosine)."""
    directions = fibonacci(samples)
    weight = 4.0 * math.pi / samples
    values = radiance(name if name != "sun" else "sky", directions)
    if name == "sun":
        values = values * 0.1
    cosine = np.maximum(normals @ directions.T, 0.0)
    result = cosine @ values * weight / math.pi
    if name == "sun":
        solid = 2.0 * math.pi * (1.0 - SUN_COS)
        result += (np.maximum(normals @ SUN, 0.0)[:, None] * np.array((1.0, 0.95, 0.85)) *
                   1.0e4 * solid / math.pi)
    return result


def octahedral(name, normals, samples, quantize=None):
    texels = exact_diffuse(name, probe_volume.interior_directions(INTERIOR).reshape(-1, 3),
                           samples).reshape(INTERIOR, INTERIOR, 3)
    texels = quantize(texels) if quantize else texels.astype(np.float16).astype(np.float64)
    tile = probe_volume.with_border(texels)
    return np.array([probe_volume.tile_sample(tile, (0, 0), INTERIOR + 2, n) for n in normals])


def small_float(values, mantissa_bits):
    """Unsigned small float (5 exponent bits, bias 15), round to nearest."""
    values = np.maximum(values, 0.0)
    exponent = np.floor(np.log2(np.maximum(values, 2.0 ** -14)))
    step = 2.0 ** (exponent - mantissa_bits)
    return np.minimum(np.round(values / step) * step, 65000.0)


def b10g11r11(texels):
    return np.stack((small_float(texels[..., 0], 6), small_float(texels[..., 1], 6),
                     small_float(texels[..., 2], 5)), axis=-1)


def sh_basis(d):
    x, y, z = d[:, 0], d[:, 1], d[:, 2]
    return np.stack((0.282095 * np.ones_like(x), 0.488603 * y, 0.488603 * z, 0.488603 * x,
                     1.092548 * x * y, 1.092548 * y * z, 0.315392 * (3 * z * z - 1),
                     1.092548 * x * z, 0.546274 * (x * x - y * y)), axis=1)


BAND = np.array((math.pi,) + (2.0 * math.pi / 3.0,) * 3 + (math.pi / 4.0,) * 5)


def sh_l2(name, normals, samples):
    directions = fibonacci(samples)
    values = radiance(name, directions)
    coefficients = sh_basis(directions).T @ values * (4.0 * math.pi / samples)
    coefficients = coefficients.astype(np.float16).astype(np.float64)
    return sh_basis(normals) @ (coefficients * BAND[:, None]) / math.pi


def errors(estimate, exact):
    level = float(exact.mean())
    difference = np.abs(estimate - exact)
    return {"mean_relative": float(difference.mean() / level),
            "max_relative": float(difference.max() / level),
            "negative_normals": int(np.sum(np.any(estimate < 0, axis=1)))}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--samples", type=int, default=SAMPLES)
    args = parser.parse_args()
    normals = fibonacci(NORMALS)
    tile = probe_volume.IRRADIANCE_TILE
    encodings = {"octahedral-rgba16f": {"bytes_per_probe": tile * tile * 8},
                 "octahedral-b10g11r11": {"bytes_per_probe": tile * tile * 4},
                 "sh-l2": {"bytes_per_probe": 9 * 3 * 2}}
    results = {}
    for name in ("sky", "sun", "window", "bleed"):
        exact = exact_diffuse(name, normals, args.samples)
        results[name] = {
            "octahedral-rgba16f": errors(octahedral(name, normals, args.samples), exact),
            "octahedral-b10g11r11": errors(octahedral(name, normals, args.samples, b10g11r11),
                                           exact),
            "sh-l2": errors(sh_l2(name, normals, args.samples), exact)}
        print(name, {k: "%.2f%% mean, %.1f%% max" % (100 * v["mean_relative"],
                                                    100 * v["max_relative"])
                     for k, v in results[name].items()})
    record = {"schema": "rfc0011-probe-encoding/v1", "normals": NORMALS,
              "quadrature_samples": args.samples, "interior": INTERIOR,
              "visibility_bytes_per_probe": probe_volume.VISIBILITY_TILE ** 2 * 4,
              "encodings": encodings, "environments": results}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
