"""Oracles for the scene-map export path: directional lightmap fit, GGX probe
prefilter, dome-light orientation and the export-quality audit.

    python3 -m unittest tools.quality.tests.test_map_export -v
"""

import json
import math
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lightmap_directional  # noqa: E402
import map_export_audit  # noqa: E402
import map_scene  # noqa: E402
import reflection_probe  # noqa: E402

RNM = ((0.816496580927726, 0.0, 0.5773502691896258),
       (-0.408248290463863, 0.7071067811865475, 0.5773502691896258),
       (-0.408248290463863, -0.7071067811865475, 0.5773502691896258))


def linear_light(a, g, directions):
    """E(n) = a + g.n for (H, W, 3) directions, as grey RGB."""
    values = a + directions @ np.asarray(g)
    return np.repeat(values[..., None], 3, axis=2)


class DirectionalFitTest(unittest.TestCase):
    def setUp(self):
        rng = np.random.default_rng(3)
        normal = rng.normal(size=(8, 8, 3))
        normal[..., 2] = np.abs(normal[..., 2]) + 0.5
        self.normal = normal / np.linalg.norm(normal, axis=2, keepdims=True)
        tangent = np.cross(np.array((0.3, 1.0, 0.2)), self.normal)
        self.tangent = tangent / np.linalg.norm(tangent, axis=2, keepdims=True)
        self.bitangent = np.cross(self.normal, self.tangent)
        self.a, self.g = 0.8, (0.2, -0.1, 0.3)

    def bakes(self):
        flat = linear_light(self.a, self.g, self.normal)
        rnm = []
        for basis in RNM:
            direction = (basis[0] * self.tangent + basis[1] * self.bitangent +
                         basis[2] * self.normal)
            rnm.append(linear_light(self.a, self.g, direction))
        return flat, rnm

    def test_linear_irradiance_is_recovered_exactly(self):
        flat, rnm = self.bakes()
        beta, residual, clamped, valid = lightmap_directional.fit(
            flat, rnm, self.tangent, self.normal)
        expected = np.asarray(self.g) / flat[..., :1]
        self.assertTrue(valid.all())
        self.assertFalse(clamped.any())
        np.testing.assert_allclose(beta, expected, atol=1e-9)
        self.assertLess(float(residual.max()), 1e-9)

    def test_unperturbed_normal_keeps_flat_irradiance(self):
        flat, rnm = self.bakes()
        beta, _, _, _ = lightmap_directional.fit(flat, rnm, self.tangent, self.normal)
        gain = 1.0 + np.sum(beta * (self.normal - self.normal), axis=2)
        np.testing.assert_allclose(gain, 1.0)

    def test_swapped_rnm_bakes_are_detected(self):
        flat, rnm = self.bakes()
        beta, residual, _, _ = lightmap_directional.fit(
            flat, [rnm[1], rnm[0], rnm[2]], self.tangent, self.normal)
        self.assertGreater(float(np.abs(beta - np.asarray(self.g) / flat[..., :1]).max()), 0.05)

    def test_mirrored_frame_is_detected(self):
        flat, rnm = self.bakes()
        beta, _, _, _ = lightmap_directional.fit(flat, rnm, -self.tangent, self.normal)
        self.assertGreater(float(np.abs(beta - np.asarray(self.g) / flat[..., :1]).max()), 0.05)


class ProbePrefilterTest(unittest.TestCase):
    def solid_angle_weights(self, image):
        height = image.shape[0]
        latitude = math.pi / 2 - (np.arange(height) + 0.5) / height * math.pi
        return np.cos(latitude)[:, None]

    def test_constant_environment_is_preserved(self):
        chain = reflection_probe.mip_chain(np.full((64, 128, 3), 0.7), samples=64)
        for mip in chain:
            np.testing.assert_allclose(mip, 0.7, atol=1e-9)

    def test_bright_spot_spreads_and_keeps_energy(self):
        environment = np.zeros((64, 128, 3))
        environment[30:32, 40:42] = 50.0
        chain = reflection_probe.mip_chain(environment, samples=128)
        energies = [float((mip[..., 0] * self.solid_angle_weights(mip)).sum() /
                          (mip.shape[0] * mip.shape[1])) for mip in chain]
        peaks = [float(mip.max()) for mip in chain]
        for energy in energies[1:4]:
            self.assertAlmostEqual(energy / energies[0], 1.0, delta=0.15)
        self.assertTrue(all(later < earlier for earlier, later in zip(peaks, peaks[1:4])))

    def test_box_filter_is_not_a_ggx_prefilter(self):
        # Negative control: the former box mips keep a small emitter far
        # brighter at roughness 1/7 than the GGX lobe does.
        environment = np.zeros((128, 256, 3))
        environment[60:62, 100:102] = 100.0
        ggx = reflection_probe.mip_chain(environment, samples=128)[1]
        box = reflection_probe.box_pyramid(environment, 4)[1]
        self.assertGreater(float(box.max()), 2.0 * float(ggx.max()))


