# Contract: `app.find_replace_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/find_replace_ops.h` · Impl: `hammer/core/app/ops/find_replace_ops.cpp`
Conformance: `unittests/hammertest/app/test_find_replace_ops.cpp`
Migration: `R08-DOMAIN`
Depends on: `app.entity_ops` (`EntitiesOf`), `scene.map_queries` (`IsVisible`)

## 1. Purpose, consumers, scope

Entity search and key-value replacement (legacy `CEntityReportDlg` filters,
`CSearchReplaceDlg` Find / Replace All). Consumers: the entity report and
find/replace presenters, the command layer and MCP. Searching inside
instanced maps and renaming with reference updates (`RenameEntity`) are out
of scope. Required.

## 2. Accepted inputs

An `EntityQuery`: class pattern and mode, key (case-insensitive exact, empty
= any), value pattern and mode (`Contains`, `Whole`, `Wildcard` with `*` and
`?`), value case sensitivity, `visibleOnly`, and an optional scope of ids
(expanded like `EntitiesOf`). A replacement string.

## 3. Results and guarantees

- An entity matches when its class matches (always case-insensitive), some
  key named `key` has a matching value (a key alone matches its presence),
  and, without `key`, a key value or a connection target or parameter
  matches; an empty query matches every entity. Results are in id order.
- Replace rewrites each matching key value (and, without `key`, connection
  targets and parameters): `Whole`/`Wildcard` replace the value, `Contains`
  every non-overlapping occurrence (legacy replaced only the first; a
  deliberate fix). `count` = values changed; case-only changes count.
- Refusals: an empty value pattern (`Rejected`); nothing matched or nothing
  changed (`Nothing`); `count` is 0 and nothing is staged.

## 4. Ownership, threading

Pure functions of the reader or edit. Single sequence.

## 5. Invariants

Only `keys` and connection targets/parameters change; classnames, ids,
outputs and inputs are never touched.

## 6. Side effects and performance

Linear in entities x (keys + connections) x pattern length.

## 7. Conformance suite and providers

`test_find_replace_ops.cpp` (32 checks, gcc and clang, `-Wall -Wextra
-Werror`): the three text modes and glob backtracking, class/key/value and
connection matching, case, visibility and scope, substring/whole/wildcard
replacement with counts, and negative checks that stage nothing. Seeded
fault detected: first-occurrence-only replacement.
