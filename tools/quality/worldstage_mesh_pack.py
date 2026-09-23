#!/usr/bin/env python3
"""Pack a verified OpenUSD world geometry layer into a WMSH v1 preview lump."""

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

from bsp2_reader import parse_legacy_header


HEADER = 128
VERTEX = struct.Struct("<fffhhhhb3xffff")
BATCH = struct.Struct("<IIIIII")
MESHLET = struct.Struct("<IIIIffffffff")
MAX_MESHLET_TRIANGLES = 21  # Deindexed corners stay within 64 vertices.


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def unit(vector):
    length = math.sqrt(sum(value * value for value in vector))
    if not math.isfinite(length) or length < 1e-8:
        raise ValueError("World Stage has a degenerate direction")
    return tuple(value / length for value in vector)


def dot(left, right):
    return sum(a * b for a, b in zip(left, right))


def cross(left, right):
    return (left[1] * right[2] - left[2] * right[1],
            left[2] * right[0] - left[0] * right[2],
            left[0] * right[1] - left[1] * right[0])


def oct16(vector):
    x, y, z = unit(vector)
    scale = abs(x) + abs(y) + abs(z)
    x, y, z = x / scale, y / scale, z / scale
    if z < 0:
        x, y = ((1 - abs(y)) * (1 if x >= 0 else -1),
                (1 - abs(x)) * (1 if y >= 0 else -1))
    return (round(max(-1, min(1, x)) * 32767),
            round(max(-1, min(1, y)) * 32767))


def tangent_frame(points, uv, normal):
    edge1 = tuple(points[1][i] - points[0][i] for i in range(3))
    edge2 = tuple(points[2][i] - points[0][i] for i in range(3))
    du1, dv1 = uv[1][0] - uv[0][0], uv[1][1] - uv[0][1]
    du2, dv2 = uv[2][0] - uv[0][0], uv[2][1] - uv[0][1]
    determinant = du1 * dv2 - du2 * dv1
    if abs(determinant) < 1e-12:
        raise ValueError("World Stage has a degenerate material UV triangle")
    tangent = tuple((edge1[i] * dv2 - edge2[i] * dv1) / determinant for i in range(3))
    tangent = unit(tuple(tangent[i] - normal[i] * dot(tangent, normal)
                         for i in range(3)))
    bitangent = tuple((edge2[i] * du1 - edge1[i] * du2) / determinant for i in range(3))
    sign = 1 if dot(cross(normal, tangent), bitangent) >= 0 else -1
    return tangent, sign


def stage_faces(stage_path):
    from pxr import Usd, UsdGeom

    stage = Usd.Stage.Open(str(stage_path))
    if stage is None:
        raise ValueError("could not open World Stage")
    root = stage.GetPrimAtPath("/World/Geometry/WorldSpawn")
    if not root:
        raise ValueError("World Stage lacks world geometry")
    result = {}
    for prim in root.GetChildren():
        if not prim.IsA(UsdGeom.Mesh) or not prim.GetName().startswith("Mesh_"):
            raise ValueError("World Stage has an unexpected world prim")
        face_id = int(prim.GetName()[5:])
        if face_id in result:
            raise ValueError("World Stage has duplicate BSP face IDs")
        mesh = UsdGeom.Mesh(prim)
        points = [tuple(map(float, point)) for point in mesh.GetPointsAttr().Get()]
        counts = list(mesh.GetFaceVertexCountsAttr().Get())
        indices = list(mesh.GetFaceVertexIndicesAttr().Get())
        normals = [unit(tuple(map(float, value))) for value in mesh.GetNormalsAttr().Get()]
        uv = [tuple(map(float, value)) for value in prim.GetAttribute("primvars:st").Get()]
        lightmap_uv = [tuple(map(float, value)) for value in
                       prim.GetAttribute("primvars:lightmap:st").Get()]
        material = prim.GetAttribute("source:materialPath").Get()
        if (not counts or any(count != 3 for count in counts) or
                len(indices) != 3 * len(counts) or len(normals) != len(counts) or
                len(uv) != len(indices) or len(lightmap_uv) != len(indices) or
                not isinstance(material, str) or not material):
            raise ValueError("World Stage face has unsupported triangulation or attributes")
        triangles = []
        for triangle in range(len(counts)):
            start = 3 * triangle
            corners = indices[start:start + 3]
            if any(corner < 0 or corner >= len(points) for corner in corners):
                raise ValueError("World Stage face has an invalid vertex index")
            positions = [points[corner] for corner in corners]
            texture = uv[start:start + 3]
            lightmap = lightmap_uv[start:start + 3]
            normal = normals[triangle]
            tangent, sign = tangent_frame(positions, texture, normal)
            triangles.append((positions, texture, lightmap, normal, tangent, sign))
        result[face_id] = {"material": material, "triangles": triangles}
    if not result:
        raise ValueError("World Stage has no world triangles")
    return result