class DomeOrientationTest(unittest.TestCase):
    def test_openexr_latlong_axes(self):
        width, height = 400, 200
        local = np.array(((0.0, 0.0, 1.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0)))
        x, y = map_scene.dome_texel_coordinates(local, "Y", width, height)
        # +Z: longitude 0 (image center); +X: longitude pi/2 (a quarter in); +Y: top row.
        np.testing.assert_allclose(x[:2], (width / 2, width / 4))
        np.testing.assert_allclose(y[:2], (height / 2, height / 2))
        self.assertAlmostEqual(float(y[2]), 0.0)

    def test_z_pole_longitude_zero_is_minus_y(self):
        x, y = map_scene.dome_texel_coordinates(np.array(((0.0, -1.0, 0.0),)), "Z", 400, 200)
        self.assertAlmostEqual(float(x[0]), 200.0)
        self.assertAlmostEqual(float(y[0]), 100.0)

    def test_constant_dome(self):
        scene = {"format": "usd", "environment": {"kind": "constant",
                                                  "radiance": [0.1, 0.2, 0.3]}}
        pixels = map_scene.environment_equirect(scene, 64)
        self.assertEqual(pixels.shape, (32, 64, 3))
        np.testing.assert_allclose(pixels[5, 7], (0.1, 0.2, 0.3), rtol=1e-6)


class AuditTest(unittest.TestCase):
    PROFILE = json.loads((Path(__file__).resolve().parents[3] /
                          "quality/map_export_profiles/source2.json").read_text())

    def build(self, directory, **overrides):
        lighting = directory / "lighting"
        lighting.mkdir(parents=True, exist_ok=True)
        receipts = {
            "lighting/atlas.exr.json": {"samples": 2048, "directional": {"basis": []}},
            "lighting/atlas-denoised.exr.json": {"denoiser": "OpenImageDenoise"},
            "lighting/atlas-directional.exr.json": {"status": "pass"},
            "lighting/atlas.ktx2.json": {"layout": "directional-2x1",
                                         "reflection_probe": {"mips": 8}},
            "content.json": {"materials": {"wood": {
                "authored_channels": ["base", "normal", "roughness"],
                "exported_channels": {"base": "texture", "normal": "texture",
                                      "roughness": "texture", "emission": None},
                "encoded_dimensions": [1024, 1024]}}},
            "map.wmsh.json": {"source_meshes": [{"triangles": 100, "normal_fallbacks": 0}]},
            "camera-boot/gate.json": {"status": "pass"},
        }
        receipts.update(overrides)
        for name, value in receipts.items():
            path = directory / name
            path.parent.mkdir(parents=True, exist_ok=True)
            if value is not None:
                path.write_text(json.dumps(value))

    def audit(self, **overrides):
        with tempfile.TemporaryDirectory() as name:
            self.build(Path(name), **overrides)
            return map_export_audit.audit(Path(name), self.PROFILE, booted=True)

    def test_complete_export_passes(self):
        self.assertEqual(self.audit()["status"], "pass")

    def test_missing_directional_fails(self):
        result = self.audit(**{"lighting/atlas.ktx2.json": {"layout": "flat",
                                                           "reflection_probe": {"mips": 8}}})
        self.assertIn("directional-lightmap", result["failed"])

    def test_dropped_normal_map_fails(self):
        content = {"materials": {"wood": {
            "authored_channels": ["base", "normal"],
            "exported_channels": {"base": "texture", "normal": "none"},
            "encoded_dimensions": [512, 512]}}}
        self.assertIn("authored-channels", self.audit(**{"content.json": content})["failed"])

    def test_low_samples_and_missing_probe_fail(self):
        result = self.audit(**{"lighting/atlas.exr.json": {"samples": 64,
                                                           "directional": {"basis": []}},
                               "lighting/atlas.ktx2.json": {"layout": "directional-2x1",
                                                            "reflection_probe": None}})
        self.assertIn("lightmap-samples", result["failed"])
        self.assertIn("reflection-probe", result["failed"])

    def test_failed_runtime_gate_fails(self):
        result = self.audit(**{"camera-boot/gate.json": {"status": "fail"}})
        self.assertIn("runtime-gate", result["failed"])


if __name__ == "__main__":
    unittest.main()
