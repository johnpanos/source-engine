#!/usr/bin/env python3
"""Self-test for scripts/waifulib/output_lock.py: one writing Waf command per
output directory at a time.

Each case runs the repository's own ./waf on a small temporary project whose
wscript installs the lock exactly as the top-level wscript does, with a build
step that records when it starts and ends. Two writers on one directory must
never overlap; the negative control shows the same oracle catches the overlap
when the project does not install the lock.

    python3 -m unittest tools/quality/tests/test_waf_output_lock.py -v
"""

import importlib.util
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time
import unittest

REPO = Path(__file__).resolve().parents[3]
WAF = REPO / "waf"
SPEC = importlib.util.spec_from_file_location("output_lock",
                                              REPO / "scripts/waifulib/output_lock.py")
output_lock = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(output_lock)

INSTALL = "import output_lock\noutput_lock.install()\n"

WSCRIPT = """
import os
import subprocess
import sys
import time
from waflib import Build
%(install)s

def options(opt):
	pass

def configure(conf):
	pass

def build(bld):
	log = os.environ['LOCK_TEST_LOG']
	tag = os.environ.get('LOCK_TEST_TAG', 'build')
	hold = float(os.environ.get('LOCK_TEST_HOLD', '0'))
	def work(task):
		with open(log, 'a') as handle:
			handle.write('start %%s %%f\\n' %% (tag, time.time()))
		time.sleep(hold)
		with open(log, 'a') as handle:
			handle.write('end %%s %%f\\n' %% (tag, time.time()))
	bld(rule=work, always=True)

class Outer(Build.BuildContext):
	'''A writing command that runs a nested ./waf build on its own directory.'''
	cmd = 'outer'
	def execute(self):
		env = dict(os.environ, LOCK_TEST_TAG='inner')
		subprocess.check_call([sys.executable, os.environ['LOCK_TEST_WAF'], 'build'], env=env)
"""

LOCK_VARIABLES = (output_lock.ENV_DISABLE, output_lock.ENV_TIMEOUT, output_lock.ENV_REMINDER,
                  output_lock.ENV_HELD, "WAFLOCK")


class Project:
    """A temporary Waf project with one or more configured output directories."""

    def __init__(self, root, install=True):
        self.root = Path(root)
        self.log = self.root / "events.log"
        (self.root / "wscript").write_text(WSCRIPT % {"install": INSTALL if install else ""})
        self.processes = []

    def env(self, waflock=None, **extra):
        env = {key: value for key, value in os.environ.items() if key not in LOCK_VARIABLES}
        env.update(LOCK_TEST_LOG=str(self.log), LOCK_TEST_WAF=str(WAF))
        if waflock:
            env["WAFLOCK"] = waflock
        env.update({key: str(value) for key, value in extra.items()})
        return env

    def run(self, *args, waflock=None, timeout=60, **extra):
        return subprocess.run([sys.executable, str(WAF)] + list(args), cwd=self.root,
                              env=self.env(waflock, **extra), capture_output=True, text=True,
                              timeout=timeout)

    def configure(self, out, waflock=None):
        result = self.run("configure", "-o", out, waflock=waflock)
        if result.returncode:
            raise AssertionError("configure failed: " + (result.stdout + result.stderr)[-800:])
        return os.path.realpath(self.root / out)

    def start(self, tag, hold, *args, waflock=None, **extra):
        output = open(self.root / ("%s.out" % tag), "w")
        process = subprocess.Popen([sys.executable, str(WAF)] + list(args or ("build",)),
                                   cwd=self.root, stdout=output, stderr=subprocess.STDOUT,
                                   env=self.env(waflock, LOCK_TEST_TAG=tag, LOCK_TEST_HOLD=hold,
                                                **extra),
                                   start_new_session=True)
        output.close()
        self.processes.append(process)
        return process

    def output(self, tag):
        return (self.root / ("%s.out" % tag)).read_text()

    def events(self):
        """{tag: (start, end)} from the build steps' records."""
        times = {}
        if self.log.exists():
            for line in self.log.read_text().splitlines():
                kind, tag, stamp = line.split()
                times.setdefault(tag, [None, None])[kind == "end"] = float(stamp)
        return times

    def wait_for(self, tag, kind="start", timeout=30):
        deadline = time.time() + timeout
        while time.time() < deadline:
            times = self.events().get(tag)
            if times and times[kind == "end"] is not None:
                return
            time.sleep(0.05)
        raise AssertionError("%s never recorded %s" % (tag, kind))

    def finish(self, process, timeout=60):
        process.wait(timeout=timeout)
        return process.returncode

    def stop(self):
        for process in self.processes:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()


def assert_serialized(events, first, second):
    """The oracle: the second writer's step starts only after the first's ends."""
    if events[second][0] < events[first][1]:
        raise AssertionError("%s started at %.3f, before %s ended at %.3f"
                             % (second, events[second][0], first, events[first][1]))


class WafOutputLockTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.project = Project(self.directory.name)
        self.out = self.project.configure("build")

    def tearDown(self):
        self.project.stop()
        self.directory.cleanup()

    def test_second_writer_on_the_same_directory_waits(self):
        project = self.project
        holder = project.start("first", 2.0)
        project.wait_for("first")
        waiter = project.start("second", 0)
        self.assertEqual(0, project.finish(holder), project.output("first"))
        self.assertEqual(0, project.finish(waiter), project.output("second"))
        assert_serialized(project.events(), "first", "second")
        text = project.output("second")
        self.assertIn("waiting for output directory %s" % self.out, text)
        self.assertIn("pid=%d command=build" % holder.pid, text)
        self.assertIn("is free after", text)

    def test_different_directories_do_not_wait(self):
        project = self.project
        project.configure("out-a", waflock=".lock-waf-a")
        project.configure("out-b", waflock=".lock-waf-b")
        holder = project.start("a", 3.0, waflock=".lock-waf-a")
        project.wait_for("a")
        other = project.start("b", 0, waflock=".lock-waf-b")
        self.assertEqual(0, project.finish(other), project.output("b"))
        self.assertIsNone(holder.poll(), "the other directory's build finished only after A")
        self.assertNotIn("waiting for output directory", project.output("b"))
        self.assertEqual(0, project.finish(holder))

    def test_killed_holder_releases_the_lock(self):
        project = self.project
        holder = project.start("killed", 60)
        project.wait_for("killed")
        os.killpg(holder.pid, signal.SIGKILL)
        holder.wait()
        began = time.time()
        waiter = project.start("after", 0, **{output_lock.ENV_TIMEOUT: 10})
        self.assertEqual(0, project.finish(waiter), project.output("after"))
        self.assertLess(time.time() - began, 10)
        self.assertNotIn("waiting for output directory", project.output("after"))

    def test_opt_out_does_not_wait(self):
        project = self.project
        holder = project.start("first", 3.0)
        project.wait_for("first")
        bypass = project.start("bypass", 0, **{output_lock.ENV_DISABLE: 0})
        self.assertEqual(0, project.finish(bypass), project.output("bypass"))
        self.assertIsNone(holder.poll())
        self.assertNotIn("waiting for output directory", project.output("bypass"))
        self.assertEqual(0, project.finish(holder))
        with self.assertRaises(AssertionError):
            assert_serialized(project.events(), "first", "bypass")

    def test_timeout_fails_with_a_clear_error(self):
        project = self.project
        project.start("first", 10)
        project.wait_for("first")
        result = project.run("build", **{output_lock.ENV_TIMEOUT: 0.5, "LOCK_TEST_TAG": "late"})
        self.assertNotEqual(0, result.returncode)
        text = result.stdout + result.stderr
        self.assertIn("gave up after 0.5s (%s) waiting for output directory %s"
                      % (output_lock.ENV_TIMEOUT, self.out), text)
        self.assertNotIn("late", project.events())

    def test_read_only_list_does_not_wait(self):
        project = self.project
        holder = project.start("first", 3.0)
        project.wait_for("first")
        result = project.run("list", **{output_lock.ENV_TIMEOUT: 0.5})
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        self.assertNotIn("waiting for output directory", result.stdout + result.stderr)
        self.assertIsNone(holder.poll())

    def test_nested_waf_on_the_same_directory_does_not_deadlock(self):
        result = self.project.run("outer", **{output_lock.ENV_TIMEOUT: 5})
        self.assertEqual(0, result.returncode, (result.stdout + result.stderr)[-800:])
        self.assertIn("inner", self.project.events())
        self.assertNotIn("waiting for output directory", result.stdout + result.stderr)

    def test_forged_marker_from_a_non_ancestor_still_waits(self):
        project = self.project
        holder = project.start("first", 10)
        project.wait_for("first")
        marker = "%s=%d" % (self.out, holder.pid)
        result = project.run("build", **{output_lock.ENV_TIMEOUT: 0.5,
                                         output_lock.ENV_HELD: marker, "LOCK_TEST_TAG": "forged"})
        self.assertNotEqual(0, result.returncode)
        self.assertIn("gave up after", result.stdout + result.stderr)

    def test_clean_keeps_the_lock_file(self):
        result = self.project.run("clean")
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        self.assertTrue(os.path.exists(os.path.join(self.out, output_lock.LOCK_NAME)))


class NegativeControlTest(unittest.TestCase):
    """Without the lock, the same-directory oracle must see the two builds overlap."""

    def test_oracle_detects_overlap_without_the_lock(self):
        with tempfile.TemporaryDirectory() as directory:
            project = Project(directory, install=False)
            try:
                project.configure("build")
                holder = project.start("first", 2.0)
                project.wait_for("first")
                other = project.start("second", 0)
                self.assertEqual(0, project.finish(other))
                self.assertEqual(0, project.finish(holder))
                with self.assertRaises(AssertionError):
                    assert_serialized(project.events(), "first", "second")
                self.assertNotIn("waiting for output directory", project.output("second"))
            finally:
                project.stop()


class InProcessTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.saved = {key: os.environ.pop(key) for key in LOCK_VARIABLES if key in os.environ}

    def tearDown(self):
        output_lock.release_all()
        os.environ.update(self.saved)
        self.directory.cleanup()

    def test_reentrant_in_one_process(self):
        path = self.directory.name
        self.assertEqual("acquired", output_lock.acquire(path, "configure"))
        self.assertEqual("held", output_lock.acquire(path, "build"))
        self.assertIn(os.path.realpath(path), os.environ[output_lock.ENV_HELD])
        output_lock.release_all()
        self.assertNotIn(output_lock.ENV_HELD, os.environ)

    def test_stale_marker_with_a_free_lock_takes_it(self):
        path = os.path.realpath(self.directory.name)
        os.environ[output_lock.ENV_HELD] = "%s=%d" % (path, os.getppid())
        self.assertEqual("acquired", output_lock.acquire(path, "build"))

    def test_missing_directory_is_skipped_unless_created(self):
        path = os.path.join(self.directory.name, "absent")
        self.assertEqual("skipped", output_lock.acquire(path, "build"))
        self.assertEqual("acquired", output_lock.acquire(path, "configure", create=True))

    def test_holder_line_names_pid_and_command(self):
        path = self.directory.name
        output_lock.acquire(path, "install")
        line = output_lock.read_holder(path)
        self.assertTrue(line.startswith("pid=%d command=install " % os.getpid()), line)


if __name__ == "__main__":
    unittest.main()
