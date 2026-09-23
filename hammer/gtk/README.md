# Hammer GTK desktop shell (`linux-gtk-desktop`)

A GTK4 + libadwaita host for the Hammer editor — the delivery target of
[RFC 0002](../../RFC/0002-hammer-responsibility-factorization.md). It composes the
**strict headless editor core** (`hammer/core`, public headers under
`public/hammer/`) with a native UI laid out like classic Hammer, plus a modern
touchpad-driven 3D/2D viewport.

This is a **separate product** from the engine's waf build. It links only the
dependency-free editor core (VMF codec, editor document, and the VMF→brush
geometry bridge) plus system GTK/GL; no MFC, no `tier0`, no engine DLLs. All
GTK/GDK/OpenGL native detail is confined to this directory — the core it drives
has no display, GPU, or platform dependency.

## What works today

- Boots a GTK4 + libadwaita window and **opens a simple VMF** (File ▸ Open, `⌃O`,
  or `--open MAP.vmf`) and **saves** one (File ▸ Save/Save As, `⌃S`) through the
  shared `SaveDocument` orchestrator over `DiskFileStore`.
- **Classic Hammer layout:** menu bar, toolbar, left tool palette, the four
  viewports (3D **camera** + 2D **top** X/Y, **front** X/Z, **side** Y/Z), the
  right **object bar** (Select · Texture group · Current texture · VisGroups ·
  Show/Edit/Mark), and a status bar with a live coordinate read-out, brush count,
  and grid/tool indicator.
- **Drag-resizable panels:** every boundary is a splitter — the tool palette, the
  object bar, and all four viewport panes resize by dragging (nested `GtkPaned`).
- **Core editing UX flows**, all through the single headless authority
  `hammer::app::EditorController`:
  - **Block tool** — drag a rectangle in any 2D view (grid-snapped, live pending
    box shown yellow in all views), press **Enter** to create an extruded brush.
  - **Selection tool** — click a brush to select (highlighted orange), drag to
    move (grid-snapped), click empty space to deselect, **Delete** to remove.
    Clicking a brush in the **3D camera view** selects it too (ray pick), the same
    way MFC Hammer selects in 3D.
  - **Undo/Redo** (`⌃Z` / `⌃Y`), **New** (`⌃N`) — one history stack for all edits.
    **Ctrl-click** adds/removes a brush from a multi-selection (in 2D or 3D); a
    nudge, drag-move or delete then applies to the whole selection. In a 2D view,
    **arrow keys** nudge the selection one grid step, and **right-click** opens a
    context menu (a selection menu when a brush is selected, otherwise the default
    view menu). Like MFC, the 3D view has no menu.
- **3D preview:** the camera view renders shaded brushes with available base textures; the
  2D views render wireframe geometry over a Hammer-style power-of-two grid with
  coloured world axes; the selected/pending brush is tinted in every view.

Not yet implemented (later RFC 0002 rows): entity/property editing, non-box brush
editing, vertex/clip tools, full Source material fidelity, and displacements.
The remaining tool-palette buttons, texture panel, and unimplemented menu items
are laid out but inert.

## Navigation

### Classic Hammer 3D camera (matches MFC)

Hover the **3D camera view** to give it focus, then:

| Input | Effect |
| --- | --- |
| **Z** | Toggle mouse-look: the cursor hides and moving the mouse turns the camera. Press **Z** again to release. |
| **W / A / S / D** | Fly forward / left / back / right along the view. |
| **Q / E** | Fly down / up (world vertical). |
| **Shift** (held) | Fly faster. |
| Left-click (no drag) | Ray-pick a brush to select it. |
| Left-drag | Orbit the camera. |

