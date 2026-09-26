#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Independent checks of a USD-compiled BSP2 map (RFC 0009 U1-U2, roadmap R59).
#
#   python3 tools/quality/usd_map_check.py check MAP.bsp \
#       --report authoring-report.json --provenance provenance.json \
#       --runtime run/runtime [--json out.json]
#
# The oracle is the authored stage as the U0 validator reports it (its
# `objects` table, with Source-space faces), not the compiler: this module
# shares no code with usd_map_compile.py or vbsp. It reads the BSP2 with the
# independent container reader (bsp2_reader.py) and the legacy lumps with
# legacy_bsp.py plus its own decoders, and checks:
#
#   container   the BSP2 directory, hashes and legacy structures validate;
#   entities    every authored entity record (class, origin, angles, model,
#               keys, authored id, targetname) and nothing else; the light's
#               _light is recomputed from the profile's light policy;
#   roles       each placement role keeps its own contract: prop_static only in
#               the static-prop lump (solid per its collision, shadows baked);
#               prop_dynamic a named, non-physics entity with its solid and
#               default animation; prop_physics an entity with .phy collision;
#               a trigger's touch filter and every authored connection (to the
#               target's name, with a declared input); a func_movelinear's move
#               recomputed from its authored displacement and duration, and its
#               faces with their authored materials;
#   props       the static-prop game lump holds exactly the authored
#               prop_static placements; models exist with the collision their
#               role needs;
#   brushes     each authored solid is its provenance brush, side for side
#               (planes, contents, material);
#   surfaces    every point of an authored face whose front is open space is
#               covered by an output face with the authored material,
#               texture mapping and provenance side key, and every drawn
#               output face lies on an authored face;
#   collision   point contents and segment/box traces against the brushes,
#               and a vphysics collision model for the world and each brush
#               entity;
#   visibility  vvis data exists with more than one cluster; open points get a
#               cluster, solid points none; clusters with a clear line of
#               sight see each other, and regions sealed from each other do
#               not;
#   lighting    vrad lit the world faces and compiled the authored light;
#               the floor where a prop_static's top casts its shadow is darker
#               than the unoccluded point mirrored across the light.
#
# Exit status 0 only when every check passes; the command prints the checks-v1
# record. Python 3 standard library plus numpy (legacy_bsp.py).
#
# ============================================================================

import argparse
import hashlib
import io
import json
import math
import random
import struct
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))

import bsp2_reader  # noqa: E402
import legacy_bsp  # noqa: E402
import source_model  # noqa: E402
from conformance_result import Checks  # noqa: E402

CHECK_SCHEMA = "source-usd-map-check/v1"
COMPILE_PROFILE = ROOT / "quality" / "usd_authoring" / "source_compile_v1.json"
LUMP_FACEIDS = 11
LUMP_PHYSCOLLIDE = 29
LUMP_GAME_LUMP = 35
LUMP_LIGHTING = 8
CONTENTS_SOLID = 0x1
MASK_SOLID = 0x1 | 0x2 | 0x8 | 0x4000 | 0x2000000 | 0x10000
SPRP = struct.unpack(">i", b"sprp")[0]
STATIC_PROP_NO_SHADOW = 0x10
SURF_BUMPLIGHT = 0x800
# The floor in a prop's baked shadow keeps at most this fraction of the light
# at its unoccluded mirror point (direct light is gone; bounce light remains).
SHADOW_RATIO = 0.6
# Static prop record sizes CStaticPropMgr::UnserializeModels reads, by (BSP
# version, lump version): StaticPropLumpV10_21_t and StaticPropLumpV10_t.
ENGINE_STATIC_PROP_RECORD = {(21, 10): 76, (20, 10): 72, (19, 10): 72}
PLAYER_HULL = ((-16.0, -16.0, 0.0), (16.0, 16.0, 72.0))
TOLERANCE = 0.01          # Source units for placements
PLANE_TOLERANCE = 0.02    # Source units for planes and polygon containment
ST_TOLERANCE = 2e-3       # texture repeats
SAMPLE_SPACING = 16.0
SEED = 20260925


def dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def close(a, b, tolerance=TOLERANCE):
    return len(a) == len(b) and all(abs(x - y) <= tolerance for x, y in zip(a, b))


def angle_close(a, b, tolerance=TOLERANCE):
    return all(abs((x - y + 180.0) % 360.0 - 180.0) <= tolerance for x, y in zip(a, b))


def numbers(text):
    return [float(x) for x in text.split()]


def inside_polygon(point, normal, polygon, margin=PLANE_TOLERANCE):
    """`point` on the polygon's plane lies inside the convex polygon (counter-
    clockwise around `normal`)."""
    for i, a in enumerate(polygon):
        b = polygon[(i + 1) % len(polygon)]
        edge = sub(b, a)
        inward = cross(normal, edge)
        length = math.sqrt(dot(inward, inward))
        if length == 0:
            continue
        if dot(sub(point, a), inward) / length < -margin:
            return False
    return True


def expected_light(record, profile):
    """The profile's light policy, recomputed here from its declaration."""
    policy = profile["light"]
    inputs = record["inputs"]
    radius_m = inputs["radius"] * policy["meters_per_source_unit"]
    candela = record["intensity"] * 2.0 ** inputs["exposure"] * inputs["diffuse"] * \
        math.pi * radius_m ** 2
    peak = max(record["color"])
    reference = policy["reference_distance_units"] * policy["meters_per_source_unit"]
    brightness = 255.0 * candela * peak / (reference ** 2 * policy["full_white_lux"])
    color = [255.0 * (c / peak) ** (1.0 / policy["srgb_gamma"]) for c in record["color"]]
    return color + [brightness]


