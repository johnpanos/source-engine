# RFC 0005: Quality and Correctness Harnesses

- Status: Proposed
- Date: 2026-09-21
- Scope: Shared verification infrastructure and acceptance gates for RFCs 0001–0004 and 0006
- Related: [Platform](0001-capability-based-platform-architecture.md), [Hammer](0002-hammer-responsibility-factorization.md), [jobs](0003-dependency-aware-job-system.md), [physics](0004-box3d-primary-physics-backend.md), [C++20](0006-modern-cpp-ownership-and-synchronization.md)
- Implementation status: Specification, partly installed. Q0 (baseline audit) and Q1 (shared runner) are done; domain suites are registered but no domain gate is complete. State and commands: [0005-progress.md](0005-progress.md). This document implements nothing by itself.

## Summary

Build eight complementary harnesses over shared test execution, fixtures,
failure injection, trace capture, and evidence reporting. Each domain owns its
definition of correct observable behavior. Shared infrastructure must not become
a universal service locator or force unrelated domains into one test binary.

The harnesses establish executable evidence for incremental migration:

| ID | Harness | Primary obligation |
| --- | --- | --- |
| Q-ARCH | Architecture and migration | Dependency direction, ownership, exact debt ratchets, evidence-backed cutover |
| Q-FOUNDATION | Foundation, lifecycle, and ABI | Provider semantics, startup rollback, resource lifetime, binary compatibility |
| Q-EDITOR | Headless editor | Authored state, geometry, transactions/history, interchangeable hosts |
| Q-JOBS | Scheduler model and concurrency | Graph validation, publication, ordering, cancellation, subsystem equivalence |
| Q-PHYSICS | VPhysics compatibility | Queries, units, callbacks, controllers, constraints, gameplay contracts |
| Q-CONTENT | Content, persistence, and tools | Semantic fidelity, hostile input handling, recovery, compiler compatibility |
| Q-PRESENTATION | Rendering and native UI | Image fidelity, GPU lifetime, native presentation, input and host behavior |
| Q-PRODUCT | Product, performance, and packaging | Complete workflows, cross-domain interactions, budgets, distribution and rollback |

Passing a finite suite establishes the stated coverage and invariants, not a
proof of all possible C++ executions. Compile-time restrictions, static checks,
runtime assertions, controlled tests, native integration, and measured product
acceptance address different failure classes. Reports must state which ran.

## Scope and authority

This RFC owns shared harness mechanics, evidence identity, and gate execution.
The domain RFCs own semantic requirements, supported behavior, tolerances, and
their delivery gates. A harness cannot weaken a domain contract to make a test
pass. A contract change updates the owning RFC/contract record and affected
providers, consumers, comparators, and tests together.

The ranked program and its current state live in [AGENTS.md](../AGENTS.md).
Domain progress records hold detailed evidence. There is one authority for each
fact: dependency permissions in `architecture/modules.json`, domain contracts
beside their tests, and run results in immutable evidence artifacts. Progress
summaries link to those records rather than duplicating mutable test outcomes.

## Existing baseline

At source revision `87955f67`, the repository has a lexical loader ratchet,
eight Python checker tests, and the legacy DLL-discovered C++ unit runner.
The test Waf composition covers tier and math tests; it is not a Hammer,
scheduler, VPhysics, or whole-product conformance composition.

Inspection of `utils/unittest/unittest.cpp` shows no explicit failure when zero
tests are discovered. `tools/archlint` does not yet implement the proposed
compiler dependency or target-graph checks. Loader telemetry source exists, but
the checked-in inventory/baseline is not current with it. These observations
are a dated starting point, not permanent descriptions of the repository.

Old test assets and utilities may be reused after validating their build and
oracle. The physics perf viewer, material screenshot utility, and engine test
scripts are useful source material; their existence does not satisfy a gate.

### Installed mechanical-style slice

