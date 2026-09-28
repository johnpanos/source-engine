# Contract: `tools.tool.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/input.h`, `public/hammer/tools/tool.h` · Impl: `hammer/core/tools/input.cpp`
Conformance: `unittests/hammertest/tools/test_tool_manager.cpp` and every tool suite
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The normalized input values and the tool contract of RFC 0002's "Input and
tool contracts". Native hosts (GTK, MFC, scripts, tests) convert each event
once into `PointerEvent`, `KeyEvent`, `WheelEvent` or a cancellation; tools
answer with a `ToolResult` and describe an `OverlayList`, a `Cursor` and a
status line. Consumers: the tool manager, every tool, UI hosts. Required.

## 2. Accepted inputs

Pointer phase Down/Move/Up with button, modifiers (Shift/Ctrl/Alt bits), view
pixel coordinates, view kind, click count and timestamp. Logical keys
(characters as lower-case ASCII with Shift in the modifiers), press/release,
repeat. Wheel notches (positive = away from the user). A `ToolContext` per
call: the `EditSession`, the view (`ViewRef`: kind plus exactly one camera),
`app::EditorSettings` (the one owner of editing settings), tool-only
`ToolSettings`, optional catalog and material ports, and the grid policy
derived from the editor settings (`GridFrom`).

## 3. Results and guarantees

- `Ignored` = not relevant (valid); `Handled`; `CaptureRequested` = a gesture
  began; `CaptureReleased` = it ended; `Failed` = a refused edit, with the
  reason, and nothing changed.
- A tool mutates the session only when a gesture completes; cancelling
  restores the pre-gesture tool state and makes no edit.
- One completed gesture is at most one `Execute` (one history unit).
- Overlays are value lists of world primitives (line, box, polygon) and
  screen primitives (rect, handle, label) with a style role; no colors.

## 4. Ownership, threading

Tools own only their gesture and pending tool state; they keep no pointer to
the context between calls. Single sequence (the session's).

## 5. Invariants

`ViewRef` names one camera matching its kind. `GridFrom` yields a power of two
in [1, 1024] and the editor's snap flag.

## 6. Side effects and performance

None beyond the session calls a completed gesture makes. Previews stage a
scratch `DocumentEdit` (copies of touched objects only).

## 7. Conformance suite and providers

The tool suites drive real `EditSession`s through a `ToolManager`; the policy
suite checks `GridFrom`. Negative checks in every suite: unsupported views,
other buttons, cancellation without edits, failures with reasons.
