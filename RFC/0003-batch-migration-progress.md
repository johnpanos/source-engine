# RFC 0003: bounded production batch migrations

Updated: 2026-09-22 (cohort defaults on; query-cache and portal-carving oracles). Portfolio: R20 / R21 / R30, all **partial**.

This increment moves four existing compute cohorts onto opt-in dependency-aware
job graphs. It preserves their synchronous gather/compute/commit boundaries and
uses the already composed engine pool. It does not add another pool, advance
world simulation, enable snapshot-send concurrency, or close the RFC's gameplay
equivalence and performance gates. Legacy mode remains the default.

## Delivered callers and ownership

| Caller / mode ConVar | Scope and preserved boundary |
| --- | --- |
| `CParticleMgr::UpdateNewEffects` / `r_particle_job_graph` | Entity control-point gathering, retirement and dirty-partition update stay on the caller; each particle effect is processed once; `DetectChanges` and `EndSimulateParticles` follow the join. A scoped binding captures the simulation time step. |
| `C_BaseAnimating::ThreadedBoneSetup` / `cl_bone_job_graph` | The previous-frame list remains borrowed until completion; each participating thread retains its own model-cache begin/end scope; parent filtering, global setup guard and list retirement keep their order. A scoped binding captures bone time. |
| `CViewRender` renderable bone batches / `r_renderable_job_graph` | NPC and non-NPC batches retain their separate ordered barriers, parent filtering and captured time. |
| `PackEntities_Normal` / `sv_packentities_job_graph` | Visibility gathering remains serial; snapshot/edict/send-table lifetimes extend through join; existing packing/baseline synchronization remains in force; change-info invalidation follows completion. Snapshot sending is not moved. |

Every mode ConVar accepts `0` (legacy), `1` (deterministic serial graph), or `2`
(pooled graph). **As of 2026-09-22 (user decision) all four default to `2`, and
the legacy gates `cl_threaded_bone_setup` and `r_threaded_renderables` default
to `1`**, so every cohort runs its pooled graph path with no configuration
(`r_threaded_particles` and `sv_parallel_packentities` already defaulted to `1`).
Before this change the bone and renderable graph paths were unreachable by
default because their legacy gates were `0`. Defaulting on does **not** close the
open gates below: engine-pool TSan is still not clean, and gameplay captures and
frame budgets are still missing. Roll back at the next batch boundary by setting
the corresponding graph ConVar to zero, or the legacy gate to zero to restore the
original serial behaviour. No graph callback or borrowed payload survives return.

## Execution and compatibility boundary

`public/jobsystem/parallel_batch.h` owns the scoped batch contract;
`public/vstdlib/jobgraph_parallel.h` adapts the existing free/member callbacks
without exposing C++20 graph types or changing legacy vtables. The descriptor
borrows a name, context and callbacks until return. Validation/sealing happens
before callbacks start; invalid requests execute no work and callers report a
fatal construction error rather than replaying partially mutated state.

`jobsystem/parallel_batch.cpp` builds a bounded set of compute nodes and an
explicit completion dependency. Node count scales with supplied execution
capacity, not entity/particle count. Independent item claims are relaxed atomic
reservations; backend queue publication and completion synchronization publish
payloads. Single-participant execution keeps the graph but avoids atomics inside
the item loop. Begin/end hooks bracket a participating runner on the same physical
thread. Nested batches from begin/process/end execute inline to prevent pool
starvation. Dynamic scopes remain the separate API for genuinely asynchronous
child graphs; this adapter adds no inter-frame overlap.

The `vstdlib` backend borrows `g_pThreadPool` through the legacy adapter or accepts
an explicitly supplied pool. Each dispatch owns bounded `QueueCall` runners and
the caller processes only that dispatch's work. It joins/aborts only its own
jobs, with an explicit job-mutex acquire before release, and never calls
`YieldWait` or drains unrelated work. Unlike the previous `CParallelProcessor`
bridge, it can use one available worker and needs no per-index array. Concurrent
and nested calls do not share mutable callback storage. The earlier private-pool
factory remains available for isolated tests; migrated game callers do not use it.

`PooledExecutor` additionally routes Compute/Sequence work through the backend,
MainThread/BlockingIO work through the caller pump, detects unserviced lanes and
their dependents, and rechecks cancellation before each callback. Empty compute
join nodes do not dispatch worker batches. These are affinity/contract repairs,
not a new dedicated blocking-IO backend.

