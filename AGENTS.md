# Working on Source Engine

## Purpose and authorities

The main north star is a **job-based engine with Vulkan rendering, SDL3 platform
integration, and platform-compliant Linux, macOS, iOS, and Android backends**.
Deliver the infrastructure to build this architecture and support every declared
platform within its normal execution and distribution rules; an engine that only compiles on a developer's Linux machine
does not meet the goal. Modernize incrementally while preserving declared
content, gameplay, tool, and binary compatibility. Keep a working, testable
consumer at every new boundary. The active program is defined by these RFCs:

| RFC | Responsibility |
| --- | --- |
| [0001](RFC/0001-capability-based-platform-architecture.md) | Capability-based platform/services, composition, SDL3/render/Vulkan, loader and tier retirement |
| [0002](RFC/0002-hammer-responsibility-factorization.md) | Headless editor core, Source content fidelity, GTK4/libadwaita host, legacy Hammer extraction |
| [0003](RFC/0003-dependency-aware-job-system.md) | Dependency graphs, deterministic execution, measured subsystem concurrency |
| [0004](RFC/0004-box3d-primary-physics-backend.md) | Box3D behind VPhysics, legacy assets, gameplay/persistence, profile-specific rollout |
| [0005](RFC/0005-quality-and-correctness-harnesses.md) | Eight harness families, shared execution/evidence, trustworthy quality gates |
| [0006](RFC/0006-modern-cpp-ownership-and-synchronization.md) | C++20 targets, results, ownership, bounded queues, CPU publication and GPU fences |
| [0007](RFC/0007-physically-based-lighting-pipeline.md) | Substitutable light baker (legacy vrad, Cycles), PBR material family, image-based lighting, compile-tool port |
| [0008](RFC/0008-canonical-world-data-and-runtime-formats.md) | Compiled USD World Stage, BSP2/KTX2, modern map and model resources, visual capabilities, incremental builds and live USD development loading |
| [0009](RFC/0009-usd-native-map-authoring.md) | Editable USD map source, world/prop role separation, native map compiler and editor workflow without VMF (proposed) |
| [0010](RFC/0010-portable-vgui-surface.md) | VGUI beneath its frozen API: UI draw list, UI scale, portable fonts, text input, optional HTML, composition (proposed; no roadmap row yet) |
| [0011](RFC/0011-runtime-indirect-lighting.md) | Runtime indirect light: probe volume with visibility, indirect policy, runtime light set, substitutable baked/radiosity/SDF/ray-query producers switchable at runtime (proposed; G0–G10 evidence on native Vulkan desktop, rows R70–R80 unranked) |
| [0012](RFC/0012-antialiasing-msaa-specular-alpha-coverage.md) | Antialiasing: per-profile 4x MSAA target policy, alpha to coverage, PBR specular AA, offline normal-variance roughness and alpha-coverage mips (proposed) |
| [0013](RFC/0013-opt-in-physics-capabilities.md) | Opt-in Box3D capabilities beside the IVP-parity contract: one versioned interface per capability, profile selection, per-capability benchmark gates; parallel step first (proposed) |
| [0014](RFC/0014-native-vulkan-and-bsp2-debug-controls.md) | Native Vulkan and BSP2 debug controls: one device-owned view catalog in separate `DEBUG_VIEW` variants, draw pick/bisection, shader reload/capture, sync/reuse checks, BSP2 lump inspection (proposed) |

RFC status and implementation status are separate. A proposed interface, tool,
directory, or command is not installed infrastructure. Read the relevant RFC
and current source before changing a boundary. This file owns the cross-RFC
work order and tracking summary; domain RFCs own semantics and acceptance rules.
The module manifest owns dependency permissions. Do not keep conflicting copies
of those facts in new registries or checkers.

This user-directed north star sets the portfolio scope. Earlier RFC references
to a first Linux profile or a single additional-OS experiment are starting
slices, not the final support boundary. The platform gates below expand that
scope without certifying implementation. Keep owning RFC contracts aligned as
each platform slice is designed; do not silently weaken them to fit a backend.

The map-authoring north star is an editable OpenUSD source for new maps (RFC
0009). RFC 0008's VMF-derived compiled World Stage is a compatibility and
bootstrap producer; it must not make VMF or BSP face IDs prerequisites for the
future native USD producer. Preserve legacy VMF/content support while that
separate authoring gate is developed.

## Platform and build infrastructure

Job-based execution means explicit dependencies, scoped inputs/outputs,
nonblocking compute scheduling, and ordered observable effects. It does not mean
every operation must execute in parallel. Serial graph execution remains a
required oracle and low-capacity mode; physical-thread affinity belongs only
to APIs that require it. Process-wide budgets must accommodate mobile power,
thermal limits, backgrounding, and other executors.

SDL3 and Vulkan are the target stack, not proof of platform parity. Keep SDKs,
native handles, lifecycle callbacks, and loader details inside providers and
pair-specific bridges. Desktop assumptions about windows, process lifetime,
filesystems, plugins, and available GPUs must not leak into portable code.

| Target | Planned runtime path | Native acceptance obligations |
| --- | --- | --- |
| Linux | SDL3 + Vulkan; independent headless server/tool compositions | Declared X11/Wayland and GPU profiles, surface loss/resize, input, filesystem/package behavior; no desktop dependencies in headless products |
| macOS | SDL3 + Vulkan through MoltenVK/Metal | Apple toolchain/SDK and declared architectures, portability features/shaders, app lifecycle/input, app bundle and ordinary distribution-channel requirements |
| iOS | Statically linked first-party modules; SDL3 + Vulkan through MoltenVK/Metal | Device arm64 profile, separate simulator tests, no first-party shared-module discovery, foreground/background and surface recreation, touch/orientation, memory pressure, app-container storage and normal device packaging |
| Android | SDL3 app integration + Vulkan | Pinned SDK/NDK/JDK/build tools and declared ABIs/API/device levels, activity/surface recreation and process death, touch/input, memory pressure, permissions/storage, signed APK/AAB for the selected channel |

