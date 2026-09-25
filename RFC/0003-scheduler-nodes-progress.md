# RFC 0003 progress: opening the host frame's nodes (R10, R20, R21/R30)

Updated: 2026-09-24. Branch `scheduler-nodes` in worktree
`source-engine-scheduler`, rebased onto `subsystem-refactor` at `d59bcda5`
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
   executor overlap host work with pool work where declarations allow.

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

## 6. Threaded listen server in the launchers (2026-09-25)

`host_thread_mode 1` runs the listen server's tick on the compute pool while
the client renders; the engine default stays 0. The Portal (`run.conf`
`JOB_ARGS`, used by `./play`) and Portal 2 (`play_p2`) launchers now pass it,
with the pooled render-start and query-cache graphs.

Frame cost, portal-frame-pacing-v1 warm pass, three interleaved rounds
(`frame_pacing.py --extra-arg="+host_thread_mode N"`, host load 11 to 18):

| `host_thread_mode` | Median frame (ms), per round | p99 (ms), per round |
| --- | --- | --- |
| 0 | 11.34, 12.59, 10.98 | 19.37, 22.31, 19.20 |
| 2 | 8.73, 8.96, 9.59 | 15.76, 14.36, 32.29 |

- The round-3 p99 of mode 2 comes from draw-emission spikes in `walk_orange`.
  Mode 0 hitches in the same frames under load.
- On this host (compute pool 3 threads), mode 2 and mode 1 select the same
  path. The launchers use 1, which stays unthreaded when the pool is empty.
- Cost: server-simulated objects reach the screen one frame later. The player
  view is predicted.
- Portal 2 with `host_thread_mode 1`: `portal2_scenarios.py --extra-arg` passes
  triple laser, catapult and the four wheatley-v1 story scenarios.
- Not measured: a Portal 2 frame-time workload, mobile profiles (the Android
  packages don't use these launchers), and input latency.

Rollback: `JOB_ARGS= ./play` or `JOB_ARGS= ./play_p2`.

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
  measured big items are draw submission (4.8 ms, a render-thread question)
  and the server tick against client rendering, which `host_thread_mode 2`
  already overlaps and which now runs; making threaded mode the default is a
  separately measured latency decision (RFC 0003).
- The pooled executor overlaps only within a wave; an executor that keeps
  batches running across several host nodes would be needed for longer
  overlap.
- Legacy render-start blocks are unaudited (`FRAME_DOMAIN_ALL`). Auditing a
  block narrows its declarations; only then can a batch move past it.
- TickServer was profiled but not split; server think, AI and VPhysics stay in
  legacy order until audited.
- No semantic live oracle for the client region (bone/particle outputs); the
  equivalence rests on the oracle and on the cohorts running the same parts in
  every mode.
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
