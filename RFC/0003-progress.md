# RFC 0003 progress: Dependency-aware job system and frame scheduling

Updated: 2026-09-21
Source revision at assessment: `a4f6f95f` (working tree; AGENTS.md portfolio row: R10)

This file is the human-readable, durable progress record for RFC 0003. It covers
**Phase A (Inventory and baseline)** and the first delivery of **Phase B
(Contracts and serial graph)** and **Phase C (Compute executor)**, plus the
frame-composition infrastructure the particle pilot (Phase D) rides on.

RFC 0003 is Proposed; this record marks **no** gate complete. The scheduler
substrate below is delivered and tested at the module level, but the R10 gate
(AGENTS.md) also requires the RFC 0005 Q-JOBS independent/adversarial harness and
legacy-capture comparison, which are **not** delivered here (see gaps). Per the
RFC, Phase A does not create a speculative machine-readable baseline "before the
scheduler API exists"; the durable Phase A artifact remains this inventory map.

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
| Reproducible **runtime** baseline (captures, budgets) | A | **Unavailable here** | No runtime target/maps/hardware; requirements listed below |
| First particle workload selection | A | Decision D-A1 | `CParticleMgr::UpdateNewEffects` |
| `Expected<T,E>` (job-system-scoped) | B | Delivered | `public/jobsystem/expected.h`; used throughout |
| Graph builder + validator (names, handles, cycles incl. sequence-induced, resource conflicts) | B | Delivered + tested | `jobsystem/job_graph.cpp`; `jobsystemtest` |
| Immutable SealedGraph + stable topological order | B | Delivered + tested | `public/jobsystem/job_graph.h` |
| Deterministic serial reference executor | B | Delivered + tested | `jobsystem/deterministic_executor.cpp` |
| Bounded worker-pool executor; release/acquire publication; no worker blocks on deps | C | Delivered + tested | `jobsystem/parallel_executor.cpp`; TSan-clean |
| Host frame coordinator + frame contributor contract + rollback path | B/D | Delivered + tested | `jobsystem/frame_graph.cpp`; `jobsystemframetest` |
| Particle reference pilot (gather/compute/commit + bone attachment) | D | Delivered as reference; **not wired to engine** | `jobsystem/pilot_particles.cpp` |
| Three-mode equivalence (legacy ref vs serial graph vs parallel graph) | B/C/D | Passing on captured inputs/private outputs | `jobsystemframetest` |
| Wave executor over an abstract worker backend (`IWorkerBackend`) | C | Delivered + tested | `jobsystem/pooled_executor.cpp`; TSan-clean |
| **Legacy pool bridge: real `vstdlib` `CThreadPool` executes job graphs** | C/R20 | **Delivered + tested** | `vstdlib/jobgraph_pool_bridge.cpp`; `jobsystembridgetest` |
| Q-JOBS independent model / adversarial schedules / negative executors | B/C | **Not delivered** | RFC 0005 R02; see gaps |
| Game-subsystem source migration; legacy-capture comparison | B/D | **Not delivered / unavailable** | needs game build + captures |

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

## Known gaps (not yet delivered; required before the R10 gate closes)

1. **Q-JOBS independent/adversarial harness** (RFC 0005 R02): bounded schedule
   exploration, an independent small-graph state model, and *negative*
   executors/queues that deliberately misorder publication or duplicate
   completion to prove the suite detects defects. Current tests are the module's
   own conformance suite plus a real-thread TSan/ASan lane and an independent
   reference oracle for the pilot — strong, but not the full Q-JOBS model.
2. **Affinity / main-thread pump enforcement**: `ExecutorKind::MainThread` and
   `Sequence` are represented and ordered via edges, but the parallel executor
   does not yet enforce affine execution, main-thread pump points, or detection
   of a wait that cannot progress on an unpumped affine executor.
3. **Blocking-IO executor and external-completion adapter**: `ExecutorKind::
   BlockingIO` exists as a token; a separate blocking lane and exactly-once
   external-completion registration are not implemented.
4. **Dynamic child scopes**: not implemented (a deliberately later RFC extension).
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
   R05 result vocabulary when it lands, not remain a second authority.

## Module layout

```
public/jobsystem/expected.h          scoped Expected<T,E> (pending R05)
public/jobsystem/job_graph.h         handles, executors, resources, builder, SealedGraph, errors
public/jobsystem/graph_executor.h    JobRunContext, FrameContext, RunResult, ITraceSink, IGraphExecutor, DeterministicExecutor
public/jobsystem/parallel_executor.h ParallelExecutor (std::thread worker pool)
public/jobsystem/worker_backend.h    IWorkerBackend (C++11-clean bridge boundary)
public/jobsystem/pooled_executor.h   PooledExecutor (wave executor over a backend)
public/jobsystem/frame_graph.h       FrameContext region, IFrameContributor, FrameCoordinator
public/jobsystem/pilot_particles.h   ParticleFrameState, reference update, pilot contributor
public/vstdlib/jobgraph_pool_bridge.h  C++11-clean factory: real CThreadPool -> IWorkerBackend
jobsystem/*.cpp                      implementations + Waf stlib (C++20)
vstdlib/jobgraph_pool_bridge.cpp     real engine-pool backend (C++11, in vstdlib)
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
# In-tree build + run (configured --tests profile):
./waf build --targets=jobsystem,jobsystemtest,jobsystemframetest
build-phase-a-tests/unittests/jobsystemtest/jobsystemtest
build-phase-a-tests/unittests/jobsystemtest/jobsystemframetest

# Standalone sanitizer lanes (clang):
clang++ -std=c++20 -O1 -g -fsanitize=thread -pthread -I public \
  jobsystem/*.cpp unittests/jobsystemtest/jobsystemtest.cpp -o /tmp/js_tsan && /tmp/js_tsan

# Architecture gates:
python3 tools/archlint/archlint.py check --changed
python3 -m unittest discover -s tools/archlint/tests
```

## Next increment

In priority order:
1. **Q-JOBS harness (RFC 0005 R02)**: an independent small-graph model, bounded
   schedule exploration, and negative executors/queues that prove the suite
   detects misordered publication / duplicated completion / early payload
   release. This is required before trusting the executor beyond its own tests.
2. **Affinity + main-thread pump** enforcement and forbidden-nested-wait
   detection in the parallel executor; a **blocking-IO** lane and an
   external-completion adapter with exactly-once semantics.
3. **Real particle migration (Phase D)**: wire the pilot pattern into
   `CParticleMgr::UpdateNewEffects` behind a diagnostic switch, gated on the
   engine build, captured workloads, three-mode equivalence on real outputs, a
   measured improvement, a low-core regression budget, and tested rollback.
4. Converge `Expected` onto R05 and recompose the runtime under the R06 kernel
   when those rows land.
