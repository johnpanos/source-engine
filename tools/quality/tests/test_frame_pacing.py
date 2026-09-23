"""Fixtures for the frame-pacing harness: scenario validation, console chaining,
frame-stream analysis (including seeded hitches and broken streams) and A/B
summaries. No product or GPU is needed."""

import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))
SPEC = importlib.util.spec_from_file_location("frame_pacing", QUALITY / "frame_pacing.py")
pacing = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(pacing)

REPO = QUALITY.parents[1]
SCENARIO = REPO / "quality/workloads/portal-frame-pacing-v1.json"


def scenario(**changes):
    result = {"schema": pacing.SCENARIO_SCHEMA, "id": "fixture", "map": "testchmb_a_02",
              "host_framerate": 60, "passes": 1, "setup": ["god"],
              "body": ["vk_frame_mark idle", "wait 10"]}
    result.update(changes)
    return result


def load(data):
    with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as stream:
        json.dump(data, stream)
    try:
        return pacing.load_scenario(stream.name)
    finally:
        Path(stream.name).unlink()


def frame(index, interval_ms, mark="", cost=None, cpu_ms=None, gpu=None):
    record = {"f": index, "t": index * 1000, "interval": int(interval_ms * 1000),
              "engine": int(interval_ms * 900), "backend": int(interval_ms * 100),
              "records": 10, "vertex_bytes": 1000, "index_bytes": 400, "upload_bytes": 0,
              "cost": cost or {}}
    if cpu_ms is not None:
        record["cpu"] = int(cpu_ms * 1000)
    if mark:
        record["mark"] = mark
    if gpu is not None:
        record["gpu"] = gpu
    return record


def stream(intervals, marks=None, costs=None):
    """Frames 1..n with a pass bracket around all but the first and last."""
    marks = dict(marks or {})
    marks.setdefault(1, "pass_1_begin")
    marks.setdefault(len(intervals), "pass_1_end")
    return [frame(i, ms, marks.get(i, ""), (costs or {}).get(i), cpu_ms=ms * 0.9)
            for i, ms in enumerate(intervals, start=1)]


class ScenarioTests(unittest.TestCase):
    def test_checked_in_scenario_is_valid(self):
        loaded = pacing.load_scenario(SCENARIO)
        self.assertEqual(loaded["id"], "portal-frame-pacing-v1")
        self.assertGreaterEqual(loaded["passes"], 2, "a warm pass needs a first pass before it")

    def test_rejects_wrong_schema_and_bad_names(self):
        for bad in (scenario(schema="other"), scenario(map="a b"), scenario(id="x;quit")):
            with self.assertRaises(pacing.ScenarioError):
                load(bad)

    def test_rejects_command_separators_and_quotes(self):
        for step in ("god; quit", 'echo "x"', "wait 1\nquit"):
            with self.assertRaises(pacing.ScenarioError):
                load(scenario(body=[step]))

    def test_marks_need_one_simple_label(self):
        for step in ("vk_frame_mark", "vk_frame_mark a b", "vk_frame_mark a,b"):
            with self.assertRaises(pacing.ScenarioError):
                load(scenario(body=[step]))

    def test_requires_a_fixed_simulation_rate_and_bounded_passes(self):
        without = scenario()
        del without["host_framerate"]
        for bad in (without, scenario(host_framerate=0), scenario(passes=0),
                    scenario(passes=17), scenario(body=[])):
            with self.assertRaises(pacing.ScenarioError):
                load(bad)


class ConsoleChainTests(unittest.TestCase):
    def test_passes_are_bracketed_and_the_run_quits(self):
        commands = pacing.scenario_commands(scenario(), 2)
        self.assertEqual(commands.count("vk_frame_mark pass_1_begin"), 1)
        self.assertEqual(commands.count("vk_frame_mark pass_2_end"), 1)
        self.assertEqual(commands[-1], "quit")

    def test_long_scenarios_chain_files_under_the_console_limit(self):
        commands = ["wait %d" % i for i in range(400)] + ["quit"]
        cfgs = pacing.scenario_cfgs(commands)
        self.assertGreater(len(cfgs), 1)
        names = list(cfgs)
        replayed = []
        for position, name in enumerate(names):
            text = cfgs[name]
            self.assertTrue(text.endswith("\n") and text.count("\n") == 1, "one line per cfg")
            self.assertLess(len(text), 512, "the console truncates longer lines")
            parts = text.strip().split(";")
            if position + 1 < len(names):
                self.assertEqual(parts[-1], "exec " + names[position + 1])
                parts = parts[:-1]
            replayed.extend(parts)
        self.assertEqual(replayed, commands, "chaining must keep every command in order")

    def test_a_single_overlong_command_is_rejected(self):
        with self.assertRaises(pacing.ScenarioError):
            pacing.scenario_cfgs(["echo " + "x" * 600])


class StatsReaderTests(unittest.TestCase):
    def write(self, lines):
        handle = tempfile.NamedTemporaryFile("w", suffix=".jsonl", delete=False)
        handle.write("\n".join(lines) + "\n")
        handle.close()
        self.addCleanup(Path(handle.name).unlink)
        return handle.name

    def test_truncated_last_line_is_counted_not_fatal(self):
        header = json.dumps({"schema": pacing.STATS_SCHEMA})
        path = self.write([header, json.dumps(frame(1, 5)), '{"f": 2, "t":'])
        _, frames, truncated = pacing.read_stats(path)
        self.assertEqual((len(frames), truncated), (1, 1))

    def test_missing_or_foreign_header_fails(self):
        with self.assertRaises(pacing.ScenarioError):
            pacing.read_stats(self.write([json.dumps(frame(1, 5))]))
        with self.assertRaises(pacing.ScenarioError):
            pacing.read_stats(self.write([""]))