The [style checker](../tools/stylelint/README.md) now supplies a pinned Source
formatting policy, an incremental `memdbgon.h` include-order check, self-tests
with negative fixtures, and a PR/master workflow. This is R04-STYLE, not the
complete Q-ARCH harness or the shared runner. Its docs-only not-applicable result
must not be counted as runtime coverage. See [RFC 0006](0006-modern-cpp-ownership-and-synchronization.md#mechanical-style-gate)
for its scope; [AGENTS.md](../AGENTS.md) tracks the bounded implementation state.

## Shared execution and evidence

### Runner contract

Provide a small host-side orchestrator and domain test executables integrated
with Waf. The orchestrator may use dependency-free Python. Ordinary contract
tests use explicitly linked registration and construction; they do not discover
tests through the loader or factory chain being migrated. Loader and ABI suites
deliberately exercise real libraries through an isolated test host.

The runner MUST:

- Enumerate selected test IDs and required provider/profile combinations before
  execution, and reconcile them with completed results afterward.
- Fail a required run on zero discovery, unmatched required selectors, missing
  fixtures, unavailable required providers, crashes, timeouts, or incomplete
  results. Optional skips identify their reason and never certify that feature.
- Keep test assertions effective in optimized builds independently of engine
  debug assertions. Propagate native process failures through shell/CI wrappers.
- Support filtering, deterministic seeds, repeats, and subprocess isolation.
  Never rerun failures until green without retaining and reporting the failure.
- Bound test duration and resource consumption. A timeout retains diagnostics
  and fails; terminating an isolated test process does not establish safe
  production shutdown.
- Produce machine-readable per-test results and human-readable first-failure
  diagnostics. Report the first semantic divergence, not only a final hash.

Test the runner with fixtures that discover no tests, deliberately fail, crash,
hang, omit a result, and request an unavailable required provider. Test outcome
handling before using it as a required CI gate.

### Evidence identity

Each gate report MUST identify:

- Source revision and dirty patch/content digest if applicable; submodule and
  external dependency revisions; build flags, compiler, standard library, and
  optimization/sanitizer configuration.
- Contract, fixture, comparator, and tolerance versions; exact test selectors;
  expected and executed test/provider counts; passes, failures, and skips.
- Application/game profile, OS/architecture, provider configuration, precision,
  worker count, solver settings, and hardware/driver details where relevant.
- Inputs, seeds, recorded schedules, commands, logs, traces, crash artifacts,
  measurements, and reproduction instructions.
- Gate decision, accountable owner, accepted deviations, and remaining gaps.

Changing a relevant contract, comparator, provider, consumer, build setting, or
fixture invalidates its prior acceptance evidence. A cached result is usable
only when its relevant inputs are proven unchanged. Manually editing a progress
status or linking an old successful run cannot certify new code.

Proposed artifact families are `tools/quality/`, `unittests/support/`,
`quality/profiles/`, and a generated `quality-results/` directory. These names
are illustrative until implemented. `tools/quality/`, `quality/profiles/` and
`quality-results/` are now installed; `unittests/support/` is not. Domain
fixtures stay with their domain.
Schemas are versioned and reject unknown versions; generated output is stable.
Record installed commands in [`RFC/0005-progress.md`](0005-progress.md) when they work.

### Fixture and oracle policy

Fixtures record provenance, redistribution constraints, feature/profile tags,
expected results, and the comparator version. Small generated/redistributable
fixtures run without game installations. Product content is a separately
declared corpus; unavailable content leaves its product gate unverified.

An oracle is an independent expectation: a contract invariant, simple reference
model, analytical result, recorded characterization, or another implementation.
Legacy behavior is evidence, not automatic authority for preserving a known bug.
Intentional behavior changes get their own reviewed expectation.

Comparators enumerate every normalization and tolerance. Exact comparisons are
used for identities, flags, authored values, and protocol bytes where promised.
Floating-point tolerances are specific to the operation and representation.
Physics trajectories and cross-backend images are not generally bit-identical.
Unknown content and difficult fields cannot be dropped from comparison.

Every critical comparator and conformance suite gets negative fixtures: change
a field, reverse a callback, retain a stale handle, or violate ownership and
verify rejection. Round trips alone are insufficient when a reader and writer
can share the same defect. Review golden updates separately from accepting a
regression; CI never rewrites expected results or architecture baselines.

## Harness obligations

### Q-ARCH: architecture and migration

Extend `tools/archlint` and the existing module manifest. Validate direct and
resolved transitive includes, target/link edges, external include roots,
native-type leakage, ownership, and cycles across supported configurations.
Compile public headers hermetically and strict implementations without legacy
PCHs or unauthorized SDKs. Link checks accompany header checks.

Preserve exact occurrence ratchets, including occurrence multiplicity and stale
entries. New strict code has no inherited debt. Validate migration dependencies,
caller sets, required test selectors, exceptions, and retirement evidence.
Seed invalid projects to demonstrate that each rule rejects its intended leak.

Mutation authority combines private APIs/edit contexts with symbol analysis and
runtime revision/history tests. Lexical matches alone cannot certify exclusive
ownership, DRY, or behavioral substitution.

### Q-FOUNDATION: foundation, lifecycle, and ABI

Run shared provider suites for clocks, paths, memory, execution primitives,
files/processes, library loading, and composition. Inject failure at each
construction/connect/init boundary; verify reverse rollback and a clean second
instance where supported. Track handles, subscriptions, tasks, and borrowers.

Loader fixtures cover missing libraries/symbols/interfaces, duplicate native
handles, nested and concurrent request contexts, structured errors, telemetry
sink lifetime/reentrancy, and unload after all objects and payload destructors.
Observer instrumentation must preserve the operation's result and semantics.

ABI fixtures use frozen consumer headers/builds with documented provenance and
toolchain. Verify actual interface calls and destruction in addition to symbols
and layouts. Rebuilding every fixture against current headers is not evidence
of binary compatibility. Exercise extension family/version/size negotiation and
ensure unadvertised historical interfaces are not implicitly certified.

### Q-EDITOR: headless editor

Drive application operations against explicit per-document sessions. Compare
authored state, graph/reference invariants, selection, committed change events,
history, saved position, and diagnostics. Generate seeded edit/undo/redo/cancel
sequences and compare against a small independent document/history model.

The reference workflow is open, resolve draft, select, transform, undo, redo,
save, reopen, and compile. Add two-document isolation, stale handles, invalid
mixed selections, no-op edits, one-drag/one-history-unit, lossless undo, and
failure atomicity. Reorder asynchronous completion and close documents before
delivery. Run applicable traces through headless, MFC, and GTK hosts when those
providers are available; unavailable MFC execution is an explicit evidence gap.

### Q-JOBS: scheduler model and concurrency

Validate the runtime against an independently implemented small graph model.
Cover sealing, conflicts, explicit and sequence-induced cycles, exactly-once
terminal states, publication, affinity, cancellation, external completions,
allocation failure, and scope lifetime. The production serial executor alone
is not an independent oracle for its own graph validator.

Explore bounded schedules for small graphs; retain failing seeds and schedules.
Use real-thread stress and race detection for publication and sleep/wake races
that a single-thread simulation cannot prove. Test zero/single/multiple compute
workers, small capacities, priority pressure, and cross-executor progress.

For each subsystem compare legacy, serial-graph, and parallel-graph execution
on captured inputs with private outputs. Preserve tick/phase boundaries,
random draws, commit order, and input/prediction latency. Never run both paths'
side effects against the same live world. RFC 0006 queue/fence suites feed this
harness but do not replace whole scheduler tests.

### Q-PHYSICS: VPhysics compatibility

Use isolated IVP and Box3D hosts with coherent physics/collision/surface exports.
Maintain method-to-consumer/test/profile coverage. Compare public semantics,
not solver internals. Use analytical tests for simple motion/conversion and
declared gameplay criteria for solver-dependent outcomes.

Prioritize impulse/force cadence, angular frames, inside-start/exit traces,
pre-/post-impact state, contact mutation, shape-to-object event aggregation,
deletion, ragdoll limits, and collision-resource lifetime. Callback state used
by damage logic is a feasibility gate before broad controller implementation.

Add controller, fluid, constraint, vehicle, world-transfer, and save scenarios
as each feature becomes supported. Test repeatability separately from IVP
compatibility, and restore/replay separately from uninterrupted stepping.
One-worker correctness precedes worker integration or speedup claims.

### Q-CONTENT: content, persistence, and tools

Version the supported VMF/MAP/RMF, BSP physics, `.phy`, cache, and save corpora.
Combine semantic comparisons with independent readers and selected compiler
pipelines. Preserve unknown fields, IDs, metadata, decomposition, and materials;
unsupported or lossy operations produce explicit diagnostics and policy checks.

Fuzz parsers/decoders and test boundary counts, invalid indices, offset/length
overflow, truncation, oversized hulls, and resource-budget exhaustion under
sanitizers. Minimize failures into permanent regression fixtures.

Inject save/write/replace failures; test recovery after interruption where the
provider promises it. Separate atomic replacement from durability and multi-file
recovery. Reject incompatible save schemas before partially restored state is
published. Verify cold/warm cache behavior and recooking from authoritative data.
Tool invocations preserve argv, working directory, environment, and exit status.

### Q-PRESENTATION: rendering and native UI

Keep command/lifetime conformance, image correctness, and native-host integration
as distinct suites. Null and recording providers test their claimed contracts;
real GPUs and native hosts establish material fidelity and presentation behavior.

Cover adapter/profile rejection, immutable capabilities, multiple surfaces,
resize without device recreation, zero-size/minimized recovery, shader artifacts,
CPU/GPU completion, resource retirement, device/context loss, and ordered unload.
Image cases include representative material permutations with reviewed tolerances.

Hammer's GTK bridge must run on declared X11 and Wayland profiles. Exercise
framebuffer/state restoration, shared resources, immutable published textures,
scale/logical-to-drawable conversion, relative navigation, capture/focus loss,
and multi-viewport teardown. Use deterministic event traces for tool semantics
and actual native events for host delivery. Software rendering is a smoke lane,
not evidence for every hardware/driver profile.

### Q-PRODUCT: workflows, performance, and packaging

Launch client, listen-server, dedicated-server, editor, and selected tools from
installed artifacts in controlled environments. Headless independence requires
both dependency inspection and execution without desktop/render providers.

Run scripted map workflows, save/load, reconnect, level transitions, long-session
resource checks, and shutdown with jobs/I/O pending. Include supported demo,
HLTV/replay, and game variants. Test interactions explicitly: physics tasks and
scheduler capacity; model eviction during bones; editor closure during compile;
render submission followed by delayed GPU completion; module unload after job
payload destruction. A boundary change schedules its interacting suites.

Use declared supported combinations rather than implying the entire Cartesian
product is supported. Pairwise coverage can supplement testing, but every
advertised shipping profile and every identified high-risk interaction needs
its own acceptance evidence. Native in-process plugins are not a security
boundary; malformed-input testing does not change that fact.

## Performance and promotion

Record baselines before optimization with fixed corpus, revisions/configurations,
hardware/driver, warmup, repetitions, and sample counts. Measure median and tail
latency, CPU/GPU time where applicable, memory, startup/load/save, queue delay,
copying, conversion, and synchronization. Attribute improvements with traces;
higher CPU utilization alone is not success.

Set absolute budgets and regression allowances per profile before measurement.
RFC 0004's proposed 10% p95 physics/memory regression threshold triggers review;
it does not replace absolute budgets or authorize gameplay regressions.
Correctness/sanitizer lanes and performance lanes use appropriate separate
configurations. Test low-core and small workloads as well as large benchmarks.

Default promotion requires all required domain and product gates, a support
matrix, recorded manual criteria where feel must be judged, and tested rollback.
A green optional prototype does not promote a provider. Preserve old builds and
compatible fixtures while rollback is required; new backend saves need not be
readable by the previous backend unless explicitly promised.

## CI and gate installation

| Lane | Required work |
| --- | --- |
| Every PR | Installed architecture/schema/evidence checks, affected hermetic builds, deterministic contracts and regression fixtures |
| Relevant merge gate | Supported native contract/corpus matrix, differential comparisons, ASan/UBSan and separate TSan lanes where supported, full compiler dependency checks |
| Native boundary changes | Real UI/window/GPU tests and supported interaction profiles before claiming that boundary complete |
| Scheduled stress | Larger randomized schedules, fuzzing, soaks, hardware/driver coverage and controlled benchmarks |
| Release/default promotion | Complete required profile evidence, product workflows, budgets, installed-package and rollback checks |

Scheduled lanes do not substitute for required acceptance evidence. If hardware
or content is unavailable, unrelated work may advance but its dependent gate
stays unverified. Scope affected-test selection through declared dependencies;
fall back to broader relevant suites when impact is uncertain.

Install each gate only after its runner and negative fixtures work. Record the
command, coverage, blind spots, and dependencies in progress evidence. Static
checks run before expensive jobs; compiler-grounded checks run once actual
configuration/dependency data exists. Never claim a proposed command already
works or infer runtime correctness from build success.

## Delivery increments and done criteria

| Increment | Deliverable | Done |
| --- | --- | --- |
| Q0 | Baseline profiles and current-check audit | Build/content availability and pre-existing failures recorded; no unsupported acceptance claims |
| Q1 | Shared runner, fixtures, result schema | Missing-test/crash/timeout/skip fixtures fail correctly; reproducible per-test artifacts |
| Q2 | Q-ARCH and Q-FOUNDATION | Installed graph/header checks reject seeded leaks; lifecycle/loader/ABI reference suites pass |
| Q3 | Domain oracles | Editor semantic comparator, independent scheduler model/three-mode comparison, and isolated physics comparison detect seeded defects |
| Q4 | Content and native integration | Required corpus, recovery, real presentation/UI, and cross-domain scenarios execute for declared profiles |
| Q5 | Promotion and maintenance | Profile gates include performance, packaging and rollback; dependency/comparator changes invalidate affected evidence |

These are infrastructure increments, not a demand to finish every domain suite
before any implementation. Build the smallest trustworthy harness for the next
migration, then grow its required coverage with each feature. The unified ranked
roadmap in AGENTS.md interleaves this work with the domain RFC milestones.

## Decision

Use shared execution/evidence infrastructure with domain-owned correctness
oracles. Preserve exact architecture ratchets, prove test sensitivity with
negative fixtures, and require current profile-specific evidence for cutover,
retirement, and default promotion.
