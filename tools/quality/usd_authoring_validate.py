#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Validator for USD-native map authoring stages (RFC 0009 U0, roadmap R59).
#
# Checks a composed authoring stage against the source-authoring profile
# (quality/usd_authoring/source_authoring_v1.json, described by
# source_authoring_v1.md): stage metadata and composition, explicit object
# roles, per-role properties, stable ids, transforms, closed convex solids,
# surface ids, texture coordinates, material bindings and model paths. It is
# the gate before the native USD map compiler (U1) reads a stage, and its
# `objects` table is the compiler's authored-id input.
#
#   PYTHONPATH=build/toolchains/openusd-25.11/lib/python /usr/bin/python3.12 \
#       tools/quality/usd_authoring_validate.py validate \
#       quality/fixtures/usd-authoring/room/room.usda [--json OUT] [--previous P]
#
# `validate` prints `FAIL <code> <prim>: <message>` per error and ends with the
# checks-v1 `CONFORMANCE <checks> <failures>` record (exit 0 only when clean).
# `--json` writes the machine-readable report (errors and objects); `--previous`
# (a stage or an earlier `--json` report) adds the id-stability checks.
#
# Independent of usd_scene.py and the RFC 0008 World Stage code: it needs only
# OpenUSD's Python bindings and the standard library. Error codes are declared
# in the profile; emitting an undeclared code is a validator bug and raises.
#
# ============================================================================

import argparse
import json
import math
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
PROFILE_PATH = os.path.join(ROOT, "quality", "usd_authoring", "source_authoring_v1.json")
REPORT_SCHEMA = "source-authoring-validation/v1"
ROLE = "sourcemap:role"


def load_profile(path=PROFILE_PATH):
    with open(path, "r", encoding="utf-8") as stream:
        return json.load(stream)


# ------------------------------------------------------------ vector math

def sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def length(a):
    return math.sqrt(dot(a, a))


def scale(a, s):
    return (a[0] * s, a[1] * s, a[2] * s)


def newell(points):
    """Unnormalized polygon normal (twice the area vector)."""
    n = [0.0, 0.0, 0.0]
    for i, p in enumerate(points):
        q = points[(i + 1) % len(points)]
        n[0] += (p[1] - q[1]) * (p[2] + q[2])
        n[1] += (p[2] - q[2]) * (p[0] + q[0])
        n[2] += (p[0] - q[0]) * (p[1] + q[1])
    return tuple(n)


# ------------------------------------------------------------- validation

class Validation:
    """Counted checks and the errors they found, keyed by profile codes."""

    def __init__(self, profile):
        self.profile = profile
        self.checks = 0
        self.errors = []

    def error(self, code, path, message):
        if code not in self.profile["errors"]:
            raise KeyError("undeclared error code %r" % code)
        self.errors.append({"code": code, "path": str(path), "message": message})

    def check(self, condition, code, path, message):
        self.checks += 1
        if not condition:
            self.error(code, path, message)
        return bool(condition)


class Conversion:
    """Authored stage space to Source space (Z up, one unit = 0.0254 m)."""

    def __init__(self, meters_per_unit, up_axis, source_meters):
        self.scale = meters_per_unit / source_meters
        self.y_up = up_axis == "Y"

    def direction(self, v):
        return (v[0], -v[2], v[1]) if self.y_up else (v[0], v[1], v[2])

    def point(self, v):
        return scale(self.direction(v), self.scale)


def exact(x):
    """A report value that is stable across platforms: 12 decimals, no -0.0."""
    return round(x, 12) + 0.0


def source_face(points, polygon, plane):
    """A solid's face for the compiler: its outward unit plane (n.p = d) and
    its corners, counter-clockwise seen from outside, in Source units."""
    n, d = plane
    return {"plane": [exact(x) for x in n] + [exact(d)],
            "points": [[round(x, 6) + 0.0 for x in points[i]] for i in polygon]}


def matrix_rows(m):
    return [[m[r][c] for c in range(4)] for r in range(4)]


def transform_point(rows, p):
    """USD row-vector convention: p' = p * M."""
    return tuple(p[0] * rows[0][c] + p[1] * rows[1][c] + p[2] * rows[2][c] + rows[3][c]
                 for c in range(3))


def determinant3(rows):
    a = rows
    return (a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1])
            - a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0])
            + a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]))


