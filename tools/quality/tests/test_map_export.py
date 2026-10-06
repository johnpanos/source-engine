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
import reflection_probe_set  # noqa: E402
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


class IndirectLayerFitTest(unittest.TestCase):
    """`lightmap_directional.py --layer indirect`: the indirect layer's own
    gradient, fitted to the indirect layer's RNM bakes (rnm_indirect<i>)."""

    SIZE = 8

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        size = self.SIZE
        self.normal = np.zeros((size, size, 3))
        self.normal[..., 2] = 1.0
        self.tangent = np.zeros((size, size, 3))
        self.tangent[..., 0] = 1.0
        bitangent = np.cross(self.normal, self.tangent)
        # Indirect light from the side (+y), direct light from above and +x.
        self.indirect = (0.3, (0.0, 0.12, 0.05))
        self.direct = (1.5, (0.9, 0.0, 0.6))
        self.directions = [basis[0] * self.tangent + basis[1] * bitangent +
                           basis[2] * self.normal for basis in RNM]

    def tearDown(self):
        self.tmp.cleanup()

    def write(self, name, rgb):
        import lightmap_denoise
        pixels = np.ones((self.SIZE, self.SIZE, 4), dtype=np.float32)
        pixels[..., :3] = rgb
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        lightmap_denoise.write_linear_exr(path, pixels)
        return path

    def run_fit(self, stem="rnm_indirect", layer="indirect"):
        sha = lightmap_directional.sha256
        bakes = self.root / "directional"
        # The indirect layer's light at each basis normal (or, for the
        # negative control, the total's under its own stem).
        light = (lambda d: linear_light(*self.indirect, d)) if stem == "rnm_indirect" else \
            (lambda d: linear_light(*self.indirect, d) + linear_light(*self.direct, d))
        rnm = [self.write("directional/%s%d.exr" % (stem, i), light(d))
               for i, d in enumerate(self.directions)]
        frame = {"t": self.write("directional/frame_t.exr", 0.5 + 0.5 * self.tangent),
                 "n": self.write("directional/frame_n.exr", 0.5 + 0.5 * self.normal)}
        coverage = self.write("coverage.exr", np.ones((self.SIZE, self.SIZE, 3)))
        indirect = self.write("layers/indirect.exr", linear_light(*self.indirect, self.normal))
        directional = {"rnm_exr_sha256": [sha(p) for p in rnm], "stem": stem,
                       "frame_exr_sha256": {k: sha(v) for k, v in frame.items()}}
        bake = self.root / "atlas.exr.json"
        bake.write_text(json.dumps({"status": "pass", "size": self.SIZE,
                                    "atlas_exr_sha256": "total",
                                    "coverage_exr_sha256": sha(coverage),
                                    "layers": {"indirect": {"exr_sha256": sha(indirect)}},
                                    "directional": directional}))
        receipt = self.root / "layers/indirect.exr.json"
        receipt.write_text(json.dumps({"status": "pass", "layer": layer,
                                       "atlas_exr_sha256": sha(indirect),
                                       "source_atlas_exr_sha256": sha(indirect)}))
        out = self.root / "indirect-directional.exr"
        argv = ["lightmap_directional.py", "--layer", "indirect", "--flat-exr", str(indirect),
                "--flat-evidence", str(receipt), "--bake-evidence", str(bake),
                "--directional-dir", str(bakes), "--coverage-exr", str(coverage),
                "--skip-denoise", "--out", str(out)]
        saved = sys.argv
        sys.argv = argv
        try:
            lightmap_directional.main()
        finally:
            sys.argv = saved
        return lightmap_directional.read_rgba(out)[..., :3], json.loads(
            out.with_name(out.name + ".json").read_text())

    def test_the_indirect_gradient_is_recovered(self):
        beta, receipt = self.run_fit()
        a, g = self.indirect
        expected = np.asarray(g) / (a + g[2])
        np.testing.assert_allclose(beta, np.broadcast_to(expected, beta.shape), atol=1e-4)
        self.assertEqual((receipt["layer"], receipt["rnm_stem"]), ("indirect", "rnm_indirect"))

    def test_it_is_not_the_totals_gradient(self):
        # Negative control: the total's beta (what the indirect page borrowed
        # before it had its own) is far from the indirect light's.
        beta, _ = self.run_fit()
        (a_i, g_i), (a_d, g_d) = self.indirect, self.direct
        total_g = np.asarray(g_i) + np.asarray(g_d)
        total_beta = total_g / (a_i + a_d + total_g[2])
        self.assertGreater(float(np.abs(beta[0, 0] - total_beta).max()), 0.2)

    def test_the_totals_rnm_bakes_are_refused_for_the_indirect_layer(self):
        with self.assertRaisesRegex(ValueError, "not the indirect page's"):
            self.run_fit(stem="rnm")

    def test_a_total_receipt_is_refused_as_the_indirect_layer(self):
        with self.assertRaisesRegex(ValueError, "flat indirect page differs"):
            self.run_fit(layer="total")


