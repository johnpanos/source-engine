"""Negative fixtures for the Portal 2 gameplay scenario evaluator and workload."""

import json
from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))
import portal2_scenarios as scenarios

ROOT = QUALITY.parents[1]
SCENARIO = {"name": "demo", "map": "sp_demo", "script": "demo.nut", "timeout_seconds": 60,
            "required_checks": ["socket.fired", "socket.parented"]}
GOOD = """noise before
QA_LOG demo t=1 start map=sp_demo steps=3
QA_CHECK demo.socket.fired PASS sphere (1 2 3)
QA_CHECK demo.socket.parented PASS parent=socket
QA_DONE demo checks=2 failures=0
"""


class EvaluateTests(unittest.TestCase):
    def evaluate(self, log=GOOD, code=0, timed_out=False, scenario=SCENARIO):
        return scenarios.evaluate(scenario, log, code, timed_out)

    def assertFailsWith(self, result, text):
        self.assertEqual(result["status"], "fail")
        self.assertTrue(any(text in failure for failure in result["failures"]), result["failures"])

    def test_complete_passing_run(self):
        result = self.evaluate()
        self.assertEqual(result["status"], "pass", result["failures"])
        self.assertEqual(result["checks"]["socket.fired"]["detail"], "sphere (1 2 3)")

    def test_failed_check(self):
        log = GOOD.replace("socket.parented PASS parent=socket", "socket.parented FAIL parent=<none>")
        log = log.replace("failures=0", "failures=1")
        self.assertFailsWith(self.evaluate(log), "socket.parented failed: parent=<none>")

    def test_missing_required_check(self):
        log = "\n".join(line for line in GOOD.splitlines() if "parented" not in line)
        log = log.replace("checks=2", "checks=1")
        self.assertFailsWith(self.evaluate(log), "required check not reported: socket.parented")

    def test_undeclared_check(self):
        log = GOOD.replace("QA_DONE demo checks=2", "QA_CHECK demo.step.04 FAIL script error\nQA_DONE demo checks=3")
        log = log.replace("failures=0", "failures=1")
        self.assertFailsWith(self.evaluate(log), "undeclared check: step.04")

    def test_duplicate_check(self):
        log = GOOD.replace("QA_DONE", "QA_CHECK demo.socket.fired PASS again\nQA_DONE")
        self.assertFailsWith(self.evaluate(log), "check reported twice: socket.fired")

    def test_check_from_another_scenario(self):
        log = GOOD.replace("QA_DONE", "QA_CHECK other.socket.fired PASS\nQA_DONE")
        self.assertFailsWith(self.evaluate(log), "check from another scenario: other.socket.fired")

    def test_missing_done_record(self):
        log = "\n".join(line for line in GOOD.splitlines() if not line.startswith("QA_DONE"))
        self.assertFailsWith(self.evaluate(log), "expected one QA_DONE record, found 0")

    def test_two_done_records(self):
        self.assertFailsWith(self.evaluate(GOOD + "QA_DONE demo checks=2 failures=0\n"),
                             "expected one QA_DONE record, found 2")

    def test_done_counts_must_match_the_log(self):
        # A check line the parser cannot read still counts in the driver's total.
        self.assertFailsWith(self.evaluate(GOOD.replace("checks=2", "checks=3")),
                             "QA_DONE counts 3/0 differ")

    def test_done_names_the_scenario(self):
        self.assertFailsWith(self.evaluate(GOOD.replace("QA_DONE demo", "QA_DONE other")),
                             "QA_DONE names scenario other")

    def test_scenario_script_error_fails(self):
        log = GOOD + "\nAN ERROR HAS OCCURED [the index 'x' does not exist]\n\nCALLSTACK\n" \
                     "*FUNCTION [main()] qa/demo.nut line [4]\n"
        self.assertFailsWith(self.evaluate(log), "scenario script error: AN ERROR HAS OCCURED")

    def test_map_script_error_is_recorded_but_not_fatal(self):
        log = GOOD + "\nAN ERROR HAS OCCURED [wrong number of parameters]\n\nCALLSTACK\n" \
                     "*FUNCTION [GladosPlayVcd()] choreo/glados.nut line [99]\n"
        result = self.evaluate(log)
        self.assertEqual(result["status"], "pass", result["failures"])
        self.assertEqual(result["map_script_errors"], ["AN ERROR HAS OCCURED [wrong number of parameters]"])

    def test_timeout_fails_even_with_complete_output(self):
        self.assertFailsWith(self.evaluate(timed_out=True), "timed out")

    def test_nonzero_exit_fails(self):
        self.assertFailsWith(self.evaluate(code=-11), "process exited with status -11")

    def test_empty_log_fails(self):
        result = self.evaluate("")
        self.assertEqual(len([f for f in result["failures"] if "required check" in f]), 2)


