"""Negative fixtures for the installed Portal boot acceptance gate."""

import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))
SPEC = importlib.util.spec_from_file_location("portal_boot", QUALITY / "portal_boot.py")
boot = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(boot)

STATUS = '''hostname: Portal
map     : testchmb_a_00 at: 0 x, 0 y, 0 z
players : 1 humans, 0 bots (1 max)
# 2 "unnamed" STEAM_ID_LAN 00:03 0 0 active loopback
'''
PROVIDERS = "RFC0001 renderer: provider=vulkan-compat\nRFC0001 window: provider=sdl3 driver=wayland\n"


class AcceptanceTests(unittest.TestCase):
    def evaluate(self, log=STATUS, screenshots=None, code=0, timeout=False,
                 requirements=(), loaded=()):
        return boot.evaluate(log, [{"path": "fresh.tga", "has_scene_detail": True}] if screenshots is None else screenshots,
                             code, timeout, "testchmb_a_00", requirements, loaded)

    def test_success_requires_map_player_image_and_exit(self):
        self.assertEqual([], self.evaluate())

    def test_exit_zero_alone_fails(self):
        failures = self.evaluate(log="", screenshots=[])
        self.assertEqual(3, len(failures))

    def test_loading_message_does_not_prove_map_activation(self):
        self.assertTrue(self.evaluate(log="Spawn Server: testchmb_a_00\n"))

    def test_other_map_does_not_pass(self):
        self.assertTrue(self.evaluate(log=STATUS.replace("testchmb_a_00", "background1")))

    def test_connected_but_not_active_player_fails(self):
        self.assertTrue(self.evaluate(log=STATUS.replace("active loopback", "connecting loopback")))

    def test_missing_image_crash_and_timeout_each_fail(self):
        self.assertTrue(self.evaluate(screenshots=[]))
        self.assertTrue(self.evaluate(code=-11))
        self.assertTrue(self.evaluate(timeout=True))

    def test_blank_image_cannot_pass_product_gate(self):
        self.assertTrue(self.evaluate(screenshots=[{"path": "blank.tga", "has_scene_detail": False}]))

    def test_requested_configuration_cannot_attest_actual_providers(self):
        self.assertTrue(self.evaluate(log=STATUS + "SDL_VIDEODRIVER=wayland Vulkan requested",
                                      requirements=("vulkan", "sdl3", "wayland"),
                                      loaded=("libvulkan.so", "libSDL3.so")))

    def test_attestation_without_loaded_libraries_fails(self):
        self.assertTrue(self.evaluate(log=STATUS + PROVIDERS,
                                      requirements=("vulkan", "sdl3", "wayland")))

    def test_actual_provider_evidence_passes(self):
        self.assertEqual([], self.evaluate(log=STATUS + PROVIDERS,
                                          requirements=("vulkan", "sdl3", "wayland"),
                                          loaded=("/usr/lib/libvulkan.so.1", "/usr/lib/libSDL3.so.0")))


class StagingTests(unittest.TestCase):
    def test_mutable_files_are_independent_and_stale_screenshots_are_absent(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            runtime, stage = root / "original", root / "staged"
            (runtime / "portal/cfg").mkdir(parents=True)
            (runtime / "portal/screenshots").mkdir()
            (runtime / "portal/gameinfo.txt").write_text("gameinfo")
            (runtime / "portal/cfg/config.cfg").write_text("original configuration")
            (runtime / "portal/content.vpk").write_bytes(b"immutable content")
            (runtime / "portal/screenshots/stale.tga").write_bytes(b"old")
            (runtime / "engine.log").write_text(STATUS)
            boot.stage_runtime(runtime, stage)
            (stage / "portal/cfg/config.cfg").write_text("modified configuration")
            self.assertEqual("original configuration", (runtime / "portal/cfg/config.cfg").read_text())
            self.assertFalse((stage / "portal/cfg/config.cfg").is_symlink())
            self.assertTrue((stage / "portal/content.vpk").is_symlink())
            self.assertFalse((stage / "engine.log").exists())
            self.assertFalse((stage / "portal/screenshots").exists())

    def test_refuses_to_stage_inside_original_runtime(self):
        with tempfile.TemporaryDirectory() as directory:
            runtime = Path(directory)
            (runtime / "portal").mkdir()
            (runtime / "portal/gameinfo.txt").write_text("gameinfo")
            with self.assertRaises(ValueError):
                boot.stage_runtime(runtime, runtime / "output")

    def test_build_override_does_not_install_other_games(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            build, stage = root / "build", root / "stage"
            for relative in ("launcher_main/hl2_launcher", "engine/libengine.so",
                             "game/client/portal/libclient.so", "game/client/hl2/libclient.so"):
                path = build / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(relative)
            installed = boot.install_build(build, stage)
            self.assertEqual({"hl2_launcher", "bin/libengine.so", "portal/bin/libclient.so"}, set(installed))
            self.assertEqual("game/client/portal/libclient.so", (stage / "portal/bin/libclient.so").read_text())


class ScreenshotTests(unittest.TestCase):
    def test_truncated_or_non_image_output_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "frame.tga"
            path.write_bytes(b"not an image")
            self.assertIsNone(boot.screenshot_info(path))
            header = bytearray(18)
            header[2], header[16] = 2, 24
            struct.pack_into("<HH", header, 12, 64, 64)
            path.write_bytes(header)
            self.assertIsNone(boot.screenshot_info(path))
            path.write_bytes(header + bytes(64 * 64 * 3))
            self.assertEqual(64, boot.screenshot_info(path)["width"])
            self.assertFalse(boot.screenshot_info(path)["has_scene_detail"])
            path.write_bytes(header + bytes([255]) * (64 * 64 * 3))
            self.assertFalse(boot.screenshot_info(path)["has_scene_detail"])


if __name__ == "__main__":
    unittest.main()
