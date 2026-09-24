#!/usr/bin/env python3
"""Make Source content (VTF/VMT + map) for a PBRT-derived BSP2 play test.

Material values come from the shared translation policy in `pbrt_scene`, so
the game preview, the Cycles stage and the lightmap bake agree. Layout:

    maps/<map>.bsp                        the BSP2 package
    materials/<map>/<material>.vmt        WMSH namespace (`PBR` preview shader;
                                          `PBRMetalRough` glass)
    materials/<map>_fallback/<material>.vmt   LightmappedGeneric/UnlitGeneric
    materials/<map>/<material>/basecolor.vtf, mrao.vtf

Base colors are encoded as sRGB (the PBR shader samples $basetexture with
sRGB read); MRAO stays linear. Transmissive (glass) materials are
`PBRMetalRough` with `$transmission`, `$ior` and `$thickness` from the same
policy, drawn two-sided; their fallback is an alpha-blended unlit preview.
Emitters use an unlit white preview, because emissive WMSH batches are not
implemented.
"""

import argparse
import hashlib
import json
import shutil
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbrt_scene  # noqa: E402
from vtf_content import compile_texture, power_of_two, verify_solid_vtf  # noqa: E402

MAX_TEXTURE = 2048
GLASS_PREVIEW_ALPHA = 0.13


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def srgb_byte(linear):
    linear = min(max(float(linear), 0.0), 1.0)
    encoded = 12.92 * linear if linear <= 0.0031308 else 1.055 * linear ** (1 / 2.4) - 0.055
    return round(encoded * 255)


def base_image(scene, summary):
    if summary["base_texture"]:
        source = Path(scene["source"]).parent / summary["base_texture"]
        with Image.open(source) as opened:
            image = opened.convert("RGB")
        size = tuple(min(power_of_two(value), MAX_TEXTURE) for value in image.size)
        if size != image.size:
            image = image.resize(size, Image.Resampling.LANCZOS)
        fdr = summary.get("coat_internal_reflectance")
        scale = summary.get("base_scale", 1.0)
        if fdr is not None or scale != 1.0:
            import numpy as np
            encoded = np.asarray(image, dtype=np.float64) / 255.0
            linear = np.where(encoded <= 0.04045, encoded / 12.92,
                              ((encoded + 0.055) / 1.055) ** 2.4) * scale
            if fdr is not None:
                linear = pbrt_scene.coated_albedo(linear, fdr)
            encoded = np.where(linear <= 0.0031308, 12.92 * linear,
                               1.055 * np.power(linear, 1 / 2.4) - 0.055)
            image = Image.fromarray(np.clip(np.rint(encoded * 255), 0, 255).astype(np.uint8))
        return image, None, sha256(source)
    color = tuple(srgb_byte(value) for value in summary["base_color"])
    return Image.new("RGB", (4, 4), color), color, None


