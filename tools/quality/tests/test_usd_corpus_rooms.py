"""Gap-corpus oracles for `usd_scene.extract` on small hand-written USD rooms.

Fixtures: quality/fixtures/usd-maps/corpus/ (see its fixture.json). Every
expectation here is computed from the fixture's declared spec, which is
restated below as constants. It is computed by hand (4x4 column-vector matrices
in the USD xformOpOrder convention, the UsdPreviewSurface UsdTransform2d
formula, and material classification from the authored outputs), never with
usd_scene's own helpers. `pxr` is used only to read the fixtures and the
normalized stage back. `FixtureSpecTest` proves that the fixtures author
exactly this spec.

Needs the pinned OpenUSD Python. The gate is the corpus runner, which counts a
skip as a failure:

    python3 tools/quality/corpus/usd_runner.py tools.quality.tests.test_usd_corpus_rooms

Plain `unittest discover` under a Python without `pxr` skips this module, as it
does test_usd_scene.

SOURCE_USD_CORPUS_OVERRIDES=name=path[;name=path] replaces a fixture (names:
box_cm, box_in, st, materials); the runner's seeded negative controls use it.
"""

import hashlib
import json
import math
import os
import sys
import tempfile
import unittest
from pathlib import Path

try:
    from pxr import Sdf, Usd, UsdGeom, UsdShade
except ImportError as error:  # no OpenUSD here; the corpus runner fails instead
    raise unittest.SkipTest("test_usd_corpus_rooms needs OpenUSD's pxr: %s" % error)

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/quality"))
import usd_scene  # noqa: E402

FIXTURES = ROOT / "quality/fixtures/usd-maps/corpus"
DEFAULT_FIXTURES = {"box_cm": "box_room_cm_yup.usda", "box_in": "box_room_in_zup.usda",
                    "st": "st_transforms.usda", "materials": "materials.usda"}
OVERRIDE_ENV = "SOURCE_USD_CORPUS_OVERRIDES"
TOLERANCE = 1e-5  # the normalized stage stores float32


def fixture(name):
    """The fixture path for `name`, honouring SOURCE_USD_CORPUS_OVERRIDES."""
    overrides = {}
    for entry in filter(None, os.environ.get(OVERRIDE_ENV, "").split(";")):
        key, _, value = entry.partition("=")
        overrides[key.strip()] = value.strip()
    unknown = set(overrides) - set(DEFAULT_FIXTURES)
    if unknown:
        raise ValueError("unknown fixture override(s): " + ", ".join(sorted(unknown)))
    path = Path(overrides.get(name) or FIXTURES / DEFAULT_FIXTURES[name])
    return path if path.is_absolute() else ROOT / path


# ------------------------------------------------------------ hand oracles

def identity():
    return [[1.0 if r == c else 0.0 for c in range(4)] for r in range(4)]


def matmul(a, b):
    return [[sum(a[r][k] * b[k][c] for k in range(4)) for c in range(4)] for r in range(4)]


def op_matrix(op, value):
    """One USD xform op as a column-vector 4x4 matrix."""
    m = identity()
    if op == "translate":
        for axis in range(3):
            m[axis][3] = float(value[axis])
    elif op == "scale":
        for axis in range(3):
            m[axis][axis] = float(value[axis])
    elif op in ("rotateX", "rotateY", "rotateZ"):
        c, s = math.cos(math.radians(value)), math.sin(math.radians(value))
        i, j = {"rotateX": (1, 2), "rotateY": (2, 0), "rotateZ": (0, 1)}[op]
        m[i][i], m[i][j], m[j][i], m[j][j] = c, -s, s, c
    else:
        raise ValueError(op)
    return m


def compose(ops):
    """xformOpOrder semantics: the first op is outermost, so p' = Op0 Op1 ... p."""
    m = identity()
    for op, value in ops:
        m = matmul(m, op_matrix(op, value))
    return m


