#!/usr/bin/env python3
"""Pack bound USD meshes into a BSP2 WMSH preview alongside BSP gameplay data.

The BSP supplies collision and leaves. Imported USD triangles use high-bit
synthetic face IDs, which deliberately have no legacy decal association. Until
spatial import is implemented, every leaf references every imported meshlet.
"""

import argparse
import json
import math
import re
from pathlib import Path

from pxr import Gf, Usd, UsdGeom, UsdShade

from worldstage_mesh_pack import cross, leaf_faces, sha256, tangent_frame, unit, write_payload


SOURCE_UNITS_PER_METER = 39.37007874015748
IMPORTED_FACE_BASE = 0x80000000


def normal_bucket(normal):
    axis = max(range(3), key=lambda index: abs(normal[index]))
    return 2 * axis + int(normal[axis] < 0)


def source_triangles(stage, material_prefix, require_lightmap_uv, include_emitters):
    if UsdGeom.GetStageUpAxis(stage) != UsdGeom.Tokens.z:
        raise ValueError("WMSH import requires a Z-up USD stage")
    meters = UsdGeom.GetStageMetersPerUnit(stage)
    if not math.isfinite(meters) or meters <= 0:
        raise ValueError("USD metersPerUnit is invalid")
    scale = SOURCE_UNITS_PER_METER * meters
    transforms = UsdGeom.XformCache()
    faces = {}
    inventory = []
    source_meshes = sorted((prim for prim in stage.Traverse()
                            if prim.IsA(UsdGeom.Mesh) and
                            re.fullmatch(r"Mesh\d{3}", prim.GetName())),
                           key=lambda prim: prim.GetName())
    emitters = sorted((prim for prim in stage.Traverse()
                       if include_emitters and prim.IsA(UsdGeom.Mesh) and
                       re.fullmatch(r"Light(?:Quad|Disk)\d{2}", prim.GetName())),
                      key=lambda prim: prim.GetName())
    meshes = source_meshes + emitters
    if not source_meshes:
        raise ValueError("USD stage has no bound source meshes")
    for mesh_index, prim in enumerate(meshes):
        mesh = UsdGeom.Mesh(prim)
        is_emitter = mesh_index >= len(source_meshes)
        material = (None if is_emitter else
                    UsdShade.MaterialBindingAPI(prim).ComputeBoundMaterial()[0])
        if not is_emitter and not material:
            raise ValueError("USD mesh has no bound material: " + str(prim.GetPath()))
        material_name = "emitter" if is_emitter else material.GetPrim().GetName().lower()
        if not re.fullmatch(r"[a-z0-9_]+", material_name):
            raise ValueError("USD material name cannot become a VMT path")
        material_path = material_prefix + "/" + material_name
        points = list(mesh.GetPointsAttr().Get() or [])
        counts = list(mesh.GetFaceVertexCountsAttr().Get() or [])
        indices = list(mesh.GetFaceVertexIndicesAttr().Get() or [])
        normals = list(mesh.GetNormalsAttr().Get() or [])
        st = UsdGeom.PrimvarsAPI(prim).GetPrimvar("st")
        if not is_emitter and (not st or st.GetInterpolation() != UsdGeom.Tokens.faceVarying):
            raise ValueError("USD mesh requires faceVarying st: " + str(prim.GetPath()))
        uv = ([(0.0, 0.0)] * len(indices) if is_emitter else
              list(st.ComputeFlattened() or []))
        lightmap_primvar = UsdGeom.PrimvarsAPI(prim).GetPrimvar("lightmap_st")
        if require_lightmap_uv and not is_emitter and (not lightmap_primvar or
                                    lightmap_primvar.GetInterpolation() !=
                                    UsdGeom.Tokens.faceVarying):
            raise ValueError("USD mesh lacks authored lightmap_st: " + str(prim.GetPath()))
        lightmap_uv = ([(0.0, 0.0)] * len(indices) if is_emitter else
                       list(lightmap_primvar.ComputeFlattened() or []) if lightmap_primvar else [])
        if (not points or not counts or any(count < 3 or
                                           (not is_emitter and count != 3) for count in counts) or
                len(indices) != sum(counts) or len(uv) != len(indices) or
                (lightmap_uv and len(lightmap_uv) != len(indices)) or
                mesh.GetNormalsInterpolation() != UsdGeom.Tokens.faceVarying or
                len(normals) != len(indices)):
            raise ValueError("USD mesh has unsupported topology or corner attributes")
        transform = transforms.GetLocalToWorldTransform(prim)
        normal_transform = transform.GetInverse().GetTranspose()
        world_points = [tuple(float(v) * scale for v in transform.Transform(
            Gf.Vec3d(point))) for point in points]
        triangle_count = 0
        tangent_fallbacks = 0
        normal_fallbacks = 0
        degenerate_triangles = 0
        triangle_corners = []
        offset = 0
        for count in counts:
            triangle_corners.extend((offset, offset + corner, offset + corner + 1)
                                    for corner in range(1, count - 1))
            offset += count
        for corner_ids in triangle_corners:
            corners = [indices[index] for index in corner_ids]
            if any(index < 0 or index >= len(points) for index in corners):
                raise ValueError("USD mesh has an out-of-range corner index")
            positions = [world_points[index] for index in corners]
            texture_uv = [tuple(map(float, uv[index])) for index in corner_ids]
            chart_uv = [tuple(map(float, value)) for value in
                        (lightmap_uv[index] for index in corner_ids)] if lightmap_uv else [(0.0, 0.0)] * 3
            transformed_normals = [tuple(map(float, normal_transform.TransformDir(
                Gf.Vec3d(normals[index])))) for index in corner_ids]
            valid_normals = [unit(value) for value in transformed_normals
                             if all(math.isfinite(component) for component in value) and
                             math.sqrt(sum(component * component for component in value)) >= 1e-8]
            if len(valid_normals) != 3:
                normal_fallbacks += 3 - len(valid_normals)
            if valid_normals:
                normal = unit(tuple(sum(value[axis] for value in valid_normals)
                                    for axis in range(3)))
            else:
                edge1 = tuple(positions[1][axis] - positions[0][axis] for axis in range(3))
                edge2 = tuple(positions[2][axis] - positions[0][axis] for axis in range(3))
                try:
                    normal = unit(cross(edge1, edge2))
                except ValueError:
                    # Blender's UV repack can emit zero corner normals on
                    # collapsed source triangles. They cover no area and
                    # cannot contribute to a WMSH draw or lightmap chart.
                    degenerate_triangles += 1
                    continue
            try:
                tangent, sign = tangent_frame(positions, texture_uv, normal)
            except ValueError:
                reference = (0.0, 0.0, 1.0) if abs(normal[2]) < 0.9 else (0.0, 1.0, 0.0)
                tangent, sign = unit(cross(reference, normal)), 1
                tangent_fallbacks += 1
            bucket = normal_bucket(normal)
            face_id = IMPORTED_FACE_BASE | (mesh_index << 3) | bucket
            item = faces.setdefault(face_id, {"material": material_path, "triangles": []})
            item["triangles"].append((positions, texture_uv,
                                      chart_uv, normal,
                                      tangent, sign))
            triangle_count += 1
        inventory.append({"prim": str(prim.GetPath()), "material": material_path,
                          "emitter": is_emitter,
                          "triangles": triangle_count,
                          "tangent_fallbacks": tangent_fallbacks,
                          "normal_fallbacks": normal_fallbacks,
                          "discarded_zero_area_triangles": degenerate_triangles})
    return faces, inventory


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--material-prefix", required=True)
    parser.add_argument("--require-lightmap-uv", action="store_true")
    parser.add_argument("--include-emitters", action="store_true")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    receipt_path = Path(str(args.out) + ".json")
    if args.out.exists() or receipt_path.exists():
        parser.error("WMSH output or receipt already exists")
    if not re.fullmatch(r"[a-z0-9_]+(?:/[a-z0-9_]+)*", args.material_prefix):
        parser.error("material prefix must be a normalized relative path")
    stage = Usd.Stage.Open(str(args.stage))
    if not stage:
        raise ValueError("could not open USD stage")
    faces, inventory = source_triangles(stage, args.material_prefix,
                                        args.require_lightmap_uv, args.include_emitters)
    leaves = leaf_faces(args.bsp)
    # Explicit first-slice visibility policy: every imported meshlet is visible
    # in every BSP leaf. This preserves visibility while spatial association is
    # still absent; the receipt exposes its cost.
    all_face_ids = set(faces)
    payload, counts = write_payload(faces, [all_face_ids for _ in leaves])
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(payload)
    evidence = {"status": "pass", "scope": "usd-imported-worldmesh-preview",
                "stage_sha256": sha256(args.stage), "bsp_sha256": sha256(args.bsp),
                "wmsh_sha256": sha256(args.out), "material_prefix": args.material_prefix,
                "visibility_policy": "all imported meshlets in every leaf",
                "authored_lightmap_uv": args.require_lightmap_uv,
                "emitter_meshes": sum(item["emitter"] for item in inventory),
                "source_meshes": inventory, **counts}
    receipt_path.write_text(json.dumps(evidence, indent=2,
                                       sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
