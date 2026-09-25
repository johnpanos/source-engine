# RFC 0003 progress: scheduler trust (R10 host graph, R20 engine pool)

Updated: 2026-09-25. Base revision `504ee284` (the history rewrite of `f7eb1cf1`;
source content identical), branch `scheduler-trust`, worktree
`source-engine-scheduler`. Owner: this increment. Roadmap rows R10 and R20;
neither row closes here (see "Open").

The increment has four parts:

1. The host frame after admission runs as an ordered serial graph and matches
   captured legacy frames.
2. The engine thread pool and the primitives it relies on are TSan-clean on
   native fixtures.
3. The CTSQueue crash is fixed.
4. Workers own bounded work-stealing deques, and waiting threads no longer run
   unrelated work.

## 1. Host frame as an ordered serial graph (R10)

`_Host_RunFrame` keeps its admission part unchanged: time accumulation, tick
count and `setjmp(host_enddemo)`. The rest of the frame, formerly hand-ordered
calls, is emitted as one legacy node per existing phase on a single sequence
lane of a jobsystem `FrameCoordinator` graph. The graph is sealed, validated and
run serially on the host thread by the deterministic executor.

| Piece | Location |
| --- | --- |
| C++11 graph facade (FrameCoordinator + DeterministicExecutor, sealed-graph reuse per frame shape, stop/cancel, input validation) | `public/jobsystem/serial_frame_graph.h`, `jobsystem/serial_frame_graph.cpp` |
| vstdlib exports used by the engine | `public/vstdlib/jobgraph_frame.h`, `vstdlib/jobgraph_frame.cpp` |
| Frame state, carry state, exit codes, `Host_RunFrameGraph` | `engine/host_frame_graph.h` (via `host.h`) |
| Phase bodies, longjmp guard, frame builder (shared with the oracle test) | `engine/host_frame_phases.h` |
| Engine translation unit that owns the graph | `engine/host_frame_graph.cpp` |
| Switch, capture, legacy rollback body | `engine/host.cpp` |

Phases:

- Common head: `CmdExecute`, `FrameSetup`.
- Unthreaded path: `SimSetup`, then per tick `TickBegin`, `TickInput`,
  `TickServer`, `TickClient`, `TickEnd`, then `HLTV`, `TimedemoClient`,
  `ClientInterpolation`.
- Threaded listen server: `ThreadedSetup`, per client tick
  `ThreadedClientTick`, `ThreadedTimedemo`, `ThreadedPrediction`, per server
  tick `ThreadedInputTick`, `ThreadedInputEnd`, `ThreadedServerSubmit`.
- Common tail: `Log`, `Render`+`Sound` or `DynamicModels`, `ClientDLLUpdate`,
  `AsyncServerJoin`, `FrameEnd`.

Each body is the legacy statement block. Guards the legacy code evaluated at
call time (HLTV presence, dedicated server, timedemo/skip, single-player join)
are still evaluated when the phase runs. Only the frame shape (tick counts,
threaded path, render branch) is fixed at build time, and the legacy code
fixed each of those before running any phase that uses it. `g_bThreadedEngine`
is written only by `Host_RunFrame`, before `_Host_RunFrame`.

Error exits: `Host_EndGame` and `Host_AbortServer` leave a frame with `longjmp`.
Unwinding the C++20 executor that way would skip non-trivial destructors, which
is undefined behavior. Each phase therefore runs under a guard that points
`host_enddemo` and `host_abortserver` at itself. A trapped exit cancels the
remaining phases and is completed by `_Host_RunFrame` from its own frame: it
returns for `host_enddemo` and re-issues the `longjmp` for `host_abortserver`,
as the legacy frame did. The guard restores the caller's jump targets either
way; the oracle test checks that byte for byte.

The threaded path's function statics became `g_HostFrameCarry`. That gives
both paths one owner, so switching modes keeps continuity.

Switch: `host_frame_graph` (default `1`). `0` runs the legacy hand-ordered body,
kept verbatim apart from that carry substitution. It is the rollback and
comparison path. **Deletion condition:** remove the legacy body once the
threaded-path live comparison below passes and one further release ships
without a rollback.

### Evidence

**Oracle** (`jobsystem.hostframe`, 4,044 checks; also Waf
`jobsystemhostframetest`):

