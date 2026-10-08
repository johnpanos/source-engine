"""The production probe bake retains light that has no runtime producer."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
SCRIPT = r'''
import json, sys
from pathlib import Path
import bpy
sys.path.insert(0, sys.argv[-2])
import pbrt_blender as shared
import probe_volume_bake as bake

shared.clear_scene()
shared.configure_cycles(32, "cpu")
shared.pin_sampling(seed=20260924)
scene = bpy.context.scene
scene.render.bake.use_clear = True
scene.render.bake.margin = 0
scene.render.bake.use_pass_color = False
world = bpy.data.worlds.new("Sky")
world.use_nodes = True
scene.world = world
background = world.node_tree.nodes["Background"]
background.inputs["Color"].default_value = (1, 1, 1, 1)
background.inputs["Strength"].default_value = 0.25
lamp = bpy.data.lights.new("RuntimeSun", "SUN")
lamp.energy = 2
sun = bpy.data.objects.new("RuntimeSun", lamp)
scene.collection.objects.link(sun)
obj, target, height = bake.receiver_mesh([[0, 0, 0]], [[0, 0, 1]])
root = Path(sys.argv[-1])
result = {}
for name, sky, runtime in (("sky", .25, False), ("runtime", 0, True),
                           ("mixed", .25, True)):
    background.inputs["Strength"].default_value = sky
    sun.hide_render = not runtime
    work = root / name
    work.mkdir()
    layers, images, static = bake.bake_irradiance_layers(obj, target, height, [], work)
    result[name] = {"total": layers[0].reshape(-1, 3)[0].tolist(),
                    "indirect": layers[1].reshape(-1, 3)[0].tolist(),
                    "static": static}

# A failed bake must restore prior visibility and emission, including a lamp
# that was already hidden. Neither context removes geometry from the scene.
background.inputs["Strength"].default_value = .25
sun.hide_render = False
hidden = bpy.data.objects.new("HiddenSun", bpy.data.lights.new("HiddenSun", "SUN"))
scene.collection.objects.link(hidden)
hidden.hide_render = True
try:
    with shared.static_emitters_only([]):
        if not sun.hide_render or background.inputs["Strength"].default_value != .25:
            raise RuntimeError("static selection is wrong")
        raise ValueError("injected bake failure")
except ValueError:
    pass
result["visibility_restored"] = not sun.hide_render and hidden.hide_render
try:
    with shared.runtime_lights_only([]):
        if background.inputs["Strength"].default_value != 0 or sun.hide_render:
            raise RuntimeError("runtime selection is wrong")
        raise ValueError("injected bake failure")
except ValueError:
    pass
result["emission_restored"] = background.inputs["Strength"].default_value == .25
print("SEPARATION " + json.dumps(result))
'''


class ProbeLightSeparation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        blender = shutil.which("blender")
        if not blender:
            raise unittest.SkipTest("probe separation requires Blender")
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "separation.py"
            path.write_text(SCRIPT)
            env = dict(os.environ, OCIO=str(ROOT / "quality/fixtures/staircase2-ocio/config.ocio"),
                       OMP_NUM_THREADS="1", OPENBLAS_NUM_THREADS="1")
            run = subprocess.run([blender, "-b", "--factory-startup", "--python-exit-code", "9",
                                  "--python", str(path), "--", str(ROOT / "tools/quality"), tmp],
                                 env=env, capture_output=True, text=True, timeout=120)
            lines = [line for line in run.stdout.splitlines() if line.startswith("SEPARATION ")]
            if run.returncode or len(lines) != 1:
                raise RuntimeError((run.stdout + run.stderr)[-4000:])
            cls.result = json.loads(lines[0][len("SEPARATION "):])

    def test_sky_direct_is_retained(self):
        for value in self.result["sky"]["indirect"]:
            self.assertAlmostEqual(value, .25, delta=.01)

    def test_runtime_direct_is_excluded(self):
        self.assertTrue(all(value > .1 for value in self.result["runtime"]["total"]))
        self.assertTrue(all(abs(value) < 1e-6 for value in self.result["runtime"]["indirect"]))

    def test_mixed_light_is_not_counted_twice(self):
        for total, indirect in zip(self.result["mixed"]["total"],
                                   self.result["mixed"]["indirect"]):
            self.assertGreater(total, indirect + .1)
            self.assertAlmostEqual(indirect, .25, delta=.01)

    def test_failure_restores_scene(self):
        self.assertTrue(self.result["visibility_restored"])
        self.assertTrue(self.result["emission_restored"])
