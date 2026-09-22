"""Execute the real SDK-free audio selection seam with release assertions enabled."""

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]


class AudioProviderTests(unittest.TestCase):
    def test_selection_contract(self):
        self.run_contract("test_audio_selection.cpp", "device_selection.cpp",
                          "PASS audio provider selection: 51 checks")

    def test_media_contract(self):
        self.run_contract("test_media_selection.cpp", "media_providers.cpp",
                          "PASS audio media selection: 24 checks")

    def run_contract(self, fixture, source, expected):
        compiler = shutil.which("c++")
        self.assertIsNotNone(compiler, "the required native C++ compiler is missing")
        with tempfile.TemporaryDirectory(prefix="audio-provider-") as output:
            executable = Path(output) / "audio_provider_conformance"
            compile_result = subprocess.run(
                [compiler, "-std=c++11", "-DNDEBUG", "-Wall", "-Wextra", "-Werror",
                 "-I", str(ROOT / "public"),
                 str(ROOT / "unittests/audioprovidertest" / fixture),
                 str(ROOT / "engine/audio" / source), "-o", str(executable)],
                text=True, capture_output=True, timeout=60)
            self.assertEqual(compile_result.returncode, 0, compile_result.stdout + compile_result.stderr)
            run = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
            self.assertIn(expected, run.stdout)


if __name__ == "__main__":
    unittest.main()
