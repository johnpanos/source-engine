#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Self-tests for tools/quality/baseline.py (RFC 0005 Q0 / R01).

The audit is trusted only after it is shown to reject a malformed declaration
and to classify every outcome: a regression, a stale known failure, a crash, a
timeout and an unavailable requirement must each be reported correctly.
"""

import contextlib
import io
import json
import os
import sys
import tempfile
import time
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import baseline  # noqa: E402

PY = sys.executable


def check(ident, argv, outcome="pass", **extra):
    entry = {"id": ident, "group": "static", "domain": "Q-ARCH", "rows": ["R01"], "argv": argv,
        "budget_seconds": 10, "timeout": 30, "baseline": {"outcome": outcome}}
    if outcome != "pass":
        entry["baseline"].update({"reason": "seeded", "owner": "R01"})
    entry.update(extra)
    return entry


def declaration(**sections):
    base = {
        "schema": baseline.SCHEMA,
        "tools": [{"id": "python", "kind": "command", "argv": [PY, "--version"]},
            {"id": "absent", "kind": "command", "argv": ["definitely-not-a-real-tool-r01"]}],
        "content": [{"id": "corpus", "candidates": ["corpus"], "marker": "marker.txt", "redistribution": "test"}],
        "profiles": [],
        "checks": [check("ok", [PY, "-c", "pass"])],
        "baselines": [],
    }
    base.update(sections)
    return base


class BaselineTestCase(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory(prefix="baseline-test-")
        self.root = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def write(self, data):
        path = os.path.join(self.root, "baseline.json")
        with open(path, "w", encoding="utf-8") as stream:
            json.dump(data, stream)
        return path

    def run_main(self, data, *argv):
        path = self.write(data)
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(out):
            code = baseline.main(["--root", self.root, "--declaration", path, *argv])
        return code, out.getvalue()

    def audit(self, data, *argv):
        out_dir = os.path.join(self.root, "evidence")
        code, text = self.run_main(data, "audit", "--out", out_dir, *argv)
        evidence_path = os.path.join(out_dir, "evidence.json")
        evidence = None
        if os.path.exists(evidence_path):
            with open(evidence_path, encoding="utf-8") as stream:
                evidence = json.load(stream)
        return code, text, evidence


class ValidationTests(BaselineTestCase):
    def assertRejected(self, data, fragment):
        code, text = self.run_main(data, "validate")
        self.assertEqual(2, code, text)
        self.assertIn(fragment, text)

    def test_valid_declaration_is_accepted(self):
        code, text = self.run_main(declaration(), "validate")
        self.assertEqual(0, code, text)

    def test_unknown_schema_is_rejected(self):
        data = declaration()
        data["schema"] = "source-baseline/v0"
        self.assertRejected(data, "unsupported schema")

    def test_unknown_tool_requirement_is_rejected(self):
        self.assertRejected(declaration(checks=[check("x", [PY, "-c", "pass"], requires=["tool:nope"])]),
            "unknown tool")

    def test_known_failure_needs_owner_and_reason(self):
        entry = check("x", [PY, "-c", "pass"])
        entry["baseline"] = {"outcome": "fail"}
        self.assertRejected(declaration(checks=[entry]), "missing required field")

    def test_profile_without_file_needs_gap(self):
        profile = {"id": "p", "tier": "north-star", "product": "x", "os": "linux", "arch": "x86_64", "profile": None}
        self.assertRejected(declaration(profiles=[profile]), "gap")

    def test_missing_profile_file_is_rejected(self):
        profile = {"id": "p", "tier": "north-star", "product": "x", "os": "linux", "arch": "x86_64",
            "profile": "quality/profiles/missing.json"}
        self.assertRejected(declaration(profiles=[profile]), "does not exist")

    def test_missing_baseline_record_is_rejected(self):
        entry = {"id": "b", "domain": "Q-JOBS", "kind": "capture", "state": "recorded", "owner": "R10",
            "note": "x", "records": ["RFC/missing.md"]}
        self.assertRejected(declaration(baselines=[entry]), "does not exist")

    def test_recorded_baseline_must_link_a_record(self):
        entry = {"id": "b", "domain": "Q-JOBS", "kind": "budget", "state": "partial", "owner": "R10", "note": "x"}
        self.assertRejected(declaration(baselines=[entry]), "must link")

    def test_duplicate_check_id_is_rejected(self):
        self.assertRejected(declaration(checks=[check("x", [PY, "-c", "pass"]), check("x", [PY, "-c", "pass"])]),
            "duplicate")


class AuditTests(BaselineTestCase):
    def test_matching_outcomes_pass_and_write_evidence(self):
        data = declaration(checks=[check("ok", [PY, "-c", "pass"]),
            check("known", [PY, "-c", "raise SystemExit(3)"], outcome="fail")])
        code, text, evidence = self.audit(data)
        self.assertEqual(0, code, text)
        self.assertEqual(baseline.EVIDENCE_SCHEMA, evidence["schema"])
        self.assertEqual("pass", evidence["decision"])
        self.assertEqual({"pass": 1, "fail": 1}, {k: v for k, v in evidence["counts"].items() if v})
        self.assertTrue(os.path.exists(evidence["checks"]["known"]["log"]))

    def test_regression_is_a_deviation(self):
        code, text, evidence = self.audit(declaration(checks=[check("ok", [PY, "-c", "raise SystemExit(1)"])]))
        self.assertEqual(1, code)
        self.assertEqual(["ok"], evidence["deviations"])
        self.assertIn("regression", evidence["checks"]["ok"]["deviation"])

    def test_known_failure_that_now_passes_is_a_deviation(self):
        code, text, evidence = self.audit(declaration(checks=[check("stale", [PY, "-c", "pass"], outcome="fail")]))
        self.assertEqual(1, code)
        self.assertIn("now passes", evidence["checks"]["stale"]["deviation"])

    def test_crash_is_classified_by_signal(self):
        crash = [PY, "-c", "import os, signal; os.kill(os.getpid(), signal.SIGSEGV)"]
        code, text, evidence = self.audit(declaration(checks=[check("c", crash, outcome="crash")]))
        self.assertEqual(0, code, text)
        self.assertEqual("crash", evidence["checks"]["c"]["outcome"])
        self.assertEqual(11, evidence["checks"]["c"]["signal"])

    def test_crash_recorded_as_pass_is_a_regression(self):
        crash = [PY, "-c", "import os, signal; os.kill(os.getpid(), signal.SIGABRT)"]
        code, text, evidence = self.audit(declaration(checks=[check("c", crash)]))
        self.assertEqual(1, code)
        self.assertIn("observed crash", evidence["checks"]["c"]["deviation"])

    def test_timeout_is_classified(self):
        hang = check("hang", [PY, "-c", "import time; time.sleep(30)"], outcome="timeout")
        hang["timeout"] = 1
        code, text, evidence = self.audit(declaration(checks=[hang]))
        self.assertEqual(0, code, text)
        self.assertEqual("timeout", evidence["checks"]["hang"]["outcome"])

    def test_unavailable_requirement_is_reported_never_passed(self):
        data = declaration(checks=[
            check("needs-tool", [PY, "-c", "pass"], requires=["tool:absent"]),
            check("needs-content", [PY, "-c", "pass"], requires=["content:corpus"]),
            check("needs-path", [PY, "-c", "pass"], requires=["path:build-missing/config.log"])])
        code, text, evidence = self.audit(data)
        self.assertEqual(0, code, text)
        self.assertEqual(3, evidence["counts"]["unavailable"])
        self.assertEqual(0, evidence["counts"]["pass"])
        code, text, evidence = self.audit(data, "--strict")
        self.assertEqual(1, code)
        self.assertEqual("fail", evidence["decision"])

    def test_available_content_path_is_substituted(self):
        os.makedirs(os.path.join(self.root, "corpus"))
        open(os.path.join(self.root, "corpus", "marker.txt"), "w").close()
        probe = [PY, "-c", "import os, sys; sys.exit(0 if os.path.isfile(sys.argv[1]) else 1)",
            "{content:corpus}/marker.txt"]
        code, text, evidence = self.audit(declaration(checks=[check("c", probe, requires=["content:corpus"])]))
        self.assertEqual(0, code, text)
        self.assertEqual("pass", evidence["checks"]["c"]["outcome"])

    def test_zero_selected_checks_is_an_error(self):
        code, text, _ = self.audit(declaration(), "--group", "no-such-group")
        self.assertEqual(2, code)
        self.assertIn("zero checks", text)

    def test_unmatched_check_selector_is_an_error(self):
        code, text, _ = self.audit(declaration(), "--check", "missing")
        self.assertEqual(2, code)
        self.assertIn("unmatched", text)

    def test_profile_facts_are_read_from_the_profile_file(self):
        os.makedirs(os.path.join(self.root, "profiles"))
        with open(os.path.join(self.root, "profiles", "p.json"), "w", encoding="utf-8") as stream:
            json.dump({"schema": "x/v1", "evidence": {"status": "unverified"}}, stream)
        profile = {"id": "p", "tier": "north-star", "product": "x", "os": "linux", "arch": "x86_64",
            "profile": "profiles/p.json", "requires": ["tool:absent"], "checks": ["ok"]}
        code, text, evidence = self.audit(declaration(profiles=[profile]))
        self.assertEqual(0, code, text)
        facts = evidence["profiles"][0]
        self.assertEqual("unverified", facts["profile_facts"]["evidence_status"])
        self.assertEqual(["tool:absent"], facts["host_requirements_missing"])
        self.assertEqual({"ok": "pass"}, facts["checks"])


def sleeper(ident, seconds, stamp_dir, **extra):
    """A check that records its start and end times, then sleeps."""
    code = ("import time,sys; p=sys.argv[1]; open(p+'.start','w').write(repr(time.time())); "
        "time.sleep(%r); open(p+'.end','w').write(repr(time.time()))" % seconds)
    return check(ident, [PY, "-c", code, os.path.join(stamp_dir, ident)], **extra)


def stamps(stamp_dir, ident):
    with open(os.path.join(stamp_dir, ident + ".start")) as a, \
            open(os.path.join(stamp_dir, ident + ".end")) as b:
        return float(a.read()), float(b.read())


class BudgetValidationTests(BaselineTestCase):
    def assertRejected(self, data, fragment):
        code, text = self.run_main(data, "validate")
        self.assertEqual(2, code, text)
        self.assertIn(fragment, text)

    def test_every_check_needs_a_budget(self):
        entry = check("x", [PY, "-c", "pass"])
        del entry["budget_seconds"]
        self.assertRejected(declaration(checks=[entry]), "budget_seconds")

    def test_budget_must_be_positive(self):
        self.assertRejected(declaration(checks=[check("x", [PY, "-c", "pass"], budget_seconds=0)]),
            "budget_seconds")

    def test_timeout_far_above_budget_needs_a_reason(self):
        loose = check("x", [PY, "-c", "pass"], budget_seconds=2, timeout=600)
        self.assertRejected(declaration(checks=[loose]), "timeout_reason")
        loose["timeout_reason"] = "a from-scratch build"
        code, text = self.run_main(declaration(checks=[loose]), "validate")
        self.assertEqual(0, code, text)

    def test_default_timeout_is_three_budgets_with_a_floor(self):
        self.assertEqual(30, baseline.check_timeout({"budget_seconds": 10}))
        self.assertEqual(10, baseline.check_timeout({"budget_seconds": 0.5}))
        self.assertEqual(7, baseline.check_timeout({"budget_seconds": 100, "timeout": 7}))

    def test_after_must_name_a_known_check(self):
        self.assertRejected(declaration(checks=[check("x", [PY, "-c", "pass"], after=["nope"])]),
            "unknown check")

    def test_after_cycle_is_rejected(self):
        self.assertRejected(declaration(checks=[
            check("a", [PY, "-c", "pass"], after=["b"]),
            check("b", [PY, "-c", "pass"], after=["a"])]), "cycle")

    def test_concurrent_check_cannot_follow_a_serial_one(self):
        self.assertRejected(declaration(checks=[
            check("s", [PY, "-c", "pass"], schedule="serial"),
            check("p", [PY, "-c", "pass"], after=["s"])]), "cannot run after serial")

    def test_unknown_schedule_is_rejected(self):
        self.assertRejected(declaration(checks=[check("x", [PY, "-c", "pass"], schedule="later")]),
            "schedule")


class SchedulingTests(BaselineTestCase):
    def setUp(self):
        super().setUp()
        self.stamps = os.path.join(self.root, "stamps")
        os.makedirs(self.stamps)

    def test_independent_checks_run_concurrently(self):
        checks = [sleeper("c%d" % i, 1.0, self.stamps) for i in range(4)]
        started = time.monotonic()
        code, text, evidence = self.audit(declaration(checks=checks), "--parallel", "4")
        self.assertEqual(0, code, text)
        self.assertLess(time.monotonic() - started, 3.0, "four 1 s checks did not overlap")
        self.assertEqual(4, evidence["parallel"])

    def test_after_waits_for_its_dependency(self):
        checks = [sleeper("build", 0.8, self.stamps),
            sleeper("run", 0.1, self.stamps, after=["build"]),
            sleeper("other", 0.1, self.stamps)]
        code, text, _ = self.audit(declaration(checks=checks), "--parallel", "3")
        self.assertEqual(0, code, text)
        self.assertGreaterEqual(stamps(self.stamps, "run")[0], stamps(self.stamps, "build")[1])
        self.assertLess(stamps(self.stamps, "other")[0], stamps(self.stamps, "build")[1])

    def test_serial_checks_run_alone_after_the_rest(self):
        checks = [sleeper("p1", 0.5, self.stamps), sleeper("p2", 0.5, self.stamps),
            sleeper("s1", 0.3, self.stamps, schedule="serial"),
            sleeper("s2", 0.3, self.stamps, schedule="serial")]
        code, text, _ = self.audit(declaration(checks=checks), "--parallel", "4")
        self.assertEqual(0, code, text)
        last_parallel_end = max(stamps(self.stamps, i)[1] for i in ("p1", "p2"))
        s1, s2 = stamps(self.stamps, "s1"), stamps(self.stamps, "s2")
        self.assertGreaterEqual(s1[0], last_parallel_end)
        self.assertGreaterEqual(s2[0], s1[1])

    def test_evidence_keeps_declaration_order(self):
        checks = [sleeper("slow", 0.6, self.stamps), sleeper("fast", 0.0, self.stamps)]
        code, text, evidence = self.audit(declaration(checks=checks), "--parallel", "2")
        self.assertEqual(0, code, text)
        self.assertEqual(["slow", "fast"], list(evidence["checks"]))

    def test_zero_parallel_is_an_error(self):
        code, text = self.run_main(declaration(), "audit", "--out", "-", "--parallel", "0")
        self.assertEqual(2, code, text)


class BudgetTests(BaselineTestCase):
    def test_over_budget_is_reported_and_fails_only_when_strict(self):
        slow = check("slow", [PY, "-c", "import time; time.sleep(0.4)"], budget_seconds=0.1, timeout=10)
        code, text, evidence = self.audit(declaration(checks=[slow]))
        self.assertEqual(0, code, text)
        self.assertEqual(["slow"], evidence["over_budget"])
        self.assertTrue(evidence["checks"]["slow"]["over_budget"])
        self.assertIn("SLOW", text)
        code, text, evidence = self.audit(declaration(checks=[slow]), "--strict-budget")
        self.assertEqual(1, code, text)

    def test_within_budget_is_not_flagged(self):
        code, text, evidence = self.audit(declaration())
        self.assertEqual(0, code, text)
        self.assertEqual([], evidence["over_budget"])

    def test_timeout_kills_descendants_holding_the_output(self):
        # The check starts a child that keeps stdout open, then hangs itself.
        # The child would leave a marker if it outlived the timeout.
        marker = os.path.join(self.root, "orphan-ran")
        hang = check("hang", ["sh", "-c", "(sleep 2; touch %s) & sleep 60" % marker],
            outcome="timeout", budget_seconds=0.1, timeout=1)
        started = time.monotonic()
        code, text, evidence = self.audit(declaration(checks=[hang]))
        self.assertLess(time.monotonic() - started, 5.0, "the timeout did not bound the check")
        self.assertEqual("timeout", evidence["checks"]["hang"]["outcome"])
        time.sleep(2.5)
        self.assertFalse(os.path.exists(marker), "a descendant survived the timeout")

    def test_exited_check_is_not_held_by_a_background_child(self):
        marker = os.path.join(self.root, "orphan-ran")
        quick = check("quick", ["sh", "-c", "(sleep 2; touch %s) & echo done" % marker],
            budget_seconds=1, timeout=10)
        started = time.monotonic()
        code, text, evidence = self.audit(declaration(checks=[quick]))
        self.assertEqual(0, code, text)
        self.assertLess(time.monotonic() - started, 5.0, "the audit waited on a background child")
        self.assertEqual("pass", evidence["checks"]["quick"]["outcome"])
        time.sleep(2.5)
        self.assertFalse(os.path.exists(marker), "a check left a process running")


class RepositoryDeclarationTests(unittest.TestCase):
    def test_checked_in_declaration_is_valid(self):
        root = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
        baseline.load(root, os.path.join(root, "quality", "baseline.json"))


if __name__ == "__main__":
    unittest.main()
