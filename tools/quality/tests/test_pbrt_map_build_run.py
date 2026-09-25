"""Step command runner of pbrt_map_build: output capture and the silence stop.

A GPU fault can leave a Cycles bake waiting forever on a lost HIP queue; the
pipeline must stop such a step instead of waiting. These run real child
processes (sh and sleep); no Blender or GPU.
"""

import io
import sys
import time
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import pbrt_map_build  # noqa: E402


class RunLoggedTest(unittest.TestCase):

    def test_output_and_exit_status_pass_through(self):
        handle, seen = io.StringIO(), []
        status, quiet = pbrt_map_build.run_logged(
            ["sh", "-c", "echo one; echo two; exit 3"], handle, on_line=seen.append, silence=10)
        self.assertEqual((status, quiet), (3, None))
        self.assertEqual(seen, ["one\n", "two\n"])
        self.assertEqual(handle.getvalue(), "one\ntwo\n")

    def test_steady_output_is_not_stopped(self):
        """Each line comes sooner than the limit, the whole run takes longer."""
        started = time.monotonic()
        status, quiet = pbrt_map_build.run_logged(
            ["sh", "-c", "for i in 1 2 3 4 5 6; do echo $i; sleep 0.3; done"], io.StringIO(),
            silence=1)
        self.assertEqual((status, quiet), (0, None))
        self.assertGreater(time.monotonic() - started, 1.5)

    def test_silent_command_and_its_children_are_stopped(self):
        handle, seen = io.StringIO(), []
        started = time.monotonic()
        status, quiet = pbrt_map_build.run_logged(
            ["sh", "-c", "sleep 60 & echo $!; wait"], handle, on_line=seen.append, silence=1)
        self.assertEqual((status, quiet), (None, 1))
        self.assertLess(time.monotonic() - started, 10)
        child = int(seen[0])
        # The background child is in the killed process group too.
        deadline = time.monotonic() + 5
        while Path("/proc/%d" % child).exists() and time.monotonic() < deadline:
            time.sleep(0.05)
        alive = Path("/proc/%d" % child).exists() and \
            Path("/proc/%d/stat" % child).read_text().split()[2] != "Z"
        self.assertFalse(alive)
        self.assertIn("no output for 1 s", handle.getvalue())

    def test_no_limit_waits(self):
        status, quiet = pbrt_map_build.run_logged(["sh", "-c", "sleep 1.2; echo done"],
                                                  io.StringIO())
        self.assertEqual((status, quiet), (0, None))


if __name__ == "__main__":
    unittest.main()