CONSOLE_SCENARIO = dict(SCENARIO, required_checks=["socket.fired", "socket.parented",
                                                   "beam.stops", "beam.gone"],
                        console_checks=[
                            {"name": "beam.stops", "window": "placed",
                             "select": r"^client laser \d+ reflector -1 ", "expect": r" hit \S*Cube$"},
                            {"name": "beam.gone", "window": "moved", "absent": r"reflector [0-9]"}])
CONSOLE_GOOD = GOOD + """QA_WINDOW placed BEGIN
client laser 35 reflector -1 segments 1 (7472.0 -5728.0 18.0) -> (7981.9 -5728.0 18.0) hit 18C_PropWeightedCube
client laser 221 reflector 11 segments 1 (8000.0 -5704.6 20.0) -> (8000.0 -5504.0 20.0) hit 7C_World
QA_WINDOW placed END
QA_WINDOW moved BEGIN
client laser 35 reflector -1 segments 1 (7472.0 -5728.0 18.0) -> (8320.0 -5728.0 18.0) hit 7C_World
QA_WINDOW moved END
client laser 221 reflector 11 segments 1 after the window
"""


class ConsoleCheckTests(unittest.TestCase):
    def evaluate(self, log):
        return scenarios.evaluate(CONSOLE_SCENARIO, log, 0, False)

    def assertFailsWith(self, result, text):
        self.assertEqual(result["status"], "fail")
        self.assertTrue(any(text in failure for failure in result["failures"]), result["failures"])

    def test_passing_windows(self):
        result = self.evaluate(CONSOLE_GOOD)
        self.assertEqual(result["status"], "pass", result["failures"])
        self.assertEqual(result["checks"]["beam.stops"]["detail"], "1 selected lines match")

    def test_selected_line_that_differs_fails(self):
        # The beam drawn through the cube before the client trace filter fix
        log = CONSOLE_GOOD.replace("(7981.9 -5728.0 18.0) hit 18C_PropWeightedCube",
                                   "(8320.0 -5728.0 18.0) hit 7C_World")
        self.assertFailsWith(self.evaluate(log), "beam.stops failed: 1 of 1 selected lines differ")

    def test_empty_selection_fails(self):
        log = CONSOLE_GOOD.replace("client laser 35 reflector -1 segments 1 (7472.0 -5728.0 18.0) -> "
                                   "(7981.9", "server laser (7981.9")
        self.assertFailsWith(self.evaluate(log), "beam.stops failed: no line in 2 matches")

    def test_absent_line_present_fails(self):
        log = CONSOLE_GOOD.replace("QA_WINDOW moved END", "client laser 221 reflector 11 x\nQA_WINDOW moved END")
        self.assertFailsWith(self.evaluate(log), "beam.gone failed: 1 lines match")

    def test_window_must_be_bracketed_once(self):
        for label, log in {
            "missing": CONSOLE_GOOD.replace("QA_WINDOW placed END\n", ""),
            "unopened": CONSOLE_GOOD.replace("QA_WINDOW moved BEGIN\n", ""),
            "repeated": CONSOLE_GOOD + "QA_WINDOW placed BEGIN\nQA_WINDOW placed END\n",
        }.items():
            with self.subTest(label):
                self.assertFailsWith(self.evaluate(log), "was not bracketed once")
        self.assertFailsWith(self.evaluate(GOOD), "beam.gone failed: window moved")

    def test_script_cannot_also_report_a_console_check(self):
        log = CONSOLE_GOOD.replace("QA_DONE demo checks=2", "QA_CHECK demo.beam.stops PASS\nQA_DONE demo checks=3")
        self.assertFailsWith(self.evaluate(log), "check reported twice: beam.stops")


