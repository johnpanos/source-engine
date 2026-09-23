#!/usr/bin/env python3
"""Make private Source material assets for the staircase2 BSP2 play test.

The VTF/VMT files bridge today's native renderer. The originating USD/PBRT
texture hashes and conversion dimensions are retained for later KTX2 rollout.
"""

import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path

from PIL import Image


MATERIALS = {
    "wall": ((228, 228, 228), None, 0, 255),
    "wood": (None, "wood5.tga", 0, 89),
    "floortiles": (None, "Tiles.tga", 0, 38),
    "wallpaper": (None, "Wallpaper.tga", 0, 255),
    "spotholder": ((255, 255, 255), None, 0, 255),
    "metal": ((238, 215, 172), None, 255, 26),
    "chrome": ((243, 233, 218), None, 255, 26),
    "glass": ((215, 230, 230), None, 0, 13),
}
EXPECTED_PBRT_MATERIALS = {
    "Wall", "Wood", "FloorTiles", "Wallpaper", "SpotHolder", "Metal", "Chrome", "Glass"
}


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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--render-evidence", type=Path, required=True)
    parser.add_argument("--bsp2", type=Path, required=True)
    parser.add_argument("--vtex", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if args.out.exists():
        parser.error("output directory already exists: " + str(args.out))
    receipt = json.loads(args.render_evidence.read_text())
    if (receipt.get("status") != "pass" or
            receipt.get("source_sha256") != sha256(args.scene) or
            receipt.get("stage_sha256") != sha256(args.stage) or
            set(receipt.get("material_assignments", {}).values()) !=
            EXPECTED_PBRT_MATERIALS):
        raise ValueError("staircase2 material source does not match its USD render")
    if not args.vtex.is_file() or not args.bsp2.is_file():
        raise FileNotFoundError("VTEX or BSP2 package is missing")
    material_root = args.out / "materials/staircase2"
    fallback_root = args.out / "materials/staircase2_fallback"
    map_path = args.out / "maps/staircase2_playable.bsp"
    map_path.parent.mkdir(parents=True)
    shutil.copy2(args.bsp2, map_path)
    assets = {}
    for name, (color, image_name, metal, roughness) in sorted(MATERIALS.items()):
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
        mrao = Image.new("RGB", (4, 4), (metal, roughness, 255))
        mrao_hash = compile_texture(mrao, directory / "mrao", args.vtex.resolve())
        vmt = (f'"PBRMetalRough"\n{{\n'
               f'\t"$basetexture" "staircase2/{name}/basecolor"\n'
               f'\t"$mraotexture" "staircase2/{name}/mrao"\n'
               f'\t"$fallbackmaterial" "staircase2_fallback/{name}"\n'
               f'\t"$surfaceprop" "tile"\n}}\n')
        (material_root / (name + ".vmt")).write_text(vmt)
        fallback_root.mkdir(parents=True, exist_ok=True)
        (fallback_root / (name + ".vmt")).write_text(
            '"LightmappedGeneric"\n{\n'
            f'\t"$basetexture" "staircase2/{name}/basecolor"\n'
            f'\t"$surfaceprop" "tile"\n}}\n')
        assets[name] = {"source_texture_sha256": source_hash,
                        "encoded_dimensions": dimensions,
                        "basecolor_vtf_sha256": base_hash,
                        "mrao_vtf_sha256": mrao_hash,
                        "vmt_sha256": sha256(material_root / (name + ".vmt")),
                        "fallback_vmt_sha256": sha256(fallback_root / (name + ".vmt"))}
    evidence = {"status": "pass", "scope": "staircase2-playable-content-preview",
                "scene_sha256": sha256(args.scene), "stage_sha256": sha256(args.stage),
                "render_evidence_sha256": sha256(args.render_evidence),
                "bsp2_sha256": sha256(args.bsp2), "map_sha256": sha256(map_path),
                "materials": assets,
                "transmission_limit": "glass is opaque until the native transmission path exists",
                "texture_container": "VTF preview bridge; KTX2 runtime binding pending"}
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2,
                                                       sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
