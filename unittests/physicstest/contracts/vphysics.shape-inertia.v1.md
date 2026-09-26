# Contract: `vphysics.shape-inertia.v1`

Module family: VPhysics providers. Claimed by `vphysics_box3d`; IVP
(`vphysics`) does not claim it.
Interface: `IPhysicsShapeInertia` (`VPhysicsShapeInertia001`,
`public/vphysics/shape_inertia.h`), reached through `IPhysics::QueryInterface`.
Contract suite: the `inertia.*` checks of `vphysics_conformance --bench contract`
(`unittests/physicstest/test_vphysics_shape_inertia.cpp`)
Gate: `shape-inertia` in `quality/budgets/physics-v1.json`, run by
`tools/quality/physics_bench.py`
Roadmap: RFC 0013 (R67); user decision 2026-09-25

This is an opt-in capability beside [`vphysics.provider.v1`](vphysics.provider.v1.md).
The legacy model is every environment's default and is exactly the parity
contract's behavior. A provider that does not claim the capability returns NULL,
and the suite reports `UNSUPPORTED VPhysicsShapeInertia001`.

## Why

IVP's inertia model keeps one value per object axis (no products of inertia),
computes it as IVP's own approximation rather than the solid's inertia, and
raises every axis to at least `rotInertiaLimit` (0.05 for every game prop)
times the inertia vector's length. Box3D's gyroscopic solve is exact for the
tensor it is given. Under the legacy model that tensor is the simplified one:
a prop modelled off its principal axes spins as if they were its object axes,
and a slender prop turns like a fat one. Under the shape model the tensor is
the solid's.

## Model

Under `PHYSICS_INERTIA_SHAPE` an object with a collision model gets the tensor
of its collision solid at uniform density about its mass center (the `.phy` or
override center), scaled to its mass and `objectparams_t::inertia`. A convex
that is one Box3D hull uses Box3D's hull mass data (`b3ComputeHullMass`). A
convex covered by overlapping patch hulls integrates its own surface triangles.
Both are combined about the mass center with the parallel-axis shift, as
`b3Body_ApplyMassFromShapes` does. Flat pieces add nothing. A sphere keeps
2/5 m r^2.

The per-axis `IPhysicsObject` API keeps its meaning, so game code written for
IVP works unchanged:
- `GetInertia` is the tensor's diagonal in object axes.
- `GetInvInertia` is the inverse tensor's diagonal.
- `SetInertia` sets the diagonal D and keeps the unit-diagonal coupling C, so
  the tensor becomes D^1/2 C D^1/2. The shadow controller's
  lock-then-restore and Portal's shadow clones' copy round-trip exactly.
- `SetMass` scales the tensor.
- `GetEnergy`, `CalculateVelocityOffset` and force-mode motion controllers use
  the full tensor.

## Clauses

| Check | Clause |
| --- | --- |
| `inertia.rejects-foreign` | NULL and foreign environments are rejected and report `LEGACY`; `GetInertiaTensor` rejects NULL and foreign objects and leaves the output untouched |
| `inertia.rejects-unknown-model` | An unknown model is rejected; a new environment is `LEGACY` |
| `inertia.default-legacy` | A legacy object's tensor is diagonal and equal to `GetInertia` |
| `inertia.rejects-populated` | The model cannot change once the environment holds an object |
| `inertia.selects` | An empty environment accepts `SHAPE` and reports it |
| `inertia.box-tensor` | A 48x24x8 box's tensor is the solid box's within 0.5% |
| `inertia.rotated-products` | A box turned inside its collide has R D R^T, products of inertia included, within 1% |
| `inertia.mass-center` | An overridden mass center adds the parallel-axis term |
| `inertia.no-rot-limit`, `inertia.legacy-rot-limit` | A 64x4x4 rod keeps its thin-axis inertia under `SHAPE`; `LEGACY` still raises it to `rotInertiaLimit * \|I\|` |
| `inertia.set-inertia-keeps-coupling` | Locking with 1e14 and restoring the saved diagonal returns the tensor; doubling the diagonal doubles it |
| `inertia.set-mass-scales` | Doubling the mass doubles the tensor |
| `inertia.inverse-diagonal` | `GetInvInertia` is the inverse tensor's diagonal |
| `inertia.velocity-offset` | `CalculateVelocityOffset` gives I^-1 times the torque |
| `inertia.energy` | `GetEnergy` is 1/2 w.I.w |
| `inertia.transfer-keeps-model` | `TransferObject` into a legacy environment keeps the object's tensor |
| `inertia.dynamics-momentum`, `inertia.dynamics-wanders` | The turned box, spun about an object axis in zero gravity, keeps the world angular momentum its tensor implies (direction within 3 degrees, magnitude 0.8 to 1.03 times) while its angular velocity wanders more than 10 degrees |

## Sensitivity

`physics_bench.py --sensitivity` injects these host faults. Each must fail
`shape-inertia.contract`:

- `inertia-model-ignored`: the model is accepted but objects stay legacy.
- `inertia-accepts-populated`: a model change is accepted after objects exist.
- `inertia-tensor-diagonal`: the products of inertia are reported as zero.
- `inertia-clamped`: `rotInertiaLimit` is applied under the shape model.

## Whole-suite diagnostic

`vphysics_conformance --suite-shape-inertia`, or
`physics_conformance.py --candidate-shape-inertia` for candidates only, runs
the whole parity suite with every environment on the shape model. It shows
what the game's opt-in changes. It is not a parity pass: parity is the legacy
model's obligation.

## Game use

`-physics_shape_inertia` selects the model for the server and client
environments when the provider offers it (`PhysApplyInertiaModel`,
`game/shared/physics_shared.cpp`). `-physics_shape_inertia_required` makes a
missing capability an error. `./play` (`run.conf` `PHYSICS_ARGS`) and
`./play_p2` pass `-physics_shape_inertia` by default; `PHYSICS_ARGS=` rolls
back. The launcher, the dedicated server and the Android APKs do not pass it.
