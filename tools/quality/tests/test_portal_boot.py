"""Negative fixtures for the installed Portal boot acceptance gate."""

import importlib.util
import contextlib
import hashlib
import io
import json
from pathlib import Path
import struct
import sys
import tarfile
import tempfile
import unittest
from unittest import mock

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))
SPEC = importlib.util.spec_from_file_location("portal_boot", QUALITY / "portal_boot.py")
boot = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(boot)
import product_profile

TRACE_SPEC = importlib.util.spec_from_file_location("trace_fixture", QUALITY / "tests/test_render_trace.py")
trace_fixture = importlib.util.module_from_spec(TRACE_SPEC)
TRACE_SPEC.loader.exec_module(trace_fixture)

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

    def test_missing_shader_permutation_fails_even_with_visible_image(self):
        self.assertTrue(self.evaluate(log=STATUS + "Couldn't load combo 42 of shader"))
        self.assertTrue(self.evaluate(log=STATUS + "Couldn't load pixel shader example_ps20"))

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

    def test_unrelated_library_paths_cannot_attest_native_providers(self):
        self.assertTrue(self.evaluate(log=STATUS + PROVIDERS,
                                      requirements=("vulkan", "sdl3"),
                                      loaded=("/libvulkan/cache/libother.so", "/tmp/fakelibSDL3.so")))