class CompiledMap:
    """Decoded lumps of a BSP2 map (legacy payload inside the container)."""

    def __init__(self, path):
        data = Path(path).read_bytes()
        self.container = bsp2_reader.Bsp2File(data)
        self.container.check_legacy_structures()
        self.legacy = self.container.export_legacy()
        self.bsp = legacy_bsp.LegacyBsp(self.legacy)
        self.normals, self.dists = self.bsp.planes()
        self.texinfo = self.bsp.texinfo()
        self.texdata = self.bsp.texdata()

    def lump(self, index, record=1):
        return self.bsp.lump(index, record)

    def models(self):
        raw = self.lump(legacy_bsp.LUMP_MODELS, legacy_bsp.MODEL_BYTES)
        result = []
        for i in range(len(raw) // legacy_bsp.MODEL_BYTES):
            values = struct.unpack_from("<9f3i", raw, i * legacy_bsp.MODEL_BYTES)
            result.append({"mins": values[0:3], "maxs": values[3:6], "headnode": values[9],
                           "firstface": values[10], "numfaces": values[11]})
        return result

    def brushes(self):
        raw = self.lump(legacy_bsp.LUMP_BRUSHES, legacy_bsp.BRUSH_BYTES)
        sides = self.lump(legacy_bsp.LUMP_BRUSHSIDES, legacy_bsp.BRUSHSIDE_BYTES)
        result = []
        for i in range(len(raw) // legacy_bsp.BRUSH_BYTES):
            first, count, contents = struct.unpack_from("<3i", raw, i * legacy_bsp.BRUSH_BYTES)
            if first < 0 or first + count > len(sides) // legacy_bsp.BRUSHSIDE_BYTES:
                raise ValueError("brush %d has a bad side range" % i)
            planes = []
            for s in range(first, first + count):
                plane, texinfo, _disp, bevel = struct.unpack_from(
                    "<Hhhh", sides, s * legacy_bsp.BRUSHSIDE_BYTES)
                planes.append({"normal": tuple(self.normals[plane]),
                               "dist": float(self.dists[plane]), "texinfo": texinfo,
                               "bevel": bool(bevel)})
            result.append({"contents": contents, "sides": planes})
        return result

    def material(self, texinfo):
        if not 0 <= texinfo < len(self.texinfo):
            return None
        data = self.texinfo[texinfo]["texdata"]
        return self.texdata[data]["name"].lower() if data >= 0 else None

    def face_ids(self):
        return np.frombuffer(self.lump(LUMP_FACEIDS, 2), dtype="<u2")

    def static_props(self):
        raw = self.lump(LUMP_GAME_LUMP)
        count = struct.unpack_from("<i", raw, 0)[0]
        for i in range(count):
            lump_id, _flags, version, offset, length = struct.unpack_from("<iHHii", raw,
                                                                         4 + 16 * i)
            if lump_id != SPRP:
                continue
            data = self.legacy[offset:offset + length]
            names = struct.unpack_from("<i", data, 0)[0]
            dictionary = [data[4 + 128 * n:4 + 128 * (n + 1)].split(b"\0")[0].decode("latin-1")
                          for n in range(names)]
            cursor = 4 + 128 * names
            leaves = struct.unpack_from("<i", data, cursor)[0]
            cursor += 4 + 2 * leaves
            props = struct.unpack_from("<i", data, cursor)[0]
            cursor += 4
            # The engine reads records of a fixed size per (BSP, lump) version.
            size = ENGINE_STATIC_PROP_RECORD.get((self.bsp.version, version))
            if size is None or cursor + size * props != len(data):
                raise ValueError("static prop lump v%d in a v%d map is not the engine's "
                                 "%r-byte records" % (version, self.bsp.version, size))
            result = []
            for p in range(props):
                values = struct.unpack_from("<3f3fHHHBBi", data, cursor + p * size)
                result.append({"origin": values[0:3], "angles": values[3:6],
                               "model": dictionary[values[6]] if values[6] < names else None,
                               "leaf_count": values[8], "solid": values[9],
                               "flags": values[10], "skin": values[11]})
            return version, dictionary, result
        return None, [], []

    def physics_models(self):
        """{model index: solid count} from the vphysics collision lump."""
        raw = self.lump(LUMP_PHYSCOLLIDE)
        cursor, result = 0, {}
        while cursor + 16 <= len(raw):
            model, data_size, key_size, solids = struct.unpack_from("<4i", raw, cursor)
            if model == -1:
                break
            result[model] = solids
            cursor += 16 + data_size + key_size
        return result

    # ------------------------------------------------------------ queries

    def point_contents(self, point):
        """Brush contents at a point (the union over containing brushes)."""
        contents = 0
        for brush in self.world_brushes:
            if all(dot(side["normal"], point) - side["dist"] < -1e-3
                   for side in brush["sides"] if not side["bevel"]):
                contents |= brush["contents"]
        return contents

    def trace(self, start, end, hull=((0, 0, 0), (0, 0, 0)), mask=MASK_SOLID):
        """Fraction of a box sweep from start to end before it touches a
        brush with `mask` contents (1.0: clear). Brush planes (bevels
        included) are pushed out by the box's support corner."""
        mins, maxs = hull
        best = 1.0
        for brush in self.world_brushes:
            if not brush["contents"] & mask:
                continue
            enter, leave, starts_out = -1.0, 1.0, False
            for side in brush["sides"]:
                n = side["normal"]
                offset = [maxs[a] if n[a] < 0 else mins[a] for a in range(3)]
                dist = side["dist"] - dot(offset, n)
                d1 = dot(start, n) - dist
                d2 = dot(end, n) - dist
                if d1 > 0:
                    starts_out = True
                if d1 > 0 and d2 >= d1:
                    enter = 2.0
                    break
                if d1 <= 0 and d2 <= 0:
                    continue
                f = d1 / (d1 - d2)
                if d1 > d2:
                    enter = max(enter, f)
                else:
                    leave = min(leave, f)
            if enter > leave or enter > 1.0:
                continue
            if not starts_out:
                return 0.0
            best = min(best, max(enter, 0.0))
        return best


def check_map(bsp2_path, report, provenance, profile, runtime, checks=None):
    """Run every check; returns the source-usd-map-check/v1 record."""
    import source_content

    c = checks or Checks(stream=io.StringIO())
    failures = []

    def check(condition, name, detail=""):
        if not c.check(condition, name, detail):
            failures.append("%s%s" % (name, (": " + detail) if detail else ""))
        return bool(condition)

    observations = {}
    try:
        m = CompiledMap(bsp2_path)
        check(True, "container.valid")
    except (ValueError, bsp2_reader.FormatError) as error:
        check(False, "container.valid", str(error))
        return {"schema": CHECK_SCHEMA, "checks": c.checks, "failures": failures,
                "observations": observations}

    objects = {o["id"]: o for o in report["objects"]}
    outputs = {o["id"]: o for o in provenance["objects"]}
    id_key = profile["entity_id_key"]
    check(sorted(objects) == sorted(outputs), "provenance.covers-every-object",
          "report %s, provenance %s" % (sorted(objects), sorted(outputs)))
    check(provenance.get("bsp2_sha256") ==
          hashlib.sha256(Path(bsp2_path).read_bytes()).hexdigest(), "provenance.bsp2-hash")

    # --------------------------------------------------------- entities
    entities = m.bsp.entities()
    check(entities[0].get("classname") == "worldspawn", "entities.worldspawn-first")
    by_id = {}
    for entity in entities[1:]:
        ident = entity.get(id_key)
        check(ident in objects, "entities.authored-only",
              "entity %s has id %r" % (entity.get("classname"), ident))
        check(ident not in by_id, "entities.unique-id", str(ident))
        by_id[ident] = entity
    resolver = source_content.ContentResolver(str(runtime))
    models = m.models()
    for ident, record in sorted(objects.items()):
        role = record["role"]
        if role in ("world_solid", "prop_static"):
            check(ident not in by_id, "entities.not-an-entity", ident)
            continue
        entity = by_id.get(ident)
        if not check(entity is not None, "entities.present", ident):
            continue
        cls = "light" if role == "light" else record.get("classname", role)
        check(entity.get("classname") == cls, "entities.class",
              "%s is %r, authored %r" % (ident, entity.get("classname"), cls))
        if role != "entity_brush":
            check(close(numbers(entity.get("origin", "")), record["origin"]),
                  "entities.origin", "%s %r vs %r" % (ident, entity.get("origin"),
                                                      record["origin"]))
        if role in ("entity_point", "prop_physics", "prop_dynamic"):
            check(angle_close(numbers(entity.get("angles", "0 0 0")), record["angles"]),
                  "entities.angles", "%s %r vs %r" % (ident, entity.get("angles"),
                                                      record["angles"]))
        if role in profile["targetname"]["roles"]:
            check(entity.get("targetname") == ident, "entities.targetname",
                  "%s is named %r" % (ident, entity.get("targetname")))
        if role in ("prop_physics", "prop_dynamic"):
            check(entity.get("model") == record["model"], "entities.model", ident)
            suffixes = [".mdl"] + ([".phy"] if role == "prop_physics" or
                                   record.get("collision") == "vphysics" else [])
            for suffix in suffixes:
                check(resolver.read(record["model"][:-4] + suffix)[0] is not None,
                      "content.model-file", record["model"][:-4] + suffix)
            check(entity.get("skin", "0") == str(record.get("skin", 0)), "entities.skin", ident)
        if role == "prop_physics":
            # Physics-only keys never reach another role, and a physics prop
            # never carries a dynamic prop's animation.
            check("DefaultAnim" not in entity, "roles.physics-not-animated", ident)
        if role == "prop_dynamic":
            check(entity.get("solid") == profile["dynamic_prop_solid"][record["collision"]],
                  "roles.dynamic-solid", "%s solid %r" % (ident, entity.get("solid")))
            check(entity.get("DefaultAnim") == record.get("default_animation"),
                  "roles.dynamic-default-animation", "%s %r vs %r" % (
                      ident, entity.get("DefaultAnim"), record.get("default_animation")))
            check("massScale" not in entity and "inertiaScale" not in entity,
                  "roles.dynamic-not-physics", ident)
        if role == "entity_brush":
            check(entity.get("model", "").startswith("*"), "entities.brush-model", ident)
            index = int(entity.get("model", "*0")[1:] or 0)
            if check(0 < index < len(models), "entities.brush-model-index", ident):
                bounds = record["bounds"]
                check(close(models[index]["mins"], bounds[0], 1.01) and
                      close(models[index]["maxs"], bounds[1], 1.01), "entities.brush-bounds",
                      "%s model %r..%r vs %r" % (ident, models[index]["mins"],
                                                 models[index]["maxs"], bounds))
            for key, value in profile["classes"][cls]["keys"].items():
                if cls.startswith("trigger_") and key == "spawnflags":
                    continue  # the touch filter's, checked under roles
                check(entity.get(key) == value, "entities.class-default",
                      "%s %s=%r" % (ident, key, entity.get(key)))
        if role == "light":
            light = numbers(entity.get("_light", ""))
            want = expected_light(record, profile)
            check(len(light) == 4 and all(abs(a - b) <= 1e-3 * max(1.0, abs(b)) + 1e-3
                                          for a, b in zip(light, want)),
                  "entities.light-policy", "%s _light %r, policy %r" % (ident, light, want))
            for key, value in profile["classes"]["light"]["keys"].items():
                check(entity.get(key) == value, "entities.light-key", "%s %s" % (ident, key))
    observations["entities"] = len(entities)
    check_roles(check, m, objects, by_id, profile, observations)

    # ----------------------------------------------------- static props
    try:
        version, dictionary, props = m.static_props()
        check(True, "props.engine-layout")
    except (ValueError, struct.error) as error:
        check(False, "props.engine-layout", str(error))
        version, dictionary, props = None, [], []
    statics = sorted((o for o in objects.values() if o["role"] == "prop_static"),
                     key=lambda o: outputs[o["id"]]["output"]["static_prop"])
    check(version is not None, "props.lump-present")
    check(len(props) == len(statics), "props.count",
          "%d static props, %d authored" % (len(props), len(statics)))
    for record in statics:
        index = outputs[record["id"]]["output"]["static_prop"]
        if not check(0 <= index < len(props), "props.provenance-index", record["id"]):
            continue
        prop = props[index]
        check(prop["model"] == record["model"], "props.model",
              "%s %r vs %r" % (record["id"], prop["model"], record["model"]))
        check(close(prop["origin"], record["origin"]), "props.origin", record["id"])
        check(angle_close(prop["angles"], record["angles"]), "props.angles", record["id"])
        check(prop["solid"] == int(profile["static_prop_solid"][record["collision"]]),
              "props.solid", "%s solid %d" % (record["id"], prop["solid"]))
        check(prop["skin"] == record.get("skin", 0), "props.skin", record["id"])
        check(not prop["flags"] & STATIC_PROP_NO_SHADOW, "props.casts-baked-shadow",
              "%s flags %#x" % (record["id"], prop["flags"]))
        check(prop["leaf_count"] > 0, "props.in-leaves", record["id"])
        suffixes = [".mdl"] + ([".phy"] if record["collision"] == "vphysics" else [])
        for suffix in suffixes:
            check(resolver.read(record["model"][:-4] + suffix)[0] is not None,
                  "content.model-file", record["model"][:-4] + suffix)
    observations["static_props"] = len(props)

    # ----------------------------------------------------------- brushes
    brushes = m.brushes()
    side_materials = {}
    # Collision queries use the brushes of the world model's tree, as the
    # engine's world traces do; brush entities have their own models.
    m.world_brushes = [brushes[b["index"]] for b in m.bsp.world_brushes()]
    solids = [o for o in objects.values() if "faces" in o]
    for record in solids:
        output = outputs[record["id"]]["output"]
        index = output.get("brush", -1)
        if not check(0 <= index < len(brushes), "brushes.provenance-index", record["id"]):
            continue
        brush = brushes[index]
        sides = [s for s in brush["sides"] if not s["bevel"]]
        check(len(sides) == len(record["faces"]), "brushes.side-count",
              "%s: %d sides, %d faces" % (record["id"], len(sides), len(record["faces"])))
        for face_index, face in enumerate(record["faces"]):
            normal, dist = face["plane"][:3], face["plane"][3]
            match = [s for s in sides if close(s["normal"], normal, 1e-4) and
                     abs(s["dist"] - dist) <= PLANE_TOLERANCE]
            if not check(len(match) == 1, "brushes.side-plane",
                         "%s face %d plane %r" % (record["id"], face_index, face["plane"])):
                continue
            expected = (record["surfaces"][face_index]["material"]
                        if authored_surfaces(record) else profile["brush_entity_material"])
            side_materials[(record["id"], face_index)] = (m.material(match[0]["texinfo"]),
                                                          expected)
        if record["role"] == "world_solid":
            check(brush["contents"] & CONTENTS_SOLID, "brushes.solid-contents", record["id"])
    observations["brushes"] = len(brushes)

    # -------------------------------------------------- tree and surfaces
    contents_of, cluster_of, _mins, _maxs = m.bsp.leaves()

    def leaf(point):
        index = m.bsp.point_leaf(np.array(point, dtype=np.float64))
        return int(contents_of[index]), int(cluster_of[index])

    faces = m.bsp.world_faces()
    face_ids = m.face_ids()
    side_keys = {}
    for record in solids:
        for key in outputs[record["id"]]["output"].get("sides", []):
            side_keys[(record["id"], key["surface"])] = key["side_key"]
    drawn = [f for f in faces if not m.texinfo[f["texinfo"]]["flags"] &
             (legacy_bsp.SURF_NODRAW | legacy_bsp.SURF_SKIP | legacy_bsp.SURF_HINT)]
    check(len(face_ids) >= len(faces), "surfaces.face-ids-present")
    world_solids = [o for o in solids if o["role"] == "world_solid"]
    output_polygons = []
    for f in drawn:
        normal = tuple(m.normals[f["plane"]])
        # A face's plane is oriented to its front; so is the authored face.
        points = [tuple(p) for p in f["points"]]
        polygon = list(reversed(points))  # stored clockwise from the front
        centroid = tuple(sum(p[a] for p in points) / len(points) for a in range(3))
        owner = None
        for record in world_solids:
            for face_index, face in enumerate(record["faces"]):
                if close(normal, face["plane"][:3], 1e-4) and \
                        abs(dot(centroid, face["plane"][:3]) - face["plane"][3]) <= \
                        PLANE_TOLERANCE and \
                        inside_polygon(centroid, face["plane"][:3], face["points"]):
                    owner = (record, face_index)
        if not check(owner is not None, "surfaces.face-on-authored-surface",
                     "face %d at %r" % (f["index"], centroid)):
            continue
        record, face_index = owner
        surface = record["surfaces"][face_index]
        check(m.material(f["texinfo"]) == surface["material"], "surfaces.face-material",
              "face %d %r vs %r" % (f["index"], m.material(f["texinfo"]), surface["material"]))
        check(f["index"] < len(face_ids) and
              int(face_ids[f["index"]]) == side_keys.get((record["id"], surface["surface"])),
              "surfaces.face-provenance", "face %d id %r, %s surface %d is %r" % (
                  f["index"], int(face_ids[f["index"]]) if f["index"] < len(face_ids) else None,
                  record["id"], surface["surface"],
                  side_keys.get((record["id"], surface["surface"]))))
        output_polygons.append((f, normal, polygon, record, face_index))

    covered = uncovered = 0
    for record in world_solids:
        for face_index, face in enumerate(record["faces"]):
            normal, polygon = face["plane"][:3], face["points"]
            st = face["st"]
            for p in face_samples(polygon, normal):
                front = tuple(p[a] + normal[a] * 0.5 for a in range(3))
                if leaf(front)[0] != 0:
                    continue
                hits = [o for o in output_polygons if o[3] is record and o[4] == face_index and
                        inside_polygon(p, o[1], o[2])]
                if not check(hits, "surfaces.open-front-covered",
                             "%s surface %d at %r" % (record["id"], face_index, p)):
                    uncovered += 1
                    continue
                covered += 1
                f = hits[0][0]
                texdata = m.texdata[m.texinfo[f["texinfo"]]["texdata"]]
                vectors = m.texinfo[f["texinfo"]]["vectors"]
                s = (dot(vectors[0][:3], p) + vectors[0][3]) / texdata["width"]
                t = 1.0 - (dot(vectors[1][:3], p) + vectors[1][3]) / texdata["height"]
                want_s = dot(st["s"][:3], p) + st["s"][3]
                want_t = dot(st["t"][:3], p) + st["t"][3]
                check(abs(s - want_s) <= ST_TOLERANCE and abs(t - want_t) <= ST_TOLERANCE,
                      "surfaces.texture-mapping", "%s surface %d at %r: st (%.4f, %.4f) vs "
                      "(%.4f, %.4f)" % (record["id"], face_index, p, s, t, want_s, want_t))
    check(covered > 0, "surfaces.some-open-surface")
    # vbsp's CompactTexinfos points a brush side that no face shows at another
    # used texinfo, so only sides with an output face keep their material.
    visible = {(owner[3]["id"], owner[4]) for owner in output_polygons}
    for (ident, face_index), (actual, expected) in sorted(side_materials.items()):
        if (ident, face_index) in visible:
            check(actual == expected, "brushes.visible-side-material",
                  "%s face %d %r vs %r" % (ident, face_index, actual, expected))
    observations["hidden_sides_substituted"] = sum(
        1 for key, (actual, expected) in side_materials.items()
        if key not in visible and actual != expected)
    observations["surface_samples_covered"] = covered

    # --------------------------------------------------------- collision
    for record in world_solids:
        lo, hi = record["bounds"]
        center = tuple((lo[a] + hi[a]) / 2 for a in range(3))
        check(m.point_contents(center) & CONTENTS_SOLID, "collision.solid-inside-brush",
              record["id"])
        check(leaf(center)[1] == -1, "visibility.solid-has-no-cluster", record["id"])
    placements = [o for o in objects.values() if "origin" in o and o["role"] != "entity_brush"]
    for record in placements:
        above = (record["origin"][0], record["origin"][1], record["origin"][2] + 1.0)
        contents, cluster = leaf(above)
        check(contents == 0 and m.point_contents(above) == 0, "collision.placement-open",
              "%s contents %x" % (record["id"], contents))
        check(cluster >= 0, "visibility.placement-has-cluster", record["id"])
    rays = []
    for record in placements:
        o = record["origin"]
        start = (o[0], o[1], o[2] + 1.0)
        for direction in ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)):
            end = tuple(start[a] + 4096.0 * direction[a] for a in range(3))
            fraction = m.trace(start, end)
            hit = tuple(start[a] + (end[a] - start[a]) * fraction for a in range(3))
            expected = expected_hit(start, direction, world_solids)
            check(fraction < 1.0 and expected is not None and
                  abs(dot(sub(hit, start), direction) - expected) <= 0.05,
                  "collision.ray-hits-authored-solid",
                  "%s dir %r: hit %.3f, authored %r" % (record["id"], direction,
                                                        dot(sub(hit, start), direction),
                                                        expected))
            rays.append({"start": start, "end": end, "fraction": fraction})
    starts = [o for o in placements if o.get("classname") == "info_player_start"]
    for record in starts:
        o = record["origin"]
        start = (o[0], o[1], o[2] + 32.0)
        end = (o[0], o[1], o[2] - 256.0)
        fraction = m.trace(start, end, PLAYER_HULL)
        floor = start[2] + (end[2] - start[2]) * fraction
        want = expected_hit((o[0], o[1], o[2] + 32.0), (0, 0, -1), world_solids, PLAYER_HULL)
        check(want is not None and abs((start[2] - floor) - want) <= 0.05,
              "collision.player-hull-lands", "%s lands at z=%.3f" % (record["id"], floor))
    physics = m.physics_models()
    check(physics.get(0, 0) > 0, "collision.world-physics-model", repr(physics))
    for record in (o for o in objects.values() if o["role"] == "entity_brush"):
        entity = by_id.get(record["id"]) or {}
        index = int(entity.get("model", "*0")[1:] or 0)
        check(physics.get(index, 0) > 0, "collision.brush-entity-physics-model", record["id"])
    observations["rays"] = rays

    # -------------------------------------------------------- visibility
    pvs = m.bsp.visibility()
    if check(pvs is not None, "visibility.vvis-data"):
        check(len(pvs) >= 2, "visibility.not-single-cell", "%d clusters" % len(pvs))
        observations["clusters"] = len(pvs)
        cells = open_cells(m, world_solids)
        regions, outside = sealed_regions(m, cells, world_solids)
        lo, hi = world_bounds(world_solids)
        inside = sorted(p for p in cells if regions[p] not in outside)
        # Open space connected to the void is filled by vbsp: solid, no cluster.
        filled = [p for p in cells if regions[p] in outside and
                  all(lo[a] < p[a] < hi[a] for a in range(3))]
        for p in filled:
            check(leaf(p)[1] == -1, "visibility.void-filled", "%r has a cluster" % (p,))
        observations["open_cells"] = len(inside)
        observations["void_cells_in_bounds"] = len(filled)
        observations["sealed_regions"] = len({regions[p] for p in inside})
        check(inside, "visibility.sealed-space-exists")
        rng = random.Random(SEED)
        pairs = [(rng.choice(inside), rng.choice(inside)) for _ in range(300)] if inside else []
        clear = separated = 0
        for a, b in pairs:
            ca, cb = leaf(a)[1], leaf(b)[1]
            if not check(ca >= 0 and cb >= 0, "visibility.open-point-has-cluster",
                         "%r/%r" % (a, b)):
                continue
            if regions[a] != regions[b]:
                separated += 1
                check(not pvs[ca][cb], "visibility.sealed-regions-hidden",
                      "cluster %d sees %d across a seal" % (ca, cb))
            elif m.trace(a, b) >= 1.0:
                clear += 1
                check(pvs[ca][cb] and pvs[cb][ca], "visibility.line-of-sight-visible",
                      "clusters %d and %d see each other at %r-%r" % (ca, cb, a, b))
        check(clear > 0, "visibility.some-line-of-sight")
        observations["visibility_pairs"] = {"clear": clear, "sealed": separated}

    # ---------------------------------------------------------- lighting
    lighting = m.lump(LUMP_LIGHTING)
    check(len(lighting) > 0, "lighting.lightmaps-present")
    lit = [f for f in drawn if f["styles"][0] != 255]
    check(drawn and len(lit) == len(drawn), "lighting.world-faces-lit",
          "%d of %d drawn faces have a lightmap style" % (len(lit), len(drawn)))
    world_lights, _lump = m.bsp.world_lights(hdr=False)
    for record in (o for o in objects.values() if o["role"] == "light"):
        compiled = [w for w in world_lights if w["type"] == "point" and
                    close(w["origin"], record["origin"])]
        check(len(compiled) == 1, "lighting.point-light-compiled", record["id"])
    observations["world_lights"] = len(world_lights)
    lights = [o for o in objects.values() if o["role"] == "light"]
    unmeasured = observations.setdefault("static_prop_shadows_unmeasured", [])
    for record in sorted((o for o in objects.values() if o["role"] == "prop_static"),
                         key=lambda o: o["id"]):
        # The mirror construction needs a single light.
        shadow = static_prop_shadow(m, record, lights[0]["origin"], resolver,
                                    world_solids) if len(lights) == 1 else None
        if shadow is None:
            unmeasured.append(record["id"])
            continue
        observations.setdefault("static_prop_shadows", {})[record["id"]] = shadow
        check(shadow["ratio"] <= SHADOW_RATIO, "lighting.static-prop-shadow",
              "%s: floor %.3f at %r vs %.3f at its mirror %r" % (
                  record["id"], shadow["shadowed"], shadow["point"], shadow["open"],
                  shadow["mirror"]))

    return {"schema": CHECK_SCHEMA, "checks": c.checks, "failures": failures,
            "observations": observations}


