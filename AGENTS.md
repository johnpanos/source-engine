# Working on Source Engine

## Purpose and authorities

Modernize this engine incrementally while preserving declared content, gameplay,
tool, and binary compatibility. Keep a working, testable consumer at every new
boundary. The active program is defined by these RFCs:

| RFC | Responsibility |
| --- | --- |
| [0001](RFC/0001-capability-based-platform-architecture.md) | Capability-based platform/services, composition, SDL3/render/Vulkan, loader and tier retirement |
| [0002](RFC/0002-hammer-responsibility-factorization.md) | Headless editor core, Source content fidelity, GTK4/libadwaita host, legacy Hammer extraction |
| [0003](RFC/0003-dependency-aware-job-system.md) | Dependency graphs, deterministic execution, measured subsystem concurrency |
| [0004](RFC/0004-box3d-primary-physics-backend.md) | Box3D behind VPhysics, legacy assets, gameplay/persistence, profile-specific rollout |
| [0005](RFC/0005-quality-and-correctness-harnesses.md) | Eight harness families, shared execution/evidence, trustworthy quality gates |
| [0006](RFC/0006-modern-cpp-ownership-and-synchronization.md) | C++20 targets, results, ownership, bounded queues, CPU publication and GPU fences |

RFC status and implementation status are separate. A proposed interface, tool,
directory, or command is not installed infrastructure. Read the relevant RFC
and current source before changing a boundary. This file owns the cross-RFC
work order and tracking summary; domain RFCs own semantics and acceptance rules.
The module manifest owns dependency permissions. Do not keep conflicting copies
of those facts in new registries or checkers.

## Architecture and engineering philosophy

- Features depend on narrow contracts; providers implement them; application
  roots select, own, and compose providers. Inject only the dependencies a
  consumer uses. No new global service locator, capability bag, or tier alias.
- Native OS/SDK types stay in backend implementations and named private interop
  bridges. Portable code requests behavior, not OS/backend identity. CPU/compiler
  layout checks remain narrowly scoped foundation concerns.
- Each authoritative state and policy has one owner. Derived caches carry
  revisions/epochs and invalidate through the owner's committed change.
- Separate mechanism from product policy, authored data from presentation,
  gathering from independent computation, and computation from ordered commit.
- Make required/optional behavior explicit. Unsupported required behavior fails
  composition or its acceptance gate; it must not silently succeed or fall back
  after partial mutation.
- Providers outlive borrowers, modules outlive callbacks and payload destructors,
  and resource storage outlives CPU/GPU consumers. Drain/cancel with acknowledgment
  before teardown. Test partial startup and shutdown as normal operations.
- Preserve advertised vtables/layouts, content/protocol formats, callback timing,
  random-input order, and observable gameplay unless the change explicitly
  includes a versioned compatibility/behavior decision.
- Deliver bounded migrations with named callers and a deletion condition.
  Extracting a directory, increasing worker count, or adding interfaces is not
  an architectural outcome by itself.

## DRY and Liskov Substitution Principle

DRY means one authoritative representation of knowledge. Give selection policy,
snapping, texture locks, unit conversion, save policy, and other shared rules a
named owner, then route callers through it. Similar syntax alone is not a reason
to merge code. Editor/compiler geometry can have different tolerances; image and
mapping dimensions can differ; 2D/3D/logical interactions have different spaces.
Preserve those distinctions as explicit policies until tests establish equivalence.
Do not keep two synchronized mutable documents, undo stacks, or provider catalogs
to make migration convenient. Shadow comparisons use copies/private outputs.

Liskov substitution is behavioral. A replacement accepts the contract's valid
inputs, preserves invariants, provides its promised results and effects, and
obeys lifetime, ordering, threading, failure, cancellation, and operation-sequence
rules. It cannot add an implicit active document, native window, singleton, or
thread requirement. Inheritance or a C++ concept alone proves none of this.

