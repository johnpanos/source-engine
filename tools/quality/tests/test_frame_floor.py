"""Negative controls for hard High qualification and the every-frame floor."""
import argparse
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import frame_floor


def arguments(**overrides):
    values = dict(floor_fps=None, low_1pct_fps=None, low_01pct_fps=None,
                  width=1920, height=1080, display="compositor", render_switch=[], extra_arg=[])
    values.update(overrides)
    return argparse.Namespace(**values)


class FrameFloorTest(unittest.TestCase):
    def setUp(self):
        self.workload = json.loads(frame_floor.DEFAULT_WORKLOAD.read_text())
        self.budget = frame_floor.configure_budget(self.workload, arguments())

    def watcher(self):
        return frame_floor.FrameWatcher("unused", 1000.0 / 120)

    def test_stats_path_is_relative_to_the_products_working_directory(self):
        root = Path("/tmp") / ("long-evidence-name-" * 20)
        args = arguments(render_budget=self.budget, start_frames=60)
        stats = root / "route/frames.jsonl"
        runtime = root / "runtime"
        command = frame_floor.game_command(args, {"name": "route", "map": "map"}, stats, runtime)
        actual = command[command.index("-vkframestats") + 1]
        self.assertEqual("../route/frames.jsonl", actual)
        self.assertEqual(stats, (runtime / actual).resolve())

    def test_profiling_enables_timers_without_an_arbitrary_quality_override(self):
        args = arguments(profile=True, start_frames=60)
        args.render_budget = frame_floor.configure_budget(self.workload, args)
        command = frame_floor.game_command(args, {"name": "route", "map": "map"},
                                           Path("/tmp/frames.jsonl"), Path("/tmp/runtime"))
        self.assertIn("-vkgputimers", command)
        self.assertIn("render_budget_high", command)
        self.assertIn("-noborder", command)
        args.extra_arg = ["+r_core_shadow_quality", "0"]
        with self.assertRaises(frame_floor.FloorError):
            frame_floor.configure_budget(self.workload, args)

    def test_opaque_batching_control_preserves_high_settings(self):
        for mode, value in (("off", "0"), ("on", "1")):
            args = arguments(opaque_batching=mode, start_frames=60)
            args.render_budget = frame_floor.configure_budget(self.workload, args)
            command = frame_floor.game_command(args, {"name": "route", "map": "map"},
                                               Path("/tmp/frames.jsonl"), Path("/tmp/runtime"))
            self.assertEqual(value, command[command.index("-vkopaquebatch") + 1])
            self.assertIn("render_budget_high", command)
            self.assertEqual("4", command[command.index("+mat_antialias") + 1])

    def test_wrapper_exit_does_not_leave_its_game_running(self):
        process = mock.Mock(pid=1234)
        process.poll.return_value = 0
        with mock.patch.object(frame_floor.os, "killpg") as kill:
            frame_floor.stop(process)
        kill.assert_called_once_with(1234, frame_floor.signal.SIGKILL)

    def test_one_slow_frame_fails_even_with_fast_p99(self):
        watcher = self.watcher()
        failure = None
        for index in range(1, 1001):
            row = {"f": index, "interval": 9000 if index == 500 else 8000}
            if index == 1:
                row["mark"] = "floor_begin"
            elif index == 1000:
                row["mark"] = "floor_end"
            failure = watcher.judge(row) or failure
        self.assertEqual(failure["frame"]["frame"], 500)
        self.assertLess(frame_floor.summarize(watcher.frames)["p99_ms"], watcher.floor_ms)
        self.assertEqual(watcher.state, "ended")

    def test_isolated_gpu_stall_is_not_hidden_by_submission_pacing_or_p99(self):
        frames = [{"interval_ms": 8.0, "cpu_ms": 4.0, "submission_ms": 2.0,
                   "gpu_render_ms": 9.0 if index == 500 else 5.0} for index in range(1000)]
        summary = frame_floor.summarize(frames)
        self.assertEqual(summary["gpu_render_p99_ms"], 5.0)
        failures = frame_floor.metric_budget_failures(summary, self.budget)
        self.assertTrue(any("gpu_render_max_ms" in failure for failure in failures))

    def test_missing_gpu_completion_sample_cannot_certify_complete_cost(self):
        frames = [{"interval_ms": 8.0, "cpu_ms": 4.0, "submission_ms": 2.0}]
        failures = frame_floor.metric_budget_failures(frame_floor.summarize(frames), self.budget)
        self.assertTrue(any("no valid gpu_render" in failure for failure in failures))

    def test_slow_end_frame_counts(self):
        watcher = self.watcher()
        watcher.judge({"f": 1, "interval": 8000, "mark": "floor_begin"})
        self.assertIsNotNone(watcher.judge({"f": 2, "interval": 9000, "mark": "floor_end"}))

    def test_slow_frame_flushed_at_process_exit_still_fails_the_run(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            runtime = root / "runtime"
            (runtime / "portal2").mkdir(parents=True)
            args = arguments(out=root / "out", render_budget=None, start_frames=60, no_stop=True)
            scenario = {"name": "route", "map": "map", "timeout_seconds": 5}

            def launch(args, runtime, command, output):
                (output / "stdout.log").write_text("")
                (output / "frames.jsonl").write_text(
                    '{"f":1,"interval":9000,"mark":"floor_begin,floor_end"}\n')
                process = mock.Mock(returncode=0, pid=1234)
                process.poll.return_value = 0
                return process, mock.Mock()

            with mock.patch.object(frame_floor, "launch", side_effect=launch), \
                    mock.patch.object(frame_floor.os, "killpg"), \
                    mock.patch.object(frame_floor, "host_context", return_value={}), \
                    mock.patch.object(frame_floor.portal2_scenarios, "evaluate",
                                      return_value={"failures": [], "checks": []}):
                result = frame_floor.run(args, {}, {"floor_fps": 120, "low_1pct_fps": 120,
                                                   "low_01pct_fps": 120}, scenario, runtime)
            self.assertEqual(result["status"], "fail")
            self.assertIsNotNone(result["first_below_floor"])

    def test_warmup_and_loading_outside_bracket_are_not_gameplay(self):
        watcher = self.watcher()
        watcher.judge({"f": 1, "interval": 100000})
        watcher.judge({"f": 2, "interval": 8000, "mark": "floor_begin"})
        watcher.judge({"f": 3, "interval": 8000, "mark": "floor_end"})
        watcher.judge({"f": 4, "interval": 100000})
        self.assertEqual(len(watcher.frames), 2)
        self.assertEqual(watcher.below, [])

    def test_missing_and_duplicate_frame_numbers_are_rejected(self):
        for next_frame in (1, 3):
            with self.subTest(next_frame=next_frame):
                watcher = self.watcher()
                watcher.judge({"f": 1, "interval": 8000, "mark": "floor_begin"})
                watcher.judge({"f": next_frame, "interval": 8000})
                self.assertTrue(watcher.invalid)

    def test_missing_zero_and_nonfinite_intervals_cannot_pass(self):
        for interval in (None, 0, -1, float("nan"), float("inf")):
            with self.subTest(interval=interval):
                watcher = self.watcher()
                watcher.judge({"f": 1, "interval": interval, "mark": "floor_begin,floor_end"})
                self.assertTrue(watcher.invalid)
                self.assertEqual(watcher.frames, [])

    def test_wrapped_cpu_duration_is_invalid_instead_of_a_real_cost(self):
        watcher = self.watcher()
        watcher.judge({"f": 1, "interval": 8000, "cpu": (1 << 64) - 32000000,
                       "mark": "floor_begin,floor_end"})
        self.assertTrue(any("invalid CPU duration" in failure for failure in watcher.invalid))
        self.assertNotIn("cpu_ms", watcher.frames[0])

    def test_corrupt_line_inside_route_is_not_silently_discarded(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "frames.jsonl"
            path.write_text('{"f":1,"interval":8000,"mark":"floor_begin"}\n{corrupt}\n')
            watcher = frame_floor.FrameWatcher(path, 1000.0 / 120)
            watcher.poll()
            self.assertTrue(watcher.invalid)

    def test_delayed_gpu_timestamp_retains_its_own_frame_identity(self):
        watcher = self.watcher()
        watcher.judge({"f": 1, "interval": 8000, "mark": "floor_begin,floor_end"})
        watcher.judge({"f": 2, "interval": 8000, "gpu": [1, 6000, 5000]})
        self.assertEqual(watcher.gpu_frames, {1: 5.0})

    def test_hard_floor_cannot_be_lowered_by_cli(self):
        for key in ("floor_fps", "low_1pct_fps", "low_01pct_fps"):
            with self.subTest(key=key), self.assertRaises(frame_floor.FloorError):
                frame_floor.floor_settings(self.workload, arguments(**{key: 119}))

    def test_cli_may_tighten_the_floor(self):
        settings = frame_floor.floor_settings(self.workload, arguments(floor_fps=144))
        self.assertEqual(settings["floor_fps"], 144)

    def test_startup_size_cannot_hide_a_shrunken_drawable(self):
        header = {"device": self.budget["conditions"]["device"]}
        for frames in ([], [{"extent": [1920, 1043]}], [{"extent": None}]):
            receipt = frame_floor.quality_receipt(self.quality_log(), header, self.budget, frames)
            self.assertEqual(receipt["status"], "fail")
            self.assertTrue(any("per-frame drawable" in failure for failure in receipt["failures"]))
        self.assertEqual(frame_floor.quality_receipt(
            self.quality_log(), header, self.budget, [{"extent": [1920, 1080]}])["status"], "pass")

    def test_small_drawable_and_quality_overrides_cannot_qualify(self):
        for overrides in ({"width": 1024, "height": 768}, {"display": "offscreen"},
                          {"extra_arg": ["+r_core_ao_quality", "0"]},
                          {"render_switch": ["--no-core-world"]}):
            with self.subTest(overrides=overrides), self.assertRaises(frame_floor.FloorError):
                frame_floor.configure_budget(self.workload, arguments(**overrides))

    def quality_log(self, width=1920, height=1080, settings=None):
        values = settings or self.budget["settings"]
        return "back buffer %dx%d\n" % (width, height) + "\n".join(
            '"%s" = "%s"' % pair for pair in values.items())

    def receipt(self, log, device=None):
        header = {"device": device or self.budget["conditions"]["device"]}
        return frame_floor.quality_receipt(log, header, self.budget)

    def test_observed_high_values_and_native_size_pass(self):
        self.assertEqual(self.receipt(self.quality_log())["status"], "pass")

    def test_launch_size_does_not_excuse_capped_actual_back_buffer(self):
        self.assertEqual(self.receipt(self.quality_log(1024, 768))["status"], "fail")

    def test_disabled_effect_missing_query_and_wrong_gpu_fail(self):
        values = dict(self.budget["settings"], r_core_ao_quality="0")
        self.assertEqual(self.receipt(self.quality_log(settings=values))["status"], "fail")
        del values["mat_antialias"]
        self.assertTrue(any("mat_antialias" in failure for failure in
                            self.receipt(self.quality_log(settings=values))["failures"]))
        self.assertEqual(self.receipt(self.quality_log(), "llvmpipe")["status"], "fail")

    def test_driver_probe_preserves_each_device_instead_of_assuming_first_gpu(self):
        summary = "GPU0:\n deviceName = llvmpipe\n driverInfo = Mesa software\n apiVersion = 1.4\n" \
                  "GPU1:\n deviceName = Radeon\n driverName = radv\n driverInfo = Mesa native\n apiVersion = 1.4\n"
        with mock.patch.object(frame_floor.subprocess, "run",
                               return_value=mock.Mock(returncode=0, stdout=summary)):
            context = frame_floor.graphics_context()
        self.assertEqual(context["devices"][1]["driverInfo"], "Mesa native")
        with mock.patch.object(frame_floor.subprocess, "run", side_effect=FileNotFoundError("vulkaninfo")):
            self.assertEqual(frame_floor.graphics_context()["status"], "unavailable")


if __name__ == "__main__":
    unittest.main()
