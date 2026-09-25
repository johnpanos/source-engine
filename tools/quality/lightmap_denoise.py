#!/usr/bin/env python3
"""Denoise a baked irradiance atlas with OpenImageDenoise's RTLightmap filter.

A finishing step: the bake's sample count is chosen so its raw noise is
modest, and the noise step (lightmap_noise.py) gates on what remains after
this filter. Each chart is denoised alone at one fixed input scale
(denoise_charts), so no chart's light reaches another however close they
sit in the atlas. An undilated white-emission UV bake identifies real chart
coverage: Cycles can mark black gutter texels alpha=1, so irradiance alpha is
not a reliable mask. Empty atlas texels are filled from the nearest covered
texel before filtering.

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
# Border (texels) around each chart's crop in denoise_charts, filled from the
# chart itself so the filter's edge handling never touches its texels.
CHART_PAD = 16
# Width (texels) of the Gaussian that continues a chart's light into its
# border. A copy of the nearest edge texel repeats that texel's noise as a
# streak the filter keeps as detail; on testchmb_a_00 (256 samples) the
# edge texels' residual p99 was 94% with copies and 50% with this fill.
CHART_FILL_SIGMA = 4.0


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
    library.oidnSetFilterFloat.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_float]
    return library


class Denoiser:
    """One OIDN CPU device for many filter runs (charts)."""

    def __init__(self, library):
        self.library = library
        self.device = library.oidnNewDevice(OIDN_DEVICE_TYPE_CPU)
        if not self.device:
            raise RuntimeError("OIDN could not create a CPU device")
        library.oidnCommitDevice(self.device)

    def run(self, color, input_scale):
        """RTLightmap over an (H, W, 3) image at a fixed input scale: the HDR
        filter's automatic exposure would make each result depend on the whole
        image it sees."""
        library = self.library
        height, width, _ = color.shape
        source = np.ascontiguousarray(color, dtype=np.float32)
        output = np.empty_like(source)
        oidn_filter = library.oidnNewFilter(self.device, b"RTLightmap")
        try:
            for name, image in ((b"color", source), (b"output", output)):
                library.oidnSetSharedFilterImage(oidn_filter, name, image.ctypes.data,
                                                 OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0)
            library.oidnSetFilterFloat(oidn_filter, b"inputScale", float(input_scale))
            library.oidnCommitFilter(oidn_filter)
            library.oidnExecuteFilter(oidn_filter)
            message = ctypes.c_char_p()
            if library.oidnGetDeviceError(self.device, ctypes.byref(message)):
                raise RuntimeError("OIDN failed: " + (message.value or b"").decode())
        finally:
            library.oidnReleaseFilter(oidn_filter)
        return output

    def close(self):
        if self.device:
            self.library.oidnReleaseDevice(self.device)
            self.device = None


def denoise_charts(color, covered, denoiser, pad=CHART_PAD, input_scale=None):
    """Denoise each chart - a connected region of covered texels - alone.

    A chart is cropped with `pad` texels of border, filled with the chart's
    own light smoothed (chart_fill), so the filter sees only that chart: no
    neighbouring chart, however close in the atlas, reaches it. One input
    scale (1 / the covered median unless given) serves
    every chart. Covered texels get the result; others keep `color`.
    Returns (image, record)."""
    result = np.array(color, dtype=np.float32, copy=True)
    values = color[covered]
    if input_scale is None:
        level = float(np.median(values.mean(axis=1))) if len(values) else 0.0
        input_scale = 1.0 / level if level > 0 else 1.0
    labels, count = ndimage.label(covered, structure=np.ones((3, 3)))
    boxes = ndimage.find_objects(labels)
    height, width = covered.shape
    for index, box in enumerate(boxes, start=1):
        y0, y1 = max(box[0].start - pad, 0), min(box[0].stop + pad, height)
        x0, x1 = max(box[1].start - pad, 0), min(box[1].stop + pad, width)
        own = labels[y0:y1, x0:x1] == index
        crop = chart_fill(color[y0:y1, x0:x1], own)
        filtered = np.maximum(denoiser.run(crop, input_scale), 0.0)
        region = result[y0:y1, x0:x1]
        region[own] = filtered[own]
    return result, {"charts": int(count), "pad": pad, "input_scale": float(input_scale)}


def chart_fill(crop, own, sigma=CHART_FILL_SIGMA):
    """`crop` with every texel outside `own` replaced by the chart's light
    smoothed: a Gaussian of width `sigma` over chart texels only, normalized
    by the chart weight it reaches; texels it does not reach take their
    nearest reached value. Chart texels are unchanged."""
    weight = ndimage.gaussian_filter(own.astype(np.float64), sigma)
    smooth = np.stack([ndimage.gaussian_filter(np.where(own, crop[..., channel], 0.0), sigma)
                       for channel in range(crop.shape[2])], axis=-1)
    reached = weight > 1e-4
    smooth = smooth / np.where(reached, weight, 1.0)[..., None]
    _, (rows, columns) = ndimage.distance_transform_edt(~reached, return_indices=True)
    result = smooth[rows, columns].astype(np.float32)
    result[own] = crop[own]
    return result


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
    # OIIO stamps the write time (EXR capDate) unless DateTime is empty; the
    # file must be a function of its pixels.
    spec.attribute("DateTime", "")
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
    parser.add_argument("--layer",
                        help="a separated-light layer (direct, indirect) named in the bake "
                             "receipt's `layers`, rather than its total atlas")
    parser.add_argument("--skip-denoise", action="store_true",
                        help="fill UV gutters without filtering covered bake texels")
    args = parser.parse_args()
    evidence = json.loads(args.bake_evidence.read_text())
    expected = evidence.get("atlas_exr_sha256")
    if args.layer:
        expected = evidence.get("layers", {}).get(args.layer, {}).get("exr_sha256")
    if evidence.get("status") != "pass" or not expected or expected != sha256(args.exr):
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
    # A layer with no light at all (the indirect layer of a scene where no
    # surface sees another) has no noise to remove, and the denoiser does not
    # map zero to zero; it passes through unfiltered.
    unlit = not pixels[covered][:, :3].any()
    skip = args.skip_denoise or unlit
    chart_record = None
    if skip:
        filtered = filled
    else:
        denoiser = Denoiser(load_oidn(args.oidn_library))
        try:
            filtered, chart_record = denoise_charts(filled, covered, denoiser)
        finally:
            denoiser.close()
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
                    "source_atlas_exr_sha256": expected,
                    "layer": args.layer or "total",
                    "source_bake_evidence_sha256": sha256(args.bake_evidence),
                    "denoiser": None if skip else "OpenImageDenoise RTLightmap (CPU), per chart",
                    "denoise_charts": chart_record,
                    "unlit_layer": unlit,
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
