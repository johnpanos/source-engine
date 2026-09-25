#!/usr/bin/env python3
"""Self-tests for the SDF GI performance fixtures (tools/quality/gi_sdf_bench.py,
quality/budgets/indirect-sdf-perf-v1.json): the declaration's validation, the
row verdicts, run parsing and the baseline ratchet. No build or GPU. Run:

    python3 -m unittest tools/quality/tests/test_gi_sdf_bench.py
"""

import copy
import json
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent.parent
sys.path.insert(0, str(HERE.parent))
import gi_sdf_bench  # noqa: E402

TOLERANCE = {"ratio": 1.5, "absolute_ms": 0.05, "improved_ratio": 0.67}


def row(baseline=None, budget=1.0):
    return {"workload": "producer/x", "metric": "frame_publish.median_ms", "owner": "R76",
            "budget_ms": budget, "baseline_ms": baseline}


def record(median, count=10, digest="00"):
    return {"frame_publish": {"count": count, "median_ms": median, "p95_ms": median * 2},
            "begin_ms": 1.0, "digest": digest}


def fixtures():
    with open(ROOT / gi_sdf_bench.DEFAULT_FIXTURES) as stream:
        return json.load(stream)


class Declaration(unittest.TestCase):
    def test_the_checked_in_fixtures_validate(self):
        data = fixtures()
        gi_sdf_bench.validate_fixtures(data)
        with open(ROOT / data["budgets"]) as stream:
            budgets = json.load(stream)
        for profile in data["profiles"].values():
            for r in profile["rows"]:
                self.assertGreater(gi_sdf_bench.resolve_budget(
                    r, budgets, profile.get("indirect_light_profile")), 0)

    def test_every_row_of_the_desktop_profile_has_a_reviewed_baseline(self):
        for r in fixtures()["profiles"]["linux-x86_64-desktop"]["rows"]:
            self.assertIsNotNone(r["baseline_ms"], "%s %s" % (r["workload"], r["metric"]))

    def test_gpu_budgets_come_from_their_owner(self):
        rows = fixtures()["profiles"]["linux-x86_64-desktop"]["rows"]
        gpu = [r for r in rows if r["metric"].startswith(("live_ms", "reference_ms"))]
        self.assertTrue(gpu)
        self.assertTrue(all(r.get("budget_ref") == "sdf.gpu_ms" for r in gpu))

    def rejects(self, mutate):
        data = fixtures()
        mutate(data)
        with self.assertRaises(gi_sdf_bench.FixtureError):
            gi_sdf_bench.validate_fixtures(data)

    def test_bad_declarations_are_rejected(self):
        rows = lambda d: d["profiles"]["linux-x86_64-desktop"]["rows"]  # noqa: E731
        self.rejects(lambda d: d.update(schema="gi-sdf-perf/v0"))
        self.rejects(lambda d: d["tolerance"].update(ratio=1.0))
        self.rejects(lambda d: rows(d).append(dict(rows(d)[0])))  # duplicate row
        self.rejects(lambda d: rows(d)[0].update(workload="producer/none"))
        self.rejects(lambda d: rows(d)[0].pop("owner"))
        self.rejects(lambda d: rows(d)[0].update(budget_ref="sdf.gpu_ms"))  # both budgets
        self.rejects(lambda d: rows(d)[0].update(baseline_ms=-1))
        self.rejects(lambda d: d["workloads"]["producer/map-door"].update(scene="nowhere"))
        self.rejects(lambda d: d["workloads"]["producer/map-door"].update(bench="cycles"))
        self.rejects(lambda d: d["scenes"]["map-door"].update(synthetic={}))
        self.rejects(lambda d: d["sensitivity"].append({"workload": "producer/none"}))

    def test_an_unknown_budget_reference_is_rejected(self):
        r = {"workload": "gpu/x", "metric": "live_ms_median", "owner": "R76",
             "budget_ref": "sdf.cpu_ms"}
        with self.assertRaises(gi_sdf_bench.FixtureError):
            gi_sdf_bench.resolve_budget(r, {"profiles": {"p": {"budgets": {"sdf": {}}}}}, "p")