def to_pipeline(point, meters_per_unit, up):
    """Stage units -> metres, then Y-up -> Z-up by +90 degrees about X
    ((x, y, z) -> (x, -z, y)); Z-up stays."""
    x, y, z = (float(v) * meters_per_unit for v in point)
    return (x, -z, y) if up == "Y" else (x, y, z)


def box_corners(low, high):
    return [(x, y, z) for x in (low[0], high[0]) for y in (low[1], high[1])
            for z in (low[2], high[2])]


def world_box(spec, mesh):
    """Axis-aligned pipeline bounds of a declared box under its declared ops."""
    entry = spec["meshes"][mesh]
    m = compose(entry["ops"])
    points = []
    for p in box_corners(*entry["box"]):
        q = [sum(m[r][k] * v for k, v in enumerate((p[0], p[1], p[2], 1.0))) for r in range(3)]
        points.append(to_pipeline(q, spec["meters_per_unit"], spec["up"]))
    return ([min(p[a] for p in points) for a in range(3)],
            [max(p[a] for p in points) for a in range(3)])


def transform2d(uv, scale=(1.0, 1.0), rotation=0.0, translation=(0.0, 0.0)):
    """UsdPreviewSurface UsdTransform2d: rotate(in * scale) + translation."""
    u, v = uv[0] * scale[0], uv[1] * scale[1]
    c, s = math.cos(math.radians(rotation)), math.sin(math.radians(rotation))
    return (c * u - s * v + translation[0], s * u + c * v + translation[1])


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b))


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def stage_mesh(stage, name):
    """(points, triangle corner indices, prim) of a normalized-stage mesh."""
    prim = stage.GetPrimAtPath("/root/%s/%s" % (name, name))
    if not prim:
        raise AssertionError("normalized stage has no mesh /root/%s/%s" % (name, name))
    mesh = UsdGeom.Mesh(prim)
    points = [tuple(map(float, p)) for p in mesh.GetPointsAttr().Get()]
    indices = list(mesh.GetFaceVertexIndicesAttr().Get())
    counts = list(mesh.GetFaceVertexCountsAttr().Get())
    if set(counts) != {3}:
        raise AssertionError("%s is not triangulated: %r" % (name, sorted(set(counts))))
    return points, indices, prim


def source_shader(shade_input):
    """The shader whose output feeds an input (pxr connection query)."""
    sources = shade_input.GetConnectedSources()[0] if shade_input else []
    if not sources:
        raise AssertionError("%s is not connected" % (shade_input.GetFullName()
                                                      if shade_input else "input"))
    return UsdShade.Shader(sources[0].source.GetPrim())


def authored_value(shade_input):
    """An input's authored value, following interface connections; None if unset."""
    if not shade_input:
        return None
    attributes = shade_input.GetValueProducingAttributes()
    return attributes[0].Get() if attributes else None


def extract(path):
    temporary = tempfile.TemporaryDirectory()
    out = Path(temporary.name)
    model = usd_scene.extract(path, out / "scene.json", out / "stage.usdc")
    return temporary, model, Usd.Stage.Open(str(out / "stage.usdc"))


# ------------------------------------------------------------- the spec

