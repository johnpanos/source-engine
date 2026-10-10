"""Negative fixtures for the installed Portal boot acceptance gate."""

import importlib.util
import contextlib
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
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
PROVIDERS = "[NativeVulkan] IShaderAPI::SetMode: device Fixture GPU up\nRFC0001 window: provider=sdl3 driver=wayland\n"



def fake_kiln():
    """portal_boot's kiln session, packaging a minimal Portal runtime."""
    def build(profile, flavor=None, up_to=None, runtime=None):
        stage = Path(runtime)
        (stage / "portal").mkdir(parents=True)
        (stage / "portal/gameinfo.txt").write_text("gameinfo")
        (stage / "hl2_launcher").write_bytes(b"launcher")
        return {"stages": [{"name": "package", "summary": "fake"}], "tree": "tree"}
    session = mock.Mock()
    session.build.side_effect = build
    sepipe = mock.Mock()
    sepipe.Session.return_value = session
    stack = contextlib.ExitStack()
    stack.enter_context(mock.patch.object(boot.sepipe_loader, "load", return_value=sepipe))
    stack.enter_context(mock.patch.object(boot.sepipe_loader, "game_of", return_value="portal"))
    return stack

class AcceptanceTests(unittest.TestCase):
    def evaluate(self, log=STATUS, screenshots=None, code=0, timeout=False,
                 requirements=(), loaded=(), renderer=None):
        return boot.evaluate(log, [{"path": "fresh.tga", "has_scene_detail": True}] if screenshots is None else screenshots,
                             code, timeout, "testchmb_a_00", requirements, loaded, renderer)

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

    def test_native_vulkan_needs_its_own_live_provider_marker(self):
        native = (STATUS + "RFC0001 window: provider=sdl3 driver=offscreen\n"
                  "[NativeVulkan] IShaderAPI::SetMode: device 'GPU' up (back buffer 1024x768)\n")
        loaded = ("/usr/lib/libvulkan.so.1", "/usr/lib/libSDL3.so.0")
        self.assertEqual([], self.evaluate(log=native, requirements=("vulkan", "sdl3"),
                                          loaded=loaded, renderer="core"))
        self.assertTrue(self.evaluate(log=STATUS + "RFC0001 window: provider=sdl3 driver=wayland\n",
                                      requirements=("vulkan", "sdl3"), loaded=loaded,
                                      renderer="core"))

    def test_unrelated_library_paths_cannot_attest_native_providers(self):
        self.assertTrue(self.evaluate(log=STATUS + PROVIDERS,
                                      requirements=("vulkan", "sdl3"),
                                      loaded=("/libvulkan/cache/libother.so", "/tmp/fakelibSDL3.so")))