ARRIVAL_SCENARIO = dict(SCENARIO, arrival={"map": "sp_next", "script": "next.nut"},
                        required_checks=["socket.fired", "socket.parented", "arrival.placed"])
ARRIVAL_GOOD = """QA_LOG demo t=1 start map=sp_demo steps=3
QA_CHECK demo.socket.fired PASS sphere (1 2 3)
QA_CHECK demo.socket.parented PASS parent=socket
QA_HANDOFF demo map=sp_demo checks=2 failures=0
---- Host_Changelevel ----
QA_LOG demo t=2 start map=sp_next steps=2
QA_CHECK demo.arrival.placed PASS in the elevator
QA_DONE demo checks=1 failures=0
"""


class ArrivalTests(unittest.TestCase):
    def evaluate(self, log, scenario=ARRIVAL_SCENARIO):
        return scenarios.evaluate(scenario, log, 0, False)

    def assertFailsWith(self, result, text):
        self.assertEqual(result["status"], "fail")
        self.assertTrue(any(text in failure for failure in result["failures"]), result["failures"])

    def test_handoff_and_arrival_pass(self):
        result = self.evaluate(ARRIVAL_GOOD)
        self.assertEqual(result["status"], "pass", result["failures"])

    def test_missing_handoff_fails(self):
        log = ARRIVAL_GOOD.replace("QA_HANDOFF demo map=sp_demo checks=2 failures=0\n", "")
        self.assertFailsWith(self.evaluate(log), "expected one QA_HANDOFF")

    def test_handoff_from_the_wrong_map_fails(self):
        log = ARRIVAL_GOOD.replace("QA_HANDOFF demo map=sp_demo", "QA_HANDOFF demo map=sp_next")
        self.assertFailsWith(self.evaluate(log), "QA_HANDOFF names")

    def test_handoff_counts_must_match_the_log(self):
        log = ARRIVAL_GOOD.replace("map=sp_demo checks=2", "map=sp_demo checks=1")
        self.assertFailsWith(self.evaluate(log), "QA_DONE counts 2/0 differ")

    def test_arrival_script_never_started_fails(self):
        # The level change failed: the arrival map's driver never ran.
        log = ARRIVAL_GOOD.split("---- Host_Changelevel ----")[0]
        result = self.evaluate(log)
        self.assertFailsWith(result, "driver started on sp_demo, expected sp_demo then sp_next")
        self.assertFailsWith(result, "expected one QA_DONE record")
        self.assertFailsWith(result, "required check not reported: arrival.placed")

    def test_reloaded_start_map_fails(self):
        log = ARRIVAL_GOOD.replace("start map=sp_next", "start map=sp_demo")
        self.assertFailsWith(self.evaluate(log), "driver started on sp_demo, sp_demo")

    def test_handoff_without_an_arrival_map_fails(self):
        log = GOOD.replace("QA_DONE", "QA_HANDOFF demo map=sp_demo checks=0 failures=0\nQA_DONE")
        self.assertFailsWith(self.evaluate(log, SCENARIO), "without an arrival map")

    def test_mapspawn_hook_is_rebuilt_and_restored(self):
        with tempfile.TemporaryDirectory() as directory:
            steam, runtime = Path(directory) / "steam", Path(directory) / "runtime"
            original = steam / "portal2/scripts/vscripts" / scenarios.MAPSPAWN
            original.parent.mkdir(parents=True)
            original.write_text('printl("==== calling mapspawn.nut")\n')
            hook = runtime / "portal2/scripts/vscripts" / scenarios.MAPSPAWN
            scenarios.install_mapspawn_hook(runtime, steam, ARRIVAL_SCENARIO)
            text = hook.read_text()
            self.assertTrue(text.startswith(original.read_text()))
            self.assertIn('GetMapName() == "sp_next"', text)
            self.assertIn('DoIncludeScript( \\"%s/next\\"' % scenarios.SCRIPT_DIRECTORY, text)
            scenarios.install_mapspawn_hook(runtime, steam, SCENARIO)
            self.assertEqual(hook.read_text(), original.read_text())


