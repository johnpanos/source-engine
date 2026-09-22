# RFC 0003 progress: Dependency-aware job system and frame scheduling

Updated: 2026-09-21
Source revision at assessment: `a4f6f95f` (working tree; AGENTS.md portfolio row: R10)

This file is the human-readable, durable progress record for RFC 0003. It opens
**Phase A (Inventory and baseline)**. RFC 0003 is Proposed and this record marks
**no** implementation gate complete. Per the RFC's own instruction, Phase A does
**not** create a speculative machine-readable baseline "before the scheduler API
exists"; the durable Phase A artifact is this reviewed ownership/inventory map.

Scope and owner of the current increment: Phase A inventory and baseline only.
Bounded to cataloging the checked-in job/thread facilities, their configuration
defaults, the existing parallel seams, documented concurrency hazards, and the
first pilot selection. No scheduler, interface, or migration is introduced here.

## Status

| Work item (Phase A) | Status | Evidence |
| --- | --- | --- |
| Catalog current job/thread-pool facilities and their scheduling policies | Complete (static) | `public/vstdlib/jobthread.h`, `vstdlib/jobthread.cpp`; sites cited below |
| Record worker-count and threading configuration defaults | Complete (static) | `-threads`/cap and 3-worker default; `host_thread_mode`; per-feature ConVar defaults, below |
| Inventory existing parallel subsystem seams with ownership/conflict class | Complete (static) | Seam matrix below, each row verified to a current `file:line` |
| Catalog documented concurrency hazards and ordered-callback barriers | Complete (static) | `sv_parallel_sendsnapshot` comment; `YieldWait` self-help note; legacy phase orderings |
| Reproducible **runtime** baseline (frame/tick captures, budgets) | **Unavailable in this environment** | No runtime target, maps, or profiling hardware present; requirements listed below |
| Select first particle workload | Decision recorded (D-A1) | `CParticleMgr::UpdateNewEffects` gather/compute/commit seam |

Assessment: the static inventory is complete and every cited seam was verified to
a current source location. The runtime portions of the Phase A exit criteria
(reproducible baseline capture, measured budgets) are **not** satisfied here and
are recorded as unavailable, not as passing. Phase A's gate is therefore
partially met: ownership map complete; baseline capture outstanding.

## Verified current facilities (observed facts)

All line numbers are at working-tree revision `a4f6f95f`.

### Core job/thread-pool API — `public/vstdlib/jobthread.h`
- `CJob` (`:440`), `IThreadPool` (`:150`), `CJobSet` (`:588`),
  `CParallelProcessor` (`:850`), `CParallelProcessorBase` (`:1071`).
- Job vocabulary: `JobStatusEnum_t` (`:71`), `JobFlags_t` (`:83`, with
  `JF_IO`, `JF_BOOST_THREAD`, `JF_SERIAL`, `JF_QUEUE`), `JobPriority_t` (`:91`,
  `JP_LOW/NORMAL/HIGH`). There is **no** dependency-edge, resource-declaration,
  or continuation-graph type in this header — confirming RFC §"Current
  implementation" observation that no public dependency-graph contract exists.
- `IThreadPool::AddJob` (`:187`); the wait entry points `YieldWait(...)`
  (`:177`–`:182`) offer the calling thread to the pool.
- `AddCall` (macro, `:228`+) executes the functor **inline** on the caller
  (`FunctorDirectCall`) when `!NumIdleThreads()`; otherwise it queues. Inline
  execution on the submitting thread is thus a first-class, default behavior.
- `ParallelProcess` free functions (`:953`, `:962`, `:971`): fork/join over an
  item array; workers claim elements via the shared processor. This is dynamic
  distribution within one batch, not worker-local stealing (RFC-consistent).

### Pool policies — `vstdlib/jobthread.cpp`
- `CGlobalThreadPool::Start` (`:307`): worker count is `-threads` minus one when
  supplied (`:309`); otherwise `nThreadsMax` is capped at **4** (`:319`).
- `CThreadPool::Start` (`:903`) default PC path warns and limits to **3** workers
  ("Current >4 processor configs don't really work so well … cache issues",
  `:923`–`:924`).
