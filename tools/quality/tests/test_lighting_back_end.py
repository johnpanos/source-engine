"""The one map-lighting back end: its manifest model, its step plan and seam.

Every front end ends in a compiled BSP; the back end (pbrt_map_build.Pipeline,
entered through map_lighting.py) lights it with an authored scene or one
derived from the BSP. These tests plan builds without running any tool: a
recording pipeline notes each step and fakes its outputs, and runs only the
baker's operations, against a recording baker. No Blender, no GPU.
"""

import json
import re
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(HERE))
import light_baker  # noqa: E402
import map_lighting  # noqa: E402
import pbrt_map_build  # noqa: E402
import radiosity_transfer  # noqa: E402
import vmf_map_build  # noqa: E402

MODEL = {"schema": "map-scene/v1", "format": "usd", "shapes": [
    {"name": "Floor", "material": "White"}, {"name": "Lamp", "material": "Glow"},
    {"name": "Cube", "material": "Glow", "role": "prop"}],
    "materials": {"White": {"base_color": [0.8, 0.8, 0.8], "metallic": 0.0, "roughness": 0.8,
                            "transmission": 0.0},
                  "Glow": {"base_color": [0.0, 0.0, 0.0], "metallic": 0.0, "roughness": 1.0,
                           "transmission": 0.0, "emission_color": [4.0, 3.0, 2.0]}},
    "emitters": [{"kind": "disk", "name": "Spot"}, {"kind": "rect"}],
    "distant_lights": [{"direction": [0, 0, -1]}], "environment": None,
    "props": [{"name": "CubeProp", "model": "models/cube.mdl", "origin_m": [0, 0, 0],
               "shapes": ["Cube"]}]}


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    return path


