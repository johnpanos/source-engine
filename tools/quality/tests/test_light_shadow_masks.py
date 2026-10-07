"""LSMK (light_shadow_masks.py): grouping, per-texel selection and the encoding."""

import struct
import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import light_shadow_masks as lsmk  # noqa: E402


class GroupTests(unittest.TestCase):
    def test_a_group_has_disjoint_reaches(self):
        lights = [{"origin": [x, 0.0, 0.0], "radius": 12.0} for x in range(0, 200, 10)]
        ids = lsmk.group_lights(lights)
        self.assertTrue(all(i is not None for i in ids))
        for a in range(len(lights)):
            for b in range(a + 1, len(lights)):
                if ids[a] == ids[b]:
                    gap = abs(lights[a]["origin"][0] - lights[b]["origin"][0])
                    self.assertGreaterEqual(gap, 24.0)

    def test_unbounded_light_has_no_id(self):
        self.assertEqual(lsmk.group_lights([{"origin": [0, 0, 0], "radius": 0.0}]), [None])


class TopFourTests(unittest.TestCase):
    def test_keeps_the_four_strongest(self):
        top = lsmk.TopFour(1, 1)
        for group, light in enumerate([0.1, 0.5, 0.3, 0.9, 0.2, 0.0], start=1):
            top.add(group, np.array([[light]]), np.array([[group / 10.0]]))
        self.assertEqual(top.ids[0, 0].tolist(), [4, 2, 3, 5])
        np.testing.assert_allclose(top.visibility[0, 0], [0.4, 0.2, 0.3, 0.5], rtol=1e-6)

    def test_unlit_groups_take_no_slot(self):
        top = lsmk.TopFour(1, 1)
        top.add(1, np.array([[0.0]]), np.array([[0.0]]))
        self.assertEqual(top.ids[0, 0].tolist(), [0, 0, 0, 0])
        self.assertEqual(top.visibility[0, 0].tolist(), [1.0] * 4)


class EncodingTests(unittest.TestCase):
    def setUp(self):
        rng = np.random.default_rng(7)
        self.visibility = rng.random((5, 7, 4))
        self.ids = rng.integers(0, 4, (5, 7, 4)).astype(np.uint8)
        self.records = [([1.0, 2.0, 3.0], 1), ([-4.0, 5.0, 6.5], 3)]
        self.data, _ = lsmk.build(self.records, self.visibility, self.ids)

    def test_round_trip(self):
        layout, visibility, ids = lsmk.decode(self.data)
        self.assertEqual((layout["width"], layout["height"]), (7, 5))
        self.assertTrue((ids == self.ids).all())
        used = ids > 0
        self.assertLessEqual(np.abs(visibility - self.visibility)[used].max(), 0.5 / 255 + 1e-9)
        self.assertTrue((visibility[~used] == 1.0).all())
        self.assertEqual([light["id"] for light in layout["lights"]], [1, 3])

    def test_rejects_defects(self):
        bad_magic = b"XXXX" + self.data[4:]
        truncated = self.data[:-1]
        nonzero_reserved = self.data[:40] + b"\x01" + self.data[41:]
        bad_id = bytearray(self.data)
        struct.pack_into("<I", bad_id, lsmk.HEADER_BYTES + 12, 0)
        bad_origin = bytearray(self.data)
        struct.pack_into("<f", bad_origin, lsmk.HEADER_BYTES, float("nan"))
        for data in (bad_magic, truncated, nonzero_reserved, bytes(bad_id), bytes(bad_origin)):
            with self.assertRaises(lsmk.MaskError):
                lsmk.read(data)

    def test_writer_refuses_bad_input(self):
        with self.assertRaises(lsmk.MaskError):
            lsmk.build([([0, 0, 0], 256)], self.visibility, self.ids)
        with self.assertRaises(lsmk.MaskError):
            lsmk.build(self.records, self.visibility[..., :3], self.ids)


def bilinear(image, y, x):
    y0, x0 = np.floor(y).astype(int), np.floor(x).astype(int)
    fy, fx = y - y0, x - x0

    def at(r, c):
        return image[np.clip(r, 0, image.shape[0] - 1), np.clip(c, 0, image.shape[1] - 1)]
    return (at(y0, x0) * (1 - fy) * (1 - fx) + at(y0, x0 + 1) * (1 - fy) * fx +
            at(y0 + 1, x0) * fy * (1 - fx) + at(y0 + 1, x0 + 1) * fy * fx)


class AreaVisibilityTests(unittest.TestCase):
    """A hard shadow edge at a slope: point-sampled once per texel (Cycles'
    bake) the runtime's bilinear reconstruction puts the 0.5 contour on a
    staircase; baked at a finer grid and averaged per texel it lies near the
    true line."""

    SLOPE, OFFSET, SIZE = 0.37, 10.3, 48

    def point_sampled(self, factor):
        n = self.SIZE * factor
        r, c = np.mgrid[0:n, 0:n].astype(float)
        # Sample at each fine texel's centre, in coarse texel units.
        y, x = (r + 0.5) / factor, (c + 0.5) / factor
        return (y < self.SLOPE * x + self.OFFSET).astype(float)

    def contour_error(self, visibility):
        errors = []
        for x in np.linspace(8, 40, 97):
            ys = np.linspace(2, 40, 3801)
            v = bilinear(visibility, ys - 0.5, np.full_like(ys, x - 0.5))
            crossing = ys[np.argmin(np.abs(v - 0.5))]
            errors.append(abs(crossing - (self.SLOPE * x + self.OFFSET)))
        return max(errors)

    def test_supersampled_edge_is_near_the_line(self):
        fine = self.point_sampled(lsmk_bake_supersample())
        unshadowed = np.ones_like(fine)
        _, visibility = lsmk.area_visibility(fine, unshadowed, lsmk_bake_supersample())
        self.assertLess(self.contour_error(visibility), 0.3)

    def test_point_sampled_edge_stair_steps(self):
        # The negative control: one sample per texel, as before supersampling.
        self.assertGreater(self.contour_error(self.point_sampled(1)), 0.4)

    def test_unlit_texels_are_visible(self):
        _, visibility = lsmk.area_visibility(np.zeros((4, 4)), np.zeros((4, 4)), 2)
        self.assertTrue((visibility == 1.0).all())

    def test_rejects_planes_not_a_multiple(self):
        with self.assertRaises(lsmk.MaskError):
            lsmk.area_visibility(np.zeros((5, 4)), np.zeros((5, 4)), 2)


def lsmk_bake_supersample():
    # light_mask_bake.py runs inside Blender; its factor is read from source.
    import re
    source = (Path(__file__).resolve().parents[1] / "light_mask_bake.py").read_text()
    return int(re.search(r"^SUPERSAMPLE = (\d+)", source, re.M).group(1))

if __name__ == "__main__":
    unittest.main()
