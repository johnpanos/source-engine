# RFC 0026: Box3D Beyond IVP, with IVP as the Fallback

- Status: Proposed (2026-10-07). The scenario maps are installed
  ([Scenario maps](#scenario-maps)). Nothing else is implemented, and no phase
  gate has passed.
- Date: 2026-10-07
- User direction (2026-10-07): "I want to solidify and extend Box3D without
  dropping support for IVP ... I want to surpass IVP in quality while still
  allowing us to 'fall back' there." Then: "what does Box3D support that IVP
  doesn't that we should integrate? Let's start there." Then: "Write the RFC
  for this and update all the docs to represent this. Add instructions for the
  agents on what done looks like and the ratchet", and "make test maps that
  are published that exercise specific physics scenarios where it's set up for
  the user".
- Ownership (one owner per fact, AGENTS.md DRY):
  - [RFC 0004](0004-box3d-primary-physics-backend.md) owns the Box3D
    provider, the IVP-parity contract
    ([`vphysics.provider.v1`](../unittests/physicstest/contracts/vphysics.provider.v1.md)),
    provider selection and default promotion.
  - [RFC 0013](0013-opt-in-physics-capabilities.md) owns the capability
    mechanism (one versioned interface per capability through
    `IPhysics::QueryInterface`, composition, the benchmark gate rules) and the
    [capability catalog](0013-opt-in-physics-capabilities.md#capability-catalog).
  - This RFC owns: the IVP fallback guarantee, what "surpasses IVP" means and
    how it is measured, which Box3D features are integrated and in what order
    (phases B0–B9), the definition of done for each phase and the program,
    the ratchets, and the scenario maps.
- Scheduling and placement: [RFC 0003](0003-dependency-aware-job-system.md).
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  (Q-PHYSICS).
- Tracking: row R98 in [AGENTS.md](../AGENTS.md). Progress:
  [RFC 0026 progress](0026-progress.md).

## Why

Box3D is the default provider in every Box3D-capable product, yet the engine
uses it only as a faster IVP. RFC 0004 hides everything Box3D does better
behind the IVP-parity contract, which is right for migration. RFC 0013 built
the mechanism to expose more, and shipped parallel stepping and the shape
inertia model, but its remaining capabilities have no order, no definition of
"better than IVP", and no rule for keeping IVP working as the fallback.

The user's goal is two-sided: Box3D must become measurably better than IVP,
and IVP must stay a working provider that any product can fall back to at
launch. This RFC turns that goal into a ranked program with machine-checked
evidence. It also records the measured places where Box3D is worse than IVP
today, because "surpass" has to include them.

## Observed starting point (2026-10-07)

Facts from the tree at `1eb4e12b` and the existing records. Nothing below
was rerun for this RFC unless it says so.

### Providers and selection

- Box3D (`vphysics_box3d`) is the default physics in the launcher (and so the
  Android APKs), the Linux dedicated server (`dedicated/sys_linux.cpp`),
  `./play`, `./play_p2`, `run.sh` and the iOS and tvOS apps.
  `-physics vphysics` selects IVP. The Windows dedicated server, a legacy
  profile, still defaults to IVP. (AGENTS.md R67 notes; RFC 0004 progress
  still says the launcher defaults to IVP and is corrected by this change.)
- Both providers are linked into every product that offers `-physics`
  (`public/vphysics/provider_catalog.h`).

### Parity

- `physics.conformance` is a recorded known `fail` (`quality/baseline.json`,
  owner R19). On the pinned fork `johnpanos/box3d` `78c90a0`, Box3D fails three
  gameplay checks (`vcollide.model-simulates`, `dynamics.tumble-travel-bounded`,
  `dynamics.held-floor-quiet`) and three observations diverge
  (RFC 0013 progress, 2026-09-25). On upstream `9e5a4cd` it passes every
  check, but `dynamics.tumble.audible-impacts` diverges (IVP 4, Box3D 1,
  tolerance ±2).

### Capabilities already delivered (RFC 0013)

| Capability | State |
| --- | --- |
| `vphysics.parallel-step.v1` | Implemented; server default where Box3D is selected |
| `vphysics.step-profile.v1` | Implemented |
| `vphysics.shape-inertia.v1` | Implemented; on in `./play` and `./play_p2`; no gameplay corpus |

### What Box3D offers that the engine does not use

From the pinned headers (`box3d/include/box3d/box3d.h`, `types.h`,
`collision.h`) and a search of `vphysics_box3d/`:

| Box3D feature | API | Used by the provider today | IVP equivalent |
| --- | --- | --- | --- |
| Continuous collision against dynamic bodies | `b3BodyDef.isBullet`, `b3Body_SetBullet`, `b3World_EnableSpeculative` | No | Look-ahead in the solver; no per-body flag |
| Capsule shapes | `b3CreateCapsuleShape`, capsule mass/AABB/ray cast/overlap | No (spheres only, `b3CreateSphereShape`) | None (hull approximation only) |
| Rolling resistance | `b3SurfaceMaterial.rollingResistance` (spheres and capsules) | No | None |
| Surface tangent velocity (conveyors) | `b3SurfaceMaterial.tangentVelocity` | No | None |
| Hit events with approach speed and threshold | `b3ShapeDef.enableHitEvents`, `b3World_SetHitEventThreshold`, `b3ContactHitEvent` | No | Pre-collision velocities in callbacks |
| Contact begin/end events with manifolds | `b3World_GetContactEvents`, `b3Shape_GetContactData` | Internally, for the parity callbacks | Collision callbacks |
| Sensor shapes with begin/end overlap | `b3ShapeDef.isSensor`, `b3World_GetSensorEvents` | Internally (fluids, triggers) | Fluid/trigger controllers |
| Joint motors, springs, limits, force readback | `b3*Joint_Enable*`, `b3Joint_GetConstraintForce` | Internally, to emulate IVP constraints | IVP constraint motors |
| Joint events and force thresholds | `b3JointEvent`, `b3JointDef.forceThreshold` | No | Breakable constraints (IVP-side) |
| Explosions with falloff | `b3World_Explode`, `b3ExplosionDef` | No | None (game scripts impulses) |
| Recording and replay validation | `b3World_StartRecording`, `b3CreatePlayer`, `b3ValidateReplay` | No | None |
| Runtime mesh, height-field and baked-compound shapes | `b3CreateMeshShape`, `b3CreateHeightFieldShape`, `b3CreateBakedCompoundShape` | No | Cooked `.phy`/BSP collision only |
| Shape-swept character mover | `b3World_CastMover`, `b3World_CollideMover` | No | None (Source moves the player itself) |
| Double-precision worlds | `BOX3D_DOUBLE_PRECISION` | No | No |

Box3D has **no cloth or soft-body simulation**. A cloth feature would be a new
solver of our own, outside this RFC.

### Where Box3D is already better, and where it is worse

From the RFC 0013 benchmark (`physics_bench.py`, Linux desktop, one worker)
and the shared suite's `gyro.*` clauses (RFC 0004 progress):

| Metric | Scene | IVP | Box3D | Standing |
| --- | --- | --- | --- | --- |
| Projectiles through thin dynamic panes | `panes-64` | 24 | **56** | **Box3D worse** (B1) |
| Projectiles through a thin static wall | `shards-64` | 3 | 0 | Box3D better |
| Projectiles through a static wall | `projectiles-64` | 0 | 0 | Equal |
| Cubes collapsed | `stack-20` | 38 | 0 | Box3D better |
| Joint error (units) | `ragdolls-128` | 57 | 0.08 | Box3D better |
| Bodies awake at the end | `pile-256` | 253 | 0 | Box3D better |
| Peak memory growth | `pile-1024` | 74 MB | 7.4 MB | Box3D better |
| Step time p95 | `pile-1024` | 30.1 ms | 2.21 ms | Box3D better |
| Torque-free rotational energy change, 3 s | `gyro.free-*` | +40.6 % | −19 % | Box3D better (no gain), but it dissipates |
| Torque-free angular momentum change, 3 s | `gyro.free-*` | +20 % | −11 % | Box3D better |
| Gyroscope axle sink rate, shape-inertia model | `gyro.gyroscope-stays-level` | 0.7 °/s | 2.6 °/s (legacy model 1.3) | **Box3D worse** |

The in-game reproduction on `phys_tunnel` (2026-10-07, current `build/` tree,
headless) matches: lane C sent 8 of 8 cubes through the hanging panes on
Box3D and 4 of 8 on IVP; lanes A and B stopped every cube on both
([progress](0026-progress.md#scenario-maps-installed-2026-10-07)).

## Decisions

Agent decisions under the user's direction and standing instruction
(choose the recommended long-term option and record why):

1. **IVP stays a supported fallback provider.** It is not retired by this
   program. [The fallback guarantee](#the-ivp-fallback-guarantee) defines what
   "supported" means. Removing the IVP runtime from any profile now needs a
   new, explicit user decision; R44 is narrowed accordingly.
2. **"Surpasses IVP" is a measured claim, per profile and capability set**,
   defined by [the scoreboard](#surpasses-ivp-the-scoreboard). No document or
   commit may claim it while any scoreboard metric is worse than IVP's.
3. **Every new Box3D feature goes through RFC 0013's mechanism**: one
   versioned interface per capability, IVP returning NULL, root-selected per
   profile. No capability bag, no extended `IPhysicsEnvironment`, no console
   variable that changes a live world.
4. **Solidify before claiming.** B0 (the parity verdict and the ratchets) does
   not block implementing B1+, but no capability is enabled by default in a
   profile, and no "surpasses" claim is made, until B0 is done.
5. **Order**: B1 continuous collision (the one metric where Box3D clearly
   loses), B2 capsules and B3 surface motion (new behavior IVP cannot do),
   B4 contact events (and the open `audible-impacts` divergence), then B5
   joint drive, B6 sensors, B7 explosions, B8 recording, B9 runtime collision.
   Rationale: fix the regression first, then the features users can see, then
   tooling and content-pipeline features that depend on other rows.
6. **Every phase ships a published scenario map** that a user can play on
   either provider, and a headless in-game check on it
   ([Scenario maps](#scenario-maps)).
7. **Out of scope**: cloth (Box3D has none), the Box3D character mover and a
   capsule player hull (RFC 0004 non-goal: Source player movement is
   preserved), double precision (the ±16384-unit world stands), and rollback
   or lockstep networking.

## The IVP fallback guarantee

These hold for every phase and are checked by the ratchets.

- **F1 Selectable.** Every product profile that ships Box3D keeps IVP linked
  and selectable with `-physics vphysics` at process start, unless the user
  retires IVP for that named profile. Switching providers never happens on a
  live world (RFC 0004).
- **F2 Untouched.** No phase changes IVP's code paths or the IVP-parity
  contract. IVP returns NULL for every capability interface. Box3D with every
  capability off must still match IVP (RFC 0013 rule 5).
- **F3 Declared degradation.** Each capability declares, in its program row
  below and its contract record, what a caller does when the capability is
  absent: keep the legacy path, substitute a declared approximation (a capsule
  becomes a convex hull), or, when the product declares it required, refuse
  by name at composition. Nothing falls back after a partial mutation.
- **F4 Tested fallback.** Each capability's first consumer has a fallback
  clause that runs on IVP and passes: the consumer works on the legacy path,
  or refuses by name. Each phase's scenario map boots and runs headless on IVP
  as well as Box3D.
- **F5 Content.** Content authored for a capability loads on IVP through its
  declared degradation. Content that requires a capability declares that
  requirement, and loading it on IVP fails by name, never partially.
- **F6 Saves.** A save made with a capability on restores on IVP only when the
  capability declares its state degradable; otherwise the restore is refused
  by name before level publication (RFC 0004 persistence rules).
- **F7 Reference.** IVP remains the reference provider of
  `physics.conformance`, and its executed check count never shrinks
  ([RT2](#the-ratchets)).

## "Surpasses IVP": the scoreboard

The scoreboard is a set of named metrics, each with a direction (lower or
higher is better), a scene, a statistic, and a tolerance that is the metric's
measured run-to-run noise. Its owner is a `superiority` section of
[`quality/budgets/physics-v1.json`](../quality/budgets/physics-v1.json),
installed by B0; this RFC's table above is a dated snapshot, not a second
copy. Sources are the benchmark scenes (`physics_bench.py`), the shared
suite's analytic clauses (`physics_conformance.py`) and the headless scenario
map checks. Each phase adds its own metrics.

Box3D with capability set C **surpasses IVP on profile P** when:

1. B0 holds on P: the parity verdict passes with every capability off;
2. on every scoreboard metric, Box3D+C is no worse than IVP beyond the
   metric's tolerance;
3. every metric marked `headline` is strictly better than IVP beyond its
   tolerance;
4. the gameplay corpus scenes (RFC 0004) pass on both providers.

Today the claim fails at (1) and (2): the parity verdict fails, and
`panes-64` and the gyroscope sink are worse than IVP.

## Program

Each phase has one capability, one gate, one first consumer, one fallback and
one scenario map. Interfaces and gates are added to RFC 0013's catalog when
the phase starts; names below are proposed until then.

| Phase | Capability | Box3D API | First consumer | IVP fallback (F3) | Superiority metric | Scenario map |
| --- | --- | --- | --- | --- | --- | --- |
| B0 | none: solidify | — | `physics.conformance`, the ratchets | — | Parity verdict | all maps boot on both providers |
| B1 | `vphysics.continuous.v1` (RFC 0013 P4) | `b3Body_SetBullet`, speculative contacts | Thrown and launched props (player throw, `trigger_catapult`-style launches) through one game-side bullet policy | No-op: IVP's look-ahead, as today | `panes-64` and `phys_tunnel` lane C through count: 0, against IVP 24/64 and 4/8 | `phys_tunnel` |
| B2 | `vphysics.capsule-shapes.v1` | `b3CreateCapsuleShape` | USD-authored physics props with capsule collision (R59 physics role); the bench | One shared helper builds the capsule's convex hull through `IPhysicsCollision` | Rolling acceleration on an incline against the analytic value; hull ripple | `phys_rolling` (capsule lane added) |
| B3 | `vphysics.surface-motion.v1` | `b3SurfaceMaterial.rollingResistance`, `.tangentVelocity` | Round props; conveyors carrying physics props | No rolling resistance; conveyors keep moving players only | Stop distance against the analytic value for the declared resistance; belt speed of a carried box | `phys_rolling` |
| B4 | `vphysics.contact-events.v1` | Hit events (approach speed, point, normal, threshold), begin/end with manifolds, joint events | Impact sounds and impact damage | The existing `IPhysicsCollisionEvent` callbacks | Approach speed against sqrt(2gh) on drops; impacts per real bounce; the `audible-impacts` divergence explained | `phys_impacts` |
| B5 | `vphysics.joint-drive.v1` (RFC 0013 P6) | Joint motors, springs, limits, `b3Joint_GetConstraintForce`, force thresholds | `phys_motor`, breakable constraints | Legacy constraint and motor-controller paths | Spring frequency and damping against analytic values; break force within tolerance; joint stretch under a 50:1 mass ratio | `phys_joints` |
| B6 | `vphysics.sensors.v1` (RFC 0013 P6) | Sensor shapes and events | One trigger class | The game's touch-based triggers | Begin/end exactness and order across worker counts; cost against touch triggers | new `phys_sensors` |
| B7 | `vphysics.explosion.v1` (RFC 0013 P6) | `b3World_Explode` | `env_physexplosion` | The game's impulse loop | Impulse proportional to facing area and falloff, against analytic cases | `phys_impacts` |
| B8 | `vphysics.recording.v1` (RFC 0013 P5) | Recording, `b3ValidateReplay` | A bug-repro tool | Refused by name on IVP (tooling only) | Replay digest equals the recorded digest at 1 and N workers | any map |
| B9 | `vphysics.runtime-collision.v1` (RFC 0013 P7, with R59/R61/R45) | Mesh, height-field and baked-compound shapes | USD-native map collision | Cooked legacy collision from the content build, or refusal by name | Query and corpus suites | new map with R59 |

Notes per phase:

- **B0.** Make the parity verdict pass on the pinned source: fix the three
  gameplay failures (or revise the restitution patch with a recorded
  decision), and settle `audible-impacts` by finding its cause, not by
  widening its tolerance. Install the ratchet tool and its declaration, the
  scoreboard section, and the `physics.bench` baseline check RFC 0013 left
  open. Record every current value by name.
- **B1.** Start by finding why Box3D lets 56 of 64 through and IVP 24: whether
  the bullet flag, speculative distance or the 2000 in/s speed limit against
  a 30-unit step decides it. The game side gets one bullet policy owner (which
  objects are bullets, and when), not per-entity flags scattered through the
  game.
- **B2/B3.** The capsule fallback helper is the only caller-side place that
  branches on the capability. Rolling-resistance data needs an owner decision
  (see the open decisions); the first slice may take it per object.
- **B4.** Delivery stays on the thread that called `Simulate`, after the step
  (RFC 0013 contract rules); the filter audit must stay clean.
- **B5–B7.** Each needs analytic oracles that IVP can also run, so the
  scoreboard compares like with like.

## Scenario maps

Installed 2026-10-07: [`tools/quality/physics_lab_maps.py`](../tools/quality/physics_lab_maps.py)
builds and publishes five playable maps through the shared VMF compile
(`vmf_map_build.py`) and `playable_maps.py`, as `gyro_lab_map.py` does. Its
wiring test is `tools/quality/tests/test_physics_lab_maps.py`.

| Map | Scenes | Console readout | Phases |
| --- | --- | --- | --- |
| `phys_tunnel` | Three lanes of 8 cubes launched at 2000 in/s: 6-unit cubes vs a 2-unit static wall, 2-unit cubes vs a 1-unit static wall, 6-unit cubes vs three 1-unit hanging panes | `physlab tunnel <lane> through` per cube | B1 |
| `phys_stack` | 55-cube pyramid, 16-cube tower, 64 Portal cubes dropped in a bin | `physlab stack <scene> fell` per cube | scoreboard, B0 |
| `phys_joints` | 8 ragdolls dropped in a bin, a 12-link chain carrying 50 link masses, a phys_motor turntable | none yet | B5 |
| `phys_impacts` | Portal cubes dropped from 32 to 512 units; a ring of drums and crates around an explosion | none yet | B4, B7 |
| `phys_rolling` | Drums, exact 24-sided cylinders, melons and canisters down a ramp; a func_conveyor | none yet | B2, B3 |

How a user runs them:

```sh
python3 tools/quality/physics_lab_maps.py            # build and publish all five
./play phys_tunnel                                   # Box3D
PHYSICS=vphysics ./play phys_tunnel                  # IVP, the fallback
```

Every scene starts at map load and restarts from the button in front of it.
Walking up to a scene shows what it tests and what to look for. Headless:

```sh
python3 tools/quality/portal_boot.py --runtime run/runtime --build build \
  --content-root quality-results/physics-lab-maps/phys_tunnel/content \
  --map phys_tunnel --physics vphysics_box3d --headless \
  --console-command "fps_max 60" --capture-wait 600 --out /tmp/claude-1000/pl/tunnel
grep physlab /tmp/claude-1000/pl/tunnel/runtime/portal/console.log
```

Pass `--build`: without it, `portal_boot.py` runs the binaries staged in
`run/runtime`, which can be weeks older than the source.

Rules for phases:

- Each phase extends its map (or adds one) with the capability's scene and a
  console readout that turns the scene into a count, and records Box3D and
  IVP results headlessly.
- A proposed `physics_lab.py` runner (installed by B0 or B1) boots each map on
  both providers, parses `physlab` lines and writes the results as scoreboard
  inputs. Until it exists, the `portal_boot.py` command above is the
  reproduction.
- Maps stay deterministic generator output: no hand edits to VMFs, and the
  wiring test passes.

## What done looks like (instructions for agents)

Read this before starting or closing any R98 slice. AGENTS.md's working
protocol applies; this section adds the physics-specific obligations.

### A capability phase (B1–B9) is done when

1. **Interface.** `public/vphysics/<name>.h` declares the versioned interface
   (`VPhysics<Name>001`). Box3D returns it from `IPhysics::QueryInterface`;
   IVP returns NULL. RFC 0013's catalog row names the header and state.
2. **Contract.** `unittests/physicstest/contracts/vphysics.<name>.v1.md`
   records the clauses: inputs, results, lifetime, threading, failure and
   operation order. The clauses run in the conformance or `--bench contract`
   host.
3. **Bad providers.** Each clause has a host-injected fault that fails it, run
   by `physics_bench.py --sensitivity` or the conformance runner's faults. A
   clause that no fault can fail is not a clause.
4. **Gate.** The capability's gate in `quality/budgets/physics-v1.json` is
   `required` and passes on `linux-x86_64-desktop` with recorded evidence.
   Other profiles are listed as unverified, or measured.
5. **Superiority.** The phase's scoreboard metric is measured on both
   providers on the same inputs, recorded in the scoreboard, and Box3D with
   the capability is better than IVP beyond the metric's tolerance. A phase
   whose metric does not beat IVP is not done; record the result and keep
   the phase open.
6. **Fallback.** The F3 behavior is implemented in the consumer, and a
   fallback clause runs on IVP and passes (F4).
7. **Consumer.** At least one real game, tool or content consumer uses the
   capability through composition (`required` or `optional`), with a
   `-physics_<name>` opt-in and a `-physics_<name>_required` form, as
   `-physics_shape_inertia` does.
8. **Parity untouched.** `physics.conformance` with the capability off shows
   no new failure and no new divergence (RT1), and IVP's results are
   unchanged (RT2).
9. **Threading.** Any new callback runs on the thread that called `Simulate`,
   or is audited by `physics_filter_audit.py`. The contract passes under TSan
   at 4 workers.
10. **Scenario map.** The phase's scene is in its published map with a
    console readout, and headless runs on Box3D and IVP are recorded.
11. **Cost.** Step-time cost is measured (benchmark rows); where the
    capability touches gameplay frames, a complete-frame measurement on the
    frame-floor route is recorded (RFC 0016 and RFC 0003 obligations).
12. **Records.** RFC 0013's catalog row, this RFC's program table, the
    [progress record](0026-progress.md), and AGENTS.md's R98 row are updated
    in the same change, with what passed, what was unavailable and the
    reproduction commands.

### A phase is not done when

- the interface exists but no consumer uses it;
- the evidence comes from Box3D alone, without the same measurement on IVP;
- a tolerance, a `REFERENCE_DEFICIENCIES` entry, a golden or a recorded
  ratchet value was loosened to get green;
- the gate is `planned`, has a declared known gap, or ran on a contended host
  and its timing rules reported not-run;
- the fallback was not run on IVP;
- the scene exists only in a synthetic benchmark and not in a published map.

### B0 is done when

- `physics.conformance` passes as a whole on the pinned Box3D source: zero
  failing checks and zero divergent observations, or each remaining
  divergence carries a reviewed user decision;
- `quality/baseline.json` records `physics.conformance` as `pass`;
- the ratchet tool, its declaration and its self-tests with seeded violations
  are installed, run in `conformance.yml`, and appear as a baseline check;
- the scoreboard section exists with every metric in the snapshot above,
  measured on both providers;
- `physics.bench` is a baseline check;
- all five scenario maps boot headless on both providers through the
  proposed `physics_lab.py` runner.

### R98 (the program) is done when

- B0–B8 are done on `linux-x86_64-desktop` and `portal-android-native-vulkan`
  (the Fold7). Apple profiles are optional (user decision, 2026-09-25) and are
  reported unverified when unmeasured. B9 is done with R59/R61/R45, or
  recorded as deferred to them by the user;
- the scoreboard claim holds on both required profiles for the capability set
  each profile enables;
- every profile that ships Box3D boots its scenario maps and its gameplay
  corpus on IVP (F1, F4);
- a gameplay soak runs on each provider on each required profile.

### Working rules

- Measure first. Reproduce the metric on both providers before changing
  anything, and keep the before numbers.
- Use the installed commands: `physics_conformance.py`, `physics_bench.py`
  (`--sensitivity`, `--wait-quiet`), `physics_filter_audit.py check`,
  `physics_lab_maps.py`, `portal_boot.py --build <tree>`. The ratchet tool
  and `physics_lab.py` are proposed until B0 installs them; do not cite them
  as evidence before then.
- Never loosen a parity tolerance, an IVP oracle or a ratchet value to pass.
  A value moves in the bad direction only with a recorded user decision.
- Keep Box3D changes in the pinned fork with a recorded revision, never as an
  uncommitted submodule patch (the 2026-09-24 restitution finding).
- Timing needs a quiet host (`max_load_per_cpu`); benchmarks that the user
  wants on dedicated hardware run there.
- One owner per policy: the bullet policy, the capsule fallback helper and the
  rolling-resistance data each get one named owner.

## The ratchets

The ratchets keep the program from sliding back while it moves forward. Each
is a recorded value that may only move in its good direction. The checker is
proposed as `tools/quality/physics_ratchet.py` with its declaration
`quality/physics_ratchet.json`, installed by B0. It reads evidence that the
installed runners write (`physics_conformance.py`, `physics_bench.py`, the
scenario-map runner); it does not run them. `check --static` validates the
declaration alone and runs in CI; `check --evidence <dir>` compares fresh
evidence and runs at merge gates and as the `physics.ratchet` baseline check.

| ID | Ratchet | Direction | Recorded starting value | Fails when |
| --- | --- | --- | --- | --- |
| RT1 | Parity debt: Box3D failing checks plus divergent observations in `physics.conformance`, every capability off | shrink only; target 0 | 3 failing checks and 3 divergences on the pinned fork (2026-09-25); B0 records them by name | a new failure or divergence appears, or a recorded one changes name without review |
| RT2 | IVP coverage: checks IVP executes and passes in `physics.conformance` | grow only | 603 checks on `build-p2` (2026-09-25); B0 re-records | IVP runs fewer checks, or fails one it passed (except `REFERENCE_DEFICIENCIES`) |
| RT3 | Scoreboard: each metric's Box3D value; the set of metrics where Box3D is at least as good as IVP | each value no worse than recorded beyond its tolerance; the set only grows | the snapshot above (9 of 11 at least as good: 8 better, 1 equal) | a metric regresses, or leaves the set |
| RT4 | Capability ledger: each catalog capability's stage (planned, contracted, gated, consumed, profile-enabled) | forward only | parallel-step and shape-inertia consumed; step-profile gated; the rest planned | a stage moves back; a consumer uses a capability whose gate is not `required`; a profile enables a capability whose gate fails there; a capability lacks a declared fallback, fallback clause or scoreboard metric |
| RT5 | Fault coverage: injected faults run and detected | grow only | 15 in `physics_bench.py`, 19 in `physics_conformance.py` | fewer faults run, or one goes undetected |
| RT6 | Fallback coverage: scenario maps and product profiles booted headless on IVP | grow only | 5 maps, each booted headless on IVP and Box3D (2026-10-07, `portal_boot.py`) | a map or profile stops booting on IVP |

Update policy:

- A good-direction change is recorded in the same commit as the evidence that
  shows it (lower RT1, higher RT2, RT5 and RT6, a better RT3 value, a later
  RT4 stage).
- A bad-direction change needs a user decision named in the declaration's
  `reviewed` field. Agents never make one on their own.
- Stale entries (a ratchet value better than the evidence can show, or a
  metric that no longer exists) fail until reviewed, as the filter audit's
  stale entries do.
- The ratchet tool ships with self-tests that seed one violation per rule and
  require each to fail.

## Composition and selection

Unchanged from RFC 0013: the application root opts in per product profile,
environment-scoped choices are fixed when the environment is created, and
missing capabilities are optional or fail composition by name. The launcher
argument pattern is `-physics_<name>` and `-physics_<name>_required`. A
capability becomes a profile default only after B0 and its own phase are
done on that profile, and the user agrees.

## Non-goals

- Cloth or soft bodies (Box3D has none; a separate RFC would own one).
- Box3D's character mover, or a capsule player hull (RFC 0004: Source player
  movement is preserved).
- Double-precision worlds or coordinates beyond ±16384 units.
- Deterministic lockstep networking, prediction or rollback.
- Retiring IVP (decision 1).
- Changing IVP's behavior to make Box3D look better.

## Risks and mitigations

| Risk | Effect | Mitigation |
| --- | --- | --- |
| Capabilities change gameplay feel on maps built for IVP | Puzzles or speedruns behave differently | Opt-in per profile; IVP fallback; gameplay corpus on both providers before any default |
| Superiority measured on synthetic scenes only | A claim that does not hold in game | Each metric also has a scenario-map readout; complete-frame measurements |
| Content starts to depend on Box3D-only behavior | IVP fallback breaks silently | F3/F5 declared degradation; RT4 and RT6 |
| B0's parity failures are deep solver differences | B0 stalls | B1+ may proceed; defaults and claims wait; record the decisions needed |
| Ratchets freeze bad numbers | Debt is never paid | Shrink-only values with a target; stale entries fail |
| Stale staged binaries in harness runs | False bugs or false passes | Always pass `--build`; evidence records executable hashes |

## Alternatives considered

- **Retire IVP once Box3D passes parity.** Rejected by the user: IVP stays as
  the fallback.
- **Make Box3D the reference and loosen parity to it.** It discards the
  oracle that keeps the fallback honest.
- **Add the features to `IPhysicsEnvironment`/`IPhysicsObject`.** It breaks
  frozen vtables for game DLLs and mods.
- **One "Box3D extensions" interface.** A capability bag; AGENTS.md forbids
  it, and it cannot carry per-capability contracts.
- **Synthetic benchmarks only, no maps.** Users could not see or check the
  result, and in-game behavior (game policies, entity I/O, triggers) would go
  unmeasured.

## Open decisions

- Where rolling-resistance and conveyor data live: a new optional
  surface-property key (ignored by IVP; a content-format decision) or per
  object only.
- The capsule's authoring path: USD collision first; a `.phy`/model-format
  extension waits for R62.
- The bullet policy: game-marked objects, a speed threshold, or both.
- Hit-event thresholds and how they map to impact sound and damage tables.
- R98's rank (an agent placement, movable by the user).

## Roadmap

R98 in [AGENTS.md](../AGENTS.md) tracks this RFC, placed directly after R67.
Prerequisites: R19 (the Box3D slice) and R67 (the capability mechanism).
B9 also depends on R59, R61 and R45. R44 is narrowed by decision 1: it closes
when no declared profile depends on IVP simulation, with IVP still built and
selectable.

## Source references

- Box3D pinned source: the `box3d` submodule, fork `johnpanos/box3d`
  `78c90a0` (upstream `9e5a4cd` plus the restitution patch).
- Box3D announcement and FAQ: <https://box2d.org/posts/2026/06/announcing-box3d/>,
  <https://github.com/erincatto/box3d/blob/main/docs/faq.md>.
- Provider: `vphysics_box3d/`. Harness: `unittests/physicstest/`,
  `tools/quality/physics_conformance.py`, `tools/quality/physics_bench.py`,
  `quality/budgets/physics-v1.json`.
- Maps: `tools/quality/physics_lab_maps.py`, `tools/quality/gyro_lab_map.py`.
