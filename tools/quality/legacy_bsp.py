#!/usr/bin/env python3
"""Read the lighting-relevant contents of a legacy VBSP (versions 19-21).

The relight path (`legacy_bsp_scene.py`) reads compiled maps through this
module: the static world's faces (model 0) as polygons with their material
path, texinfo flags and material `st`; the solid world brushes' sides as
convex polygons (vrad's occluders: its shadow rays test the brush tree, so a
nodraw side blocks light although it has no face); the world lights vrad
compiled (either record layout) and the entity lump.

    python3 tools/quality/legacy_bsp.py info <map.bsp>

Only reads; every index is bounds-checked and a malformed map raises
`ValueError`.
"""

import argparse
import io
import json
import re
import struct
import sys
import zipfile
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from bsp2_reader import parse_legacy_header  # noqa: E402

LUMP_ENTITIES = 0
LUMP_PLANES = 1
LUMP_TEXDATA = 2
LUMP_VERTEXES = 3
LUMP_VISIBILITY = 4
LUMP_NODES = 5
LUMP_TEXINFO = 6
LUMP_FACES = 7
LUMP_LEAFS = 10
LUMP_EDGES = 12
LUMP_SURFEDGES = 13
LUMP_MODELS = 14
LUMP_WORLDLIGHTS = 15
LUMP_LEAFBRUSHES = 17
LUMP_BRUSHES = 18
LUMP_BRUSHSIDES = 19
LUMP_PAKFILE = 40
LUMP_TEXDATA_STRING_DATA = 43
LUMP_TEXDATA_STRING_TABLE = 44
LUMP_WORLDLIGHTS_HDR = 54

# public/bspflags.h
SURF_LIGHT = 0x0001
SURF_SKY2D = 0x0002
SURF_SKY = 0x0004
SURF_WARP = 0x0008
SURF_TRANS = 0x0010
SURF_TRIGGER = 0x0040
SURF_NODRAW = 0x0080
SURF_HINT = 0x0100
SURF_SKIP = 0x0200
SURF_NOLIGHT = 0x0400
CONTENTS_SOLID = 0x1
CONTENTS_WINDOW = 0x2
CONTENTS_OPAQUE = 0x80

# public/bspfile.h emittype_t
EMIT_SURFACE, EMIT_POINT, EMIT_SPOTLIGHT, EMIT_SKYLIGHT, EMIT_QUAKELIGHT, EMIT_SKYAMBIENT = \
    range(6)
EMIT_NAMES = {EMIT_SURFACE: "surface", EMIT_POINT: "point", EMIT_SPOTLIGHT: "spot",
              EMIT_SKYLIGHT: "sky", EMIT_QUAKELIGHT: "quake", EMIT_SKYAMBIENT: "sky_ambient"}
# dworldlight_t: version 1 adds shadow_cast_offset after normal.
WORLDLIGHT_V1 = struct.Struct("<3f3f3f3fiiifffffffiii")
WORLDLIGHT_V0 = struct.Struct("<3f3f3fiiifffffffiii")
FACE = struct.Struct("<HBBihhhh4BiF2i2iiHHI".replace("F", "f"))
TEXINFO_BYTES = 72
TEXDATA_BYTES = 32
PLANE_BYTES = 20
NODE_BYTES = 32
LEAF_BYTES = 32
BRUSH_BYTES = 12
BRUSHSIDE_BYTES = 8
MODEL_BYTES = 48
HUGE = 65536.0


