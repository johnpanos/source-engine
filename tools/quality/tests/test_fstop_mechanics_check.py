import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import fstop_mechanics_check as check  # noqa: E402
import fstop_mechanics_scenarios as scenarios  # noqa: E402

LOG = """\
(3.00) output: (prop_android_dispenser,dispenser) -> (probe.dispenser.OnSpawnNPC,Trigger)()
(3.00) output: (prop_android_dispenser,dispenser) -> (probe.dispenser.OnSpawnNPC,Trigger)()
(4.29) input : check_server.Command(report_entities)
Class: npc_android_basic (2)
Class: prop_physics (10)
Total 139 entities (0 empty, 133 edicts)
setpos -1150.000000 0.000000 64.031250;setang 0.000000 0.000000 0.000000
(10.56) output: (prop_android_dispenser,dispenser) -> (probe.dispenser.OnSpawnNPC,Trigger)()
setpos 200.000000 0.000000 64.031250;setang 0.000000 0.000000 0.000000
(15.28) input : check_server.Command(report_entities)
Class: npc_android_basic (1)
Total 143 entities (0 empty, 137 edicts)
  health: 100
(sv) EmitSound:  'NPC_Chicken.Clucks' emitted as 'npc/chicken/cluck1.wav' (ent 42)
  far_radius: 5.00
  health: 64
"""


class JudgeTests(unittest.TestCase):
    def judge(self, check_fn, log=LOG):
        return check_fn(log)[0]

    def test_fired_counts_distinct_game_times(self):
        self.assertTrue(self.judge(check.fired("dispenser", "OnSpawnNPC", 2)))
        self.assertFalse(self.judge(check.fired("dispenser", "OnSpawnNPC", 3)))
        self.assertFalse(self.judge(check.fired("dispenser", "OnChildKilled")))
        self.assertTrue(self.judge(check.not_fired("dispenser", "OnChildKilled")))
        self.assertFalse(self.judge(check.not_fired("dispenser", "OnSpawnNPC")))

    def test_entity_tables_in_order(self):
        tables = check.entity_tables(LOG)
        self.assertEqual([t["npc_android_basic"] for t in tables], [2, 1])
        self.assertTrue(self.judge(check.count_change("npc_android_basic", -1)))
        self.assertFalse(self.judge(check.count_change("npc_android_basic", 0)))
        self.assertTrue(self.judge(check.count_at_least("npc_android_basic", 1)))
        self.assertFalse(self.judge(check.count_at_least("prop_physics", 1)))  # last table

    def test_player_motion(self):
        self.assertEqual(len(check.positions(LOG)), 2)
        self.assertTrue(self.judge(check.displaced(1000)))
        self.assertFalse(self.judge(check.displaced(2000)))
        self.assertTrue(self.judge(check.jumped("x", 1000)))
        self.assertFalse(self.judge(check.peak("z", 1)))
        self.assertTrue(self.judge(check.moved("x", 1300)))

    def test_sounds_dumps_and_health(self):
        self.assertTrue(self.judge(check.emitted("NPC_Chicken.Clucks")))
        self.assertFalse(self.judge(check.emitted("sphere02")))
        self.assertTrue(self.judge(check.dumped("far_radius", "5.00")))
        self.assertFalse(self.judge(check.dumped("far_radius", "0.00")))
        self.assertTrue(self.judge(check.health_dropped()))
        self.assertFalse(self.judge(check.health_dropped(), LOG.replace("health: 64",
                                                                         "health: 100")))

    def test_missing_evidence_fails(self):
        for judge in (check.displaced(1), check.jumped("x", 1), check.peak("z", 1),
                      check.count_change("npc_android_basic", 0), check.health_dropped()):
            self.assertFalse(judge("")[0])


class ScenarioTests(unittest.TestCase):
    def test_every_scenario_compiles_to_whole_second_ent_fires_ending_in_quit(self):
        names = [s.name for s in scenarios.SCENARIOS]
        self.assertEqual(len(names), len(set(names)))
        for scenario in scenarios.SCENARIOS:
            lines = scenario.cfg().splitlines()
            self.assertTrue(all(line.startswith("ent_fire check_") for line in lines))
            self.assertTrue(lines[-1].startswith('ent_fire check_server Command "quit"'))
            self.assertTrue(scenario.checks, scenario.name)

    def test_rejects_fractional_times_and_quotes(self):
        with self.assertRaises(ValueError):
            check.Scenario("x", "", [(1.5, "echo")], [lambda log: (True, "")]).cfg()
        with self.assertRaises(ValueError):
            check.Scenario("x", "", [(1, 'echo "hi"')], [lambda log: (True, "")]).cfg()


if __name__ == "__main__":
    unittest.main()