Waf now includes `jobsystem` in game and dedicated compositions and links it
privately into `vstdlib`. The module's final standard flag is C++20; the facade,
bridge, engine/client and the real-pool consumer fixture remain C++11. Existing
ABI0/toolchain/FP flags are preserved. The verified profile is Linux x86_64 GCC
16.2.1/libstdc++, with separate Clang 22.1.8 sanitizer runs. Apple/Android and
other compatibility-profile acceptance remain unverified.

## Verification and evidence

The authoritative headless registrations are in
`quality/conformance.manifest.json`; Waf owns real-library test linkage in
`unittests/jobsystemtest/wscript`. Local collected build commands, source hashes,
benchmark samples, static-check failures and sanitizer logs are indexed by
`quality-results/job-batch-validation-20260922/evidence.json`. Reproduction
commands used here:

```sh
python3 tools/quality/conformance.py check --domain Q-JOBS
WAFLOCK=.lock-waf-composition python3 waf build \
  --targets=jobsystemtest,jobsystemframetest,jobsystemdynamictest,jobsystemqjobstest,jobsystempooledtest,jobsystembatchtest,jobsystembridgetest,jobsystemlegacybatchtest -j8
LD_LIBRARY_PATH=build-composition/tier0:build-composition/vstdlib \
  build-composition/unittests/jobsystemtest/jobsystemlegacybatchtest
LD_LIBRARY_PATH=build-composition/tier0:build-composition/vstdlib \
  build-composition/unittests/jobsystemtest/jobsystemlegacybatchtest --benchmark
WAFLOCK=.lock-waf-portal-vulkan python3 waf build --targets=engine,client,server -j8
```

Existing profile configurations were retained. When new source targets required
configuration, the same saved configure arguments were replayed under their own
Waf lock; the default build profile was not replaced.

- Shared Q-JOBS gate: **5/5 suites passed**, evidence
  `quality-results/conformance.20260922T091616Z.json` (source/dirty identity,
  toolchain, exact commands and per-suite output).
- All eight Waf executables ran: scheduler 274, frame 84, dynamic scope 140,
  Q-JOBS 7,719, pooled executor 174, batch 1,058, existing real bridge 91 and new
  C++11 real batch 20,932 checks; no failures. The benchmark invocation performs
  additional output comparisons: 99,592 checks, no failures.
- Batch coverage includes an independent item-value/count oracle, serial order,
  invalid/empty inputs, capacity limits, hook ownership, delayed publication,
  saturated nested work, and deliberately corrupted/omitted/duplicated outputs.
  The real-pool fixture additionally covers concurrent submissions, zero/one/
  three workers, observed one-worker execution, a suspended pool with unrelated
  queued work, immediate payload retirement and no unrelated queue helping.
- Pooled tests compare terminal outcomes against other executors; deliberately
  dispatching affine jobs to workers or dropping the start-time cancellation
  check produces failures. Both new portable suites pass separate ASan/UBSan and
  TSan runs. Instrumented bridge/core/`jobthread.cpp` plus the C++11 fixture pass
  ASan/UBSan (20,932 full and 14,623 `--graphs-only` checks); **engine-pool
  TSan is not clean**, as detailed below.
- The real engine, Portal client and Portal server compile and link with the
  mixed-dialect boundary. A transient engine link failure during concurrent
  audio-provider edits was retained separately; the final product build passes.
- Architecture fixtures (51), style fixtures (38) and quality-runner/tool fixtures
  (134) passed at baseline. Final full archlint has **zero new violations** but
  fails six stale shader/video loader entries from concurrent provider work;
  inventory additionally reports an uninstrumented video loader site. Those
  changes and ratchets were not modified here. Task-owned C++ regions pass the
  pinned formatter against the starting `f5fb0991` revision; the shared tree
  also contains unrelated style changes.

Native experiment:
`quality-results/job-batch-native-20260922T090958Z/summary.json` records **6/6
passing runs**, legacy/serial/pooled modes on `testchmb_a_00` and
`testchmb_a_01`. Each verifies active player/map, nonblank scene capture, clean
shutdown, actual SDL3/Wayland/Vulkan, and console receipts for all four graph
settings, four legacy threading settings and `cl_localnetworkbackdoor=0`.
The original content/config tree was unchanged. The evidence's source identity
is `c89431f753619e2e321f936940cbde8fcd7efb22` plus dirty digest
`e00235656227725411ad53a3981c6addc0d71ce74546a4d62bd9b0ef09d598d6`.
Its local `run_native.py` records exact staging, config and runner invocations;
each product run retains executable/dependency hashes, logs and screenshots.

