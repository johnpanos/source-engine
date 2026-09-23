#!/usr/bin/env python3
"""Compare a compiled World Stage geometry layer with its source VBSP face lumps.

Run with the Python version used to build OpenUSD and set PYTHONPATH to its
installed Python directory. This is a bounded F2 geometry oracle, not a full
World Stage content or Cycles acceptance test.
"""

import argparse
import json
import re
import struct
import sys
from pathlib import Path

from bsp2_reader import parse_legacy_header


def lump(data, header, index):
    offset, length, _version, _uncompressed = header["lumps"][index]
    return data[offset:offset + length]


def source_faces(path):
    data = path.read_bytes()
    header = parse_legacy_header(data, len(data))
    if header["version"] not in (19, 20, 21):
        raise ValueError("unsupported VBSP face version")
    models = lump(data, header, 14)
    faces = lump(data, header, 7)
    edges = lump(data, header, 12)
    surfedges = lump(data, header, 13)
    vertices = lump(data, header, 3)
    texinfo = lump(data, header, 6)
    texdata = lump(data, header, 2)
    string_table = lump(data, header, 44)
    string_data = lump(data, header, 43)
    if len(models) != 48:
        raise ValueError("this geometry slice requires one world model")
    first_face, face_count = struct.unpack_from("<ii", models, 40)
    if face_count <= 0 or first_face < 0 or first_face + face_count > len(faces) // 56:
        raise ValueError("invalid world face range")
    result = {}
    for face_id in range(first_face, first_face + face_count):
        offset = face_id * 56
        first_edge, count = struct.unpack_from("<ih", faces, offset + 4)
        chart_width, chart_height = struct.unpack_from("<ii", faces, offset + 36)
        texinfo_id = struct.unpack_from("<h", faces, offset + 10)[0]
        smoothing_group = struct.unpack_from("<I", faces, offset + 52)[0]
        lightmap_mins = struct.unpack_from("<ii", faces, offset + 28)
        if count < 3 or first_edge < 0 or first_edge + count > len(surfedges) // 4:
            raise ValueError("invalid BSP face edges")
        if texinfo_id < 0 or texinfo_id >= len(texinfo) // 72:
            raise ValueError("invalid BSP texinfo")
        texture = texinfo[72 * texinfo_id:72 * (texinfo_id + 1)]
        material_id = struct.unpack_from("<i", texture, 68)[0]
        if material_id < 0 or material_id >= len(texdata) // 32:
            raise ValueError("invalid BSP texdata")
        material_info = texdata[32 * material_id:32 * (material_id + 1)]
        string_id, image_width, image_height = struct.unpack_from("<iii", material_info, 12)
        if (string_id < 0 or string_id >= len(string_table) // 4
                or image_width <= 0 or image_height <= 0):
            raise ValueError("invalid BSP material data")
        string_offset = struct.unpack_from("<i", string_table, string_id * 4)[0]
        if string_offset < 0 or string_offset >= len(string_data):
            raise ValueError("invalid BSP material name offset")
        end = string_data.find(b"\0", string_offset)
        if end < 0:
            raise ValueError("unterminated BSP material name")
        material_path = string_data[string_offset:end].decode("utf-8")
        points = []
        for i in range(first_edge, first_edge + count):
            surfedge = struct.unpack_from("<i", surfedges, 4 * i)[0]
            edge_index = abs(surfedge)
            if edge_index >= len(edges) // 4:
                raise ValueError("invalid BSP surfedge")
            a, b = struct.unpack_from("<HH", edges, 4 * edge_index)
            vertex_index = b if surfedge < 0 else a
            if vertex_index >= len(vertices) // 12:
                raise ValueError("invalid BSP vertex")
            points.append(struct.unpack_from("<fff", vertices, 12 * vertex_index))
        vectors = [struct.unpack_from("<ffff", texture, 16 * i) for i in range(4)]
        result[face_id] = {
            "points": points, "chart": (chart_width + 1, chart_height + 1),
            "material": material_path, "smoothing": smoothing_group,
            "material_uv": [(project(point, vectors[0]) / image_width,
                             project(point, vectors[1]) / image_height) for point in points],
            "lightmap_uv_luxels": [(project(point, vectors[2]) - lightmap_mins[0],
                                     project(point, vectors[3]) - lightmap_mins[1])
                                    for point in points],
        }
    return result


