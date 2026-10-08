# RFC 0003 progress: Dependency-aware job system and frame scheduling

Updated: 2026-09-25 (current-state summary); the sections below keep their
dated evidence.
Source revision at assessment: `0649f377` (working tree; AGENTS.md portfolio row: R10)

**Current state (2026-09-25, `d6260d90`):** both scheduler increments are
merged into `subsystem-refactor`; their worktree branches are not needed to
reproduce them. The manifest has 16 Q-JOBS rows (14 required; the two
`jobsystem.radiosity.tsan` rows are optional and need `CONFORMANCE_TSAN`).
Defaults: `host_frame_graph 1`, `cl_render_start_graph 0`,
`host_thread_mode 0`; the particle, bone, renderable and entity-packing cohorts
`2` (pooled); query-cache and carve `0`. The Portal launchers (`run.conf`,
`play_p2`) also pass `cl_render_start_graph 2` and `sv_querycache_job_graph 2`,
and `run.conf` passes `portal_carve_job_graph 2`. No RFC 0003 phase gate is
closed.

**Latest increment (2026-09-24):** [scheduler nodes](0003-scheduler-nodes-progress.md) (render sub-nodes, threaded deadlock, pool bounds and budgets, declared frame graph; section 6 records the 2026-09-25 `host_thread_mode 1` launcher trial and its withdrawal); before it, [scheduler trust](0003-scheduler-trust-progress.md)
runs the host frame as an ordered serial graph that matches captured legacy
frames (R10). It also makes the engine pool TSan-clean on native fixtures,
fixes the CTSQueue crash, adds bounded worker steal deques, and forbids waits
from running unrelated work (R20). It closes neither row.

**Earlier performance increment (2026-09-23):** [scheduler microbenchmarks and
contract-preserving optimization](0003-scheduler-performance-progress.md) adds
oracle-validated microbenchmarks, a differential seal oracle and an executor
stress suite. It optimizes Seal, both graph executors, the batch facade and dynamic
scopes without changing their contracts. It closes no gate.

**Latest production-caller increment:** [bounded batch migrations and native
Portal evidence](0003-batch-migration-progress.md) records the particle, bone,
renderable and entity-packing graph paths (pooled by default since 2026-09-22),
the query-cache and carve kernels, borrowed engine-pool execution,
affinity fix, real C++11/C++20 tests, measured dispatch costs and remaining race /
semantic / performance gates. It supersedes the older statements below that no
`game/` or `engine/` callers have migrated or that a runnable game is unavailable.
The earlier substrate evidence is retained as historical evidence; no full RFC
gate is closed by the new slice. `jobsystem/expected.h` now aliases the foundation
result vocabulary, so the historical convergence gap has also been resolved.

This file is the human-readable, durable progress record for RFC 0003. It covers
**Phase A (Inventory and baseline)** and the first delivery of **Phase B
(Contracts and serial graph)** and **Phase C (Compute executor)**, plus the
frame-composition infrastructure the particle pilot (Phase D) rides on.

RFC 0003 is Proposed; this record marks **no** gate complete. The scheduler
substrate below is delivered and tested at the module level. The **2026-09-22
increment** additionally delivers the module-implementable contracts the R10 gate
named as gaps: lane affinity with a main-thread pump and blocking-IO lane and
stall detection, an exactly-once external-completion adapter, dynamic child
scopes, and the **RFC 0005 Q-JOBS independent + adversarial harness** (an
independent reference model, seeded stress comparison, and negative executors
that prove the comparator detects planted defects). What the R10 gate still lacks
is **not** module code: it is the game-subsystem source migration, the
legacy-capture comparison, and the runtime performance/latency baselines, all
gated on a buildable/runnable game profile, captured workloads, and hardware that
are unavailable here (see gaps 5-6). Per the RFC, Phase A does not create a
speculative machine-readable baseline "before the scheduler API exists"; the
durable Phase A artifact remains this inventory map.

Scope and owner of this increment: build the portable scheduler runtime and the
host frame-composition seam as a self-contained, strict **C++20** module, and
bridge it to the **real engine `vstdlib` thread pool** so actual engine worker
threads execute dependency-aware job graphs. Verified by compilable/running
tests in the `--tests` configuration. Game-subsystem source
(`game/client/particlemgr.cpp`, `engine/host.cpp`) is not yet modified — that
migration needs the game build and captured workloads (see gaps).

## Status

| Work item | Phase | Status | Evidence |
| --- | --- | --- | --- |
| Catalog job/thread facilities, config defaults, seams, hazards | A | Complete (static) | Inventory sections below; each seam verified to a current `file:line` |
| Reproducible **runtime** baseline (captures, budgets) | A | **Captures recorded (2026-09-26); frame budgets open (R21)** | The versioned legacy host-frame capture `quality/fixtures/host-frame/testchmb_a_00-v1.json` is rechecked in both modes by `jobs.host-frame-capture` (R10-CAPTURE). `jobs.legacy-captures` is `recorded`. Cohort gameplay captures (`jobs.cohort-captures`) and frame budgets (`jobs.frame-budgets`) are owned by R21 |
| First particle workload selection | A | Decision D-A1 | `CParticleMgr::UpdateNewEffects` |
| `Expected<T,E>` | B | Delivered; aliases `foundation::Expected` | `public/jobsystem/expected.h` (includes `foundation/expected.h`); used throughout |
| Graph builder + validator (names, handles, cycles incl. sequence-induced, resource conflicts) | B | Delivered + tested | `jobsystem/job_graph.cpp`; `jobsystemtest` |
| Immutable SealedGraph + stable topological order | B | Delivered + tested | `public/jobsystem/job_graph.h` |
| Deterministic serial reference executor | B | Delivered + tested | `jobsystem/deterministic_executor.cpp` |
| Bounded worker-pool executor; release/acquire publication; no worker blocks on deps | C | Delivered + tested | `jobsystem/parallel_executor.cpp`; TSan-clean |
| Host frame coordinator + frame contributor contract + rollback path | B/D | Delivered + tested | `jobsystem/frame_graph.cpp`; `jobsystemframetest` |
| Particle reference pilot (gather/compute/commit + bone attachment) | D | Delivered as reference; **not wired to engine** | `jobsystem/pilot_particles.cpp` |
| Three-mode equivalence (legacy ref vs serial graph vs parallel graph) | B/C/D | Passing on captured inputs/private outputs | `jobsystemframetest` |
| Wave executor over an abstract worker backend (`IWorkerBackend`) | C | Delivered + tested | `jobsystem/pooled_executor.cpp`; TSan-clean |
| **Legacy pool bridge: real `vstdlib` `CThreadPool` executes job graphs** | C/R20 | **Delivered + tested** | `vstdlib/jobgraph_pool_bridge.cpp`; `jobsystembridgetest` |
| Lane affinity: main-thread pump, blocking-IO lane, stall detection | C | **Delivered + tested (2026-09-22)** | `jobsystem/parallel_executor.cpp`; `jobsystemtest`; conformance `jobsystem.scheduler` |
| External-completion adapter (exactly-once; register/complete/cancel/repeat races) | C | **Delivered + tested (2026-09-22)** | `public/jobsystem/external_completion.h`; `jobsystemtest`, `jobsystemqjobstest` |
| Dynamic child scopes (reserved completion ownership; producer/continuation; no worker blocks) | — | **Delivered + tested (2026-09-22)** | `jobsystem/dynamic_scope.cpp`; conformance `jobsystem.dynamicscope` |
| Q-JOBS independent model / adversarial schedules / negative executors | B/C | **Delivered + tested (2026-09-22)** | `unittests/jobsystemtest/qjobstest.cpp`; conformance `jobsystem.qjobs`; TSan-clean |
| Game-subsystem source migration; legacy-capture comparison | B/D | **Partial (2026-09-25)** | Cohorts: [batch migration](0003-batch-migration-progress.md). Host frame and render steps: live legacy/graph captures match ([trust](0003-scheduler-trust-progress.md), [nodes](0003-scheduler-nodes-progress.md)). Semantic gameplay captures of the cohorts are missing |
| Runtime performance / latency / low-core budgets | D+ | **Partial (2026-09-25)** | Pool capacity and overhead budgets, 1-worker rows included: `quality/budgets/scheduler-v1.json` (Linux desktop). No frame, p95/p99, mobile or power budget (`jobs.frame-budgets` is `missing` in `quality/baseline.json`) |

## Verification performed (this increment)

All at working-tree revision `a4f6f95f`.

- **In-tree Waf build** (real toolchain): `jobsystem` static lib + two test
  programs build under the configured `--tests` profile. The module sets
  `-std=c++20` on its own targets, overriding the tree-wide `-std=c++11`
  (`wscript:579`) — it is a deliberately migrated strict C++20 target (RFC 0006),
  self-contained over standard atomics/threads with no tier0/legacy-jobthread
  dependency.
  - `./waf build --targets=jobsystem,vstdlib,jobsystemtest,jobsystemframetest,jobsystembridgetest`
  - `jobsystemtest` → 212 checks, 0 failures
  - `jobsystemframetest` → 84 checks, 0 failures
  - `jobsystembridgetest` → 91 checks, 0 failures — job graphs (the particle
    gather/compute/commit graph and a 70-node stress graph) run on the **real
    engine `vstdlib` `CThreadPool`** worker threads via the production
    `CParallelProcessor` fork/join path, matching the serial reference and the
    deterministic executor bit-for-bit, exactly-once. Run with the built
    tier0/vstdlib `.so`s on `LD_LIBRARY_PATH`.
