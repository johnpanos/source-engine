# Contract: `presenters.material_browser.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/material_browser.h` · Impl: `hammer/core/presenters/material_browser.cpp`
Conformance: `unittests/hammertest/presenters/test_material_browser.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The texture browser model (legacy `texturebrowser.cpp`, Source 2 Asset
Browser material view): material names, keyword filter, used-in-map view with
face counts, recent list and the active material.

## 2. Accepted inputs

A session, an `IMaterialInfo`, the `app::EditorSettings` owner, a recent-list
bound. Keyword text, the used-only flag, a material to activate.

## 3. Results and guarantees

- Names compare case-insensitively with `/` and `\` equivalent.
- Keyword filter: whitespace-separated words, all must be case-insensitive
  substrings of the name.
- Without used-only: the port's names in its order with face counts. With it:
  every material the map uses, sorted by normalized name, with `known = false`
  for ones the port lacks.
- Face counts follow edits, undo and replacement.
- `SetActive` refuses unknown materials without changing anything; otherwise
  it writes the port's spelling to `EditorSettings::faceTexture.material`
  (the one owner; `Active()` reads it live) and pushes it on the bounded
  recent list.

## 4. Ownership, threading

Owns filter/recent view state; the active material belongs to
`EditorSettings`. Single sequence. RAII subscription.

## 5. Invariants

Rows pass the keyword filter; counts equal the document's sides per material.

## 6. Side effects and performance

Recount is O(sides) per document change; filtering is O(names x words).

## 7. Conformance suite and providers

`test_material_browser.cpp` (19 checks, gcc and clang) with the fake material
port; negative checks for unknown materials, no-match filters and destruction
order.
