# RFC 0002 progress: Hammer responsibility factorization

Updated: 2026-09-22
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
- **Decision (updated with Wine/Proton evidence):** Two distinct things were
  conflated in the original "not reproducible" claim, now separated:
  - **`windows-pe-core` — VERIFIED.** The strict, MFC-free core builds as native
    Windows PE (MinGW-w64 g++ 16.1.1, static) and all 19 conformance suites pass
    under **Wine 11 Staging and GE-Proton11-5**, matching the Linux gcc/clang runs.
    Harness `tools/quality/parity_wine.py`, profile
    `quality/profiles/windows-pe-wine.json`, evidence
    `architecture/hammer_windows_parity.json`. This is real Windows↔Linux parity
    for everything extracted so far.
  - **`windows-msvc-legacy-core` — VERIFIED.** The real MSVC v142 + MFC toolchain
    (cl 19.29.30159, toolset 14.29.30133 with `atlmfc`) is now provisioned via
    `mstorsjo/msvc-wine`. The **unmodified** legacy `hammer/boundbox.cpp` compiles
    with the real Microsoft compiler against the real `mathlib/vector.h`, links
    against the extracted `hammer::geometry::AxisAlignedBox`, and every observable
    operation is byte-identical — including the `SnapToGrid` `.5`-boundary that
    pins `V_rint`'s round-half-away-from-zero. A mutant re-introducing
    `std::rint` round-half-to-even is caught (exit 1), so the oracle is sound.
    This is stronger than the MinGW self-parity above: the extracted library
    reproduces the *actual legacy code compiled by the actual MSVC compiler*.
    Evidence `architecture/hammer_legacy_parity.json`; driver
    `scratchpad legacy-parity/run_legacy_parity.sh`.
  - **`windows-mfc-legacy` — still `unverified`.** With the toolchain provisioned,
    individual legacy translation units compile with the real MSVC+MFC compiler,
    but the full MFC *shell* (`afxwin.h`) still does not LINK here: it needs the
    whole Source Windows engine (tier0 / vgui2 / materialsystem / filesystem import
    libraries), which is out of scope for the extraction gate.
- **Consequence:** The strict extractions are verified on the Windows target on two
  toolchains (MinGW self-parity, and now real-MSVC-vs-legacy parity), not just
  Linux. Preserved legacy *runtime* behavior (the full MFC shell) remains gated on
  a full VS/MFC engine build.

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

### D5 — Sibling UI shells stand on a shared Source-value seam (HAM-SOURCEADAPTER-001)

- **Decision:** The "thin MFC/GTK UI as siblings" structure is realized by giving
  the reusable strict libraries their own value types (`hammer::geometry::Vec3`
  etc.) and marshalling Source engine types across a single named boundary owner,
  `hammer.adapters.source` (`hammer/adapters/source/vector_interop.h`). Both thin
  UI siblings — `hammer.adapters.mfc` and `hammer.adapters.gtk` — route Source
  geometry through this one seam; neither open-codes `Vector`↔`Vec3` copies (DRY).
- **Why the seam is non-strict:** `mathlib/vector.h` transitively pulls in the full
  `platform.h`/`threadtools.h` stack, which `#error`s outside a real Source build.
  The strict libraries therefore must not touch it; the adapter is the one place
  allowed to, so it compiles only on profiles that provide the Source platform
  layer (the engine build, or the MSVC/Wine parity lane), never on
  `linux-headless-core`. This is the architectural reason the strict modules own
  their own `Vec3`.
- **Proof (the substitution guarantee):** Under the **real MSVC toolchain (cl 19.29
  under Wine)**, for the unmodified legacy `BoundBox`, running a reusable geometry
  operation directly equals `ToAABB → op → FromAABB` for `SnapToGrid(16)` (incl. the
  `.5` boundary), `SnapToGrid(8)`, and `Rotate90` on every axis, with exact
  `Vector`↔`Vec3` round-trips. A deliberately-broken seam (y/z swap) is caught by 9
  clauses. Contract `adapters.source.vector_interop.v1`; suites
  `unittests/hammertest/adapters/test_vector_interop{,_negative}.cpp`; evidence
  `architecture/hammer_legacy_parity.json` (adapters section); driver
  `scratchpad legacy-parity/run_adapter_parity.sh`.
