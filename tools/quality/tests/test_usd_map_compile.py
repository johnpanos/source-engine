"""Oracles for the native USD map compiler (usd_map_compile.py, RFC 0009 U1-U2).

    SOURCE_USD_MAP_TOOLS=<installed tools with vbsp -authored> \\
    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 -m unittest tools.quality.tests.test_usd_map_compile -v

Needs OpenUSD's `pxr`, installed compile tools (vbsp built with the
-authored input, vvis, vrad, bsp2tool) named by $SOURCE_USD_MAP_TOOLS, and
Portal content ($SOURCE_USD_MAP_RUNTIME, default run/runtime). Every variant
is generated from a private copy of the U0 room or the U2 role fixture; every
build publishes into a private store, which a failed build must leave
byte-for-byte unchanged. Seeded mutants replace one compiler step and must be
caught.
"""

import hashlib
import json
import math
import os
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

try:
    from pxr import Gf, Sdf, UsdGeom, UsdShade
except ImportError as error:  # system Python has no OpenUSD; see the docstring
    raise unittest.SkipTest("test_usd_map_compile needs OpenUSD's pxr: %s" % error)

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/quality"))
sys.path.insert(0, str(ROOT / "tools/quality/tests"))
import playable_maps  # noqa: E402
import usd_authoring_validate as validator  # noqa: E402
import usd_map_check  # noqa: E402
import usd_map_compile as compiler  # noqa: E402
from test_usd_authoring import (BOX_FACES, DROP_TRIGGER, FIXTURE, GHOST, LIFT,  # noqa: E402
                                ROLES, SWITCH, Roles, Room, box_points, new_prim, st_for)

TOOLS = os.environ.get("SOURCE_USD_MAP_TOOLS")
RUNTIME = Path(os.environ.get("SOURCE_USD_MAP_RUNTIME", ROOT / "run/runtime"))
PROFILE = compiler.load_compile_profile()
MAP = "usd_room"
STAGES = ["validate", "content", "game", "brushset", "vbsp", "vvis", "vrad", "assemble", "check"]
FLOOR = "/Map/World/Floor"
DESK = "/Map/Props/LabDesk"
BOX = "/Map/Props/MetalBox"
LAMP = "/Map/Lights/CeilingLamp"
S = Sdf.ValueTypeNames


def snapshot(store):
    """Every file in the store (hidden staging directories included) by hash."""
    result = {}
    for path in sorted(Path(store).rglob("*")):
        if path.is_file():
            result[str(path.relative_to(store))] = hashlib.sha256(path.read_bytes()).hexdigest()
        elif path.is_dir():
            result[str(path.relative_to(store)) + "/"] = None
    return result


def add_world_solid(room, path, ident, lo, hi, material="/Map/Looks/Wall"):
    points = box_points(lo, hi)
    mesh = UsdGeom.Mesh(new_prim(room, "world", path, "Mesh"))
    mesh.CreateSubdivisionSchemeAttr().Set("none")
    mesh.CreatePointsAttr().Set([Gf.Vec3f(*p) for p in points])
    mesh.CreateFaceVertexCountsAttr().Set([len(f) for f in BOX_FACES])
    mesh.CreateFaceVertexIndicesAttr().Set([i for f in BOX_FACES for i in f])
    UsdGeom.PrimvarsAPI(mesh).CreatePrimvar("st", S.TexCoord2fArray,
                                            UsdGeom.Tokens.faceVarying).Set(
        [Gf.Vec2f(*uv) for f in BOX_FACES for uv in st_for(points, f)])
    for name, type_, value in (("sourcemap:role", S.Token, "world_solid"),
                               ("sourcemap:id", S.String, ident),
                               ("sourcemap:contents", S.Token, "solid"),
                               ("sourcemap:surfaceIds", S.IntArray, list(range(6)))):
        attr = mesh.GetPrim().CreateAttribute(name, type_)
        attr.SetVariability(Sdf.VariabilityUniform)
        attr.Set(value)
    UsdShade.MaterialBindingAPI.Apply(mesh.GetPrim()).Bind(
        UsdShade.Material(room.stage.GetPrimAtPath(material)))


def set_prefab(room, name, value):
    """Edit the referenced prefab layer (not part of the root layer stack)."""
    spec = room.spec("prefab", "/MetalBox")
    spec.attributes[name].default = value