def source_entities(path):
    data = path.read_bytes()
    header = parse_legacy_header(data, len(data))
    payload = lump(data, header, 0)
    if not payload.endswith(b"\0"):
        raise ValueError("BSP entity lump has no final NUL")
    records = []
    current = None
    for line in payload[:-1].decode("utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        if line == "{":
            if current is not None:
                raise ValueError("nested BSP entity")
            current = []
        elif line == "}":
            if current is None:
                raise ValueError("unexpected BSP entity close")
            records.append(current)
            current = None
        else:
            match = re.fullmatch(r'"([^\"]*)"\s+"([^\"]*)"', line)
            if current is None or match is None:
                raise ValueError("malformed BSP entity key/value line")
            current.append(match.groups())
    if current is not None or not records:
        raise ValueError("BSP entity lump has an incomplete or empty entity list")
    return records


def project(point, vector):
    return sum(point[i] * vector[i] for i in range(3)) + vector[3]


def close_pair(actual, expected, tolerance=1e-5):
    return all(abs(a - b) <= tolerance for a, b in zip(actual, expected))


def compare(bsp_faces, bsp_entities, stage):
    from pxr import UsdGeom

    world = stage.GetPrimAtPath("/World")
    if not world or not world.HasAPI("SourceWorldAPI"):
        raise ValueError("missing SourceWorldAPI")
    if (stage.GetDefaultPrim() != world or
            UsdGeom.GetStageUpAxis(stage) != UsdGeom.Tokens.z or
            UsdGeom.GetStageMetersPerUnit(stage) != 0.0254 or
            world.GetAttribute("source:schemaVersion").Get() != 2):
        raise ValueError("World Stage identity, units or schema version diverges")
    width = world.GetAttribute("source:lightmapAtlasWidth").Get()
    height = world.GetAttribute("source:lightmapAtlasHeight").Get()
    ids = list(world.GetAttribute("source:chartFaceIds").Get())
    rects = list(world.GetAttribute("source:chartRects").Get())
    if not width or not height or len(ids) != len(rects) or set(ids) != set(bsp_faces):
        raise ValueError("invalid lightmap chart table")
    chart_by_face = dict(zip(ids, rects))
    if len(chart_by_face) != len(ids):
        raise ValueError("duplicate lightmap chart face")
    for face_id, rect in chart_by_face.items():
        x, y, w, h = rect
        if ((w, h) != bsp_faces[face_id]["chart"] or x < 0 or y < 0
                or x + w > width or y + h > height):
            raise ValueError("chart extent diverges from BSP face")
    for i, left in enumerate(rects):
        for right in rects[i + 1:]:
            if (left[0] < right[0] + right[2] and right[0] < left[0] + left[2]
                    and left[1] < right[1] + right[3] and right[1] < left[1] + left[3]):
                raise ValueError("lightmap charts overlap")

    mesh_root = stage.GetPrimAtPath("/World/Geometry/WorldSpawn")
    if not mesh_root:
        raise ValueError("missing world geometry scope")
    seen = set()
    triangle_count = 0
    for prim in mesh_root.GetChildren():
        if not prim.IsA(UsdGeom.Mesh) or not prim.HasAPI("SourceMeshAPI"):
            raise ValueError("unexpected world geometry prim")
        if not prim.GetName().startswith("Mesh_"):
            raise ValueError("unidentified world mesh")
        face_id = int(prim.GetName()[5:])
        if face_id not in bsp_faces or face_id in seen:
            raise ValueError("unexpected or duplicate world face")
        seen.add(face_id)
        mesh = UsdGeom.Mesh(prim)
        actual = [tuple(point) for point in mesh.GetPointsAttr().Get()]
        expected = bsp_faces[face_id]["points"]
        if actual != expected:
            raise ValueError("world face points diverge from BSP")
        counts = list(mesh.GetFaceVertexCountsAttr().Get())
        indices = list(mesh.GetFaceVertexIndicesAttr().Get())
        expected_indices = [vertex for corner in range(1, len(expected) - 1)
                            for vertex in (0, corner, corner + 1)]
        if counts != [3] * (len(expected) - 2) or indices != expected_indices:
            raise ValueError("world face triangulation diverges from BSP")
        triangle_count += len(counts)
        for attr, value in (("primvars:source:faceId", face_id),
                            ("primvars:source:lightmapChartId", face_id)):
            if list(prim.GetAttribute(attr).Get()) != [value] * len(counts):
                raise ValueError("world face identity diverges from BSP")
        if not prim.GetAttribute("source:materialPath").Get():
            raise ValueError("world face lacks material path")
        if prim.GetAttribute("source:materialPath").Get() != bsp_faces[face_id]["material"]:
            raise ValueError("world face material path diverges from BSP")
        if list(prim.GetAttribute("primvars:source:smoothingGroup").Get()) != [
                bsp_faces[face_id]["smoothing"]] * len(counts):
            raise ValueError("world face smoothing group diverges from BSP")
        material_uv = prim.GetAttribute("primvars:st").Get()
        if (len(material_uv) != len(indices) or any(
                not close_pair(material_uv[corner], bsp_faces[face_id]["material_uv"][vertex])
                for corner, vertex in enumerate(indices))):
            raise ValueError("world face material UV diverges from BSP")
        uv = prim.GetAttribute("primvars:lightmap:st").Get()
        if len(uv) != len(indices):
            raise ValueError("lightmap UV cardinality differs from triangle corners")
        x, y, w, h = chart_by_face[face_id]
        for corner, (s, t) in enumerate(uv):
            if not (x / width <= s <= (x + w) / width and
                    y / height <= t <= (y + h) / height):
                raise ValueError("lightmap UV escapes its face chart")
            luxel_s, luxel_t = bsp_faces[face_id]["lightmap_uv_luxels"][indices[corner]]
            if not close_pair((s, t), ((x + luxel_s + 0.5) / width,
                                       (y + luxel_t + 0.5) / height)):
                raise ValueError("world face lightmap UV diverges from BSP")
    if seen != set(bsp_faces):
        raise ValueError("World Stage dropped a BSP face")

    entity_root = stage.GetPrimAtPath("/World/Entities")
    if not entity_root:
        raise ValueError("World Stage lacks compiled entities")
    entity_prims = list(entity_root.GetChildren())
    if len(entity_prims) != len(bsp_entities):
        raise ValueError("World Stage dropped a BSP entity")
    for index, record in enumerate(bsp_entities):
        prim = stage.GetPrimAtPath("/World/Entities/Entity_" + str(index))
        if (not prim or not prim.HasAPI("SourceEntityAPI") or
                prim.GetAttribute("source:entityIndex").Get() != index):
            raise ValueError("World Stage entity identity diverges from BSP")
        keys = prim.GetAttribute("source:keyNames").Get()
        values = prim.GetAttribute("source:keyValues").Get()
        if list(zip(keys, values)) != record:
            raise ValueError("World Stage entity records diverge from BSP")
    return {"faces": len(seen), "triangles": triangle_count,
            "entities": len(bsp_entities), "atlas_width": width, "atlas_height": height}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--negative-self-test", action="store_true")
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    from pxr import Usd

    try:
        stage = Usd.Stage.Open(str(args.stage))
        if stage is None:
            raise ValueError("OpenUSD could not open stage")
        faces = source_faces(args.bsp)
        entities = source_entities(args.bsp)
        evidence = compare(faces, entities, stage)
        if args.negative_self_test:
            stage.RemovePrim("/World/Geometry/WorldSpawn/Mesh_" + str(min(faces)))
            try:
                compare(faces, entities, stage)
            except ValueError as error:
                if "dropped a BSP face" not in str(error):
                    raise
                evidence["negative_missing_face_rejected"] = True
            else:
                raise ValueError("missing face negative control was accepted")
            stage.Reload()
            compare(faces, entities, stage)
            stage.RemovePrim("/World/Entities/Entity_" + str(len(entities) - 1))
            try:
                compare(faces, entities, stage)
            except ValueError as error:
                if "dropped a BSP entity" not in str(error):
                    raise
                evidence["negative_missing_entity_rejected"] = True
            else:
                raise ValueError("missing entity negative control was accepted")
        evidence["status"] = "pass"
    except (OSError, ValueError) as error:
        evidence = {"status": "fail", "reason": str(error)}
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
