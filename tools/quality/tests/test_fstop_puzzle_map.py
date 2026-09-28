"""The F-Stop puzzle chamber generator (tools/quality/fstop_puzzle_map.py):
its entity I/O is closed, its models are declared, the numbers the intended
solution relies on hold, and the walkthrough is valid console input."""

import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import fstop_puzzle_map as puzzle  # noqa: E402

# Targets the engine resolves itself rather than by name.
SPECIAL_TARGETS = {"!self", "!activator", "!caller", "!player"}


def entities(text):
    """(classname, keyvalues, outputs) for every entity block of a VMF."""
    found = []
    for block in text.split("\nentity\n")[1:]:
        keys = dict(re.findall(r'^\t"([^"]+)" "([^"]*)"$', block, re.MULTILINE))
        outputs = re.findall(r'^\t\t"(On\w+)" "([^"]*)"$', block, re.MULTILINE)
        found.append((keys.get("classname"), keys, outputs))
    return found


class ChamberTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.text = puzzle.vmf_text()
        cls.entities = entities(cls.text)
        cls.names = {kv["targetname"] for _, kv, _ in cls.entities if "targetname" in kv}

    def test_deterministic(self):
        self.assertEqual(self.text, puzzle.vmf_text())

    def test_every_output_target_exists(self):
        for classname, _, outputs in self.entities:
            for event, connection in outputs:
                target = connection.split(",")[0]
                if target not in SPECIAL_TARGETS:
                    self.assertIn(target, self.names, "%s %s -> %s" % (classname, event, target))

    def test_filters_and_parents_exist(self):
        for _, kv, _ in self.entities:
            for key in ("filtername", "parentname", "target", "partnername"):
                if key in kv and kv.get("classname") != "filter_activator_name":
                    self.assertIn(kv[key], self.names, "%s=%s" % (key, kv[key]))
            for key, value in kv.items():
                if re.fullmatch(r"Filter\d\d", key):
                    self.assertIn(value, self.names)

    def test_models_are_declared(self):
        # Every model is one the generator checks, never Valve's missing
        # F-Stop centerpieces (dollhouse04, tombstone001).
        models = puzzle.referenced_models(self.text)
        self.assertTrue(models)
        self.assertLessEqual(set(models), set(puzzle.MODELS))
        for classname, _, _ in self.entities:
            self.assertNotIn(classname, ("prop_building", "prop_tombstone"))

    def test_no_raw_newlines_in_values(self):
        # Hammer's VMF format has no multi-line values; game_text takes "\n".
        for line in self.text.splitlines():
            self.assertEqual(line.count('"') % 2, 0, line)

    def test_puzzle_pieces(self):
        classes = [c for c, _, _ in self.entities]
        for classname in ("weapon_camera", "trigger_photo_eraser", "func_door", "filter_size",
                          "filter_multi", "info_teleport_destination"):
            self.assertIn(classname, classes)
        self.assertEqual(classes.count("info_placement_helper"), 2)
        captured = {kv["targetname"] for c, kv, _ in self.entities
                    if kv.get("canbecaptured") == "1" and "targetname" in kv}
        self.assertLessEqual({"cube", "haybale", "anvil"}, captured)
        camera = next(kv for c, kv, _ in self.entities if c == "weapon_camera")
        self.assertEqual(camera["canscale"], "0")   # the size scroller enables it

    def test_solvable(self):
        s = puzzle.solvability()
        # 1: the pit is wider than a running jump (~170); the button is in reach.
        self.assertGreater(s["pit_width"], 170)
        self.assertLessEqual(s["button1_reach_from_edge"], s["placement_reach"])
        # 2: only the 2x bale makes the ledge climbable.
        reach, ledge = s["jump_reach"], s["ledge_height"]

        def climbable(step):
            return step <= reach and ledge - step <= reach

        self.assertGreater(ledge, reach)
        self.assertTrue(climbable(s["bale_heights"][2.0]))
        for scale in (0.25, 0.5, 1.0, 4.0):
            self.assertFalse(climbable(s["bale_heights"][scale]), scale)
        self.assertFalse(climbable(s["cube_height"]))
        # 3 and 4: the anvil is carried only when shrunk.
        self.assertGreater(s["anvil_mass_full"], s["lift_limit"])
        self.assertLessEqual(s["anvil_mass_half"], s["lift_limit"])


class WalkthroughTests(unittest.TestCase):
    def test_chain(self):
        cfgs = puzzle.walkthrough_cfgs()
        names = list(cfgs)
        self.assertGreater(len(names), 1)
        for i, name in enumerate(names):
            line = cfgs[name]
            self.assertEqual(line.count("\n"), 1, name)        # wait works only on one line
            self.assertLess(len(line), 512, name)
            if i + 1 < len(names):
                self.assertTrue(line.rstrip().endswith("exec " + names[i + 1][:-4]))
        self.assertTrue(cfgs[names[0]].startswith("sv_cheats 1"))
        self.assertTrue(cfgs[names[-1]].rstrip().endswith("quit"))

    def test_expected_outputs_name_map_entities(self):
        text = puzzle.vmf_text()
        for expected in puzzle.WALKTHROUGH_OUTPUTS:
            target = re.search(r"-> \((\w+),", expected).group(1)
            self.assertIn('"targetname" "%s"' % target, text)

    def test_writes_cfgs(self):
        with tempfile.TemporaryDirectory() as directory:
            for name, line in puzzle.walkthrough_cfgs().items():
                (Path(directory) / name).write_text(line)
            self.assertTrue((Path(directory) / (puzzle.NAME + "_walk_0.cfg")).is_file())


class ModelCheckTests(unittest.TestCase):
    def test_missing_models_reported(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "portal" / "models" / "props").mkdir(parents=True)
            (root / "portal" / "models" / "props" / "metal_box.mdl").write_bytes(b"IDST")
            game = root / "fstop"
            game.mkdir()
            valve = root / "fstop_valve_tempcontent" / "models" / "props_farm"
            valve.mkdir(parents=True)
            (valve / "anvil.mdl").write_bytes(b"IDST")
            missing = puzzle.missing_models(
                ["models/props/metal_box.mdl", "models/props_farm/anvil.mdl",
                 "models/props_fstop/dollhouse04.mdl"], root, game)
            self.assertEqual(missing, ["models/props_fstop/dollhouse04.mdl"])


if __name__ == "__main__":
    unittest.main()