def source_angles(forward, left, up):
    """Source QAngle (pitch, yaw, roll) in degrees for an orthonormal frame."""
    horizontal = math.hypot(forward[0], forward[1])
    pitch = math.degrees(math.atan2(-forward[2], horizontal))
    if horizontal > 1e-6:
        yaw = math.degrees(math.atan2(forward[1], forward[0]))
        roll = math.degrees(math.atan2(left[2], up[2]))
    else:  # looking straight up or down: fold roll into yaw
        yaw = math.degrees(math.atan2(-left[0], left[1]))
        roll = 0.0
    return [round(pitch, 6) + 0.0, round(yaw, 6) + 0.0, round(roll, 6) + 0.0]


class StageValidator:
    def __init__(self, pxr, profile, stage, validation):
        self.Usd, self.UsdGeom, self.UsdShade, self.UsdLux = pxr
        self.profile = profile
        self.stage = stage
        self.v = validation
        self.namespace = profile["namespace"]
        self.tol = profile["tolerances"]
        self.objects = []
        self.conversion = None

    # ---------------------------------------------------------- stage level

    def stage_metadata(self):
        stage, v, prof = self.stage, self.v, self.profile["stage"]
        root = stage.GetRootLayer()
        data = root.customLayerData.get(prof["layer_data_key"])
        if v.check(isinstance(data, dict) and "profile" in data and "version" in data,
                   "stage.profile-missing", "/",
                   "customLayerData.%s needs profile and version" % prof["layer_data_key"]):
            v.check(data["profile"] == self.profile["profile"]
                    and data["version"] == self.profile["version"], "stage.profile-version", "/",
                    "declares %s v%s; this validator implements %s v%d"
                    % (data["profile"], data["version"], self.profile["profile"],
                       self.profile["version"]))
        default = stage.GetDefaultPrim()
        v.check(bool(default) and default.IsDefined(), "stage.default-prim-missing", "/",
                "the root layer's defaultPrim is unset or undefined")

        pseudo = root.pseudoRoot
        meters = pseudo.GetInfo("metersPerUnit") if pseudo.HasInfo("metersPerUnit") else None
        meters_ok = v.check(meters is not None and math.isfinite(meters) and meters > 0,
                            "stage.units", "/",
                            "metersPerUnit is %s on the root layer"
                            % ("not authored" if meters is None else meters))
        up = pseudo.GetInfo("upAxis") if pseudo.HasInfo("upAxis") else None
        up_ok = v.check(up in prof["up_axes"], "stage.up-axis", "/",
                        "upAxis is %s on the root layer" % ("not authored" if up is None else up))
        # Later checks assume the profile's convention (Source units, Z up) for
        # a missing value, so one omission is one error.
        source = prof["source_meters_per_unit"]
        self.conversion = Conversion(meters if meters_ok else source, up if up_ok else "Z",
                                     source)

        for layer in stage.GetUsedLayers():
            if layer == root or layer.anonymous:
                continue
            lp = layer.pseudoRoot
            for key, value, ok in (("metersPerUnit", meters, meters_ok), ("upAxis", up, up_ok)):
                if ok and lp.HasInfo(key):
                    other = lp.GetInfo(key)
                    same = abs(other - value) <= 1e-12 * value if key == "metersPerUnit" \
                        else other == value
                    v.check(same, "composition.units-mismatch", layer.identifier,
                            "%s is %s; the root layer's is %s (USD does not convert)"
                            % (key, other, value))

    def composition(self):
        errors = self.stage.GetCompositionErrors()
        for error in errors:
            self.v.check(False, "composition.error", "/",
                         "%s: %s" % (type(error).__name__, " ".join(str(error).split())))
        self.v.check(True, "composition.error", "/", "")
        return not errors

    # ----------------------------------------------------------- prim level

    def sourcemap_properties(self, prim):
        return [p for p in prim.GetAuthoredProperties()
                if p.GetName().startswith(self.namespace)]

    def compiled_data(self, prim):
        compiled = self.profile["compiled_schema"]
        props = [p.GetName() for p in prim.GetAuthoredProperties()
                 if p.GetName().startswith(compiled["property_prefix"])]
        schemas = [s for s in prim.GetPrimTypeInfo().GetAppliedAPISchemas()
                   if s.split(":")[0] in compiled["api_schemas"]]
        return self.v.check(not props and not schemas, "schema.compiled-data", prim.GetPath(),
                            "compiled World Stage data is not authoring data: %s"
                            % ", ".join(props + schemas))

    def name_hint(self, prim):
        name = prim.GetName().lower()
        return next((h for h in self.profile["name_hints"] if h in name), None)

    def has_role_descendant(self, prim):
        return any(d.HasAttribute(ROLE) for d in self.Usd.PrimRange(prim) if d != prim)

    def is_geometry_or_light(self, prim):
        return prim.IsA(self.UsdGeom.Gprim) or prim.HasAPI(self.UsdLux.LightAPI)

    def roleless(self, prim):
        """A prim with no role that is not inside a role prim."""
        v, path = self.v, prim.GetPath()
        type_name = prim.GetTypeName()
        props = self.sourcemap_properties(prim)
        if type_name == "Material":
            allowed = self.profile["material"]["property"]
            extra = [p.GetName() for p in props if p.GetName() != allowed]
            v.check(not extra, "property.unknown", path,
                    "a Material may carry only %s, not %s" % (allowed, ", ".join(extra)))
            return
        if type_name == "PointInstancer":
            v.check(False, "feature.unsupported", path, "point instancers are unsupported")
            return
        hint = self.name_hint(prim)
        if hint and not self.has_role_descendant(prim):
            v.check(False, "role.name-only", path,
                    "named like %s but declares no %s; roles are never inferred from names"
                    % (hint, ROLE))
            return
        if self.is_geometry_or_light(prim) or props:
            v.check(False, "role.missing", path,
                    "%s prim%s has no %s" % (type_name or "untyped",
                                            " with sourcemap: data" if props else "", ROLE))
            return
        v.check(type_name in self.profile["structural_prim_types"], "feature.unsupported",
                path, "prim type %s is not part of the profile" % type_name)

    def role_prim(self, prim):
        v, path = self.v, prim.GetPath()
        attr = prim.GetAttribute(ROLE)
        if not v.check(str(attr.GetTypeName()) == "token", "property.type", path,
                       "%s must be a token, not %s" % (ROLE, attr.GetTypeName())):
            return
        role = attr.Get()
        spec = self.profile["roles"].get(role)
        if not v.check(spec is not None, "role.unknown", path, "unknown role %r" % role):
            return
        if not v.check(spec["support"] == "supported", "role.unsupported", path,
                       "role %s is reserved for %s, not profile version %d"
                       % (role, spec.get("phase", "a later profile"), self.profile["version"])):
            return
        if not v.check(prim.GetTypeName() in spec["prim_types"],
                       spec.get("prim_type_code", "role.prim-type"), path,
                       "role %s needs a %s prim, not %s"
                       % (role, " or ".join(spec["prim_types"]), prim.GetTypeName() or "untyped")):
            return
        if not self.children(prim, spec):
            return
        values = self.properties(prim, role, spec)
        record = {"id": values.get("sourcemap:id"), "role": role, "path": str(path)}
        if not self.static(prim):
            return
        rows = self.transform(prim, spec)
        if rows is None:
            return
        if "sourcemap:classname" in values:
            record["classname"] = values["sourcemap:classname"]
        if "sourcemap:model" in values:
            record["model"] = values["sourcemap:model"]
        for name, key in (("sourcemap:collision", "collision"), ("sourcemap:skin", "skin"),
                          ("sourcemap:massScale", "mass_scale")):
            if name in values:
                record[key] = values[name]
        if spec["transform"] == "rigid":
            self.placement(rows, record)
        if spec.get("geometry") == "convex_solid":
            self.solid(prim, spec, rows, record)
        if role == "light":
            self.light(prim, spec, record)
        self.objects.append(record)

    def children(self, prim, spec):
        family = self.profile["material"]["subset_family"]
        bad = []
        for child in self.Usd.PrimRange(prim):
            if child == prim:
                continue
            subset = child.GetTypeName() == "GeomSubset" and "Mesh" in spec["prim_types"] and \
                self.UsdGeom.Subset(child).GetFamilyNameAttr().Get() == family
            if not subset:
                bad.append(str(child.GetPath()))
        return self.v.check(not bad, "role.children-unsupported", prim.GetPath(),
                            "only %s face subsets may be nested: %s" % (family, ", ".join(bad)))

    def properties(self, prim, role, spec):
        v, path, values = self.v, prim.GetPath(), {}
        declared = spec["properties"]
        for name, rule in declared.items():
            attr = prim.GetAttribute(name)
            present = bool(attr) and attr.HasAuthoredValue()
            if not present:
                if rule["required"]:
                    v.check(False, rule.get("missing_code", "property.missing"), path,
                            "role %s requires %s" % (role, name))
                continue
            if not v.check(str(attr.GetTypeName()) == rule["type"], "property.type", path,
                           "%s must be %s, not %s" % (name, rule["type"], attr.GetTypeName())):
                continue
            value = attr.Get()
            if "values" in rule:
                reserved = value in rule.get("reserved", [])
                if not v.check(value in rule["values"],
                               rule.get("unsupported_code", "property.value-unsupported"), path,
                               "%s = %s is %s; profile v%d supports %s"
                               % (name, value, "reserved" if reserved else "unknown",
                                  self.profile["version"], ", ".join(rule["values"]))):
                    continue
            if rule.get("model") and not v.check(
                    re.match(self.profile["model_pattern"], value) is not None,
                    "prop.model-invalid", path,
                    "%s %r is not a lowercase relative models/....mdl game path" % (name, value)):
                continue
            if "min" in rule and not v.check(value >= rule["min"], "property.value-unsupported",
                                             path, "%s = %s < %s" % (name, value, rule["min"])):
                continue
            if rule.get("positive") and not v.check(
                    math.isfinite(value) and value > 0, "property.value-unsupported", path,
                    "%s = %s is not positive" % (name, value)):
                continue
            values[name] = list(value) if rule["type"].endswith("[]") else value
        allowed = set(declared) | {ROLE}
        for prop in self.sourcemap_properties(prim):
            v.check(prop.GetName() in allowed, "property.unknown", path,
                    "role %s does not declare %s" % (role, prop.GetName()))
        ident = values.get("sourcemap:id")
        if ident is not None and not v.check(
                re.match(self.profile["id_pattern"], ident) is not None, "id.invalid", path,
                "id %r does not match %s" % (ident, self.profile["id_pattern"])):
            values.pop("sourcemap:id")
        return values

    def static(self, prim):
        """No time samples on the object or anything that places it."""
        varying = [a.GetName() for a in prim.GetAttributes() if a.GetNumTimeSamples() > 0]
        parent = prim.GetParent()
        while parent and not parent.IsPseudoRoot():
            xformable = self.UsdGeom.Xformable(parent)
            if xformable and xformable.TransformMightBeTimeVarying():
                varying.append(str(parent.GetPath()) + " transform")
            parent = parent.GetParent()
        if prim.IsInstance() or prim.IsInstanceable():
            return self.v.check(False, "feature.unsupported", prim.GetPath(),
                                "instancing is unsupported; author each object")
        return self.v.check(not varying, "feature.unsupported", prim.GetPath(),
                            "time samples are unsupported: %s" % ", ".join(varying))

    def transform(self, prim, spec):
        v, path = self.v, prim.GetPath()
        matrix = self.UsdGeom.Xformable(prim).ComputeLocalToWorldTransform(
            self.Usd.TimeCode.Default())
        rows = matrix_rows(matrix)
        if not v.check(all(math.isfinite(x) for row in rows for x in row),
                       "transform.non-finite", path, "the local-to-world matrix is not finite"):
            return None
        det = determinant3(rows)
        if not v.check(abs(det) > 1e-12, "transform.singular", path,
                       "the local-to-world matrix is singular"):
            return None
        if spec["transform"] == "rigid":
            # Judged in Source space: model assets are in Source units, so a
            # centimetre stage places them under a 2.54 scale.
            tol, s = self.tol["rigid"], self.conversion.scale
            axes = [scale(tuple(rows[i][:3]), s) for i in range(3)]
            ortho = all(abs(dot(axes[i], axes[j]) - (1.0 if i == j else 0.0)) <= tol
                        for i in range(3) for j in range(3))
            if not v.check(ortho and det > 0, "transform.not-rigid", path,
                           "role needs rotation and translation only in Source units "
                           "(det %.6g)" % (det * s ** 3)):
                return None
        return rows

    def placement(self, rows, record):
        c = self.conversion
        frame = [scale(c.direction(tuple(rows[i][:3])), c.scale) for i in range(3)]
        record["origin"] = [round(x, 6) + 0.0 for x in c.point(tuple(rows[3][:3]))]
        record["angles"] = source_angles(*frame)

    # ------------------------------------------------------------- solids

    def solid(self, prim, spec, rows, record):
        v, path = self.v, prim.GetPath()
        mesh = self.UsdGeom.Mesh(prim)
        counts = list(mesh.GetFaceVertexCountsAttr().Get() or [])
        indices = list(mesh.GetFaceVertexIndicesAttr().Get() or [])
        local = [tuple(p) for p in (mesh.GetPointsAttr().Get() or [])]
        if not v.check(counts and indices and local, "geometry.missing", path,
                       "a %s needs a Mesh with points and faces" % record["role"]):
            return
        faces = self.surface_ids(prim, spec, len(counts), record)
        geometry = self.solid_geometry(prim, mesh, counts, indices, local, rows)
        points = None
        if geometry is not None:
            points, polygons, planes = geometry
            record["bounds"] = [[round(min(p[a] for p in points), 6) + 0.0 for a in range(3)],
                                [round(max(p[a] for p in points), 6) + 0.0 for a in range(3)]]
            record["faces"] = [source_face(points, polygon, plane)
                               for polygon, plane in zip(polygons, planes)]
        if record["role"] == "world_solid":
            self.materials(prim, mesh, len(counts), faces, record)
            if points is not None:  # texture checks need planar, well-formed faces
                self.texture_coordinates(prim, mesh, counts, indices, local, rows, record)

    def solid_geometry(self, prim, mesh, counts, indices, local, rows):
        """Ordered topology and shape checks; the first failure ends the chain.
        Returns (Source-space points, outward polygons, planes) for a valid
        solid, else None."""
        v, path, tol = self.v, prim.GetPath(), self.tol
        if not v.check(mesh.GetSubdivisionSchemeAttr().Get() == "none", "geometry.subdivision",
                       path, "subdivisionScheme is %s; brush solids are polygonal"
                       % mesh.GetSubdivisionSchemeAttr().Get()):
            return None
        if not v.check(all(math.isfinite(x) for p in local for x in p), "geometry.non-finite",
                       path, "a point is not finite"):
            return None
        points = [self.conversion.point(transform_point(rows, p)) for p in local]
        polygons, start = [], 0
        for face, count in enumerate(counts):
            polygon = indices[start:start + count]
            start += count
            valid = count >= 3 and len(polygon) == count and len(set(polygon)) == count and \
                all(0 <= i < len(points) for i in polygon)
            area = 0.5 * length(newell([points[i] for i in polygon])) if valid else 0.0
            if not v.check(valid and area > tol["area"], "geometry.degenerate-face", path,
                           "face %d has %s" % (face, "invalid vertex indices" if not valid
                                               else "area %.3g" % area)):
                return None
            polygons.append(polygon)
        if not v.check(start == len(indices), "geometry.degenerate-face", path,
                       "faceVertexIndices has %d entries for %d" % (len(indices), start)):
            return None
        # Face orientation: USD's orientation flips under a mirroring transform.
        flip = (self.UsdGeom.Mesh(prim).GetOrientationAttr().Get() == "leftHanded") != \
            (determinant3(rows) < 0)
        if flip:
            polygons = [list(reversed(p)) for p in polygons]
        planes = []
        for face, polygon in enumerate(polygons):
            corners = [points[i] for i in polygon]
            n = newell(corners)
            n = scale(n, 1.0 / length(n))
            d = sum(dot(n, c) for c in corners) / len(corners)
            worst = max(abs(dot(n, c) - d) for c in corners)
            if not v.check(worst <= tol["distance"], "geometry.non-planar-face", path,
                           "face %d is %.4g units off its plane" % (face, worst)):
                return None
            planes.append((n, d))
        edges = {}
        for polygon in polygons:
            for i, a in enumerate(polygon):
                edge = (a, polygon[(i + 1) % len(polygon)])
                edges[edge] = edges.get(edge, 0) + 1
        repeated = [e for e, n in edges.items() if n > 1]
        if not v.check(not repeated, "geometry.non-manifold", path,
                       "edge %s is used %d times in one direction"
                       % (repeated[0], edges[repeated[0]]) if repeated else ""):
            return None
        unmatched = [e for e in edges if (e[1], e[0]) not in edges]
        if unmatched:
            welded = len({tuple(round(x / tol["distance"]) for x in p) for p in points})
            hint = " (points coincide: weld them)" if welded < len(points) else ""
        if not v.check(not unmatched, "geometry.open", path,
                       "%d edges have no opposite edge, e.g. %s%s"
                       % (len(unmatched), unmatched[0], hint) if unmatched else ""):
            return None
        volume = 0.0
        for polygon in polygons:
            p0 = points[polygon[0]]
            for i in range(1, len(polygon) - 1):
                volume += dot(p0, cross(points[polygon[i]], points[polygon[i + 1]])) / 6.0
        if not v.check(volume > 0, "geometry.inside-out", path,
                       "encloses %.6g cubic units; faces must wind outward" % volume):
            return None
        # On a closed solid this also rejects every concave face polygon.
        for face, (n, d) in enumerate(planes):
            ahead = max(dot(n, p) - d for p in points)
            if not v.check(ahead <= tol["distance"], "geometry.non-convex", path,
                           "a vertex is %.4g units in front of face %d" % (ahead, face)):
                return None
        for a in range(len(planes)):
            for b in range(a + 1, len(planes)):
                (na, da), (nb, db) = planes[a], planes[b]
                same = dot(na, nb) >= self.tol["coplanar_normal_dot"] and \
                    abs(da - db) <= tol["distance"]
                if not v.check(not same, "geometry.coplanar-faces", path,
                               "faces %d and %d share a plane; merge them" % (a, b)):
                    return None
        return points, polygons, planes

    def surface_ids(self, prim, spec, face_count, record):
        values = prim.GetAttribute("sourcemap:surfaceIds")
        if not (values and values.HasAuthoredValue()) or \
                str(values.GetTypeName()) != "int[]":
            return None  # reported by the property checks
        ids = list(values.Get())
        path = prim.GetPath()
        if not self.v.check(len(ids) == face_count and all(i >= 0 for i in ids),
                            "surface.ids-invalid", path,
                            "%d surface ids for %d faces, all non-negative"
                            % (len(ids), face_count)):
            return None
        if not self.v.check(len(set(ids)) == len(ids), "surface.ids-duplicate", path,
                            "surface ids repeat within the solid"):
            return None
        record["surfaces"] = [{"surface": i} for i in ids]
        return ids

    def materials(self, prim, mesh, face_count, surface_ids, record):
        v, path = self.v, prim.GetPath()
        rule = self.profile["material"]
        family = rule["subset_family"]
        assigned = [None] * face_count
        default = self.bound_material(prim)
        if default is False:
            return
        subsets = self.UsdGeom.Subset.GetGeomSubsets(mesh, self.UsdGeom.Tokens.face, family)
        if subsets:
            valid, reason = self.UsdGeom.Subset.ValidateFamily(mesh, self.UsdGeom.Tokens.face,
                                                               family)
            if not v.check(valid, "material.invalid", path, "materialBind subsets: " + reason):
                return
        for subset in subsets:
            bound = self.bound_material(subset.GetPrim())
            if bound is False:
                return
            for face in subset.GetIndicesAttr().Get() or []:
                assigned[face] = bound
        missing = [f for f in range(face_count) if not (assigned[f] or default)]
        if not v.check(not missing, "material.missing", path,
                       "faces %s bind no Material" % missing[:8]):
            return
        names = []
        for face in range(face_count):
            material = assigned[face] or default
            attr = material.GetPrim().GetAttribute(rule["property"])
            name = attr.Get() if attr and attr.HasAuthoredValue() else None
            if not v.check(isinstance(name, str) and re.match(rule["pattern"], name) is not None,
                           "material.invalid", path,
                           "%s has %s %r; expected a materials/-relative VMT path without "
                           "extension" % (material.GetPath(), rule["property"], name)):
                return
            names.append(name)
        if surface_ids is not None:
            record["surfaces"] = [{"surface": s, "material": m}
                                  for s, m in zip(surface_ids, names)]

    def bound_material(self, prim):
        """The resolved Material, None when unbound, False after an error. The
        binding prim must apply MaterialBindingAPI: USD is phasing out bindings
        without it, so they would silently vanish in a later release."""
        material, relationship = self.UsdShade.MaterialBindingAPI(prim).ComputeBoundMaterial()
        if not material:
            return None
        owner = relationship.GetPrim()
        if not self.v.check(owner.HasAPI(self.UsdShade.MaterialBindingAPI), "material.invalid",
                            prim.GetPath(), "%s binds a Material without MaterialBindingAPI"
                            % owner.GetPath()):
            return False
        return material

    def texture_coordinates(self, prim, mesh, counts, indices, local, rows, record):
        v, path = self.v, prim.GetPath()
        primvar = self.UsdGeom.PrimvarsAPI(prim).GetPrimvar("st")
        if not v.check(bool(primvar) and primvar.HasAuthoredValue(), "uv.missing", path,
                       "world solids need primvars:st"):
            return
        st = primvar.ComputeFlattened()
        interpolation = primvar.GetInterpolation()
        expected = {"faceVarying": len(indices), "vertex": len(local)}.get(interpolation)
        if not v.check(st is not None and expected == len(st), "uv.invalid", path,
                       "primvars:st is %s with %d values; needs faceVarying (%d) or vertex (%d)"
                       % (interpolation, len(st or []), len(indices), len(local))):
            return
        points = [self.conversion.point(transform_point(rows, p)) for p in local]
        start = 0
        for face, count in enumerate(counts):
            corner = list(range(start, start + count))
            start += count
            where = corner if interpolation == "faceVarying" else [indices[c] for c in corner]
            xyz = [points[indices[c]] for c in corner]
            uv = [tuple(st[w]) for w in where]
            fit = self.affine_fit(xyz, uv)
            error = fit[0] if fit else None
            if not v.check(error is not None and error <= self.tol["uv"], "uv.not-affine", path,
                           "face %d st %s" % (face, "is degenerate" if error is None
                                              else "deviates %.3g from affine" % error)):
                return
            if "faces" in record:
                record["faces"][face]["st"] = {"s": fit[1], "t": fit[2]}

    @staticmethod
    def affine_fit(xyz, uv):
        """(largest deviation, s row, t row) of the affine map fixed by three
        corners, or None when the map is degenerate (a texinfo needs two axes).
        A row [Gx, Gy, Gz, g0] gives the coordinate G.p + g0 in repeats at the
        Source-space point p; G lies in the face plane."""
        n = newell(xyz)
        n = scale(n, 1.0 / length(n))
        u_axis = sub(xyz[1], xyz[0])
        u_axis = scale(u_axis, 1.0 / length(u_axis))
        v_axis = cross(n, u_axis)
        flat = [(dot(sub(p, xyz[0]), u_axis), dot(sub(p, xyz[0]), v_axis)) for p in xyz]
        # The corner that makes the largest triangle with the first two.
        k = max(range(2, len(flat)), key=lambda i: abs(flat[1][0] * flat[i][1]
                                                       - flat[1][1] * flat[i][0]))
        (x1, y1), (x2, y2) = flat[1], flat[k]
        det = x1 * y2 - y1 * x2
        if abs(det) < 1e-12:
            return None
        du1, dv1 = uv[1][0] - uv[0][0], uv[1][1] - uv[0][1]
        du2, dv2 = uv[k][0] - uv[0][0], uv[k][1] - uv[0][1]
        # st = uv0 + [x y] * [[a b] [c d]]
        a = (du1 * y2 - du2 * y1) / det
        c = (du2 * x1 - du1 * x2) / det
        b = (dv1 * y2 - dv2 * y1) / det
        d = (dv2 * x1 - dv1 * x2) / det
        if abs(a * d - b * c) < 1e-12:
            return None
        error = max(max(abs(uv[0][0] + x * a + y * c - s), abs(uv[0][1] + x * b + y * d - t))
                    for (x, y), (s, t) in zip(flat, uv))
        rows = []
        for along_u, along_v, origin in ((a, c, uv[0][0]), (b, d, uv[0][1])):
            gradient = tuple(along_u * u_axis[i] + along_v * v_axis[i] for i in range(3))
            rows.append([exact(g) for g in gradient] + [exact(origin - dot(gradient, xyz[0]))])
        return error, rows[0], rows[1]

    # ------------------------------------------------------------- lights

    def light(self, prim, spec, record):
        v, path = self.v, prim.GetPath()
        shaped = [s for s in prim.GetPrimTypeInfo().GetAppliedAPISchemas()
                  if s in spec["unsupported_api_schemas"]]
        if not v.check(not shaped, "light.type-unsupported", path,
                       "%s is unsupported in profile v%d" % (", ".join(shaped),
                                                             self.profile["version"])):
            return
        values = {}
        for name in spec["required_inputs"]:
            attr = prim.GetAttribute(name)
            if not v.check(bool(attr) and attr.HasAuthoredValue(), "light.inputs", path,
                           "%s is not authored" % name):
                return
            values[name] = attr.Get()
        intensity = values["inputs:intensity"]
        if v.check(math.isfinite(intensity) and intensity > 0, "light.inputs", path,
                   "inputs:intensity %s is not positive" % intensity):
            record["intensity"] = intensity
            record["color"] = [round(float(x), 6) for x in values["inputs:color"]]
            # The remaining UsdLux inputs, authored or fallback, for the
            # compiler's light policy; the radius is in Source units.
            api = self.UsdLux.LightAPI(prim)
            record["inputs"] = {
                "exposure": float(api.GetExposureAttr().Get()),
                "diffuse": float(api.GetDiffuseAttr().Get()),
                "normalize": bool(api.GetNormalizeAttr().Get()),
                "enableColorTemperature": bool(api.GetEnableColorTemperatureAttr().Get()),
                "radius": exact(float(self.UsdLux.SphereLight(prim).GetRadiusAttr().Get())
                                * self.conversion.scale)}

    # --------------------------------------------------------------- stage

    def run(self):
        self.stage_metadata()
        if not self.composition():
            return
        role_prims = []
        iterator = iter(self.Usd.PrimRange(self.stage.GetPseudoRoot()))
        for prim in iterator:
            if prim.IsPseudoRoot():
                continue
            self.compiled_data(prim)
            if prim.HasAttribute(ROLE) and prim.GetAttribute(ROLE).HasAuthoredValue():
                role_prims.append(prim)
                iterator.PruneChildren()  # role_prim checks its own descendants
            else:
                self.roleless(prim)
        for prim in role_prims:
            self.role_prim(prim)
        self.stage_objects()

    def stage_objects(self):
        seen = {}
        for record in self.objects:
            ident = record["id"]
            if ident is None:
                continue
            if ident in seen:
                self.v.check(False, "id.duplicate", record["path"],
                             "id %r is also used by %s" % (ident, seen[ident]))
            else:
                self.v.check(True, "id.duplicate", record["path"], "")
                seen[ident] = record["path"]
        starts = [r for r in self.objects if r.get("classname") == "info_player_start"]
        self.v.check(starts, "map.player-start-missing", "/",
                     "the map needs at least one info_player_start")


