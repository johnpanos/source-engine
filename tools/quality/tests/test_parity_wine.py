#!/usr/bin/env python3
"""Self-tests for tools/quality/parity_wine.py.

The Wine parity lane is a thin sibling of the native runner; these tests pin
the parts it owns: output capture that never waits on a helper process which
inherited the suite's stdout, CRLF output from PE binaries, one shared Wine
server that cannot block the run, and concurrent builds that leave results
and their order unchanged. Every test uses fakes (shell scripts standing in
for Wine and the PE binaries), so no Wine or MinGW install is needed, and each
has a time budget of seconds: a regression shows up as a failure, not a hang.
"""

import os
import shutil
import stat
import sys
import tempfile
import time
import unittest

QUALITY_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, QUALITY_DIR)

import conformance  # noqa: E402
import parity_wine  # noqa: E402

# A helper that outlives the suite by this long would stall a pipe-based run.
LINGER_SECONDS = 20
# Every run below must finish well inside this, whatever the helpers do.
RUN_BUDGET_SECONDS = 3.0


def write_script(directory, name, body):
    path = os.path.join(directory, name)
    with open(path, "w", encoding="utf-8") as handle:
        handle.write("#!/bin/sh\n" + body)
    os.chmod(path, os.stat(path).st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
    return path


def suite(sid="s", expect="pass"):
    return {"id": sid, "expect": expect, "result_protocol": conformance.RESULT_PROTOCOL,
            "sources": []}


class RunBuiltTest(unittest.TestCase):
    def setUp(self):
        self.work = tempfile.mkdtemp(prefix="parity-test-")
        self.addCleanup(shutil.rmtree, self.work, True)
        # Stands in for `wine`: starts a helper that keeps the inherited
        # stdout/stderr open long after the program exits, like wineserver,
        # services.exe and winedevice.exe do, then runs the program.
        self.lingering_wine = write_script(
            self.work, "wine",
            "(sleep %d) &\nexec \"$@\"\n" % LINGER_SECONDS)

    def _program(self, body):
        return write_script(self.work, "suite.exe", body)

    def _run(self, program, record_suite=None):
        s = record_suite or suite()
        started = time.monotonic()
        record = parity_wine.run_built(s, parity_wine.new_record(s), program,
                                       [self.lingering_wine], timeout=30)
        return record, time.monotonic() - started

    def test_lingering_helper_does_not_stall_the_run(self):
        record, elapsed = self._run(self._program("printf 'CONFORMANCE 3 0\\n'\n"))
        self.assertLess(elapsed, RUN_BUDGET_SECONDS,
                        "the run waited on a helper holding the suite's stdout")
        self.assertEqual(record["outcome"], "pass")
        self.assertEqual(record["checks"], 3)
        self.assertTrue(record["matched"])

    def test_crlf_output_is_parsed(self):
        record, _ = self._run(self._program("printf 'line one\\r\\nCONFORMANCE 7 0\\r\\n'\n"))
        self.assertEqual(record["outcome"], "pass")
        self.assertEqual(record["checks"], 7)
        self.assertEqual(record["detail"], "CONFORMANCE 7 0")

    def test_failing_record_still_fails(self):
        record, _ = self._run(self._program("printf 'CONFORMANCE 4 1\\r\\n'\n"))
        self.assertEqual(record["outcome"], "fail")
        self.assertFalse(record["matched"])

    def test_missing_record_fails(self):
        record, _ = self._run(self._program("printf 'no record here\\r\\n'\n"))
        self.assertEqual(record["outcome"], "fail")

    def test_timeout_is_reported_quickly(self):
        s = suite()
        started = time.monotonic()
        record = parity_wine.run_built(s, parity_wine.new_record(s),
                                       self._program("sleep 30\n"), [], timeout=1)
        self.assertEqual(record["outcome"], "timeout")
        self.assertLess(time.monotonic() - started, RUN_BUDGET_SECONDS)

    def test_duration_is_recorded(self):
        record, _ = self._run(self._program("printf 'CONFORMANCE 1 0\\n'\n"))
        self.assertIsInstance(record["duration_s"], float)


class WineServerTest(unittest.TestCase):
    def setUp(self):
        self.work = tempfile.mkdtemp(prefix="parity-server-")
        self.addCleanup(shutil.rmtree, self.work, True)
        self.path = os.environ.get("PATH", "")
        self.addCleanup(os.environ.__setitem__, "PATH", self.path)
        # Stands in for `wineserver -p`: daemonizes, and the daemon keeps the
        # standard streams it inherited.
        write_script(self.work, "wineserver",
                     "echo \"$@\" > \"%s/args\"\n(sleep %d) &\nexit 0\n"
                     % (self.work, LINGER_SECONDS))
        os.environ["PATH"] = self.work + os.pathsep + self.path

    def test_daemonizing_server_does_not_block(self):
        started = time.monotonic()
        started_server = parity_wine.start_wineserver(["wine"], linger=17)
        self.assertLess(time.monotonic() - started, RUN_BUDGET_SECONDS,
                        "start_wineserver waited on the daemon's inherited streams")
        self.assertIn("-p17", started_server)
        with open(os.path.join(self.work, "args"), encoding="utf-8") as handle:
            self.assertEqual(handle.read().split(), ["-p17"])

    def test_not_started_for_other_wrappers(self):
        self.assertIsNone(parity_wine.start_wineserver([], linger=5))
        self.assertIsNone(parity_wine.start_wineserver(["qemu-x86_64"], linger=5))
        self.assertFalse(os.path.exists(os.path.join(self.work, "args")))


class BuildCommandsTest(unittest.TestCase):
    PROFILE = {"cxx_std": "c++20", "base_flags": ["-O1"], "include_roots": ["inc"]}
    SUITE = {"id": "x", "sources": ["a.cpp", "b.cpp"], "extra_flags": ["-DX"]}

    def test_without_launcher_is_one_command(self):
        commands = parity_wine.build_commands("/r", "cc", self.PROFILE, self.SUITE, "/o/x.exe",
                                              static=True)
        self.assertEqual(commands, [parity_wine.build_command(
            "/r", "cc", self.PROFILE, self.SUITE, "/o/x.exe", static=True)])

    def test_launcher_compiles_each_source_then_links(self):
        commands = parity_wine.build_commands("/r", "cc", self.PROFILE, self.SUITE, "/o/x.exe",
                                              static=True, launcher="ccache")
        self.assertEqual(len(commands), 3)
        self.assertTrue(all(c[:2] == ["ccache", "cc"] for c in commands))
        self.assertEqual([c[c.index("-c") + 1] for c in commands[:2]], ["/r/a.cpp", "/r/b.cpp"])
        link = commands[-1]
        self.assertNotIn("-c", link)
        self.assertIn("-static", link)
        self.assertEqual(link[-2:], ["-o", "/o/x.exe"])
        # Compiles keep every flag of the single-command build.
        single = parity_wine.build_command("/r", "cc", self.PROFILE, self.SUITE, "/o/x.exe",
                                           static=True)
        for flag in ("-std=c++20", "-O1", "-DX", "-static"):
            self.assertIn(flag, single)
            self.assertIn(flag, commands[0])


if __name__ == "__main__":
    unittest.main()
