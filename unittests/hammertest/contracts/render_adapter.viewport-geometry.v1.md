# Contract: `render_adapter.viewport-geometry.v1`

Module: `hammer.adapters.render` (RFC 0002; RFC 0016 "Editor viewports")
Headers: `hammer/adapters/render/scene_geometry.h`, `hammer/adapters/render/viewport_renderer.h`,
`hammer/adapters/render/viewport_service.h`, `hammer/adapters/render/material_textures.h`
Suites: `unittests/hammertest/render/test_viewport_geometry.cpp`
(`hammer.adapters.render.geometry`, headless), `test_viewport_renderer.cpp`
(`hammer.adapters.render.viewport.null`, headless), `test_viewport_renderer_vulkan.cpp`
(`hammer.adapters.render.viewport`, `linux-native-vulkan-gpu`) and `test_viewport_service.cpp`
(`hammer.adapters.render.service`, headless, with a TSan row)
Migration: R08-GTK-WORKSPACE follow-up (RFC 0016 decision "Editor viewports"; R17)

## Purpose

The editor's viewports on the RFC 0016 render core. The pure half turns what the headless editor
presents (`viewport::RenderSnapshot`, the workspace's cameras, grid lines and the active tool's
`tools::OverlayList`) into per-material face batches for the material families and line items for
`render.pass.lines`; `ViewportRenderer` draws one view per call offscreen on the device the
composition root passes in and hands back sRGB RGBA8 pixels: solids through `render.pass.opaque`
and the `unlit` family (RFC 0016 K4), edges, grid and overlay through `render.pass.lines`.
`ViewportService` runs the renderer on a render sequence and replies on the host's.

## Obligations

| Clause | Obligation |
| --- | --- |
| G1 | A snapshot's solids become two triangles per quad face (a fan per polygon) and one edge per face side; displacements their grid; point entities shaded marker boxes with edges. Without texture sizes every face is in one untextured batch |
| G2 | Selection is data: the selected solid's faces carry the selection fill and its edges the selection edge color; brush-entity solids their entity color; others the plain edge color |
| G3 | Faces carry the fixed two-light shading of their outward normal (fullbright preview) |
| G4 | Every overlay kind maps to its items: world lines, boxes and polygons untested in every view; screen rects as outlines; handles filled squares or discs; labels not drawn |
| G5 | Grid lines become full-width or full-height screen lines on pixel centers |
| G6 | `ViewFor(Camera2D)` projects exactly like `Camera2D::WorldToScreen` for every 2D kind; `ViewFor(Camera3D)` like `Camera3D::WorldToScreen`, with depth in [0, 1] growing with distance |
| G7 | Each projection check rejects a seeded wrong view |
| G8 | Textured preview: a face whose material has a texture size goes into that material's batch, colored by the shading alone (the selection tint over a selected solid or face), with uv = (dot(p, axis) / scale + shift) / size from the side's texture axes (`FaceDraw::uAxis`, `vAxis`); marker boxes stay untextured; a wrong size is rejected |
| G9 | A material without a size, or with a zero size, stays in the untextured batch |
| V1 | The scene is staged once per caller key |
| V2 | A 2D view runs the grid pass (clearing) and the scene pass (loading): edges then overlay. The 3D view runs the opaque pass (clearing; every batch resolves) and then the lines pass: edges, overlay |
| V3 | Nothing blocks: `Take` returns nothing until the device completes the frame |
| V4 | A view without its camera, a zero size and an unknown ticket are refused |
| V5 | Everything the renderer made is released once it is gone |
| V6 | With a material source, the source is asked once per material (not per restage); a material with a texture gets its own batch and program; one without stays untextured; the camera view draws both batches, then the edges; everything is released |
| V7 | On a device that exports no images, `CanExport()` is false and an external frame is refused with `kUnsupported` |
| R1 | On a real device, the pixel under a top face's projected center has that face's built color within one level (the unlit family reads vertex colors as gamma 2.2; the renderer re-encodes the display colors for it) |
| R2 | Edge, grid and overlay pixels land where the camera projects them, in their colors |
| R3 | A restage follows the selection; the same inputs give byte-identical frames; the validation layer reports nothing |
| R4 | Textured: where the side's axes put u in a texture's left (red) half the top face shows red, in its right (blue) half blue, each the texel times the shading in linear light within two levels; a source with no texture leaves the R1 colors |
| R5 | Exported frames (clause D18): an external frame's memory, mapped through its description, equals the read-back frame of the same view; a leased image is not drawn into (a second frame takes a new image), a returned one is, and a resize replaces the free images |
| S1 | A job's pixels arrive only through the reply runner, never on the render thread, at the requested size |
| S2 | Jobs reply in submission order |
| S3 | A view without its camera and a missing scene reply with `kInvalidView` |
| S4 | Destroying the service with jobs in flight: no reply runs afterwards, and once the destructor has returned and the device has polled, it holds nothing the renderer made |
| S5 | A shut-down render runner refuses jobs, and destroying the service then does not wait |
| S6 | A material source set on the service is asked only on the render thread and is destroyed there with the service |

## Inputs and outputs

A `RenderSnapshot` and a caller key (the snapshot revision and selection); per view a
`ViewRequest` (kind, the workspace's camera, grid lines, overlay, framebuffer size); optionally an
`IMaterialTextures` source of base textures (sRGB RGBA8). Output: sRGB RGBA8 rows, top first, or with `ViewRequest::external` an exported image
(`ExternalFrame`: a dmabuf handle and plane, and a lease). The image is not drawn into again until
the host returns the lease (`ReturnFrame`); the handle stays the renderer's.

## Failure behavior

Refusals are `ViewportStatus` values; a refused view submits nothing and leaks nothing. A missing
or undecodable texture draws its faces untextured (counted in `SceneStats::missingTextures`); a
draw the opaque pass cannot resolve is counted in `ViewStats::unresolved`.

## Lifetime and threading

The renderer borrows the device and the material source and is destroyed before them; its
destructor waits for its own frames. It is used from one sequence. `ViewportService` owns the
material source and the renderer on its render sequence (RFC 0016 decision "threading").

## Evidence and oracles

Pure checks against independently restated rules (G), the null device's recorded command stream
(V), relational pixel checks on Vulkan (R) and thread and lifetime checks on a real render thread
(S), not golden images. Textures are sampled from mip 0 only (no mip chain yet), and translucent
and alpha-tested materials draw opaque in the preview.