def open_stage(path):
    from pxr import Usd
    try:
        return Usd.Stage.Open(path, Usd.Stage.LoadAll)
    except Exception:  # pxr raises Tf.ErrorException for unreadable layers
        return None


def validate(path, previous=None, profile=None):
    """The report dict for the stage at `path`; `previous` is a stage path or an
    earlier report (dict or JSON path) for the id-stability checks."""
    from pxr import Usd, UsdGeom, UsdLux, UsdShade
    profile = profile or load_profile()
    validation = Validation(profile)
    path = os.path.abspath(path)
    stage = open_stage(path) if os.path.isfile(path) else None
    objects = []
    layers = []
    if validation.check(stage is not None, "stage.open-failed", path,
                        "cannot open %s as a USD stage" % path):
        checker = StageValidator((Usd, UsdGeom, UsdShade, UsdLux), profile, stage, validation)
        checker.run()
        objects = checker.objects
        layers = sorted(layer.identifier for layer in stage.GetUsedLayers()
                        if not layer.anonymous)
    report = {"schema": REPORT_SCHEMA, "profile": profile["profile"],
              "version": profile["version"], "stage": path, "layers": layers,
              "checks": validation.checks, "errors": validation.errors,
              "objects": sorted(objects, key=lambda r: (r["id"] or "", r["path"]))}
    if previous is not None:
        report["diff"] = stability(validation, previous, objects)
        report["checks"], report["errors"] = validation.checks, validation.errors
    return report


