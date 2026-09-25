"""Negative and positive fixtures for the physics benchmark gate (RFC 0013)."""

import copy
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
ROOT = QUALITY.parents[1]
sys.path.insert(0, str(QUALITY))
SPEC = importlib.util.spec_from_file_location("physics_bench", QUALITY / "physics_bench.py")
bench = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(bench)


def output(samples=3, digest="00000000000000aa", failed=0, metrics=None, extra=""):
    lines = ["PROVIDER lib.so", "PASS boot module.load"]
    if failed:
        lines.append("FAIL gameplay bench.pile.finite: non-finite")
    lines.append("BENCH pile 0 8 0")
    lines.append("BUILD_US 10.0")
    if samples is not None:
        lines.append("SAMPLES " + " ".join("%d.0" % (i + 1) for i in range(samples)))
    for name, value in (metrics or {"lost": 0}).items():
        lines.append("METRIC %s %s" % (name, value))
    if digest:
        lines.append("DIGEST " + digest)
    lines.append("MEMORY 100 200 300 400")
    lines.append(extra)
    lines.append("TIER boot 1 0")
    lines.append("TIER gameplay %d %d" % (failed, failed))
    lines.append("CONFORMANCE %d %d" % (1 + failed, failed))
    return "\n".join(lines) + "\n"


def run_result(outcome="pass", p50=100.0, p95=200.0, digests=("a", "a"), metrics=None, growth=1000):
    stats = {"p50": p50, "p95": p95, "p99": p95, "max": p95, "total": p50 * 10}
    return {"outcome": outcome, "rounds": len(digests), "stats": stats, "memory_growth_kb": growth,
            "digests": list(digests), "metrics": metrics or {}, "failed_checks": [], "bench": None}


class ParseAndClassifyTests(unittest.TestCase):
    def test_complete_run_passes(self):
        parsed = bench.parse_bench(output())
        self.assertEqual("pass", bench.classify(parsed, 0, False, 3))
        self.assertEqual(300, parsed["memory"]["growth_kb"])
        self.assertEqual(0.0, parsed["metrics"]["lost"])

    def test_missing_samples_are_incomplete(self):
        parsed = bench.parse_bench(output(samples=None))
        self.assertEqual("incomplete", bench.classify(parsed, 0, False, 3))

    def test_short_samples_are_incomplete(self):
        parsed = bench.parse_bench(output(samples=2))
        self.assertEqual("incomplete", bench.classify(parsed, 0, False, 3))

    def test_duplicate_samples_are_incomplete(self):
        text = output().replace("BUILD_US 10.0", "BUILD_US 10.0\nSAMPLES 1.0 2.0 3.0")
        self.assertEqual("incomplete", bench.classify(bench.parse_bench(text), 0, False, 3))

    def test_missing_digest_is_incomplete(self):
        self.assertEqual("incomplete", bench.classify(bench.parse_bench(output(digest=None)), 0, False, 3))

    def test_zero_checks_is_incomplete(self):
        text = "SAMPLES 1.0 2.0 3.0\nDIGEST 00\nCONFORMANCE 0 0\n"
        self.assertEqual("incomplete", bench.classify(bench.parse_bench(text), 0, False, 3))

    def test_summary_disagreeing_with_records_is_incomplete(self):
        text = output().replace("CONFORMANCE 1 0", "CONFORMANCE 5 0")
        self.assertEqual("incomplete", bench.classify(bench.parse_bench(text), 0, False, 3))

    def test_failed_check_fails(self):
        self.assertEqual("fail", bench.classify(bench.parse_bench(output(failed=1)), 1, False, 3))

    def test_crash_and_timeout(self):
        parsed = bench.parse_bench(output())
        self.assertEqual("crash", bench.classify(parsed, -11, False, 3))
        self.assertEqual("timeout", bench.classify(parsed, None, True, 3))

    def test_unsupported_needs_its_record(self):
        parsed = bench.parse_bench(output(extra="UNSUPPORTED VPhysicsParallelStep001"))
        self.assertEqual("unsupported", bench.classify(parsed, bench.UNSUPPORTED_EXIT, False, 3))
        # Exit status 3 without the record is not a declared absence.
        self.assertEqual("fail", bench.classify(bench.parse_bench(output()), bench.UNSUPPORTED_EXIT, False, 3))

    def test_percentiles(self):
        stats = bench.sample_stats([float(v) for v in range(1, 101)])
        self.assertEqual(51.0, stats["p50"])
        self.assertEqual(95.0, stats["p95"])
        self.assertEqual(100.0, stats["max"])