class LegacyBsp:
    """A legacy VBSP file's lumps, bounds-checked on access."""

    def __init__(self, data):
        self.data = data
        header = parse_legacy_header(data, len(data))
        if header["version"] not in (19, 20, 21):
            raise ValueError("unsupported VBSP version %d" % header["version"])
        self.version = header["version"]
        self.lumps = header["lumps"]

    @classmethod
    def read(cls, path):
        return cls(Path(path).read_bytes())

    def lump(self, index, record=1):
        offset, length, _version, _uncompressed = self.lumps[index]
        if offset < 0 or length < 0 or offset + length > len(self.data):
            raise ValueError("lump %d is out of bounds" % index)
        if length % record:
            raise ValueError("lump %d has a partial record" % index)
        return self.data[offset:offset + length]

    def lump_version(self, index):
        return self.lumps[index][2]

    # entities ----------------------------------------------------------
    def entities(self):
        text = self.lump(LUMP_ENTITIES).rstrip(b"\0").decode("latin-1")
        result = []
        for block in re.findall(r"\{([^{}]*)\}", text):
            pairs = re.findall(r'"([^"]*)"\s+"([^"]*)"', block)
            entity = {}
            for key, value in pairs:
                # Repeated keys (entity outputs) keep every value.
                if key in entity:
                    previous = entity[key]
                    entity[key] = (previous if isinstance(previous, list) else [previous]) + \
                        [value]
                else:
                    entity[key] = value
            result.append(entity)
        if not result or result[0].get("classname") != "worldspawn":
            raise ValueError("entity lump does not start with worldspawn")
        return result

    def pakfile(self):
        """The embedded pak lump as a ZipFile (possibly empty)."""
        data = self.lump(LUMP_PAKFILE)
        if not data:
            return None
        return zipfile.ZipFile(io.BytesIO(data))

    # geometry ----------------------------------------------------------
    def planes(self):
        raw = self.lump(LUMP_PLANES, PLANE_BYTES)
        values = np.frombuffer(raw, dtype=np.dtype([("normal", "<f4", 3), ("dist", "<f4"),
                                                    ("type", "<i4")]))
        return values["normal"].astype(np.float64), values["dist"].astype(np.float64)

    def material_names(self):
        table = self.lump(LUMP_TEXDATA_STRING_TABLE, 4)
        strings = self.lump(LUMP_TEXDATA_STRING_DATA)
        names = []
        for offset in struct.unpack("<%di" % (len(table) // 4), table):
            end = strings.find(b"\0", offset)
            if offset < 0 or end < 0:
                raise ValueError("texdata string table is malformed")
            names.append(strings[offset:end].decode("latin-1"))
        return names

    def texdata(self):
        """[{name, reflectivity, width, height}]"""
        raw = self.lump(LUMP_TEXDATA, TEXDATA_BYTES)
        names = self.material_names()
        result = []
        for index in range(len(raw) // TEXDATA_BYTES):
            reflectivity = struct.unpack_from("<3f", raw, index * TEXDATA_BYTES)
            name_id, width, height = struct.unpack_from("<3i", raw, index * TEXDATA_BYTES + 12)
            if not 0 <= name_id < len(names) or width <= 0 or height <= 0:
                raise ValueError("texdata %d is malformed" % index)
            result.append({"name": names[name_id], "reflectivity": reflectivity,
                           "width": width, "height": height})
        return result

    def texinfo(self):
        """[{vectors (2x4 texture), flags, texdata}]"""
        raw = self.lump(LUMP_TEXINFO, TEXINFO_BYTES)
        count = len(self.lump(LUMP_TEXDATA, TEXDATA_BYTES)) // TEXDATA_BYTES
        result = []
        for index in range(len(raw) // TEXINFO_BYTES):
            base = index * TEXINFO_BYTES
            vectors = np.array(struct.unpack_from("<8f", raw, base), np.float64).reshape(2, 4)
            flags, texdata = struct.unpack_from("<ii", raw, base + 64)
            if not -1 <= texdata < count:
                raise ValueError("texinfo %d names a missing texdata" % index)
            result.append({"vectors": vectors, "flags": flags, "texdata": texdata})
        return result

    def models(self):
        raw = self.lump(LUMP_MODELS, MODEL_BYTES)
        return [dict(zip(("headnode", "firstface", "numfaces"),
                         struct.unpack_from("<3i", raw, index * MODEL_BYTES + 36)))
                for index in range(len(raw) // MODEL_BYTES)]

    def world_faces(self):
        """Model 0's faces: [{index, points (N,3), vertices (N BSP vertex
        indices), plane (BSP plane index), plane_normal, texinfo, dispinfo,
        styles, area}]; windings are
        clockwise seen from the front. Faces share vertices by index, as vbsp
        wrote them."""
        faces = self.lump(LUMP_FACES, FACE.size)
        edges = np.frombuffer(self.lump(LUMP_EDGES, 4), dtype="<u2").reshape(-1, 2)
        surfedges = np.frombuffer(self.lump(LUMP_SURFEDGES, 4), dtype="<i4")
        vertices = np.frombuffer(self.lump(LUMP_VERTEXES, 12), dtype="<f4").reshape(-1, 3)
        normals, _ = self.planes()
        texinfo_count = len(self.lump(LUMP_TEXINFO, TEXINFO_BYTES)) // TEXINFO_BYTES
        world = self.models()[0]
        first, count = world["firstface"], world["numfaces"]
        if first < 0 or count < 0 or first + count > len(faces) // FACE.size:
            raise ValueError("world model face range is out of bounds")
        result = []
        for index in range(first, first + count):
            (plane, side, _on_node, first_edge, edge_count, texinfo, dispinfo, _fog,
             s0, s1, s2, s3, _light_offset, area, *_rest) = FACE.unpack_from(
                 faces, index * FACE.size)
            if plane >= len(normals) or not 0 <= texinfo < texinfo_count:
                raise ValueError("face %d has a bad plane or texinfo" % index)
            if edge_count < 3 or first_edge < 0 or first_edge + edge_count > len(surfedges):
                raise ValueError("face %d has a bad edge range" % index)
            ids = surfedges[first_edge:first_edge + edge_count]
            if np.abs(ids).max() >= len(edges):
                raise ValueError("face %d names a missing edge" % index)
            corner = np.where(ids >= 0, edges[np.abs(ids), 0], edges[np.abs(ids), 1])
            if corner.max() >= len(vertices):
                raise ValueError("face %d names a missing vertex" % index)
            # vbsp stores each plane with its flip beside it (planenum ^ 1) and
            # gives a face its own oriented plane; `side` only records the flip.
            normal = normals[plane]
            result.append({"index": index, "points": vertices[corner].astype(np.float64),
                           "vertices": corner.astype(np.int64), "plane": int(plane),
                           "plane_normal": normal, "texinfo": texinfo,
                           "dispinfo": dispinfo, "styles": (s0, s1, s2, s3), "area": area})
        return result

    def world_brushes(self):
        """The brushes reachable from model 0's head node, as [{contents, sides:
        [(plane, texinfo, bevel)]}] (brush entities have their own trees)."""
        nodes = self.lump(LUMP_NODES, NODE_BYTES)
        leaves = self.lump(LUMP_LEAFS, LEAF_BYTES)
        if self.lump_version(LUMP_LEAFS) != 1:
            raise ValueError("leaf lump version %d is unsupported" %
                             self.lump_version(LUMP_LEAFS))
        leafbrushes = np.frombuffer(self.lump(LUMP_LEAFBRUSHES, 2), dtype="<u2")
        brushes = self.lump(LUMP_BRUSHES, BRUSH_BYTES)
        sides = self.lump(LUMP_BRUSHSIDES, BRUSHSIDE_BYTES)
        node_count, leaf_count = len(nodes) // NODE_BYTES, len(leaves) // LEAF_BYTES
        found = set()
        stack = [self.models()[0]["headnode"]]
        visited = 0
        while stack:
            node = stack.pop()
            visited += 1
            if visited > node_count + leaf_count:
                raise ValueError("BSP node tree has a cycle")
            if node < 0:
                leaf = -1 - node
                if leaf >= leaf_count:
                    raise ValueError("node names a missing leaf")
                first, count = struct.unpack_from("<HH", leaves, leaf * LEAF_BYTES + 24)
                if first + count > len(leafbrushes):
                    raise ValueError("leaf %d has a bad brush range" % leaf)
                found.update(int(b) for b in leafbrushes[first:first + count])
                continue
            if node >= node_count:
                raise ValueError("node tree names a missing node")
            stack.extend(struct.unpack_from("<ii", nodes, node * NODE_BYTES + 4))
        result = []
        for brush in sorted(found):
            if brush >= len(brushes) // BRUSH_BYTES:
                raise ValueError("leaf names a missing brush")
            first, count, contents = struct.unpack_from("<3i", brushes, brush * BRUSH_BYTES)
            if first < 0 or count < 0 or first + count > len(sides) // BRUSHSIDE_BYTES:
                raise ValueError("brush %d has a bad side range" % brush)
            result.append({"index": brush, "contents": contents,
                           "sides": [struct.unpack_from("<Hhhh", sides, i * BRUSHSIDE_BYTES)
                                     for i in range(first, first + count)]})
        return result

    def brush_side_polygons(self, brush):
        """Convex polygon of each non-bevel side of a brush: [(side, points)].
        Brush planes face outward; the solid is where n . x <= d."""
        normals, dists = self.planes()
        planes = [(normals[p], dists[p]) for p, _t, _d, bevel in brush["sides"]]
        result = []
        for index, (plane, _texinfo, _disp, bevel) in enumerate(brush["sides"]):
            if bevel:
                continue
            winding = base_winding(normals[plane], dists[plane])
            for other, (normal, dist) in enumerate(planes):
                if other == index or winding is None:
                    continue
                if np.allclose(normal, normals[plane]) and abs(dist - dists[plane]) < 1e-3:
                    continue
                winding = clip_winding(winding, normal, dist)
            if winding is not None and len(winding) >= 3:
                result.append((index, winding))
        return result

    # visibility --------------------------------------------------------
    def leaves(self):
        """Every leaf's contents, cluster (-1: none) and bounds, as arrays."""
        if self.lump_version(LUMP_LEAFS) != 1:
            raise ValueError("leaf lump version %d is unsupported" %
                             self.lump_version(LUMP_LEAFS))
        raw = np.frombuffer(self.lump(LUMP_LEAFS, LEAF_BYTES), dtype=np.dtype([
            ("contents", "<i4"), ("cluster", "<i2"), ("flags", "<i2"), ("mins", "<i2", 3),
            ("maxs", "<i2", 3), ("rest", "V12")]))
        return (raw["contents"].astype(np.int64), raw["cluster"].astype(np.int64),
                raw["mins"].astype(np.float64), raw["maxs"].astype(np.float64))

    def point_leaf(self, point):
        """The leaf of model 0's tree holding `point` (front of a plane on ties)."""
        nodes = self.lump(LUMP_NODES, NODE_BYTES)
        normals, dists = self.planes()
        node = self.models()[0]["headnode"]
        for _ in range(len(nodes) // NODE_BYTES + 1):
            if node < 0:
                return -1 - node
            plane, front, back = struct.unpack_from("<iii", nodes, node * NODE_BYTES)
            node = front if np.dot(normals[plane], point) - dists[plane] >= 0 else back
        raise ValueError("BSP node tree has a cycle")

    def visibility(self):
        """The potentially visible set: a (clusters, clusters) bool matrix,
        row c the clusters visible from cluster c; None without vis data."""
        data = self.lump(LUMP_VISIBILITY)
        if len(data) < 4:
            return None
        count = struct.unpack_from("<i", data, 0)[0]
        if count <= 0 or 4 + 8 * count > len(data):
            raise ValueError("visibility lump is malformed")
        width = (count + 7) // 8
        pvs = np.zeros((count, count), dtype=bool)
        for cluster in range(count):
            offset = struct.unpack_from("<i", data, 4 + 8 * cluster)[0]
            row = bytearray()
            while len(row) < width:
                if offset >= len(data):
                    raise ValueError("visibility row %d runs past the lump" % cluster)
                byte = data[offset]
                if byte:
                    row.append(byte)
                    offset += 1
                else:
                    if offset + 1 >= len(data):
                        raise ValueError("visibility row %d is truncated" % cluster)
                    row.extend(b"\0" * data[offset + 1])
                    offset += 2
            bits = np.unpackbits(np.frombuffer(bytes(row[:width]), np.uint8), bitorder="little")
            pvs[cluster] = bits[:count].astype(bool)
        # A cluster sees itself (vvis sets its own bit; be sure).
        pvs[np.arange(count), np.arange(count)] = True
        return pvs

    # lights ------------------------------------------------------------
    def world_lights(self, hdr=True):
        """dworldlight_t records (HDR lump when present and `hdr`)."""
        index = LUMP_WORLDLIGHTS_HDR if hdr and self.lumps[LUMP_WORLDLIGHTS_HDR][1] else \
            LUMP_WORLDLIGHTS
        raw = self.lump(index)
        # The compile tools write version-1 records under lump version 0; the
        # length decides (as bsp_worldlights.py does).
        record = WORLDLIGHT_V1 if self.lump_version(index) >= 1 or \
            len(raw) % WORLDLIGHT_V0.size else WORLDLIGHT_V0
        if len(raw) % record.size:
            raise ValueError("world light lump %d has a partial record" % index)
        result = []
        for i in range(len(raw) // record.size):
            values = record.unpack_from(raw, i * record.size)
            if record is WORLDLIGHT_V0:
                values = values[:9] + (0.0, 0.0, 0.0) + values[9:]
            (x, y, z, r, g, b, nx, ny, nz, _sx, _sy, _sz, cluster, kind, style, stopdot,
             stopdot2, exponent, radius, constant, linear, quadratic, flags, texinfo,
             owner) = values
            if kind not in EMIT_NAMES:
                raise ValueError("world light %d has unknown type %d" % (i, kind))
            result.append({"index": i, "origin": (x, y, z), "intensity": (r, g, b),
                           "normal": (nx, ny, nz), "cluster": cluster,
                           "type": EMIT_NAMES[kind], "style": style, "stopdot": stopdot,
                           "stopdot2": stopdot2, "exponent": exponent, "radius": radius,
                           "attenuation": (constant, linear, quadratic), "flags": flags,
                           "texinfo": texinfo, "owner": owner})
        return result, index


def base_winding(normal, dist):
    """A huge square on the plane n . x = d, wound counter-clockwise seen
    from the front (the normal's side)."""
    axis = int(np.argmax(np.abs(normal)))
    up = np.array((0.0, 0.0, 1.0)) if axis != 2 else np.array((1.0, 0.0, 0.0))
    up = up - normal * np.dot(up, normal)
    up /= np.linalg.norm(up)
    right = np.cross(up, normal)
    centre = normal * dist
    return np.array([centre + (-right + up) * HUGE, centre + (right + up) * HUGE,
                     centre + (right - up) * HUGE, centre + (-right - up) * HUGE])


def clip_winding(points, normal, dist, epsilon=1e-3):
    """Keep the part of a convex winding with n . x <= d; None when empty."""
    side = points @ normal - dist
    if (side <= epsilon).all():
        return points
    if (side >= -epsilon).all():
        return None
    result = []
    count = len(points)
    for i in range(count):
        a, b = points[i], points[(i + 1) % count]
        sa, sb = side[i], side[(i + 1) % count]
        if sa <= epsilon:
            result.append(a)
        if (sa < -epsilon and sb > epsilon) or (sa > epsilon and sb < -epsilon):
            t = sa / (sa - sb)
            result.append(a + (b - a) * t)
    return np.array(result) if len(result) >= 3 else None


def polygon_area(points):
    total = np.zeros(3)
    for i in range(1, len(points) - 1):
        total += np.cross(points[i] - points[0], points[i + 1] - points[0])
    return 0.5 * float(np.linalg.norm(total))


def cmd_info(args):
    bsp = LegacyBsp.read(args.bsp)
    texinfo = bsp.texinfo()
    faces = bsp.world_faces()
    lights, lump = bsp.world_lights()
    brushes = bsp.world_brushes()
    kinds = {}
    for light in lights:
        kinds[light["type"]] = kinds.get(light["type"], 0) + 1
    print(json.dumps({"version": bsp.version, "world_faces": len(faces),
                      "displacement_faces": sum(f["dispinfo"] != -1 for f in faces),
                      "sky_faces": sum(bool(texinfo[f["texinfo"]]["flags"] & SURF_SKY)
                                       for f in faces),
                      "world_brushes": len(brushes), "world_light_lump": lump,
                      "world_lights": kinds, "models": len(bsp.models()),
                      "entities": len(bsp.entities())}, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    info = commands.add_parser("info")
    info.add_argument("bsp", type=Path)
    info.set_defaults(func=cmd_info)
    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