def stability(validation, previous, objects):
    """Ids persist across revisions and never change role."""
    if isinstance(previous, str):
        if previous.endswith(".json"):
            with open(previous, "r", encoding="utf-8") as stream:
                previous = json.load(stream)
        else:
            previous = validate(previous)
    before = {r["id"]: r for r in previous.get("objects", []) if r.get("id")}
    after = {r["id"]: r for r in objects if r.get("id")}
    for ident in sorted(set(before) & set(after)):
        validation.check(before[ident]["role"] == after[ident]["role"], "id.role-changed",
                         after[ident]["path"],
                         "id %r was a %s at %s; a new role needs a new id"
                         % (ident, before[ident]["role"], before[ident]["path"]))
    return {"added": sorted(set(after) - set(before)),
            "removed": sorted(set(before) - set(after)),
            "moved": sorted(i for i in set(before) & set(after)
                            if before[i]["path"] != after[i]["path"])}


def main():
    parser = argparse.ArgumentParser(
        description="Validate a USD map authoring stage against source-authoring v1.")
    sub_parsers = parser.add_subparsers(dest="command", required=True)
    run = sub_parsers.add_parser("validate", help="validate one stage")
    run.add_argument("stage")
    run.add_argument("--previous", help="earlier stage or --json report for id stability")
    run.add_argument("--json", help="write the machine-readable report here")
    run.add_argument("--profile", default=PROFILE_PATH)
    args = parser.parse_args()

    sys.path.insert(0, HERE)
    from conformance_result import report_conformance

    report = validate(args.stage, args.previous, load_profile(args.profile))
    for error in report["errors"]:
        print("FAIL %s %s: %s" % (error["code"], error["path"], error["message"]))
    if args.json:
        with open(args.json, "w", encoding="utf-8") as stream:
            json.dump(report, stream, indent=2, sort_keys=True)
            stream.write("\n")
    return report_conformance(report["checks"], len(report["errors"]))


if __name__ == "__main__":
    sys.exit(main())