Fly keys work whenever the 3D view is focused; **Z** additionally frees the mouse
to look around (Hammer's free-look). Movement is time-integrated, so it is smooth
and frame-rate independent. (GTK4 cannot warp the pointer, so mouse-look rotates
by pointer delta with the cursor hidden rather than re-centering each frame.)

### 2D views (matches MFC)

| Input | Effect |
| --- | --- |
| **Space + left-drag** | Pan the view (MFC's pan idiom). |
| **Tab** | Cycle the view orientation: Top → Front → Side. |
| Left-click / drag | Select a brush / drag-move it (grid-snapped). |
| **Ctrl + click** | Add / remove a brush from the multi-selection. |
| **Arrow keys** | Nudge the selection one grid step. |
| **+ / −** | Zoom in / out, anchored at the cursor. |
| **1 – 9 / 0** | Preset zoom levels / frame the whole map. |
| **Right-click** | Context menu (selection menu when a brush is selected). |

### Touchpad / mouse (Apple/Figma-style, additive)

| Input | 2D views | 3D camera |
| --- | --- | --- |
| Two-finger scroll (touchpad) | Pan (kinetic) | Orbit |
| Pinch | Zoom, anchored at the pinch point | Dolly |
| ⌃ + scroll / wheel | Zoom, anchored at the cursor | Dolly |
| Middle-drag | Pan | Orbit |

## Build

Requires `gtk4`, `libadwaita-1`, and `epoxy` (via `pkg-config`) and a C++20
compiler. From anywhere in the checkout:

```sh
hammer/gtk/build.sh                 # builds hammer/gtk/hammer_gtk
hammer/gtk/hammer_gtk --open hammer/gtk/samples/room.vmf
```

To enable packaged KTX2 previews in the material browser and textured
viewport, point the build at the source and CMake build of the revision pinned
in `quality/product_profiles/ktx2-linux-tools.json`:

```sh
KTX_SOURCE_ROOT=/tmp/rfc0008-ktx-pin \
KTX_BUILD_ROOT=/tmp/rfc0008-ktx-pin/build-rfc0008 \
  hammer/gtk/build.sh /tmp/rfc0008-hammer-ktx-preview
python3 tools/quality/hammer_ktx2_preview.py \
  --binary /tmp/rfc0008-hammer-ktx-preview \
  --out quality-results/rfc0008-f3-hammer-ktx-product.json
```

This preview accepts packaged RGBA8/BGRA8 2D KTX2 assets through the shared
runtime reader. Present KTX2 assets take precedence over VTF and must decode;
compressed KTX2 formats are not yet previewed. A build without the pinned
reader keeps the legacy VTF preview path.

## Verify without a window server

The same renderer drives an offscreen EGL path, so the 3D preview can be checked
headlessly (Mesa's software rasteriser is sufficient):

```sh
# Single 3D view to a PPM:
hammer/gtk/hammer_gtk --screenshot out.ppm hammer/gtk/samples/room.vmf --width 800 --height 600

# The classic 2x2 quad (camera / top / front / side) to one PPM:
hammer/gtk/hammer_gtk --quad quad.ppm hammer/gtk/samples/room.vmf --width 1600 --height 1200

# Build a map with SIMULATED editing input (no file) and render it:
hammer/gtk/hammer_gtk --demo demo.ppm --width 1600 --height 1200

# Load a VMF THROUGH the EditorController and render it (proves real brush shapes,
# not bounding boxes — e.g. the wedge ramp shows a triangle in the front view):
hammer/gtk/hammer_gtk --cquad cwedge.ppm hammer/gtk/samples/wedge.vmf --width 1600 --height 1200

# Render a displacement (dispinfo terrain) map — the +Z face is a subdivided,
# displaced hill (3D shaded relief + the triangulated grid in the 2D views):
hammer/gtk/hammer_gtk --quad disp.ppm hammer/gtk/samples/displacement.vmf --width 1600 --height 1200

# Automated smoke test (build + render + assert non-blank geometry):
hammer/gtk/tests/viewport_smoke.sh
```

The editing UX flows are covered headlessly, independent of GTK, by the
`hammer.app.editor_controller` Q-EDITOR conformance suite
(`unittests/hammertest/app/test_editor_controller.cpp`), which drives the same
`EditorController` the GUI uses with simulated input to build, edit, undo, and
save a map. Run it via `unittests/hammertest/run_headless.sh`.

Set `HAMMER_GTK_DEBUG=1` to print each viewport's GL version on realize.

## Files

| File | Responsibility |
| --- | --- |
| `app.cpp` | Window, classic layout, gestures/keys, tools, File▸New/Open/Save, actions |
| `renderer.{h,cpp}` | Pure-GL renderer: shaded 3D + 2D ortho wireframe/grid + highlight. No GTK |
| `offscreen.cpp` | EGL offscreen render for `--screenshot` / `--quad` / `--demo` verification |
| `samples/room.vmf` | A minimal valid Source room (floor/ceiling/walls/pillar) |
| `tests/viewport_smoke.sh` | Build + render + content-assert smoke test |

The editing authority (`hammer::app::EditorController`) and file I/O
(`hammer::adapters::platform::DiskFileStore`, `SaveDocument`) live in the shared
core, not here — the host only presents them. There is no host-local file store.

The geometry bridge itself lives in the strict core
(`public/hammer/geometry/brush.h`, `hammer/core/geometry/brush.cpp`) and is
covered by the `hammer.geometry.brush` Q-EDITOR conformance suites
(`unittests/hammertest/`), so it is exercised headlessly under gcc and clang
independently of this GTK product.