- The pre-change `_Host_RunFrame` body, extracted mechanically from `f7eb1cf1`
  (`unittests/jobsystemtest/hostframe_legacy_oracle.h`), and the engine kernel
  are compiled against the same recording stand-ins. Each call records its
  arguments plus a snapshot of every host tick/time/simulation global.
- 4,000 seeded multi-frame scenarios produce identical traces.
- Scenario coverage:
  - 1,505 threaded and 2,495 unthreaded scenarios; 992 dedicated.
  - 2,774 zero-tick frames and 8,443 multi-tick frames.
  - HLTV appearing mid-frame, timedemo/skip hacks, single-player and
    multiplayer async joins.
  - 430 `Host_EndGame` and 451 `Host_AbortServer` longjmps injected from
    inside phases.
- The guard restores the caller's jump targets.
- Sensitivity: swapping, dropping or duplicating any of the first six phases is
  detected (18 of 18 mutants).
- The facade contract is checked: order, per-run args, shape-keyed reuse, stop,
  invalid input, empty frame.
- Passes on g++ and clang++, in default and release configurations, and under
  TSan.

**Live Portal captures** (`-hostframetrace`, `tools/quality/host_frame_capture.py`,
8 comparator tests):

- Setup: native Vulkan, headless, `testchmb_a_00`/`testchmb_a_01`,
  `host_framerate` fixed, `cl_clock_correction 0`.
- Each run records every host-level call with the tick and simulation state
  it observed. Two legacy and two graph runs per configuration.
- Final binaries, 12 runs, 18 pairwise comparisons:

  | Configuration | Frames / calls | Result |
  | --- | --- | --- |
  | `testchmb_a_00`, 1 tick per frame (0.015) | 245 / 1,712 | Same-mode pairs exactly identical. Cross-mode pairs identical in call order, arguments and all integer tick/simulation state; `ia` differs by at most 1e-4. |
  | `testchmb_a_01`, alternating 1/2 ticks (0.0225) | 196 / 1,660 | All 6 pairs identical apart from `ia` (at most 1e-4), legacy-vs-legacy included (552 tolerated `ia` differences; legacy1-vs-graph1 had 36). |
  | `testchmb_a_00`, 0/1 ticks (0.0075) | 326 / 1,790 | 3 of 6 pairs exactly identical, including two legacy-vs-graph pairs; the rest differ only in `ia`. The runs fail the harness's unrelated screenshot-detail check at this frame rate. |

- `ia` is the interpolation amount. It carries the wall-clock tick remainder
  from frames that ran before `sv_cheats` lets `host_framerate` apply (and
  before `cl_clock_correction 0`, which removed a frame-0 drift term). Its noise
  is the same with and without the graph.
- The comparator is exact by default; the `ia=0.0001` tolerance is explicit,
  counted and reported.

Overhead is measured, not gated. The serial graph adds about 1.8 to 2.2 µs per
frame for 16 to 31 phases on a loaded 32-thread host, against 0.3 to 0.4 µs
for a direct loop. The cost is per-run `RunResult` state and `std::function`
dispatch; phase lists and sealed graphs are reused.

## 2. Engine pool and primitives TSan-clean (R20)

TSan found three problems at baseline on the existing real-pool fixtures. It
reported 3 warnings for `jobsystemlegacybatchtest` and 5 for
`jobsystembridgetest`:

- `CThread::Start` raced the new thread on the init-success flag. This was a
  real bug: `Start` wrote `true` after `pthread_create` and could overwrite a
  failed `Init()`. Fixed by leaving the flag to the thread.
- `CInterlockedPtr`/`CInterlockedIntT` read their value with plain volatile
  loads, racing their own interlocked updates. This covers the
  `CParallelProcessor` cursor and every queue/idle/job count. Reads now use
  `ThreadAtomicLoad` (acquire), with layout unchanged. `operator=` returns the
  stored value instead of rereading it.
- `CThreadFastMutex` read its owner plainly, and its debug checks read the
  owner-private depth before acquiring. Both are fixed.

Found while fixing those:

- `ThreadInterlockedExchange`/`ThreadInterlockedExchangePointer` used
  `__sync_lock_test_and_set`, which is acquire-only. `CThreadFastMutex::Unlock`
  therefore had no release barrier: a real reordering bug on ARM64/Android.
  They are now `__atomic_exchange_n(SEQ_CST)`, matching `InterlockedExchange`.
