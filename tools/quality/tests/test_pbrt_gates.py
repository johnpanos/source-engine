"""Sensitivity tests for the PBRT map pipeline's pixel, noise and traversal gates."""

from pathlib import Path
import sys
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import pbrt_traversal
import reference_compare


def smooth_frame(seed=0):
    """A 256x256 frame: a flat wall, a lit gradient and one hard edge."""
    y, x = np.mgrid[0:256, 0:256].astype(np.float64)
    luminance = 80 + 0.3 * x + 0.2 * y
    luminance[:, :100] = 120.0
    luminance[:, 200:] += 60
    return np.repeat(luminance[:, :, None], 3, axis=2)


class NoiseGateTest(unittest.TestCase):
    def setUp(self):
        self.reference = smooth_frame()
        self.rng = np.random.default_rng(1)

    def test_identical_frame_has_unit_ratios(self):
        result = reference_compare.grain(self.reference, self.reference.copy())
        self.assertGreater(result["smooth_pixels"], 1000)
        self.assertAlmostEqual(result["ratio"], 1.0, places=6)
        self.assertAlmostEqual(result["mottle_ratio"], 1.0, places=6)

    def test_fine_grain_is_detected(self):
        noisy = self.reference + self.rng.normal(0, 4, self.reference.shape)
        self.assertGreater(reference_compare.grain(self.reference, noisy)["ratio"], 5)

    def test_blotches_raise_mottle_not_just_grain(self):
        from scipy.ndimage import gaussian_filter
        blotches = gaussian_filter(self.rng.normal(0, 40, self.reference.shape[:2]), 4)
        mottled = self.reference + blotches[:, :, None]
        result = reference_compare.grain(self.reference, mottled)
        self.assertGreater(result["mottle_ratio"], 3)

    def test_negative_controls_reject_black_and_mirror(self):
        gate = {"max_mae": 10, "min_ssim": 0.9}
        controls = reference_compare.negative_controls(self.reference, self.reference, gate)
        self.assertTrue(all(control["rejected"] for control in controls.values()))


class TraversalTest(unittest.TestCase):
    RECEIPT = {"walkable_tops": [{"x": 10.0, "y": 20.0, "z": 100.0, "brush": 1},
                                 {"x": -5.0, "y": 0.0, "z": 50.0, "brush": 2}],
               "spawn": {"origin": [0, 0, 4], "floor_z": 0.0}}

    def console(self, heights):
        probes = pbrt_traversal.probes(self.RECEIPT)
        lines = ["setpos 0.000000 0.000000 64.000000;setang 0 0 0"]
        lines += ["setpos %.6f %.6f %.6f;setang 0 0 0" % (probe["x"], probe["y"], feet + 64)
                  for probe, feet in zip(probes, heights)]
        return "\n".join(lines) + "\n"

    def test_resting_on_every_top_passes(self):
        result = pbrt_traversal.evaluate(self.console([100.03, 50.0, 0.03]), self.RECEIPT)
        self.assertEqual(result["status"], "pass")

    def test_falling_through_a_top_fails(self):
        result = pbrt_traversal.evaluate(self.console([0.03, 50.0, 0.03]), self.RECEIPT)
        self.assertEqual(result["status"], "fail")

    def test_missing_readings_fail(self):
        result = pbrt_traversal.evaluate(self.console([100.0]), self.RECEIPT)
        self.assertEqual(result["status"], "fail")

    def test_probe_line_fits_the_frame_budget(self):
        commands = pbrt_traversal.commands(self.RECEIPT)
        self.assertEqual(len(commands), 1)
        self.assertIn("wait %d" % pbrt_traversal.SETTLE_FRAMES, commands[0])


if __name__ == "__main__":
    unittest.main()


class ReflectionProbeTest(unittest.TestCase):
    def faces(self, size=32):
        """Each face texel stores its own world direction."""
        import reflection_probe
        result = {}
        for name in reflection_probe.FACES:
            forward, up, right = reflection_probe.face_basis(name)
            t = (np.arange(size) + 0.5) / size * 2 - 1
            x, y = np.meshgrid(t, -t)
            directions = forward + x[..., None] * right + y[..., None] * up
            result[name] = directions / np.linalg.norm(directions, axis=2, keepdims=True)
        return result

    def test_cube_faces_resample_to_their_directions(self):
        import reflection_probe
        equirect = reflection_probe.cube_to_equirect(self.faces(), 128)
        expected = reflection_probe.equirect_directions(128)
        cosine = np.sum(equirect * expected, axis=2) / np.linalg.norm(equirect, axis=2)
        self.assertGreater(cosine.min(), np.cos(np.radians(4)))

    def test_band_layout_matches_the_shader(self):
        import reflection_probe
        mips = reflection_probe.mip_chain(np.ones((64, 128, 3)))
        atlas = np.zeros((512, 512, 4))
        layout = reflection_probe.write_band(atlas, mips)
        self.assertEqual(tuple(atlas[0, 511]), (len(mips), 128, 64, -1.0))
        # world_pbr.frag: mip `level` starts at x = 2 * W0 * (1 - 2^-level).
        x = 0
        for level, mip in enumerate(mips):
            self.assertEqual(x, 2 * 128 * (1 - 2.0 ** -level))
            x += mip.shape[1]
        self.assertEqual(layout["band_rows"], 64)
        self.assertTrue(np.all(atlas[64:, :, 3] == 0))

    def test_chain_too_wide_for_atlas_fails(self):
        import reflection_probe
        with self.assertRaises(ValueError):
            reflection_probe.write_band(np.zeros((256, 200, 4)),
                                        reflection_probe.mip_chain(np.ones((64, 128, 3))))
