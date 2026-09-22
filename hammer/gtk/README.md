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

## What works today (first bootable slice)

- Boots a GTK4 + libadwaita window and **opens a simple VMF** (File ▸ Open, `⌃O`,
  or `--open MAP.vmf`), parsing it through the core's VMF keyvalues codec and
  resolving every world/entity brush *solid* (its side planes) into a convex mesh
  via `hammer::geometry::BuildSceneFromDocument`.
- **Classic Hammer layout:** menu bar, toolbar, left tool palette, the four
  viewports (3D **camera** + 2D **top** X/Y, **front** X/Z, **side** Y/Z, each a
  resizable pane), the right **object bar** (Select · Texture group · Current
  texture · VisGroups · Show/Edit/Mark), and a status bar with a live coordinate
  read-out and grid/snap indicator.
- **3D preview:** the camera view renders shaded, per-solid-coloured brushes with
  a wireframe overlay; the 2D views render wireframe geometry over a Hammer-style
  power-of-two grid with coloured world axes.

Not yet implemented (later RFC 0002 rows): editing tools, selection/undo wired to
the live document, real texture/material rendering, displacements, VMF save from
the UI. The tool palette, texture panel, and menu items beyond Open/Quit/Reset
Views are laid out but not yet functional.

## Navigation (Apple/Figma-style)

| Input | 2D views | 3D camera |
| --- | --- | --- |
| Two-finger scroll (touchpad) | Pan (kinetic) | Orbit |
| Pinch | Zoom, anchored at the pinch point | Dolly |
| ⌃ + scroll / wheel | Zoom, anchored at the cursor | Dolly |
| Drag (mouse) | Pan | Orbit |

## Build

Requires `gtk4`, `libadwaita-1`, and `epoxy` (via `pkg-config`) and a C++20
compiler. From anywhere in the checkout:

```sh
hammer/gtk/build.sh                 # builds hammer/gtk/hammer_gtk
hammer/gtk/hammer_gtk --open hammer/gtk/samples/room.vmf
```

## Verify without a window server

The same renderer drives an offscreen EGL path, so the 3D preview can be checked
headlessly (Mesa's software rasteriser is sufficient):

```sh
# Single 3D view to a PPM:
hammer/gtk/hammer_gtk --screenshot out.ppm hammer/gtk/samples/room.vmf --width 800 --height 600

# The classic 2x2 quad (camera / top / front / side) to one PPM:
hammer/gtk/hammer_gtk --quad quad.ppm hammer/gtk/samples/room.vmf --width 1600 --height 1200

# Automated smoke test (build + render + assert non-blank geometry):
hammer/gtk/tests/viewport_smoke.sh
```

Set `HAMMER_GTK_DEBUG=1` to print each viewport's GL version on realize.

## Files

| File | Responsibility |
| --- | --- |
| `app.cpp` | Window, classic layout, gesture controllers, File▸Open, action wiring |
| `renderer.{h,cpp}` | Pure-GL renderer: shaded 3D + 2D ortho wireframe/grid. No GTK |
| `offscreen.cpp` | EGL offscreen render for `--screenshot` / `--quad` verification |
| `posix_file_store.h` | Host-owned `IFileStore` provider over the real filesystem |
| `samples/room.vmf` | A minimal valid Source room (floor/ceiling/walls/pillar) |
| `tests/viewport_smoke.sh` | Build + render + content-assert smoke test |

The geometry bridge itself lives in the strict core
(`public/hammer/geometry/brush.h`, `hammer/core/geometry/brush.cpp`) and is
covered by the `hammer.geometry.brush` Q-EDITOR conformance suites
(`unittests/hammertest/`), so it is exercised headlessly under gcc and clang
independently of this GTK product.
