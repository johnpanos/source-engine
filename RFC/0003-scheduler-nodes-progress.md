# RFC 0003 progress: opening the host frame's nodes (R10, R20, R21/R30)

Updated: 2026-09-25 (sections 6 and 7). Branch `scheduler-nodes` in worktree
`source-engine-scheduler`, rebased onto `subsystem-refactor` at `d59bcda5`
and since merged into it (tip `568a8d02`, 2026-09-24)
(parent increment: [scheduler trust](0003-scheduler-trust-progress.md)).
Roadmap rows R10, R20, R21 and R30 stay `partial`; see "Open".

The increment:

1. measured where the frame goes before splitting anything;
2. split the host `Render` phase into oracle-verified sub-nodes;
3. fixed the `host_thread_mode 2` deadlock (a per-thread array overrun) and the
   uncapped filesystem I/O pool;
4. closed the remaining R20 items: bounded shared queue with an overflow
   policy, nested-wait and starvation detection, capacity and overhead
   budgets;
5. made the client's render-start region (with the particle and
   previous-frame bone cohorts) a declared frame graph, and made the pooled
   executor overlap host work with pool work where declarations allow;
6. tried the threaded listen server in the launchers and withdrew it;
7. added a live shadow verifier that checks the pooled previous-frame bone
   batch against a serial rerun in the same process, and recorded why the
   particle batch needs a different invariant.

## 1. Profile before splitting

`perf record --call-graph dwarf` of the frame-pacing workload
(`testchmb_a_02`, headless native Vulkan, 3781 frames). Render stacks are
deeper than perf's 64 KB copy, so each sample is attributed to the nearest
enclosing known region (a script over the collapsed stacks). Main thread,
steady state 7.41 ms/frame:

| Region | ms/frame | Share |
| --- | --- | --- |
| Draw submission (native Vulkan emit) | 4.84 | 65% |
| Present | 0.41 | 5.5% |
| VGUI | 0.26 | 3.5% |
| Model draw setup | 0.20 | 2.6% |
| Server tick, all of it (commands 0.14, game frame 0.06, packing 0.06, think/AI/physics 0.05, VPhysics 0.03) | 0.35 | 4.7% |
| Particle draw | 0.12 | 1.6% |
| Caller-side pooled batch dispatch (particles, bones) | 0.07 | 1.0% |
| Bone setup (on demand) | 0.04 | 0.6% |
| Particle simulation | 0.005 | 0.1% |
| Host frame graph executor | < 0.01 | |

Findings that shaped the rest: the frame is draw-submission bound; the server
tick is small in single-player Portal; the particle and bone cohorts cost less
than their own dispatch. Overlapping them cannot give a measurable win on this
workload; their value is as the first declared nodes.

## 2. Render split into sub-nodes (R10)

`_Host_RunFrame_Render` (with `Host_UpdateScreen` and `SCR_UpdateScreen` inside
it) is now 14 ordered steps (`engine/host_render_steps.h`): `RenderBegin`,
`UpdateScreenBegin`, `ScreenAdmit`, `ScreenBeginFrame`, `ClientRenderStart`,
`EngineFrameBegin`, `ViewRender`, `Present`, `ClientRenderEnd`,
`EngineFrameEnd`, `DynamicModels`, `MaterialsEndFrame`, `DecayLights`,
`RenderEnd`. Each is the original statement block. The early returns of
`SCR_UpdateScreen` became the admission step's result, evaluated when the
later screen steps run. One authority: `_Host_RunFrame_Render` (the
`host_frame_graph 0` path) and `SCR_UpdateScreen` (its other callers) run the
same steps in order, and the host frame graph runs each as its own node
(`engine/gl_screen.cpp`; `cl_null.cpp` keeps the dedicated stubs).
`Host_UpdateScreen` had no other caller and is gone.

Verification:

- Oracle (`jobsystem.hostframe`, 4152 checks): the verbatim pre-change render
  stage (`hostframe_render_legacy_oracle.h`, from `504ee284`) and the steps run
  under the same recording stand-ins in 4000 seeded multi-frame scenarios, now
  with render guards (loading, uninitialized screen/console, demo playback,
  window-size failure, snapshots, `mat_norendering`, `scr_nextdrawtick`) and
  longjmp exits from inside render steps. Coverage: 10269 render stages, 5696
  admitted screen updates, 409 VGUI-only frames, 172 render-step exits.
