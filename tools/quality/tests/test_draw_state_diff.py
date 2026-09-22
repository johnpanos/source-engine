"""Comparator fixtures for draw-state diffs, including seeded differences."""

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


QUALITY = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("draw_state_diff", QUALITY / "draw_state_diff.py")
diff = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(diff)


def draw(**changes):
    result = {"seq": 0, "material": "concrete/wall", "shader": "LightmappedGeneric", "pass": 0,
              "pass_count": 1, "target": "backbuffer", "viewport": [0, 0, 640, 480],
              "depth_range": [0, 1],
              "samplers": [{"stage": 0, "texture": "concrete/wall", "address_u": "wrap",
                            "address_v": "wrap", "filter": "linear"}],
              "blend": False, "src_blend": "one", "dst_blend": "zero", "depth_test": True,
              "depth_write": True, "alpha_test_ref": None, "modulation": [1, 1, 1, 1],
              "base_texture_transform": [1, 0, 0, 0, 0, 1, 0, 0], "submitted": True}
    result.update(changes)
    return result


class DrawStateDiffTest(unittest.TestCase):
    def test_identical_state_has_no_differences(self):
        report = diff.compare([draw()], [draw(seq=5)])
        self.assertEqual(report["differences"], [])
        self.assertEqual(report["compared_groups"], 1)

    def test_viewport_follows_the_window_and_is_not_compared(self):
        report = diff.compare([draw()], [draw(viewport=[0, 0, 2880, 1620])])
        self.assertEqual(report["differences"], [])

    def test_clamped_sampler_is_detected(self):
        clamped = [{"stage": 0, "texture": "concrete/wall", "address_u": "clamp",
                    "address_v": "clamp", "filter": "linear"}]
        report = diff.compare([draw()], [draw(samplers=clamped)])
        self.assertEqual([item["field"] for item in report["differences"]], ["samplers"])

    def test_register_difference_beyond_tolerance_is_detected(self):
        report = diff.compare([draw()], [draw(modulation=[1, 1, 1, 0.5])])
        self.assertEqual(report["differences_by_field"], {"modulation": 1})
        near = diff.compare([draw()], [draw(modulation=[1, 1, 1, 1.00001])])
        self.assertEqual(near["differences"], [])

    def test_dropped_draw_is_detected(self):
        report = diff.compare([draw()], [draw(submitted=False, drop_reason="no base texture")])
        self.assertEqual(report["differences_by_field"], {"submitted": 1})

    def test_materials_drawn_by_only_one_backend_are_listed(self):
        report = diff.compare([draw(), draw(material="dev/engine_post")], [draw()])
        self.assertEqual(report["reference_only"], ["dev/engine_post#0"])
        self.assertEqual(report["candidate_only"], [])

    def test_header_draw_count_must_match(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "short.jsonl"
            path.write_text(json.dumps({"schema": diff.SCHEMA, "draws": 2}) + "\n"
                            + json.dumps(draw()) + "\n")
            with self.assertRaises(diff.FixtureError):
                diff.read_fixture(path)

    def test_exit_codes(self):
        with tempfile.TemporaryDirectory() as temp:
            same = Path(temp) / "a.jsonl"
            other = Path(temp) / "b.jsonl"
            header = json.dumps({"schema": diff.SCHEMA, "draws": 1}) + "\n"
            same.write_text(header + json.dumps(draw()) + "\n")
            other.write_text(header + json.dumps(draw(depth_write=False)) + "\n")
            self.assertEqual(diff.main(["--reference", str(same), "--candidate", str(same)]), 0)
            self.assertEqual(diff.main(["--reference", str(same), "--candidate", str(other)]), 1)
            bad = Path(temp) / "bad.jsonl"
            bad.write_text("{}\n")
            self.assertEqual(diff.main(["--reference", str(bad), "--candidate", str(same)]), 2)


if __name__ == "__main__":
    unittest.main()
