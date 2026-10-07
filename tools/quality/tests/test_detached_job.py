import json
import os
import signal
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path

TOOL = Path(__file__).resolve().parents[1] / "detached_job.py"


class DetachedJobTest(unittest.TestCase):
    def setUp(self):
        self.jobs = tempfile.mkdtemp(prefix="detached-job-")

    def run_tool(self, *arguments, timeout=60):
        return subprocess.run([sys.executable, str(TOOL), "--jobs", self.jobs, *arguments],
                              capture_output=True, text=True, timeout=timeout)

    def start(self, name, script):
        done = self.run_tool("start", "--name", name, "--", sys.executable, "-c", script)
        self.assertEqual(done.returncode, 0, done.stderr)
        return json.loads(done.stdout)

    def test_success_reports_zero_and_the_log(self):
        self.start("ok", "print('[bake] running...'); print('[bake] done in 1s')")
        done = self.run_tool("wait", "ok")
        self.assertEqual(done.returncode, 0, done.stdout)
        self.assertIn("succeeded", done.stdout)
        self.assertIn("last finished: bake", done.stdout)

    def test_crash_is_reported_with_its_code_and_error(self):
        self.start("crash", "print('[prop-points] running...'); "
                            "import sys; sys.stderr.write('error: unrecognized arguments\\n'); "
                            "sys.exit(2)")
        done = self.run_tool("wait", "crash")
        self.assertEqual(done.returncode, 2)
        self.assertIn("failed with exit code 2", done.stdout)
        self.assertIn("last step started: prop-points", done.stdout)
        self.assertIn("unrecognized arguments", done.stdout)

    def test_killed_job_is_reported(self):
        job = self.start("killed", "import time; time.sleep(60)")
        os.kill(job["pid"], signal.SIGKILL)
        done = self.run_tool("wait", "killed")
        self.assertNotEqual(done.returncode, 0)
        self.assertIn("killed by signal 9", done.stdout)

    def test_lost_supervisor_does_not_hang(self):
        job = self.start("lost", "import time; time.sleep(60)")
        os.kill(job["supervisor_pid"], signal.SIGKILL)
        done = self.run_tool("wait", "lost")
        self.assertEqual(done.returncode, 125)
        self.assertIn("supervisor lost", done.stdout)
        os.kill(job["pid"], signal.SIGKILL)

    def test_wait_timeout_leaves_the_job_running(self):
        job = self.start("slow", "import time; time.sleep(30)")
        done = self.run_tool("wait", "slow", "--timeout", "1")
        self.assertEqual(done.returncode, 124)
        self.assertIn("still running", self.run_tool("status", "slow").stdout + done.stdout)
        os.kill(job["pid"], signal.SIGKILL)
        self.assertNotEqual(self.run_tool("wait", "slow").returncode, 0)

    def test_running_job_cannot_be_started_twice(self):
        job = self.start("once", "import time; time.sleep(30)")
        again = self.run_tool("start", "--name", "once", "--", sys.executable, "-c", "pass")
        self.assertNotEqual(again.returncode, 0)
        os.kill(job["pid"], signal.SIGKILL)
        self.run_tool("wait", "once")

    def test_job_survives_its_launcher_shell(self):
        # The launching shell's process group is killed; the job is not.
        shell = subprocess.Popen(
            [sys.executable, str(TOOL), "--jobs", self.jobs, "start", "--name", "orphan", "--",
             sys.executable, "-c", "import time; time.sleep(2); print('finished')"],
            start_new_session=True, stdout=subprocess.PIPE, text=True)
        job = json.loads(shell.communicate(timeout=30)[0])
        try:
            os.killpg(shell.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        done = self.run_tool("wait", "orphan")
        self.assertEqual(done.returncode, 0, done.stdout)
        self.assertIn("finished", done.stdout)
        self.assertTrue(job["pid"])


if __name__ == "__main__":
    unittest.main()