Each replaceable contract records those obligations and runs one shared suite
against every claiming implementation, including fakes. Include deliberately
bad providers to show the suite detects violations. Split or negotiate a narrower
capability when behavior differs: a wireframe renderer cannot claim full material
fidelity. A no-op virtual function or downcast is a review signal, not by itself
proof of an LSP violation.

## C++ and code style

Use C++20 in new strict and deliberately migrated first-party targets after
their compiler/standard-library profile is validated. Existing Waf flags are
not proof of C++20 support. Keep legacy target/header dialects and Box3D's C17
configuration explicit; do not change the entire tree's standard or FP flags.

Prefer values, RAII, strong IDs, scoped enums, `[[nodiscard]]`, `std::optional`,
and bounded views where they clarify a contract. Use project `Expected<T, E>`
for recoverable errors; `std::expected` is C++23. Views do not extend lifetimes.
Use `std::unique_ptr` for exclusive heap ownership and custom deleters for native
resources. Use `std::shared_ptr` only for real shared ownership with an explained
destruction context and cost; it does not make mutable state thread-safe.
Do not pass these types or exceptions across preserved legacy/extension ABIs.

Keep functions cohesive, ownership visible, and failure paths explicit. Match
the surrounding Source naming, indentation, brace, and include conventions;
avoid unrelated formatting churn. Keep `memdbgon.h` last in source files that
use it. Preserve license headers. Avoid clever template machinery, premature
frameworks, hidden logging/allocation, and broad locks around unexamined callbacks.
Hot-path allocation and abstraction costs need measurements, not assumptions.

Ring buffers need a real bounded-transport use case and documented topology,
ordering, capacity, overflow/backpressure, payload lifetime, and close/drain.
Never silently drop required gameplay work. SPSC, MPSC/MPMC, and work-stealing
deques are different algorithms. Prefer a simple synchronized implementation
until a more complex one has test and performance evidence.

CPU publication, task completion, and GPU completion are different contracts.
Document happens-before edges; use acquire/release or justified stronger ordering.
Notification and `volatile` are not publication. Standalone atomic fences need
a specific matching synchronization argument. Recycle GPU/upload-ring resources
only after the provider's completion token allows it, not after CPU submission
or a fixed number of frames. Use sequence runners for feature work and reserve
physical-thread affinity for APIs that require it.

## Harness design and acceptance

Share runner/fixtures/failure injection/evidence infrastructure; keep domain
oracles independently meaningful. RFC 0005 defines the full requirements:

| Harness | Required evidence |
| --- | --- |
| Q-ARCH | Ownership and include/link DAG; strict headers/implementations; exact ratchets; seeded leaks rejected |
| Q-FOUNDATION | Shared provider contracts; failure rollback; no leaked borrowers/handles/tasks; frozen-header ABI fixtures |
| Q-EDITOR | Authored state and references; generated edit/history sequences; two documents; open/edit/undo/save/reopen/compile |
| Q-JOBS | Independent graph model; adversarial schedules and native races; legacy/serial-graph/parallel-graph equivalence |
| Q-PHYSICS | Isolated IVP/Box3D public-contract comparisons; analytical units/query cases; callbacks and gameplay scenes |
| Q-CONTENT | Versioned semantic corpus; independent readers/tools; malformed-input fuzzing; failure/recovery and schema compatibility |
| Q-PRESENTATION | Command/lifetime tests plus real images and native host tests; GPU fences, loss, resize, scale, capture and multi-view |
| Q-PRODUCT | Installed client/server/editor/tools; cross-domain workflows; budgets, supported profiles, packaging and rollback |

Required runs fail on zero/missing tests, missing assets/providers, crashes,
timeouts, or incomplete results. Skipped coverage cannot certify a gate. Test
assertions remain effective in release builds. Test the runner, comparators,
and critical suites with negative fixtures before trusting them.

