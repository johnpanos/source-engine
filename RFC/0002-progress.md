# RFC 0002 progress: Hammer responsibility factorization

Updated: 2026-09-25
Source revision at assessment: `2d7e01d5` (working tree; AGENTS.md portfolio row: R08).
Current-state review: `d6260d90` (2026-09-25); see
[Current state](#current-state-2026-09-25).

This file is the human-readable gate-decision record for RFC 0002. The machine
facts live in the versioned artifacts under `architecture/` and the conformance
tests under `unittests/hammertest/`; those artifacts are authoritative when they
disagree with prose here.

RFC 0002 is Proposed. Accepting it establishes the architecture and migration
protocol; it does not mark any delivery gate complete. This record opens Phase H0
and answers the open decisions the RFC deferred to measured H0/R1 results.

## Current state (2026-09-25)

Checked at `d6260d90` against the machine artifacts. The dated sections below
are history.

- AGENTS.md row R08 is `active`. No RFC 0002 delivery gate is complete.
  `tools/roadmap/roadmap.py show` reports R08 and R17 startable. R13, R22–R25,
  R33 and R43 are `planned` and blocked on prerequisites.
- **`archlint hammer --verify` passes again (2026-09-25).** From `7035c29e`
  (2026-09-22, the RFC 0007 R47 PBR schema) it rejected the `hammer.formats`
  → `render.contracts` edge, because the Hammer graph validator knew only
  Hammer modules. By user decision in the R01 re-audit, the validator now
  accepts a Hammer module's edge to a module registered in
  `capabilityModules`. It still rejects unknown targets and cycles, and it
  rejects a Hammer module that reuses a capability module's id (3 new tests
  in `tools/archlint/tests/test_hammer.py`). The inventory, ledger,
  compatibility and HAM003 checks pass.
- Strict sources exist for `hammer.geometry` (5), `hammer.scene` (2),
  `hammer.formats` (16) and `hammer.app` (7), plus three `hammer.ports`
  headers. `hammer.viewport`, `hammer.tools` and `hammer.presenters` are
  declared but empty; the interactive tools live in `hammer.app`
  (`EditorController`).
- Ledger: 28 migrations. 11 are `extracted`, 11 `characterized`, 2
  `substitutable`, 2 `isolated` and 2 `inventoried`; none is `cutover` or
  `retired`. Legacy Hammer is still the live authority for every legacy caller.
- Inventory: 46 authored records. `hammer --coverage` reports 46 of 530 files
  classified; the authored total is still 452.