def chamfered_floor(room, surface_ids):
    """The floor slab with its outer south-bottom edge clipped by a new face:
    seven faces, one of them non-axial."""
    points = [(-272, -200, -16), (272, -200, -16), (272, 208, -16), (-272, 208, -16),
              (-272, -208, 0), (272, -208, 0), (272, 208, 0), (-272, 208, 0),
              (-272, -208, -8), (272, -208, -8)]
    r = 1.0 / math.sqrt(2.0)
    planes = [((0, 0, -1), 16), ((0, 0, 1), 0), ((0, 1, 0), 208), ((1, 0, 0), 272),
              ((-1, 0, 0), 272), ((0, -1, 0), 208), ((0, -r, -r), 216 * r)]
    faces = []
    for normal, dist in planes:
        on = [i for i, p in enumerate(points)
              if abs(sum(n * x for n, x in zip(normal, p)) - dist) < 1e-6]
        center = [sum(points[i][a] for i in on) / len(on) for a in range(3)]
        axis = max(range(3), key=lambda a: abs(normal[a]))
        u, v = [a for a in range(3) if a != axis]
        sign = 1 if normal[axis] > 0 else -1
        if axis == 1:
            sign = -sign  # (u, v) = (x, z) is left-handed about +y
        on.sort(key=lambda i: sign * math.atan2(points[i][v] - center[v],
                                               points[i][u] - center[u]))
        faces.append(on)
    mesh = UsdGeom.Mesh(room.prim("world", FLOOR))
    mesh.GetPointsAttr().Set([Gf.Vec3f(*p) for p in points])
    mesh.GetFaceVertexCountsAttr().Set([len(f) for f in faces])
    mesh.GetFaceVertexIndicesAttr().Set([i for f in faces for i in f])
    UsdGeom.PrimvarsAPI(mesh).GetPrimvar("st").Set(
        [Gf.Vec2f(*uv) for f in faces for uv in st_for(points, f)])
    room.set("world", FLOOR, "sourcemap:surfaceIds", surface_ids, S.IntArray)


