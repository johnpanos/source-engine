# Contract: `app.create_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/create_ops.h` · Impl: `hammer/core/app/ops/create_ops.cpp`
Conformance: `unittests/hammertest/app/test_create_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Creating objects: legacy stock primitives (block, wedge, cylinder, spike, sphere), arches, and point entities with catalog defaults. Consumers: the block and entity tools, the command layer. Required.

## 2. Accepted inputs

A box, a PrimitiveSpec (kind, sides 3..32, axis), an ArchSpec, a face texture template; an entity class, origin and an optional catalog.

## 3. Results and guarantees

- Primitive vertices follow legacy polyMake (ellipse in the box, start at +Y, clockwise from above) rounded to whole units; sides face outward with world-aligned axes.
- Degenerate boxes, side counts out of range, bad axes and empty materials are refused.
- Arches are one convex solid per segment inside a new group; a wall of half the box or more collapses the inside to the center (legacy).
- Placed entities take the catalog spelling of the class, every key with a default, and spawnflags as the sum of default-on flags; unknown and solid classes are refused.

## 4. Ownership, threading

Pure functions over a borrowed `scene::DocumentEdit`; the session commits the
edit as one history unit. Single sequence.

## 5. Invariants

Every successful result passes `scene::ValidateEdit`. A refusal returns
`Rejected` or `Nothing`; the session then discards the staged edit.

## 6. Side effects and performance

None beyond the staged edit. Cost is proportional to the objects touched.

## 7. Conformance suite and providers

`test_create_ops.cpp` (52 checks): each primitive's face count, orientation, whole-unit vertices and bounds; per-axis orientation; arches and spirals; entity defaults; every refusal. Built with gcc and clang (`-Wall -Wextra -Werror`) on
`linux-headless-core`.
