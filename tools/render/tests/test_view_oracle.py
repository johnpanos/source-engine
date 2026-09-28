"""Fixtures for tools/render/view_oracle.py: capture parsing, the view tree,
the comparator's detection of seeded defects, and the scenario script."""
import gzip
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import view_oracle as vo


def view(kind="3d", target="backbuffer", origin=(0.0, 0.0, 64.0), fov=75.0, explicit=False,
         stack=1):
    info = {"type": kind, "stack": stack, "target": target, "explicit_target": explicit,
            "viewport": [0, 0, 640, 480], "origin": list(origin), "angles": [0.0, 90.0, 0.0],
            "fov": fov, "ortho": False, "flags": 0, "nodraw": False}
    return {"event": "label_begin", "name": vo.VIEW_PREFIX + json.dumps(info)}


def end():
    return {"event": "label_end"}


def draw(material, index_count=6, target="backbuffer"):
    return {"event": "draw", "material": material, "shader": "LightmappedGeneric", "pass": 0,
            "pass_count": 1, "primitive": 2, "first_index": 0, "index_count": index_count,
            "vertex_count": 4, "world_batch": False, "target": target,
            "viewport": [0, 0, 640, 480], "submitted": True}


def numbered(events):
    return [dict(event, seq=index) for index, event in enumerate(events)]


def portal_frame():
    """Main view; a portal view nested two deep; a view model; water targets; 2D."""
    return numbered([
        {"event": "clear", "color": True, "depth": True, "stencil": True, "target": "backbuffer"},
        view(),
        draw("concrete/wall"), draw("concrete/floor"),
        {"event": "label_begin", "name": "CSimpleWorldView::Draw"},  # not a view
        view(origin=(128.0, 0.0, 64.0), stack=2), draw("concrete/wall"),
        view(origin=(256.0, 0.0, 64.0), stack=3), draw("concrete/wall"), end(),
        end(),
        end(),
        view(fov=54.0, stack=2), draw("models/weapons/v_portalgun"), end(),
        view(target="_rt_WaterReflection", explicit=True, stack=2), draw("nature/sky"), end(),
        view(target="_rt_WaterRefraction", explicit=True, stack=2), end(),
        draw("dev/engine_post"),
        end(),
        view(kind="2d"), draw("vgui/hud"), end(),
        {"event": "marker", "name": "anything"},
    ])


def write_capture(path, event_list, **header):
    body = {"schema": vo.SCHEMA, "backend": vo.BACKEND, "reason": "screenshot", "index": 0,
            "events": len(event_list), "truncated": False}
    body.update(header)
    text = json.dumps(body) + "\n" + "".join(json.dumps(event) + "\n" for event in event_list)
    if str(path).endswith(".gz"):
        with gzip.open(path, "wt") as stream:
            stream.write(text)
    else:
        Path(path).write_text(text)


class ViewTreeTest(unittest.TestCase):
    def test_views_nest_and_non_view_labels_are_ignored(self):
        views = vo.build_frame(portal_frame())
        paths = [v.path for v in views]
        self.assertEqual(paths, ["frame", "frame/0", "frame/0/0", "frame/0/0/0", "frame/0/1",
                                 "frame/0/2", "frame/0/3", "frame/1"])
        main = views[1]
        self.assertEqual([item[0] for item in main.items],
                         ["draw", "draw", "view", "view", "view", "view", "draw"])
        # The clear before any view belongs to the frame itself.
        self.assertEqual(views[0].items[0][0], "clear")

    def test_kinds_and_nesting_depth(self):
        views = vo.build_frame(portal_frame())
        kinds = {v.path: v.kind for v in views}
        self.assertEqual(kinds["frame/0"], "main")
        self.assertEqual(kinds["frame/0/0"], "nested")
        self.assertEqual(kinds["frame/0/0/0"], "nested")
        self.assertEqual(kinds["frame/0/1"], "viewmodel")
        self.assertEqual(kinds["frame/0/2"], "water_reflection")
        self.assertEqual(kinds["frame/0/3"], "water_refraction")
        self.assertEqual(kinds["frame/1"], "2d")
        summary = vo.frame_summary(views)
        self.assertEqual(summary["max_nested_depth"], 2)
        self.assertEqual(summary["draws"], 8)

    def test_nested_view_inside_a_monitor_is_nested(self):
        events = numbered([view(), view(target="_rt_Camera", explicit=True, stack=2),
                           view(target="_rt_Camera", origin=(9.0, 0.0, 0.0), stack=3), end(),
                           end(), end()])
        kinds = [v.kind for v in vo.build_frame(events)]
        self.assertEqual(kinds, ["frame", "main", "monitor", "nested"])

    def test_unbalanced_labels_are_rejected(self):
        with self.assertRaises(vo.OracleError):
            vo.build_frame(numbered([view(), draw("a")]))
        with self.assertRaises(vo.OracleError):
            vo.build_frame(numbered([end()]))
        with self.assertRaises(vo.OracleError):
            vo.build_frame(numbered([{"event": "surprise"}]))