BOX_OPS_PROPS = [("translate", (0, 0, -100))]
BOX_CM = {
    "fixture": "box_cm", "meters_per_unit": 0.01, "up": "Y",
    "meshes": {
        "Floor": {"path": "/Room/Floor", "box": ((-300, -10, -400), (300, 0, 400)), "ops": [],
                  "literal": ((-3.0, -4.0, -0.1), (3.0, 4.0, 0.0))},
        "Crate": {"path": "/Room/Props/Crate", "box": ((-25, -25, -25), (25, 25, 25)),
                  "local_ops": [("translate", (100, 25, 50)), ("rotateY", 90),
                                ("scale", (2, 1, 0.5))],
                  "literal": ((0.875, 0.0, 0.0), (1.125, 1.0, 0.5))},
        "Mirror": {"path": "/Room/Props/Mirror", "box": ((0, 0, 0), (40, 60, 20)),
                   "local_ops": [("translate", (-150, 30, 0)), ("scale", (-1, 1, 1))],
                   "literal": ((-1.9, 0.8, 0.3), (-1.5, 1.0, 0.9))},
    },
    "parents": {"Crate": BOX_OPS_PROPS, "Mirror": BOX_OPS_PROPS},
    "skipped": {"/Room/Props/Marker": "Cube"},
}
BOX_IN = {
    "fixture": "box_in", "meters_per_unit": 0.0254, "up": "Z",
    "meshes": {
        "Slab": {"path": "/Room/Slab", "box": ((0, 0, -2), (120, 96, 0)), "ops": [],
                 "literal": ((0.0, 0.0, -0.0508), (3.048, 2.4384, 0.0))},
        # Half extents after rotateZ 30: 24 cos30 + 6 sin30 = 23.78461,
        # 24 sin30 + 6 cos30 = 17.19615 (inches), about the centre (60, 48, 36).
        "Board": {"path": "/Room/Shelf/Board", "box": ((-24, -6, -1), (24, 6, 1)),
                  "local_ops": [("translate", (0, 0, 36))],
                  "literal": (((60 - 23.784610) * 0.0254, (48 - 17.196152) * 0.0254, 0.889),
                              ((60 + 23.784610) * 0.0254, (48 + 17.196152) * 0.0254, 0.9398))},
        "LeftHanded": {"path": "/Room/LeftHanded", "box": ((0, 0, 0), (10, 10, 10)),
                       "local_ops": [("translate", (100, 10, 0))],
                       "literal": ((2.54, 0.254, 0.0), (2.794, 0.508, 0.254))},
    },
    "parents": {"Board": [("translate", (60, 48, 0)), ("rotateZ", 30)]},
    "skipped": {},
}
for _spec in (BOX_CM, BOX_IN):
    for _name, _entry in _spec["meshes"].items():
        _entry.setdefault("ops", _spec["parents"].get(_name, []) + _entry.get("local_ops", []))

ST_AUTHORED = [(0.25, 0.5), (1.5, 0.5), (1.5, 2.0), (0.25, 2.0)]
QUAD_CORNERS = [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)]
IDENTITY_2D = {"scale": [1.0, 1.0], "rotation": 0.0, "translation": [0.0, 0.0]}
ST = {  # shape -> (material, x offset, per-corner input uv, transform, primvar)
    "QuadIdentity": ("identity", 0, ST_AUTHORED, IDENTITY_2D, "st"),
    "QuadScale": ("scale", 2, ST_AUTHORED,
                  {"scale": [2.0, 0.5], "rotation": 0.0, "translation": [0.0, 0.0]}, "st"),
    "QuadRotate": ("rotate", 4, ST_AUTHORED,
                   {"scale": [1.0, 1.0], "rotation": 90.0, "translation": [0.0, 0.0]}, "st"),
    "QuadFull": ("full", 6, ST_AUTHORED,
                 {"scale": [2.0, 0.5], "rotation": 30.0, "translation": [0.25, -1.0]}, "st"),
    # uvB is faceVarying with indices [2, 3, 0, 1] into (0,0) (1,0) (1,1) (0,1).
    "QuadAlt": ("altprimvar", 8, [(1.0, 1.0), (0.0, 1.0), (0.0, 0.0), (1.0, 0.0)],
                {"scale": [1.0, 1.0], "rotation": 0.0, "translation": [0.5, 0.5]}, "uvB"),
    "QuadMixed": ("mixed", 10, ST_AUTHORED,
                  {"scale": [2.0, 2.0], "rotation": 0.0, "translation": [0.0, 0.0]}, "st"),
}
# Worked by hand from the spec formula (not from transform2d):
#   Scale  corner 1 (1.5, 0.5) * (2, 0.5)                     -> (3, 0.25)
#   Rotate corner 1 (1.5, 0.5) rotated 90 CCW                  -> (-0.5, 1.5)
#   Full   corner 0 (0.25, 0.5) * (2, .5) = (.5, .25); rot 30:
#          (.5 c30 - .25 s30, .5 s30 + .25 c30) = (.3080127, .4665064); + (.25, -1)
#   Full   corner 2 (1.5, 2) * (2, .5) = (3, 1); rot 30:
#          (3 c30 - s30, 3 s30 + c30) = (2.0980762, 2.3660254); + (.25, -1)
#   Alt    corner 0 uvB (1, 1) + (.5, .5)                      -> (1.5, 1.5)
#   Mixed  corner 2 (1.5, 2) * 2                               -> (3, 4)
ST_LITERALS = {("QuadScale", 1): (3.0, 0.25), ("QuadRotate", 1): (-0.5, 1.5),
               ("QuadFull", 0): (0.5580127, -0.5334936), ("QuadFull", 2): (2.3480762, 1.3660254),
               ("QuadAlt", 0): (1.5, 1.5), ("QuadMixed", 2): (3.0, 4.0)}

