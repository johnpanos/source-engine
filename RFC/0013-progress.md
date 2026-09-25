# RFC 0013 progress: Opt-in Box3D Physics Capabilities

Roadmap row: R67 (`active` for P0–P1). Design: [RFC 0013](0013-opt-in-physics-capabilities.md).

## Goal and scope (2026-09-24)

Build an opt-in way to use Box3D behavior that IVP cannot provide, and a
benchmark gate that decides when each capability may be turned on. Work is
isolated in the `box3d-optin` worktree (`../source-engine-box3d-optin`,
branch `box3d-optin`) so in-flight work in the shared tree is untouched.

1. RFC 0013: each Box3D-only capability is a small versioned interface reached
   through `IPhysics::QueryInterface`. The VPhysics ABI and the IVP-parity
   contract do not change, and IVP does not offer the capabilities.
2. P1: `vphysics.parallel-step.v1` in the Box3D provider. The worker count is
   fixed at environment creation, the game's collision solver is serialized,
   and IVP-parity conformance must still pass.
3. P0: the benchmark gate. It is a `--bench` mode of the existing
   `vphysics_conformance` host, driven by `tools/quality/physics_bench.py`:
   - IVP and Box3D on identical inputs in a debris pile, a pyramid stack, a
     ragdoll pile, and fast projectiles against thin walls and panes, at 1 and
     N workers;
   - step time (p50/p95/p99), memory, stack collapse, tunneling, joint error
     and a bitwise state digest for determinism;
   - limits in `quality/budgets/physics-v1.json` from interleaved
     measurements, and injected faults that prove the gate fails when it
     should.

Done means the RFC is written, the capability passes its contract, the gate
passes with recorded evidence, every negative control is detected, and this
record and the roadmap say what passed and what is unverified.

## Delivered

| Item | Where |
| --- | --- |
| Capability interface | [`public/vphysics/parallel_step.h`](../public/vphysics/parallel_step.h) |
| Box3D implementation | `vphysics_box3d/main.cpp` (`CPhysicsParallelStepBox3D`), `vphysics_box3d/physics_environment.{h,cpp}` (worker count at construction, solver mutex) |
| Scenes and contract clauses | [`unittests/physicstest/test_vphysics_bench.cpp`](../unittests/physicstest/test_vphysics_bench.cpp); `--bench` mode in `test_vphysics_conformance.cpp` |
| Gate runner | [`tools/quality/physics_bench.py`](../tools/quality/physics_bench.py), 29 unit tests in [`tests/test_physics_bench.py`](../tools/quality/tests/test_physics_bench.py) |
| Declaration and limits | [`quality/budgets/physics-v1.json`](../quality/budgets/physics-v1.json) |
| Contract record | [`vphysics.parallel-step.v1`](../unittests/physicstest/contracts/vphysics.parallel-step.v1.md) |

Nothing in the engine or game calls the capability.

## Evidence

Runner: AMD Ryzen AI Max+ PRO 395 (16 cores / 32 threads), Linux 7.2, g++
16.2.1, release native tree configured in the worktree. Box3D at the pinned
`9e5a4cd` with no local changes. Source: `3ef969ad` plus this change.

### Parallel-step contract

`vphysics_conformance --bench contract` against Box3D: 34 checks pass,
including:

- invalid counts rejected with no environment created;
- counts reported for created, plain, foreign and destroyed environments;
- the solver never entered concurrently (with a probe that is shown to detect
  a forced overlap);
- events only on the calling thread;
- bitwise-identical state for the plain environment, 1 worker and 4 workers.

IVP reports `UNSUPPORTED VPhysicsParallelStep001` (exit 3), as designed.

Observed: with 4 workers on a 512-cube pile, 3.5k of 5.9k game-solver calls
ran on worker threads, serialized by the provider's mutex.

### ThreadSanitizer

A clang `--sanitize=thread` build (`build-tsan` in the worktree, private Waf
lock) ran these with no reports:

- the contract;
- pile-1024 at 8 workers;
- ragdolls-128 at 4 workers;
- stack-20 at 4 workers.

Box3D and provider objects are instrumented (`__tsan` calls in
`contact_solver.c.o` and `physics_environment.cpp.o`). A deliberate two-thread
race on the same host is reported through the same `log_path` setting, so a
clean run is meaningful.

### Static checks

- `stylelint.py --changed`: 0 failures in 7 C++ files. Only changed lines
  were formatted, with the pinned clang-format 22.1.8.
- `archlint.py check --all`: the same 64 new and 1 stale occurrences as the
  shared tree. This change only shifts the line numbers of two existing
  `vphysics_box3d` ARCH105 entries and adds no loader site; the benchmark reuses
  the conformance host's loader.
