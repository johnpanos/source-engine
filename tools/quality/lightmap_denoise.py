#!/usr/bin/env python3
"""Denoise a baked irradiance atlas with OpenImageDenoise's RTLightmap filter.

Path-traced bakes of sky-lit interiors are dominated by noisy indirect light.
OIDN's lightmap filter removes that noise without a higher sample count. Empty
atlas texels are first filled from the nearest chart texel so the filter never
pulls black gutters into chart edges; coverage (alpha) is restored afterwards.

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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exr", type=Path, required=True)
    parser.add_argument("--bake-evidence", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--oidn-library", default="libOpenImageDenoise.so.2")
    args = parser.parse_args()
    evidence = json.loads(args.bake_evidence.read_text())
    if evidence.get("status") != "pass" or evidence.get("atlas_exr_sha256") != sha256(args.exr):
        raise ValueError("atlas differs from its passing bake receipt")
    pixels = iio.imread(args.exr).astype(np.float32)
    if pixels.ndim != 3 or pixels.shape[2] != 4 or not np.isfinite(pixels).all():
        raise ValueError("bake atlas must be finite RGBA")
    covered = pixels[:, :, 3] > 0
    if not covered.any():
        raise ValueError("bake atlas has no covered texels")
    # Fill gutters from the nearest covered texel before filtering.
    _, (rows, columns) = ndimage.distance_transform_edt(~covered, return_indices=True)
    filled = pixels[rows, columns, :3]
    result = pixels.copy()
    result[:, :, :3] = np.where(covered[:, :, None],
                                denoise(load_oidn(args.oidn_library), filled), 0.0)
    result[:, :, :3] = np.maximum(result[:, :, :3], 0.0)
    if not np.isfinite(result).all():
        raise ValueError("OIDN produced non-finite texels")
    before = pixels[covered][:, :3]
    after = result[covered][:, :3]
    # High-frequency energy: mean absolute difference from a 3x3 box blur.
    def roughness(image):
        return float(np.abs(image - ndimage.uniform_filter(image, size=(3, 3, 1)))[covered].mean())
    args.out.parent.mkdir(parents=True, exist_ok=True)
    iio.imwrite(args.out, result)
    receipt = {key: evidence[key] for key in ("size", "lighting_stage_sha256", "scene_sha256",
                                               "source_stage_sha256") if key in evidence}
    receipt.update({"status": "pass", "scope": evidence["scope"] + "-denoised",
                    "atlas_exr_sha256": sha256(args.out),
                    "source_atlas_exr_sha256": evidence["atlas_exr_sha256"],
                    "source_bake_evidence_sha256": sha256(args.bake_evidence),
                    "denoiser": "OpenImageDenoise RTLightmap (CPU)",
                    "covered_texels": int(covered.sum()),
                    "mean_rgb_before": before.mean(axis=0).tolist(),
                    "mean_rgb_after": after.mean(axis=0).tolist(),
                    "high_frequency_before": roughness(pixels[:, :, :3]),
                    "high_frequency_after": roughness(result[:, :, :3])})
    # Denoising must remove noise without changing overall light level.
    drift = np.abs(after.mean(axis=0) - before.mean(axis=0)) / np.maximum(before.mean(axis=0), 1e-6)
    if (drift > 0.05).any():
        raise ValueError("denoising changed mean irradiance by more than 5%%: %s" % drift)
    args.out.with_name(args.out.name + ".json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: receipt[k] for k in ("status", "high_frequency_before",
                                              "high_frequency_after")}))


if __name__ == "__main__":
    main()
