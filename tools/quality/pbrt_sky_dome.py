#!/usr/bin/env python3
"""Add a visible sky dome to a baked PBRT map stage.

The game preview has no sky pass, so windows showed the clear color. This
writes a render stage that sublayers the lighting stage (left unchanged, so the
bake never sees the dome) and adds `/root/SkyDome`, a large inward-facing UV
sphere bound to material `Sky`. Its `st` follows the pipeline's equirect sky
convention (`pbrt_scene.environment_equirect`, row 0 = zenith), and it gets a
zero `lightmap_st`. The sky texture itself comes from the pipeline's
environment step (`pbrt_scene.sky_display`).

Run with the OpenUSD Python (`pxr`) on PYTHONPATH.
"""

import argparse
import hashlib
import json
import math
import os
from pathlib import Path

from pxr import Gf, Sdf, Usd, UsdGeom, UsdShade, Vt

LONGITUDES = 64
LATITUDES = 32


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def stage_bounds(stage):
    cache = UsdGeom.BBoxCache(Usd.TimeCode.Default(), [UsdGeom.Tokens.default_])
    box = cache.ComputeWorldBound(stage.GetPseudoRoot()).ComputeAlignedRange()
    if box.IsEmpty():
        raise ValueError("lighting stage has no geometry bounds")
    return box.GetMin(), box.GetMax()


def dome(center, radius):
    """Corner-expanded triangles, inward normals, equirect st.

    `st` follows USD's bottom-left origin (v = 1 at the zenith row), like
    every other stage mesh; the WMSH packer flips it for top-left sampling.
    """
    points, st, normals = [], [], []
    for row in range(LATITUDES):
        for column in range(LONGITUDES):
            quad = []
            for dr, dc in ((0, 0), (0, 1), (1, 1), (1, 0)):
                u = (column + dc) / LONGITUDES
                v = (row + dr) / LATITUDES
                phi = (0.5 - u) * 2.0 * math.pi
                elevation = (0.5 - v) * math.pi
                direction = (math.cos(elevation) * math.cos(phi),
                             math.cos(elevation) * math.sin(phi), math.sin(elevation))
                quad.append((direction, (u, v)))
            for a, b, c in ((0, 1, 2), (0, 2, 3)):
                corners = [quad[a], quad[b], quad[c]]
                positions = [Gf.Vec3f(*[center[i] + radius * d[i] for i in range(3)])
                             for d, _ in corners]
                normal = Gf.Cross(positions[1] - positions[0], positions[2] - positions[0])
                if normal.GetLength() < 1e-9:
                    continue  # pole fan collapses one corner
                inward = -Gf.Vec3f(*[sum(d[i] for d, _ in corners) for i in range(3)])
                if Gf.Dot(normal, inward) < 0:
                    corners = [corners[0], corners[2], corners[1]]
                    positions = [positions[0], positions[2], positions[1]]
                for (direction, uv), position in zip(corners, positions):
                    points.append(position)
                    st.append(Gf.Vec2f(uv[0], 1.0 - uv[1]))
                    normals.append(Gf.Vec3f(*[-value for value in direction]))
    return points, st, normals


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True, help="baked lighting stage")
    parser.add_argument("--sky-texture", type=Path, required=True,
                        help="display texture from the environment step (recorded only)")
    parser.add_argument("--out-stage", type=Path, required=True)
    parser.add_argument("--radius-scale", type=float, default=4.0,
                        help="dome radius as a multiple of the scene's half diagonal")
    args = parser.parse_args()
    lighting = Usd.Stage.Open(str(args.stage))
    if not lighting or UsdGeom.GetStageUpAxis(lighting) != UsdGeom.Tokens.z:
        raise ValueError("sky dome requires a readable Z-up lighting stage")
    low, high = stage_bounds(lighting)
    center = [(low[i] + high[i]) / 2 for i in range(3)]
    radius = args.radius_scale * (high - low).GetLength() / 2
    args.out_stage.parent.mkdir(parents=True, exist_ok=True)
    stage = Usd.Stage.CreateNew(str(args.out_stage))
    stage.GetRootLayer().subLayerPaths.append(
        os.path.relpath(args.stage.resolve(), args.out_stage.resolve().parent))
    UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
    UsdGeom.SetStageMetersPerUnit(stage, UsdGeom.GetStageMetersPerUnit(lighting))
    root = stage.OverridePrim("/root")
    stage.SetDefaultPrim(root)
    points, st, normals = dome(center, radius)
    mesh = UsdGeom.Mesh.Define(stage, "/root/SkyDome")
    mesh.CreatePointsAttr(Vt.Vec3fArray(points))
    mesh.CreateFaceVertexCountsAttr(Vt.IntArray([3] * (len(points) // 3)))
    mesh.CreateFaceVertexIndicesAttr(Vt.IntArray(list(range(len(points)))))
    mesh.CreateNormalsAttr(Vt.Vec3fArray(normals))
    mesh.SetNormalsInterpolation(UsdGeom.Tokens.faceVarying)
    primvars = UsdGeom.PrimvarsAPI(mesh)
    primvars.CreatePrimvar("st", Sdf.ValueTypeNames.TexCoord2fArray,
                           UsdGeom.Tokens.faceVarying).Set(Vt.Vec2fArray(st))
    primvars.CreatePrimvar("lightmap_st", Sdf.ValueTypeNames.TexCoord2fArray,
                           UsdGeom.Tokens.faceVarying).Set(
                               Vt.Vec2fArray([Gf.Vec2f(0, 0)] * len(st)))
    material = UsdShade.Material.Define(stage, "/root/_sky_materials/Sky")
    UsdShade.MaterialBindingAPI.Apply(mesh.GetPrim()).Bind(material)
    stage.GetRootLayer().Save()
    receipt = {"status": "pass", "scope": "pbrt-sky-dome",
               "lighting_stage_sha256": sha256(args.stage),
               "sky_texture_sha256": sha256(args.sky_texture),
               "render_stage_sha256": sha256(args.out_stage),
               "center": center, "radius": radius, "triangles": len(points) // 3}
    args.out_stage.with_name(args.out_stage.name + ".json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: receipt[k] for k in ("status", "radius", "triangles")}))


if __name__ == "__main__":
    main()