def vmt(shader, lines):
    return '"%s"\n{\n%s}\n' % (shader, "".join('\t"%s" "%s"\n' % pair for pair in lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage-receipt", type=Path, required=True)
    parser.add_argument("--bsp2", type=Path, required=True)
    parser.add_argument("--vtex", type=Path, required=True)
    parser.add_argument("--map-name", required=True)
    parser.add_argument("--sky-texture", type=Path,
                        help="display texture for the SkyDome mesh (unlit, two-sided)")
    parser.add_argument("--out", type=Path, required=True,
                        help="content root; the receipt is written beside it as <out>.json")
    args = parser.parse_args()
    if args.out.exists():
        parser.error("output directory already exists: " + str(args.out))
    scene = pbrt_scene.parse(args.scene)
    receipt = json.loads(args.stage_receipt.read_text())
    if receipt.get("status") != "pass" or receipt.get("source_sha256") != scene["source_sha256"]:
        raise ValueError("stage receipt does not match the PBRT scene")
    if not args.vtex.is_file() or not args.bsp2.is_file():
        raise FileNotFoundError("VTEX or BSP2 package is missing")
    prefix = args.map_name
    material_root = args.out / "materials" / prefix
    fallback_root = args.out / "materials" / (prefix + "_fallback")
    map_path = args.out / "maps" / (args.map_name + ".bsp")
    map_path.parent.mkdir(parents=True)
    shutil.copy2(args.bsp2, map_path)
    fallback_root.mkdir(parents=True)
    used = sorted({shape["material"] for shape in scene["shapes"]})
    entries = [(name.lower(), pbrt_scene.material_summary(scene, name)) for name in used]
    if scene["emitters"]:
        entries.append(("emitter", {"name": "emitter", "base_texture": None,
                                    "base_color": (1.0, 1.0, 1.0), "metallic": 0.0,
                                    "roughness": 1.0, "transmission": 0.0,
                                    "pbrt_type": "emitter", "approximation": "unlit preview"}))
    if args.sky_texture:
        entries.append(("sky", {"name": "Sky", "base_texture": None, "base_color": None,
                                "metallic": 0.0, "roughness": 1.0, "transmission": 0.0,
                                "pbrt_type": "sky", "approximation": "display-mapped sky"}))
    assets = {}
    for name, summary in entries:
        directory = material_root / name
        directory.mkdir(parents=True)
        if summary["pbrt_type"] == "sky":
            with Image.open(args.sky_texture) as opened:
                image = opened.convert("RGB")
            solid, source_hash = None, sha256(args.sky_texture)
        else:
            image, solid, source_hash = base_image(scene, summary)
        base_hash = compile_texture(image, directory / "basecolor", args.vtex.resolve())
        if solid:
            verify_solid_vtf(directory / "basecolor.vtf", solid)
        mrao_rgb = (round(summary["metallic"] * 255), round(summary["roughness"] * 255), 255)
        mrao_hash = compile_texture(Image.new("RGB", (4, 4), mrao_rgb), directory / "mrao",
                                    args.vtex.resolve())
        verify_solid_vtf(directory / "mrao.vtf", mrao_rgb)
        texture = "%s/%s/basecolor" % (prefix, name)
        common = [("$surfaceprop", "default")]
        if summary["pbrt_type"] == "diffusetransmission":
            common.append(("$nocull", "1"))
        if summary["transmission"] > 0:
            preview = "pbr-glass"
            fallback = vmt("UnlitGeneric", [("$basetexture", texture), ("$translucent", "1"),
                                            ("$alpha", "%g" % GLASS_PREVIEW_ALPHA)] + common)
            world = vmt("PBRMetalRough", [
                ("$basetexture", texture), ("$mraotexture", "%s/%s/mrao" % (prefix, name)),
                ("$transmission", "%g" % summary["transmission"]),
                ("$ior", "%g" % summary["ior"]), ("$thickness", "0"), ("$nocull", "1"),
                ("$fallbackmaterial", "%s_fallback/%s" % (prefix, name))] + common)
        elif summary["pbrt_type"] == "sky":
            preview = "unlit-sky"
            fallback = vmt("UnlitGeneric", [("$basetexture", texture), ("$nocull", "1"),
                                            ("$nofog", "1")] + common)
            world = fallback
        elif summary["pbrt_type"] == "emitter":
            preview = "unlit"
            fallback = vmt("UnlitGeneric", [("$basetexture", texture), ("$nocull", "1")] + common)
            world = fallback
        else:
            preview = "pbr"
            fallback = vmt("LightmappedGeneric", [("$basetexture", texture)] + common)
            world = vmt("PBR", [("$basetexture", texture),
                                ("$mraotexture", "%s/%s/mrao" % (prefix, name)),
                                ("$fallbackmaterial", "%s_fallback/%s" % (prefix, name))] + common)
        (fallback_root / (name + ".vmt")).write_text(fallback)
        (material_root / (name + ".vmt")).write_text(world)
        assets[name] = {"pbrt_material": summary["name"], "pbrt_type": summary["pbrt_type"],
                        "preview": preview, "approximation": summary["approximation"],
                        "source_texture_sha256": source_hash,
                        "encoded_dimensions": list(image.size),
                        "basecolor_srgb": list(solid) if solid else None,
                        "mrao": list(mrao_rgb), "basecolor_vtf_sha256": base_hash,
                        "mrao_vtf_sha256": mrao_hash,
                        "vmt_sha256": sha256(material_root / (name + ".vmt")),
                        "fallback_vmt_sha256": sha256(fallback_root / (name + ".vmt"))}
    evidence = {"status": "pass", "scope": "pbrt-playable-content-preview",
                "map": args.map_name, "scene_sha256": scene["source_sha256"],
                "stage_receipt_sha256": sha256(args.stage_receipt),
                "bsp2_sha256": sha256(args.bsp2), "map_sha256": sha256(map_path),
                "material_namespace": prefix, "materials": assets,
                "texture_container": "VTF preview bridge; KTX2 runtime binding pending"}
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2,
                                                              sort_keys=True) + "\n")
    print(json.dumps({"status": "pass", "materials": len(assets), "map": str(map_path)}))


if __name__ == "__main__":
    main()
