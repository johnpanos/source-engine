"""Negative structural-edge fixtures for render parity."""

import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from edge_parity import edge_parity


def frame(lines):
    image = np.full((128, 128, 3), 128, dtype=np.uint8)
    for x0, y0, x1, y1 in lines:
        image[y0:y1, x0:x1] = 240
    return image


class EdgeParityTest(unittest.TestCase):
    def test_identical_and_small_camera_shift(self):
        reference = frame([(30, 12, 33, 115), (60, 40, 110, 43)])
        same, _, _ = edge_parity(reference, reference)
        shifted, _, _ = edge_parity(reference,
                                   frame([(32, 12, 35, 115), (60, 42, 110, 45)]))
        self.assertEqual(same["whole_frame"]["f1"], 1.0)
        self.assertGreater(shifted["whole_frame"]["f1"], 0.8)

    def test_missing_and_stray_edges_reduce_score(self):
        reference = frame([(30, 12, 33, 115), (60, 40, 110, 43)])
        missing, _, _ = edge_parity(reference, frame([(60, 40, 110, 43)]))
        stray, _, _ = edge_parity(reference,
                                  frame([(30, 12, 33, 115), (60, 40, 110, 43),
                                         (90, 60, 93, 115)]))
        self.assertLess(missing["whole_frame"]["recall"], 0.8)
        self.assertLess(stray["whole_frame"]["precision"], 0.8)

    def test_gap_in_visible_edge_reduces_recall(self):
        reference = frame([(30, 12, 33, 115)])
        broken = frame([(30, 12, 33, 52), (30, 78, 33, 115)])
        score, _, _ = edge_parity(reference, broken)
        self.assertLess(score["whole_frame"]["recall"], 0.8)

    def test_blank_frame_cannot_pass(self):
        reference = frame([(30, 12, 33, 115)])
        with self.assertRaisesRegex(ValueError, "nonempty"):
            edge_parity(reference, np.zeros_like(reference))


if __name__ == "__main__":
    unittest.main()
