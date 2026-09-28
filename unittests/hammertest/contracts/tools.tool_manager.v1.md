# Contract: `tools.tool_manager.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/tool_manager.h` · Impl: `hammer/core/tools/tool_manager.cpp`
Conformance: `unittests/hammertest/tools/test_tool_manager.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Owns the tools, the active tool and pointer-capture bookkeeping, and routes
normalized input to the active tool (replacing legacy `CBaseTool`'s repeated
2D/3D/logical transport). Consumers: UI hosts. Global keys (save, undo, grid,
tool selection) are not routed here; they belong to the action catalog.

## 2. Accepted inputs

Tools by `unique_ptr` (unique names); activation by name; pointer, key and
wheel events with a `ToolContext`; focus loss, capture loss and explicit
cancellation (for example `DocumentReplaced`).

## 3. Results and guarantees

- An event whose view differs from its context's view, or a context without a
  camera, is `Failed` and not routed.
- Without capture, events for views outside the tool's `SupportedViews` are
  `Ignored` before the tool sees them; no active tool ignores everything.
- `CaptureRequested` holds capture for that view: only that view's pointer
  events reach the tool until its gesture ends; the manager drops capture
  when the tool reports no gesture (a `Handled` result then becomes
  `CaptureReleased`; `Failed` stays `Failed`). Hosts release their native
  grab when `HasCapture()` turns false.
- Focus loss, capture loss and switching tools cancel the active gesture (no
  edit) and drop capture; switching also calls `Deactivate` on the old tool.
  Re-activating the active tool changes nothing.

## 4. Ownership, threading

Owns the tools. Single sequence.

## 5. Invariants

Capture is held only while the active tool is in a gesture.

## 6. Side effects and performance

Routing is a name lookup and two checks per event.

## 7. Conformance suite and providers

`test_tool_manager.cpp` (40 checks, gcc and clang) with a recording probe
tool and the real Selection tool (switch mid-drag: no edit, same selection).
Negative checks: duplicate/null registration, unknown activation, mismatched
contexts, other views during capture, no active tool.
