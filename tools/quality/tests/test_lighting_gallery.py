# ==== Copyright Valve Corporation, All rights reserved. ======================
"""lighting_gallery: the display math and the page, without render_lab."""

import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lighting_gallery as lg  # noqa: E402
import lighting_fixtures as lf  # noqa: E402


class DisplayMath(unittest.TestCase):
    def test_same_exposure_keeps_brightness_differences(self):
        reference = np.full((4, 4, 3), 0.5)
        key = lg.log_average(reference)
        brighter = lg.tone_map(reference * 2.0, key)
        same = lg.tone_map(reference, key)
        self.assertTrue((brighter > same).all())

    def test_error_map_is_dark_at_zero_red_at_the_limit_and_grey_where_skipped(self):
        reference = np.full((2, 2, 3), 1.0)
        test = reference.copy()
        test[0, 1] += 2.0
        mask = np.array([[True, True], [False, True]])
        colors = lg.error_map(test, reference, mask, limit=2.0)
        self.assertEqual(tuple(colors[0, 0]), (18, 20, 26))
        self.assertEqual(tuple(colors[0, 1]), (215, 40, 40))
        self.assertEqual(tuple(colors[1, 0]), (128, 128, 128))


class Page(unittest.TestCase):
    def test_portal_transport_is_not_a_cycles_receiver_oracle(self):
        names = lf.cycles_oracle_names()
        self.assertIn("material-sweep", names)
        self.assertNotIn("portal-pair", names)
        with self.assertRaisesRegex(ValueError, "not Cycles receiver oracles: portal-pair"):
            lf.cycles_oracle_names(["portal-pair"])

    def test_page_counts_and_marks_every_view(self):
        uri = lg.jpeg_uri(np.zeros((2, 2, 3), np.uint8))
        result = {"pass": False, "mean": 0.3, "p99": 2.0,
                  "tolerance": {"mean": 0.06, "p99": 0.6},
                  "emitters": {"pixels": 4, "mean": 1.2, "p99": 1.4,
                               "black_control_mean": 1.3}}
        entries = [{"fixture": "a", "state": "default", "camera": "c", "result": result,
                    "status": "preview", "images": [uri, uri, uri, uri]},
                   {"fixture": "b", "state": "default", "camera": "c",
                    "error": "render_lab: model x does not load"}]
        text = lg.page(entries, Path("/x/render_lab"), "abc123 test", "now")
        self.assertIn("<title>Lighting Lab Gallery</title>", text)
        self.assertIn("0 pass", text)
        self.assertIn("1 fail", text)
        self.assertIn("1 not rendered", text)
        self.assertIn("preview references", text)
        self.assertIn("model x does not load", text)
        self.assertIn("black control 1.300", text)
        self.assertEqual(text.count("<img "), 4)


if __name__ == "__main__":
    unittest.main()
