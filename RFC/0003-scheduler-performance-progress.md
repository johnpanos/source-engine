# RFC 0003 progress: scheduler microbenchmarks and contract-preserving optimization

Updated: 2026-09-23
Baseline source: `8c2c4268` (job-system sources unchanged from that revision).
Roadmap rows: R20 (bounded worker contracts, measured overhead) and R10 (serial
graph). This is a bounded performance slice. It closes no RFC 0003 gate, sets no
frame budget and makes no gameplay/FPS claim.

## Scope

At the user's direction, this slice added microbenchmarks for every job-system
entry point and optimized the implementation **without changing the external
contract**. The following are unchanged:
- public signatures and observable semantics;
- every job's terminal state, `executed`/`stalled` accounting and trace lifecycle;
- lane affinity, and serial ascending batch order;
- `SealedGraph` topological order, prerequisite/dependent order and the reported
  seal error pair;
- the batch hook and participant bounds.

Header changes are private members only: `ParallelExecutor` gains an
out-of-line destructor and explicit copy operations (it stays copyable), and
`DynamicScope` gains a second condition variable.

## What changed

| Component | Change | Contract argument |
| --- | --- | --- |
| `JobGraphBuilder::Seal` | Counting-sort edge dedup, CSR adjacency, one scratch arena, and a sorted-roots + min-heap Kahn order. Replaces O(E²) dedup, an O(N²) scan and an O(N³) reachability matrix. Resource conflicts are checked per version group with an exact chain test: distinct writers must form a reachability chain, and every other accessor must be ordered between its neighboring writers. The pairwise scan runs only in a failing group, to report the same first pair. | Differential oracle against a naive reference model (below). A rejected builder keeps its declared contents (appended sequence edges are rolled back). |
| `ParallelExecutor` | Once an executor is reused, its workers persist and park between runs. The first run, and any concurrent or nested run, uses transient workers as before. Each role (compute, blocking, main) has its own condition variable; completions wake only as many sleepers as there is new work, instead of `notify_all` on every job. Inline (0-worker) mode runs without the lock. | Same queues, LIFO pop order, trace calls under the lock, stall and cancel decisions. Each queued job either wakes a sleeper or is found by a running servicer, which rechecks under the lock before sleeping. |
| `PooledExecutor` | Per-wave lists share one allocation. Admitted jobs are pre-marked Succeeded, so workers write their state slot only on failure or cancellation (no false sharing on neighboring slots). | Slots are read only after the wave's join barrier. |
| `ExecuteParallelBatch` | Guided self-scheduling claims contiguous index ranges (a share of the remainder per participant, shrinking to single items), replacing one atomic per item. Sealed batch graphs are cached per thread by (name, participants); jobs capture nothing and read per-call state from `RunOptions::frame.user`. | Each index runs once; hooks surround a participant's items on one thread; empty participants call no hooks. Serial still uses the deterministic graph executor in ascending order. Cache entries own a copy of the name, are never evicted (safe under nesting) and are capped at 32 per thread (then sealed per call). |
| `DynamicScope` | A separate owner condition variable, targeted worker wakeups, and each child's function moved out instead of copied (children run once). | Same readiness, cancellation and drain semantics. |

A `yield` spin before blocking was tried and rejected: under host load it turned
94 µs into about 2 ms, and it conflicts with mobile power budgets.

## Oracles added

- `unittests/jobsystemtest/jobsystembench.cpp` (`jobsystem.bench.smoke`,
  Waf `jobsystembench`): 196 benchmarks. Every sample is validated (terminal
  counts, per-item outputs, hook balance), so a faster scheduler that drops work
  fails. The default is a validated smoke run; `--full` gives numbers and
  `--json` writes `jobsystem-bench/v1`.
- `sealequivalencetest.cpp` (`jobsystem.sealequivalence`): an independent naive
  reference `Seal` compared on 4,000 seeded graphs plus large chain/fan/conflict
  shapes. It compares success/failure, error code and job pair, topological
  order, and exact prerequisite/dependent order, including resealing after a
  rejection. The generator reaches every error code, and the comparator has
  negative controls. Mutations of the new implementation (heap order, dedup
  order, conflict chain, rollback) each produce failures. The pre-optimization
  builder passes the same suite.
- `executorstresstest.cpp` (`jobsystem.executorstress`): random mixed-lane graphs
  with failures, Terminal edges and cancellation. It covers:
  - long-lived 0/1/3/4-worker `ParallelExecutor`s and pooled executors;
  - one executor shared by four threads, and a nested `Execute` from a job;
  - copies and assignment;
  - stall and cancellation across pool reuse;
  - `DynamicScope` with children spawning children concurrently, checked by a
    replayed-outcome oracle;
  - batch-graph cache re-entrancy, over-cap names and reused name buffers.

  Checks: terminal states, `executed`, exactly-once execution and plain-read
  publication digests, all against `DeterministicExecutor`. The
  pre-optimization executors also pass.

TSan found one real race during development: collecting a pooled run after
releasing the pool. The functional checks missed it. It was fixed by collecting
before release.

## Results (x86_64, 32 hardware threads, shared host)

