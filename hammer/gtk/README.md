# Hammer GTK desktop shell (`linux-gtk-desktop`)

A GTK4 + libadwaita host for the Hammer editor — the delivery target of
[RFC 0002](../../RFC/0002-hammer-responsibility-factorization.md). It composes the
**strict headless editor core** (`hammer/core`, public headers under
`public/hammer/`) with a native UI laid out like classic Hammer, plus a modern
touchpad-driven 3D/2D viewport.

This is a **separate product scope**: the Waf target `hammer_gtk` of the tools
product, built when GTK 4, libadwaita and the RFC 0016 render core's Vulkan
adapter are available. It links the editor core, the format libraries and the
render core plus system GTK; no MFC, no `tier0`, no engine DLLs. All GTK/GDK
native detail is confined to this directory, and all GPU detail to the render
core; the editor core it drives has no display, GPU or platform dependency.

## Architecture

The shell is a thin host over `hammer::presenters::EditorWorkspace`
(`public/hammer/presenters/editor_workspace.h`), which owns the one document and
undo history (`app::EditSession`), the editor settings (grid, snap, active
material and entity class), the named command catalog (`app::SessionCommands`,
the same commands `hammer_cli` scripts and MCP clients run), the tools, the
viewport cameras and their navigation (`tools::CameraController`), the render
snapshot and the `ActionCatalog` (menus and shortcuts). The host:

- turns GTK pointer, key and scroll events into `tools::` events and hands them
  to the workspace, which offers them to catalog shortcuts, camera navigation
  and the active tool, in that order;
- shows each view as drawn by the RFC 0016 render core (RFC 0016 "Editor
  viewports"): the host composes the core (`RenderCore_Create`, Vulkan device,
  no legacy backend), and `hammer::render_adapter::ViewportRenderer`
  (`hammer/adapters/render/`) draws the workspace's `viewport::RenderSnapshot`,
  grid lines and tool overlay through the workspace's cameras with
  `render.pass.lines`, offscreen, into a readback the viewport widget
  (`viewport_widget.{h,cpp}`) shows as a texture. What is drawn and what a
  click means come from one camera; views render only when behind the
  workspace, and nothing waits on the GPU;
- fulfils what only a host can: file dialogs, running the map, and F9's build
  off the UI thread (`app::MapBuildQueue` with the GLib and thread runners);
- builds its menu bar from the `ActionCatalog` (one `app.act-*` action each,
  enabled and checked state from the catalog).

Maps are read and written by the strict VMF codec (`formats::VmfMapCodec`):
every piece of map state loads into typed fields, and content it does not model
is an open error naming the block and line.

## What works today

- **Classic Hammer layout:** menu bar, toolbar, left tool palette, the four
  viewports (3D **camera** + 2D **top** X/Y, **front** X/Z, **side** Y/Z), the
  right **object bar**, and a status bar with a live coordinate read-out, brush
  count and grid/tool indicator. Every boundary is a drag-resizable splitter.
- **Files:** New, Open (`--open MAP.vmf`), Save, Save As through the workspace.
- **Tools** (the domain tools; their headers under `public/hammer/tools/`
  document each state machine): Selection (click, Ctrl-toggle, marquee, move,
  scale/rotate handles, arrow nudge), Block (drag a box in a 2D view, drag
  again in another view for its height, **Enter** creates it), Entity (click in
  a 2D view, or on a surface in the 3D view), Clip, Vertex and Face (**Shift+A**,
  which also opens the Texture Application window).
- **Every catalog action** from the menus or its shortcut: undo/redo, cut/copy/
  paste/duplicate, hide/unhide, grid **[ ]**, snap, group/ungroup, tie to entity,
  carve, **F** make hollow, apply material, texture lock, selection granularity,
  check/fix problems, **F9** build and **Shift+F9** build and run.
- **Rendering:** the 3D view shades brushes (with base textures when game assets
  are mounted), displacements and entity markers; the 2D views draw wireframes
  over the workspace's grid; selections, faces, pending boxes and handles are
  highlighted in every view.

## Navigation

The workspace's `CameraController` owns the bindings
(`public/hammer/tools/camera_controller.h`):

| Input | 2D views | 3D camera |
| --- | --- | --- |
| Middle-drag | Pan | Pan |
| **Space** + left-drag | Pan | Look |
| Right-drag | — | Look (Source 2) |
| **Alt** + left-drag | — | Orbit |
| Wheel | Zoom about the cursor | Dolly |
| **W A S D / E Q** (held; **Shift** faster) | — | Fly |
| **= / −** | Zoom about the centre | — |