def authored_surfaces(record):
    """World solids and brush entities whose faces carry authored materials."""
    return bool(record.get("surfaces")) and all("material" in s for s in record["surfaces"])


def angle_vectors(angles):
    """Source's AngleVectors forward vector (mathlib), for pitch yaw roll."""
    pitch, yaw = math.radians(angles[0]), math.radians(angles[1])
    return (math.cos(pitch) * math.cos(yaw), math.cos(pitch) * math.sin(yaw), -math.sin(pitch))


def outputs_of(entity):
    """Entity outputs as (output, [target, input, parameter, delay, times])."""
    result = []
    for key, value in entity.items():
        if key.startswith("On"):
            for item in value if isinstance(value, list) else [value]:
                result.append((key, item.split(",")))
    return sorted(result)


def check_roles(check, m, objects, by_id, profile, observations):
    """Each role's compiled contract, from the authored records and the policy."""
    names = {e.get("targetname"): e for e in by_id.values() if e.get("targetname")}
    models = m.models()
    for ident, record in sorted(objects.items()):
        entity = by_id.get(ident)
        cls = record.get("classname")
        if entity is None or record["role"] != "entity_brush":
            continue
        if cls.startswith("trigger_"):
            policy = profile["trigger_touch_filter"]
            wanted = sum(policy["flags"][f] for f in set(record.get("touch_filter",
                                                                   policy["default"])))
            check(entity.get("spawnflags") == str(wanted), "roles.trigger-touch-filter",
                  "%s spawnflags %r, filter %r" % (ident, entity.get("spawnflags"),
                                                   record.get("touch_filter")))
        authored = sorted((c["output"], [c["target"], c["input"], c["parameter"]])
                          for c in record.get("connections", []))
        compiled = outputs_of(entity)
        check([(o, v[:3]) for o, v in compiled] == authored, "roles.connections",
              "%s compiled %r, authored %r" % (ident, compiled, authored))
        for output, fields in compiled:
            target = names.get(fields[0]) if fields else None
            check(len(fields) == 5 and target is not None, "roles.connection-target",
                  "%s %s -> %r" % (ident, output, fields))
            if len(fields) == 5:
                check(float(fields[3]) == profile["connections"]["delay"] and
                      int(fields[4]) == profile["connections"]["times"],
                      "roles.connection-policy", "%s %s %r" % (ident, output, fields))
        if cls == "func_movelinear":
            forward = angle_vectors(numbers(entity.get("movedir", "0 0 0")))
            distance = float(entity.get("movedistance", "0"))
            moved = [forward[a] * distance for a in range(3)]
            check(close(moved, record["move"], 0.01), "roles.movelinear-displacement",
                  "%s moves %r, authored %r" % (ident, moved, record["move"]))
            speed = float(entity.get("speed", "0"))
            check(speed > 0 and abs(distance / speed - record["move_seconds"]) <= 1e-3,
                  "roles.movelinear-duration", "%s speed %r" % (ident, speed))
            index = int(entity.get("model", "*0")[1:] or 0)
            if 0 < index < len(models):
                check_brush_entity_faces(check, m, models[index], record)
    observations["targetnames"] = sorted(n for n in names if n)