- Sensitivity: every position of the longest frames (render steps included)
  swapped, dropped and duplicated: 120 of 120 detected. Placements that only
  move `UpdateScreenBegin` (a `_DEBUG`-only step) run unmutated instead.
- Live: the trace records each render step. 18 pairs of Portal captures
  (three map/rate configurations, legacy and graph modes, two runs each) are
  identical in call order and state apart from the interpolation noise that
  two legacy runs also show (5162 events per run).

## 3. Threaded listen server deadlock and I/O pool capacity (R20)

`host_thread_mode 2` hung within about 8 frames on this branch and on the
baseline build. Cause: tier0 hands out thread ids up to 127, but
`MAX_THREADS_SUPPORTED` was 32, and two per-thread arrays are indexed by
`g_nThreadID`: the voxel tree's `m_pVisits` (engine spatial partition) and the
dirty-partition list's `m_nReadLockCount` (game DLLs). A thread with id 32 or
more wrote past `m_pVisits` into the members after it, the tree's
`CThreadSpinRWLock` among them, and the server job then spun forever in
`SpinLockForWrite`. Fix: one bound, `MAX_THREADS_SUPPORTED` 128, used by
tier0's id allocator too. A fixture starts 48 threads at once and checks every
id fits (failed before the fix: ids up to 48 against 32).

Threaded Portal now runs: every threaded capture since the fix completes
(every threaded run hung before). Two trace corrections make threaded runs
comparable: the trace no longer reads server state while the async server job
is in flight (that read raced with the job; the fields print `-` between
submit and join), and it leaves out the job's own events, because the job runs
on a worker or, when no worker has started it, on the host thread at the join,
which varies run to run. Legacy (`host_frame_graph 0`, `cl_render_start_graph
0`) and full-graph (`1`, `2`) threaded captures on the rebased build are
identical: 6 of 6 pairs, 5398 events, 237 of 245 frames threaded.

Why ids reached 32: the filesystem's async I/O pool had
`nThreadsMax = MIN( params.nThreads, 4 )`, and `nThreads` is -1 ("choose"), so
the cap was -1 (none) and the pool started one thread per logical CPU (32
here). This came from upstream commit `f2fa241a` (2023), which replaced the
original single PC I/O thread. The cap of 4 its comment intends is restored.
Census of a live Portal run (tier0 now names pool threads for the OS on Linux:
`CmpJob0`, `IOJob2`, ...): `CmpJob` 3, `IOJob` 4, `SaveJob` 1; the baseline
build ran 28 more engine threads.

`CThread::Join` read the thread-id fields without the thread's lock while the
exiting thread wrote them under it (found by TSan with the new fixture); it
now reads and clears them under the lock and joins outside it.

## 4. Remaining R20 items

### Bounded shared queue

`CJobQueue` admits through `TryPush`, which never runs or drops a job. The old
overflow path (pop the oldest job and run it on the pushing thread) is gone:
it would run unrelated work at the pusher's point. The shared queue holds 4096
jobs by default (`TP_DEFAULT_SHARED_QUEUE_CAPACITY`,
`SetThreadPoolSharedQueueCapacity`). When full (`AdmitToFullSharedQueue`):

- a worker of the pool runs the job it is submitting (blocking a worker could
  stall every worker);
- any other thread waits for space, running nothing and holding no pool lock;
- while the pool cannot consume (suspended, or no workers), the job is
  admitted beyond capacity and counted, instead of blocking forever.
  `SuspendExecution` wakes blocked producers.

Direct (affinity) queues stay unbounded. Re-insertions of admitted work
(priority changes, quiescent spills and put-backs) use `ForcePush`.

### Nested waits and starvation