- `CWorkerThread::WaitForReply` wrote `m_ReturnVal` on timeout while the worker
  could be replying. Only the worker writes it now.
- `CThread::m_ExitEvent` was auto-reset. A reply wait could consume the exit
  signal, and shutdown then joined forever. The worker-wake change below made
  this reproducible, and it is now manual-reset. The event is re-armed by
  `Start`.

`CJob` status accesses were already interlocked (earlier change). The new
fixtures verify them under TSan.

New native fixture `jobsystemthreadpooltest` (C++11, real tier0/tier1/vstdlib,
2,501 checks):

- Shared-queue exactly-once and accounting.
- Steal-deque placement and stealing.
- Bounded spill: exactly 256 in the deque and 744 in the shared queue for
  1,000 spawns with one worker.
- The wait rule (next section).
- `AbortAll`/`ExecuteAll`/`Stop` with populated deques.
- Recursive fan-out with nested waits.
- `ParallelProcess`/`ParallelLoopProcess`.
- Concurrent status observation.
- CTSQueue MPMC exactly-once and per-producer FIFO.

It builds under the clang TSan Waf profile (`--sanitize thread`).

| Fixture (clang, TSan) | Before | After |
| --- | --- | --- |
| `jobsystemthreadpooltest` | n/a | 2,501 checks, 0 warnings |
| `jobsystemlegacybatchtest` | 20,932 checks, 3 warnings | 20,932 checks, 0 warnings |
| `jobsystembridgetest` | 91 checks, 5 warnings | 91 checks, 0 warnings |
| `jobsystemhostframetest` | n/a | 4,044 checks, 0 warnings |
| legacy `RunTSQueueTests` (driver, 3,000 items) | race reports in the test's start flag only | 0 warnings over the first 59 test lines. The 64-thread single-producer cases stall under TSan (63 spinning poppers on one mutex) and were stopped. Natively the full suite passes 3 of 3 in about 11 s |

The legacy suite's own start flag was a plain `volatile bool`; it is now
interlocked.

## 3. CTSQueue crash (R20)

The lock-free queue had three defects:

- `Push` read the tail without synchronization.
- Its help path could install a recycled node's free-list link as the tail.
- The `pNext` CAS had no ABA tag.

The gcc `unittest_legacy` crashed in 4 of 4 runs. Following the RFC 0006 rule
of a simple synchronized implementation until a lock-free one has evidence,
`CTSQueue` is now a mutex-guarded dummy-node list. It keeps the node and item
API, FIFO order, `Pop`'s caller-owned node semantics and node recycling;
element copies happen under the lock. Consumers only use
`PushItem`/`PopItem`/`Count`.

Evidence:

- gcc `unittest_legacy`: 3 of 3 runs pass all 84 CTSQueue and 84 CTSList lines,
  then stop at the pre-existing R07 `ModuleLoadTelemetry` fixture-path crash
  that `quality/baseline.json` already records for clang. The gcc baseline
  entry was re-attributed.
- The legacy queue suite passes 10 of 10 runs in a standalone driver.
- The fixture above covers MPMC under TSan.

`CTSListBase` (the tagged Treiber stack behind `CTSList`/`CTSPool`) still
reads freed nodes by design and is not changed here.

## 4. Bounded work stealing and the wait rule (R20)

**Topology:** each `CThreadPool` worker owns a `CJobStealDeque`
(`vstdlib/job_steal_deque.h`, capacity 256). Work a worker spawns goes to its
own deque unless it is `JF_SERIAL` or bound to a service thread. The owner
pops newest-first; other workers steal oldest-first after their direct queue,
own deque and the shared queue are empty.

**Overflow:** a full deque spills to the shared injection queue. Nothing is
dropped, blocked on or run early.

**Payload lifetime:** each entry holds one job reference, released after
service, abort or spill.

**Quiescent operations:** `ExecuteToPriority` spills deques into the shared
queue, and `AbortAll`/`Stop` abort them.

**Synchronization:** one short critical section per deque operation. A
Chase-Lev deque needs stress and performance evidence first.

**Wake protocol:** the POSIX worker wait used to poll with 100 ms/10 ms
timeouts. Each worker now has an auto-reset wake event, set by every producer
after publishing (shared, direct, deque, call). Workers re-check for work
after each wake. A 1 s fallback timeout bounds any missed wake to latency and
is not part of the protocol. `Stop`/`SuspendExecution` send without waiting,
wake, then wait for the reply.

