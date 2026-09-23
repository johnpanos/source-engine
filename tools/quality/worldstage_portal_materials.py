#!/usr/bin/env python3
"""Resolve compiled USD face material paths to Portal remaster PNG masters.

Run with the Python/OpenUSD installation used by the vbsp2 host profile.
This manifest is for a Cycles inspection render, not a runtime asset package.
"""

import argparse
import hashlib
import json
import os
import re
from pathlib import Path

from pxr import Gf, Sdf, Usd, UsdGeom, UsdShade


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def vmt_value(text, key):
    found = re.findall(r'"\$' + re.escape(key) + r'"\s*"([^"\r\n]+)"',
                       text, flags=re.IGNORECASE)
    if len(found) != 1:
        raise ValueError("expected exactly one $" + key + " in Portal VMT")
    return found[0].lower()


def resolve_texture(root, name):
    relative = Path(name)
    if relative.is_absolute() or ".." in relative.parts or not name.startswith("portal_pbr/"):
        raise ValueError("invalid Portal texture path: " + name)
    path = root / "textures" / (name + ".png")
    if not path.is_file():
        raise FileNotFoundError(path)
    return {"path": str(path), "sha256": digest(path)}


def author_preview_stage(geometry, output, meshes, materials):
    """Compose the geometry with a separate, preview-only UsdShade layer."""
    if output.exists():
        raise ValueError("material stage already exists: " + str(output))
    composed = Usd.Stage.CreateNew(str(output))
    relative = Path(os.path.relpath(geometry.resolve(), output.parent.resolve()))
    composed.GetRootLayer().subLayerPaths.append(str(relative))
    asset_set = json.dumps(materials, sort_keys=True, separators=(",", ":")).encode()
    composed.GetRootLayer().customLayerData = {
        "sourceGeometrySha256": digest(geometry),
        "sourcePortalAssetsSha256": hashlib.sha256(asset_set).hexdigest(),
    }
    UsdGeom.SetStageUpAxis(composed, UsdGeom.Tokens.z)
    UsdGeom.SetStageMetersPerUnit(composed, 0.0254)
    composed.SetDefaultPrim(composed.GetPrimAtPath("/World"))
    for index, path in enumerate(sorted(materials)):
        spec = materials[path]
        material = UsdShade.Material.Define(composed, f"/World/Materials/Portal_{index}")
        material.GetPrim().CreateAttribute("source:materialPath", Sdf.ValueTypeNames.String).Set(path)
        uv = UsdShade.Shader.Define(composed, material.GetPath().AppendChild("SourceUV"))
        uv.CreateIdAttr("UsdPrimvarReader_float2")
        uv.CreateInput("varname", Sdf.ValueTypeNames.String).Set("st")
        uv_out = uv.CreateOutput("result", Sdf.ValueTypeNames.Float2)
        shader = UsdShade.Shader.Define(composed, material.GetPath().AppendChild("PreviewSurface"))
        shader.CreateIdAttr("UsdPreviewSurface")
        shader_out = shader.CreateOutput("surface", Sdf.ValueTypeNames.Token)
        material.CreateSurfaceOutput().ConnectToSource(shader_out)
        for role, asset in (("basecolor", spec["basecolor"]), ("mrao", spec["mrao"])):
            texture = UsdShade.Shader.Define(composed, material.GetPath().AppendChild(
                "BaseColor" if role == "basecolor" else "MetalRough"))
            texture.CreateIdAttr("UsdUVTexture")
            texture.CreateInput("file", Sdf.ValueTypeNames.Asset).Set(asset["path"])
            texture.CreateInput("sourceColorSpace", Sdf.ValueTypeNames.Token).Set(
                "sRGB" if role == "basecolor" else "raw")
            texture.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(uv_out)
            if role == "basecolor":
                shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(
                    texture.CreateOutput("rgb", Sdf.ValueTypeNames.Float3))
            else:
                shader.CreateInput("metallic", Sdf.ValueTypeNames.Float).ConnectToSource(
                    texture.CreateOutput("r", Sdf.ValueTypeNames.Float))
                shader.CreateInput("roughness", Sdf.ValueTypeNames.Float).ConnectToSource(
                    texture.CreateOutput("g", Sdf.ValueTypeNames.Float))
        if "normal" in spec:
            normal = UsdShade.Shader.Define(composed, material.GetPath().AppendChild("Normal"))
            normal.CreateIdAttr("UsdUVTexture")
            normal.CreateInput("file", Sdf.ValueTypeNames.Asset).Set(spec["normal"]["path"])
            normal.CreateInput("sourceColorSpace", Sdf.ValueTypeNames.Token).Set("raw")
            normal.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(uv_out)
            normal.CreateInput("scale", Sdf.ValueTypeNames.Float4).Set(Gf.Vec4f(2, 2, 2, 1))
            normal.CreateInput("bias", Sdf.ValueTypeNames.Float4).Set(Gf.Vec4f(-1, -1, -1, 0))
            shader.CreateInput("normal", Sdf.ValueTypeNames.Normal3f).ConnectToSource(
                normal.CreateOutput("rgb", Sdf.ValueTypeNames.Float3))
        for mesh_name, material_path in sorted(meshes.items()):
            if material_path == path:
                prim = composed.GetPrimAtPath("/World/Geometry/WorldSpawn/" + mesh_name)
                if not prim:
                    raise ValueError("material layer cannot find mesh " + mesh_name)
                UsdShade.MaterialBindingAPI.Apply(prim).Bind(material)
    composed.GetRootLayer().Save()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--portal-assets", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--composed-stage", type=Path,
                        help="author a separate UsdShade preview layer over the geometry")
    args = parser.parse_args()
    stage = Usd.Stage.Open(str(args.stage.resolve()))
    if not stage:
        raise ValueError("could not open compiled World Stage")
    root = args.portal_assets.resolve()
    world = stage.GetPrimAtPath("/World/Geometry/WorldSpawn")
    if not world:
        raise ValueError("missing world geometry")
    stage_world = stage.GetPrimAtPath("/World")
    face_ids = list(stage_world.GetAttribute("source:chartFaceIds").Get())
    chart_rects = list(stage_world.GetAttribute("source:chartRects").Get())
    if not face_ids or len(face_ids) != len(chart_rects):
        raise ValueError("stage has an invalid lightmap chart table")
    meshes = {}
    materials = {}
    for prim in world.GetChildren():
        if not prim.IsA(UsdGeom.Mesh) or not prim.HasAPI("SourceMeshAPI"):
            raise ValueError("world has an unsupported geometry prim")
        name = prim.GetName()
        path = prim.GetAttribute("source:materialPath").Get().lower()
        if not path.startswith("portal_pbr/") or ".." in Path(path).parts:
            raise ValueError("world face lacks a Portal PBR material: " + name)
        meshes[name] = path
        if path in materials:
            continue
        vmt = root / "materials" / (path + ".vmt")
        if not vmt.is_file():
            raise FileNotFoundError(vmt)
        text = vmt.read_text(encoding="utf-8")
        if not text.lstrip().lower().startswith('"pbrmetalrough"'):
            raise ValueError("unsupported Portal VMT shader: " + str(vmt))
        materials[path] = {
            "vmt_sha256": digest(vmt),
            "basecolor": resolve_texture(root, vmt_value(text, "basetexture")),
            "mrao": resolve_texture(root, vmt_value(text, "mraotexture")),
        }
        if re.search(r'"\$bumpmap"', text, flags=re.IGNORECASE):
            materials[path]["normal"] = resolve_texture(root, vmt_value(text, "bumpmap"))
    if not meshes or not materials:
        raise ValueError("stage has no Portal world materials")
    manifest = {
        "stage_sha256": digest(args.stage), "meshes": meshes, "materials": materials,
        "lightmap_atlas": {
            "width": stage_world.GetAttribute("source:lightmapAtlasWidth").Get(),
            "height": stage_world.GetAttribute("source:lightmapAtlasHeight").Get(),
            "charts": {str(face): list(rect) for face, rect in zip(face_ids, chart_rects)},
        },
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    if args.composed_stage:
        args.composed_stage.parent.mkdir(parents=True, exist_ok=True)
        author_preview_stage(args.stage, args.composed_stage, meshes, materials)
    print(json.dumps({"status": "pass", "meshes": len(meshes),
                      "materials": len(materials)}, sort_keys=True))


if __name__ == "__main__":
    main()
