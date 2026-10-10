# Working on Source Engine

The north star is a **job-based engine with Vulkan rendering, SDL3 platform
integration, and platform-compliant Linux, macOS, iOS and Android
backends**, modernized incrementally while preserving declared content,
gameplay, tool and binary compatibility, with a working, testable consumer
at every new boundary. The RFCs own semantics and acceptance; the roadmap
owns the cross-RFC order; `architecture/modules.json` owns dependency
permissions. RFC status is not implementation status: a proposed interface
or command is not installed until its record says so.

## Where the rules live

This file is loaded into every session; the rest are read when the work
touches them. They hold rules and one-line states, not history: evidence
lives in each row's progress record, and the full pre-split text is in the
[2026-10-10 archive](RFC/agents-archive-2026-10-10.md).

| File | Read it when |
| --- | --- |
| [docs/agents/project.md](docs/agents/project.md) | Always, at session start: the deliverable, milestones, who owns what, and your assigned task |
| [docs/agents/roadmap.md](docs/agents/roadmap.md) | Choosing or closing work: render priority, the ranked table, current facts |
| [docs/agents/architecture.md](docs/agents/architecture.md) | Changing a boundary, render code, C++ code or a harness: architecture rules, render binding rules, style, acceptance |
| [docs/agents/platforms.md](docs/agents/platforms.md) | Platform, toolchain, profile, packaging or distribution work |
| [docs/agents/rfcs.md](docs/agents/rfcs.md) | Finding the RFC that owns a concept |

Always in force, whatever the task:

- **Render work** follows [RFC 0016's binding rules](RFC/0016-render-core.md#binding-rules-for-all-render-work-user-decision-2026-09-28)
  (user decisions; only the user changes them). Device adapters are frozen
  and R89's scene path comes first ([2026-10-10](RFC/0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10)).
  Read [architecture.md](docs/agents/architecture.md#render-binding-rules-user-decisions-only-the-user-can-change-them)
  before any render change.
- **Ratchets only fall.** Never raise a ceiling, add a `pending` entry or
  update a golden or baseline to get green.
- **Delete the old copy** in the change that replaces it.
- **Keep these files short.** One line per table cell; detail goes in the
  domain progress record.

## Working protocol

No subagents (user decision, 2026-09-29): no subagents, forks, background
agents or multi-agent workflows. Messaging another session the user started
is allowed.

1. Take the highest-ranked dependency-ready task (`python3
   tools/roadmap/roadmap.py ready`) unless the user selects another scope.
   Read its RFC and current source first; preserve others' work.
2. Record observed facts separately from hypotheses and decisions.
3. Add the smallest trustworthy oracle, including failure and lifetime cases.
4. Introduce the seam and migrate a bounded caller cohort; one live authority.
5. Run the relevant installed checks; report pre-existing failures separately.
6. Delete obsolete callers when the retirement condition is met.
7. Update the row's state in [the roadmap](docs/agents/roadmap.md) in one line, and its evidence in the domain
   progress record, in the same change.

Products are built and launched only through `./kiln`; trees live in
`out/<profile>/<flavor>/`; harnesses launch through `kiln.api`
(`tools/kiln/sepipe_loader.py`).

```sh
./kiln profiles list
./kiln build <profile> [--flavor dev|release|ktx]
./kiln package <profile> [--runtime DIR]
./kiln play <profile> [map] [--set <switch>]... [--mounts <set>] [-- engine args]
./kiln switches <profile>
```

Architecture checks (repository root):

```sh
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
python3 -m unittest discover -s tools/archlint/tests -v
python3 tools/archlint/archlint.py structure --verify   # CAP012/CAP013
python3 tools/archlint/archlint.py structure --report
python3 tools/archlint/archlint.py graph [--family render|product|kiln] [--focus M --depth 2] [--format png|svg|json|dot|graphml|mermaid]
python3 tools/roadmap/roadmap.py check
```

- CAP012 counts only fall: record decreases with `structure --write`. CAP013
  ceilings grow only through `structure --raise AREA --reason TEXT`, a
  reviewed decision. Never raise a ceiling or re-adopt a rule to get green.
- On a change that adds, splits or moves a module, changes `allowedEdges` or
  a layer contract, regenerate the affected graph before and after and
  record what you saw (upward edges, new cycles or pending edges, `outside`
  modules, core modules gaining format or legacy edges, fan-in/out jumps).
- Style: `python3 tools/stylelint/stylelint.py --changed --diff` (local) or
  `--changed --base origin/master --diff` (branch); install the pinned
  formatter per [tools/stylelint/README.md](tools/stylelint/README.md).
- Check the Waf configuration before building; don't overwrite someone's
  build profile. Bound command output with `head`/`tail`/`grep`; save long
  logs to files.
