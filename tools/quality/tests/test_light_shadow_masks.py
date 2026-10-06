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


if __name__ == "__main__":
    unittest.main()