class ReadCaptureTest(unittest.TestCase):
    def test_round_trip_plain_and_gzip(self):
        with tempfile.TemporaryDirectory() as temp:
            for name in ("a.jsonl", "a.jsonl.gz"):
                path = Path(temp) / name
                write_capture(path, portal_frame())
                header, events = vo.read_capture(path)
                self.assertEqual(len(events), header["events"])

    def test_incomplete_captures_are_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "a.jsonl"
            write_capture(path, portal_frame(), events=3)
            with self.assertRaises(vo.OracleError):
                vo.read_capture(path)
            write_capture(path, portal_frame(), truncated=True)
            with self.assertRaises(vo.OracleError):
                vo.read_capture(path)
            write_capture(path, portal_frame(), schema="other/v1")
            with self.assertRaises(vo.OracleError):
                vo.read_capture(path)
            events = portal_frame()
            events[3]["seq"] = 99
            write_capture(path, events)
            with self.assertRaises(vo.OracleError):
                vo.read_capture(path)


class CompareTest(unittest.TestCase):
    def setUp(self):
        self.events = portal_frame()
        self.views = vo.build_frame(self.events)

    def diverges(self, events):
        return vo.compare_frames(self.views, vo.build_frame(numbered(events)))

    def test_identical_frames_agree(self):
        self.assertEqual(vo.compare_frames(self.views, vo.build_frame(portal_frame())), [])

    def test_removing_any_single_draw_is_detected(self):
        draws = [i for i, event in enumerate(self.events) if event["event"] == "draw"]
        for index in draws:
            found = self.diverges(self.events[:index] + self.events[index + 1:])
            self.assertTrue(found, "draw %d" % index)
        # The report names the view and the first diverging draw.
        found = self.diverges(self.events[:6] + self.events[7:])
        self.assertEqual(found[0]["view"], "frame/0/0")
        self.assertEqual(found[0]["key"], "frame/0/0#0:concrete/wall/0")

    def test_removing_an_empty_view_is_detected(self):
        begin = next(i for i, e in enumerate(self.events)
                     if "WaterRefraction" in e.get("name", ""))
        found = self.diverges(self.events[:begin] + self.events[begin + 2:])
        self.assertTrue(any(item.get("view") == "frame/0/3" for item in found))

    def test_a_changed_field_and_a_swap_are_detected(self):
        changed = [dict(e) for e in self.events]
        changed[3]["index_count"] = 7
        self.assertTrue(self.diverges(changed))
        swapped = list(self.events)
        swapped[2], swapped[3] = swapped[3], swapped[2]
        self.assertTrue(self.diverges(swapped))

    def test_pose_is_compared_within_tolerance(self):
        moved = list(self.events)
        near = view(origin=(0.1, 0.0, 64.0))
        far = view(origin=(0.3, 0.0, 64.0))
        self.assertEqual(self.diverges(moved[:1] + [near] + moved[2:]), [])
        self.assertEqual(self.diverges(moved[:1] + [far] + moved[2:])[0]["field"], "origin")

    def test_a_changed_view_target_is_detected(self):
        events = list(self.events)
        events[1] = view(target="_rt_other")
        self.assertTrue(self.diverges(events))