class WorkloadTests(unittest.TestCase):
    def write(self, directory, workload, scripts=("driver.nut", "demo.nut")):
        for script in scripts:
            (directory / script).write_text("// fixture\n")
        path = directory / "scenarios.json"
        path.write_text(json.dumps(workload))
        return path

    def workload(self, **changes):
        scenario = dict(SCENARIO, script="demo.nut")
        scenario.update(changes)
        return {"schema": scenarios.SCHEMA, "driver": "driver.nut", "scenarios": [scenario]}

    def test_installed_workload_is_valid(self):
        workload = scenarios.load_workload(ROOT / scenarios.DEFAULT_WORKLOAD)
        names = [scenario["name"] for scenario in workload["scenarios"]]
        self.assertEqual(sorted(names), ["sp_a1_intro5", "sp_a1_intro7", "sp_a1_wakeup", "sp_a2_core"])
        for scenario in workload["scenarios"]:
            # Every required check must be reported by its script.
            script = (ROOT / scenarios.DEFAULT_WORKLOAD).parent / scenario["script"]
            text = script.read_text()
            self.assertIn('QA_Start( "%s" )' % scenario["name"], text)
            for check in scenario["required_checks"]:
                self.assertIn('"%s"' % check, text, "%s does not report %s" % (script.name, check))

    def test_installed_transition_workload_is_valid(self):
        path = ROOT / "quality/workloads/portal2-transition-v1/scenarios.json"
        workload = scenarios.load_workload(path)
        for scenario in workload["scenarios"]:
            self.assertIn("arrival", scenario)
            start = (path.parent / scenario["script"]).read_text()
            arrival = (path.parent / scenario["arrival"]["script"]).read_text()
            for text in (start, arrival):
                self.assertIn('QA_Start( "%s" )' % scenario["name"], text)
            self.assertIn("QA_Handoff()", start)
            console = {check["name"] for check in scenario.get("console_checks", [])}
            for check in scenario["required_checks"]:
                if check not in console:
                    self.assertIn('"%s"' % check, start + arrival,
                                  "no script reports %s" % check)

    def test_rejects_bad_workloads(self):
        cases = {
            "schema": dict(self.workload(), schema="other"),
            "duplicate": dict(self.workload(), scenarios=[self.workload()["scenarios"][0]] * 2),
            "map": self.workload(map="../maps/x"),
            "timeout": self.workload(timeout_seconds=0),
            "checks": self.workload(required_checks=[]),
            "check name": self.workload(required_checks=["undotted"]),
            "script": self.workload(script="missing.nut"),
            "console name": self.workload(console_checks=[
                {"name": "other.check", "window": "w", "absent": "x"}]),
            "console window": self.workload(console_checks=[
                {"name": "socket.fired", "window": "a b", "absent": "x"}]),
            "console keys": self.workload(console_checks=[
                {"name": "socket.fired", "window": "w", "select": "x"}]),
            "console mixed": self.workload(console_checks=[
                {"name": "socket.fired", "window": "w", "select": "x", "expect": "y", "absent": "z"}]),
            "console pattern": self.workload(console_checks=[
                {"name": "socket.fired", "window": "w", "absent": "("}]),
            "console duplicate": self.workload(console_checks=[
                {"name": "socket.fired", "window": "w", "absent": "x"}] * 2),
        }
        for label, workload in cases.items():
            with self.subTest(label), tempfile.TemporaryDirectory() as directory:
                with self.assertRaises(scenarios.ScenarioError):
                    scenarios.load_workload(self.write(Path(directory), workload))

    def test_accepts_minimal_workload(self):
        with tempfile.TemporaryDirectory() as directory:
            workload = scenarios.load_workload(self.write(Path(directory), self.workload()))
            self.assertEqual(workload["scenarios"][0]["name"], "demo")


if __name__ == "__main__":
    unittest.main()