NEUTRAL = (0.18, 0.18, 0.18)  # UsdPreviewSurface diffuseColor default
MATERIALS = {  # shape -> (material, authored surface outputs, expected summary facts)
    "QuadMtlxOnly": ("mtlxonly", {"mtlx": "ND_standard_surface_surfaceshader"},
                     {"approximation": "unsupported mtlx surface "
                                       "ND_standard_surface_surfaceshader; neutral grey fallback",
                      "base_color": NEUTRAL}),
    "QuadToon": ("toon", {"": "MyStudio_Toon"},
                 {"approximation_contains": ["MyStudio_Toon", "neutral grey fallback"],
                  "base_color": NEUTRAL}),
    "QuadBare": ("bare", {},
                 {"approximation": "material has no UsdPreviewSurface; neutral grey fallback",
                  "base_color": NEUTRAL}),
    "QuadTextured": ("textured", {"": "UsdPreviewSurface"},
                     {"approximation": None, "base_color": None, "metallic": 1.0,
                      "roughness": 0.25}),
    "QuadPreviewAndMtlx": ("previewandmtlx", {"": "UsdPreviewSurface",
                                              "mtlx": "ND_standard_surface_surfaceshader"},
                           {"approximation": None, "base_color": (0.25, 0.5, 0.75),
                            "roughness": 0.75}),
    "QuadMdl": ("mdl", {"mdl": ""},
                {"approximation_contains": ["unsupported mdl surface", "neutral grey fallback"],
                 "base_color": NEUTRAL}),
}


# ------------------------------------------------------------------ tests