- **Sanitizers** (standalone clang, same sources): TSan clean and ASan+UBSan
  (`-fno-sanitize-recover=all`) clean on both suites — the parallel executor's
  producer→consumer publication is race-free, satisfying the RFC requirement that
  "a single-thread simulation cannot establish native memory visibility."
- **Warnings**: g++ `-Wall -Wextra -Wshadow -O2` clean.
- **Architecture gates**: `archlint check --changed` matches the RFC 0001 Phase A
  baseline (no new loader/ABI violations); `python3 -m unittest discover -s
  tools/archlint/tests` → 34 pass.

### 2026-09-22 increment (lane affinity, external completion, dynamic scopes, Q-JOBS harness)

At working-tree revision `0649f377`. All standalone under g++ 16.2.1 and clang++
22.1.8; also run through the shared conformance runner.

- **Shared conformance gate**: `python3 tools/quality/conformance.py check` →
  **33/33 suites pass** (g++, profile `linux-headless-core`, `-Wall -Wextra
  -Werror`), including the three Q-JOBS rows `jobsystem.scheduler` (274 checks),
  `jobsystem.dynamicscope` (140 checks) and `jobsystem.qjobs` (7719 checks incl.
  200× a Complete/Cancel thread race). Evidence under `quality-results/`.
- **Lane affinity**: `ParallelExecutor` now routes Compute/Sequence to compute
  workers or the pumping main thread, `MainThread` to the caller only (verified to
  run on the caller thread id, never a worker), and `BlockingIO` to a dedicated
  blocking lane (or the main pump when there is none) so a compute worker never
  absorbs a blocking wait. A lane with no servicer is reported as
  `RunResult::stalled` with the affected jobs + Success-dependents left
  non-terminal — the RFC's "wait that cannot make progress" diagnostic, detected
  up front rather than hung.
- **External completion**: `ExternalCompletion` gives exactly-once transitions
  (first `Complete()`/`Cancel()` wins; repeats are no-ops) and exactly-once
  continuation firing whether registered before or after the event; the
  Complete/Cancel/Wait thread race is TSan-clean. `MakeExternalWait` adapts a
  token onto a BlockingIO job so an external completion gates a graph consumer
  through normal Success/cancel semantics.
- **Dynamic child scopes**: `DynamicScope` reserves completion ownership with an
  owner keepalive before any child is published, admits children that depend only
  on already-admitted children, splits producers into producer + continuation
  (verified to complete with a *single* worker, proving no worker blocks on child
  jobs), cascades cancel, and rejects children once drained.
- **Q-JOBS independent + adversarial harness** (`qjobstest.cpp`): an independent
  fixpoint reference model (order-invariance proven over seeded ready-orders),
  seeded stress comparison of the Deterministic and Parallel executors against the
  model with exactly-once run counts and a producer→consumer publication
  read-check, and four **negative executors** (drop / duplicate / misorder /
  no-cancel) that the comparator + run-count + publication oracles each detect —
  proving the suite catches planted defects rather than passing vacuously.
- **Sanitizers**: TSan and ASan+UBSan (`-fno-sanitize-recover=all`) clean on
  `jobsystemtest`, `dynamicscopetest`, and `qjobstest`; 80× stress loop, no hangs.
- **Style**: pinned clang-format 22.1.8; new files conform in full and the
  branch-diff check (`stylelint --changed --base <merge-base>`) reports no
  jobsystem findings.

Note: the shared Waf tree was configured by a concurrent agent for a
dedicated-server profile (no `--tests`), so this increment was verified through
the standalone conformance runner and direct g++/clang builds rather than the
in-tree Waf `--tests` targets. The new Waf targets (`jobsystemdynamictest`,
`jobsystemqjobstest`, and `dynamic_scope.cpp` in the `jobsystem` lib) are wired in
the wscripts but were not built in the shared tree to avoid disturbing that lock.

## What the substrate provides (delivered contracts)

- **Build/validate/seal/submit** discipline: a graph is built privately, then
  `Seal()` validates and freezes it; roots cannot start mid-construction.
  Submission failure returns a structured `GraphError` and leaves the graph
  unexecuted.
- **Validation** rejects (rather than repairs): empty names, stale/unknown
  handles, self-dependencies, cycles *including sequence-induced ordering edges*,
  and ambiguous resource conflicts. Resource conflict uses DAG reachability: an
  unordered write/write or read/write on the same `(domain,epoch,partition)` is
  rejected; read/read and disjoint partitions are accepted. It never invents an
  order from registration order.
- **Two executors, one contract** (`IGraphExecutor`): the deterministic executor
  runs the stable topological order as a reference; the parallel executor uses a
  bounded worker budget (0..N) with a single synchronized ready queue (the RFC's
  preferred first implementation). Both must — and in tests do — agree on every
  job's terminal state.
- **Dependency/failure semantics**: only prerequisite-satisfied jobs become
  ready; an unfinished job never occupies a worker to wait. A `Success` prereq
  that did not succeed cancels its consumer (cascading); a `Terminal` prereq runs
  cleanup/error continuations regardless of outcome. Cooperative cancellation
  never force-terminates running work. Exactly-once execution holds under
  concurrency.
- **Frame composition**: `FrameCoordinator` emits one ordered legacy phase per
  existing phase on a sequence lane; an `IFrameContributor` inserts gather/
  compute/commit nodes into a bracketed region without reordering legacy phases.
  Building with contributors disabled is the legacy/rollback path.
- **Publication argument**: the scheduling mutex provides the happens-before edge
  from a producer's output writes (before it locks to finish) to a consumer's
  reads (after it locks to dequeue). This is a simple, TSan-verified guarantee,
  to be revisited if/when a lock-free ready structure earns its evidence.

## Delivered in the 2026-09-22 increment (were gaps 1-4)

1. **Q-JOBS independent/adversarial harness** (RFC 0005 R02): **delivered.** An
   independent fixpoint state model, seeded stress comparison against the
   production Deterministic/Parallel executors, and negative executors that
   deliberately drop / duplicate / misorder / skip-cancel to prove the comparator
   detects defects. See `qjobstest.cpp` / conformance `jobsystem.qjobs`. Schedule
   exploration is exercised through seeded graphs × worker counts × real-thread
   stress + TSan rather than exhaustive interleaving enumeration.
2. **Affinity / main-thread pump enforcement**: **delivered.** MainThread work
   runs only on the pumping caller; a blocking-IO lane keeps blocking work off
   compute workers; an unserviced lane is a detected stall, not a hang.
3. **Blocking-IO lane and external-completion adapter**: **delivered.** Dedicated
   blocking lane plus `ExternalCompletion` with the four exactly-once races
   (register-before/after, repeat, cancel) and a `MakeExternalWait` graph adapter.
4. **Dynamic child scopes**: **delivered.** `DynamicScope` with reserved
   completion ownership, producer/continuation splitting, no worker blocking on
   children, cascade cancel, and post-drain rejection.

## Known gaps (still required before the R10 gate closes)

(2026-09-25) Gaps 5 and 7 below are superseded. The cohorts, the host frame
and the render-start region have migrated (see the status table), and
`jobsystem/expected.h` now aliases `foundation::Expected`. Gap 6 is partly
met: host-frame live captures and scheduler budgets exist, but semantic
gameplay captures and frame budgets do not.

5. **Game-subsystem source migration**: the real engine *thread pool* (`vstdlib`)
   now executes job graphs (delivered above), but nothing in `game/` or `engine/`
   game-subsystem source is modified. The particle pilot is a faithful model, not
   a migration of `CParticleMgr::UpdateNewEffects`. Wiring the pilot into that
   call site requires the game/client build, captured particle workloads, and the
   equivalence/latency/rollback gates of Phase D (the `--tests` config does not
   build `game/client`).
6. **Legacy-capture comparison and performance acceptance**: no baseline capture,
   no measured improvement, no low-core regression budget — all gated on a
   buildable/runnable engine profile (see Phase A runtime gap).
7. **`Expected<T,E>` convergence**: the scoped type must fold into the engine-wide
   R05 result vocabulary when it lands, not remain a second authority. A
   `foundation.expected` conformance suite has since appeared in the shared
   manifest, so this convergence is becoming actionable; it is a deliberate
   follow-up, not part of this increment.

## R10-RUNNERS: task runners, sequences and virtual time (slice done 2026-09-26)

