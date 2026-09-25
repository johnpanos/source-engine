# RFC 0004 progress: Box3D Primary Physics Backend

Updated: 2026-09-25 (status reconciled with the tree)
Source revision at assessment: `d6260d90` (earlier entries: `87955f67`; AGENTS.md portfolio rows: R09, R19, R31, R34, R37, R44, R45)

This file is the human-readable, durable progress record for RFC 0004. It tracks the phased implementation of replacing IVP/Havana with Box3D as Source's primary rigid-body physics backend.

## Status

Current state (2026-09-25, read from the tree; nothing rerun):

- The parity runner does not pass as a whole in the latest recorded runs. It
  fails on any divergent observation (`physics_conformance.py`, default
  `--require all`).
  - Upstream Box3D `9e5a4cd` (worktree, 2026-09-24): 602 of 602 checks pass,
    but `dynamics.tumble.audible-impacts` diverges (IVP 4, Box3D 1, tolerance
    ±2).
  - The restitution patch ("only a touching point bounces") is now the pinned
    source (user decision, 2026-09-25): commit `78c90a0` on branch
    `source-engine-restitution` of the fork `johnpanos/box3d`, which
    `.gitmodules` now names. With it, three Box3D gameplay checks fail
    (`vcollide.model-simulates`, `dynamics.tumble-travel-bounded`,
    `dynamics.held-floor-quiet`; R01 re-audit, 2026-09-25).
  - `quality/baseline.json` records `physics.conformance` as a known `fail`
    owned by R19 (user decision, 2026-09-25).
- Provider selection: the launcher and the dedicated server default to
  `-physics vphysics` (IVP), and Waf's `--physics-backend` defaults to `ivp`.
  `./play`, `./play_p2` and `run.sh` select Box3D. The Android launcher
  (`launcher_main/android_main.cpp`) passes no `-physics`, so the APKs, which
  package both providers, run IVP unless `commandline.txt` selects Box3D.
- `vphysics_box3d` links no IVP library and includes no IVP header. Its
  `.phy`/BSP decoder is `legacy_collision.cpp`; it shares only the keyvalue
  parser `vphysics/vcollide_parse.cpp`. `vbsp` and `studiomdl` still load
  the IVP `vphysics` module.
- CI runs the physics runner unit tests and the filter audit
  (`conformance.yml`), not the parity suite or the benchmark.