Method: 3 interleaved rounds of baseline and new binaries (`--full --samples 15`,
at least 2 ms per sample), keeping each benchmark's best median. The host was
shared with other agents (load 3–9), so single cases vary about ±10% between
runs. Differences below that are noise.

| Benchmark (items / body steps) | Baseline | New | Speedup |
| --- | ---: | ---: | ---: |
| `seal.wide/2048` | 1225 µs | 113 µs | 10.8× |
| `seal.layered.resources/2048` | 11255 µs | 268 µs | 42× |
| `exec.parallel.w4.chain/2048/0` | 571 µs | 44.7 µs | 12.8× |
| `exec.parallel.w4.wide/16/0` (reused executor) | 40.5 µs | 4.0 µs | 10.1× |
| `exec.parallel.w4.layered/2048/256` | 1439 µs | 790 µs | 1.8× |
| `exec.parallel.w0.wide/2048/0` (inline) | 29.4 µs | 16.5 µs | 1.8× |
| `exec.parallel.w4.lanes/256` | 128 µs | 34.1 µs | 3.8× |
| `exec.pooled.w4.chain/2048/0` | 38.7 µs | 21.2 µs | 1.8× |
| `batch.serial.w0/1/0` | 0.15 µs | 0.03 µs | 4.3× |
| `batch.parallel.w4/2048/0` | 68.8 µs | 7.3 µs | 9.4× |
| `batch.parallel.w4/2048/256` | 523 µs | 494 µs | 1.06× |
| `dynamic.w4.continuations/2048` | 1218 µs | 639 µs | 1.9× |

Family geometric means:
- seal: 4.2–18.8×
- `ParallelExecutor`: 0 workers 1.31×, 1 worker 2.23×, 4 workers 3.48×
- `PooledExecutor`: 1.12–1.29× (dominated by the benchmark backend's wake latency)
- batch: 1.6–1.7×
- `DynamicScope`: 1.21–1.38×
- `DeterministicExecutor` and `ExternalCompletion`: unchanged (0.97–1.05×)

Regressions and limits:
- A fresh `ParallelExecutor` used for exactly one run behaves as before, but
  thread creation dominates it. Interleaved reruns put it at parity to about 5%
  slower with 4 workers; one A/B round showed 0.78–0.84×.
- A few pooled/dynamic cases read 0.82–0.95× in one round and at parity on
  rerun.

Real engine pool (`jobsystemlegacybatchtest --benchmark`, vstdlib `CThreadPool`,
same run; legacy `CParallelProcessor` is the in-run control, and the earlier
figures are from [batch-migration measurements](0003-batch-migration-progress.md#measurements-and-open-gates)):

| Workers / items / steps | Legacy | Pooled graph (before) | Pooled graph (now) |
| --- | ---: | ---: | ---: |
| 0 / 1 / 0 | 0.05 µs | — | 0.08 µs |
| 1 / 2048 / 0 | 9.3 µs | 38.8 µs | 4.4 µs |
| 3 / 8 / 0 | 0.69 µs | 4.8 µs | 0.74 µs |
| 3 / 2048 / 0 | 61.6 µs | — | 6.8 µs |
| 3 / 2048 / 2048 | 1759 µs | 1794 µs | 1748 µs |

## Verification

- Q-JOBS gate: **10/10** with g++ 16.2.1 and clang++ 22.1.8
  (`quality-results/conformance.20260923T001256Z.json`,
  `…T001321Z.json`). Runner self-tests: 225 passed.
- All nine standalone programs ran plain, under clang TSan and under
  ASan/UBSan: zero sanitizer reports. The stress test was TSan-clean 15× and
  plain-clean 100×.
- Waf composition tree: all 11 job-system executables pass, including the real
  vstdlib bridge (91 checks) and the C++11 legacy batch fixture (20,932 checks).
  This tree predated the `mapcontainer` module, so it was reconfigured by
  replaying its recorded argv under its own lock.
- Style: job-system files are clean against `8c2c4268`. Archlint: no job-system
  findings; its 7 new / 3 stale failures come from concurrent vphysics_box3d and
  shaderapivulkan work.
- Evidence (git-ignored): `quality-results/jobsystem-bench-20260923/`, containing
  the A/B JSON, summary, real-pool output, scripts and identity.

## Reproduction

```sh
python3 tools/quality/conformance.py check --domain Q-JOBS [--cxx clang++]
g++ -std=c++20 -O2 -pthread -I public -o jobsystembench \
    jobsystem/*.cpp unittests/jobsystemtest/jobsystembench.cpp
./jobsystembench --full --json new.json          # baseline: build from 8c2c4268 sources
WAFLOCK=.lock-waf-composition python3 waf build --targets=jobsystembench,jobsystemstresstest,jobsystemsealeqtest,jobsystemlegacybatchtest -j16
LD_LIBRARY_PATH=build-composition/tier0:build-composition/vstdlib \
    build-composition/unittests/jobsystemtest/jobsystemlegacybatchtest --benchmark
```

## Still open

Not addressed: engine-pool TSan, frame and p95/p99 latency budgets,
low-core/mobile profiles, and non-x86_64 hardware. Microbenchmarks are not a
default-selection gate. Work-stealing or lock-free queues were deliberately not
introduced (RFC 0003 prefers synchronized queues until such evidence exists).
