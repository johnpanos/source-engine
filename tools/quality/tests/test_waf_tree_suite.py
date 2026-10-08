"""tools/quality/waf_tree_suite.py: its failures print a failing checks-v1 record."""

import contextlib
import io
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import waf_tree_suite as suite  # noqa: E402


def run(*args):
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        code = suite.main(list(args))
    return code, out.getvalue()


class WafTreeSuiteTest(unittest.TestCase):
    def test_unconfigured_tree_fails_with_a_record(self):
        with tempfile.TemporaryDirectory() as tree:
            code, out = run("--tree", tree, "--target", "t", "--program", "p")
        self.assertEqual(code, 1)
        self.assertIn("CONFORMANCE 1 1", out)
        self.assertIn("not a configured Waf tree", out)

    def test_failed_build_fails_with_a_record(self):
        with tempfile.TemporaryDirectory() as tree:
            (Path(tree) / "c4che").mkdir()
            failed = subprocess.CompletedProcess([], 1, "build output", "")
            with mock.patch.object(suite.subprocess, "run", return_value=failed):
                code, out = run("--tree", tree, "--target", "t", "--program", "p")
        self.assertEqual(code, 1)
        self.assertIn("building t", out)
        self.assertIn("CONFORMANCE 1 1", out)

    def test_missing_program_fails_with_a_record(self):
        with tempfile.TemporaryDirectory() as tree:
            (Path(tree) / "c4che").mkdir()
            built = subprocess.CompletedProcess([], 0, "", "")
            with mock.patch.object(suite.subprocess, "run", return_value=built):
                code, out = run("--tree", tree, "--target", "t", "--program", "missing")
        self.assertEqual(code, 1)
        self.assertIn("was not built", out)

    def test_the_program_runs_with_the_tree_libraries(self):
        with tempfile.TemporaryDirectory() as tree:
            root = Path(tree)
            (root / "c4che").mkdir()
            (root / "lib").mkdir()
            (root / "lib/libx.so").write_text("")
            program = root / "prog"
            program.write_text("")
            calls = []

            def fake(argv, **kwargs):
                calls.append((argv, kwargs))
                return subprocess.CompletedProcess(argv, 0, "", "")

            with mock.patch.object(suite.subprocess, "run", side_effect=fake):
                code, _ = run("--tree", tree, "--target", "t", "--program", "prog")
        self.assertEqual(code, 0)
        self.assertEqual(calls[1][0], [str(program)])
        self.assertIn(str(root / "lib"), calls[1][1]["env"]["LD_LIBRARY_PATH"])


if __name__ == "__main__":
    unittest.main()
