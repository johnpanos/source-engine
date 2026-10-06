"""Negative and positive fixtures for the recorded-demo frame runner."""
import argparse
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import demo_frames


def workload():
    loaded = demo_frames.load_workload(demo_frames.DEFAULT_WORKLOAD)
    loaded["demo_name"] = loaded["map"] + ".dem"
    return loaded


def frame(number, interval_ms, records=100, gpu_ms=None, extent=(1920, 1080), **extra):
    row = {"f": number, "t": 1000000 + number * 30000, "interval": int(interval_ms * 1000),
           "cpu": 5000, "engine": 1000, "backend": 2000, "records": records,
           "extent": list(extent), "cost": {"record": [1, 1500]}}
    if gpu_ms is not None:
        row["gpu"] = [number, int(gpu_ms * 1000), int(gpu_ms * 900)]
    row.update(extra)
    return row


def stream(load=5, playback=60, playback_ms=20.0, gpu_ms=19.0):
    """Load frames (slow, little drawn), settled playback, then the quit frame."""
    rows = [frame(i + 1, 900.0, records=3) for i in range(load)]
    rows += [frame(load + i + 1, playback_ms, gpu_ms=gpu_ms) for i in range(playback)]
    rows.append(frame(load + playback + 1, 300.0, gpu_ms=1.0))
    return rows


CONSOLE = ('Playing demo from sp_a1_intro4_relit.dem.\n"r_temporal_scale" = "0" min. 0 max. 1\n'
           '"mat_antialias" = "0"\n"mat_queue_mode" = "2"\n"r_core_world" = "1" ( def. "0" )\n')


def write_run(directory, rows, console=CONSOLE):
    lines = [json.dumps({"schema": "vulkan-frame-stats/v1"})] + [json.dumps(row) for row in rows]
    (directory / "frames.jsonl").write_text("\n".join(lines) + "\n")
    (directory / "console.log").write_text(console)


class PlaybackWindowTest(unittest.TestCase):
    def test_excludes_level_load_and_the_quit_frame(self):
        rows = stream(load=5, playback=60)
        selected = demo_frames.playback_window(rows, workload()["playback_window"])
        self.assertEqual(6, selected[0]["f"])
        self.assertEqual(65, selected[-1]["f"])

    def test_a_late_load_hitch_moves_the_start_past_it(self):
        rows = stream(load=5, playback=80)
        rows[10]["interval"] = 1400000  # frame 11: a load-time hitch inside the first settle run
        selected = demo_frames.playback_window(rows, workload()["playback_window"])
        self.assertEqual(12, selected[0]["f"])

    def test_a_wrapped_cpu_counter_is_not_playback(self):
        rows = stream(load=5, playback=80)
        rows[5]["cpu"] = (1 << 64) - 19571000  # frame 6: a negative delta wrapped into uint64
        selected = demo_frames.playback_window(rows, workload()["playback_window"])
        self.assertEqual(7, selected[0]["f"])

    def test_a_demo_that_never_settles_is_rejected(self):
        rows = [frame(i + 1, 400.0) for i in range(100)]
        with self.assertRaises(demo_frames.DemoError):
            demo_frames.playback_window(rows, workload()["playback_window"])

    def test_frames_that_draw_no_world_are_not_playback(self):
        rows = [frame(i + 1, 10.0, records=3) for i in range(100)]
        with self.assertRaises(demo_frames.DemoError):
            demo_frames.playback_window(rows, workload()["playback_window"])


class StatisticsTest(unittest.TestCase):
    def test_hitches_lows_and_gpu_bound_frames(self):
        rows = [frame(i + 1, 20.0, gpu_ms=19.0) for i in range(98)]
        rows += [frame(99, 120.0, gpu_ms=10.0), frame(100, 60.0, gpu_ms=59.0)]
        stats = demo_frames.frame_stats(rows, [33.3, 50, 100])
        self.assertEqual({"over_33.3_ms": 2, "over_50_ms": 2, "over_100_ms": 1}, stats["hitches"])
        self.assertEqual(99, stats["gpu_bound_frames"])
        self.assertEqual(100, stats["gpu_result_frames"])
        self.assertAlmostEqual(1000 / 120.0, stats["low_1pct_fps"], places=1)

    def test_settings_drift_and_unreported_settings_fail(self):
        drifted = CONSOLE.replace('"mat_antialias" = "0"', '"mat_antialias" = "4"')
        drifted = drifted.replace('"mat_queue_mode" = "2"\n', "")
        failures = demo_frames.setting_failures(drifted, workload()["settings"])
        self.assertEqual(2, len(failures))
        self.assertTrue(any("mat_antialias is 4" in failure for failure in failures))
        self.assertTrue(any("mat_queue_mode is unreported" in failure for failure in failures))


