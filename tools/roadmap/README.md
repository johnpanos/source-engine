# roadmap

Navigator and consistency gate for the unified ranked roadmap in `AGENTS.md`.

The working protocol's first step is to *"choose the highest-ranked
dependency-ready bounded task."* This tool reads the roadmap table in `AGENTS.md`
(the single authority — it never mutates it or copies its facts) and answers that
question directly, while guarding against a mistyped prerequisite id, a
dependency cycle, or a `done` row that depends on unfinished work.

## Commands

```sh
python3 tools/roadmap/roadmap.py check         # validate the table (a gate; non-zero on error)
python3 tools/roadmap/roadmap.py ready         # ranked, actionable task set
python3 tools/roadmap/roadmap.py show R05      # one task: prerequisites + what it gates
python3 tools/roadmap/roadmap.py blocks R05    # everything R05 gates, transitively
python3 -m unittest discover -s tools/roadmap/tests -v
```

`ready`, `show`, and `blocks` accept `--json` for agent consumption. `ready`
accepts `--all` to include blocked tasks.

## Readiness definitions

- **gate-ready** — every prerequisite is `done`; the task can be *completed* now.
- **startable** — every prerequisite is at least underway (`partial`/`active`/`done`);
  baseline/feasibility work may begin, matching the roadmap's note that such work
  may precede a full gate.
- **blocked** — a prerequisite has not been started.

## What `check` enforces

Errors (exit 1): duplicate/malformed ids, unknown or self prerequisites,
dependency cycles, invalid states, and the hard-gate invariant that a `done` task
may not depend on an unfinished one. Notes (non-fatal): non-contiguous ranks
(child tasks may share a parent's band) and in-progress tasks whose prerequisites
have not been started (allowed only for baseline/feasibility work).