def check_brush_entity_faces(check, m, model, record):
    """Every face of an authored-surface brush entity's model lies on one of
    its authored faces, with that face's material and texture mapping."""
    faces = m.lump(legacy_bsp.LUMP_FACES, legacy_bsp.FACE.size)
    first, count = model["firstface"], model["numfaces"]
    check(count > 0, "roles.brush-entity-faces", record["id"])
    for index in range(first, first + count):
        values = legacy_bsp.FACE.unpack_from(faces, index * legacy_bsp.FACE.size)
        texinfo = values[5]
        normal = tuple(m.normals[values[0]])
        points = face_points(m, values)
        centroid = tuple(sum(p[a] for p in points) / len(points) for a in range(3))
        owner = [i for i, face in enumerate(record["faces"])
                 if close(normal, face["plane"][:3], 1e-4) and
                 abs(dot(centroid, face["plane"][:3]) - face["plane"][3]) <= PLANE_TOLERANCE]
        if not check(len(owner) == 1, "roles.brush-entity-face-on-authored",
                     "%s face %d" % (record["id"], index)):
            continue
        face = record["faces"][owner[0]]
        check(m.material(texinfo) == record["surfaces"][owner[0]]["material"],
              "roles.brush-entity-material", "%s face %d %r" % (record["id"], index,
                                                                m.material(texinfo)))
        texdata = m.texdata[m.texinfo[texinfo]["texdata"]]
        vectors = m.texinfo[texinfo]["vectors"]
        s_value = (dot(vectors[0][:3], centroid) + vectors[0][3]) / texdata["width"]
        t_value = 1.0 - (dot(vectors[1][:3], centroid) + vectors[1][3]) / texdata["height"]
        want_s = dot(face["st"]["s"][:3], centroid) + face["st"]["s"][3]
        want_t = dot(face["st"]["t"][:3], centroid) + face["st"]["t"][3]
        check(abs(s_value - want_s) <= ST_TOLERANCE and abs(t_value - want_t) <= ST_TOLERANCE,
              "roles.brush-entity-texture-mapping", "%s face %d" % (record["id"], index))