`YieldWait` from inside a worker is a nested wait (counted). Waiting for an
unstarted job of the same pool that the worker may not run (bound to another
worker) is a forbidden nested wait: counted and warned about once; it still
waits, as the legacy contract requires. A worker may now run a waited job
bound to itself (its service thread, or `JF_SERIAL` for worker 0): the
previous rule excluded bound jobs, which deadlocked a worker waiting for its
own queued work. Every worker blocked in a wait with work queued is counted
and warned about as starvation. All of it is in
`ThreadPoolSchedulingStats_t`.

Fixture (`jobsystemthreadpooltest`, 2700 checks): blocked producer, worker
caller-runs (40 exactly), suspended over-capacity admission, suspend releasing
a blocked producer, forbidden wait, own-bound waits (service thread and
`JF_SERIAL`), starvation, thread ids. Mutation checks, each applied to the
pool and rebuilt: capacity ignored (8 checks fail), own-bound inline disabled
(hang, runner timeout), forbidden wait not counted (1 fails), worker blocking
on a full queue (hang), suspended pool blocking (hang), starvation not
detected (1 fails): 6 of 6 detected.

### Budgets

[`quality/budgets/scheduler-v1.json`](../quality/budgets/scheduler-v1.json),
checked by [`tools/quality/scheduler_budgets.py`](../tools/quality/scheduler_budgets.py)
(`check`, `run`, `census`; 9 positive/negative self-tests) and recorded in
`quality/baseline.json` (`scheduler.budgets`, `jobs.scheduler-budgets`):

- Capacity (Linux desktop): `CmpJob` 3, `IOJob` 4, `SaveJob` 1, `MatQueue` 1,
  at most 9 pool workers in total. Census: 8.
- Overhead: pool microbenchmarks at 3 workers and at 1 worker (low-core), the
  CTSQueue shapes, and the serial graph's cost per node. Limits are about
  twice the worst of three medians on this shared, frequency-scaled host, so
  they catch lost wakes, polling and serialization. Measured run: all 13 rows
  and all 5 capacity rows pass (e.g. wake latency 17 us against 150, graph
  13 ns per node against 50).
- Android and Apple rows are declared unverified (owner R29).

## 5. Declared frame graph and the render-start region (R20/R21/R30)

### Pooled executor overlap

`PooledExecutor` runs graphs in waves. Within a wave it used to run the
compute jobs and then the caller's (host-thread) jobs. The new
`IWorkerBackend::ParallelForWithCaller` lets a backend start the compute jobs
and run the caller's jobs meanwhile; the engine pool bridge does, and the
interface default keeps the old order, so every existing backend behaves as
before. Tests: a host job and a compute job that each wait for the other to
start both finish (only concurrency satisfies that), on the jobsystem test
backend and on the real engine pool (20 of 20 rounds).

### DeclaredFrameGraph

`public/jobsystem/declared_frame_graph.h` (C++11 surface, C++20
implementation, vstdlib exports `CreateDeclaredFrameGraph` /
`RunDeclaredFrameGraph`). A region is nodes in legacy order:

- host nodes run on the calling thread, always in array order;
- batch nodes run independent items on bounded runners, with begin/end around
  each participating runner (e.g. a model-cache lock);
- each node declares the resource domains it reads or writes;
  `FRAME_DOMAIN_ALL` marks an unaudited node that conflicts with everything;
- a node runs after an earlier node it conflicts with; nothing else orders a
  batch, so in pooled mode it may overlap host nodes it does not conflict
  with (those that become ready in the same wave);
- the batch's join job carries its declarations, so `Seal()` independently
  checks that no conflicting pair is left unordered; `MayOverlap` and
  `OverlappingPairs` report the unordered pairs;
- a graph started from inside a batch item runs serially.

Contract test (`jobsystem.declaredframe`, 173 checks): serial array order,
conflict edges, the ALL domain, real overlap of an unordered batch and host
node, publication of 1000+ items to a conflicting commit on several runners,
exactly-once items, runner brackets, nested batches, invalid input, graph
reuse.

### Render start

`OnRenderStart`'s blocks are host nodes (`game/client/client_render_start_steps.h`,
statics of `CRenderStartSteps`, a friend of the classes that befriend
`OnRenderStart`). The two cohorts are split into gather / batch / commit
without changing their legacy path, which now runs the same three parts:

