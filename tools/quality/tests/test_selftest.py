#!/usr/bin/env python3
"""Self-tests for tools/quality/selftest.py, the parallel tiered runner.

Each test writes a throwaway test directory and runs the runner on it, so the
runner's own rules are pinned: tier selection, parallel classes, chunking,
failures, import errors, module skips, zero discovery, worker timeouts and
per-test budgets. Every case finishes in a few seconds.
"""

import contextlib
import io
import json
import os
import shutil
import sys
import tempfile
import textwrap
import time
import unittest

QUALITY_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, QUALITY_DIR)

import selftest  # noqa: E402

HEADER = "import sys, time, unittest\nsys.path.insert(0, %r)\nfrom selftest import slow\n" % QUALITY_DIR


class RunnerTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp(prefix="selftest-test-")
        self.addCleanup(shutil.rmtree, self.dir, True)

    def module(self, name, body):
        with open(os.path.join(self.dir, name), "w", encoding="utf-8") as handle:
            handle.write(HEADER + textwrap.dedent(body))

    def run_runner(self, *argv):
        out = io.StringIO()
        report = os.path.join(self.dir, "report.json")
        started = time.monotonic()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(out):
            code = selftest.main(["--start", self.dir, "--json", report, *argv])
        elapsed = time.monotonic() - started
        data = None
        if os.path.exists(report):
            with open(report, encoding="utf-8") as handle:
                data = json.load(handle)
        return code, out.getvalue(), data, elapsed

    def listed(self, *argv):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            selftest.main(["--start", self.dir, "--list", *argv])
        return sorted(out.getvalue().split())

    TIERED = """
        class T(unittest.TestCase):
            def test_quick(self):
                pass

            @slow
            def test_heavy(self):
                pass

        @slow
        class Heavy(unittest.TestCase):
            def test_all_of_it(self):
                pass
        """

    def test_fast_tier_excludes_slow_tests(self):
        self.module("test_tiers.py", self.TIERED)
        self.assertEqual(["test_tiers.T.test_quick"], self.listed())

    def test_slow_tier_is_only_slow_tests(self):
        self.module("test_tiers.py", self.TIERED)
        self.assertEqual(["test_tiers.Heavy.test_all_of_it", "test_tiers.T.test_heavy"],
                         self.listed("--tier", "slow"))

    def test_all_tier_is_everything(self):
        self.module("test_tiers.py", self.TIERED)
        self.assertEqual(3, len(self.listed("--tier", "all")))

    def test_classes_run_in_parallel(self):
        for index in range(4):
            self.module("test_p%d.py" % index, """
                class P(unittest.TestCase):
                    def test_wait(self):
                        time.sleep(1.0)
                """)
        code, text, data, elapsed = self.run_runner("--jobs", "4")
        self.assertEqual(0, code, text)
        self.assertEqual(4, data["ran"])
        self.assertLess(elapsed, 3.0, "four 1 s classes did not overlap")

    def test_large_class_is_split_into_chunks(self):
        self.module("test_big.py", "class Big(unittest.TestCase):\n" + "".join(
            "    def test_%d(self):\n        time.sleep(0.5)\n" % i for i in range(6)))
        code, text, data, elapsed = self.run_runner("--jobs", "6", "--chunk", "2")
        self.assertEqual(0, code, text)
        self.assertEqual(6, data["ran"])
        self.assertIn("in 3 group(s)", text)
        self.assertLess(elapsed, 2.5, "chunks of one class did not overlap")

    def test_failure_fails_the_run(self):
        self.module("test_f.py", """
            class F(unittest.TestCase):
                def test_ok(self):
                    pass

                def test_bad(self):
                    self.fail("seeded failure")
            """)
        code, text, data, _ = self.run_runner()
        self.assertEqual(1, code)
        self.assertEqual(1, data["failures"])
        self.assertIn("seeded failure", text)

    def test_import_error_is_reported_not_dropped(self):
        self.module("test_ok.py", """
            class A(unittest.TestCase):
                def test_ok(self):
                    pass
            """)
        self.module("test_broken.py", "import no_such_module_for_selftest\n")
        code, text, data, _ = self.run_runner()
        self.assertEqual(1, code)
        self.assertEqual(1, data["errors"])
        self.assertIn("no_such_module_for_selftest", text)

    def test_module_level_skip_is_a_skip(self):
        self.module("test_ok.py", """
            class A(unittest.TestCase):
                def test_ok(self):
                    pass
            """)
        self.module("test_skip.py", "raise unittest.SkipTest('optional dependency absent')\n")
        code, text, data, _ = self.run_runner()
        self.assertEqual(0, code, text)
        self.assertEqual(1, data["skipped"])
        self.assertIn("optional dependency absent", text)

    def test_zero_selected_tests_fail(self):
        self.module("test_only_slow.py", """
            @slow
            class S(unittest.TestCase):
                def test_x(self):
                    pass
            """)
        code, text, _, _ = self.run_runner()
        self.assertEqual(1, code)
        self.assertIn("zero tests", text)

    def test_hung_worker_is_killed_at_its_timeout(self):
        self.module("test_hang.py", """
            class H(unittest.TestCase):
                def test_hang(self):
                    time.sleep(60)
            """)
        code, text, data, elapsed = self.run_runner("--timeout", "1")
        self.assertEqual(1, code)
        self.assertEqual(["test_hang.H"], data["worker_problems"])
        self.assertLess(elapsed, 5.0)

    def test_over_budget_test_is_reported(self):
        self.module("test_slowish.py", """
            class S(unittest.TestCase):
                def test_slowish(self):
                    time.sleep(0.6)

                def test_quick(self):
                    pass
            """)
        code, text, data, _ = self.run_runner("--test-budget", "0.3")
        self.assertEqual(0, code, text)
        self.assertEqual(["test_slowish.S.test_slowish"], data["over_budget"])
        self.assertIn("OVER BUDGET", text)


if __name__ == "__main__":
    unittest.main()
