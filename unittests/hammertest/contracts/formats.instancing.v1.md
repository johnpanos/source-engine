# Contract: `formats.instancing.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/instancing.h` · Impl: `hammer/core/formats/instancing.cpp`
Conformance: `unittests/hammertest/formats/test_instancing.cpp` (+ `_negative`)
Migration: `HAM-INSTANCE-001`
Depends on: `hammer.formats` (keyvalues codec), `hammer.ports` (`IFileStore`),
`hammer.geometry` (`Vec3d`)

Expands VMF `func_instance` entities: each references another VMF (`file`) placed
at an `origin`/`angles`, and expansion loads that child through the file-store
port, rotates and translates its world solids and entities into the parent, and
replaces the instance with the merged result. This is Source's nested-prefab
mechanism, headless and MFC-free.

## 1. Purpose, consumers, required vs optional

Own instance flattening. Consumers: map load/compile (a map with instances must
flatten before rendering/building) and the editor's "collapse instance" command.
Required for any map using `func_instance`.

## 2. Accepted inputs

- `ExpandInstances( root, store, maxDepth = 16 )`: a parsed VMF document `root`, a
  `hammer::ports::IFileStore` that resolves instance `file` paths, and a recursion
  bound. Each `func_instance` entity carries `file` (required), `origin` (default
  `0 0 0`), and `angles` (Source QAngle `pitch yaw roll`, default `0 0 0`).

## 3. Results and guarantees

- On success: `ok == true`, `document` is the flattened VMF with **no**
  `func_instance` entities remaining, `expanded` counts every instance expanded at
  all nesting levels.
- World solids from each instance are transformed (rotate then translate) and
  merged into the parent world; non-instance entities are appended with their
  `origin` transformed.
- Rotation uses the Source QAngle→matrix convention; nesting composes transforms:
  a child point `p` under instance `(Rᵢ, oᵢ)` inside placement `(R, o)` maps to
  `(R·Rᵢ)·p + (R·oᵢ + o)`.
- On error: `ok == false`, `error` is a human-readable diagnostic, `document` is
  empty. Error cases: a `func_instance` with no `file`; an unreadable/unparseable
  child file; recursion beyond `maxDepth` (a self- or cyclic reference).

## 4. Ownership, threading

- Pure transform over the input tree + the injected store; no globals, no shared
  mutable state. Reads the store; performs no writes.

## 5. Invariants

- **No instances remain** in a successful result.
- **Bounded**: a cyclic reference terminates with an error at `maxDepth`, never an
  infinite loop.
- **Composition**: nested placement composes as matrix product + placed origin
  (verified: `b` at `+100` inside `a` at `+5` lands at `+105`).

## 6. Side effects and performance

- One `IFileStore::Read` per (possibly repeated) instance file; O(total solids +
  entities) copies. No logging or globals.

## 7. Conformance suite and providers

- `test_instancing.cpp`: translate-only placement (child solid moved by origin),
  yaw-90 rotation (`(16,0,0) → (0,16,0)`), and nested composition (`+100` then
  `+5` → `+105`), each also asserting the `func_instance` is removed and solids
  merge into one world. Uses the `InMemoryFileStore` fake for child VMFs.
- `test_instancing_negative.cpp`: proves a well-formed instance expands while a
  missing file, a cyclic reference (depth guard), and a `func_instance` with no
  `file` are all reported as errors (oracle soundness).
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Transforms world solid **plane geometry** and entity **origins**. Does **not**
  yet: rotate side texture axes (`uaxis`/`vaxis`), compose entity `angles`
  (only position moves), apply `$fixup`/`replace*` instance variables, or merge
  instance-level visgroups/groups. Those are later increments; the geometry
  flattening and the transform composition are the core this slice pins.