- `C_BaseAnimating::ThreadedBoneSetupBegin / Count / Item / RunnerBegin /
  RunnerEnd / End` (`ThreadedBoneSetup` = these, with its existing dispatch);
- `CParticleMgr::SimulateBegin / SimulateBatchCount / SimulateBatchItem /
  SimulateEnd` (`Simulate` = these; the measuring mode runs whole in Begin).

`cl_render_start_graph`: 0 runs the blocks in order with each cohort's own
dispatch (default); 1 and 2 run the declared graph serially or on the engine
pool, and the graph then owns both batches.

Declarations: every legacy block is unaudited (`FRAME_DOMAIN_ALL`); the
batches declare animation state, transforms and bone caches (bones) and
particle state plus world queries (particles). The edges leave no pair
unordered: each batch's commit immediately follows its gather in the legacy
order, so no host node can run beside it, and every other node is unaudited.
This is the honest result for this region: its cohorts become declared nodes,
but legacy order leaves nothing to overlap.

Oracle (`jobsystem.renderstart`, 2066 checks): the verbatim pre-change
`OnRenderStart` (`renderstart_legacy_oracle.h`) against the legacy node table,
the serial graph and the pooled graph (3 runner threads), 2000 seeded frames:
the first two must match exactly, the pooled one after sorting each batch's
items (1335 bone and 1965 particle batches; pooled bone batches split across
runners). 0 overlapping node pairs asserted. 62 of 62 mutated tables
(swapped, dropped, duplicated nodes) detected.

Live: Portal captures in modes 0, 1 and 2 give identical host traces; final
screenshots differ between two runs of the same mode, so they are not an
oracle here.

Frame cost (frame-pacing workload, warm pass, three interleaved rounds per
mode, host load 5 to 9, CPU frequency scaling active):

| `cl_render_start_graph` | Median frame (ms), per round | Median | p99 median |
| --- | --- | --- | --- |
| 0 (legacy order, cohorts pooled by their own ConVars) | 14.59, 14.87, 14.77 | 14.77 | 21.19 |
| 1 (declared graph, serial) | 13.59, 14.11, 14.67 | 14.11 | 21.18 |
| 2 (declared graph, pooled) | 14.64, 13.25, 14.49 | 14.49 | 21.09 |

No measurable difference, as the profile predicted (the cohorts are well under
0.1 ms). The default stays 0: making the graph the default would also move
control of both cohorts from their existing ConVars to this one, which is a
product decision rather than a measured gain.

(2026-09-25) The engine default is still 0. The Portal launchers pass
`cl_render_start_graph 2`: `run.conf` since 2026-09-24 and `play_p2` since
2026-09-25. `JOB_ARGS=` rolls this back.

## 6. Threaded listen server: tried in the launchers, then withdrawn (2026-09-25)

`host_thread_mode 1` runs the listen server's tick on the compute pool while
the client renders; the engine default stays 0. The Portal (`run.conf`) and
Portal 2 (`play_p2`) launchers passed it for part of 2026-09-25 and no longer
do. The pooled render-start, query-cache and carve graphs don't depend on it
and stay on.

Why it was withdrawn:

