# Contract: `presenters.entity_inspector.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/entity_inspector.h` (+ `object_label.h`) · Impl: `hammer/core/presenters/entity_inspector.cpp`, `object_label.cpp`
Conformance: `unittests/hammertest/presenters/test_entity_inspector.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The Object Properties model (legacy `ObjectPage`/`op_entity` SmartEdit,
`op_flags`, `op_output`, `op_input`) with no toolkit: class, key rows, flags,
outputs and inputs of the entities the selection stands for
(`ops::EntitiesOf`), plus the property draft. Consumers: the GTK inspector
widget, scripts. Required for property editing.

## 2. Accepted inputs

A session and an optional `IEntityCatalog`. Drafts: any key except `classname`,
`id`, empty, quoted, line-broken or read-only keys. Immediate requests: class,
flag bit, key removal, output add/replace/remove.

## 3. Results and guarantees

- Rows: schema keys in catalog order (single class) or the keys every class
  declares (mixed classes; none if a class is unknown), then extra keys in
  first-seen order; Flags-typed keys go to the flags section. SmartEdit off:
  raw keys, flags key included.
- Values are `app::PropertyValue`: present on some entities and absent on
  others is Mixed; absent on all is Unset; `""` is Single.
- Flags are On/Off/Mixed; an absent flags key reads as the schema default
  (its default value, else the `defaultOn` bits).
- Outputs carry target validity (procedural `!` targets, or a targetname
  match with `*` wildcards, or a classname match); inputs are connections of
  other entities naming a selected entity.
- With no entity inspected (nothing selected, or only world brushes/faces)
  the worldspawn is shown (`IsWorld`): class `worldspawn`, its catalog schema,
  `DocumentSettings::worldKeys`; drafts commit with `ops::SetWorldKey` ("Edit
  world properties"); class/flag/key-removal/output operations are refused.
- `Commit` applies every drafted key in ONE `Execute("Edit properties")` —
  `targetname` through `ops::RenameEntity` with catalog-aware reference
  updates, other keys through `ops::SetKey` — or refuses the whole draft
  (Rejected, naming each invalid key) and changes nothing. Validation rules
  are listed in the header (`ValidateKeyValue`).
- Selection guard: no draft allows; an invalid draft vetoes and reports the
  key; a valid draft is committed inside the guard (its own undo step,
  recorded before the selection changes, so undo restores the drafted
  selection); a commit the session refuses vetoes the change.
- A draft whose entities change without the guard (undo/redo, edits that
  select) is discarded and reported, never applied elsewhere.
- Class change, flag toggle, key removal and output edits are one labeled undo
  step each ("Change class to X", "Set flag L"/"Clear flag L", "Remove key K",
  "Add output O", "Edit output O", "Remove output O" via
  `ops::RemoveConnectionAt`, exactly one connection).

## 4. Ownership, threading

Owns only the draft and view flags. Single sequence (the session's).
Subscription and guard are RAII.

## 5. Invariants

The draft always targets the entities it was started on. The document changes
only through `EditSession::Execute` with ops.

## 6. Side effects and performance

Rebuilds on every session event: O(selected entities x keys) plus O(entities x
connections) for outputs/inputs.

## 7. Conformance suite and providers

`test_entity_inspector.cpp` (102 checks, gcc and clang) with the fake catalog;
negative checks for invalid drafts, reserved/read-only keys, vetoes,
world-refused operations, discarded drafts, replacement and destruction order.
