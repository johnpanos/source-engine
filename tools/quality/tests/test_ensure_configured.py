#!/usr/bin/env python3
"""Stale-configuration detection for tools/quality/ensure_configured.py."""

import importlib.util
import os
from pathlib import Path
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("ensure_configured",
                                              QUALITY / "ensure_configured.py")
ensure = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(ensure)


class StaleInputTests(unittest.TestCase):
    def make_tree(self, directory):
        root = Path(directory)
        (root / "scripts/waifulib").mkdir(parents=True)
        (root / "quality/toolchain").mkdir(parents=True)
        wscript = root / "wscript"
        for path in (wscript, root / "waf", root / "scripts/waifulib/tool.py",
                     root / "quality/toolchain/policy.json"):
            path.write_text("input")
            os.utime(path, (1000, 1000))
        build = root / "out"
        (build / "c4che").mkdir(parents=True)
        (build / "configuration.py").write_text("OPTIONS = {}\n")
        (build / ensure.LOCK).write_text("files = %r\nhash = b''\n" % [str(wscript)])
        (build / "c4che/_cache.py").write_text("CXXFLAGS = []\n")
        os.utime(build / "c4che/_cache.py", (2000, 2000))
        return root, build

    def test_current_tree_is_not_stale(self):
        with tempfile.TemporaryDirectory() as directory:
            root, build = self.make_tree(directory)
            self.assertEqual([], ensure.stale_inputs(build, root))

    def test_newer_policy_tool_or_wscript_is_stale(self):
        for relative in ("quality/toolchain/policy.json", "scripts/waifulib/tool.py", "wscript"):
            with self.subTest(relative=relative), tempfile.TemporaryDirectory() as directory:
                root, build = self.make_tree(directory)
                os.utime(root / relative, (3000, 3000))
                self.assertEqual([(root / relative).resolve()],
                                 ensure.stale_inputs(build, root))

    def test_removed_recorded_wscript_is_stale(self):
        with tempfile.TemporaryDirectory() as directory:
            root, build = self.make_tree(directory)
            (root / "wscript").unlink()
            self.assertEqual([root / "wscript"], ensure.stale_inputs(build, root))

    def test_missing_lock_or_cache_is_stale(self):
        for missing in (ensure.LOCK, "c4che/_cache.py"):
            with self.subTest(missing=missing), tempfile.TemporaryDirectory() as directory:
                root, build = self.make_tree(directory)
                (build / missing).unlink()
                self.assertTrue(ensure.stale_inputs(build, root))

    def test_tree_without_stored_options_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root, build = self.make_tree(directory)
            (build / "configuration.py").unlink()
            with self.assertRaisesRegex(ValueError, "no stored configuration"):
                ensure.stale_inputs(build, root)


if __name__ == "__main__":
    unittest.main()
