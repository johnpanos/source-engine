"""Independent malformed, stale-shader and real native-collector trace fixtures."""

import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest


QUALITY = Path(__file__).resolve().parents[1]
ROOT = QUALITY.parents[1]
SPEC = importlib.util.spec_from_file_location("render_trace", QUALITY / "render_trace.py")
trace = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(trace)


def shader(name="sample_ps20", failed=False):
    return {"name": name, "static_index": 0, "dynamic_index": 0, "failed": failed}


def draw(**changes):
    result = {"event": "draw", "frame": 1, "material": "brick", "kind": "indexed",
              "primitives": 2, "result": 0, "viewport": [320, 240], "target_size": [320, 240],
              "target_format": 21, "target": 100, "vs_bound": 200, "ps_bound": 300,
              "texture_mask": 1, "depth_test": 1, "depth_write": 1, "color_write": 15,
              "cull": 3, "blend": 0, "query_errors": 0,
              "vs_requested": shader("quad_vs20"), "ps_requested": shader()}
    result.update(changes)
    return result


def capture(*draws):
    events = [{"event": "capture_begin", "schema": trace.SCHEMA, "first_frame": 1,
               "frame_count": 1, "max_draws": 100}]
    events.extend(draws)
    events.extend([{"event": "present", "frame": 1, "result": 0},
                   {"event": "frame", "frame": 1, "draws": len(draws),
                    "primitives": sum(max(e["primitives"], 0) for e in draws),
                    "draw_errors": sum(e["result"] < 0 for e in draws),
                    "failed_shader_requests": sum(e["vs_requested"]["failed"] or e["ps_requested"]["failed"] for e in draws),
                    "skipped": 0},
                   {"event": "capture_end", "frames": 1, "draw_details": len(draws), "shader_failures": 0}])
    return events