The native results do **not** prove nonzero execution of every kernel, exact
particle/bone/network state equivalence, frame latency, arbitrary game content,
parented-animation/model-eviction behavior, or a clean engine-wide race gate.
Screenshots and startup duration are not substituted for those oracles.

## Query-cache maintenance and portal placement carving (2026-09-22)

Two further cohorts now share one kernel between production and conformance,
and are proven output-equivalent to their original serial code. Both stay
**default legacy (`0`)** because measurements show no benefit (below).

| Cohort / mode ConVar | Kernel (single authority) | Production caller |
| --- | --- | --- |
| Query-cache maintenance / `sv_querycache_job_graph` | `game/shared/querycache_maintenance.h`: gather, worker-safe classification, ordered commit and victim publication | `UpdateQueryCache` (`game/shared/querycache.cpp`) |
| Portal placement carving / `portal_carve_job_graph` (`FCVAR_REPLICATED`) | `game/shared/portal/portal_carve.h`: `ClipBatch`/`ClipInOrder`/`ClipGroupsInOrder`, wall bounds and the whole "(Holy) Wall" carve; owns the wall geometry constants | `CPortalSimulator::CreatePolyhedrons` (world brushes, static props, wall) |

Carving boundary: each clip reads one immutable polyhedron and returns a new heap
polyhedron. Results fill indexed slots and are appended in input order on the
owner thread. The hole-interaction filter uses the shared temporary polyhedron,
so it stays serial. Cache lookups, `ConvertPolyhedronsToCollideable` (IVP) and
all physics-object creation are unchanged and serial. Query-cache workers only
classify; entity/trace callbacks and all list and counter mutation commit in
chain order on the owner.

Equivalence oracles (reference = the original code, transcribed; candidates =
the production kernel in legacy-loop, serial-graph and pooled-graph modes):

- `jobsystem.querycache` (manifest, Q-JOBS). Two cache copies are driven by one
  generated workload of tick-quantized time, hitches, entity deletion and
  revival, hot/cold key pools and victim exhaustion. After every frame it
  compares all entry fields bit-exact, hash-chain and victim order and links,
  every counter, and the ordered refresh-callback log. Configurations: the
  production geometry (1024 entries / 2048 chains / 8 splits), a small cache, an
  uneven chain count (100 chains) and 3 splits. Candidates run with 0/1/3/8
  real threads. Result: **37,440 frames, 146 checks, 0 failures** under g++ 16.2.1
  and clang++ 22.1.8. Coverage is asserted: refreshes, invalidations, wasted
  expiry, victim publication and successful speculation. Writing the kernel
  exposed that the migrated partition formula (`i*n/s`) differed from the
  original (`n/s`, remainder to the last split) for uneven chain counts; the
  kernel uses the original formula.
- `jobsystem.querycache.sensitivity`: 6/6 plausible defects are rejected
  (`>=` becoming `>`, a dropped item, uncounted wasted speculation, reversed
  victim publication, reversed commit order, stale refresh time).
- `jobsystemportalcarvetest` (Waf, `unittests/jobsystemtest`, C++11, real mathlib
  and real vstdlib pools with 0/1/3 workers). The reference is the original
  helpers and loops plus the "(Holy) Wall" block, extracted mechanically from
  `3d3e5e68` with only identifier substitutions. It runs 160 generated
  placements: axis-aligned and angled walls, floors and ceilings on tiled walls
  with seams, recesses, room shells, rotated clutter, wedges, static-prop pieces,
  missing brushes and invalid ids. Every output polyhedron is compared bit-exact
  (vertices, lines, indices, polygon normals) and in order, along with prop group
  counts. Result: **11,982 polyhedra, 1,611 checks, 0 failures**. All four
  deliberate defects are detected: swapped wall pieces, a dropped result, clip
  epsilon drift and a shifted prop group.

```sh
python3 tools/quality/conformance.py check --suite jobsystem.querycache --suite jobsystem.querycache.sensitivity
WAFLOCK=.lock-waf-composition python3 waf build --targets=jobsystemportalcarvetest
LD_LIBRARY_PATH=build-composition/tier0:build-composition/vstdlib \
  build-composition/unittests/jobsystemtest/jobsystemportalcarvetest --benchmark
```