class RuleTests(unittest.TestCase):
    def test_complete_rejects_any_bad_run(self):
        results = {"p/w/w0": run_result(), "p/x/w0": run_result(outcome="crash")}
        ok, detail = bench.evaluate_rule({"kind": "complete"}, results, {})
        self.assertFalse(ok)
        self.assertIn("crash", detail)
        self.assertFalse(bench.evaluate_rule({"kind": "complete"}, {}, {})[0])

    def test_metric_max_uses_the_worst_run_and_rejects_missing_metrics(self):
        rule = {"kind": "metric_max", "workload": "w", "provider": "p", "metric": "lost", "max": 0}
        results = {"p/w/w0": run_result(metrics={"lost": 0}), "p/w/w4": run_result(metrics={"lost": 2})}
        self.assertFalse(bench.evaluate_rule(rule, results, {})[0])
        self.assertTrue(bench.evaluate_rule(rule, {"p/w/w0": run_result(metrics={"lost": 0})}, {})[0])
        self.assertFalse(bench.evaluate_rule(rule, {"p/w/w0": run_result(metrics={})}, {})[0])
        self.assertIsNone(bench.evaluate_rule(rule, {}, {})[0])

    def test_digest_repeatable(self):
        rule = {"kind": "digest_repeatable", "workload": "w", "provider": "p"}
        self.assertTrue(bench.evaluate_rule(rule, {"p/w/w0": run_result(digests=("a", "a"))}, {})[0])
        self.assertFalse(bench.evaluate_rule(rule, {"p/w/w0": run_result(digests=("a", "b"))}, {})[0])
        # One round cannot show repeatability.
        self.assertFalse(bench.evaluate_rule(rule, {"p/w/w0": run_result(digests=("a",))}, {})[0])

    def test_digest_invariant_across_worker_counts(self):
        rule = {"kind": "digest_invariant", "workload": "w", "provider": "p"}
        same = {"p/w/w1": run_result(digests=("a", "a")), "p/w/w4": run_result(digests=("a", "a"))}
        diff = {"p/w/w1": run_result(digests=("a", "a")), "p/w/w4": run_result(digests=("b", "b"))}
        self.assertTrue(bench.evaluate_rule(rule, same, {})[0])
        self.assertFalse(bench.evaluate_rule(rule, diff, {})[0])
        self.assertIsNone(bench.evaluate_rule(rule, {"p/w/w1": run_result()}, {})[0])

    def test_relative_ratio(self):
        rule = {"kind": "relative", "workload": "w", "stat": "p95", "max_ratio": 1.10,
                "candidate": {"provider": "b", "workers": 0}, "reference": {"provider": "i", "workers": 0}}
        self.assertTrue(bench.evaluate_rule(rule, {"b/w/w0": run_result(p95=100), "i/w/w0": run_result(p95=100)}, {})[0])
        self.assertFalse(bench.evaluate_rule(rule, {"b/w/w0": run_result(p95=120), "i/w/w0": run_result(p95=100)}, {})[0])

    def test_speedup(self):
        rule = {"kind": "speedup", "workload": "w", "provider": "p", "stat": "p50", "base_workers": 1, "workers": 4,
                "min_ratio": 1.5}
        self.assertTrue(bench.evaluate_rule(rule, {"p/w/w1": run_result(p50=300), "p/w/w4": run_result(p50=100)}, {})[0])
        self.assertFalse(bench.evaluate_rule(rule, {"p/w/w1": run_result(p50=120), "p/w/w4": run_result(p50=100)}, {})[0])

    def test_timing_and_memory(self):
        timing = {"kind": "timing", "workload": "w", "provider": "p", "workers": 0, "stat": "p95", "limit_us": 150}
        memory = {"kind": "memory", "workload": "w", "provider": "p", "workers": 0, "limit_kb": 500}
        self.assertFalse(bench.evaluate_rule(timing, {"p/w/w0": run_result(p95=200)}, {})[0])
        self.assertTrue(bench.evaluate_rule(timing, {"p/w/w0": run_result(p95=100)}, {})[0])
        self.assertFalse(bench.evaluate_rule(memory, {"p/w/w0": run_result(growth=900)}, {})[0])

    def test_contract(self):
        rule = {"kind": "contract", "provider": "p"}
        self.assertIsNone(bench.evaluate_rule(rule, {}, {})[0])
        bad = {"outcome": "fail", "failed_checks": ["gameplay parallel.rejects-zero: "]}
        self.assertFalse(bench.evaluate_rule(rule, {}, {"p": bad})[0])
        self.assertTrue(bench.evaluate_rule(rule, {}, {"p": {"outcome": "pass", "failed_checks": []}})[0])


class ContentionTests(unittest.TestCase):
    def test_contention_threshold(self):
        self.assertIsNone(bench.contention([4.0, 8.0], 32, 0.35))
        self.assertIn("per CPU", bench.contention([4.0, 12.0], 32, 0.35))
        self.assertIsNone(bench.contention([100.0], 32, None))
        self.assertIsNone(bench.contention([], 32, 0.35))

    def test_contended_host_skips_only_timing_rules(self):
        timing = {"kind": "timing", "workload": "w", "provider": "p", "workers": 0, "stat": "p95", "limit_us": 150}
        quality = {"kind": "metric_max", "workload": "w", "provider": "p", "metric": "lost", "max": 0}
        results = {"p/w/w0": run_result(p95=100, metrics={"lost": 2})}
        self.assertIsNone(bench.evaluate_rule(timing, results, {}, "load 40")[0])
        self.assertFalse(bench.evaluate_rule(quality, results, {}, "load 40")[0])


