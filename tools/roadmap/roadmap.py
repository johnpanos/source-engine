#!/usr/bin/env python3
"""Roadmap navigator and consistency gate for the AGENTS.md work order.

The unified ranked roadmap in ``AGENTS.md`` is the single authoritative
representation of portfolio priority, task state, and hard-gate prerequisites.
The working protocol's first step is to "choose the highest-ranked
dependency-ready bounded task"; doing that by hand means reading a 46-row table,
resolving every prerequisite, and cross-checking states with nothing guarding
against a mistyped prerequisite id, a dependency cycle, or a ``done`` row that
depends on unfinished work.

This tool parses that table (and only that table -- it never mutates it or keeps
a second copy of its facts), validates it, and answers the questions an agent
asks at the start of a session:

    roadmap.py check              structural + semantic validation (a gate)
    roadmap.py ready              the ranked, actionable task set
    roadmap.py show R05           one task, its prerequisites, and its dependents
    roadmap.py blocks R05         everything R05 gates, directly and transitively

``check`` exits non-zero on any error, so it can run in the same spirit as the
archlint ratchet.  The other commands accept ``--json`` for agent consumption.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Iterable, Sequence


VALID_STATES = ("planned", "partial", "active", "blocked", "done")
# States in which a prerequisite is considered "underway" -- enough for a
# dependent's feasibility/baseline work to begin, per the roadmap's note that
# "baseline captures and bounded feasibility experiments may precede a full
# implementation gate".  Only ``done`` satisfies a hard completion gate.
UNDERWAY_STATES = frozenset({"done", "active", "partial"})
DONE_STATE = "done"

HEADER_CELLS = ("Rank / ID", "Work and RFC scope", "Prerequisites", "Done looks like", "State")
RANK_ID_PATTERN = re.compile(r"^\s*(\d+)\s*/\s*(R\d+)\s*$")
ID_PATTERN = re.compile(r"^R\d+$")

# Readiness classification for a non-done task.
GATE_READY = "gate-ready"       # every prerequisite is done -> completable now
STARTABLE = "startable"         # every prerequisite is underway (>= partial)
BLOCKED = "blocked"             # some prerequisite has not been started
DONE = "done"                   # the task itself is complete


@dataclass(frozen=True)
class Task:
    rank: int
    id: str
    scope: str
    prerequisites: tuple[str, ...]
    done_looks_like: str
    state: str


@dataclass
class Roadmap:
    tasks: list[Task]
    by_id: dict[str, Task] = field(default_factory=dict)

    @classmethod
    def build(cls, tasks: Iterable[Task]) -> "Roadmap":
        ordered = list(tasks)
        return cls(tasks=ordered, by_id={task.id: task for task in ordered})

    def dependents(self, task_id: str) -> list[str]:
        """Direct dependents: tasks that name ``task_id`` as a prerequisite."""
        return [task.id for task in self.tasks if task_id in task.prerequisites]

    def transitive_dependents(self, task_id: str) -> list[str]:
        seen: set[str] = set()
        frontier = [task_id]
        while frontier:
            current = frontier.pop()
            for dependent in self.dependents(current):
                if dependent not in seen:
                    seen.add(dependent)
                    frontier.append(dependent)
        return sorted(seen, key=lambda tid: self.by_id[tid].rank)


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------


def repo_root_from_script() -> Path:
    return Path(__file__).resolve().parents[2]


def _split_row(line: str) -> list[str] | None:
    stripped = line.strip()
    if not stripped.startswith("|"):
        return None
    # Drop the leading and trailing pipe, then split.  Roadmap cells never
    # contain an unescaped pipe, matching the rest of the table tooling.
    inner = stripped.strip("|")
    return [cell.strip() for cell in inner.split("|")]


def _is_separator(cells: Sequence[str]) -> bool:
    return all(set(cell) <= {"-", ":"} and cell for cell in cells)


def parse_prerequisites(cell: str) -> tuple[str, ...]:
    text = cell.strip()
    if text in ("", "-", "—"):  # em dash means "no prerequisites"
        return ()
    parts = [part.strip() for part in text.split(",")]
    return tuple(part for part in parts if part)


def parse_roadmap(markdown: str) -> Roadmap:
    """Extract the ranked roadmap table.  Raises ValueError if it is absent."""
    lines = markdown.splitlines()
    header_index = None
    for index, line in enumerate(lines):
        cells = _split_row(line)
        if cells is not None and len(cells) == len(HEADER_CELLS) and tuple(cells) == HEADER_CELLS:
            header_index = index
            break
    if header_index is None:
        raise ValueError(
            "roadmap table not found: expected a header row "
            f"'| {' | '.join(HEADER_CELLS)} |' in the document"
        )

    tasks: list[Task] = []
    for line in lines[header_index + 1 :]:
        cells = _split_row(line)
        if cells is None:
            break  # table ended
        if _is_separator(cells):
            continue
        if len(cells) != len(HEADER_CELLS):
            raise ValueError(f"roadmap row has {len(cells)} cells, expected {len(HEADER_CELLS)}: {line.strip()}")
        rank_id, scope, prereqs, done, state = cells
        match = RANK_ID_PATTERN.match(rank_id)
        if not match:
            raise ValueError(f"roadmap row has malformed 'Rank / ID' cell: {rank_id!r}")
        tasks.append(
            Task(
                rank=int(match.group(1)),
                id=match.group(2),
                scope=scope,
                prerequisites=parse_prerequisites(prereqs),
                done_looks_like=done,
                state=state,
            )
        )
    if not tasks:
        raise ValueError("roadmap table header found but no task rows followed it")
    return Roadmap.build(tasks)


def load_roadmap(root: Path) -> Roadmap:
    return parse_roadmap((root / "AGENTS.md").read_text(encoding="utf-8"))


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------


def _find_cycle(edges: dict[str, list[str]]) -> list[str]:
    """Return a cyclic path through ``edges`` if one exists, else []."""
    color: dict[str, int] = {}
    stack: list[str] = []

    def visit(node: str) -> list[str]:
        color[node] = 1
        stack.append(node)
        for nxt in edges.get(node, []):
            if nxt not in edges:
                continue
            if color.get(nxt) == 1:
                return stack[stack.index(nxt) :] + [nxt]
            if color.get(nxt, 0) == 0:
                found = visit(nxt)
                if found:
                    return found
        color[node] = 2
        stack.pop()
        return []

    for node in edges:
        if color.get(node, 0) == 0:
            found = visit(node)
            if found:
                return found
    return []


def validate(roadmap: Roadmap) -> tuple[list[str], list[str]]:
    """Return (errors, notes).  Errors mean the roadmap is inconsistent."""
    errors: list[str] = []
    notes: list[str] = []
    tasks = roadmap.tasks

    seen_ids: set[str] = set()
    for task in tasks:
        if not ID_PATTERN.match(task.id):
            errors.append(f"{task.id}: id does not match the R<number> convention")
        if task.id in seen_ids:
            errors.append(f"{task.id}: id declared more than once")
        seen_ids.add(task.id)
        if task.state not in VALID_STATES:
            errors.append(
                f"{task.id}: state {task.state!r} is not one of {', '.join(VALID_STATES)}"
            )

    ranks = [task.rank for task in tasks]
    for rank in sorted({rank for rank in ranks if ranks.count(rank) > 1}):
        holders = ", ".join(task.id for task in tasks if task.rank == rank)
        errors.append(f"rank {rank} is shared by multiple rows: {holders}")
    expected = list(range(1, len(tasks) + 1))
    if sorted(ranks) != expected and len(set(ranks)) == len(ranks):
        notes.append(
            "ranks are not the contiguous sequence 1.."
            f"{len(tasks)} (child tasks may legitimately reuse a parent's band)"
        )

    edges: dict[str, list[str]] = {}
    for task in tasks:
        edges[task.id] = []
        for prereq in task.prerequisites:
            if prereq == task.id:
                errors.append(f"{task.id}: lists itself as a prerequisite")
                continue
            if prereq not in roadmap.by_id:
                errors.append(f"{task.id}: prerequisite {prereq!r} is not a known task id")
                continue
            edges[task.id].append(prereq)

    cycle = _find_cycle(edges)
    if cycle:
        errors.append("prerequisite cycle: " + " -> ".join(cycle))

    # Hard-gate invariant: a completed task cannot depend on unfinished work.
    for task in tasks:
        if task.state != DONE_STATE:
            continue
        for prereq in task.prerequisites:
            other = roadmap.by_id.get(prereq)
            if other is not None and other.state != DONE_STATE:
                errors.append(
                    f"{task.id} is '{DONE_STATE}' but prerequisite {prereq} is "
                    f"'{other.state}' (a hard-gate prerequisite must be done first)"
                )

    # Advisory: an in-progress task whose prerequisites have not been started.
    # The roadmap explicitly permits baseline/feasibility work to precede a
    # gate, so this is a note, not an error.
    for task in tasks:
        if task.state not in ("active", "partial"):
            continue
        not_started = [
            prereq
            for prereq in task.prerequisites
            if (other := roadmap.by_id.get(prereq)) is not None
            and other.state not in UNDERWAY_STATES
        ]
        if not_started:
            notes.append(
                f"{task.id} is '{task.state}' while prerequisite(s) "
                f"{', '.join(not_started)} have not been started "
                "(acceptable only for baseline/feasibility work)"
            )

    return errors, notes


# ---------------------------------------------------------------------------
# Readiness
# ---------------------------------------------------------------------------


def readiness(roadmap: Roadmap, task: Task) -> str:
    if task.state == DONE_STATE:
        return DONE
    prereq_tasks = [roadmap.by_id[p] for p in task.prerequisites if p in roadmap.by_id]
    if all(other.state == DONE_STATE for other in prereq_tasks):
        return GATE_READY
    if all(other.state in UNDERWAY_STATES for other in prereq_tasks):
        return STARTABLE
    return BLOCKED


def unmet_prerequisites(roadmap: Roadmap, task: Task) -> list[tuple[str, str]]:
    """Prerequisites of ``task`` that are not yet done, as (id, state) pairs."""
    result: list[tuple[str, str]] = []
    for prereq in task.prerequisites:
        other = roadmap.by_id.get(prereq)
        state = other.state if other is not None else "MISSING"
        if state != DONE_STATE:
            result.append((prereq, state))
    return result


def task_summary(roadmap: Roadmap, task: Task) -> dict:
    return {
        "rank": task.rank,
        "id": task.id,
        "state": task.state,
        "readiness": readiness(roadmap, task),
        "scope": task.scope,
        "prerequisites": list(task.prerequisites),
        "unmetPrerequisites": [
            {"id": pid, "state": state} for pid, state in unmet_prerequisites(roadmap, task)
        ],
        "directDependents": roadmap.dependents(task.id),
        "unblocksCount": len(roadmap.transitive_dependents(task.id)),
    }


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------


def check_command(roadmap: Roadmap) -> int:
    errors, notes = validate(roadmap)
    for note in notes:
        print(f"roadmap: note: {note}")
    for error in errors:
        print(f"roadmap: error: {error}")
    if errors:
        print(f"roadmap: failed with {len(errors)} error(s), {len(notes)} note(s)")
        return 1
    print(
        f"roadmap: {len(roadmap.tasks)} tasks are internally consistent "
        f"({len(notes)} note(s))"
    )
    return 0


def _readiness_label(value: str) -> str:
    return {
        GATE_READY: "READY   ",
        STARTABLE: "startable",
        BLOCKED: "blocked ",
        DONE: "done    ",
    }.get(value, value)


def ready_command(roadmap: Roadmap, as_json: bool, include_all: bool) -> int:
    rows = sorted(roadmap.tasks, key=lambda task: task.rank)
    actionable = [
        task
        for task in rows
        if include_all or readiness(roadmap, task) in (GATE_READY, STARTABLE) or task.state == "active"
    ]
    if as_json:
        print(json.dumps([task_summary(roadmap, task) for task in actionable], indent=2))
        return 0

    if not actionable:
        print("roadmap: no tasks are currently ready to start")
        return 0

    print("Actionable tasks (highest rank first). READY = every prerequisite is done;")
    print("startable = every prerequisite is at least underway.\n")
    for task in actionable:
        level = readiness(roadmap, task)
        marker = "*" if task.state == "active" else " "
        unblocks = len(roadmap.transitive_dependents(task.id))
        print(
            f"{marker} #{task.rank:<2} {task.id}  [{_readiness_label(level)}] "
            f"state={task.state:<8} unblocks={unblocks}"
        )
        print(f"       {task.scope}")
        unmet = unmet_prerequisites(roadmap, task)
        if unmet:
            rendered = ", ".join(f"{pid}({state})" for pid, state in unmet)
            print(f"       waiting on: {rendered}")
    print("\n(* = already active)")
    return 0


def show_command(roadmap: Roadmap, task_id: str, as_json: bool) -> int:
    task = roadmap.by_id.get(task_id)
    if task is None:
        print(f"roadmap: unknown task {task_id!r}")
        return 2
    if as_json:
        print(json.dumps(task_summary(roadmap, task), indent=2))
        return 0
    print(f"#{task.rank} {task.id}  state={task.state}  readiness={readiness(roadmap, task)}")
    print(f"  scope:  {task.scope}")
    print(f"  done:   {task.done_looks_like}")
    if task.prerequisites:
        print("  prerequisites:")
        for prereq in task.prerequisites:
            other = roadmap.by_id.get(prereq)
            state = other.state if other else "MISSING"
            print(f"    {prereq:<5} {state}")
    else:
        print("  prerequisites: none")
    dependents = roadmap.dependents(task.id)
    if dependents:
        print(f"  directly gates: {', '.join(dependents)}")
        transitive = roadmap.transitive_dependents(task.id)
        print(f"  unblocks (transitive): {len(transitive)} task(s): {', '.join(transitive)}")
    else:
        print("  directly gates: nothing")
    return 0


def blocks_command(roadmap: Roadmap, task_id: str, as_json: bool) -> int:
    if task_id not in roadmap.by_id:
        print(f"roadmap: unknown task {task_id!r}")
        return 2
    transitive = roadmap.transitive_dependents(task_id)
    if as_json:
        print(json.dumps({"id": task_id, "unblocks": transitive}, indent=2))
        return 0
    direct = set(roadmap.dependents(task_id))
    if not transitive:
        print(f"{task_id} gates no other tasks")
        return 0
    print(f"{task_id} gates {len(transitive)} task(s):")
    for tid in transitive:
        kind = "direct" if tid in direct else "transitive"
        print(f"  {tid:<5} #{roadmap.by_id[tid].rank:<2} ({kind}) state={roadmap.by_id[tid].state}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", type=Path, default=None, help="repository root (defaults to this checkout)")
    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("check", help="validate roadmap consistency (a gate; non-zero on error)")

    ready = subparsers.add_parser("ready", help="list the ranked, actionable task set")
    ready.add_argument("--json", action="store_true", help="machine-readable output")
    ready.add_argument("--all", action="store_true", help="include blocked tasks too")

    show = subparsers.add_parser("show", help="show one task with prerequisites and dependents")
    show.add_argument("task_id")
    show.add_argument("--json", action="store_true")

    blocks = subparsers.add_parser("blocks", help="list everything a task gates, transitively")
    blocks.add_argument("task_id")
    blocks.add_argument("--json", action="store_true")

    return parser


def main(argv: Sequence[str] | None = None, root: Path | None = None) -> int:
    args = build_parser().parse_args(argv)
    root = root or args.root or repo_root_from_script()
    try:
        roadmap = load_roadmap(root)
    except (OSError, ValueError) as error:
        print(f"roadmap: {error}", file=sys.stderr)
        return 2

    if args.command == "check":
        return check_command(roadmap)
    if args.command == "ready":
        return ready_command(roadmap, args.json, args.all)
    if args.command == "show":
        return show_command(roadmap, args.task_id.upper(), args.json)
    if args.command == "blocks":
        return blocks_command(roadmap, args.task_id.upper(), args.json)
    raise AssertionError(f"unhandled command {args.command!r}")


if __name__ == "__main__":
    sys.exit(main())
