# RFC 0013: Opt-in Box3D Physics Capabilities

- Status: Proposed (2026-09-24). P0 to P3 are implemented on the Linux desktop
  profile: parallel stepping on the engine pool, the step profile, the game
  collision-filter audit, and the server environment. **Parallel stepping is
  the server default** (user decision, 2026-09-25), with `-physics_workers 1`
  to turn it off.
- Date: 2026-09-24
- Scope: How Box3D behavior that IVP cannot provide is exposed beside the
  VPhysics compatibility contract: one narrow, versioned interface per
  capability, selection by the application root, and an acceptance gate per
  capability. The first capability is parallel stepping.
- Parity baseline: [RFC 0004](0004-box3d-primary-physics-backend.md) owns the
  Box3D provider, its IVP-parity contract
  ([`vphysics.provider.v1`](../unittests/physicstest/contracts/vphysics.provider.v1.md))
  and default promotion. This RFC never loosens that contract.
- Scheduling: [RFC 0003](0003-dependency-aware-job-system.md) owns the engine
  worker pool and its budgets (R20).
- Composition: [RFC 0001](0001-capability-based-platform-architecture.md) owns
  provider selection and product profiles.
- Synchronization: [RFC 0006](0006-modern-cpp-ownership-and-synchronization.md)
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md) (Q-PHYSICS)
- Progress: [RFC 0013 progress](0013-progress.md)

## Decision and boundary

RFC 0004 makes Box3D behave like IVP behind the VPhysics interfaces. That is
the right migration contract, but it hides everything Box3D does better:
stepping on several workers, worker-count-invariant determinism, bullets,
sensors, joint motors, recording and runtime collision. This RFC adds those as
opt-in capabilities:

1. **One capability, one interface.** Each capability is a small abstract
   class with a versioned name (`vphysics.<name>.v<N>`, interface string
   `VPhysics<Name>NNN`), reached through `IPhysics::QueryInterface`, the path
   the app framework already uses for sibling interfaces. `IPhysics`,
   `IPhysicsEnvironment` and `IPhysicsObject` keep their vtables. There is no
   capability bag, flags word or "extended environment" base class.
2. **Absence is normal.** IVP returns NULL for every capability. A caller
   declares each capability required or optional at composition. A required
   capability that is missing fails composition before a level loads. An
   optional one leaves the caller on the parity path. Nothing falls back after
   a partial mutation.
3. **The application root opts in, per product profile.** Environment-scoped
   choices are fixed when the environment is created, not toggled mid-level by
   a console variable.
4. **Each capability has a contract and a gate.** Its clauses run against
   every claiming provider with deliberate faults. Its acceptance rules live in
   [`quality/budgets/physics-v1.json`](../quality/budgets/physics-v1.json),
   evaluated by [`physics_bench.py`](../tools/quality/physics_bench.py). A
   profile may enable a capability only when that capability's gate is
   `required` and passes on that profile.
5. **Parity stays whole.** With every capability off, Box3D must still pass
   `physics.conformance` against IVP.

## Observed starting point (2026-09-24)

Facts from source revision `3ef969ad`, Box3D `9e5a4cd`, and the benchmark runs
recorded in the [progress record](0013-progress.md).

- The Box3D provider passes the IVP-parity suite (559 checks) and created every
  world with one worker (`vphysics_box3d/physics_environment.cpp`).
- Box3D creates its task scheduler only when a world is created with more than
  one worker or with task callbacks (`box3d/src/physics_world.c`, world
  creation). `b3World_SetWorkerCount` on a serial world only repartitions the
  work, which still runs inline. So the worker count must be chosen when the
  environment is created.
- Box3D calls the custom filter from broad-phase pair tasks and sensor tasks
  (`broad_phase.c`, `sensor.c`), and pre-solve from contact tasks
  (`contact.c`). The provider's custom filter calls the game's
  `IPhysicsCollisionSolver::ShouldCollide`. With 4 workers on a 512-cube pile,
  3.5k of 5.9k solver calls ran on worker threads.
- `vphysics_box3d/job_scheduler.h` is unused. It was written against a
  Box2D 3.0-style task signature, not the pinned `b3TaskCallback( void * )`,
  and it passes thread context 0 to every item. It is not a bridge; P2
  replaces it.
