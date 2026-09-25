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
| Reproducible **runtime** baseline (captures, budgets) | A | **Partial (2026-09-25)** | Portal now runs here: host-frame trace captures and frame-pacing measurements are in the linked records. `jobs.legacy-captures` is `partial` in `quality/baseline.json`; no versioned frame capture or frame budget |
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
