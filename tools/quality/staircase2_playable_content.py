#!/usr/bin/env python3
"""Make private Source material assets for the staircase2 BSP2 play test.

The VTF/VMT files bridge today's native renderer. The originating USD/PBRT
texture hashes and conversion dimensions are retained for later KTX2 rollout.
"""

import argparse
import hashlib
import json
import re
import shutil
import subprocess
from pathlib import Path

from PIL import Image

from staircase2_materials import linear_to_srgb_byte, solid_linear_base_colors


MATERIALS = {
    "wall": (None, None, 0, 255),
    "wood": (None, "wood5.tga", 0, 89),
    "floortiles": (None, "Tiles.tga", 0, 38),
    "wallpaper": (None, "Wallpaper.tga", 0, 255),
    "spotholder": ((255, 255, 255), None, 0, 255),
    "metal": (None, None, 255, 26),
    "chrome": (None, None, 255, 26),
    "glass": ((215, 230, 230), None, 0, 13),
    "emitter": ((255, 255, 255), None, 0, 255),
}
EXPECTED_PBRT_MATERIALS = {
    "Wall", "Wood", "FloorTiles", "Wallpaper", "SpotHolder", "Metal", "Chrome", "Glass"
}
UNLIT_PREVIEW_MATERIALS = {"glass", "spotholder", "emitter"}


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def power_of_two(value):
    return 1 << (value - 1).bit_length()


def compile_texture(image, destination, vtex):
    destination.parent.mkdir(parents=True, exist_ok=True)
    image.save(destination.with_suffix(".png"))
    subprocess.run([str(vtex), str(destination.with_suffix(".png"))],
                   check=True, capture_output=True, text=True, timeout=120)
    encoded = destination.with_suffix(".vtf")
    if not encoded.is_file():
        raise RuntimeError("VTEX did not produce " + str(encoded))
    destination.with_suffix(".png").unlink()
    return sha256(encoded)


