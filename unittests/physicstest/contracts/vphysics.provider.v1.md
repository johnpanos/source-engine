# Contract: `vphysics.provider.v1`

Module family: VPhysics providers (`vphysics` = IVP, `vphysics_box3d` = Box3D)
Interfaces: `IPhysics` (`VPhysics031`), `IPhysicsCollision` (`VPhysicsCollision007`),
`IPhysicsSurfaceProps` (`VPhysicsSurfaceProps001`) — `public/vphysics_interface.h`
Conformance suite: `unittests/physicstest/test_vphysics_*.cpp` (host and
reporting in `test_vphysics_conformance.cpp`, shared fixtures in
`vphysics_conformance.h`; Waf target `vphysics_conformance`)
Runner: `tools/quality/physics_conformance.py` (evidence `physics-conformance-evidence/v1`)
Roadmap: RFC 0004 Phases B–D (R19, R31, R34), RFC 0005 Q-PHYSICS

This is the behavioral contract a VPhysics provider must meet before the engine
can select it for a product. Every provider claiming it runs the **same**
suite, loaded through its exported `CreateInterface` exactly as the engine
loads it. IVP is the legacy oracle: a check that fails on IVP is a wrong
check and is fixed in the suite, not waived.

## 1. Tiers

- **boot** — what a map load needs to finish without crashing: module exports
  and sibling-interface resolution, the authored surface database, object-pair
  hashes and collision sets, `.phy` loading and keyvalue parsing, object and
  controller creation with coherent bookkeeping.
- **gameplay** — correct physical behavior: decoded collision geometry, traces,
  simulation, contact/sleep events, collision rules, controllers and constraints
  that actually move objects.

A product may select a provider only when it passes every tier the product
requires and its observations agree with the oracle. Passing **boot** alone
proves a map loads; it does not prove the game plays.

## 2. Clauses (grouped by check prefix)

