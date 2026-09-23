#!/usr/bin/env python3
"""Independently compare a WMSH v1 payload with OpenUSD and compiled BSP."""

import argparse
from collections import defaultdict
import hashlib
import json
import math
import struct
from pathlib import Path

import bsp2_reader
from bsp2_reader import parse_legacy_header


SIZES = (40, 4, 4, 24, 48, 8, 4, None)


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def close(left, right, tolerance=2e-4):
    return len(left) == len(right) and all(abs(a - b) <= tolerance
                                           for a, b in zip(left, right))


def dot(left, right):
    return sum(a * b for a, b in zip(left, right))


def cross(left, right):
    return (left[1] * right[2] - left[2] * right[1],
            left[2] * right[0] - left[0] * right[2],
            left[0] * right[1] - left[1] * right[0])


def normalize(vector):
    length = math.sqrt(dot(vector, vector))
    if length < 1e-8 or not math.isfinite(length):
        raise ValueError("WMSH has a degenerate direction")
    return tuple(component / length for component in vector)


def decode_oct(pair):
    x, y = (value / 32767.0 for value in pair)
    z = 1.0 - abs(x) - abs(y)
    if z < 0:
        x, y = ((1.0 - abs(y)) * (1 if x >= 0 else -1),
                (1.0 - abs(x)) * (1 if y >= 0 else -1))
    return normalize((x, y, z))