Version fixtures, comparators, tolerances, and profiles. Record revision/dirty
digest, dependency pins, toolchain/flags, provider settings, inputs/seeds/schedules,
actual test counts, logs, first divergence, and reproduction commands. Do not
discard difficult semantic fields or update goldens/baselines merely to get green.
Use exact comparison where promised and operation-specific tolerances elsewhere;
do not demand identical IVP/Box3D trajectories or cross-backend pixels.

Run relevant static and deterministic checks per PR, full relevant native/corpus
and sanitizer lanes at merge gates, real UI/GPU checks for changed boundaries,
and stress/fuzz/soak and controlled benchmarks on their declared runners. A
required release gate remains unverified if its hardware/content is unavailable.
Use separate supported ASan/UBSan and TSan configurations. Set performance and
latency/memory budgets before optimization and include low-core/small workloads.

## Working protocol

1. Choose the highest-ranked dependency-ready bounded task below, unless the
   user's request selects another scope. Read its domain contracts and identify
   existing changes before editing. Preserve unrelated work and submodule state.
2. Identify current consumers, ownership, supported profiles, and baseline
   behavior. Record observed facts separately from hypotheses and target decisions.
3. Add the smallest trustworthy oracle for the boundary, including failure and
   lifetime behavior. Routine documentation/mechanical edits need proportionate
   verification, not artificial runtime tests.
4. Introduce the seam and migrate a bounded caller cohort. Keep one live authority;
   compare old/new paths only against immutable inputs or private copies.
5. Run the relevant installed checks, report pre-existing failures separately,
   and retain reproducible artifacts. Do not invent a command from a proposed RFC.
6. Remove obsolete callers/glue when the stated retirement condition is met.
   Update exact inventory/baseline changes only after reviewing their classification.
7. Update roadmap state and link evidence in the same change. State what passed,
   what was unavailable, and which dependent gates remain unverified.

Current installed architecture commands (run from the repository root):

```sh
python3 tools/archlint/archlint.py check --all
python3 tools/archlint/archlint.py baseline --verify
python3 tools/archlint/archlint.py inventory --verify
python3 -m unittest discover -s tools/archlint/tests -v
```

`check --changed` is a local convenience, not a full gate. `--compile-deps` and
the new domain runners are proposed until implementation records working
commands. Check the current Waf configuration before building; do not overwrite
someone's build profile just to run unrelated tests. Configure/build actual
products as well as the limited `--tests` composition when their gate requires it.

## Unified ranked roadmap

This is portfolio priority, not a promise of calendar duration or a requirement
that independent work wait for all earlier rows. Dependencies are hard gate
prerequisites; lower-ranked work may proceed when its prerequisites and resources
are available without delaying higher-ranked ready work. Baseline captures and
bounded feasibility experiments may precede a full implementation gate, but must
not claim that gate complete. No row requires parallel agents by default.

IDs stay stable when priorities change. Add child tasks for caller cohorts or
feature families; a parent closes only when its full declared scope passes.
Each row's done condition also includes RFC 0005 evidence for the selected
profiles and the applicable domain RFC's complete acceptance requirements.
RFC 0001 rank numbers and other RFC phases below identify coverage, not a second
portfolio ordering. Native tool builds needed by early fixtures are established
with those fixtures; later tool cleanup is not a prerequisite to using them.

States: `planned`, `partial` (existing work, gate incomplete), `active`, `blocked`,
`done`. `planned` does not imply dependency-ready. Initial states reflect source
revision `87955f67`; documentation alone marks no implementation gate done.