class CorePassTest(unittest.TestCase):
    def test_a_row_deeper_than_its_predecessor_is_not_misattributed(self):
        console = ("cl_render_debug_stats: core GPU passes, mean of 10 frame(s):\n"
                   "  core world view                   20.000 ms  x2.0\n"
                   "    core model depth                 0.070 ms  x1.0\n"
                   "        world / pbr                  9.800 ms  x1.0\n")
        passes = {item["pass"]: item["mean_ms"]
                  for item in demo_frames.core_pass_means(demo_frames.render_profile.core_reports(console))}
        self.assertEqual(0.07, passes["core world view > core model depth"])
        self.assertNotIn("core world view > core model depth > world / pbr", passes)
        self.assertEqual(9.8, passes["(unplaced) > (unplaced) > (unplaced) > world / pbr"])


class AnalyzeTest(unittest.TestCase):
    def analyze(self, rows, console=CONSOLE, extent=None):
        with tempfile.TemporaryDirectory() as directory:
            write_run(Path(directory), rows, console)
            return demo_frames.analyze(Path(directory), workload(), extent)

    def test_complete_run(self):
        result = self.analyze(stream())
        self.assertEqual([], result["failures"])
        self.assertEqual(60, result["frames"]["frames"])
        self.assertEqual(5, result["window"]["load_frames"])

    def test_no_playback_in_the_console_fails(self):
        result = self.analyze(stream(), console=CONSOLE.replace("Playing demo", "Failed demo"))
        self.assertIn("the console does not show the demo playing", result["failures"])

    def test_wrong_drawable_extent_fails(self):
        rows = stream()
        for row in rows:
            row["extent"] = [1024, 768]
        self.assertTrue(any("extents" in failure for failure in self.analyze(rows)["failures"]))
        self.assertEqual([], self.analyze(rows, extent=(1024, 768))["failures"])


class CommandTest(unittest.TestCase):
    def arguments(self, out, **overrides):
        values = dict(out=out, width=1920, height=1080, profile=False, extra_arg=[], renderdoc_frames=[])
        values.update(overrides)
        return argparse.Namespace(**values)

    def test_stats_path_is_relative_to_the_runtime(self):
        root = Path(tempfile.gettempdir()) / ("long-evidence-name-" * 20)
        command = demo_frames.game_command(self.arguments(root / "run"), workload(),
                                           root / "run/frames.jsonl", root / "runtime")
        self.assertEqual("../run/frames.jsonl", command[command.index("-vkframestats") + 1])
        self.assertEqual("sp_a1_intro4_relit", command[-1])

    def test_renderdoc_frames_wrap_the_game_and_name_the_frames(self):
        if not demo_frames.shutil.which("renderdoccmd"):
            self.skipTest("renderdoccmd is not installed")
        with tempfile.TemporaryDirectory() as directory:
            args = self.arguments(Path(directory), renderdoc_frames=[900, 930])
            command = demo_frames.game_command(args, workload(), Path(directory) / "frames.jsonl",
                                               Path(directory))
        self.assertIn("--opt-hook-children", command)
        self.assertIn("SDL_VIDEODRIVER=x11", command)
        self.assertEqual("900,930", command[command.index("-vkrenderdocframes") + 1])

    def test_missing_or_unmatched_captures_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory)
            (out / "renderdoc").mkdir()
            write_run(out, [frame(1, 20.0), frame(2, 20.0, renderdoc_capture=True)])
            args = self.arguments(out, renderdoc_frames=[2, 3])
            failures = demo_frames.renderdoc_failures(args, {})
            self.assertTrue(any("did not capture frames [3]" in failure for failure in failures))
            self.assertTrue(any("1 frames tagged but 0 .rdc" in failure for failure in failures))
            (out / "renderdoc/frame_frame2.rdc").write_bytes(b"rdc")
            self.assertEqual([], demo_frames.renderdoc_failures(self.arguments(out, renderdoc_frames=[2]), {}))


class WorkloadTest(unittest.TestCase):
    def test_a_changed_demo_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            declared = json.loads(demo_frames.DEFAULT_WORKLOAD.read_text())
            demo = Path(directory) / "demo.dem"
            demo.write_bytes(b"not the recorded demo")
            declared["demo"] = str(demo)
            path = Path(directory) / "workload.json"
            path.write_text(json.dumps(declared))
            with self.assertRaises(demo_frames.DemoError):
                demo_frames.load_workload(path)


if __name__ == "__main__":
    unittest.main()
