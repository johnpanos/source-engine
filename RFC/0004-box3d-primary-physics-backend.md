# RFC 0004: Box3D as the Primary Physics Backend

- Status: Proposed
- Date: 2026-09-21
- Scope: VPhysics, collision assets, physics-dependent gameplay, content tools,
  and client/dedicated-server builds
- Related: [RFC 0001: Capability-Based Platform Architecture](0001-capability-based-platform-architecture.md)
- Verification: [RFC 0005: Quality and Correctness Harnesses](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006: C++20, Ownership, and Synchronization](0006-modern-cpp-ownership-and-synchronization.md)
- Evaluated dependency: Box3D `9e5a4cde862fba95ff19f096b79567f3ea6c01fd`
  (`v0.1.0-24-g9e5a4cd`, branch `main`)

## Summary

This RFC proposes replacing IVP/Havana as Source's primary rigid-body physics
backend with Box3D while preserving the public VPhysics interfaces used by the
engine, game DLLs, and tools.

Box3D provides the essential simulation primitives: three-dimensional rigid
bodies, convex hulls, static triangle meshes, continuous collision detection,
collision filters, sensors, spatial queries, several joint types, and parallel
simulation. The integration is nevertheless substantial because VPhysics also
defines gameplay behavior, collision asset formats, controller interfaces,
callback timing, and persistence semantics.

The proposed implementation has three boundaries:

1. A VPhysics compatibility adapter retains Source's public ABI and behavior.
2. A physics runtime owns Box3D worlds, bodies, shapes, joints, and simulation
   scheduling.
3. A collision asset layer reads existing Source data and produces reusable
   geometry for the new runtime.

Box3D will initially be opt-in. IVP remains available for reference testing and
rollback. Box3D becomes the default only for application/platform combinations
that pass defined compatibility, gameplay, and performance gates. Removing the
IVP runtime and removing all legacy collision-format dependencies are separate
milestones.

The first delivery is a bounded vertical slice: load an existing map and model
collision assets, simulate dynamic props and one ragdoll, exercise traces and
collision events, and restore a save using Box3D. That experiment determines
whether the public upstream API is sufficient and identifies any required
upstream changes before the broader migration is committed.

## Motivation

The current physics implementation combines an older simulation backend with
Source-specific behavior across a large adapter. Its build links
`ivp_physics`, `ivp_compactbuilder`, `havana_constraints`, `hk_math`, and
`hk_base` into `vphysics`.

Box3D offers a smaller conceptual dependency surface for future engine work:
a C API, opaque IDs, explicit worlds and definitions, allocator hooks,
simulation profiling, buffered events, and configurable worker scheduling.
Its geometry and query APIs can support both runtime physics and content
processing. Its MIT license is recorded in the upstream repository; adopting
it does not change the licensing or provenance of other repository components.

The expected benefits are easier backend maintenance, a path to parallel
simulation, and better tools for reproducing physics problems. Performance and
stability improvements remain hypotheses until measured on Source workloads.
An upstream feature list or successful unit test run is insufficient evidence
that Source gameplay will improve.

## Goals

- Make Box3D the primary supported rigid-body backend for validated targets.
- Preserve the engine/game boundary represented by `IPhysics`,
  `IPhysicsEnvironment`, `IPhysicsObject`, and related VPhysics interfaces.
- Load existing model `.phy` files and BSP physics collision data without
  requiring an immediate content rebuild.
- Preserve surface properties, contents filtering, collision groups, model
  solid identities, and gameplay callbacks.
- Support Source props, ragdolls, shadow/player/motion controllers, fluids,
  constraints, and vehicle types through explicit compatibility work.
- Preserve existing player movement and server authority while replacing the
  rigid-body simulation beneath them.
- Introduce repeatable conformance, gameplay, asset, and performance tests.
- Keep backend selection centralized and lifecycle-safe in accordance with
  RFC 0001.
- Establish a path to retire IVP simulation and eventually its collision data
  dependencies.

## Non-goals

- Reproducing IVP's trajectories bit for bit.
- Replacing Source's complete BSP collision, spatial partition, hitbox,
  movement, or networking systems in the initial migration.
- Replacing Source player movement with Box3D's experimental character mover.
- Switching physics providers while a level or physics object is live.
- Mixing IVP and Box3D simulated bodies in one interacting environment.
- Treating a Box3D recording as a Source savegame or multiplayer protocol.
- Promising that arbitrary existing binary mods, saves, or demos remain
  compatible without qualification and testing.
- Making every historical platform or game variant a prerequisite for the
  first opt-in prototype.
- Implementing the migration merely by accepting this design document.

## Evidence and current architecture

### Evaluation baseline

The dependency was cloned as a sibling checkout at `/home/john/src/box3d`.
That path is an evaluation convenience and MUST NOT become a build or release
requirement.

The initial source survey used Source commit `ed8209cc`; follow-up inspection
during RFC drafting used `b5eb9915bdebf5af1045c5821d4ac3d9af15dbb2`.
Counts below are approximate sizing evidence from the survey, not an ABI
inventory or a promise that every method requires a new implementation:

- approximately 23,000 lines of C++ and headers under `vphysics/`;
- approximately 119,000 lines across IVP source and headers;
- 287 matching pure-virtual declarations in `vphysics_interface.h` and 77 in
  the supplementary `public/vphysics/` headers, including callback contracts;
- 128 files outside `vphysics/` and `ivp/` referencing VPhysics headers or
  interfaces, including tools and build descriptions;
- no VPhysics references found in the surveyed `unittests/` tree.

