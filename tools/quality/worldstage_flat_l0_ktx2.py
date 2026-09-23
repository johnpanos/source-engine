#!/usr/bin/env python3
"""Package a checked Cycles World Stage flat bake as an HDR KTX2 L0 preview."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile

import imageio.v3 as iio
import numpy as np

from ktx2_probe import check_build, profile_at, run


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def package(args):
    profile = profile_at(args.profile)
    tool, revision, tool_version = check_build(profile, args.ktx_source, args.ktx_build)
    bake = json.loads(args.bake_evidence.read_text())
    manifest = json.loads(args.manifest.read_text())
    if (bake["status"] != "pass" or bake["renderer"] != "Blender Cycles bake preview" or
            bake["source_basis"] != "flat" or
            bake["exr_sha256"] != sha256(args.exr) or
            bake["stage_sha256"] != sha256(args.material_stage) or
            manifest["stage_sha256"] != sha256(args.geometry_stage) or
            set(bake["chart_max_linear"]) != set(manifest["lightmap_atlas"]["charts"])):
        raise ValueError("Cycles bake and World Stage atlas evidence differ")
    pixels = iio.imread(args.exr)
    if (pixels.dtype != np.float32 or pixels.shape !=
            (bake["height"], bake["width"], 3) or
            not np.isfinite(pixels).all() or pixels.min() < 0 or pixels.max() > 65504):
        raise ValueError("Cycles EXR is not a finite nonnegative half-float atlas")
    atlas = manifest["lightmap_atlas"]
    width, height = int(atlas["width"]), int(atlas["height"])
    if (width <= 0 or height <= 0 or pixels.shape[1] % width or
            pixels.shape[0] % height or
            pixels.shape[1] // width != pixels.shape[0] // height):
        raise ValueError("Cycles EXR does not match the World Stage chart grid")
    rgba = np.empty((pixels.shape[0], pixels.shape[1], 4), dtype="<f2")
    # USD chart V is bottom-first; the KTX2 reader requires top-left origin.
    # Flip the EXR scanlines so the shader's authored lightmap:st samples the
    # same chart without an extra per-material UV transform.
    rgba[:, :, :3] = pixels[::-1].astype("<f2")
    rgba[:, :, 3] = 1.0
    roundtrip = rgba[::-1, :, :3].astype(np.float32)
    quantization_error = float(np.max(np.abs(roundtrip - pixels)))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="worldstage-l0-", dir=args.out.parent) as directory:
        root = Path(directory)
        raw = root / "atlas.rgba16f"
        package_path = root / "atlas.ktx2"
        extracted = root / "extracted.rgba16f"
        raw.write_bytes(rgba.tobytes(order="C"))
        run([tool, "create", "--format", "R16G16B16A16_SFLOAT", "--raw",
             "--width", pixels.shape[1], "--height", pixels.shape[0],
             "--assign-tf", "linear", "--assign-texcoord-origin", "top-left",
             raw, package_path])
        run([tool, "validate", package_path])
        info = json.loads(run([tool, "info", "--format", "mini-json", package_path]))
        header = info["header"]
        if (not info["valid"] or header["vkFormat"] != "VK_FORMAT_R16G16B16A16_SFLOAT" or
                (header["pixelWidth"], header["pixelHeight"]) !=
                (pixels.shape[1], pixels.shape[0]) or header["levelCount"] != 1):
            raise ValueError("KTX2 atlas format, dimensions or mip topology differ")
        run([tool, "extract", "--raw", package_path, extracted])
        if extracted.read_bytes() != raw.read_bytes():
            raise ValueError("KTX2 extraction changed a half-float atlas pixel")
        encoded_hash = sha256(package_path)
        os.replace(package_path, args.out)
    result = {"status": "pass", "scope": "worldstage-cycles-flat-l0-ktx2-preview",
              "geometry_stage_sha256": sha256(args.geometry_stage),
              "material_stage_sha256": sha256(args.material_stage),
              "manifest_sha256": sha256(args.manifest),
              "cycles_bake_evidence_sha256": sha256(args.bake_evidence),
              "cycles_exr_sha256": sha256(args.exr),
              "ktx2_sha256": encoded_hash, "ktx2_bytes": args.out.stat().st_size,
              "ktx_software_revision": revision, "ktx_tool_version": tool_version,
              "format": header["vkFormat"], "orientation": "top-left (EXR rows flipped)",
              "width": pixels.shape[1], "height": pixels.shape[0],
              "stage_atlas_width": width, "stage_atlas_height": height,
              "chart_count": len(atlas["charts"]),
              "max_half_quantization_error": quantization_error,
              "exact_extracted_half_bytes": True}
    args.out.with_suffix(".json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("exr", "bake-evidence", "manifest", "geometry-stage", "material-stage",
                 "ktx-source", "ktx-build", "out"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--profile", type=Path, default=Path(
        "quality/product_profiles/ktx2-linux-tools.json"))
    args = parser.parse_args()
    if args.out.suffix != ".ktx2":
        parser.error("output must be a KTX2 file")
    print(json.dumps(package(args), sort_keys=True))


if __name__ == "__main__":
    main()