- **It broke movement after the first map.** Threaded input stamped user
  commands with `host_tickcount`, which never resets. After a map load the
  new server's tick starts over, and `CBasePlayer::IsUserCmdDataValid`
  (a window of 2.5 s either side) made every command inert. Fixed in
  `ff6f287f`: threaded input uses the client's tick
  (`cl.GetClientTickCount() + tick`) in `engine/host.cpp`,
  `engine/host_frame_phases.h` and the legacy oracle. Evidence: a two-map
  walk in Portal 2 and Portal 1 (`getpos` and the server's own origin) moves
  on both maps; before the fix the server logged "UserCommand out-of-range"
  for every command on map 2.
- **The recorded gain did not reproduce.** The first record (three rounds at
  host load 11 to 18) gave warm medians of 11.0 to 12.6 ms for mode 0 and
  8.7 to 9.6 ms for mode 2. At host load about 2, six warm passes per mode
  over three interleaved rounds fall into two clusters in every mode: about
  12.5 ms and about 17 ms per frame. In a slow run every CPU-side cost is
  about 30% higher (emit 10.6 against 8.1 ms, backend 1.05 against 0.71 ms),
  so the cluster follows CPU scheduling, not the mode. Fast runs only:
  mode 0 12.5 and 13.4 ms, mode 1 12.5, 12.6 and 13.1 ms. That matches the
  profile in section 1: the whole single-player server tick is 0.35 ms, so
  overlapping it can save no more than that.
- **It adds a frame of movement latency.** The earlier note said the player
  view is predicted; in single-player it is not (`cl_predict 0`, Portal 2
  retail too), so the view shows the server's result from the frame before.

Method note for later frame-pacing comparisons: compare `engine` and `emit`
per run, not only the interval median, so a run in the slow cluster is
recognized rather than read as a gain or a loss.

The frame is emission-bound (8 to 10.6 ms of emit in a 12.5 to 17 ms frame).
The queued material system (`mat_queue_mode 2`, R32-QUEUED) moves emission to
the render thread: in the same session it measured 9.9, 8.4 and 12.9 ms
against 12.8, 23.1 (noisy) and 16.5 ms for mode 0 in three interleaved
rounds, with the p99 about 19 ms instead of about 24 ms. Its launcher trial
is recorded in the
[queued rendering record](0001-native-vulkan-queued-rendering-progress.md).

Rollback of the job args: `JOB_ARGS= ./play` or `JOB_ARGS= ./play_p2`.

## 7. Live shadow verifier: previous-frame bones (2026-09-25)

Problem: the pooled render-start cohorts were shown equal to legacy only
offline (`jobsystem.renderstart` and the cohort suites) and by identical
host traces. Two runs of the same scenario are not deterministic, so
pooled and serial outputs can't be compared across runs. The verifier
compares them inside one run.

### Protocol

`public/jobsystem/batch_shadow_verify.h` (header-only, C++11 and later)
owns the protocol. When verification is on, the cohort's gather captures
every item's state. After the batch joins, the commit:

1. asks the cohort which items it can verify;
2. captures each verifiable item's pooled state;
3. restores every verifiable item to its captured pre-state;
4. reruns those items serially, in item order, on the host thread;
5. byte-compares each serial state with its pooled state;
6. restores the pooled state and captures it again. A restore that is not
   byte-exact is reported as a restore mismatch.

The frame therefore continues with the pooled outputs. Restoring every item
before rerunning any gives serial-loop semantics when one item reads
another's state (IK attachments and bone-merge followers can). Items that
can't be verified are neither restored nor rerun; they are counted by
reason. The protocol starts no threads.

### Bones: what an item writes

One item is `SetupBones( NULL, -1, -1, t )` on an unparented entity with
the threaded-setup flag set. It writes the following, and the snapshot in
`game/client/bone_setup_shadow_verify.cpp` covers each one:

- the bone cache, the accessor's readable and writable masks, the
  previous and accumulated masks, the last setup time and the model bone
  counter;
- `m_nSequence` (reset when it is out of range), the sequence
  transitioner's queue, and the new-sequence parity;
- attachments (`PutAttachment` also reads the previous value, for the
  origin velocity);
- `m_pRagdollInfo->m_bActive` (`UnragdollBlend`);
- jiggle-bone state (`BuildTransformations`);
- overlay layers, their event cycles and their interpolation history
  (`CheckForLayerChanges`, `BlendWeight`). History is restored through
  private `CInterpolatedVar::Copy` copies. `SetNumAnimOverlays` can leave
  fewer histories than layers, so the two are counted separately;
- flex weights (`C_BaseFlex::BuildTransformations`).

Records pack `C_AnimationLayer` member by member, because its trailing
padding would otherwise make equal layers compare unequal.

Skipped, by reason:

| Reason | Why the item isn't rerun |
| --- | --- |
| `parented` | The item does no work. |
| `ik` | The entity has an IK context, or the pooled item created one. `CIKTarget` has private members and padding that can't be captured byte-exactly from outside `CIKContext`. |
| `jiggle-alloc`, `bone-merge-alloc` | The pooled item created or released the object. When the object already existed, the rerun is exact: the bone-merge cache is a memo keyed on the followed entity and both model headers. |
| `renderfx` | `kRenderFxDistort` and `kRenderFxHologram` draw from the global random stream in `ApplyBoneMatrixTransform`. A rerun would advance that stream. |
| `mouth` | `ControlMouth` writes a pose parameter and its interpolation history. |
| `layout` | A bone, attachment, layer or history count changed. |

These effects fall outside the snapshot:

- the model-cache lock, which the rerun brackets like a runner;
- `EFL_SETTING_UP_BONES`, which is set and cleared inside the call (it is
  compared but not restored);
- `CalculateIKLocks`' partition and abs-recompute state (IK items are
  skipped);
