"""The device-free parts of ios_frame_pacing.py: budget rows, pass windows and
the engine arguments."""

import argparse
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import ios_frame_pacing as pacing  # noqa: E402
from profile_extends import load_profile  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
SCENARIO = {"map": "testchmb_a_02", "host_framerate": 60}


def arguments(vsync=False, extra=()):
    return argparse.Namespace(vsync=vsync, fps_max=1000, extra_arg=list(extra))


def frame(interval_ms, mark=""):
    return {"interval": int(interval_ms * 1000), "mark": mark}


class BudgetRowTests(unittest.TestCase):
    def test_modes_select_their_limits_without_the_description(self):
        vsync = pacing.budget_limits("tvos-portal-frame-pacing-60", True)
        headroom = pacing.budget_limits("tvos-portal-frame-pacing-60", False)
        self.assertEqual(vsync["mode"], "vsync")
        self.assertEqual(vsync["refresh_hz"], 60)
        self.assertIn("max_missed_refreshes", vsync)
        self.assertNotIn("meaning", vsync)
        self.assertEqual(headroom["mode"], "headroom")
        self.assertIn("max_gpu_render_p99_ms", headroom)
        self.assertNotIn("refresh_hz", headroom)

    def test_an_unknown_row_is_an_error(self):
        with self.assertRaises(ValueError):
            pacing.budget_limits("no-such-row", True)


class PassIntervalTests(unittest.TestCase):
    def test_windows_follow_the_marks_and_a_missing_pass_is_empty(self):
        frames = [frame(16.7, "pass_1_begin"), frame(16.7), frame(33.3), frame(16.7, "pass_1_end"),
                  frame(10.0)]
        self.assertEqual(pacing.pass_intervals(frames, 2), [[16.7, 33.3, 16.7], []])


class EngineArgumentTests(unittest.TestCase):
    def test_sound_is_never_muted(self):
        # volume is archived: a muted benchmark mutes the next ordinary launch.
        for vsync in (False, True):
            args = pacing.engine_arguments(SCENARIO, "first.cfg", arguments(vsync), True)
            self.assertNotIn("+volume", args)

    def test_settings_follow_the_pinned_presentation_and_precede_the_map(self):
        args = pacing.engine_arguments(SCENARIO, "first.cfg", arguments(), True)
        settings = args.index(pacing.SETTINGS_CFG)
        self.assertGreater(settings, args.index("+mat_antialias"))
        self.assertLess(settings, args.index("+map"))
        self.assertNotIn(pacing.SETTINGS_CFG,
                         pacing.engine_arguments(SCENARIO, "first.cfg", arguments(), False))

    def test_vsync_uncaps_nothing_and_off_caps_at_fps_max(self):
        on = pacing.engine_arguments(SCENARIO, "first.cfg", arguments(True), False)
        off = pacing.engine_arguments(SCENARIO, "first.cfg", arguments(False), False)
        self.assertEqual(on[on.index("+mat_vsync") + 1], "1")
        self.assertEqual(on[on.index("+fps_max") + 1], "0")
        self.assertEqual(off[off.index("+fps_max") + 1], "1000")
        self.assertEqual(off[-1], "first.cfg")


class ProfileTests(unittest.TestCase):
    def test_each_apple_profile_names_its_platform_app_and_container(self):
        expected = {"portal-ios-native-vulkan.json": ("ios", "Documents"),
                    "portal-tvos-native-vulkan.json": ("tvos", "Library/Caches")}
        for name, (platform, container) in expected.items():
            profile = load_profile(REPO / "quality/product_profiles" / name)
            self.assertEqual(profile["target"]["os"], platform)
            self.assertIn(platform, pacing.DEVICE_PLATFORMS)
            self.assertEqual(profile["content"]["container_directory"], container)
            self.assertTrue(profile[platform]["bundle_id"] and profile[platform]["executable"])


if __name__ == "__main__":
    unittest.main()
