#!/usr/bin/env python3
"""The probe grid over a map's bounds, and its fit to the PRBV probe limit
(probe_volume.grid_for / fit_spacing; probe_volume_bake.py --fit-limit)."""

import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import probe_volume  # noqa: E402

# escape_02's world bounds in metres (the relit stage, 2026-09-29): 3.3M
# probes at the relight profiles' 2 m.
ESCAPE_02 = (np.array([-52.4, -106.9, -189.0]), np.array([273.5, 179.2, 97.9]))


def count(low, high, spacing):
    return int(np.prod(probe_volume.grid_for(low, high, spacing)[2]))


class GridTest(unittest.TestCase):
    def test_grid_spans_the_bounds_exactly(self):
        origin, step, dims = probe_volume.grid_for([0, 0, 0], [10, 4, 3], 2.0)
        self.assertEqual(dims.tolist(), [6, 3, 3])
        np.testing.assert_allclose(origin + step * (dims - 1), [10, 4, 3])
        self.assertTrue((step <= 2.0).all())

    def test_a_flat_axis_keeps_two_layers(self):
        _, step, dims = probe_volume.grid_for([0, 0, 0], [4, 4, 0], 2.0)
        self.assertEqual(dims.tolist(), [3, 3, 2])
        self.assertEqual(step[2], 2.0)


class FitTest(unittest.TestCase):
    def test_a_grid_that_fits_keeps_its_spacing(self):
        self.assertEqual(probe_volume.fit_spacing([0, 0, 0], [100, 60, 40], 2.0), 2.0)

    def test_a_large_map_widens_just_enough(self):
        low, high = ESCAPE_02
        self.assertGreater(count(low, high, 2.0), probe_volume.MAX_PROBES)
        fitted = probe_volume.fit_spacing(low, high, 2.0)
        self.assertLessEqual(count(low, high, fitted), probe_volume.MAX_PROBES)
        # Within 1% of the smallest spacing that fits.
        self.assertGreater(count(low, high, fitted / 1.01), probe_volume.MAX_PROBES)
        self.assertLess(fitted, 3.2)

    def test_the_limit_is_a_parameter(self):
        fitted = probe_volume.fit_spacing([0, 0, 0], [10, 10, 10], 1.0, limit=64)
        self.assertLessEqual(count([0, 0, 0], [10, 10, 10], fitted), 64)


if __name__ == "__main__":
    unittest.main()
