"""Profiling oracles: delayed timestamps, coverage, overlap and corrupt evidence."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import render_profile


def frame(index, **extra):
    return {"f": index, "interval": 10000, "cpu": 2000, "engine": 2500,
            "backend": 7000, "cost": {"emit": [1, 1000], "emit_convert": [1, 800]}, **extra}


class RenderProfileTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / "frames.jsonl"

    def write(self, rows):
        records = [{"schema": "vulkan-frame-stats/v1", "device": "fixture"}, *rows]
        self.path.write_text("".join(json.dumps(row) + "\n" for row in records))

    def complete(self):
        self.write([frame(1, mark="floor_begin,arrival"), frame(2, mark="floor_end,reverse",
                    gpu=[1, 7000, 6000], gpu_passes=[["world", 1, 6000], ["present", 1, 1000]]),
                    frame(3, gpu=[2, 4000, 3000], gpu_passes=[["world", 1, 3000], ["present", 1, 1000]])])

    def test_delayed_results_are_joined_to_their_original_phase(self):
        self.complete()
        result = render_profile.analyze(self.path)
        self.assertEqual(result["status"], "complete")
        self.assertEqual(result["phases"]["arrival"]["metrics"]["gpu_render"]["mean_ms"], 6)
        self.assertEqual(result["phases"]["reverse"]["metrics"]["gpu_render"]["mean_ms"], 3)
        self.assertEqual(result["phases"]["arrival"]["gpu_passes"]["segments"]["world"]["mean_ms"], 6)
        self.assertNotIn("present", result["phases"]["arrival"]["cpu_costs"])

    def test_identical_repeated_gpu_result_is_counted_once(self):
        self.complete()
        rows = self.path.read_text().splitlines()
        duplicate = json.loads(rows[-1]); duplicate["f"] = 4
        self.path.write_text("\n".join(rows) + "\n" + json.dumps(duplicate) + "\n")
        self.assertEqual(render_profile.analyze(self.path)["summary"]["metrics"]["gpu_render"]["samples"], 2)

    def test_conflicting_gpu_result_fails(self):
        self.complete()
        self.path.write_text(self.path.read_text() + json.dumps(frame(4, gpu=[2, 5000, 3000])) + "\n")
        with self.assertRaisesRegex(render_profile.ProfileError, "conflicting"):
            render_profile.analyze(self.path)

    def test_missing_passes_or_completions_are_incomplete_not_zero(self):
        self.write([frame(1, mark="floor_begin,floor_end")])
        result = render_profile.analyze(self.path)
        self.assertEqual(result["status"], "incomplete")
        self.assertIsNone(result["summary"]["metrics"]["gpu_render"])
        self.assertIsNone(result["summary"]["gpu_passes"])
        self.assertEqual(len(result["failures"]), 2)

    def test_inclusive_cpu_costs_are_not_added_to_one_another_or_gpu(self):
        self.complete()
        result = render_profile.analyze(self.path)["summary"]
        self.assertEqual(result["cpu_costs"]["emit"]["mean_ms"], 1)
        self.assertEqual(result["cpu_costs"]["emit_convert"]["mean_ms"], .8)
        self.assertNotIn("total", result["cpu_costs"])
        self.assertEqual(result["metrics"]["cpu"]["mean_ms"], 2)

    def test_core_scopes_keep_depth_and_cpu_separate_and_detect_overflow(self):
        reports = render_profile.core_reports(
            "cl_render_debug_stats: core GPU passes, mean of 20 frame(s):\n"
            "  core world view                   60.000 ms  x104.0\n"
            "    core world                      56.000 ms  x104.0\n"
            "  core world view (CPU recording, render sequence)  12.000 ms  x104.0\n"
            "  (timestamps dropped)               0.000 ms  x40.0\n")
        self.assertEqual(reports[0]["passes"][1]["depth"], 1)
        self.assertEqual(reports[0]["passes"][2]["kind"], "cpu")
        self.assertEqual(reports[0]["overflowed"], 40)
        self.complete()
        console = Path(self.directory.name) / "console.log"
        console.write_text("cl_render_debug_stats: core GPU passes, mean of 20 frame(s):\n"
                           "  world   5.000 ms  x1.0\n  (timestamps dropped)  0.000 ms  x40.0\n")
        self.assertEqual(render_profile.analyze(self.path, console)["status"], "incomplete")

    def test_invalid_warmup_cpu_is_excluded_but_invalid_measured_cpu_fails(self):
        self.write([frame(1, cpu=2**64-1), frame(2, mark="floor_begin,floor_end")])
        self.assertEqual(render_profile.analyze(self.path)["summary"]["frames"], 1)
        with self.assertRaisesRegex(render_profile.ProfileError, "cpu duration"):
            render_profile.analyze(self.path, begin=None, end=None)

    def test_missing_end_and_empty_measurements_fail(self):
        for rows in ([frame(1, mark="floor_begin")], [frame(1)]):
            self.write(rows)
            with self.assertRaises(render_profile.ProfileError):
                render_profile.analyze(self.path)

    def test_truncation_duplicate_json_keys_bad_numbers_and_frame_gaps_fail(self):
        self.complete()
        text = self.path.read_text()
        for bad in (text[:-1], text.replace('"cpu": 2000', '"cpu": 2000, "cpu": 1', 1),
                    text.replace('"f": 2', '"f": 4', 1),
                    text.replace('"cpu": 2000', '"cpu": NaN', 1)):
            with self.subTest(bad=bad[-40:]):
                self.path.write_text(bad)
                with self.assertRaises(render_profile.ProfileError):
                    render_profile.analyze(self.path)

    def test_tsv_contains_one_grepable_line_per_metric_and_pass(self):
        self.complete()
        text = render_profile.tsv(render_profile.analyze(self.path))
        self.assertIn("gpu_segment\tarrival\tworld\t6.000", text)
        self.assertIn("cpu_inclusive\tarrival\temit\t1.0000", text)

    def test_bad_labels_counts_and_extents_fail_without_corrupting_tsv(self):
        for extra in ({"mark": 4}, {"mark": "arrival\nreverse"},
                      {"extent": [[1920], 1080]}, {"extent": [1920, False]},
                      {"cost": {"bad\tname": [1, 2]}}, {"cost": {"emit": [1.5, 2]}},
                      {"gpu": [1, 7, 6], "gpu_passes": [["world", .5, 6]]},
                      {"gpu": [1, 7, 6], "gpu_passes": [["world\n", 1, 6]]}):
            with self.subTest(extra=extra):
                self.write([frame(1, mark="floor_begin,floor_end", **{
                    key: value for key, value in extra.items() if key != "mark"})])
                if "mark" in extra:
                    self.write([frame(1, **extra)])
                with self.assertRaises(render_profile.ProfileError):
                    render_profile.analyze(self.path)


if __name__ == "__main__":
    unittest.main()