- **Consequence:** This is the enabling dependency for advancing HAM-GEOMETRY-001
  from `characterized` toward `cutover`: a live MFC-side `BoundBox` caller can now
  delegate geometry to the reusable library through the seam and get byte-identical
  results. The adapter is permanent sibling infrastructure, not a retirement target.

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
| `tools/archlint/archlint.py hammer --coverage` | Diffs the hammer source universe against the inventory; lists unclassified files with lexically-detected effects; warns when the authored total drifts | Installed (HAM-INVENTORY-001 aid) |
| `tools/archlint/archlint.py hammer --scaffold` | Emits schema-valid inventory stubs (`evidence: "hypothesis"`, with `reviewTODO`) for a batch of unclassified files; never mutates the artifact | Installed (HAM-INVENTORY-001 aid) |
| `tools/archlint/tests/test_hammer.py` | Negative fixtures proving the validator rejects bad ownership, edges, cycles, statuses, and native tokens; coverage/scaffold accuracy fixtures | 24 tests, passing |
| `unittests/hammertest/{contracts,fixtures}/` | Contract-suite and characterization-corpus scaffolding | Directory + provenance conventions only |

### Commands

```sh
python3 tools/archlint/archlint.py hammer --verify
python3 tools/archlint/archlint.py hammer --coverage            # what is still unclassified
python3 tools/archlint/archlint.py hammer --scaffold --path hammer/ --limit 25 > /tmp/stubs.json
python3 -m unittest discover -s tools/archlint/tests -v
```

`--coverage` and `--scaffold` assist HAM-INVENTORY-001: they enumerate the
source universe, report the gap, and emit reviewable stubs. Every scaffolded
field is a lexical hypothesis (`evidence: "hypothesis"`); an author must read the
file, correct the fields, drop `reviewTODO`, and re-evidence before merging.
Coverage flips to `complete` only by that human review, never by the tool.

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
| H0 | **active / partial** | Migration schema, module graph, ratchet increment 1, corpus scaffolding, and a **headless strict C++20 build+test target** (`unittests/hammertest/run_headless.sh`, HAM-BUILD-001) installed and passing under gcc 16 and clang. Remaining for H0 exit: exhaustive inventory coverage and the versioned semantic comparator with a negative corpus. |
| H1 | **active (geometry + scene seams)** | `hammer.geometry`: `AxisAlignedBox`, `RoundHalfAwayFromZero` (DRY grid-rounding owner), angle policies. `hammer.scene`: generational `HandleTable` (stale-reference rejection, independent documents) and `SceneGraph` (handle-addressed, validated/atomic reparent with cycle rejection, atomic subtree delete). All headless-verified under gcc + clang with negative providers. See below. |
| H3 | **partial (independent models)** | Ahead-of-authority reference models landed and pinned: `DocumentHistory` (revision vs saved-position, no-op neutrality, undo-to-saved clears modified), `PropertyValue` (empty-vs-unset-vs-mixed), and `UpdateHint` (HAM-UPDATEHINT-001) — the reusable core of MFC `CUpdateHint`, composing `hammer.geometry` + `hammer.scene` (notify-code buckets + unioned affected region, legacy `MAX_NOTIFY_CODES`=16 preserved; legacy source is orphaned dead code, so this is a reconstruction of intent). Not yet the live authority (needs H2 + legacy cutover). |
| H2 | **partial (codec + save seams)** | `hammer.formats`: VMF/keyvalues parser + writer + versioned semantic comparator (unknown-chunk preservation, malformed diagnostics, data-loss detection). `hammer.ports` + `hammer.app`: `IFileStore` port and `SaveDocument` transactional save (temp-write + atomic rename; failed save preserves prior file), verified with an in-memory fault-injecting fake. Remaining: VMF↔scene import/export, real file-store provider, fixtures corpus. |
| H4–H7, R1 | planned | Blocked on H0 exit and, for R1, the render contract + GTK profile. |