**Wait rule (`IThreadPool::YieldWait`):**

- A waiting thread never services unrelated queued work.
- Waiting on events runs nothing.
- Waiting on jobs may run only those jobs, inline, and only when no worker
  has started them and they are eligible for that thread: owned by this pool,
  not `JF_SERIAL`, not bound to a service thread, pool not
  `bExecOnThreadPoolThreadsOnly`.

The old `YieldWait` popped arbitrary shared-queue jobs. The single-player
async-server join already avoided that because it would run queued AI mid-frame;
the multiplayer join (`WaitForFinishAndRelease`), `CJobSet` waits, the
navigation-query wait and the material system's waits now follow the rule too.

**Sensitivity:** the fixture built against the pre-change `jobthread.cpp`
(stats stubbed) fails 11 checks. Unrelated jobs run on the waiting main thread
(event wait, job wait, `CJobSet`), and steal placement and stealing are absent.

Scheduling counters are exported for diagnostics and tests through
`GetThreadPoolSchedulingStats`, a new free function; the `IThreadPool` vtable is
unchanged.

## Benchmarks (2026-09-25)

Shared 32-thread host with other agents active; interleaved A/B rounds; best
median reported. A = unmodified base `504ee284` (separate worktree, same Portal
release configuration), B = this branch.

**Engine pool and CTSQueue** (`unittests/jobsystemtest/threadpoolbench.cpp`,
3 workers, 5 rounds × 15 samples). The bench uses only the long-standing API, so
the same source is built against each tree's headers and libraries. Every
sample is validated (exactly once) before its time counts. Load average rose
from 6 to 37 during the run.

| Workload | A | B | B/A |
| --- | --- | --- | --- |
| Shared-queue fan-out, 4,096 jobs | 4,051 µs | 2,038 µs | 0.50 |
| Worker-spawned children, 8 × 256 | 3,328 µs | 1,452 µs | 0.44 |
| Recursive fan-out with nested waits, 1,365 jobs | 1,524 µs | 739 µs | 0.48 |
| `ParallelProcess`, 8,192 uneven items | 4,173 µs | 1,936 µs | 0.46 |
| Wake latency (AddJob to job start) | 17.1 µs | 22.0 µs | 1.29 |
| CTSQueue 1 producer / 1 consumer, 200k | 44.4 ms | 51.5 ms | 1.16 |
| CTSQueue 4 / 4, 800k | crashes 5 of 5 | 220.7 ms | — |

The serial cost of `ParallelProcess` is identical (about 8,990 µs with 0
workers). With 3 workers A reaches about 2.0 to 2.4 times and B about 4.1 to
4.7 times (4 is ideal with the caller participating). The likely cause is the
old POSIX worker wait, not measured directly. It blocked on the shared-queue
event (100 ms) and then the direct-queue event (10 ms) in turn, so work arriving
during the direct slice waited; the per-worker wake event removes that. The
wake event costs about 5 µs more per isolated wake, and the synchronized queue
costs 16% on the uncontended 1:1 case in exchange for not crashing under
contention.

**In-game frames** (`tools/quality/frame_pacing.py`, portal-frame-pacing-v1 on
`testchmb_a_02`, native Vulkan, `fps_max 1000`, warm pass 3, 3 interleaved
rounds, output under `quality-results/fp/`):

| Comparison | Warm median | p99 |
| --- | --- | --- |
| `host_frame_graph 0` vs `1` (same build B) | medians 10.53 / 10.74 / 13.84 vs 10.94 / 10.81 / 13.02 ms; per-round deltas +0.41, +0.07, −0.82 ms | 16.72 / 16.12 / 20.08 vs 16.11 / 15.86 / 19.74 ms |
| Build A vs build B (graph on, new pool) | 10.80 vs 10.88 ms (B/A 1.008) | 16.21 vs 18.86 ms (1.16) |

Frame cost is dominated by render emit (about 7 ms). Run-to-run variation is
about ±0.8 ms, far above the graph's microbenchmarked 1.8 to 2.2 µs per frame,
so these runs show no measurable in-game difference from either change. In
round 3 of the build A/B, B was an outlier at 13.5 ms with emit also inflated;
the other two rounds are within 0.1 to 0.3 ms. p99 needs more rounds on a quiet
host before any budget is set.

