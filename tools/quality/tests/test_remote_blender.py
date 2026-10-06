import io
import json
import shlex
import sys
import tempfile
import subprocess
import time
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


class ProvisioningTest(unittest.TestCase):
    """The host profile's digests, the install prefix a host can use, and the
    block a provisioned host gets."""

    def test_paths_follow_the_install_prefix(self):
        canonical = remote_blender.paths("/opt")
        self.assertEqual(canonical["blender"],
                         "/opt/blender-%s-linux-x64/blender" % remote_blender.HOST["blender"]["version"])
        self.assertEqual(canonical["oidn_lib_dir"],
                         "/opt/oidn-%s.x86_64.linux/lib" % remote_blender.HOST["oidn"]["version"])
        self.assertEqual(canonical["python_target"], "/opt/source-python")
        home = remote_blender.paths("/home/bazzite/.local/opt")
        for key, value in canonical.items():
            if key == "base":
                continue
            self.assertEqual(home[key], value.replace("/opt", "/home/bazzite/.local/opt", 1), key)

    def test_the_block_names_the_installed_python_environment(self):
        block = remote_blender.block_for("bazzite@192.168.0.13", "/opt", "ssh -p 2222")
        self.assertEqual(block["ssh"], ["ssh", "-p", "2222"])
        self.assertEqual(block["python"], remote_blender.paths("/opt")["python"])
        self.assertEqual(block["env"]["LD_LIBRARY_PATH"], "/opt/oidn-%s.x86_64.linux/lib"
                         % remote_blender.HOST["oidn"]["version"])
        self.assertEqual(block["tools"]["openimagedenoise"]["sha256"],
                         remote_blender.HOST["oidn"]["sha256"])
        for step in remote_blender.REMOTE_STEPS + remote_blender.TOOL_STEPS:
            self.assertIn(step, block["steps"], step)
        blender_only = remote_blender.block_for("h", "/opt", tools=False)
        self.assertNotIn("python", blender_only)
        self.assertNotIn("env", blender_only)

    def test_a_host_that_already_has_the_tools_installs_no_packages(self):
        present = {"tools": {name: True for name in remote_blender.BASE_TOOLS}}
        script = remote_blender.provision_script("/opt", "debian", present)
        self.assertNotIn("apt-get", script)
        self.assertNotIn("dnf", script)

    def test_a_fedora_host_with_gaps_gets_dnf_and_a_debian_one_apt(self):
        gaps = {"tools": {name: True for name in remote_blender.BASE_TOOLS
                          if name not in ("tar", "xz")}}
        self.assertIn("dnf install", remote_blender.provision_script("/opt", "fedora", gaps))
        self.assertNotIn("apt-get", remote_blender.provision_script("/opt", "fedora", gaps))
        self.assertIn("apt-get install", remote_blender.provision_script("/opt", "debian", gaps))

    def test_an_unknown_distribution_is_refused_by_name(self):
        gaps = {"tools": {}}
        with self.assertRaises(ValueError) as raised:
            remote_blender.provision_script("/opt", "plan9", gaps)
        self.assertIn("plan9", str(raised.exception))

    def test_an_immutable_image_uses_the_prefix_the_user_owns(self):
        # a read-only /opt and an image whose ID is only its parent's ID_LIKE
        facts = {"tools": {"rsync": True, "curl": True, "tar": True, "xz": True,
                           "sha256sum": True, "nvidia-smi": True}}
        os_release = ("NAME=\"Bazzite\"\nID=bazzite\nID_LIKE=\"fedora\"\nVERSION_ID=44\n")
        commands = {"cat /etc/os-release": os_release, "printf %s \"$HOME\"": "/home/bazzite"}
        for tool in facts["tools"]:
            commands["command -v " + tool] = ""
        remote = self.remote(commands, "/opt")  # not writable
        surveyed = remote_blender.survey(remote)
        self.assertEqual(surveyed["distro"], "fedora")
        self.assertFalse(surveyed["base_writable"])
        self.assertEqual(surveyed["base"], "$HOME/.local/opt")
        self.assertEqual(surveyed["home"], "/home/bazzite")

    def test_the_mirror_lands_at_the_same_absolute_path_on_the_host(self):
        """A push names paths relative to the checkout and writes them into the
        host's copy of it, with --relative; a pull reads the mirror back without
        it. Both matter: an immutable host's root is read-only, so `/` is no
        destination, and --relative on a pull would rebuild the whole absolute
        path under the destination."""
        remote = self.remote()
        self.assertEqual(self.rsync_arguments(remote.push, [], Path("/src/engine/out")), [
            # the support trees first, then the work directory, which is deleted there
            ["-a", "--relative", "--exclude=__pycache__", "--exclude=.previous",
             "--exclude=logs", "tools/quality", "tools/texture", "quality", "h:/src/engine/"],
            ["-a", "--relative", "--exclude=__pycache__", "--exclude=.previous",
             "--exclude=logs", "--delete", "out", "h:/src/engine/"]])
        self.assertEqual(self.rsync_arguments(remote.pull, Path("/src/engine/out"), update=True),
                         [["-a", "--exclude=__pycache__", "--exclude=.previous",
                           "--exclude=logs", "--update", "h:/src/engine/out/", "/src/engine/out/"]])

    def test_an_input_outside_the_checkout_is_refused_by_name(self):
        remote = self.remote()
        with self.assertRaises(ValueError) as raised:
            self.rsync_arguments(remote.push, ["/etc/hostname"], Path("/src/engine/out"))
        self.assertIn("/etc/hostname", str(raised.exception))

    @staticmethod
    def rsync_arguments(issue, *arguments, **keywords):
        """The rsync argument lists `issue` runs, without running rsync: the
        transport `-e <ssh>` is dropped, since it is not a path."""
        recorded = []

        class Result:
            returncode, stdout, stderr = 0, "", ""

        def run(command, **kwargs):
            recorded.append([str(part) for part in command[1:]])
            return Result()

        real = subprocess.run
        subprocess.run = run
        try:
            issue(*arguments, **keywords)
        finally:
            subprocess.run = real
        calls = []
        for arguments in recorded:
            if "-e" in arguments:
                index = arguments.index("-e")
                del arguments[index:index + 2]
            calls.append(arguments)
        return calls

    @staticmethod
    def remote(commands=None, unwritable=None, root="/src/engine"):
        """A RemoteBlender whose ssh answers `commands` (an exact string or a
        prefix match on its first word), and fails for `unwritable`."""
        remote = remote_blender.RemoteBlender({"host": "h", "blender": "/opt/blender"},
                                              root=Path(root))
        remote.ssh = ["ssh"]
        calls = []

        class Result:
            def __init__(self, returncode):
                self.returncode, self.stdout, self.stderr = returncode, "", ""

        def answer(command, **kwargs):
            calls.append(command)
            for expected, stdout in (commands or {}).items():
                if command == expected or command.startswith(expected):
                    result = Result(0)
                    result.stdout = stdout
                    return result
            if unwritable and command.startswith("test -w " + unwritable):
                return Result(1)
            return Result(127)

        remote.remote = answer
        remote.calls = calls
        return remote


