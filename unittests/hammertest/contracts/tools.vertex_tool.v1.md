# Contract: `tools.vertex_tool.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/vertex_tool.h` · Impl: `hammer/core/tools/vertex_tool.cpp`
Conformance: `unittests/hammertest/tools/test_vertex_tool.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Vertex editing of the selected solids (legacy `ToolMorph.cpp`). Handles at
vertices (`SolidVertexList`) and edge midpoints (`SolidEdgeList`).

## 2. Accepted inputs

Clicks (Ctrl toggles), 2D drags (Shift constrains, Alt frees), Escape.

## 3. Results and guarantees

- Handle selection is tool state: a click selects every handle drawn at the
  spot (stacked vertices in 2D, as legacy); Ctrl toggles; a click on nothing
  clears (Ctrl keeps). It is dropped after any document change other than the
  tool's own commit.
- A 2D drag moves the selected handles so the grabbed one lands on the grid;
  preview per solid via `RebuildFromVertices` (Error role when invalid).
- Release: one `Execute` "Move vertices" calling `MoveVertices` for every
  affected solid (edge handles move both vertices); moved handles stay
  selected. A refused move is `Failed` with the reason and no edit.
- Escape/focus/capture loss/tool switch: the previous handle selection, no
  edit. 3D: picking only; drags make no edit.

## 4. Ownership, threading

Owns the handle selection (by solid and index) and the session revision it
belongs to.

## 5. Invariants

At most one edit per drag, covering all affected solids.

## 6. Side effects and performance

Handle lists are rebuilt from the document per request.

## 7. Conformance suite and providers

`test_vertex_tool.cpp` (45 checks, gcc and clang). Negative checks: a
flattening move and a corner dragged into the solid's interior (Error
preview, `Failed`), cancellation, an external undo
dropping the handle selection, 3D drags.
