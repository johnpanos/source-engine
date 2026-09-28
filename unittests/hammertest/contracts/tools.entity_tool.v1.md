# Contract: `tools.entity_tool.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/entity_tool.h` · Impl: `hammer/core/tools/entity_tool.cpp`
Conformance: `unittests/hammertest/tools/test_entity_tool.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Place point entities of `EditorSettings::entityClass` (legacy `ToolEntity.cpp`,
Source 2 click-to-place). All views; Left button.

## 2. Accepted inputs

Presses, drags and releases; the optional entity catalog (class defaults,
marker boxes).

## 3. Results and guarantees

- Placement on release (below the threshold: at the press point) as one
  `ExecuteSelecting` "Place entity" with `ops::PlaceEntity`, selecting it.
- 2D: the snapped plane point; depth from the last 3D placement, else 0.
- 3D: the first solid surface hit, snapped on the tangent axes, pushed along
  the normal until the class marker box touches the surface.
- Hover shows the marker where a click would place it (in that view).
- No class, an unknown or brush class (with a catalog), or no surface:
  `Failed` with the reason, no edit. Escape and focus/capture loss place
  nothing.

## 4. Ownership, threading

Owns the press, the hover preview and the last 3D placement.

## 5. Invariants

At most one edit per press.

## 6. Side effects and performance

A 3D hover casts one ray per pointer move.

## 7. Conformance suite and providers

`test_entity_tool.cpp` (35 checks, gcc and clang) with the fake catalog.
Negative checks: cancellation, missing/unknown/brush classes, sky clicks,
other buttons.
