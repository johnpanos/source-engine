"""Oracles for the USD map authoring validator (`usd_authoring_validate.py`,
RFC 0009 U0-U2); needs OpenUSD's `pxr`.

    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 -m unittest tools.quality.tests.test_usd_authoring -v

Every bad variant is generated from a private copy of a good fixture
(quality/fixtures/usd-authoring/room, profile v1, or .../roles, profile v2) by
editing the layer that owns the data, and must produce exactly its intended
error code at its intended prim.
"""

import json
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

try:
    from pxr import Gf, Sdf, Usd, UsdGeom, UsdShade
except ImportError as error:  # system Python has no OpenUSD; see the docstring
    raise unittest.SkipTest("test_usd_authoring needs OpenUSD's pxr: %s" % error)

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/quality"))
import usd_authoring_validate as validator  # noqa: E402

FIXTURE = ROOT / "quality/fixtures/usd-authoring/room"
ROLES = ROOT / "quality/fixtures/usd-authoring/roles"
VALIDATOR = ROOT / "tools/quality/usd_authoring_validate.py"
PROFILE = validator.load_profile()
LAYERS = {"room": "room.usda", "world": "world.usda", "entities": "entities.usda",
          "prefab": "prefabs/metal_box.usda"}
FLOOR = "/Map/World/Floor"
DESK = "/Map/Props/LabDesk"
BOX = "/Map/Props/MetalBox"
TRIGGER = "/Map/Entities/Trigger"
LAMP = "/Map/Lights/CeilingLamp"
START = "/Map/Entities/info_player_start"
SWITCH = "/Map/Props/LeverSwitch"
LIFT = "/Map/Entities/Lift"
DROP_TRIGGER = "/Map/Entities/DropTrigger"
GHOST = "/Map/Props/GhostDesk"


# ------------------------------------------------------------- fixture edits

class Room:
    """A private copy of the fixture whose layers are edited and saved."""

    def __init__(self, parent):
        self.directory = Path(parent) / ("room%d" % Room.serial)
        Room.serial += 1
        shutil.copytree(FIXTURE, self.directory)
        self.layers = {k: Sdf.Layer.FindOrOpen(str(self.directory / v))
                       for k, v in LAYERS.items()}
        self.stage = Usd.Stage.Open(str(self.root))

    serial = 0

    @property
    def root(self):
        return self.directory / "room.usda"

    def spec(self, layer, path):
        return self.layers[layer].GetPrimAtPath(path)

    def prim(self, layer, path):
        """The composed prim, with edits going to `layer`."""
        self.stage.SetEditTarget(Usd.EditTarget(self.layers[layer]))
        return self.stage.GetPrimAtPath(path)

    def remove(self, layer, path, name):
        spec = self.spec(layer, path)
        spec.RemoveProperty(spec.properties[name])

    def set(self, layer, path, name, value, type_name=Sdf.ValueTypeNames.Token):
        prim = self.prim(layer, path)
        attr = prim.GetAttribute(name) or prim.CreateAttribute(name, type_name)
        attr.Set(value)

    def save(self):
        """Write the edits and drop every handle, so the validator reopens the
        layers from disk (a deleted layer must not resolve from memory)."""
        for layer in self.layers.values():
            if layer.dirty:
                layer.Save()
        self.layers, self.stage = None, None
        return str(self.root)


class Roles(Room):
    """A private copy of the U2 role fixture, which shares the room's world
    and prefab layers by relative path, so both fixture directories are
    copied."""

    LAYERS = {"room": "roles/roles.usda", "entities": "roles/entities.usda",
              "world": "room/world.usda", "prefab": "room/prefabs/metal_box.usda"}

    def __init__(self, parent):
        self.directory = Path(parent) / ("roles%d" % Room.serial)
        Room.serial += 1
        shutil.copytree(FIXTURE.parent, self.directory)
        self.layers = {k: Sdf.Layer.FindOrOpen(str(self.directory / v))
                       for k, v in self.LAYERS.items()}
        self.stage = Usd.Stage.Open(str(self.root))

    @property
    def root(self):
        return self.directory / "roles/roles.usda"


def st_for(points, polygon):
    """World-aligned st (128 units per repeat) on the face's dominant axis."""
    corners = [points[i] for i in polygon]
    n = validator.newell(corners)
    axis = max(range(3), key=lambda a: abs(n[a]))
    keep = [a for a in range(3) if a != axis]
    return [(p[keep[0]] / 128.0, p[keep[1]] / 128.0) for p in corners]


def set_solid(room, layer, path, points, faces, surface_ids=True):
    mesh = UsdGeom.Mesh(room.prim(layer, path))
    mesh.GetPointsAttr().Set([Gf.Vec3f(*p) for p in points])
    mesh.GetFaceVertexCountsAttr().Set([len(f) for f in faces])
    mesh.GetFaceVertexIndicesAttr().Set([i for f in faces for i in f])
    st = [uv for f in faces for uv in st_for(points, f)]
    UsdGeom.PrimvarsAPI(mesh).GetPrimvar("st").Set([Gf.Vec2f(*uv) for uv in st])
    if surface_ids:
        room.set(layer, path, "sourcemap:surfaceIds", list(range(len(faces))),
                 Sdf.ValueTypeNames.IntArray)


BOX_FACES = [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (2, 3, 7, 6), (1, 2, 6, 5), (3, 0, 4, 7)]


