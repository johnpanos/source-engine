import math
import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import legacy_bsp_scene as scene  # noqa: E402


def texel(rgb):
    image = np.zeros((1, 1, 4), np.uint8)
    image[0, 0, :3] = [round(c * 255) for c in rgb]
    image[0, 0, 3] = 255
    return image


def decoded(normal):
    return normal[0, 0].astype(float) / 255 * 2 - 1


class SsbumpTest(unittest.TestCase):
    def test_flat_texel_is_straight_up_and_unoccluded(self):
        flat = 1 / math.sqrt(3)
        normal, occlusion = scene.ssbump_maps(texel((flat, flat, flat)))
        np.testing.assert_allclose(decoded(normal), (0, 0, 1), atol=0.01)
        self.assertEqual(occlusion[0, 0], 255)

    def test_light_from_the_first_basis_tilts_toward_u(self):
        normal, occlusion = scene.ssbump_maps(texel((0.9, 0.3, 0.3)))
        n = decoded(normal)
        self.assertGreater(n[0], 0.3)
        self.assertAlmostEqual(n[1], 0, delta=0.01)
        self.assertLess(occlusion[0, 0], 255)

    def test_second_basis_is_flipped_to_opengl(self):
        # Basis 1 points toward DirectX +Y (down the texture), OpenGL -Y.
        normal, _ = scene.ssbump_maps(texel((0.3, 0.9, 0.3)))
        self.assertLess(decoded(normal)[1], -0.3)

    def test_grooves_darken(self):
        _, occlusion = scene.ssbump_maps(texel((0.3, 0.3, 0.3)))
        self.assertAlmostEqual(occlusion[0, 0] / 255, 0.3 * math.sqrt(3), delta=0.01)


class EnvmapStrengthTest(unittest.TestCase):
    def test_tint_scales_the_gloss(self):
        self.assertAlmostEqual(scene.envmap_strength({"$envmaptint": ".2 .2 .2"}), 0.5)
        self.assertEqual(scene.envmap_strength({"$envmaptint": "[1 1 1]"}), 1.0)
        self.assertEqual(scene.envmap_strength({}), 1.0)


if __name__ == "__main__":
    unittest.main()
