# Contract: `app.property_value.v1`

Module: `hammer.app` · Type: `hammer::app::PropertyValue`
Header: `public/hammer/app/property_value.h` · Impl: `hammer/core/app/property_value.cpp`
Conformance: `unittests/hammertest/app/test_property_value.cpp` (+ `_negative`)
Migration: `HAM-SEL-001`

Owns the multi-selection aggregation of a single property, preserving the RFC
requirement that mixed values stay distinguishable from empty strings and from
unset properties. This is a foundational value type for the selection/property
worked migration; it does not yet own the property-draft commit policy.

## 1. Purpose, consumers

Aggregate one property across a selection into a display/edit state. Consumers:
the property inspector and the selection-change draft resolution. Required.

## 2. States and inputs

- Three states: `kUnset` (no contributor), `kSingle` (all agree; the agreed value
  MAY be the empty string), `kMixed` (contributors disagree).
- `AddContributor( value )` folds one contributor's string value in.

## 3. Results and guarantees

- `Unset().AddContributor(v)` -> `Single(v)`.
- `Single(v).AddContributor(v)` -> `Single(v)` (agreement, including `"" + ""`).
- `Single(a).AddContributor(b)` with `a != b` -> `Mixed`.
- `Mixed().AddContributor(v)` -> `Mixed` (absorbing).
- `Value()` is meaningful only in `kSingle`; the empty string there is a real
  value, never conflated with `kUnset`.
- Equality distinguishes all three states (and single values).

## 4. Ownership, threading

- Immutable value type (folds return new values). No globals, no I/O, not shared
  mutable state; safe to copy freely.

## 5. Invariants

- **empty != unset**: a lone empty-string contributor yields `Single("")`, never
  `kUnset`.
- **mixed is distinct**: `kMixed` is never equal to any `kSingle` or to `kUnset`.
- **mixed is absorbing**: once mixed, further contributors keep it mixed.

## 6. Side effects and performance

- None; O(1) per fold.

## 7. Conformance suite and providers

- `test_property_value.cpp`: three-state distinctness and the full aggregation
  table (agreement, empty vs nonempty, absorbing mixed).
- `test_property_value_negative.cpp`: an aggregator that skips empty strings
  (collapsing `Single("")` into `Unset`) must be **detected**.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.