**Scope** (roadmap R10, `active`; RFC 0001 "Threads, sequences, and
injectable scheduling", RFC 0003 "Sequences and physical affinity"). R10's
row names "runner/clock/sequence contracts", but no task-runner contract
exists yet: only `platform::IMonotonicClock` and a test-only virtual clock.
This slice adds:

- **The contract** `public/platform/contracts/task_runner.h`
  (`platform.task-runner.v1`), with standard-library types only:
  - a move-only `Task`;
  - `ITaskRunner` for independent posting, with delayed posting measured
    on the runner's clock;
  - `ISequencedTaskRunner` for ordered, non-overlapping execution, with a
    current-sequence query;
  - `ISingleThreadTaskRunner` for physical-thread affinity, with a
    current-thread query;
  - a `[[nodiscard]]` post result that says whether a shut-down runner
    refused the task.
- **Providers** in a new capability module `platform.runners`, with a
  strict C++20 Waf library:
  - `VirtualClock`, an installed virtual-time `IMonotonicClock`;
  - `ManualTaskRunner`, a deterministic sequenced runner on virtual time.
    The owner runs it with `RunUntilIdle` and `AdvanceBy`, so consumers'
    tests need no sleeps;
  - `ThreadTaskRunner`, a native single-thread runner. Its owner shuts it
    down with acknowledgment: queued tasks are destroyed without running,
    and no task runs after `Shutdown` returns;
  - `SequencedTaskRunner`, a sequence over any `ITaskRunner`, so ordered
    work needs no dedicated thread.
- **One shared suite** runs against every provider (`platform.task_runner`),
  and a sensitivity suite runs it against bad providers. The bad providers
  reorder a sequence, let a sequence overlap, run delayed work early,
  accept and then drop tasks after shutdown, and run tasks after shutdown.
  Each must be caught.
- **Consumer:** the next Hammer slice, async F9, which builds on a
  `ThreadTaskRunner` and returns to the GTK main loop.
- **Out of scope:** a pool-backed `ITaskRunner` over the engine
  `CThreadPool`, and migrating engine queues to runners.

**Delivered.**

- **The contract** `public/platform/contracts/task_runner.h`, documented in
  `unittests/platformtest/contracts/platform.task-runner.v1.md`. The owner
  shuts a runner down; consumers cannot.
- **`platform.runners`** (`platform/runners/`, Waf `platform_runners`,
  strict C++20; its only edges are `foundation` and `platform.contracts`):
  - `VirtualClock` and `ManualTaskRunner`. `AdvanceBy` stops at each due
    time, so a task sees the clock at its own due time.
  - `ThreadTaskRunner`. Its thread id is fixed at construction, and
    concurrent `Shutdown` calls all return after one join.
  - `SequencedTaskRunner`. Its state is shared with in-flight base tasks,
    and it keeps delayed tasks itself, so `Shutdown` destroys them. A base
    runner's refusal shuts the sequence down and refuses the post.
- **Tests** in `unittests/platformtest/task_runner/` (module
  `platform.runners.tests`), with a 4-thread `TestPool` that is test-only.
- **Found and fixed by the suite:**
  - the sequence adapter answered `kAccepted` for a task its refusing base
    had just dropped;
  - its delayed tasks survived `Shutdown` inside the base runner.

**Evidence (2026-09-26).**

| Check | Result |
| --- | --- |
| `platform.task_runner`, g++ and clang++, default and release | 110 checks, pass |
| `platform.task_runner` `--repeat 20` | 20 of 20 |
| `platform.task_runner.tsan` (clang++, TSan) | 110 checks, clean |
| `platform.task_runner.sensitivity` | 8 checks: the control passes, and each of the 7 broken providers fails on its own clause |
| Q-FOUNDATION + Q-EDITOR headless, g++ and clang++ | 104 of 105 each; the skip is the optional TSan lane |
| `archlint check --all --compile-deps build-r03-tools`, `targets --verify --partial`, `hermetic` (g++ and clang++, 64 headers) | pass |
| `baseline.py audit --group static` | 11 pass and 1 known fail (`roadmap.check`, R16/R14), 0 deviations |
| stylelint on the slice's files | clean |

`quality.selftest` took 36.8 s against its 20 s budget in that audit; the
budget is advisory.

**Still needed for R10 to be `done`:**

1. **Runner bindings** (Phase B deliverable). Done in R10-BINDINGS below. The scheduler's affinity lanes
   (main-thread pump, blocking I/O) and its external completion should bind
   to `ISingleThreadTaskRunner` / `ISequencedTaskRunner`, rather than only
   to the jobsystem's own lane types.
2. **A reproducible baseline** (Phase A exit; `jobs.legacy-captures` is
   `partial`, owned by R10). The legacy and graph host-frame captures match
   live, but no versioned capture is recorded or checked.
3. **A consumer:** Hammer's asynchronous F9 build is the first planned
   consumer of the runners (RFC 0002 record).

## R10-BINDINGS: the executor's affinity lanes bound to runners (slice done 2026-09-26)

**Scope** (roadmap R10; RFC 0003 Phase B deliverable "runner bindings", and
"Sequences and physical affinity": "An execution lane in a frame diagram is
a binding to that contract").

- **Options.** `RunOptions` gains `mainThreadRunner`
  (`platform::ISingleThreadTaskRunner`) and `blockingRunner`
  (`platform::ISequencedTaskRunner`). When set, `ParallelExecutor` posts
  that lane's ready jobs to the runner instead of servicing them on the
  pumping caller or on dedicated blocking workers.
- **Completion** publishes through the scheduling mutex like any other job,
  and a bound lane never stalls.
- **A caller that already is the runner** (on its thread or in its sequence)
  services that lane itself. It cannot wait for its own runner.
- **A runner that refuses a job, or drops it at shutdown,** resolves that job
  as unserviceable (the existing stall diagnostic). The run returns
  `stalled` instead of hanging.
- **Inline mode** (0 workers, the serial low-capacity mode) and the
  deterministic executor keep servicing every lane on the caller, and ignore
  bindings.
- **Oracle:** a new Q-JOBS suite, `jobsystem.runner-bindings`, checks:
  - thread and sequence identity of bound jobs;
  - cross-lane dependencies and publication;
  - no stall where the unbound configuration stalls;
  - a caller that is its own runner;
  - refusal and drop-at-shutdown as stalls, without hangs;
  - cancellation;
  - terminal-state equivalence with `DeterministicExecutor` on seeded
    random graphs;
  - a TSan lane.
- **Negative controls:** the unbound configuration must stall, and seeded
  executor mutants must fail.

**Delivered.**

- **`RunOptions::mainThreadRunner` and `blockingRunner`**
  (`public/jobsystem/graph_executor.h`). `ParallelExecutor` posts each bound
  job from `EnqueueLocked`. Inline mode, `DeterministicExecutor` and the
  wave-based `PooledExecutor` ignore bindings, as documented.
- **An ownership token** settles each posted job exactly once as ran,
  accepted-then-dropped, or refused. The runner never runs a task inside the
  post, so posting under the scheduling mutex cannot re-enter it.
- **A caller that is the runner** (its thread, or its sequence) becomes the
  pump for that lane.
- **Found and fixed by the suite:** `ThreadTaskRunner::BelongsToCurrentThread`
  compared against its exited thread's id. glibc reuses thread ids, so a new
  thread "belonged" to a shut-down runner, and the executor then treated the
  caller as the runner.
  - The id is now cleared after the join.
  - `thread.no-owner-after-shutdown` in `platform.task_runner` checks this.
    All 64 of its probe threads reused the id here, and the pre-fix code
    fails the clause 64 times.

**Evidence (2026-09-26).**

| Check | Result |
| --- | --- |
| `jobsystem.runner-bindings`, g++ and clang++ | 18 checks, pass; the unbound control stalls 16 jobs |
| `jobsystem.runner-bindings.tsan`, `platform.task_runner.tsan` | clean |
| `platform.task_runner` | 111 checks, pass |
| Seeded executor mutants | 9 of 9 detected: bindings ignored (main, blocking), refusal or drop unresolved, caller-is-runner undetected, cancel ignored, no wake after completion, bound lanes still stall, bound jobs on the caller. Four are hangs, which the suite's own timeout also fails. |
| Q-JOBS + Q-FOUNDATION headless, g++ and clang++ | 59 of 65 each; the 6 skips are the optional TSan lanes |
| `waf build --targets=jobsystem` (`build-r03-tests`) | pass |
| `archlint check --all` | pass |
| stylelint on the changed lines | clean |

`RunOptions` gained two pointers. Other trees rebuild `jobsystem`'s
consumers on their next build; I rebuilt none of them here, to spare the
host.

**Still needed for R10 to be `done`:**

- a versioned legacy host capture (`jobs.legacy-captures`, Phase A);
- the first product consumer of the runners: delivered by Hammer's
  asynchronous F9 (RFC 0002 R08-ASYNC-BUILD, 2026-09-26). It uses
  `MapBuildQueue` over a `ThreadTaskRunner`, and a `GlibTaskRunner` that
  passes the shared suite. It also refined contract clause 5 for main-loop
  runners.

## R10-CAPTURE: a versioned legacy host-frame capture (slice done 2026-09-26)

**Scope** (roadmap R10; RFC 0003 Phase A exit "reproducible baseline", and
R10's "ordered serial host graph matches legacy captures").
`jobs.legacy-captures` is `partial`: live legacy-versus-graph captures match,
but nothing is versioned or rechecked.

- **The fixture:** a gzip-compressed legacy capture in
  `quality/fixtures/host-frame/`, with a JSON manifest. The manifest holds
  the map, the startup commands, the frame and event counts, the declared
  `ia` tolerance, the recording revision and the rule for updating it. The
  capture is `-hostframetrace` on `testchmb_a_00`, native Vulkan, headless,
  `host_framerate 0.015`, `cl_clock_correction 0`.
- **`tools/quality/host_frame_baseline.py`:**
  - `record` writes a reviewed update;
  - `check` boots the installed product in both modes (`host_frame_graph`
    0 and 1) and compares each run to the fixture with
    `host_frame_capture.compare`, reporting checks-v1.
- **Self-tests:** the fixture must be complete and match its manifest. A
  mutated capture (a dropped event, swapped order, a changed tick field) must
  fail against it, and a tolerated `ia` change must pass.
- **Registration:** a baseline check, `jobs.host-frame-capture` (GPU, the
  Portal native tree). `jobs.legacy-captures` then records host-frame
  captures, and semantic gameplay captures of the cohorts go to a separate
  entry owned by R21.
- **Known limit:** the first host frame runs before any command executes, in
  either mode, so frame 0 of every capture takes the default (graph) path.
  Both paths produce the same trace.

**Delivered.**

- **The fixture:** `quality/fixtures/host-frame/testchmb_a_00-v1.json` and
  `.trace.gz` (35 KB). It holds 395 frames and 8,312 host calls, recorded at
  `73e37bd9+dirty` from the rebuilt `build-r03-portal-native` install.
- **`tools/quality/host_frame_baseline.py`** (`check`, `record`). Baseline
  check `jobs.host-frame-capture` (runtime, serial, 60 s budget).
- **Baseline entries:** `jobs.legacy-captures` is now `recorded`. The
  cohorts' semantic gameplay captures moved to a new
  `jobs.cohort-captures` (`partial`, R21).

**Evidence (2026-09-26).**

- **Before recording:** two legacy and two graph runs. Both graph runs
  matched the first legacy run exactly (395 frames, 8,312 events). The
  second legacy run matched it with 6,735 `ia` differences, all within the
  declared 1e-4.
- **`check`:** both modes match the fixture (5 checks, about 18 s, two
  boots).
- **`baseline.py audit --check jobs.host-frame-capture`:** pass, 0
  deviations.
- **Self-tests** (`test_host_frame_baseline.py`, 8 cases):
  - a copy of the fixture matches;
  - a dropped event, two swapped events, a changed `ht`, and an `ia` change
    beyond tolerance each fail;
  - an `ia` change inside the tolerance passes and is counted;
  - a manifest whose counts disagree is rejected.

**Still needed for R10 to be `done`** (RFC 0001 rank 11 exit):

1. **Diagnostic sequence checks.** RFC 0001: "Thread-affinity and
   sequence-affinity checks are enabled in diagnostic builds."
2. **Adapting an existing queue to the runner contracts** ("Adapt existing
   queues; do not replace the job system wholesale"): a runner over the
   engine's `vstdlib` thread pool.

## R10-SEQCHECK: diagnostic sequence-affinity checks (slice done 2026-09-26)

**Scope** (roadmap R10; RFC 0001 rank 11 "diagnostic sequence checks", and
"Thread-affinity and sequence-affinity checks are enabled in diagnostic
builds").

- **Current sequence.** `platform::CurrentSequence()` (in
  `task_runner.h`) names the sequence whose task the calling thread is
  running. `ManualTaskRunner` and `SequencedTaskRunner` set it around each
  task with `ScopedCurrentSequence`. Single-thread runners leave it unset:
  their sequence is their thread.
- **`platform::SequenceChecker`** (`sequence_checker.h`, contract module,
  header only) binds on first use to the current sequence, or to the thread
  when no sequence is running. `CalledOnValidSequence()` then holds only on
  that sequence, from whichever thread runs it; `Detach()` rebinds.
  `PLATFORM_CHECK_SEQUENCE(checker)` aborts on a violation in diagnostic
  builds (on unless `NDEBUG`, overridable with `PLATFORM_SEQUENCE_CHECKS`).
- **Consumer:** `hammer::app::MapBuildQueue` checks that its reply-sequence
  state is used only on that sequence.
- **Oracle:** `platform.sequence_checker` checks that one sequence over a
  4-thread pool stays valid across threads, another sequence or a plain
  thread is invalid, thread binding works outside tasks and inside
  single-thread runners, nested sequences use the innermost, `Detach`
  rebinds, and the diagnostic macro aborts in a child process. A
  thread-only checker must fail the cross-thread sequence clause, which is
  the negative control.
- **Limit:** the current-sequence identity is per module image, so checkers
  and the runners they observe must be linked into the same image. This
  holds for the static composition that iOS requires and that the tests and
  Hammer use.

**Delivered and evidenced (2026-09-26).**

- **`public/platform/contracts/sequence_checker.h`** and the
  current-sequence identity in `task_runner.h`. `ManualTaskRunner` and
  `SequencedTaskRunner` set the identity.
- **`MapBuildQueue`** states its reply-sequence rule with the checker. Its
  suite builds with checks on and queries `Busy()` on the reply sequence.
- **`platform.sequence_checker`:** 16 checks on g++ and clang++, in default
  and release. It checks that one sequence stays valid across the threads
  of a 4-thread pool; the thread-only checker fails that clause, as the
  negative control requires. The macro aborts a child process that breaks
  the rule.

## R10-POOLRUNNER: the engine thread pool as a runner (slice done 2026-09-26)

**Scope and delivery** (RFC 0001 rank 11 "Adapt existing queues; do not
replace the job system wholesale").

- **The factory:** `public/vstdlib/task_runner_pool_bridge.h`
  (`CreateThreadPoolTaskRunner(IThreadPool *, platform::ITaskRunner *timer)`,
  `ShutdownThreadPoolTaskRunner`, `DestroyThreadPoolTaskRunner`), in
  `vstdlib/task_runner_pool_bridge.cpp`.
  - It posts each task to the borrowed `CThreadPool` as a `JF_QUEUE` job.
    Without that flag, `AddJob` runs a job inside the post when no worker
    is idle.
  - Delays wait on a timer runner that the composition root injects, so
    `vstdlib` needs only the header-only contract and no new link
    dependency in any product.
  - Queued and delayed tasks live in shared state, which shutdown drops.
  - A pool without threads, a null pool, or a missing timer is refused.
- **`platform_runners_legacyabi`** twin (old libstdc++ ABI) for legacy
  consumers such as the test.
- **Test:** `unittests/jobsystemtest/poolrunnertest.cpp`, Waf program
  `jobsystempoolrunnertest` in the `--tests` tree (`scheduler-tests` owner
  group). It runs the shared runner suite against the pool runner and
  against a `SequencedTaskRunner` over it, on a real 4-thread pool.
- **The shared suite was strengthened** after two seeded pool mutants
  survived it:
  - `never-inline-under-load` holds the workers with blocking tasks until
    they have settled, then posts;
  - shutdown now happens while work is queued behind busy workers.
  - The blocking tasks give up after 250 ms, so a broken provider fails
    instead of hanging.
- **Registered:** manifest `corpus.jobs.pool-runner` (`min_checks` 36) and
  baseline check `jobs.pool-runner`.

**Evidence (2026-09-26).**

| Check | Result |
| --- | --- |
| `jobsystempoolrunnertest` | 36 checks, pass; 10 of 10 repeats |
| Seeded pool mutants (compiled into a scratch executable, where they override the library's symbols) | 3 of 3 detected: without `JF_QUEUE` (`never-inline-under-load`), without dropping queued work at shutdown (`nothing-runs-after-shutdown`), timer ignored (`delayed-not-early`) |
| `platform.task_runner` with the stronger suite | 117 checks, and under TSan |
| `platform.task_runner.sensitivity` | still catches all seven broken providers |
| `corpus.hammer.glib-runner` | 21 checks |
| Every runner suite, `--repeat 5`, g++ and clang++ | pass |

## R10 closure (done 2026-09-26)

R10's done condition, and the exits it covers from RFC 0001 rank 11 and
RFC 0003 phases A and B:

| Requirement | Evidence |
| --- | --- |
| Virtual time | `VirtualClock` and `ManualTaskRunner`, judged by `platform.task_runner` (R10-RUNNERS) |
| Independent graph model | `jobsystem.qjobs` (independent model, adversarial schedules, negative executors) |
| Validation, publication, affinity and failure tests | `jobsystem.scheduler`, `jobsystem.qjobs`, `jobsystem.dynamicscope`, `jobsystem.runner-bindings`, with TSan lanes |
| Ordered serial host graph matches legacy captures | The versioned capture, rechecked in both modes by `jobs.host-frame-capture` (R10-CAPTURE) |
| RFC 0001 rank 11: clock, task runner, sequenced runner, delayed scheduling, virtual-time provider | `platform.task-runner.v1` and `platform.runners` (R10-RUNNERS) |
| RFC 0001 rank 11: diagnostic sequence checks | `SequenceChecker` and `PLATFORM_CHECK_SEQUENCE` (R10-SEQCHECK) |
| RFC 0001 rank 11: adapt existing queues | The engine `CThreadPool` runner (R10-POOLRUNNER) |
| RFC 0003 Phase A: reviewed ownership map, reproducible baseline, first particle workload | The Phase A inventory (above), `jobs.legacy-captures` recorded, D-A1 |
| RFC 0003 Phase B: conformance tests; the ordered host graph preserves baseline behavior; runner bindings | The rows above, and R10-BINDINGS |
| A consumer at the new boundary | Hammer's asynchronous F9 (`MapBuildQueue`, `GlibTaskRunner`) |
| Hard-gate prerequisites | R05 and R06 are `done` |

**Closing checks (2026-09-26).**

- Q-EDITOR + Q-FOUNDATION + Q-JOBS headless, g++ and clang++: 123 of 129
  each. The 6 skips are optional TSan lanes; the R10 lanes pass under
  `CONFORMANCE_TSAN=1`.
- `baseline.py audit` of the nine related checks (`jobs.pool-runner`,
  `hammer.ui`, `hammer.mcp`, the viewport smoke, `arch.check`,
  `arch.hermetic`, `arch.hammer`, `quality.selftest` and
  `baseline.validate`): 0 deviations. `jobs.host-frame-capture` passed in
  its own audit.
- `archlint check --all`, `targets --verify --partial build-r03-tools
  build-r03-tests`, `hermetic` (65 headers, both compilers) and
  `hammer --verify`: pass.
- stylelint on the slices' files: clean.

**Not claimed by this closure** (owned elsewhere):

- frame, p95/p99, low-core and mobile budgets (`jobs.frame-budgets`, R21);
- cohort gameplay captures (`jobs.cohort-captures`, R21);
- a native `IMonotonicClock` provider and the other foundation providers
  (R26);
- Android and Apple runtime evidence for the scheduler (R20/R21/R29);
- hosted CI runs.

The `portal-native` tree was rebuilt for the capture. The other product
trees rebuild the changed `jobsystem` and `vstdlib` on their next build.

## R20-COMPUTE-POOL: the render core borrows the engine's compute pool (slice done 2026-09-28)

**Scope.** Found in a review of RFC 0016 K3/K5 with the render-core owner.
Product culling (`render/composition/render_core.cpp`) ran on a private
`jobsystem::ParallelExecutor` with four threads of its own, a fixed number,
outside the process worker budget. K5 step 4 would run it every frame. The
core now borrows the root's pool, as R67 P2 made physics do.

- **`CreateComputePoolWorkerBackend`** (`public/vstdlib/jobgraph_pool_bridge.h`):
  `g_pThreadPool`, the pool the engine starts as `CmpJob`, as a borrowed
  `IWorkerBackend`. It never starts or stops the pool, and its worker count
  follows the pool: 0 (inline on the caller) until the engine starts it and
  after it stops it. It binds no other pool, so a root cannot lend a provider
  a pool its callers run on, such as the material system's `MatQueue` pool.
  Destroying it leaves the pool running (`DestroyThreadPoolWorkerBackend`
  now stops only pools it owns).
- **Nested-call guard.** A call made on one of the compute pool's own workers
  runs inline on that worker and is counted
  (`ComputePoolWorkerBackendNestedCalls`) instead of queuing runners behind
  itself, which is RFC 0003's forbidden nested wait. `IsThreadPoolWorkerThread`
  (`public/vstdlib/jobthread.h`, a new export beside
  `GetThreadPoolSchedulingStats`; no vtable change) tells the backend. Private
  pool backends and the stack-bound bridges used by the cohorts keep their
  behavior.
- **Render core.** `RenderCoreConfig::computeWorkers` takes the root's
  backend, borrowed. The core's pooled culling is a `jobsystem::PooledExecutor`
  over it, created at composition. Null runs inline on the caller, the serial
  reference; Hammer passes none. The launcher lends the compute pool and
  destroys the backend after the core.
- **Composition check.** The render-core owner asked for a composition-time
  check that refuses the `MatQueue` pool. That pool doesn't exist when the
  launcher composes the core: the material system creates it later, on first
  use. So the guarantee is structural instead. The only factory a root has
  binds the compute pool alone, and a nested call on that pool is caught at
  run time.

**Evidence (2026-09-28).**

| Check | Result |
| --- | --- |
| `corpus.jobs.pool-bridge` (`jobsystembridgetest`, now checks-v1 and in the manifest) | 168 checks, pass; 5 of 5 direct runs and 3 of 3 runner repeats. New cases: follows the pool from not started through started (3 workers) to stopped; pooled graphs equal `DeterministicExecutor`; destroying the backend leaves the pool running; a `ParallelFor` and a pooled graph from a compute worker run inline on it and are counted; the same from another pool's worker (the `MatQueue` pool's place) stay pooled and uncounted |
| Seeded defect: guard disabled | detected: the nested case's bodies spread across threads and nothing is counted (2 failures) |
| `jobsystem.pooled` | 180 checks, pass |
| Product: `tools/render/culling_capture.py suite --scenario portal2_sp_a1_wakeup` on the `build-p2` tree installed to a private destdir | 39 checks, 0 failures, including the pooled draw list equal to the serial one item for item on every shot; the boot's compute pool has 3 threads |
| `archlint check --all` | 2 new ARCH105 occurrences, both in `game/shared/fstop/blob_networkbypass.*`, not this slice |
| stylelint on this slice's files | clean |

**Not claimed:** K5 itself; a Portal 1 capture (the P2 scenario covers the
product path); TSan of the product with core culling on, which the K5 step-4
evidence adds.

**Commands** (repository root):

```sh
WAFLOCK=.lock-waf-r03-tests ./waf build --targets=jobsystembridgetest,vstdlib
python3 tools/quality/conformance.py check --suite corpus.jobs.pool-bridge --repeat 3 --out <dir>
python3 -m unittest tools.quality.tests.test_frame_pacing
WAFLOCK=.lock-waf-p2 ./waf install --destdir=<dest>
python3 tools/render/culling_capture.py suite --out <dir> --scenario portal2_sp_a1_wakeup \
    --p2-build <dest>/usr/local --p2-runtime <Portal 2 content runtime>
```

## Module layout

```
public/jobsystem/expected.h            job-system names for foundation::Expected
public/jobsystem/job_graph.h           handles, executors, resources, builder, SealedGraph, errors
public/jobsystem/graph_executor.h      JobRunContext, FrameContext, RunOptions(pumpMainThread), RunResult(stalled), executors
public/jobsystem/parallel_executor.h   ParallelExecutor (lane-aware std::thread pool: compute/main/blocking)
public/jobsystem/external_completion.h ExternalCompletion (exactly-once) + MakeExternalWait adapter
public/jobsystem/dynamic_scope.h       DynamicScope (live producer/continuation child scopes)
public/jobsystem/worker_backend.h      IWorkerBackend (C++11-clean bridge boundary)
public/jobsystem/pooled_executor.h     PooledExecutor (wave executor over a backend)
public/jobsystem/frame_graph.h         FrameContext region, IFrameContributor, FrameCoordinator
public/jobsystem/pilot_particles.h     ParticleFrameState, reference update, pilot contributor
public/jobsystem/parallel_batch.h      scoped batch contract (batch migration)
public/jobsystem/serial_frame_graph.h  C++11 serial host-frame graph facade (scheduler trust)
public/jobsystem/declared_frame_graph.h  declared frame-graph regions (scheduler nodes)
public/vstdlib/jobgraph_pool_bridge.h  C++11-clean factory: real CThreadPool -> IWorkerBackend
public/vstdlib/jobgraph_parallel.h     JobGraphParallelProcess adapter for legacy callers
public/vstdlib/jobgraph_frame.h        vstdlib exports for the host frame graph
jobsystem/*.cpp                        implementations + Waf stlib (C++20)
vstdlib/jobgraph_pool_bridge.cpp       real engine-pool backend (C++11, in vstdlib)
unittests/jobsystemtest/qjobstest.cpp  Q-JOBS independent model + adversarial negative executors
unittests/jobsystemtest/dynamicscopetest.cpp  DynamicScope conformance
unittests/jobsystemtest/*            C++20 conformance + engine-bridge programs + Waf wiring
```

---

## Phase A inventory (retained)

### Worker-count and threading configuration baseline

| Setting | Default (this tree) | Site |
| --- | --- | --- |
| Global pool worker cap (no `-threads`) | 4 | `vstdlib/jobthread.cpp:319` |
| Default compute-pool worker limit (PC path) | 3 | `vstdlib/jobthread.cpp:923`–`924` |
| `host_thread_mode` | `0` on PC (`1` on X360) | `engine/host.cpp:463` |
| `sv_parallel_packentities` | `1` (enabled) | `engine/sv_packedentities.cpp:389` |
| `sv_parallel_sendsnapshot` | `0` (disabled; documented crash) | `engine/sv_main.cpp:1830` |

### Verified core facilities

- No public dependency-graph/continuation contract in `public/vstdlib/jobthread.h`
  (`CJob :440`, `IThreadPool :150`, `CParallelProcessor :850`).
- Inline-on-caller execution when no idle worker: `AddCall` (jobthread.h `:233`),
  `CThreadPool::AddJob` (`vstdlib/jobthread.cpp:691`–`697`).
- `-threads`/4-cap and 3-worker default (`jobthread.cpp:309`–`320`, `:923`–`924`).
- `YieldWait` drains shared queue then waits; documented that worker-spawned jobs
  do not wake a waiting main thread (`jobthread.cpp:621`–`640`).
- Direct + shared queues; `JF_SERIAL` pinned to thread 0 (`InsertJobInQueue :719`).

### Existing parallel subsystem seams (verified)

| Seam | Verified site | Pattern | Initial conflict class (hypothesis) |
| --- | --- | --- | --- |
| Particle simulation | `game/client/particlemgr.cpp:1855` (`ProcessPSystem`); commit `:596` | serial gather → parallel compute → serial change publication | disjoint per-system outputs; serial control-point gather + publication |
| Bone setup | `game/client/c_baseanimating.cpp:2693` | parallel over previous-frame bone setups | parent/attachment/model-cache deps; model pins |
| Query-cache maintenance | `game/shared/querycache.cpp:244` | parallel disjoint hash chains → serial merge | disjoint chains; ordered victim merge |
| Entity packing | `engine/sv_packedentities.cpp:454` (default on) | parallel per-entity packing | stable snapshot; disjoint output; encoding audit |
| Client leaf — insert | `game/client/clientleafsystem.cpp:566` | parallel under frame lock | shared-tree exclusivity vs visibility |
| Client leaf — translucency | `game/client/clientleafsystem.cpp:1372` | parallel FX-blend compute | transparency/draw-order preserved |
| Snapshot send | `engine/sv_main.cpp:1830` (disabled) | per-client parallel send | **known-unsafe:** shared `m_FrameSnapshots`; HLTV/replay excluded |

Ordered gameplay/callback barriers to preserve (not seams to parallelize):
`CServerGameDLL::GameFrame` order, `Physics_RunThinkFunctions` `curtime`
reset + deletion suppression, client `SimulateEntities`/`OnRenderStart`, and
`IGameSystem` registration-order updates under model-cache scopes.

### Documented concurrency hazards

- Snapshot manager race (`engine/sv_main.cpp:1823`–`1830`): concurrent
  `WriteDeltaEntities`/`WriteTempEntities` on `m_FrameSnapshots` crashed; ships
  disabled, excludes HLTV/replay. Migration needs stable snapshot inputs,
  independent outputs, and an explicit owner for shared bookkeeping.
- Wait-time self-help boundary (`jobthread.cpp:628`–`640`): a waiting caller does
  not pick up worker-spawned jobs, and helping arbitrary queued work can violate
  phase assumptions.

### Runtime baseline availability (honest gap)

No runtime baseline was captured: no built product, captured maps, or profiling
hardware here. To close it later requires a client + dedicated server on a
declared profile, representative captures, and pre-agreed per-subsystem budgets.

## Decisions

### D-A1 — First particle workload
`CParticleMgr::UpdateNewEffects` (`particlemgr.cpp:1855`): already separates a
serial gather, a parallel `ProcessPSystem` batch, and a serial `DetectChanges`
publication — the RFC's canonical shape with the least entity-mutation
entanglement. Target selection, not a completed migration.

### D-B1 — Substrate built ahead of R05/R06 as a self-contained C++20 target
Phase B/C prerequisites in AGENTS.md are R05 (`Expected`/IDs) and R06
(composition kernel), both `planned`. Rather than block, the scheduler is built
as a portable module over standard C++20 primitives with a small job-system-
scoped `Expected`. This keeps one authority per concern (no competing global
result type, no new service locator) and is honestly a bounded feasibility
delivery: it must converge onto R05's vocabulary and be recomposed under R06's
lifecycle kernel when those land. It does **not** mark R05/R06 done.

## Commands

```text
# Shared conformance gate (authoritative; no Waf lock needed):
python3 tools/quality/conformance.py check --suite jobsystem.scheduler \
  --suite jobsystem.dynamicscope --suite jobsystem.qjobs

# Standalone build + run (self-contained C++20 module):
g++ -std=c++20 -O2 -Wall -Wextra -Wshadow -pthread -I public \
  jobsystem/*.cpp unittests/jobsystemtest/qjobstest.cpp -o /tmp/qjobstest && /tmp/qjobstest

# Standalone sanitizer lanes (clang):
clang++ -std=c++20 -O1 -g -fsanitize=thread -pthread -I public \
  jobsystem/*.cpp unittests/jobsystemtest/jobsystemtest.cpp -o /tmp/js_tsan && /tmp/js_tsan

# In-tree Waf build + run (when a --tests profile is configured):
./waf build --targets=jobsystem,jobsystemtest,jobsystemframetest,jobsystemdynamictest,jobsystemqjobstest

# Architecture gates:
python3 tools/archlint/archlint.py check --changed
python3 -m unittest discover -s tools/archlint/tests
```

## Next increment

(2026-09-25) Superseded. Item 1 is partly done: particles run on the graph
behind `r_particle_job_graph`, pooled by default, without the measured
improvement or frame budget. Item 3's `Expected` convergence is done.
Forbidden nested waits are detected in the engine pool (scheduler nodes). The
current open lists are in the
[scheduler nodes](0003-scheduler-nodes-progress.md#open-rows-stay-partial) and
[batch migration](0003-batch-migration-progress.md#measurements-and-open-gates)
records. The original list:
1. **Real particle migration (Phase D)**: wire the pilot pattern into
   `CParticleMgr::UpdateNewEffects` behind a diagnostic switch, gated on the
   engine build, captured workloads, three-mode equivalence on real outputs, a
   measured improvement, a low-core regression budget, and tested rollback.
2. **Legacy-capture comparison and performance/latency baselines**: gated on a
   buildable/runnable engine profile, maps, and hardware.
3. Converge `Expected` onto R05 (`foundation::Expected`) and recompose the runtime
   under the R06 kernel when those rows land.
4. Optional hardening: forbidden-nested-wait detection across executors, and a
   lock-free ready structure only once it earns correctness/performance evidence.

## S4 GPU culling placement and the two-queue graph model (2026-10-05, user goal)

User goal: "RFC 0003: S4's GPU culling falls under its CPU/GPU placement
rule, with the CPU culler kept as the oracle. S8 (async compute) needs the
graph model extended to two queues." Owner: this session. Plan:
[RFC 0016 GPU-driven submission](0016-render-core.md#gpu-driven-submission-plan-2026-10-05-user-direction).

**Installed.**

- `render.pass.cull` (new layer-6 module, `public/render/pass/cull/cull.h`,
  `render/pass/cull/cull.comp`): frustum and view-mask culling of scene
  instances on the GPU, one visibility bit per instance, as a compute graph
  pass that asks for the async compute queue. The kernel follows the CPU
  culler's order of operations and forbids contraction (`precise`), so the
  oracle is exact. Contract `render.cull.v1`; suite `render.cull`.
- `render.graph` two queues: `Queue` on `PassDecl` (`PassBuilder::OnQueue`,
  compute passes only, else `kQueueKindMismatch`); `CompileOptions`
  (`asyncCompute`, default off, so existing graphs compile unchanged);
  per compiled pass its queue, one `waitFor` (the latest pass on the other
  queue it must follow, omitted when the queue already knows it complete)
  and the `acquires` whose ownership moves; `GraphTrace::waits`; the
  validator's `kMissingQueueWait` (vector-clock reachability over the
  compiled waits, independent of the compiler's last-writer/reader state).
  Contract clause G12 of `render.graph.v1`.

**Evidence** (Linux desktop, RADV Strix Halo; g++ 16 and clang++ 22):

| Suite | Result |
| --- | --- |
| `render.cull` | 12 checks: GPU mask equals `scene::BuildDrawList` bit for bit on 200 seeded scenes (up to 20,000 instances, straddling boxes, empty boxes, view masks, view bit 32); seeded near-corner, ignored-mask and kept-empty kernels disagree on 39, 32 and 33 of 40 scenes (3 of 3 rejected); async and graphics placements both read back the oracle mask; null device without compute fails `kNoCompute`; validation layer silent |
| `render.graph.v1` | 96 checks; G12: waits equal the reference model on 1,000 two-queue graphs (671 need a wait), all validate, ownership moves match, serial executor runs them, no waits without async compute, render pass refused on the compute queue |
| `render.graph.v1.sensitivity` | 16 checks; dropped wait and too-early wait each detected in 671 of 671 seeded graphs; the five K2 mutations unchanged |
| `render.graph.v1.vulkan`, `render.skinning` | pass (unchanged behaviour) |
| `render.graph.v1.tsan` | passes on clang++; the g++ lane cannot link here (`libtsan.so` missing on the host, a known host gap) |

**Placement decision (the rule's equivalent-output, full-cost comparison).**
Median microseconds over 21 runs; the CPU culler's timings include its
draw-list sort, which biases the comparison toward the GPU:

| Instances | CPU serial | CPU pooled (4) | GPU round trip | GPU, instances resident |
| --- | --- | --- | --- | --- |
| 1,024 | 42 | 56 | 230 | 193 |
| 4,096 | 120 | 90 | 300 | 180 |
| 16,384 | 412 | 210 | 603 | 192 |
| 65,536 | 3,762 | 2,644 | 1,750 | 203 |

Today's consumer is the CPU draw list, so the GPU path must read the mask
back and wait. Portal maps hold about 1,500–2,400 BSP leaves plus a few
hundred props per scene, well under the measured crossover (between 16,384
and 65,536 instances). **The CPU culler stays the product path** for
CPU-recorded draw lists; the GPU kernel is installed and proven equal, not
wired into a product. The ~190 µs resident floor is submission and a
blocking wait, which S3 removes by consuming the mask on the GPU; the
decision is re-taken then, with in-frame GPU timestamps against the pooled
CPU culler, and for R63's dense USD scenes, which exceed the crossover.

**Not done.** Executing a real second queue (the Vulkan adapter does not
claim `kAsyncCompute`; executors still run every pass on graphics in
compiled order, which satisfies every wait), queue-family ownership
transfers in the adapter, GPU compaction into indirect commands (S3/S4),
HiZ occlusion, Fold7 and Apple measurements, and a game frame using either.

## Compaction into indirect draw commands (2026-10-05, user goal)

User goal: "compacting the culled list into indirect draw commands", the
open item of the S4 record above. Owner: this session.

**Installed.**

- Port (`render.device.v2`): `DrawIndexedIndirectCommand` (20 bytes),
  `CommandEncoder::DrawIndexedIndirect` (D30, `Capability::kMultiDrawIndirect`)
  and `DrawIndexedIndirectCount` (D31, `Capability::kDrawIndirectCount`),
  with one argument rule, `IndirectRecordsFit`, every adapter uses. Vulkan
  claims them where the device has `multiDrawIndirect` and Vulkan 1.2's
  `drawIndirectCount` (enabled at device creation, also on a host's
  chain); the null adapter validates them; GL and ES record the calls and
  refuse the submission with `kUnsupported` (not claimed in this slice).
- `render.pass.cull`: `compact.comp`, `CompactKernel`, `AddCompactPass` and
  the oracle `CompactReference`. One workgroup prefix-scans the mask and
  writes, in instance order, the kept-instance count and one
  `{ indexCount, 1, firstIndex, vertexOffset, instance }` command per kept
  instance from its `DrawTemplate`; one buffer is both records and count
  for `DrawIndexedIndirectCount( out, kCommandsOffset, out, 0, count, 20 )`.

**Evidence** (RADV Strix Halo, Mesa llvmpipe for GL/ES):

| Suite | Result |
| --- | --- |
| `render.device.v2.*` | D30/D31 on null, Vulkan, GL and ES: claimed adapters accept well-formed calls and refuse out-of-range records, a short stride, a buffer not in `kIndirect` and a count outside its buffer (`kInvalidState`); GL/ES refuse `kUnsupported`; on Vulkan two records draw both halves, zero draws nothing, a GPU count of 1 draws the first record. Sensitivity: an adapter that clamps the records and one that ignores the count offset are each caught |
| `render.cull` | compaction equals `CompactReference` on the CPU culler's mask on 60 seeded scenes (0 to 20,000 instances); seeded dropped-last and wrong-instance kernels disagree (2 of 2); on 8 scenes the image drawn by `DrawIndexedIndirectCount` from the compacted buffer equals direct draws of the CPU-kept instances byte for byte, each kept quad lighting exactly its pixels; validation layer silent |

**Placement.** Unchanged: no product pass draws from the commands yet, so
the CPU culler stays the product path. The decision is re-taken when S3
gives the opaque, prepass and shadow passes shared geometry buffers and
per-pipeline buckets that consume these commands without a readback.

**Not done.** That product pass; GL 4.3/4.6 and ES indirect draws; per-pipeline
bucketing in the compaction (one bucket today); a real second queue.

## Game pass on indirect commands, GL/ES indirect, a real second queue, occlusion culling (2026-10-06, user goal)

User goal: complete the four open items of the compaction record above.
Owner: this session.

**Installed.**

- **A game pass drawing from the commands.** `render.pass.cull` moved to
  `render.culling`, a layer-5 mechanism (beside `render.frame`) that feature
  passes may use; the layer contract keeps passes independent. Compaction is
  bucketed: one command list per bucket (`DrawBucket`), each bucket's count
  as word b and its commands in its instances' slots, deterministic (two
  phases in one workgroup, at most 98,304 instances). The world pass's main
  draw (`WorldTarget::gpuSubmission`, engine `r_core_world_gpu_submit`,
  default 0) uploads the view's surface list with cached per-surface world
  bounds, culls it per surface and compacts it into one list per (material,
  lightmap page) bucket before rendering, then binds each bucket exactly as
  the per-surface path does (one shared `bindSurfaceMaterial`) and issues one
  `DrawIndexedIndirectCount` from the world's shared vertex and index
  buffers. Per-frame buffers and bind groups retire by frame. Devices
  without `kDrawIndirectCount` draw per surface and count a fallback.
- **Indirect draws on GL and ES.** GL 4.5 claims D30
  (`glMultiDrawElementsIndirect`; a bound index offset is honoured by a GPU
  copy of the indices into a scratch element buffer) and D31 where the
  context has `glMultiDrawElementsIndirectCount`; ES 3.1 claims D30 as one
  GPU-read `glDrawElementsIndirect` per record. New
  `Capability::kIndirectFirstInstance` (records with a nonzero
  `firstInstance`): Vulkan claims it with `drawIndirectFirstInstance`
  (enabled at creation, missing before); GL and ES do not.
- **A real second queue.** The Vulkan adapter selects a compute-only family,
  creates its queue, timeline and command pools, claims `kAsyncCompute`, and
  creates resources with concurrent sharing between the two families.
  Tokens name their queue; submissions wait on either timeline; compute
  encoders refuse rendering, draws, host work and timestamps; their uploads
  take staging buffers (the ring retires by the graphics timeline); barriers
  on the compute queue use the generic all-commands scopes. The serial graph
  executor runs a two-queue graph as one submission per same-queue run with
  the compiled waits, ending in a graphics submission that covers both.
- **Occlusion culling.** `hiz.comp` builds a max-depth pyramid from a depth
  texture (one dispatch per level); `occlusion.comp` removes frustum-kept
  instances whose nearest projected depth lies behind the pyramid's
  farthest over their screen rectangle (first level spanning at most 2x2
  texels; boxes crossing the near plane stay). `OcclusionKernels`,
  `AddOcclusionPass`, and the oracles `DepthPyramidReference` and
  `OcclusionReference`.

**Evidence** (RADV Strix Halo; Mesa for GL/ES):

| Check | Result |
| --- | --- |
| `render.device.v2.gl` / `.gles` | D30/D31 refusals and pixels on GL (claims both) and ES (claims D30); 1,177 and 1,062 checks |
| `render.device.v2.vulkan` | D15 runs a dispatch on the compute queue; compute encoders exist exactly when async compute is claimed; sensitivity's false claim moved to async transfer |
| `render.graph.v1.vulkan` | 200 two-queue random graphs (130 with cross-queue waits) ran in 638 submissions on the compute and graphics queues; synchronization validation silent |
| `render.cull` | bucketed compaction equals the reference on 60 scenes (1 to 64 buckets); multi-bucket indirect image equals direct draws; occlusion: 412 of 1,608 frustum-kept cubes culled over 6 scenes, pyramid equal to its CPU rebuild, 0 disagreements with the CPU reference, images with and without occlusion identical byte for byte, far-corner and one-texel kernels change 5 and 6 of 6 images |
| `render.lab.gpu-submission` | 400 quads, six materials: per-surface and GPU-driven images identical pixel for pixel, one indirect draw per bucket, no fallback, validation silent |
| Game, `testchmb_a_01` (`build-r03-portal-native`, headless, `-deterministicrender`, `r_core_world 1`) | with `r_core_world_gpu_submit 1`: 204 views GPU-driven, 7,956 indirect draws, 0 fallbacks, 0 failed views; the screenshot equals the per-surface run's (0 of 786,432 pixels differ) |

**Placement.** The world pass's GPU path is opt-in (default 0): it still
builds the CPU list and the per-surface mip footprints, so its CPU saving is
the draw calls and bindings only, and its whole-frame cost against the
per-surface path is not yet measured with the resolution sweep. RFC 0003's
rule selects the product default from that measurement; occlusion culling
is not yet wired into the world pass (it needs the previous frame's depth).

**Not done.** The whole-frame measurement and default; occlusion in the
world pass; the remaining views that draw per surface (prepass, models,
shadows); per-instance data through `kIndirectFirstInstance` in a product
shader; the pooled executor still records two-queue graphs on graphics;
Fold7 and Apple runs.

## R94: one task system (2026-10-07)

User goal: "Implement R94 with a rock solid ratchet - we need a pure task
system", in the shared tree (no worktree). Row R94 (RFC 0003 phase I, goals
J1–J7) moves from `planned` to `active`. Then (user direction during the
slice): "Instead of micro benchmarks let's have a focus on real world intro4
demo".

### What changed

- **The backend runs tasks only.** `IWorkerBackend` (`public/jobsystem/worker_backend.h`,
  still C++11-clean) is `WorkerCount`, `PostTask`, `SettleTask` and
  `ShouldRunInline`. `ParallelFor` and `ParallelForWithCaller` are deleted,
  so no scheduler can be built on a barrier. `SettleTask` withdraws a task
  that has not started or waits for one that has; a refused post leaves the
  work to the scheduler. The engine pool bridge (`vstdlib/jobgraph_pool_bridge.cpp`)
  posts one `CJob` per task (`QueueCall`) and settles it with `Abort` under
  the job's lock; a graph entered on one of the compute pool's own workers
  still runs inline and is counted.
- **`TaskExecutor`** (`public/jobsystem/task_executor.h`) is the product
  executor. It is `ParallelExecutor`'s scheduler (one implementation in
  `jobsystem/parallel_executor.cpp`: lanes, stall rules, runner bindings,
  terminal states) with runner tasks in place of dedicated threads. A job is
  ready the moment its last prerequisite resolves. Runners drain the ready
  queue and return their worker when it is empty, so no borrowed worker parks
  inside a graph. Whenever ready compute work exceeds the runners that will
  look at the queue, more runners are posted, up to the backend's worker
  count (J1's invariant; `TaskRunStats::uncoveredReady` counts violations).
  The caller helps with compute and is the only servicer of main-thread and
  blocking work, so progress never depends on the backend starting a task.
  At the end, unstarted runners are withdrawn and started ones waited for: no
  task touches a run after `Execute` returns. Two measured refinements: the
  caller counts as the first servicer (a lone job, a chain or a
  one-participant batch posts nothing), and a long ready queue is taken in a
  guided share of at most 32 jobs per lock.
- **`PooledExecutor` is deleted**, with its three product callers moved to
  `TaskExecutor`: `ExecuteParallelBatch` (particles, bones, entity packing,
  query cache, portal carving, emit conversion), `DeclaredFrameGraph`
  (`cl_render_start_graph`), and the render core's pooled culling. In a
  declared frame graph a batch now overlaps every host node its declarations
  leave unordered, including nodes that become ready after the batch starts
  (J2's unit oracle).
- **`ThreadWorkerBackend`** (`public/jobsystem/thread_worker_backend.h`): a
  backend over its own threads for test and tool roots. Eleven test backends
  that implemented `ParallelFor` now use it or implement tasks.
- **J5 control:** `-compute_workers N` sizes the engine's `CmpJob` compute
  pool (`engine/host.cpp`); 0 runs every graph inline. `-threads` still sizes
  only the global pool.
- **J3 runtime census:** the engine's `thread_census` console command lists
  the process's live threads by OS name (pool indices stripped).
  `tools/quality/thread_census.py run|check|sensitivity` boots the product,
  classifies every thread against `quality/budgets/thread-census-v1.json`
  (declared owners: main, compute pool, MatQueue, filesystem I/O, the save
  lane, SDL audio, driver and device threads, each with a reason), requires
  exactly N `CmpJob` workers under `-compute_workers N`, and holds the
  undeclared threads to an exact shrink-only list.
- **The ratchet** (`tools/quality/jobs_ratchet.py`, `tools/quality/jobs_ratchet.json`).
  Invariants that are zero and that `--write` refuses to record: no wave
  executor or fork/join hook (`PooledExecutor`, `ParallelForWithCaller`) in
  first-party code; no `ParallelFor` in the job system or its pool bridge;
  `IWorkerBackend`'s virtual functions are exactly its four; no
  thread-owning executor (`ParallelExecutor`, `DynamicScope`,
  `ThreadWorkerBackend`) constructed outside the job system and its tests.
  Exact per-file ratchets: `thread-create` outside J3's declared owners (J3
  needs 0), host-side `fork-join` calls, blocking `pool-wait`s, and
  `unaudited-node` (serial host-frame phases and `FRAME_DOMAIN_ALL`
  declarations: J6's host-graph census). A count that grows fails, and so
  does one that shrinks without being recorded; a declared owner needs a
  reason and a live site. Comments, string literals and `#define` bodies do
  not count.

### Evidence (Linux desktop, g++ 16.2.1 and clang++ 22.1.8)

| Check | Result |
| --- | --- |
| `jobsystem.continuous` | 30,697 checks pass. 1,000 seeded graphs at budgets 1, 2 and 4 (4,500 compared runs, 1,500 cancellation-race runs checked for exactly-once) equal `DeterministicExecutor` (pumped) or `ParallelExecutor` (unpumped stalls). J1 oracles reject the retired wave executor 10/10 and a one-runner executor 10/10; a success-only executor fails equivalence on 85 of 149 graphs. Delayed, refused, never-started and run-inside-post tasks; no task outlives `Execute`; `uncoveredReady` 0 on every run |
| `jobsystem.continuous.tsan` | clean under clang ThreadSanitizer (30,697 checks) |
| Mutations of `TaskExecutor` | caught: runner budget 1 (saturation oracle), posting under the scheduling lock (deadlock, timeout), returning without settling (lifetime oracle, 6,289 failures), a caller that never helps (deadlock on never-started tasks, timeout), runners that park in the run (timeout). Survives: a runner that exits after one job; its reservations are posted by the next dispatcher, so it is still continuous scheduling (an equivalent mutant) |
| `jobsystem.declaredframe` | 223 checks; the J2 oracle (a batch item waits for a host node that becomes ready after the batch starts) passes, and fails 10/10 with the wave executor swapped in |
| Q-JOBS (`--domain Q-JOBS`) | 17 of 17 runnable suites pass; TSan lanes pass under `CONFORMANCE_TSAN=1 --cxx clang++` |
| `corpus.jobs.pool-bridge` | 220 checks on the real `CThreadPool`, including the J1 no-wave oracle |
| `jobs.ratchet` / `.sensitivity` | 23 and 37 checks; recorded 129 sites in 49 files (thread-create 38, fork-join 32, pool-wait 32, unaudited-node 27), 31 thread-create sites in 15 declared owner files; every invariant at 0 |
| `thread_census.py` | `testchmb_a_00` headless native Vulkan: CmpJob 3 by default, and exactly 0, 1 and 2 under `-compute_workers 0/1/2`; undeclared recorded: `AchievementSave` 1, `QueuedPacketSen` 1, 2 unnamed threads (J3 needs 0); sensitivity 13 checks |
| Portal boots | `testchmb_a_00` passes with the default pool and `-compute_workers 0`, `1`, `2` |
| Toolchain ABI fixture | the C++11 consumer implements the task backend; `toolchain.abi.v1.md` updated |

**Microbenchmarks** (bazzite, Ryzen 7 3700X; `jobsystempoolgraphbench` on the
real engine pool against the retired wave algorithm, separate processes,
ABBA, median of 4). The continuous executor wins where its design says it
should: layered graphs with work run in 0.72–0.83 of the time. Shapes with
real work per job (4,096 steps) are at parity, 1.00–1.13. Small fan-outs of
short jobs are slower: 8- and 64-item batches of 256-step items at 2.3x and
1.8x, chains of empty jobs at 2.2x (about 20 ns per job). A start-time trace
puts the batch gap in ramp-up: the task executor's runners reach their first
job 5.8–7.1 µs after `Execute` starts, against 1.5–2.9 µs. Not fixed in this
slice; the in-game demo below is the judge (user direction). One pitfall is
recorded: alternating the two executors inside one process biased whichever
ran second, so only separate-process runs are reported.

### The intro4 demo (real-world judge, user direction)

`quality/workloads/portal2-intro4-demo-v1` (the user's `sp_a1_intro4_relit`
recording, sha256 `d88908c2…`) on bazzite: Ryzen 7 3700X (16 threads), RTX
3070, NVIDIA driver, fullscreen 2560x1440 on the desktop compositor (60 Hz
display), with the workload's settings (FSR built in, `r_temporal_scale 0`,
`mat_antialias 0`, `mat_queue_mode 2`, `r_core_world 1`) and `play.sh`'s job
flags (`+cl_render_start_graph 2 +sv_querycache_job_graph 2 -vkemitparallel 1`).
Each run is judged by `demo_frames.analyze` (demo played, settings queried,
playback window, extent); all runs complete with no failures.

**Matched builds.** The baseline is `git archive HEAD`; the candidate is the
same archive with only this slice's files. Both were configured from
`build-p2-fsr`'s stored options (identical configuration caches) and built
in scratch, so other sessions' uncommitted work is in neither. The installs
differ in `libengine`, `liblauncher`, `libtier0` and `libvstdlib`. Both ran
from hardlinked copies of the box's deployment, with every file the game
writes unshared. Warm-up run per side, then 8 ABBA rounds.

| Metric (median over 8 runs of each run's value) | baseline (wave) | candidate (task) | ratio |
| --- | --- | --- | --- |
| frame interval, median | 16.67 ms | 16.66 ms | 1.000 |
| frame interval, p99 | 42.18 ms | 42.01 ms | 0.996 |
| CPU critical path, median / p95 | 7.34 / 28.00 ms | 7.34 / 28.05 ms | 1.000 / 1.002 |
| engine (main thread), median / p95 | 1.81 / 7.13 ms | 1.82 / 7.08 ms | 1.007 / 0.992 |
| command recording, median | 1.88 ms | 1.89 ms | 1.006 |
| GPU render, median | 12.21 ms | 12.19 ms | 0.998 |
| 1% low | 19.05 fps | 19.25 fps | 1.010 |
| frames over 50 ms | 4 | 3 | |

The new task system is neutral on this demo: every metric is within run-to-
run spread. Both sides are bimodal in the tail (some runs have about half the
frames over 34 ms of the others, with about 0.7 ms less GPU time in those
frames), and the mode does not follow the build or the run order, so it is
GPU state, not the scheduler. The demo's frame is GPU- and present-bound
(long frames: about 27 ms of CPU critical path, mostly acquire and present
waits, against about 4 ms of engine work), and its job graphs carry little
work, which is also why the microbenchmarks' small-batch gaps do not show.

**J5 on the demo** (candidate, 3 ABBA rounds): `-compute_workers 3` (the
default), `1` and `0` (serial) are indistinguishable: engine median 1.80 /
1.78 / 1.77 ms, CPU median 7.26 / 7.27 / 7.33 ms, GPU 12.17 / 12.14 / 12.15
ms, p99 43.2 / 42.5 / 43.0 ms. Pooled is no slower than serial. On this
workload it is no faster either, which is the measure of how little of the
intro4 frame runs as jobs today (J6's target).

Reproduction: build the two archives as above; on bazzite,
`~/r94demo/run.sh <base|cand> <label> [args]` plays the demo fullscreen and
copies `frames.jsonl` and `console.log` to `~/r94demo/results`; the
summaries come from `demo_frames.analyze` over each run.

### Not done

- J2's product evidence: a frame trace showing unordered declared nodes
  overlapping at least once per 100 frames (the unit oracle passes).
- J4: queueing, wake and join time on a product frame's critical path
  against the pooled work it runs (5 % rule).
- The Fold7 and iPhone rows of J3 and J5; the J7 ratchet.
- Small fan-outs of short jobs ramp up slower than the wave executor did
  (above); not visible in the demo, and the next candidate fix (a bounded
  spin on the scheduling lock before blocking) is unmeasured, so it is not
  applied.
- Work stealing between runners: not added. The ready queue stays one
  synchronized queue, as RFC 0003 requires until a stealing deque has its
  own correctness and performance evidence; the demo gives no reason yet.
- `thread-create` (38 sites) and the runtime census's 4 undeclared threads
  must reach 0 for J3; `unaudited-node` (27) shrinks with J6.
