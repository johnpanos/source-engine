# RFC 0005 progress: Quality and correctness harnesses

Updated: 2026-09-21
Source revision at assessment: `95798d15` (working tree; AGENTS.md portfolio row: R02)

This file is the human-readable, durable gate-decision record for RFC 0005. The
machine facts live in the versioned artifacts (`quality/conformance.manifest.json`,
`quality/profiles/`) and the generated evidence under `quality-results/`; those
artifacts are authoritative when they disagree with prose here.

RFC 0005 is Proposed. This record opens the shared-infrastructure increments. It
marks **no** domain gate complete: installing a runner does not certify any
domain's correctness, and this document makes no such claim.

## Increment status (RFC 0005 §"Delivery increments")

| Increment | Deliverable | State | Evidence |
| --- | --- | --- | --- |
| Q0 | Baseline profiles and current-check audit | partial | AGENTS.md R01 notes; RFC 0002/0003 progress records |
| Q1 | Shared runner, fixtures, result schema | **partial (installed)** | This record + `tools/quality/` |
| Q2 | Q-ARCH and Q-FOUNDATION reference suites | planned | `tools/archlint`, `tools/stylelint` (R04-STYLE only) |
| Q3 | Domain oracles | partial | Q-EDITOR + Q-JOBS suites registered (below) |
| Q4 | Content and native integration | planned | — |
| Q5 | Promotion and maintenance | planned | — |

## Q1 — R02-RUNNER: shared conformance runner (installed, bounded)

`active`/`partial` for the bounded slice actually delivered: a dependency-free
host orchestrator, a single authoritative suite manifest, per-profile toolchain
facts, versioned evidence, and negative self-tests. It does **not** close R02:
there is no installed CI lane yet, and native/GPU/device profiles are absent.

### What is installed

- **Runner** — [`tools/quality/conformance.py`](../tools/quality/conformance.py):
  builds each declared suite with its profile's toolchain, runs it under a bounded
  timeout in subprocess isolation, classifies the outcome
  (`pass`/`fail`/`crash`/`timeout`/`compile-error`/`missing-source`), reconciles
  expected vs. executed suite ids, and writes `conformance-evidence/v1` JSON.
- **Authoritative manifest** —
  [`quality/conformance.manifest.json`](../quality/conformance.manifest.json)
  (`conformance-manifest/v1`): the single owner of which suites exist, their
  sources, contract, RFC/migration, `kind`, `profile`, and expected outcome.
- **Profile facts** —
  [`quality/profiles/linux-headless-core.json`](../quality/profiles/linux-headless-core.json)
  (`conformance-profile/v1`): C++ standard, flags, include roots, timeout.
- **Negative self-tests** — [`tools/quality/tests/`](../tools/quality/tests):
  18 tests over fixtures that pass, fail, crash, hang, fail to compile, go
  missing, plus zero-discovery / unmatched-selector / unknown-schema. These prove
  the runner detects each failure class before it is trusted (RFC 0005 §"Runner
  contract").

### Installed commands (verified working here)

```sh
python3 tools/quality/conformance.py list
python3 tools/quality/conformance.py check                 # all suites, writes evidence
python3 tools/quality/conformance.py check --rfc 0002 --domain Q-EDITOR
python3 -m unittest discover -s tools/quality/tests -v
unittests/hammertest/run_headless.sh                       # delegates to the runner
```

Local execution passed all self-tests and all registered suites under **g++
16.2.1** and **clang++ 22.1.8** on the `linux-headless-core` profile
(`-std=c++20 -Wall -Wextra -Werror`).

### Registered suites (Q3 domain oracles that already exist)

The authoritative, evolving list is the manifest itself — run
`python3 tools/quality/conformance.py list` (do not duplicate the mutable set
here). At the time of writing it covers **Q-EDITOR** (RFC 0002 Hammer geometry
and scene: AABB, rounding, angle, and scene handle-table contracts, each with a
`sensitivity` twin that a deliberately-broken provider must fail) and **Q-JOBS**
(RFC 0003 dependency-aware scheduler). Every registered suite currently matches
its expected `pass` outcome under both compilers on `linux-headless-core`; the
per-run detail (check counts, first divergence, repro) lives in the generated
`quality-results/` evidence, not here.

### Drift found and fixed while installing the runner

`unittests/hammertest/run_headless.sh` compiled `aabb.cpp` **without**
`rounding.cpp`, but `aabb.cpp` had gained a call to
`hammer::geometry::RoundHalfAwayFromZero` — so the AABB conformance suites no
longer linked and the script was silently broken. It also never built the newer
`test_rounding.cpp` suite. Both are exactly the "suite silently drops out of the
gate" failure the shared manifest prevents. `run_headless.sh` now delegates to
the runner, so the source lists live only in the manifest and cannot drift again.

### Honest non-claims

- This is Q1 infrastructure plus the already-existing Q-EDITOR/Q-JOBS suites. It
  does **not** certify RFC 0002, 0003, or any other domain gate.
- No required CI lane is installed; making any check required remains repository
  administrator policy. The runner is ready to be wired into a PR lane.
- Only the `linux-headless-core` profile exists. macOS/iOS/Android, native GPU,
  device, ABI, and product profiles are **unverified** and out of this slice.
- Q-ARCH/Q-FOUNDATION reference suites (Q2), the content/native and cross-domain
  scenarios (Q4), and performance/packaging/rollback promotion (Q5) are planned.
- Evidence in `quality-results/` is git-ignored and per-run; a linked old run
  cannot certify new code (RFC 0005 §"Evidence identity").

## Next steps

- Wire `python3 tools/quality/conformance.py check` into the PR lane alongside
  the archlint/stylelint checks once administrators opt in.
- Grow the manifest as each RFC lands a suite (Q-FOUNDATION lifecycle/loader/ABI,
  Q-CONTENT corpora, Q-PHYSICS IVP/Box3D comparisons, Q-PRESENTATION command
  suites). Each addition is one manifest row plus a domain contract + suite.
- Add native/device profiles (and their `conformance-profile/v1` files) as the
  R03 toolchain boundary and R14–R18 native seams provide runnable targets.