- Calibrated benchmark medians on the Linux desktop runner (step time, 300
  ticks at 66.7 Hz, one-worker parity path):

  | Scene | IVP p50 / p95 | Box3D p50 / p95 | Scene quality, IVP / Box3D |
  | --- | --- | --- | --- |
  | pile, 256 Portal cubes | 0.63 / 1.90 ms | 0.29 / 0.57 ms | 253 / 0 bodies still awake at the end |
  | pile, 1024 | 7.05 / 30.1 ms | 1.29 / 2.21 ms | peak memory growth 74 / 7.4 MB |
  | pile, 4096 | 114 / 756 ms (one run) | 6.2 / 12.6 ms | |
  | stack, base 20 | 0.40 / 0.50 ms | 0.005 / 0.33 ms | 38 / 0 cubes collapsed |
  | ragdolls, 128 synthetic | 18.8 / 24.4 ms | 0.64 / 2.69 ms | joint error 57 / 0.08 units |

- Box3D's state is bitwise identical for the plain environment and for 1, 2,
  4 and 8 workers on every scaled scene, and repeats across rounds.
  Workers pay off only on large scenes:

  | Box3D p50 by workers (1 / 2 / 4 / 8) | |
  | --- | --- |
  | pile, 1024 | 1.26 / 0.84 / 0.94 / 0.49 ms |
  | pile, 4096 | 5.63 / 3.33 / 2.18 / 1.58 ms |
  | ragdolls, 128 | 0.64 / 0.55 / 0.47 / 0.49 ms |

  Under host load, single rounds at 2 to 8 workers showed 40 to 70 ms p95
  spikes when scheduler threads were descheduled.
- Tunneling, with 64 boxes at the 2000 in/s object speed limit:

  | Barrier | IVP | Box3D |
  | --- | --- | --- |
  | 6-unit boxes, 2-unit static wall | 0 | 0 |
  | 2-unit boxes, 1-unit static wall | 3 | 0 |
  | 6-unit boxes, 1-unit dynamic panes | 24 | **56** |

  IVP's look-ahead already stops most tunneling at gameplay speeds.
  Box3D's default continuous collision covers static geometry only, so it
  loses to IVP against thin dynamic bodies. That case is the job of
  `vphysics.continuous.v1`.
- The ragdoll scene uses a synthetic 11-box humanoid, because Chell's `.phy`
  cannot be posed without the model's bone transforms. One IVP ragdoll is
  stable (0.6 units of joint error). Several IVP ragdolls diverge on 2 of 6
  spawn seeds (22 to 35 units, a part at the speed limit). Treat this as an
  IVP result on this rig, not a claim about shipped content.

## Goals

- Reach Box3D behavior that IVP lacks without changing the legacy ABI or the
  parity contract.
- Make each capability selectable per product profile, with explicit
  required and optional semantics.
- Give each capability a contract, deliberate bad providers, and a measured
  acceptance gate, before any game code depends on it.
- Keep game callbacks single-threaded and on documented threads.
- Keep worker use inside a process-wide budget that mobile profiles can
  lower.

## Non-goals

- Coordinates beyond Source's ±16384-unit world. Box3D's large-world support
  does not lift the limits that BSP files and network encodings set.
- Deterministic lockstep networking or physics prediction and rollback. Box3D
  documents that it has no rollback determinism, and Source stays
  server-authoritative.
- Box3D's character mover (RFC 0004 non-goal).
- Changing any default. Promotion stays per profile under RFC 0004 and this
  RFC's gates.
- Mixing providers, or moving a live world between worker counts.

## Capability catalog

