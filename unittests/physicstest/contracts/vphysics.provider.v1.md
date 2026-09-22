# Contract: `vphysics.provider.v1`

Module family: VPhysics providers (`vphysics` = IVP, `vphysics_box3d` = Box3D)
Interfaces: `IPhysics` (`VPhysics031`), `IPhysicsCollision` (`VPhysicsCollision007`),
`IPhysicsSurfaceProps` (`VPhysicsSurfaceProps001`) — `public/vphysics_interface.h`
Conformance suite: `unittests/physicstest/test_vphysics_conformance.cpp`
(Waf target `vphysics_conformance`)
Runner: `tools/quality/physics_conformance.py` (evidence `physics-conformance-evidence/v1`)
Roadmap: RFC 0004 Phase A/B (R09, R19), RFC 0005 Q-PHYSICS

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
| `trace.` | `TraceBox`/`TraceCollide` hit at the analytic fraction with the correct normal, miss cleanly, report `startsolid` when starting inside, honor swept extents and rotation, and always write the trace | engine `ClipRayToVPhysics` (player, bullets, portal placement against props) |
| `vcollide.` | `VCollideLoad` yields every solid and the trailing keyvalue text; decoded solids have finite non-degenerate AABBs, positive volume, hittable upward faces and clean misses; unload zeroes the record | `mdlcache`, static props, ragdolls |
| `keyparser.` | Every solid has a parsed `solid` block; a multi-solid ragdoll has `solidCount - 1` valid `ragdollconstraint` blocks | prop/ragdoll creation |
| `env.` / `object.` | Environment settings round-trip; objects keep game data, mass, material, collide and transforms; matrices and local/world conversions agree; teleport is immediate; flags round-trip; object lists track create/destroy; the simulation clock advances by the stepped time | entity physics sync |
| `shadow.` / `player.` / `motion.` / `constraint.` | Controllers and constraints are created, report their parameters, and (gameplay) move objects: shadows reach their targets, motion-controller accelerations apply, fixed constraints carry the attached object | player/NPC shadows, held objects, physcannon/portal gun |
| `sim.` | Free fall under gravity, static and motion-disabled objects stay put, active-object lists contain moving objects, velocities integrate, impulses apply, dynamic objects come to rest on static floors | `PhysFrame`, entity position updates |
| `events.` / `contacts.` / `rules.` | Pre/post collision events arrive in matched pairs naming both objects; resting objects sleep and report it; friction snapshots list the touching object; the collision solver's `ShouldCollide` filters pairs | impact damage/sounds, sleep networking, portal collision rules |

## 3. Observations and tolerances

The suite also prints observations (`OBS <key> <tolerance> <values>`) that the
runner compares against the oracle: the full surface-property table and parsed
keyvalues (exact), decoded geometry (AABB ±0.5 units, volume ±5%, mass center
±0.5, convex count exact), trace contact heights (±0.5) and free-fall distance
(±25%). Trajectories are **not** required to match IVP; simulation clauses are
qualitative ranges.

## 4. Fixtures

The runner extracts real content from the staged runtime in `gameinfo.txt`
search order (Portal VPK, Portal, HL2 VPK, HL2): the surface-property manifest
and its files, and the collision models for the weighted cube, the floor button
base, the turret and Chell's 23-solid ragdoll. Game content is never committed.

## 5. Sensitivity

Each fault below is injected into the oracle provider (a decorator over the
real interface, or suppression at the `Simulate` call boundary); the runner
fails unless the targeted checks fail:

`surfaceprops-null`, `surfaceprops-index`, `pairhash-ordered`,
`collision-unit-aabb`, `collision-trace-miss`, `vcollide-keyvalues`, `sim-noop`.

## 6. Not yet covered

Save/restore (`Save`/`Restore`, object serialization), fluids, vehicles,
springs, hinge/ballsocket/sliding/pulley/length/ragdoll constraint behavior,
player-controller movement, triggers, `TransferObject`, displacement virtual
meshes, BSP world collision (`CM` physics lump), threading contexts, and
performance budgets. A provider passing this contract is not certified for
those features.
