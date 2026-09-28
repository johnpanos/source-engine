# Contract: `app.transform_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/transform_ops.h` · Impl: `hammer/core/app/ops/transform_ops.cpp`
Conformance: `unittests/hammertest/app/test_transform_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Affine transforms of solids (authored points, texture with or without lock, displacement rows), entities (origin, angles or legacy yaw-only angle) and groups (their members), plus translate/rotate/scale-to-box/mirror/snap/align. Consumers: the selection tool, the clipboard, the command layer. Required.

## 2. Accepted inputs

Object ids (groups and brush entities expand to leaves); an Affine map; TransformOptions (texture lock).

## 3. Results and guarantees

- The complete selection is validated before any write: a map that makes any solid degenerate refuses everything.
- Mirrors swap side points so planes stay outward; transformed points within 1e-6 of an integer are snapped to it.
- Entity orientation composes forward and up through the map and is re-orthonormalized; yaw is written in [0, 360). Pure translations leave orientation keys untouched.
- Displacement start positions transform as points; normals and offsets as vectors, with a normal's stretch moved into its distance.
- Zero translations and full turns are Nothing; bad axes and flat target boxes are rejected.

## 4. Ownership, threading

Pure functions over a borrowed `scene::DocumentEdit`; the session commits the
edit as one history unit. Single sequence.

## 5. Invariants

Every successful result passes `scene::ValidateEdit`. A refusal returns
`Rejected` or `Nothing`; the session then discards the staged edit.

## 6. Side effects and performance

None beyond the staged edit. Cost is proportional to the objects touched.

## 7. Conformance suite and providers

`test_transform_ops.cpp` (44 checks): translation with lock and without, rotation (solids, angles, yaw-only), scale-to-box with texture stretch, mirror with outward planes and mirrored textures, atomic refusal of degenerate maps, snap and align, displacement rows. Built with gcc and clang (`-Wall -Wextra -Werror`) on
`linux-headless-core`.
