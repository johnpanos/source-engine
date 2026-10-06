#!/usr/bin/env python3
"""Self-tests for tools/quality/scanout_probe.py (no display or DRM node)."""
import importlib.util
from pathlib import Path
import unittest

SPEC = importlib.util.spec_from_file_location(
    "scanout_probe", Path(__file__).resolve().parents[1] / "scanout_probe.py")
probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(probe)


class FeedbackTests(unittest.TestCase):
    def test_counts_scanout_tranches(self):
        log = ("[1.0] zwp_linux_dmabuf_feedback_v1#40.tranche_flags(0)\n"
               "[1.1] zwp_linux_dmabuf_feedback_v1#40.tranche_flags(1)\n"
               "[1.2] zwp_linux_dmabuf_feedback_v1#41.tranche_flags(1)\n")
        self.assertEqual(probe.parse_feedback(log), {"tranches": 3, "scanout_tranches": 2})

    def test_requests_are_not_events(self):
        # A client request line never carries tranche_flags; nothing else counts.
        log = "[1.0]  -> zwp_linux_dmabuf_v1#9.get_surface_feedback(new id #40, wl_surface#3)\n"
        self.assertEqual(probe.parse_feedback(log), {"tranches": 0, "scanout_tranches": 0})


class PlaneTests(unittest.TestCase):
    def test_full_size_keeps_the_display_sized_planes(self):
        entries = {
            1: (10, 5, 1, 2880, 1800, "XR30", 0),
            2: (11, 5, 2, 64, 64, "AR24", 0),  # cursor
            3: (10, 5, 3, 2880, 1800, "XR24", 0),
        }
        self.assertEqual(sorted(probe.full_size(entries)), [1, 3])

    def test_full_size_of_nothing(self):
        self.assertEqual(probe.full_size({}), {})

    def test_fourcc(self):
        self.assertEqual(probe.fourcc(808665688), "XB30")  # the game's HDR10 buffers
        self.assertEqual(probe.fourcc(0x30335258), "XR30")


if __name__ == "__main__":
    unittest.main()
