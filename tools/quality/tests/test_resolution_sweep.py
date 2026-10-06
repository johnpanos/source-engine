"""Tests for tools/quality/resolution_sweep.py: run order, point summaries,
exclusions and changes, on synthetic demo_frames evidence."""

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import resolution_sweep  # noqa: E402


def evidence(width, height, interval, cpu, gpu, status="complete", failures=(), others=0):
    metrics = {name: {"median_ms": value, "p95_ms": value * 1.5}
               for name, value in (("interval", interval), ("cpu", cpu), ("gpu_render", gpu))}
    return {"status": status, "failures": list(failures),
            "host": {"other_game_processes": ["game"] * others},
            "profile": {"frames": 100, "render_extents": [[width, height]], "metrics": metrics}}


class ResolutionSweepTests(unittest.TestCase):
    def sweep(self, runs):
        root = Path(tempfile.mkdtemp())
        plan = {"resolutions": ["1024x768", "3840x2160"], "variants": {"base": [], "new": []}}
        (root / "plan.json").write_text(json.dumps(plan))
        for (resolution, variant, repeat), record in runs.items():
            directory = root / "runs" / resolution / variant / ("r%d" % repeat)
            directory.mkdir(parents=True)
            (directory / "evidence.json").write_text(json.dumps(record))
        return resolution_sweep.build_report(root)

    def test_order_interleaves_and_rotates_variants(self):
        order = resolution_sweep.run_order(2, [(1, 1), (2, 2)], ["a", "b"])
        self.assertEqual(order, [(0, (1, 1), "a"), (0, (1, 1), "b"), (0, (2, 2), "a"),
                                 (0, (2, 2), "b"), (1, (1, 1), "b"), (1, (1, 1), "a"),
                                 (1, (2, 2), "b"), (1, (2, 2), "a")])

    def test_points_changes_and_bounds(self):
        report = self.sweep({
            ("1024x768", "base", 0): evidence(1024, 768, 20.0, 19.5, 7.0),
            ("1024x768", "base", 1): evidence(1024, 768, 22.0, 21.0, 7.0, others=1),
            ("1024x768", "new", 0): evidence(1024, 768, 18.0, 17.5, 7.0),
            ("1024x768", "new", 1): evidence(1024, 768, 18.0, 17.5, 7.0),
            ("3840x2160", "base", 0): evidence(3840, 2160, 40.0, 10.0, 39.0),
            ("3840x2160", "new", 0): evidence(3840, 2160, 30.0, 10.0, 29.0),
        })
        base = report["points"]["1024x768"]["base"]
        self.assertEqual(base["interval_median_ms"], 21.0)
        self.assertEqual(base["interval_median_spread_ms"], 2.0)
        self.assertEqual(base["runs_beside_other_games"], 1)
        self.assertEqual(base["bound"], "cpu")
        self.assertEqual(report["points"]["3840x2160"]["base"]["bound"], "gpu")
        change = report["changes"]["3840x2160"]["new"]["interval_median_ms"]
        self.assertEqual(change, {"delta_ms": -10.0, "percent": -25.0})
        self.assertEqual(report["excluded"], [])

    def test_bad_runs_are_excluded_and_points_unavailable(self):
        report = self.sweep({
            ("1024x768", "base", 0): evidence(1024, 768, 20.0, 19.0, 7.0),
            ("1024x768", "new", 0): evidence(1024, 768, 20.0, 19.0, 7.0, failures=["x"]),
            ("3840x2160", "base", 0): evidence(1024, 768, 20.0, 19.0, 7.0),
            ("3840x2160", "new", 0): evidence(3840, 2160, 20.0, 19.0, 7.0, status="error"),
        })
        self.assertEqual(report["points"]["1024x768"]["new"]["bound"], "unavailable")
        self.assertEqual(report["points"]["3840x2160"]["base"]["bound"], "unavailable")
        reasons = sorted(item["reason"].split(":")[0].split(" ")[0] for item in report["excluded"])
        self.assertEqual(reasons, ["drawable", "run", "run"])
        self.assertEqual(report["changes"]["1024x768"]["new"], {})

    def test_variant_and_resolution_parsing(self):
        self.assertEqual(resolution_sweep.parse_variant("gpu=+r_x,1"), ("gpu", ["+r_x", "1"]))
        self.assertEqual(resolution_sweep.parse_resolution("3840x2160"), (3840, 2160))
        with self.assertRaises(resolution_sweep.SweepError):
            resolution_sweep.parse_resolution("4k")
        with self.assertRaises(resolution_sweep.SweepError):
            resolution_sweep.parse_variant("=x")


if __name__ == "__main__":
    unittest.main()