| Prefix | Clause | Legacy consumers |
| --- | --- | --- |
| `module.` | Exports all three interfaces; `IPhysics::QueryInterface` returns the same sibling pointers; `Connect`/`Init` succeed | AppSystem group, engine/game factories |
| `surfaceprops.` | Parses the manifest-listed files; `default` is index 0; unknown names are -1; names round-trip; `GetSurfaceData` is never null for any index; re-parsing a file is a no-op returning 0; `$MATERIAL_INDEX_SHADOW` is reserved (0xF000, friction 0.8, elasticity 0.001); sound names resolve | player `CategorizePosition`, footsteps, impact sounds |
| `pairhash.` | Pair lookups are symmetric; counts/lists track add/remove; removing all pairs for an object clears it | `g_EntityCollisionHash`, `CTraceFilterEntity` |
| `collisionset.` | Sets are stable per id; every pair starts **disabled**; enable/disable are symmetric; destroy-all clears | ragdoll self-collision rules |
| `collide.` | Box collides report exact AABBs (including rotated), support-map extents and volume; convex, hull and query-model paths build usable geometry; debug meshes lie inside the AABB | `CCollisionProperty`, tools, debug overlays |
| `trace.` | `TraceBox`/`TraceCollide` hit at the analytic fraction with the correct normal, miss cleanly, report `startsolid` only when starting inside (not merely within `DIST_EPSILON`), let a hull resting `DIST_EPSILON` from a surface move away, along, or past it, honor swept extents and rotation, and always write the trace | engine `ClipRayToVPhysics` (player, bullets, portal placement against props) |
| `vcollide.` | `VCollideLoad` yields every solid and the trailing keyvalue text; decoded solids have finite non-degenerate AABBs, positive volume, hittable upward faces and clean misses; unload zeroes the record | `mdlcache`, static props, ragdolls |
| `keyparser.` | Every solid has a parsed `solid` block; a multi-solid ragdoll has `solidCount - 1` valid `ragdollconstraint` blocks | prop/ragdoll creation |
| `env.` / `object.` | Environment settings round-trip; objects keep game data, mass, material, collide and transforms; matrices and local/world conversions agree; teleport is immediate; flags round-trip; object lists track create/destroy; the simulation clock advances by the stepped time | entity physics sync |
| `shadow.` / `motion.` | Shadow controllers reach their targets, honor rotation/translation locks, restore object state on detach, and stay finite with a non-rotating heavy shadow; motion-controller forces/accelerations apply in the right frame | player/NPC shadows, held objects, physcannon/portal gun |
| `constraint.` | Hinge (swing, soft limits, motors), ballsocket, sliding (axis, limits, motor), length (rope, slack, rigid), pulley, ragdoll (3-DOF limits, hinge conversion), fixed; breakable limits break (and report) or hold; groups activate their constraints together; object deletion ends constraints | doors, ropes, ragdolls, props with hinges, elevators |
| `sim.` | Free fall under gravity, static and motion-disabled objects stay put, active-object lists contain moving objects, velocities integrate, impulses apply, dynamic objects come to rest on static floors | `PhysFrame`, entity position updates |
| `object.` / `drag.` | Mass/inertia (including `.phy` rotational inertia and IVP's limits), damping, drag bases and drag force, energy, force/torque helpers, motion/gravity/drag modes | prop physics, `physics_impact_damage` |
| `events.` / `contacts.` / `rules.` / `trigger.` / `lifetime.` | Pre/post collision events arrive in matched pairs naming both objects; touch start/end (static partners only with `CALLBACK_GLOBAL_TOUCH_STATIC`); friction scrapes; resting objects sleep and report it; friction snapshots list the touching object; `ShouldCollide` filters pairs, and a contact deleted through a snapshot drops for a step and returns only if the rules allow it; `RecheckCollisionFilter` re-applies changed rules to existing and nearby pairs (dropping rejected contacts, restoring allowed ones) without disturbing contacts that stay allowed; trigger volumes report overlaps; deferred deletion inside callbacks; `TransferObject` | impact damage/sounds, sleep networking, portal collision rules, triggers |
| `fluid.` / `spring.` | BSP-style fluid controllers: surface plane, density/contents, buoyancy (objects float and sink by density), damping, current, fluid touch events, opt-out; springs pull to their natural length, only-stretch mode, endpoints | water, `phys_spring` |
| `save.` / `restore.` | Objects (transform, velocity, properties, pinned/asleep, shadow), constraints, groups, springs and motion controllers save and restore through `ISave`/`IRestore` with pointer remapping; restored links work | save games, level transitions |
| `player.` | The player controller tracks its target within per-axis speed budgets, teleports past the 24-unit error bound (unless the handler refuses), clamps push velocity at over-limit contacts, reports ground-relative velocity, survives ground deletion, steps up, and attaches/detaches like IVP (flag, damping, drag) | `CBasePlayer` physics shadow |
| `vehicle.` | HL2's jeep and airboat scripts on their real bodies: the controller creates wheels, settles, drives (gearbox, speed), steers, brakes, boosts, reports wheel contacts and debug rays; the airboat floats and drives on water and land; body deletion shuts the car system down; the vehicle saves and restores with its wheels | `CFourWheelVehiclePhysics` (jeep, buggy, airboat) |
| `polysoup.` / `virtualmesh.` / `bboxcache.` / `cone.` / `collide.write-*` | Triangle collides from polysoups and displacement-style virtual meshes support objects and (polysoups) traces; identical boxes share one cached collide that survives `DestroyCollide`; box/cone queries; `CollideWrite` emits the legacy compact-surface format (checked by an independent reader against IVP's own writer) and reads back unchanged | static props, displacements, `vbsp`/`studiomdl`, memory reports |
| `bsp.` | A shipped map's world collision (the BSP physics lump) decodes and traces identically | map load (`CM` world physics) |
| `stats.` | Simulation counters accumulate and clear | `physics_report`-style diagnostics |

## 3. Observations and tolerances

The suite also prints observations (`OBS <key> <tolerance> <values>`) that the
runner compares against the oracle: the full surface-property table and parsed
keyvalues (exact), decoded geometry (AABB ±0.5 units, volume ±5%, mass center
±0.5, rotational inertia ±10%, convex count exact), trace contact heights
(±0.5–1), free-fall distance (±25%), map world traces (576 samples, ±1),
serialized ledge/triangle/point counts (exact), controller outcomes (player
walk distance ±4, clamped push speed ±5) and vehicle outcomes (ride height ±4,
drive distance/speed ±35%, turn direction exact). Trajectories are not required
to match IVP beyond these; the other simulation clauses are qualitative
ranges.

## 4. Fixtures

The runner extracts real content from the staged runtime in `gameinfo.txt`
search order (Portal VPK, Portal, HL2 VPK, HL2): the surface-property manifest
and its files; the collision models for the weighted cube, the floor button
base, the turret and Chell's 23-solid ragdoll; HL2's `jeep_test.txt` with the
buggy body and `airboat.txt` with the airboat body; and `testchmb_a_00.bsp`
for world collision. Corpus mode sweeps every `.phy` in the Portal and HL2
packs. Game content is never committed.

## 5. Sensitivity

Each fault below is injected into the oracle provider (a decorator over the
real interface, or suppression at the `Simulate` call boundary); the runner
fails unless the targeted checks fail:

`surfaceprops-null`, `surfaceprops-index`, `pairhash-ordered`,
`collision-unit-aabb`, `collision-trace-miss`, `vcollide-keyvalues`, `sim-noop`,
`constraint-inactive`, `fluid-inert`, `spring-inert`, `events-silent`,
`drag-off`, `save-dropped`, `soup-null`, `collide-write-stub`, `player-inert`,
`vehicle-stub`.

## 6. Legacy behaviors the suite pins down

Where IVP's behavior is surprising, the suite follows IVP rather than the
obvious design, and records why next to the check:

- `IsAttachedToConstraint` returns false; hinge limits are soft; static objects
  report inverse mass `1/m` and motion enabled.
- The player controller only corrects along axes with a speed budget, and its
  "press down" step makes IVP's player hop on its contacts, so ground carry is
  friction-dependent; the checks cover the deterministic parts.
- `IsBoxIntersectingCone` reports every box as intersecting on IVP (no engine
  caller); only intersecting cases are required.
- IVP answers no AABB/trace queries on virtual meshes, misses some flat
  polysoup triangles with point rays, and ignores perfectly planar virtual
  meshes; the checks sample what IVP resolves.
- IVP leaves a dangling listener when a vehicle controller is destroyed before
  its body; the suite uses the game's order (body first).

## 7. Not yet covered

Threading contexts beyond create/destroy, performance budgets, the
`VEHICLE_TYPE_CAR_RAYCAST` car system (no game consumer; Box3D drives it with
the real-wheel system), IVP's wheel side-contact friction override (Box3D's
friction mixing has no contact normal), `WasFrozen`, and exact trajectory
equivalence. A provider passing this contract is not certified for those.
