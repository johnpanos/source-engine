"""External presentation evidence never substitutes for GPU/pass duration."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import p2ce_present
import render_profile


class P2cePresentTest(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        self.csv = self.root / "present.csv"
        self.marks = self.root / "marks.jsonl"
        self.clock = self.root / "clock.json"
        self.start = 1700000000000000000
        self.clock.write_text(json.dumps({"schema": "present-clock/v1",
                                        "logging_start_command_ns": self.start}))
        self.csv.write_text("gpu,driver\nfixture,fixture\n"
                            "frametime,elapsed,gpu_core_clock,gpu_temp,gpu_vram_used\n" +
                            "".join("2,%d,1800,75,3\n" % (i * 500000000) for i in range(25)))
        rows = [(0, "QA_CHECK test.map.loaded PASS"),
                (1, "PROFILE_MARK floor_begin"), (1, "PROFILE_MARK arrival"),
                (1, "QA_CHECK test.view.arrival PASS"), (5, "PROFILE_MARK reverse"),
                (5, "QA_CHECK test.view.reverse PASS"), (9, "PROFILE_MARK return"),
                (9, "QA_CHECK test.view.return PASS"), (12, "PROFILE_MARK floor_end"),
                (12, "QA_DONE checks=4 failures=0")]
        self.marks.write_text("".join(json.dumps({"observed_ns": self.start + int(t * 1e9),
                                                "line": line}) + "\n" for t, line in rows))

    def analyze(self):
        return p2ce_present.analyze(self.csv, self.marks, self.clock)

    def rejected(self, text=None):
        return self.assertRaisesRegex(render_profile.ProfileError, text) if text else self.assertRaises(ValueError)

    def test_completed_phases_preserve_interval_semantics_and_clock(self):
        report = self.analyze()
        self.assertEqual(report["status"], "complete")
        self.assertEqual(report["phases"]["arrival"]["presentation_interval"]["samples"], 7)
        self.assertEqual(report["phases"]["return"]["presentation_interval"]["median_ms"], 2)
        self.assertEqual(report["phases"]["arrival"]["gpu_clock_mean_mhz"], 1800)
        self.assertIn("GPU execution and pass breakdown unavailable", report["semantics"])
        self.assertEqual(len(report["sources"]), 3)

    def test_missing_marker_or_check_is_rejected(self):
        self.marks.write_text(self.marks.read_text().replace("PROFILE_MARK reverse", "other"))
        with self.rejected("incomplete"):
            self.analyze()

    def test_failed_check_or_duplicate_completion_is_rejected(self):
        original = self.marks.read_text()
        self.marks.write_text(original.replace("view.return PASS", "view.return FAIL"))
        with self.rejected("failed"):
            self.analyze()
        self.marks.write_text(original + original.splitlines()[-1] + "\n")
        with self.rejected("duplicate completion"):
            self.analyze()

    def test_reordered_marker_clock_is_rejected(self):
        self.marks.write_text(self.marks.read_text().replace(str(self.start + 5 * 10**9), str(self.start)))
        with self.rejected("unordered"):
            self.analyze()

    def test_truncated_observation_and_csv_are_rejected(self):
        original = self.marks.read_text()
        self.marks.write_text(original.rstrip())
        with self.rejected("truncated"):
            self.analyze()
        self.marks.write_text(original)
        self.csv.write_text(self.csv.read_text().rstrip())
        with self.rejected("truncated"):
            self.analyze()

    def test_duplicate_or_invalid_csv_columns_are_rejected(self):
        self.csv.write_text(self.csv.read_text().replace("frametime,elapsed", "elapsed,elapsed"))
        with self.rejected("CSV"):
            self.analyze()

    def test_unordered_or_nonfinite_samples_are_rejected(self):
        original = self.csv.read_text()
        self.csv.write_text(original + "2,0,1800,75,3\n")
        with self.rejected("unordered"):
            self.analyze()
        self.csv.write_text(original.replace("2,0,", "nan,0,"))
        with self.rejected("invalid"):
            self.analyze()

    def test_no_samples_never_become_zero_duration(self):
        self.csv.write_text("\n".join(self.csv.read_text().splitlines()[:3]) + "\n")
        with self.rejected("missing presentation"):
            self.analyze()

    def test_invalid_edge_guard_is_rejected(self):
        for guard in (0, 3, float("nan")):
            with self.rejected("guard"):
                p2ce_present.analyze(self.csv, self.marks, self.clock, guard)

    def test_invalid_clock_is_rejected(self):
        for value in (True, -1, "1700000000000000000"):
            self.clock.write_text(json.dumps({"schema": "present-clock/v1", "logging_start_command_ns": value}))
            with self.rejected("clock"):
                self.analyze()


if __name__ == "__main__":
    unittest.main()