class BoxRoomChecks:
    SPEC = None

    @classmethod
    def setUpClass(cls):
        cls.path = fixture(cls.SPEC["fixture"])
        cls.temporary, cls.model, cls.stage = extract(cls.path)
        cls.shapes = {shape["name"]: shape for shape in cls.model["shapes"]}

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def assertBounds(self, actual, expected, label):
        for side, (a, e) in enumerate(zip(actual, expected)):
            for axis in range(3):
                self.assertAlmostEqual(a[axis], e[axis], delta=TOLERANCE,
                                       msg="%s %s[%d]: %r vs expected %r" % (
                                           label, ("min", "max")[side], axis, a, e))

    def test_bounds_match_authored_points_times_transforms(self):
        for name in self.SPEC["meshes"]:
            self.assertIn(name, self.shapes)
            self.assertBounds(self.shapes[name]["bounds"], world_box(self.SPEC, name),
                              name + " (composed ops)")
            self.assertBounds(self.shapes[name]["bounds"], self.SPEC["meshes"][name]["literal"],
                              name + " (hand literal)")

    def test_room_bounds_are_the_union_of_the_meshes(self):
        boxes = [world_box(self.SPEC, name) for name in self.SPEC["meshes"]]
        union = ([min(b[0][a] for b in boxes) for a in range(3)],
                 [max(b[1][a] for b in boxes) for a in range(3)])
        self.assertBounds((self.model["bounds_m"]["min"], self.model["bounds_m"]["max"]), union,
                          "room")
        self.assertEqual(sorted(self.shapes), sorted(self.SPEC["meshes"]))

    def test_non_mesh_gprims_are_recorded_as_skipped(self):
        for path, kind in self.SPEC["skipped"].items():
            self.assertIn("%s %s is not a mesh and was skipped" % (kind, path),
                          self.model["approximations"])

    def test_source_units_and_axis_are_recorded(self):
        self.assertEqual(self.model["up_axis"], self.SPEC["up"])
        self.assertEqual(self.model["meters_per_unit"], self.SPEC["meters_per_unit"])

    def test_stage_readback_is_z_up_metres(self):
        self.assertEqual(UsdGeom.GetStageUpAxis(self.stage), UsdGeom.Tokens.z)
        self.assertEqual(UsdGeom.GetStageMetersPerUnit(self.stage), 1.0)
        for name in self.SPEC["meshes"]:
            points, _, _ = stage_mesh(self.stage, name)
            actual = ([min(p[a] for p in points) for a in range(3)],
                      [max(p[a] for p in points) for a in range(3)])
            self.assertBounds(actual, world_box(self.SPEC, name), name + " (stage)")

    def test_stage_triangles_face_outward(self):
        """Every triangle's winding normal points away from its box centre,
        including the mirrored (negative-scale) and leftHanded boxes."""
        for name in self.SPEC["meshes"]:
            points, indices, _ = stage_mesh(self.stage, name)
            low, high = world_box(self.SPEC, name)
            centre = tuple((a + b) / 2 for a, b in zip(low, high))
            self.assertEqual(len(indices), 36, name)  # 6 quads -> 12 triangles
            for t in range(0, len(indices), 3):
                a, b, c = (points[i] for i in indices[t:t + 3])
                normal = cross(sub(b, a), sub(c, a))
                middle = tuple((x + y + z) / 3 for x, y, z in zip(a, b, c))
                self.assertGreater(dot(normal, sub(middle, centre)), 0,
                                   "%s triangle %d faces inward" % (name, t // 3))


    def test_stage_normals_agree_with_winding(self):
        """Each corner normal lies on the outer side of its triangle's winding
        (the box corners' smoothed normals are diagonal, never tangent). The
        leftHanded box is in test_usd_corpus_defects (KNOWN_NORMAL_DEFECTS)."""
        for name in self.SPEC["meshes"]:
            if name in KNOWN_NORMAL_DEFECTS:
                continue
            points, indices, prim = stage_mesh(self.stage, name)
            mesh = UsdGeom.Mesh(prim)
            self.assertEqual(mesh.GetNormalsInterpolation(), UsdGeom.Tokens.faceVarying, name)
            normals = [tuple(map(float, n)) for n in mesh.GetNormalsAttr().Get()]
            self.assertEqual(len(normals), len(indices), name)
            for t in range(0, len(indices), 3):
                a, b, c = (points[i] for i in indices[t:t + 3])
                winding = cross(sub(b, a), sub(c, a))
                for corner in range(3):
                    self.assertGreater(dot(winding, normals[t + corner]), 0,
                                       "%s triangle %d corner %d normal opposes its winding" %
                                       (name, t // 3, corner))


# Meshes whose stage normals are known to oppose their winding; each has a
# failing oracle in test_usd_corpus_defects.py and returns here once fixed.
KNOWN_NORMAL_DEFECTS = {"LeftHanded"}


class BoxRoomCmTest(BoxRoomChecks, unittest.TestCase):
    """Y-up centimetres: explicit boxes under translate/rotateY/scale and a
    mirroring negative scale; a Cube is recorded and left out."""
    SPEC = BOX_CM

    def test_cube_only_stage_fails_loudly(self):
        stage = Usd.Stage.Open(str(self.path))
        for name, entry in self.SPEC["meshes"].items():
            stage.GetPrimAtPath(entry["path"]).SetActive(False)  # session edit only
        with self.assertRaisesRegex(ValueError, "no visible meshes"):
            usd_scene.Extractor(stage, "memory").run()


class BoxRoomInchTest(BoxRoomChecks, unittest.TestCase):
    """Z-up inches: a rotateZ 30 parent and a leftHanded box."""
    SPEC = BOX_IN


class StTransformTest(unittest.TestCase):
    """UsdTransform2d on the primvar reader chain reaches the stage UVs."""

    @classmethod
    def setUpClass(cls):
        cls.temporary, cls.model, cls.stage = extract(fixture("st"))
        cls.shapes = {shape["name"]: shape for shape in cls.model["shapes"]}

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def corner_uvs(self, name):
        """{authored corner index: [stage uv, ...]} for one stage quad."""
        _, offset, _, _, _ = ST[name]
        points, indices, prim = stage_mesh(self.stage, name)
        st = UsdGeom.PrimvarsAPI(prim).GetPrimvar("st")
        self.assertEqual(st.GetInterpolation(), UsdGeom.Tokens.faceVarying, name)
        values = st.ComputeFlattened()
        self.assertEqual(len(values), len(indices), name)
        corners = {}
        for corner, index in enumerate(indices):
            x, y, z = points[index]
            key = min(range(4), key=lambda k: abs(x - offset - QUAD_CORNERS[k][0]) +
                      abs(y - QUAD_CORNERS[k][1]))
            self.assertLess(abs(x - offset - QUAD_CORNERS[key][0]) +
                            abs(y - QUAD_CORNERS[key][1]) + abs(z), TOLERANCE, name)
            corners.setdefault(key, []).append(tuple(map(float, values[corner])))
        self.assertEqual(sorted(corners), [0, 1, 2, 3], name)
        return corners

    def assertUv(self, actual, expected, label):
        self.assertTrue(abs(actual[0] - expected[0]) <= TOLERANCE and
                        abs(actual[1] - expected[1]) <= TOLERANCE,
                        "%s: stage uv %r, expected %r" % (label, actual, expected))

    def test_every_stage_corner_follows_the_transform_formula(self):
        for name, (_, _, inputs, transform, _) in ST.items():
            for corner, uvs in self.corner_uvs(name).items():
                expected = transform2d(inputs[corner], transform["scale"], transform["rotation"],
                                       transform["translation"])
                for uv in uvs:
                    self.assertUv(uv, expected, "%s corner %d" % (name, corner))

    def test_hand_computed_corners(self):
        for (name, corner), expected in ST_LITERALS.items():
            for uv in self.corner_uvs(name)[corner]:
                self.assertUv(uv, expected, "%s corner %d" % (name, corner))

    def test_model_records_each_authored_transform(self):
        for name, (material, _, _, transform, _) in ST.items():
            self.assertEqual(self.shapes[name]["material"], material)
            recorded = self.model["materials"][material]["st_transform"]
            self.assertEqual(recorded["scale"], transform["scale"], name)
            self.assertAlmostEqual(recorded["rotation"], transform["rotation"], places=5,
                                   msg=name)
            self.assertEqual(recorded["translation"], transform["translation"], name)

    def test_named_primvar_is_read(self):
        for name, (_, _, _, _, primvar) in ST.items():
            self.assertEqual(self.shapes[name]["st_primvar"], primvar, name)

    def test_differing_texture_transforms_are_recorded(self):
        self.assertIn("textures use different st transforms; the base colour's is applied",
                      self.model["materials"]["mixed"]["approximation"] or "")
        for name, (material, _, _, _, _) in ST.items():
            if material != "mixed":
                self.assertIsNone(self.model["materials"][material]["approximation"], name)


class MaterialTest(unittest.TestCase):
    """Non-preview, missing and textured surfaces are classified as authored."""

    @classmethod
    def setUpClass(cls):
        cls.path = fixture("materials")
        cls.temporary, cls.model, cls.stage = extract(cls.path)
        cls.shapes = {shape["name"]: shape for shape in cls.model["shapes"]}

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def test_each_quad_binds_its_material(self):
        for name, (material, _, _) in MATERIALS.items():
            self.assertEqual(self.shapes[name]["material"], material, name)

    def test_summaries_match_the_authored_surface(self):
        for name, (material, _, facts) in MATERIALS.items():
            summary = self.model["materials"][material]
            if "approximation" in facts:
                self.assertEqual(summary["approximation"], facts["approximation"], material)
            for text in facts.get("approximation_contains", []):
                self.assertIn(text, summary["approximation"] or "", material)
            expected = facts["base_color"]
            actual = summary["base_color"]
            if expected is None:
                self.assertIsNone(actual, material)
            else:
                self.assertEqual(len(actual), 3, material)
                for a, e in zip(actual, expected):
                    self.assertAlmostEqual(a, e, places=6, msg=material)
            for key in ("metallic", "roughness"):
                if key in facts:
                    self.assertAlmostEqual(summary[key], facts[key], places=6, msg=material)

    def test_only_a_universal_preview_surface_is_read(self):
        """Classification from the authored outputs alone: a material is
        read (no fallback) exactly when its universal surface output is a
        UsdPreviewSurface; every other material records a neutral fallback."""
        for name, (material, outputs, _) in MATERIALS.items():
            summary = self.model["materials"][material]
            preview = outputs.get("") == "UsdPreviewSurface"
            fallback = "neutral grey fallback" in (summary["approximation"] or "")
            self.assertEqual(fallback, not preview, material)
            if not preview:
                self.assertEqual(summary["textures"], {}, material)

    def test_textured_preview_surface(self):
        texture = str((FIXTURES / "textures/checker.png").resolve())
        summary = self.model["materials"]["textured"]
        self.assertEqual(summary["base_texture"], texture)
        self.assertEqual(sorted(summary["textures"]), ["base"])
        base = summary["textures"]["base"]
        self.assertEqual(base["file"], texture)
        self.assertEqual(base["channel"], "rgb")
        self.assertEqual(base["colorspace"], "sRGB")
        self.assertEqual(base["wrap"], ["repeat", "mirror"])
        self.assertEqual(base["scale"], [0.5, 0.5, 0.5, 1.0])
        self.assertEqual(base["bias"], [0.125, 0.125, 0.125, 0.0])
        self.assertEqual(base["fallback"], [1.0, 0.0, 1.0, 1.0])
        self.assertEqual(base["primvar"], "st")
        self.assertEqual(base["transform"], IDENTITY_2D)
        self.assertIn(texture, [entry["filename"] for entry in self.model["textures"].values()])
        # The scene digest covers the texture and the layer it came from.
        self.assertIn(texture, self.model["source_files"])
        self.assertIn(str(self.path.resolve()), self.model["source_files"])


class FixtureSpecTest(unittest.TestCase):
    """The fixtures author exactly the spec above (read with pxr), so an
    expectation failure elsewhere is the extractor's, not fixture drift."""

    def test_box_rooms_author_the_spec(self):
        for spec in (BOX_CM, BOX_IN):
            stage = Usd.Stage.Open(str(fixture(spec["fixture"])))
            self.assertEqual(UsdGeom.GetStageMetersPerUnit(stage), spec["meters_per_unit"])
            self.assertEqual(UsdGeom.GetStageUpAxis(stage), spec["up"])
            for name, entry in spec["meshes"].items():
                prim = stage.GetPrimAtPath(entry["path"])
                self.assertTrue(prim and prim.IsA(UsdGeom.Mesh), entry["path"])
                points = [tuple(map(float, p)) for p in UsdGeom.Mesh(prim).GetPointsAttr().Get()]
                self.assertEqual(sorted(points), sorted(box_corners(*entry["box"])), name)
                ops = []
                current = prim
                while current and not current.IsPseudoRoot():
                    local = []
                    for op in UsdGeom.Xformable(current).GetOrderedXformOps():
                        value = op.Get()
                        local.append((op.GetOpName().split(":")[1],
                                      tuple(map(float, value)) if hasattr(value, "__len__")
                                      else float(value)))
                    ops = local + ops
                    current = current.GetParent()
                expected = [(op, tuple(map(float, value)) if isinstance(value, tuple)
                             else float(value)) for op, value in entry["ops"]]
                self.assertEqual(ops, expected, name)

    def test_sidecar_records_every_fixture_file(self):
        """fixture.json lists every file with its digest, and each layer's
        doc metadata names the same corpus version."""
        record = json.loads((FIXTURES / "fixture.json").read_text())
        on_disk = sorted(str(path.relative_to(FIXTURES)) for path in FIXTURES.rglob("*")
                         if path.is_file() and path.name != "fixture.json")
        self.assertEqual(sorted(record["files"]), on_disk)
        for name, entry in record["files"].items():
            digest = hashlib.sha256((FIXTURES / name).read_bytes()).hexdigest()
            self.assertEqual(digest, entry["sha256"], name)
            if name.endswith(".usda"):
                layer = Sdf.Layer.FindOrOpen(str(FIXTURES / name))
                self.assertTrue(layer.documentation.startswith(
                    "corpus-usd-rooms v%d" % record["version"]), name)

    def test_st_fixture_authors_the_spec(self):
        stage = Usd.Stage.Open(str(fixture("st")))
        for name, (material, offset, _, transform, primvar) in ST.items():
            prim = stage.GetPrimAtPath("/Room/" + name)
            points = [tuple(map(float, p)) for p in UsdGeom.Mesh(prim).GetPointsAttr().Get()]
            self.assertEqual(points, [(offset + x, y, 0.0) for x, y in QUAD_CORNERS], name)
            st = [tuple(map(float, v)) for v in
                  UsdGeom.PrimvarsAPI(prim).GetPrimvar("st").ComputeFlattened()]
            self.assertEqual(st, ST_AUTHORED, name)
            if primvar != "st":
                values = UsdGeom.PrimvarsAPI(prim).GetPrimvar(primvar).ComputeFlattened()
                self.assertEqual([tuple(map(float, v)) for v in values], ST[name][2], name)
            bound = UsdShade.MaterialBindingAPI(prim).ComputeBoundMaterial()[0]
            self.assertEqual(bound.GetPrim().GetName().lower(), material, name)
            # The base texture's st chain: an optional UsdTransform2d whose
            # inputs (through interface connections) are the spec's.
            surface = bound.ComputeSurfaceSource()[0]
            texture = source_shader(surface.GetInput("diffuseColor"))
            chain = source_shader(texture.GetInput("st"))
            authored = dict(IDENTITY_2D)
            if chain.GetShaderId() == "UsdTransform2d":
                for key in ("scale", "rotation", "translation"):
                    value = authored_value(chain.GetInput(key))
                    if value is not None:
                        authored[key] = list(map(float, value)) if hasattr(value, "__len__") \
                            else float(value)
                chain = source_shader(chain.GetInput("in"))
            self.assertEqual(chain.GetShaderId(), "UsdPrimvarReader_float2", name)
            self.assertEqual(str(authored_value(chain.GetInput("varname"))), primvar, name)
            self.assertEqual(authored, transform, name)

    def test_materials_fixture_authors_the_spec(self):
        stage = Usd.Stage.Open(str(fixture("materials")))
        for name, (material, outputs, _) in MATERIALS.items():
            prim = stage.GetPrimAtPath("/Room/" + name)
            bound = UsdShade.MaterialBindingAPI(prim).ComputeBoundMaterial()[0]
            self.assertEqual(bound.GetPrim().GetName().lower(), material, name)
            authored = {}
            for output in bound.GetSurfaceOutputs():
                context = output.GetBaseName().rpartition(":")[0]
                sources = output.GetConnectedSources()[0]
                if sources:
                    shader = UsdShade.Shader(sources[0].source.GetPrim())
                    authored[context] = str(shader.GetShaderId() or "")
            self.assertEqual(authored, outputs, name)


def load_tests(loader, standard_tests, pattern):
    """Extraction oracles first, the fixture-spec readback last: a seeded
    fixture then reports the extraction check that caught it first."""
    suite = unittest.TestSuite()
    for case in (BoxRoomCmTest, BoxRoomInchTest, StTransformTest, MaterialTest, FixtureSpecTest):
        suite.addTests(loader.loadTestsFromTestCase(case))
    return suite


if __name__ == "__main__":
    unittest.main()
