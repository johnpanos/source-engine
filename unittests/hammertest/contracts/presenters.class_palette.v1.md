# Contract: `presenters.class_palette.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/class_palette.h` · Impl: `hammer/core/presenters/class_palette.cpp`
Conformance: `unittests/hammertest/presenters/test_class_palette.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The Entity tool's searchable class list (Source 2) and the class dropdown:
catalog classes grouped, filtered, ranked; recent classes; the active class.

## 2. Accepted inputs

An `IEntityCatalog`, the `app::EditorSettings` owner, an optional session (map
counts), a recent-list bound. Search text, kind filter, optional category,
class to activate.

## 3. Results and guarantees

- `CategoryOf`: first matching prefix ignoring case, in the order info_ Info,
  func_ Func, light Lights, prop_ Props, trigger_ Triggers, logic_ Logic, env_
  Environment, npc_ NPCs, point_ Point, weapon_ Weapons, item_ Items, filter_
  Filters, ai_ AI, path_ Paths, game_ Game; else Other.
- `Categories()` lists the used categories in that order with counts of
  entries passing search and kind (not the category filter).
- Kind filter: Point includes point-like `Other` classes; Solid only brush
  classes.
- Search (trimmed, case-insensitive): rank 0 name prefix, 1 name substring, 2
  description substring; catalog order within a rank.
- `SetActive` refuses unknown classes without changing anything; otherwise it
  writes the catalog spelling to `EditorSettings::entityClass` (the one owner;
  `Active()` reads it live) and pushes it on the bounded recent list (most
  recent first, no duplicates).
- With a session, `countInMap` counts entities per class and follows edits.

## 4. Ownership, threading

Owns search/filter/recent view state; the active class belongs to
`EditorSettings`. Single sequence. RAII subscription.

## 5. Invariants

Entries are a subset of the catalog's classes; ranks are non-decreasing.

## 6. Side effects and performance

Refresh is O(classes + entities) on document changes and filter changes.
Settings writes by another owner do not move `Revision()` (no notification
exists).

## 7. Conformance suite and providers

`test_class_palette.cpp` (25 checks, gcc and clang) with the fake catalog;
negative checks for unknown classes, no-match searches and destruction order.