The evaluated Box3D checkout built on Linux x86-64 using GCC 16.2.1 and CMake
4.3.0 with samples and benchmarks disabled. The complete upstream test
executable reported success. GCC emitted `-Wmaybe-uninitialized` warnings in
the SIMD/AABB path; their cause was not diagnosed in this evaluation.

The test configuration was Release. It did not establish debug-assertion,
sanitizer, other-platform, or in-engine correctness. Temporary build outputs
were removed after the check; the Box3D checkout remained clean.

### Integration seams

| Responsibility | Current source | Migration significance |
| --- | --- | --- |
| Public contracts and units | `public/vphysics_interface.h`, `public/vphysics/*.h` | Preserve signatures, ordering, and established semantics |
| Exported physics service | `vphysics/main.cpp` | Own provider lifecycle and interface exposure |
| Simulation and object wrappers | `vphysics/physics_environment.cpp`, `physics_object.cpp` | Replace IVP ownership and stepping |
| Geometry, serialization, and queries | `vphysics/physics_collide.cpp`, `physics_trace.h`, `trace.cpp` | Decode legacy geometry and implement trace semantics |
| Displacement collision | `engine/cmodel_disp.cpp`, `vphysics/physics_virtualmesh.cpp` | Adapt virtual mesh callbacks to static geometry |
| Model collision loading | `datacache/mdlcache.cpp` | Preserve `.phy` loading and cache lifetimes |
| BSP physics loading | `engine/cmodel_bsp.cpp` | Preserve physics lumps and model indices |
| Gameplay event handling | `game/server/physics.cpp` | Preserve pre/post velocity and damage behavior |
| Controllers and constraints | `vphysics/physics_shadow.cpp`, `physics_constraint.cpp`, controller sources | Port policy and identify solver gaps |
| Persistence | `vphysics/vphysics_saverestore.cpp`, `game/shared/physics_saverestore.h` | Preserve object association and restore ordering |
| Content generation | `utils/studiomdl/collisionmodel.cpp`, `utils/vbsp/`, `utils/common/bsplib.cpp` | Define future collision cooking and format changes |
| Build and bootstrap | `wscript`, `vphysics/wscript`, launcher/dedicated startup | Select a consistent provider across consumers |

The principal exported versions are `VPhysics031`, `VPhysicsCollision007`, and
`VPhysicsSurfaceProps001`. The repository also contains a V30 compatibility
header; Phase A must determine which historical interfaces are actually
exported and required before committing to binary compatibility claims.

The opaque `CPhysCollide` boundary is useful: most consumers do not know its
implementation. Internally, however, the current implementation exposes IVP
compact surfaces and ledges. Those internal representations cannot simply be
passed to Box3D.

## Design principles

### Preserve behavior at the compatibility boundary

The adapter MUST preserve public layout and vtable ordering for interface
versions it advertises. Semantic compatibility requires tests in addition to
ABI compatibility. Returning a success value from an unimplemented method is
not acceptable.

Every required method will be classified as directly mapped, implemented in
the adapter, implemented through an approved backend extension, or unsupported
for a specifically declared prototype profile. Unsupported required behavior
MUST prevent that profile from being promoted to a supported default.

### Keep the dependency private

Box3D headers and IDs MUST stay inside backend implementation targets. Engine
and game consumers continue to receive VPhysics interfaces. Internal code may
use narrow typed dependencies, consistent with RFC 0001, without creating a
second public physics API during this migration.

### Make compatibility choices explicit

Approximation is sometimes reasonable, especially for tuning and constraint
behavior. Each approximation MUST record its affected feature, test coverage,
and acceptance decision. Silent omission of friction mutation, pulley
behavior, or ragdoll limits is not an approximation policy.

## Proposed architecture

```text
Engine, game DLLs, and content tools
                  |
          Existing VPhysics ABI
                  |
       VPhysics compatibility adapter
          |                   |
  Source behavior         Collision asset layer
  and persistence         |                  |
          |          Legacy IVP decoder   Native geometry/cache
          |                   |                  |
          +-------------------+------------------+
                              |
                       Box3D runtime
                 worlds / bodies / shapes / joints
                              |
              Allocation, logging, jobs, and diagnostics
```

The legacy decoder supplies geometry and metadata. It does not run a second
simulation. It may initially use isolated IVP parsing/query code, provided
that no IVP runtime pointers escape into the new backend.

An illustrative implementation layout is:

```text
vphysics_box3d/
    main.cpp
    environment.cpp
    object.cpp
    collision.cpp
    trace.cpp
    conversion.cpp
    events.cpp
    constraints.cpp
    controllers/
    persistence.cpp
    legacy_collision/
    wscript
thirdparty/box3d/             # proposed pinned dependency location
unittests/vphysics_test/
tools/physics_conformance/
```

Names are provisional. `thirdparty/` is already a submodule in this repository;
dependency placement must follow its ownership model or use a separately
managed external directory. This RFC does not require a nested submodule.

### Provider selection and composition

A proposed build option, `--physics-backend=ivp|box3d`, selects the production
provider. Initially its default is `ivp`. A development build may produce both
providers in distinct output directories for comparison.

The preferred first integration installs the selected implementation under the
existing `vphysics` module basename, retaining current launcher and tool
expectations. A separately named `vphysics_box3d` module is useful for tests,
but runtime selection of that name requires coordinated composition changes
for client, dedicated server, datacache, and tools.

All three primary interface exports MUST come from a coherent provider.
Loading `IPhysicsCollision` from IVP and passing its opaque objects directly to
a Box3D `IPhysicsEnvironment` is prohibited. A legacy decoder behind the
collision adapter is an explicit conversion operation, not mixed ownership.

Selection occurs before consumers connect. Startup records the backend,
dependency revision, compatibility profile, precision mode, and solver
configuration. An explicitly requested backend that cannot initialize fails
with an actionable diagnostic. It MUST NOT silently fall back after partially
loading a map or creating objects.