- `roadmap.py check`: the 2 recorded errors (R15, R16) and a note that R67 is
  active while R19 is `planned`. RFC 0004 progress shows the R19 slice working,
  but the row was not advanced.
- `baseline.py validate`: valid.

### IVP-parity conformance with the change

`physics_conformance.py` (worktree build): IVP and Box3D each pass 602 of 602
checks. The runner still fails on 1 of 12,416 observations,
`dynamics.tumble.audible-impacts` (IVP 4, Box3D 1, tolerance ±2). HEAD's
provider without this change, built in the same tree, gives the same value, so
this change does not cause it. See the finding below.

### Benchmark gate

Calibration: 3 interleaved rounds (host load 7 to 15). Medians:

| Workload | IVP p50 / p95 | Box3D p50 / p95 | Quality, IVP / Box3D |
| --- | --- | --- | --- |
| pile-256 | 0.63 / 1.90 ms | 0.29 / 0.57 ms | awake at end 253 / 0 |
| pile-1024 | 7.05 / 30.1 ms | 1.29 / 2.21 ms | peak growth 74 / 7.4 MB |
| pile-4096 | not in gate (one run: 114 / 756 ms) | 6.16 / 12.6 ms | lost 0 |
| stack-20 | 0.40 / 0.50 ms | 0.005 / 0.33 ms | collapsed 38 / 0 |
| ragdolls-32 | 2.16 / 3.00 ms | 0.24 / 0.67 ms | joint error 34 / 0.10 |
| ragdolls-128 | 18.8 / 24.4 ms | 0.64 / 2.69 ms | joint error 57 / 0.08 |
| projectiles-64 | 0.33 / 0.98 ms | 0.07 / 0.11 ms | tunneled 0 / 0 |
| shards-64 | 0.25 / 0.59 ms | 0.04 / 0.07 ms | tunneled 3 / 0 |
| panes-64 | 0.58 / 1.16 ms | 0.04 / 0.07 ms | tunneled 24 / **56** |

Box3D p50 by worker count:

| Workload | 1 | 2 | 4 | 8 |
| --- | --- | --- | --- | --- |
| pile-1024 | 1.26 ms | 0.84 ms | 0.94 ms | 0.49 ms |
| pile-4096 | 5.63 ms | 3.33 ms | 2.18 ms | 1.58 ms |
| ragdolls-128 | 0.64 ms | 0.55 ms | 0.47 ms | 0.49 ms |

Every Box3D configuration repeated bitwise across rounds. Each scaled workload
had one digest across all worker counts. IVP also repeated bitwise in fresh
processes.

