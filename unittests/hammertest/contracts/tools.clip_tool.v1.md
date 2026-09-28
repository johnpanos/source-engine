# Contract: `tools.clip_tool.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/clip_tool.h` · Impl: `hammer/core/tools/clip_tool.cpp`
Conformance: `unittests/hammertest/tools/test_clip_tool.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Clip the selected solids by a plane drawn as a line in a 2D view (legacy
`ToolClipper.cpp`). 2D views only.

## 2. Accepted inputs

A two-point line drag; drags of either point; Shift+X, Enter, Escape.

## 3. Results and guarantees

- The plane contains the snapped line and the view's free axis; its normal is
  Normalize((b - a) x viewDirection), so Front = left of the line as drawn.
- Shift+X cycles Front -> Back -> Both (legacy order, starting at Front).
- Preview: the kept pieces (`ops::SplitSolid`) of every solid the plane
  crosses, the line and its point handles in the drawing view kind.
- Enter: one `ExecuteSelecting` "Clip" (`ops::ClipSolids`, cap faces take
  the editor face texture) keeping the pieces selected; the line is cleared.
  No selected solid, or a plane crossing none of them: `Failed`, no edit, the
  line stays.
- Clicks and coincident points make no line; Escape/focus loss restore the
  previous line; Escape without a drag and tool switches clear it.

## 4. Ownership, threading

Owns the line and the keep mode.

## 5. Invariants

A stored line has two distinct points.

## 6. Side effects and performance

Previews split each selected solid per overlay request.

## 7. Conformance suite and providers

`test_clip_tool.cpp` (50 checks, gcc and clang). Negative checks: clicks,
coincident points, cancellation, missing selection, missed plane, 3D events
`Ignored`.
