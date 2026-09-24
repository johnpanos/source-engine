# render.display-modes.v1

`public/render/render_display_modes.h` owns the video modes a desktop-fullscreen
presenter offers. SDL3 fullscreen always covers the desktop and the presenter
scales the back buffer to the drawable, so a mode is a back-buffer size, not a
display mode switch. `BuildBackBufferModeList( desktop )` returns the desktop
size, each standard 4:3/16:9/16:10 size (togl's table plus 1440p and 2160p)
that fits, the desktop-aspect size of each fitting standard width, and half the
desktop. Every entry fits the desktop in both dimensions and carries the desktop
refresh rate. The list is sorted by width then height, with no duplicates. A
desktop without a positive extent yields an empty list; the caller must never
invent a size. Filtering small modes and choosing refresh rates stay with the
engine's video-mode owner (`engine/sys_getmodes.cpp`).

The suite checks these invariants on landscape, ultrawide, portrait and small
desktops, plus the empty case. Its sensitivity build adds an oversize mode, which
the oracle must reject. Sizes are in the launcher's units (window points). A crisp
HiDPI fullscreen mode is not provided and remains an open item.
