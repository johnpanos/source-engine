"""Oracles for the ArmorPaint remaster pipeline, with deliberately bad inputs."""

import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import armorpaint_remaster as ar  # noqa: E402

RECIPE_DIR = Path(__file__).resolve().parents[1] / "recipes"
SIZE = 64


def pillow_height(size=SIZE, tiles=4):
    """A periodic field of raised pillows, like metalwall048b."""
    u = (np.arange(size) % (size // tiles)) / (size // tiles) - 0.5
    return 3.0 * (0.5 - np.maximum(np.abs(u)[None, :], np.abs(u)[:, None]))


def normal_from_height(h, green_sign):
    gx, grow = ar.gradients(h)
    # OpenGL (+Y up) stores -dh/dy_up = +dh/drow; DirectX stores -dh/drow.
    n = np.dstack([-gx, green_sign * grow, np.ones_like(h)])
    return n / np.linalg.norm(n, axis=-1, keepdims=True)


def base_recipe():
    return ar.load_recipe(RECIPE_DIR / "metal/metalwall048b.json")


class RecipeTest(unittest.TestCase):
    def test_checked_in_recipes_are_valid(self):
        paths = sorted(RECIPE_DIR.glob("**/*.json"))
        self.assertGreaterEqual(len(paths), 5)
        for path in paths:
            recipe = ar.load_recipe(path)
            self.assertEqual(path.relative_to(RECIPE_DIR).with_suffix("").as_posix(), recipe["material"])

    def test_bad_recipes_are_rejected(self):
        good = json.loads((RECIPE_DIR / "metal/metalwall048b.json").read_text())
        mutations = [
            lambda r: r.update(size=3000),
            lambda r: r.update(material="../escape"),
            lambda r: r["metal"].update(constant=0.0),              # both metal modes
            lambda r: r.update(cavity={"source": "none"}),         # cavity_range without cavity
            lambda r: r["roughness"].pop("from_spec"),              # no roughness mode
            lambda r: r["roughness"].update(seam_range=[0.9, 0.1]),
            lambda r: r["delight"].update(strength=2.0),
            lambda r: r.pop("occlusion"),
            lambda r: r.pop("tiling"),
            lambda r: r["roughness"].update(from_exponent={}),     # two roughness modes
            lambda r: r.update(tiling=False),                      # atlas cannot be de-lit
            lambda r: r.update(emission={"source": "glow", "scale": 1}),
        ]
        for index, mutate in enumerate(mutations):
            recipe = copy.deepcopy(good)
            mutate(recipe)
            with tempfile.TemporaryDirectory() as tmp, self.subTest(mutation=index):
                path = Path(tmp) / "bad.json"
                path.write_text(json.dumps(recipe))
                with self.assertRaises(ValueError):
                    ar.load_recipe(path)


class NormalConventionTest(unittest.TestCase):
    def test_integrability_identifies_each_convention(self):
        h = pillow_height()
        gl = ar.convention_residuals(normal_from_height(h, +1))
        dx = ar.convention_residuals(normal_from_height(h, -1))
        self.assertLess(gl["opengl"], gl["directx"])
        self.assertLess(dx["directx"], dx["opengl"])

    def test_ssbump_decode_matches_basis(self):
        # Equal SSBump weights are the flat surface.
        n = ar.decode_normal(np.full((4, 4, 3), 0.5), True)
        np.testing.assert_allclose(n[..., :2], 0, atol=1e-9)
        self.assertTrue(np.all(n[..., 2] > 0.999))


class SeamTest(unittest.TestCase):
    def test_periodic_texture_passes_and_shifted_edge_fails(self):
        periodic = (pillow_height() - pillow_height().min())[..., None] * np.ones(3)
        self.assertLessEqual(ar.seam_ratio(periodic), 1.0)
        broken = periodic.copy()
        broken[:, -1] += 5.0
        self.assertGreater(ar.seam_ratio(broken), 1.0)


class ConversionTest(unittest.TestCase):
    def test_tileable_noise_is_periodic(self):
        noise = ar.tileable_noise(128, 12, "fixture")
        self.assertLessEqual(ar.seam_ratio(noise), 1.0)
        self.assertAlmostEqual(float(noise.mean()), 0.5, delta=0.05)
        self.assertGreater(float(noise.std()), 0.05)

    def test_exponent_roughness_follows_the_phong_decode(self):
        rough = ar.exponent_roughness(np.array([0.0, 0.5, 1.0]))
        self.assertTrue(np.all(np.diff(rough) < 0))                 # sharper highlight, smoother
        # n = 1 + 149 * 0.5 = 75.5; alpha = sqrt(2 / (4n + 2)); perceptual = sqrt(alpha)
        self.assertAlmostEqual(float(rough[1]), (2 / (4 * 75.5 + 2)) ** 0.25, places=9)


class GuidedUpsampleTest(unittest.TestCase):
    def test_blocky_diagonal_edge_follows_the_sharp_guide(self):
        size = 128
        y, x = np.mgrid[0:size, 0:size]
        guide = (x + 0.5 * y > 80).astype(float)                   # sharp full-resolution edge
        cy, cx = np.mgrid[0:8, 0:8] * 16 + 8
        coarse = (cx + 0.5 * cy > 80).astype(float)                # the same edge on a 16x grid
        plain = ar.upscale_float(coarse, size, mode="edge")
        guided = ar.guided_upsample(coarse, guide, size, "edge")
        error = lambda m: float(np.abs(m - guide).mean())
        self.assertLess(error(guided), 0.6 * error(plain))            # measured 0.55x


class GraphTest(unittest.TestCase):
    def test_references_resolve_and_outputs_are_wired(self):
        for path in sorted(RECIPE_DIR.glob("**/*.json")):
            calls = ar.graph_calls(ar.load_recipe(path))
            saved = {c["save"] for c in calls if "save" in c}

            def refs(value):
                if isinstance(value, dict):
                    if set(value) == {"$ref"}:
                        yield value["$ref"]
                    for item in value.values():
                        yield from refs(item)
            for call in calls:
                for ref in refs(call.get("args", {})):
                    self.assertIn(ref, saved, path.name)
            wired = {c["args"]["to_socket"] for c in calls
                     if c["tool"] == "ap_node_connect" and c["args"]["to_id"] == 0}
            self.assertEqual({0, 2, 3, 4, 5, 7}, wired, path.name)


class QaTest(unittest.TestCase):
    def write_set(self, tmp, normal_gl, mrao, size=SIZE, base=None):
        paths = {"basecolor": tmp / "basecolor.png", "mrao": tmp / "mrao.png", "normal": tmp / "normal.png"}
        ar.save8(paths["basecolor"], base if base is not None else np.full((size, size, 3), 0.5))
        ar.save8(paths["mrao"], mrao)
        ar.save8(paths["normal"], normal_gl * 0.5 + 0.5)
        return paths

    def setUp(self):
        self.recipe = dict(base_recipe(), size=SIZE)
        h = pillow_height()
        self.gl = normal_from_height(h, +1)
        self.dx = normal_from_height(h, -1)
        rng = np.random.default_rng(1)
        rough = 0.5 + 0.05 * np.sin(np.arange(SIZE) * 2 * np.pi / 16)[None, :] * np.ones((SIZE, 1))
        rough = rough + 0.01 * rng.standard_normal((SIZE, SIZE))
        self.mrao = np.dstack([np.ones((SIZE, SIZE)), rough, np.ones((SIZE, SIZE))])

    def run_qa(self, normal_gl, mrao=None, prepared_dx=None, size=SIZE):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            paths = self.write_set(tmp, normal_gl, self.mrao if mrao is None else mrao, size)
            prepared = tmp / "prepared-dx.png"
            ar.save8(prepared, (self.dx if prepared_dx is None else prepared_dx) * 0.5 + 0.5)
            return ar.qa(self.recipe, paths, prepared, {})["problems"]

    def test_good_set_passes(self):
        self.assertEqual([], self.run_qa(self.gl))

    def test_directx_export_is_rejected(self):
        problems = self.run_qa(self.dx)
        self.assertTrue(any("not OpenGL" in p for p in problems), problems)

    def test_tilted_normal_is_rejected(self):
        tilted = self.gl + np.array([0.23, 0.23, 0.0])
        tilted /= np.linalg.norm(tilted, axis=-1, keepdims=True)
        problems = self.run_qa(tilted)
        self.assertTrue(any("tilted" in p for p in problems), problems)

    def test_constant_roughness_is_rejected(self):
        flat = self.mrao.copy()
        flat[..., 1] = 0.55
        problems = self.run_qa(self.gl, flat)
        self.assertTrue(any("constant" in p for p in problems), problems)

    def test_roughness_floor_and_size_are_enforced(self):
        low = self.mrao.copy()
        low[..., 1] = np.clip(low[..., 1] - 0.5, 0, 1)
        self.assertTrue(any("0.02 floor" in p for p in self.run_qa(self.gl, low)))
        self.recipe["size"] = SIZE * 2
        self.assertTrue(any("expected" in p for p in self.run_qa(self.gl)))

    def test_seam_is_not_judged_on_an_atlas(self):
        broken = self.mrao.copy()
        broken[..., 1] = np.linspace(0.3, 0.8, SIZE)[None, :] * np.ones((SIZE, 1))
        self.recipe["tiling"] = False
        self.assertEqual([], self.run_qa(self.gl, broken))

    def test_black_emission_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            paths = self.write_set(tmp, self.gl, self.mrao)
            paths["emission"] = tmp / "emission.png"
            ar.save8(paths["emission"], np.zeros((SIZE, SIZE, 3)))
            prepared = tmp / "prepared-dx.png"
            ar.save8(prepared, self.dx * 0.5 + 0.5)
            problems = ar.qa(self.recipe, paths, prepared, {})["problems"]
        self.assertTrue(any("emission" in p for p in problems), problems)

    def test_seam_in_export_is_rejected(self):
        broken = self.mrao.copy()          # a ramp does not tile: small interior steps, one big wrap step
        broken[..., 1] = np.linspace(0.3, 0.8, SIZE)[None, :] * np.ones((SIZE, 1))
        problems = self.run_qa(self.gl, broken)
        self.assertTrue(any("wrap seam" in p for p in problems), problems)


if __name__ == "__main__":
    unittest.main()