class AnalysisTests(unittest.TestCase):
    def test_steady_stream_has_no_hitches(self):
        report, failures = pacing.analyze(stream([5.0] * 50), 1)
        self.assertEqual(failures, [])
        warm = report["passes"][0]
        self.assertEqual(warm["hitch_count"], 0)
        self.assertEqual(warm["summary"]["frames"], 49)
        self.assertEqual(warm["summary"]["median_ms"], 5.0)
        self.assertAlmostEqual(warm["summary"]["cpu_median_ms"], 4.5)

    def test_seeded_hitch_is_found_and_attributed(self):
        intervals = [5.0] * 50
        intervals[20] = 40.0
        costs = {21: {"pipeline_create": [18, 30000], "record": [1, 500]}}
        report, _ = pacing.analyze(stream(intervals, marks={10: "fire_orange"}, costs=costs), 1)
        warm = report["passes"][0]
        self.assertEqual(warm["hitch_count"], 1)
        hitch = warm["hitches"][0]
        self.assertEqual((hitch["frame"], hitch["phase"], hitch["dominant"]), (21, "fire_orange", "pipeline_create"))
        self.assertEqual(warm["summary"]["pipeline_create"], {"count": 18, "ms": 30.0})
        self.assertEqual(warm["phases"]["fire_orange"]["hitches"], 1)

    def test_small_jitter_under_the_floor_is_not_a_hitch(self):
        intervals = [1.0] * 50
        intervals[10] = 3.0  # 3x the median, but under median + 4 ms
        report, _ = pacing.analyze(stream(intervals), 1)
        self.assertEqual(report["passes"][0]["hitch_count"], 0)

    def test_missing_pass_marks_fail(self):
        frames = [frame(i, 5.0) for i in range(1, 20)]
        _, failures = pacing.analyze(frames, 1)
        self.assertEqual(failures, ["pass 1 marks missing from the frame stream"])

    def test_marks_collapsed_onto_one_frame_fail(self):
        # What the console did when a cfg line overflowed: marks land together.
        frames = [frame(1, 5.0, "pass_1_begin,pass_1_end")] + [frame(i, 5.0) for i in range(2, 10)]
        _, failures = pacing.analyze(frames, 1)
        self.assertEqual(failures, ["pass 1 marks missing from the frame stream"])

    def test_a_pass_that_never_appears_fails_even_if_others_do(self):
        _, failures = pacing.analyze(stream([5.0] * 20), 2)
        self.assertEqual(failures, ["pass 2 marks missing from the frame stream"])

    def test_gpu_time_joins_by_frame_id(self):
        intervals = [5.0] * 30
        intervals[10] = 30.0
        frames = stream(intervals)
        frames[12]["gpu"] = [11, 7500]
        report, _ = pacing.analyze(frames, 1)
        self.assertEqual(report["passes"][0]["hitches"][0]["gpu_ms"], 7.5)


class BudgetTests(unittest.TestCase):
    def report(self, intervals):
        return pacing.analyze(stream(intervals), 1)[0]

    def test_budgets_gate_the_last_pass(self):
        intervals = [5.0] * 50
        intervals[20] = 40.0
        report = self.report(intervals)
        self.assertEqual(pacing.check_budgets(report, {}), [])
        self.assertEqual(len(pacing.check_budgets(report, {"max_hitches": 0})), 1)
        self.assertEqual(len(pacing.check_budgets(report, {"max_frame_ms": 16.0})), 1)
        self.assertEqual(pacing.check_budgets(report, {"max_frame_ms": 50.0, "max_hitches": 1}), [])

    def test_no_measured_pass_fails_every_budget(self):
        self.assertEqual(pacing.check_budgets({"passes": []}, {}), ["no measured pass"])


class ABTests(unittest.TestCase):
    def evidence(self, median, status="pass"):
        return {"status": status, "analysis": {"passes": [{"hitch_count": 0, "summary": {
            "median_ms": median, "cpu_median_ms": median, "emit_median_ms": median / 2,
            "p99_ms": median * 2, "max_ms": median * 3}}]}}

    def test_medians_over_rounds_and_ratio(self):
        runs = {"a": [self.evidence(m) for m in (18.0, 20.0, 19.0)],
                "b": [self.evidence(m) for m in (6.0, 5.0, 7.0)]}
        summary = pacing.ab_summary(runs)
        self.assertEqual(summary["a"]["median_ms"], 19.0)
        self.assertEqual(summary["b"]["median_ms"], 6.0)
        self.assertEqual(summary["b_over_a"]["median_ms"], round(6.0 / 19.0, 3))

    def test_failed_runs_are_excluded(self):
        runs = {"a": [self.evidence(10.0), self.evidence(99.0, status="fail")],
                "b": [self.evidence(5.0)]}
        self.assertEqual(pacing.ab_summary(runs)["a"]["rounds"], 1)


if __name__ == "__main__":
    unittest.main()
