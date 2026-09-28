# Contract: `app.vertex_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/vertex_ops.h` · Impl: `hammer/core/app/ops/vertex_ops.cpp`
Conformance: `unittests/hammertest/app/test_vertex_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Vertex and face editing of convex solids: vertex/edge lists, moving vertices (hull rebuild), face push/pull and face extrusion. Consumers: the vertex tool, the command layer. Required.

## 2. Accepted inputs

A solid id and vertex indices into SolidVertexList; a delta; a face reference and a distance.

## 3. Results and guarantees

- A move that would leave a moved vertex strictly inside the hull (concave) or flatten the solid is refused with nothing staged; merging vertices is legal.
- Faces on an unchanged plane keep their side data and persistent id; changed faces take the texture of the closest old normal and a fresh id.
- Push/pull keeps the face's id and refuses a distance that would remove the face; extrusion creates a new solid in the same owner and group.

## 4. Ownership, threading

Pure functions over a borrowed `scene::DocumentEdit`; the session commits the
edit as one history unit. Single sequence.

## 5. Invariants

Every successful result passes `scene::ValidateEdit`. A refusal returns
`Rejected` or `Nothing`; the session then discards the staged edit.

## 6. Side effects and performance

None beyond the staged edit. Cost is proportional to the objects touched.

## 7. Conformance suite and providers

`test_vertex_ops.cpp` (28 checks): vertex, edge and merge moves, concave and flat refusals, push/pull, extrusion. Built with gcc and clang (`-Wall -Wextra -Werror`) on
`linux-headless-core`.