class Verdicts(unittest.TestCase):
    def judge(self, r, values):
        return gi_sdf_bench.judge(r, r["budget_ms"], values, TOLERANCE)

    def test_within_budget(self):
        self.assertEqual(self.judge(row(0.5), [0.4, 0.5, 0.45])["verdict"], "within")

    def test_debt_is_a_baseline_over_budget(self):
        result = self.judge(row(3.0), [3.1, 2.9, 3.0])
        self.assertEqual(result["verdict"], "debt")
        self.assertAlmostEqual(result["debt_ms"], 2.0)

    def test_a_regression_beyond_the_baseline_fails(self):
        result = self.judge(row(3.0), [4.7, 4.8, 4.9])
        self.assertEqual(result["verdict"], "regressed")

    def test_new_debt_on_a_row_within_budget_fails(self):
        self.assertEqual(self.judge(row(0.5), [1.6, 1.7, 1.6])["verdict"], "regressed")

    def test_noise_within_tolerance_passes(self):
        self.assertEqual(self.judge(row(0.5), [1.4, 1.5, 1.5])["verdict"], "within")

    def test_improvement_asks_for_a_lower_baseline(self):
        result = self.judge(row(3.0), [1.2, 1.3, 1.1])
        self.assertEqual(result["verdict"], "improved")

    def test_a_tiny_improvement_under_the_absolute_floor_is_noise(self):
        self.assertEqual(self.judge(row(0.03, budget=0.01), [0.001])["verdict"], "debt")

    def test_the_median_across_rounds_is_judged(self):
        self.assertEqual(self.judge(row(3.0), [9.0, 3.0, 3.1])["verdict"], "debt")

    def test_a_round_without_the_metric_is_missing(self):
        self.assertEqual(self.judge(row(1.0), [0.5, None])["verdict"], "missing")
        self.assertEqual(self.judge(row(1.0), [])["verdict"], "missing")


class Runs(unittest.TestCase):
    OK = 'BENCH {"frame_publish": {"count": 3, "median_ms": 0.5}, "digest": "ab"}\nCONFORMANCE 8 0\n'

    def test_parse_accepts_one_record_and_passing_oracles(self):
        rec, problem = gi_sdf_bench.parse_run(self.OK, "producer")
        self.assertIsNone(problem)
        self.assertEqual(rec["digest"], "ab")

    def test_parse_rejects_failed_or_missing_oracles(self):
        for text in (self.OK.replace("8 0", "8 1"), self.OK.replace("8 0", "0 0"),
                     self.OK.replace("CONFORMANCE 8 0\n", ""), self.OK + self.OK):
            self.assertIsNotNone(gi_sdf_bench.parse_run(text, "producer")[1], text)

    def test_the_gpu_bench_needs_no_conformance_record(self):
        self.assertIsNone(gi_sdf_bench.parse_run('BENCH {"live_ms_median": 1}\n', "gpu")[1])

    def test_a_stage_measured_over_no_samples_has_no_value(self):
        self.assertIsNone(gi_sdf_bench.metric_value(record(0.5, count=0),
                                                    "frame_publish.median_ms"))
        self.assertEqual(gi_sdf_bench.metric_value(record(0.5), "frame_publish.median_ms"), 0.5)
        self.assertEqual(gi_sdf_bench.metric_value(record(0.5), "begin_ms"), 1.0)
        self.assertIsNone(gi_sdf_bench.metric_value(record(0.5), "frame_publish.max_ms"))
        # The GPU bench's [before, after] pairs judge the after value.
        self.assertEqual(gi_sdf_bench.metric_value({"schedule_cpu_ms_median": [0.1, 0.7]},
                                                   "schedule_cpu_ms_median"), 0.7)

    def evaluate(self, records, problems=()):
        data = {"tolerance": TOLERANCE,
                "profiles": {"p": {"rows": [row(1.0)]}}}
        runs = {"producer/x": {"records": records, "problems": list(problems)}}
        return gi_sdf_bench.evaluate(data, "p", runs, {}, {})[0]

    def test_a_failed_run_fails_its_rows_whatever_it_measured(self):
        result = self.evaluate([record(0.5)], ["round 1: oracle: 1 of 8 checks failed"])
        self.assertEqual(result["verdict"], "failed")

    def test_a_crashed_round_leaves_the_row_missing(self):
        self.assertEqual(self.evaluate([record(0.5), None])["verdict"], "missing")

    def evaluate_loaded(self, loads, problems=()):
        data = {"tolerance": TOLERANCE, "host": {"max_load_per_cpu": 1.0},
                "profiles": {"p": {"rows": [row(1.0)]}}}
        runs = {"producer/x": {"records": [record(9.0)] * len(loads), "problems": list(problems),
                               "loads": loads}}
        return gi_sdf_bench.evaluate(data, "p", runs, {}, {})[0]

    def test_timings_from_an_overloaded_host_are_unverified_not_judged(self):
        result = self.evaluate_loaded([0.5, 2.7])
        self.assertEqual(result["verdict"], "unverified")
        self.assertEqual(result["timing_verdict"], "regressed")
        self.assertEqual(self.evaluate_loaded([0.5, 0.9])["verdict"], "regressed")

    def test_an_oracle_failure_fails_even_on_an_overloaded_host(self):
        result = self.evaluate_loaded([3.0], ["round 1: oracle: 1 of 8 checks failed"])
        self.assertEqual(result["verdict"], "failed")

    def test_overloaded_rows_never_become_baselines(self):
        data = {"profiles": {"p": {"rows": [row(3.0)]}}}
        results = [{"workload": "producer/x", "metric": "frame_publish.median_ms",
                    "verdict": "unverified", "timing_verdict": "improved", "measured_ms": 0.1}]
        self.assertEqual(gi_sdf_bench.update_baselines(data, "p", results, calibrate=True), [])

    def test_unavailable_scenes_are_unverified_never_passed(self):
        data = {"tolerance": TOLERANCE, "profiles": {"p": {"rows": [row(1.0)]}}}
        result = gi_sdf_bench.evaluate(data, "p", {}, {}, {"producer/x": "scene absent"})[0]
        self.assertEqual(result["verdict"], "unverified")
        self.assertNotIn(result["verdict"], ("within", "debt"))


