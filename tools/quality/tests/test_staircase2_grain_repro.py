"""Negative fixtures for the staircase2 lightmap grain ablation."""

import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from bsp2_reader import fourcc, write_bsp2
from staircase2_grain_repro import lightmap_only_change, patch_metrics


class GrainReproTest(unittest.TestCase):
    def test_lmap_only_change_accepts_and_world_mesh_change_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)

            def write(name, world, light):
                path = base / name
                path.write_bytes(write_bsp2(0, [
                    (fourcc("WMSH"), 1, 0, 16, world),
                    (fourcc("LMAP"), 1, 0, 16, light)]))
                return path

            first = write("first.bsp2", b"same world", b"noisy light")
            fixed = write("fixed.bsp2", b"same world", b"clean light")
            altered = write("altered.bsp2", b"other world", b"clean light")
            self.assertEqual(lightmap_only_change(first, fixed), 2)
            with self.assertRaisesRegex(ValueError, "only LMAP"):
                lightmap_only_change(first, altered)

    def test_flat_patch_detects_added_grain(self):
        rng = np.random.default_rng(8)
        flat = np.full((1024, 1024, 3), 160, dtype=np.uint8)
        noisy = flat.copy()
        noisy[80:390, 700:970] = np.clip(
            160 + rng.normal(0, 12, (310, 270, 3)), 0, 255).astype(np.uint8)
        self.assertGreater(patch_metrics(Image.fromarray(noisy))["high_frequency_mad"],
                           patch_metrics(Image.fromarray(flat))["high_frequency_mad"] + 5)


if __name__ == "__main__":
    unittest.main()