- `CThreadPool::AddJob` (`:667`): with no idle thread and no `JF_IO`/`JF_QUEUE`,
  the job is **executed inline** on the caller (`:691`–`:697`); zero worker
  threads also forces inline `Execute()` (`:680`–`:687`). `JF_QUEUE`/`JF_IO`
  suppress the inline path.
- `InsertJobInQueue` (`:719`): non-serial jobs go to a per-thread direct queue
  (when a service thread is pinned) or the shared queue; `JF_SERIAL` jobs are
  forced onto thread 0's direct queue (`:735`–`:737`). Confirms "direct queues +
  a shared queue; no general worker-local stealing."
- `YieldWait` (`:609`): drains the shared queue, then waits on completion events
  once the queue is empty (`:621`–`:640`). The in-code comment records that a job
  spawned by a worker will **not** wake a main thread already waiting on events
  ("jobs are only ever added from the main thread") — the documented self-help
  limitation the RFC cites, and a concrete reason waits are execution-policy
  boundaries.

## Worker-count and threading configuration baseline

| Setting | Default (this tree) | Site |
| --- | --- | --- |
| Global pool worker cap (no `-threads`) | 4 | `jobthread.cpp:319` |
| Default compute-pool worker limit (PC path) | 3 | `jobthread.cpp:923`–`924` |
| `host_thread_mode` | `0` on PC (`1` on X360) | `engine/host.cpp:463` |
| `sv_parallel_packentities` | **`1` (enabled)** | `engine/sv_packedentities.cpp:389` |
| `sv_parallel_sendsnapshot` | **`0` (disabled; documented crash)** | `engine/sv_main.cpp:1830` |

Observation, not target: the only broadly-enabled server-side parallel seam by
default here is entity packing; the snapshot-send seam is disabled with a
recorded crash. Any capacity policy must account for existing material, I/O,
audio, and legacy pools already claiming workers under these caps.

## Existing parallel subsystem seams (verified inventory)

Each row verified to a current site. "Conflict class" is the initial,
conservative Phase-A classification for later resource-declaration audit — a
hypothesis to be proven by migration-time audit, **not** an established fact.

| Seam | Verified site | Pattern | Initial conflict class (hypothesis) |
| --- | --- | --- | --- |
| Particle simulation | `particlemgr.cpp:1855` (`ParallelProcess ProcessPSystem`); commit `DetectChanges` `:596` | gather (serial `Update`) → parallel `ProcessPSystem` → serial `DetectChanges` | Per-system outputs likely disjoint; control-point gather + change publication must stay serial |
| Bone setup | `c_baseanimating.cpp:2693` (`ThreadedBoneSetup`); worker `SetupBonesOnBaseAnimating` `:2656` | parallel over previous-frame bone setups | Parent/attachment/model-cache dependencies; model pins required |
| Query-cache maintenance | `querycache.cpp:244` (`ProcessQueryCacheUpdate`, `N_WAYS_TO_SPLIT`) | parallel disjoint hash chains → serial victim merge | Disjoint chains; ordered obsolete-entry merge |
| Entity packing | `sv_packedentities.cpp:454` (`PackWork_t::Process`); default on | parallel per-entity packing | Stable snapshot lifetime; disjoint per-entity output; encoding-helper audit |
| Client leaf — insert | `clientleafsystem.cpp:566` (`PreRender` → `InsertIntoTree`, Frame lock) | parallel under frame lock | Shared-tree exclusivity vs. visibility consumers |
| Client leaf — translucency | `clientleafsystem.cpp:1372` (`ComputeTranslucentRenderLeaf`) | parallel FX-blend compute | Preserve transparency/draw-order requirements |
| Snapshot send | `sv_main.cpp:1830` (`SV_ParallelSendSnapshot`, disabled) | per-client parallel send | **Known-unsafe:** shared `m_FrameSnapshots`; HLTV/replay excluded |

Ordered gameplay/callback barriers to preserve (not seams to parallelize):
`CServerGameDLL::GameFrame` explicit system order, `Physics_RunThinkFunctions`
`curtime` reset + deletion suppression, client `SimulateEntities`/`OnRenderStart`
ordering, and `IGameSystem` registration-order updates under model-cache scopes
(RFC §"Mutable simulation and ordered callbacks"). These remain conservative
mutation barriers until individually audited.

