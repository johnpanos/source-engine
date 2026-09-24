#!/usr/bin/env python3
"""Fit a directional lightmap from Cycles RNM-basis bakes.

`pbrt_lightmap_bake.py --directional-dir` bakes diffuse irradiance for the
smooth normal N (the flat atlas) and for Source's three RNM basis normals
d_i = 0.8165 (cos phi_i T + sin phi_i B) + 0.5774 N (phi = 0, 120, 240
degrees) in each texel's lightmap tangent frame, plus that frame itself.
Irradiance near N is modelled as linear in the normal (an L1 irradiance
model):

    E(n) = a + g . n

The four measurements determine a and g exactly. The atlas stores it as a
colour-preserving luminance gradient relative to the flat irradiance:

    beta = g_luminance / E0_luminance        (world space, per texel)
    E_rgb(n) = E0_rgb * clamp(1 + beta . (n - N), 0, GAIN_MAX)

so an unperturbed normal reproduces the flat bake exactly and a normal-mapped
normal receives the baked light arriving from its direction. `world_pbr.frag`
evaluates it; `lightmap_ktx2.py --directional-exr` stores beta beside the
flat atlas (a 2:1 LMAP page). The RNM atlases are gutter-filled and denoised
with OpenImageDenoise like the flat atlas before fitting. The receipt records
the colour residual of the luminance model at the RNM directions and how many
texels were clamped.
"""

import argparse
import hashlib
import json
import sys
from pathlib import Path

import imageio.v3 as iio
import numpy as np
from scipy import ndimage

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lightmap_denoise  # noqa: E402

SCOPE = "cycles-rnm-l1-directional"
LUMINANCE = np.array((0.2126, 0.7152, 0.0722))
# |n - N| <= 2, so this bounds the gain at 1 + 2 * BETA_MAX.
BETA_MAX = 2.0
GAIN_MAX = 4.0
MIN_LUMINANCE = 1e-4
TILT_COS = 0.5773502691896258
TILT_SIN = 0.816496580927726
AZIMUTHS = np.radians((0.0, 120.0, 240.0))


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def read_rgba(path):
    pixels = iio.imread(path).astype(np.float32)
    if pixels.ndim != 3 or pixels.shape[2] != 4 or not np.isfinite(pixels).all():
        raise ValueError("bake EXR must be finite RGBA: " + str(path))
    return pixels


def decode_unit(pixels):
    vectors = pixels[..., :3].astype(np.float64) * 2.0 - 1.0
    length = np.linalg.norm(vectors, axis=2, keepdims=True)
    return np.divide(vectors, length, out=np.zeros_like(vectors), where=length > 1e-6)


def fill_and_denoise(pixels, covered, library):
    _, (rows, columns) = ndimage.distance_transform_edt(~covered, return_indices=True)
    filled = pixels[rows, columns, :3]
    filtered = filled if library is None else np.maximum(
        lightmap_denoise.denoise(library, filled), 0.0)
    return lightmap_denoise.extend_gutters(filtered, covered, rows, columns)


