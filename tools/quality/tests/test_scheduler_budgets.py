"""Positive and negative fixtures for tools/quality/scheduler_budgets.py."""

import copy
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import scheduler_budgets as sb  # noqa: E402

BENCH_OUTPUT = """\
bench shared_fanout_4096        median     2038.4 us  min     1900.2  p90     2300.0  (n=15, workers=3)
bench wake_latency              median       22.0 us  min       18.1  p90       30.3  (n=15, workers=3)
CONFORMANCE 90 0
"""

HOSTFRAME_OUTPUT = """\
coverage: threaded 1 unthreaded 1
overhead: 16 phases: graph 112 ns/frame, direct loop 5 ns/frame (6.7 ns/phase added)
overhead: 31 phases: graph 197 ns/frame, direct loop 9 ns/frame (6.1 ns/phase added)
CONFORMANCE 4152 0
"""


def minimal():
    return {
        "schema": sb.SCHEMA,
        "profiles": {
            "desk": {
                "capacity": {"owner": "R20", "pools": {"Glob": 3, "IOJob": 4},
                             "total_pool_workers": 9},
                "overhead": {
                    "fanout": {"owner": "R20", "kind": "pool_bench",
                               "workload": "shared_fanout_4096", "workers": 3, "limit": 3000},
                    "node": {"owner": "R10", "kind": "graph_node_ns", "phases": 31, "limit": 20},
                },
            },
            "phone": {"status": "unverified", "owner": "R29"},
            "laptop": {"inherits": "desk"},
        },
    }


class DeclarationTests(unittest.TestCase):
    def test_repository_declaration_passes(self):
        self.assertEqual(sb.check(sb.load()), [])

    def test_minimal_declaration_passes(self):
        self.assertEqual(sb.check(minimal()), [])

    def test_rejects_missing_owner_bad_limit_and_unknown_kind(self):
        budgets = minimal()
        row = budgets["profiles"]["desk"]["overhead"]["fanout"]
        del row["owner"]
        row["limit"] = -1
        budgets["profiles"]["desk"]["overhead"]["node"]["kind"] = "wall_clock"
        problems = sb.check(budgets)
        self.assertTrue(any("no owner" in p for p in problems))
        self.assertTrue(any("limit must be positive" in p for p in problems))
        self.assertTrue(any("unknown kind" in p for p in problems))

    def test_rejects_pools_over_total_and_unowned_unverified(self):
        budgets = minimal()
        budgets["profiles"]["desk"]["capacity"]["total_pool_workers"] = 5
        del budgets["profiles"]["phone"]["owner"]
        problems = sb.check(budgets)
        self.assertTrue(any("exceed total_pool_workers" in p for p in problems))
        self.assertTrue(any("unverified profile has no owner" in p for p in problems))

    def test_rejects_inheritance_cycle(self):
        budgets = minimal()
        budgets["profiles"]["desk"] = {"inherits": "laptop"}
        self.assertTrue(any("cycle" in p for p in sb.check(budgets)))


class MeasurementTests(unittest.TestCase):
    def test_parse_and_evaluate_pass(self):
        rows = minimal()["profiles"]["desk"]["overhead"]
        bench = sb.parse_bench(BENCH_OUTPUT)
        graph = sb.parse_graph_overhead(HOSTFRAME_OUTPUT)
        self.assertEqual(bench[("shared_fanout_4096", 3)], 2038.4)
        self.assertAlmostEqual(graph[31], (197 - 9) / 31.0)
        results = sb.evaluate(rows, bench, graph)
        self.assertEqual({r["status"] for r in results.values()}, {"pass"})

    def test_over_limit_and_missing_fail(self):
        rows = copy.deepcopy(minimal()["profiles"]["desk"]["overhead"])
        rows["fanout"]["limit"] = 1000
        rows["node"]["phases"] = 44  # not measured
        results = sb.evaluate(rows, sb.parse_bench(BENCH_OUTPUT),
                              sb.parse_graph_overhead(HOSTFRAME_OUTPUT))
        self.assertEqual(results["fanout"]["status"], "fail")
        self.assertEqual(results["node"]["status"], "missing")

    def test_bench_line_needs_its_worker_count(self):
        bench = sb.parse_bench(BENCH_OUTPUT.replace("workers=3", "workers=1"))
        rows = minimal()["profiles"]["desk"]["overhead"]
        self.assertEqual(sb.evaluate(rows, bench, {31: 1.0})["fanout"]["status"], "missing")

    def test_census(self):
        capacity = minimal()["profiles"]["desk"]["capacity"]
        ok = sb.evaluate_census(capacity, {"Glob": 3, "IOJob": 4, "hl2_launcher": 1})
        self.assertEqual({r["status"] for r in ok.values()}, {"pass"})
        over = sb.evaluate_census(capacity, {"Glob": 3, "IOJob": 32})
        self.assertEqual(over["IOJob"]["status"], "fail")
        self.assertEqual(over["total_pool_workers"]["status"], "fail")


if __name__ == "__main__":
    unittest.main()
