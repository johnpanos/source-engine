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
  `@MoveClass`, `@KeyFrameClass`; also `@include "file"`. Class header: helpers (`base(...)`, `size(...)`, `color(...)`, …)
  up to `=`, then the classname, an optional `: "description"`, and a `[ … ]`
  body of properties. A property is `name(type) [modifiers] [: display [: default
  [: help]]] [= [ value : "label" [: flagdefault] … ]]`; a member may also be
  `input Name(type) [: "help"]` or `output Name(type) [: "help"]`. `//`
  comments and string `"a" + "b"` concatenation are handled; a dangling `+`
  after a string (Valve's `halflife2.fgd` has one) ends the value.
- `ResolveClass( classes, name )`: flattens `base()` inheritance.

## 3. Results and guarantees

- On success `ok == true` with one `EntityClass` per class: `kind`, `name`,
  `description`, `bases` (in declared order), `line` (of the directive), own
  `properties` (each with `name`, `type`, `displayName`, `defaultValue`,
  `help`, `readOnly` from the `readonly` modifier, and `choices` for
  choices/flags, a flag's third field in `FgdChoice::defaultValue`), own
  `inputs` and `outputs` (`name`, `type`, `help`), and `helpers`: every header
  helper except `base()`, in order, with a lower-cased `name` and `args` split
  at commas (words of one argument joined by one space, strings unquoted; a
  bare helper word has no arguments). Colon fields may be empty and are
  preserved as empty.
- `@include "file"` directives are recorded in `includes` (source order) with
  their lines in the parallel `includeLines`; the files are not loaded (see
  `formats.fgd_entity_catalog.v1`). `@include` without a file name is an error.
- Other unknown top-level directives (`@mapsize`, `@MaterialExclusion`,
  `@AutoVisGroup`, …) are **skipped, not errors**.
- On malformed class syntax `ok == false` with `error` + 1-based `errorLine`.
- `ResolveClass` returns the class with base properties merged first (recursively,
  in `base()` order) then its own, a later same-key property overriding an earlier
  one in place; inputs and outputs merge the same way. A class's own helpers
  replace every inherited helper of the same name (the group takes the first
  inherited one's position), so repeated own helpers survive. Class, key, I/O
  and helper names compare case-insensitively (an exact class-name match is
  preferred). `nullopt` for an unknown class; an unknown base contributes
  nothing. Cyclic bases are visited once (no loop).

## 4. Ownership, threading

- Pure function over the input string; no globals, no I/O, no shared state.

## 5. Invariants

- **Inheritance order**: resolved properties list base-inherited keys before own.
- **Override**: a class property, input or output replaces a same-named
  (case-insensitive) inherited one in place.
- **Bounded**: cyclic `base()` references terminate (each class visited once).

## 6. Side effects and performance

- One linear tokenize pass + one parse pass; O(text). No logging or globals.

## 7. Conformance suite and providers

- `test_fgd.cpp`: class kinds (`@PointClass`/`@SolidClass`/`@BaseClass`), typed
  properties, a `choices` list (values + labels), a `flags` list, an empty default
  field followed by help text, `base(Targetname, Angles)`, and `ResolveClass`
  putting inherited keys first with own keys present. R08-DOMAIN additions:
  `@include` names and lines, `@KeyFrameClass`, helpers and their comma-split
  arguments (including `sphere()` and a bare helper word), `readonly`, flag
  default fields, stored inputs/outputs with concatenated help, a dangling
  `+`, `@include` without a file rejected, and resolution of helpers (in-place
  replacement, repeated own helpers), inputs/outputs and case-insensitive
  base and key names.
- `test_fgd_negative.cpp`: proves a well-formed FGD parses while a property missing
  `(type)`, an unterminated class body, a missing class name, and a stray
  top-level token are all reported as errors (oracle soundness).
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Helper arguments are kept as text; interpreting them (box, color, model,
  sprite hints) belongs to `formats.fgd_entity_catalog.v1`. `@include` is
  recorded, not loaded. `@MaterialExclusion` and `@AutoVisGroup` bodies are
  skipped.
- Behavior change (R08-DOMAIN): base-class lookup falls back to a
  case-insensitive match, and a property override matches its inherited key
  case-insensitively. Previously an unmatched-case base was ignored and a
  differently-cased key was appended as a second key.
