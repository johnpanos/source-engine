#!/usr/bin/env python3
"""Tests for the retail mirror's private write directory (stage_portal2_runtime)."""

from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))

import stage_portal2_runtime  # noqa: E402


def fake_install(root):
    install = root / "Portal 2"
    update = install / stage_portal2_runtime.RETAIL_WRITE_DIR
    (update / "cfg").mkdir(parents=True)
    (update / "pak01_dir.vpk").write_bytes(b"vpk")
    (update / "cfg/config.cfg").write_text('hud_quickinfo "1"\n')
    return install


class PrivateRetailWriteDirTests(unittest.TestCase):
    def test_config_writes_stay_in_the_mirror(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            install, mirror = fake_install(root), root / "mirror"
            # A mirror made before the fix links update/ whole.
            mirror.mkdir()
            (mirror / "update").symlink_to(install / "update", target_is_directory=True)

            update = stage_portal2_runtime.private_retail_write_dir(install, mirror)

            self.assertFalse(update.is_symlink())
            self.assertTrue((update / "pak01_dir.vpk").is_symlink())
            self.assertEqual((update / "pak01_dir.vpk").resolve(),
                             (install / "update/pak01_dir.vpk").resolve())
            self.assertFalse((update / "cfg").is_symlink())
            self.assertEqual((update / "cfg/config.cfg").read_text(), 'hud_quickinfo "1"\n')
            # What the retail binary does on quit after a harness's hud_quickinfo 0.
            (update / "cfg/config.cfg").write_text('hud_quickinfo "0"\n')
            self.assertEqual((install / "update/cfg/config.cfg").read_text(),
                             'hud_quickinfo "1"\n')

    def test_repeat_keeps_the_mirror_config(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            install, mirror = fake_install(root), root / "mirror"
            update = stage_portal2_runtime.private_retail_write_dir(install, mirror)
            (update / "cfg/config.cfg").write_text('hud_quickinfo "0"\n')
            (install / "update/pak02_dir.vpk").write_bytes(b"vpk")

            stage_portal2_runtime.private_retail_write_dir(install, mirror)

            self.assertEqual((update / "cfg/config.cfg").read_text(), 'hud_quickinfo "0"\n')
            self.assertTrue((update / "pak02_dir.vpk").is_symlink())

    def test_linked_update_leaks_to_the_installation(self):
        # Control: the layout the helper replaces does write through.
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            install, mirror = fake_install(root), root / "mirror"
            mirror.mkdir()
            (mirror / "update").symlink_to(install / "update", target_is_directory=True)
            (mirror / "update/cfg/config.cfg").write_text('hud_quickinfo "0"\n')
            self.assertEqual((install / "update/cfg/config.cfg").read_text(),
                             'hud_quickinfo "0"\n')


if __name__ == "__main__":
    unittest.main()