class GateTests(unittest.TestCase):
    def budget(self):
        return {
            "schema": bench.BUDGET_SCHEMA, "reference": "i", "providers": ["i", "b"],
            "workloads": {"w": {"scene": "pile", "count": 1, "ticks": 3, "seed": 1, "workers": [0, 4]}},
            "gates": {
                "req": {"state": "required", "owner": "R1", "rules": [
                    {"id": "r.lost", "kind": "metric_max", "workload": "w", "provider": "b", "metric": "lost", "max": 0}]},
                "plan": {"state": "planned", "owner": "R2", "rules": [
                    {"id": "p.tunnel", "kind": "metric_max", "workload": "w", "provider": "b", "metric": "tunneled", "max": 0}]},
            },
            "profiles": {"x": {"status": "measured", "rules": []}},
        }

    def test_planned_gate_failure_does_not_fail_the_run(self):
        budget = self.budget()
        results = {"b/w/w0": run_result(metrics={"lost": 0, "tunneled": 5})}
        verdicts = bench.evaluate_gates(budget, budget["profiles"]["x"], results, {})
        self.assertEqual("pass", verdicts["req"]["verdict"])
        self.assertEqual("fail", verdicts["plan"]["verdict"])
        self.assertEqual([], bench.run_status(verdicts, "measured"))

    def test_required_gate_failure_fails_the_run(self):
        budget = self.budget()
        results = {"b/w/w0": run_result(metrics={"lost": 3, "tunneled": 0})}
        verdicts = bench.evaluate_gates(budget, budget["profiles"]["x"], results, {})
        self.assertEqual(1, len(bench.run_status(verdicts, "measured")))

    def test_unrun_rule_leaves_the_gate_incomplete(self):
        budget = self.budget()
        verdicts = bench.evaluate_gates(budget, budget["profiles"]["x"], {}, {})
        self.assertEqual("incomplete", verdicts["req"]["verdict"])
        self.assertTrue(bench.run_status(verdicts, "measured"))

    def test_known_gap_keeps_the_gate_open_and_a_passing_gap_needs_review(self):
        budget = self.budget()
        budget["gates"]["req"]["rules"][0]["known_gap"] = {"owner": "R1", "note": "tracked"}
        failing = {"b/w/w0": run_result(metrics={"lost": 3, "tunneled": 0})}
        verdicts = bench.evaluate_gates(budget, budget["profiles"]["x"], failing, {})
        self.assertEqual("open", verdicts["req"]["verdict"])
        passing = {"b/w/w0": run_result(metrics={"lost": 0, "tunneled": 0})}
        verdicts = bench.evaluate_gates(budget, budget["profiles"]["x"], passing, {})
        self.assertTrue(any("known gap now passes" in f for f in bench.run_status(verdicts, "measured")))

    def test_unverified_profile_cannot_certify(self):
        budget = self.budget()
        results = {"b/w/w0": run_result(metrics={"lost": 0, "tunneled": 0})}
        verdicts = bench.evaluate_gates(budget, budget["profiles"]["x"], results, {})
        self.assertTrue(bench.run_status(verdicts, "unverified"))

    def test_reference_runs_only_the_parity_path(self):
        budget = self.budget()
        runs = bench.planned_runs(budget, budget["providers"])
        self.assertIn(("i", "w", 0), runs)
        self.assertNotIn(("i", "w", 4), runs)
        self.assertIn(("b", "w", 4), runs)
        budget["workloads"]["w"]["reference"] = False
        self.assertNotIn(("i", "w", 0), bench.planned_runs(budget, budget["providers"]))


class DeclarationTests(unittest.TestCase):
    def load(self, budget):
        with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as stream:
            json.dump(budget, stream)
        return bench.load_budget(stream.name)

    def test_installed_declaration_loads_and_names_real_rules(self):
        budget = bench.load_budget(str(ROOT / bench.DEFAULT_BUDGET))
        for profile in budget["profiles"].values():
            for case in budget["sensitivity"]:
                if profile.get("status") == "measured":
                    bench.rule_by_id(budget, profile, case["rule"])

    def test_rejects_unknown_rule_kind_workload_and_state(self):
        base = bench.load_budget(str(ROOT / bench.DEFAULT_BUDGET))
        bad = copy.deepcopy(base)
        bad["gates"]["box3d-parity"]["rules"].append({"kind": "vibes"})
        with self.assertRaises(bench.BudgetError):
            self.load(bad)
        bad = copy.deepcopy(base)
        bad["gates"]["box3d-parity"]["rules"].append({"kind": "metric_max", "workload": "nope", "provider": "p", "metric": "m", "max": 0})
        with self.assertRaises(bench.BudgetError):
            self.load(bad)
        bad = copy.deepcopy(base)
        bad["gates"]["box3d-parity"]["state"] = "optional"
        with self.assertRaises(bench.BudgetError):
            self.load(bad)
        bad = copy.deepcopy(base)
        bad["gates"]["box3d-parity"]["rules"][1]["known_gap"] = {"note": "no owner"}
        with self.assertRaises(bench.BudgetError):
            self.load(bad)


if __name__ == "__main__":
    unittest.main()