- caches that the pooled item already filled (studio headers, abs
  transforms);
- `UpdateVisibility` from `CheckForLayerChanges`, which is idempotent;
- nested `SetupBones` on other entities, whose caches are already valid for
  the frame.

The pose debugger (`ent_posedebug`) keeps per-model state and is not
supported while verifying.

### Controls

- `cl_bone_setup_verify` 0/1, default 0;
- `cl_bone_setup_verify_fault` (negative control, only while verifying):
  1 moves the first unparented item's bone 0 translation by one ulp inside
  the pooled item; 2 drops that item's work;
- `cl_bone_setup_verify_report`, also printed at client shutdown. It gives
  batches, items, verified and skipped counts (by reason), mismatches,
  restore mismatches, batches whose items ran on two or more threads, items
  that ran off the host thread, and gather and verify time. The first 16
  mismatches are printed with the entity, field and byte, and bone values
  where a bone differs.

When off, each item pays one static `bool` test and each batch one ConVar
read. All three dispatch paths route their items through the same function:
the render-start graph (`ThreadedBoneSetupItem`), the cohort's job graph and
legacy `ParallelProcess`.

### Evidence

All runs used the frame-pacing workload: `testchmb_a_02`, three passes,
headless offscreen native Vulkan. The player model is drawn through the
portals, so a batch has two or more items. `build-bonever` is a Waf
release tree at `002d968e` plus this change.

| Run | Batches | Items verified / skipped | Mismatches | Restore mismatches | Multi-thread batches |
| --- | ---: | ---: | ---: | ---: | ---: |
| `cl_render_start_graph 2` (launcher config) | 1934 | 3958 / 0 | 0 | 0 | 1618 |
| `cl_render_start_graph 1` (serial graph) | 1934 | 3958 / 0 | 0 | 0 | 0 |
| `cl_render_start_graph 0` (cohort graph, pooled) | 1934 | 3958 / 0 | 0 | 0 | 1596 |
| `cl_render_start_graph 0`, `cl_bone_job_graph 0` (legacy `ParallelProcess`) | 1934 | 3958 / 0 | 0 | 0 | 70 |
| graph 2, fault 1 (one ulp) | 1934 | 3958 / 0 | 1934 | 0 | 1585 |
| graph 2, fault 2 (drop) | 1934 | 3958 / 0 | 1927 | 0 | 93 |
| legacy, fault 1 | 1934 | 3958 / 0 | 1934 | 0 | 86 |

- The verified items are the view model (`v_portalgun.mdl`, 45 bones), the
  player (`chell.mdl`, 68 bones, an overlay and flex entity) and
  `box_dropper_cover.mdl`. Only models with 16 or more bones join the
  cohort. The floor turret (9 bones), security camera (4) and GLaDOS's
  body (12) don't.
- Fault 1 is reported in every batch, at `bones` byte 12 (bone 0,
  translation x). For example, pooled -905.468689 against serial
  -905.46875.
- Fault 2 is reported in 1927 of 1934 batches. In the other 7 the dropped
  item was already a no-op (its bones were current for the frame), so
  dropping it changes nothing.
- Skip path, checked live: with `renderfx 15` on the player (third person,
  `testchmb_a_02`), 193 player items were skipped as `renderfx`, 1000
  items were verified, and there were 0 mismatches.
- IK, jiggle, mouth, bone-merge and layout skips never occurred in Portal
  content.

