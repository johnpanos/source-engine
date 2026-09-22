from __future__ import annotations

import importlib.util
import sys
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).resolve().parents[1] / "roadmap.py"
SPEC = importlib.util.spec_from_file_location("roadmap", MODULE_PATH)
assert SPEC and SPEC.loader
roadmap = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = roadmap
SPEC.loader.exec_module(roadmap)


HEADER = "| Rank / ID | Work and RFC scope | Prerequisites | Done looks like | State |"
SEPARATOR = "| --- | --- | --- | --- | --- |"


def table(*rows: str) -> str:
    return "\n".join(["# doc", "", HEADER, SEPARATOR, *rows, "", "trailing prose"])


def row(rank: int, rid: str, prereqs: str, state: str, scope: str = "work", done: str = "done") -> str:
    return f"| {rank} / {rid} | {scope} | {prereqs} | {done} | {state} |"


class ParseTests(unittest.TestCase):
    def test_parses_ranks_ids_prereqs_and_state(self) -> None:
        parsed = roadmap.parse_roadmap(table(row(1, "R01", "—", "partial"), row(2, "R02", "R01", "planned")))
        self.assertEqual([1, 2], [task.rank for task in parsed.tasks])
        self.assertEqual((), parsed.by_id["R01"].prerequisites)
        self.assertEqual(("R01",), parsed.by_id["R02"].prerequisites)
        self.assertEqual("partial", parsed.by_id["R01"].state)

    def test_multiple_prerequisites_split_on_commas(self) -> None:
        parsed = roadmap.parse_roadmap(
            table(row(1, "R01", "—", "done"), row(2, "R02", "—", "done"), row(3, "R03", "R01, R02", "planned"))
        )
        self.assertEqual(("R01", "R02"), parsed.by_id["R03"].prerequisites)

    def test_missing_table_raises(self) -> None:
        with self.assertRaises(ValueError):
            roadmap.parse_roadmap("# doc\n\nno table here\n")

    def test_ends_at_first_non_row_line(self) -> None:
        parsed = roadmap.parse_roadmap(table(row(1, "R01", "—", "done")))
        self.assertEqual(1, len(parsed.tasks))


class ValidateTests(unittest.TestCase):
    def validate(self, *rows: str) -> tuple[list[str], list[str]]:
        return roadmap.validate(roadmap.parse_roadmap(table(*rows)))

    def test_consistent_roadmap_has_no_errors(self) -> None:
        errors, _ = self.validate(row(1, "R01", "—", "done"), row(2, "R02", "R01", "active"))
        self.assertEqual([], errors)

    def test_unknown_prerequisite_is_an_error(self) -> None:
        errors, _ = self.validate(row(1, "R01", "R99", "planned"))
        self.assertTrue(any("R99" in message for message in errors))

    def test_self_prerequisite_is_an_error(self) -> None:
        errors, _ = self.validate(row(1, "R01", "R01", "planned"))
        self.assertTrue(any("itself" in message for message in errors))

    def test_prerequisite_cycle_is_an_error(self) -> None:
        errors, _ = self.validate(row(1, "R01", "R02", "planned"), row(2, "R02", "R01", "planned"))
        self.assertTrue(any("cycle" in message for message in errors))

    def test_done_task_with_unfinished_prerequisite_is_an_error(self) -> None:
        # The hard-gate invariant: a completed task cannot depend on unfinished work.
        errors, _ = self.validate(row(1, "R01", "—", "active"), row(2, "R02", "R01", "done"))
        self.assertTrue(any("hard-gate" in message for message in errors))

    def test_duplicate_rank_is_an_error(self) -> None:
        errors, _ = self.validate(row(1, "R01", "—", "done"), row(1, "R02", "—", "done"))
        self.assertTrue(any("rank 1" in message for message in errors))

    def test_invalid_state_is_an_error(self) -> None:
        errors, _ = self.validate(row(1, "R01", "—", "almost"))
        self.assertTrue(any("state" in message for message in errors))

    def test_started_before_prerequisite_is_a_note_not_error(self) -> None:
        errors, notes = self.validate(row(1, "R01", "—", "planned"), row(2, "R02", "R01", "active"))
        self.assertEqual([], errors)
        self.assertTrue(any("R02" in note for note in notes))


class ReadinessTests(unittest.TestCase):
    def parsed(self, *rows: str) -> "roadmap.Roadmap":
        return roadmap.parse_roadmap(table(*rows))

    def test_no_prerequisites_is_gate_ready(self) -> None:
        rm = self.parsed(row(1, "R01", "—", "planned"))
        self.assertEqual(roadmap.GATE_READY, roadmap.readiness(rm, rm.by_id["R01"]))

    def test_all_prerequisites_done_is_gate_ready(self) -> None:
        rm = self.parsed(row(1, "R01", "—", "done"), row(2, "R02", "R01", "planned"))
        self.assertEqual(roadmap.GATE_READY, roadmap.readiness(rm, rm.by_id["R02"]))

    def test_prerequisite_underway_is_startable(self) -> None:
        rm = self.parsed(row(1, "R01", "—", "active"), row(2, "R02", "R01", "planned"))
        self.assertEqual(roadmap.STARTABLE, roadmap.readiness(rm, rm.by_id["R02"]))

    def test_prerequisite_not_started_is_blocked(self) -> None:
        rm = self.parsed(row(1, "R01", "—", "planned"), row(2, "R02", "R01", "planned"))
        self.assertEqual(roadmap.BLOCKED, roadmap.readiness(rm, rm.by_id["R02"]))

    def test_transitive_dependents_are_ranked(self) -> None:
        rm = self.parsed(
            row(1, "R01", "—", "done"),
            row(2, "R02", "R01", "planned"),
            row(3, "R03", "R02", "planned"),
        )
        self.assertEqual(["R02", "R03"], rm.transitive_dependents("R01"))


if __name__ == "__main__":
    unittest.main()