def box_points(lo, hi):
    (x0, y0, z0), (x1, y1, z1) = lo, hi
    return [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
            (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]


FLOOR_POINTS = box_points((-272, -208, -16), (272, 208, 0))


def l_prism():
    """A closed, planar, outward-wound but non-convex L-shaped floor."""
    outline = [(-272, -208), (0, -208), (0, 0), (-136, 0), (-136, 208), (-272, 208)]
    points = [(x, y, -16) for x, y in outline] + [(x, y, 0) for x, y in outline]
    n = len(outline)
    faces = [tuple(reversed(range(n))), tuple(range(n, 2 * n))]
    faces += [(i, (i + 1) % n, (i + 1) % n + n, i + n) for i in range(n)]
    return points, faces


def dented_top():
    """The floor box with its top pushed in to a point: every face is convex,
    the solid is not."""
    points = FLOOR_POINTS + [(0, 0, -8)]
    faces = BOX_FACES[:1] + [(4, 5, 8), (5, 6, 8), (6, 7, 8), (7, 4, 8)] + BOX_FACES[2:]
    return points, faces


def split_top():
    """The floor box with its top face split in two: two coplanar faces."""
    points = FLOOR_POINTS + [(0, -208, 0), (0, 208, 0)]
    faces = [(0, 3, 2, 1), (4, 8, 9, 7), (8, 5, 6, 9), (0, 1, 5, 8, 4), (2, 3, 7, 9, 6),
             (1, 2, 6, 5), (3, 0, 4, 7)]
    return points, faces


def add_xform_op(room, layer, path, kind, value):
    xformable = UsdGeom.Xformable(room.prim(layer, path))
    op = {"scale": xformable.AddScaleOp, "translate": xformable.AddTranslateOp}[kind](
        opSuffix="u0")
    op.Set(value)
    order = xformable.GetOrderedXformOps()
    xformable.SetXformOpOrder(order)


def apply_schema(room, layer, path, schema):
    spec = room.spec(layer, path)
    current = spec.GetInfo("apiSchemas")
    spec.SetInfo("apiSchemas", Sdf.TokenListOp.Create(
        prependedItems=list(current.prependedItems) + [schema]))


def new_prim(room, layer, path, type_name, **attrs):
    stage = room.stage
    stage.SetEditTarget(Usd.EditTarget(room.layers[layer]))
    prim = stage.DefinePrim(path, type_name)
    for name, (type_, value) in attrs.items():
        prim.CreateAttribute(name.replace("__", ":"), type_).Set(value)
    return prim


# Each case: (name, edit(room) -> optional validate kwargs, code, prim path).
S = Sdf.ValueTypeNames
CASES = [
    # --- stage and profile
    ("profile missing", lambda r: setattr(r.layers["room"], "customLayerData", {}),
     "stage.profile-missing", "/"),
    ("profile version", lambda r: setattr(r.layers["room"], "customLayerData",
                                          {"sourcemap": {"profile": "source-authoring",
                                                         "version": 3}}),
     "stage.profile-version", "/"),
    ("default prim missing", lambda r: r.layers["room"].ClearDefaultPrim(),
     "stage.default-prim-missing", "/"),
    ("units metadata missing", lambda r: r.layers["room"].pseudoRoot.ClearInfo("metersPerUnit"),
     "stage.units", "/"),
    ("up axis missing", lambda r: r.layers["room"].pseudoRoot.ClearInfo("upAxis"),
     "stage.up-axis", "/"),
    # --- composition
    ("sublayer units mismatch",
     lambda r: r.layers["entities"].pseudoRoot.SetInfo("metersPerUnit", 0.01),
     "composition.units-mismatch", "entities.usda"),
    ("unresolved prefab reference",
     lambda r: os.remove(r.directory / LAYERS["prefab"]), "composition.error", "/"),
    ("unresolved sublayer", lambda r: os.remove(r.directory / LAYERS["world"]),
     "composition.error", "/"),
    ("compiled schema applied", lambda r: apply_schema(r, "world", FLOOR, "SourceMeshAPI"),
     "schema.compiled-data", FLOOR),
    ("compiled property", lambda r: r.set("world", FLOOR, "source:materialPath",
                                          "metal/metalwall048b", S.String),
     "schema.compiled-data", FLOOR),
    # --- unsupported features
    ("instanced prop", lambda r: setattr(r.spec("entities", BOX), "instanceable", True),
     "feature.unsupported", BOX),
    ("time-sampled transform",
     lambda r: r.prim("entities", DESK).GetAttribute("xformOp:translate").Set(
         Gf.Vec3d(0, 0, 0), 1.0), "feature.unsupported", DESK),
    ("point instancer", lambda r: new_prim(r, "entities", "/Map/Props/Scatter",
                                           "PointInstancer"),
     "feature.unsupported", "/Map/Props/Scatter"),
    ("unsupported prim type", lambda r: new_prim(r, "entities", "/Map/Rig", "Skeleton"),
     "feature.unsupported", "/Map/Rig"),
    # --- roles
    ("missing role", lambda r: r.remove("world", FLOOR, "sourcemap:role"),
     "role.missing", FLOOR),
    ("role given only by name",
     lambda r: new_prim(r, "entities", "/Map/Props/prop_physics_crate", "Xform",
                        sourcemap__model=(S.String, "models/props/metal_box.mdl")),
     "role.name-only", "/Map/Props/prop_physics_crate"),
    ("unknown role", lambda r: r.set("entities", DESK, "sourcemap:role", "prop_ragdoll"),
     "role.unknown", DESK),
    ("role newer than the stage", lambda r: r.set("entities", DESK, "sourcemap:role",
                                                  "prop_dynamic"),
     "role.unsupported", DESK),
    ("role on the wrong prim type",
     lambda r: setattr(r.spec("entities", DESK), "typeName", "Mesh"), "role.prim-type", DESK),
    ("nested prim under a prop", lambda r: new_prim(r, "entities", DESK + "/Handle", "Xform"),
     "role.children-unsupported", DESK),
    # --- properties
    ("contents missing", lambda r: r.remove("world", FLOOR, "sourcemap:contents"),
     "property.missing", FLOOR),
    ("collision as a string",
     lambda r: (r.remove("entities", DESK, "sourcemap:collision"),
                r.set("entities", DESK, "sourcemap:collision", "vphysics", S.String)),
     "property.type", DESK),
    ("reserved contents", lambda r: r.set("world", FLOOR, "sourcemap:contents", "detail"),
     "property.value-unsupported", FLOOR),
    ("undeclared property", lambda r: r.set("entities", DESK, "sourcemap:frobnicate", 1, S.Int),
     "property.unknown", DESK),
    # --- ids
    ("missing id", lambda r: r.remove("entities", DESK, "sourcemap:id"), "id.missing", DESK),
    ("invalid id", lambda r: r.set("entities", DESK, "sourcemap:id", "Lab Desk", S.String),
     "id.invalid", DESK),
    ("duplicate id", lambda r: r.set("world", "/Map/World/Ceiling", "sourcemap:id",
                                     "world.floor", S.String),
     "id.duplicate", "/Map/World/Ceiling"),
    ("id reused for another role",
     lambda r: (r.set("entities", DESK, "sourcemap:role", "prop_physics"),
                r.remove("entities", DESK, "sourcemap:collision"),
                {"previous": str(FIXTURE / "room.usda")}),
     "id.role-changed", DESK),
    # --- transforms
    ("scaled prop", lambda r: add_xform_op(r, "entities", DESK, "scale", Gf.Vec3f(2, 2, 2)),
     "transform.not-rigid", DESK),
    ("mirrored prop", lambda r: add_xform_op(r, "entities", DESK, "scale", Gf.Vec3f(-1, 1, 1)),
     "transform.not-rigid", DESK),
    ("flattened solid", lambda r: add_xform_op(r, "world", FLOOR, "scale", Gf.Vec3f(1, 1, 0)),
     "transform.singular", FLOOR),
    ("non-finite placement",
     lambda r: r.prim("entities", DESK).GetAttribute("xformOp:translate").Set(
         Gf.Vec3d(float("nan"), 0, 0)), "transform.non-finite", DESK),
    # --- geometry
    ("trigger without geometry",
     lambda r: setattr(r.spec("entities", TRIGGER), "typeName", "Xform"),
     "geometry.missing", TRIGGER),
    ("solid without points",
     lambda r: UsdGeom.Mesh(r.prim("world", FLOOR)).GetPointsAttr().Set([]),
     "geometry.missing", FLOOR),
    ("subdivided solid", lambda r: r.set("world", FLOOR, "subdivisionScheme", "catmullClark"),
     "geometry.subdivision", FLOOR),
    ("non-finite point",
     lambda r: set_solid(r, "world", FLOOR, [(float("nan"), 0, 0)] + FLOOR_POINTS[1:],
                         BOX_FACES), "geometry.non-finite", FLOOR),
    ("out-of-range index",
     lambda r: UsdGeom.Mesh(r.prim("world", FLOOR)).GetFaceVertexIndicesAttr().Set(
         [i for f in BOX_FACES[:5] + [(3, 0, 4, 99)] for i in f]),
     "geometry.degenerate-face", FLOOR),
    ("non-planar face",
     lambda r: set_solid(r, "world", FLOOR, FLOOR_POINTS[:6] + [(272, 208, 4)]
                         + FLOOR_POINTS[7:], BOX_FACES), "geometry.non-planar-face", FLOOR),
    ("duplicated face",
     lambda r: set_solid(r, "world", FLOOR, FLOOR_POINTS, BOX_FACES + [BOX_FACES[1]]),
     "geometry.non-manifold", FLOOR),
    ("open solid", lambda r: set_solid(r, "world", FLOOR, FLOOR_POINTS, BOX_FACES[:5]),
     "geometry.open", FLOOR),
    ("inside-out solid", lambda r: r.set("world", FLOOR, "orientation", "leftHanded"),
     "geometry.inside-out", FLOOR),
    ("mirrored left-handed solid",
     lambda r: (r.set("world", FLOOR, "orientation", "leftHanded"),
                add_xform_op(r, "world", FLOOR, "scale", Gf.Vec3f(-1, 1, 1))),
     "geometry.inside-out", FLOOR),
    ("non-convex solid", lambda r: set_solid(r, "world", FLOOR, *l_prism()),
     "geometry.non-convex", FLOOR),
    ("dented solid with convex faces", lambda r: set_solid(r, "world", FLOOR, *dented_top()),
     "geometry.non-convex", FLOOR),
    ("coplanar faces", lambda r: set_solid(r, "world", FLOOR, *split_top()),
     "geometry.coplanar-faces", FLOOR),
    # --- surfaces, texture coordinates and materials
    ("surface ids missing", lambda r: r.remove("world", FLOOR, "sourcemap:surfaceIds"),
     "surface.ids-missing", FLOOR),
    ("surface ids short", lambda r: r.set("world", FLOOR, "sourcemap:surfaceIds",
                                          [0, 1, 2, 3, 4], S.IntArray),
     "surface.ids-invalid", FLOOR),
    ("surface ids repeat", lambda r: r.set("world", FLOOR, "sourcemap:surfaceIds",
                                           [0, 1, 2, 3, 4, 4], S.IntArray),
     "surface.ids-duplicate", FLOOR),
    ("st missing", lambda r: r.remove("world", FLOOR, "primvars:st"), "uv.missing", FLOOR),
    ("st uniform", lambda r: UsdGeom.PrimvarsAPI(r.prim("world", FLOOR)).GetPrimvar(
        "st").SetInterpolation(UsdGeom.Tokens.uniform), "uv.invalid", FLOOR),
    ("st not affine", lambda r: perturb_st(r, 3, 0.5), "uv.not-affine", FLOOR),
    ("material unbound", lambda r: r.remove("world", FLOOR, "material:binding"),
     "material.missing", FLOOR),
    ("material path with extension",
     lambda r: r.set("world", "/Map/Looks/Floor", "sourcemap:material",
                     "materials/metal/metal_modular_floor001.vmt", S.String),
     "material.invalid", FLOOR),
    ("binding without MaterialBindingAPI",
     lambda r: r.spec("world", FLOOR).ClearInfo("apiSchemas"), "material.invalid", FLOOR),
    ("material subset out of range", lambda r: bind_top_face(r, [1, 6]),
     "material.invalid", FLOOR),
    # --- props, entities, lights and the map
    ("prop without model", lambda r: r.remove("entities", DESK, "sourcemap:model"),
     "prop.model-missing", DESK),
    ("prefab without model", lambda r: r.remove("prefab", "/MetalBox", "sourcemap:model"),
     "prop.model-missing", BOX),
    ("backslash model path",
     lambda r: r.set("entities", DESK, "sourcemap:model",
                     "models\\props\\lab_desk01\\lab_desk01.mdl", S.String),
     "prop.model-invalid", DESK),
    ("escaping model path",
     lambda r: r.set("entities", DESK, "sourcemap:model", "models/../lab_desk01.mdl",
                     S.String), "prop.model-invalid", DESK),
    ("trigger without class", lambda r: r.remove("entities", TRIGGER, "sourcemap:classname"),
     "entity.class-missing", TRIGGER),
    ("point class on a trigger",
     lambda r: r.set("entities", TRIGGER, "sourcemap:classname", "info_player_start"),
     "entity.class-unsupported", TRIGGER),
    ("unsupported moving brush class",
     lambda r: r.set("entities", TRIGGER, "sourcemap:classname", "func_door"),
     "entity.class-unsupported", TRIGGER),
    ("rect light", lambda r: setattr(r.spec("entities", LAMP), "typeName", "RectLight"),
     "light.type-unsupported", LAMP),
    ("spot light", lambda r: apply_schema(r, "entities", LAMP, "ShapingAPI"),
     "light.type-unsupported", LAMP),
    ("light intensity unauthored", lambda r: r.remove("entities", LAMP, "inputs:intensity"),
     "light.inputs", LAMP),
    ("dark light", lambda r: r.set("entities", LAMP, "inputs:intensity", 0.0, S.Float),
     "light.inputs", LAMP),
    ("no player start", lambda r: setattr(r.spec("entities", START), "active", False),
     "map.player-start-missing", "/"),
]


# Bad variants of the U2 role fixture (profile v2): (name, edit, code, path).
ROLE_CASES = [
    ("physics mass on a dynamic prop",
     lambda r: r.set("entities", SWITCH, "sourcemap:massScale", 2.0, S.Float),
     "property.misplaced", SWITCH),
    ("default animation on a static prop",
     lambda r: r.set("entities", DESK, "sourcemap:defaultAnimation", "idle", S.String),
     "property.misplaced", DESK),
    ("dynamic prop parenting",
     lambda r: r.set("entities", SWITCH, "sourcemap:parent", "prop.lab-desk", S.String),
     "property.unsupported", SWITCH),
    ("unknown key on a dynamic prop",
     lambda r: r.set("entities", SWITCH, "sourcemap:renderColor", "255 0 0", S.String),
     "property.unknown", SWITCH),
    ("dynamic prop without collision", lambda r: r.remove("entities", SWITCH,
                                                          "sourcemap:collision"),
     "property.missing", SWITCH),
    ("bad animation name",
     lambda r: r.set("entities", SWITCH, "sourcemap:defaultAnimation", "wave hand", S.String),
     "property.value-unsupported", SWITCH),
    ("unknown touch filter",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:touchFilter", ["npcs"],
                     S.TokenArray),
     "property.value-unsupported", DROP_TRIGGER),
    ("lift without a move", lambda r: r.remove("entities", LIFT, "sourcemap:moveDelta"),
     "property.missing", LIFT),
    ("lift with a zero move",
     lambda r: r.set("entities", LIFT, "sourcemap:moveDelta", Gf.Vec3f(0, 0, 0), S.Float3),
     "property.value-unsupported", LIFT),
    ("lift without materials", lambda r: r.remove("entities", LIFT, "material:binding"),
     "material.missing", LIFT),
    ("move key on a trigger",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:moveSeconds", 1.0, S.Float),
     "property.misplaced", DROP_TRIGGER),
    ("connection with an unknown output",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnPressed entity.lift Open"], S.StringArray),
     "io.invalid", DROP_TRIGGER),
    ("connection without an input",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnStartTouch entity.lift"], S.StringArray),
     "io.invalid", DROP_TRIGGER),
    ("connection to a missing id",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnStartTouch entity.no-such-lift Open"], S.StringArray),
     "io.target-missing", DROP_TRIGGER),
    ("connection input the target lacks",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnStartTouch prop.lever-switch Open"], S.StringArray),
     "io.input-unsupported", DROP_TRIGGER),
    ("connection to a static prop",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnStartTouch prop.lab-desk SetAnimation down"], S.StringArray),
     "io.input-unsupported", DROP_TRIGGER),
    ("SetAnimation without a sequence",
     lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnStartTouch prop.lever-switch SetAnimation"], S.StringArray),
     "io.input-unsupported", DROP_TRIGGER),
    ("roles swapped against the previous revision",
     lambda r: (r.set("entities", GHOST, "sourcemap:role", "prop_dynamic"),
                {"previous": str(ROLES / "roles.usda")}),
     "id.role-changed", GHOST),
    # v2 features in a stage that declares v1
    ("dynamic prop in a v1 stage",
     lambda r: (r.remove("entities", DROP_TRIGGER, "sourcemap:connections"),
                r.remove("entities", DROP_TRIGGER, "sourcemap:touchFilter"),
                setattr(r.spec("entities", LIFT), "active", False),
                setattr(r.layers["room"], "customLayerData",
                        {"sourcemap": {"profile": "source-authoring", "version": 1}})),
     "role.unsupported", SWITCH),
    ("func_movelinear in a v1 stage",
     lambda r: (r.remove("entities", DROP_TRIGGER, "sourcemap:connections"),
                r.remove("entities", DROP_TRIGGER, "sourcemap:touchFilter"),
                setattr(r.spec("entities", SWITCH), "active", False),
                setattr(r.layers["room"], "customLayerData",
                        {"sourcemap": {"profile": "source-authoring", "version": 1}})),
     "entity.class-unsupported", LIFT),
    ("touch filter in a v1 stage",
     lambda r: (r.remove("entities", DROP_TRIGGER, "sourcemap:connections"),
                setattr(r.spec("entities", SWITCH), "active", False),
                setattr(r.spec("entities", LIFT), "active", False),
                setattr(r.layers["room"], "customLayerData",
                        {"sourcemap": {"profile": "source-authoring", "version": 1}})),
     "property.unsupported", DROP_TRIGGER),
]