The host adds: **Tab** cycles a 2D pane Top → Front → Side; touchpad scrolling
pans a 2D view (**Ctrl**: zooms) and dollies the 3D view; pinch zooms; View ▸
Reset Views (`⌃R`) frames the map; right-click in a 2D view opens a context
menu.

## Build

Requires `gtk4`, `libadwaita-1` and the Vulkan loader (via `pkg-config`), and
what the tools product needs, including the pinned shader tools
(`python3 tools/render/shader_toolchain.py build`). From anywhere in the
checkout:

```sh
hammer/gtk/build.sh                 # builds hammer/gtk/hammer_gtk
hammer/gtk/hammer_gtk --open hammer/gtk/samples/room.vmf
```

`build.sh` builds the Waf target in its own tools tree (`build-hammer-gtk`,
or `HAMMER_GTK_TREE`), configured with `--render-core-vulkan=on` when the tree
is new or older than a `wscript`, and copies the program to the path given.

The 3D preview is flat-shaded with the editor's fixed two-light shading.
Textured previews come with the render core's material families (RFC 0016 K4:
`render.material` over the material catalog's texels); until then the material
browser and the object bar's swatch show textures, and `--textured` refuses.

## Verify without a window server

The offscreen modes open maps through the same `EditorWorkspace` and draw them
through the same render core and `ViewportRenderer`, with no window server (a
Vulkan device is needed; Mesa's lavapipe is sufficient):

```sh
# Single 3D view to a PPM:
hammer/gtk/hammer_gtk --screenshot out.ppm hammer/gtk/samples/room.vmf --width 800 --height 600

# The classic 2x2 quad (camera / top / front / side) to one PPM:
hammer/gtk/hammer_gtk --quad quad.ppm hammer/gtk/samples/room.vmf --width 1600 --height 1200

# Build a map by driving the workspace with SIMULATED input and render it:
hammer/gtk/hammer_gtk --demo demo.ppm --width 1600 --height 1200

# Render a displacement (dispinfo terrain) map — the +Z face is a subdivided,
# displaced hill (3D shaded relief + the triangulated grid in the 2D views):
hammer/gtk/hammer_gtk --quad disp.ppm hammer/gtk/samples/displacement.vmf --width 1600 --height 1200

# Automated smoke test (build + render + assert non-blank geometry):
hammer/gtk/tests/viewport_smoke.sh
```

The offscreen path opens maps through the same `EditorWorkspace` the window
uses. The editing flows are covered headlessly by the Q-EDITOR suites of the
layers under the workspace (`hammer.presenters.editor_workspace`, the tool and
command suites), and end to end by `corpus.hammer.ui`
(`tools/quality/hammer_ui_test.py`), which drives this shell in an isolated
compositor the way a user makes a map.

With `HAMMER_GTK_FRAME_DIR` set, the window also writes each view's latest frame
there (`camera.ppm`, `top.ppm`, `front.ppm`, `side.ppm`); the UI-driven suite
judges what the live editor showed from them.

## Files

| File | Responsibility |
| --- | --- |
| `app.cpp` | Window, classic layout, input translation, dialogs, async build, catalog menus |
| `viewport_widget.{h,cpp}` | The viewport widget: shows the render core's frame for its view, reports size changes |
| `offscreen.cpp` | Offscreen render through the render core for `--screenshot` / `--quad` / `--demo` |
| `wscript`, `build.sh` | The Waf target and its build wrapper |
| `samples/room.vmf` | A minimal valid Source room (floor/ceiling/walls/pillar) |
| `tests/viewport_smoke.sh` | Build + render + content-assert smoke test |

The editing authority (`EditorWorkspace` and the layers under it) and file I/O
(`hammer::adapters::platform::DiskFileStore`) live in the shared core, not here
— the host only presents them. There is no host-local document or file store.

The geometry bridge itself lives in the strict core: brush geometry in
`public/mapgeometry/brush.h` and `mapgeometry/brush.cpp`, and the
VMF decoder in `public/vmf/vmf_geometry.h` and
`vmf/vmf_geometry.cpp`. Both are covered by the
`hammer.geometry.brush` Q-EDITOR conformance suites (`unittests/hammertest/`),
so they are exercised headlessly under gcc and clang independently of this GTK
product.
