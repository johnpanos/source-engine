#!/usr/bin/env python3
"""Make Source content (VTF/VMT + map) for a scene-derived BSP2 play test.

Material values come from the shared translation policy (`map_scene`: PBRT
scenes and extracted USD scenes), so the game preview, the Cycles stage and
the lightmap bake agree. Layout:

    maps/<map>.bsp                        the BSP2 package
    materials/<map>/<material>.vmt        WMSH namespace (`PBRMetalRough`)
    materials/<map>_fallback/<material>.vmt   LightmappedGeneric/UnlitGeneric
    materials/<map>/<material>/basecolor.vtf, mrao.vtf[, normal.vtf, emission.vtf]

Every authored channel is carried: base colour textures (with UsdUVTexture
scale/bias), metallic/roughness/occlusion textures packed per texel into MRAO,
tangent-space normal maps ($bumpmap, OpenGL +Y), and cut-out opacity as base
alpha with $alphatest, and emission as an sRGB $emissiontexture times a linear
$emissionscale (HDR emission above 1 survives). Base colors are encoded as sRGB
(the shader samples $basetexture with sRGB read); MRAO and normals stay
linear. The receipt lists, per material, which channels were exported and
which were approximated. Transmissive (glass) materials are
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
import map_scene  # noqa: E402
from vtf_content import compile_texture, power_of_two, verify_solid_vtf  # noqa: E402

MAX_TEXTURE = 2048
GLASS_PREVIEW_ALPHA = 0.13


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def srgb_byte(linear):
    linear = min(max(float(linear), 0.0), 1.0)
    encoded = 12.92 * linear if linear <= 0.0031308 else 1.055 * linear ** (1 / 2.4) - 0.055
    return round(encoded * 255)


def srgb_decode(encoded):
    import numpy as np
    return np.where(encoded <= 0.04045, encoded / 12.92, ((encoded + 0.055) / 1.055) ** 2.4)


def srgb_encode(linear):
    import numpy as np
    linear = np.clip(linear, 0.0, 1.0)
    return np.where(linear <= 0.0031308, 12.92 * linear, 1.055 * np.power(linear, 1 / 2.4) - 0.055)


def target_size(size):
    return tuple(min(power_of_two(value), MAX_TEXTURE) for value in size)


def read_texture(root, texture, color):
    """A texture record as linear float RGBA (H, W, 4) after scale and bias.

    `colorspace` sRGB decodes; raw keeps values; auto decodes 8-bit colour
    inputs (UsdUVTexture's documented default) and keeps data inputs linear.
    """
    import numpy as np
    path = root / texture["file"]
    if path.suffix.lower() in (".exr", ".hdr"):
        import imageio.v3 as iio
        pixels = np.asarray(iio.imread(path), dtype=np.float64)
        encoded = False
    else:
        with Image.open(path) as opened:
            mode = opened.mode
            if mode in ("I;16", "I;16B", "I"):
                pixels = np.asarray(opened, dtype=np.float64) / 65535.0
            else:
                pixels = np.asarray(opened.convert("RGBA"), dtype=np.float64) / 255.0
        encoded = True
    if pixels.ndim == 2:
        pixels = np.dstack([pixels] * 3 + [np.ones_like(pixels)])
    if pixels.shape[2] == 3:
        pixels = np.dstack((pixels, np.ones(pixels.shape[:2])))
    space = texture.get("colorspace", "auto")
    if encoded and (space == "sRGB" or (space == "auto" and color)):
        pixels[..., :3] = srgb_decode(pixels[..., :3])
    scale = np.asarray(texture.get("scale", [1.0] * 4), dtype=np.float64)
    bias = np.asarray(texture.get("bias", [0.0] * 4), dtype=np.float64)
    return pixels * scale + bias, sha256(path)


def resized(pixels, size):
    """Resize float pixels channel by channel (Lanczos) to `size` (w, h)."""
    import numpy as np
    if (pixels.shape[1], pixels.shape[0]) == size:
        return pixels
    return np.dstack([np.asarray(Image.fromarray(pixels[..., c].astype(np.float32), "F").resize(
        size, Image.Resampling.LANCZOS)) for c in range(pixels.shape[2])])


def channel_values(pixels, channel):
    return pixels[..., :3] if channel == "rgb" else pixels[..., "rgba".index(channel)]


def base_image(scene, summary):
    """sRGB base colour (+ cut-out alpha); (image, solid rgb or None, hashes)."""
    import numpy as np
    root = map_scene.material_root(scene)
    textures = summary["textures"]
    cutout = summary["opacity_threshold"] > 0 and "opacity" in textures
    hashes = {}
    if summary["base_texture"]:
        record = textures.get("base") or {"file": summary["base_texture"], "channel": "rgb",
                                          "colorspace": "sRGB"}
        pixels, hashes["base"] = read_texture(root, record, color=True)
        linear = channel_values(pixels, "rgb") if record.get("channel", "rgb") == "rgb" else \
            np.repeat(channel_values(pixels, record["channel"])[..., None], 3, axis=2)
        size = target_size((linear.shape[1], linear.shape[0]))
        linear = resized(linear, size) * summary.get("base_scale", 1.0)
        fdr = summary.get("coat_internal_reflectance")
        if fdr is not None:
            linear = map_scene.coated_albedo(linear, fdr)
        solid = None
    else:
        linear = None
        solid = tuple(srgb_byte(value) for value in summary["base_color"])
        size = (4, 4)
    alpha = None
    if cutout:
        record = textures["opacity"]
        pixels, hashes["opacity"] = read_texture(root, record, color=False)
        opacity = channel_values(pixels, record.get("channel", "a"))
        if linear is None:
            size = target_size((opacity.shape[1], opacity.shape[0]))
        alpha = np.clip(resized(opacity[..., None], size)[..., 0], 0.0, 1.0)
    if linear is None:
        if alpha is None:
            return Image.new("RGB", size, solid), solid, hashes
        linear = np.broadcast_to(srgb_decode(np.asarray(solid) / 255.0), size[::-1] + (3,))
    encoded = np.clip(np.rint(srgb_encode(linear) * 255), 0, 255).astype(np.uint8)
    if alpha is not None:
        encoded = np.dstack((encoded, np.rint(alpha * 255).astype(np.uint8)))
    return Image.fromarray(encoded), (solid if alpha is None else None), hashes


def mrao_image(scene, summary):
    """Metal/roughness/occlusion per texel from textures or constants."""
    import numpy as np
    root = map_scene.material_root(scene)
    textures = summary["textures"]
    planes, hashes, size = {}, {}, None
    for channel in ("metallic", "roughness", "occlusion"):
        if channel in textures:
            pixels, hashes[channel] = read_texture(root, textures[channel], color=False)
            planes[channel] = channel_values(pixels, textures[channel].get("channel", "r"))
            size = size or target_size((planes[channel].shape[1], planes[channel].shape[0]))
    constants = {"metallic": summary["metallic"], "roughness": summary["roughness"],
                 "occlusion": 1.0}
    if not planes:
        rgb = tuple(round(min(max(constants[c], 0.0), 1.0) * 255)
                    for c in ("metallic", "roughness", "occlusion"))
        return Image.new("RGB", (4, 4), rgb), rgb, hashes
    stacked = []
    for channel in ("metallic", "roughness", "occlusion"):
        if channel in planes:
            stacked.append(resized(planes[channel][..., None], size)[..., 0])
        else:
            stacked.append(np.full(size[::-1], constants[channel]))
    pixels = np.clip(np.rint(np.dstack(stacked) * 255), 0, 255).astype(np.uint8)
    return Image.fromarray(pixels), None, hashes


def normal_image(scene, summary):
    """Tangent-space normal texel xyz in [0, 1] (UsdPreviewSurface +Y = +v)."""
    import numpy as np
    record = summary["textures"].get("normal")
    if not record:
        return None, None
    pixels, digest = read_texture(map_scene.material_root(scene), record, color=False)
    normal = pixels[..., :3]
    normal = normal / np.maximum(np.linalg.norm(normal, axis=2, keepdims=True), 1e-8)
    normal[..., 2] = np.maximum(normal[..., 2], 0.0)
    size = target_size((normal.shape[1], normal.shape[0]))
    normal = resized(normal, size)
    normal /= np.maximum(np.linalg.norm(normal, axis=2, keepdims=True), 1e-8)
    encoded = np.clip(np.rint((normal * 0.5 + 0.5) * 255), 0, 255).astype(np.uint8)
    return Image.fromarray(encoded), digest


def emission_linear(scene, summary):
    """Linear emission radiance (H, W, 3) or None."""
    import numpy as np
    record = summary["textures"].get("emission")
    if record:
        pixels, _ = read_texture(map_scene.material_root(scene), record, color=True)
        return channel_values(pixels, "rgb") if record.get("channel", "rgb") == "rgb" else \
            np.repeat(channel_values(pixels, record["channel"])[..., None], 3, axis=2)
    if summary["emission_color"] and max(summary["emission_color"]) > 0:
        return np.broadcast_to(np.asarray(summary["emission_color"], dtype=np.float64),
                               (4, 4, 3))
    return None


def emission_texture(emission):
    """(linear scale, sRGB image) with emission = decode(image) * scale."""
    import numpy as np
    if emission is None:
        return 1.0, None
    peak = float(np.max(emission))
    if peak <= 0:
        return 1.0, None
    scale = max(1.0, peak)
    size = target_size((emission.shape[1], emission.shape[0]))
    encoded = srgb_encode(resized(np.clip(emission / scale, 0.0, 1.0), size))
    return scale, Image.fromarray(np.clip(np.rint(encoded * 255), 0, 255).astype(np.uint8))


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
    scene = map_scene.parse(args.scene)
    receipt = json.loads(args.stage_receipt.read_text())
    if receipt.get("status") != "pass" or receipt.get("source_sha256") != scene["source_sha256"]:
        raise ValueError("stage receipt does not match the scene")
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
    entries = [(name.lower(), map_scene.material_summary(scene, name)) for name in used]
    # USD `doubleSided` meshes draw both faces.
    double_sided = {shape["material"].lower() for shape in scene["shapes"]
                    if shape.get("double_sided")}
    if scene["emitters"]:
        entries.append(("emitter", dict(map_scene.CHANNEL_DEFAULTS, **{
            "name": "emitter", "base_texture": None, "base_color": (1.0, 1.0, 1.0),
            "metallic": 0.0, "roughness": 1.0, "transmission": 0.0, "pbrt_type": "emitter",
            "approximation": "unlit preview"})))
    if args.sky_texture:
        entries.append(("sky", dict(map_scene.CHANNEL_DEFAULTS, **{
            "name": "Sky", "base_texture": None, "base_color": None, "metallic": 0.0,
            "roughness": 1.0, "transmission": 0.0, "pbrt_type": "sky",
            "approximation": "display-mapped sky"})))
    assets = {}
    for name, summary in entries:
        directory = material_root / name
        directory.mkdir(parents=True)
        emission = emission_linear(scene, summary)
        emission_scale, emission_image = emission_texture(emission)
        if summary["pbrt_type"] == "sky":
            with Image.open(args.sky_texture) as opened:
                image = opened.convert("RGB")
            solid, source_hashes = None, {"sky": sha256(args.sky_texture)}
        else:
            image, solid, source_hashes = base_image(scene, summary)
        base_hash = compile_texture(image, directory / "basecolor", args.vtex.resolve())
        if solid:
            verify_solid_vtf(directory / "basecolor.vtf", solid)
        mrao, mrao_rgb, mrao_hashes = mrao_image(scene, summary)
        mrao_hash = compile_texture(mrao, directory / "mrao", args.vtex.resolve())
        if mrao_rgb:
            verify_solid_vtf(directory / "mrao.vtf", mrao_rgb)
        normal, normal_source = normal_image(scene, summary)
        normal_hash = compile_texture(normal, directory / "normal", args.vtex.resolve()) \
            if normal else None
        emission_hash = compile_texture(emission_image, directory / "emission",
                                        args.vtex.resolve()) if emission_image else None
        texture = "%s/%s/basecolor" % (prefix, name)
        common = [("$surfaceprop", "default")]
        cutout = image.mode == "RGBA"
        if cutout:
            common += [("$alphatest", "1"),
                       ("$alphatestreference", "%g" % summary["opacity_threshold"])]
        if name in double_sided and summary["pbrt_type"] != "diffusetransmission":
            common.append(("$nocull", "1"))
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
            bump = [("$bumpmap", "%s/%s/normal" % (prefix, name))] if normal else []
            glow = [("$emissiontexture", "%s/%s/emission" % (prefix, name)),
                    ("$emissionscale", "%g" % emission_scale)] if emission_image else []
            world = vmt("PBRMetalRough", [("$basetexture", texture),
                                          ("$mraotexture", "%s/%s/mrao" % (prefix, name))] +
                        bump + glow +
                        [("$fallbackmaterial", "%s_fallback/%s" % (prefix, name))] + common)
        (fallback_root / (name + ".vmt")).write_text(fallback)
        (material_root / (name + ".vmt")).write_text(world)
        channels = {
            "base": "texture" if summary["base_texture"] else "constant",
            "metallic": "texture" if "metallic" in mrao_hashes else "constant",
            "roughness": "texture" if "roughness" in mrao_hashes else "constant",
            "occlusion": "texture" if "occlusion" in mrao_hashes else "none",
            "normal": "texture" if normal else "none",
            "opacity": "cutout" if cutout else (
                "transmission" if summary["transmission"] > 0 else "opaque"),
            "emission": None if emission is None else (
                "texture" if emission_image else "dropped: sky/emitter preview")}
        assets[name] = {"pbrt_material": summary["name"], "pbrt_type": summary["pbrt_type"],
                        "preview": preview, "approximation": summary["approximation"],
                        "source_texture_sha256": source_hashes.get("base") or
                        source_hashes.get("sky"),
                        "source_textures_sha256": dict(source_hashes, **mrao_hashes,
                                                       **({"normal": normal_source}
                                                          if normal else {})),
                        "authored_channels": sorted(summary["textures"]),
                        "exported_channels": channels,
                        "encoded_dimensions": list(image.size),
                        "basecolor_srgb": list(solid) if solid else None,
                        "mrao": list(mrao_rgb) if mrao_rgb else "texture",
                        "mrao_dimensions": list(mrao.size),
                        "normal_dimensions": list(normal.size) if normal else None,
                        "basecolor_vtf_sha256": base_hash,
                        "mrao_vtf_sha256": mrao_hash, "normal_vtf_sha256": normal_hash,
                        "emission_vtf_sha256": emission_hash,
                        "emission_scale": emission_scale if emission_image else None,
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