RFC 0001's composition and module-lifetime rules apply. Backend work MUST use
the established module boundary and avoid introducing new dynamic-loader
dependencies in gameplay code. It does not depend on completion of the entire
platform architecture migration.

### World and object ownership

Each `IPhysicsEnvironment` owns one Box3D world and stable C++ wrappers for its
objects, constraints, and controllers. Box3D IDs are implementation details;
they MUST NOT be reinterpreted as VPhysics pointers or serialized as permanent
game identities.

A wrapper preserves Source-only state such as game data, flags, material and
contents information, original static status, motion enablement, controller
attachments, and pending deletion. `IsStatic()` and `IsMotionEnabled()` retain
their distinct meanings when an initially dynamic object is temporarily pinned.

Collision resources may be shared by several bodies or environments. They
remain alive until the last body and query borrower releases them. This is
especially important for Box3D mesh and height-field shapes, which retain
references to input geometry. Resource policy must follow each API's actual
ownership contract rather than assuming all shapes clone their data.

Deletion during callbacks is queued. The adapter copies transient events before
world mutation, resolves IDs to stable wrappers, and retains the metadata
necessary to report end-touch events involving destroyed shapes. Shutdown
drains work, destroys controllers and joints, destroys bodies/worlds, releases
shared collision resources, and only then unloads the provider.

`TransferObject` requires an explicit recreation protocol between worlds.
The destination must receive equivalent shape, mass, velocity, filtering, and
controller state while preserving the public wrapper identity where required.
Attached constraints and controllers cannot be silently dropped. Portal
workloads will be used to define and test these semantics.

## Units, transforms, and physical quantities

Source's public physics coordinates remain inches with Z up, and its angle
interfaces remain degrees. Box3D will use meters, kilograms, seconds, radians,
and Source's axis orientation with Z-directed gravity.

This refines the preliminary scoping suggestion of calling
`b3SetLengthUnitsPerMeter(39.3700787f)`. That API scales internal tolerances; it
does not convert density, force, or inertia. Keeping meters follows the
existing IVP conversion boundary and makes Box3D tuning easier to compare with
upstream examples. The adapter does not need IVP's historical axis permutation
for newly constructed geometry, but the legacy decoder must undo it correctly.

| Quantity | Boundary rule |
| --- | --- |
| Position, distance, linear velocity, acceleration | Multiply Source values by `0.0254` on entry; divide on return |
| Mass | Preserve kilograms |
| Volume and area | Apply cubic or squared length conversion as appropriate |
| Density | Preserve documented `kg/m^3`; do not apply a second length conversion |
| Orientation | Convert Source rotation conventions through tested matrices/quaternions |
| Angular velocity | Convert degrees to radians and object-local/world frames as required |
| Linear impulse | Convert length units and use the Box3D impulse API |
| Continuous force | Convert units and integrate through force APIs at the defined step cadence |
| Inertia, torque, angular impulse, break thresholds | Follow audited existing implementation semantics and golden tests |

The last row deliberately does not prescribe a single dimensional formula.
The current `GetInertia`/`SetInertia` implementation permutes IVP components
without scaling them to square inches, while public comments describe several
angular and constraint quantities in historical conventions. Mechanical
conversion based only on field names would change gameplay.

For example, `ApplyForceCenter` takes an impulse despite its name; mapping it
to `b3Body_ApplyForceToCenter` would incorrectly introduce a timestep factor.
Similarly, Source object angular velocity is documented in local axes while
Box3D angular velocity is in world space.

Conversion tests MUST cover nonidentity rotations, off-center mass, asymmetric
inertia, offset impulses, inverse transformations, and legacy geometry winding.
They must distinguish point, direction, plane, velocity, force, and impulse
operations with named helpers.

## Collision assets and queries

### Legacy format support

Model `.phy` data enters through `datacache/mdlcache.cpp`. BSP physics data
enters through `VCollideLoad` in `engine/cmodel_bsp.cpp`. The current decoder
recognizes VPHY headers and IVP compact surfaces; MOPP support is disabled in
the inspected implementation. Supporting every historical MOPP variant is not
an implicit prerequisite of preserving the current content envelope.

The new decoder MUST preserve:

- solid indices and per-convex game data;
- convex decomposition and local transforms;
- triangle material identifiers and surface-property mappings;
- mass-center and drag-area metadata;
- keyvalue descriptions for solids, joints, fluids, and vehicles;
- current supported byte-order and format variants.

Rendering triangles are not an acceptable replacement for authored collision
geometry. Dynamic concave props must retain their convex decomposition. Static
brush geometry may use hulls or meshes according to verified trace and contact
semantics.

The first decoder may reuse existing IVP structures and extraction code behind
a private target. A later independent decoder can remove that dependency after
it passes the same asset corpus. Neither stage requires a second live physics
provider or changes to consumer-facing `CPhysCollide` ownership.

### Geometry limits and failure policy

At the evaluated revision, Box3D limits hull vertices, faces, and edges to 128
each. Auditing only vertex count is insufficient. Asset conversion MUST report
the model/solid and failed limit; it MUST NOT silently replace a hull with a
bounding box or accept reduced geometry without an explicit cooking policy.

Possible resolutions include preserving more convex pieces, splitting an
oversized hull, or approved simplification with measured geometric error.
Static assets may use meshes where contents and material behavior permit.
Dynamic and kinematic concave assets use multiple convex shapes on a body:
Box3D's baked compound shape is static-only, and mesh contact generation is
restricted to static bodies.

