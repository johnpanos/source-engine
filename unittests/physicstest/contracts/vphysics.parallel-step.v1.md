# Contract: `vphysics.parallel-step.v1`

Module family: VPhysics providers. Claimed by `vphysics_box3d`; IVP
(`vphysics`) does not claim it.
Interfaces: `IPhysicsParallelStep` (`VPhysicsParallelStep001`,
`public/vphysics/parallel_step.h`) and `IPhysicsStepProfile`
(`VPhysicsStepProfile001`, `public/vphysics/step_profile.h`), both reached
through `IPhysics::QueryInterface`.
Contract suite: `vphysics_conformance --bench contract`
(`unittests/physicstest/test_vphysics_bench.cpp`)
Gate: `parallel-step` in `quality/budgets/physics-v1.json`, run by
`tools/quality/physics_bench.py` (evidence `physics-bench-evidence/v1`)
Roadmap: RFC 0013 P1–P3 (R67)

This is an opt-in capability beside [`vphysics.provider.v1`](vphysics.provider.v1.md).
Claiming it never changes a provider's parity obligations. A provider that
does not claim it returns NULL from `QueryInterface`, and the suite reports
`UNSUPPORTED VPhysicsParallelStep001` with exit status 3. The runner does not
treat that as a pass for a claiming provider.

## Clauses

| Check | Clause |
| --- | --- |
| `parallel.max-workers` | `GetMaxWorkerCount()` is at least 1 |
| `parallel.host-pool` (boot) | The host started a pool large enough for a parallel environment |
| `parallel.requires-pool`, `parallel.rejects-over-pool` | More than one worker needs a pool, and at most its threads plus the caller |
| `parallel.census-detects-thread` | Negative control: the thread census sees one extra thread |
| `parallel.no-provider-threads` | Stepping on several workers adds no threads to the process |
| `parallel.nested-simulate`, `parallel.nested-stats` | `Simulate` inside a pool job, with one other thread free, completes with the serial digest, and the pool records no forbidden nested waits or starvation |
| `parallel.pool-stopped` | A pool stopped mid-run leaves the environment working, with the serial digest |
| `parallel.pool-stats` | No forbidden nested waits or starvation after a parallel pile |
| `parallel.rejects-zero`, `parallel.rejects-over-max` | Out-of-range worker counts return NULL and create no environment |
| `parallel.creates` | Valid requests create environments that `IPhysics` lists |
| `parallel.count-created`, `parallel.count-plain` | `GetWorkerCount` reports the created count, and 1 for `IPhysics::CreateEnvironment` |
| `parallel.count-foreign`, `parallel.destroy` | NULL, foreign and destroyed pointers report 0; `IPhysics::DestroyEnvironment` removes the environment |
| `parallel.probe-detects-overlap` | Negative control: the concurrency probe sees a forced two-thread overlap |
| `parallel.solver-serialized` | The game's `IPhysicsCollisionSolver` is never entered concurrently while a pile steps on several workers |
| `parallel.events-on-caller` | Collision events arrive only on the thread that called `Simulate` |
| `parallel.worker-invariant` | When `IsWorkerCountInvariant()` is true, the plain environment, 1 worker and N workers give bitwise-identical object state |
| `profile.rejects-foreign` | `GetLastSimulate` rejects NULL and foreign environments and leaves the output untouched |
| `profile.zero-before-first` | Every field is zero before the first `Simulate` |
| `profile.consistent` | One step per tick; times are non-negative and solver plus adapter time fits in the call; off-caller calls do not exceed calls; the worker count matches |
| `profile.solver-counts` | The cumulative game-solver counts equal the probe solver's own counts |
| `profile.read-harmless` | Reading the profile every tick leaves the digest unchanged |

The whole parity suite also runs at N workers: `vphysics_conformance
--suite-workers N`, or `physics_conformance.py --candidate-workers N` for
candidates only. Every suite environment then comes from this capability on a
host pool.

The solver may run on a pool thread, one call at a time, while the calling
thread is blocked in `Simulate`. RFC 0013 P3 accepts this for the game's
solvers while `tools/quality/physics_filter_audit.py` finds no forbidden
construct in any filter they can reach.

## Sensitivity

`physics_bench.py --sensitivity` injects these host faults. Each must fail
`parallel.contract` (or `parallel.invariant-pile-1024`):

- `parallel-accepts-invalid`
- `parallel-count-lies`
- `parallel-solver-concurrent`
- `parallel-nondeterministic`
- `parallel-private-threads`
- `parallel-nested-timeout`
- `profile-lies`
