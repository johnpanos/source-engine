# Contract: `formats.map_export.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/map_export.h` · Impl: `hammer/core/formats/map_export.cpp`
Conformance: `unittests/hammertest/formats/test_map_export.cpp` (+ `_negative`)
Migration: `HAM-EXPORT-001`
Depends on / composes: `hammer.formats` instancing (`ExpandInstances`), visgroups
(`ApplyVisGroupFilter`), cordon (`ApplyCordon`); `hammer.ports` (`IFileStore`).

Turns an authored VMF into the compile-ready geometry a map compiler sees, by
composing the single-purpose format cores in the correct order. It is the
headless entry point behind the shell's "Run Map" / "Export" action. Strict,
MFC-free, GPU-free — it adds no new transform logic, only ordering.

## 1. Purpose, consumers, required vs optional

Own the export/compile-prep pipeline. Consumers: the compile/run path and export.
Required for compiling maps that use instances/visgroups/cordon.

## 2. Accepted inputs

- `PrepareForCompile( root, store, CompileOptions{hiddenVisGroups, useCordon,
  cordon} )`: the authored document, a file store (for instance files), and the
  compile options.

## 3. Results and guarantees

- Runs, in order: **(1)** `ExpandInstances` (flatten func_instances via `store`),
  **(2)** `ApplyVisGroupFilter` (drop objects in hidden VisGroups — editor-hidden
  geometry is not compiled), **(3)** optionally `ApplyCordon` (clip to the box for
  a partial compile).
- Returns the resulting `document` plus `instancesExpanded`,
  `solidsRemovedByVisGroup`, `solidsRemovedByCordon`.
- **Failure propagates**: if instance expansion fails (missing/cyclic/malformed
  instance file), `ok == false` with a diagnostic and no partial document is
  emitted.

## 4. Ownership, threading

- Pure composition over the cores + injected store; no globals. Reads the store.

## 5. Invariants

- **Ordering**: instances expand before visgroup/cordon filtering, so
  instance-merged geometry is subject to the same filters.
- **Composition, not reimplementation**: each stage is the existing core; this
  module contains no transform/intersection logic of its own (DRY).
- **All-or-nothing on stage failure**: a failed stage aborts with an error.

## 6. Side effects and performance

- One instance expansion + one visgroup pass + at most one cordon pass. No
  logging or globals.

## 7. Conformance suite and providers

- `test_map_export.cpp`: a map with a func_instance, a hidden-visgroup solid, and
  an out-of-cordon solid → the instance expands, the hidden solid drops, the far
  solid clips, and exactly the expected solids survive (with/without cordon).
- `test_map_export_negative.cpp`: a func_instance with a missing file makes the
  whole prepare fail with a diagnostic (not a silent partial); a clean map
  succeeds.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Composes instancing + visgroups + cordon. It does not run BSP/VIS/RAD, invoke
  external compilers, or resolve materials; and cordon-boundary solid splitting is
  the cordon core's declared limit. Those are later increments / the compiler
  toolchain.
