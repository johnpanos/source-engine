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
| [0015](RFC/0015-asset-identity-content-build-graph.md) | Asset identity (`AssetRef`), one content build graph and compiler contract for every asset kind, legacy-format passthrough, per-profile packages with an asset index, runtime resolver, desktop live reload (proposed; rows R81–R85, R57 carries C2) |
| [0016](RFC/0016-render-core.md) | Render core as ports and adapters beneath the frozen material API: backend-neutral `render.device.v2` with Vulkan, OpenGL and null adapters, render graph, GPU scene and views, material families, clustered lights and shadow atlas, one legacy frontend running `IMatRenderContext`/`IShaderAPI` as graph passes; layers enforced by archlint CAP011; ToGL kept for mods (proposed; rows R86–R92) |

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

Keep command output bounded. Pipe noisy build, test and log commands through
`head`, `tail` or `grep`, and read large files with `offset`/`limit` ranges
instead of `cat`. Save full logs to a file when later inspection may need them.

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
`done`. `planned` does not imply dependency-ready. States were first set at
source revision `87955f67` and are kept current per row; documentation alone
marks no implementation gate done.

| Rank / ID | Work and RFC scope | Prerequisites | Done looks like | State |
| --- | --- | --- | --- | --- |
| 1 / R01 | Reproducible baseline and profile inventory; 0005 Q0, baseline portions of all domains | — | Current checks/failures recorded; exact build/content/tool availability and supported profiles established; baseline captures and budgets identified | done (re-audited 2026-09-25; [Q0 baseline](RFC/0005-progress.md#re-audit-2026-09-25)) |
| 2 / R02 | Trustworthy runner, fixtures, evidence; 0005 Q1 | R01 | Zero/missing tests, skips, crashes, timeouts and incomplete output fail correctly; explicit test composition and reproducible artifacts work | done ([Q1 runner](RFC/0005-progress.md#q1--r02-runner-shared-conformance-runner)) |
| 3 / R03 | Per-target C++20/toolchain boundary; 0006 M0 | R01, R02 | Compile/link/run proof; final flags verified; legacy/C17 settings and frozen-consumer ABI combinations preserved | done (2026-09-25 for the required Linux and Android profiles; Apple/MSVC optional by user decision; [R03 closure](RFC/0006-progress.md#r03-closure-2026-09-25)) |
| 4 / R04 | Full architecture and migration enforcement; 0001 rank 1, 0002 H0 enforcement, Q-ARCH | R01, R02 | Ownership, direct/transitive includes, Waf/link graph, hermetic builds, exact debt and evidence schemas enforced; negative projects fail | done (2026-09-25; every recorded target declares `arch_module` or is in a row-owned legacy group, with a build-time check and the `arch.targets` gate; include, hermetic, link-graph and iOS shared-library ratchets enforced; hosted CI not run; [closure](RFC/0001-phase-a-progress.md#r04-closure-done-2026-09-25)) |
| 5 / R05 | Results, IDs, quantities, ownership vocabulary; 0001 rank 2, 0006 M1 | R03, R04 | `Expected`, borrowing/scoped resources and matchers pass value/lifetime/ABI tests; a real consumer uses them | done (2026-09-25: `Expected`, compact errors (`foundation::Error`), `StrongId`, `ScopedResource`, `testing::Checks` and `units`, each with a suite, sensitivity rows and a real consumer; CAP010 keeps them out of preserved ABI headers; hosted CI not run; [closure](RFC/0006-progress.md#r05-closure-done-2026-09-25)) |
| 6 / R06 | Composition/lifecycle kernel and minimal test providers; 0001 rank 3, Q-FOUNDATION | R02, R05 | Unit runner composes typed providers without ambient factories; required/optional validation, failure-at-each-stage rollback and repeat-instance tests pass | done (2026-09-25: typed-descriptor unit runner, required/optional validation, rollback at every stage, repeat instances, ordering, legacy bridges and negative providers; fresh Q-FOUNDATION run on g++ and clang++; native providers and hosted CI not claimed; [conformance record](RFC/0001-conformance-progress.md)) |
| 7 / R07 | Loader containment, telemetry and ABI fixtures; 0001 rank 4 / retirement A | R04, R06 | Scoped ownership, structured errors, legacy bridge and fake/native suites pass; telemetry handles failed/duplicate/nested requests; reviewed ratchet/inventory current | done (2026-09-25: POSIX loader provider with a required load-site observer, scoped `LoadedLibrary`, provider events on the Tier 0 stream, fake and native suites, and legacy telemetry and frozen-ABI cases on gcc and clang; `Sys_*` stay the frozen instrumented bridge and retire with R39/R41 (RFC 0001 step-6 decision); Win32 and hosted CI not claimed; [closure](RFC/0001-phase-a-progress.md#r07-closure-done-2026-09-25)) |
| 8 / R08 | Hammer H0 corpus and migration inventory; 0002, Q-EDITOR/Q-CONTENT | R02, R03, R04 | Exhaustive ownership/callers and migration records; legacy build evidence/gaps; headless target; semantic comparator detects seeded data loss | active (map-building-loop slices done 2026-09-25/26: R08-CMD command layer, R08-LIBS layered format libraries, R08-LOOP Waf-built headless `hammer_cli` with the `corpus.hammer.loop` author → compile → boot suite, R08-UI-P1 Source 2 P1 commands and GTK wiring, R08-UI-TEST the `corpus.hammer.ui` suite that drives the real GTK editor in an isolated compositor, R08-MCP the command catalog as MCP tools (`hammer_cli --mcp`), R08-ASYNC-BUILD F9 compiles off the UI thread through `hammer::app::MapBuildQueue`; open for `done`: exhaustive ownership/caller inventory (38 of 531 files classified), complete migration records, legacy build evidence; [record](RFC/0002-progress.md#map-building-loop-direction-and-r08-cmd-2026-09-25)) |
| 9 / R09 | Physics A feasibility and IVP baseline; 0004, Q-PHYSICS | R01, R02, R05 | Method/profile inventory, units/assets and measurements; tested solution or explicit scope decision for impact state, contact mutation, ragdoll limits and hull/decoder blockers | partial ([0004 progress](RFC/0004-progress.md)) |
| 10 / R10 | Runner/clock/sequence contracts and serial graph; 0001 rank 11, 0003 A–B | R05, R06 | Virtual time and independent graph model; validation/publication/affinity/failure tests; ordered serial host graph matches legacy captures | done (2026-09-26: `platform.task-runner.v1` with virtual time, thread and sequence providers, diagnostic `SequenceChecker`, the engine `CThreadPool` as a runner, `ParallelExecutor` lane bindings, and a versioned legacy host-frame capture rechecked in both modes; budgets and mobile evidence stay with R20/R21/R26/R29; [closure](RFC/0003-progress.md#r10-closure-done-2026-09-26)) |
| 11 / R11 | Paths and module resolution; 0001 rank 5 | R05, R07 | Native/virtual paths distinct; resolution/verification separate from opening; encoding/search/failure corpus passes | partial (`paths.h` contract passes on the test backend, `platform.paths`; no native provider or corpus) |
| 12 / R12 | Dedicated-server composition; 0001 rank 6 | R06, R07, R11 | Installed startup/shutdown and partial failure pass; link/runtime evidence shows render and desktop UI absent | partial ([composition migration slice](RFC/0001-dedicated-composition-progress.md)) |
| 13 / R13 | Hammer geometry and scene seams; 0002 H1 | R05, R08 | Strict headless targets; geometry/reference/reparent tests and independent documents pass; selected legacy callers route through shared owner | partial ([0002 current state](RFC/0002-progress.md#current-state-2026-09-25)) |
| 14 / R14 | Window/input contracts and SDL3 provider; 0001 rank 7 | R06 | The shared window/input suite passes against the SDL3 provider and the fake backend; current product behavior captured and preserved; normalized events, optional behavior, surface ownership and input lifecycle conformance pass | partial (window/input contracts with a fake-backend suite, `platform.window` + sensitivity; no SDL3 window/input provider yet, and SDL3 products reach SDL through the `platform/sdl3/legacy_include` adapter; the SDL2 adapter is legacy-profile only (user decision, 2026-09-26); R16 delivered the surface slice) |
| 15 / R15 | Render seam, scoped legacy services and null provider; 0001 rank 8 | R06 | Explicit provider/caps/profile selection; null and legacy contract suites; material consumer tested without new shader globals | done ([render seam](RFC/0001-render-seam-progress.md)) |
| 16 / R16 | Pair-specific presentation bridges; 0001 rank 9, 0006 M3 | R14, R15 | Native handles confined; multi-surface resize/zero-size/loss/shutdown pass; delayed GPU completion prevents early reuse | done ([presentation bridges](RFC/0001-presentation-bridge-progress.md); R14 surface slice only) |
| 17 / R17 | Hammer real renderer feasibility; 0002 R1 | R08, R15, R16 | Source-material viewport on declared GTK X11/Wayland profiles; state/target restoration, scale, capture, sharing and teardown measured | partial ([R17-CORE](RFC/0002-progress.md#r17-core-the-gtk-viewports-on-the-render-core-slice-done-2026-09-28), 2026-09-28: the GTK viewports draw through the RFC 0016 core (`render.pass.lines`, `ViewportRenderer`); the GL renderer is deleted; `corpus.hammer.ui` judges the live frames; flat-shaded, no Source materials until K4; scale, capture, sharing and teardown unmeasured) |
| 18 / R18 | SDL3 provider completion; 0001 rank 10 | R14, R16 | The SDL3 window/input suites and representative behavior pass on every declared SDL3 profile; SDK dependency is private and SDL is off generic include paths; supported interop pairs tested; the SDL2 legacy profiles still build | partial ([Portal slice](RFC/0001-portal-vulkan-progress.md)) |
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
| 29 / R29 | Four-platform architecture proof; 0001 rank 15 expanded to Linux/macOS/iOS/Android | R12, R18, R26, R28 | Each target passes foundation and SDL3/Vulkan native lifecycle smoke; Apple portability, iOS static composition and mobile packaging demonstrated; headless roles tested where declared | partial (children R29-IOS-STATIC and R29-ANDROID-BUILD: iOS static build and iPhone runs, Android APK on the Fold7; no lifecycle gate, no macOS) |
| 30 / R30 | Existing parallel kernels; 0003 E | R21 | Each bones/query-cache/entity-packing/leaf/shadow cohort independently passes three-mode, ownership, latency/performance and rollback gates | partial ([batch migration](RFC/0003-batch-migration-progress.md)) |
| 31 / R31 | Physics core compatibility; 0004 C | R19 | Required traces, filters, events, materials, constraints/ragdolls, controllers and persistence pass client/dedicated gameplay corpus | partial (provider level; [0004 progress](RFC/0004-progress.md)) |
| 32 / R32 | Native Vulkan functional MVP; 0001 rank 16 | R10, R28 | Representative map renders opt-in; resource/pipeline/upload/sync/swapchain contracts pass; unsupported features fail explicitly | active ([video options](RFC/0001-native-vulkan-video-options-progress.md); [queued rendering](RFC/0001-native-vulkan-queued-rendering-progress.md)) |
| 33 / R47 | PBR material family core; 0007 A/D | R02, R15 | BRDF analytic and white-furnace tests; `pbr` pixel family matches Cycles references; negative controls fail; capability and validated fallback on D3D9/DXVK | active ([0007 progress](RFC/0007-progress.md)) |
| 34 / R86 | Render core prerequisites, device contract and Vulkan provider; 0016 K0–K1 | R02, R05, R10, R16 | Layer contract (archlint CAP011) rejects its seeded violations; `jobs.graph` module; pinned shader compiler; material/shader/studio headers in `legacyAbi.paths` with vtable fixtures; view and per-draw oracles captured; `render.device.v2` shared suite catches the bad providers; port headers reach no Vulkan/GL/SDL header; `CVulkanContext` allocates, uploads and retires through the Vulkan adapter; no idle waits on frame paths; pixel families byte-identical; per-profile feature support recorded | partial ([R86-LAYOUT](RFC/0016-progress.md#r86-layout-layout-layer-contract-and-runtime-wiring-2026-09-26); [K0 met](RFC/0016-progress.md#k0-prerequisites-and-frozen-oracles-2026-09-26): layer contract, `jobs.graph`, pinned shader compiler, frozen headers, 8 of 8 vtable fixtures, view and draw-state oracles, desktop and Fold7 budgets; K1: `render.device.v2` on the null and Vulkan adapters, 10 of 10 bad adapters, no single Vulkan stack yet; branch `render-core`) |
| 35 / R87 | Render graph and inversion; 0016 K2–K3 | R86 | Graph compiler agrees with the independent model and catches bad graphs; sync validation silent; legacy frontend records `IMatRenderContext` into stage passes; render `QueryInterface` side channels gone; pixel families, view oracles and per-draw fixtures byte-identical in both queued modes | partial ([R86-LAYOUT](RFC/0016-progress.md#r86-layout-layout-layer-contract-and-runtime-wiring-2026-09-26); [K2 core](RFC/0016-progress.md#k2-k5-k6-and-k3k9-tooling-core-slices-2026-09-28): aliasing, sync validator, pooled executor, transient pool, 5 of 5 bad graphs, graph suite on null and Vulkan; product passes on the graph and the K3 inversion open; [K3](RFC/0016-progress.md#k3-slice-3-stage-passes-and-the-queued-tsan-lane-2026-09-28), 2026-09-28: frames run as stage passes of the frame graph, side channels gone, record replay gone, pixels, views and draw state unchanged in both queued modes, K1 set, Portal 2 and resize boots pass, queued TSan lane clean against the triage list; open: frame time on a quiet desktop host and the Fold7) |
| 36 / R88 | Shader library and materials v2; 0016 K4 | R87 | Families with schemas and parameter blocks; `legacy`, `pbr`, `lightmapped`, `vertexlit` and `unlit` on the core with VMT importers; each matches its port; proxy corpus passes; four-group ceiling enforced | partial ([K4 groundwork](RFC/0016-progress.md#k4-shader-library-and-materials-groundwork-2026-09-28): per-target artifacts with pinned SPIRV-Cross, VMT import and corpus, material suite, legacy proxy captures; [2026-09-28](RFC/0016-progress.md#k4-slice-spir-v-headers-generated-by-the-build-2026-09-28): SPIR-V headers generated by the build, none committed; [`unlit` family](RFC/0016-progress.md#k4-family-slice-unlit-and-port-clause-d17-2026-09-28) matches its port within 2 levels, port clause D17; `lightmapped`, `vertexlit` (Hammer session) and `pbr`, and the proxy corpus's frontend side open) |
| 37 / R92 | OpenGL device adapter; 0016 K10 | R88 | OpenGL 4.5 adapter passes `render.device.v2` for every capability it claims, including the conventions section; no portable module changes or compares the backend identity; capability negotiation selects declared fallbacks or fails composition by name; pixel families within the recorded cross-backend tolerance; `portal-linux-gl` boots; ToGL legacy profiles unchanged | planned ([RFC 0016](RFC/0016-render-core.md)) |
| 38 / R89 | GPU scene, views, world, props and skinned models; 0016 K5–K6 | R88 | Scene change sets and snapshots; world, props and models drawn from the scene; culling matches legacy; compute skinning matches the CPU oracle; submission time improves against its budget; a second scene works in-process | partial ([K5, K6 slices](RFC/0016-progress.md#k2-k5-k6-and-k3k9-tooling-core-slices-2026-09-28): pooled culling, publication TSan lane, two scenes drawn on the GPU; GPU skinning matches studiorender's software skinning on a real-model corpus; world, props, product skinning, pixels and costs open) |
| 39 / R90 | Clustered lights and shadow atlas; 0016 K7 | R89 | Clustered lighting over `render.light-set.v1`; spot, flashlight and sun-cascade shadows with oracles; dlight behavior decision recorded; atlas budgets pass on desktop and the Fold7 | partial ([K7 headless slice](RFC/0016-progress.md#k7-headless-slice-clustered-light-assignment-and-shadow-atlas-2026-09-28): cluster assignment with zero false negatives over 1,000 scenes, shadow atlas, spot, flashlight and cascade views; GPU lane, pixel oracles, flashlight scene, dlight decision and budgets open) |
| 40 / R91 | Remaining render cohorts and legacy-stream retirement; 0016 K8–K9 | R89, R90 | Particles, decals, sprites/beams, post, UI, water, sky and portal/mirror/monitor views on the core; first-party legacy-stream use zero on native Portal/Portal 2 (ratchet); mod-style fixture renders through the frontend | partial ([K9 static ratchet](RFC/0016-progress.md#k3-and-k9-static-scans-842bb152): 490 first-party legacy-stream sites, shrink-only; no cohort moved) |
| 41 / R65 | Runtime antialiasing: 4x MSAA targets, alpha to coverage, PBR specular AA; 0012 A0–A3, A5 | R02, R32, R47 | Edge/alpha/shimmer/identity oracles with negative providers pass; glass keeps scene depth under MSAA; per-profile target memory policy and 4x recommendation follow measured Linux and Fold7 budgets | planned ([RFC 0012](RFC/0012-antialiasing-msaa-specular-alpha-coverage.md)) |
| 42 / R48 | Compile tools on Waf and bake seam; 0007 B, vvis track | R01, R02, R03 | vbsp/vvis/vrad build on a declared profile; byte-identical legacy lumps and PVS vs legacy executables; shared baker suite passes the legacy provider and rejects bad providers | partial ([Linux compiler host smoke](RFC/0007-progress.md#r48-host-compiler-preparation-2026-09-23); [R48-BAKER](RFC/0007-progress.md#r48-baker-light-baker-seam-and-pipeline-consolidation) planned) |
| 43 / R53 | BSP2 container and map-reader seam; 0008 F1 | R02, R04 | Legacy lumps carried byte-identically; client/server load both containers; independent reader, fuzzing and dedicated-server link evidence pass | active ([0008 progress](RFC/0008-progress.md)) |
| 44 / R55 | KTX2 textures; 0008 F3 | R15, R53 | Container-neutral texture reader; UASTC encode and per-profile transcode; native Vulkan BC/ASTC/ETC2 formats; `ktx validate` and per-format pixel fixtures; missing required format fails composition | partial ([KTX2 host-tool feasibility](RFC/0008-progress.md#f3-ktx2-host-tool-feasibility-2026-09-23)) |
| 45 / R66 | Offline texture filtering: normal-variance roughness and coverage-preserving alpha mips; 0012 A4 | R55, R65 | KTX2 writer bakes both with pairing validation and a filter version in metadata/cache keys; distance shimmer and alpha-coverage oracles improve over runtime-only | planned ([RFC 0012](RFC/0012-antialiasing-msaa-specular-alpha-coverage.md)) |
| 46 / R54 | Compiled USD World Stage and lightmap charts; 0008 F2 | R48, R53 | vbsp2 emits the geometry layer with charts; `usdchecker` clean; stage renders in pinned Cycles; semantic comparator detects seeded loss; face-ID-free compiled fixture validates; native USD authoring remains R59–R60 | partial ([0008 progress](RFC/0008-progress.md#f2-compiled-world-geometry-slice-2026-09-23)) |
| 47 / R49 | Cycles light baker; 0007 C/E | R05, R48, R54 | Feasibility decision recorded; SH L1/RNM, probe and reflection outputs pass analytic, comparative and negative oracles | planned |
| 48 / R56 | BSP2 native Vulkan world path; 0008 F4–F5 | R32, R49, R54, R55, R90 | World mesh uploaded without rebuild; style-layer lightmaps, probe volume and clustered dynamic lights; each engine feature cohort passes; legacy payload renders on D3D9/DXVK | partial ([WMSH with Cycles preview light](RFC/0008-progress.md#f4-world-stage-geometry-and-cycles-light-in-the-playable-wmsh-view-2026-09-23)) |
| 49 / R50 | Image-based lighting; 0007 F | R47, R56 | Baked reflection probes and legacy runtime prefilter pass IBL pixel fixtures and cache invalidation; legacy families unchanged | partial ([R50-PARALLAX](RFC/0007-progress.md#r50-parallax-parallax-corrected-blended-reflection-probes-bounded-r50-slice-2026-09-25)) |
| 50 / R51 | Stage reference rendering; 0007 G | R49, R54 | Versioned Cycles reference fixtures rendered from stages; seeded material-mapping error detected | planned |
| 51 / R52 | Hammer compile/preview and vvis job graph; 0007 H | R20, R25, R49 | GTK compile/run and progressive preview with cancellation/recovery; serial/parallel/legacy PVS byte equivalence | planned |
| 52 / R81 | Asset identity, asset index and content build graph core; 0015 C0–C1 | R02, R05, R10 | `AssetRef` and kind table; package index with an independent reader and Portal/HL2 closure reports; compiler contract whose shared suite catches the bad compilers; content store, atomic publication, serial and pooled executors and traces; incremental builds equal clean builds and change-class traces pass | planned ([RFC 0015](RFC/0015-asset-identity-content-build-graph.md)) |
| 53 / R57 | Map builds on the content build graph; 0008 F6, 0015 C2 | R52, R54, R81 | Cache-hit traces per change class and source-producer identity; cancellation leaves the previous package intact; native USD inputs extend the graph under R59; `usd_map_compile`, `vmf_map_build` and `pbrt_map_build` private caches and two-rename publication removed | planned |
| 54 / R59 | USD-native map schema and compiler; 0009 U0–U2 | R05, R48, R53, R54 | A hand-authored USD room compiles without VMF or prior BSP and runs in client/server; world, static, dynamic and physics roles validate distinctly; collision/visibility and negative fixtures pass | active (U0 authoring profile, fixture room and validator; U1 first slice: the U0 room compiles to BSP2 without VMF or a prior BSP and boots in dedicated, client and headless native Vulkan; U2 slice: static, dynamic and physics props, a trigger with I/O and a `func_movelinear` compile to their own contracts and behave at runtime headless, 2026-09-25; [RFC 0009 progress](RFC/0009-progress.md#u2-prop-role-cohorts-and-geometric-entities-slice-done-2026-09-25)) |
| 55 / R60 | USD-native editor workflow and VMF migration; 0009 U3–U4 | R13, R25, R57, R59 | USD owns save/reopen/history and compile; role-aware block/mesh/prop editing, material/light viewport, object-linked diagnostics, edit-to-preview budgets, two-document workflow, external edit, import loss reports and installed product gates pass | planned ([RFC 0009](RFC/0009-usd-native-map-authoring.md)) |
| 56 / R83 | Runtime asset index and resolver; 0015 C5 | R81 | Packages mount their index; `CTexture` selects VTF/KTX2 variants through it; missing references name their referrer; closure prefetch matches serial load; dedicated server links no render dependency | planned ([RFC 0015](RFC/0015-asset-identity-content-build-graph.md)) |
| 57 / R82 | Legacy compiler adoption; 0015 C3 | R03, R81 | `studiomdl` and `captioncompiler` build under Waf; a `scene.image` compiler drives the `scenes.image` writer extracted into `content.scene-image`; nav generation and `sound.cache` are graph nodes; each kind's output matches its legacy corpus and loads in client and dedicated products | planned ([RFC 0015](RFC/0015-asset-identity-content-build-graph.md)) |
| 58 / R61 | Modern map spatial/gameplay data; 0008 F8 | R31, R53, R59 | Versioned USD-native geometry/collision/visibility payload passes client/server semantic and malformed-input suites; legacy BSP bytes and behavior remain compatible | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 59 / R62 | Modern model asset path; 0008 F9 | R47, R55, R59, R81 | Authored/compiled model assets serve static, dynamic and physics roles with materials, LOD, collision and required animation; MDL corpus and lifecycle gates pass | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 60 / R63 | Modern visual parity and geometry scalability; 0008 F10 | R47, R50, R56, R61, R62 | Representative USD maps pass registered material, reflection, transparency, shadow, lighting and dense-geometry oracles; real-time GI and geometry-scaling methods meet per-profile image, frame-time and memory budgets | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 61 / R85 | Desktop live-reload loop; 0015 C6 | R81, R83 | Watcher, overlay package and change notices; material/texture, then model/particle/sound reloaders pass the reload-equals-cold-start oracle and their latency budgets; installed and mobile products contain no watcher, compiler or reload channel | planned ([RFC 0015](RFC/0015-asset-identity-content-build-graph.md)) |
| 62 / R64 | Direct USD development-runtime iteration; 0008 F11 | R56, R59, R60, R85 | Desktop edit/reload/play retains authored identities and runtime parity; mobile opt-in decision follows measured package, startup, memory and lifecycle results; compiled-package path remains supported | planned ([RFC 0008](RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 63 / R84 | Profile packages; 0015 C4 | R81, R83 | Closure packages per product profile with texture variants; unindexed-content ratchet; desktop installed-package smoke; mobile packages in the platform container supply F7's mechanics | planned ([RFC 0015](RFC/0015-asset-identity-content-build-graph.md)) |
| 64 / R58 | Mobile map packages; 0008 F7 | R29, R55, R56, R84 | Device format queries recorded; per-profile packages pass installed-package smoke tests on R29 runners | planned |
| 65 / R33 | Hammer feature families; 0002 H6 | R25 | Each declared displacement/instance/manifest/overlay/texture/preview family passes load/edit/undo/save/build, recovery and performance gates | planned |
| 66 / R34 | Physics gameplay and tool completion; 0004 D | R31 | Required fluids, vehicle modes, pulley/group and selected Portal/game features pass; compiler/content workflows preserve supported formats | partial (provider level; [0004 progress](RFC/0004-progress.md)) |
| 67 / R35 | New audited compute seams; 0003 F | R30 | Animation/render-list/AI/streaming cohorts have stable inputs, correct cross-system edges and ordered commit; individual equivalence/budget gates pass | planned |
| 68 / R36 | Vulkan parity and four-platform release readiness; 0001 rank 17 | R29, R32 | Per-platform materials/images, loss/recovery, cache, hardware budgets and normal package/store-compatibility checks pass; default selection is a separate product decision | planned |
| 69 / R37 | Physics parallel rollout and default gate; 0004 E | R20, R34 | Worker-count determinism, nested-work/callback/shutdown bridge, platform packaging, budgets and supported client/server combinations pass; IVP rollback tested | planned |
| 70 / R67 | Opt-in Box3D capabilities; 0013 P0–P7 (phase prerequisites in the RFC) | R19 | Each capability has its own interface, contract with bad providers and required gate in `physics-v1.json`; parallel step passes on declared profiles; game opt-in only after its filter-threading policy and gameplay corpus pass | active ([0013 progress](RFC/0013-progress.md); P0–P3 on Linux desktop; pool step scheduler measured on Linux and the iPhone 16 Pro, 2026-09-26) |
| 71 / R38 | Stateful scheduling migrations; 0003 G | R30, R35, R37 | Snapshot-send ownership and selected entity/physics cohorts preserve legacy observations/order or record intentional change; network/latency/lifetime gates pass | planned |
| 72 / R39 | First-party module retirement; 0001 rank 18 / retirement B–D | R12, R18 | Pseudo-modules removed; mandatory systems and provider catalogs use typed linked factories; no filename/string discovery for migrated services | active ([Phase D](RFC/0001-phase-b-progress.md#later-work-not-claimed-here)) |
| 73 / R40 | Tool executable/process cleanup; 0001 rank 19 / retirement E | R11, R12, R22 | Launchable-DLL wrappers retired by cohort; structured argv/process protocol, outputs/cancellation and required compiler workflows pass; integrations tool-only | active ([Phase E](RFC/0001-phase-e-progress.md)) |
| 74 / R41 | Extension hosts and public-loader removal; 0001 rank 20 / retirement F–G | R07, R11, R39, R40 | Family-owned versioned ABI/trust/lifetime fixtures pass; Waf enumerates boundaries; only approved hosts load; Tier1/filesystem general loader APIs retired | planned |
| 75 / R42 | Scheduler consolidation; 0003 H | R35, R38 | Redundant queues/waits have zero consumers; process worker budget controlled; supported host modes retain correctness/latency and rollback evidence | planned |
| 76 / R43 | Hammer legacy retirement; 0002 H7 | R33, R60 | Declared product parity/recovery gate met; old consumer counts zero; superseded shell/glue/build references and stale exceptions removed | planned |
| 77 / R44 | IVP simulation retirement; 0004 F first gate | R37 | Declared profiles no longer depend on IVP simulation; gameplay/save/package gates pass and rollback/support decision recorded; decoder dependency remains explicit | planned |
| 78 / R45 | Independent collision decoding/cooking; 0004 F second gate | R40, R44 | Legacy/native format corpus and tool compatibility pass without IVP code; dependency audit clean; schema and old-content policy explicit | planned |
| 79 / R46 | Tier-global/domain retirement; 0001 rank 21 | R39, R41, R42, R43, R45 | All declared domain cohorts use explicit ownership; old globals have zero consumers; cohesive targets pass architecture/product gates; tiers removed only when empty | planned |

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

- R67 (RFC 0013): added 2026-09-24 at the user's direction, `active`. The
  `box3d-optin` branch is merged (2026-09-25), and work continues on the
  main branch. Box3D-only behavior is exposed as opt-in
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
    Unmeasured on the Fold7. It applies only where Box3D is selected. Box3D
    is the default provider since 2026-09-26 (user decision): the launcher
    (and so the Android APKs), the Linux dedicated server, `./play`,
    `./play_p2`, `run.sh` and the iOS and tvOS apps; `-physics vphysics`
    selects IVP. The Windows dedicated server (a legacy profile) still
    defaults to IVP, and the conformance harnesses pin `-physics vphysics`.
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
    `AWAKE` from the bench, `awake_min` 512). The whole-run p50 had timed
    sleeping steps after the restitution pin. All 15 faults are detected.
  - Pool step scheduler (2026-09-26): `CPoolStepScheduler` replaces the
    per-task pool bridge; it still borrows only the root's pool.
    - Linux, uncontended: every required gate passes. Speedups from 1 to 4
      workers are pile-4096 2.69x (was 2.0x), pile-1024 2.03x and
      ragdolls-128 2.08x.
    - iPhone 16 Pro: box3d-parity and shape-inertia pass, and 15 of 15
      faults are detected. pile-1024 speeds up 1.36x and ragdolls-128
      1.32x. pile-4096 gives 1.49x against its 1.50x rule; the A18 Pro's
      two performance cores bound the gain.
  - Not done:
    - the parity runner's verdict fails. On upstream Box3D, 602/602 checks
      pass but `dynamics.tumble.audible-impacts` diverges (2026-09-25). On
      the pinned fork `johnpanos/box3d` (`78c90a0`, with the restitution
      patch), three Box3D gameplay checks fail;
    - pile-4096 misses its speedup rule on the iPhone (1.49x against 1.50x);
    - the client environment has one worker;
    - no gameplay soak;
    - no Android (Fold7) measurements;
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
  - Lightmap layout and seams (2026-09-25): the pipeline now writes its own
    lightmap layout (planar by default, xatlas charts on curved surfaces)
    instead of Blender's, and a `seams` gate checks it
    ([record](RFC/0007-progress.md#lightmap-layout-seams-and-noise-installed-2026-09-25)).
    This is not the seam.
  - One lighting back end (2026-09-28, user direction): every map is lit by
    `pbrt_map_build`'s pipeline through `map_lighting.py`: a compiled BSP
    plus an authored scene, or one derived from the BSP. Front ends differ
    only in how they make the BSP: VMF (`vmf_map_build.py --lighting`),
    Hammer (`MapBuildRequest::lighting`), generators, `vrad_cycles.py`,
    shipped maps, USD-native (`usd_map_compile.py --lighting`), and PBRT/USD
    scenes, whose collision BSP now compiles before any bake. Its
    `identity` gate covers every map. The seam's place is fixed:
    `light_baker.py` is the one table of bake operations, and no other tool
    runs a bake script. The World Stage preview bridge scripts are retired.
  - The `ILightBaker` contract, its providers and the shared suite are not
    installed, so the row stays `planned`. See the
    [R48-BAKER record](RFC/0007-progress.md#one-lighting-back-end-and-the-seams-place-installed-2026-09-28).
  - Interim hook (2026-09-25, user direction): `tools/quality/vrad_cycles.py`
    is a vrad drop-in. It runs vrad, then the Cycles relight of its BSP, so a
    regular vbsp/vvis/vrad compile feeds the Blender pipeline. It is not the
    seam and closes no R48 criterion
    ([record](RFC/0007-progress.md#regular-compile-hook-vrad_cyclespy-installed-2026-09-25)).
  - Cycles bakes default to the CPU (user decision 2026-09-25, `d57305aa`);
    `gpu` and `auto` are opt-ins.

- R65–R66 (RFC 0012): added 2026-09-24 as `planned` at the user's direction.
  R65 was ranked directly after R47: it is bounded, it closes the native
  `VK_UNIMPLEMENTED` alpha-to-coverage stub, and specular AA is cheapest to
  add before the R47 PBR fixtures settle. Since 2026-09-26 it follows the
  render core rows R86–R91 (RFC 0016), so its pipeline and target work is
  built once, on the core. R66 follows R55, whose KTX2 writer
  it extends. MSAA itself exists (R32-VIDEO-OPTIONS P6). It has been
  measured only on the Apple TV 4K (2026-09-26): 2x and 4x MSAA both exceed
  the 60 fps budget, and 4x at 4K was killed within seconds. There is no
  Linux or Fold7 measurement, and mobile target policy and defaults wait for
  A0 budgets and Fold7 measurements. Temporal AA stays out of scope.

- R47 energy compensation, shared BRDF and grouped descriptor sets
  (2026-09-25, user direction, Filament comparison follow-ups):
  - The PBR lobe is energy-compensated for multiple scattering. The rough
    white-metal furnace closes to 1.
  - `shaders/pbr_brdf.glsl` is the one GLSL BRDF. The `render.pbr-brdf.glsl`
    GPU suite matches it to `pbr_brdf.h` on 1,080 cases.
  - The PBR and GI stages bind three sets (frame, material, constants)
    instead of up to eleven. They pass their pixel suites as a four-set
    device (`.four-sets`).
  - Legacy LightmappedGeneric binds two sets (a grouped texture set and its
    constants; 2026-09-25, for MoltenVK's eight). `$phong` still needs seven.
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
- R81–R85 (RFC 0015): added 2026-09-26 as `planned`. The user asked for the
  RFC after a review of the gap to a Source 2 content pipeline.
  - Why: RFC 0008's ledger was map-only. No asset kind had an identity, a
    reference record, a package or live reload. `studiomdl` and
    `captioncompiler` don't build, `scenes.image` has no driver, and
    `CTexture` can't select KTX2.
  - Scope: one `AssetRef` and one content build graph for every kind.
    Legacy formats pass through byte-identically. Packages carry an asset
    index, and a desktop-only reload loop sits on the same graph.
  - Ranks (agent decision under the user's standing instruction,
    2026-09-26):
    - R81 (graph core) directly before R57, which now carries C2 and
      depends on it;
    - R83 (resolver) and R82 (legacy compilers) after R60;
    - R85 (reload) before R64;
    - R84 (packages) before R58.
    - Ranks from R57 down moved by up to five.
  - Added prerequisites: R58 → R84, R60 → R57, R62 → R81, R64 → R85.
  - Nothing is implemented, and no other row changes state. R81 is
    dependency-ready.
  - The RFC's six open decisions were answered on 2026-09-26 at the user's
    direction ("what will pay off in the long term"): block-container
    archives for packages, with VPK read-only for base content;
    ASCII-lowercase identity plus a portable set for new names;
    garbage collection from kept versions and leases; a dedicated reload
    endpoint; eager hashing of base archives; and a `content.scene-image`
    library.
- R86–R92 (RFC 0016): R86–R91 added 2026-09-26 as `planned`. The user asked for "a
  real graphics system" that breaks free of the legacy pipeline while
  keeping compatibility.
  - Why: the material system and the client's `CViewRender` own the frame,
    and native Vulkan translates their D3D9-shaped stream in one
    `CVulkanContext`. Modern features reach it through `QueryInterface`
    side channels. There are no passes, transient or float/MRT targets,
    shadow maps or GPU skinning, and the timeline-token provider runs only
    in tests.
  - Scope: a strict C++20 render core (`render.device.v2` over Vulkan, a
    render graph, a GPU scene with views, materials as families with
    parameter blocks, clustered lights and a shadow atlas) that owns the
    frame. One legacy frontend records `IMatRenderContext`, `IShaderAPI` and
    shader-DLL work into stage passes; the inversion (K3) must be
    byte-identical. Mod D3D shader bytecode stays on the D3D9 profile.
  - Ranks (agent decision under the user's standing instruction,
    2026-09-26): R86–R91 directly after R47, so R65, R56, R50, R63 and R36
    are built on the core rather than twice. Ranks from R65 down moved by
    six. Added prerequisite: R56 → R90 (clustered dynamic lights move to
    RFC 0016 K7).
  - Revised the same day at the user's direction: the core is ports and
    adapters. `render.device.v2` names no graphics API (abstract resource
    usages, fixed conventions, capability negotiation, per-target shader
    artifacts through pinned SPIRV-Cross). Vulkan is the first adapter and
    OpenGL 4.5 the second (K10, row R92, ranked after R88 as the RFC 0001
    step 10 proof). ToGL stays on the SDL2 legacy-renderer profiles for mod
    shader DLLs (user decision, 2026-09-26).
  - Layout and layers: `public/render/<module>/` and `render/<module>/`;
    eight declared layers plus an adapter column, enforced by a new archlint
    rule CAP011 (down-only edges, independent siblings, no portable edge to
    an adapter) in a `layerContracts` section of `architecture/modules.json`.
    Every gate K0–K10 is a table of named checks with machine-decided pass
    conditions.
  - Decisions (agent, same instruction): four bind groups; VMA,
    timeline semaphores, synchronization2 and dynamic rendering private to
    the Vulkan adapter and recorded per profile at K1; GLSL source; the
    frontend and the GL context on the render sequence; portals move last.
    Ranks from R89 down moved by one for R92.
  - Prerequisites found while writing it: the job system has no capability
    module, the GLSL compiler isn't pinned, and no material or shader-API
    header is in `legacyAbi.paths`. All three are K0 work.
  - Nothing was implemented when the RFC landed; R86 was dependency-ready.
  - R86-LAYOUT (2026-09-26, user goal: "implement this according to the RFC
    directory layout and architectural arrow"), on branch `render-core` in
    the worktree `../source-engine-render-core`:
    - Fourteen strict static libraries under `render/` and
      `public/render/<module>/`, one per layer-contract module, built for
      client, tool and test products only; the dedicated product never adds
      them. The Vulkan adapter builds with the native Vulkan backend; the GL
      adapter option fails configure until K10.
    - archlint CAP011 reads the new `layerContracts` section; its six RFC
      fixtures fail with their rule, and `check --all` passes.
    - Runtime: the launcher composes the core around the selected legacy
      backend (the frontend keeps its id; `-norendercore` rolls back), binds
      the engine (`Engine_BindRenderCore`) and adds `RenderStageMarkers001`.
      The engine drives frames in the host render steps, marks views at
      `CRender::Push3DView`/`PopView` and owns the world scene; the client
      marks content stages. Hammer projects its document into its own scene.
    - Evidence: `render.device.v2` null (279) and Vulkan (499, 0 validation
      messages), 10 of 10 bad adapters, graph (1,000 random graphs agree with
      the model), scene, frame, composition and Hammer suites, g++ and
      clang++. A Portal boot on the null device ran 208 frames with 0 stage
      order violations, in both queued modes and with the Vulkan adapter;
      pixels match `-norendercore` within run-to-run noise. The dedicated link
      map holds no render code, the engine defines none, and the static
      composition check passes with the core in the program.
    - Not done: every gate. K0's job-system module, compiler pin, frozen
      headers and oracles; K1's single Vulkan stack; K2's model, pooled
      executor and aliasing; the K3 inversion and the `rendercore` rename.
      The iOS app builds with the core statically linked, but has not run on
      a device. No Android build and no frame-time measurement.
      See the [record](RFC/0016-progress.md).
  - K0 (2026-09-26, user direction: "K0 is required"): every check passes
    on the Linux desktop and the Fold7. The job system is the strict
    `jobs.graph` module; shaderc v2026.1 is pinned by source archive and
    rebuilds every committed SPIR-V module byte-identically; 24 frozen render
    headers are under CAP010; `legacy.render-abi` holds 541 vtable slots
    (8 of 8 seeded reorders detected); the view and draw-state oracles cover
    `testchmb_a_00`, `testchmb_a_08`, the legacy-ports set and Portal 2's
    `sp_a1_wakeup` (21,632 draws, every single-draw removal detected) under
    a new opt-in `-deterministicrender`; `render-v1.json` has desktop and
    Fold7 k0_records (the Fold7 limits revised at the user's direction after
    its first measurement). Gap: the Portal 2 client compiles monitors out
    (`USE_MONITORS`), a parity gap against retail, so its monitor view is
    not captured. See the
    [K0 record](RFC/0016-progress.md#k0-prerequisites-and-frozen-oracles-2026-09-26).
- R61–R64 (RFC 0008 F8–F11): added 2026-09-23 as `planned` for versioned native
  map spatial data, a modern model asset path, visual parity and geometry
  scalability, and direct USD development-runtime iteration. They extend the
  Source 2-like outcome without marking F1–F7 or the current preview complete.

- Hammer scope (user direction, 2026-09-25, clarified): not a perfect or
  legacy-complete Hammer, but one that can always build a map. "We need
  hammer and a game, and they both need to evolve together."
  - Every editor slice keeps a working, fast author → save → compile → play
    loop. A conformance suite checks it by driving the real UI the way a user
    does (scripted clicks and keys in an isolated compositor) and running the
    same commands headlessly.
  - Behavior lives in shared headless domain libraries (a document/command
    layer). The GTK UI is a thin layer that turns input into commands and
    presents state. An MCP server can sit on the same command layer.
  - Legacy-parity breadth (MFC parity, R33 families, R43) is added as the
    map-building workflow needs it, not for its own sake. Cheap enforcement
    (HAM002/HAM003, the module graph) stays. Row definitions are unchanged.
  - Layered libraries (user direction, 2026-09-25): format and domain code
    (KeyValues, VMF, FGD, VPK, VTF, materials, BSP2, the USD stage) lives in
    small libraries. Each is its own `architecture/modules.json` module and
    its own Waf static library declaring `arch_module`, with edges pointing
    only down, following composition.
    - Editor, tools, engine and game compose these libraries; a library never
      depends on an application.
    - Enforced by the existing include, link, owner and cycle checks.
    - `hammer.formats` is split incrementally into per-format libraries owned
      outside Hammer, so the compile tools and the engine can adopt them.
  - UI/UX (user direction, 2026-09-25): take inspiration from the Source 2
    tools' ergonomics. Each GTK UI slice first records the documented Source
    2 Hammer behavior it adopts, with sources, and expresses it through the
    shared command layer.
  - Play-in-editor (user direction, 2026-09-25): the editor embeds the native
    renderer and a game preview with play/stop, as Unreal does. Planned
    design:
    - the engine runs as a child process through a new
      `render.presentation.v1` pair that exports frames as dmabufs;
    - the GTK host shows them zero-copy (`GdkDmabufTexture`) and forwards
      input as normalized events;
    - play/stop starts and stops the child on the freshly compiled map.

    It is sequenced after the command layer and the author → compile → play
    loop suite. Nothing is implemented yet.
  - Editor viewports (agent decision under the user's standing instruction,
    2026-09-28, agreed with the Hammer session; supersedes "the same bridge
    serves a native-renderer viewport"): the viewports render in-process.
    - The GTK host composes the RFC 0016 render core (`RenderCore_Create`,
      Vulkan device, no legacy backend) and renders each view offscreen from
      `presenters::EditorWorkspace`'s `viewport::RenderSnapshot`, with the
      workspace's cameras passed per frame.
    - Frames reach GTK as a dmabuf in a `GdkDmabufTexture`, with a readback
      into `GdkMemoryTexture` as the first step. The document, cameras and
      overlays already live in-process, and a child process would have to
      stream every edit.
    - The child-process `render.presentation.v1` bridge stays for
      play-in-editor, and both share the dmabuf-to-GTK presentation step.
    - Owner: the Hammer session (R17). The GL renderer is deleted once the
      core covers cameras, 2D wireframe and grid, selection, tool overlays,
      displacements and entity markers.
    - Done 2026-09-28 (R17-CORE): the viewports draw through the core, and
      the GL renderer is deleted. `hammer_gtk` is a Waf target of the tools
      product (`--render-core-vulkan`). The frames are read back into a
      `GdkMemoryTexture`, not yet a dmabuf, and the solids are drawn by
      `render.pass.lines` until K4's families draw `render.scene` instances.
      See the
      [record](RFC/0002-progress.md#r17-core-the-gtk-viewports-on-the-render-core-slice-done-2026-09-28).
- R08: `active`. See the
  [RFC 0002 current state](RFC/0002-progress.md#current-state-2026-09-25).
  - R08-CMD (2026-09-25, first slice of the map-building loop):
    - a named, serializable command layer (`hammer::app::EditorCommands`,
      one table for dispatch and catalog, file-store I/O, structured errors,
      line scripts), shared by GTK, scripts, tests and MCP;
    - exact domain operations and undoable world properties;
    - the legacy texture-axis rule as a `hammer.geometry` owner (now in the
      `mapgeometry` library, R08-LIBS);
    - byte-stable VMF reload.

    At this slice, `hammer.app.editor_commands` (54 checks) and the
    controller suite (233) passed, and 10 mutants were detected. All 61
    Q-EDITOR suites passed on both compilers. By R08-UI-P1 the command suite
    has 94 checks and the controller suite 248. See the
    [record](RFC/0002-progress.md#map-building-loop-direction-and-r08-cmd-2026-09-25).
  - R08-LIBS (2026-09-25): the first layered format libraries, `mapgeometry`,
    `kvtext` and `vmf`, moved out of Hammer as capability modules and Waf
    libraries with enforced downward edges.
  - R08-LOOP (2026-09-25): the Waf-built `hammer_cli` runs command scripts.
    `vmf_map_build.py` is the one VMF → bootable-map tool. The
    `corpus.hammer.loop` / `hammer.loop` suite checks author → save →
    leak-free compile → headless boot in about 9 s, with leak and unlit
    negative controls.
    - Fixed along the way:
      - every saved plane was inside-out for vbsp;
      - no map had HDR lighting;
      - the compile tools failed on uppercase paths;
      - the GTK sample room leaked.
  - R08-UI-P1 (2026-09-26):
    - Source 2 P1 commands: `select`, `select_none`, `move_selection`,
      `hollow`, `describe`, `raycast`, `place_on_surface` and `build_map`.
      `build_map` runs through the `hammer::ports::IMapBuilder` port and
      `ToolProcessMapBuilder` over the POSIX tool-process provider. This is
      the first product consumer of R40's tool-process contract.
    - The GTK shell runs Open, Save, hollow and build through the commands,
      with Shift+B/E/S tools, F for a room, a 3D surface-click Entity tool
      with a class palette, F9 build and Shift+F9 build and run.
  - R08-UI-TEST (2026-09-26): `corpus.hammer.ui` / `hammer.ui` builds
    `hammer_gtk` and drives it in a private headless mutter session.
    - Widgets are found by accessible name over AT-SPI. Pointer and keys go
      through the compositor's RemoteDesktop input, and view positions come
      from the status-bar coordinates.
    - The user steps: grid `[ [`, a block dragged in the top view, Return,
      F, then a player start and a light clicked into the front view, then
      F9.
    - The oracle judges the saved VMF (six walls, both entities inside) and
      a leak-free build record. The `no-hollow` and `no-light` controls are
      rejected.
    - Evidence: 12 checks in 53 s, and 3 of 3 repeats pass.
    - Not covered: a CI lane, the Wayland backend and 3D-view placement.
  - R08-MCP (2026-09-26): `hammer.adapters.mcp` serves the command catalog
    as MCP tools over newline JSON-RPC (`hammer_cli --mcp`). Each tool call
    is one `EditorCommands::Execute`, so agents share the one document
    authority and undo history.
    - `hammer.adapters.mcp` (90 checks on both compilers): a room authored
      by tool calls saves byte-identically to the scripted room.
    - `corpus.hammer.mcp` / `hammer.mcp` (21 checks): drives the real
      process, parses replies with Python's `json`, and builds the room,
      judged by the UI suite's oracle.
    - 13 of 14 seeded faults are detected; the survivor is equivalent.
    - Not done: the live GTK editor serving MCP, and a run with a real MCP
      client.
  - R08-DOMAIN (2026-09-28, user goal "all the headless domain logic for
    hammer, ready to be hooked up to the UI"): the scene, ports, formats,
    app, viewport, tools and presenters layers, with `EditSession`,
    `SessionCommands` and `presenters::EditorWorkspace` as the one object a
    UI binds. The VMF codec has no escape hatches (user requirement): real
    Portal 2 maps load into typed fields and round-trip. 59 suites, 2,925
    checks, both compilers; `hammer_cli` and MCP run on the new stack. See the
    [record](RFC/0002-progress.md#r08-domain-headless-domain-logic-for-the-editor-slice-done-2026-09-28).
  - R08-GTK-WORKSPACE (2026-09-28, user direction "replace EditorController
    with our new domain models"): `hammer/gtk` is a thin host over
    `EditorWorkspace`. Its renderer draws the render snapshot, grid and tool
    overlay through the workspace cameras, and its menus come from the
    `ActionCatalog`. `EditorController`, `EditorCommands` and their four
    suites are deleted. Q-EDITOR passes 118 of 118 suites on both compilers,
    and `corpus.hammer.ui` passes. See the
    [record](RFC/0002-progress.md#r08-gtk-workspace-the-gtk-shell-on-editorworkspace-slice-done-2026-09-28).
  - R08-ASYNC-BUILD (2026-09-26): F9 saves on the UI thread and compiles
    through `hammer::app::MapBuildQueue`. The builder runs on a
    `ThreadTaskRunner`, and the reply returns on `hammer::gtk::GlibTaskRunner`.
    This is the first product consumer of R10's runners.
    - `hammer.app.map_build_queue` (15 checks).
    - `corpus.hammer.glib-runner`: 20 checks, the shared runner suite.
    - `corpus.hammer.ui` passes with the asynchronous F9.
    - Contract clause 5 is refined for main-loop runners.
    - Not done: cancelling a running compile, and a streamed build log.
  - The ledger has 28 migrations, 11 of them extracted format cores.
    The inventory has 46 authored records. `archlint hammer --coverage`
    reports 38 of 531 files classified, because 8 records were extracted into
    capability libraries and no longer count. Its authored total (452) is
    stale. There are 122 Q-EDITOR suites (2026-09-28): 118 headless and four
    corpus suites (loop, ui, mcp and glib-runner).
  - `archlint hammer --verify` passes again (2026-09-25, user decision): the
    validator accepts Hammer edges to registered capability modules, such as
    the R47 schema's `hammer.formats` → `render.contracts`.
  - Strict Hammer include-graph checking (HAM002, include edges and cycles) is installed (R04-HAMGRAPH,
    2026-09-25). The VMF decoders moved from geometry to formats, which
    removed the geometry↔formats cycle. App → formats is a recorded
    exception owned by R22 until a `hammer.ports` persistence contract
    exists.

- R14/R18 SDL3 retarget (user decision, 2026-09-26): the window/input
  providers target SDL3, not SDL2.
  - Why: every north-star client profile (Linux DXVK and native Vulkan,
    macOS, iOS/tvOS, Android) already runs on SDL3. SDL2 was the reference
    only because it was the running provider when RFC 0001 was written.
  - R14 is done when the shared window/input suite passes against an SDL3
    provider (and the fake backend). R18 is done when the SDL3 suites pass on
    every declared SDL3 profile, SDL is off generic include paths (the
    `platform/sdl3/legacy_include` adapter's callers are migrated), and the
    SDL2 legacy profiles still build. SDL2 behavior parity is not required.
  - SDL2 stays only as the provider for the legacy compatibility profiles
    (the Windows client, `linux-i386-legacy`, `android-armv7a-legacy`,
    `freebsd-legacy`). It gets no further work, and
    `platform/sdl2/window_system` needs no suite. Retiring those profiles, or
    SDL2 itself, is a separate user decision.
  - Waf defaults (2026-09-26, user direction): `--render-backend` is `auto`,
    selecting `native-vulkan` for 64-bit Linux, Android and iOS/tvOS clients
    and `legacy` for dedicated, test and tool products, 32-bit, GLES, other
    OSes and an explicit `--platform-provider=sdl2`.
    `scripts/build-ubuntu-amd64.sh` pins `--render-backend=legacy` for the
    legacy client lane. `--physics-backend` defaults to `box3d` but is not
    read by the build (both providers are always linked; existing trees
    stored the old `ivp`, so wiring it would drop Box3D from them).
  - `--platform-provider` defaults to `auto`: `sdl3` for every Vulkan
    client and iOS/tvOS, `sdl2` only for legacy-renderer products. Vulkan
    builds no longer need the flag; an explicit `sdl2` with a Vulkan backend
    fails configure.
  - Changes no row's state: R14 and R18 stay `partial`, and R16's hard-gate
    violation stays until R14 closes.

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
    here. R14 is now `partial`: its window/input contracts exist, but no
    SDL3 window/input provider does (see the R14/R18 SDL3 retarget below).
  - Open: the D3D9/DXVK pair stays on the legacy `SetMode` membrane (no D3D9
    new-contract device); Android, macOS and iOS surfaces are unverified; no CI
    lane. The four Vulkan helper headers that archlint reported as `CAP002`
    from 2026-09-24 are registered in `render.vulkan.core` since R04-CAP
    (2026-09-25); the no-SDL rule still holds. Findings: Wayland cannot unminimize; hiding after a FIFO present kills
    the connection. See the [presentation bridge record](RFC/0001-presentation-bridge-progress.md).

- R01: `done` (2026-09-22) for the Q0 baseline and profile inventory:
  - One declaration, [`quality/baseline.json`](quality/baseline.json), holds
    host tools, content corpora, the support matrix (17 profiles at
    closure, 18 with tvOS), installed checks with recorded outcomes and
    owners (33 at closure), and the captures and budgets per
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
    deviations and 0 unavailable. On 2026-09-26 `baseline.json` has 53
    checks: 50 recorded `pass` and 3 known `fail` (`roadmap.check`,
    `physics.conformance`, `gi.references`), with no recorded crash.
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

- R29-IOS-STATIC / R39: `partial` (2026-09-25, user goal: "build the iOS
  version, but before we can do that you need to get static composition
  compiling").
  - `./waf configure --static-composition` builds each first-party shared
    library as a module object. The module is partially linked with `-r` and
    its unexported symbols are localized, so the shared-library boundary is
    kept. `scripts/waifulib/static_composition.py` does this; each module
    exports `StaticModule_<target>_CreateInterface`.
  - The Portal client links into one program with no first-party shared
    library. `tools/quality/static_composition.py check` passes (22 linked
    modules); its 8 self-tests detect 7 seeded defects.
  - With every first-party `.so` removed, the static product boots
    `testchmb_a_00` and exits 0 with the null and native Vulkan renderers, IVP
    and Box3D. Loader telemetry shows 0 events; the desktop control shows 219.
  - Typed linked factories now cover physics (`-physics` catalog), the file
    system and queued loader, and the tool framework. `Engine_BindLinkedGameModules`
    binds the client, server, GameUI and the game-declared app systems for
    static products. The desktop product uses the first three too; the
    dedicated root still loads physics by filename.
  - Agent decisions: localization over renaming; `linked_game_modules.h` joins
    the legacy ABI package; no server browser or platform-menu loader in
    static products.
  - iOS host toolchain (no Mac needed): `tools/ios/build_toolchain.py` builds
    pinned LLVM 22.1.8 (clang, ld64.lld), libdispatch and cctools-port ld64
    from `quality/product_profiles/portal-ios-native-vulkan.json`.
    - Mach-O module objects use `ld64 -r`, which localizes hidden symbols;
      ld64.lld has no `-r`.
    - The checker reads Mach-O, and its seeded defects pass for both formats.
    - MoltenVK v1.4.2 (iOS static) is pinned.
    - Static products refuse first-party module loads, and the refusal is
      recorded.
    - The native Vulkan backend enables portability enumeration and
      `VK_KHR_portability_subset`.
  - First iOS build (2026-09-25): `./build-ios-app.sh` builds an unsigned
    `Portal.app` on Linux, using the iPhoneOS 26.5 SDK from the user's macOS
    VM.
    - The binary is arm64 for iOS 17.0 and loads only system
      frameworks/libraries.
    - The static-composition check passes on Mach-O (22 modules).
    - On the Mac: the plist lints and an ad-hoc signature verifies.
    - Bundle id `com.panos.sourceengine`.
  - Device runs (2026-09-26, iPhone 16 Pro, iOS 27.0): the app runs, with
    LightmappedGeneric within MoltenVK's eight descriptor sets. Touch
    controls, gyro aiming and portrait are built but untested on the device.
    `tools/quality/ios_conformance.py` runs the manifest's compiled suites on
    the phone (profile `quality/profiles/ios-arm64-device.json`):
    - Portal frame pacing passes (warm pass: 9.16 ms median, 0 hitches);
    - 196 suites: 186 matched, 6 skipped by declaration; the other 4 were
      harness or test defects, since fixed;
    - scheduler budgets pass (new iOS rows);
    - the physics bench passes box3d-parity and shape-inertia. Box3D is the
      iOS/tvOS provider (user decision 2026-09-26); the pool step scheduler
      raised its 1->4 worker speedup from 1.00-1.18x to 1.32-1.49x (two
      performance cores bound it; pile-4096 sits at its 1.50x rule).

    See the [device record](RFC/0005-ios-device-progress.md).
  - Not done: the AGENTS.md iOS lifecycle, memory-pressure and simulator
    obligations, a CI lane and a sanitizer run. See the
    [record](RFC/0001-static-composition-progress.md).
  - IOS-P2 (2026-09-26, user direction: "let's build play_p2 but for iOS"):
    extra product scope, like TVOS-PROFILE, and no R29 criterion.
    - `portal2-ios-native-vulkan.json` extends the Portal iOS profile.
      `./build-ios-portal2-app.sh` builds `Portal2.app`, with `vscript`
      composed as a linked app system in static products (`c4d6407b`).
    - The static check passes: 25 module objects, 23 linked entries.
    - Not done: signing (no provisioning profile for the bundle id), content
      on the phone, a device run, and Bink video.
      See the [record](RFC/0001-static-composition-progress.md#ios-portal-2-build-2026-09-26).
  - Home-screen icons (`e86cb26d`): `tools/ios/app_icons.py` makes the iOS
    and tvOS icons from pinned key art; `actool` on the Mac compiles the
    asset catalog.

- TVOS-PROFILE: `partial` (2026-09-25, user direction: "add a tvos product
  profile", then "run the build in the background and use the cache
  folder"). This is extra product scope. It adds no north-star target and
  no R29 criterion.
  - [`portal-tvos-native-vulkan.json`](quality/product_profiles/portal-tvos-native-vulkan.json)
    is registered as the optional `tvos-arm64-device` row in
    `quality/baseline.json`. It reuses the iOS profile's static composition,
    toolchain and SDL3 pins. MoltenVK's tvOS slice comes from the v1.4.2
    all-platforms archive. The AppleTVOS 26.5 SDK was copied from the macOS
    VM.
  - `./build-tvos-app.sh` builds an unsigned `Portal.app`. Waf treats tvOS as
    the `ios` UIKit family, with `PLATFORM_TVOS`, selected by the SDK.
    - Content lives in `Library/Caches` (user decision); the system may
      purge it.
    - The static check passes (22 linked modules).
    - The app loads no Core Motion.
    - On the Mac, the plist lints and an ad-hoc signature verifies.
    - The iOS build still passes.
  - On an Apple TV 4K (2026-09-26):
    - It installs signed and runs, at 1080p scaled to 4K, with no MSAA and
      Low settings (user direction). At 4K with 4x MSAA it was killed
      within seconds.
    - An Xbox controller drives it.
    - Missing content triggers an alert.
  - 60 fps on the Apple TV 4K (2026-09-26): the budget
    `tvos-portal-frame-pacing-60` in `quality/budgets/render-v1.json` passes on
    `portal-frame-pacing-v1`. GPU render is 10.4 ms median and 15.8 ms p99, with
    0 missed refreshes in the warm passes.
    - Changes: specialized uber-shader combos and alpha test, depth and
      stencil liveness, and deferred large uploads.
    - The tvOS defaults are now high textures and models, 16x anisotropic
      filtering, medium shadows, no bloom and sound on.
    - The margin is thin, and only one map was measured. See the
      [record](RFC/0001-native-vulkan-frame-pacing-progress.md#apple-tv-4k-at-60-fps-tvos-profile-2026-09-26).
  - Device testing (`ios_frame_pacing.py --profile`, the budget's meaning on
    a FIFO-only display, shader experiments and device traps) is in the
    [device record](RFC/0005-ios-device-progress.md#apple-tv-4k-tvos-profile-2026-09-26).
  - Console UI and controller rumble (2026-09-26, user direction): tvOS, and
    iOS with a gamepad at launch, start with the Xbox 360 GameUI
    (`+gameui_xbox 1`, now applied before GameUI starts). Its dialogs no
    longer crash, and it has a built-in option list. Gamepads rumble through
    `SDL_RumbleGamepad` (`input.gamepad-rumble`, 33 checks; 12 of 12 bad
    policies rejected). Desktop headless evidence only; no device run. See
    the
    [record](RFC/0001-static-composition-progress.md#console-ui-and-controller-rumble-on-tvos-and-ios-2026-09-26).
  - Not done: other maps at 60 fps, a simulator run, and remote-driven
    menus (out of scope). See the
    [record](RFC/0001-static-composition-progress.md#first-tvos-build-2026-09-25).

- R01/R29-ANDROID-BUILD: `partial` (2026-09-22).
  - [`build-android-apk.sh`](build-android-apk.sh) builds the SDL3/native
    Vulkan Portal APK. Every input comes from pinned archives in the
    [Android profile](quality/product_profiles/portal-android-native-vulkan.json):
    NDK r30, SDL3, SDK platform and build-tools.
  - The script ends with the independent APK verifier
    [`android_apk.py`](tools/quality/android_apk.py). It checks the declared
    modules and ABIs, ELF machine type and 16 KB alignment, `DT_NEEDED`
    closure, the manifest, the touch assets, and `zipalign`/`apksigner`.
  - The verifier has 39 negative/positive fixture tests, and there are 17
    profile/manifest/touch-asset source guards. No NDK is needed; they run in
    `composition.yml`.
  - CI: [`android.yml`](.github/workflows/android.yml) builds every declared
    ABI. It has not run on hosted CI yet.
  - Clean-worktree builds of both ABIs pass locally.
  - The arm64 APK ran on a Galaxy Z Fold7, including fold and rotation.
  - Emulator smoke tests, surface recreation and store checks remain
    unverified. See the
    [profile record](quality/product_profiles/README.md#android-portal-sdl3native-vulkan-profile).
  - Portal 2 APK (extra product scope, no R29 criterion):
    `build-android-portal2-apk.sh` builds from
    `portal2-android-native-vulkan.json`, which extends the Portal profile.
    Its device evidence is `unverified` in the
    [profile record](quality/product_profiles/README.md#android-portal-2-profile).

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
  - Since then (2026-09-26): the manifest has 280 suites (190
    `linux-headless-core`, 75 `linux-host-corpus`, 15
    `linux-native-vulkan-gpu`). Hosted CI passed twice on 2026-09-23; the
    three 2026-09-24 runs failed linking the BSP2 reader self-test before any
    suite ran. It passes locally at HEAD, which is not pushed. The gcc
    CTSQueue crash is fixed (R20), and `legacy.unittest-legacy-clang` is now
    recorded as `pass`.
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
  faster. Q-JOBS has 23 manifest rows (18 required, 5 optional TSan) on 2026-09-26. This sets no frame budget and closes no gate.

- R10-HOST-GRAPH / R20-POOL-TRUST: the R20 work is `partial` (2026-09-25); the R10
  host graph closed with R10 (2026-09-26). The items still open below are R20/R21
  work. The host frame after
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
      - mobile budgets (`scheduler-v1.json` has an iPhone row since
        2026-09-26; Android is missing);
      - full-product TSan.

- R10-RUNNERS: done as a slice (2026-09-26); R10 closed the same day. RFC 0001's
  execution vocabulary now exists:
  - the contract `public/platform/contracts/task_runner.h`
    (`platform.task-runner.v1`), covering independent, sequenced and
    single-thread runners, delayed posting, and owner-only shutdown;
  - providers in `platform.runners`: `VirtualClock` + `ManualTaskRunner`
    (virtual time), `ThreadTaskRunner`, and `SequencedTaskRunner` over any
    runner.
  - Evidence:
    - `platform.task_runner` (110 checks) runs one shared suite against six
      configurations on g++ and clang++, in default and release, and under
      TSan. It passes 20 of 20 repeats.
    - `platform.task_runner.sensitivity`: seven broken providers are each
      caught on their own clause.
  - The items this slice left open for R10 `done` closed the same day: lane
    bindings (R10-BINDINGS), the versioned legacy host capture (R10-CAPTURE;
    `jobs.legacy-captures` is `recorded`) and the first consumer, Hammer's
    async F9 (R08-ASYNC-BUILD).
  - See the [record](RFC/0003-progress.md#r10-runners-task-runners-sequences-and-virtual-time-slice-done-2026-09-26).
- R10-BINDINGS: done as a slice (2026-09-26). `RunOptions::mainThreadRunner`
  and `blockingRunner` bind `ParallelExecutor`'s affinity lanes to the
  runner contracts.
  - Bound jobs run on their runner. Refused or dropped jobs stall instead of
    hanging, and a caller that is the runner services the lane itself.
  - `jobsystem.runner-bindings` (18 checks, and a TSan lane): 60 seeded
    random graphs match `DeterministicExecutor`, and 9 of 9 executor
    mutants are detected.
  - Fixed: a shut-down `ThreadTaskRunner` claimed new threads that reused its
    thread's id.
  - See the [record](RFC/0003-progress.md#r10-bindings-the-executors-affinity-lanes-bound-to-runners-slice-done-2026-09-26).
- R10: `done` (2026-09-26). R10-SEQCHECK added `platform::SequenceChecker`
  and `PLATFORM_CHECK_SEQUENCE` (diagnostic builds), with a thread-only
  checker as the negative control; `MapBuildQueue` uses it.
  - R10-POOLRUNNER made the engine `CThreadPool` a runner. It queues with
    `JF_QUEUE` and takes an injected delay timer, so no product gains a
    dependency.
  - Two surviving mutants hardened the shared suite with never-inline and
    shutdown-under-load clauses; 3 of 3 pool mutants are now detected.
  - See the [closure](RFC/0003-progress.md#r10-closure-done-2026-09-26).
- R10-CAPTURE: done as a slice (2026-09-26). A versioned legacy host-frame
  capture is recorded in `quality/fixtures/host-frame/`: `testchmb_a_00`, 395
  frames, 8,312 host calls.
  - `jobs.host-frame-capture` boots the Portal native install in both host
    frame modes and requires each to match it, exact except the declared
    `ia` tolerance.
  - `jobs.legacy-captures` is now `recorded`. Cohort gameplay captures
    moved to `jobs.cohort-captures` (R21, `partial`).
  - 8 self-tests detect dropped, swapped and changed events.
  - The remaining R10 items closed in R10-SEQCHECK and R10-POOLRUNNER.

- R32-DEBUG-CONTROLS: `planned` (2026-09-25, user direction). These are the
  `cl_vk_debug_*` and `cl_bsp2_*` controls of
  [RFC 0014](RFC/0014-native-vulkan-and-bsp2-debug-controls.md), in phases
  D0–D7. Each control ships with an oracle and a negative control. With all
  controls at default, shipped shader modules and pixels must be unchanged.
  `mat_indirect_view` and `VK_DEBUG_LIGHTMAPPED` move into the new owner and
  are deleted once their callers have moved. D6–D7 also support R53/R54/R56.
  Nothing is implemented yet, and this closes no other row's criterion.
  The backend's `-vkgputimers` flag already reports GPU time per pass, copy
  and capture; D4's `cl_vk_debug_gpu_timers` should absorb it.

- R32-RENDER-BUDGETS: `partial` (2026-09-25, user direction). This sets
  per-profile render budgets, which close the `presentation.frame-budgets`
  entry in [`quality/baseline.json`](quality/baseline.json). R32 owns it.
  - Recorded (2026-09-26): `quality/budgets/render-v1.json` exists with one
    passing row, `tvos-portal-frame-pacing-60` (Apple TV 4K). The desktop
    Wayland and Fold7 rows are pending, and no budget script checks the file
    yet. The baseline entry is `partial`. Device measurements come
    from `frame_pacing_device.py` and `ios_frame_pacing.py`.
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
  - Per-pass GPU rows can use `-vkgputimers` now; `cl_vk_debug_gpu_timers`
    (R32-DEBUG-CONTROLS D4) replaces it later. The frame-level rows need
    neither.
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
  - `run.conf` and `run.sh` pass `+mat_queue_mode 2`. `portal_boot.py`, the
    Android launcher and the iOS/tvOS launcher still pin 0.
    `frame_pacing.py` defaults to 0 and takes `--mat-queue-mode 2`.
  - A TSan run of the product tree found 46 signatures only in mode 2, each
    triaged in the record. `portal_boot --resize-stress` passes in both modes
    since R32-RESIZE. No device-loss or mobile evidence. See the
    [queued rendering record](RFC/0001-native-vulkan-queued-rendering-progress.md).

- R32-RESIZE: `partial` (2026-09-26, user direction). This makes window
  resize and surface recreation robust on native Vulkan.
  `portal_boot --resize-stress` now passes in queued and sync modes on
  Wayland and X11, three runs each. It had never passed on native.
  - Fixed: a resize queued just before rendering left queued mode (a
    screenshot, a config change) was never published as complete, and the
    engine stopped rendering for good. Every queue drain now publishes it
    (`CMaterialSystem::PublishExecutedWindowResize`).
  - Fixed: a new video mode rebuilt the whole swapchain, which costs about a
    vsync on X11. Now it rebuilds only the back buffers
    (`RecreateBackBuffers`). The slowest sync request on X11 fell from
    21-24 ms to 7-9 ms.
  - Harness (workload version 2):
    - a fixed frame time and margins, because `wait` counts command-buffer
      passes;
    - a queued settle sweep that catches the lost completion, 2 of 2 runs
      with the fix removed;
    - per-path request budgets;
    - a bound on the longest run of scaled presents, in place of "none" in
      queued mode.
  - Not done: Android surface recreation, iOS backgrounding, a Fold7 fold
    run, device loss, a clang build, and budgets for the main-thread UI
    relayout (up to 40 ms) and the native screenshot readback. See the
    [record](RFC/0001-native-vulkan-progress.md#window-resize-and-surface-recreation-r32-resize-2026-09-26).

- R32-LEGACY-SHADERS: `partial` (2026-09-25, user direction: "implement all
  missing shader types in vulkan native"). 84 GLSL ports of stdshader_dx9 pairs
  on one generic native pipeline family, **on by default** since 2026-09-26
  (user decision; `-vklegacyports` is still accepted). `-novklegacyports`
  rolls back: the backend then routes and draws as it did before the ports
  (no port routes, no port-only native passes, no D3D9 state they add).
  - Default-on evidence (2026-09-26): the oracle passes 267/267 in both HDR
    modes; 27 of 28 material pixel families pass with the ports on, and
    integer `sky` fails identically with `-novklegacyports` (existing
    failure); a headless queued (`mat_queue_mode 2`) testchmb_a_01 boot
    passes with window glass showing the room behind it.
  - History: merged as `b5652908` and reverted as `0559a463` the same night.
    With the ports on, the user's `./play` (testchmb_a_01, run.conf,
    `mat_queue_mode 2`) showed opaque black diagonal bands, a missing chamber
    sign, broken monitors and no pause-menu UI. Headless spawn-view boots had
    not caught any of it. Cause: the D3D9 math constants written into c0/c1
    were read as the native backend's legacy c0 MVP alias, so native draws
    (a desk model among them) took them as a transform. Fixed; re-landed gated as `a1ea763f`.
    Switch on and off now match on the testchmb_a_01 view set and the pause
    menu, and the oracle passes 267/267.
  - With `-vklegacyports`: native families keep the combos they draw
    (`-vklegacyvertexlit`, `-vklegacyskin`, `-vklegacylightmapped` send every
    combo to the port); refract, bloomadd and `sky_ps20b` stay native unless
    `-vklegacyrefract`, `-vklegacybloomadd`, `-vklegacysky`.
  - Oracle: each pass is replayed on bytecode compiled from this tree's `.fxc`
    with the pinned FXC (`legacy_shader_conformance.py` passes
    `-vklegacyports`); 267 default cases passed in both HDR modes at the merge.
  - Default-on gate: the user played `./play` on their Wayland session with
    the new default and approved it ("glass works", 2026-09-26).
  - Glass oracle (2026-09-26): the `glass` material-pixel family draws
    testchmb_a_01's frosted window, refract window and box-dropper tube
    glass, plus an opaque control, over two walls through a perspective
    camera. It measures transmission: 1 in linear light for additive glass,
    `$refracttint` for Refract, 0 for the control. It passes in both HDR
    modes with the ports on and off, and 12 seeded defects are detected
    (black or opaque panes, undrawn panes, ignored tint, normal blending, a
    clear control, a missing wall, a fallback shader).
  - `forced/` cases pass in both HDR modes (64 cases each, with their
    `-vklegacy*` switches), `sky` integer included.
  - Frame time (`frame_pacing.py`, 6 interleaved rounds, `mat_queue_mode 2`,
    Box3D): ports on 4.04 ms warm median, off 4.42 ms (p99 6.39 vs 6.66
    ms); two ports-on rounds landed in the host's slow mode (5.7 ms).
  - View set (2026-09-26, `tools/quality/legacy_ports_views.py`):
    testchmb_a_01 at run.conf's settings, 8 yaws, 2 upward views and the
    pause menu, ports on vs `-novklegacyports`, with a second ports-on run as
    the noise measure. Every view is within 0.12 % of pixels (> 16/255); a
    mismatched-view negative control differs by 65 %. With this the
    default-on gate is met.
  - Depth feathering (2026-09-26): every native frame copy now carries the
    scene's projected z over the dest-alpha range in its alpha, as D3D9 PC's
    destination alpha does (`depth_to_alpha.frag`, `-novkdepthalpha` rolls
    back), and the native SpriteCard stage applies DEPTHBLEND (58 of
    Portal's 59 depth-blended materials). The `softparticle` pixel family
    matches D3D9's DepthFeathering in both HDR modes, also under 4x MSAA
    (the multisampled depth is read in place and its samples averaged, as
    D3D9's resolve averages dest alpha), and fails with the copy unchanged;
    the other families are unchanged (integer `sky` still fails as before).
    A queued testchmb_a_01 boot at the runtime's 4x MSAA with `-vkvalidate`
    logs no validation messages.
  - pbr_ps30 parallax (2026-09-26, `4070b214`): the HLSL loop the pinned FXC
    miscompiled is restructured; the case is in the default run (268/268).
  - Wrinkle maps and hardware-flexed faces (2026-09-26): native reports
    stream offsets, and `SetFlexMesh` binds studiorender's flex stream (D3D9's
    stream 2 layout in `vulkan_mesh_layout`); its position and normal deltas
    are added to the record before skinning and the wrinkle weight reaches
    the `skin_vs20` port. The `flex` pixel family matches skin_ps20b's
    WRINKLEMAP blend and the moved face in both HDR modes; the queued mesh
    contract (158 checks) covers the flex layout; a testchmb_a_01 frame is
    within 6 levels of the pre-change frame (static-prop color pooling now
    on) with no validation messages.
  - Unverified or open: flashlight passes.
  - No gate closes. See the
    [legacy shader record](RFC/0001-native-vulkan-legacy-shaders-progress.md).

- R32-VIDEO-OPTIONS: `partial` (2026-09-23). The Video options take effect on
  native Vulkan: real display modes and mode-change callbacks, vsync through
  `render.present-policy.v1`, brightness applied at present, DirectX 95 caps,
  real adapter identity with `dxsupport.cfg` recommendations shared with D3D9
  (0 mismatches over 676k cases), and MSAA with resolves. There are five new
  `render.contracts` owners with sensitivity rows. The GPU suites, the full
  pixel oracle at level 95, and DXVK regression boots pass.
  - Flashlight shadow depth (P7) is not started. It is not reported cleanly
    as unsupported everywhere: on tvOS, shadows High ends the app
    (2026-09-26).
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
  The Apple TV 4K reaches 60 fps (2026-09-26); see TVOS-PROFILE and the
  [record](RFC/0001-native-vulkan-frame-pacing-progress.md#apple-tv-4k-at-60-fps-tvos-profile-2026-09-26).

Current RFC 0001 evidence (updated 2026-09-26):

- [Native Vulkan slice](RFC/0001-native-vulkan-progress.md)
  ([current state](RFC/0001-native-vulkan-progress.md#current-state-2026-09-25))
  records R28/R32. Portal renders a lit, textured scene natively:
  `portal_boot.py --renderer native-vulkan` passes on testchmb_a_01,
  testchmb_a_08, escape_00 and escape_02.
  - Implemented: render targets, world texture residency, flat and bumped
    lightmaps, model lighting, integer HDR, PortalRefract, VGUI, fog,
    bloom/color correction and model shadows.
  - Declined by name: motion blur, water, eyes, teeth and flashlight (the
    legacy shader ports, on by default, draw the first four). Line and
    point draws are dropped, alpha to coverage is a stub, and skinning runs on
    the CPU.
  - Oracles: `material_pixel_conformance.py` has 14 families, most judged
    against D3D9 references. Waf GPU suites: bring-up 98, material-facing 25,
    equivalence 64 checks. The 15 suites of the manifest's
    `linux-native-vulkan-gpu` profile pass. A `-vkvalidate` boot logs no
    messages.
  - The 84 legacy stdshader ports (R32-LEGACY-SHADERS) are on by default
    since 2026-09-26 (`-novklegacyports` rolls back).
  - Devices: native Vulkan runs on the Fold7 (Adreno 840), the iPhone 16 Pro
    and the Apple TV 4K (MoltenVK). The Apple TV reaches a measured 60 fps
    through specialized uber-shader combos, compiled-out alpha test and
    depth/stencil liveness (`1255e01b`, `9f12b3c4`, `8b46f5ac`). These are
    device runs, not the R29/R36 platform gates.
  - The 2026-09-22 false-positive correction is history. No hosted CI
    lane.
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
  descriptors (2026-09-25). The launcher links the file system and a physics
  catalog; the dedicated root still loads physics by filename, which prevents
  global Phase D closure. R40: the launchable-DLL
  wrappers and `ilaunchabledll.h` are deleted (`66e2a40c`) and the tool process
  contract is installed. Its POSIX provider is built in `platform_posix`,
  and `hammer_cli` and the GTK editor use it for `build_map`, the contract's
  first product consumer (R08-UI-P1). The Hammer build suites exercise it
  end to end, but no contract suite runs the POSIX provider
  ([Phase E](RFC/0001-phase-e-progress.md)).

Historical: initial evidence observed at `87955f67`, when this roadmap was
written. R01 and R07 have since closed with newer evidence (see above); this
section is kept only as the starting point.

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
