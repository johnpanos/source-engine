#!/usr/bin/env python3
"""Make a private sealed-room variant with a point light and Portal materials."""

import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "quality/fixtures/vbsp-host"
OLD_ORIGIN = '"origin" "0 0 224"'
NEW_ORIGIN = '"origin" "-128 -128 224"'
PORTAL_MATERIALS = {
    "DEV/DEV_MEASUREWALL01A": "portal_pbr/tile/observation_tilefloor001a",
    "DEV/DEV_MEASUREGENERIC01B": "portal_pbr/metal/metalwall_bts_006b",
}
FALLBACK_BASE_TEXTURES = {
    "portal_pbr/tile/observation_tilefloor001a": "tile/observation_tilefloor001a",
    "portal_pbr/metal/metalwall_bts_006b": "metal/metalwall_bts_006b",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--portal-assets", type=Path,
                        help="portal-pbr-remastered root; copies the authored VMTs")
    parser.add_argument("--vtex", type=Path,
                        help="installed VTEX; converts private PNG copies to playable VTFs")
    args = parser.parse_args()
    if args.vtex and not args.portal_assets:
        parser.error("--vtex requires --portal-assets")
    if args.portal_assets and not args.vtex:
        parser.error("--portal-assets requires --vtex for compilable texture dimensions")
    output = args.out_dir.resolve()
    if output.exists():
        parser.error("output directory already exists: " + str(output))
    source = BASE / "sealed_room.vmf"
    vmf = source.read_text(encoding="utf-8")
    if vmf.count(OLD_ORIGIN) != 1 or NEW_ORIGIN in vmf:
        raise ValueError("sealed-room point-light origin changed; review the variant")
    output.mkdir(parents=True)
    shutil.copytree(BASE / "game", output / "game")
    generated = vmf.replace(OLD_ORIGIN, NEW_ORIGIN)
    copied_materials = {}
    converted_textures = {}
    if args.portal_assets:
        asset_root = args.portal_assets.resolve()
        material_root = asset_root / "materials"
        for old, new in PORTAL_MATERIALS.items():
            if generated.count('"material" "' + old + '"') == 0:
                raise ValueError("fixture material changed: " + old)
            generated = generated.replace('"material" "' + old + '"',
                                          '"material" "' + new + '"')
            source_vmt = material_root / (new + ".vmt")
            if not source_vmt.is_file():
                raise FileNotFoundError(source_vmt)
            target_vmt = output / "game/materials" / (new + ".vmt")
            target_vmt.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_vmt, target_vmt)
            copied_materials[new] = hashlib.sha256(source_vmt.read_bytes()).hexdigest()
            fallback = FALLBACK_BASE_TEXTURES[new]
            source_fallback = material_root / "portal_pbr_fallback" / (new.removeprefix(
                "portal_pbr/") + ".vmt")
            if not source_fallback.is_file():
                raise FileNotFoundError(source_fallback)
            target_fallback = output / "game/materials/portal_pbr_fallback" / (
                new.removeprefix("portal_pbr/") + ".vmt")
            target_fallback.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_fallback, target_fallback)
            copied_materials["portal_pbr_fallback/" + new.removeprefix(
                "portal_pbr/")] = hashlib.sha256(source_fallback.read_bytes()).hexdigest()
            if args.vtex:
                texture_dir = asset_root / "textures" / new
                for channel in ("basecolor", "mrao", "normal"):
                    source_png = texture_dir / (channel + ".png")
                    if not source_png.is_file():
                        if channel == "normal":
                            continue
                        raise FileNotFoundError(source_png)
                    private_png = output / "texture-src" / new / (channel + ".png")
                    private_png.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(source_png, private_png)
                    subprocess.run([str(args.vtex.resolve()), str(private_png)],
                                   check=True, capture_output=True, text=True, timeout=120)
                    private_vtf = private_png.with_suffix(".vtf")
                    if not private_vtf.is_file():
                        raise RuntimeError("VTEX did not create " + str(private_vtf))
                    target_vtf = output / "game/materials" / new / (channel + ".vtf")
                    target_vtf.parent.mkdir(parents=True, exist_ok=True)
                    shutil.move(private_vtf, target_vtf)
                    converted_textures[new + "/" + channel] = {
                        "png_sha256": hashlib.sha256(source_png.read_bytes()).hexdigest(),
                        "vtf_sha256": hashlib.sha256(target_vtf.read_bytes()).hexdigest(),
                    }
                    if channel == "basecolor":
                        fallback_vtf = output / "game/materials" / (fallback + ".vtf")
                        fallback_vtf.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(target_vtf, fallback_vtf)
    (output / "sealed_room.vmf").write_text(generated, encoding="utf-8")
    evidence = {
        "base_vmf_sha256": hashlib.sha256(vmf.encode()).hexdigest(),
        "variant_vmf_sha256": hashlib.sha256(generated.encode()).hexdigest(),
        "replacement": {"from": OLD_ORIGIN, "to": NEW_ORIGIN},
        "portal_material_vmt_sha256": copied_materials,
        "converted_portal_textures": converted_textures,
    }
    (output / "variant.json").write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