def verify_solid_vtf(path, rgb):
    """Check the compiled 4x4 preview texels against authored RGB channels."""
    payload = path.read_bytes()
    if (len(payload) < 48 or payload[:4] != b"VTF\0" or
            int.from_bytes(payload[16:18], "little") != 4 or
            int.from_bytes(payload[18:20], "little") != 4 or
            int.from_bytes(payload[52:56], "little") != 3 or
            payload[-48:] != bytes((rgb[2], rgb[1], rgb[0])) * 16):
        raise ValueError("VTEX changed solid material channels: " + str(path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--render-evidence", type=Path, required=True)
    parser.add_argument("--bsp2", type=Path, required=True)
    parser.add_argument("--vtex", type=Path, required=True)
    parser.add_argument("--world-pbr-prefix",
                        help="optional WMSH PBR material namespace")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if args.world_pbr_prefix and (
            not re.fullmatch(r"[a-z0-9_]+(?:/[a-z0-9_]+)*", args.world_pbr_prefix) or
            args.world_pbr_prefix in {"staircase2", "staircase2_fallback"}):
        parser.error("WMSH PBR prefix must be a distinct normalized relative path")
    if args.out.exists():
        parser.error("output directory already exists: " + str(args.out))
    receipt = json.loads(args.render_evidence.read_text())
    solid_colors = solid_linear_base_colors(args.scene.read_text())
    if (receipt.get("status") != "pass" or
            receipt.get("source_sha256") != sha256(args.scene) or
            receipt.get("stage_sha256") != sha256(args.stage) or
            receipt.get("solid_linear_base_colors") !=
            {name: list(values) for name, values in solid_colors.items()} or
            set(receipt.get("material_assignments", {}).values()) !=
            EXPECTED_PBRT_MATERIALS):
        raise ValueError("staircase2 material source does not match its USD render")
    if not args.vtex.is_file() or not args.bsp2.is_file():
        raise FileNotFoundError("VTEX or BSP2 package is missing")
    material_root = args.out / "materials/staircase2"
    fallback_root = args.out / "materials/staircase2_fallback"
    pbr_root = (args.out / "materials" / args.world_pbr_prefix
                if args.world_pbr_prefix else None)
    map_path = args.out / "maps/staircase2_playable.bsp"
    map_path.parent.mkdir(parents=True)
    shutil.copy2(args.bsp2, map_path)
    assets = {}
    for name, (color, image_name, metal, roughness) in sorted(MATERIALS.items()):
        if name in solid_colors:
            color = tuple(linear_to_srgb_byte(value) for value in solid_colors[name])
        directory = material_root / name
        directory.mkdir(parents=True)
        if image_name:
            source = args.scene.parent / "textures" / image_name
            if receipt["textures"].get("textures/" + image_name) != sha256(source):
                raise ValueError("source texture differs from Cycles receipt: " + image_name)
            with Image.open(source) as opened:
                base = opened.convert("RGB")
                dimensions = (power_of_two(base.width), power_of_two(base.height))
                if dimensions != base.size:
                    base = base.resize(dimensions, Image.Resampling.LANCZOS)
            source_hash = sha256(source)
        else:
            base = Image.new("RGB", (4, 4), color)
            dimensions = base.size
            source_hash = None
        base_hash = compile_texture(base, directory / "basecolor", args.vtex.resolve())
        if color is not None:
            verify_solid_vtf(directory / "basecolor.vtf", color)
        mrao = Image.new("RGB", (4, 4), (metal, roughness, 255))
        mrao_hash = compile_texture(mrao, directory / "mrao", args.vtex.resolve())
        verify_solid_vtf(directory / "mrao.vtf", (metal, roughness, 255))
        fallback_root.mkdir(parents=True, exist_ok=True)
        fallback_shader = ("UnlitGeneric" if name in UNLIT_PREVIEW_MATERIALS
                           else "LightmappedGeneric")
        glass_alpha = '\t"$translucent" "1"\n\t"$alpha" "0.13"\n' if name == "glass" else ""
        emitter_nocull = '\t"$nocull" "1"\n' if name == "emitter" else ""
        fallback_vmt = (
            f'"{fallback_shader}"\n{{\n'
            f'\t"$basetexture" "staircase2/{name}/basecolor"\n'
            f'{glass_alpha}'
            f'{emitter_nocull}'
            f'\t"$surfaceprop" "tile"\n}}\n')
        (fallback_root / (name + ".vmt")).write_text(fallback_vmt)
        vmt = (fallback_vmt if name in UNLIT_PREVIEW_MATERIALS else
               f'"PBRMetalRough"\n{{\n'
               f'\t"$basetexture" "staircase2/{name}/basecolor"\n'
               f'\t"$mraotexture" "staircase2/{name}/mrao"\n'
               f'\t"$fallbackmaterial" "staircase2_fallback/{name}"\n'
               f'\t"$surfaceprop" "tile"\n}}\n')
        (material_root / (name + ".vmt")).write_text(vmt)
        pbr_vmt_hash = None
        if pbr_root:
            pbr_root.mkdir(parents=True, exist_ok=True)
            if name in UNLIT_PREVIEW_MATERIALS:
                preview_vmt = fallback_vmt
            else:
                preview_vmt = (f'"PBR"\n{{\n'
                               f'\t"$basetexture" "staircase2/{name}/basecolor"\n'
                               f'\t"$mraotexture" "staircase2/{name}/mrao"\n'
                               f'\t"$fallbackmaterial" "staircase2_fallback/{name}"\n'
                               f'\t"$surfaceprop" "tile"\n}}\n')
            pbr_vmt = pbr_root / (name + ".vmt")
            pbr_vmt.write_text(preview_vmt)
            pbr_vmt_hash = sha256(pbr_vmt)
        assets[name] = {"source_texture_sha256": source_hash,
                        "encoded_dimensions": dimensions,
                        "basecolor_vtf_sha256": base_hash,
                        "mrao_vtf_sha256": mrao_hash,
                        "vmt_sha256": sha256(material_root / (name + ".vmt")),
                        "fallback_vmt_sha256": sha256(fallback_root / (name + ".vmt"))}
        if pbr_vmt_hash:
            assets[name]["world_pbr_vmt_sha256"] = pbr_vmt_hash
    evidence = {"status": "pass", "scope": "staircase2-playable-content-preview",
                "scene_sha256": sha256(args.scene), "stage_sha256": sha256(args.stage),
                "render_evidence_sha256": sha256(args.render_evidence),
                "bsp2_sha256": sha256(args.bsp2), "map_sha256": sha256(map_path),
                "world_pbr_prefix": args.world_pbr_prefix,
                "solid_linear_base_colors": solid_colors,
                "materials": assets,
                "transmission_limit": "glass uses alpha blending; refraction is pending",
                "texture_container": "VTF preview bridge; KTX2 runtime binding pending"}
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2,
                                                       sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