Off means no behaviour change. Host frame captures (`-hostframetrace`,
`cl_clock_correction 0`, one pass of the same workload) were identical in
731 frames and 15611 events in all these comparisons:

- the base build (`002d968e`) against this build with the verifier off;
- verifier off against verifier on;
- two runs with the verifier on.

The comparison uses `host_frame_capture.py --tolerance ia=0.0001`. The `ia`
differences are wall-clock noise: two identical verifier-on runs differ in
1309 of them. In a static `testchmb_a_01` boot, where no batch forms, the
captures were byte-identical (395 frames). The zero restore mismatches in
every run above show that the frame keeps the pooled state when the
verifier is on.

Overhead, from the verifier's own timers over 1934 batches per run:

- verify, 29 to 40 µs per batch (56 to 77 ms per run);
- gather, 3 to 4 µs per batch;
- together, under 0.05 ms per frame.

Six interleaved off/on rounds of the warm pass (`cl_render_start_graph 2`,
host load 2.5 to 7) could not resolve that:

- off medians: 8.87, 5.78, 5.28, 8.02, 6.06 and 5.41 ms (median 5.92);
- on medians: 5.80, 5.80, 5.84, 5.93, 6.17 and 6.07 ms (median 5.89).

The spread is the bimodal clustering described in section 6.

Standalone fixture, `jobsystem.shadowverify`, 2179 checks. It runs the
protocol against a stand-in cohort on three real threads:

- a correct batch: 0 mismatches, each item rerun once, and the state after
  comparison byte-identical to the pooled state;
- skipped items are neither restored nor rerun;
- seeded faults are reported at the right item, field and byte range:
  epsilon, drop, and two items swapped;
- cross-item reads: a forward loop matches, a reverse-order pooled run is
  reported;
- negative cohorts: a snapshot that doesn't restore state the item reads
  gives a mismatch on every item, and a lossy restore gives a restore
  mismatch on every item;
- record comparison: layout and byte divergences, -0 against +0, NaN
  payloads, `Read`.

It passes under g++ in the default and release configurations, under
clang++ with ASan/UBSan, and under TSan with 0 warnings. The Q-JOBS domain
has 17 suites: 15 match and 2 optional TSan suites are skipped.

### Particles: why not the same protocol

`CParticleMgr::SimulateBatchItem` simulates one `CNewParticleEffect` and
its whole child tree:

- the bounding-box update;
- `SetDrawn`;
- `Simulate(dt)`: operators, emitters, initializers, kill lists and
  children;
- `GatherLight`;
- the remove flag.

The random stream is per collection and deterministic (`m_nRandomSeed`
plus sample ids and `m_nRandomQueryCount`), so randomness alone would not
prevent a rerun. Three things do:

- There is no way to copy a collection's state. The particle attribute
  memory, the per-operator context blocks (which may hold lazily allocated
  heap state, such as collision caches), the control points and the child
  collections would all need deep copies, and `CParticleCollection` has no
  copy or restore interface. Kill lists come from the manager's global
  pool.
- Operators and initializers query the world through
  `IParticleSystemQuery`: traces, lighting, and hitboxes on the controlling
  entity.
- The seed of a collection created with seed 0 (the common case) is
  `this + Plat_MSTime()`. That is why runs can't be compared: each run
  seeds differently.

Proposed smallest sound invariant (not implemented):

- a test-only fixed-seed mode, so each new collection is seeded from its
  definition and creation order;
- a per-effect state hash: current time, active count, the bytes of the
  active particles' attributes, the bounding box and the random query
  count;
- **twin effects**: pairs of identical fixed-seed effects on identical
  control points in a static test map, dispatched in the same pooled
  batch. Each frame their hashes must be equal. That holds whatever the
  run-to-run noise is, and it catches cross-item interference (shared
  operator-instance state, kill lists).

A serial-versus-pooled comparison of the same hashes then needs only a
static scene at a fixed `host_framerate`.

### Not verified

- The renderable bone batches (`r_renderable_job_graph`).
- Particles, entity packing, query cache and portal carving.
- IK, jiggle and mouth entities; none occur in Portal's cohort.
- Portal 2 (`./play_p2`): the client target compiles in a private
  `--build-games=portal2` tree, but it has not been run. Android and Apple
  have not been run either.
