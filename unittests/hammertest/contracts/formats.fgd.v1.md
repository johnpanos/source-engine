# Contract: `formats.fgd.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/fgd.h` · Impl: `hammer/core/formats/fgd.cpp`
Conformance: `unittests/hammertest/formats/test_fgd.cpp` (+ `_negative`)
Migration: `HAM-FGD-001`
Depends on: C++ standard library only (self-contained tokenizer + parser).

Parses FGD (Forge Game Data) text into a queryable entity-class table: the schema
that drives the editor's entity property sheet ("FGD-driven entity editing").
Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, required vs optional

Own the entity schema. Consumers: the entity property editor (which keys a class
exposes, their types/defaults/choices) and validation of authored entity keys.
Required for entity editing.

## 2. Accepted inputs

- `ParseFgd( text )`: FGD source. Recognized class directives: `@PointClass`,
  `@SolidClass`, `@BaseClass`, `@KeyValueClass`, `@NPCClass`, `@FilterClass`,
  `@MoveClass`. Class header: helpers (`base(...)`, `size(...)`, `color(...)`, …)
  up to `=`, then the classname, an optional `: "description"`, and a `[ … ]`
  body of properties. A property is `name(type) [modifiers] [: display [: default
  [: help]]] [= [ value : "label" [: flagdefault] … ]]`. `//` comments and string
  `"a" + "b"` concatenation are handled.
- `ResolveClass( classes, name )`: flattens `base()` inheritance.

## 3. Results and guarantees

- On success `ok == true` with one `EntityClass` per class: `kind`, `name`,
  `description`, `bases` (in declared order), and own `properties` (each with
  `name`, `type`, `displayName`, `defaultValue`, `help`, and `choices` for
  choices/flags). Colon fields may be empty and are preserved as empty.
- Unknown top-level directives (`@mapsize`, `@include`, `@MaterialExclusion`,
  `@AutoVisGroup`, …) are **skipped, not errors**.
- On malformed class syntax `ok == false` with `error` + 1-based `errorLine`.
- `ResolveClass` returns the class with base properties merged first (recursively,
  in `base()` order) then its own, a later same-key property overriding an earlier
  one; `nullopt` for an unknown class. Cyclic bases are visited once (no loop).

## 4. Ownership, threading

- Pure function over the input string; no globals, no I/O, no shared state.

## 5. Invariants

- **Inheritance order**: resolved properties list base-inherited keys before own.
- **Override**: a class property replaces a same-named inherited one.
- **Bounded**: cyclic `base()` references terminate (each class visited once).

## 6. Side effects and performance

- One linear tokenize pass + one parse pass; O(text). No logging or globals.

## 7. Conformance suite and providers

- `test_fgd.cpp`: class kinds (`@PointClass`/`@SolidClass`/`@BaseClass`), typed
  properties, a `choices` list (values + labels), a `flags` list, an empty default
  field followed by help text, `base(Targetname, Angles)`, and `ResolveClass`
  putting inherited keys first with own keys present.
- `test_fgd_negative.cpp`: proves a well-formed FGD parses while a property missing
  `(type)`, an unterminated class body, a missing class name, and a stray
  top-level token are all reported as errors (oracle soundness).
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Parses class key properties and choices/flags. `input`/`output` (I/O)
  declarations are parsed but **not stored**; helper metadata other than `base`
  (size/color/iconsprite/studio/…) is ignored; `@include` is not resolved (the
  referenced file is not loaded). Those are later increments.