@unittest.skipUnless(TOOLS, "set SOURCE_USD_MAP_TOOLS to installed tools with vbsp -authored")
class CompilerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not (RUNTIME / "portal/portal_pak_dir.vpk").is_file():
            raise unittest.SkipTest("Portal content missing at %s" % RUNTIME)
        cls.temporary = tempfile.TemporaryDirectory(prefix="usd-map-compile-test-")
        cls.scratch = Path(cls.temporary.name)
        cls.store = cls.scratch / "store"
        cls.good = compiler.compile_stage(FIXTURE / "room.usda", MAP, TOOLS, RUNTIME,
                                          cls.scratch / "good")
        compiler.publish(cls.good, MAP, cls.store)
        cls.serial = 0

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def work(self):
        CompilerTest.serial += 1
        return self.scratch / ("work%d" % CompilerTest.serial)

    fixture = Room
    map_name = MAP

    def room(self, edit):
        room = self.fixture(self.scratch)
        edit(room)
        return room.save()

    def build(self, stage, previous=None, checker=None):
        return compiler.compile_stage(stage, self.map_name, TOOLS, RUNTIME, self.work(),
                                      previous, checker=checker)

    def expect_failure(self, stage, code, detail=None, previous=None, checker=None):
        """The build fails with `code` (and a message containing `detail`) and
        the published store is unchanged."""
        before = snapshot(self.store)
        with self.assertRaises(compiler.CompileError) as caught:
            result = self.build(stage, previous, checker)
            compiler.publish(result, self.map_name, self.store)
        self.assertEqual(caught.exception.code, code, caught.exception.messages)
        if detail:
            self.assertTrue(any(detail in m for m in caught.exception.messages),
                            caught.exception.messages)
        self.assertEqual(snapshot(self.store), before)
        return caught.exception

    # ---------------------------------------------------------- positive

    def test_fixture_compiles_and_publishes(self):
        self.assertEqual(self.good["check"]["failures"], [])
        self.assertGreater(self.good["check"]["checks"], 1000)
        published = self.store / MAP
        data = (published / "maps" / (MAP + ".bsp")).read_bytes()
        self.assertEqual(data[:8], b"SRCBSP2\x1a")
        self.assertEqual(hashlib.sha256(data).hexdigest(),
                         self.good["provenance"]["bsp2_sha256"])
        for sidecar in ("published.json", "provenance.json", "build.json"):
            self.assertTrue((published / sidecar).is_file(), sidecar)
        self.assertEqual(sorted(playable_maps.published(self.store)), [MAP])

    def test_no_vmf_and_no_prior_bsp(self):
        work = self.good["build"].parent
        self.assertEqual([p for p in work.rglob("*") if p.suffix.lower() in (".vmf", ".vmm")],
                         [])
        log = (work / "stages/vbsp/vbsp.log").read_text()
        self.assertIn("Loading authored brush set", log)
        self.assertNotIn(".vmf", log)
        stages = json.loads(self.good["build"].read_text())["stages"]
        # No stage up to and including vbsp reads a BSP.
        self.assertFalse([k for s in stages[:5] for k in s["inputs"]
                          if k in ("bsp", "bsp2") or k.endswith(".bsp")])
        brushset = json.loads(next((work / "stages/brushset").glob("*.json")).read_text())
        self.assertEqual(brushset["schema"], "source-authored-brushset/v1")

    def test_stage_records_chain_inputs_to_outputs(self):
        stages = json.loads(self.good["build"].read_text())["stages"]
        self.assertEqual([s["stage"] for s in stages], STAGES)
        by_name = {s["stage"]: s for s in stages}
        for stage in stages:
            self.assertEqual(stage["status"], "pass")
            self.assertTrue(stage["outputs"])
            self.assertEqual(len(stage["key"]), 64)
        # Each consumer names the exact bytes its producer wrote.
        self.assertEqual(by_name["vbsp"]["inputs"]["brushset"]["sha256"],
                         by_name["brushset"]["outputs"]["brushset"]["sha256"])
        self.assertEqual(by_name["vvis"]["inputs"]["bsp"]["sha256"],
                         by_name["vbsp"]["outputs"]["bsp"]["sha256"])
        self.assertEqual(by_name["vrad"]["inputs"]["bsp"]["sha256"],
                         by_name["vvis"]["outputs"]["bsp"]["sha256"])
        self.assertEqual(by_name["assemble"]["inputs"]["bsp"]["sha256"],
                         by_name["vrad"]["outputs"]["bsp"]["sha256"])
        layers = [k for k in by_name["validate"]["inputs"] if k.startswith("layer:")]
        self.assertEqual(len(layers), 3)  # two sublayers and the prefab
        self.assertEqual(PROFILE["tools"]["vvis"], ["-threads", "1"])
        self.assertEqual(PROFILE["tools"]["vrad"], ["-threads", "1"])

    def test_rebuild_is_identical(self):
        again = self.build(FIXTURE / "room.usda")
        first = json.loads(self.good["build"].read_text())["stages"]
        second = json.loads(again["build"].read_text())["stages"]
        for a, b in zip(first, second):
            self.assertEqual(a["key"], b["key"], a["stage"])
            self.assertEqual({k: v["sha256"] for k, v in a["outputs"].items()},
                             {k: v["sha256"] for k, v in b["outputs"].items()}, a["stage"])

    def test_provenance_maps_every_authored_id(self):
        provenance = self.good["provenance"]
        report = validator.validate(str(FIXTURE / "room.usda"))
        self.assertEqual(sorted(o["id"] for o in provenance["objects"]),
                         sorted(o["id"] for o in report["objects"]))
        keys = [s["side_key"] for o in provenance["objects"]
                for s in o["output"].get("sides", [])]
        self.assertEqual(sorted(keys), list(range(1, len(keys) + 1)))
        outputs = {o["id"]: o["output"] for o in provenance["objects"]}
        self.assertEqual(outputs["prop.lab-desk"], {"static_prop": 0})
        self.assertEqual(outputs["entity.trigger-center"]["entity_class"], "trigger_multiple")

    def test_divided_room_culls_visibility(self):
        # A full-height wall splits the room into two sealed rooms, so vvis
        # must hide each from the other.
        stage = self.room(lambda r: add_world_solid(r, "/Map/World/Divider", "world.divider",
                                                    (96, -192, 0), (112, 192, 192)))
        result = self.build(stage)
        observations = result["check"]["observations"]
        self.assertEqual(observations["sealed_regions"], 2)
        self.assertGreater(observations["visibility_pairs"]["sealed"], 0)
        self.assertGreaterEqual(observations["clusters"], 2)

    def test_chamfered_solid_with_a_fresh_surface_id(self):
        stage = self.room(lambda r: chamfered_floor(r, list(range(7))))
        result = self.build(stage)
        self.assertEqual(result["check"]["failures"], [])
        floor = [o for o in result["provenance"]["objects"] if o["id"] == "world.floor"][0]
        self.assertEqual(len(floor["output"]["sides"]), 7)

    # ---------------------------------------------------------- negative

    def test_bad_seal_fails_in_the_compiler(self):
        stage = self.room(lambda r: setattr(r.spec("world", "/Map/World/WallEast"), "active",
                                            False))
        self.assertEqual(validator.validate(stage)["errors"], [])  # the seal is a compile fact
        self.expect_failure(stage, "compile.leak")

    def test_missing_model(self):
        stage = self.room(lambda r: r.set("entities", DESK, "sourcemap:model",
                                          "models/props/lab_desk01/no_such_desk.mdl", S.String))
        self.expect_failure(stage, "compile.model-missing", "no_such_desk.mdl")

    def test_physics_model_without_collision(self):
        stage = self.room(lambda r: set_prefab(r, "sourcemap:model",
                                               "models/props/cake/cake.mdl"))
        self.expect_failure(stage, "compile.model-collision-missing", "cake.phy")

    def test_missing_material(self):
        stage = self.room(lambda r: r.set("world", "/Map/Looks/Floor", "sourcemap:material",
                                          "metal/no_such_floor", S.String))
        self.expect_failure(stage, "compile.material-missing", "no_such_floor")

    def test_wrong_role_newer_than_the_stage(self):
        # The room declares profile v1; prop_dynamic arrived in v2.
        stage = self.room(lambda r: set_prefab(r, "sourcemap:role", "prop_dynamic"))
        self.expect_failure(stage, "compile.authoring-invalid", "role.unsupported")

    def test_wrong_role_against_the_published_package(self):
        # prop.lab-desk was published as a prop_static; the same id as a
        # prop_physics is a new object that needs a new id.
        def edit(room):
            room.set("entities", DESK, "sourcemap:role", "prop_physics")
            room.remove("entities", DESK, "sourcemap:collision")
        stage = self.room(edit)
        previous = compiler.published_previous(self.store, MAP)
        self.assertIsNotNone(previous)
        self.expect_failure(stage, "compile.authoring-invalid", "id.role-changed", previous)

    def test_invalid_reference(self):
        stage = self.room(lambda r: os.remove(r.directory / "prefabs/metal_box.usda"))
        self.expect_failure(stage, "compile.authoring-invalid", "composition.error")

    def test_stale_surface_id(self):
        # A clip made a new face that kept the id of the face it was cut from.
        stage = self.room(lambda r: chamfered_floor(r, [0, 1, 2, 3, 4, 5, 5]))
        self.expect_failure(stage, "compile.authoring-invalid", "surface.ids-duplicate")

    def test_unsupported_light_input(self):
        stage = self.room(lambda r: r.set("entities", LAMP, "inputs:normalize", True, S.Bool))
        self.expect_failure(stage, "compile.light-unsupported", "normalize")

    # ----------------------------------------------------------- mutants

    def mutant(self, patch, code, detail, stage=None):
        """Replace one compiler function for one build; the build must fail."""
        name, replacement = patch
        original = getattr(compiler, name)
        setattr(compiler, name, replacement(original))
        try:
            return self.expect_failure(stage or FIXTURE / "room.usda", code, detail)
        finally:
            setattr(compiler, name, original)

    def test_mutant_swapped_texture_axes(self):
        def swap(original):
            def build(report, profile, name):
                brushset, objects = original(report, profile, name)
                # The ceiling's underside (face 0) faces into the room.
                side = brushset["entities"][0]["solids"][0]["sides"][0]
                side["s"], side["t"] = side["t"], side["s"]
                return brushset, objects
            return build
        self.mutant(("build_brushset", swap), "compile.output-check", "surfaces.texture-mapping")

    def test_mutant_dropped_face(self):
        def drop(original):
            def build(report, profile, name):
                brushset, objects = original(report, profile, name)
                del brushset["entities"][0]["solids"][2]["sides"][1]
                return brushset, objects
            return build
        # vbsp's rebuilt brush no longer matches the authored solid.
        self.mutant(("build_brushset", drop), "compile.tool-failed", "authored")

    def test_mutant_shuffled_provenance(self):
        def shuffle(original):
            def build(report, profile, name):
                brushset, objects = original(report, profile, name)
                sides = objects[0]["output"]["sides"]
                sides[0]["side_key"], sides[1]["side_key"] = \
                    sides[1]["side_key"], sides[0]["side_key"]
                return brushset, objects
            return build
        self.mutant(("build_brushset", shuffle), "compile.output-check",
                    "surfaces.face-provenance")

    def test_mutant_light_policy(self):
        def brighter(original):
            def keys(record, profile):
                result = original(record, profile)
                values = result["_light"].split()
                values[3] = compiler.number(float(values[3]) * 2)
                result["_light"] = " ".join(values)
                return result
            return keys
        self.mutant(("light_keys", brighter), "compile.output-check", "entities.light-policy")

    def test_mutant_moved_entity(self):
        def move(original):
            def build(report, profile, name):
                brushset, objects = original(report, profile, name)
                entity = [e for e in brushset["entities"]
                          if e.get("classname") == "info_player_start"][0]
                entity["origin"] = [entity["origin"][0] + 8] + entity["origin"][1:]
                return brushset, objects
            return build
        self.mutant(("build_brushset", move), "compile.output-check", "entities.origin")

    def test_mutant_skipped_visibility(self):
        def skip(original):
            def stage(tool, extra_inputs=()):
                if tool != "vvis":
                    return original(tool, extra_inputs)

                def copy(directory, inputs, params, record):
                    bsp = directory / (params["map"] + ".bsp")
                    shutil.copyfile(inputs["bsp"], bsp)
                    return {"bsp": bsp}
                return copy
            return stage
        self.mutant(("tool_stage", skip), "compile.output-check", "visibility.vvis-data")

    def test_mutant_skipped_leak_check_is_caught_by_vbsp(self):
        # Without the driver's leak test (.lin present, no .prt), vbsp's own
        # authored-mode leak error still stops the build. The C++ half is
        # mutated separately (RFC 0009 progress record).
        def without_leak_test(original):
            def stage(directory, inputs, params, record):
                profile = compiler.load_compile_profile(inputs["profile"])
                brushset = directory / (params["map"] + ".json")
                shutil.copyfile(inputs["brushset"], brushset)
                status, log = compiler.run_tool(
                    directory, "vbsp", [inputs["vbsp"]] + profile["tools"]["vbsp"] +
                    ["-game", Path(inputs["gameinfo"]).parent, brushset], params["timeout"])
                bsp = directory / (params["map"] + ".bsp")
                if status != 0 or not bsp.is_file():
                    compiler.tool_failed(profile, "vbsp", status, log)
                return {"bsp": bsp, "portals": directory / (params["map"] + ".prt")}
            return stage
        leak = self.room(lambda r: setattr(r.spec("world", "/Map/World/WallEast"), "active",
                                           False))
        self.mutant(("stage_vbsp", without_leak_test), "compile.tool-failed",
                    "authored map leaked", leak)

    def test_checker_rejects_another_maps_output(self):
        # The oracle is the authored stage: the divided room's BSP2 checked
        # against the fixture's objects has faces on no authored surface.
        stage = self.room(lambda r: add_world_solid(r, "/Map/World/Divider", "world.divider",
                                                    (96, -192, 0), (112, 192, 192)))
        other = self.build(stage)
        report = validator.validate(str(FIXTURE / "room.usda"))
        provenance = dict(other["provenance"])
        provenance["objects"] = [o for o in provenance["objects"] if o["id"] != "world.divider"]
        result = usd_map_check.check_map(other["bsp2"], report, provenance, PROFILE,
                                         str(RUNTIME))
        names = {failure.split(":")[0] for failure in result["failures"]}
        self.assertIn("surfaces.face-on-authored-surface", names)


