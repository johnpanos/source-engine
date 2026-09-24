"""The PBRT solids must encode to the same colors in Cycles and Source."""

import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/quality"))

from staircase2_materials import linear_to_srgb_byte, solid_linear_base_colors


class Staircase2MaterialTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = (ROOT / "staircase2/scene-v4.pbrt").read_text()

    def test_solid_reflectances_and_srgb_bytes(self):
        colors = solid_linear_base_colors(self.source)
        self.assertEqual(set(colors), {"wall", "metal", "chrome"})
        self.assertEqual(tuple(map(linear_to_srgb_byte, colors["wall"])), (243, 243, 243))
        self.assertEqual(tuple(map(linear_to_srgb_byte, colors["metal"])), (247, 246, 246))
        self.assertEqual(tuple(map(linear_to_srgb_byte, colors["chrome"])), (216, 211, 216))

    def test_source_change_changes_encoded_color(self):
        changed = self.source.replace('"rgb reflectance" [ 0.893289 0.893289 0.893289 ]',
                                      '"rgb reflectance" [ 0.5 0.5 0.5 ]', 1)
        self.assertNotEqual(tuple(map(linear_to_srgb_byte,
                                      solid_linear_base_colors(changed)["wall"])),
                            (243, 243, 243))

    def test_invalid_source_and_encoding_rejected(self):
        with self.assertRaises(ValueError):
            solid_linear_base_colors(self.source.replace('"rgb eta"', '"rgb nope"', 1))
        with self.assertRaises(ValueError):
            linear_to_srgb_byte(1.1)


if __name__ == "__main__":
    unittest.main()
