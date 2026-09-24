#!/usr/bin/env python3
"""Compare a compiled World Stage geometry layer with its source VBSP face lumps.

Run with the Python version used to build OpenUSD and set PYTHONPATH to its
installed Python directory. This is a bounded F2 geometry oracle, not a full
World Stage content or Cycles acceptance test.
"""

import argparse
import hashlib
import json
import math
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
    planes = lump(data, header, 1)
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
        plane_id = struct.unpack_from("<H", faces, offset)[0]
        if plane_id >= len(planes) // 20:
            raise ValueError("invalid BSP face plane")
        normal = struct.unpack_from("<fff", planes, plane_id * 20)
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
        vertex_ids = []
        for i in range(first_edge, first_edge + count):
            surfedge = struct.unpack_from("<i", surfedges, 4 * i)[0]
            edge_index = abs(surfedge)
            if edge_index >= len(edges) // 4:
                raise ValueError("invalid BSP surfedge")
            a, b = struct.unpack_from("<HH", edges, 4 * edge_index)
            vertex_index = b if surfedge < 0 else a
            if vertex_index >= len(vertices) // 12:
                raise ValueError("invalid BSP vertex")
            vertex_ids.append(vertex_index)
            points.append(struct.unpack_from("<fff", vertices, 12 * vertex_index))
        vectors = [struct.unpack_from("<ffff", texture, 16 * i) for i in range(4)]
        result[face_id] = {
            "points": points, "vertex_ids": vertex_ids,
            "chart": (chart_width + 1, chart_height + 1),
            "material": material_path, "smoothing": smoothing_group,
            "normal": normal,
            "material_uv": [(project(point, vectors[0]) / image_width,
                             project(point, vectors[1]) / image_height) for point in points],
            "lightmap_uv_luxels": [(project(point, vectors[2]) - lightmap_mins[0],
                                     project(point, vectors[3]) - lightmap_mins[1])
                                    for point in points],
        }
    return result


def surface_corners(vertex_ids):
    """Winding positions bounding a face's surface, or [] for no surface.

    Legacy vertex welding can fold a face thinner than the weld distance onto
    itself (vertices a b a c). Repeated vertices and spikes (a b a -> a) are
    removed by BSP vertex index; each survivor maps to its first position.
    """
    ring = list(vertex_ids)
    changed = True
    while changed and len(ring) >= 3:
        changed = False
        for i in range(len(ring)):
            following = ring[(i + 1) % len(ring)]
            if ring[i] == following:
                del ring[(i + 1) % len(ring)]
                changed = True
                break
            if ring[i] == ring[(i + 2) % len(ring)]:
                for index in sorted({(i + 1) % len(ring), (i + 2) % len(ring)}, reverse=True):
                    del ring[index]
                changed = True
                break
    return [vertex_ids.index(vertex) for vertex in ring] if len(ring) >= 3 else []


def newell_alignment(points, corners, normal):
    total = [0.0, 0.0, 0.0]
    for position, corner in enumerate(corners):
        a, b = points[corner], points[corners[(position + 1) % len(corners)]]
        total[0] += a[1] * b[2] - a[2] * b[1]
        total[1] += a[2] * b[0] - a[0] * b[2]
        total[2] += a[0] * b[1] - a[1] * b[0]
    return sum(total[i] * normal[i] for i in range(3))


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


def linear_channel(value):
    channel = value / 255.0
    return channel / 12.92 if channel <= 0.04045 else ((channel + 0.055) / 1.055) ** 2.4