class DrawStateSeedTest(unittest.TestCase):
    def test_every_state_field_seed_changes_the_record(self):
        record = {"material": "m", "shader": "s", "target": "backbuffer",
                  "samplers": [{"stage": 0, "texture": "t"}], "blend": False,
                  "src_blend": "one", "dst_blend": "zero", "depth_test": True,
                  "depth_write": True, "alpha_test_ref": None, "modulation": [1, 1, 1, 1],
                  "base_texture_transform": [1, 0, 0, 0, 0, 1, 0, 0], "submitted": True}
        for field in vo.draw_state_diff.STATE_FIELDS:
            changed = vo.seed_state_change(record, field)
            self.assertNotEqual(changed[field], record[field], field)
            self.assertTrue(vo.draw_state_diff.compare_exact(
                [record], [changed], vo.DECLARED_NONDETERMINISTIC), field)


class ScenarioTest(unittest.TestCase):
    def scenario(self):
        return {"id": "s", "game": "portal", "map": "m", "intro_frames": 100,
                "setup": ["noclip", 'ent_fire a NewLocation "1 2 3 0 0 0"'],
                "shots": [{"view_set": "legacy-ports", "camera": [1, 2, 3]},
                          {"name": "extra", "commands": ["cmd setpos 1 2 3", "r_x 1"],
                           "settle": 5}]}

    def test_legacy_ports_set_expands_with_the_pause_menu_last(self):
        shots = vo.expand_shots(self.scenario())
        names = [shot["name"] for shot in shots]
        self.assertEqual(len(names), len(vo.legacy_ports_views.VIEWS) + 2)
        self.assertEqual(names[-2:], ["extra", "ports_pause_menu"])
        self.assertEqual(shots[0]["commands"][0], "cmd setpos 1 2 3")

    def test_script_chains_one_alias_per_shot(self):
        scenario = self.scenario()
        shots = vo.expand_shots(scenario)
        lines = vo.console_script(scenario, shots)
        self.assertEqual(lines[:2], scenario["setup"])
        aliases = [line for line in lines if line.startswith("alias ")]
        self.assertEqual(len(aliases), len(shots))
        self.assertTrue(aliases[0].endswith('; vo_shot1"'))
        self.assertTrue(all(line.count("screenshot") == 1 for line in aliases))
        self.assertEqual(lines[-1], "wait 100; vo_shot0")
        # The camera is placed again right before the frame.
        extra = aliases[-2]
        self.assertLess(extra.rindex("cmd setpos"), extra.index("screenshot"))
        self.assertGreater(extra.rindex("cmd setpos"), extra.index("wait 5"))

    def test_a_quoted_shot_command_is_refused(self):
        scenario = self.scenario()
        scenario["shots"] = [{"name": "q", "commands": ['ent_fire a b "c d"']}]
        with self.assertRaises(vo.OracleError):
            vo.console_script(scenario, vo.expand_shots(scenario))

    def test_coverage_expectations(self):
        views = vo.build_frame(portal_frame())
        self.assertEqual(vo.coverage_problems({"expect": {"nested_depth": 2}}, views), [])
        self.assertTrue(vo.coverage_problems({"expect": {"nested_depth": 3}}, views))
        self.assertEqual(vo.coverage_problems(
            {"expect": {"kinds": {"water_reflection": 1}, "materials": ["wall"]}}, views), [])
        self.assertTrue(vo.coverage_problems({"expect": {"kinds": {"monitor": 1}}}, views))
        self.assertTrue(vo.coverage_problems({"expect": {"materials": ["glass"]}}, views))

    def test_the_workload_is_valid(self):
        workload = vo.load_workload()
        ids = {scenario["id"] for scenario in workload["scenarios"]}
        self.assertIn("portal_testchmb_a_00", ids)
        self.assertIn("portal_testchmb_a_08", ids)
        self.assertTrue(any(s["game"] == "portal2" for s in workload["scenarios"]))
        for scenario in workload["scenarios"]:
            vo.console_script(scenario, vo.expand_shots(scenario))


if __name__ == "__main__":
    unittest.main()