| Capability | Interface | Enables | Box3D API | State | Gate |
| --- | --- | --- | --- | --- | --- |
| `vphysics.parallel-step.v1` | `IPhysicsParallelStep` ([header](../public/vphysics/parallel_step.h)) | An environment stepped on N workers of the caller's thread pool, fixed at creation | world `workerCount`, `enqueueTask`/`finishTask` | Implemented (P1–P3); server opt-in `-physics_workers N` | `parallel-step` (required) |
| `vphysics.step-profile.v1` | `IPhysicsStepProfile` ([header](../public/vphysics/step_profile.h)) | Solver against adapter time per `Simulate`, body/awake/contact counts, game-solver calls on and off the calling thread | `b3World_GetCounters`, provider timers | Implemented (P2); `physics_step_profile` server command | contract clauses in `parallel-step` |
| `vphysics.continuous.v1` | planned | Per-object bullet flag: continuous collision against dynamic bodies | `b3Body_SetBullet` | planned (P4) | `ccd-bullets` (planned; fails today) |
| `vphysics.recording.v1` | planned | Record a session; validate a replay | `b3World_StartRecording`, `b3ValidateReplay` | planned (P5) | replay equals digest |
| `vphysics.sensors.v1` | planned | Shape sensors with begin/end overlap events | sensor shapes and events | planned (P6) | event-order and overlap suite |
| `vphysics.joint-drive.v1` | planned | Joint springs, motors, limits; constraint force readback | joint defs, `b3Joint_GetConstraintForce` | planned (P6) | analytic drive cases |
| `vphysics.explosion.v1` | planned | Radial impulses with falloff | `b3World_Explode` | planned (P6) | impulse/falloff cases |
| `vphysics.runtime-collision.v1` | planned | Collision built at runtime: meshes, baked compounds, height fields | `b3CreateMeshShape`, `b3CreateBakedCompoundShape`, `b3CreateHeightFieldShape` | planned (P7, with R59/R61/R45) | corpus and query suite |

Each planned capability gets its own interface, contract clauses and gate
before any consumer uses it. A capability can be split or narrowed when a
provider cannot meet all of it (see the LSP rules in AGENTS.md). IVP never
claims one.

## Parallel step (`vphysics.parallel-step.v1`)

### Contract

- `GetMaxWorkerCount()` is at least 1.
- `CreateParallelEnvironment( params )` returns NULL and creates nothing
  when:
  - `workerCount` is below 1 or above the maximum;
  - `workerCount` is above 1 and `pThreadPool` is NULL;
  - `workerCount` exceeds the pool's threads plus the calling thread.

  Otherwise it returns an environment that `IPhysics` owns, lists and
  destroys like any other. The pool must outlive the environment.
- `GetWorkerCount()` returns the count for a live environment this provider
  created (1 for `IPhysics::CreateEnvironment`) and 0 for any other pointer,
  including a destroyed one.
- Stepping starts no threads. Worker tasks run on the given pool or on the
  thread that called `Simulate`. Waiting for a task runs only that task, so
  `Simulate` may run inside a pool job without deadlock. A stopped pool still
  works: its tasks run on the waiting thread.
- Game callbacks are never entered concurrently. Collision events, object
  events, constraint events and controllers run on the thread that called
  `Simulate`. The collision solver may run on a pool thread, one call at a
  time, while that thread is blocked in `Simulate`.
- When `IsWorkerCountInvariant()` is true, a fixed call sequence gives
  bitwise-identical object state for every accepted worker count.

`vphysics.step-profile.v1` adds:

- `GetLastSimulate` rejects NULL, foreign and destroyed environments
  without touching the output.
- Every field is zero before the first `Simulate`.
- Solver time plus adapter time never exceeds the call's time.
- Reading the profile does not change the simulation.
- The cumulative game-solver counts equal what the solver itself observed.

The clauses run as `vphysics_conformance --bench contract` (48 checks;
[contract record](../unittests/physicstest/contracts/vphysics.parallel-step.v1.md)).
The gate proves that each clause can fail, using host-injected faults: a
provider that
- accepts invalid counts;
- reports the wrong count;
- enters the solver concurrently;
- diverges by worker count;
- starts its own thread;
- deadlocks inside a pool job;
- reports an impossible profile.

### Implementation

- `CPhysicsEnvironmentBox3D` takes the worker count and the pool at
  construction.
- With more than one worker, it sets Box3D's `enqueueTask` to the pool's
  `QueueCall` and `finishTask` to `CJob::WaitForFinish` plus `Release`.
  Box3D's own scheduler, which starts threads for each world, is never used.
- The pool's `YieldWait` runs a waited job on the waiting thread if no worker
  has started it, and never runs unrelated work. Box3D tasks are leaves, and
  the thread in `b3World_Step` orchestrates the solver itself, so every step
  completes even when no pool thread is free.
- The environment serializes calls into the game's solver with a mutex, and
  counts them for the step profile.
