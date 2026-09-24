#!/usr/bin/env python3
"""Denoise a baked irradiance atlas with OpenImageDenoise's RTLightmap filter.

Path-traced bakes of sky-lit interiors are dominated by noisy indirect light.
OIDN's lightmap filter removes that noise without a higher sample count. An
undilated white-emission UV bake identifies real chart coverage: Cycles can mark
black gutter texels alpha=1, so irradiance alpha is not a reliable mask. Empty
atlas texels are filled from the nearest covered texel before filtering.

The input receipt must be a passing bake receipt for the EXR. The output
receipt copies its identity fields, records the source atlas hash and uses
`<input scope>-denoised` as scope. OIDN is loaded from the system library
(`libOpenImageDenoise.so.2`) with ctypes; `--oidn-library` overrides it.
"""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path

import imageio.v3 as iio
import numpy as np
import OpenImageIO as oiio
from scipy import ndimage

OIDN_DEVICE_TYPE_CPU = 1
OIDN_FORMAT_FLOAT3 = 3


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load_oidn(path):
    library = ctypes.CDLL(path)
    library.oidnNewDevice.restype = ctypes.c_void_p
    library.oidnNewDevice.argtypes = [ctypes.c_int]
    library.oidnCommitDevice.argtypes = [ctypes.c_void_p]
    library.oidnNewFilter.restype = ctypes.c_void_p
    library.oidnNewFilter.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    library.oidnSetSharedFilterImage.argtypes = [
        ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_int, ctypes.c_size_t,
        ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t]
    library.oidnCommitFilter.argtypes = [ctypes.c_void_p]
    library.oidnExecuteFilter.argtypes = [ctypes.c_void_p]
    library.oidnGetDeviceError.restype = ctypes.c_int
    library.oidnGetDeviceError.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p)]
    library.oidnReleaseFilter.argtypes = [ctypes.c_void_p]
    library.oidnReleaseDevice.argtypes = [ctypes.c_void_p]
    return library


def denoise(library, color):
    height, width, _ = color.shape
    source = np.ascontiguousarray(color, dtype=np.float32)
    output = np.empty_like(source)
    device = library.oidnNewDevice(OIDN_DEVICE_TYPE_CPU)
    if not device:
        raise RuntimeError("OIDN could not create a CPU device")
    try:
        library.oidnCommitDevice(device)
        oidn_filter = library.oidnNewFilter(device, b"RTLightmap")
        try:
            for name, image in ((b"color", source), (b"output", output)):
                library.oidnSetSharedFilterImage(oidn_filter, name, image.ctypes.data,
                                                 OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0)
            library.oidnCommitFilter(oidn_filter)
            library.oidnExecuteFilter(oidn_filter)
            message = ctypes.c_char_p()
            if library.oidnGetDeviceError(device, ctypes.byref(message)):
                raise RuntimeError("OIDN failed: " + (message.value or b"").decode())
        finally:
            library.oidnReleaseFilter(oidn_filter)
    finally:
        library.oidnReleaseDevice(device)
    return output


def extend_gutters(color, covered, rows, columns):
    """Give linear sampling a nearest-chart color outside authored coverage."""
    result = color.copy()
    result[~covered] = color[rows[~covered], columns[~covered]]
    return result


def uv_coverage_mask(coverage, pixels):
    if (coverage.shape != pixels.shape or not np.isfinite(coverage).all() or
            np.max(coverage[:, :, :3]) < 0.9):
        raise ValueError("UV coverage bake is invalid")
    return np.min(coverage[:, :, :3], axis=2) > 0.5


