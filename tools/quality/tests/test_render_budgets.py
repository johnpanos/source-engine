"""Seeded fixtures for tools/quality/render_budgets.py: each defect must be reported."""
import copy
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import render_budgets

LIMITS = {"max_p50_ms": 8.0, "max_p95_ms": 10.0, "max_p99_ms": 16.0, "max_gpu_render_p99_ms": 8.0,
          "max_submission_p99_ms": 8.0, "max_hitches": 0}
RECORD = {"p50_ms": 4.0, "p95_ms": 5.0, "p99_ms": 6.0, "gpu_render_median_ms": 1.0,
          "gpu_render_p99_ms": 1.5, "submission_median_ms": 3.0, "submission_p99_ms": 3.5, "hitches": 0,
          "revision": "abc", "measured": "2026-09-26", "evidence_command": ["x"]}


def evidence(median=4.0, p99=6.0, hitches=0):
    summary = {"frames": 600, "median_ms": median, "p95_ms": 5.0, "p99_ms": p99,
               "gpu_render_median_ms": 1.0, "gpu_render_p99_ms": 1.5, "submission_median_ms": 3.0,
               "submission_p99_ms": 3.5}
    return {"analysis": {"passes": [{"summary": summary, "hitch_count": hitches}]},
            "source": {"revision": "abc", "dirty": False}, "started_utc": "2026-09-26", "command": ["x"]}


class RenderBudgetsTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        for name in ("p.json", "w.json"):
            (self.root / name).write_text("{}")
        row = {"id": "desk", "profile": "p.json", "workload": "w.json", "hardware": "gpu", "set": "before",
               "presentation": "headless", "modes": {"headroom": dict(LIMITS)}, "k0_record": dict(RECORD)}
        self.budgets = {"schema": "source-render-budgets/v1", "allowance": {"median": 1.05, "p99": 1.10},
                        "rows": [row]}

    def check(self, budgets=None):
        return render_budgets.check(budgets or self.budgets, self.root, ("desk",))

    def test_complete_row_passes(self):
        self.assertEqual(self.check(), [])

    def test_missing_required_row_fails(self):
        self.assertTrue(render_budgets.check(self.budgets, self.root, ("desk", "fold7")))

    def test_missing_limit_fails(self):
        budgets = copy.deepcopy(self.budgets)
        del budgets["rows"][0]["modes"]["headroom"]["max_p95_ms"]
        self.assertTrue(any("max_p95_ms" in p for p in self.check(budgets)))

    def test_missing_record_fails(self):
        budgets = copy.deepcopy(self.budgets)
        del budgets["rows"][0]["k0_record"]
        self.assertTrue(any("k0_record" in p for p in self.check(budgets)))

    def test_undeclared_miss_fails(self):
        budgets = copy.deepcopy(self.budgets)
        budgets["rows"][0]["k0_record"]["submission_p99_ms"] = 9.0
        self.assertTrue(any("not declared" in p for p in self.check(budgets)))

    def test_declared_miss_passes(self):
        budgets = copy.deepcopy(self.budgets)
        budgets["rows"][0]["k0_record"]["submission_p99_ms"] = 9.0
        budgets["rows"][0]["over_budget"] = {"fields": ["submission_p99_ms"], "owner": "R32", "reason": "slow"}
        self.assertEqual(self.check(budgets), [])

    def test_stale_or_unowned_declaration_fails(self):
        budgets = copy.deepcopy(self.budgets)
        budgets["rows"][0]["over_budget"] = {"fields": ["p99_ms"], "owner": "someone", "reason": ""}
        problems = self.check(budgets)
        self.assertTrue(any("no longer exceeds" in p for p in problems))
        self.assertTrue(any("roadmap-row owner" in p for p in problems))

    def test_missing_profile_fails(self):
        budgets = copy.deepcopy(self.budgets)
        budgets["rows"][0]["profile"] = "absent.json"
        self.assertTrue(any("does not exist" in p for p in self.check(budgets)))

    def test_record_from_evidence(self):
        record = render_budgets.record_from(evidence())
        self.assertEqual(record["p50_ms"], 4.0)
        self.assertEqual(record["hitches"], 0)

    def test_report_within_allowance_passes(self):
        _, failures = render_budgets.report(self.budgets["rows"][0], evidence(median=4.1),
                                            self.budgets["allowance"])
        self.assertEqual(failures, [])

    def test_report_over_allowance_fails(self):
        _, failures = render_budgets.report(self.budgets["rows"][0], evidence(median=4.3),
                                            self.budgets["allowance"])
        self.assertTrue(any("frame allowance" in f for f in failures))

    def test_report_over_limit_and_hitches_fail(self):
        _, failures = render_budgets.report(self.budgets["rows"][0], evidence(p99=20.0, hitches=2),
                                            self.budgets["allowance"])
        self.assertEqual(len(failures), 3)  # p99 limit, hitches, allowance

    def test_maximum_frame_is_enforced_independently_of_p99(self):
        row = copy.deepcopy(self.budgets["rows"][0])
        row["modes"]["headroom"]["max_frame_ms"] = 8.0
        run = evidence()
        run["analysis"]["passes"][0]["summary"]["max_ms"] = 9.0
        _, failures = render_budgets.report(row, run, self.budgets["allowance"])
        self.assertTrue(any("max_frame_ms" in failure for failure in failures))

    def test_missing_and_nonfinite_maximum_cannot_pass(self):
        row = copy.deepcopy(self.budgets["rows"][0])
        row["modes"]["headroom"]["max_frame_ms"] = 8.0
        for value in (None, float("nan"), float("inf"), -1):
            with self.subTest(value=value):
                run = evidence()
                run["analysis"]["passes"][0]["summary"]["max_ms"] = value
                _, failures = render_budgets.report(row, run, self.budgets["allowance"])
                self.assertTrue(any("no valid max_frame_ms" in failure for failure in failures))

    def test_hard_floor_cannot_have_looser_maximum_or_missing_conditions(self):
        row = self.budgets["rows"][0]
        row.update(acceptance="hard", minimum_fps=120, baseline_required=False)
        row["modes"]["headroom"]["max_frame_ms"] = 16.0
        problems = self.check()
        self.assertTrue(any("max_frame_ms must meet" in problem for problem in problems))
        self.assertTrue(any("resolution, quality and device" in problem for problem in problems))


if __name__ == "__main__":
    unittest.main()