def read_payload(data):
    if len(data) < 128:
        raise ValueError("WMSH header is truncated")
    fields = struct.unpack_from("<4s13I", data)
    if fields[0] != b"WMSH" or fields[1:4] != (1, 128, 0) or fields[13] != 0:
        raise ValueError("WMSH header version or flags are invalid")
    vertices, indices, triangles, batches, meshlets, leaves, refs, materials = fields[4:12]
    material_bytes = fields[12]
    if (not vertices or indices != vertices or indices != 3 * triangles or
            not batches or not meshlets or not materials):
        raise ValueError("WMSH counts are inconsistent or empty")
    offsets = struct.unpack_from("<9Q", data, 56)
    if offsets[-1] != len(data):
        raise ValueError("WMSH total size differs from payload")
    counts = (vertices, indices, triangles, batches, meshlets, leaves, refs,
              material_bytes)
    sections = []
    cursor = 128
    for offset, count, size in zip(offsets, counts, SIZES):
        if offset != ((cursor + 15) & ~15):
            raise ValueError("WMSH sections are not contiguous and aligned")
        length = count if size is None else count * size
        if offset + length > len(data):
            raise ValueError("WMSH section escapes payload")
        sections.append(memoryview(data)[offset:offset + length])
        cursor = offset + length
    if cursor != len(data):
        raise ValueError("WMSH contains unreferenced trailing bytes")
    parsed_materials = []
    material_data = sections[7]
    cursor = 0
    for _ in range(materials):
        if cursor + 4 > len(material_data):
            raise ValueError("WMSH material table is truncated")
        length = struct.unpack_from("<I", material_data, cursor)[0]
        end = cursor + 4 + length
        padded = (end + 3) & ~3
        if not length or padded > len(material_data) or any(material_data[end:padded]):
            raise ValueError("WMSH material string is malformed")
        text = bytes(material_data[cursor + 4:end]).decode("utf-8")
        if (text.endswith(".vmt") or "\\" in text or ":" in text or
                any(ord(char) < 32 or ord(char) == 127 for char in text) or
                any(part in ("", ".", "..") for part in text.split("/"))):
            raise ValueError("WMSH material path is invalid")
        parsed_materials.append(text)
        cursor = padded
    if cursor != len(material_data) or parsed_materials != sorted(set(parsed_materials)):
        raise ValueError("WMSH material table is unordered or has extra bytes")
    parsed_vertices = []
    for position in range(vertices):
        raw = sections[0][40 * position:40 * (position + 1)]
        if bytes(raw[21:24]) != b"\0\0\0":
            raise ValueError("WMSH vertex padding is nonzero")
        values = struct.unpack("<fffhhhhb3xffff", raw)
        point = values[0:3]
        normal = decode_oct(values[3:5])
        tangent = decode_oct(values[5:7])
        sign = values[7]
        uv, lightmap_uv = values[8:10], values[10:12]
        if (not all(math.isfinite(value) for value in (*point, *uv, *lightmap_uv)) or
                sign not in (-1, 1) or abs(dot(normal, tangent)) > 5e-4):
            raise ValueError("WMSH vertex attributes are invalid")
        parsed_vertices.append((point, normal, tangent, sign, uv, lightmap_uv))
    parsed_indices = struct.unpack("<%dI" % indices, sections[1])
    if parsed_indices != tuple(range(indices)):
        raise ValueError("WMSH v1 must use deindexed triangle corners")
    parsed_faces = struct.unpack("<%dI" % triangles, sections[2])
    parsed_batches = [struct.unpack_from("<6I", sections[3], 24 * i)
                      for i in range(batches)]
    parsed_meshlets = [struct.unpack_from("<4I8f", sections[4], 48 * i)
                       for i in range(meshlets)]
    leaf_ranges = [struct.unpack_from("<II", sections[5], 8 * i)
                   for i in range(leaves)]
    leaf_refs = struct.unpack("<%dI" % refs, sections[6]) if refs else ()
    index_cursor = 0
    meshlet_cursor = 0
    for material_id, first, count, first_meshlet, meshlet_count, reserved in parsed_batches:
        if (material_id >= materials or first != index_cursor or count % 3 or not count or
                first_meshlet != meshlet_cursor or not meshlet_count or reserved):
            raise ValueError("WMSH batches are not contiguous")
        index_cursor += count
        meshlet_cursor += meshlet_count
    if index_cursor != indices or meshlet_cursor != meshlets:
        raise ValueError("WMSH batches omit geometry")
    index_cursor = 0
    meshlet_faces = []
    for first, count, first_vertex, vertex_count, *bounds in parsed_meshlets:
        if (first != index_cursor or first_vertex != first or count != vertex_count or
                count % 3 or not 0 < count <= 64 or count // 3 > 126):
            raise ValueError("WMSH meshlet span is invalid")
        center, radius, axis, cutoff = bounds[:3], bounds[3], bounds[4:7], bounds[7]
        if (not all(math.isfinite(value) for value in bounds) or radius < 0 or
                abs(dot(axis, axis) - 1.0) > 1e-4 or not -1 <= cutoff <= 1):
            raise ValueError("WMSH meshlet bounds or normal cone are invalid")
        faces_in_meshlet = set(parsed_faces[first // 3:(first + count) // 3])
        if len(faces_in_meshlet) != 1:
            raise ValueError("WMSH preview meshlet crosses BSP face identity")
        meshlet_faces.append(next(iter(faces_in_meshlet)))
        for vertex in parsed_vertices[first:first + count]:
            if math.dist(vertex[0], center) > radius + 1e-4 or dot(vertex[1], axis) < cutoff - 1e-4:
                raise ValueError("WMSH meshlet bound does not contain its vertices")
        index_cursor += count
    if index_cursor != indices:
        raise ValueError("WMSH meshlets omit triangles")
    ref_cursor = 0
    parsed_leaves = []
    for first, count in leaf_ranges:
        if first != ref_cursor or first + count > len(leaf_refs):
            raise ValueError("WMSH leaf references are not contiguous")
        selected = leaf_refs[first:first + count]
        if tuple(sorted(set(selected))) != selected or any(value >= meshlets for value in selected):
            raise ValueError("WMSH leaf references are invalid")
        parsed_leaves.append(selected)
        ref_cursor += count
    if ref_cursor != refs:
        raise ValueError("WMSH has unreferenced leaf entries")
    return {"vertices": parsed_vertices, "faces": parsed_faces,
            "batches": parsed_batches, "meshlets": parsed_meshlets,
            "meshlet_faces": meshlet_faces, "leaves": parsed_leaves,
            "materials": parsed_materials,
            "counts": {"vertices": vertices, "indices": indices, "triangles": triangles,
                       "batches": batches, "meshlets": meshlets, "leaves": leaves,
                       "leaf_references": refs, "materials": materials}}


def expected_stage(stage_path):
    from pxr import Usd, UsdGeom

    stage = Usd.Stage.Open(str(stage_path))
    if stage is None:
        raise ValueError("cannot open World Stage")
    root = stage.GetPrimAtPath("/World/Geometry/WorldSpawn")
    if not root:
        raise ValueError("World Stage lacks world geometry")
    triangles = defaultdict(list)
    for prim in root.GetChildren():
        mesh = UsdGeom.Mesh(prim)
        face_id = int(prim.GetName().removeprefix("Mesh_"))
        points = mesh.GetPointsAttr().Get()
        indices = mesh.GetFaceVertexIndicesAttr().Get()
        uv = prim.GetAttribute("primvars:st").Get()
        lightmap = prim.GetAttribute("primvars:lightmap:st").Get()
        normals = mesh.GetNormalsAttr().Get()
        material = prim.GetAttribute("source:materialPath").Get()
        for triangle in range(len(indices) // 3):
            corners = indices[3 * triangle:3 * triangle + 3]
            triangles[face_id].append((material,
                                       [tuple(points[index]) for index in corners],
                                       [tuple(value) for value in uv[3 * triangle:3 * triangle + 3]],
                                       [tuple(value) for value in
                                        lightmap[3 * triangle:3 * triangle + 3]],
                                       tuple(normals[triangle])))
    return triangles


def expected_leaves(bsp_path):
    data = bsp_path.read_bytes()
    header = parse_legacy_header(data, len(data))
    if header["version"] != 21 or header["lumps"][10][2] != 1:
        raise ValueError("BSP leaf version differs from WMSH preview")
    leaf_offset, leaf_size, *_ = header["lumps"][10]
    ref_offset, ref_size, *_ = header["lumps"][16]
    if leaf_size % 32 or ref_size % 2:
        raise ValueError("BSP leaf or face-reference size is invalid")
    leaves = []
    for leaf in range(leaf_size // 32):
        first, count = struct.unpack_from("<HH", data, leaf_offset + 32 * leaf + 20)
        if first + count > ref_size // 2:
            raise ValueError("BSP leaf face references escape their lump")
        leaves.append({struct.unpack_from("<H", data, ref_offset + 2 * (first + i))[0]
                       for i in range(count)})
    return leaves


def compare(payload, stage_path, bsp_path):
    mesh = read_payload(payload)
    expected = expected_stage(stage_path)
    if set(mesh["faces"]) != set(expected):
        raise ValueError("WMSH dropped or added a compiled world face")
    material_by_triangle = []
    for batch in mesh["batches"]:
        material_by_triangle.extend([mesh["materials"][batch[0]]] * (batch[2] // 3))
    if len(material_by_triangle) != len(mesh["faces"]):
        raise ValueError("WMSH material batches do not cover triangles")
    used = defaultdict(set)
    for triangle, face_id in enumerate(mesh["faces"]):
        corners = mesh["vertices"][3 * triangle:3 * triangle + 3]
        found = False
        for index, (material, positions, uv, lightmap, normal) in enumerate(expected[face_id]):
            if index in used[face_id] or material != material_by_triangle[triangle]:
                continue
            if all(close(corners[corner][0], positions[corner]) and
                   close(corners[corner][4], uv[corner]) and
                   close(corners[corner][5], lightmap[corner]) and
                   dot(corners[corner][1], normal) > 0.9999
                   for corner in range(3)):
                used[face_id].add(index)
                found = True
                break
        if not found:
            raise ValueError("WMSH triangle differs from World Stage face " + str(face_id))
    if any(len(used[face_id]) != len(triangles) for face_id, triangles in expected.items()):
        raise ValueError("WMSH omitted a World Stage triangle")
    leaves = expected_leaves(bsp_path)
    if len(leaves) != len(mesh["leaves"]):
        raise ValueError("WMSH leaf count differs from compiled BSP")
    for index, source_faces in enumerate(leaves):
        expected_refs = {meshlet for meshlet, face_id in enumerate(mesh["meshlet_faces"])
                         if face_id in source_faces}
        if set(mesh["leaves"][index]) != expected_refs:
            raise ValueError("WMSH leaf culling map differs from compiled BSP")
    return mesh["counts"]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wmsh", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--bsp2", type=Path)
    parser.add_argument("--container-legacy", type=Path)
    parser.add_argument("--negative-self-test", action="store_true")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    payload = args.wmsh.read_bytes()
    counts = compare(payload, args.stage, args.bsp)
    if bool(args.bsp2) != bool(args.container_legacy):
        raise ValueError("--bsp2 and --container-legacy must be used together")
    container_evidence = {}
    if args.bsp2:
        container = bsp2_reader.Bsp2File(args.bsp2.read_bytes(),
                                         known=(bsp2_reader.fourcc("WMSH"),))
        entry = container.by_id.get(bsp2_reader.fourcc("WMSH"))
        if (entry is None or entry["version"] != 1 or entry["flags"] != 0 or
                entry["alignment"] != 4096 or container.lump(entry) != payload or
                container.export_legacy() != args.container_legacy.read_bytes()):
            raise ValueError("BSP2 WMSH carriage or legacy export differs")
        container_evidence = {"bsp2_sha256": sha256(args.bsp2),
                              "container_legacy_sha256": sha256(args.container_legacy),
                              "bsp2_wmsh_byte_identical": True,
                              "bsp2_legacy_export_byte_identical": True}
    negative_rejected = False
    if args.negative_self_test:
        changed = bytearray(payload)
        face_offset = struct.unpack_from("<Q", changed, 72)[0]
        changed[face_offset] ^= 0x7F
        try:
            compare(bytes(changed), args.stage, args.bsp)
        except ValueError:
            negative_rejected = True
        if not negative_rejected:
            raise ValueError("seeded triangle-to-face corruption was accepted")
    evidence = {"status": "pass", "scope": "worldstage-wmsh-v1-independent",
                "wmsh_sha256": sha256(args.wmsh), "stage_sha256": sha256(args.stage),
                "bsp_sha256": sha256(args.bsp),
                "negative_wrong_face_rejected": negative_rejected, **container_evidence,
                **counts}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
