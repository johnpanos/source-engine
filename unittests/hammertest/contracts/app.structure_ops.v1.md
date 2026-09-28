# Contract: `app.structure_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/structure_ops.h` · Impl: `hammer/core/app/ops/structure_ops.cpp`
Conformance: `unittests/hammertest/app/test_structure_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Structural operations: delete with cleanup, group/ungroup, tie to entity, move to world, quick hide. Consumers: the selection tool, the outliner, the command layer. Required.

## 2. Accepted inputs

Object ids; a class name and optional catalog; an existing brush entity id.

## 3. Results and guarantees

- Deleting a group deletes its members; deleting an entity deletes its solids; brush entities emptied by an operation and groups it emptied are removed; authored empty groups stay.
- A solid of a brush entity stands for its entity when grouping; a shared parent group nests the new group; grouping into oneself is refused.
- Tie to entity needs a solid class (catalog) and applies its defaults; move to world returns solids to the entity's group.
- Hide unselected hides every visible top-level object not containing a kept one, descending into groups and brush entities that do.

## 4. Ownership, threading

Pure functions over a borrowed `scene::DocumentEdit`; the session commits the
edit as one history unit. Single sequence.

## 5. Invariants

Every successful result passes `scene::ValidateEdit`. A refusal returns
`Rejected` or `Nothing`; the session then discards the staged edit.

## 6. Side effects and performance

None beyond the staged edit. Cost is proportional to the objects touched.

## 7. Conformance suite and providers

`test_structure_ops.cpp` (36 checks), every result checked with ValidateEdit. Built with gcc and clang (`-Wall -Wextra -Werror`) on
`linux-headless-core`.