def leaf_faces(bsp_path):
    data = bsp_path.read_bytes()
    header = parse_legacy_header(data, len(data))
    if header["version"] != 21:
        raise ValueError("WMSH preview requires a compiled v21 BSP")

    def lump(index):
        offset, size, *_ = header["lumps"][index]
        return data[offset:offset + size]

    leaves = lump(10)
    references = lump(16)
    if (header["lumps"][10][2] != 1 or not leaves or len(leaves) % 32 or
            len(references) % 2):
        raise ValueError("BSP leaf layout is unsupported")
    result = []
    for index in range(len(leaves) // 32):
        first, count = struct.unpack_from("<HH", leaves, index * 32 + 20)
        if first + count > len(references) // 2:
            raise ValueError("BSP leaf face range escapes the reference lump")
        result.append({struct.unpack_from("<H", references, (first + i) * 2)[0]
                       for i in range(count)})
    return result


def write_payload(faces, leaves):
    materials = sorted({face["material"] for face in faces.values()})
    if any(not material or material.endswith(".vmt") or "\\" in material or
           ":" in material or
           any(ord(char) < 32 or ord(char) == 127 for char in material) or
           any(part in ("", ".", "..") for part in material.split("/"))
           for material in materials):
        raise ValueError("WMSH v1 requires normalized relative material paths")
    material_ids = {name: index for index, name in enumerate(materials)}
    vertex_bytes = bytearray()
    indices = []
    triangle_faces = []
    batches = []
    meshlets = []
    face_meshlets = {face_id: [] for face_id in faces}
    for material in materials:
        first_index = len(indices)
        first_meshlet = len(meshlets)
        for face_id in sorted(faces):
            if faces[face_id]["material"] != material:
                continue
            triangles = faces[face_id]["triangles"]
            for chunk_start in range(0, len(triangles), MAX_MESHLET_TRIANGLES):
                chunk = triangles[chunk_start:chunk_start + MAX_MESHLET_TRIANGLES]
                meshlet_first_index = len(indices)
                meshlet_first_vertex = len(vertex_bytes) // VERTEX.size
                positions = []
                normals = []
                for points, texture, lightmap, normal, tangent, sign in chunk:
                    for point, uv, chart_uv in zip(points, texture, lightmap):
                        if not all(math.isfinite(value) for value in (*point, *uv, *chart_uv)):
                            raise ValueError("World Stage mesh has a non-finite vertex")
                        vertex_bytes.extend(VERTEX.pack(*point, *oct16(normal),
                                                        *oct16(tangent), sign, *uv, *chart_uv))
                        indices.append(len(indices))
                        positions.append(point)
                        normals.append(normal)
                    triangle_faces.append(face_id)
                center = tuple(sum(point[axis] for point in positions) / len(positions)
                               for axis in range(3))
                radius = max(math.dist(center, point) for point in positions) + 1e-5
                axis = unit(tuple(sum(normal[i] for normal in normals) for i in range(3)))
                cutoff = min(dot(axis, normal) for normal in normals) - 1e-5
                meshlets.append((meshlet_first_index, len(indices) - meshlet_first_index,
                                 meshlet_first_vertex, len(positions), *center, radius,
                                 *axis, cutoff))
                face_meshlets[face_id].append(len(meshlets) - 1)
        batches.append((material_ids[material], first_index, len(indices) - first_index,
                        first_meshlet, len(meshlets) - first_meshlet, 0))
    leaf_ranges = []
    leaf_refs = []
    for leaf in leaves:
        selected = sorted({meshlet for face_id in leaf for meshlet in
                           face_meshlets.get(face_id, ())})
        leaf_ranges.append((len(leaf_refs), len(selected)))
        leaf_refs.extend(selected)
    if len(indices) != len(vertex_bytes) // VERTEX.size or len(indices) != 3 * len(triangle_faces):
        raise ValueError("WMSH deindexed triangle counts diverge")
    sections = [bytes(vertex_bytes),
                struct.pack("<%dI" % len(indices), *indices),
                struct.pack("<%dI" % len(triangle_faces), *triangle_faces),
                b"".join(BATCH.pack(*batch) for batch in batches),
                b"".join(MESHLET.pack(*meshlet) for meshlet in meshlets),
                b"".join(struct.pack("<II", *record) for record in leaf_ranges),
                struct.pack("<%dI" % len(leaf_refs), *leaf_refs),
                b"".join(struct.pack("<I", len(name.encode())) + name.encode() +
                         b"\0" * (-(4 + len(name.encode())) % 4) for name in materials)]
    payload = bytearray(HEADER)
    offsets = []
    for section in sections:
        payload.extend(b"\0" * (-len(payload) % 16))
        offsets.append(len(payload))
        payload.extend(section)
    if any(count > 0xFFFFFFFF for count in (len(indices), len(triangle_faces),
                                            len(meshlets), len(leaf_refs))):
        raise ValueError("WMSH exceeds v1 count range")
    struct.pack_into("<4s13I", payload, 0, b"WMSH", 1, HEADER, 0,
                     len(vertex_bytes) // VERTEX.size, len(indices), len(triangle_faces),
                     len(batches), len(meshlets), len(leaves), len(leaf_refs),
                     len(materials), len(sections[-1]), 0)
    struct.pack_into("<9Q", payload, 56, *offsets, len(payload))
    return bytes(payload), {"vertices": len(vertex_bytes) // VERTEX.size,
                            "indices": len(indices), "triangles": len(triangle_faces),
                            "batches": len(batches), "meshlets": len(meshlets),
                            "leaves": len(leaves), "leaf_references": len(leaf_refs),
                            "materials": materials}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--geometry-comparison", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    checked = json.loads(args.geometry_comparison.read_text())
    if (checked["status"] != "pass" or checked["stage_sha256"] != sha256(args.stage) or
            checked["bsp_sha256"] != sha256(args.bsp)):
        raise ValueError("World Stage geometry lacks a matching independent BSP comparison")
    faces = stage_faces(args.stage)
    payload, counts = write_payload(faces, leaf_faces(args.bsp))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(payload)
    evidence = {"status": "pass", "scope": "worldstage-wmsh-v1-preview",
                "stage_sha256": sha256(args.stage), "bsp_sha256": sha256(args.bsp),
                "geometry_comparison_sha256": sha256(args.geometry_comparison),
                "wmsh_sha256": sha256(args.out), "wmsh_bytes": len(payload), **counts}
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2,
                                                        sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
