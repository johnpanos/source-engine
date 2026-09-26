"""Self-tests for the legacy shader ports view comparison (legacy_ports_views.py)."""

import importlib.util
from pathlib import Path
import unittest

import numpy

QUALITY = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("legacy_ports_views",
                                              QUALITY / "legacy_ports_views.py")
views = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(views)


def frame(value=100, size=(20, 20)):
    return numpy.full(size + (3,), value, dtype=numpy.int16)


class DifferingFractionTest(unittest.TestCase):
    def test_identical_frames_do_not_differ(self):
        self.assertEqual(views.differing_fraction(frame(), frame()), 0.0)

    def test_changes_within_the_threshold_are_ignored(self):
        b = frame()
        b[:, :, 1] += views.PIXEL_THRESHOLD
        self.assertEqual(views.differing_fraction(frame(), b), 0.0)

    def test_one_changed_channel_counts_the_pixel(self):
        b = frame()
        b[3, 4, 2] += views.PIXEL_THRESHOLD + 1
        self.assertAlmostEqual(views.differing_fraction(frame(), b), 1 / 400.0)

    def test_black_band_is_detected(self):
        # The first ports merge drew diagonal black bands over the scene.
        b = frame()
        for i in range(20):
            b[i, max(0, i - 2):i + 2] = 0
        fraction = views.differing_fraction(frame(), b)
        self.assertGreater(fraction, views.FRACTION_LIMIT)
        self.assertFalse(views.judge(fraction, 0.0)[0])

    def test_size_mismatch_is_a_full_difference(self):
        self.assertEqual(views.differing_fraction(frame(), frame(size=(10, 20))), 1.0)


class JudgeTest(unittest.TestCase):
    def test_small_difference_passes(self):
        self.assertTrue(views.judge(views.FRACTION_LIMIT, 0.0)[0])

    def test_noisy_view_widens_its_limit(self):
        passed, limit = views.judge(0.02, 0.01)
        self.assertTrue(passed)
        self.assertAlmostEqual(limit, views.NOISE_FACTOR * 0.01)

    def test_difference_beyond_noise_fails(self):
        self.assertFalse(views.judge(0.05, 0.01)[0])


class CaptureScriptTest(unittest.TestCase):
    def test_one_screenshot_per_view_and_the_pause_menu(self):
        script = views.capture_script()
        self.assertEqual(script.count("screenshot"), len(views.VIEWS) + 1)
        self.assertIn("gameui_activate", script)
        # One line: a cfg's separate lines would run without the waits.
        self.assertNotIn("\n", script)


if __name__ == "__main__":
    unittest.main()