- Parallel stepping on the engine pool is the server default when Box3D is
  selected ([RFC 0013](0013-progress.md#default-on-2026-09-25-user-decision)).
  The client environment keeps one worker.

Roadmap criteria against this record (2026-09-25):

| Row | Has evidence | Missing |
| --- | --- | --- |
| R09 | Impact state (pre-step velocities, `events.*`), contact mutation (`contacts.*`), asymmetric ragdoll limits (`constraint.ragdoll-*`), decoder over every Portal and HL2 `.phy` (`corpus.*`), with convexes over Box3D's hull limits split into patch hulls (`BuildHulls`); synthetic IVP measurements (`physics-v1.json`) | Method-level consumer inventory; map/gameplay and load-time IVP measurements |
| R19 | BSP and `.phy` load, compound props, inside-start traces, impact events, one ragdoll, save/restore (`bsp.*`, `vcollide.*`, `trace.*`, `events.*`, `save.*`/`restore.*`) | The pinned patched source's three gameplay failures fixed, or the patch revised; a passing runner; a CI lane |
| R31 | Provider-level traces, filters, events, materials, constraints, controllers and persistence | Client/dedicated gameplay corpus; the dedicated builds pass since 2026-09-25, but installed dedicated startup (R12) is unverified |

Evidence (2026-09-24): `python3 tools/quality/physics_conformance.py --out <dir>`
passes. IVP and Box3D each pass 559 checks (149 boot, 410 gameplay); 12,373
observations agree; all 17 sensitivity faults are detected. The contract and its
known gaps are in
[`vphysics.provider.v1`](../unittests/physicstest/contracts/vphysics.provider.v1.md).
`portal_boot.py --physics vphysics_box3d` passes on `testchmb_a_00`, and Box3D is
the default physics in `./play` (`PHYSICS=vphysics` selects IVP). This closes no
roadmap gate: there are no performance budgets, no CI lane, no dedicated-server
or non-Linux profile, and no gameplay soak.

Surface-property race (fixed 2026-09-25): Box3D's surface database used a
plain `CUtlSymbolTable`, whose `Find` writes a shared search context. Pooled
particle traces reach `GetSurfaceIndex` on several workers at once
(`CEngineTrace::ClipRayToCollideable`), so a lookup could return another
thread's index; a queued-mode TSan run of Portal found it. The table is now a
`CUtlSymbolTableMT`, as IVP's is. The shared suite's new
`surfaceprops.concurrent-lookup` (four threads, 200,000 lookups) failed Box3D
before the fix (54,724 wrong) and passes both providers after it; the bad
provider `surfaceprops-unsynchronized` is detected. The gate on `build-p2`:
IVP 603/603; Box3D 600/603, whose three gameplay failures
(`vcollide.model-simulates`, `dynamics.tumble-travel-bounded`,
`dynamics.held-floor-quiet`) come from the shared tree's uncommitted
`box3d/src/contact_solver.c` patch and fail the same way without this fix;
all 18 sensitivity faults are detected.

Gameplay regression fixes (2026-09-22): with Box3D, walking into props left the
player stuck, and walking into a portal stopped the player short of it. Both came
from provider traces and the player controller, not from the game:

- `TraceBox` swept boxes with Box3D's shape cast. It reports any start within its
  linear slop (about 0.25 inch) as a hit at fraction 0 with no normal. Movement
  leaves the player `DIST_EPSILON` from what it touched, so every later trace was
  blocked. Inside a portal environment, movement traces the carved local-world
  collides, so the floor alone blocked it. Sweeps now follow IVP's
  `CTraceSolver::SweepSingleConvex`: the swept volume is tested first, a sweep that
  stays apart hits only while closing on the surface, and a sweep that reaches the
  hull advances conservatively to `DIST_EPSILON`.
- `startsolid` used `b3OverlapHull`, which also counts anything within 0.02 inch.
  It now requires actual penetration, as IVP does.
- Box3D's soft contacts pitch a pushed prop by about 2e-4 rad. The slide then rose
  slightly, and the game treated that as a step and stopped pushing. Swept traces
  now report planes within 0.005 of vertical as vertical.
- The friction snapshot listed speculative contact points up to 0.8 inch away, so
  the player controller treated a wall and a prop it had not reached as a crease
  and froze. It now lists points within IVP's 0.25 inch collision tolerance, plus
  points that pushed this step.

Evidence:

- Seven new `trace.box-*` checks cover the resting-contact cases. IVP passes them.
  The pre-fix Box3D module fails six.
- A local headless Portal movement repro on `testchmb_a_01` compared IVP and
  Box3D trajectories: portal walk-through, free box push, and box-against-wall
  pushes. The portal path is identical to IVP. Box3D pushes the box about 20–30%
  slower than IVP and never sticks. The repro is not installed as a tool; a
  Q-PHYSICS gameplay-scene runner is still open.
- No CI lane runs these checks yet.

Portal-under-prop fix (2026-09-23): with Box3D, a prop resting where a floor
portal opened stayed on the portal instead of falling through. The portal
simulator takes ownership of the prop, and the game's collision rules then reject
the prop/world pair. The game applies this through `CollisionRulesChanged` →
`RecheckCollisionFilter`, `Wake`, `RecheckContactPoints`. Box3D consults the
rules only when a contact is created, and `b3Shape_SetFilter` ignores unchanged
filter bits, so the existing floor contact survived.

`RecheckCollisionFilter` now re-runs the rules on the object's pairs, as IVP's
`recheck_collision_filter` does:

- Candidates come from a broadphase query of the body bounds, inflated past the
  fat-AABB margin, so they include non-touching contacts.
- A pair the rules now reject is held off through the snapshot-deletion
  pre-solve list until the rules allow it again. Contacts that are still allowed
  are left alone, so they produce no extra touch or impact events.
- A dynamic object that overlaps an allowed partner without a touching contact
  (the rules rejected the pair when it formed) has that shape's proxy reset, so
  Box3D pairs it again.

Evidence:

- Four new `rules.recheck-*` checks: resting contact dropped, allowed contact
  kept, near pair dropped, and pair restored. IVP passes all four; the pre-fix
  Box3D module fails three.
- A local headless `testchmb_a_02` repro spawned a cube, opened a floor portal
  under it with `NewLocation`, and logged the result with
  `sv_portal_debug_touch`. IVP and the fixed Box3D teleport the cube; the pre-fix
  Box3D never does. The repro is not installed as a tool.

Player speed budget after a game-set velocity (2026-09-24): in Portal 2, turning
noclip off inside geometry left the player about 9 units lower on Box3D than on
IVP. `CBasePlayer::SetVCollisionState` teleports the shadow and sets its
velocity, then `Update` passes the movement speed as the controller's per-axis
budget. IVP's `SetVelocity` zeroes the core's speed and holds the new velocity in
`speed_change` until the next step, and `MaxSpeed` reads only the core's speed.
So IVP removes at most the offered speed per axis, and the shadow keeps the rest
for that step. Box3D's `MaxSpeed` read the full new velocity, got a budget larger
than it, and stopped the shadow. The stuck check then pulled the player back to
the shadow for several ticks.

Box3D objects now record the velocity the game set or added since the last step
(`SetVelocity`, `AddVelocity`, `ApplyForceCenter`/`ApplyForceOffset`;
`SetVelocityInstantaneous` records none, as in IVP). `PreStep` commits it, and
`MaxSpeed` budgets against the committed velocity. The same path runs on every
collision-state change: noclip off, ducking and portal teleports.

Evidence:

- New `player.set-velocity-budget` check and `player.set-velocity-step`
  observation. IVP passes; the pre-fix Box3D module fails, stopping the shadow
  instead of keeping (325, -425, 525).
- Headless `sp_a2_triple_laser` noclip-off in geometry: after the fix, Box3D
  follows IVP within 0.15 units. Before, it ended about 9 units lower. The 0.15
  is Box3D applying gravity after the controller. Fall, walk, stop, jump and
  noclip runs match IVP within 0.09 units.
- A cube push on that map varies from run to run in this harness. IVP's sampled
  path varies, but its endpoint repeats. In three runs each, the fixed Box3D
  ended at IVP's endpoint twice and 1.1 units off once. The pre-fix module ended
  there once and up to 14 units off otherwise. The source of Box3D's endpoint
  variance was not found; the world uses one worker.

Benchmark gate and Box3D source finding (2026-09-24,
[RFC 0013 progress](0013-progress.md)):
`tools/quality/physics_bench.py` compares IVP and Box3D on piles, stacks,
ragdolls and projectile scenes with calibrated limits. It also found that the
shared tree's `box3d/src/contact_solver.c` carries an uncommitted restitution
patch that the pinned submodule commit does not record. With the pinned source,
Box3D passes all 602 checks, and 1 observation diverges
(`dynamics.tumble.audible-impacts`). With the patch, three gameplay checks fail.
Those are the three failures earlier noted as pre-existing on main's build.

| Work item | Phase | Status | Evidence |
| --- | --- | --- | --- |
| Interface & Consumer Inventory | A | Partial | Every VPhysics interface implemented and covered by the contract; no method-level consumer inventory |
| Asset Corpus & IVP Baselines | A | Partial | Conformance corpus: every `.phy` in the Portal and HL2 packs, `testchmb_a_00.bsp`, HL2 vehicle scripts, with IVP as the oracle; no performance baselines |
| Event & Contact-mutation Prototypes | A | Done (provider) | Pre-step velocities for `PreCollision`; snapshot contact deletion via pre-solve (`contacts.*`) |
| Box3D Build Integration (Pinned) | B | Done | `box3d/` pinned, private C17 target; since 2026-09-25 the pin is the fork `johnpanos/box3d` `78c90a0` (upstream `9e5a4cd` plus the restitution patch) |
| VPhysics Adapter & World Lifecycle | B | Done | `vphysics_box3d` module, `-physics` selection |
| Legacy Geometry Decoder | B | Done | `.phy`/BSP decode (`vcollide.*`, `corpus.*`, `bsp.*`), IVP-format writer (`collide.write-*`) |
| Trace & Query Foundation | B | Done | `trace.*`, `bsp.world-traces` |
| One-ragdoll & Impact Event | B | Done | Chell ragdoll fixture, `events.*` |
| Constraints & Player Controllers | C | Done | `constraint.*`, `player.*`, `shadow.*`, `motion.*` |
| Persistence & Materials | C | Done | `save.*`, `restore.*`, `vehicle.restore-*`, `surfaceprops.*` |
| Fluids & Vehicles | D | Done (gaps recorded) | `fluid.*`, `vehicle.*`; the raycast car type and wheel side-friction are listed gaps |
| Tool Workflows (`studiomdl`, `vbsp`) | D | Partial | `CollideWrite` emits the legacy format; tools not rebuilt against Box3D |
| Parallel Scheduler Integration | E | Partial | [RFC 0013](0013-opt-in-physics-capabilities.md) P1–P3: `vphysics.parallel-step.v1` on the engine pool; the server steps in parallel by default when Box3D is selected (2026-09-25); client one worker; Linux desktop only; pool-bridge timing not certified on a quiet host |
| Performance Budgets & Packaging | E | Partial | Synthetic-scene budgets and IVP comparison for Linux desktop in `quality/budgets/physics-v1.json` ([RFC 0013 progress](0013-progress.md)); no gameplay or mobile budgets |
| Independent Collision Cooking | F | Not started | `vphysics_box3d` decodes `.phy`/BSP without IVP code; `vbsp` and `studiomdl` still load the IVP `vphysics` module; no dependency audit |
| IVP Simulation Retirement | F | Not started | — |

## Scoped Actionable Tasks

### Phase A: Inventory and Feasibility (R09)
* **A1. Interface & Consumer Inventory:** Map all `IPhysics*` interface usages across engine, game DLLs, and tools. Produce a method-level inventory recording consumers, implementation strategy, tests, and profile requirements.
* **A2. Asset & Content Corpus Selection:** Identify the first supported game configuration (e.g., Linux x86-64 HL2-family). Collect representative maps, problematic assets (oversized hulls, multi-material meshes), and save games.
* **A3. IVP Baseline Measurements:** Measure memory usage, load time, and median/p95/p99 frame execution time for the content corpus using the existing IVP backend. Set explicit performance budgets.
* **A4. Pre/Post Collision State Prototype:** Build a prototype resolving the callback timing difference between Source (reads velocities in `PreCollision`) and Box3D (buffers contacts). 
* **A5. Contact Mutation Prototype:** Implement and verify a strategy for `IPhysicsFrictionSnapshot` mutations (clearing friction impulses, deleting contacts).
* **A6. Ragdoll Limits Analysis:** Fit Source's 3-axis ragdoll limits to Box3D's spherical joint cone/twist limits and determine if a solver extension is required.

### Phase B: Runtime Vertical Slice (R19)
* **B1. Box3D Build Integration:** Pin Box3D (`9e5a4cde...`), configure C17 compilation via Waf under a private target, preserving standard/FP settings without inheriting Source's C++ flags.
* **B2. Adapter & World Lifecycle:** Implement `vphysics_box3d` export, `IPhysicsEnvironment` wrapping a Box3D world, and provider selection via `--physics-backend`.
* **B3. Legacy Geometry Decoder:** Implement a decoder to read legacy `.phy` and BSP physics lumps to generate Box3D hulls, baked compounds, and static meshes, preserving IDs and surface properties.
* **B4. Trace Compatibility:** Implement basic raycasts and swept AABB traces for `IPhysicsCollision`, adding explicit handling for inside-starts, backfaces, and exit fractions.
* **B5. Single Dynamic Prop & Event Bridge:** Spawn a compound dynamic prop, deliver a verified impact event using Phase A's prototype, and ensure basic sleep/wake states match.
* **B6. One Ragdoll & Basic Save:** Spawn a single ragdoll utilizing the Phase A limits prototype. Implement a basic save/restore round trip for supported objects.

### Phase C: Core Compatibility (R31)
* **C1. Advanced Traces & Filters:** Complete `IConvexInfo` filtering, displacement surface collision, compound model traces, and exact clipping tolerances.
* **C2. Constraint Completion:** Implement hinge, sliding, fixed, ball socket, and length joints with tested equivalence for limits, motors, and break behavior.
* **C3. Shadow & Player Controllers:** Port `vphysics/physics_shadow.cpp`. Map shadow controllers and player pushes to Box3D kinematic/dynamic paradigms.
* **C4. Motion Controllers:** Implement local/global force and acceleration controllers with the correct step cadence and attachment lifetime rules.
* **C5. Full Persistence:** Finalize `Save`/`Restore` protocol. Ensure identity remapping, controller reconstruction, and strict dependency-order restoration.
* **C6. Materials & Object Lifecycle:** Finalize deferred deletion, cross-world `TransferObject`, and exact mapping of mass/drag/contents metadata.

### Phase D: Gameplay Completion (R34)
* **D1. Fluids & Buoyancy:** Implement fluid volume sensors, submerged volume computation, drag, buoyancy overrides, and touch events.
* **D2. Vehicles:** Implement all raycast and wheel-joint vehicle modes, covering steering curves, suspension, traction, script reloading, and water interactions.
* **D3. Complex Constraints:** Implement pulley semantics (coupled rope length/gearing) and constraint groups with group-specific solver iteration policies.
* **D4. Game Variants & Portal Support:** Validate specific variants like Portal object transfer, shadow clones, and related collision rules.
* **D5. Tool Workflows:** Update `studiomdl`, `vbsp`, and inspection utilities to seamlessly handle Box3D representations and size limits.

### Phase E: Performance & Rollout (R37)
* **E1. Parallel Scheduler Integration:** Bridge Box3D's worker system to Source's job system (RFC 0003), ensuring deterministic worker counts and preventing deadlocks.
* **E2. Platform Expansion:** Expand validation checks and builds to Windows x86-64, macOS, iOS, and Android.
* **E3. Profiling & Optimization:** Optimize against the Phase A baseline budgets. Confirm memory, load time, and frame cost meet targets.
* **E4. Soak Testing:** Run extended multi-player and client/server dedicated sessions to verify stability, networking determinism, and authority.
* **E5. Packaging & Rollback:** Finalize the `--physics-backend=ivp\|box3d` packaging, guaranteeing a safe rollback path with documented limitations.

### Phase F: Retirement (R44, R45)
* **F1. Native Cooking Pipeline:** Define a new versioned collision format. Support pre-cooking Box3D native geometry to avoid runtime legacy conversion overhead.
* **F2. Independent Legacy Decoder:** Decouple the legacy `.phy` parser completely from the old IVP implementation.
* **F3. IVP Simulation Removal:** Remove the IVP runtime simulation once all profiles have successfully promoted Box3D as the default.