class ProbePrefilterTest(unittest.TestCase):
    def solid_angle_weights(self, image):
        # Cube texels differ in solid angle by at most 3x; weight by it.
        size = image.shape[1]
        s = (np.arange(size) + 0.5) / size * 2 - 1
        x, y = np.meshgrid(s, s)
        return (1.0 / (1.0 + x * x + y * y) ** 1.5)[None]

    def test_constant_environment_is_preserved(self):
        chain = reflection_probe.cube_mip_chain(np.full((6, 32, 32, 3), 0.7), samples=64)
        for mip in chain:
            np.testing.assert_allclose(mip, 0.7, atol=1e-9)

    def test_bright_spot_spreads_and_keeps_energy(self):
        environment = np.zeros((6, 32, 32, 3))
        environment[0, 15:17, 15:17] = 50.0
        chain = reflection_probe.cube_mip_chain(environment, samples=1024)
        energies = [float((mip[..., 0] * self.solid_angle_weights(mip)).sum() /
                          (6 * self.solid_angle_weights(mip).sum())) for mip in chain]
        peaks = [float(mip.max()) for mip in chain]
        for energy in energies[1:4]:
            self.assertAlmostEqual(energy / energies[0], 1.0, delta=0.15)
        self.assertTrue(all(later < earlier for earlier, later in zip(peaks, peaks[1:4])))

    def test_box_filter_is_not_a_ggx_prefilter(self):
        # Negative control: the former box mips keep a small emitter far
        # brighter at roughness 1/7 than the GGX lobe does.
        environment = np.zeros((6, 64, 64, 3))
        environment[0, 31:33, 31:33] = 100.0
        ggx = reflection_probe.cube_mip_chain(environment, samples=128)[1]
        box = reflection_probe.cube_pyramid(environment, 4)[1]
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
            "lighting/atlas.ktx2.json": {"layout": "directional-2x1"},
            "lighting/reflection_probes.rprb.json": {
                "status": "pass", "probes": 2, "width": 512, "max_mean_relative_residual": 0.08,
                "candidate_grid": {"dimensions": [reflection_probe_set.CANDIDATE_DIM] * 3,
                                   "bytes": reflection_probe_set.candidate_bytes(2),
                                   "max_candidates": 2},
                "fits": [{"index": 0, "role": "room", "mean_relative_residual": 0.08},
                         {"index": 1, "role": "glossy", "mean_relative_residual": 0.03}],
                "placement": {"walkable_samples": 120, "uncovered_walkable": 0,
                              "glossy_samples": 40, "glossy_servable": 30,
                              "unserved_glossy": 2, "room_stop": "covered",
                              "glossy_stop": "min_area", "max_probes": 8}},
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
        result = self.audit(**{"lighting/atlas.ktx2.json": {"layout": "flat"}})
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
                               "lighting/reflection_probes.rprb.json": None})
        self.assertIn("lightmap-samples", result["failed"])
        self.assertIn("reflection-probe", result["failed"])
        self.assertIn("reflection-candidates", result["failed"])
        self.assertIn("reflection-probe-fit", result["failed"])

    def test_badly_fitted_or_sparse_probes_fail(self):
        receipt = {"status": "pass", "probes": 1, "width": 512,
                   "max_mean_relative_residual": 3.16,
                   "fits": [{"index": 0, "role": "room", "mean_relative_residual": 3.16}],
                   "placement": {"walkable_samples": 100, "uncovered_walkable": 30,
                                 "glossy_samples": 10, "glossy_servable": 10,
                                 "unserved_glossy": 8, "room_stop": "max_probes",
                                 "glossy_stop": "max_probes", "max_probes": 1}}
        result = self.audit(**{"lighting/reflection_probes.rprb.json": receipt})
        for check in ("reflection-probe-fit", "reflection-probe-coverage",
                      "reflection-probe-glossy", "reflection-probe-budget"):
            self.assertIn(check, result["failed"])

    def test_pre_capture_checks_match_final_audit(self):
        placement = {"probes": 16, "max_probes": 16, "walkable_samples": 1773,
                     "uncovered_walkable": 744, "glossy_samples": 14723,
                     "glossy_servable": 2931, "unserved_glossy": 2442,
                     "room_stop": "max_probes", "glossy_stop": "max_probes"}
        before = map_export_audit.probe_placement_checks(placement, self.PROFILE["audit"])
        result = self.audit(**{"lighting/reflection_probes.rprb.json": {
            "status": "pass", "probes": 16, "placement": placement}})
        names = {check["check"] for check in before}
        after = [check for check in result["checks"] if check["check"] in names]
        self.assertEqual(before, after)
        self.assertEqual(len(before), 3)
        self.assertTrue(all(check["status"] == "fail" for check in before))

    def test_production_does_not_consult_reference_gate(self):
        result = self.audit(**{"camera-boot/gate.json": {"status": "fail"}})
        self.assertNotIn("runtime-gate", result["failed"])


if __name__ == "__main__":
    unittest.main()