def compare(bsp_faces, bsp_entities, stage):
    from pxr import UsdGeom, UsdLux

    world = stage.GetPrimAtPath("/World")
    if not world or not world.HasAPI("SourceWorldAPI"):
        raise ValueError("missing SourceWorldAPI")
    if (stage.GetDefaultPrim() != world or
            UsdGeom.GetStageUpAxis(stage) != UsdGeom.Tokens.z or
            UsdGeom.GetStageMetersPerUnit(stage) != 0.0254 or
            world.GetAttribute("source:schemaVersion").Get() != 4):
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
    degenerate_faces = []
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
        counts = list(mesh.GetFaceVertexCountsAttr().Get() or [])
        indices = list(mesh.GetFaceVertexIndicesAttr().Get() or [])
        normal = bsp_faces[face_id]["normal"]
        if not all(math.isfinite(value) for value in normal) or abs(
                math.sqrt(sum(value * value for value in normal)) - 1) > 1e-3:
            raise ValueError("BSP face has an invalid plane normal")
        corners = surface_corners(bsp_faces[face_id]["vertex_ids"])
        # Newell's normal: T-junction points can make the first three collinear.
        alignment = newell_alignment(expected, corners, normal) if corners else 0.0
        if not math.isfinite(alignment):
            raise ValueError("BSP face has a non-finite winding")
        if abs(alignment) < 1e-5:
            corners = []
            degenerate_faces.append(face_id)
        expected_indices = [corners[fan] for corner in range(1, len(corners) - 1)
                            for fan in ((0, corner + 1, corner) if alignment < 0
                                        else (0, corner, corner + 1))]
        if counts != [3] * max(0, len(corners) - 2) or indices != expected_indices:
            raise ValueError("world face triangulation diverges from BSP")
        if [tuple(value) for value in mesh.GetNormalsAttr().Get() or []] != [normal] * len(counts):
            raise ValueError("world face normal diverges from BSP plane")
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
    light_root = stage.GetPrimAtPath("/World/Lights")
    if not light_root:
        raise ValueError("World Stage lacks its lights scope")
    expected_lights = [(index, dict(record)) for index, record in enumerate(bsp_entities)
                       if dict(record).get("classname") == "light"]
    if len(list(light_root.GetChildren())) != len(expected_lights):
        raise ValueError("World Stage dropped a BSP point light")
    light_styles = {}
    for index, record in expected_lights:
        authored_styles = [value for key, value in bsp_entities[index] if key == "style"]
        if (len(authored_styles) > 1 or
                (authored_styles and not re.fullmatch(r"[0-9]+", authored_styles[0]))):
            raise ValueError("invalid compiled BSP light style")
        style = int(authored_styles[0]) if authored_styles else 0
        if style >= 255:
            raise ValueError("compiled BSP light style is out of range")
        light_styles[str(index)] = style
        prim = stage.GetPrimAtPath("/World/Lights/Light_" + str(index))
        if (not prim or not prim.IsA(UsdLux.SphereLight) or
                not prim.HasAPI("SourceLightAPI") or
                prim.GetAttribute("source:entityIndex").Get() != index or
                str(prim.GetAttribute("source:lightingPolicy").Get()) != "preview-v1"):
            raise ValueError("World Stage point light identity diverges from BSP")
        if prim.GetAttribute("source:styleId").Get() != style:
            raise ValueError("World Stage point light style diverges from BSP")
        origin = [float(value) for value in record["origin"].split()]
        source_light = [float(value) for value in record["_light"].split()]
        if len(origin) != 3 or len(source_light) != 4 or not all(
                math.isfinite(value) for value in origin + source_light):
            raise ValueError("invalid BSP light data")
        sphere = UsdLux.SphereLight(prim)
        operations = UsdGeom.Xformable(prim).GetOrderedXformOps()
        if (len(operations) != 1 or not close_pair(operations[0].Get(), origin)
                or abs(sphere.GetIntensityAttr().Get() - source_light[3]) > 1e-5
                or abs(sphere.GetRadiusAttr().Get() - 8.0) > 1e-5
                or not close_pair(sphere.GetColorAttr().Get(),
                                  [linear_channel(value) for value in source_light[:3]])):
            raise ValueError("World Stage point light values diverge from BSP")
    return {"faces": len(seen), "triangles": triangle_count,
            "faces_without_surface": sorted(degenerate_faces),
            "entities": len(bsp_entities), "point_lights": len(expected_lights),
            "light_styles": light_styles,
            "atlas_width": width, "atlas_height": height}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--negative-self-test", action="store_true")
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    from pxr import Gf, Usd

    try:
        stage = Usd.Stage.Open(str(args.stage))
        if stage is None:
            raise ValueError("OpenUSD could not open stage")
        faces = source_faces(args.bsp)
        entities = source_entities(args.bsp)
        evidence = compare(faces, entities, stage)
        if args.negative_self_test:
            stage.GetPrimAtPath("/World/Geometry/WorldSpawn/Mesh_" + str(min(faces))).SetActive(
                False)
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
            mesh = stage.GetPrimAtPath("/World/Geometry/WorldSpawn/Mesh_" + str(min(faces)))
            normals = mesh.GetAttribute("normals")
            normals.Set([Gf.Vec3f(-value[0], -value[1], -value[2])
                         for value in normals.Get()])
            try:
                compare(faces, entities, stage)
            except ValueError as error:
                if "world face normal diverges" not in str(error):
                    raise
                evidence["negative_bad_normal_rejected"] = True
            else:
                raise ValueError("bad normal negative control was accepted")
            stage.Reload()
            compare(faces, entities, stage)
            stage.GetPrimAtPath("/World/Entities/Entity_" + str(len(entities) - 1)).SetActive(
                False)
            try:
                compare(faces, entities, stage)
            except ValueError as error:
                if "dropped a BSP entity" not in str(error):
                    raise
                evidence["negative_missing_entity_rejected"] = True
            else:
                raise ValueError("missing entity negative control was accepted")
            stage.Reload()
            compare(faces, entities, stage)
            # Surface rule: a real face may not lose its triangles, and a face
            # welded to no surface may not gain any.
            folded = set(evidence["faces_without_surface"])
            cases = [("emptied_surface", min(set(faces) - folded), [], [])]
            if folded:
                cases.append(("folded_face_triangulated", min(folded), [3], [0, 1, 2]))
            for key, face_id, counts, indices in cases:
                mesh = stage.GetPrimAtPath("/World/Geometry/WorldSpawn/Mesh_" + str(face_id))
                mesh.GetAttribute("faceVertexCounts").Set(counts)
                mesh.GetAttribute("faceVertexIndices").Set(indices)
                try:
                    compare(faces, entities, stage)
                except ValueError as error:
                    if "triangulation diverges" not in str(error):
                        raise
                    evidence["negative_%s_rejected" % key] = True
                else:
                    raise ValueError("%s negative control was accepted" % key)
                stage.Reload()
                compare(faces, entities, stage)
            light_indices = [index for index, record in enumerate(entities)
                             if dict(record).get("classname") == "light"]
            if light_indices:
                light = stage.GetPrimAtPath("/World/Lights/Light_" + str(light_indices[0]))
                current_style = light.GetAttribute("source:styleId").Get()
                light.GetAttribute("source:styleId").Set(0 if current_style else 32)
                try:
                    compare(faces, entities, stage)
                except ValueError as error:
                    if "point light style diverges" not in str(error):
                        raise
                    evidence["negative_bad_light_style_rejected"] = True
                else:
                    raise ValueError("bad light style negative control was accepted")
                stage.Reload()
                compare(faces, entities, stage)
                stage.GetPrimAtPath("/World/Lights/Light_" + str(light_indices[0])).SetActive(
                    False)
                try:
                    compare(faces, entities, stage)
                except ValueError as error:
                    if "dropped a BSP point light" not in str(error):
                        raise
                    evidence["negative_missing_light_rejected"] = True
                else:
                    raise ValueError("missing light negative control was accepted")
        evidence["status"] = "pass"
        evidence["bsp_sha256"] = hashlib.sha256(args.bsp.read_bytes()).hexdigest()
        evidence["stage_sha256"] = hashlib.sha256(args.stage.read_bytes()).hexdigest()
    except (OSError, ValueError) as error:
        evidence = {"status": "fail", "reason": str(error)}
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
