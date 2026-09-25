"""Bake progress from Cycles' debug log (bake_progress.py)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import bake_progress  # noqa: E402


class Clock:
    def __init__(self):
        self.now = 0.0

    def __call__(self):
        return self.now


def cycles(text):
    return "00:01.000  cycles           | " + text


class BakeProgressTests(unittest.TestCase):
    def test_labelled_pass_reports_tiles_fraction_and_eta(self):
        clock = Clock()
        progress = bake_progress.CyclesProgress(clock, interval=10.0)
        self.assertEqual(progress.feed(bake_progress.progress_line(
            event="bake", label="direct", size=[2048, 2048])), [])
        self.assertEqual(progress.feed(cycles("Using tile size of (1024, 1024)")),
                         ["pass 1 (direct) started (4 tiles)"])
        clock.now = 5.0
        self.assertEqual(progress.feed(cycles(
            "Rendered 100 samples in 5.0 seconds (0.05 seconds per sample), occupancy: 1")), [])
        clock.now = 12.0
        self.assertEqual(progress.feed(cycles(
            "Rendered 100 samples in 7.0 seconds (0.07 seconds per sample), occupancy: 1")),
            ["pass 1 (direct): tile 1/4, 200 samples, 12s elapsed"])
        clock.now = 30.0
        self.assertEqual(progress.feed(cycles("Write tile result.")),
                         ["pass 1 (direct): tile 1/4 (25%), 30s elapsed, ~1m30s left"])
        # Cycles' summary line (no per-sample rate) is not a batch.
        self.assertEqual(progress.feed(cycles("Rendered 200 samples in 7.000 seconds")), [])

    def test_samples_give_the_fraction_within_a_tile(self):
        clock = Clock()
        progress = bake_progress.CyclesProgress(clock, interval=10.0)
        progress.feed(bake_progress.progress_line(event="bake", label="total",
                                                  size=[2048, 1024], samples=1000))
        progress.feed(cycles("Using tile size of (1024, 1024)"))
        clock.now = 20.0
        progress.feed(cycles("Write tile result."))
        clock.now = 40.0
        self.assertEqual(progress.feed(cycles(
            "Rendered 500 samples in 20.0 seconds (0.04 seconds per sample), occupancy: 1")),
            ["pass 1 (total): tile 2/2, 500/1000 samples (75%), 40s elapsed, ~13s left"])

    def test_unlabelled_bake_counts_tiles_and_the_label_is_used_once(self):
        progress = bake_progress.CyclesProgress(Clock())
        progress.feed(bake_progress.progress_line(event="bake", label="total", size=[512, 512]))
        progress.feed(cycles("Using tile size of (512, 512)"))
        self.assertEqual(progress.feed(cycles("Write tile result.")),
                         ["pass 1 (total): tile 1/1 (100%), 0s elapsed, ~0s left"])
        self.assertEqual(progress.feed(cycles("Using tile size of (2048, 2048)")),
                         ["pass 2 started (tile 2048x2048)"])
        self.assertEqual(progress.feed(cycles("Write tile result.")),
                         ["pass 2: tile 1 done, 0s elapsed"])

    def test_messages_pass_through_and_noise_is_ignored(self):
        progress = bake_progress.CyclesProgress(Clock())
        self.assertEqual(progress.feed(bake_progress.progress_line(message="packing UVs")),
                         ["packing UVs"])
        self.assertEqual(progress.feed("PROGRESS not json"), ["not json"])
        self.assertEqual(progress.feed(cycles("Global memory allocate: objects, 512 bytes.")), [])
        self.assertEqual(progress.feed(cycles("Write tile result.")), [])  # no pass yet

    def test_durations(self):
        self.assertEqual([bake_progress.duration(s) for s in (4, 75, 3725)],
                         ["4s", "1m15s", "1h02m"])


if __name__ == "__main__":
    unittest.main()
