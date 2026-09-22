# RFC 0002 progress: Hammer responsibility factorization

Updated: 2026-09-21
Source revision at assessment: `2d7e01d5` (working tree; AGENTS.md portfolio row: R08)

This file is the human-readable gate-decision record for RFC 0002. The machine
facts live in the versioned artifacts under `architecture/` and the conformance
tests under `unittests/hammertest/`; those artifacts are authoritative when they
disagree with prose here.

RFC 0002 is Proposed. Accepting it establishes the architecture and migration
protocol; it does not mark any delivery gate complete. This record opens Phase H0
and answers the open decisions the RFC deferred to measured H0/R1 results.

## Decisions (answers to the RFC's open questions)

These are recorded **Decisions** in the RFC's evidence vocabulary: intentional
targets with acceptance rules. They are revisable from H0/R1 measurements and are
not gate completions.

### D1 — Supported product profile (initial)

- **Primary portability target:** `linux-headless-core` (Linux x86_64, no display,
  no GPU). Geometry, scene, ports, formats, app, and viewport-query modules build
  and test here first. This matches the RFC's "Linux 64-bit headless core is the
  initial portability target."
- **Shell delivery target:** `linux-gtk-desktop` (GTK4 + libadwaita, X11 and
  Wayland), gated on H2–H4 and R1. Not promised for Windows/macOS in this program.
- **Rationale vs. goals:** AGENTS.md requires "a working, testable consumer at
  every new boundary" and portable code that "requests behavior, not OS/backend
  identity." A headless core is the only profile that can be tested without the
  unresolved renderer/host questions, so it anchors H1–H4.

Recorded in `architecture/hammer_compatibility.json` (profiles).

### D2 — Legacy build reproducibility (HAM-BUILD-001, honest H0 resolution)

- **Observed:** The historical Hammer editor is described by a Windows/MFC VPC
  project (`hammer/hammer_dll.vpc`) and depends on MFC (`afxwin.h`, `afxext.h`)
  through `hammer/stdafx.h`. There is **no `hammer/wscript`** and the root
  `wscript` does not define an editor target.
- **Decision:** The MFC/Windows build is **not reproducible on the Linux host**
  used for this program. We do not claim preserved legacy runtime behavior. The
  `windows-mfc-legacy` profile is marked `unverified`; it becomes a comparison
  target only if a working build is later established, with evidence linked here.
- **Consequence:** Per the RFC, independent extractions may proceed, but claims of
  preserved legacy runtime behavior remain gated. HAM-BUILD-001 stays `inventoried`
  / `blocked` until a Linux headless editor test target exists (depends on R03, the
  per-target C++20/toolchain boundary).

### D3 — Renderer bridge approach (R1 / HAM-RENDER-001)

- **Decision:** The R1 spike evaluates **approach 1 first** — execute the adapted
  Source renderer into the `GtkGLArea`-provided context/target during the permitted
  render callback — because it avoids a cross-context resource-sharing dependency
  for the first viewport. **Approach 2** (offscreen render + shared/transferred
  image resources) is the fallback if state/lifetime handling in the shared context
  proves unacceptable. The choice is recorded with measurements when R1 runs; a
  wireframe substitute does **not** pass the Source-material fidelity gate.
- **Blocked on:** the H0 minimal render contract and the pinned GTK/Wayland profile.

### D4 — Toolkit and language boundary (initial pins, finalized at H5/R03)

- **GTK/libadwaita minimums (provisional):** GTK ≥ 4.12, libadwaita ≥ 1.4. Pinned
  and tested exactly at H5; the RFC's toolkit references may describe newer APIs.
- **C++ dialect:** New strict editor modules (`public/hammer/**`, `hammer/core/**`)
  target C++20 per RFC 0006, with compiler/standard-library evidence established by
  R03/HAM-BUILD-001. Legacy MFC-facing headers keep their supported dialect; new
  language/library types must not cross an unchanged binary interface.
- **Content/fidelity/docking scope:** deferred to H5 with H0/R1 measurements, per
  the RFC. Recorded here when decided.

## Installed H0 machinery (this change)

Delivered as the first bounded H0 slice (HAM-INVENTORY-001 and HAM-RATCHET-001
foundations). All of it is executable and tested; none of it certifies a delivery
gate.