class AnalyzerTests(unittest.TestCase):
    def codes(self, *draws):
        return {item["code"] for item in trace.analyze(capture(*draws))["observations"]}

    def test_draw_success_is_not_visibility(self):
        report = trace.analyze(capture(draw()))
        self.assertEqual("complete", report["status"])
        self.assertFalse(report["visibility_verified"])
        self.assertEqual(2, report["materials"]["brick"]["primitives"])
        self.assertEqual(["brick"], report["targets"]["100"]["materials"])

    def test_constant_shader_remains_bound_after_failed_material_request(self):
        report = trace.analyze(capture(draw(ps_requested=shader("constant_ps20")),
                                      draw(ps_requested=shader("lightmapped_ps20b", True))))
        stale = next(item for item in report["observations"] if item["code"] == "stale_shader_binding")
        self.assertEqual(300, stale["first"]["bound"])
        self.assertEqual("constant_ps20", stale["first"]["previous_request_at_bound_pointer"]["name"])
        self.assertEqual("lightmapped_ps20b", stale["first"]["requested"]["name"])
        self.assertEqual("issues_observed", report["status"])

    def test_failed_request_without_prior_identity_is_still_reported(self):
        self.assertIn("failed_request_with_bound_shader", self.codes(draw(ps_requested=shader(failed=True))))
        self.assertNotIn("stale_shader_binding", self.codes(draw(ps_requested=shader(failed=True))))

    def test_missing_shader_is_distinct_from_fixed_function(self):
        self.assertIn("requested_shader_unbound", self.codes(draw(ps_bound=0)))
        self.assertIn("failed_request_without_shader", self.codes(draw(ps_bound=0, ps_requested=shader(failed=True))))
        self.assertNotIn("requested_shader_unbound", self.codes(draw(ps_bound=0, ps_requested=shader(""))))

    def test_each_geometry_state_and_draw_failure_has_own_diagnosis(self):
        cases = (({"primitives": 0}, "no_geometry"), ({"viewport": [0, 240]}, "zero_viewport"),
                 ({"target_size": [320, 0]}, "missing_target"), ({"result": -1}, "draw_error"),
                 ({"color_write": 0}, "no_color_writes"))
        for fields, code in cases:
            with self.subTest(code=code):
                self.assertIn(code, self.codes(draw(**fields)))

    def test_depth_only_draw_is_observation_not_failure(self):
        report = trace.analyze(capture(draw(color_write=0)))
        self.assertEqual("complete", report["status"])
        self.assertEqual("observation", report["observations"][0]["severity"])
        self.assertEqual([], report["actionable_errors"])

    def test_zero_primitive_submission_is_not_a_render_contract_failure(self):
        report = trace.analyze(capture(draw(primitives=0)))
        self.assertEqual("complete", report["status"])
        self.assertEqual("no_geometry", report["observations"][0]["code"])

    def test_failed_state_query_does_not_invent_zero_state(self):
        codes = self.codes(draw(query_errors=1, viewport=[0, 0], target_size=[0, 0],
                               target=0, color_write=0, ps_bound=0))
        self.assertEqual({"state_query_unavailable"}, codes)

    def test_present_error(self):
        events = capture(draw())
        events[-3]["result"] = -1
        report = trace.analyze(events)
        self.assertEqual(1, report["totals"]["present_errors"])
        self.assertEqual("issues_observed", report["status"])

    def test_missing_or_duplicate_terminal_and_schema_fail(self):
        good = capture(draw())
        variants = [good[:-1], good + [draw()], [good[0]] + good,
                    [{**good[0], "schema": "other/v1"}] + good[1:]]
        for events in variants:
            with self.subTest(events=events):
                with self.assertRaises(trace.TraceError):
                    trace.analyze(events)

    def test_empty_and_no_draw_or_present_evidence_fail(self):
        for events in ([], capture(), [e for e in capture(draw()) if e["event"] != "present"]):
            with self.assertRaises(trace.TraceError):
                trace.analyze(events)

    def test_missing_state_fields_and_wrong_types_fail(self):
        for field in draw():
            events = capture(draw())
            del events[1][field]
            with self.subTest(field=field), self.assertRaises(trace.TraceError):
                trace.analyze(events)
        for fields in ({"ps_bound": True}, {"viewport": [320]}, {"ps_requested": None},
                       {"result": "0"}, {"primitives": 1.5}):
            events = capture(draw())
            events[1].update(fields)
            with self.subTest(fields=fields), self.assertRaises(trace.TraceError):
                trace.analyze(events)

    def test_counts_bounds_and_frame_consistency_fail(self):
        good = capture(draw())
        for index, fields in ((-1, {"draw_details": 2}), (-1, {"frames": 2}),
                              (-1, {"shader_failures": 1}), (-2, {"draws": 0}),
                              (0, {"first_frame": 2}), (0, {"max_draws": 0})):
            events = copy.deepcopy(good)
            events[index].update(fields)
            with self.subTest(fields=fields), self.assertRaises(trace.TraceError):
                trace.analyze(events)

    def test_json_lines_truncation_blank_and_duplicate_fields_fail(self):
        good = "\n".join(json.dumps(e) for e in capture(draw())) + "\n"
        for content in (good[:-1], good + "\n", good + "{\"event\":", '{"event":"a","event":"b"}\n'):
            with tempfile.TemporaryDirectory() as directory:
                path = Path(directory) / "trace.jsonl"
                path.write_text(content)
                with self.assertRaises(trace.TraceError):
                    trace.read_events(path)

    def test_cli_records_invalid_evidence_instead_of_success(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "report.json"
            result = subprocess.run(["python3", str(QUALITY / "render_trace.py"), "--input",
                                     str(Path(directory) / "missing.jsonl"), "--out", str(output)],
                                    capture_output=True, text=True)
            self.assertEqual(2, result.returncode)
            self.assertEqual("invalid", json.loads(output.read_text())["status"])


class CollectorIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.directory.cleanup)
        cls.executable = Path(cls.directory.name) / "collector"
        subprocess.run(["g++", "-std=c++11", "-pthread", "-I" + str(ROOT / "materialsystem/shaderapidx9"),
                        str(QUALITY / "tests/fixtures/cpp/render_trace.cpp"),
                        str(ROOT / "materialsystem/shaderapidx9/render_diagnostics.cpp"),
                        "-o", str(cls.executable)], check=True, capture_output=True, text=True)

    def test_real_collector_escaping_stale_shader_and_bounded_details(self):
        path = Path(self.directory.name) / "real.jsonl"
        subprocess.run([str(self.executable), str(path)], check=True)
        events, digest = trace.read_events(path)
        report = trace.analyze(events)
        self.assertEqual(64, len(digest))
        self.assertIn('material"\\\n\tname', report["materials"])
        self.assertEqual(4, report["totals"]["draws"])
        self.assertEqual(3, report["coverage"]["draw_details"])
        self.assertEqual(1, report["coverage"]["undetailed_selected_draws"])
        self.assertTrue(report["coverage"]["detail_limit_reached"])
        self.assertEqual(3, report["totals"]["failed_shader_requests"])
        self.assertIn("stale_shader_binding", {item["code"] for item in report["observations"]})

    def test_disabled_collector_writes_nothing(self):
        before = set(Path(self.directory.name).iterdir())
        result = subprocess.run([str(self.executable)], capture_output=True, text=True, check=True)
        self.assertEqual("", result.stdout + result.stderr)
        self.assertEqual(before, set(Path(self.directory.name).iterdir()))


if __name__ == "__main__":
    unittest.main()
