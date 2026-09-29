import io
import json
import shlex
import sys
import tempfile
import unittest
from unittest import mock
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import remote_blender  # noqa: E402


class RemoteBlenderTest(unittest.TestCase):
    def remote(self, **extra):
        config = dict({"host": "user@gpu", "blender": "/opt/blender 5/blender"}, **extra)
        return remote_blender.RemoteBlender(config, root=Path("/src/engine"))

    def test_only_cycles_steps_run_remotely_by_default(self):
        remote = self.remote()
        for step in remote_blender.REMOTE_STEPS:
            self.assertTrue(remote.applies(step))
        self.assertFalse(remote.applies("stage"))
        self.assertFalse(remote.applies("pack"))

    def test_non_blender_steps_are_rejected(self):
        with self.assertRaises(ValueError):
            self.remote(steps=["bake", "pack"])
        with self.assertRaises(ValueError):
            remote_blender.RemoteBlender({"host": "h"})

    def test_command_runs_in_the_mirrored_root_with_the_environment(self):
        command = self.remote().command(["-b", "--python", "/src/engine/x y.py"],
                                        {"OCIO": "/src/engine/o.ocio", "OMP_NUM_THREADS": "1"})
        self.assertEqual(command[2:4], ["exec", "--ssh"])
        self.assertEqual(json.loads(command[4]), list(remote_blender.DEFAULT_SSH))
        self.assertEqual(command[5:8], ["--host", "user@gpu", "--"])
        self.assertEqual(shlex.split(command[-1]),
                         ["cd", "/src/engine", "&&", "env", "OCIO=/src/engine/o.ocio",
                          "OMP_NUM_THREADS=1", "/opt/blender 5/blender", "-b", "--python",
                          "/src/engine/x y.py"])

    def test_custom_transport(self):
        remote = self.remote(ssh=["ssh", "-p", "2222"])
        self.assertEqual(json.loads(remote.command([], {})[4]), ["ssh", "-p", "2222"])

    def test_inside(self):
        root = Path(tempfile.mkdtemp())
        (root / "a" / "b").mkdir(parents=True)
        self.assertTrue(remote_blender._inside(root / "a" / "b", [root / "a"]))
        self.assertTrue(remote_blender._inside(root / "a", [root / "a"]))
        self.assertFalse(remote_blender._inside(root, [root / "a"]))

    def test_cache_identity_names_the_blender_not_the_host(self):
        remote = self.remote()
        remote._identity = {"host": "user@gpu", "version": "5.2.2", "sha256": "ab"}
        other = self.remote(host="root@1.2.3.4")
        other._identity = {"host": "root@1.2.3.4", "version": "5.2.2", "sha256": "ab"}
        self.assertEqual(remote.cache_identity(), other.cache_identity())
        self.assertNotIn("host", remote.cache_identity())

    def test_tool_steps_are_opt_in_and_need_a_python(self):
        self.assertFalse(self.remote().applies("denoise"))
        with self.assertRaises(ValueError):
            self.remote(steps=["bake", "denoise"])
        remote = self.remote(steps=["bake", "denoise"], python="/opt/py/python3.13",
                             env={"PYTHONPATH": "/opt/site"},
                             tools={"openimagedenoise": {"version": "2.4.0", "sha256": "ab"}})
        self.assertTrue(remote.applies("denoise"))
        command = remote.tool_command(
            ["/src/engine/tools/quality/lightmap_denoise.py", "--exr", "/src/engine/o/a b.exr"],
            {"EXTRA": "1"})
        self.assertEqual(shlex.split(command[-1]),
                         ["cd", "/src/engine", "&&", "env", "EXTRA=1", "PYTHONPATH=/opt/site",
                          "/opt/py/python3.13", "/src/engine/tools/quality/lightmap_denoise.py",
                          "--exr", "/src/engine/o/a b.exr"])
        self.assertEqual(remote.tool_identity("openimagedenoise"),
                         {"version": "2.4.0", "sha256": "ab", "remote": True})
        self.assertIsNone(remote.tool_identity("ktx"))

    def test_toolchain_without_a_block_stays_local(self):
        self.assertIsNone(remote_blender.from_toolchain({"blender": "blender"}))


# A stand-in ssh: runs the command locally; with FAKE_SSH_DROP set, the first
# `tail` it is asked for prints part of the output and fails as a dropped
# connection does (exit 255).
FAKE_SSH = """
import os, subprocess, sys
command = sys.argv[2]
flag = os.environ.get("FAKE_SSH_DROP")
if flag and command.startswith("tail") and not os.path.exists(flag):
    open(flag, "w").close()
    subprocess.run(["bash", "-c", "sleep 1; head -c 4 " + command.split()[-1]])
    sys.exit(255)
sys.exit(subprocess.run(["bash", "-c", command], stdin=sys.stdin).returncode)
"""


class DetachedJobTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())
        (self.tmp / "ssh.py").write_text(FAKE_SSH)
        self.ssh = [sys.executable, str(self.tmp / "ssh.py")]
        self.jobs = remote_blender.JOBS
        remote_blender.JOBS = str(self.tmp / "jobs")

    def tearDown(self):
        remote_blender.JOBS = self.jobs

    def test_output_and_exit_status_come_back(self):
        out = io.BytesIO()
        code = remote_blender.execute(self.ssh, "host", "echo one; echo two >&2; exit 3", out,
                                      sleep=lambda s: None)
        self.assertEqual(code, 3)
        self.assertEqual(sorted(out.getvalue().decode().split()), ["one", "two"])

    def test_a_dropped_connection_resumes_where_it_stopped(self):
        out = io.BytesIO()
        with mock.patch.dict("os.environ", {"FAKE_SSH_DROP": str(self.tmp / "dropped")}):
            code = remote_blender.execute(self.ssh, "host",
                                          "printf 'abcdefgh'; sleep 2; printf 'ijkl'", out,
                                          sleep=lambda s: None)
        self.assertEqual(code, 0)
        text = out.getvalue().decode()
        self.assertIn("connection lost", text)
        self.assertEqual(text.replace("[remote_blender] connection lost; reconnecting\n", ""),
                         "abcdefghijkl")

    def test_the_job_runs_detached_with_a_watchdog(self):
        script = remote_blender.start_script("/tmp/remote-blender/x", "blender -b")
        self.assertIn("setsid bash -c '(blender -b); echo $? > /tmp/remote-blender/x/rc'", script)
        self.assertIn("-gt %d" % remote_blender.LEASE_S, script)


if __name__ == "__main__":
    unittest.main()