class Recording(pbrt_map_build.Pipeline):
    """A pipeline that plans: each step is noted and its outputs faked; only
    the baker's operations run (against the recording baker)."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.order, self.baked = [], []
        self.baker = light_baker.CyclesBaker(
            lambda operation, script, arguments: self.baked.append((operation, script)) or 0.0)

    def step(self, name, inputs, settings, scripts, outputs, action):
        self.order.append(name)
        if name in light_baker.OPERATIONS:
            action()
        for output in map(Path, outputs):
            if output.suffix in ("", ".d") and not output.name.endswith("scene.usda"):
                output.mkdir(parents=True, exist_ok=True)
            elif output.name == "scene.json":
                write(output, json.dumps(MODEL))
            else:
                write(output, "{}")
        if name == "radiosity":
            write(self.paths["rtrn_work"] / "rtrn-bake.json", json.dumps({"sources": []}))

    def finish(self, scene, environment, reference):
        self.order.append("finish")


def plan(manifest, out):
    path = write(Path(out) / "manifest.json", json.dumps(manifest))
    tools = {key: "/nonexistent/" + key for key in (
        "blender", "ocio", "usd_python", "usd_pythonpath", "compile_tools", "ktx", "xatlas",
        "runtime", "client_build", "bsp2tool")}
    pipeline = Recording(pbrt_map_build.load_manifest(path), tools, Path(out), None, False,
                         publish=False)
    pipeline.build()
    return pipeline


class ManifestTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        self.bsp = write(self.tmp / "room.bsp", "VBSP")
        self.scene = write(self.tmp / "room.usda", "#usda 1.0\n")

    def load(self, **fields):
        return pbrt_map_build.load_manifest(write(self.tmp / "m.json", json.dumps(
            dict({"schema": "pbrt-map-manifest/v1", "map": "room"}, **fields))))

    def test_a_bsp_alone_derives_its_scene(self):
        manifest = self.load(bsp=str(self.bsp))
        self.assertIsNone(manifest["scene"])
        self.assertEqual(manifest["quality"], pbrt_map_build.LEGACY_QUALITY)

    def test_the_old_name_is_the_same_key(self):
        self.assertEqual(self.load(legacy_bsp=str(self.bsp))["bsp"], str(self.bsp))
        with self.assertRaises(ValueError):
            self.load(legacy_bsp=str(self.bsp), bsp=str(self.bsp))

    def test_a_bsp_with_an_authored_scene(self):
        manifest = self.load(bsp=str(self.bsp), scene=str(self.scene))
        self.assertEqual((manifest["bsp"], manifest["scene"], manifest["scene_format"]),
                         (str(self.bsp), str(self.scene), "usd"))

    def test_a_compiled_bsp_has_no_collision(self):
        with self.assertRaises(ValueError):
            self.load(bsp=str(self.bsp), collision={})

    def test_map_lighting_writes_the_back_end_manifest(self):
        manifest = map_lighting.manifest_for(self.bsp, "room", scene=self.scene,
                                             runtime=self.tmp, device="gpu")
        self.assertEqual(manifest["bsp"], str(self.bsp.resolve()))
        self.assertEqual(manifest["scene"], str(self.scene.resolve()))
        self.assertEqual(manifest["lightmap"], {"device": "gpu"})
        self.assertEqual(manifest["legacy_runtime"], str(self.tmp.resolve()))


class PlanTest(unittest.TestCase):
    """Front end (collision, compile) before the back end; bakes via the seam."""

    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        self.bsp = write(self.tmp / "room.bsp", "VBSP")
        self.model = write(self.tmp / "room.json", json.dumps(MODEL))

    def check_back_end(self, pipeline):
        order = pipeline.order
        for step in ("bake", "probe-volume", "radiosity", "sdf", "pack", "identity", "finish"):
            self.assertIn(step, order)
        self.assertLess(order.index("pack"), order.index("identity"))
        self.assertLess(order.index("identity"), order.index("finish"))
        self.assertEqual([script for _, script in pipeline.baked],
                         [light_baker.OPERATIONS[op][0] for op, _ in pipeline.baked])
        self.assertEqual({op for op, _ in pipeline.baked},
                         {"bake", "probe-volume", "radiosity", "sdf"})

    def test_derived_scene(self):
        pipeline = plan({"schema": "pbrt-map-manifest/v1", "map": "room", "bsp": str(self.bsp),
                         "quality": "legacy-relight-preview"}, self.tmp / "derived")
        self.assertTrue(pipeline.derived)
        self.assertEqual(pipeline.order[0], "legacy-scene")
        self.assertNotIn("collision", pipeline.order)
        self.assertEqual(pipeline.paths["bsp"], self.bsp)
        self.check_back_end(pipeline)

    def test_authored_scene_over_a_given_bsp(self):
        pipeline = plan({"schema": "pbrt-map-manifest/v1", "map": "room", "bsp": str(self.bsp),
                         "scene": str(self.model), "quality": "legacy-relight-preview"},
                        self.tmp / "authored")
        self.assertFalse(pipeline.derived)
        self.assertNotIn("legacy-scene", pipeline.order)
        self.assertNotIn("collision", pipeline.order)
        self.check_back_end(pipeline)

    def test_scene_map_compiles_its_bsp_before_any_bake(self):
        pipeline = plan({"schema": "pbrt-map-manifest/v1", "map": "room",
                         "scene": str(self.model), "quality": "legacy-relight-preview"},
                        self.tmp / "scene")
        order = pipeline.order
        self.assertLess(order.index("collision"), order.index("compile"))
        self.assertLess(order.index("compile"), order.index("layout"))
        self.assertLess(order.index("compile"), order.index("bake"))
        self.check_back_end(pipeline)

    def test_step_order_constant(self):
        steps = pbrt_map_build.STEPS
        self.assertLess(steps.index("compile"), steps.index("bake"))
        self.assertEqual(steps.index("identity"), steps.index("pack") + 1)


class SeamTest(unittest.TestCase):
    def test_every_operation_has_its_script(self):
        for operation, (script, reads) in light_baker.OPERATIONS.items():
            for name in [script] + reads:
                self.assertTrue((HERE / name).is_file(), (operation, name))

    @staticmethod
    def bake_call():
        """A line that hands a bake script to Blender or a runner."""
        scripts = [script for script, _ in light_baker.OPERATIONS.values()]
        return scripts, re.compile(r"(blender|--python|\.bake\(|\.run\()[^\n]*(%s)" %
                                   "|".join(re.escape(s) for s in scripts))

    def test_the_scan_catches_a_direct_call(self):
        _, call = self.bake_call()
        for line in ('tools.blender("probe_volume_bake.py", arguments, log)',
                     'self.run("x", [blender, "--python", HERE / "sdf_volume_bake.py"])',
                     'self.blender("bake", "pbrt_lightmap_bake.py", bake_args)'):
            self.assertTrue(call.search(line), line)
        for line in ('baker.bake("probe-volume", arguments)',
                     'per glossy surface (pbrt_reflection_probe.py), six Cycles cube faces'):
            self.assertFalse(call.search(line), line)

    def test_only_the_seam_runs_a_bake_script(self):
        # A bake script is executed only through light_baker.OPERATIONS: no
        # other tool passes one to Blender or names one as a call argument.
        scripts, call = self.bake_call()
        offenders = []
        for path in sorted(HERE.glob("*.py")):
            if path.name == "light_baker.py" or path.name in scripts:
                continue
            for number, line in enumerate(path.read_text().splitlines(), 1):
                if call.search(line) and not line.lstrip().startswith("#"):
                    offenders.append("%s:%d: %s" % (path.name, number, line.strip()))
        self.assertEqual(offenders, [])

    def test_pipeline_bakes_name_operations_not_scripts(self):
        source = (HERE / "pbrt_map_build.py").read_text()
        for operation, (script, _) in light_baker.OPERATIONS.items():
            self.assertNotIn('self.blender("%s"' % operation, source)
            self.assertIn('self.baker.bake("%s"' % operation, source)

    def test_the_path7_bridge_is_retired(self):
        for name in ("worldstage_cycles_bake_preview.py", "worldstage_legacy_lightmap_preview.py",
                     "worldstage_cycles_supplemental_bakes.py"):
            self.assertFalse((HERE / name).exists(), name)


class SceneSourcesTest(unittest.TestCase):
    def test_order_names_and_styles(self):
        sources = radiosity_transfer.scene_sources(MODEL)
        self.assertEqual([(s["name"], s["kind"], s["style"]) for s in sources],
                         [("Spot", "light", 32), ("LightQuad01", "light", 33),
                          ("Sun00", "light", 34), ("Glow", "emissive", 35)])
        self.assertEqual(sources[0]["emitter"], 0)
        self.assertEqual(sources[2]["sun"], 0)

    def test_prop_only_materials_do_not_emit(self):
        model = dict(MODEL, shapes=[{"name": "Floor", "material": "White"},
                                    {"name": "Cube", "material": "Glow"}])
        self.assertEqual(radiosity_transfer.emissive_materials(model), [])

    def test_authored_styles_are_kept(self):
        model = dict(MODEL, authored_light_styles=True,
                     emitters=[{"kind": "disk", "name": "Named", "style": 40},
                               {"kind": "rect"}])
        styles = [s["style"] for s in radiosity_transfer.scene_sources(model)]
        self.assertEqual(styles, [40, -1, -1, -1])

    def test_switchable_cut_off(self):
        model = dict(MODEL, emitters=[{"kind": "rect"} for _ in range(40)])
        switchable = radiosity_transfer.switchable_sources(model)
        self.assertEqual(len(switchable), radiosity_transfer.MAX_SWITCHED)
        self.assertEqual(switchable[-1]["style"], 63)

    def test_duplicate_names_are_made_unique(self):
        model = dict(MODEL, emitters=[{"kind": "rect", "name": "A"}, {"kind": "disk", "name": "A"}],
                     distant_lights=[])
        names = [s["name"] for s in radiosity_transfer.scene_sources(model)]
        self.assertEqual(names[:2], ["A", "A_1"])


class TransferSourcesTest(unittest.TestCase):
    @staticmethod
    def sources(lights, switched=0, emissive=0):
        out = [{"name": "light%d" % i, "kind": "light",
                "style": radiosity_transfer.FIRST_STYLE + i if i < switched else -1}
               for i in range(lights)]
        return out + [{"name": "sign%d" % i, "kind": "emissive", "style": -1}
                      for i in range(emissive)]

    def test_each_source_stays_its_own_within_the_limit(self):
        sources = self.sources(60, switched=5, emissive=4)
        groups = radiosity_transfer.transfer_sources(sources)
        self.assertEqual([g["name"] for g in groups], [s["name"] for s in sources])
        self.assertEqual([g["members"] for g in groups], [[i] for i in range(64)])

    def test_fixed_lights_become_one_source_over_the_limit(self):
        """testchmb_a_15: 84 lights (1 switched) and 3 emissive surfaces."""
        sources = self.sources(84, switched=1, emissive=3)
        groups = radiosity_transfer.transfer_sources(sources)
        self.assertEqual([g["name"] for g in groups],
                         ["light0", "sign0", "sign1", "sign2", "fixed_lights"])
        self.assertEqual(groups[-1]["style"], -1)
        self.assertEqual(groups[-1]["members"], list(range(1, 84)))
        # Every source is transported exactly once.
        self.assertEqual(sorted(i for g in groups for i in g["members"]), list(range(87)))

    def test_too_many_separate_sources_fail(self):
        with self.assertRaises(ValueError):
            radiosity_transfer.transfer_sources(self.sources(40, switched=32, emissive=40))

    def test_the_aggregate_name_does_not_collide(self):
        sources = self.sources(70) + [{"name": "fixed_lights", "kind": "emissive", "style": -1}]
        names = [g["name"] for g in radiosity_transfer.transfer_sources(sources)]
        self.assertEqual(names, ["fixed_lights", "fixed_lights_1"])


class FrontEndTest(unittest.TestCase):
    def test_vmf_front_end_hands_its_bsp_to_the_back_end(self):
        tmp = Path(tempfile.mkdtemp())
        content = tmp / "content"
        write(content / "maps" / "room.bsp", "VBSP")
        record = {"status": "pass", "map": "room", "content_root": str(content)}
        with mock.patch.object(map_lighting, "light",
                               return_value={"status": "pass"}) as light, \
                mock.patch.object(map_lighting, "load_toolchain", return_value={"t": 1}):
            identity = vmf_map_build.light(record, tmp, "legacy-relight-preview", tmp / "rt",
                                           device="gpu", publish=False)
        self.assertEqual(identity["status"], "pass")
        args, kwargs = light.call_args
        self.assertEqual(args[:3], (content / "maps" / "room.bsp", "room", tmp / "lighting"))
        self.assertEqual(kwargs["quality"], "legacy-relight-preview")
        self.assertEqual(kwargs["device"], "gpu")
        self.assertEqual(record["lighting"]["identity"], "pass")


if __name__ == "__main__":
    unittest.main()
