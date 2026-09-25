#!/usr/bin/env python3
"""The bake-noise gate measures known noise correctly and rejects bad pairs."""

import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lightmap_noise as noise  # noqa: E402

SIZE = 128


def pair(relative_sigma_of_mean, seed=0, level=None):
    """Two halves of a bake of a smooth light field whose mean has the given
    relative noise: each half has sqrt(2) times that."""
    rng = np.random.default_rng(seed)
    y, x = np.mgrid[0:SIZE, 0:SIZE] / SIZE
    light = 0.5 + 0.4 * x * y if level is None else np.full((SIZE, SIZE), level)
    sigma_half = relative_sigma_of_mean * np.sqrt(2) * light
    halves = []
    for _ in range(2):
        value = light + rng.standard_normal(light.shape) * sigma_half
        halves.append(np.dstack([value] * 3 + [np.ones_like(value)]))
    coverage = np.zeros((SIZE, SIZE), bool)
    coverage[8:-8, 8:-8] = True
    return halves[0], halves[1], coverage


class NoiseTest(unittest.TestCase):
    def test_estimate_matches_the_true_noise(self):
        for truth in (0.003, 0.01, 0.05):
            with self.subTest(truth=truth):
                a, b, coverage = pair(truth, seed=int(truth * 1e4))
                result = noise.measure(a, b, coverage, 1024, 1.0)
                self.assertAlmostEqual(result["raw"]["median_relative_noise"] / truth, 1.0,
                                       delta=0.1)
                # The 99th percentile of a 25-texel average sits a little above.
                self.assertLess(result["relative_noise"] / truth, 1.8)
                self.assertGreater(result["relative_noise"] / truth, 1.0)

    def test_required_samples_follow_one_over_root_n(self):
        """Four times the samples halves the noise, and the recommendation
        made at N predicts the sample count that meets the target."""
        self.assertEqual(noise.required_samples(256, 0.02, 0.01), 1024)
        a, b, coverage = pair(0.02, seed=1)
        coarse = noise.measure(a, b, coverage, 256, 0.01)
        a, b, coverage = pair(0.02 / 2, seed=2)                  # what 4x samples give
        fine = noise.measure(a, b, coverage, 1024, 0.01)
        self.assertAlmostEqual(fine["relative_noise"] / coarse["relative_noise"], 0.5, delta=0.1)
        self.assertLess(abs(coarse["required_samples"] - fine["required_samples"]) /
                        coarse["required_samples"], 0.25)

    def test_gate_passes_and_fails_on_the_target(self):
        a, b, coverage = pair(0.004, seed=3)
        self.assertEqual(noise.measure(a, b, coverage, 1024, 0.01)["status"], "pass")
        a, b, coverage = pair(0.03, seed=4)
        self.assertEqual(noise.measure(a, b, coverage, 1024, 0.01)["status"], "fail")

    def test_identical_halves_are_rejected(self):
        """Equal halves mean the second bake reused the seed: no noise
        measurement, not zero noise."""
        a, _, coverage = pair(0.01, seed=5)
        with self.assertRaises(ValueError):
            noise.measure(a, a.copy(), coverage, 1024, 0.01)

    def test_empty_coverage_is_rejected(self):
        a, b, coverage = pair(0.01, seed=6)
        with self.assertRaises(ValueError):
            noise.measure(a, b, np.zeros_like(coverage), 1024, 0.01)

    def test_denoised_gate_judges_the_residual(self):
        """After per-chart denoising the residual difference is far below the
        raw noise, and the record keeps both."""
        try:
            import lightmap_denoise
            library = lightmap_denoise.load_oidn("libOpenImageDenoise.so.2")
        except OSError:
            self.skipTest("OpenImageDenoise is not installed")
        denoiser = lightmap_denoise.Denoiser(library)
        try:
            a, b, coverage = pair(0.05, seed=9)
            result = noise.measure(a.astype(np.float32), b.astype(np.float32), coverage, 256,
                                   0.02, denoiser)
        finally:
            denoiser.close()
        self.assertEqual(result["judged"], "denoised")
        self.assertLess(result["denoised"]["relative_noise"], 0.5 * result["raw"]["relative_noise"])

    def test_near_black_texels_do_not_dominate(self):
        """Noise in texels far below the floor is judged against the floor."""
        a, b, coverage = pair(0.004, seed=7)
        dark = np.zeros((SIZE, SIZE), bool)
        dark[40:60, 40:60] = True
        rng = np.random.default_rng(8)
        for half in (a, b):
            half[dark, :3] = 1e-6 * rng.random((dark.sum(), 1))
        self.assertLess(noise.measure(a, b, coverage, 1024, 0.01)["relative_noise"], 0.01)


if __name__ == "__main__":
    unittest.main()