- `CPhysicsParallelStepBox3D` and `CPhysicsStepProfileBox3D` are exported
  through the module factory. The unused, mismatched `job_scheduler.h` is
  deleted.

### Game collision filters on worker threads (P3 decision)

Policy (a) is adopted: the game's filter is called on a worker thread,
serialized. Policies (b), a main-thread table of pair decisions, and (c), a
game-declared pure filter, are not needed for today's filters.

The decision holds only while an audit keeps proving it:
[`physics_filter_audit.py`](../tools/quality/physics_filter_audit.py) with
the reviewed declaration
[`quality/physics_filter_audit.json`](../quality/physics_filter_audit.json).

- **Scope.** It finds every definition the filter can reach, 61 sites in all
  games:
  - the server and client `CCollisionEvent::ShouldCollide` and
    `WheelCollidesWith`;
  - every `CGameRules`-derived `ShouldCollide( int, int )`;
  - `ForceVPhysicsCollide` and `PhysicsSolidMaskForEntity` overrides;
  - the entity accessors the filters call.
- **Scan.** It checks each body for 13 rule classes. Seven are forbidden
  outright under this policy:
  - thread identity;
  - engine or sound calls;
  - entity or physics mutation;
  - random numbers;
  - prediction state;
  - console-variable writes.

  The other six need a reviewed reason:
  - member writes;
  - statics;
  - allocation;
  - console output;
  - lazy transform caches;
  - the solver's own callback counter.
- **Ratchet.** New sites, changed bodies, stale entries and unreviewed
  findings fail. It runs in CI (`conformance.yml`) and as the
  `physics.filter-audit` baseline check.
- **Result.** The only finding in all 61 sites is `CallbackContext` in the
  two root filters. It increments `CCollisionEvent::m_inCallback` under the
  provider's mutex, and the step's join orders it before main-thread reads.
  Every game-rules filter is a pure function of the two collision groups plus
  member reads.
- **Provider side.** The provider calls the filters reach
  (`IPhysicsObject` getters, the pair hash, collision sets) are plain reads
  of state the main thread does not change during `Simulate`.
- **Condition.** A frame graph that overlaps physics with other
  entity-mutating work must declare the filter's entity reads first.

### Scheduler bridge and budgets (P2)

- The worker budget is the pool the application root passes in. In products
  that is the engine compute pool (`g_pThreadPool`, `CmpJob`), whose capacity
  row in [`scheduler-v1.json`](../quality/budgets/scheduler-v1.json) is 3 on
  the Linux desktop. So physics gets at most 4 workers by default and adds no
  threads.
- Mobile profiles clamp the pool, not physics.
- `Simulate` inside a pool job is tested, not forbidden. With a 2-thread pool
  and 3 workers, the nested step completes, matches the serial digest, and
  records no forbidden nested waits or starvation.
- Box3D's solver workers spin until the orchestrator finishes a solve. While
  they do, the pool threads running them are busy for that part of the step
  and unavailable to other pool work. That is the cost of sharing the pool.
- The step profile shows the adapter's serial share: `PreStep` and
  `PostStep` take a constant 0.29 ms per tick for 128 ragdolls, 21% of the
  step at 1 worker and 40% at 4. That, not the solver, limits scaling on
  mid-size scenes, and it is the next optimization target.

## Benchmark gate

The benchmark is a mode of the existing provider host, so there is one loader
for the conformance suite and the benchmark, and no new `dlopen` site.

- Command: `python3 tools/quality/physics_bench.py --build <tree> --runtime
  <staged runtime> --rounds 3 --sensitivity`. Evidence goes to
  `<out>/evidence.json` (`physics-bench-evidence/v1`).
- Scenes ([source](../unittests/physicstest/test_vphysics_bench.cpp)) use
  public interfaces only, identical inputs for every provider, and the game's
  collision rule that parts of one owner do not collide:

  | Scene | What it stresses |
  | --- | --- |
  | `pile` | Portal's authored cube, dropped in jittered layers into a pit: broad phase and contacts at scale |
  | `stack` | A 2D pyramid: solver stability and sleeping |
  | `ragdolls` | 11-box humanoids in constraint groups, lying in a pile: joints under contact |
  | `projectiles`, `shards`, `panes` | Fast boxes against thin static walls or dynamic panes: tunneling |
  | `contract` | The parallel-step clauses |