def perturb_st(room, corner, delta):
    primvar = UsdGeom.PrimvarsAPI(room.prim("world", FLOOR)).GetPrimvar("st")
    values = list(primvar.Get())
    values[corner] = Gf.Vec2f(values[corner][0] + delta, values[corner][1])
    primvar.Set(values)


def bind_top_face(room, indices):
    """Bind the Wall material to the given Floor faces through a subset."""
    mesh = UsdGeom.Mesh(room.prim("world", FLOOR))
    subset = UsdGeom.Subset.CreateGeomSubset(mesh, "TopFace", UsdGeom.Tokens.face, indices,
                                             "materialBind")
    UsdShade.MaterialBindingAPI.Apply(subset.GetPrim()).Bind(
        UsdShade.Material(room.stage.GetPrimAtPath("/Map/Looks/Wall")))


def errors_of(report):
    return [(e["code"], e["path"]) for e in report["errors"]]


class Scratch(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        cls.scratch = Path(cls.temporary.name)
        cls.good = validator.validate(str(FIXTURE / "room.usda"))

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def variant(self, edit, fixture=Room):
        room = fixture(self.scratch)
        kwargs = edit(room)
        kwargs = kwargs[-1] if isinstance(kwargs, tuple) and isinstance(kwargs[-1], dict) \
            else kwargs if isinstance(kwargs, dict) else {}
        return validator.validate(room.save(), **kwargs), room

    def objects(self, report):
        return {o["id"]: o for o in report["objects"]}


class GoodFixtureTest(Scratch):
    def test_fixture_is_clean(self):
        self.assertEqual(errors_of(self.good), [])
        self.assertGreater(self.good["checks"], 500)
        self.assertEqual(len(self.good["layers"]), 4)

    def test_objects_and_roles(self):
        roles = {}
        for record in self.good["objects"]:
            roles[record["role"]] = roles.get(record["role"], 0) + 1
        self.assertEqual(roles, {"world_solid": 6, "entity_point": 1, "entity_brush": 1,
                                 "prop_static": 1, "prop_physics": 1, "light": 1})
        objects = self.objects(self.good)
        self.assertEqual(objects["prop.lab-desk"]["model"],
                         "models/props/lab_desk01/lab_desk01.mdl")
        # Role and model come from the referenced prefab, the id from the placement.
        box = objects["prop.metal-box"]
        self.assertEqual((box["role"], box["model"], box["path"]),
                         ("prop_physics", "models/props/metal_box.mdl", BOX))
        self.assertEqual(objects["entity.trigger-center"]["classname"], "trigger_multiple")

    def test_placements_in_source_space(self):
        objects = self.objects(self.good)
        start = objects["entity.player-start"]
        self.assertEqual(start["origin"], [-192.0, -128.0, 8.0])
        self.assertEqual(start["angles"], [0.0, 90.0, 0.0])
        self.assertEqual(objects["prop.metal-box"]["angles"], [0.0, 30.0, 0.0])
        self.assertEqual(objects["prop.lab-desk"]["angles"][1], 180.0)
        self.assertEqual(objects["world.floor"]["bounds"], [[-272, -208, -16], [272, 208, 0]])
        self.assertEqual(objects["light.ceiling-lamp"]["intensity"], 2000.0)

    def test_surfaces_carry_materials(self):
        floor = self.objects(self.good)["world.floor"]
        self.assertEqual([s["surface"] for s in floor["surfaces"]], list(range(6)))
        self.assertEqual({s["material"] for s in floor["surfaces"]},
                         {"metal/metal_modular_floor001"})

    def test_solids_carry_source_faces(self):
        # The compiler's input: each face's outward plane, corners and st rows.
        floor = self.objects(self.good)["world.floor"]
        self.assertEqual(len(floor["faces"]), 6)
        self.assertEqual(floor["faces"][1]["plane"], [0.0, 0.0, 1.0, 0.0])
        stage = Usd.Stage.Open(str(FIXTURE / "room.usda"))
        mesh = UsdGeom.Mesh(stage.GetPrimAtPath(FLOOR))
        st = list(UsdGeom.PrimvarsAPI(mesh).GetPrimvar("st").Get())
        corner = 0
        for face in floor["faces"]:
            n, d = face["plane"][:3], face["plane"][3]
            self.assertAlmostEqual(sum(x * x for x in n), 1.0, places=9)
            for p in face["points"]:
                self.assertAlmostEqual(sum(a * b for a, b in zip(n, p)), d, places=6)
                s, t = (sum(g * x for g, x in zip(face["st"][k][:3], p)) + face["st"][k][3]
                        for k in ("s", "t"))
                self.assertAlmostEqual(s, st[corner][0], places=6)
                self.assertAlmostEqual(t, st[corner][1], places=6)
                corner += 1
            # Outward: the solid's centre is behind every face.
            self.assertLess(sum(a * b for a, b in zip(n, (0, 0, -8))), d)
        self.assertNotIn("st", self.objects(self.good)["entity.trigger-center"]["faces"][0])

    def test_mirrored_solid_keeps_its_faces(self):
        report, _ = self.variant(
            lambda r: add_xform_op(r, "world", FLOOR, "scale", Gf.Vec3f(-1, 1, 1)))
        planes = sorted(tuple(f["plane"]) for f in self.objects(report)["world.floor"]["faces"])
        self.assertEqual(planes, sorted(tuple(f["plane"]) for f in
                                        self.objects(self.good)["world.floor"]["faces"]))

    def test_light_records_its_inputs(self):
        lamp = self.objects(self.good)["light.ceiling-lamp"]
        self.assertEqual(lamp["inputs"], {"exposure": 0.0, "diffuse": 1.0, "normalize": False,
                                          "enableColorTemperature": False, "radius": 4.0})
        report, _ = self.variant(lambda r: r.set("entities", LAMP, "inputs:radius", 2.0,
                                                 S.Float))
        self.assertEqual(self.objects(report)["light.ceiling-lamp"]["inputs"]["radius"], 2.0)

    def test_flattened_stage_matches_composition(self):
        stage = Usd.Stage.Open(str(FIXTURE / "room.usda"))
        flat = self.scratch / "flat.usda"
        stage.Flatten().Export(str(flat))
        report = validator.validate(str(flat))
        self.assertEqual(errors_of(report), [])
        self.assertEqual(report["objects"], self.good["objects"])
        # Only the three sublayers' metrics comparisons are gone.
        self.assertEqual(report["checks"], self.good["checks"] - 6)

    def test_y_up_centimetres_convert_to_the_same_source_space(self):
        stage = Usd.Stage.Open(str(FIXTURE / "room.usda"))
        layer = stage.Flatten()
        layer.pseudoRoot.SetInfo("metersPerUnit", 0.01)
        layer.pseudoRoot.SetInfo("upAxis", "Y")
        path = self.scratch / "yup_cm.usda"
        layer.Export(str(path))
        converted = Usd.Stage.Open(str(path))
        xformable = UsdGeom.Xformable(converted.GetPrimAtPath("/Map"))
        xformable.AddRotateXOp().Set(-90.0)
        xformable.AddScaleOp().Set(Gf.Vec3f(2.54, 2.54, 2.54))
        converted.GetRootLayer().Save()
        report = validator.validate(str(path))
        self.assertEqual(errors_of(report), [])
        before, after = self.objects(self.good), self.objects(report)
        self.assertEqual(sorted(before), sorted(after))
        for ident, record in before.items():
            for key in ("origin", "angles", "bounds"):
                if key in record:
                    for a, b in zip(sum(record[key], []) if key == "bounds" else record[key],
                                    sum(after[ident][key], []) if key == "bounds"
                                    else after[ident][key]):
                        self.assertAlmostEqual(a, b, places=3, msg="%s %s" % (ident, key))

    def test_mirrored_solid_keeps_outward_faces(self):
        report, _ = self.variant(
            lambda r: add_xform_op(r, "world", FLOOR, "scale", Gf.Vec3f(-1, 1, 1)))
        self.assertEqual(errors_of(report), [])

    def test_face_subset_binds_one_surface(self):
        report, _ = self.variant(lambda r: bind_top_face(r, [1]))
        self.assertEqual(errors_of(report), [])
        surfaces = self.objects(report)["world.floor"]["surfaces"]
        self.assertEqual(surfaces[1]["material"], "metal/metalwall048b")
        self.assertEqual(surfaces[0]["material"], "metal/metal_modular_floor001")

    def test_usdchecker_clean_fixture_is_not_enough(self):
        # A generic-valid stage with no roles at all is not a playable map.
        path = self.scratch / "generic.usda"
        stage = Usd.Stage.CreateNew(str(path))
        UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
        UsdGeom.SetStageMetersPerUnit(stage, 0.0254)
        stage.SetDefaultPrim(UsdGeom.Xform.Define(stage, "/Map").GetPrim())
        UsdGeom.Cube.Define(stage, "/Map/Box")
        stage.GetRootLayer().Save()
        codes = {code for code, _ in errors_of(validator.validate(str(path)))}
        self.assertEqual(codes, {"stage.profile-missing", "role.missing",
                                 "map.player-start-missing"})


class RoleFixtureTest(Scratch):
    """The U2 fixture: every placement role, two geometric entities, I/O."""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.roles = validator.validate(str(ROLES / "roles.usda"))

    def test_roles_fixture_is_clean(self):
        self.assertEqual(errors_of(self.roles), [])
        counts = {}
        for record in self.roles["objects"]:
            counts[record["role"]] = counts.get(record["role"], 0) + 1
        self.assertEqual(counts, {"world_solid": 6, "entity_point": 1, "entity_brush": 2,
                                  "prop_static": 2, "prop_dynamic": 1, "prop_physics": 1,
                                  "light": 1})
        # The world is the room's layer, reused rather than copied.
        self.assertIn(str(FIXTURE / "world.usda"), self.roles["layers"])

    def test_role_records(self):
        objects = self.objects(self.roles)
        switch = objects["prop.lever-switch"]
        self.assertEqual((switch["role"], switch["model"], switch["collision"],
                          switch["default_animation"]),
                         ("prop_dynamic", "models/props/switch001.mdl", "vphysics", "idle"))
        self.assertNotIn("mass_scale", switch)
        self.assertEqual(objects["prop.ghost-desk"]["collision"], "none")
        lift = objects["entity.lift"]
        self.assertEqual((lift["classname"], lift["move"], lift["move_seconds"]),
                         ("func_movelinear", [0.0, 0.0, 64.0], 1.0))
        self.assertEqual({s["material"] for s in lift["surfaces"]}, {"metal/metalwall048b"})
        self.assertTrue(all("st" in face for face in lift["faces"]))
        trigger = objects["entity.drop-trigger"]
        self.assertEqual(trigger["touch_filter"], ["clients", "physics"])
        self.assertEqual(trigger["connections"], [
            {"output": "OnStartTouch", "target": "prop.lever-switch", "input": "SetAnimation",
             "parameter": "down"},
            {"output": "OnStartTouch", "target": "entity.lift", "input": "Open",
             "parameter": ""}])

    def test_roles_swapped_between_two_props(self):
        # The static desk and the dynamic switch trade roles: the switch keeps
        # a dynamic-only key, and the trigger's SetAnimation now aims at a
        # prop_static. The compiler's published-package check adds
        # id.role-changed (test_usd_map_compile).
        report, _ = self.variant(lambda r: (
            r.set("entities", DESK, "sourcemap:role", "prop_dynamic"),
            r.set("entities", SWITCH, "sourcemap:role", "prop_static")), Roles)
        self.assertEqual(sorted(errors_of(report)),
                         [("io.input-unsupported", DROP_TRIGGER),
                          ("property.misplaced", SWITCH)])

    def test_move_follows_the_prim_transform(self):
        # A displacement is in the prim's space: a quarter turn about z turns it.
        report, _ = self.variant(lambda r: (
            r.set("entities", LIFT, "sourcemap:moveDelta", Gf.Vec3f(64, 0, 0), S.Float3),
            UsdGeom.Xformable(r.prim("entities", LIFT)).AddRotateZOp().Set(90.0)), Roles)
        self.assertEqual(errors_of(report), [])
        move = self.objects(report)["entity.lift"]["move"]
        for got, want in zip(move, (0.0, 64.0, 0.0)):
            self.assertAlmostEqual(got, want, places=4)

    def test_roles_survive_save_and_reopen(self):
        # Save as one flattened binary layer, reopen and revalidate: every id
        # keeps its role, placement, keys and connections.
        stage = Usd.Stage.Open(str(ROLES / "roles.usda"))
        crate = self.scratch / "roles.usdc"
        stage.Flatten().Export(str(crate))
        report = validator.validate(str(crate), previous=self.roles)
        self.assertEqual(errors_of(report), [])
        self.assertEqual(report["objects"], self.roles["objects"])
        self.assertEqual(report["diff"], {"added": [], "removed": [], "moved": []})

    def test_v1_stage_with_no_v2_features_is_clean(self):
        report, _ = self.variant(lambda r: (
            r.remove("entities", DROP_TRIGGER, "sourcemap:connections"),
            r.remove("entities", DROP_TRIGGER, "sourcemap:touchFilter"),
            setattr(r.spec("entities", SWITCH), "active", False),
            setattr(r.spec("entities", LIFT), "active", False),
            setattr(r.layers["room"], "customLayerData",
                    {"sourcemap": {"profile": "source-authoring", "version": 1}})), Roles)
        self.assertEqual(errors_of(report), [])


class IdStabilityTest(Scratch):
    def rename_desk(self, room):
        layer = room.layers["entities"]
        Sdf.CopySpec(layer, DESK, layer, "/Map/Entities/Desk")
        props = layer.GetPrimAtPath("/Map/Props")
        del props.nameChildren["LabDesk"]

    def test_rename_keeps_ids(self):
        report, _ = self.variant(lambda r: (self.rename_desk(r),
                                            {"previous": str(FIXTURE / "room.usda")}))
        self.assertEqual(errors_of(report), [])
        self.assertEqual(report["diff"], {"added": [], "removed": [], "moved": ["prop.lab-desk"]})
        self.assertEqual(self.objects(report)["prop.lab-desk"]["path"], "/Map/Entities/Desk")

    def test_reparent_into_hint_named_group(self):
        # A grouping prim may be named like a role when it holds role prims.
        def regroup(room):
            layer = room.layers["entities"]
            Sdf.CreatePrimInLayer(layer, "/Map/Props/prop_physics_set").specifier = \
                Sdf.SpecifierDef
            layer.GetPrimAtPath("/Map/Props/prop_physics_set").typeName = "Scope"
            Sdf.CopySpec(layer, BOX, layer, "/Map/Props/prop_physics_set/MetalBox")
            del layer.GetPrimAtPath("/Map/Props").nameChildren["MetalBox"]
            return {"previous": self.good}
        report, _ = self.variant(regroup)
        self.assertEqual(errors_of(report), [])
        self.assertEqual(report["diff"]["moved"], ["prop.metal-box"])

    def test_previous_report_file(self):
        previous = self.scratch / "previous.json"
        previous.write_text(json.dumps(self.good))
        report, _ = self.variant(lambda r: (r.remove("entities", LAMP, "sourcemap:id"),
                                            r.set("entities", LAMP, "sourcemap:id",
                                                  "light.lamp-2", S.String),
                                            {"previous": str(previous)}))
        self.assertEqual(errors_of(report), [])
        self.assertEqual(report["diff"]["added"], ["light.lamp-2"])
        self.assertEqual(report["diff"]["removed"], ["light.ceiling-lamp"])

    def test_binary_round_trip_keeps_ids(self):
        stage = Usd.Stage.Open(str(FIXTURE / "room.usda"))
        crate = self.scratch / "room.usdc"
        stage.Flatten().Export(str(crate))
        report = validator.validate(str(crate), previous=self.good)
        self.assertEqual(errors_of(report), [])
        self.assertEqual(report["objects"], self.good["objects"])
        self.assertEqual(report["diff"], {"added": [], "removed": [], "moved": []})


class BadFixtureTest(Scratch):
    """One test per bad variant (added below from CASES and ROLE_CASES)."""

    def expect_only(self, edit, code, path, fixture=Room):
        report, room = self.variant(edit, fixture)
        expected_path = path if path.startswith("/") else str(room.directory / path)
        self.assertEqual(errors_of(report), [(code, expected_path)])

    def test_open_failures(self):
        missing = validator.validate(str(self.scratch / "absent.usda"))
        self.assertEqual([c for c, _ in errors_of(missing)], ["stage.open-failed"])
        broken = self.scratch / "broken.usda"
        broken.write_text("#usda 1.0\ndef Xform \"Map\" {\n")
        report = validator.validate(str(broken))
        self.assertEqual([c for c, _ in errors_of(report)], ["stage.open-failed"])


def bad_variant_test(edit, code, path, fixture):
    return lambda self: self.expect_only(edit, code, path, fixture)


for _cases, _fixture in ((CASES, Room), (ROLE_CASES, Roles)):
    for _name, _edit, _code, _path in _cases:
        _method = "test_" + re.sub(r"[^a-z0-9]+", "_", _name.lower()).strip("_")
        if hasattr(BadFixtureTest, _method):
            raise RuntimeError("duplicate bad-variant name %r" % _name)
        setattr(BadFixtureTest, _method, bad_variant_test(_edit, _code, _path, _fixture))


class ProfileTest(unittest.TestCase):
    def test_every_declared_code_has_a_bad_variant(self):
        exercised = {case[2] for case in CASES + ROLE_CASES} | {"stage.open-failed"}
        self.assertEqual(sorted(set(PROFILE["errors"]) - exercised), [])
        self.assertEqual(sorted(exercised - set(PROFILE["errors"])), [])

    def test_validator_and_profile_codes_are_declared(self):
        source = VALIDATOR.read_text()
        literal = set(re.findall(r'"([a-z]+\.[a-z]+(?:-[a-z]+)*)"', source))
        literal = {c for c in literal if c.split(".")[0] in
                   {"stage", "composition", "schema", "feature", "role", "property", "id",
                    "transform", "geometry", "surface", "uv", "material", "prop", "entity",
                    "light", "map", "io"}}
        referenced = set()
        for role in PROFILE["roles"].values():
            referenced.add(role.get("prim_type_code", "role.prim-type"))
            rules = list(role.get("properties", {}).values())
            for cls in role.get("classes", {}).values():
                rules += list(cls.get("properties", {}).values())
            for rule in rules:
                referenced.update(rule[k] for k in ("missing_code", "unsupported_code")
                                  if k in rule)
        self.assertEqual(sorted((literal | referenced) - set(PROFILE["errors"])), [])
        with self.assertRaises(KeyError):
            validator.Validation(PROFILE).error("role.made-up", "/", "")

    def test_roles_are_complete(self):
        self.assertEqual(sorted(PROFILE["roles"]),
                         ["entity_brush", "entity_point", "light", "prop_dynamic",
                          "prop_physics", "prop_static", "world_solid"])
        for name, role in PROFILE["roles"].items():
            if role["support"] == "supported":
                self.assertIn("sourcemap:id", role["properties"], name)
                self.assertTrue(role["prim_types"], name)
            else:
                self.assertEqual(role["support"], "reserved", name)
            self.assertIn(role.get("since", 1), PROFILE["accepted_versions"], name)
            if name != "light":
                self.assertIn(name, PROFILE["name_hints"])

    def test_io_declarations_are_consistent(self):
        brush = PROFILE["roles"]["entity_brush"]
        classes = brush["properties"]["sourcemap:classname"]["values"]
        for cls in PROFILE["io"]["outputs"]:
            self.assertIn(cls, classes)
            self.assertIn("sourcemap:connections", brush["classes"][cls]["properties"])
        for kind, inputs in PROFILE["io"]["inputs"].items():
            self.assertTrue(kind in PROFILE["roles"] or kind in classes, kind)
            self.assertTrue(set(inputs.values()) <= {"none", "sequence"}, kind)

    def test_source_angles(self):
        self.assertEqual(validator.source_angles((0, 1, 0), (-1, 0, 0), (0, 0, 1)),
                         [0.0, 90.0, 0.0])

    def test_source_angles_invert_angle_vectors(self):
        # Oracle: Source's AngleVectors (mathlib/mathlib_base.cpp), left = -right.
        for angles in ((30, 0, 0), (0, 0, 45), (20, -35, 50), (-60, 170, -120)):
            sp, sy, sr = (math.sin(math.radians(a)) for a in angles)
            cp, cy, cr = (math.cos(math.radians(a)) for a in angles)
            forward = (cp * cy, cp * sy, -sp)
            right = (-sr * sp * cy + cr * sy, -sr * sp * sy - cr * cy, -sr * cp)
            up = (cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp)
            left = tuple(-x for x in right)
            for got, want in zip(validator.source_angles(forward, left, up), angles):
                self.assertAlmostEqual(got, want, places=5, msg=str(angles))


class CommandLineTest(unittest.TestCase):
    def run_validator(self, *args):
        env = dict(os.environ, PYTHONDONTWRITEBYTECODE="1")
        return subprocess.run([sys.executable, str(VALIDATOR), "validate", *args], cwd=ROOT,
                              capture_output=True, text=True, env=env, timeout=120)

    def test_good_stage_reports_checks_v1(self):
        with tempfile.TemporaryDirectory() as scratch:
            out = Path(scratch) / "report.json"
            run = self.run_validator(str(FIXTURE / "room.usda"), "--json", str(out))
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
            record = [ln for ln in run.stdout.splitlines() if ln.startswith("CONFORMANCE ")]
            self.assertEqual(len(record), 1)
            checks, failures = map(int, record[0].split()[1:])
            self.assertGreater(checks, 0)
            self.assertEqual(failures, 0)
            report = json.loads(out.read_text())
            self.assertEqual(report["schema"], "source-authoring-validation/v1")
            self.assertEqual(report["errors"], [])

    def test_bad_stage_fails_with_machine_readable_errors(self):
        with tempfile.TemporaryDirectory() as scratch:
            room = Room(scratch)
            room.set("world", "/Map/World/Ceiling", "sourcemap:id", "world.floor", S.String)
            out = Path(scratch) / "report.json"
            run = self.run_validator(room.save(), "--json", str(out))
            self.assertEqual(run.returncode, 1)
            self.assertIn("FAIL id.duplicate /Map/World/Ceiling:", run.stdout)
            self.assertTrue(run.stdout.rstrip().endswith(" 1"), run.stdout)
            errors = json.loads(out.read_text())["errors"]
            self.assertEqual([(e["code"], e["path"]) for e in errors],
                             [("id.duplicate", "/Map/World/Ceiling")])


if __name__ == "__main__":
    unittest.main()