| Rank / ID | Work and RFC scope | Prerequisites | Done looks like | State |
| --- | --- | --- | --- | --- |
| 1 / R01 | Reproducible baseline and profile inventory; 0005 Q0, baseline portions of all domains | — | Current checks/failures recorded; exact build/content/tool availability and supported profiles established; baseline captures and budgets identified | partial |
| 2 / R02 | Trustworthy runner, fixtures, evidence; 0005 Q1 | R01 | Zero/missing tests, skips, crashes, timeouts and incomplete output fail correctly; explicit test composition and reproducible artifacts work | planned |
| 3 / R03 | Per-target C++20/toolchain boundary; 0006 M0 | R01, R02 | Compile/link/run proof; final flags verified; legacy/C17 settings and frozen-consumer ABI combinations preserved | planned |
| 4 / R04 | Full architecture and migration enforcement; 0001 rank 1, 0002 H0 enforcement, Q-ARCH | R01, R02 | Ownership, direct/transitive includes, Waf/link graph, hermetic builds, exact debt and evidence schemas enforced; negative projects fail | planned |
| 5 / R05 | Results, IDs, quantities, ownership vocabulary; 0001 rank 2, 0006 M1 | R03, R04 | `Expected`, borrowing/scoped resources and matchers pass value/lifetime/ABI tests; a real consumer uses them | planned |
| 6 / R06 | Composition/lifecycle kernel and minimal test providers; 0001 rank 3, Q-FOUNDATION | R02, R05 | Unit runner composes typed providers without ambient factories; required/optional validation, failure-at-each-stage rollback and repeat-instance tests pass | planned |
| 7 / R07 | Loader containment, telemetry and ABI fixtures; 0001 rank 4 / retirement A | R04, R06 | Scoped ownership, structured errors, legacy bridge and fake/native suites pass; telemetry handles failed/duplicate/nested requests; reviewed ratchet/inventory current | partial |
| 8 / R08 | Hammer H0 corpus and migration inventory; 0002, Q-EDITOR/Q-CONTENT | R02, R03, R04 | Exhaustive ownership/callers and migration records; legacy build evidence/gaps; headless target; semantic comparator detects seeded data loss | planned |
| 9 / R09 | Physics A feasibility and IVP baseline; 0004, Q-PHYSICS | R01, R02, R05 | Method/profile inventory, units/assets and measurements; tested solution or explicit scope decision for impact state, contact mutation, ragdoll limits and hull/decoder blockers | planned |
| 10 / R10 | Runner/clock/sequence contracts and serial graph; 0001 rank 11, 0003 A–B | R05, R06 | Virtual time and independent graph model; validation/publication/affinity/failure tests; ordered serial host graph matches legacy captures | planned |
| 11 / R11 | Paths and module resolution; 0001 rank 5 | R05, R07 | Native/virtual paths distinct; resolution/verification separate from opening; encoding/search/failure corpus passes | planned |
| 12 / R12 | Dedicated-server composition; 0001 rank 6 | R06, R07, R11 | Installed startup/shutdown and partial failure pass; link/runtime evidence shows render and desktop UI absent | planned |
| 13 / R13 | Hammer geometry and scene seams; 0002 H1 | R05, R08 | Strict headless targets; geometry/reference/reparent tests and independent documents pass; selected legacy callers route through shared owner | planned |
| 14 / R14 | Window/input contracts and SDL2 adapter; 0001 rank 7 | R06 | Existing behavior captured and preserved; normalized events, optional behavior, surface ownership and input lifecycle conformance pass | planned |
| 15 / R15 | Render seam, scoped legacy services and null provider; 0001 rank 8 | R06 | Explicit provider/caps/profile selection; null and legacy contract suites; material consumer tested without new shader globals | planned |
| 16 / R16 | Pair-specific presentation bridges; 0001 rank 9, 0006 M3 | R14, R15 | Native handles confined; multi-surface resize/zero-size/loss/shutdown pass; delayed GPU completion prevents early reuse | planned |
| 17 / R17 | Hammer real renderer feasibility; 0002 R1 | R08, R15, R16 | Source-material viewport on declared GTK X11/Wayland profiles; state/target restoration, scale, capture, sharing and teardown measured | planned |
| 18 / R18 | SDL3 provider parity; 0001 rank 10 | R14, R16 | Same window/input suites and representative behavior pass for SDL2/SDL3; SDK dependency is private; supported interop pairs tested | planned |
| 19 / R19 | Box3D one-worker vertical slice; 0004 B | R05, R09 | Pinned coherent provider loads existing BSP/PHY, compound prop, inside trace, verified impact, ragdoll and matching-schema restore | planned |
| 20 / R20 | Parallel scheduler and controlled legacy bridge; 0003 C, 0006 M2 | R10 | Bounded queue/worker contracts, publication/wake/overflow and native stress pass; no forbidden helping/nested wait; total capacity and overhead measured | planned |
| 21 / R21 | Particle reference migration; 0003 D | R20 | Legacy/serial/parallel captured outputs agree; attachment/lifetime tests, improvement and small-workload budgets pass; quiescent rollback works | planned |
| 22 / R22 | Hammer persistence slice; 0002 H2 | R11, R13 | Declared VMF features round-trip and compile; independent acceptance, unknown/loss reporting, detached import and save failure/recovery pass | planned |
| 23 / R23 | Hammer application authority; 0002 H3 | R13, R22 | One selection/mutation/history owner; draft resolution, transform/cancel/undo/redo/save-position and generated sequences pass headlessly | planned |
| 24 / R24 | Hammer tools and presenters; 0002 H4 | R23 | Normalized traces share policies across entry points; two-document and close/focus/capture tests pass; no widgets in tools/presenters | planned |
| 25 / R25 | GTK editor workflow; 0002 H5 | R17, R22, R24 | Open/edit/undo/save/reopen/compile/run with multiple views, inspector and textures; declared fidelity and no hidden MFC runtime dependency | planned |
| 26 / R26 | Remaining foundation providers; 0001 rank 12 | R10, R11 | Native clock/thread/memory/process/environment/paths/diagnostics suites pass for supported profiles, including failure and cleanup | planned |
| 27 / R27 | Vulkan compatibility waypoint; 0001 rank 13 | R10, R16, R18 | Deployment, shader artifacts, profile selection and SDL3 presentation proven by a measured compatibility experiment; limitations recorded | planned |
| 28 / R28 | Native Vulkan bootstrap; 0001 rank 14 | R10, R16, R18 | Native adapter/device/queues and SDL3 bridge present smoke frame; required-profile failure and validation diagnostics work | planned |
| 29 / R29 | Additional-OS architecture proof; 0001 rank 15 | R12, R18, R26, R28 | Dedicated composition plus SDL3/Vulkan smoke and foundation suites pass using provider/composition changes; portable leaks treated as defects | planned |
| 30 / R30 | Existing parallel kernels; 0003 E | R21 | Each bones/query-cache/entity-packing/leaf/shadow cohort independently passes three-mode, ownership, latency/performance and rollback gates | planned |
| 31 / R31 | Physics core compatibility; 0004 C | R19 | Required traces, filters, events, materials, constraints/ragdolls, controllers and persistence pass client/dedicated gameplay corpus | planned |
| 32 / R32 | Native Vulkan functional MVP; 0001 rank 16 | R10, R28 | Representative map renders opt-in; resource/pipeline/upload/sync/swapchain contracts pass; unsupported features fail explicitly | planned |
| 33 / R33 | Hammer feature families; 0002 H6 | R25 | Each declared displacement/instance/manifest/overlay/texture/preview family passes load/edit/undo/save/build, recovery and performance gates | planned |
| 34 / R34 | Physics gameplay and tool completion; 0004 D | R31 | Required fluids, vehicle modes, pulley/group and selected Portal/game features pass; compiler/content workflows preserve supported formats | planned |
| 35 / R35 | New audited compute seams; 0003 F | R30 | Animation/render-list/AI/streaming cohorts have stable inputs, correct cross-system edges and ordered commit; individual equivalence/budget gates pass | planned |
| 36 / R36 | Vulkan parity and release readiness; 0001 rank 17 | R32 | Required materials/images, loss/recovery, cache, multi-surface and hardware budgets pass; default selection is a separately recorded product decision | planned |
| 37 / R37 | Physics parallel rollout and default gate; 0004 E | R20, R34 | Worker-count determinism, nested-work/callback/shutdown bridge, platform packaging, budgets and supported client/server combinations pass; IVP rollback tested | planned |
| 38 / R38 | Stateful scheduling migrations; 0003 G | R30, R35, R37 | Snapshot-send ownership and selected entity/physics cohorts preserve legacy observations/order or record intentional change; network/latency/lifetime gates pass | planned |
| 39 / R39 | First-party module retirement; 0001 rank 18 / retirement B–D | R12, R18 | Pseudo-modules removed; mandatory systems and provider catalogs use typed linked factories; no filename/string discovery for migrated services | planned |
| 40 / R40 | Tool executable/process cleanup; 0001 rank 19 / retirement E | R11, R12, R22 | Launchable-DLL wrappers retired by cohort; structured argv/process protocol, outputs/cancellation and required compiler workflows pass; integrations tool-only | planned |
| 41 / R41 | Extension hosts and public-loader removal; 0001 rank 20 / retirement F–G | R07, R11, R39, R40 | Family-owned versioned ABI/trust/lifetime fixtures pass; Waf enumerates boundaries; only approved hosts load; Tier1/filesystem general loader APIs retired | planned |
| 42 / R42 | Scheduler consolidation; 0003 H | R35, R38 | Redundant queues/waits have zero consumers; process worker budget controlled; supported host modes retain correctness/latency and rollback evidence | planned |
| 43 / R43 | Hammer legacy retirement; 0002 H7 | R33 | Declared product parity/recovery gate met; old consumer counts zero; superseded shell/glue/build references and stale exceptions removed | planned |
| 44 / R44 | IVP simulation retirement; 0004 F first gate | R37 | Declared profiles no longer depend on IVP simulation; gameplay/save/package gates pass and rollback/support decision recorded; decoder dependency remains explicit | planned |
| 45 / R45 | Independent collision decoding/cooking; 0004 F second gate | R40, R44 | Legacy/native format corpus and tool compatibility pass without IVP code; dependency audit clean; schema and old-content policy explicit | planned |
| 46 / R46 | Tier-global/domain retirement; 0001 rank 21 | R39, R41, R42, R43, R45 | All declared domain cohorts use explicit ownership; old globals have zero consumers; cohesive targets pass architecture/product gates; tiers removed only when empty | planned |