Acceptance runs: see [gate runs](#gate-runs) below.

## Findings

- **The shared tree's Box3D has an uncommitted local patch.**
  `box3d/src/contact_solver.c` is modified (a restitution change: "only a
  touching point bounces"). The shared tree does not initialize the
  submodule, and the parent commit pins pristine `9e5a4cd`, so a clean
  checkout does not reproduce the shared tree's physics. Measured in this
  worktree:
  - with the patch: Box3D fails `vcollide.model-simulates`,
    `dynamics.tumble-travel-bounded` and `dynamics.held-floor-quiet` (the
    three failures earlier recorded as pre-existing on main's build);
  - with the pinned source: those pass, leaving only the audible-impacts
    divergence.

  This change measures the pinned source. The patch needs an owner decision:
  commit it to a pinned fork or revision, or drop it. It was not modified.
- **Box3D's default continuous collision loses to IVP against thin dynamic
  bodies** (56 of 64 projectiles through panes, IVP 24). IVP's look-ahead also
  stops most tunneling against static walls at the 2000 in/s speed limit. The
  `ccd-bullets` gate tracks the fix (P4, `b3Body_SetBullet`).
- **Parallel stepping does not survive a contended host.** In one acceptance
  run the load rose from 12 to 37. 4 workers gave 0.83 to 1.00× instead of
  2 to 3.4×, and single rounds spiked 40 to 70 ms. The runner now marks timing
  rules not-run above 0.5 load per CPU instead of reporting a regression. This
  is also the risk for loaded servers and thermally limited phones; P2's pool
  bridge and budgets own it.
- **Workers stop paying off early on mid-size scenes.** The 1024 pile gains
  1.9× at 8 workers but nothing from 2 to 4 workers. `PreStep` and
  `PostStep` walk every object on the calling thread twice per step. P2
  measures the split with `vphysics.step-profile.v1`.
- **The synthetic ragdoll exposes IVP joint divergence.** One IVP ragdoll is
  stable. With several, 2 of 6 spawn seeds reach 22 to 35 units of joint error
  with a part at the speed limit. This is not a claim about shipped content.
- `vphysics_box3d/job_scheduler.h` was unused and did not match the pinned task
  API. P2 deleted it.

## Gate runs

Command: `python3 tools/quality/physics_bench.py --build build --runtime
../source-engine/run/runtime --rounds 3 --sensitivity --wait-quiet 1800`
(worktree). Evidence is kept in the worktree's git-ignored `quality-results/`.

| Run | Host load | Result |
| --- | --- | --- |
| Calibration (`physics-bench-2026-09-24-calibration`) | 7–15 | Placeholder limits; the 7 faults with real rules detected; `bench-slow` was not timed (a harness bug, fixed) |
| First acceptance attempt | 12 → 37 | Timing, speedup and ratio rules failed (0.83–1.00× at 4 workers). Quality, determinism, contract and all 8 faults passed. This led to the contention guard. |
| Second attempt (`physics-bench-2026-09-24-contended`) | 31 | Timing rules not-run (0.96 per CPU > 0.50); gates `incomplete`; everything else passed; 8 of 8 faults detected |
| **Acceptance (`physics-bench-2026-09-24`)** | 15 → 9 | **`box3d-parity` pass, `parallel-step` pass, `ccd-bullets` (planned) fail at 56 of 64; 8 of 8 faults detected; status pass** |

Acceptance margins:

- p95 against IVP: 0.08× (pile-1024) to 0.27× (pile-256). The stack total
  is 0.08×.
- Box3D timing limits are met with about 2× headroom, for example pile-4096
  p95 11.6 ms against 25 ms.
- Speedup from 1 to 4 workers:
  - pile-4096: 2.75× (p50);
  - pile-1024: 2.26× (p50);
  - ragdolls-128: 1.71× (p95).
- One digest across 5 worker counts on each scaled workload. Every
  configuration repeats bitwise.
- Peak memory growth: pile-4096 25 MB serial and 26 MB at 4 workers.

After the acceptance run, only formatting changed (clang-format on edited
lines). The rebuilt binaries give the same pile-1024 4-worker digest
(`e4768bb28d586a8f`), and the contract still passes 34 of 34.

## P2 and P3 (2026-09-24, second goal)

Goal: complete P2, sharing the engine's job pool, and P3, the check on the
game's collision filters.

### P2: engine pool, budget, step profile

- **Worker tasks on the caller's pool.** `physics_parallelparams_t` now
  carries the `IThreadPool` the application root owns. Box3D's
  `enqueueTask`/`finishTask` map to `QueueCall` and to
  `CJob::WaitForFinish` + `Release`.
  - More than one worker needs a pool, and at most pool threads + 1 workers
    are accepted. In products that is the engine's `CmpJob` pool, so the
    existing capacity row is the physics budget.
  - Box3D's built-in scheduler is no longer used anywhere, and
    `vphysics_box3d/job_scheduler.h` is deleted.
- **`vphysics.step-profile.v1`**
  ([header](../public/vphysics/step_profile.h)) reports per `Simulate`:
  - solver, pre-step and post-step time;
  - bodies, awake bodies and contacts;
  - cumulative game-solver calls, and how many ran off the calling thread.
- **Contract**: 48 checks pass on Box3D. IVP reports `UNSUPPORTED`. New
  clauses:
  - a pool is required, and the pool's limit is enforced;
  - no provider threads, measured by a `/proc` census that has a negative
    control;
  - `Simulate` inside a pool job, with one free thread, matches the serial
    digest with no forbidden nested waits or starvation;
  - a pool stopped mid-run still matches the serial digest;
  - clean pool statistics;
  - the step-profile clauses, including a cross-check of its solver counts
    against a probe solver.
- **Results are preserved.** The pool bridge gives the same digests the
  built-in scheduler gave (pile-1024 `e4768bb28d586a8f`, pile-4096
  `fafa2937d6a46054`, ragdolls-128 `3e47047fdd8ce5e1`), with 0 threads added
  in every run.
- **Scaling** (p50, single runs, host load 3 to 9):
  - pile-1024: 1.20 / 0.79 / 0.55 / 0.68 ms at 1 / 2 / 4 / 8 workers;
  - pile-4096: 5.25 / 3.10 / 2.04 / 1.52 ms;
  - ragdolls-128: 0.62 / 0.67 / 0.47 / 0.61 ms.
- **Adapter serial share** (step profile): constant adapter time while
  solver time falls:
  - pile-1024: 9% at 1 worker, 20% at 4;
  - pile-4096: 11% to 27%;
  - ragdolls-128: 21% to 40% (0.29 ms per tick).
- **ThreadSanitizer** (clang `build-tsan`) is clean on:
  - the 48-check contract, including the nested pool job and the stopped
    pool;
  - pile-1024 at 8 workers, pile-4096 at 4, ragdolls-128 at 4;
  - the whole 603-check parity suite at 4 workers.

### P3: the game collision-filter check

- **Tool**: [`tools/quality/physics_filter_audit.py`](../tools/quality/physics_filter_audit.py)
  with the reviewed declaration
  [`quality/physics_filter_audit.json`](../quality/physics_filter_audit.json).
  - It covers 61 reachable sites across every game in the tree.
  - Seven rule classes are forbidden outright; six need a reviewed reason.
  - New, changed or stale sites and unreviewed findings fail.
  - 16 unit tests cover it: every rule fires, and comments, strings,
    comparisons and `static_cast` do not count. Call sites and the
    `CBaseEntity::ShouldCollide( group, mask )` trace filter are not
    collected. Each problem class fails, and forbidden or reasonless
    acceptances are rejected.
  - It runs in CI (`conformance.yml`) and as the `physics.filter-audit`
    check in `quality/baseline.json`.
- **Result**: 0 problems. The only finding is `CallbackContext` in the server
  and client `ShouldCollide_2`, accepted with its reason. The game-rules
  filters read only the two collision groups and members.
- **Decision**: policy (a), serialized on a worker, recorded in RFC 0013 and
  the contract record.
- **Server opt-in**: `-physics_workers N` (`game/server/physics.cpp`), read
  once at level start and clamped to the compute pool. `-physics_workers_required`
  turns a missing capability into an error, and the `physics_step_profile`
  command prints the profile.

Evidence (worktree build, headless native-Vulkan Portal, `testchmb_a_02`,
8 cubes spawned and settled):

| Run | Result |
| --- | --- |
| Box3D, `-physics_workers 4 -physics_workers_required` | Pass. "server environment steps on 4 workers (requested 4, compute pool 3 threads)". The game's `ShouldCollide` ran 113,949 times, **1,114 on pool threads**. No pool warnings, asserts or crash. |
| Box3D, default | Pass. One worker; 112,473 solver calls, 0 off the main thread. |
| IVP, `-physics_workers 4 -physics_workers_required` | `Sys_Error` at level start: capability missing (expected). |
| IVP, `-physics_workers 4` | Pass. Logs the fallback and runs one worker; `physics_step_profile` reports not available. |

Parity suite at 4 workers (`physics_conformance.py --candidate-workers 4`):
- Box3D passes 603 of 603, including the new `suite.workers` boot check.
- Against IVP: the same single known divergence as serial
  (`dynamics.tumble.audible-impacts`).
- **All 12,416 Box3D observations are bitwise identical to its serial run**,
  including touch, impact and sleep/wake events.

Most in-game solver calls stay on the main thread because Portal constantly
rechecks collision rules outside the step. The 1,114 off-thread calls are the
broad-phase pair tests Box3D runs on pool workers.

## Default on (2026-09-25, user decision)

At the user's direction, the server environment now steps in parallel by
default. Auto uses the compute pool's threads plus the calling thread.
`-physics_workers N` sets the count, `-physics_workers 1` turns it off, and
`-physics_workers_required` makes a missing capability an error. IVP keeps
one worker silently.

Evidence (rebased build, headless Portal, `testchmb_a_02`):

| Boot | Result |
| --- | --- |
| Box3D, no arguments | "steps on 4 workers (default, compute pool 3 threads)"; the game's filter ran 159,557 times, 1,521 on pool threads |
| Box3D, `-physics_workers 1` | One worker; 0 calls off the main thread |
| IVP, no arguments | One worker; no warning; step profile not available |

In the default run, the last step took 6.6 ms, against 0.23–0.33 ms at 1
worker and in the earlier 4-worker run. It is one sample taken at host
load ~50. It matches the benchmark's contended-host spikes, which are this
default's main risk. No timing gate has certified the default on a
contended host or on any mobile profile.

## Unverified and open

- No product enables parallel stepping by default; that is a per-profile
  decision. The client environment keeps one worker. No gameplay soak or
  gameplay timing capture at N workers exists, and the dedicated server has no
  profile row.
- The adapter's serial `PreStep`/`PostStep` share limits scaling (above).
- Android (Fold7) and Apple profiles are `unverified` in `physics-v1.json`.
- No CI lane. `physics.bench` is not yet a `quality/baseline.json` check,
  because the declared tree (`build-r03-portal-native`) must first be rebuilt
  with the `--bench` host.
- Event order across worker counts is compared only through the parity
  suite's event-derived observations (identical at 1 and 4 workers), not as a
  direct event-sequence clause.
- TSan runs are local (clang, `build-tsan`), not a CI lane.
