"""Lightmap gutter and EXR channel regression fixtures."""

import sys
import tempfile
import unittest
from pathlib import Path

import imageio.v3 as iio
import numpy as np
from scipy import ndimage

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lightmap_denoise as denoise  # noqa: E402
from lightmap_denoise import extend_gutters, uv_coverage_mask, write_linear_exr  # noqa: E402


class LightmapDenoiseTest(unittest.TestCase):
    def test_gutter_uses_nearest_covered_color(self):
        covered = np.zeros((5, 5), dtype=bool)
        covered[2, 1] = True
        covered[2, 3] = True
        color = np.zeros((5, 5, 3), dtype=np.float32)
        color[2, 1] = (0.25, 0.5, 1.0)
        color[2, 3] = (2.0, 1.0, 0.5)
        _, (rows, columns) = ndimage.distance_transform_edt(
            ~covered, return_indices=True)
        result = extend_gutters(color, covered, rows, columns)
        np.testing.assert_array_equal(result[2, 0], color[2, 1])
        np.testing.assert_array_equal(result[2, 4], color[2, 3])
        np.testing.assert_array_equal(result[covered], color[covered])

    def test_linear_exr_roundtrip_preserves_hdr_and_alpha(self):
        pixels = np.zeros((8, 8, 4), dtype=np.float32)
        pixels[:, :, :3] = (0.25, 1.5, 4.25)
        pixels[:, :, 3] = 1.0
        pixels[0, 0, 3] = 0.0
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "lightmap.exr"
            write_linear_exr(path, pixels)
            np.testing.assert_array_equal(iio.imread(path), pixels)

    def test_white_footprint_rejects_black_gutter_with_alpha_one(self):
        pixels = np.ones((3, 3, 4), dtype=np.float32)
        pixels[1, 1, :3] = 0.0
        coverage = np.ones_like(pixels)
        coverage[1, 1, :3] = 0.0
        covered = uv_coverage_mask(coverage, pixels)
        self.assertFalse(covered[1, 1])
        self.assertTrue(covered[0, 0])
        _, (rows, columns) = ndimage.distance_transform_edt(
            ~covered, return_indices=True)
        result = extend_gutters(pixels[:, :, :3], covered, rows, columns)
        np.testing.assert_array_equal(result[1, 1], (1.0, 1.0, 1.0))


if __name__ == "__main__":
    unittest.main()


def _oidn():
    try:
        return denoise.load_oidn("libOpenImageDenoise.so.2")
    except OSError:
        return None


@unittest.skipIf(_oidn() is None, "OpenImageDenoise is not installed")
class PerChartDenoiseTest(unittest.TestCase):
    """Each chart is denoised from its own light only."""

    def setUp(self):
        self.denoiser = denoise.Denoiser(_oidn())
        rng = np.random.default_rng(4)
        size = 96
        self.covered = np.zeros((size, size), bool)
        self.covered[8:40, 8:40] = True       # chart A
        self.covered[8:40, 44:88] = True      # chart B, 4 texels from A
        self.covered[48:90, 8:88] = True      # chart C
        light = np.where(self.covered, 0.4, 0.0)[..., None] * np.ones(3)
        self.color = (light * (1 + 0.3 * rng.standard_normal(light.shape))).astype(np.float32)
        self.color = np.maximum(self.color, 0)

    def tearDown(self):
        self.denoiser.close()

    def test_other_charts_cannot_change_a_chart(self):
        out, record = denoise.denoise_charts(self.color, self.covered, self.denoiser,
                                             input_scale=2.5)
        self.assertEqual(record["charts"], 3)
        louder = self.color.copy()
        louder[self.covered & (np.arange(96)[None, :] >= 44)] *= 10   # B and C's right
        again, _ = denoise.denoise_charts(louder, self.covered, self.denoiser, input_scale=2.5)
        chart_a = np.zeros_like(self.covered)
        chart_a[8:40, 8:40] = True
        np.testing.assert_array_equal(out[chart_a], again[chart_a])

    def test_whole_atlas_filter_leaks_across_charts(self):
        """Negative control: filtering the atlas in one piece lets chart B's
        light into chart A, so the isolation test above can fail."""
        chart_a = np.zeros_like(self.covered)
        chart_a[8:40, 8:40] = True
        louder = self.color.copy()
        louder[self.covered & ~chart_a] *= 10
        whole = self.denoiser.run(self.color, 2.5)
        again = self.denoiser.run(louder, 2.5)
        self.assertGreater(np.abs(whole[chart_a] - again[chart_a]).max(), 1e-4)

    def test_same_input_same_bytes_and_gutters_untouched(self):
        a, _ = denoise.denoise_charts(self.color, self.covered, self.denoiser, input_scale=2.5)
        b, _ = denoise.denoise_charts(self.color.copy(), self.covered, self.denoiser,
                                      input_scale=2.5)
        self.assertEqual(a.tobytes(), b.tobytes())
        np.testing.assert_array_equal(a[~self.covered], self.color[~self.covered])

    def test_noise_falls_and_light_level_holds(self):
        out, _ = denoise.denoise_charts(self.color, self.covered, self.denoiser)
        before, after = self.color[self.covered], out[self.covered]
        self.assertLess(after.std(), 0.5 * before.std())
        self.assertAlmostEqual(after.mean() / before.mean(), 1.0, delta=0.03)