Native Portal (SDL3/Wayland/Vulkan, `build-portal-vulkan`, private staged
runtimes):

- `quality-results/job-defaults-native-20260922T232350Z`: 4/4 runs pass on
  `testchmb_a_00` and `testchmb_a_01`, in `defaults` and `new-pooled` modes.
  Console receipts with **no ConVar overrides** confirm the new defaults.
- `quality-results/job-defaults-native-placement-20260922T233209Z`: 4/4 pass with
  real portal-gun placements (`sv_portal_placement_never_fail 1`) and a
  `developer 2` carve diagnostic. On `testchmb_a_01`, legacy and pooled carve the
  same two placements with identical counts (world 13/props 2/tube 4/wall 15, and
  8/2/4/13). `testchmb_a_00` produced no linked placement in either mode.
  An earlier run of this experiment found that `portal_carve_job_graph`, being
  defined in both client and server, was refused linkage ("Parent cvar in
  server.dll not allowed"). It is now `FCVAR_REPLICATED`.

Measurements (why both remain default legacy): the whole carve costs **0.05–0.12 ms
per placement in-game**. Pooled was slower on the same placements (0.106 vs
0.061 ms, 0.078 vs 0.051 ms), and across the 160 synthetic placements
(25.0 vs 21.3 ms total). Query-cache classification is a few compares per entry,
the tiny-work case where pooled dispatch regresses in the batch microbenchmark.
The native maps issued 0 cache queries, so maintenance is not exercised there.
Placement cost is more likely in the serial IVP collideable conversion, which is
not migrated. Revisit the defaults with gameplay measurements.

## Measurements and open gates

The real-pool fixture provides an exploratory interleaved comparison of legacy,
serial-graph and pooled-graph dispatch: 10 warmups, 101 measured samples, 0/1/3
workers, counts 1/8/128/2048 and cheap/heavier independent integer kernels.
These are scheduler microbenchmarks on a shared development host, without
controlled CPU frequency or a preapproved gameplay budget. No rollout gate or
game FPS claim follows from them. Representative same-run medians, microseconds:

| Workers / items / iterations per item | Legacy | Serial graph | Pooled graph |
| --- | ---: | ---: | ---: |
| 0 / 2048 / 0 | 10.199 | 3.447 | 3.487 |
| 1 / 128 / 2048 | 220.133 | 220.625 | 115.747 |
| 1 / 2048 / 0 | 9.478 | 2.886 | 38.783 |
| 3 / 8 / 0 | 2.845 | 0.531 | 4.819 |
| 3 / 2048 / 2048 | 1781.398 | 3528.021 | 1793.881 |

The one-worker case can benefit when compute dominates dispatch; tiny work can
regress, and three-worker heavy work is approximately unchanged in this sample.
Per-call graph allocation, contention and scheduling/granularity decisions still
need workload-specific budgets and measured frame/p95/p99 latency before default
selection. Original timings and the earlier implementation's measurements are
retained in the validation evidence; neither was treated as an acceptance gate.

Instrumenting the real pool exposes unsynchronized legacy `CJob` status reads,
queue counts, list/queue heads and the old parallel cursor. Some runtime/library
portions remain uninstrumented. The reports are retained without suppression;
ASan success and portable-executor TSan success cannot certify this engine pool.
Classify and repair those boundaries with native race fixtures before closing
R20 or enabling any migrated cohort by default.

Deferred consumers have concrete reasons:

- Query-cache maintenance: the hazard (shared counters, victim-list insertion
  and gameplay callbacks on workers) is resolved. Workers only classify; the
  owner commits in order. The equivalence oracle above covers this, but the
  default stays legacy until a workload shows a benefit.
- Leaf/shadow parallel branches are hardcoded disabled. This work does not enable
  them or pretend that editing an unreachable dispatch establishes performance.
- Snapshot sending remains disabled for the documented shared snapshot-manager
  race. Async navigation, host-overlap and save/network queues need scoped
  lifetimes and explicit continuations; they are not synchronous batch renames.
- Audio/native callback threads and blocking uploads are not compute work and
  must retain their required executor/affinity contracts.

Retire each cohort's legacy branch only after its captured-state comparison,
rollback, lifetime/race and workload-specific performance gates pass. Until then,
the graph modes are deliberate experimental paths with a working product consumer.