| Artifact | Purpose | Status |
| --- | --- | --- |
| `architecture/modules.json` → `hammerModules` | Editor module ownership + allowed dependency edges + strict include roots + native-token list | Installed; graph validated acyclic |
| `architecture/hammer_inventory.json` | Owned-file inventory with responsibility/effect/state/factorization/evidence and symbol splits | Installed; **coverage partial (13/452 files)** |
| `architecture/hammer_migrations.json` | Migration ledger (IDs, dependency DAG, status, contracts, test selectors, retirement conditions) | Installed; 8 seed migrations |
| `architecture/hammer_baseline.json` | Exact editor-rule (HAM003) violations; strict modules zero-debt | Installed; **empty** (no strict modules yet) |
| `architecture/hammer_compatibility.json` | Declared features/formats/profiles/limitations per gate | Installed; H0 declares no editor features |
| `tools/archlint/archlint.py` → `hammer` subcommand | Validates the four artifacts + module graph; runs HAM003 native-token scan over strict roots | Installed |
| `tools/archlint/tests/test_hammer.py` | Negative fixtures proving the validator rejects bad ownership, edges, cycles, statuses, and native tokens | 18 tests, passing |
| `unittests/hammertest/{contracts,fixtures}/` | Contract-suite and characterization-corpus scaffolding | Directory + provenance conventions only |

### Commands

```sh
python3 tools/archlint/archlint.py hammer --verify
python3 -m unittest discover -s tools/archlint/tests -v
```

The loader freeze commands are unchanged and remain authoritative for RFC 0001:

```sh
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
```

### Enforcement increments (what these checks do and do NOT prove)

Per the RFC's three-increment plan, only increment 1 is installed:

1. **Installed:** deterministic inventory/schema/DAG validation and the HAM003
   lexical native-token ratchet over strict roots.
2. **Not yet:** hermetic header builds and resolved target/link-graph checks
   (HAM001 ownership-in-build-graph, HAM002 forbidden edges via includes/link).
3. **Not yet:** compiler-grounded transitive include and symbol checks (HAM004
   ambient access, HAM005 transaction authority, HAM009 pick-ID narrowing).

HAM003 currently finds zero occurrences because the strict include roots do not
exist yet; the scan is proven live by `test_hammer.py`, not by a populated
baseline. No gate claims a stronger guarantee than these installed checks provide.

## Gate status

| Phase | State | Notes |
| --- | --- | --- |
| H0 | **active / partial** | Migration schema, module graph, ratchet increment 1, and corpus scaffolding installed and tested. Remaining for H0 exit: exhaustive inventory coverage, versioned semantic comparator with negative corpus, and the strict headless build target (HAM-BUILD-001). |
| H1–H7, R1 | planned | Blocked on H0 exit and, for R1, the render contract + GTK profile. |

## Pre-existing failures (reported separately, not introduced here)

- **RFC 0001 loader gate is currently failing** at the working tree: `check --all`
  and `baseline --verify` report **51 new and 31 stale** occurrences, and
  `inventory --verify` reports stale. The AGENTS.md snapshot at `87955f67` recorded
  10 new / 3 stale; HEAD (`2d7e01d5`) has drifted further. This is the R01/R07
  reconciliation item from AGENTS.md — an intentional review action, not a
  regression from RFC 0002 work. This change touches no scanned source and no
  loader manifest section, so it does not affect that count.

## Next dependency-ready migrations

1. **HAM-INVENTORY-001 (continue):** expand `hammer_inventory.json` toward
   exhaustive per-file/per-symbol coverage; flip `coverage.status` to `complete`
   only when every hammer source file resolves to one owner and a negative fixture
   proves unowned files fail.
2. **HAM-BUILD-001:** establish the Linux 64-bit headless editor test target and
   record the final C++20 flags (coordinated with R02/R03).
3. **HAM-CORPUS-001:** author the versioned semantic comparator and its negative
   corpus alongside the first VMF/geometry fixtures.
4. **HAM-GEOMETRY-001:** once a strict target exists, extract `BoundBox` into
   `hammer.geometry`, compile without the MFC PCH, and route callers through it.