ROLES_MAP = "usd_roles"


def entity_of(brushset, ident):
    return next(e for e in brushset["entities"] if e.get("id") == ident)


@unittest.skipUnless(TOOLS, "set SOURCE_USD_MAP_TOOLS to installed tools with vbsp -authored")
class RolesCompilerTest(CompilerTest):
    """U2: the role fixture's static, dynamic and physics props and its
    geometric entities compile to their own contracts."""

    fixture = Roles
    map_name = ROLES_MAP

    @classmethod
    def setUpClass(cls):
        if not (RUNTIME / "portal/portal_pak_dir.vpk").is_file():
            raise unittest.SkipTest("Portal content missing at %s" % RUNTIME)
        cls.temporary = tempfile.TemporaryDirectory(prefix="usd-map-roles-test-")
        cls.scratch = Path(cls.temporary.name)
        cls.store = cls.scratch / "store"
        cls.good = compiler.compile_stage(ROLES / "roles.usda", ROLES_MAP, TOOLS, RUNTIME,
                                          cls.scratch / "good")
        compiler.publish(cls.good, ROLES_MAP, cls.store)
        cls.serial = 0

    def work(self):
        RolesCompilerTest.serial += 1
        return self.scratch / ("work%d" % RolesCompilerTest.serial)

    # Inherited U1 tests that are specific to the room fixture do not apply.
    for _name in [n for n in dir(CompilerTest) if n.startswith("test_")]:
        locals()[_name] = None
    del _name

    # ---------------------------------------------------------- positive

    def test_roles_fixture_compiles_and_publishes(self):
        self.assertEqual(self.good["check"]["failures"], [])
        self.assertGreater(self.good["check"]["checks"], 1000)
        self.assertEqual(sorted(playable_maps.published(self.store)), [ROLES_MAP])
        observations = self.good["check"]["observations"]
        self.assertEqual(observations["targetnames"],
                         ["entity.drop-trigger", "entity.lift", "prop.drop-box",
                          "prop.lever-switch"])
        self.assertEqual(observations["static_props"], 2)
        shadows = observations["static_prop_shadows"]
        self.assertEqual(sorted(shadows), ["prop.ghost-desk", "prop.lab-desk"])
        self.assertEqual(observations["static_prop_shadows_unmeasured"], [])
        for shadow in shadows.values():
            self.assertLess(shadow["ratio"], usd_map_check.SHADOW_RATIO)

    def test_role_outputs_in_provenance(self):
        outputs = {o["id"]: o["output"] for o in self.good["provenance"]["objects"]}
        self.assertEqual(outputs["prop.ghost-desk"], {"static_prop": 0})
        self.assertEqual(outputs["prop.lab-desk"], {"static_prop": 1})
        self.assertEqual(outputs["prop.lever-switch"], {"entity_class": "prop_dynamic"})
        self.assertEqual(outputs["prop.drop-box"], {"entity_class": "prop_physics"})
        self.assertEqual(outputs["entity.lift"]["entity_class"], "func_movelinear")

    # ---------------------------------------------------------- negative

    def test_roles_swapped_against_the_published_package(self):
        def edit(room):
            room.set("entities", "/Map/Props/LabDesk", "sourcemap:role", "prop_dynamic")
            room.set("entities", SWITCH, "sourcemap:role", "prop_static")
            room.remove("entities", SWITCH, "sourcemap:defaultAnimation")
            room.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnStartTouch entity.lift Open"], S.StringArray)
        stage = self.room(edit)
        previous = compiler.published_previous(self.store, ROLES_MAP)
        self.assertIsNotNone(previous)
        error = self.expect_failure(stage, "compile.authoring-invalid", "id.role-changed",
                                    previous)
        self.assertEqual(sum("id.role-changed" in m for m in error.messages), 2)

    def test_roles_swapped_in_a_fresh_document(self):
        # Without a previous revision the stage is well formed, but the
        # switch's model is not a $staticprop, so it cannot be a prop_static.
        def edit(room):
            room.set("entities", "/Map/Props/LabDesk", "sourcemap:role", "prop_dynamic")
            room.set("entities", SWITCH, "sourcemap:role", "prop_static")
            room.remove("entities", SWITCH, "sourcemap:defaultAnimation")
            room.set("entities", DROP_TRIGGER, "sourcemap:connections",
                     ["OnStartTouch entity.lift Open"], S.StringArray)
        self.expect_failure(self.room(edit), "compile.model-role-mismatch", "$staticprop")

    def test_physics_prop_without_collision(self):
        stage = self.room(lambda r: set_prefab(r, "sourcemap:model",
                                               "models/props/cake/cake.mdl"))
        self.expect_failure(stage, "compile.model-collision-missing", "cake.phy")

    def test_dynamic_prop_with_physics_keys(self):
        stage = self.room(lambda r: r.set("entities", SWITCH, "sourcemap:massScale", 2.0,
                                          S.Float))
        self.expect_failure(stage, "compile.authoring-invalid", "property.misplaced")

    def test_unsupported_parenting(self):
        stage = self.room(lambda r: r.set("entities", SWITCH, "sourcemap:parent",
                                          "prop.lab-desk", S.String))
        self.expect_failure(stage, "compile.authoring-invalid", "property.unsupported")

    def test_unknown_key(self):
        stage = self.room(lambda r: r.set("entities", SWITCH, "sourcemap:renderColor",
                                          "255 0 0", S.String))
        self.expect_failure(stage, "compile.authoring-invalid", "property.unknown")

    def test_animation_the_model_lacks(self):
        stage = self.room(lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                                          ["OnStartTouch prop.lever-switch SetAnimation dance"],
                                          S.StringArray))
        self.expect_failure(stage, "compile.model-sequence-missing", "'dance'")

    def test_default_animation_the_model_lacks(self):
        stage = self.room(lambda r: r.set("entities", SWITCH, "sourcemap:defaultAnimation",
                                          "wave", S.String))
        self.expect_failure(stage, "compile.model-sequence-missing", "default animation")

    def test_connection_to_a_missing_target(self):
        stage = self.room(lambda r: r.set("entities", DROP_TRIGGER, "sourcemap:connections",
                                          ["OnStartTouch entity.gone Open"], S.StringArray))
        self.expect_failure(stage, "compile.authoring-invalid", "io.target-missing")

    # ----------------------------------------------------------- mutants

    def brushset_mutant(self, change, code, detail, stage=None):
        def patch(original):
            def build(report, profile, name):
                brushset, objects = original(report, profile, name)
                change(brushset)
                return brushset, objects
            return build
        return self.mutant(("build_brushset", patch), code, detail,
                           stage or ROLES / "roles.usda")

    def test_mutant_dynamic_prop_emitted_as_physics(self):
        def change(brushset):
            switch = entity_of(brushset, "prop.lever-switch")
            switch["classname"] = "prop_physics"
            for key in ("DefaultAnim", "solid"):
                switch["keys"].pop(key)
        self.brushset_mutant(change, "compile.output-check", "entities.class")

    def test_mutant_roles_swapped_in_the_brushset(self):
        def change(brushset):
            box = entity_of(brushset, "prop.drop-box")
            switch = entity_of(brushset, "prop.lever-switch")
            box["classname"], switch["classname"] = switch["classname"], box["classname"]
        self.brushset_mutant(change, "compile.output-check", "entities.class")

    def test_mutant_dropped_collision_requirement(self):
        def lenient(original):
            def check(report, profile, resolver):
                relaxed = json.loads(json.dumps(profile))
                for rule in relaxed["model_files"].values():
                    rule["always"] = [".mdl"]
                    rule.pop("collision:vphysics", None)
                return original(report, relaxed, resolver)
            return check
        stage = self.room(lambda r: set_prefab(r, "sourcemap:model",
                                               "models/props/cake/cake.mdl"))
        self.mutant(("check_content", lenient), "compile.output-check", "cake.phy", stage)

    def test_mutant_trigger_touch_filter_ignored(self):
        def change(brushset):
            entity_of(brushset, "entity.drop-trigger")["keys"]["spawnflags"] = "1"
        self.brushset_mutant(change, "compile.output-check", "roles.trigger-touch-filter")

    def test_mutant_connections_dropped(self):
        def change(brushset):
            entity_of(brushset, "entity.drop-trigger")["connections"].pop()
        self.brushset_mutant(change, "compile.output-check", "roles.connections")

    def test_mutant_lift_speed(self):
        def change(brushset):
            keys = entity_of(brushset, "entity.lift")["keys"]
            keys["speed"] = compiler.number(float(keys["speed"]) * 2)
        self.brushset_mutant(change, "compile.output-check", "roles.movelinear-duration")

    def test_mutant_lift_direction(self):
        def change(brushset):
            entity_of(brushset, "entity.lift")["keys"]["movedir"] = "90 0 0"
        self.brushset_mutant(change, "compile.output-check", "roles.movelinear-displacement")

    def test_mutant_lift_tool_material(self):
        def change(brushset):
            for side in entity_of(brushset, "entity.lift")["solids"][0]["sides"]:
                side["material"] = "metal/metal_modular_floor001"
        self.brushset_mutant(change, "compile.output-check", "roles.brush-entity-material")

    def test_mutant_static_prop_without_baked_shadow(self):
        def change(brushset):
            entity_of(brushset, "prop.lab-desk")["keys"]["disableshadows"] = "1"
        error = self.brushset_mutant(change, "compile.output-check", "props.casts-baked-shadow")
        self.assertTrue(any("lighting.static-prop-shadow" in m for m in error.messages),
                        error.messages)

    def test_mutant_static_prop_collision(self):
        def change(brushset):
            entity_of(brushset, "prop.lab-desk")["keys"]["solid"] = "0"
        self.brushset_mutant(change, "compile.output-check", "props.solid")

    def test_mutant_default_animation_dropped(self):
        def change(brushset):
            entity_of(brushset, "prop.lever-switch")["keys"].pop("DefaultAnim")
        self.brushset_mutant(change, "compile.output-check", "roles.dynamic-default-animation")

    def test_mutant_unnamed_dynamic_prop(self):
        def change(brushset):
            entity_of(brushset, "prop.lever-switch")["keys"].pop("targetname")
        self.brushset_mutant(change, "compile.output-check", "entities.targetname")


