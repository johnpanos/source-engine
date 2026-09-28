# Contract: `viewport.camera.v1`

Module: `hammer.viewport` (uses `hammer.scene` `Box`, `world.map-geometry` vectors)
Header: `public/hammer/viewport/camera.h` · Impl: `hammer/core/viewport/camera.cpp`
Conformance: `unittests/hammertest/viewport/test_camera.cpp`
Migration: R08-DOMAIN (supersedes the camera math in `hammer/gtk/renderer.cpp`
and `hammer/gtk/app.cpp`; those stay the GTK shell's until it is hooked to
`hammer.viewport`)

The projection owner for every Hammer viewport: the orthographic 2D camera of
the Top, Front and Side views and the perspective 3D camera. Tools, picking,
the grid and presenters project through these values, so screen space, the
view axis mapping and the view angle convention have one owner.

## 1. Purpose, consumers, scope

- `Camera2D`: view kind, centre (world units on the view's two axes), zoom
  (pixels per unit) and viewport size; world/plane/screen conversions,
  pan, zoom about a pixel, framing.
- `Camera3D`: eye position, Source yaw/pitch, vertical field of view and
  viewport size; basis vectors, pixel rays, projection, fly, look, orbit,
  look-at and framing.
- Consumers: `viewport.grid.v1`, `viewport.picking.v1`,
  `viewport.extraction.v1` (2D wireframes), `hammer.tools` (pointer
  gestures), the GTK host (drawing and navigation). Required.
- Out of scope: native widgets, GL matrices, input devices. RFC 0002's rule
  holds: a 2D point has no universal 3D inverse, so there is no
  `ClientToWorld`; `ScreenToPlane` returns the two view-axis coordinates and
  `PlaneToWorld`/`ScreenToWorld` take the free-axis depth from the caller.

## 2. Accepted inputs

- Screen pixels: origin top-left, x right, y down, continuous coordinates.
- 2D view kinds `Top`, `Front`, `Side`; `Camera3D` is rejected by
  `Camera2D::SetKind`.
- Axis mapping (identical to `EditorController::ViewAxes`): Top = X/Y (free
  Z, looking down -Z), Front = X/Z (free Y, looking along +Y), Side = Y/Z (free
  X, looking along -X). u is screen right, v is screen up.
- Zoom in (0, inf), clamped to [1/64, 256] (legacy `ZOOM_MAX` 256).
- 3D angles in degrees, Source convention: yaw about +Z from +X, pitch
  positive looks down, clamped to +/-89, yaw normalized to (-180, 180]; no roll.
  Field of view clamped to [1, 170] (default 60, the GTK shell's).
- Viewport sizes: negative sizes are stored as zero.

## 3. Results and guarantees

- Round trip: `ScreenToWorld( WorldToScreen( p ), DepthOf( p ) ) == p` (within
  floating-point rounding) for every 2D kind; the centre maps to the viewport
  centre; larger v is higher on screen.
- `ZoomAt( px, py, f )` multiplies the zoom by `f` (clamped) and keeps the world
  point under `(px, py)` fixed, also when the zoom clamps.
- `PanPixels( dx, dy )` moves content with the pointer by exactly (dx, dy) px.
- `Camera2D::Frame( box, margin )` centres the box and picks the largest zoom
  showing its view-axis extent with the margin free; a zero-extent axis keeps
  the zoom.
- `Camera3D`: Forward = (cos p cos y, cos p sin y, -sin p), Right =
  (sin y, -cos y, 0), Up = Right x Forward, orthonormal. The centre pixel's ray
  is Forward from the eye; `WorldToScreen` inverts `RayThroughPixel`; points at
  or behind `kNearDepth` along Forward do not project.
- `Fly` moves along Forward/Right and world +Z; `Look` keeps the eye; `Orbit`
  keeps the distance to the pivot and ends looking at it; `LookAt` aims at a
  target; `Camera3D::Frame` keeps the angles and fits the box's bounding
  sphere in the narrower field of view.
- GTK shell equivalence: its orbit camera (eye = target + d * (cos p cos y,
  cos p sin y, sin p)) is `LookAt( target )` from that eye; its `PixelToRay`,
  `FlyMove`, `FlyLook`, `PixelToWorld`, `DragBy` and `ZoomAtPixel` formulas are
  reproduced exactly.
- Rejections return false (or nothing) and change nothing: non-finite values,
  non-positive zoom or zoom factors, invalid boxes, margins that leave no area,
  a 3D ray or projection without a viewport, `LookAt` of the eye itself.

## 4. Ownership, threading

- Plain values: no allocation, no references to documents, hosts or GL. Copies
  are independent. Not synchronized; each view owns its camera.

## 5. Invariants

- `Camera2D`: kind is a 2D kind; zoom in [kMinZoom, kMaxZoom]; centre finite;
  sizes >= 0.
- `Camera3D`: pitch in [-89, 89]; yaw in (-180, 180]; fov in [1, 170];
  position finite; sizes >= 0.

## 6. Side effects and performance

- None beyond the value itself. Every operation is O(1) with no allocation.

## 7. Conformance suite and providers

- `test_camera.cpp` (139 checks): axis mapping and view directions; 2D
  defaults, rejection and clamping; projection examples and round trips for
  all three kinds; zoom-about-cursor (including at the clamp), pan and GTK 2D
  formulas; 2D framing and its rejections; 3D angle convention, clamping and
  orthonormal basis; ray/projection round trip, behind-camera and empty-viewport
  rejection; fly/look/orbit/look-at; 3D framing (every corner on screen); GTK
  3D equivalence.
- Negative controls: an un-anchored zoom (seeded mutant) fails the fixed-point
  oracle; the unframed view fails the corners-on-screen oracle.
- Runs headlessly with g++ and clang++ (`-std=c++20 -Wall -Wextra -Werror`).