## Documented concurrency hazards (evidence to honor)

- **Snapshot manager race** — `sv_main.cpp:1823`–`1830`: concurrent
  `WriteDeltaEntities`/`WriteTempEntities` on `g_FrameSnapshotManager.m_FrameSnapshots`
  caused customer crashes; the option ships disabled and excludes HLTV/replay.
  Establishes the RFC's migration requirement: per-client work needs stable
  snapshot inputs, independently owned outputs, and an explicit owner for shared
  snapshot bookkeeping. Enabling the existing ConVar is **not** an acceptance
  criterion.
- **Wait-time self-help boundary** — `jobthread.cpp:628`–`640`: a waiting caller
  does not pick up worker-spawned jobs; and the single-player wait warns against
  executing queued AI operations at the wrong point. Confirms that "an
  unfinished job must never consume a worker merely to wait" and that helping
  arbitrary queued work can violate phase assumptions.

## Runtime baseline availability (honest gap)

No runtime baseline was captured. This environment has no built product, no
captured maps/demos, and no profiling hardware, so the RFC Phase A exit item
"reproducible baseline; budgets identified" is **unavailable**, not passing. To
close it later, the following are required and currently absent:
- a configured/built client + dedicated-server on a declared supported profile;
- representative captures (a particle-heavy client scene; a populated
  dedicated-server tick) with fixed warmup and sample policy;
- per-subsystem budgets (median/tail frame & tick time, critical-path duration,
  scheduling overhead, retained memory) agreed **before** any migration measurement.

## Decisions

### D-A1 — First particle workload (pilot selection)
Select `CParticleMgr::UpdateNewEffects` (`particlemgr.cpp:1855`) as the RFC's
first gather/compute/commit reference. Rationale: it already separates a serial
control-point/entity `Update`, a parallel `ProcessPSystem` compute batch, and a
serial `DetectChanges` publication — matching the RFC's canonical migration
shape with the least entity-mutation entanglement. This is a target selection,
not a completed migration; equivalence and measurement gates are Phase D.

## Prerequisites and gating (why Phase B is not started here)

Phase A proceeds independently of RFC 0001 provider migrations (RFC §"Delivery
plan"). Phase B (contracts + serial graph, AGENTS.md R10) depends on the
`Expected<T,E>` and result/ID vocabulary (R05) and the composition/lifecycle
kernel + task-runner contracts (R06), both `planned` at this revision. Phase B
also requires the RFC 0005 independent graph model and legacy-vs-serial
comparison harness (R02, `planned`). Building the sealed-graph builder,
validator, or deterministic executor before those contracts exist would either
duplicate an unbuilt `Expected`/runner authority or invent a graph API against
placeholders — both disallowed by AGENTS.md ("do not invent a command from a
proposed RFC"; "one authoritative representation"). Phase A is therefore the
correct dependency-ready increment.

## Commands

No RFC-0003-specific runner exists yet (it is Phase B/C work). The seam sites in
this record can be re-verified against source with, e.g.:

```text
grep -n "ParallelProcess\|CParallelProcessor" game/client/particlemgr.cpp \
  game/client/clientleafsystem.cpp game/shared/querycache.cpp \
  engine/sv_packedentities.cpp game/client/c_baseanimating.cpp
grep -n "sv_parallel_sendsnapshot\|m_FrameSnapshots" engine/sv_main.cpp
grep -n "nThreadsMax = 4\|worker threads\|NumIdleThreads" vstdlib/jobthread.cpp
```

## Next increment

Phase A is inventory-complete. The next dependency-ready work is either (a) the
runtime baseline capture above, once a supported profile is buildable, or (b)
Phase B contract design once R05/R06/R02 land. Do not begin the sealed-graph
builder or executor until those contracts exist; when they do, Phase B must
deliver the independent graph model and the ordered serial host graph that
reproduces baseline behavior, verified by the Q-JOBS harness (RFC 0005), before
any parallel executor or particle migration is claimed.