# A stand-in ssh: runs the command locally; with FAKE_SSH_DROP set, the first
# `tail` it is asked for prints part of the output and fails as a dropped
# connection does (exit 255).
FAKE_SSH = """
import os, subprocess, sys
# Real ssh makes the descriptors it inherits non-blocking (its stdout it
# writes itself, handling EAGAIN; stdin and stderr are shared with its parent).
for fd in (0, 2):
    try:
        os.set_blocking(fd, False)
    except OSError:
        pass
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

    def test_a_large_burst_through_a_real_pipe_survives_non_blocking_ssh(self):
        """The client, as the pipeline runs it (stdout and stderr one pipe),
        streams a burst larger than the pipe buffer while ssh sets its
        inherited descriptors non-blocking."""
        script = ("import sys; sys.path.insert(0, %r); import remote_blender as r; "
                  "r.JOBS = %r; sys.exit(r.execute(%r, 'host', "
                  "'head -c 3000000 /dev/zero | tr \\\\0 x; echo; exit 5'))"
                  % (str(Path(remote_blender.__file__).parent), remote_blender.JOBS, self.ssh))
        reader = subprocess.Popen([sys.executable, "-c", script], stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT)
        received = 0
        for chunk in iter(lambda: reader.stdout.read(4096), b""):
            received += len(chunk)
            time.sleep(0.0005)   # a slow reader, as the pipeline's log loop is
        self.assertEqual(reader.wait(), 5)
        self.assertGreaterEqual(received, 3000000)

    def test_a_rerun_replaces_its_live_orphan(self):
        """A job with the same command line as a live one stops it first."""
        line = "sleep 30; echo late"
        first = subprocess.Popen([sys.executable, "-c",
                                  "import sys; sys.path.insert(0, %r); import remote_blender as r; "
                                  "r.JOBS = %r; sys.exit(r.execute(%r, 'host', %r))"
                                  % (str(Path(remote_blender.__file__).parent), remote_blender.JOBS,
                                     self.ssh, line)],
                                 stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        jobs = Path(remote_blender.JOBS)
        for _ in range(100):
            if any((d / "pid").is_file() for d in jobs.glob("*")):
                break
            time.sleep(0.1)
        orphan = next(d for d in jobs.glob("*") if (d / "pid").is_file())
        first.kill()   # the client dies; its job lives on
        out = io.BytesIO()
        started = time.monotonic()
        code = remote_blender.execute(self.ssh, "host", "echo second", out, sleep=lambda s: None)
        self.assertEqual((code, out.getvalue()), (0, b"second\n"))
        for _ in range(50):
            if subprocess.run(["kill", "-0", (orphan / "pid").read_text().strip()],
                              capture_output=True).returncode:
                break
            time.sleep(0.1)
        self.assertFalse((orphan / "rc").is_file())   # stopped, not finished
        self.assertLess(time.monotonic() - started, 20)

    def test_the_job_runs_detached_with_a_watchdog(self):
        script = remote_blender.start_script("/tmp/remote-blender/x", "blender -b")
        self.assertIn("setsid bash -c '(blender -b); echo $? > /tmp/remote-blender/x/rc'", script)
        self.assertIn("-gt %d" % remote_blender.LEASE_S, script)


if __name__ == "__main__":
    unittest.main()