- Each run reports:
  - a step-time sample per tick (p50, p95, p99, max and total);
  - peak memory growth;
  - scene metrics: lost bodies, collapsed cubes, joint error, tunneled
    projectiles and awake bodies;
  - an FNV-1a digest of the bits of every body's state.
- Runs are interleaved across rounds. Statistics are medians across rounds.
- Gates:

  | Gate | State | Rules |
  | --- | --- | --- |
  | `box3d-parity` | required (R37) | Every run completes. Box3D repeats bitwise and keeps scenes intact. Box3D p95 is at most 1.10× IVP (the RFC 0004 review threshold). Per-profile p95 and memory limits. |
  | `parallel-step` | required (R67) | The contract passes. Digests are invariant across worker counts and repeat across rounds. Minimum speedups from 1 to 4 workers (1.5× on the 4096 pile). Per-profile limits. |
  | `ccd-bullets` | planned (R67) | No projectile passes through the dynamic panes. Fails today at 56 of 64; reported, never certified. |

- A rule may be a declared known gap with an owner. The gap keeps its gate
  open, and a gap that starts passing is flagged for review.
- A profile without measurements is `unverified` and cannot certify a gate.
  Android and Apple profiles are unverified.
- Timing needs a quiet host. Each profile declares `max_load_per_cpu`
  (0.5 on the Linux desktop). The runner samples the 1-minute load before each
  round and after the last. Above the limit, timing, ratio and speedup rules
  report not-run and their gates stay `incomplete`. Quality, determinism,
  memory and contract rules still count. `--wait-quiet <seconds>` waits for
  the host before each round.
- Parallel runs also report added threads and the pool's forbidden nested
  waits and starvation events. Each must be 0.
- `--sensitivity` runs 11 host faults and requires each one's target rule to
  fail:
  - a slow step;
  - a collapsing stack;
  - broken joints;
  - a non-colliding wall;
  - worker-count divergence;
  - six contract violations: invalid counts accepted, a wrong count, a
    concurrent solver, a private thread, a nested-step timeout, and an
    impossible profile.

## Composition and selection (P3)

- **Default on (user decision, 2026-09-25).** When the provider offers the
  capability and the engine compute pool is running, the server's level
  environment (`CPhysicsHook::LevelInitPreEntity`, `game/server/physics.cpp`)
  steps on every pool thread plus the calling thread. That is 4 workers on
  the Linux desktop's 3-thread `CmpJob` pool.
  - `-physics_workers N` sets the count instead; `-physics_workers 1` turns
    parallel stepping off.
  - The count is read once, at level start, and clamped to the pool.
  - A provider without the capability (IVP) keeps one worker silently. With
    an explicit `N > 1` it logs a warning instead.
  - `-physics_workers_required`, with or without `N`, turns a missing
    capability into a `Sys_Error`.
- The default applies on every platform that runs this server code,
  including the Android listen server, whose profile has no physics
  measurements yet (see the open decisions).

  Portal's simulators share this environment (`physenv_main`), so they are
  covered. Nothing reads a console variable to change workers on a live
  environment.
- A product profile opts out with `-physics_workers 1` in its launch
  arguments. None does today.
- The client environment keeps one worker until its own captures exist. The
  dedicated server uses the same server code and needs its own profile row.
- `physics_step_profile` prints the server environment's last step profile,
  including the game-solver calls made off the main thread.

## Delivery plan

| Phase | Deliverable | Exit gate | State |
| --- | --- | --- | --- |
| P0 | This RFC, benchmark scenes, `physics_bench.py`, `physics-v1.json`, IVP and Box3D measurements | Runner tests; each fault detected; evidence recorded | Done (Linux desktop) |
| P1 | `vphysics.parallel-step.v1` in the Box3D provider, contract clauses | `parallel-step` and `box3d-parity` pass on Linux desktop; `physics.conformance` still passes | Done (Linux desktop) |
| P2 | Engine-pool bridge, pool-owned worker budget, `vphysics.step-profile.v1`, adapter serial fraction measured; delete `job_scheduler.h` | Nested-work and shutdown tests; TSan-clean scaled scenes; no provider threads | Done (Linux desktop); Android and Apple unmeasured |
| P3 | Server opt-in, filter audit and policy, parity suite and Portal gameplay at N workers | Conformance unchanged at N workers; filter audit clean; live gameplay at N workers with filters on pool threads; per-profile limits on Linux and Fold7 | Done on Linux desktop except Fold7 limits and a gameplay soak |
| P4 | `vphysics.continuous.v1` bullets | `ccd-bullets` becomes required and passes |
| P5 | `vphysics.recording.v1` with Source-level filter/controller capture | Replay reproduces digests; recordings tagged with versions (RFC 0004) |
| P6 | Sensors, joint drive, explosion | Each capability's own suite with bad providers |
| P7 | Runtime collision (with R59/R61/R45) | Corpus, query and cooking gates |