def face_points(m, values):
    """The corner points of a raw dface_t (FACE fields)."""
    first_edge, edge_count = values[3], values[4]
    edges = np.frombuffer(m.lump(legacy_bsp.LUMP_EDGES, 4), dtype="<u2").reshape(-1, 2)
    surfedges = np.frombuffer(m.lump(legacy_bsp.LUMP_SURFEDGES, 4), dtype="<i4")
    vertices = np.frombuffer(m.lump(legacy_bsp.LUMP_VERTEXES, 12), dtype="<f4").reshape(-1, 3)
    ids = surfedges[first_edge:first_edge + edge_count]
    corners = np.where(ids >= 0, edges[np.abs(ids), 0], edges[np.abs(ids), 1])
    return [tuple(float(x) for x in vertices[c]) for c in corners]


def luxel_luminance(m, point):
    """The baked style-0 lightmap luminance of the world floor face (+z) under
    `point`, from the luxel nearest the point; None when no face covers it."""
    faces = m.lump(legacy_bsp.LUMP_FACES, legacy_bsp.FACE.size)
    texinfo = m.lump(legacy_bsp.LUMP_TEXINFO, legacy_bsp.TEXINFO_BYTES)
    lighting = m.lump(LUMP_LIGHTING)
    for face in m.bsp.world_faces():
        normal = tuple(face["plane_normal"])
        polygon = list(reversed([tuple(p) for p in face["points"]]))
        if not close(normal, (0.0, 0.0, 1.0), 1e-4) or \
                abs(polygon[0][2] - point[2]) > PLANE_TOLERANCE or \
                not inside_polygon(point, normal, polygon, margin=0.0):
            continue
        values = legacy_bsp.FACE.unpack_from(faces, face["index"] * legacy_bsp.FACE.size)
        styles, offset = values[8:12], values[12]
        mins, size = values[14:16], values[16:18]
        if styles[0] == 255 or offset < 0:
            return None
        lightmap = struct.unpack_from("<8f", texinfo, face["texinfo"] *
                                      legacy_bsp.TEXINFO_BYTES + 32)
        s = dot(lightmap[0:3], point) + lightmap[3] - mins[0]
        t = dot(lightmap[4:7], point) + lightmap[7] - mins[1]
        s = min(max(int(round(s)), 0), size[0])
        t = min(max(int(round(t)), 0), size[1])
        r, g, b, e = struct.unpack_from("<BBBb", lighting, offset + 4 * (t * (size[0] + 1) + s))
        return (0.2126 * r + 0.7152 * g + 0.0722 * b) * 2.0 ** e
    return None