class ResizeAcceptanceTests(unittest.TestCase):
    def test_resize_workload_uses_the_frame_command_buffer(self):
        self.assertEqual(
            ["mat_queue_mode 2", "wait 120", "mat_resizewindow 641 479", "wait 12",
             "screenshot", "wait 1", "wait 10", "quit"],
            boot.resize_commands(((641, 479),)))

    def test_resize_script_is_one_line_and_records_its_hash(self):
        with tempfile.TemporaryDirectory() as root:
            stage = Path(root)
            result = boot.install_resize_script(stage, ((641, 479),))
            script = stage / result["path"]
            self.assertEqual(1, len(script.read_text().splitlines()))
            self.assertIn("wait 120; mat_resizewindow 641 479; wait 12", script.read_text())
            self.assertEqual(boot.sha256(script), result["sha256"])

    def test_long_resize_workload_chains_scripts_below_the_line_limit(self):
        with tempfile.TemporaryDirectory() as root:
            stage = Path(root)
            result = boot.install_resize_script(stage, boot.RESIZE_WORKLOAD, "sync")
            files = [stage / result["path"]] + [stage / item["path"] for item in result["chained"]]
            self.assertGreater(len(files), 1)
            replayed = []
            for index, path in enumerate(files):
                lines = path.read_text().splitlines()
                self.assertEqual(1, len(lines))
                self.assertLessEqual(len(lines[0]), boot.RESIZE_SCRIPT_LINE_LIMIT)
                commands = lines[0].split("; ")
                if index + 1 < len(files):
                    self.assertEqual("exec " + files[index + 1].stem, commands[-1])
                    commands = commands[:-1]
                replayed += commands
            self.assertEqual(result["commands"], replayed)
            self.assertEqual("quit", replayed[-1])

    def test_every_resize_needs_consumption_and_nonblank_matching_image(self):
        expected = ((641, 479), (1024, 768))
        log = "".join(
            "RFC0001 resize observed: logical=%dx%d drawable=%dx%d\n"
            "RFC0001 resize queued: serial=%d drawable=%dx%d request_us=12\n"
            "RFC0001 resize complete: serial=%d drawable=%dx%d main_wait_us=0\n" %
            (w, h, w * 2, h * 2, serial, w * 2, h * 2, serial, w * 2, h * 2)
            for serial, (w, h) in enumerate(expected, 1))
        images = [{"width": w * 2, "height": h * 2, "has_scene_detail": True}
                  for w, h in expected]
        trace = [{"event": "present", "cropped": False, "result": 0}]
        self.assertEqual("pass", boot.inspect_resize(log, images, expected, trace)["status"])
        self.assertEqual("fail", boot.inspect_resize(log, images[:1], expected, trace)["status"])
        self.assertEqual("fail", boot.inspect_resize("", images, expected, trace)["status"])
        images[0]["has_scene_detail"] = False
        self.assertEqual("fail", boot.inspect_resize(log, images, expected, trace)["status"])

    def test_sync_resize_workload_runs_without_the_render_worker_and_drags(self):
        commands = boot.resize_commands(((641, 479),), "sync")
        self.assertEqual(["mat_queue_mode 0", "wait 120", "mat_resizewindow 641 479", "wait 12",
                          "screenshot", "wait 1"], commands[:6])
        drag = ["mat_resizewindow %d %d" % size for size in boot.RESIZE_DRAG_WORKLOAD]
        self.assertEqual(drag, [c for c in commands[6:] if c.startswith("mat_resizewindow")])
        # One size per frame, with frames captured mid-drag.
        self.assertEqual("wait 1", commands[commands.index(drag[0]) + 1])
        self.assertEqual(len(boot.RESIZE_DRAG_WORKLOAD) // 4 + 2, commands.count("screenshot"))
        self.assertEqual(["wait 10", "quit"], commands[-2:])

    def test_sync_resize_rejects_unresized_drawables_and_foreign_images(self):
        log = ("RFC0001 resize observed: logical=641x479 drawable=1282x958\n"
               "RFC0001 resize queued: serial=1 drawable=1282x958 request_us=9000\n"
               "RFC0001 resize complete: serial=1 drawable=1282x958 main_wait_us=0\n"
               "[vulkan] presents=40 scaled=0\n")
        images = [{"width": 1282, "height": 958, "has_scene_detail": True}]
        def status(text, frames):
            return boot.inspect_resize(text, frames, ((641, 479),), mode="sync",
                                       require_unscaled=True)["status"]
        self.assertEqual("pass", status(log, images))
        # A drawable seen mid-drag that was never resized to.
        self.assertEqual("fail", status(
            log + "RFC0001 resize observed: logical=700x500 drawable=1400x1000\n", images))
        # A captured frame the renderer never resized to (a stretched or stale
        # size), and a blank one.
        self.assertEqual("fail", status(log, images + [{"width": 1300, "height": 958,
                                                          "has_scene_detail": True}]))
        self.assertEqual("fail", status(log, images + [{"width": 1282, "height": 958,
                                                          "has_scene_detail": False}]))

    def test_sync_resize_budget_is_one_frame_and_requires_unscaled_presents(self):
        def log(request_us, census):
            return ("RFC0001 resize observed: logical=641x479 drawable=1282x958\n"
                    "RFC0001 resize queued: serial=1 drawable=1282x958 request_us=%d\n"
                    "RFC0001 resize complete: serial=1 drawable=1282x958 main_wait_us=0\n%s"
                    % ( request_us, census ))
        images = [{"width": 1282, "height": 958, "has_scene_detail": True}]
        unscaled = "[vulkan] presents=40 scaled=0\n"
        def status(text, **kwargs):
            return boot.inspect_resize(text, images, ((641, 479),), mode="sync", **kwargs)
        # A synchronous resize may spend a frame; the queued 2 ms publication
        # budget does not apply to it.
        self.assertEqual("pass", status(log(9000, unscaled), require_unscaled=True)["status"])
        self.assertEqual("fail", status(log(17000, unscaled), require_unscaled=True)["status"])
        # A stretched present or a missing census fails.
        self.assertEqual("fail", status(log(9000, "[vulkan] presents=40 scaled=1\n"),
                                        require_unscaled=True)["status"])
        self.assertEqual("fail", status(log(9000, ""), require_unscaled=True)["status"])

    def test_resize_rejects_main_thread_wait_budget_and_cropped_present(self):
        log = ("RFC0001 resize observed: logical=641x479 drawable=1282x958\n"
               "RFC0001 resize queued: serial=1 drawable=1282x958 request_us=2001\n"
               "RFC0001 resize complete: serial=1 drawable=1282x958 main_wait_us=1\n")
        images = [{"width": 1282, "height": 958, "has_scene_detail": True}]
        result = boot.inspect_resize(
            log, images, ((641, 479),), [{"event": "present", "cropped": True}])
        self.assertEqual("fail", result["status"])
        self.assertEqual(3, len(result["failures"]))


class ProviderCatalogTests(unittest.TestCase):
    initialized = (
        "RFC0001 input: provider=sdl3\n"
        "RFC0001 audio: provider=sdl3\n"
        "RFC0001 shaders: provider=source-standard-materials shaders=400\n"
        "RFC0001 video: providers=0\n"
    )
    telemetry = ("ModuleLoadTelemetry: op=0 id=1 requester=game:1 requested=client.so "
                 "resolved=/runtime/portal/bin/client.so entry= success=1\n")

    def test_actual_initialized_providers_and_retained_game_host_pass(self):
        result = boot.inspect_provider_catalog(self.initialized + self.telemetry)
        self.assertEqual("pass", result["status"])
        self.assertEqual(1, result["telemetry_records"])

    def test_missing_or_null_audio_is_not_native_audio_acceptance(self):
        for log in ("", self.initialized, self.telemetry,
                    self.initialized.replace("audio: provider=sdl3", "audio: provider=null") + self.telemetry,
                    self.initialized.replace("shaders=400", "shaders=0") + self.telemetry):
            with self.subTest(log=log):
                self.assertEqual("fail", boot.inspect_provider_catalog(log)["status"])

    def test_even_failed_builtin_discovery_violates_retirement(self):
        for name in ("stdshader_dx9.so", "/runtime/bin/libstdshader_dx9.so", "shaderapidx9.dll",
                     "video_bink.so", "video_webm.dll", "video_quicktime.dylib", "vaudio_minimp3.so", "libvaudio_opus.so"):
            bad = self.telemetry.replace("requested=client.so", "requested=" + name).replace("success=1", "success=0")
            with self.subTest(name=name):
                result = boot.inspect_provider_catalog(self.initialized + bad)
                self.assertEqual("fail", result["status"])
                self.assertEqual([name], result["forbidden_attempts"])

    def test_malformed_telemetry_fails_closed(self):
        self.assertEqual("fail", boot.inspect_provider_catalog(
            self.initialized + "ModuleLoadTelemetry: truncated\n")["status"])


class StagingTests(unittest.TestCase):
    def make_build(self, root, games="portal"):
        build, stage = root / "build", root / "stage"
        for relative in ("launcher_main/hl2_launcher", "engine/libengine.so",
                         "game/client/libclient.so", "game/server/libserver.so"):
            path = build / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(relative)
        (build / "c4che").mkdir()
        if games is not None:
            (build / "c4che/_cache.py").write_text("GAMES = %r\n" % games)
        return build, stage

    def add_native_package(self, root, build):
        profile = product_profile.load_profile()
        dependency = profile["dependencies"]["dxvk_native"]
        cache = root / "dependencies"
        tree = cache / dependency["extracted_directory"]
        prefix = tree / dependency["prefix"]
        for relative in ("include/dxvk/d3d9.h", "include/dxvk/wsi/native_sdl3.h",
                         "lib/libdxvk_d3d9.so.0.20701"):
            path = prefix / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(relative.encode())
        (prefix / "lib/libdxvk_d3d9.so.0").symlink_to("libdxvk_d3d9.so.0.20701")
        (prefix / "lib/libdxvk_d3d9.so").symlink_to("libdxvk_d3d9.so.0")
        archive = cache / dependency["cache_archive"]
        with tarfile.open(archive, "w:gz") as stream:
            stream.add(prefix, arcname="usr")
        dependency["sha256"] = hashlib.sha256(archive.read_bytes()).hexdigest()
        dependency["archive_bytes"] = archive.stat().st_size
        profile_path = root / "profile.json"
        profile_path.write_text(json.dumps(profile))
        with (build / "c4che/_cache.py").open("a") as configuration:
            configuration.write("LIBPATH_DXVK = %r\n" % [str(prefix / "lib")])
            configuration.write("DXVK_ROOT = %r\n" % str(prefix))
            configuration.write("PRODUCT_PROFILE = %r\n" % str(profile_path))
        return prefix, dependency

    def test_unqualified_portal_game_outputs_are_installed_in_gamebin(self):
        with tempfile.TemporaryDirectory() as directory:
            build, stage = self.make_build(Path(directory))
            installed = boot.install_build(build, stage)
            self.assertIn("portal/bin/libclient.so", installed)
            self.assertIn("portal/bin/libserver.so", installed)
            self.assertNotIn("bin/libclient.so", installed)
            self.assertNotIn("bin/libserver.so", installed)

    def test_dedicated_launcher_uses_same_gamebin_staging(self):
        with tempfile.TemporaryDirectory() as directory:
            build, stage = self.make_build(Path(directory))
            (build / "launcher_main/hl2_launcher").unlink()
            launcher = build / "dedicated_main/dedicated_launcher"
            launcher.parent.mkdir(parents=True)
            launcher.write_text("dedicated")
            installed = boot.install_build(build, stage, launcher_name="dedicated_launcher")
            self.assertEqual("dedicated", (stage / "dedicated_launcher").read_text())
            self.assertIn("portal/bin/libserver.so", installed)
            self.assertNotIn("bin/libserver.so", installed)
            with self.assertRaisesRegex(ValueError, "hl2_launcher"):
                boot.install_build(build, stage)

    def test_portal2_game_outputs_use_separate_gamebin(self):
        with tempfile.TemporaryDirectory() as directory:
            build, stage = self.make_build(Path(directory), "portal2")
            installed = boot.install_build(build, stage, game="portal2")
            self.assertIn("portal2/bin/libclient.so", installed)
            self.assertIn("portal2/bin/libserver.so", installed)
            self.assertNotIn("portal/bin/libclient.so", installed)

    def test_unqualified_game_outputs_require_unambiguous_portal_selection(self):
        for selection in (None, "hl2", "portal,hl2"):
            with self.subTest(selection=selection), tempfile.TemporaryDirectory() as directory:
                build, stage = self.make_build(Path(directory), selection)
                installed = boot.install_build(build, stage)
                self.assertNotIn("portal/bin/libclient.so", installed)
                self.assertNotIn("portal/bin/libserver.so", installed)

    def test_conflicting_waf_game_selections_do_not_label_other_game_as_portal(self):
        with tempfile.TemporaryDirectory() as directory:
            build, stage = self.make_build(Path(directory))
            (build / "c4che/other_cache.py").write_text("GAMES = 'hl2'\n")
            installed = boot.install_build(build, stage)
            self.assertNotIn("portal/bin/libclient.so", installed)

    def test_build_cache_is_data_and_cannot_execute_python(self):
        with tempfile.TemporaryDirectory() as directory:
            build, stage = self.make_build(Path(directory))
            sentinel = Path(directory) / "must-not-exist"
            (build / "c4che/_cache.py").write_text(
                "GAMES = __import__('pathlib').Path(%r).touch()\n" % str(sentinel))
            with self.assertRaises(ValueError):
                boot.install_build(build, stage)
            self.assertFalse(sentinel.exists())

    def test_ambiguous_output_names_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            build, stage = self.make_build(Path(directory))
            (build / "duplicate").mkdir()
            (build / "duplicate/libengine.so").write_text("other engine")
            with self.assertRaisesRegex(ValueError, "ambiguous"):
                boot.install_build(build, stage)

    def test_nested_waf_profile_outputs_are_not_staged(self):
        with tempfile.TemporaryDirectory() as directory:
            build, stage = self.make_build(Path(directory))
            (build / "materialsystem/stdshaders").mkdir(parents=True)
            (build / "materialsystem/stdshaders/libstdshader_dx9.so").write_text(
                "active profile shader")
            nested = build / "pbr-native"
            (nested / "c4che").mkdir(parents=True)
            (nested / "materialsystem/stdshaders").mkdir(parents=True)
            (nested / "materialsystem/stdshaders/libstdshader_dx9.so").write_text(
                "other profile shader")
            (nested / "launcher_main/hl2_launcher").parent.mkdir(parents=True)
            (nested / "launcher_main/hl2_launcher").write_text("other profile launcher")

            installed = boot.install_build(build, stage)

            self.assertEqual("engine/libengine.so", (stage / "bin/libengine.so").read_text())
            self.assertEqual("active profile shader",
                             (stage / "bin/libstdshader_dx9.so").read_text())
            self.assertIn("bin/libstdshader_dx9.so", installed)
            self.assertNotIn("pbr-native", installed["hl2_launcher"]["source"])

    def test_native_sonames_are_staged_as_independent_verified_files(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            build, stage = self.make_build(root)
            prefix, dependency = self.add_native_package(root, build)
            installed = boot.install_build(build, stage)
            for name in ("libdxvk_d3d9.so", "libdxvk_d3d9.so.0", "libdxvk_d3d9.so.0.20701"):
                destination = stage / "bin" / name
                self.assertFalse(destination.is_symlink())
                self.assertEqual(destination.read_bytes(), (prefix / "lib" / name).read_bytes())
                self.assertEqual(installed["bin/" + name]["sha256"], boot.sha256(destination))

    def test_tampered_native_library_is_rejected_after_configuration(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            build, stage = self.make_build(root)
            prefix, _ = self.add_native_package(root, build)
            (prefix / "lib/libdxvk_d3d9.so.0.20701").write_bytes(b"tampered")
            with self.assertRaisesRegex(ValueError, "differs from pinned archive"):
                boot.install_build(build, stage)
            self.assertFalse((stage / "bin/libdxvk_d3d9.so").exists())

    def test_extra_native_search_path_cannot_replace_pinned_soname(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            build, stage = self.make_build(root)
            prefix, _ = self.add_native_package(root, build)
            other = root / "other"
            other.mkdir()
            (other / "libdxvk_d3d9.so").write_bytes(b"wrong provider")
            with (build / "c4che/_cache.py").open("a") as configuration:
                configuration.write("LIBPATH_DXVK = %r\n" % [str(other)])
            with self.assertRaises(ValueError):
                boot.install_build(build, stage)

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

    def test_portal2_content_only_stages_vpks_without_retail_binaries(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            runtime, stage = root / "retail", root / "staged"
            (runtime / "portal2/bin").mkdir(parents=True)
            (runtime / "bin").mkdir()
            (runtime / "portal2/gameinfo.txt").write_text("gameinfo")
            (runtime / "portal2/pak01_dir.vpk").write_bytes(b"vpk")
            (runtime / "portal2/bin/client.so").write_bytes(b"retail")
            (runtime / "bin/engine.so").write_bytes(b"retail")

            boot.stage_runtime(runtime, stage, game="portal2", content_only=True)

            self.assertTrue((stage / "portal2/pak01_dir.vpk").is_symlink())
            self.assertTrue((stage / "portal2/gameinfo.txt").is_file())
            self.assertFalse((stage / "portal2/bin/client.so").exists())
            self.assertFalse((stage / "bin/engine.so").exists())

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
            path.write_bytes(header + bytes([128]) * (64 * 64 * 3))
            self.assertFalse(boot.screenshot_info(path)["has_scene_detail"])
            path.write_bytes(header + bytes([20, 90, 170]) * (64 * 64))
            self.assertFalse(boot.screenshot_info(path)["has_scene_detail"])
            path.write_bytes(header + bytes(range(256)) * 48)
            self.assertTrue(boot.screenshot_info(path)["has_scene_detail"])


class ShaderOverlayTests(unittest.TestCase):
    def fixture(self, root):
        repo, artifacts, stage = root / "repo", root / "artifacts", root / "stage"
        source = repo / "materialsystem/shaders/Downsample_vs20.fxc"
        source.parent.mkdir(parents=True)
        source.write_text("matching source")
        compiler = repo / "fxc.exe"
        compiler.write_bytes(b"pinned compiler")
        binary = artifacts / "shaders/fxc/Downsample_vs20.vcs"
        binary.parent.mkdir(parents=True)
        binary.write_bytes(b"compiled shader")
        manifest = {"schema": 1, "status": "passed", "coverage": "observed-static-sets",
                    "compiler": {"path": "fxc.exe", "sha256": boot.sha256(compiler)},
                    "shaders": [{"name": "Downsample_vs20", "path": "shaders/fxc/Downsample_vs20.vcs",
                                 "sha256": boot.sha256(binary),
                                 "sources": {str(source.relative_to(repo)): boot.sha256(source)}}]}
        (artifacts / "manifest.json").write_text(json.dumps(manifest))
        (stage / "portal").mkdir(parents=True)
        (stage / "portal/gameinfo.txt").write_text(
            '"GameInfo" { FileSystem { SearchPaths { game+mod portal/portal_pak.vpk } } }')
        return repo, artifacts, stage, manifest

    def test_only_manifest_shaders_are_copied_into_private_game_tree(self):
        with tempfile.TemporaryDirectory() as directory:
            repo, artifacts, stage, manifest = self.fixture(Path(directory))
            (artifacts / "shaders/fxc/unlisted.vcs").write_text("not enumerated")
            evidence = boot.install_shader_artifacts(artifacts, stage, repo)
            destination = stage / "portal/custom/source-engine-shaders/shaders/fxc/Downsample_vs20.vcs"
            self.assertEqual(b"compiled shader", destination.read_bytes())
            self.assertFalse(destination.is_symlink())
            self.assertFalse((stage / "portal/custom/source-engine-shaders/shaders/fxc/unlisted.vcs").exists())
            self.assertEqual(1, len(evidence["shaders"]))
            self.assertEqual(boot.sha256(artifacts / "manifest.json"), evidence["manifest_sha256"])

    def test_shader_search_priority_precedes_packaged_assets_without_mutating_original(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            repo, artifacts, stage, manifest = self.fixture(root)
            original = root / "original"
            stage.rename(original)
            before = boot.sha256(original / "portal/gameinfo.txt")
            boot.stage_runtime(original, stage)
            boot.install_shader_artifacts(artifacts, stage, repo)
            self.assertEqual(before, boot.sha256(original / "portal/gameinfo.txt"))
            gameinfo = (stage / "portal/gameinfo.txt").read_text()
            self.assertRegex(gameinfo, r"SearchPaths\s*\{\s*game\+mod\s+portal/custom/source-engine-shaders")
            self.assertLess(gameinfo.index("portal/custom/source-engine-shaders"),
                            gameinfo.index("portal/portal_pak.vpk"))
            self.assertFalse((original / "portal/custom").exists())

    def test_missing_duplicate_and_commented_searchpaths_fail_before_copy(self):
        for gameinfo in ('"GameInfo" { FileSystem { } }',
                         '"GameInfo" { FileSystem { // SearchPaths { }\n } }',
                         '"GameInfo" { FileSystem { SearchPaths { } SearchPaths { } } }'):
            with self.subTest(gameinfo=gameinfo), tempfile.TemporaryDirectory() as directory:
                repo, artifacts, stage, manifest = self.fixture(Path(directory))
                (stage / "portal/gameinfo.txt").write_text(gameinfo)
                with self.assertRaisesRegex(ValueError, "SearchPaths"):
                    boot.install_shader_artifacts(artifacts, stage, repo)
                self.assertFalse((stage / "portal/custom").exists())
                self.assertEqual(gameinfo, (stage / "portal/gameinfo.txt").read_text())

    def test_incomplete_empty_and_wrong_schema_manifests_fail(self):
        for change in ({"status": "incomplete"}, {"shaders": []}, {"schema": 2}, {"shaders": None}):
            with self.subTest(change=change), tempfile.TemporaryDirectory() as directory:
                repo, artifacts, stage, manifest = self.fixture(Path(directory))
                manifest.update(change)
                (artifacts / "manifest.json").write_text(json.dumps(manifest))
                with self.assertRaises(ValueError):
                    boot.install_shader_artifacts(artifacts, stage, repo)
                self.assertFalse((stage / "portal/custom").exists())

    def test_changed_source_or_shader_fails_before_copy(self):
        for changed in ("source", "shader", "compiler"):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as directory:
                repo, artifacts, stage, manifest = self.fixture(Path(directory))
                path = (repo / next(iter(manifest["shaders"][0]["sources"]))) if changed == "source" else (
                    artifacts / manifest["shaders"][0]["path"])
                if changed == "compiler":
                    path = repo / manifest["compiler"]["path"]
                path.write_text("changed since compiler run")
                with self.assertRaisesRegex(ValueError, "hash"):
                    boot.install_shader_artifacts(artifacts, stage, repo)
                self.assertFalse((stage / "portal/custom").exists())

    def test_malformed_and_escaping_shader_entries_fail_before_copy(self):
        for field, value in (("path", "../outside.vcs"), ("name", "../outside"),
                             ("sources", {}), ("sources", {"../outside.fxc": "0" * 64}),
                             ("sha256", None)):
            with self.subTest(field=field, value=value), tempfile.TemporaryDirectory() as directory:
                repo, artifacts, stage, manifest = self.fixture(Path(directory))
                manifest["shaders"][0][field] = value
                (artifacts / "manifest.json").write_text(json.dumps(manifest))
                with self.assertRaises(ValueError):
                    boot.install_shader_artifacts(artifacts, stage, repo)
                self.assertFalse((stage / "portal/custom").exists())

    def test_duplicate_shader_cannot_replace_valid_entry(self):
        with tempfile.TemporaryDirectory() as directory:
            repo, artifacts, stage, manifest = self.fixture(Path(directory))
            manifest["shaders"].append(manifest["shaders"][0])
            (artifacts / "manifest.json").write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError, "duplicate"):
                boot.install_shader_artifacts(artifacts, stage, repo)
            self.assertFalse((stage / "portal/custom").exists())

    def test_late_invalid_shader_does_not_partially_replace_existing_stage(self):
        with tempfile.TemporaryDirectory() as directory:
            repo, artifacts, stage, manifest = self.fixture(Path(directory))
            destination = stage / "portal/custom/source-engine-shaders/shaders/fxc/Downsample_vs20.vcs"
            destination.parent.mkdir(parents=True)
            destination.write_text("original installed shader")
            manifest["shaders"].append({"name": "invalid"})
            (artifacts / "manifest.json").write_text(json.dumps(manifest))
            with self.assertRaises(ValueError):
                boot.install_shader_artifacts(artifacts, stage, repo)
            self.assertEqual("original installed shader", destination.read_text())


class TraceCaptureTests(unittest.TestCase):
    def run_capture(self, root, write_trace):
        runtime, output = root / "original", root / "capture"
        (runtime / "portal").mkdir(parents=True)
        (runtime / "portal/gameinfo.txt").write_text("gameinfo")
        (runtime / "hl2_launcher").write_bytes(b"launcher")

        def product(command, stage, environment, timeout, stdout):
            self.assertEqual(str(output / "render-trace.jsonl"), environment["SOURCE_RENDER_TRACE"])
            self.assertNotIn(str(runtime), environment["SOURCE_RENDER_TRACE"])
            Path(stdout).write_text(STATUS)
            screenshot = stage / "portal/screenshots/frame.tga"
            screenshot.parent.mkdir(parents=True)
            header = bytearray(18)
            header[2], header[16] = 2, 24
            struct.pack_into("<HH", header, 12, 64, 64)
            screenshot.write_bytes(header + bytes(range(256)) * 48)
            if write_trace:
                events = trace_fixture.capture(trace_fixture.draw())
                contents = "".join(json.dumps(event) + "\n" for event in events)
                Path(environment["SOURCE_RENDER_TRACE"]).write_text(
                    '{"event":"present"}\n' if write_trace == "invalid" else contents)
            return 0, False, [], 1.0

        with mock.patch.object(boot, "run_product", side_effect=product), mock.patch.object(
                boot.conformance, "source_identity", return_value={}), contextlib.redirect_stdout(io.StringIO()):
            result = boot.main(["--runtime", str(runtime), "--out", str(output), "--render-trace"])
        return result, json.loads((output / "evidence.json").read_text())

    def test_requested_trace_is_recorded_with_content_hash(self):
        with tempfile.TemporaryDirectory() as directory:
            result, evidence = self.run_capture(Path(directory), True)
            self.assertEqual(0, result)
            trace = evidence["render_trace"]
            self.assertEqual(boot.sha256(trace["path"]), trace["sha256"])
            self.assertGreater(trace["bytes"], 0)

    def test_incomplete_trace_fails_even_when_product_capture_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            result, evidence = self.run_capture(Path(directory), "invalid")
            self.assertEqual(1, result)
            self.assertEqual("invalid", evidence["render_trace"]["status"])

    def test_missing_requested_trace_fails_even_when_product_capture_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            result, evidence = self.run_capture(Path(directory), False)
            self.assertEqual(1, result)
            self.assertIn("requested renderer trace was not produced", evidence["failures"])


if __name__ == "__main__":
    unittest.main()


class TraceGateTests(unittest.TestCase):
    def report(self, contents):
        with tempfile.TemporaryDirectory() as directory:
            trace = Path(directory) / "trace.jsonl"
            report = Path(directory) / "report.json"
            trace.write_text(contents)
            result = boot.inspect_render_trace(trace, report)
            self.assertEqual(json.loads(json.dumps(result)), json.loads(report.read_text()))
            return result

    def test_existing_but_truncated_trace_fails(self):
        self.assertEqual("invalid", self.report('{"event":"capture_begin"}')['status'])

    def test_zero_test_capture_fails(self):
        self.assertEqual("invalid", self.report('')['status'])

    def test_real_collector_capture_and_seeded_shader_failure(self):
        # Reuse the standalone outcome fixture, never the production collector's
        # classification code, to exercise the installed product gate.
        for failed, expected in ((False, "complete"), (True, "issues_observed")):
            events = trace_fixture.capture(trace_fixture.draw(ps_requested=trace_fixture.shader(failed=failed)))
            result = self.report("".join(json.dumps(event) + "\n" for event in events))
            self.assertEqual(expected, result['status'])
            self.assertFalse(result['visibility_verified'])