## Roadmap

R67 in [AGENTS.md](../AGENTS.md) tracks this RFC. It depends on R19 (the
Box3D slice). P2 depends on R20, P3 on R31 and R12, and P7 on R59 and R45.
Enabling any capability by default in a profile is a separate product
decision after that profile's gates pass.

## Risks and mitigations

| Risk | Effect | Mitigation |
| --- | --- | --- |
| Default-on parallel stepping on contended or mobile hosts | Frame spikes when solver workers are descheduled | `-physics_workers 1` opt-out per profile; `physics_step_profile` to diagnose; Fold7 measurement owed |
| Game filter code assumes the main thread | Subtle gameplay bugs, asserts | Filter audit with forbidden rule classes and an exact ratchet, in CI; the serialized policy is recorded in the contract |
| Oversubscription from several worlds | Frame spikes (40–70 ms seen under load) | Worker tasks run on the engine pool, whose size is the budget, and the provider adds no threads. Spinning solver workers still occupy pool threads during a solve, so medians and per-profile limits apply. |
| Capabilities drift into parity behavior | IVP rollback breaks | Parity gate stays required; capability-off runs match IVP |
| Synthetic scenes miss real content behavior | Wrong promotion | P3 gameplay corpus; scenes use authored cubes where possible |
| Timing limits flake on a shared host | Noisy gate | Interleaved rounds, medians, limits at about 2× median, profile-owned numbers |

## Alternatives considered

- **Add methods to `IPhysicsEnvironment`.** This breaks the frozen vtable for
  game DLLs and mods.
- **A capability flags query or bag.** AGENTS.md forbids it, and flags cannot
  carry per-capability contracts.
- **A console variable for worker count.** It changes a live world's
  partitioning and hides the choice from composition.
- **Always parallel.** The small scenes gain nothing, the spikes under load
  are real, and game filters are unaudited.

## Open decisions and required evidence

- Whether the default suits every profile.
  - It is on everywhere by user decision.
  - On a contended host, spinning solver workers lose their cores: 40–70 ms
    benchmark spikes, and one 6.6 ms in-game step at host load 50 against
    0.23 ms at load 3.
  - The Fold7 and Apple profiles have no measurements.
  - A profile that fails its gate should opt out with `-physics_workers 1`
    until the adapter or scheduler is fixed.
- Event order across worker counts: all 12,416 parity-suite observations,
  including touch, impact and sleep/wake events, are identical at 1 and 4
  workers. A direct event-sequence comparison is not yet a contract clause.
- Whether the dedicated server should use parallel stepping, given its
  small per-tick cost (RFC 0003 profile: 0.35 ms server tick).
- The client environment and a gameplay soak at N workers.
- Reducing the adapter's serial `PreStep`/`PostStep` share.

## Source references

- Box3D announcement and motivation: <https://box2d.org/posts/2026/06/announcing-box3d/>
- Box3D FAQ (determinism, threading): <https://github.com/erincatto/box3d/blob/main/docs/faq.md>
- Box3D pinned revision: `box3d` submodule `9e5a4cde862fba95ff19f096b79567f3ea6c01fd`
- Provider: `vphysics_box3d/main.cpp`, `vphysics_box3d/physics_environment.cpp`
- Harness: `unittests/physicstest/test_vphysics_bench.cpp`,
  `tools/quality/physics_bench.py`, `quality/budgets/physics-v1.json`

## Proposed decision

Adopt the per-capability interface model, keep parity as the default, ship
`vphysics.parallel-step.v1` as P1 with the benchmark gate as its acceptance
criterion, and require each later capability to arrive with its own contract
and gate.
