# Contract: `app.csg_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/csg_ops.h` · Impl: `hammer/core/app/ops/csg_ops.cpp`
Conformance: `unittests/hammertest/app/test_csg_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Constructive operations on convex solids: clip by a plane, carve, and hollow (legacy Make Hollow). Consumers: the clip tool, the command layer. Required.

## 2. Accepted inputs

Solid ids (groups and brush entities expand); a plane; a keep mode; a cap texture; a wall thickness.

## 3. Results and guarantees

- The first piece of a split solid keeps its id and untouched sides keep their persistent ids; other pieces are new with fresh side ids and the same owner, group and editor data.
- Solids that do not span the clip plane are untouched; carving leaves non-overlapping targets and the carvers untouched, and removes fully contained targets; cut faces take the carving face's texture.
- Hollow carves each solid by its offset copy and groups the walls (brush entities keep ownership); a solid too thin for the walls refuses the whole selection.
- Piece volumes sum to the original (clip) or original minus the carver (carve).

## 4. Ownership, threading

Pure functions over a borrowed `scene::DocumentEdit`; the session commits the
edit as one history unit. Single sequence.

## 5. Invariants

Every successful result passes `scene::ValidateEdit`. A refusal returns
`Rejected` or `Nothing`; the session then discards the staged edit.

## 6. Side effects and performance

None beyond the staged edit. Cost is proportional to the objects touched.

## 7. Conformance suite and providers

`test_csg_ops.cpp` (35 checks): keep back/front/both with ids and volumes, carve volumes and textures, contained targets, hollow shells in and out, brush entity walls, and refusals. Built with gcc and clang (`-Wall -Wextra -Werror`) on
`linux-headless-core`.
