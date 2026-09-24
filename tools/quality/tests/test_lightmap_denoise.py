"""Lightmap gutter and EXR channel regression fixtures."""

import sys
import tempfile
import unittest
from pathlib import Path

import imageio.v3 as iio
import numpy as np
from scipy import ndimage

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lightmap_denoise import extend_gutters, repair_narrow_dropouts, write_linear_exr


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

    def test_narrow_dropout_repair_keeps_wide_dark_region(self):
        pixels = np.full((32, 32, 3), 0.8, dtype=np.float32)
        pixels[4:15, 7:9] = 0.0
        pixels[18:29, 18:29] = 0.0
        repaired, count = repair_narrow_dropouts(pixels)
        self.assertGreater(count, 0)
        self.assertGreater(repaired[10, 7, 0], 0.7)
        self.assertEqual(repaired[23, 23, 0], 0.0)
        np.testing.assert_array_equal(repaired[0, 0], pixels[0, 0])


if __name__ == "__main__":
    unittest.main()
