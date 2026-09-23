#!/usr/bin/env python3
"""Put Cycles World Stage bakes into one legacy light style's luxels.

This is an inspection bridge for a private fixture, not the RFC 0007 baker or
the RFC 0008 LMAP writer. Other styles stay as VRAD produced them. The source
BSP must already have been lit by VRAD.
"""

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

import imageio.v3 as iio
import numpy as np

from bsp2_reader import open_any
from legacy_lighting_audit import (FACE_BYTES, SURF_BUMPLIGHT, TEXINFO_BYTES,
                                   audit_lighting_lumps)


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def rgbexp32(linear):
    """Encode linear light in Source's RGBExp32 lightmap units."""
    peak = float(max(linear))
    if not math.isfinite(peak) or min(linear) < 0:
        raise ValueError("bake has invalid linear lighting")
    if peak == 0:
        return b"\0\0\0\0"
    exponent = math.ceil(math.log2(peak))
    if not -127 <= exponent <= 127:
        raise ValueError("bake lighting exceeds RGBExp32 range")
    scale = 255.0 / (2.0 ** exponent)
    return bytes(min(255, int(float(channel) * scale)) for channel in linear) + bytes(
        [exponent & 255])


def patch_bsp(source, manifest, pixels, style, gain, basis_pixels=None,
              update_style_average=False):
    kind, header = open_any(source)
    if kind != "legacy" or header["version"] != 21:
        raise ValueError("preview requires a legacy v21 VRAD BSP")
    if pixels.ndim != 3 or pixels.shape[2] < 3 or pixels.dtype != np.float32:
        raise ValueError("preview requires a float32 RGB EXR")
    if basis_pixels is not None and (set(basis_pixels) != {"rnm0", "rnm1", "rnm2"} or
                                     any(image.shape != pixels.shape or image.dtype != np.float32
                                         for image in basis_pixels.values())):
        raise ValueError("RNM preview requires three matching float32 EXRs")
    layout = manifest["lightmap_atlas"]
    width, height = layout["width"], layout["height"]
    if pixels.shape[1] % width or pixels.shape[0] % height:
        raise ValueError("EXR size is not an integer multiple of Stage atlas")
    scale_x, scale_y = pixels.shape[1] // width, pixels.shape[0] // height
    if scale_x != scale_y or scale_x < 1:
        raise ValueError("preview requires equal integral chart scale")

    def lump(index):
        offset, size, *_ = header["lumps"][index]
        return source[offset:offset + size]

    faces, texinfos, lighting = lump(7), lump(6), lump(8)
    audit_lighting_lumps(faces, texinfos, lighting)
    if lump(53) or lump(58):
        raise ValueError("preview supports LDR face and lighting lumps only")
    if len(layout["charts"]) != len(faces) // FACE_BYTES:
        raise ValueError("Stage chart count differs from BSP faces")
    updated = bytearray(source)
    lighting_offset = header["lumps"][8][0]
    changed = 0
    bumped = 0
    bumped_samples = 0
    for index in range(len(faces) // FACE_BYTES):
        face = faces[index * FACE_BYTES:(index + 1) * FACE_BYTES]
        texinfo = struct.unpack_from("<h", face, 10)[0]
        flags = struct.unpack_from("<i", texinfos, texinfo * TEXINFO_BYTES + 64)[0]
        styles = face[16:20]
        if style not in styles:
            raise ValueError(f"face {index} lacks requested light style {style}")
        style_index = styles.index(style)
        face_offset = struct.unpack_from("<i", face, 20)[0]
        chart_w, chart_h = struct.unpack_from("<ii", face, 36)
        chart_w += 1
        chart_h += 1
        x, y, chart_width, chart_height = layout["charts"][str(index)]
        if (chart_width, chart_height) != (chart_w, chart_h):
            raise ValueError(f"face {index} chart differs from Stage")
        basis_count = 4 if flags & SURF_BUMPLIGHT else 1
        bumped += basis_count == 4
        first = lighting_offset + face_offset + 4 * style_index * basis_count * chart_w * chart_h
        flat_samples = []
        for row in range(chart_h):
            # OpenEXR readers return top-first scanlines; USD charts use bottom-first V.
            top = pixels.shape[0] - (y + row + 1) * scale_y
            for column in range(chart_w):
                left = (x + column) * scale_x
                block = pixels[top:top + scale_y, left:left + scale_x, :3]
                if block.shape != (scale_y, scale_x, 3):
                    raise ValueError(f"face {index} chart escapes EXR")
                sample = block.mean(axis=(0, 1), dtype=np.float64) * gain
                if not np.isfinite(sample).all():
                    raise ValueError(f"face {index} has non-finite light")
                offset = first + 4 * (row * chart_w + column)
                updated[offset:offset + 4] = rgbexp32(sample)
                flat_samples.append(sample)
                changed += 1
                if basis_pixels is not None and basis_count == 4:
                    for basis_index, name in enumerate(("rnm0", "rnm1", "rnm2"), start=1):
                        basis_block = basis_pixels[name][top:top + scale_y,
                                                         left:left + scale_x, :3]
                        basis_sample = basis_block.mean(axis=(0, 1), dtype=np.float64) * gain
                        basis_offset = offset + 4 * basis_index * chart_w * chart_h
                        updated[basis_offset:basis_offset + 4] = rgbexp32(basis_sample)
                        bumped_samples += 1
        if basis_pixels is not None or update_style_average:
            style_count = styles.index(255) if 255 in styles else 4
            average = np.median(flat_samples, axis=0)
            average_offset = lighting_offset + face_offset - 4 * style_count + 4 * style_index
            updated[average_offset:average_offset + 4] = rgbexp32(average)
    audit_lighting_lumps(lump(7), lump(6), bytes(
        updated[lighting_offset:lighting_offset + len(lighting)]))
    return bytes(updated), {"faces": len(faces) // FACE_BYTES, "bumped_faces": bumped,
                            "style": style, "gain": gain, "flat_luxels": changed,
                            "rnm_luxels": bumped_samples,
                            "preserved_bump_bases": basis_pixels is None,
                            "updated_style_average": basis_pixels is not None or
                            update_style_average,
                            "preserved_other_styles": True}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--geometry-stage", type=Path, required=True)
    parser.add_argument("--material-stage", type=Path, required=True)
    parser.add_argument("--geometry-comparison", type=Path, required=True,
                        help="independent BSP/Stage comparison with compiled light styles")
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--exr", type=Path, required=True)
    for basis in ("rnm0", "rnm1", "rnm2"):
        parser.add_argument("--" + basis, type=Path)
    parser.add_argument("--directional-comparison", type=Path,
                        help="required with RNM EXRs; checks bases against compiled BSP")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--style", type=int,
                        help="select a style from the geometry comparison; inferred if unique")
    parser.add_argument("--gain", type=float, default=1.0)
    parser.add_argument("--update-style-average", action="store_true",
                        help="update the selected style's average while retaining VRAD RNM")
    args = parser.parse_args()
    if ((args.style is not None and not 0 <= args.style < 255) or
            not math.isfinite(args.gain) or args.gain <= 0):
        parser.error("style must be 0..254 and gain finite and positive")
    if args.out.resolve() == args.bsp.resolve():
        parser.error("output must differ from source BSP")
    directional_paths = {name: getattr(args, name) for name in ("rnm0", "rnm1", "rnm2")}
    directional = any(directional_paths.values())
    if directional and (not all(directional_paths.values()) or not args.directional_comparison):
        parser.error("RNM preview requires all three EXRs and --directional-comparison")
    if args.directional_comparison and not directional:
        parser.error("--directional-comparison requires all three RNM EXRs")
    manifest = json.loads(args.manifest.read_text())
    geometry_hash = sha256(args.geometry_stage)
    if manifest["stage_sha256"] != geometry_hash:
        raise ValueError("Portal manifest does not match World Stage geometry")
    comparison = json.loads(args.geometry_comparison.read_text())
    if (comparison["status"] != "pass" or
            comparison["bsp_sha256"] != sha256(args.bsp) or
            comparison["stage_sha256"] != geometry_hash):
        raise ValueError("independent geometry comparison does not match BSP and Stage")
    styles = set(comparison["light_styles"].values())
    if not styles or any(type(style) is not int or not 0 <= style < 255 for style in styles):
        raise ValueError("comparison has no valid compiled light styles")
    if args.style is None:
        if len(styles) != 1:
            raise ValueError("multiple compiled light styles require --style")
        style = next(iter(styles))
    elif args.style not in styles:
        raise ValueError("requested style is absent from the compiled World Stage")
    else:
        style = args.style
    material_text = args.material_stage.read_text()
    if 'sourceGeometrySha256 = "' + geometry_hash + '"' not in material_text:
        raise ValueError("material stage does not bind the World Stage geometry")
    bake_evidence = json.loads(args.exr.with_suffix(".json").read_text())
    if (bake_evidence["status"] != "pass" or
            bake_evidence["stage_sha256"] != sha256(args.material_stage) or
            bake_evidence["exr_sha256"] != sha256(args.exr)):
        raise ValueError("Cycles bake evidence does not match the EXR and material stage")
    chart_maxima = bake_evidence.get("chart_max_linear", {})
    if (set(chart_maxima) != set(manifest["lightmap_atlas"]["charts"]) or
            any(value <= 1e-5 for value in chart_maxima.values())):
        raise ValueError("Cycles bake did not light every required Stage chart")
    if directional:
        checked = json.loads(args.directional_comparison.read_text())
        expected_hashes = {"flat": sha256(args.exr), **{name: sha256(path)
                                                         for name, path in directional_paths.items()}}
        if (checked["status"] != "pass" or
                checked["scope"] != "cycles-rnm-preview-directions" or
                checked["bsp_sha256"] != sha256(args.bsp) or
                checked["stage_sha256"] != sha256(args.material_stage) or
                checked["manifest_sha256"] != sha256(args.manifest) or
                checked["exr_sha256"] != expected_hashes or
                not checked["negative_wrong_direction_rejected"]):
            raise ValueError("RNM bake comparison does not match BSP, Stage, and EXRs")
    pixels = iio.imread(args.exr)
    basis_pixels = {name: iio.imread(path) for name, path in directional_paths.items()
                    } if directional else None
    result, counts = patch_bsp(args.bsp.read_bytes(), manifest, pixels,
                               style, args.gain, basis_pixels,
                               args.update_style_average)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(result)
    evidence = {"status": "pass", "scope": "legacy-rnm-style-preview" if directional else
                "legacy-flat-style-preview",
                "source_bsp_sha256": sha256(args.bsp),
                "geometry_stage_sha256": geometry_hash,
                "geometry_comparison_sha256": sha256(args.geometry_comparison),
                "material_stage_sha256": sha256(args.material_stage),
                "portal_manifest_sha256": sha256(args.manifest),
                "cycles_exr_sha256": sha256(args.exr),
                "directional_comparison_sha256": sha256(args.directional_comparison) if
                directional else None,
                "output_bsp_sha256": sha256(args.out), **counts}
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2,
                                                        sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