class PolicyTest(unittest.TestCase):
    """Compiler policy that needs no tools."""

    @classmethod
    def setUpClass(cls):
        cls.report = validator.validate(str(FIXTURE / "room.usda"))

    def lamp(self, **inputs):
        record = json.loads(json.dumps([o for o in self.report["objects"]
                                        if o["role"] == "light"][0]))
        record["inputs"].update(inputs)
        return record

    def test_light_policy_values(self):
        keys = compiler.light_keys(self.lamp(), PROFILE)
        r, g, b, brightness = map(float, keys["_light"].split())
        # 2000 nits over a 4-unit sphere: I = 2000 * pi * (0.1016 m)^2 cd;
        # B = 255 * I / (2.54^2 * 10 lux).
        candela = 2000 * math.pi * 0.1016 ** 2
        self.assertAlmostEqual(brightness, 255 * candela / (2.54 ** 2 * 10.0), places=2)
        self.assertEqual(r, 255.0)
        self.assertAlmostEqual(g, 255 * 0.93 ** (1 / 2.2), places=2)
        self.assertAlmostEqual(b, 255 * 0.82 ** (1 / 2.2), places=2)
        self.assertEqual(keys["_quadratic_attn"], "1")

    def test_light_policy_scales(self):
        base = float(compiler.light_keys(self.lamp(), PROFILE)["_light"].split()[3])
        for inputs, factor in (({"exposure": 1.0}, 2.0), ({"diffuse": 0.5}, 0.5),
                               ({"radius": 8.0}, 4.0)):
            value = float(compiler.light_keys(self.lamp(**inputs), PROFILE)["_light"].split()[3])
            self.assertAlmostEqual(value / base, factor, places=3, msg=str(inputs))

    def test_light_policy_rejects(self):
        for inputs in ({"normalize": True}, {"enableColorTemperature": True}, {"radius": 0.0}):
            with self.assertRaises(compiler.CompileError) as caught:
                compiler.light_keys(self.lamp(**inputs), PROFILE)
            self.assertEqual(caught.exception.code, "compile.light-unsupported", inputs)

    def test_brushset_order_and_keys(self):
        brushset, objects = compiler.build_brushset(self.report, PROFILE, MAP)
        entities = brushset["entities"]
        self.assertEqual(entities[0]["classname"], "worldspawn")
        self.assertNotIn("id", entities[0])
        self.assertEqual(len(entities[0]["solids"]), 6)
        self.assertEqual([e["classname"] for e in entities[1:]],
                         ["info_player_start", "trigger_multiple", "prop_static",
                          "prop_physics", "light"])
        for entity in entities:
            self.assertFalse({"classname", "origin", "angles", "sourcemap_id"} &
                             set(entity["keys"]))
        brush_keys = [s["brush_key"] for e in entities for s in e.get("solids", [])]
        self.assertEqual(brush_keys, list(range(1, 8)))
        trigger = entities[2]["solids"][0]
        self.assertEqual({s["material"] for s in trigger["sides"]}, {"tools/toolstrigger"})
        self.assertEqual(len(objects), len(self.report["objects"]))

    def test_role_brushset_keys(self):
        report = validator.validate(str(ROLES / "roles.usda"))
        self.assertEqual(report["errors"], [])
        brushset, _ = compiler.build_brushset(report, PROFILE, ROLES_MAP)
        self.assertEqual([e["classname"] for e in brushset["entities"][1:]],
                         ["info_player_start", "trigger_multiple", "func_movelinear",
                          "prop_static", "prop_static", "prop_dynamic", "prop_physics",
                          "light"])
        switch = entity_of(brushset, "prop.lever-switch")
        self.assertEqual(switch["keys"], {"model": "models/props/switch001.mdl",
                                          "targetname": "prop.lever-switch", "solid": "6",
                                          "DefaultAnim": "idle"})
        self.assertEqual(entity_of(brushset, "prop.drop-box")["keys"],
                         {"model": "models/props/metal_box.mdl",
                          "targetname": "prop.drop-box"})
        self.assertEqual(entity_of(brushset, "prop.ghost-desk")["keys"]["solid"], "0")
        self.assertEqual(entity_of(brushset, "prop.lab-desk")["keys"]["disableshadows"], "0")
        trigger = entity_of(brushset, "entity.drop-trigger")
        self.assertEqual(trigger["keys"]["spawnflags"], "9")
        self.assertEqual(trigger["connections"], [
            {"output": "OnStartTouch", "target": "prop.lever-switch", "input": "SetAnimation",
             "parameter": "down", "delay": 0, "times": -1},
            {"output": "OnStartTouch", "target": "entity.lift", "input": "Open",
             "parameter": "", "delay": 0, "times": -1}])
        lift = entity_of(brushset, "entity.lift")
        self.assertEqual({k: lift["keys"][k] for k in ("movedir", "movedistance", "speed")},
                         {"movedir": "-90 0 0", "movedistance": "64", "speed": "64"})
        self.assertEqual({side["material"] for side in lift["solids"][0]["sides"]},
                         {"metal/metalwall048b"})
        self.assertNotIn("connections", entity_of(brushset, "prop.lever-switch"))

    def test_direction_angles_invert_angle_vectors(self):
        for vector in ((0, 0, 64), (0, 0, -3), (1, 0, 0), (0, -2, 0), (3, 4, 5), (-1, 2, -7)):
            forward = usd_map_check.angle_vectors(compiler.direction_angles(vector))
            length = math.sqrt(sum(x * x for x in vector))
            for got, want in zip(forward, vector):
                self.assertAlmostEqual(got, want / length, places=9, msg=str(vector))

    def test_every_error_code_is_declared_and_exercised(self):
        source = (ROOT / "tools/quality/usd_map_compile.py").read_text()
        tests = Path(__file__).read_text()
        for code in PROFILE["errors"]:
            self.assertIn('"%s"' % code, source, code)
            self.assertIn('"%s"' % code, tests, code)
        with self.assertRaises(KeyError):
            compiler.fail(PROFILE, "compile.made-up", [])

    def test_stage_runner_contract(self):
        with tempfile.TemporaryDirectory() as scratch:
            build = compiler.Build(Path(scratch) / "work")
            source = Path(scratch) / "input.txt"
            source.write_text("a")
            outside = Path(scratch) / "outside.txt"
            outside.write_text("x")
            with self.assertRaises(RuntimeError):
                build.run("escape", {"input": source}, {}, lambda d, i, p, r: {"out": outside})
            first = build.run("one", {"input": source}, {"n": 1},
                              lambda d, i, p, r: {"out": self.write(d / "out", "1")})
            source.write_text("b")
            second = build.run("two", {"input": source}, {"n": 1},
                               lambda d, i, p, r: {"out": self.write(d / "out", "1")})
            records = build.record()["stages"]
            self.assertNotEqual(records[1]["inputs"]["input"]["sha256"],
                                records[2]["inputs"]["input"]["sha256"])
            self.assertEqual(first["out"].read_text(), second["out"].read_text())

    @staticmethod
    def write(path, text):
        path.write_text(text)
        return path

    def test_publish_sidecar_names(self):
        with tempfile.TemporaryDirectory() as scratch:
            content = Path(scratch) / "content/maps"
            content.mkdir(parents=True)
            (content / "m.bsp").write_bytes(b"x")
            summary = {"map": "m", "status": "pass", "content_root": str(content.parent)}
            for bad in ("published.json", "../x.json", "notes.txt"):
                with self.assertRaises(ValueError, msg=bad):
                    playable_maps.publish(summary, Path(scratch) / "store", {bad: "{}"})
            playable_maps.publish(summary, Path(scratch) / "store", {"provenance.json": "{}"})
            self.assertTrue((Path(scratch) / "store/m/provenance.json").is_file())


if __name__ == "__main__":
    unittest.main()
