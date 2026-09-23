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
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import baseline  # noqa: E402

PY = sys.executable


def check(ident, argv, outcome="pass", **extra):
    entry = {"id": ident, "group": "static", "domain": "Q-ARCH", "rows": ["R01"], "argv": argv,
        "timeout": 30, "baseline": {"outcome": outcome}}
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


class RepositoryDeclarationTests(unittest.TestCase):
    def test_checked_in_declaration_is_valid(self):
        root = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
        baseline.load(root, os.path.join(root, "quality", "baseline.json"))


if __name__ == "__main__":
    unittest.main()
