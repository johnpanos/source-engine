"""Negative controls for the external VTF corpus result gate."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'vtf_container_corpus.py'


class CorpusResultTests(unittest.TestCase):
    def run_fixture(self, summary, exit_code=0, has_texture=True):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            binary = root / 'fake-native'
            binary.write_text('#!' + sys.executable + '\nimport sys\n'
                              'sys.stdin.buffer.read()\n'
                              f'print({summary!r})\nsys.exit({exit_code})\n')
            binary.chmod(0o755)
            tree = b'\0'
            if has_texture:
                tree = (b'vtf\0materials\0test\0' +
                        struct.pack('<IHHIIH', 0, 0, 0x7fff, 0, 1, 0xffff) + b'\0\0\0')
            vpk = root / 'test_dir.vpk'
            vpk.write_bytes(struct.pack('<III', 0x55aa1234, 1, len(tree)) + tree + b'x')
            result = subprocess.run([sys.executable, str(SCRIPT), '--binary', str(binary),
                                     '--vpk', str(vpk), '--out', str(root / 'evidence')],
                                    capture_output=True, text=True, timeout=10)
            self.assertTrue((root / 'evidence/result.json').is_file(), result.stderr)
            return result.returncode

    def test_complete_result_passes(self):
        self.assertEqual(0, self.run_fixture('VTF_CORPUS 1\nCONFORMANCE 2 0'))

    def test_missing_zero_duplicate_and_failing_checks_are_rejected(self):
        for summary in ('VTF_CORPUS 1', 'VTF_CORPUS 1\nCONFORMANCE 0 0',
                        'VTF_CORPUS 1\nCONFORMANCE 2 1',
                        'VTF_CORPUS 1\nCONFORMANCE 2 0\nCONFORMANCE 2 0'):
            with self.subTest(summary=summary):
                self.assertNotEqual(0, self.run_fixture(summary))

    def test_incomplete_texture_coverage_is_rejected(self):
        self.assertNotEqual(0, self.run_fixture('VTF_CORPUS 0\nCONFORMANCE 2 0'))

    def test_success_text_cannot_hide_process_failure(self):
        self.assertNotEqual(0, self.run_fixture('VTF_CORPUS 1\nCONFORMANCE 2 0', 1))

    def test_empty_archive_cannot_certify_coverage(self):
        self.assertNotEqual(0, self.run_fixture('VTF_CORPUS 0\nCONFORMANCE 2 0',
                                              has_texture=False))


if __name__ == '__main__':
    unittest.main()