class UserDisplayTests(unittest.TestCase):
    LOGIN = {"WAYLAND_DISPLAY": "wayland-0", "DISPLAY": ":0"}

    def test_a_windowed_run_on_the_login_session_is_caught(self):
        self.assertEqual(boot.user_display_in_use(
            {"WAYLAND_DISPLAY": "wayland-0", "DISPLAY": ":0"}, self.LOGIN),
            ["DISPLAY=:0", "WAYLAND_DISPLAY=wayland-0"])
        self.assertEqual(boot.user_display_in_use({"DISPLAY": ":0"}, self.LOGIN),
                         ["DISPLAY=:0"])

    def test_offscreen_and_private_compositor_runs_pass(self):
        self.assertEqual(boot.user_display_in_use(
            {"SDL_VIDEODRIVER": "offscreen", "DISPLAY": ":0"}, self.LOGIN), [])
        self.assertEqual(boot.user_display_in_use(
            {"WAYLAND_DISPLAY": "wl-resize-42"}, self.LOGIN), [])
        self.assertEqual(boot.user_display_in_use({"DISPLAY": ":3"}, self.LOGIN), [])

    def test_no_login_session_means_no_guard(self):
        self.assertEqual(boot.user_display_in_use({"DISPLAY": ":0"}, {}), [])

    def test_main_refuses_a_live_desktop_run_before_the_product_starts(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "boot"
            product = mock.Mock()
            with mock.patch.dict(os.environ, {"WAYLAND_DISPLAY": "wayland-0"}), mock.patch.object(
                    boot, "run_kiln", product), mock.patch.object(
                    boot, "login_session_displays", return_value=self.LOGIN), mock.patch.object(
                    boot.conformance, "source_identity", return_value={}), fake_kiln(), \
                    contextlib.redirect_stdout(io.StringIO()):
                result = boot.main(["--profile", "portal", "--out", str(output)])
            self.assertNotEqual(result, 0)
            product.assert_not_called()
            self.assertIn("live desktop", json.dumps(json.loads((output / "evidence.json").read_text())))


class ResizeAcceptanceTests(unittest.TestCase):
    def test_resize_workload_uses_the_frame_command_buffer(self):
        commands = boot.resize_commands(((641, 479),))
        self.assertEqual(
            ["mat_queue_mode 2", "host_framerate 0.015", "wait 120", "mat_resizewindow 641 479",
             "wait 30", "screenshot", "wait 6"], commands[:7])
        self.assertEqual(["wait 10", "quit"], commands[-2:])
        # The settle sweep: a screenshot at each pass offset from a resize.
        sweep = commands[7:-2]
        for (width, height), offset in boot.RESIZE_SETTLE_SWEEP:
            step = ["mat_resizewindow %d %d" % (width, height)]
            step += ["wait %d" % offset] if offset else []
            step += ["screenshot", "wait 30"]
            self.assertEqual(step, sweep[:len(step)])
            sweep = sweep[len(step):]
        self.assertEqual([], sweep)
        self.assertEqual(list(range(25)), [offset for _, offset in boot.RESIZE_SETTLE_SWEEP])

    def test_resize_script_is_one_line_and_records_its_hash(self):
        with tempfile.TemporaryDirectory() as root:
            stage = Path(root)
            result = boot.install_resize_script(stage, ((641, 479),))
            script = stage / result["path"]
            self.assertEqual(1, len(script.read_text().splitlines()))
            self.assertIn("wait 120; mat_resizewindow 641 479; wait 30", script.read_text())
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

    def test_portal2_resize_scripts_are_executable_by_the_selected_game(self):
        with tempfile.TemporaryDirectory() as root:
            stage = Path(root)
            result = boot.install_resize_script(stage, game="portal2")
            files = [result["path"]] + [item["path"] for item in result["chained"]]
            self.assertGreater(len(files), 1)
            for path in files:
                self.assertEqual(Path("portal2/cfg"), Path(path).parent)
                self.assertTrue((stage / path).is_file())
            self.assertFalse((stage / "portal").exists())

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
        self.assertEqual(["mat_queue_mode 0", "host_framerate 0.015", "wait 120",
                          "mat_resizewindow 641 479", "wait 30", "screenshot", "wait 6"],
                         commands[:7])
        drag = ["mat_resizewindow %d %d" % size for size in boot.RESIZE_DRAG_WORKLOAD]
        self.assertEqual(drag, [c for c in commands[7:] if c.startswith("mat_resizewindow")])
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

    def test_sync_resize_is_unscaled_at_settled_sizes_on_every_window_system(self):
        def log(driver, settled, dragged):
            return ("RFC0001 window: provider=sdl3 driver=%s\n"
                    "RFC0001 resize observed: logical=641x479 drawable=641x479\n"
                    "RFC0001 resize queued: serial=1 drawable=641x479 request_us=9000\n"
                    "RFC0001 resize complete: serial=1 drawable=641x479 main_wait_us=0\n"
                    "[vulkan] presents=40 scaled=%d longest_scaled_run=%d\n"
                    "RFC0001 resize observed: logical=820x610 drawable=820x610\n"
                    "RFC0001 resize queued: serial=2 drawable=820x610 request_us=9000\n"
                    "RFC0001 resize complete: serial=2 drawable=820x610 main_wait_us=0\n"
                    "[vulkan] presents=60 scaled=%d longest_scaled_run=%d\n"
                    % (driver, settled, settled, settled + dragged, max(settled, dragged)))
        images = [{"width": 641, "height": 479, "has_scene_detail": True}]
        def status(text):
            return boot.inspect_resize(text, images, ((641, 479),), mode="sync",
                                       require_unscaled=True)["status"]
        self.assertEqual("pass", status(log("wayland", 0, 0)))
        # X11 resizes the window before the client learns of it, so a drag may
        # present scaled frames there; a settled size may not, on any system.
        self.assertEqual("pass", status(log("x11", 0, 3)))
        self.assertEqual("fail", status(log("x11", 2, 0)))
        self.assertEqual("fail", status(log("wayland", 0, 3)))
        # Without a census at the settled sizes nothing certifies them.
        self.assertEqual("fail", status(re.sub(r"\[vulkan\].*\n", "", log("x11", 0, 0))))

    def test_queued_request_budget_follows_the_path_the_engine_took(self):
        def failures(micros, path):
            log = ("RFC0001 resize observed: logical=641x479 drawable=641x479\n"
                   "RFC0001 resize queued: serial=1 drawable=641x479 request_us=%d ui_us=9 "
                   "path=%s\n"
                   "RFC0001 resize complete: serial=1 drawable=641x479 main_wait_us=0\n"
                   % (micros, path))
            images = [{"width": 641, "height": 479, "has_scene_detail": True}]
            return boot.inspect_resize(log, images, ((641, 479),), mode="queued")["failures"]
        self.assertEqual([], failures(40, "worker"))
        # A frame without the render worker (after a screenshot) resizes on the
        # main thread; that request is judged by the one-frame budget.
        self.assertEqual([], failures(6000, "main"))
        self.assertEqual(["main-thread resize request 641x479 exceeded 2000us (worker path)"],
                         failures(6000, "worker"))
        self.assertEqual(["main-thread resize request 641x479 exceeded 16667us (main path)"],
                         failures(17000, "main"))

    def test_queued_settle_sweep_requires_every_size_to_complete(self):
        sweep = (((860, 540), 0), ((868, 546), 1))
        def log(completed):
            text = ""
            for serial, ((w, h), _offset) in enumerate(sweep, 1):
                text += ("RFC0001 resize observed: logical=%dx%d drawable=%dx%d\n"
                         "RFC0001 resize queued: serial=%d drawable=%dx%d request_us=30\n"
                         % (w, h, w, h, serial, w, h))
                if serial in completed:
                    text += ("RFC0001 resize complete: serial=%d drawable=%dx%d main_wait_us=0\n"
                             % (serial, w, h))
            return text
        def failures(text):
            return boot.inspect_resize(text, [], (), mode="queued", sweep=sweep)["failures"]
        self.assertEqual([], failures(log((1, 2))))
        # A resize queued as a screenshot left queued rendering never completes.
        self.assertEqual(["renderer did not complete sweep drawable 868x546 "
                          "(a screenshot during the settle)"], failures(log((1,))))
        self.assertEqual(["SDL window did not reach sweep size 868x546"],
                         failures(log((1,)).split("RFC0001 resize observed: logical=868")[0]))

    def test_queued_resize_bounds_the_longest_scaled_present_run(self):
        log = ("RFC0001 resize observed: logical=641x479 drawable=1282x958\n"
               "RFC0001 resize queued: serial=1 drawable=1282x958 request_us=40\n"
               "RFC0001 resize complete: serial=1 drawable=1282x958 main_wait_us=0\n")
        images = [{"width": 1282, "height": 958, "has_scene_detail": True}]
        limit = boot.QUEUED_SCALED_PRESENT_RUN_LIMIT
        def result(census):
            return boot.inspect_resize(log + census, images, ((641, 479),), mode="queued",
                                       require_unscaled=True)
        # Frames of the old size are scaled while a queued resize settles.
        settled = result("[vulkan] presents=400 scaled=30 longest_scaled_run=%d\n" % limit)
        self.assertEqual("pass", settled["status"])
        self.assertEqual(limit, settled["longest_scaled_present_run"])
        # A back buffer that stays behind its drawable longer than the settle.
        self.assertEqual("fail", result(
            "[vulkan] presents=400 scaled=30 longest_scaled_run=%d\n" % (limit + 1))["status"])
        # A server-sized window system (X11) adds its own frames of lag.
        x11 = "RFC0001 window: provider=sdl3 driver=x11\n"
        allowance = limit + boot.SERVER_SIZED_SCALED_PRESENT_RUN_ALLOWANCE
        self.assertEqual("pass", result(
            x11 + "[vulkan] presents=400 scaled=30 longest_scaled_run=%d\n" % allowance)["status"])
        self.assertEqual("fail", result(
            x11 + "[vulkan] presents=400 scaled=30 longest_scaled_run=%d\n"
            % (allowance + 1))["status"])
        # A census without the run, or none at all, cannot certify it.
        self.assertEqual("fail", result("[vulkan] presents=400 scaled=30\n")["status"])
        self.assertEqual("fail", result("")["status"])

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

    def test_private_content_is_validated_before_staging(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            content, stage = root / "content", root / "stage"
            (content / "maps").mkdir(parents=True)
            (content / "materials").mkdir()
            (stage / "portal/materials").mkdir(parents=True)
            (content / "maps/sample.bsp").write_bytes(b"bsp2")
            (content / "materials/sample.vmt").write_bytes(b"vmt")
            (content / "materials/unsupported.txt").write_bytes(b"bad")
            with self.assertRaisesRegex(ValueError, "unsupported file"):
                boot.install_content(content, stage)
            self.assertFalse((stage / "portal/maps/sample.bsp").exists())
            (content / "materials/unsupported.txt").unlink()
            (stage / "portal/materials/sample.vmt").write_bytes(b"installed")
            with self.assertRaisesRegex(ValueError, "replace installed content"):
                boot.install_content(content, stage)
            self.assertFalse((stage / "portal/maps/sample.bsp").exists())
            (stage / "portal/materials/sample.vmt").unlink()
            installed = boot.install_content(content, stage)
            self.assertEqual({"maps/sample.bsp", "materials/sample.vmt"}, set(installed))
            self.assertEqual(b"bsp2", (stage / "portal/maps/sample.bsp").read_bytes())

    def test_private_content_mount_precedes_published_custom_content(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            content, stage = root / "content", root / "stage"
            (content / "maps").mkdir(parents=True)
            (content / "materials").mkdir()
            (stage / "portal2/materials").mkdir(parents=True)
            (content / "maps/sample.bsp").write_bytes(b"private map")
            (content / "materials/sample.vmt").write_bytes(b"private material")
            (stage / "portal2/materials/sample.vmt").write_bytes(b"published material")
            gameinfo = '"GameInfo" { FileSystem { SearchPaths { ' \
                       'game+mod |gameinfo_path|custom/* ' \
                       'game+mod |gameinfo_path|. } } }'
            (stage / "portal2/gameinfo.txt").write_text(gameinfo)

            mount = "custom/portal-boot-content"
            boot.install_content(content, stage, game="portal2", mount=mount)
            search = boot.prepend_game_search_path(gameinfo, "portal2/" + mount)
            (stage / "portal2/gameinfo.txt").write_text(search)

            self.assertEqual(b"private material",
                             (stage / "portal2" / mount / "materials/sample.vmt").read_bytes())
            self.assertEqual(b"published material",
                             (stage / "portal2/materials/sample.vmt").read_bytes())
            self.assertLess(search.index("portal2/" + mount), search.index("custom/*"))


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
    def fixture(self, root, game="portal"):
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
        (stage / game).mkdir(parents=True)
        (stage / game / "gameinfo.txt").write_text(
            '"GameInfo" { FileSystem { SearchPaths { game+mod %s/%s_pak.vpk } } }' % (game, game))
        return repo, artifacts, stage, manifest

    def test_portal2_shaders_use_selected_game_without_rewriting_portal(self):
        with tempfile.TemporaryDirectory() as directory:
            repo, artifacts, stage, manifest = self.fixture(Path(directory), game="portal2")
            (stage / "portal").mkdir()
            other = stage / "portal/gameinfo.txt"
            other.write_text("unrelated Portal gameinfo")
            evidence = boot.install_shader_artifacts(artifacts, stage, repo, game="portal2")
            destination = stage / "portal2/custom/source-engine-shaders/shaders/fxc/Downsample_vs20.vcs"
            self.assertEqual(b"compiled shader", destination.read_bytes())
            self.assertEqual("portal2/custom/source-engine-shaders", evidence["search_path"])
            self.assertIn("portal2/custom/source-engine-shaders", (stage / "portal2/gameinfo.txt").read_text())
            self.assertEqual("unrelated Portal gameinfo", other.read_text())
            self.assertFalse((stage / "portal/custom").exists())

    def test_unsupported_shader_game_fails_before_mutation(self):
        with tempfile.TemporaryDirectory() as directory:
            repo, artifacts, stage, manifest = self.fixture(Path(directory))
            before = boot.sha256(stage / "portal/gameinfo.txt")
            with self.assertRaises(ValueError):
                boot.install_shader_artifacts(artifacts, stage, repo, game="../portal")
            self.assertEqual(before, boot.sha256(stage / "portal/gameinfo.txt"))
            self.assertFalse((stage / "portal/custom").exists())

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
            shutil.copytree(original, stage, symlinks=True)
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
        output = root / "capture"

        def product(args, command, stage, environment, timeout, stdout):
            self.assertEqual(str(output / "render-trace.jsonl"), environment["SOURCE_RENDER_TRACE"])
            self.assertIn(str(output), environment["SOURCE_RENDER_TRACE"])
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

        # The fake product opens no window, so the live-desktop guard has no
        # login session to protect here.
        with mock.patch.object(boot, "run_kiln", side_effect=product), mock.patch.object(
                boot.conformance, "source_identity", return_value={}), mock.patch.object(
                boot, "login_session_displays", return_value={}), fake_kiln(), \
                contextlib.redirect_stdout(io.StringIO()):
            result = boot.main(["--profile", "portal", "--out", str(output), "--render-trace"])
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