class Ratchet(unittest.TestCase):
    def test_only_improved_rows_move_and_only_down(self):
        data = {"profiles": {"p": {"rows": [row(3.0), dict(row(0.5), metric="begin_ms")]}}}
        results = [{"workload": "producer/x", "metric": "frame_publish.median_ms",
                    "verdict": "improved", "measured_ms": 1.2345},
                   {"workload": "producer/x", "metric": "begin_ms", "verdict": "within",
                    "measured_ms": 0.9}]
        changes = gi_sdf_bench.update_baselines(copy.deepcopy(data), "p", results)
        self.assertEqual(changes, [("producer/x", "frame_publish.median_ms", 3.0, 1.234)])

    def test_calibrate_sets_every_measured_row_but_not_failed_ones(self):
        data = {"profiles": {"p": {"rows": [row(None), dict(row(None), metric="begin_ms")]}}}
        results = [{"workload": "producer/x", "metric": "frame_publish.median_ms",
                    "verdict": "regressed", "measured_ms": 2.0},
                   {"workload": "producer/x", "metric": "begin_ms", "verdict": "failed",
                    "measured_ms": 0.9}]
        changes = gi_sdf_bench.update_baselines(data, "p", results, calibrate=True)
        self.assertEqual(changes, [("producer/x", "frame_publish.median_ms", None, 2.0)])

    def test_the_debt_list_ranks_by_size(self):
        results = [dict(row(2.0), verdict="debt", debt_ms=1.0),
                   dict(row(9.0), metric="b", verdict="debt", debt_ms=8.0),
                   dict(row(0.5), metric="c", verdict="within", debt_ms=0.0)]
        self.assertEqual([d["debt_ms"] for d in gi_sdf_bench.debt_list(results)], [8.0, 1.0])


class Workloads(unittest.TestCase):
    def test_argv_names_the_scene_files_and_options(self):
        files = {"prbv": "v.prbv", "sdfv": "f.sdfv", "focus": "focus.txt",
                 "wmsh": "w.wmsh", "lmap": "l.ktx2"}
        producer = gi_sdf_bench.workload_argv(
            "bench", {"bench": "producer", "focus": True, "budget": 64, "proxy": [0, 1, 2, 3, 4, 5]},
            files, "schedule=3")
        self.assertEqual(producer, ["bench", "producer", "v.prbv", "f.sdfv", "--budget", "64",
                                    "--focus", "focus.txt", "--proxy", "0", "1", "2", "3", "4",
                                    "5", "--seed-slow", "schedule=3"])
        occlusion = gi_sdf_bench.workload_argv(
            "bench", {"bench": "occlusion", "synthetic_texels": 100, "synthetic_lights": 2,
                      "workers": 4}, {})
        self.assertEqual(occlusion, ["bench", "occlusion", "--synthetic", "100", "2",
                                     "--workers", "4"])
        gpu = gi_sdf_bench.workload_argv("gpu", {"bench": "gpu"}, files)
        self.assertEqual(gpu, ["gpu", "--bench", "v.prbv", "f.sdfv"])

    def test_a_scene_with_an_absent_file_is_none(self):
        scene = {"files": {"prbv": "quality-results/none/at/all.prbv"}}
        self.assertIsNone(gi_sdf_bench.scene_files(str(ROOT), "x", scene, "/nonexistent"))


if __name__ == "__main__":
    unittest.main()