Baked compounds also limit the material mapping for an individual child mesh
to four entries at this revision. A many-material Source mesh can remain a
standalone mesh shape or be partitioned deliberately. Material truncation is
not acceptable.

Displacement virtual meshes initially become cached static triangle meshes
through the existing callback data. Height fields are an optional later
optimization: arbitrary displacement geometry, holes, and flags must be
preserved before selecting that representation.

### Cooking and cache format

Phase B may convert in memory on load. A subsequent disk cache SHOULD key data
by source content hash, conversion version, units, cooking settings, backend
geometry version, and any architecture/precision assumptions of the payload.
Cache failure must be recoverable by recooking the authoritative source data.

A future native format will store a versioned envelope with explicit sizes,
offsets, solid identifiers, and metadata. It must distinguish portable source
geometry from backend-specific acceleration data. Raw pointers, vtables, and
unversioned internal memory dumps MUST NOT form a durable asset format.

`CollideSize`, `CollideWrite`, `UnserializeCollide`, query models, debug meshes,
convex construction, and polyhedron extraction all need documented behavior.
Legacy writers may be retained during transition, or new output must be
explicitly versioned and paired with updated readers. Merely keeping the
`.phy` extension does not make a changed payload compatible with older tools.

`studiomdl`, `vbsp`, related BSP conversion utilities, and collision-inspecting
tools are part of this work. Their build wiring must be checked independently;
their presence in the source tree does not imply they are all built by the
default Waf target.

### Trace compatibility

Implementing `IPhysicsCollision` queries is separate from querying a simulated
world. Callers can trace a collision model without creating a physics body,
and may require thread contexts. The adapter SHOULD query immutable geometry
directly or use explicitly owned query acceleration structures.

The trace conformance suite covers:

- rays, swept AABBs, convex sweeps, and zero-length tests;
- `fraction`, `startsolid`, `allsolid`, and `fractionleftsolid`;
- normals, plane distances, contents, materials, and hit identities;
- per-convex contents filtering through `IConvexInfo`;
- inside starts, tangencies, seams, triangle backfaces, and boundary tolerances;
- rotated models, compound models, and displacement surfaces.

Box3D casts ignore some initial overlaps and mesh backfaces. A direct cast
result therefore cannot populate all Source trace fields. The adapter needs
explicit overlap handling and, where required, exit-fraction computation.
Source trace clipping tolerances must remain consistent with engine callers.

The initial migration retains engine BSP traces and movement logic. Replacing
every `IEngineTrace` operation with a Box3D world query is a separate proposal.

## Simulation, events, and threading

### Step contract

The environment preserves Source's simulation timestep, clock, and caller
cadence. It MUST NOT introduce an independent 60 Hz loop. Box3D substeps are
an internal quality setting; their initial value is a measured configuration,
not a compatibility assumption.

A step consists of controlled mutation, controller evaluation, simulation,
event capture/delivery, active-object publication, and deferred cleanup.
Controller forces and impulses must be applied at a tested cadence. Splitting
one call into several world steps can change both callback timing and solver
behavior and requires separate validation.

The implementation MUST define `IsInSimulation()` during compatibility
callbacks and maintain consistent deletion/reentrancy rules. Sleeping objects
and wake transitions need explicit tracking; movement events alone may not
encode every Source object-event transition.

### Collision callback fidelity

Source requires matched `PreCollision`/`PostCollision` pairs. Its game handler
reads object velocities in `PreCollision` and uses them in later damage logic.
Box3D buffers contact events until after the step; its pre-solve callback has
limited data and prohibits world mutation.

Calling both Source callbacks after the step while exposing final velocities
would violate that contract. Capturing velocities at the beginning of the
entire tick is also not necessarily equivalent to pre-impact velocities after
earlier impacts within the same tick.

The vertical slice MUST resolve this gap through one of these explicit paths:

1. A narrow backend instrumentation extension records the required pre/post
   impact state at the correct solver point, and the adapter replays callbacks
   with a well-defined read view and queued mutations.
2. A compatibility implementation proves that the required values can be
   reconstructed from exposed data for the supported behavior.
3. A separately reviewed game-side change adopts an event payload with
   explicit pre/post state and documents the resulting binary compatibility
   boundary.

This is an early feasibility gate. A queued callback system solves thread
ownership, but by itself does not solve event semantics. Callbacks whose writes
must affect the current solve may require additional backend work or an
explicitly revised contract.

The event bridge also preserves callback flags, collision speed, time since
collision, per-contact materials, shadow-collision classification, friction
energy, trigger/fluid touch events, and `PostSimulationFrame` ordering.
Shape-level begin/end events must be aggregated into object-pair events so
multi-shape props do not emit duplicate enters or premature leaves.

### Contact inspection and mutation

Box3D exposes contact data suitable for snapshots, but its public contact API
at the evaluated revision does not provide direct equivalents for all
`IPhysicsFrictionSnapshot` mutations: clearing friction impulses, marking
individual contacts for deletion, and deleting marked contacts.

Read-only snapshots must copy or bound the lifetime of transient manifold
data. Mutation methods require a call-site audit and either a supported
implementation or a tested backend extension. Recreating an entire body or
changing its filter is not automatically equivalent to deleting one contact.

### Threading policy

The first integrated backend uses one worker to isolate semantic bugs.
Multithreading follows only after event and filtering contracts pass.

Game entity callbacks MUST NOT execute concurrently on Box3D worker threads.
Custom filters and pre-solve logic use immutable per-step data or other proven
thread-safe state. Changes in game collision rules become visible at defined
boundaries, with invalidation when the contract requires immediate rechecks.

Box3D's internal scheduler is a valid initial implementation. A later bridge
to Source's job system must respect fork/join behavior: blocking a job while
its child tasks exhaust the same pool can deadlock. Scheduler integration
must include nested-work and shutdown tests.