- Conformance: 60 Q-EDITOR suites are registered
  (`conformance.py plan --rfc 0002 --domain Q-EDITOR`), all on `checks-v1`.
  The last recorded full run is the 2026-09-22 R02 evidence: g++ and clang++,
  default and release, and Wine PE 60/60
  ([RFC 0005 Q1](0005-progress.md#q1--r02-runner-shared-conformance-runner)).
  This review did not rerun them.
- 31 contract records exist under `unittests/hammertest/contracts/`.
  `unittests/hammertest/fixtures/` holds only its README; the sample maps are
  `hammer/gtk/samples/{room,wedge,displacement}.vmf`.
- Build: there is still no `hammer/wscript`. The first Waf target that compiles
  strict Hammer sources is `hammer_ktx2_preview_conformance`
  (`unittests/texturecontainertest/wscript`, RFC 0008 F3). The GTK shell is
  still built by `hammer/gtk/build.sh`.
- Work this record did not yet describe is summarized in
  [Additional work recorded 2026-09-25](#additional-work-recorded-2026-09-25).

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
- **2026-09-25:** the engine render seam is `done` (R15, `render.profile.v1`), and
  so are the presentation bridges (R16, `render.presentation.v1`). Neither has
  a GTK host pair. HAM-RENDER-001 is still `inventoried`, and the contract IDs
  it names (`render.baseline.v1`, `host.surface.v1`) exist nowhere else in the
  tree. The GTK shell's GL viewport is a substitute renderer, so it does not
  count toward this decision.

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

### D6 — A real thin GTK sibling + a real file-store provider (HAM-GTKSHELL-001, HAM-DISKSTORE-001)

- **Decision:** Realize the "thin MFC/GTK UI as siblings" structure concretely by
  (a) giving the reusable app libraries their first **production** `IFileStore`
  provider, `hammer::adapters::platform::DiskFileStore` (atomic temp-write+rename),
  and (b) standing up an actual **thin GTK4/libadwaita shell**
  (`hammer/adapters/gtk/hammer_gtk_shell.cpp`) whose every action delegates to
  `hammer::app::EditorDocument` persisted through that store. The shell holds only
  toolkit glue and view state; no editor policy.
- **Verified here:**
  - `IFileStore` is now an enforced polymorphic contract (`ports.file_store.v1`):
    one shared suite runs against the real `DiskFileStore`, the in-memory fake, and
    a deliberately broken provider (caught) — green on gcc, clang, and MinGW PE
    under Wine.
  - The GTK sibling **compiles and links to a real ELF binary** against gtk4 4.22 +
    libadwaita 1.9 + the reusable libraries and nothing else
    (`unittests/hammertest/adapters/build_gtk_shell.sh`), proving it carries no
    hidden editor logic and no MFC dependency.
  - The shell's delegated workflow — New → Set Key → Save → reopen → Undo/Redo — is
    verified headlessly with the real disk store (`adapters.gtk_shell.v1`,
    `test_shell_workflow.cpp`), since a GUI needs a display (none here).
- **Honest gaps:** the GUI is not run here (no headless display); full H5 editor
  parity (multi-view map rendering, inspector, textures, the R1 renderer viewport)
  is not built; and the MFC sibling is not built (needs the Source Windows engine,
  D2). This is the sibling architecture proven end-to-end for **one** buildable
  shell, not a shipped editor. Profile `linux-gtk-desktop` is now `unverified`
  (was `planned`) to reflect the buildable-but-not-run status.

### D7 — The MFC sibling too (HAM-MFCSHELL-001) — the D2 legacy-shell caveat does not block a NEW thin MFC shell

- **Correction to earlier framing:** D2's "the full MFC shell needs the whole Source
  Windows engine" is true of the **legacy** Hammer shell (`CMapDoc`/`CMapView`), not
  of a **new** thin MFC sibling. A new sibling — parallel to the GTK one — depends
  only on MFC + the reusable, engine-free libraries, so it builds against the
  extracted libraries alone.
- **Verified here (local MSVC+MFC/Wine gate):**
  - `hammer/adapters/mfc/hammer_mfc_shell.cpp` (a `CWinApp`/`CFrameWnd` whose command
    handlers delegate to `EditorDocument` + `DiskFileStore`) **compiles and links to
    a real PE32+ GUI executable** with the real MSVC+MFC toolchain (cl 19.29,
    atlmfc, shared `mfc140.dll`) against MFC + the reusable library sources and
    nothing else.
  - `test_mfc_workflow.cpp` — a console **MFC-linked** binary — **runs under Wine**
    and exercises New → Set Key → Save → reopen → Undo/Redo through
    `EditorDocument` + the real `DiskFileStore` (exit 0), proving MFC and the
    reusable engine-free libraries coexist in one binary and the delegated workflow
    executes at runtime.
  - Contract `adapters.mfc_shell.v1`; evidence `architecture/hammer_mfc_parity.json`;
    repro `unittests/hammertest/adapters/build_mfc_shell.sh`; profile
    `windows-mfc-sibling` = `verified` (distinct from `windows-mfc-legacy`).
- **Both siblings now build on the same reusable libraries** — `adapters.gtk`
  (GTK4/libadwaita) and `adapters.mfc` (MFC) — persisting through the same
  `DiskFileStore` and serializing via the same `hammer::formats` codec, with only
  toolkit glue per sibling (DRY). That is the "thin MFC/GTK UI as siblings" shape
  realized for both shells.
- **Honest gaps (unchanged):** neither GUI is run here (no display); full H5 editor
  parity (map rendering, inspector, textures, R1 viewport) is not built; the
  **legacy** MFC shell (`windows-mfc-legacy`) is still unverified (needs the whole
  Source engine). This proves the factored architecture on both UI sides, not a
  shipped editor.

### D8 — A shared entity editor: both siblings drive one selection + property-editing authority (HAM-SEL-001)

- **Gap this closes:** D6/D7 proved the GTK and MFC shells are thin siblings over
  the same `EditorDocument`, but the only editing verb was `SetFirstBlockKey` — a
  placeholder that edits the first block. The RFC's multi-selection property model
  (`PropertyValue`, empty-vs-unset-vs-mixed) existed as an **orphaned oracle**: no
  method produced it from a live selection, so any real entity editing would have
  been re-derived independently in each shell. That is exactly the duplication the
  factoring goal forbids.
- **Decision / landed:** the shared **entity editor** now lives in `hammer.app` as
  the single authority both siblings route through:
  - `hammer::app::EntitySelection` (`public/hammer/app/entity_selection.h` +
    `hammer/core/app/entity_selection.cpp`): the ordered, deduplicated set of entity
    indices a shell owns as view state. No editor policy.
  - `EditorDocument::EntityCount/EntityName/EntityClassName` enumerate the document's
    top-level entities; `AggregateProperty(selection, key)` folds a property across
    the selection into `PropertyValue` under an explicit **presence-only** policy
    (a non-contributor never forces `Mixed`; a `""` value stays `Single("")`, not
    `Unset`); `SetPropertyOnSelection(selection, key, value)` applies a property to
    every selected entity as **one atomic history unit** — a single `Undo` reverts
    the whole group.
  - Contract `app.entity_editor.v1`; suites `hammer.app.entity_editor` (+`.sensitivity`,
    a non-atomic per-entity editor the oracle catches), green under gcc and clang on
    `linux-headless-core` via `run_headless.sh`.
- **The GTK sibling now drives it end to end.** `hammer/adapters/gtk/hammer_gtk_shell.cpp`
  gained a multi-select entity list; selection changes call `AggregateProperty` to
  show the value (with `<multiple values>` / `(unset)` states), and "Set Key" calls
  `SetPropertyOnSelection` — no editor policy in the shell. It still builds to a real
  ELF against gtk4 4.22 + libadwaita 1.9 + the reusable libraries alone
  (`build_gtk_shell.sh`), and the delegated workflow (New → select both entities →
  Set Key as one atomic edit → Save → reopen → Undo/Redo) is verified headlessly by
  `test_shell_workflow.cpp` (no display needed). So tested logic == shipped logic.
- **Why this is the factoring the goal asks for:** selection, multi-selection
  aggregation, and atomic multi-entity editing are now defined **once** and consumed
  by both the GTK sibling (D6) and the MFC sibling (D7) through the same
  `EditorDocument` — neither re-derives them. The empty-vs-unset distinction can no
  longer be silently lost in a shell.
- **Honest gaps:** the MFC sibling's `test_mfc_workflow.cpp` still exercises the
  older single-target `SetFirstBlockKey` verb (routing it through the new entity
  path is the concurrent MFC owner's follow-up); entities here are addressed by
  index, not yet scene `NodeHandle`s; the property-draft commit policy
  (commit-valid/report-invalid/discard) and legacy `CSelection`/property-sheet
  cutover remain (they need the legacy build). Status stays `characterized`: legacy
  Hammer is still the live authority. See `HAM-SEL-001`.

## Installed H0 machinery (this change)

Delivered as the first bounded H0 slice (HAM-INVENTORY-001 and HAM-RATCHET-001
foundations). All of it is executable and tested; none of it certifies a delivery
gate.

| Artifact | Purpose | Status |
| --- | --- | --- |
| `architecture/modules.json` → `hammerModules` | Editor module ownership + allowed dependency edges + strict include roots + native-token list | Installed; graph validated acyclic |
| `architecture/hammer_inventory.json` | Owned-file inventory with responsibility/effect/state/factorization/evidence and symbol splits | Installed; **coverage partial** (13/452 at install; 46 records, 46/530 files on 2026-09-25) |
| `architecture/hammer_migrations.json` | Migration ledger (IDs, dependency DAG, status, contracts, test selectors, retirement conditions) | Installed; 8 seed migrations (28 on 2026-09-25) |
| `architecture/hammer_baseline.json` | Exact editor-rule (HAM003) violations; strict modules zero-debt | Installed; **empty** (strict roots populated since; still zero entries on 2026-09-25) |
| `architecture/hammer_compatibility.json` | Declared features/formats/profiles/limitations per gate | Installed; H0 declares no editor features |
| `tools/archlint/archlint.py` → `hammer` subcommand | Validates the four artifacts + module graph; runs HAM003 native-token scan over strict roots | Installed |
| `tools/archlint/archlint.py hammer --coverage` | Diffs the hammer source universe against the inventory; lists unclassified files with lexically-detected effects; warns when the authored total drifts | Installed (HAM-INVENTORY-001 aid) |
| `tools/archlint/archlint.py hammer --scaffold` | Emits schema-valid inventory stubs (`evidence: "hypothesis"`, with `reviewTODO`) for a batch of unclassified files; never mutates the artifact | Installed (HAM-INVENTORY-001 aid) |
| `tools/archlint/tests/test_hammer.py` | Negative fixtures proving the validator rejects bad ownership, edges, cycles, statuses, and native tokens; coverage/scaffold accuracy fixtures | 24 tests, passing (rerun 2026-09-25) |
| `unittests/hammertest/{contracts,fixtures}/` | Contract-suite and characterization-corpus scaffolding | Conventions at install; 31 contract records on 2026-09-25, `fixtures/` still README only |

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
2. **Partly installed (2026-09-25):** HAM002's include edges and cycle check
   over the strict roots (see the resolution below). Not yet: hermetic header
   builds for Hammer, and resolved target/link-graph checks (HAM001
   ownership-in-build-graph, HAM002 via the link graph).
3. **Not yet:** compiler-grounded transitive include and symbol checks (HAM004
   ambient access, HAM005 transaction authority, HAM009 pick-ID narrowing).

HAM003 currently finds zero occurrences because the strict include roots do not
exist yet; the scan is proven live by `test_hammer.py`, not by a populated
baseline. No gate claims a stronger guarantee than these installed checks provide.

2026-09-25: the roots `public/hammer/` and `hammer/core/` now hold strict
sources, and HAM003 still finds zero occurrences. Because increment 2 is not
installed, the strict sources already use edges `modules.json` does not allow,
and nothing fails:

- `hammer/geometry/brush.h` and `displacement.h` include
  `hammer/formats/keyvalues.h`. `hammer.geometry` declares no edges, and
  `hammer.formats` → `hammer.geometry` is allowed, so this is a
  geometry↔formats cycle at the include level.
- `hammer/app/editor_document.h` and `hammer/core/app/editor_controller.cpp`
  include `hammer/formats/keyvalues.h`; `hammer.app` does not allow
  `hammer.formats`.

Either the graph or the includes must change before increment 2 can pass.

**Resolved 2026-09-25 (R04-HAMGRAPH).** Increment 2's include check is
installed as HAM002 (forbidden module edge or architectural cycle, RFC 0002's rule table) in `archlint hammer --verify`. Each strict file
(`hammer/core/<area>/`, `public/hammer/<area>/`) belongs to `hammer.<area>`,
and its includes of another Hammer area or of a registered capability module
must be allowed edges. The module include graph the sources really form must
be acyclic; no exception can excuse a cycle. A deviation needs an exact-count
`hammerModules.includeExceptions` entry with an owner row, a tracking record
and a removal condition. Stale or miscounted entries fail. There are
7 fixtures, and three mutants (cycle, edge, stale exception) are each
detected.

- **The cycle is gone.** The VMF decoders `BuildSolidFromBlock`,
  `BuildSceneFromDocument` and `ParseDispInfo` moved from `hammer.geometry`
  into `hammer.formats` (`public/hammer/formats/vmf_geometry.h`,
  `hammer/core/formats/vmf_geometry.cpp`, namespace `hammer::formats`) with
  unchanged bodies. Geometry exposes the key-value-free helpers they need:
  `ParseVec3`, `FindFaceOnPlane`, `WorldScene::AddSolid` and
  `WorldScene::AddDisplacement`. `EditorController::BuildScene` now uses the
  last two, so the scene-bounds rule has one owner.
- **App → formats is a recorded exception.** `editor_document.h` and
  `editor_controller.cpp` hold one include each, owned by R22. The removal
  condition is that the document content and the controller's VMF
  read/write go through a `hammer.ports` persistence contract that formats
  implements, as RFC 0002 requires.
- **Evidence.**
  - Q-EDITOR 60/60 with g++ and with clang++, 1,570 checks each, the same
    counts as before the move.
  - The Wine parity run 60/60.
  - `build.tools` and `hammer.gtk-viewport-smoke` pass.
  - `archlint hammer --verify` passes (104 archlint tests).
  - The static audit shows 0 deviations.

## Gate status

| Phase | State | Notes |
| --- | --- | --- |
| H0 | **active / partial** | Migration schema, module graph, ratchet increment 1, corpus scaffolding, and a **headless strict C++20 build+test target** (`unittests/hammertest/run_headless.sh`, HAM-BUILD-001) installed and passing under gcc 16 and clang. Remaining for H0 exit: exhaustive inventory coverage and the versioned semantic comparator with a negative corpus. 2026-09-25: the comparator and its negative case exist (`CompareKeyValues`, HAM-CORPUS-001); still open are inventory coverage (46/530), the RFC minimum fixture corpus, a Waf/CI-integrated target with R03 flags, and the failing `archlint hammer --verify` ([current state](#current-state-2026-09-25)). |
| H1 | **active (geometry + scene seams)** | `hammer.geometry`: `AxisAlignedBox`, `RoundHalfAwayFromZero` (DRY grid-rounding owner), angle policies. `hammer.scene`: generational `HandleTable` (stale-reference rejection, independent documents) and `SceneGraph` (handle-addressed, validated/atomic reparent with cycle rejection, atomic subtree delete). All headless-verified under gcc + clang with negative providers. See below. |
| H3 | **partial (independent models)** | Ahead-of-authority reference models landed and pinned: `DocumentHistory` (revision vs saved-position, no-op neutrality, undo-to-saved clears modified), `PropertyValue` (empty-vs-unset-vs-mixed), and `UpdateHint` (HAM-UPDATEHINT-001) — the reusable core of MFC `CUpdateHint`, composing `hammer.geometry` + `hammer.scene` (notify-code buckets + unioned affected region, legacy `MAX_NOTIFY_CODES`=16 preserved; legacy source is orphaned dead code, so this is a reconstruction of intent). Not yet the live authority (needs H2 + legacy cutover). The **shared entity editor** (`EntitySelection` + `EditorDocument` entity methods, HAM-SEL-001/D8) now wires `PropertyValue` into live multi-selection editing with atomic multi-entity undo, and the GTK sibling drives it end to end. |
| H2 | **partial (codec + save seams)** | `hammer.formats`: VMF/keyvalues parser + writer + versioned semantic comparator (unknown-chunk preservation, malformed diagnostics, data-loss detection). `hammer.ports` + `hammer.app`: `IFileStore` port and `SaveDocument` transactional save (temp-write + atomic rename; failed save preserves prior file), verified with an in-memory fault-injecting fake. Remaining: VMF↔scene import/export, real file-store provider, fixtures corpus. 2026-09-25: `DiskFileStore` is the real provider (D6); `EditorController::LoadVmf`/`ToVmf` round-trip world brushes, per-face materials and point entities, but not brush entities or texture axes, and not through `hammer.scene` handles; 11 format/geometry cores are `extracted` ([additional work](#additional-work-recorded-2026-09-25)). Detached import, compile acceptance and the fixture corpus remain. |
| H4–H7, R1 | planned | Blocked on H0 exit and, for R1, the render contract + GTK profile. 2026-09-25: R15 and R16 are `done` and `roadmap.py` reports R17 (R1) startable; HAM-RENDER-001 is still `inventoried` and no Source-material GTK viewport exists. A bounded `linux-gtk-desktop` **boot + VMF-render feasibility slice** now exists (below): it is feasibility evidence, not R1/H5 completion — no editing tools, live-document authority, or material fidelity yet. |

**Strict-module conformance suites** are registered in the shared RFC 0005 runner
(`quality/conformance.manifest.json`, `tools/quality/conformance.py`): the RFC 0002
Q-EDITOR suites (positive + sensitivity pairs), each proven to catch a seeded
regression, green under gcc and clang on `linux-headless-core` (23 matched on the
current manifest) and cross-verified as native Windows PE under Wine (23/23,
`architecture/hammer_windows_parity.json`). Inventory now classifies 40 files;
14 migrations tracked. (A UTF-8 decode crash in the shared
runner's git-evidence step was fixed here so the gate survives binary files in the
working tree — this also unblocked the RFC 0003 lane on the same runner.)
2026-09-25: the manifest now registers 60 Q-EDITOR suites. The committed
`hammer_windows_parity.json` records 26/26, and the R02 evidence records 60/60
under Wine. Inventory has 46 records and the ledger 28 migrations.

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

### VMF→brush geometry bridge and first `linux-gtk-desktop` boot (feasibility)

A second `hammer.geometry` extraction and the first bootable shell for the D1
delivery target landed together as a working, testable consumer at a new boundary.

- **Headless geometry bridge (HAM-GEOMETRY-001, tested).**
  `public/hammer/geometry/brush.h` + `hammer/core/geometry/brush.cpp` turn a
  parsed VMF (the `hammer.formats` keyvalues tree) into renderable convex solids:
  each brush `solid`'s `side` planes are intersected into per-face polygons with
  outward normals. It is dependency-free (C++ stdlib + `hammer.formats`), computed
  in double precision, and **winding-independent** — outward orientation is derived
  from an interior point found from the polytope's own vertices, not from VMF point
  order (this fixed a real bug where an origin-relative heuristic mis-oriented an
  off-origin brush). Contract: `unittests/hammertest/contracts/geometry.brush.v1.md`.
  Suites `hammer.geometry.brush` (+`.sensitivity`, an unclipped provider the oracle
  catches) pass under gcc and clang on `linux-headless-core` via
  `run_headless.sh`. This is `characterized`: legacy Hammer's own VMF/brush code
  remains the sole live authority; this is the strict, render-facing importer, not
  a replacement of a legacy caller, and it does not yet cover displacements,
  texture axes, or scene→VMF export.
- **Shell delivery target — first boot (`hammer/gtk/`, separate product).** A GTK4
  + libadwaita host composes the strict core (`EditorDocument` + the geometry
  bridge, over a host-owned `IFileStore`) and:
  - boots an Adwaita window and **opens a simple VMF** (File▸Open / `--open`);
  - lays out the **classic Hammer UI** — menu bar, toolbar, tool palette, the four
    viewports (3D camera + 2D top/front/side, resizable panes), the object bar
    (Select · texture group · current texture · VisGroups · Show/Edit/Mark) and a
    status bar with a live world-coordinate read-out;
  - renders a shaded 3D preview and 2D wireframe-with-grid views, with
    touchpad-native navigation (two-finger pan/orbit, pinch- and ⌃-scroll zoom
    anchored at the cursor, kinetic scrolling).
  Native GTK/GDK/GL detail is confined to `hammer/gtk/`; the core it drives stays
  headless. See `hammer/gtk/README.md`.
- **Verified.** Builds `-Wall -Wextra -Werror` under g++ 16 / clang 22 against
  system gtk4 4.22 + libadwaita 1.9 + epoxy. The interactive window boots on the
  live Wayland session with a GL 4.6 core context on all four viewports and stays
  running (no crash). The 3D preview is checked **without a window server** through
  a shared offscreen EGL path: `--screenshot` (single view) and `--quad` (the 2×2
  camera/top/front/side composite) render the sample `hammer/gtk/samples/room.vmf`
  correctly; `hammer/gtk/tests/viewport_smoke.sh` asserts non-blank geometry and
  the expected solid/triangle counts. (External capture of the on-screen window is
  blocked by the session compositor's screenshot policy; the offscreen path renders
  the identical viewport content.)
  - **Toolchain note:** GtkGLArea defaults to a GLES context here, which crashed
    Mesa's GLSL linker on the `#version 330 core` shaders; forcing desktop GL core
    (`gtk_gl_area_set_allowed_apis(GDK_GL_API_GL)` + required 3.3) fixes it and
    matches the offscreen EGL profile.
- **Not claimed.** This closes **no** gate. It is R1/H5 feasibility only, and the
  legacy MFC Hammer remains the authority. Still absent: entity/property editing,
  non-box brush *editing* (they render and round-trip, but the Block tool only
  makes boxes), vertex/clip tools, real material/texture rendering, displacements.

### Core editing loop and interaction authority (HAM-WORKFLOW-001, feasibility)

A single headless **interaction authority**, `hammer::app::EditorController`
(`public/hammer/app/editor_controller.h` + `hammer/core/app/editor_controller.cpp`),
now owns the core Hammer UX flows over *normalized* input, with the GTK shell as a
thin presenter that maps gestures/keys to it. This is the H3/H4 "one
selection/mutation/history owner" realised for spatial editing:

- **Flows:** Block tool (drag a grid-snapped rectangle in a 2D view → extruded
  brush, live pending box shown; Enter commits), Selection tool (pick/move/delete),
  and **undo/redo** — all through **one** `DocumentHistory` stack (no second stack;
  `IsModified`/`MarkSaved` delegate to it). Grid snapping routes through the shared
  `RoundHalfAwayFromZero` owner; geometry through the shared brush bridge.
- **Real brush shape (not bounding boxes):** a brush is stored as its set of
  half-space planes (+ per-face materials), so a loaded non-box brush (e.g. a
  triangular-prism ramp) **renders and round-trips as its true shape** — verified
  by the `hammer.app.editor_controller` suite (a 5-plane wedge stays 5-faced through
  load → `BuildScene` → `ToVmf` → reload) and visually via `hammer_gtk --cquad`
  (the front view shows a triangle, not a rectangle). The AABB is only a pick/grid
  cache. This resolves the earlier "loads as bounding box" limitation.
- **One save path (DRY):** save = `EditorController::ToVmf()` → `SaveDocument` over
  `hammer::adapters::platform::DiskFileStore`; load = `DiskFileStore::Read` →
  `LoadVmf`. The host holds no second `EditorDocument` or file store; the
  host-local `posix_file_store.h` was retired in favour of `DiskFileStore`.
- **Conformance UI test:** `unittests/hammertest/app/test_editor_controller.cpp`
  drives the controller with simulated input to build, snap, extrude, select, move,
  delete, undo/redo, preserve a non-box shape, and save-round-trip a map;
  `_negative.cpp` proves the grid-snap oracle catches a no-snap provider. Contract
  `unittests/hammertest/contracts/app.editor_controller.v1.md`, green under gcc and
  clang on `linux-headless-core`. The panels of the shell are drag-resizable
  (nested `GtkPaned`). Single-authority design reviewed by the sibling sessions.

#### Texturing + point entities: a full VMF built from scratch (HAM-WORKFLOW-001, this change)

The same authority now covers the two remaining pieces the contract flagged as
"later migrations" for a from-scratch authoring flow — **per-face texturing** and
**point-entity placement/keyvalue editing** — both routed through the one history
stack, with no new document, global, or second selection owner:

- **Material tool:** a click on a brush (`Tool::Material`) applies the active
  material (`SetActiveMaterial`) to every face; `ApplyActiveMaterialToSelection()`
  is the menu/button form. Both route through one private `RetextureBrush(id)`
  (DRY), recording **one** history unit iff a face actually changed.
- **Entity tool:** a click (`Tool::Entity`) places a point entity of the active
  class (`SetEntityClass`, default `info_player_start`) at the snapped location;
  `SetEntityProperty(id,key,value)` sets extra keys (`targetname`, `angles`, …) —
  one unit iff changed, with `classname`/`origin`/`id` reserved. The Selection tool
  picks/deletes entities too (nearest within a half-grid tolerance when no brush is
  hit); placement/deletion undo/redo through the shared stack.
- **VMF round-trip:** `ToVmf()` emits sibling top-level `entity` blocks
  (`classname`, extra keys, `origin`); `LoadVmf` reconstructs point entities from
  them (skipping brush entities, which still load as world brushes — a declared
  limitation). Entities and per-face materials survive save → reload in order.
- **The headline evidence** in `test_editor_controller.cpp` (`TestBuildRoomFromScratch`)
  builds a complete little room **from scratch through simulated input**: four walls
  (Block tool), a `BRICK/BRICKWALL001A` material applied to all four with the
  Material tool, and an `info_player_start` + `weapon_portalgun` placed with the
  Entity tool — then parses the saved VMF (4 world solids, 24 brick sides, 2 named
  entities at the expected origins) and reloads it to confirm every brush, material,
  and entity round-trips. `TestEntityPlaceDeleteUndo` covers place/delete undo/redo.
  All 59 RFC 0002 Q-EDITOR suites remain green under gcc and clang on
  `linux-headless-core`. Contract `app.editor_controller.v1.md` updated to match.
  **Not claimed:** brush (solid) entities, non-box brush *editing*, vertex/clip
  tools, and multi-select remain later migrations.

## Pre-existing failures (reported separately, not introduced here)

- **RFC 0001 loader gate is currently failing** at the working tree: `check --all`
  and `baseline --verify` report **51 new and 31 stale** occurrences, and
  `inventory --verify` reports stale. The AGENTS.md snapshot at `87955f67` recorded
  10 new / 3 stale; HEAD (`2d7e01d5`) has drifted further. This is the R01/R07
  reconciliation item from AGENTS.md — an intentional review action, not a
  regression from RFC 0002 work. This change touches no scanned source and no
  loader manifest section, so it does not affect that count.
- 2026-09-25 at `d6260d90`: `check --all` and `baseline --verify` report 64 new
  and 1 stale occurrence (0 relocated). `inventory --verify` reports 13
  uninstrumented native sites. None of them is in a Hammer path.
- 2026-09-25: `archlint hammer --verify` failed on the `render.contracts` edge
  that the RFC 0007 PBR schema slice added to `hammer.formats`. It passes again
  after the validator change above ([current state](#current-state-2026-09-25)).

### VPK + texture (VTF/VMT) loading: asset catalog and textured viewport (HAM-ASSET-001)

- **Gap this closes:** the shell rendered brushes flat-shaded; there was no path
  from a Source game's packed assets to a material preview or a textured viewport.
  The legacy Hammer path drove everything through the engine `IMaterialSystem` /
  `IMaterial` globals, which the strict core may not use.
- **Strict-core codecs (dependency-free: C++ stdlib + narrow ports only).**
  - `hammer::ports::IByteStore` (`public/hammer/ports/byte_store.h`): ranged,
    read-only byte source. Adapter `hammer::adapters::platform::DiskByteStore`
    (`<fstream>`, non-strict) so a texture is pulled from a data archive without
    reading the whole file.
  - `hammer::ports::IAssetSource` (`asset_source.h`): logical asset lookup +
    enumeration. `hammer::formats::SearchPathAssets` composes several providers
    first-hit-wins (a `.vmt` in one VPK naming a `.vtf` in another).
  - `hammer::formats::VpkArchive` (`vpk_archive.{h,cpp}`): dependency-free VPK v1/v2
    directory reader over `IByteStore`, resolving inline/preload/external chunks —
    the clean-core replacement for the tier2-coupled `vpklib` `CPackedStore`.
  - `hammer::formats::VtfImage` / `DecodeVtf` (`vtf_image.{h,cpp}`): VTF 7.1–7.5
    decoder to mip-0 RGBA8 (RGBA/BGR/BGRA/… + DXT1/3/5), header read by byte offset
    (packing/endian independent). Unsupported formats fail with a diagnostic.
  - `hammer::formats::MaterialCatalog` (`material_catalog.{h,cpp}`): enumerates
    `materials/**.vmt`, canonicalizes authored names, resolves `$basetexture`
    (following one level of `patch` `include`), and decodes it to RGBA, caching
    both. It **builds on** `hammer::formats::ParseMaterial` / `Material`
    (`material.{h,cpp}`, owned separately, HAM-MATERIAL-001) — one VMT parser, no
    duplicate. A patch's own `$basetexture` (incl. `replace`/`insert`) comes from
    the parser's `ResolvedParam`; the catalog additionally follows the patch
    `include` one level. Deeper include chains are out of scope.
- **Conformance (RFC 0005 runner, `linux-headless-core`, g++ 16.2 + clang 22.1,
  `-Wall -Wextra -Werror`).** Seven new suites, positive + sensitivity, all green:
  `hammer.formats.vpk_archive[.sensitivity]`, `.vtf_image[.sensitivity]`,
  `.material_catalog[.sensitivity]`, `.search_path_assets`. Fixtures are built by
  independent serializers (a VPK-blob builder, a VTF-blob builder) — the reader is
  never its own oracle — and the sensitivity suites reject malformed archives,
  truncated/unsupported VTFs, and missing/corrupt textures. Full RFC 0002 Q-EDITOR
  gate is 53/53. Contracts: `formats.{vpk_archive,vtf_image,material_catalog,
  search_path_assets}.v1.md`.
- **Real shipped content (integration smoke, not a required gate — asset absent in
  CI).** Mounting HL2 `hl2_misc_dir.vpk` + `hl2_textures_dir.vpk` (18 796 + 5 232
  entries) enumerated 5 292 materials and decoded DXT1/DXT5 512×512 base textures
  end-to-end through the search path.
- **GTK frontend (`hammer/gtk/`, additive).** The renderer textures the 3D view:
  a location-3 `aTexCoord` + `sampler2D`, per-face texture bind keyed by
  `BrushFace.material` via a borrowed `MaterialCatalog`, world-planar UVs at
  Source's 0.25 texels/unit, lazily-uploaded/cached GL textures. **Safety net:** no
  catalog → a single flat draw, byte-for-byte the prior mesh path (the offscreen
  `--quad`/`--demo`/`--cquad`/`--screenshot` paths and 2D/grid/highlight are
  unaffected). New `--textured OUT.ppm MAP.vmf VPK[,VPK]` offscreen mode is the
  headless evidence: `room.vmf` over the HL2 VPKs renders the `dev/dev_measure`
  grid texture tiled on the faces (13 → 126 distinct colours vs the flat render);
  `viewport_smoke.sh` still PASSes. A material-browser dock (thumbnail grid),
  object-bar current-texture swatch/name, and a `File ▸ Mount Game Assets…` folder
  chooser (auto-discovers `*_dir.vpk`) wire the catalog into the live shell.
- **Not claimed.** No R17/R25 material-fidelity gate is closed: UVs are
  world-planar (VMF `uaxis`/`vaxis` are not yet parsed), only `$basetexture` is
  shown (no bumpmaps/proxies/blends), and HDR/float VTF and cubemaps are out of
  scope per `vtf_image.v1`. This is the asset-loading + preview slice, not the full
  Source viewport.

### 3D camera navigation and view interaction parity (HAM-WORKFLOW-001, feasibility)

Brought the GTK shell's view interaction in line with legacy MFC Hammer for the
core navigation/selection flows the user exercises constantly. All additive; the
existing touchpad/orbit/pan and 2D editing paths are unchanged.

- **Classic 3D free-fly camera.** `Z` toggles mouse-look (cursor hidden; pointer
  motion turns the camera); `W/A/S/D` fly along the view, `Q/E` rise/fall, `Shift`
  boosts speed. Movement is time-integrated on a frame-clock tick that runs only
  while a key is held. Confined to the presenter and renderer: new
  `hammergtk::Renderer::FlyMove`/`FlyLook` operate on the existing orbit camera
  (FlyLook rotates in place keeping the eye fixed; FlyMove translates the eye along
  the view basis), so no camera-state duplication. GTK4 cannot warp the pointer, so
  look uses pointer deltas with a hidden cursor rather than MFC's recenter-each-frame
  loop — same feel, noted limitation.
- **3D click-to-select.** A non-drag left click in the camera view builds a world
  ray (`Renderer::PixelToRay`, matching the view's projection) and calls the new
  `EditorController::PickByRay` — an AABB broad phase (the **same cached bound the
  2D pick uses**, so selection stays one policy) followed by a precise ray/convex-
  polytope test against the brush's real face planes, so a click through a non-box
  brush's empty AABB corner correctly misses. It mutates only the selection (no undo
  step), like the 2D pick, and a miss deselects. Selection stays owned by the single
  `EditorController` authority.
- **Right-click context menus.** 2D views show a `GtkPopoverMenu` (a selection menu
  when a brush is selected, otherwise the default view menu), reusing the existing
  `app.*` actions. Faithful to MFC, the 3D view has no right-click menu.
- **2D view interaction.** **Space + left-drag** pans (MFC's pan idiom); **Tab**
  cycles a view's orientation Top → Front → Side (renderer mode + controller edit
  axes move together); **arrow keys** nudge the selection one grid step through the
  new `EditorController::MoveSelectionBy` (one undo unit, single authority);
  **+ / −** zoom anchored at the cursor; **1–9 / 0** are preset zoom levels /
  frame-all.
- **Multi-selection.** **Ctrl-click** (2D or 3D ray pick) toggles a brush in/out of
  a selection set; nudge, drag-move and delete all operate on the whole set as one
  undo unit; every selected brush is highlighted. Implemented as a primary
  (`m_selection`) plus an extra-set (`m_extraSelected`) in the one `EditorController`
  authority, so the single-select path is unchanged and all 60 Q-EDITOR suites still
  pass. The renderer gained `SetHighlights(set)` for the multi-highlight.
- **Evidence.** New headless oracles, both green under g++/clang:
  - `hammer.app.editor_controller.pick` conformance suite
    (`unittests/hammertest/app/test_editor_controller_pick.cpp`, registered in
    `quality/conformance.manifest.json`, Q-EDITOR): nearest-along-ray selection,
    reversed-ray far pick, miss/facing-away deselect, no-undo invariant, a wedge
    (non-box) case proving the convex test rejects an empty-AABB-corner ray a
    bounding-box pick would falsely hit, the arrow-nudge move/undo path, and
    multi-select (Ctrl-add/toggle, count, move-all, delete-all + restore).
  - The full **60-suite Q-EDITOR domain** still passes under g++ after the selection
    model change, confirming the single-select drag/move/delete/undo paths are intact.
  - `hammer/gtk/tests/camera_nav_test.sh` + `test_camera_nav.cpp`: FlyMove/FlyLook/
    PixelToRay camera math (eye stays fixed under look, forward/strafe/vertical
    move correctly, 2D views are no-ops). The full shell builds `-Werror`, style
    checker clean, `--cquad`/`viewport_smoke` render paths unaffected.
- **Not claimed.** No H5/R25 workflow gate is closed. Live GTK event dispatch
  (Z/WASD/right-click in a real window) is not automated here (needs a display;
  this environment's compositor blocks external capture) — the interaction *logic*
  is what the headless oracles cover. **View navigation and selection parity with
  MFC is complete** for the shipped editing tools: Z/WASD/QE fly + mouse-look,
  space+drag pan, orbit, 3D + 2D single- and multi-select (Ctrl), face-precise 3D
  picking, right-click menus, arrow-nudge, Tab view-cycle, and +/-/number zoom.
  Space+RMB strafe is covered by A/D + Q/E (the same camera motion via the keyboard).
  What remains is **not view-interaction parity** but separate editing *tools* with
  their own RFC rows — displacement (R33/H6), vertex and clip editing — each a
  feature family, not a uniform view behaviour. The GTK4-vs-MFC mouse-look nuance
  (pointer delta + hidden cursor vs recenter-each-frame) is a platform constraint,
  not a deferred feature.

## Additional work recorded 2026-09-25

The ledger and git history show Hammer work that the sections above do not
describe. The ledger is authoritative for each entry.

- **Format and geometry cores (2026-09-22).** Eleven strict headless modules
  are `extracted` with `authority: new`. Each has a contract record and a
  positive and sensitivity suite:
  - HAM-DISP-001 (`geometry.displacement.v1`);
  - HAM-INSTANCE-001 (`formats.instancing.v1`);
  - HAM-FGD-001 (`formats.fgd.v1`);
  - HAM-CORDON-001 (`formats.cordon.v1`);
  - HAM-OVERLAY-001 (`formats.overlay.v1`);
  - HAM-VISGROUP-001 (`formats.visgroups.v1`);
  - HAM-MATERIAL-001 (`formats.material.v1`);
  - HAM-GROUP-001 (`formats.groups.v1`);
  - HAM-PREFAB-001 (`formats.prefab.v1`, on the shared `vmf_transform`);
  - HAM-EXPORT-001 (`formats.map_export.v1`: instances, then hidden
    visgroups, then cordon);
  - HAM-PROPSHEET-001 (`formats.entity_property_sheet.v1`, FGD rows for the
    entity editor).

  No legacy caller routes through them, and none closes an H2 or H6 family
  gate. HAM-ASSET-001, named in the VPK/VTF section above, has no ledger entry.
- **`checks-v1` results (R02, `2134571b`).** Every Q-EDITOR suite reports one
  `testing::ReportConformance` record. `run_headless.sh` fails when a listed
  compiler is missing ([RFC 0005 Q1](0005-progress.md#q1--r02-runner-shared-conformance-runner)).
- **PBR material validation (RFC 0007 R47, `7035c29e`).** `MaterialCatalog`
  validates `PBRMetalRough` definitions against
  `public/render/pbr_material_schema.h` and follows primary and fallback patch
  chains with depth and cycle limits. The `render.pbr-material-schema` suite
  (Q-CONTENT) compiles the Hammer catalog sources. This slice added the
  `render.contracts` edge, which `archlint hammer --verify` accepts since
  2026-09-25 as an edge to a registered capability module. See the
  [RFC 0007 record](0007-progress.md).
- **KTX2 preview (RFC 0008 F3, `97e298c6`).** `MaterialCatalog` prefers a
  packaged KTX2 asset over the VTF of the same `$basetexture` through a decoder
  injected at composition. `hammer/adapters/source/ktx2_preview.cpp` supplies
  it to the GTK shell. The Waf-built `hammer_ktx2_preview_conformance` suite
  recorded 10 checks and 0 failures. Compressed-format preview and
  cross-provider precedence remain open. See the
  [RFC 0008 record](0008-progress.md#f3-hammer-ktx2-preview-caller-2026-09-23).

Still true on 2026-09-25: the MFC sibling (`hammer_mfc_shell.cpp`,
`test_mfc_workflow.cpp`) still uses `SetFirstBlockKey`, not the shared entity
editor (D8).

## Next dependency-ready migrations

2026-09-25 status: item 1 is open. For items 2–4, the first step named below
is done and the rest remains, as noted in each item. `archlint hammer
--verify` passes again ([current state](#current-state-2026-09-25)).

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
     automatically. On 2026-09-25 the universe is 530 files, with 46 classified.
2. **HAM-BUILD-001:** establish the Linux 64-bit headless editor test target and
   record the final C++20 flags (coordinated with R02/R03).
   - 2026-09-25: `run_headless.sh` is the headless target, and CI runs it.
     Still open: a Waf-integrated editor target and R03 flag validation.
3. **HAM-CORPUS-001:** author the versioned semantic comparator and its negative
   corpus alongside the first VMF/geometry fixtures.
   - 2026-09-25: the comparator exists (`CompareKeyValues`) and its negative
     case passes. Still open: the RFC minimum fixture corpus and its provenance
     records under `unittests/hammertest/fixtures/`.
4. **HAM-GEOMETRY-001:** once a strict target exists, extract `BoundBox` into
   `hammer.geometry`, compile without the MFC PCH, and route callers through it.
   - 2026-09-25: `AxisAlignedBox` is extracted and characterized, and the
     `Vector`↔`Vec3` seam exists (D5). No legacy `BoundBox` caller routes
     through it yet, so the status stays `characterized`.
