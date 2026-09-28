# Contract: `tools.camera_controller.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/camera_controller.h` · Impl: `hammer/core/tools/camera_controller.cpp`
Conformance: `unittests/hammertest/tools/test_camera_controller.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Camera navigation for hosts (legacy `ToolCamera.cpp`, Source 2 RMB+WASD). It
moves host-owned `Camera2D`/`Camera3D` values through their own functions and
never edits the document.

## 2. Accepted inputs

Pointer events, wheel events, key events (Space, W/A/S/D/E/Q, '=' and '-'),
`Advance(seconds)`, an optional orbit pivot, the session for framing.

## 3. Results and guarantees

- 2D: Middle or Space+Left drag pans (content follows the pointer); the wheel
  zooms about the cursor; '=' / '-' zoom about the centre.
- 3D: a lookButton (default Right) press is armed (`Ignored`) and becomes a
  look (`CaptureRequested`) past the drag threshold; Space+Left looks;
  Alt+Left orbits about the pivot (default 256 units ahead); Middle pans; the
  wheel dollies; held fly keys move at flySpeed (Shift: x4) in `Advance`.
- Host protocol: offer pointer events to the controller first; when it
  returns `CaptureRequested`, call `ToolManager::OnCaptureLost()`. An armed
  release without a drag is `Ignored` (the tool's click).
- Framing centres (2D) or backs off (3D) to the selection bounds; nothing
  selected changes nothing. Focus loss releases keys and drags.

## 4. Ownership, threading

Owns held keys and the drag; the cameras belong to the host.

## 5. Invariants

The controller never mutates the session.

## 6. Side effects and performance

Constant time per event.

## 7. Conformance suite and providers

`test_camera_controller.cpp` (49 checks, gcc and clang), including the
arbitration protocol with the Face tool (a right click applies, a right drag
looks with no edit). Negative checks: plain Left presses `Ignored`, focus
loss, framing nothing.