Read-only world queries can be parallelized outside stepping as permitted by
upstream. Geometry loading, world creation/destruction, allocator callbacks,
and global Box3D configuration need their own synchronization and lifecycle
rules. Multiple client/server/Portal worlds must not oversubscribe cores by
each creating an unrestricted worker pool.

## Feature mapping and gameplay work

| Source feature | Box3D mechanism | Required compatibility work |
| --- | --- | --- |
| Polygon and sphere objects | Bodies plus hull/sphere shapes | Mass overrides, flags, sleep, motion/collision enablement |
| Static world and props | Static hulls, meshes, baked compounds | Contents, surface metadata, resource sharing |
| Triggers | Sensors | Object-pair aggregation, destruction, timing, supported visitor shapes |
| Hinge | Revolute joint | Local frames, angular limits, motors, friction |
| Sliding | Prismatic joint | Reference frames, limits, motor/force units |
| Fixed | Weld joint | Softness, break behavior, activation |
| Ball socket | Spherical joint without angular limits | Anchors and break behavior |
| Length and spring | Distance joint or adapter force | Minimum/maximum length, stretch-only behavior, damping |
| Ragdoll | Spherical joints and possibly extensions | Three-axis limits, per-axis friction, angular-only modes |
| Pulley | No direct public pulley joint | Coupled rope-length/ratio constraint implementation |
| Constraint groups | Adapter-managed group | Error reporting, activation, penetration solving, iteration policy |
| Shadow/player controllers | Forces, impulses, motors, and queries | Source tracking, push limits, stairs, contact, freeze semantics |
| Motion controllers | Adapter callbacks | Priority, local/global force/acceleration, step cadence |
| Fluids | Sensors and custom forces | Submerged volume, buoyancy, drag, currents, touch callbacks |
| Vehicles | Wheel joints, casts, custom forces | All Source vehicle scripts, operating state, and handling |
| Save/restore | Adapter-owned records | Identity remapping and controller/constraint reconstruction |

### Constraints and ragdolls

Common joint types have useful analogues, but parameter equivalence must be
demonstrated. Source's ragdoll interface specifies three axis limits and
per-axis motor/friction terms. Box3D's spherical joint uses a cone and twist
limits, with different motor controls. Those feasible rotation regions are
not generally identical.

A representative ragdoll corpus must establish whether fitted cone/twist
limits are acceptable or whether a solver extension is required. Asymmetric
limits, `onlyAngularLimits`, clockwise conventions, and updated ragdoll frames
must be covered. The one-ragdoll vertical slice is an experiment, not proof of
full ragdoll compatibility.

Joint force/torque thresholds produce events; the adapter implements Source's
break/deactivation and notification semantics. It also audits per-constraint
body mass scaling, strength, and group-specific iteration settings, which do
not have obvious one-to-one public mappings. Two independent distance joints
do not implement a pulley with coupled rope length and gearing.

### Shadow, player, and motion controllers

These controllers are central to Source interaction: NPC proxies, moving
objects, carrying props, the gravity gun, and player pushes. A kinematic body
alone cannot reproduce every physically controlled shadow. In particular,
Box3D kinematic bodies do not normally collide with static or other kinematic
bodies in the same way as dynamic bodies.

Preserve the game-authored movement target and implement the appropriate
dynamic tracking, query, impulse, or motor behavior for each controller mode.
Tests must cover teleport thresholds, target arrival time, translation/rotation
permissions, maximum push mass/speed, step-up, jumping, contact reporting,
moving platforms, and transitions between standing and crouching shapes.

Motion controllers retain their local/global and force/acceleration meanings.
Their priorities and attached-object lifetime rules remain observable. Using
the experimental Box3D character mover to replace the entire player movement
pipeline is outside this RFC.

### Fluids and drag

Fluid volumes are sensor-backed regions with Source metadata. The adapter
computes buoyancy and drag for immersed bodies, including partial submersion,
surface planes, current velocity, buoyancy overrides, wake behavior, and
per-object opt-out flags.

Box3D provides an aerodynamic wind helper, but that does not establish parity
with Source's drag areas or fluid controllers. Existing parameters must be
audited and tested against floating, sinking, rotating, and entering/exiting
bodies. Buoyancy and airboat development share tests but remain separate
features.

### Vehicles

The public vehicle interface defines wheel cars, raycast cars, raycast jetskis,
and raycast airboats. A wheel joint is only one component of that interface.
Required behavior includes engine/transmission state, steering curves,
suspension, traction, braking, skid materials, booster state, entry/exit,
water queries, script reload, and operating/debug data.

Use the existing controller implementations and vehicle scripts as behavioral
references. Raycast modes continue to honor `IPhysicsGameTrace`, including its
water semantics. Vehicle delivery requires representative maps and handling
tests; plausible movement on a flat plane is insufficient.

## Persistence, networking, and replay

The adapter preserves the `Save`, `PreRestore`, `Restore`, and `PostRestore`
protocol and the association of physics records with game entities. Records
contain stable logical IDs, collision asset references, transforms, velocities,
mass properties, flags, controller configuration/state, constraints, and
environment settings.

Restore proceeds in dependency order: collision resources and bodies,
constraints/controllers, game associations, then activation and callbacks.
`SerializeObjectToBuffer` and `UnserializeObjectFromBuffer` are separate public
paths and require equivalent versioning and round-trip coverage.

The initial support contract guarantees new Box3D saves restoring with the
matching supported backend/schema. Importing old IVP saves is a separate
compatibility deliverable. Saves must record or otherwise unambiguously
identify the physics schema/provider; incompatible data must be rejected
before a partially restored level becomes playable.