def static_prop_shadow(m, record, light, resolver, world_solids):
    """Baked-shadow evidence for a prop_static: the floor point where the line
    from the light through the top of the prop's model (at its origin) lands,
    and that point mirrored across the vertical plane y = light y. The oracle
    assumes a scene symmetric about that plane apart from the prop, as the
    fixtures are. None when the construction does not apply (the light below
    the prop's top, no floor under the prop, or a point on no lit floor face);
    the caller records such props as unmeasured."""
    data, _ = resolver.read(record["model"])
    if data is None:
        return None
    hull = source_model.read_model(data)["hull"]
    top = record["origin"][2] + hull[1][2]
    if light[2] <= top + 1.0:
        return None
    floor = max((o["bounds"][1][2] for o in world_solids
                 if o["bounds"][0][0] <= record["origin"][0] <= o["bounds"][1][0] and
                 o["bounds"][0][1] <= record["origin"][1] <= o["bounds"][1][1] and
                 o["bounds"][1][2] <= record["origin"][2]), default=None)
    if floor is None:
        return None
    scale_to_floor = (light[2] - floor) / (light[2] - top)
    point = (light[0] + (record["origin"][0] - light[0]) * scale_to_floor,
             light[1] + (record["origin"][1] - light[1]) * scale_to_floor, floor)
    mirror = (point[0], 2.0 * light[1] - point[1], floor)
    shadowed, open_value = luxel_luminance(m, point), luxel_luminance(m, mirror)
    if shadowed is None or not open_value:
        return None
    return {"point": [round(x, 3) for x in point], "mirror": [round(x, 3) for x in mirror],
            "shadowed": shadowed, "open": open_value, "ratio": shadowed / open_value}