**Strict-module conformance suites** are registered in the shared RFC 0005 runner
(`quality/conformance.manifest.json`, `tools/quality/conformance.py`): the RFC 0002
Q-EDITOR suites (positive + sensitivity pairs), each proven to catch a seeded
regression, green under gcc and clang on `linux-headless-core` (23 matched on the
current manifest) and cross-verified as native Windows PE under Wine (23/23,
`architecture/hammer_windows_parity.json`). Inventory now classifies 40 files;
14 migrations tracked. (A UTF-8 decode crash in the shared
runner's git-evidence step was fixed here so the gate survives binary files in the
working tree — this also unblocked the RFC 0003 lane on the same runner.)

### First H1 extraction — `hammer.geometry` AABB (HAM-GEOMETRY-001)

The first real code extraction is landed as a working, testable consumer at a new
boundary:

- **Owner:** `public/hammer/geometry/aabb.h` + `hammer/core/geometry/aabb.cpp`
  define `hammer::geometry::AxisAlignedBox` (and an owned `Vec3`), a
  dependency-free port of legacy `hammer/BoundBox`. No MFC, no `tier0/platform.h`,
  no PCH, no GPU — it compiles on the `linux-headless-core` profile with only the
  C++ standard library, under **gcc 16 and clang** with `-Wall -Wextra -Werror`.
- **Oracle:** `unittests/hammertest/geometry/test_aabb.cpp` characterizes the
  BoundBox behavior (reset invalidity, point/box growth, open-face intersection vs
  closed-face containment, size-preserving grid snap, `Rotate90`). Run via
  `unittests/hammertest/run_headless.sh`.
- **Intentional difference (declared, not silent):** `SnapToGrid` rejects a
  non-positive grid size instead of dividing by it as the legacy `Snap()` did.
  Documented in the header and covered by the oracle; recorded in the migration
  ledger's `intentionalDifferences`.
- **Authority discipline:** legacy `BoundBox` remains the **sole live authority**;
  no dual mutable model exists. Status is `characterized`. Advancing to `extracted`
  requires routing `BoundBox` callers through the shared owner via a named
  `Vector`↔`Vec3` boundary adapter (`hammer.adapters.source`/`mfc`) — the next
  bounded step, which needs the Source `Vector` (and thus the platform config) on
  the legacy side of the seam.
- **Why this proves the toolchain question:** compiling any `tier0`/`mathlib`
  header headlessly requires the full Waf-generated Source platform configuration
  (`platform.h` hard-errors otherwise). A strict `hammer.geometry` that owns its
  own value type is therefore the only way to get a genuinely hermetic, portable,
  testable geometry module now — which is exactly the RFC's "None" dependency rule
  for that module.

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
   proves unowned files fail. Use `hammer --coverage` to see the remaining files
   and `hammer --scaffold` to draft stubs for review.
   - **Observed (tooling):** `hammer --coverage` measures the live source universe
     (files under `hammer/` plus the strict roots such as `public/hammer/`) and
     warns when it exceeds the authored `coverage.totalHammerSourceFiles` (still
     `452`). The tree has grown past that figure (new `hammer/*.cpp` and
     `public/hammer/geometry/aabb.h`); run the command for the current count and
     update the authored total as part of the next reviewed inventory write, not
     automatically.
2. **HAM-BUILD-001:** establish the Linux 64-bit headless editor test target and
   record the final C++20 flags (coordinated with R02/R03).
3. **HAM-CORPUS-001:** author the versioned semantic comparator and its negative
   corpus alongside the first VMF/geometry fixtures.
4. **HAM-GEOMETRY-001:** once a strict target exists, extract `BoundBox` into
   `hammer.geometry`, compile without the MFC PCH, and route callers through it.