Rollback to IVP is guaranteed for original content and compatible IVP saves.
It does not imply that IVP can read newly authored Box3D-native collision files
or saves. Tests and packaging must preserve that distinction.

Source's server-authoritative networking remains in place. Box3D determinism
does not justify replacing replication or assuming clients reconstruct the
same result from arbitrary inputs. Audit predicted physics interactions,
client ragdolls, interpolation, and authoritative corrections. Establish a
supported client/server backend combination policy before default promotion;
mixed-backend gameplay is not implicitly supported.

Upstream recordings are diagnostic artifacts tagged with exact dependency,
adapter, build, precision, and tuning versions. They complement Source-level
input/event capture. Custom filter decisions, controller inputs, and game
callbacks must be recorded or reproduced where necessary; an upstream replay
alone cannot prove that the entire game is deterministic.

## Build, packaging, and dependency policy

The production dependency is pinned to a reviewed commit. Updates include an
API/format diff, upstream tests, adapter conformance results, and the gameplay
benchmark corpus. Preserve upstream license notices and isolate any patches
with an explanation and regression test.

Waf integration may compile the upstream C sources as a private static target
or invoke a controlled CMake build. The preferred long-term choice is the one
that reliably preserves upstream compiler settings without maintaining two
inconsistent source lists. Both require:

- C17 compilation for Box3D, separate from Source's C++ settings;
- position-independent objects when linked into the shared physics module;
- correct math/thread linkage for the target toolchain;
- upstream floating-point settings, including disabled FMA contraction where
  required, without inheriting incompatible fast-math flags;
- matching precision definitions between library and adapter;
- allocator alignment and ownership compatible with the host;
- compatible runtime-library choices on Windows;
- private symbol visibility and no samples/graphics dependency in the runtime;
- independent client and dedicated-server packaging checks.

New adapter/runtime C++ targets follow RFC 0006's C++20 policy; Box3D remains
C17. Keep standard/FP/runtime settings private to their targets. Use scoped
ownership and result values internally without changing advertised VPhysics
vtables or exposing C++ standard-library types across its preserved ABI.
Buffered event handoff declares capacity, publication, ownership, and overflow
behavior. Required collision events cannot be overwritten or dropped to fit a
ring; capture capacity failures need an explicit failure/recovery policy.

The initial target is Linux x86-64. Windows x86-64 and other actively supported
targets follow through explicit validation. Source advertises additional
platforms and architectures, including Android, FreeBSD, and 32-bit builds;
portable C and available SIMD fallbacks do not establish those ports as tested.

Single-precision positions in meters are the initial configuration. Box3D's
optional double-precision world positions may be evaluated later, but they do
not remove precision limits in Source's float-based engine and asset pipeline.

## Validation and acceptance criteria

### Contract and asset conformance

Phase A produces a method-level inventory of the interfaces the provider
advertises. Each entry records consumers, implementation strategy, tests, and
profile requirements. Header method counts are not an adequate substitute.

The conformance harness runs equivalent operations against each provider in
separate processes or otherwise isolated compositions. It compares observable
contracts, using exact comparisons for identities/flags and documented
tolerances for geometry/physics quantities. It does not compare raw solver
memory or require identical trajectories.

Required suites include conversions, lifetime/deletion, filters, traces,
events, sleep/wake, constraints, controller behavior, serialization, and
malformed asset rejection. Asset tests cover multi-solid ragdolls, oversized
hulls, many-material meshes, displacements, unusual center-of-mass offsets,
and the supported legacy format variants.

Binary decoders validate lengths, offsets, indices, arithmetic overflow, and
resource budgets before constructing backend objects. Truncated or malformed
content must produce a deterministic load failure rather than a crash or
partially colliding level.

### Executable compatibility and feasibility gates

Q-PHYSICS in RFC 0005 owns isolated provider execution and reports; this RFC owns
the method/profile requirements and semantic expectations. Q-FOUNDATION adds
frozen-header binary consumers and lifetime fixtures; Q-CONTENT adds independent
asset readers, fuzzing, recovery, and compiler checks. Required method coverage
references executable test IDs and actual consumers, not just method counts.

Compare normalized operation inputs, public outputs, callback state/order, and
logical identity. Keep provider-native pointers and solver memory out of the
oracle. Analytical cases check simple conversions/motion independently of IVP;
gameplay scenes use declared outcomes and numeric tolerances. Negative adapter
fixtures must expose force-versus-impulse errors, lost materials, wrong trace
flags, duplicate touches, and callbacks that report final instead of pre-impact
velocity. A plausible trajectory cannot compensate for a failed contract.

Phase A must produce executable evidence for pre/post collision state, including
several impacts in one tick, callback reads/queued writes, and resulting damage.
Contact deletion/friction mutation, asymmetric ragdoll limits, oversized real
asset hulls, and decoder ownership also need tested solutions or explicit scope
decisions. A required unresolved feature blocks its profile; it is not marked
passed by an unsupported result or skipped test.

Phase B begins with one worker and covers the documented BSP/PHY/compound-prop,
inside-start trace, verified impact, ragdoll, and save/restore slice. Later worker
integration uses RFC 0003's scheduler tests plus physics-specific immutable
filters, callback affinity, bounded event transport, nested work, oversubscription,
and shutdown. Test the bridge in both client/listen-server and dedicated profiles
where supported. Introducing the scheduler must not simultaneously redefine the
physics semantic baseline.

Save tests distinguish matching Box3D-schema restore, old-IVP-save import, and
IVP rollback. Corrupt/incompatible data fails before level publication; test
partial restore cleanup and interruption as well as successful round trips.
The content comparator retains all solid IDs, material metadata, decomposition,
and supported unknown fields. A writer and reader sharing a defect do not prove
compatibility; selected legacy/tool consumers must accept promised formats.

