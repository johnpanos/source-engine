# Contract: `render_adapter.viewport-geometry.v1`

Module: `hammer.adapters.render` (RFC 0002; RFC 0016 "Editor viewports")
Headers: `hammer/adapters/render/scene_geometry.h`, `hammer/adapters/render/viewport_renderer.h`
Suites: `unittests/hammertest/render/test_viewport_geometry.cpp`
(`hammer.adapters.render.geometry`, headless), `test_viewport_renderer.cpp`
(`hammer.adapters.render.viewport.null`, headless) and `test_viewport_renderer_vulkan.cpp`
(`hammer.adapters.render.viewport`, `linux-native-vulkan-gpu`)
Migration: R08-GTK-WORKSPACE follow-up (RFC 0016 decision "Editor viewports"; R17)

## Purpose

The editor's viewports on the RFC 0016 render core. The pure half turns what the headless editor
presents (`viewport::RenderSnapshot`, the workspace's cameras, grid lines and the active tool's
`tools::OverlayList`) into what `render.pass.lines` draws; `ViewportRenderer` draws one view per
call offscreen on the device the composition root passes in and hands back RGBA8 pixels.

## Obligations

| Clause | Obligation |
| --- | --- |
| G1 | A snapshot's solids become two triangles per quad face (a fan per polygon) and one edge per face side; displacements their grid; point entities shaded marker boxes with edges |
| G2 | Selection is data: the selected solid's faces carry the selection fill and its edges the selection edge color; brush-entity solids their entity color; others the plain edge color |
| G3 | Faces carry the fixed two-light shading of their outward normal (fullbright preview) |
| G4 | Every overlay kind maps to its items: world lines, boxes and polygons untested in every view; screen rects as outlines; handles filled squares or discs; labels not drawn |
| G5 | Grid lines become full-width or full-height screen lines on pixel centers |
| G6 | `ViewFor(Camera2D)` projects exactly like `Camera2D::WorldToScreen` for every 2D kind; `ViewFor(Camera3D)` like `Camera3D::WorldToScreen`, with depth in [0, 1] growing with distance |
| G7 | Each projection check rejects a seeded wrong view |
| V1 | The scene is staged once per caller key |
| V2 | A 2D view runs the grid pass (clearing) and the scene pass (loading): edges then overlay; the 3D view one pass: faces, edges, overlay |
| V3 | Nothing blocks: `Take` returns nothing until the device completes the frame |
| V4 | A view without its camera, a zero size and an unknown ticket are refused |
| V5 | Everything the renderer made is released once it is gone |
| R1 | On a real device, the pixel under a top face's projected center has that face's built color |
| R2 | Edge, grid and overlay pixels land where the camera projects them, in their colors |
| R3 | A restage follows the selection; the same inputs give byte-identical frames; the validation layer reports nothing |

## Inputs and outputs

A `RenderSnapshot` and a caller key (the snapshot revision and selection); per view a
`ViewRequest` (kind, the workspace's camera, grid lines, overlay, framebuffer size). Output: RGBA8
rows, top first.

## Failure behavior

Refusals are `ViewportStatus` values; a refused view submits nothing and leaks nothing.

## Lifetime and threading

The renderer borrows the device and is destroyed before it; its destructor waits for its own
frames. It is used from one sequence (the GTK main loop first, a render sequence later, RFC 0016
decision "threading").

## Evidence and oracles

Pure checks against independently restated rules (G), the null device's recorded command stream
(V) and relational pixel checks on Vulkan (R), not golden images.