Reproduce: `scratchpad` scripts are not retained. Build `threadpoolbench.cpp`
against each tree (engine flags `-O2 -march=core2 -mfpmath=sse -ffast-math`,
link that tree's `libtier1.a`, `libvstdlib.so`, `libtier0.so`) and alternate runs
of `--filter <workload> --samples 15`. For frames, run
`frame_pacing.py --runtime <portal runtime> --build <A> --ab-build <B> --rounds 3 --out <short dir>`
(the engine command line limit is 512 characters, so keep `--out` short). Pass
`--extra-arg=+host_frame_graph --extra-arg=<0|1>` for the mode comparison.

## Gates run

| Gate | Result |
| --- | --- |
| `conformance.py check` (g++, all headless) | 124 of 124 suites pass |
| `conformance.py check --domain Q-JOBS` (clang++; release) | 11 of 11 on clang++ and 11 of 11 in release; `jobsystem.hostframe` also passes on clang++ release |
| gcc `--tests` job-system programs (14) | all pass (see counts above) |
| `tools/quality/tests/test_host_frame_capture.py` | 8 pass |
| `baseline.py validate`; `test_baseline` | valid; 21 pass |
| `stylelint --changed --base 504ee284` | 0 failures (20 files) |
| `archlint check --all` / `baseline --verify` | fail with 64 new and 1 stale occurrences, all pre-existing Portal 2/matchmaking/Box3D sites, none in changed files |
| `archlint inventory --verify` | 3 pre-existing uninstrumented loader sites (not changed here) |
| archlint unit tests | pass |

## Open (R10/R20 remain partial)

- Threaded listen server (`host_thread_mode 2`) live comparison: not possible
  yet. Portal deadlocks after about 8 frames in either frame mode, and **also
  on the unmodified baseline build** (`504ee284`, killed by the harness
  timeout at 245 s), so the fault predates this work.
  - Stacks (graph mode): the main thread is in `AsyncServerJoin` →
    `ExecuteAndRelease`, blocked on the server job's mutex. The worker running
    the server frame spins in `CThreadSpinRWLock::SpinLockForWrite` on the
    spatial-partition voxel tree (`CVoxelTree::InsertIntoTree`, reached from
    `PhysicsTouchTriggers` → `OnPreQuery`). All other pool workers are idle, so
    a partition read lock appears to be leaked or held across the join.
  - The oracle covers the threaded path's ordering (1,505 threaded scenarios).
- R10 also needs the runner/clock (virtual time) contracts of RFC 0001 rank
  11. This increment delivers only the ordered serial host graph matching
  legacy captures.
- R20 still needs:
  - a bound and backpressure policy for the shared injection queue (steal
    deques are bounded; the shared queue is not);
  - forbidden nested-wait detection;
  - pool capacity and overhead budgets for workloads;
  - a full product run under TSan (the whole engine is not instrumented here).
- Mobile (Android/Apple) profiles were not built or run. The
  `ThreadInterlockedExchange` release fix matters most there.

## Reproduction

```sh
# Oracle and facade (shared runner):
python3 tools/quality/conformance.py check --suite jobsystem.hostframe
# TSan tree and native pool fixtures:
WAFLOCK=.lock-waf-sched-tsan CC=clang CXX=clang++ ./waf configure --tests --use-sdl=0 \
  --use-togl=0 --disable-warns -T debug --sanitize thread -o build-sched-tsan
WAFLOCK=.lock-waf-sched-tsan ./waf build --targets=jobsystemthreadpooltest,jobsystemlegacybatchtest,jobsystembridgetest,jobsystemhostframetest
cd build-sched-tsan && LD_LIBRARY_PATH=$PWD/tier0:$PWD/tier1:$PWD/vstdlib \
  ./unittests/jobsystemtest/jobsystemthreadpooltest
# Live capture (Portal product tree built with --platform-provider=sdl3
# --render-backend=native-vulkan --build-games=portal):
python3 tools/quality/portal_boot.py --runtime <portal runtime> --build <tree> --out <dir> \
  --headless --renderer native-vulkan --engine-arg=-hostframetrace --engine-arg=h.t \
  --startup-command="host_framerate 0.015" --startup-command="cl_clock_correction 0" \
  --startup-command="host_frame_graph 0"   # then 1
python3 tools/quality/host_frame_capture.py legacy.trace graph.trace [--tolerance ia=0.0001]
```
