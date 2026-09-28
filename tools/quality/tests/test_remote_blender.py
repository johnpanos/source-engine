import shlex
import sys
import tempfile
import unittest
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
        self.assertEqual(command[:-1], list(remote_blender.DEFAULT_SSH) + ["user@gpu"])
        self.assertEqual(shlex.split(command[-1]),
                         ["cd", "/src/engine", "&&", "env", "OCIO=/src/engine/o.ocio",
                          "OMP_NUM_THREADS=1", "/opt/blender 5/blender", "-b", "--python",
                          "/src/engine/x y.py"])

    def test_custom_transport(self):
        remote = self.remote(ssh=["ssh", "-p", "2222"])
        self.assertEqual(remote.command([], {})[:3], ["ssh", "-p", "2222"])

    def test_inside(self):
        root = Path(tempfile.mkdtemp())
        (root / "a" / "b").mkdir(parents=True)
        self.assertTrue(remote_blender._inside(root / "a" / "b", [root / "a"]))
        self.assertTrue(remote_blender._inside(root / "a", [root / "a"]))
        self.assertFalse(remote_blender._inside(root, [root / "a"]))

    def test_toolchain_without_a_block_stays_local(self):
        self.assertIsNone(remote_blender.from_toolchain({"blender": "blender"}))


if __name__ == "__main__":
    unittest.main()
