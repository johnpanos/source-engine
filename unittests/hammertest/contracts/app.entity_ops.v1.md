# Contract: `app.entity_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/entity_ops.h` · Impl: `hammer/core/app/ops/entity_ops.cpp`
Conformance: `unittests/hammertest/app/test_entity_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Entity property operations: keys on many entities, class changes, spawnflags, connections, rename with reference updates, world keys. Consumers: the entity inspector, the command layer. Required.

## 2. Accepted inputs

Object ids (groups expand; brush solids stand for their entity); keys and values; connections; an optional catalog.

## 3. Results and guarantees

- Multi-entity edits are one operation; a value every entity already has is Nothing.
- 'classname' and 'id' are reserved; class changes must keep the point/solid kind (catalog) and add the new class's defaults.
- Spawnflag edits take a single bit; connections need an output, a target and an input and a non-negative delay.
- Rename updates connection targets and entity-naming keys (catalog target_destination keys, or target/parentname/filtername/damagefilter/lightingorigin) that matched the old name exactly.

## 4. Ownership, threading

Pure functions over a borrowed `scene::DocumentEdit`; the session commits the
edit as one history unit. Single sequence.

## 5. Invariants

Every successful result passes `scene::ValidateEdit`. A refusal returns
`Rejected` or `Nothing`; the session then discards the staged edit.

## 6. Side effects and performance

None beyond the staged edit. Cost is proportional to the objects touched.

## 7. Conformance suite and providers

`test_entity_ops.cpp` (41 checks), with refusals for reserved keys, kind mismatches, non-bit flags, incomplete connections and rename collisions. Built with gcc and clang (`-Wall -Wextra -Werror`) on
`linux-headless-core`.
