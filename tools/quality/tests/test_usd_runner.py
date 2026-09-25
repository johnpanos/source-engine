"""Self-tests of tools/quality/corpus/usd_runner.py (no OpenUSD needed).

A fake `pxr` package and fake test modules in a temporary directory stand in
for the pinned toolchain, so these run under any Python 3:

    python3 -m unittest tools/quality/tests/test_usd_runner.py -v

They prove the runner cannot pass silently: skips (module-level and per test),
import errors, setUpClass errors, expected failures, unexpected successes, zero
tests, a missing pxr, a version mismatch, a missing or malformed toolchain and
an unset-but-searched toolchain all fail with a named check.
"""

import json
import os
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
RUNNER = ROOT / "tools/quality/corpus/usd_runner.py"

FAKES = {
    "usdrunner_fake_pass": """
        import os, unittest
        class T(unittest.TestCase):
            def test_one(self):
                self.assertEqual(1 + 1, 2)
            def test_no_ambient_override(self):
                self.assertNotIn("SOURCE_USD_CORPUS_OVERRIDES", os.environ)
    """,
    "usdrunner_fake_override": """
        import os, unittest
        class T(unittest.TestCase):
            def test_override_reaches_the_child(self):
                self.assertEqual(os.environ.get("SOURCE_USD_CORPUS_OVERRIDES"), "st=x.usda")
    """,
    "usdrunner_fake_module_skip": """
        import unittest
        raise unittest.SkipTest("needs pxr")
    """,
    "usdrunner_fake_test_skip": """
        import unittest
        class T(unittest.TestCase):
            def test_skipped(self):
                self.skipTest("not today")
    """,
    "usdrunner_fake_zero": """
        import unittest
    """,
    "usdrunner_fake_import_error": """
        import usdrunner_no_such_module
    """,
    "usdrunner_fake_setup_class": """
        import unittest
        class T(unittest.TestCase):
            @classmethod
            def setUpClass(cls):
                raise RuntimeError("fixture broke")
            def test_never_runs(self):
                pass
    """,
    "usdrunner_fake_expected_failure": """
        import unittest
        class T(unittest.TestCase):
            @unittest.expectedFailure
            def test_known(self):
                self.assertEqual(1, 2)
    """,
    "usdrunner_fake_unexpected_success": """
        import unittest
        class T(unittest.TestCase):
            @unittest.expectedFailure
            def test_known(self):
                pass
    """,
    "usdrunner_fake_failure": """
        import unittest
        class T(unittest.TestCase):
            def test_a_pass(self):
                pass
            def test_b_fail(self):
                self.assertEqual("left", "right")
    """,
    "usdrunner_fake_subtest": """
        import unittest
        class T(unittest.TestCase):
            def test_cases(self):
                for k in range(3):
                    with self.subTest(k=k):
                        self.assertNotEqual(k, 1)
    """,
}


def fake_pxr(directory, version):
    package = directory / "pxr"
    package.mkdir(parents=True, exist_ok=True)
    (package / "__init__.py").write_text("")
    (package / "Usd.py").write_text("def GetVersion():\n    return %r\n" % (version,))


class RunnerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        base = Path(cls.temporary.name)
        cls.good = base / "good"
        cls.old = base / "old"
        cls.empty = base / "empty"
        for directory in (cls.good, cls.old, cls.empty):
            directory.mkdir()
            for name, source in FAKES.items():
                (directory / (name + ".py")).write_text(textwrap.dedent(source))
        fake_pxr(cls.good, (0, 25, 11))
        fake_pxr(cls.old, (0, 24, 11))
        # A pin profile pair for this interpreter: OpenUSD v25.11, this Python.
        openusd = base / "openusd.json"
        openusd.write_text(json.dumps({"dependencies": {"openusd": {
            "tag": "v25.11", "revision": "0" * 40}}}))
        cls.profile = base / "profile.json"
        cls.profile.write_text(json.dumps({
            "schema": "source-host-tool-profile/v1", "id": "usd-runner-self-test",
            "toolchain": {"usd_python": sys.executable,
                          "python": "%d.%d.%d" % sys.version_info[:3]},
            "profiles": {"openusd": str(openusd)}}))
        cls.base = base

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def run_runner(self, modules, pythonpath=None, extra=(), env=None, interpreter=True):
        argv = [sys.executable, str(RUNNER), "--pin-profile", str(self.profile)]
        if interpreter:
            argv += ["--usd-python", sys.executable,
                     "--usd-pythonpath", str(pythonpath or self.good)]
        environment = dict(os.environ, **(env or {}))
        environment.pop("CONFORMANCE_OUT", None)
        out = subprocess.run(argv + list(extra) + list(modules), capture_output=True, text=True,
                             env=environment, cwd=str(ROOT), timeout=120)
        records = [ln for ln in out.stdout.splitlines() if ln.startswith("CONFORMANCE ")]
        fails = [ln for ln in out.stdout.splitlines() if ln.startswith("FAIL")]
        self.assertEqual(len(records), 1, out.stdout + out.stderr)
        self.assertEqual(len([ln for ln in out.stdout.splitlines()
                              if ln.startswith("PROVENANCE ")]), 1)
        return out.returncode, records[0], fails

    def assertFirstFailure(self, result, text):
        code, record, fails = result
        self.assertNotEqual(code, 0, record)
        self.assertTrue(fails, record)
        self.assertIn(text, fails[0])

    def test_passing_module_passes(self):
        code, record, fails = self.run_runner(
            ["usdrunner_fake_pass"], env={"SOURCE_USD_CORPUS_OVERRIDES": "st=leak.usda"})
        self.assertEqual((code, fails), (0, []), record)
        # toolchain, completed, load, 2 tests, tests-ran, 4 interpreter checks
        self.assertEqual(record, "CONFORMANCE 10 0")

    def test_override_is_passed_explicitly(self):
        code, record, fails = self.run_runner(["usdrunner_fake_override"],
                                              extra=["--override", "st=x.usda"])
        self.assertEqual((code, fails), (0, []), record)

    def test_module_skip_fails(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_module_skip"]),
                                "FAIL usdrunner_fake_module_skip.load: skipped at import")

    def test_test_skip_fails(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_test_skip"]),
                                "FAIL usdrunner_fake_test_skip.T.test_skipped: skip: not today")

    def test_zero_tests_fail(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_zero"]),
                                "FAIL usdrunner_fake_zero.tests-ran: zero tests ran")

    def test_import_error_fails(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_import_error"]),
                                "FAIL usdrunner_fake_import_error.load: import/load error")

    def test_setup_class_error_fails(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_setup_class"]),
                                "fixture broke")

    def test_expected_failure_and_unexpected_success_fail(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_expected_failure"]),
                                "expected-failure")
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_unexpected_success"]),
                                "unexpected-success")

    def test_failure_names_the_test(self):
        code, record, fails = self.run_runner(["usdrunner_fake_failure"])
        self.assertEqual(fails, ["FAIL usdrunner_fake_failure.T.test_b_fail: fail: "
                                 "AssertionError: 'left' != 'right' - left + right"])
        self.assertEqual(record, "CONFORMANCE 10 1")

    def test_subtest_failure_fails(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_subtest"]),
                                "FAIL usdrunner_fake_subtest.T.test_cases: fail: (k=1)")

    def test_class_selection(self):
        code, record, fails = self.run_runner(["usdrunner_fake_failure:T"])
        self.assertTrue(fails[0].startswith("FAIL usdrunner_fake_failure.T.test_b_fail: fail"),
                        fails)
        self.assertEqual(record, "CONFORMANCE 10 1")
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_failure:Missing"]),
                                "FAIL usdrunner_fake_failure:Missing.load: import/load error: "
                                "usdrunner_fake_failure has no test case class Missing")

    def test_missing_pxr_fails(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_pass"], pythonpath=self.empty),
                                "FAIL openusd.import: ModuleNotFoundError")

    def test_version_mismatch_fails(self):
        self.assertFirstFailure(self.run_runner(["usdrunner_fake_pass"], pythonpath=self.old),
                                "FAIL openusd.version: expected '0.25.11', got '0.24.11'")

    def write_toolchain(self, name, **values):
        path = self.base / name
        path.write_text(json.dumps(dict({"schema": "pbrt-map-toolchain/v1"}, **values)))
        return path

    def test_toolchain_file_is_used(self):
        path = self.write_toolchain("toolchain.json", usd_python=sys.executable,
                                    usd_pythonpath=str(self.good))
        code, record, fails = self.run_runner(["usdrunner_fake_pass"], interpreter=False,
                                              extra=["--toolchain", str(path)])
        self.assertEqual((code, fails), (0, []), record)

    def test_bad_toolchains_fail(self):
        for name, values, text in (
                ("missing.json", None, "no toolchain file"),
                ("schema.json", {"schema": "other"}, "schema is 'other'"),
                ("keys.json", {}, "usd_python missing"),
                ("paths.json", {"usd_python": "/nonexistent/python", "usd_pythonpath": "/nx"},
                 "does not exist")):
            with self.subTest(name=name):
                path = self.base / name
                if values is not None:
                    path = self.write_toolchain(name, **values)
                result = self.run_runner(["usdrunner_fake_pass"], interpreter=False,
                                         extra=["--toolchain", str(path)])
                self.assertFirstFailure(result, "FAIL toolchain.resolved: ")
                self.assertIn(text, result[2][0])

    def test_environment_toolchain_has_no_fallback(self):
        self.assertFirstFailure(
            self.run_runner(["usdrunner_fake_pass"], interpreter=False,
                            env={"SOURCE_USD_TOOLCHAIN": str(self.base / "absent.json")}),
            "FAIL toolchain.resolved: no toolchain file (searched %s)" % (
                self.base / "absent.json"))


if __name__ == "__main__":
    unittest.main()
