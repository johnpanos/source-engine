# Contract: `app.entity_editor.v1`

Module: `hammer.app` · Types: `hammer::app::EntitySelection`, entity methods on
`hammer::app::EditorDocument`
Headers: `public/hammer/app/entity_selection.h`, `public/hammer/app/editor_document.h`
Impl: `hammer/core/app/entity_selection.cpp`, `hammer/core/app/editor_document.cpp`
Conformance: `unittests/hammertest/app/test_entity_editor.cpp` (+ `_negative`)
Migration: `HAM-SEL-001`

Owns the **shared entity editor logic**: how a UI shell's selection over the
document's top-level entities turns into a multi-selection property display
(`PropertyValue`) and into atomic multi-entity edits on the one history authority.
Both sibling shells (`hammer.adapters.gtk`, `hammer.adapters.mfc`) route through
this; neither re-derives selection or multi-select editing. Required.

## 1. Purpose, consumers

Give the property inspector and its editing commands one owner. A shell owns only
the `EntitySelection` (view state); the aggregation and mutation policy belong to
`EditorDocument`, which also owns content and history. This is what lets the GTK
and MFC shells share identical entity-editing behavior.

## 2. States and inputs

- **Entities** are the document's top-level blocks (`Content().children`),
  addressed by a stable index in document order. `EntityCount()`, `EntityName(i)`
  (the VMF block keyword), `EntityClassName(i)` (its `classname`, or `""`).
- **`EntitySelection`** is an ordered, deduplicated set of indices. `Add`, `Remove`,
  `Toggle`, `Clear`, `Contains`. Click order is not preserved; `Indices()` is
  sorted ascending and unique.
- **`AggregateProperty(selection, key)`** and
  **`SetPropertyOnSelection(selection, key, value)`** consume a selection.

## 3. Results and guarantees

- `AggregateProperty` returns `PropertyValue` under an explicit **presence-only**
  policy: only entities that *define* `key` contribute; an entity lacking it is a
  non-contributor (never forces `Mixed`). An entity whose value is `""` DOES
  contribute `""`. Empty selection or no contributor -> `Unset`.
- `SetPropertyOnSelection` sets `key`=`value` on every selected in-range entity,
  adding the key where absent, and commits **exactly one** history unit iff the
  document actually changed.

## 4. Ownership, threading

- `EntitySelection` is a plain value type (no policy, no I/O, freely copyable).
- `EditorDocument` remains the single content + history authority; entity edits go
  through the same `CommitSnapshot` path as every other edit. No globals, no MFC,
  no tier0/PCH/GPU.

## 5. Invariants

- **atomic multi-entity edit**: a set across N selected entities is ONE undo unit;
  a single `Undo` restores every touched entity. (Sensitivity target.)
- **no-op neutrality**: an edit that changes nothing records no history and does
  not mark the document modified.
- **empty != unset** in aggregation: a selection whose only contributor is `""`
  aggregates to `Single("")`, never `Unset`.
- **range safety**: out-of-range/stale indices are ignored by both aggregation and
  mutation; a selection of only out-of-range indices is a no-op edit.
- **selection is order-independent**: selections built by different click orders
  compare equal and iterate identically.

## 6. Side effects and performance

- `AggregateProperty` is read-only, O(selection × keys-per-entity). A grouped edit
  copies the content once and compares once (same cost model as `SetFirstBlockKey`).

## 7. Conformance suite and providers

- `test_entity_editor.cpp`: selection value-type laws; enumeration; the full
  aggregation table (agree/disagree/empty/unset/partial/stale); atomic multi-entity
  undo/redo; add-absent-key and empty/out-of-range edit edge cases.
- `test_entity_editor_negative.cpp`: a non-atomic editor that commits one history
  unit per entity (so one `Undo` reverts only the last) must be **detected**.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.