- A TSan run of the product with the verifier on.
- Pixels. In-game screenshots aren't deterministic across runs, so the
  frame's use of the pooled state rests on the restore check.

### Reproduction

```sh
# Fixture:
python3 tools/quality/conformance.py check --suite jobsystem.shadowverify
# Live (short --out: the engine command line is limited to 512 characters):
python3 tools/quality/frame_pacing.py --runtime run/runtime --build <waf tree> \
  --out /tmp/<short>/g2 --extra-arg=+cl_render_start_graph --extra-arg=2 \
  --extra-arg=+cl_bone_setup_verify --extra-arg=1 \
  [--extra-arg=+cl_bone_setup_verify_fault --extra-arg=1]
grep -a bone_setup_verify /tmp/<short>/g2/runtime/portal/console.log
# Off/on host captures: add --passes 1 --extra-arg=-hostframetrace --extra-arg=<file>
#   --extra-arg=+cl_clock_correction --extra-arg=0, then
python3 tools/quality/host_frame_capture.py a.t b.t --tolerance ia=0.0001
```

## Gates run

- gcc release `--tests` tree: all 15 job-system programs pass after the rebase
  (thread pool 2700, host frame 4152, declared frame 173, render start 2066,
  pooled 180, bridge 131, batch 1058, legacy batch 20932, frame 84, dynamic 140,
  Q-JOBS 7719, seal equivalence 6377, stress 166789, scheduler 274, portal
  carve 1611).
- clang TSan tree: thread pool, declared frame, render start, bridge, pooled,
  host frame and legacy batch fixtures: 0 warnings.
- Shared runner, whole manifest (new rows `jobsystem.declaredframe`,
  `jobsystem.renderstart`): 138 suites, 136 matched, 0 mismatched, 2 optional
  skipped.
- Style: `stylelint.py --changed --base subsystem-refactor`, 47 files, 0
  failures. Archlint: 64 new occurrences, none in files this branch touches
  (all from the concurrent Portal 2 work).
- `scheduler_budgets.py check` and a measured `run` with the census pass.

## Open (rows stay partial)

- Legacy order leaves nothing to overlap in the render-start region. The
  measured big item is draw submission (4.8 ms), which the queued material
  system moves to its render thread (section 6). Overlapping the server tick
  with client rendering (`host_thread_mode`) runs now, but it saves at most
  the 0.35 ms tick and adds a frame of latency; see section 6.
- The pooled executor overlaps only within a wave; an executor that keeps
  batches running across several host nodes would be needed for longer
  overlap.
- Legacy render-start blocks are unaudited (`FRAME_DOMAIN_ALL`). Auditing a
  block narrows its declarations; only then can a batch move past it.
- TickServer was profiled but not split; server think, AI and VPhysics stay in
  legacy order until audited.
- A live semantic oracle exists for the previous-frame bone batch only
  (section 7: 3958 items verified in each mode, 0 mismatches). The particle
  batch has a proposed invariant (fixed-seed twin effects) and no check yet.
  The renderable bone batches, entity packing, query cache and portal
  carving still rest on their offline oracles.
- Android/Apple budgets and census, and a full-product TSan run, remain open.

## Reproduction

```sh
# Oracles and contracts (shared runner):
python3 tools/quality/conformance.py check --suite jobsystem.hostframe \
  --suite jobsystem.declaredframe --suite jobsystem.renderstart
# Budgets:
python3 tools/quality/scheduler_budgets.py check
python3 tools/quality/scheduler_budgets.py run --build build-sched-tests \
  --profile linux-x86_64-desktop [--census census.json]
python3 tools/quality/scheduler_budgets.py census --pid <hl2_launcher pid>
# Live capture (as in the parent record), threaded:
python3 tools/quality/portal_boot.py ... --engine-arg=-hostframetrace --engine-arg=h.t \
  --startup-command="host_framerate 0.015" --startup-command="host_thread_mode 2" \
  --startup-command="host_frame_graph 1"   # then 0
python3 tools/quality/host_frame_capture.py legacy.trace graph.trace --tolerance ia=0.0001
```
