#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Self-tests for the shared conformance runner (RFC 0005 Q1 / roadmap R02).
#
# These prove the runner's outcome handling before it is trusted as a gate: it
# must detect suites that pass, fail, crash, hang, fail to compile, and go
# missing, and it must fail on zero discovery, unmatched selectors, and an
# unsupported schema. Run:
#
#   python3 -m unittest discover -s tools/quality/tests -v
#
# ============================================================================

import json
import os
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY_DIR = os.path.dirname(HERE)
sys.path.insert(0, QUALITY_DIR)

import conformance  # noqa: E402

REPO = conformance.repo_root()
CPP = "tools/quality/tests/fixtures/cpp"
PROFILES_DIR = "tools/quality/tests/fixtures/profiles"
CXX = os.environ.get("CXX", "g++")


def make_suite(sid, sources, expect="pass", kind="self-test", extra_flags=None,
               timeout=None):
    s = {
        "id": sid,
        "domain": "Q-SELFTEST",
        "rfc": "0005",
        "kind": kind,
        "profile": "self-test",
        "sources": [CPP + "/" + x for x in sources],
        "expect": expect,
    }
    if extra_flags:
        s["extra_flags"] = extra_flags
    if timeout is not None:
        s["timeout_seconds"] = timeout
    return s


class RunSuiteClassificationTest(unittest.TestCase):
    """run_suite() must classify every outcome correctly."""

    @classmethod
    def setUpClass(cls):
        cls.profile = conformance.load_profile(
            os.path.join(REPO, PROFILES_DIR), "self-test")
        cls.build_dir = tempfile.mkdtemp(prefix="conf-selftest-")

    def _run(self, suite):
        return conformance.run_suite(REPO, CXX, self.profile, suite, self.build_dir)

    def test_pass(self):
        r = self._run(make_suite("pass", ["pass.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)
        self.assertTrue(r["matched"])
        self.assertTrue(r["build_ok"])

    def test_failing_is_detected(self):
        # A failing suite declared as expect=pass must be reported as a mismatch.
        r = self._run(make_suite("failing", ["failing.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertFalse(r["matched"])
        self.assertIn("FAIL", r["first_divergence"] or "")

    def test_crash_is_detected(self):
        r = self._run(make_suite("crash", ["crash.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_CRASH)
        self.assertFalse(r["matched"])
        self.assertIsNotNone(r["signal"])

    def test_timeout_is_detected(self):
        r = self._run(make_suite("hang", ["hang.cpp"], timeout=2))
        self.assertEqual(r["outcome"], conformance.OUTCOME_TIMEOUT)
        self.assertFalse(r["matched"])

    def test_compile_error_is_detected(self):
        r = self._run(make_suite("broken", ["compile_error.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_COMPILE_ERROR)
        self.assertFalse(r["matched"])
        self.assertFalse(r["build_ok"])

    def test_missing_source_is_detected(self):
        r = self._run(make_suite("gone", ["does_not_exist.cpp"]))
        self.assertEqual(r["outcome"], conformance.OUTCOME_MISSING_SOURCE)
        self.assertFalse(r["matched"])

    def test_expect_fail_matches_a_failing_suite(self):
        # A suite declared expect=fail that fails is a MATCH (self-test semantics).
        r = self._run(make_suite("neg", ["failing.cpp"], expect="fail"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertTrue(r["matched"])

    def test_expect_fail_flags_a_passing_suite(self):
        # A suite declared expect=fail that PASSES is a mismatch.
        r = self._run(make_suite("neg2", ["pass.cpp"], expect="fail"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)
        self.assertFalse(r["matched"])

    def test_sensitivity_ok(self):
        r = self._run(make_suite("sens", ["sensitivity_ok.cpp"], kind="sensitivity"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_PASS)
        self.assertTrue(r["matched"])

    def test_vacuous_sensitivity_is_flagged(self):
        r = self._run(make_suite("sensv", ["sensitivity_vacuous.cpp"], kind="sensitivity"))
        self.assertEqual(r["outcome"], conformance.OUTCOME_FAIL)
        self.assertFalse(r["matched"])


class ManifestValidationTest(unittest.TestCase):
    """Structural manifest problems are fatal."""

    def _write(self, obj):
        f = tempfile.NamedTemporaryFile(
            "w", suffix=".json", delete=False, dir=self.tmp)
        json.dump(obj, f)
        f.close()
        return f.name

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="conf-manifest-")

    def test_unknown_schema_rejected(self):
        path = self._write({"schema": "bogus/v9", "suites": []})
        with self.assertRaises(conformance.ManifestError):
            conformance.load_manifest(path)

    def test_duplicate_id_rejected(self):
        path = self._write({
            "schema": conformance.MANIFEST_SCHEMA,
            "suites": [make_suite("dup", ["pass.cpp"]), make_suite("dup", ["pass.cpp"])],
        })
        with self.assertRaises(conformance.ManifestError):
            conformance.load_manifest(path)

    def test_invalid_expect_rejected(self):
        bad = make_suite("x", ["pass.cpp"])
        bad["expect"] = "sometimes"
        path = self._write({"schema": conformance.MANIFEST_SCHEMA, "suites": [bad]})
        with self.assertRaises(conformance.ManifestError):
            conformance.load_manifest(path)


class EndToEndTest(unittest.TestCase):
    """Drive the full `check` command through main()."""

    def _check(self, suites, extra_args=None):
        f = tempfile.NamedTemporaryFile("w", suffix=".json", delete=False)
        json.dump(
            {"schema": conformance.MANIFEST_SCHEMA,
             "profiles_dir": PROFILES_DIR, "suites": suites}, f)
        f.close()
        args = ["--root", REPO, "--manifest", f.name, "check", "--cxx", CXX,
                "--out", "-", "--build-dir", tempfile.mkdtemp(prefix="conf-e2e-")]
        if extra_args:
            args += extra_args
        try:
            return conformance.main(args)
        finally:
            os.unlink(f.name)

    def test_all_pass_returns_zero(self):
        self.assertEqual(self._check([make_suite("p", ["pass.cpp"])]), 0)

    def test_any_fail_returns_one(self):
        rc = self._check([make_suite("p", ["pass.cpp"]),
                          make_suite("f", ["failing.cpp"])])
        self.assertEqual(rc, 1)

    def test_zero_discovery_is_fatal(self):
        self.assertEqual(self._check([]), 2)

    def test_unmatched_selector_is_fatal(self):
        rc = self._check([make_suite("p", ["pass.cpp"])],
                         extra_args=["--suite", "nope"])
        self.assertEqual(rc, 2)

    def test_selector_runs_only_matching(self):
        rc = self._check([make_suite("p", ["pass.cpp"]),
                          make_suite("f", ["failing.cpp"])],
                         extra_args=["--suite", "p"])
        self.assertEqual(rc, 0)  # the failing suite is filtered out


if __name__ == "__main__":
    unittest.main()