def fit(flat, rnm, tangent, normal):
    """(beta, gain residual) per texel. Arrays are (H, W, 3)."""
    tangent = tangent - normal * np.sum(tangent * normal, axis=2, keepdims=True)
    tangent /= np.maximum(np.linalg.norm(tangent, axis=2, keepdims=True), 1e-12)
    bitangent = np.cross(normal, tangent)
    flat_l = flat @ LUMINANCE
    rnm_l = [values @ LUMINANCE for values in rnm]
    average = sum(rnm_l) / 3.0
    g_n = (flat_l - average) / (1.0 - TILT_COS)
    g_t = (2.0 / 3.0) / TILT_SIN * sum(v * np.cos(phi) for v, phi in zip(rnm_l, AZIMUTHS))
    g_b = (2.0 / 3.0) / TILT_SIN * sum(v * np.sin(phi) for v, phi in zip(rnm_l, AZIMUTHS))
    gradient = (g_t[..., None] * tangent + g_b[..., None] * bitangent +
                g_n[..., None] * normal)
    valid = flat_l > MIN_LUMINANCE
    beta = np.where(valid[..., None], gradient / np.maximum(flat_l, MIN_LUMINANCE)[..., None],
                    0.0)
    magnitude = np.linalg.norm(beta, axis=2)
    clamped = magnitude > BETA_MAX
    beta = np.where(clamped[..., None], beta * (BETA_MAX / np.maximum(magnitude, 1e-12))[..., None],
                    beta)
    # Colour residual of E0 * gain at the measured directions.
    residuals = []
    for phi, measured in zip(AZIMUTHS, rnm):
        direction = (TILT_SIN * (np.cos(phi) * tangent + np.sin(phi) * bitangent) +
                     TILT_COS * normal)
        gain = np.clip(1.0 + np.sum(beta * (direction - normal), axis=2), 0.0, GAIN_MAX)
        predicted = flat * gain[..., None]
        residuals.append(np.abs(predicted - measured).sum(axis=2) /
                         np.maximum(measured.sum(axis=2), 1e-6))
    return beta, np.mean(residuals, axis=0), clamped, valid


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--flat-exr", type=Path, required=True,
                        help="denoised (or gutter-filled) flat atlas")
    parser.add_argument("--flat-evidence", type=Path, required=True,
                        help="its passing denoise receipt")
    parser.add_argument("--bake-evidence", type=Path, required=True)
    parser.add_argument("--directional-dir", type=Path, required=True)
    parser.add_argument("--coverage-exr", type=Path, required=True)
    parser.add_argument("--skip-denoise", action="store_true")
    parser.add_argument("--oidn-library", default="libOpenImageDenoise.so.2")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    bake = json.loads(args.bake_evidence.read_text())
    flat_receipt = json.loads(args.flat_evidence.read_text())
    directional = bake.get("directional")
    if bake.get("status") != "pass" or not directional:
        raise ValueError("bake receipt has no directional bakes")
    if (flat_receipt.get("status") != "pass" or
            flat_receipt.get("atlas_exr_sha256") != sha256(args.flat_exr) or
            flat_receipt.get("source_atlas_exr_sha256") != bake.get("atlas_exr_sha256")):
        raise ValueError("flat atlas differs from its denoise receipt or bake")
    if sha256(args.coverage_exr) != bake.get("coverage_exr_sha256"):
        raise ValueError("UV coverage differs from the bake receipt")
    rnm_paths = [args.directional_dir / ("rnm%d.exr" % i) for i in range(3)]
    frame_paths = {axis: args.directional_dir / ("frame_%s.exr" % axis) for axis in ("t", "n")}
    if [sha256(path) for path in rnm_paths] != directional["rnm_exr_sha256"] or \
            {axis: sha256(path) for axis, path in frame_paths.items()} != \
            directional["frame_exr_sha256"]:
        raise ValueError("directional bakes differ from the bake receipt")
    flat = read_rgba(args.flat_exr)
    coverage = read_rgba(args.coverage_exr)
    covered = lightmap_denoise.uv_coverage_mask(coverage, flat)
    library = None if args.skip_denoise else lightmap_denoise.load_oidn(args.oidn_library)
    rnm = [fill_and_denoise(read_rgba(path), covered, library).astype(np.float64)
           for path in rnm_paths]
    tangent = decode_unit(read_rgba(frame_paths["t"]))
    normal = decode_unit(read_rgba(frame_paths["n"]))
    frame_ok = covered & (np.linalg.norm(normal, axis=2) > 0.5) & \
        (np.linalg.norm(tangent, axis=2) > 0.5)
    beta, residual, clamped, valid = fit(flat[..., :3].astype(np.float64), rnm, tangent, normal)
    usable = frame_ok & valid
    if not usable.any():
        raise ValueError("no covered texel has a usable frame and irradiance")
    beta[~usable] = 0.0
    # Linear filtering near chart edges reads gutter texels: give them the
    # nearest usable texel's gradient.
    _, (rows, columns) = ndimage.distance_transform_edt(~usable, return_indices=True)
    beta = beta[rows, columns]
    output = np.zeros(flat.shape, dtype=np.float32)
    output[..., :3] = beta
    output[..., 3] = 1.0
    args.out.parent.mkdir(parents=True, exist_ok=True)
    lightmap_denoise.write_linear_exr(args.out, output)
    magnitude = np.linalg.norm(beta[usable], axis=1)
    receipt = {"status": "pass", "scope": SCOPE, "model": "E0_rgb * clamp(1 + beta.(n - N), 0, "
               "%g), beta = luminance gradient / E0 luminance, world space" % GAIN_MAX,
               "beta_max": BETA_MAX, "gain_max": GAIN_MAX,
               "directional_exr_sha256": sha256(args.out),
               "flat_exr_sha256": sha256(args.flat_exr),
               "flat_evidence_sha256": sha256(args.flat_evidence),
               "bake_evidence_sha256": sha256(args.bake_evidence),
               "size": bake["size"], "reserved_rows": bake.get("reserved_rows", 0),
               "denoiser": None if args.skip_denoise else "OpenImageDenoise RTLightmap (CPU)",
               "covered_texels": int(covered.sum()), "fitted_texels": int(usable.sum()),
               "frame_rejected_texels": int((covered & ~frame_ok).sum()),
               "clamped_fraction": float(clamped[usable].mean()),
               "beta_magnitude": {"mean": float(magnitude.mean()),
                                  "p95": float(np.percentile(magnitude, 95))},
               "rnm_colour_residual": {"mean": float(residual[usable].mean()),
                                       "p95": float(np.percentile(residual[usable], 95))}}
    args.out.with_name(args.out.name + ".json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: receipt[k] for k in ("status", "fitted_texels", "clamped_fraction",
                                              "beta_magnitude", "rnm_colour_residual")}))


if __name__ == "__main__":
    main()