def write_linear_exr(path, pixels):
    """Write named RGBA channels and verify that HDR and alpha survive."""
    height, width, channels = pixels.shape
    if channels != 4 or pixels.dtype != np.float32:
        raise ValueError("linear EXR requires float32 RGBA pixels")
    spec = oiio.ImageSpec(width, height, 4, oiio.FLOAT)
    spec.channelnames = ["R", "G", "B", "A"]
    spec.alpha_channel = 3
    output = oiio.ImageOutput.create(str(path))
    if not output or not output.open(str(path), spec):
        raise OSError("could not open denoised EXR for writing")
    try:
        if not output.write_image(pixels):
            raise OSError("could not write denoised EXR: " + output.geterror())
    finally:
        if not output.close():
            raise OSError("could not close denoised EXR: " + output.geterror())
    saved = iio.imread(path)
    if saved.shape != pixels.shape or not np.array_equal(saved, pixels):
        raise ValueError("denoised EXR readback changed RGB or alpha channels")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exr", type=Path, required=True)
    parser.add_argument("--coverage-exr", type=Path,
                        help="undilated white-emission bake named by the bake receipt")
    parser.add_argument("--bake-evidence", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--oidn-library", default="libOpenImageDenoise.so.2")
    parser.add_argument("--skip-denoise", action="store_true",
                        help="fill UV gutters without filtering covered bake texels")
    args = parser.parse_args()
    evidence = json.loads(args.bake_evidence.read_text())
    if evidence.get("status") != "pass" or evidence.get("atlas_exr_sha256") != sha256(args.exr):
        raise ValueError("atlas differs from its passing bake receipt")
    pixels = iio.imread(args.exr).astype(np.float32)
    if pixels.ndim != 3 or pixels.shape[2] != 4 or not np.isfinite(pixels).all():
        raise ValueError("bake atlas must be finite RGBA")
    if evidence.get("coverage_exr_sha256"):
        if (not args.coverage_exr or
                sha256(args.coverage_exr) != evidence["coverage_exr_sha256"]):
            raise ValueError("UV coverage differs from the passing bake receipt")
        coverage = iio.imread(args.coverage_exr).astype(np.float32)
        covered = uv_coverage_mask(coverage, pixels)
    else:
        if args.coverage_exr:
            raise ValueError("bake receipt does not identify a UV coverage atlas")
        covered = pixels[:, :, 3] > 0
    if not covered.any():
        raise ValueError("bake atlas has no covered texels")
    # Fill gutters from the nearest covered texel before filtering.
    _, (rows, columns) = ndimage.distance_transform_edt(~covered, return_indices=True)
    filled = pixels[rows, columns, :3]
    result = pixels.copy()
    filtered = (filled if args.skip_denoise else
                np.maximum(denoise(load_oidn(args.oidn_library), filled), 0.0))
    result[:, :, :3] = extend_gutters(filtered, covered, rows, columns)
    if not np.isfinite(result).all():
        raise ValueError("lightmap processing produced non-finite texels")
    before = pixels[covered][:, :3]
    after = result[covered][:, :3]
    # High-frequency energy: mean absolute difference from a 3x3 box blur.
    def roughness(image):
        return float(np.abs(image - ndimage.uniform_filter(image, size=(3, 3, 1)))[covered].mean())
    args.out.parent.mkdir(parents=True, exist_ok=True)
    write_linear_exr(args.out, result)
    receipt = {key: evidence[key] for key in ("size", "lighting_stage_sha256", "scene_sha256",
                                               "source_stage_sha256", "reserved_rows",
                                               "uv_extents") if key in evidence}
    suffix = "-gutter-filled" if args.skip_denoise else "-denoised"
    receipt.update({"status": "pass", "scope": evidence["scope"] + suffix,
                    "atlas_exr_sha256": sha256(args.out),
                    "source_atlas_exr_sha256": evidence["atlas_exr_sha256"],
                    "source_bake_evidence_sha256": sha256(args.bake_evidence),
                    "denoiser": None if args.skip_denoise else "OpenImageDenoise RTLightmap (CPU)",
                    "covered_texels": int(covered.sum()),
                    "filled_gutter_texels": int((~covered).sum()),
                    "coverage_exr_sha256": evidence.get("coverage_exr_sha256"),
                    "mean_rgb_before": before.mean(axis=0).tolist(),
                    "mean_rgb_after": after.mean(axis=0).tolist(),
                    "high_frequency_before": roughness(pixels[:, :, :3]),
                    "high_frequency_after": roughness(result[:, :, :3])})
    # Denoising must remove noise without changing overall light level.
    drift = np.abs(after.mean(axis=0) - before.mean(axis=0)) / np.maximum(before.mean(axis=0), 1e-6)
    if (drift > 0.05).any():
        raise ValueError("lightmap processing changed mean irradiance by more than 5%%: %s" % drift)
    args.out.with_name(args.out.name + ".json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: receipt[k] for k in ("status", "high_frequency_before",
                                              "high_frequency_after")}))


if __name__ == "__main__":
    main()