R39–R46 describe completion gates, not a reason to retain dead code until late.
Delete each unused adapter/global/queue when its bounded cohort has passed its
own gate. Likewise R30/R33–R35 are portfolios of independently accepted slices;
no broad parallelization or feature-parity claim is granted by starting a row.

## Tracking and current evidence

When starting work, change its row to `active` and record the bounded scope and
owner in the relevant domain progress record (create one when first needed).
For `blocked`, record the missing dependency/toolchain/content/decision and what
can still proceed. For `done`, link the current revision/profile evidence and
confirm every required child and exit criterion. Reopen affected gates when a
contract/provider/comparator/consumer changes; do not preserve stale completion.
Keep the table concise and link details below or from the domain progress file.

Initial evidence, observed at `87955f67` before these documentation changes:

- R01: existing checker fixtures report eight passes. Full loader check/baseline
  reports 10 new and 3 stale occurrences; inventory verification reports stale.
  This is an existing failure to reconcile through review, not permission to
  regenerate baselines automatically. Native product/harness gates are unverified.
- R07: [telemetry source](tier1/module_load_telemetry.cpp) and
  [request API](public/tier1/module_load_telemetry.h) exist. Required native,
  failure/lifetime and ABI evidence has not been established by this assessment;
  see [Phase A progress](RFC/0001-phase-a-progress.md).
- RFCs 0005/0006 and this roadmap define future implementation work. No runner,
  C++20 build migration, physics backend, editor port, or scheduler is delivered
  merely by adding these documents.
