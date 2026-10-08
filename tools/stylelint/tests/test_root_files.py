"""The repository root allowlist (tools/stylelint/root_files.py, RFC 0027)."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import root_files  # noqa: E402


def git_repository(files, submodule=None):
    directory = tempfile.TemporaryDirectory()
    root = Path(directory.name)
    subprocess.run(["git", "init", "-q"], cwd=root, check=True)
    for name in files:
        path = root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("x\n")
    subprocess.run(["git", "add", "-A"], cwd=root, check=True)
    if submodule:
        # A gitlink (mode 160000), as a submodule records itself.
        subprocess.run(["git", "update-index", "--add", "--cacheinfo",
                        "160000,%s,%s" % ("1" * 40, submodule)], cwd=root, check=True)
    return directory, root


class RootFilesTests(unittest.TestCase):
    def test_the_repository_passes(self):
        self.assertEqual([], root_files.violations(root_files.tracked_root_entries(root_files.ROOT)))

    def test_allowed_files_directories_and_submodules_pass(self):
        directory, root = git_repository(["wscript", "kiln", "README.md", "tools/x.py"],
                                         submodule="thirdparty")
        with directory:
            entries = root_files.tracked_root_entries(root)
            self.assertTrue(entries["tools"] and entries["thirdparty"])
            self.assertEqual([], root_files.violations(entries, pending={}))

    def test_seeded_stray_root_file_is_caught(self):
        directory, root = git_repository(["wscript", "BC7.patch", "fix_paths.py"])
        with directory:
            problems = root_files.violations(root_files.tracked_root_entries(root), pending={})
        self.assertEqual(2, len(problems))
        self.assertTrue(any(p.startswith("BC7.patch:") for p in problems))
        self.assertTrue(any(p.startswith("fix_paths.py:") for p in problems))

    def test_a_pending_entry_for_a_deleted_file_fails(self):
        directory, root = git_repository(["wscript"])
        with directory:
            problems = root_files.violations(root_files.tracked_root_entries(root),
                                             pending={"build-3ds.sh": "L7"})
        self.assertEqual(["build-3ds.sh: gone; remove its PENDING entry (L7); the list only shrinks"],
                         problems)

    def test_a_pending_file_is_allowed_while_it_exists(self):
        directory, root = git_repository(["wscript", "ios-deploy.sh"])
        with directory:
            self.assertEqual([], root_files.violations(root_files.tracked_root_entries(root),
                                                       pending={"ios-deploy.sh": "L7"}))


if __name__ == "__main__":
    unittest.main()