### Gameplay corpus

Select and document maps/scenes covering:

- prop piles, thin geometry, fast throws, and repeated destruction;
- stairs, doors, elevators, player pushes, crouching, and carried objects;
- ragdolls, constrained props, breakable joints, pulleys, and gravity-gun use;
- impact damage, sound/material selection, friction, triggers, and fluids;
- each required vehicle mode;
- save/load, level transitions, client/server operation, and long sessions;
- Portal environments, object transfer, and shadow clones if Portal is in the
  target support profile.

Every required scene has a machine-checkable completion condition where
practical and a recorded review criterion where feel must be judged manually.
Acceptance requires no unresolved progression blocker, missing collision,
invalid lifetime access, or silently unsupported required call.

### Performance and determinism

Measure total physics cost, including conversions, event staging, controller
work, cache construction, and synchronization. Report median, p95, and p99
frame cost, cold/warm load time, peak memory, body/contact counts, and sleeping
behavior. Compare identical maps, gameplay inputs, hardware, and build modes.

Phase A records numerical budgets for each target profile before optimization.
As an initial review threshold, investigate any repeatable p95 physics cost or
peak physics memory regression above 10% against IVP on the agreed corpus.
This is a proposed acceptance threshold, not a measured outcome; changing it
requires recorded justification. Every target must also meet its absolute
frame and memory budgets.

Run determinism checks at fixed inputs and configuration across repeated runs
and supported worker counts. Cross-platform checks use matched settings and
explicit expectations. Repeated stepping and restore/replay determinism are
distinct tests. A one-worker correctness baseline precedes speedup claims.

## Delivery plan

| Phase | Deliverables | Exit gate |
| --- | --- | --- |
| A: Inventory and feasibility | Interface/call-site inventory, asset corpus, IVP measurements, units tests, event and contact-mutation prototypes | Known solution or explicit decision for every vertical-slice blocker |
| B: Runtime vertical slice | Pinned build, adapter exports, worlds/bodies, legacy geometry conversion, traces, props, one ragdoll, event bridge, basic save round trip | Existing map/model loads and controlled scene works end to end |
| C: Core compatibility | Full required traces/filters, persistence, materials, object lifecycle, constraint/ragdoll support, shadow/player/motion controllers | Core gameplay corpus passes in client and dedicated builds |
| D: Gameplay completion | Fluids, vehicles, pulley/group semantics, selected game variants and Portal support, tool workflows | Declared feature profile has no unsupported required behavior |
| E: Performance and rollout | Parallel execution, platform CI, profiling, diagnostics, soak testing, packaging | Per-profile default-promotion checklist passes |
| F: Retirement | Native cooking where useful, independent legacy decoder, dependency removal | Box3D runtime no longer needs IVP simulation; later removal of remaining IVP code is separately verified |

Every phase uses current profile-specific RFC 0005 evidence. A/B establish the
compatibility oracle before broad gameplay migration; C/D expand method, asset,
and scene coverage; E requires actual installed client/server packages, measured
budgets, and rollback. F has two independent gates: no IVP simulation dependency,
then no remaining IVP decoder/cooking dependency after equivalent corpus results.
The cross-RFC portfolio order and tracked state live in [AGENTS.md](../AGENTS.md).

### Phase A: Highest-risk questions first

Before substantial controller implementation, settle pre/post collision state,
friction contact mutation, ragdoll limit representation, hull limits on real
assets, and legacy decoder feasibility. If these demand invasive changes to
the solver, the RFC must be revised with their scope and maintenance cost.

Select the first supported game configuration and representative maps. The
working assumption is a Linux x86-64 HL2-family configuration because it
exercises a broad set of Source physics features. The inventory confirms the
actual configured target and available content before establishing a baseline.

### Phase B: Demonstration boundary

The vertical slice must include more than a falling box. It loads an existing
BSP and `.phy`, preserves materials and contents, simulates a compound prop,
tests starting inside collision, delivers a verified impact event, runs one
ragdoll, and restores its supported state. Unsupported prototype features
remain explicit and prevent declaring the build a general replacement.

### Default promotion and rollback

Promotion is per application/platform profile. It requires:

1. complete required method coverage and passing conformance tests;
2. passing asset and gameplay corpus with documented physics-feel decisions;
3. passing persistence, level-transition, and client/server tests;
4. validated performance/memory budgets and worker configuration;
5. reproducible dependency and packaging configuration;
6. an IVP rollback package and documented save/content limitations.

Keep the IVP configuration in CI during the transition. Rollback selects a
different provider at process startup and uses compatible content/saves; it
never moves a running world between solvers.

### Program sizing

The preliminary estimates are planning ranges for experienced engine/physics
developers, assuming representative game content is available:

- vertical slice: approximately 4–6 engineer-weeks;
- playable core with controllers, ragdolls, and persistence: approximately
  3–5 person-months cumulative;
- production default for an agreed game/platform profile, including fluids,
  vehicle work, and regression testing: approximately 6–12 person-months
  cumulative.

These ranges overlap; they are not additive. They are not a commitment to full
parity across every Source game and historical platform. Solver extensions,
arbitrary binary-mod compatibility, old-save import, complete tool support,
and complete IVP retirement can exceed them. Phase A must replace these ranges
with task estimates based on actual blockers. Calendar time depends on staffing
and integration dependencies and will not scale linearly with team size.

## Risks and mitigations