def face_samples(polygon, normal, spacing=SAMPLE_SPACING):
    """Points on a convex authored face at spacing/2 + k*spacing on its two
    in-plane world axes (off the 16-unit grid that solids snap to), 1 unit
    inside its edges."""
    axis = max(range(3), key=lambda a: abs(normal[a]))
    keep = [a for a in range(3) if a != axis]
    lo = [min(p[a] for p in polygon) for a in range(3)]
    hi = [max(p[a] for p in polygon) for a in range(3)]
    n, d = normal, dot(normal, polygon[0])
    result = []
    u = math.floor(lo[keep[0]] / spacing) * spacing + spacing / 2
    while u < hi[keep[0]]:
        v = math.floor(lo[keep[1]] / spacing) * spacing + spacing / 2
        while v < hi[keep[1]]:
            p = [0.0, 0.0, 0.0]
            p[keep[0]], p[keep[1]] = u, v
            p[axis] = (d - n[keep[0]] * u - n[keep[1]] * v) / n[axis]
            if inside_polygon(p, normal, polygon, margin=-1.0):
                result.append(tuple(p))
            v += spacing
        u += spacing
    return result


def expected_hit(start, direction, solids, hull=((0, 0, 0), (0, 0, 0))):
    """Distance along `direction` to the first authored world solid, from the
    authored faces alone (each solid as its planes), for a box hull."""
    mins, maxs = hull
    best = None
    for record in solids:
        enter, leave = -math.inf, math.inf
        outside = False
        for face in record["faces"]:
            n = face["plane"][:3]
            offset = [maxs[a] if n[a] < 0 else mins[a] for a in range(3)]
            dist = face["plane"][3] - dot(offset, n)
            d0 = dot(start, n) - dist
            rate = dot(direction, n)
            if d0 > 0:
                outside = True
            if abs(rate) < 1e-12:
                if d0 > 0:
                    enter = math.inf
                continue
            t = -d0 / rate
            if rate < 0:
                enter = max(enter, t)
            else:
                leave = min(leave, t)
        if outside and enter <= leave and enter >= 0 and (best is None or enter < best):
            best = enter
    return best