MoltenVK implements a Vulkan portability subset over Metal; query and validate
the required feature profile and shader translations, rather than claiming full
desktop Vulkan equivalence. Keep Metal details private to the Apple bridge.
See [MoltenVK](https://github.com/KhronosGroup/MoltenVK) and SDL's
[iOS](https://wiki.libsdl.org/SDL3/README-ios) /
[Android](https://wiki.libsdl.org/SDL3/README-android) integration documentation.
These references describe dependencies, not evidence that this engine supports
those combinations. Exact OS/SDK/deployment minima and hardware coverage must be
pinned in implementation profiles before their support claim; no invented flags.

The Linux GTK Hammer shell is a separate product scope, not an obligation to
ship GTK on mobile. Existing Windows, FreeBSD, and other compatibility profiles
are preserved unless explicitly retired; they do not substitute for any of the
four north-star targets.

Build infrastructure MUST provide:

- A versioned profile per product/OS/architecture with dependency revisions,
  compiler/standard library, SDK/deployment target, ABI, C++20/legacy/C17 settings,
  render capabilities, shader compiler settings, packaging and required tests.
  Profiles own facts; CI, packaging, and reports consume them instead of copying
  conflicting platform matrices. Record target intent separately from evidence.
- Clean isolated output directories per profile and separation of host tools
  from target executables. Waf remains the engine build entry point; controlled
  upstream builds and native Apple/Android package tooling are allowed without
  duplicating the engine's source/dependency authority.
- Pinned dependencies, repeatable clean builds, dependency-keyed caches, shader
  artifacts, and recorded toolchain/build settings. No reliance on sibling
  checkouts or unpinned network downloads. Use the platform's normal package
  tooling; do not build custom signing/provenance infrastructure for its own sake.
- Android cross-build and device infrastructure, and Linux native/GPU runners.
  Cross-build checks run early; real device/native tests are required for
  lifecycle, graphics, permissions, and release claims. Simulators and software
  rendering are additional profiles, not replacements for hardware.
- macOS/iOS (Apple) and MSVC runners are **optional** (user decision,
  2026-09-25). Their profiles stay declared, with `runner_requirement:
  optional` in `quality/baseline.json`, and are reported as unavailable when
  their runner is missing. No roadmap gate waits on them. An Apple or MSVC
  support claim still needs that platform's own evidence.
- Shared conformance plus profile-specific integration, sanitizer/fuzz coverage
  where supported, performance/power/memory budgets, installed-package smoke
  tests, symbolized crash diagnostics, and documented rebuild/reproduction steps.

## Platform-compliant composition and distribution

Here, supporting platforms securely primarily means respecting their execution,
sandbox, and app-store rules. Keep security work proportionate to real engine
boundaries. Do not turn this roadmap into a mandatory custom signing service,
updater, SBOM program, or enterprise compliance project.

- **iOS uses static first-party composition.** Build engine, game, physics, render,
  and other selected provider modules as static libraries/objects linked into the
  app. Resolve them through typed factories at build/composition time. Preserve
  module ownership and narrow interfaces; static linkage does not mean one giant
  source target or new globals. Resolve duplicate legacy entry-point names with
  private named adapters, not filename/string lookup or `dlopen`/`dlsym`.
- Exclude first-party shared-module loaders and native plugin discovery from the
  iOS target. Test the final link map/package and run startup with module-search
  locations empty to prove no hidden loader requirement. Normal system frameworks
  are permitted; this is our composition policy, not a claim that Apple prohibits
  every bundled dynamic framework. Desktop retained extension hosts stay private
  to the profiles that explicitly support them.
- Store-targeted mobile builds do not download/load native plugins or depend on
  JIT, executable-memory tricks, private SDK APIs, or background-execution bypasses.
  Data/asset downloads are distinct from executable updates; script/mod features
  need an explicit review against the selected store's current rules before being
  enabled. Android's packaged native libraries need not be banned merely because
  the iOS first-party build is statically linked.
- Use documented lifecycle, graphics, input, storage, and networking APIs. Keep
  writes in platform-approved locations; request only required permissions and
  entitlements. Do not disable sandboxing or certificate verification to make a
  port work. Maintain basic bounds/path validation, malformed-input tests, and
  protection of credentials from source/logs and untrusted CI jobs.
- Use ordinary platform packaging and the signing/provisioning required for the
  selected channel. Release credentials are needed for the packaging/submission
  stage, not every headless build or unit test. Prefer store-managed executable
  updates. No extra signing/attestation infrastructure is implied by this goal.

Check current [Apple App Review Guidelines](https://developer.apple.com/app-store/review/guidelines/)
(particularly public APIs and self-contained apps in 2.5.1/2.5.2) and the selected
Android store's policies when designing distribution-sensitive features and
before submission. Approval also depends on the actual product and code/content
rights; static linkage alone cannot guarantee it. Preserve the repository's
existing provenance/distribution warning.

A support claim requires the declared build, contract, native runtime, and
package checks. Missing required device tests leave that profile unverified;
missing release credentials leave distribution unverified without preventing
independent engine work. A successful cross-build or smoke frame is not full
platform acceptance.

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

Every in-tree C++ target compiles as C++20 (user decision, 2026-09-22;
`quality/toolchain/policy.json` owns the dialect per target):

- unmigrated code uses `cxx20-permissive`;
- new and deliberately migrated targets use strict `cxx20`;
- `legacy-cxx11` exists only for frozen external-consumer fixtures;
- Box3D keeps its private `box3d-c17` configuration.

A permissive C++20 compile is not proof of a target's C++20 support; that
needs its validated compiler/standard-library profile. Do not change
floating-point flags tree-wide, and never select a dialect with target-local
`-std` flags.

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

[`.clang-format`](.clang-format) owns mechanical formatting for new/edited
first-party code: tabs of width four, Allman braces, Source parenthesis/pointer
spacing, and a 100-column target. Use the pinned, read-only
[style checker](tools/stylelint/README.md); existing files are checked around
edited lines and new files in full. Do not reformat unrelated legacy code.
The checker also rejects newly introduced includes after literal `memdbgon.h`.
Do not treat formatting as proof of ownership, DRY, LSP, or fence correctness.

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

Installed style commands (install the pinned formatter as described in the
[setup instructions](tools/stylelint/README.md) first):

```sh
python3 -m unittest discover -s tools/stylelint/tests -v
python3 tools/stylelint/stylelint.py --changed --diff
python3 tools/stylelint/stylelint.py --changed --base origin/master --diff
```

The last command checks the complete branch diff; plain `--changed` is a local
HEAD comparison. CI runs the style fixtures and the relevant branch/push diff.
Missing tools and invalid history fail; no eligible changes are explicitly
not-applicable. Whole-tree `--all` style checking is an optional legacy-debt audit,
not a request for global formatting. Keep policy changes and fixture tests together.

`check --changed` is a local convenience, not a full gate. `check --all
--compile-deps <built tree>` (repeatable) checks strict modules' transitive
includes from the tree's `.d` files (R04-DEPS; coverage and blind spots in the
[Phase A record](RFC/0001-phase-a-progress.md)). `archlint targets --verify
<declared trees>` checks that each recorded Waf target has one owner and its
link graph (R04-OWNERS; the tree list is `arch.targets` in
`quality/baseline.json`). New domain runners stay
proposed until implementation records working commands. Check the current Waf configuration before building; do not overwrite
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
| 1 / R01 | Reproducible baseline and profile inventory; 0005 Q0, baseline portions of all domains | — | Current checks/failures recorded; exact build/content/tool availability and supported profiles established; baseline captures and budgets identified | done (re-audited 2026-09-25; [Q0 baseline](RFC/0005-progress.md#re-audit-2026-09-25)) |
| 2 / R02 | Trustworthy runner, fixtures, evidence; 0005 Q1 | R01 | Zero/missing tests, skips, crashes, timeouts and incomplete output fail correctly; explicit test composition and reproducible artifacts work | done ([Q1 runner](RFC/0005-progress.md#q1--r02-runner-shared-conformance-runner)) |
| 3 / R03 | Per-target C++20/toolchain boundary; 0006 M0 | R01, R02 | Compile/link/run proof; final flags verified; legacy/C17 settings and frozen-consumer ABI combinations preserved | done (2026-09-25 for the required Linux and Android profiles; Apple/MSVC optional by user decision; [R03 closure](RFC/0006-progress.md#r03-closure-2026-09-25)) |
| 4 / R04 | Full architecture and migration enforcement; 0001 rank 1, 0002 H0 enforcement, Q-ARCH | R01, R02 | Ownership, direct/transitive includes, Waf/link graph, hermetic builds, exact debt and evidence schemas enforced; negative projects fail | done (2026-09-25; every recorded target declares `arch_module` or is in a row-owned legacy group, with a build-time check and the `arch.targets` gate; include, hermetic, link-graph and iOS shared-library ratchets enforced; hosted CI not run; [closure](RFC/0001-phase-a-progress.md#r04-closure-done-2026-09-25)) |
| 5 / R05 | Results, IDs, quantities, ownership vocabulary; 0001 rank 2, 0006 M1 | R03, R04 | `Expected`, borrowing/scoped resources and matchers pass value/lifetime/ABI tests; a real consumer uses them | done (2026-09-25: `Expected`, compact errors (`foundation::Error`), `StrongId`, `ScopedResource`, `testing::Checks` and `units`, each with a suite, sensitivity rows and a real consumer; CAP010 keeps them out of preserved ABI headers; hosted CI not run; [closure](RFC/0006-progress.md#r05-closure-done-2026-09-25)) |
| 6 / R06 | Composition/lifecycle kernel and minimal test providers; 0001 rank 3, Q-FOUNDATION | R02, R05 | Unit runner composes typed providers without ambient factories; required/optional validation, failure-at-each-stage rollback and repeat-instance tests pass | done (2026-09-25: typed-descriptor unit runner, required/optional validation, rollback at every stage, repeat instances, ordering, legacy bridges and negative providers; fresh Q-FOUNDATION run on g++ and clang++; native providers and hosted CI not claimed; [conformance record](RFC/0001-conformance-progress.md)) |
| 7 / R07 | Loader containment, telemetry and ABI fixtures; 0001 rank 4 / retirement A | R04, R06 | Scoped ownership, structured errors, legacy bridge and fake/native suites pass; telemetry handles failed/duplicate/nested requests; reviewed ratchet/inventory current | partial (R07-INVENTORY: inventory current, telemetry complete, `unittest_legacy` fixed on both compilers including a tier0 logger race, 2026-09-25; [Phase A record](RFC/0001-phase-a-progress.md#r07-inventory-loader-inventory-reconciled-slice-done-2026-09-25)) |
| 8 / R08 | Hammer H0 corpus and migration inventory; 0002, Q-EDITOR/Q-CONTENT | R02, R03, R04 | Exhaustive ownership/callers and migration records; legacy build evidence/gaps; headless target; semantic comparator detects seeded data loss | active |
| 9 / R09 | Physics A feasibility and IVP baseline; 0004, Q-PHYSICS | R01, R02, R05 | Method/profile inventory, units/assets and measurements; tested solution or explicit scope decision for impact state, contact mutation, ragdoll limits and hull/decoder blockers | partial ([0004 progress](RFC/0004-progress.md)) |
| 10 / R10 | Runner/clock/sequence contracts and serial graph; 0001 rank 11, 0003 A–B | R05, R06 | Virtual time and independent graph model; validation/publication/affinity/failure tests; ordered serial host graph matches legacy captures | active ([0003 progress](RFC/0003-progress.md); [host graph](RFC/0003-scheduler-trust-progress.md); [render nodes](RFC/0003-scheduler-nodes-progress.md)) |
| 11 / R11 | Paths and module resolution; 0001 rank 5 | R05, R07 | Native/virtual paths distinct; resolution/verification separate from opening; encoding/search/failure corpus passes | planned |
| 12 / R12 | Dedicated-server composition; 0001 rank 6 | R06, R07, R11 | Installed startup/shutdown and partial failure pass; link/runtime evidence shows render and desktop UI absent | partial ([composition migration slice](RFC/0001-dedicated-composition-progress.md)) |
| 13 / R13 | Hammer geometry and scene seams; 0002 H1 | R05, R08 | Strict headless targets; geometry/reference/reparent tests and independent documents pass; selected legacy callers route through shared owner | partial ([0002 current state](RFC/0002-progress.md#current-state-2026-09-25)) |
| 14 / R14 | Window/input contracts and SDL2 adapter; 0001 rank 7 | R06 | Existing behavior captured and preserved; normalized events, optional behavior, surface ownership and input lifecycle conformance pass | planned |
| 15 / R15 | Render seam, scoped legacy services and null provider; 0001 rank 8 | R06 | Explicit provider/caps/profile selection; null and legacy contract suites; material consumer tested without new shader globals | done ([render seam](RFC/0001-render-seam-progress.md)) |
| 16 / R16 | Pair-specific presentation bridges; 0001 rank 9, 0006 M3 | R14, R15 | Native handles confined; multi-surface resize/zero-size/loss/shutdown pass; delayed GPU completion prevents early reuse | done ([presentation bridges](RFC/0001-presentation-bridge-progress.md); R14 surface slice only) |
| 17 / R17 | Hammer real renderer feasibility; 0002 R1 | R08, R15, R16 | Source-material viewport on declared GTK X11/Wayland profiles; state/target restoration, scale, capture, sharing and teardown measured | planned |
| 18 / R18 | SDL3 provider parity; 0001 rank 10 | R14, R16 | Same window/input suites and representative behavior pass for SDL2/SDL3; SDK dependency is private; supported interop pairs tested | partial ([Portal slice](RFC/0001-portal-vulkan-progress.md)) |
| 19 / R19 | Box3D one-worker vertical slice; 0004 B | R05, R09 | Pinned coherent provider loads existing BSP/PHY, compound prop, inside trace, verified impact, ragdoll and matching-schema restore | partial ([0004 progress](RFC/0004-progress.md)) |
| 20 / R20 | Parallel scheduler and controlled legacy bridge; 0003 C, 0006 M2 | R10 | Bounded queue/worker contracts, publication/wake/overflow and native stress pass; no forbidden helping/nested wait; total capacity and overhead measured | partial ([batch migration](RFC/0003-batch-migration-progress.md); [pool trust](RFC/0003-scheduler-trust-progress.md); [bounds, budgets](RFC/0003-scheduler-nodes-progress.md)) |
| 21 / R21 | Particle reference migration; 0003 D | R20 | Legacy/serial/parallel captured outputs agree; attachment/lifetime tests, improvement and small-workload budgets pass; quiescent rollback works | partial ([batch migration](RFC/0003-batch-migration-progress.md)) |
| 22 / R22 | Hammer persistence slice; 0002 H2 | R11, R13 | Declared VMF features round-trip and compile; independent acceptance, unknown/loss reporting, detached import and save failure/recovery pass | partial ([0002 current state](RFC/0002-progress.md#current-state-2026-09-25)) |
| 23 / R23 | Hammer application authority; 0002 H3 | R13, R22 | One selection/mutation/history owner; draft resolution, transform/cancel/undo/redo/save-position and generated sequences pass headlessly | partial ([0002 current state](RFC/0002-progress.md#current-state-2026-09-25)) |
| 24 / R24 | Hammer tools and presenters; 0002 H4 | R23 | Normalized traces share policies across entry points; two-document and close/focus/capture tests pass; no widgets in tools/presenters | planned |
| 25 / R25 | GTK editor workflow; 0002 H5 | R17, R22, R24 | Open/edit/undo/save/reopen/compile/run with multiple views, inspector and textures; declared fidelity and no hidden MFC runtime dependency | planned |
| 26 / R26 | Remaining foundation providers; 0001 rank 12 | R10, R11 | Native clock/thread/memory/process/environment/paths/diagnostics suites pass for supported profiles, including failure and cleanup | planned |
| 27 / R27 | Vulkan compatibility waypoint; 0001 rank 13 | R10, R16, R18 | Deployment, shader artifacts, profile selection and SDL3 presentation proven by a measured compatibility experiment; limitations recorded | partial ([Portal slice](RFC/0001-portal-vulkan-progress.md)) |
| 28 / R28 | Native Vulkan bootstrap; 0001 rank 14 | R10, R16, R18 | Native adapter/device/queues and SDL3 bridge present smoke frame; required-profile failure and validation diagnostics work | partial ([native Vulkan slice](RFC/0001-native-vulkan-progress.md)) |
| 29 / R29 | Four-platform architecture proof; 0001 rank 15 expanded to Linux/macOS/iOS/Android | R12, R18, R26, R28 | Each target passes foundation and SDL3/Vulkan native lifecycle smoke; Apple portability, iOS static composition and mobile packaging demonstrated; headless roles tested where declared | planned |
| 30 / R30 | Existing parallel kernels; 0003 E | R21 | Each bones/query-cache/entity-packing/leaf/shadow cohort independently passes three-mode, ownership, latency/performance and rollback gates | partial ([batch migration](RFC/0003-batch-migration-progress.md)) |
| 31 / R31 | Physics core compatibility; 0004 C | R19 | Required traces, filters, events, materials, constraints/ragdolls, controllers and persistence pass client/dedicated gameplay corpus | partial (provider level; [0004 progress](RFC/0004-progress.md)) |
| 32 / R32 | Native Vulkan functional MVP; 0001 rank 16 | R10, R28 | Representative map renders opt-in; resource/pipeline/upload/sync/swapchain contracts pass; unsupported features fail explicitly | active ([video options](RFC/0001-native-vulkan-video-options-progress.md); [queued rendering](RFC/0001-native-vulkan-queued-rendering-progress.md)) |
| 33 / R47 | PBR material family core; 0007 A/D | R02, R15 | BRDF analytic and white-furnace tests; `pbr` pixel family matches Cycles references; negative controls fail; capability and validated fallback on D3D9/DXVK | active ([0007 progress](RFC/0007-progress.md)) |
| 34 / R65 | Runtime antialiasing: 4x MSAA targets, alpha to coverage, PBR specular AA; 0012 A0–A3, A5 | R02, R32, R47 | Edge/alpha/shimmer/identity oracles with negative providers pass; glass keeps scene depth under MSAA; per-profile target memory policy and 4x recommendation follow measured Linux and Fold7 budgets | planned ([RFC 0012](RFC/0012-antialiasing-msaa-specular-alpha-coverage.md)) |
| 35 / R48 | Compile tools on Waf and bake seam; 0007 B, vvis track | R01, R02, R03 | vbsp/vvis/vrad build on a declared profile; byte-identical legacy lumps and PVS vs legacy executables; shared baker suite passes the legacy provider and rejects bad providers | partial ([Linux compiler host smoke](RFC/0007-progress.md#r48-host-compiler-preparation-2026-09-23); [R48-BAKER](RFC/0007-progress.md#r48-baker-light-baker-seam-and-pipeline-consolidation) planned) |
| 36 / R53 | BSP2 container and map-reader seam; 0008 F1 | R02, R04 | Legacy lumps carried byte-identically; client/server load both containers; independent reader, fuzzing and dedicated-server link evidence pass | active ([0008 progress](RFC/0008-progress.md)) |
| 37 / R55 | KTX2 textures; 0008 F3 | R15, R53 | Container-neutral texture reader; UASTC encode and per-profile transcode; native Vulkan BC/ASTC/ETC2 formats; `ktx validate` and per-format pixel fixtures; missing required format fails composition | partial ([KTX2 host-tool feasibility](RFC/0008-progress.md#f3-ktx2-host-tool-feasibility-2026-09-23)) |
| 38 / R66 | Offline texture filtering: normal-variance roughness and coverage-preserving alpha mips; 0012 A4 | R55, R65 | KTX2 writer bakes both with pairing validation and a filter version in metadata/cache keys; distance shimmer and alpha-coverage oracles improve over runtime-only | planned ([RFC 0012](RFC/0012-antialiasing-msaa-specular-alpha-coverage.md)) |
| 39 / R54 | Compiled USD World Stage and lightmap charts; 0008 F2 | R48, R53 | vbsp2 emits the geometry layer with charts; `usdchecker` clean; stage renders in pinned Cycles; semantic comparator detects seeded loss; face-ID-free compiled fixture validates; native USD authoring remains R59–R60 | partial ([0008 progress](RFC/0008-progress.md#f2-compiled-world-geometry-slice-2026-09-23)) |
| 40 / R49 | Cycles light baker; 0007 C/E | R05, R48, R54 | Feasibility decision recorded; SH L1/RNM, probe and reflection outputs pass analytic, comparative and negative oracles | planned |
| 41 / R56 | BSP2 native Vulkan world path; 0008 F4–F5 | R32, R49, R54, R55 | World mesh uploaded without rebuild; style-layer lightmaps, probe volume and clustered dynamic lights; each engine feature cohort passes; legacy payload renders on D3D9/DXVK | partial ([WMSH with Cycles preview light](RFC/0008-progress.md#f4-world-stage-geometry-and-cycles-light-in-the-playable-wmsh-view-2026-09-23)) |
| 42 / R50 | Image-based lighting; 0007 F | R47, R56 | Baked reflection probes and legacy runtime prefilter pass IBL pixel fixtures and cache invalidation; legacy families unchanged | partial ([R50-PARALLAX](RFC/0007-progress.md#r50-parallax-parallax-corrected-blended-reflection-probes-bounded-r50-slice-2026-09-25)) |
| 43 / R51 | Stage reference rendering; 0007 G | R49, R54 | Versioned Cycles reference fixtures rendered from stages; seeded material-mapping error detected | planned |
| 44 / R52 | Hammer compile/preview and vvis job graph; 0007 H | R20, R25, R49 | GTK compile/run and progressive preview with cancellation/recovery; serial/parallel/legacy PVS byte equivalence | planned |
| 45 / R57 | Incremental map build graph; 0008 F6 | R52, R54 | Cache-hit traces per change class and source-producer identity; cancellation leaves the previous package intact; native USD inputs extend the graph under R59 | planned |
| 46 / R59 | USD-native map schema and compiler; 0009 U0–U2 | R05, R48, R53, R54 | A hand-authored USD room compiles without VMF or prior BSP and runs in client/server; world, static, dynamic and physics roles validate distinctly; collision/visibility and negative fixtures pass | active (U0 authoring profile, fixture room and validator; U1 first slice: the U0 room compiles to BSP2 without VMF or a prior BSP and boots in dedicated, client and headless native Vulkan; U2 slice: static, dynamic and physics props, a trigger with I/O and a `func_movelinear` compile to their own contracts and behave at runtime headless, 2026-09-25; [RFC 0009 progress](RFC/0009-progress.md#u2-prop-role-cohorts-and-geometric-entities-slice-done-2026-09-25)) |
| 47 / R60 | USD-native editor workflow and VMF migration; 0009 U3–U4 | R13, R25, R59 | USD owns save/reopen/history and compile; role-aware block/mesh/prop editing, material/light viewport, object-linked diagnostics, edit-to-preview budgets, two-document workflow, external edit, import loss reports and installed product gates pass | planned ([RFC 0009](RFC/0009-usd-native-map-authoring.md)) |
| 48 / R61 | Modern map spatial/gameplay data; 0008 F8 | R31, R53, R59 | Versioned USD-native geometry/collision/visibility payload passes client/server semantic and malformed-input suites; legacy BSP bytes and behavior remain compatible | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 49 / R62 | Modern model asset path; 0008 F9 | R47, R55, R59 | Authored/compiled model assets serve static, dynamic and physics roles with materials, LOD, collision and required animation; MDL corpus and lifecycle gates pass | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 50 / R63 | Modern visual parity and geometry scalability; 0008 F10 | R47, R50, R56, R61, R62 | Representative USD maps pass registered material, reflection, transparency, shadow, lighting and dense-geometry oracles; real-time GI and geometry-scaling methods meet per-profile image, frame-time and memory budgets | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 51 / R64 | Direct USD development-runtime iteration; 0008 F11 | R56, R59, R60 | Desktop edit/reload/play retains authored identities and runtime parity; mobile opt-in decision follows measured package, startup, memory and lifecycle results; compiled-package path remains supported | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 52 / R58 | Mobile map packages; 0008 F7 | R29, R55, R56 | Device format queries recorded; per-profile packages pass installed-package smoke tests on R29 runners | planned |
| 53 / R33 | Hammer feature families; 0002 H6 | R25 | Each declared displacement/instance/manifest/overlay/texture/preview family passes load/edit/undo/save/build, recovery and performance gates | planned |
| 54 / R34 | Physics gameplay and tool completion; 0004 D | R31 | Required fluids, vehicle modes, pulley/group and selected Portal/game features pass; compiler/content workflows preserve supported formats | partial (provider level; [0004 progress](RFC/0004-progress.md)) |
| 55 / R35 | New audited compute seams; 0003 F | R30 | Animation/render-list/AI/streaming cohorts have stable inputs, correct cross-system edges and ordered commit; individual equivalence/budget gates pass | planned |
| 56 / R36 | Vulkan parity and four-platform release readiness; 0001 rank 17 | R29, R32 | Per-platform materials/images, loss/recovery, cache, hardware budgets and normal package/store-compatibility checks pass; default selection is a separate product decision | planned |
| 57 / R37 | Physics parallel rollout and default gate; 0004 E | R20, R34 | Worker-count determinism, nested-work/callback/shutdown bridge, platform packaging, budgets and supported client/server combinations pass; IVP rollback tested | planned |
| 58 / R67 | Opt-in Box3D capabilities; 0013 P0–P7 (phase prerequisites in the RFC) | R19 | Each capability has its own interface, contract with bad providers and required gate in `physics-v1.json`; parallel step passes on declared profiles; game opt-in only after its filter-threading policy and gameplay corpus pass | active ([0013 progress](RFC/0013-progress.md); P0–P3 on Linux desktop) |
| 59 / R38 | Stateful scheduling migrations; 0003 G | R30, R35, R37 | Snapshot-send ownership and selected entity/physics cohorts preserve legacy observations/order or record intentional change; network/latency/lifetime gates pass | planned |
| 60 / R39 | First-party module retirement; 0001 rank 18 / retirement B–D | R12, R18 | Pseudo-modules removed; mandatory systems and provider catalogs use typed linked factories; no filename/string discovery for migrated services | active ([Phase D](RFC/0001-phase-b-progress.md#later-work-not-claimed-here)) |
| 61 / R40 | Tool executable/process cleanup; 0001 rank 19 / retirement E | R11, R12, R22 | Launchable-DLL wrappers retired by cohort; structured argv/process protocol, outputs/cancellation and required compiler workflows pass; integrations tool-only | active ([Phase E](RFC/0001-phase-e-progress.md)) |
| 62 / R41 | Extension hosts and public-loader removal; 0001 rank 20 / retirement F–G | R07, R11, R39, R40 | Family-owned versioned ABI/trust/lifetime fixtures pass; Waf enumerates boundaries; only approved hosts load; Tier1/filesystem general loader APIs retired | planned |
| 63 / R42 | Scheduler consolidation; 0003 H | R35, R38 | Redundant queues/waits have zero consumers; process worker budget controlled; supported host modes retain correctness/latency and rollback evidence | planned |
| 64 / R43 | Hammer legacy retirement; 0002 H7 | R33, R60 | Declared product parity/recovery gate met; old consumer counts zero; superseded shell/glue/build references and stale exceptions removed | planned |
| 65 / R44 | IVP simulation retirement; 0004 F first gate | R37 | Declared profiles no longer depend on IVP simulation; gameplay/save/package gates pass and rollback/support decision recorded; decoder dependency remains explicit | planned |
| 66 / R45 | Independent collision decoding/cooking; 0004 F second gate | R40, R44 | Legacy/native format corpus and tool compatibility pass without IVP code; dependency audit clean; schema and old-content policy explicit | planned |
| 67 / R46 | Tier-global/domain retirement; 0001 rank 21 | R39, R41, R42, R43, R45 | All declared domain cohorts use explicit ownership; old globals have zero consumers; cohesive targets pass architecture/product gates; tiers removed only when empty | planned |

R39–R46 describe completion gates, not a reason to retain dead code until late.
Delete each unused adapter/global/queue when its bounded cohort has passed its
own gate. Likewise R30/R33–R35 are portfolios of independently accepted slices;
no broad parallelization or feature-parity claim is granted by starting a row.

## Tracking and current evidence

North-star infrastructure work is tracked within the existing ranks:

- R01/R03: define all four platform build profiles and establish host/target
  toolchains early, including Apple SDK and Android cross-build probes. Do not
  wait for the entire Linux renderer before testing mobile build feasibility.
- R04/R06: enforce iOS static first-party composition and explicit factories as
  an early bounded slice; do not defer it until whole-tree module retirement.
- R10/R20/R21: establish serial graph correctness, then bounded parallel execution
  and the measured pilot; include low-capacity and mobile lifecycle cases.
- R18/R28/R29: deliver SDL3/Vulkan and separately record Linux, macOS, iOS, and
  Android evidence. One additional-OS smoke test does not close the expanded R29.
- R36: close each platform's runtime, performance, package, and distribution
  checks. Signing credentials gate the corresponding release step only; no
  custom security infrastructure is required. These additions mark no gate done.

When starting work, change its row to `active` and record the bounded scope and
owner in the relevant domain progress record (create one when first needed).
For `blocked`, record the missing dependency/toolchain/content/decision and what
can still proceed. For `done`, link the current revision/profile evidence and
confirm every required child and exit criterion. Reopen affected gates when a
contract/provider/comparator/consumer changes; do not preserve stale completion.
Keep the table concise and link details below or from the domain progress file.

- R67 (RFC 0013): added 2026-09-24 at the user's direction, `active`. It is
  in the `box3d-optin` worktree. Box3D-only behavior is exposed as opt-in
  capabilities, each a versioned interface reached through
  `IPhysics::QueryInterface` with its own gate. The IVP-parity contract is
  unchanged.
  - P0–P1 done on Linux desktop:
    - `vphysics.parallel-step.v1`;
    - the `--bench` scenes, `physics_bench.py` and
      `quality/budgets/physics-v1.json`;
    - `box3d-parity` and `parallel-step` pass, and the injected faults are
      detected;
    - `ccd-bullets` is planned and fails: Box3D lets 56 of 64 projectiles
      through thin dynamic panes; IVP 24.
  - P2 done on Linux desktop:
    - worker tasks run on the pool the root passes in (the engine `CmpJob`
      pool in products), so the provider starts no threads;
    - `vphysics.step-profile.v1`;
    - `job_scheduler.h` deleted;
    - 48 contract checks, including a nested pool job and a stopped pool;
    - TSan-clean on the contract, scaled scenes and the whole parity suite at
      4 workers.
  - P3 done on Linux desktop:
    - `tools/quality/physics_filter_audit.py` checks all 61 game-filter
      sites, in CI and as a baseline check; policy (a), serialized on a
      worker;
    - server opt-in: `-physics_workers N` and `-physics_workers_required`;
    - live Portal at 4 workers ran the game's filter 1,114 times on pool
      threads;
    - the parity suite at 4 workers matches its serial observations bitwise.
  - Parallel stepping is the server default (user decision, 2026-09-25):
    auto workers from the compute pool, with `-physics_workers 1` to opt out.
    Unmeasured on the Fold7 and Apple profiles. It applies only where Box3D is
    selected (`./play`, `./play_p2`, `run.sh`). The launcher, the dedicated
    server, Waf and the Android APKs default to IVP.
  - `vphysics.shape-inertia.v1` (user decision, 2026-09-25): objects take
    their collision solid's full inertia tensor (Box3D hull mass data,
    products of inertia, no `rotInertiaLimit`) instead of IVP's per-axis
    approximation. The legacy model stays the default and is unchanged.
    - 20 `inertia.*` contract checks pass, and 4 injected faults are
      detected; the `shape-inertia` gate is required.
    - Game opt-in: `-physics_shape_inertia`, on by default in `./play` and
      `./play_p2` (user decision; `PHYSICS_ARGS=` rolls back).
    - Open: no gameplay corpus under the model. Box3D's backward-Euler
      gyroscopic step makes a fast top sink at the game tick, so
      `gyro.gyroscope-stays-level` fails under `--candidate-shape-inertia`.
  - The `parallel-step` gate passes again (2026-09-25).
    `parallel.speedup-pile-1024` is judged over awake steps (per-tick
    `AWAKE` from the bench, `awake_min` 512): 2.02x. The whole-run p50 had
    timed sleeping steps after the restitution pin. All 15 faults are
    detected.
  - Not done:
    - the parity runner's verdict fails: 602/602 checks pass but
      `dynamics.tumble.audible-impacts` diverges (2026-09-25);
    - the `box3d` restitution patch is pinned in the fork `johnpanos/box3d`
      (`78c90a0`, 2026-09-25), and with it three Box3D gameplay checks fail;
    - pool-bridge timing on the merged code is uncertified (busy host);
    - the client environment has one worker;
    - no gameplay soak;
    - no Android or Apple measurements;
    - no CI lane for the benchmark itself.

    This closes no R37 criterion. See the
    [RFC 0013 progress record](RFC/0013-progress.md).

- R47–R58 (RFC 0007/0008): added 2026-09-22 as `planned` at the user's direction,
  ranked after R32 so legacy-content fidelity on native Vulkan stays ahead. The user
  authorized upgrading the renderer, BSP and intermediate formats and chose OpenUSD for
  the World Stage. R53 (F1) is `active` as a bounded prototype (2026-09-22):
  - Done: the container library, the engine/filesystem seam, the independent
    Python reader, the `world.map-container` suite and a 54-map byte-identical
    corpus.
  - The Portal client and listen server load BSP2 maps. Game lumps and the pak
    lump match legacy, and DXVK frames match within run-to-run noise.
  - The dedicated-server and broader content gates remain open; see the
    current evidence and limitations in [RFC 0008 progress](RFC/0008-progress.md).
  - R54 has a compiled World Stage, a Portal-material Cycles preview and a
    4096×256 linear atlas prototype with all 16 fixture charts lit. A
    BSP2 map boots in native Vulkan with Portal materials. A preview bridge
    places the Cycles atlas in the legacy flat lightmap channel for the
    compiled style 32, now carried by the Stage's typed light API; the in-game
    frame changes while executable and material hashes stay fixed.
    Canonical RNM/SH lighting and BSP2 render lumps remain open. That bridge
    is the VMF path; PBRT/USD maps carry their Cycles atlas in `LMAP`
    directly.
  - Since then (2026-09-25): WMSH draws by default (`r_worldmesh_draw 2`) with
    cone and occlusion culling; `RPRB`, `PRBV`, `RTRN` and `SDFV` lumps exist.
    Installed encodings differ from the RFC's plan (`LMAP` is one RGBA16F
    page, `RPRB` a raw atlas, WMSH keeps face IDs). See
    [RFC 0008 current state](RFC/0008-progress.md). R57–R58 remain planned.

- R48-BAKER: `planned` (2026-09-24, user direction). This is one
  `ILightBaker` seam with the legacy vrad and pinned standalone-Cycles
  providers, one shared suite with the five bad providers, and one owner for the
  RNM basis, SH L1 fit and KTX2 lighting encoding. The PBRT and World Stage
  scripts become callers of the baker.
  - Why: a 2026-09-24 audit found three Blender-driven bakers and duplicated
    basis constants and L1 fits. It also found cache keys that miss scene files
    and tool revisions, and steps that delete the previous package before
    rerunning. `pbrt_map_build.py` has since fixed its cache keys and keeps the
    previous outputs (`c1b67422`, `d112d001`); the duplicated basis and L1 code
    remain, and the baker-level requirement stands.
  - Done also requires complete cache keys, atomic output replacement, and a
    dirty-subset bake request for R52/R57.
  - No baker code is installed. See the
    [R48-BAKER record](RFC/0007-progress.md#r48-baker-light-baker-seam-and-pipeline-consolidation).
  - Interim hook (2026-09-25, user direction): `tools/quality/vrad_cycles.py`
    is a vrad drop-in. It runs vrad, then the Cycles relight of its BSP, so a
    regular vbsp/vvis/vrad compile feeds the Blender pipeline. It is not the
    seam and closes no R48 criterion
    ([record](RFC/0007-progress.md#regular-compile-hook-vrad_cyclespy-installed-2026-09-25)).
  - Cycles bakes default to the CPU (user decision 2026-09-25, `d57305aa`);
    `gpu` and `auto` are opt-ins.

- R65–R66 (RFC 0012): added 2026-09-24 as `planned` at the user's direction.
  R65 is ranked directly after R47: it is bounded, it closes the native
  `VK_UNIMPLEMENTED` alpha-to-coverage stub, and specular AA is cheapest to
  add before the R47 PBR fixtures settle. R66 follows R55, whose KTX2 writer
  it extends. MSAA itself exists (R32-VIDEO-OPTIONS P6); no MSAA frame has
  been measured on any profile, and mobile target policy and defaults wait
  for A0 budgets and Fold7 measurements. Temporal AA stays out of scope.

- R47 energy compensation, shared BRDF and grouped descriptor sets
  (2026-09-25, user direction, Filament comparison follow-ups):
  - The PBR lobe is energy-compensated for multiple scattering. The rough
    white-metal furnace closes to 1.
  - `shaders/pbr_brdf.glsl` is the one GLSL BRDF. The `render.pbr-brdf.glsl`
    GPU suite matches it to `pbr_brdf.h` on 1,080 cases.
  - The PBR and GI stages bind three sets (frame, material, constants)
    instead of up to eleven. They pass their pixel suites as a four-set
    device (`.four-sets`).
  - Legacy LightmappedGeneric and `$phong` still need nine and seven sets.
  - Clear coat is on world and model PBR (`cd29e77a`); model glass is still
    dropped.
  - No Apple, Mali or Fold7 run. This closes no R29 criterion. See the
    [record](RFC/0007-progress.md#energy-compensation-one-glsl-brdf-and-grouped-descriptor-sets-r47--r29-prep-2026-09-25).

- R50-PARALLAX: `partial` (2026-09-25, user direction), a bounded R50
  slice. Scene maps carry automatically placed reflection probes in RFC
  0008's `RPRB` lump:
  - per room and per glossy surface, each with a box fitted to its own depth
    pass;
  - parallax-corrected, with distance-based roughness;
  - blended per pixel on native Vulkan, in world, glass and model PBR.
  - Sources: 3kliksphilip (2019), Lagarde and Zanuttini (SIGGRAPH 2012), and
    Frostbite 2014 notes for distance-based roughness.
  - A Python oracle, the C++ reader and the GLSL agree
    (`world.reflection-probes`, `render.reflection-probes.glsl`).
  - In-game gates against Cycles pass with negative controls:
    - mirror floor off-probe: blended 1.8x wall error, direction-only
      8x, a wrong box 5x;
    - walk through a doorway: weight step 0.047, while the nearest-capture
      control shows a 1.0 seam.
  - Not done: the legacy runtime prefilter, IBL pixel fixtures and cache
    invalidation; a KTX2 payload; mobile and Apple runs. R47 and R56 remain
    open. See the
    [record](RFC/0007-progress.md#r50-parallax-parallax-corrected-blended-reflection-probes-bounded-r50-slice-2026-09-25).
- R50-RELIGHT: `partial` (2026-09-25, user goal; RFC 0011 decision 5), a
  bounded R50 slice. Baked probes are relit, so runtime light reaches
  specular:
  - RPRB v2 relight bands (albedo, distance, normal) come from the probe
    faces' Diffuse Color, Normal and Depth passes;
  - `world_pbr` adds albedo times RFC 0011's change (the change volume and
    the unbaked lights' SDF-shadowed direct light) at the point each probe
    saw (after McAuley, Far Cry 4, GDC 2015, with the distance for shadows).
  - The Python, C++ and GLSL versions agree, with negative controls.
  - `mirror-lamp` in game: relit floor/wall 1.38 against Cycles; the probes
    as baked fail at 3.02. Cost 0.32 ms at 1080p (budget 0.5).
  - Not done: glass and model lookups; rough-lobe accuracy; mobile and
    Apple runs. See the
    [record](RFC/0007-progress.md#r50-relight-relightable-reflection-probes-bounded-r50-slice-2026-09-25).

- R70–R80 (RFC 0011): not yet ranked in the table; ranking is a user
  decision. On 2026-09-24/25 the user directed G0–G10 in order, and every gate
  has a done record on native Vulkan desktop
  ([gate status](RFC/0011-progress.md#gate-status)).
  - Deferred or unverified: the G1.7 DXVK capture; Apple (G8.3); the Fold7
    with grouped descriptor sets; G10 radiosity on an Android device. The
    Android soak was shortened to 5 minutes by the user. SDF and ray-query
    producers are declared unsupported on Android.
  - `gi_reference.py check` fails at HEAD: the six hand-built fixtures'
    references predate `980565cd`, and `portal-light`'s `room` camera has
    empty regions. The gallery and `swing` pass.
  - None of these rows can be `done` while its prerequisites (R20, R28, R29,
    R32, R47, R49, R53, R54) are open, so `partial` is the honest state once
    ranked.

- R59–R60 (RFC 0009): added 2026-09-23 as `planned` for USD-native map
  compilation and editing. R54's VMF-derived compiled World Stage remains a
  separate gate. R59 requires a playable map compiled from authored USD without
  VMF or a prior BSP; R60 requires USD to own editor persistence and a declared
  VMF import path, with role-aware tools and measured edit-to-preview behavior.
  Current USD previews do not satisfy either gate. `pbrt_map_build.py` accepts
  USD scenes but compiles them through a generated VMF, which is not R59's
  compiler. R59's compiler is `usd_map_compile.py` (U1 first slice,
  2026-09-25): staged, with `vbsp -authored` building brushes in memory.
  U2 (2026-09-25, profile and compile policy version 2): `prop_dynamic`,
  per-role model and collision checks, a static-prop bake policy, trigger
  touch filters, entity I/O by id and `func_movelinear`, each with its own
  compile-time and headless runtime oracle (`usd_map_runtime.py --roles`) and
  seeded mutants. The compiled World Stage and render payload for USD maps
  remain, as do R59's prerequisites.
- R61–R64 (RFC 0008 F8–F11): added 2026-09-23 as `planned` for versioned native
  map spatial data, a modern model asset path, visual parity and geometry
  scalability, and direct USD development-runtime iteration. They extend the
  Source 2-like outcome without marking F1–F7 or the current preview complete.

- Hammer scope (user direction, 2026-09-25): the goal is not a perfect or
  legacy-complete Hammer. "We need hammer and a game, and they both need to
  evolve together."
  - Prefer thin vertical slices where an editor capability lands with the map
    or gameplay that uses it: R17's renderer, a minimal R25 workflow, and
    R59/R60 USD author → compile → play.
  - Legacy-parity breadth (R08 inventory coverage, MFC parity, R33 feature
    families, R43 retirement) proceeds only when a game feature needs it.
  - Cheap enforcement (HAM002/HAM003, the module graph) stays. Row
    definitions are unchanged; this sets priority among ready work.
- R08: `active`. See the
  [RFC 0002 current state](RFC/0002-progress.md#current-state-2026-09-25).
  - The ledger has 28 migrations, 11 of them extracted format cores.
    Inventory coverage is 46 of 530 files. There are 60 Q-EDITOR suites.
  - `archlint hammer --verify` passes again (2026-09-25, user decision): the
    validator accepts Hammer edges to registered capability modules, such as
    the R47 schema's `hammer.formats` → `render.contracts`.
  - Strict Hammer include-graph checking (HAM002, include edges and cycles) is installed (R04-HAMGRAPH,
    2026-09-25). The VMF decoders moved from geometry to formats, which
    removed the geometry↔formats cycle. App → formats is a recorded
    exception owned by R22 until a `hammer.ports` persistence contract
    exists.

- R16: `done` (2026-09-22) for the rank 9 scope:
  - Contract: `public/render/render_presentation.h` (`render.presentation.v1`).
    Devices no longer present; pair-specific bridges do, with structured
    composition errors and `ReleaseDevice` ordering. Back buffers and swapchains
    retire only behind device completion tokens.
  - Pairs: headless-null (46 checks; 14 of 14 sensitivity defects detected) and
    SDL3–Vulkan. SDL3–Vulkan passes the same suite natively on isolated
    X11 (48 checks) and Wayland (46 checks, 2 recorded skips) profiles, with
    GPU completion held by a timeline semaphore and real two-window pixels.
  - Native handles confined: the Vulkan core and `shaderapivulkan.cpp` include
    no SDL. `SetMode`'s window reference is interpreted only by the bridge's
    legacy host. `architecture/modules.json` enforces this (seeded violations
    rejected). Portal native-Vulkan boot passes through the new host.
  - Prerequisite note: R16 uses only R14's surface-ownership slice, delivered
    here. R14's event/input contracts and SDL2 adapter remain `planned`.
  - Open: the D3D9/DXVK pair stays on the legacy `SetMode` membrane (no D3D9
    new-contract device); Android, macOS and iOS surfaces are unverified; no CI
    lane. The four Vulkan helper headers that archlint reported as `CAP002`
    from 2026-09-24 are registered in `render.vulkan.core` since R04-CAP
    (2026-09-25); the no-SDL rule still holds. Findings: Wayland cannot unminimize; hiding after a FIFO present kills
    the connection. See the [presentation bridge record](RFC/0001-presentation-bridge-progress.md).

- R01: `done` (2026-09-22) for the Q0 baseline and profile inventory:
  - One declaration, [`quality/baseline.json`](quality/baseline.json), holds
    host tools, content corpora, the 17-profile support matrix, 33 installed
    checks with recorded outcomes and owners, and the captures and budgets per
    domain. [`baseline.py audit`](tools/quality/baseline.py) reproduces it and
    fails on any deviation. It has 21 negative self-tests.
  - At `2b2ee370` plus a dirty tree, all 32 non-package checks matched: 24 pass,
    6 known fail, 2 known crash.
  - Recorded failures: ARCH105 drift (R04), 10 uninstrumented loader sites (R07),
    roadmap hard-gate violations for R15/R16, the dedicated-server compile error
    (R12), and `unittest_legacy` crashes on gcc (TSList) and clang (fixture path).
  - Unavailable here: gcc sanitizer runtimes, i386 multilib, MSVC, Xcode/MoltenVK,
    OpenUSD and KTX tools. No performance budget exists for any profile; each
    missing budget has an owning row.
  - This certifies no domain or platform gate. See the
    [Q0 record](RFC/0005-progress.md#q0--r01-baseline-and-profile-inventory).
    Reopen R01 when an audit deviates and the declaration is not reviewed.
  - Reopened and closed again 2026-09-25: the closing audit over all five
    groups has 41 checks (33 pass, 6 known fail, 2 known crash), with 0
    deviations and 0 unavailable.
    - Fixes: quality self-test drift, including USD scene oracles that had
      never passed under the pinned OpenUSD, and clang C++20 errors in the
      tools, including a real 64-bit pointer truncation in vrad.
    - Fixes: the unused-parameter and `dlsym` errors in the dedicated clang
      build, and a toolchain recorder gap. C sources in cxx-only targets
      escaped dialect checks, and `waf install` never re-recorded
      invocations.
    - User decisions: the dedicated builds are recorded as `pass`;
      `archlint hammer` accepts edges to capability modules; the Box3D
      restitution patch is pinned in the fork `johnpanos/box3d`;
      `physics.conformance` (R19) and `gi.references` (R70) are recorded as
      known fails. See the
      [re-audit](RFC/0005-progress.md#re-audit-2026-09-25).

- R15: `done` (2026-09-22) for the rank 8 scope:
  - Contracts: feature profile, quirks and structured selection errors
    (`render.profile.v1`).
  - A `LegacyRenderBackendProvider` around `IShaderDeviceMgr`, with backend
    adapter facts for null, D3D9 and native Vulkan.
  - Explicit profile requests from the launcher and dedicated roots.
  - Profile selection in material-system `Init`. The texture manager's
    `IsOpenGL()` test became a documented quirk.
  - Evidence: the shared suite passes against the real null and native Vulkan
    legacy modules and rejects bad legacy backends; the headless profile suite
    passes with sensitivity tests; the binding suite passes on DXVK and
    native-Vulkan trees; DXVK and null boots log the selected profile.
  - Unverified: the D3D9 module through the shared suite (it needs a composed
    material system), a togl profile, and a required CI lane. The fuller RFC 0001
    render completion (no process-global shader interfaces, step 8, R16) stays
    open.
  - See the [render seam record](RFC/0001-render-seam-progress.md).

- R01/R29-ANDROID-BUILD: `partial` (2026-09-22).
  - [`build-android-apk.sh`](build-android-apk.sh) builds the SDL3/native
    Vulkan Portal APK. Every input comes from pinned archives in the
    [Android profile](quality/product_profiles/portal-android-native-vulkan.json):
    NDK r30, SDL3, SDK platform and build-tools.
  - The script ends with the independent APK verifier
    [`android_apk.py`](tools/quality/android_apk.py). It checks the declared
    modules and ABIs, ELF machine type and 16 KB alignment, `DT_NEEDED`
    closure, the manifest, the touch assets, and `zipalign`/`apksigner`.
  - The verifier has 31 negative/positive fixture tests, and there are 12
    profile/manifest/touch-asset source guards. No NDK is needed; they run in
    `composition.yml`.
  - CI: [`android.yml`](.github/workflows/android.yml) builds every declared
    ABI. It has not run on hosted CI yet.
  - Clean-worktree builds of both ABIs pass locally.
  - The arm64 APK ran on a Galaxy Z Fold7, including fold and rotation.
  - Emulator smoke tests, surface recreation and store checks remain
    unverified. See the
    [profile record](quality/product_profiles/README.md#android-portal-sdl3native-vulkan-profile).

- R04-STYLE: `done` for the bounded mechanical-style slice requested by the user:
  pinned formatter, Source configuration, incremental include-order check,
  read-only CLI, and PR/master workflow are installed. The
  [style checker evidence and reproduction commands](tools/stylelint/README.md#ci-and-acceptance-evidence)
  cover positive/negative fixtures; local execution passed 29 tests on Python
  3.14.7 / clang-format 22.1.8. Hosted CI has not been executed here, and making
  its check required remains repository-administrator policy. This child does
  not close R04, establish C++20 target support, or certify runtime harnesses.

- R02: `done` (2026-09-22) for the RFC 0005 Q1 runner scope.
  - Runner: [`tools/quality/conformance.py`](tools/quality/conformance.py)
    runs `plan`/`check` against the manifest and writes
    `conformance-evidence/v2` with full per-suite logs.
  - Result record: every suite that expects `pass` must report exactly one
    `checks-v1` record through
    [`conformance_result.h`](public/testing/conformance_result.h). All 82
    suites were converted, so zero checks, a missing or duplicate record, and
    incomplete output all fail even with exit status 0.
  - Also fail correctly:
    - required providers that are unavailable (`optional` suites are skipped
      with a reason and never certified);
    - crashes, timeouts (the whole process group is killed), memory overruns,
      and `assert()`-based oracles;
    - zero discovery and partially unmatched selectors;
    - interrupted runs, which leave `incomplete` evidence.
  - Also supported: repeats and seeds.
  - Self-tests: 44 negative and positive runner self-tests (55 at
    2026-09-25).
  - Legacy hosts: `unittest_legacy`, `run_headless.sh` and `parity_wine.py`
    now fail on zero discovery.
  - Evidence (93/93 each):
    - g++ 16.2.1 and clang++ 22.1.8 in the default and `-O2 -DNDEBUG`
      configurations;
    - GCC 13.3 and Clang 18.1 in `ubuntu:24.04`.
  - Wine parity: 60/60.
  - CI: [`conformance.yml`](.github/workflows/conformance.yml) runs the matrix.
    It hasn't run on hosted CI yet, and making it required is administrator
    policy.
  - The gcc legacy `unittest_legacy` crash is a tier0 `CTSQueue` lock-free
    defect, triaged to R20.
  - Not delivered: native, GPU, device and sanitizer runner profiles, and any
    domain gate.
  - Since then (2026-09-25): the manifest has 172 suites (157 headless, 15 in
    the `linux-native-vulkan-gpu` profile). Hosted CI passed twice on
    2026-09-23; the three 2026-09-24 runs failed linking the BSP2 reader
    self-test before any suite ran. It passes locally at HEAD, which is not
    pushed. The gcc CTSQueue crash is fixed (R20); the remaining
    `unittest_legacy` crash is R07's fixture path.
  - See [Q1](RFC/0005-progress.md#q1--r02-runner-shared-conformance-runner).

- R20-BATCH / R21-PARTICLE / R30-BONES-PACKING: `partial`. A C++11 facade
  runs bounded C++20 job graphs on the existing engine pool. Particles,
  previous-frame/renderable bones and entity packing have serial and pooled graph
  paths, now **pooled by default** by user decision; the bone and renderable
  legacy gates also default on. Shared contract tests, mixed-dialect real-pool
  tests and Portal native smoke runs pass. Query-cache maintenance and Portal
  placement carving each share one kernel with their conformance suites and are
  proven output-equivalent to the original code. They stay default legacy
  because pooled measured no faster. The engine-pool fixtures are TSan-clean;
  the whole product has never run TSan-clean, and semantic gameplay captures
  and frame/performance gates remain open. The launchers pass
  `sv_querycache_job_graph 2` and `portal_carve_job_graph 2`. Commit
  `66e2a40c` moved six further call sites (leaf-system dispatches, shadow bone
  batches, snapshot send, nav visibility) onto pooled dispatch with no mode
  switch, oracle or record; snapshot send belongs to R38's scope. See the
  [scope, evidence, rollback and deferred consumers](RFC/0003-batch-migration-progress.md).
  Contract-preserving scheduler optimizations are
  [recorded with microbenchmarks and oracles](RFC/0003-scheduler-performance-progress.md).
  Examples: real-pool 1-worker 2048-item dispatch 38.8→4.4 µs, and Seal up to 42×
  faster. Q-JOBS has 16 manifest rows (14 required, 2 optional TSan). This sets no frame budget and closes no gate.

- R10-HOST-GRAPH / R20-POOL-TRUST: `partial` (2026-09-25). The host frame after
  admission runs as an ordered serial graph (one legacy node per phase,
  `host_frame_graph 1` by default, `0` = legacy rollback). A longjmp guard keeps
  `Host_EndGame`/`Host_AbortServer` exits off the executor. An oracle built from
  the pre-change body matches it over 4,000 seeded scenarios and detects every
  mutant. Live Portal captures of legacy and graph frames match; see the record
  for the tolerated `ia` noise.
  - The engine pool fixtures are TSan-clean. Real bugs fixed: the CThread
    init-flag race, the acquire-only `ThreadInterlockedExchange` (no release on
    ARM64), the `WaitForReply` race and the consumed exit event.
  - CTSQueue is a synchronized queue, so the gcc `unittest_legacy` CTSQueue
    crash is gone. Workers own bounded steal deques that spill to the shared
    queue.
  - A wait runs only the jobs it waits on (when eligible), never unrelated
    queued work.
  - Benchmarks against base `504ee284`: the pool runs its microbenchmark
    workloads in 0.44 to 0.50 of the old time (the old worker wait missed work).
    Wake latency is +5 µs. CTSQueue is 16% slower at 1:1 but no longer crashes
    at 4:4. In-game frames show no measurable difference (build B/A median
    1.008, within ±0.8 ms noise).
  - See the [scheduler trust record](RFC/0003-scheduler-trust-progress.md).
  - Follow-up (2026-09-24, [scheduler nodes record](RFC/0003-scheduler-nodes-progress.md)):
    - Profile first: the headless Portal main thread spends 4.8 of 7.4 ms/frame
      in draw submission; the whole server tick is 0.35 ms; particles and
      bones cost less than their pooled dispatch (0.07 ms).
    - The `Render` phase is 14 oracle-verified sub-nodes (120/120 step-order
      mutants detected); live legacy/graph captures match.
    - The `host_thread_mode 2` deadlock was a per-thread array overrun
      (`MAX_THREADS_SUPPORTED` 32 against thread ids up to 127, into the
      spatial partition's lock). Threaded Portal now runs; legacy and graph
      threaded captures are identical. The filesystem I/O pool's lost cap of 4
      is restored (it ran 32 threads here).
    - R20 items: bounded shared queue with a no-drop overflow policy,
      forbidden nested-wait and starvation detection, and
      [capacity/overhead budgets](quality/budgets/scheduler-v1.json) with a
      live census.
    - Declared frame graph: nodes ordered by declared resource access; the
      pooled executor overlaps host and pool work where they allow. The
      client render-start region (with the particle and bone cohorts as
      gather/batch/commit) runs on it under `cl_render_start_graph` (default
      0); its legacy order leaves no pair to overlap, which the oracle
      asserts.
    - Live shadow verifier (2026-09-25):
      - `cl_bone_setup_verify 1` (default off) reruns each pooled
        previous-frame bone batch serially from captured state and
        byte-compares the result. The frame keeps the pooled outputs.
      - In every dispatch mode, 3958 items were verified with 0
        mismatches. Seeded faults are caught in every batch. The cost is
        under 0.05 ms per frame.
      - It closes no gate. See
        [section 7](RFC/0003-scheduler-nodes-progress.md#7-live-shadow-verifier-previous-frame-bones-2026-09-25).
    - Open:
      - an executor that overlaps across several host nodes;
      - audited declarations for legacy blocks;
      - a TickServer split;
      - live oracles for the particle batch (proposed: fixed-seed twin
        effects) and the other cohorts;
      - mobile budgets;
      - full-product TSan.

- R32-DEBUG-CONTROLS: `planned` (2026-09-25, user direction). These are the
  `cl_vk_debug_*` and `cl_bsp2_*` controls of
  [RFC 0014](RFC/0014-native-vulkan-and-bsp2-debug-controls.md), in phases
  D0–D7. Each control ships with an oracle and a negative control. With all
  controls at default, shipped shader modules and pixels must be unchanged.
  `mat_indirect_view` and `VK_DEBUG_LIGHTMAPPED` move into the new owner and
  are deleted once their callers have moved. D6–D7 also support R53/R54/R56.
  Nothing is implemented yet, and this closes no other row's criterion.

- R32-RENDER-BUDGETS: `planned` (2026-09-25, user direction). This sets
  per-profile render budgets, which close the `presentation.frame-budgets`
  entry in [`quality/baseline.json`](quality/baseline.json). It is currently
  `missing`, and R32 owns it.
  - Method: RFC 0005's "Performance and promotion". Set absolute budgets
    and regression allowances per profile before measuring or optimizing.
  - Profiles: `portal-linux-wayland-native-vulkan` and
    `portal-android-native-vulkan` (Fold7) first. Apple rows stay
    unverified until R29.
  - Rows: profile × workload, starting with
    `quality/workloads/portal-frame-pacing-v1.json`. Each row covers frame
    time p50/p95/p99, GPU time, GPU memory and render-pass count. A small
    workload is included.
  - Record: `quality/budgets/render-v1.json`, checked by a budget script
    listed in `baseline.json`, like `gi.budgets`. Measurements come from
    `tools/quality/frame_pacing.py`.
  - Feature budgets stay with their owners: `indirect-light-v1.json`
    (RFC 0011), RFC 0012 A0 (MSAA) and RFC 0008 F10. The render file links
    to them and doesn't copy their numbers.
  - Per-pass GPU rows wait for `cl_vk_debug_gpu_timers` (R32-DEBUG-CONTROLS
    D4). The frame-level rows don't.
  - Done: both first profiles have recorded budgets and a passing check;
    the baseline entry is `recorded`; and an RFC 0001 progress record holds
    the measurement method and the reproduction commands.
  - How to split the frame's GPU time across features is out of scope. It
    needs its own decision.

- R32-QUEUED: `partial` (2026-09-25, user goal). Native Vulkan runs the queued
  material system (`mat_queue_mode 2`): the main thread builds the next frame
  while the `MatQueue` render thread replays the current one.
  - One owner of the mesh vertex record (`vulkan_mesh_layout`) for the mesh
    lock and `ComputeVertexDescription`; a CPU oracle (173 checks, packed-layout
    negative control) proves a queued replay equals a direct build.
  - The device follows the material system's thread ownership, with a
    cross-thread census; GPU compute is thread-safe; world-mesh and light-set
    calls reach the provider through frame-ordered queue adapters.
  - Fixed on the way: a 0-byte dynamic VB size that exhausted the call queue,
    and a GI brush relight that the queued material system dropped.
  - Evidence: 14 pixel families byte-identical to the pre-change backend;
    Portal maps and `gi_door` boot queued with 0 cross-thread calls;
    `gi_door` frames byte-identical across modes; frame pacing on a loaded
    host shows mode 2 at least as fast with a lower p99.
  - `run.conf` and `run.sh` pass `+mat_queue_mode 2`; `portal_boot.py`,
    `frame_pacing.py` and Android still pin 0.
  - A TSan run of the product tree found 46 signatures only in mode 2, each
    triaged in the record. `portal_boot --resize-stress` fails in both modes.
    No device-loss or mobile evidence. See the
    [queued rendering record](RFC/0001-native-vulkan-queued-rendering-progress.md).

- R32-VIDEO-OPTIONS: `partial` (2026-09-23). The Video options take effect on
  native Vulkan: real display modes and mode-change callbacks, vsync through
  `render.present-policy.v1`, brightness applied at present, DirectX 95 caps,
  real adapter identity with `dxsupport.cfg` recommendations shared with D3D9
  (0 mismatches over 676k cases), and MSAA with resolves. There are five new
  `render.contracts` owners with sensitivity rows. The GPU suites, the full
  pixel oracle at level 95, and DXVK regression boots pass.
  - Flashlight shadow depth (P7) is not started and stays reported unsupported.
  - Wayland FIFO under headless mutter can stall acquires. That predates this
    work; native now recovers through a 1 s acquire timeout, but it is
    unverified on a real session.
  - Native now honors the saved `mat_vsync 0` (no longer always FIFO), and
    re-autoconfigures once because the saved adapter IDs were 0.
  - See the [video options record](RFC/0001-native-vulkan-video-options-progress.md).

- R32-EMIT-PARALLEL: `partial` (2026-09-25). Emit's per-vertex conversion is
  81-82 % of `emit`, and draws of 1024+ unique vertices hold 83 % of it
  (`emit_convert` and per-size buckets in `-vkframestats`).
  `mat_vk_emit_parallel 1` converts those draws in chunks on the engine pool
  (`RunThreadPoolJobBatch`; legal from the main and `MatQueue` threads).
  - Default 0 (serial: oracle and rollback). On desktop it cuts the warm
    median to 0.70-0.78x and emit to 0.57-0.63x in modes 0 and 2, every
    round faster.
  - Oracles: in-game verify 0 of 21,000+ pooled draws mismatched; 14 pixel
    families byte-identical with one-vertex chunks; CPU fixture
    `render.vulkan.emit-convert-batch` (269 checks) with a TSan lane; seeded
    chunk-offset and shared-maximum defects detected.
  - Desktop launchers enable it: `run.conf`'s `JOB_ARGS` and `play_p2` pass
    `-vkemitparallel 1` (agent decision under the user's standing
    instruction, 2026-09-25). The engine default stays 0, and Android stays
    off until measured on the Fold7. Apple is unmeasured, and no full-product
    TSan run with it on exists. Merged into the shared tree from the worktree
    branch (`30068dd2`). The CPU suite (269 checks, g++ and clang++), the
    TSan lane and its race control pass there. See the
    [record](RFC/0001-native-vulkan-frame-pacing-progress.md#emit-conversion-on-the-engine-pool-r32-emit-parallel-2026-09-25).

- R32-FRAME-PACING: `partial` (2026-09-22). The Android portal stutter is
  reproduced on Linux by [`frame_pacing.py`](tools/quality/frame_pacing.py) with
  the [portal scenario](quality/workloads/portal-frame-pacing-v1.json) and the
  backend's `-vkframestats` stream. Native-backend fixes: emit rewrite with
  indexed draws, per-slot stream buffers (a real frame-overlap race), within-frame
  geometry reuse with a shadow verifier, a prewarmed pipeline store, and deferred
  texel uploads. Interleaved A/B warm median 18.4 -> 6.1 ms; material-pixel
  captures byte-identical. Android not measured. Mobile GPU cost (2026-09-23):
  sRGB/UNORM view breaks around color-masked draws are merged, so a linked-portal
  frame uses 32 -> 7 render passes, and all material-pixel families are
  byte-identical. On the Fold7 (Adreno 840, native 2448x1848) passes fall
  31 -> 5 but GPU time does not (7.6 vs 7.3 ms, within noise), so the modeled
  traffic saving does not hold for that driver. See the
  [frame pacing record](RFC/0001-native-vulkan-frame-pacing-progress.md#mobile-gpu-cost-render-pass-breaks-2026-09-23).

Current RFC 0001 evidence (updated 2026-09-25):

- [Native Vulkan slice](RFC/0001-native-vulkan-progress.md)
  ([current state](RFC/0001-native-vulkan-progress.md#current-state-2026-09-25))
  records R28/R32. Portal renders a lit, textured scene natively:
  `portal_boot.py --renderer native-vulkan` passes on testchmb_a_01,
  testchmb_a_08, escape_00 and escape_02.
  - Implemented: render targets, world texture residency, flat and bumped
    lightmaps, model lighting, integer HDR, PortalRefract, VGUI, fog,
    bloom/color correction and model shadows.
  - Declined by name: motion blur, water, eyes, teeth and flashlight. Line and
    point draws are dropped, alpha to coverage is a stub, and skinning runs on
    the CPU.
  - Oracles: `material_pixel_conformance.py` has 14 families, most judged
    against D3D9 references. Waf GPU suites: bring-up 98, material-facing 25,
    equivalence 64 checks. The 15 suites of the manifest's
    `linux-native-vulkan-gpu` profile pass. A `-vkvalidate` boot logs no
    messages.
  - The 84 legacy stdshader ports (R32-LEGACY-SHADERS) are not in this tree;
    they exist only as uncommitted changes in the `source-engine-vkshaders`
    worktree.
  - The 2026-09-22 false-positive correction is history. No hosted CI lane;
    Android and Apple are unverified.
- [Portal SDL3/Wayland/Vulkan progress](RFC/0001-portal-vulkan-progress.md) records
  the verified Linux compatibility slice, real images and GPU/lifecycle tests.
  DXVK Native retains the D3D9 material implementation; R28/R32 native Vulkan
  implementation and R29/R36 platform/release gates are not delivered by it.
- The linked renderer/null binding, source-matched shader build and opt-in render
  trace have outcome tests and deliberately bad fixtures. Product staging
  verifies dependency/source/artifact hashes and preserves the supplied runtime.
- Phase A/B progress records remain scoped completion evidence. R39 is partial:
  typed first-party composition has migrated caller cohorts. The standard
  shader library is now linked and bound by a typed descriptor, and the
  launcher picks window, input, video and audio providers from typed
  descriptors (2026-09-25). Physics and the filesystem are still loaded by
  filename, which prevents global Phase D closure. R40: the launchable-DLL
  wrappers and `ilaunchabledll.h` are deleted (`66e2a40c`) and the tool process
  contract is installed; its POSIX provider is not built or tested
  ([Phase E](RFC/0001-phase-e-progress.md)).

Initial evidence, observed at `87955f67` before these documentation changes:

- R01: existing checker fixtures report eight passes. Full loader check/baseline
  reports 10 new and 3 stale occurrences; inventory verification reports stale.
  This is an existing failure to reconcile through review, not permission to
  regenerate baselines automatically. Native product/harness gates are unverified.
- R07: [telemetry source](tier0/module_load_telemetry.cpp) and
  [request API](public/tier1/module_load_telemetry.h) exist. Required native,
  failure/lifetime and ABI evidence has not been established by this assessment;
  see [Phase A progress](RFC/0001-phase-a-progress.md).
- RFCs 0005/0006 and this roadmap define future implementation work. No runner,
  C++20 build migration, physics backend, editor port, or scheduler is delivered
  merely by adding these documents.