| Risk | Consequence | Mitigation or decision gate |
| --- | --- | --- |
| Box3D is a maturing v0.1 engine | API churn and solver defects | Pin commits, maintain replay cases, gate updates |
| Callback timing differs | Incorrect impact damage and gameplay events | Prove pre/post state capture in Phase A |
| Ragdoll limits differ | Poses, stability, and gameplay change | Corpus-driven fitting or scoped solver extension |
| Legacy assets exceed geometry limits | Missing or altered collisions | Audit before runtime rollout; explicit cooking policy |
| Controller semantics are approximated | Broken carrying, movement, NPCs, or vehicles | Feature-specific acceptance scenes |
| CCD behavior differs | Fast objects tunnel or articulated chains stretch | Thin-geometry/throw tests, tuning, targeted casts, upstream fixes |
| Threaded filters touch game state | Races, deadlocks, inconsistent contacts | Immutable data, queued events, staged worker rollout |
| Save or cooked formats drift | Failed restores or content regressions | Versioned envelopes and defined support matrix |
| Duplicate geometry consumes memory | Load hitches and memory growth | Shared resources, caching, load/unload profiling |
| Local backend patches accumulate | Expensive dependency upgrades | Small reviewed patch series with regression tests |

Box3D documents limited general dynamic-versus-dynamic CCD, soft constraint
flexibility, static-only mesh/height-field contact support, and an experimental
character mover. These limitations should be tested where relevant rather than
assumed equivalent to IVP. Increasing substeps is a tuning option with cost;
it does not automatically resolve every semantic or collision limitation.

## Alternatives considered

### Rewrite engine/game code directly against Box3D

This would expose useful native APIs but spread migration across many consumers
and break the existing module contract. It remains possible for isolated new
features after the compatibility backend is established. It is not the initial
strategy.

### Preserve IVP collision and traces indefinitely

Keeping the legacy collision implementation reduces early asset/query work.
However, running Box3D against a second geometry representation can increase
memory and produce trace-versus-simulation discrepancies. A temporary decoder
or compatibility query implementation is acceptable with measured behavior and
a retirement plan; permanent dependence is a separate decision.

### Rebuild all assets before integrating the runtime

Native cooking simplifies the eventual dependency graph but requires working
content tools and source assets up front. Load-time conversion gives earlier
evidence with the existing content corpus and keeps asset migration off the
first critical path.

### Use Source inches inside Box3D

Scaling Box3D tolerances is supported and can reduce position conversions.
It still requires careful treatment of density, inertia, forces, angles, and
global scale configuration. This RFC chooses meters to match the existing
backend conversion model and upstream tuning conventions. A later change
requires complete conversion and performance evidence.

### Select another maintained 3D backend

The VPhysics seam can support evaluating other engines if Box3D fails the
feasibility gates. No comparative benchmark was performed in this scope, so
this RFC does not claim Box3D is faster or more compatible than alternatives.

## Open decisions and required evidence

| Decision | Proposed default | Evidence required before finalizing |
| --- | --- | --- |
| First supported profile | Linux x86-64, selected HL2-family configuration | Configured build, available test content, baseline scenes |
| Dependency placement/build method | Pinned private dependency | Submodule ownership and reproducible Waf integration |
| Collision pre/post state | Narrow capture extension if necessary | Damage/callback prototype and ABI review |
| Friction contact mutation | Adapter implementation or scoped extension | Required call sites and equivalent observable behavior |
| Ragdoll limit compatibility | Validated mapping where possible | Representative asymmetric-limit corpus |
| Old IVP saves | Separate import deliverable | Format coverage and restore tests |
| Tool and historical ABI support | Explicit profile inventory | Actual exports and tool consumers |
| Initial substep/worker counts | One worker; measured substeps | Stability, performance, and callback results |
| Removal of remaining IVP code | After independent decoding/cooking | Dependency audit and asset corpus passes |

## Source references

The implementation survey is grounded in the following repository sources:

- [VPhysics contracts and units](../public/vphysics_interface.h)
- [Constraint contracts](../public/vphysics/constraints.h)
- [Player controller](../public/vphysics/player_controller.h),
  [vehicles](../public/vphysics/vehicles.h), and
  [friction snapshots](../public/vphysics/friction.h)
- [Current backend build](../vphysics/wscript)
- [Existing conversion helpers](../vphysics/convert.h)
- [Collision representation and legacy decoding](../vphysics/physics_collide.cpp)
- [Collision event implementation](../vphysics/physics_environment.cpp)
- [Game-side pre-collision handling](../game/server/physics.cpp)
- [Physics save/restore](../vphysics/vphysics_saverestore.cpp)
- [Model collision loading](../datacache/mdlcache.cpp)
- [BSP loading](../engine/cmodel_bsp.cpp) and
  [displacement collision](../engine/cmodel_disp.cpp)

Upstream references are pinned to the evaluated revision:

- [Box3D public API](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/include/box3d/box3d.h)
- [Definitions, events, and joints](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/include/box3d/types.h)
- [Geometry limits](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/include/box3d/constants.h)
- [Documented limitations](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/docs/loose_ends.md)
- [Threading guidance](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/docs/foundation.md)
- [Recording and replay](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/docs/recording.md)
- [Build configuration](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/CMakeLists.txt)
- [License](https://github.com/erincatto/box3d/blob/9e5a4cde862fba95ff19f096b79567f3ea6c01fd/LICENSE)

## Proposed decision

Adopt the preserved VPhysics boundary and staged migration described here.
Proceed first with the inventory and runtime vertical slice, using the pinned
Box3D revision and representative existing Source content. Keep IVP as the
default until the selected profile passes the compatibility and rollout gates.

Approval of the architecture is distinct from approval to make Box3D the
default. That decision requires the measured results, resolved API gaps, and
explicit support matrix produced by the implementation phases.