def world_bounds(solids):
    return ([min(o["bounds"][0][a] for o in solids) for a in range(3)],
            [max(o["bounds"][1][a] for o in solids) for a in range(3)])


def open_cells(m, solids, spacing=16.0, offset=3.5):
    """Points of a grid over the authored world bounds, plus one layer of
    points outside them, that are in no solid brush (by the brushes' own
    point contents). The grid is offset from the 16-unit lattice solids are
    authored on, so no point lies on a face."""
    lo, hi = world_bounds(solids)
    cells = set()
    x = lo[0] + offset - spacing
    while x < hi[0] + spacing:
        y = lo[1] + offset - spacing
        while y < hi[1] + spacing:
            z = lo[2] + offset - spacing
            while z < hi[2] + spacing:
                if m.point_contents((x, y, z)) == 0:
                    cells.add((x, y, z))
                z += spacing
            y += spacing
        x += spacing
    return cells


def sealed_regions(m, cells, solids, spacing=16.0):
    """Connected components of open grid points, as ({point: label}, the
    labels that reach outside the authored world bounds). Neighbours
    (6-connected) join when the segment between them is clear of world
    brushes. An opening narrower than the spacing may be missed, which can
    only split a region (a false seal), never join two."""
    lo, hi = world_bounds(solids)
    region, outside, label = {}, set(), 0
    for start in sorted(cells):
        if start in region:
            continue
        stack = [start]
        region[start] = label
        while stack:
            p = stack.pop()
            if any(p[a] < lo[a] or p[a] > hi[a] for a in range(3)):
                outside.add(label)
            for axis in range(3):
                for step in (-spacing, spacing):
                    q = list(p)
                    q[axis] += step
                    q = tuple(q)
                    if q in cells and q not in region and m.trace(p, q) >= 1.0:
                        region[q] = label
                        stack.append(q)
        label += 1
    return region, outside


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    run = commands.add_parser("check")
    run.add_argument("map")
    run.add_argument("--report", required=True, help="usd_authoring_validate.py --json report")
    run.add_argument("--provenance", required=True, help="the compiler's provenance sidecar")
    run.add_argument("--runtime", required=True, help="game content (read only)")
    run.add_argument("--profile", default=str(COMPILE_PROFILE))
    run.add_argument("--json", help="write the check record here")
    args = parser.parse_args(argv)
    report = json.loads(Path(args.report).read_text())
    provenance = json.loads(Path(args.provenance).read_text())
    profile = json.loads(Path(args.profile).read_text())
    checks = Checks()
    record = check_map(args.map, report, provenance, profile, args.runtime, checks)
    if args.json:
        Path(args.json).write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
