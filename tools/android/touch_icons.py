#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Generate the touch-control icons the client's default layout names.

game/client/touch.cpp draws its buttons from materials/vgui/touch/<name>.vtf
(square, power-of-two RGBA8888, packed into an atlas at runtime), but no game
ships that art. This draws each icon from signed-distance shapes -- no image
library, font or third-party art -- and writes uncompressed VTF 7.2 files plus
a manifest listing them, which the Android application root installs into
<game>/custom/ (launcher_main/android_main.cpp).

    python3 tools/android/touch_icons.py OUTPUT_DIR

Output is deterministic: the same script writes byte-identical files.
"""

import math
import struct
import sys
from pathlib import Path

SIZE = 128
OUTLINE = 5.0  # dark halo width in pixels, so white glyphs read over bright scenes
HALO_ALPHA = 0.55
MANIFEST = "touch_icons.txt"
INSTALL_DIR = "materials/vgui/touch"

# ---------------------------------------------------------------------------
# Signed distance primitives, in a 0..1 icon space (y down). Negative inside.
# ---------------------------------------------------------------------------


def circle(cx, cy, r):
    return lambda x, y: math.hypot(x - cx, y - cy) - r


def ring(cx, cy, r, w):
    return lambda x, y: abs(math.hypot(x - cx, y - cy) - r) - w / 2


def segment(ax, ay, bx, by, w):
    def sdf(x, y):
        px, py, dx, dy = x - ax, y - ay, bx - ax, by - ay
        h = max(0.0, min(1.0, (px * dx + py * dy) / (dx * dx + dy * dy)))
        return math.hypot(px - dx * h, py - dy * h) - w / 2
    return sdf


def polyline(points, w):
    parts = [segment(*points[i], *points[i + 1], w) for i in range(len(points) - 1)]
    return union(*parts)


def box(x0, y0, x1, y1, radius=0.0):
    cx, cy, hx, hy = (x0 + x1) / 2, (y0 + y1) / 2, (x1 - x0) / 2 - radius, (y1 - y0) / 2 - radius

    def sdf(x, y):
        qx, qy = abs(x - cx) - hx, abs(y - cy) - hy
        outside = math.hypot(max(qx, 0.0), max(qy, 0.0))
        return outside + min(max(qx, qy), 0.0) - radius
    return sdf


def polygon(points):
    """Exact signed distance to a simple polygon (Inigo Quilez)."""
    def sdf(x, y):
        d = (x - points[0][0]) ** 2 + (y - points[0][1]) ** 2
        s = 1.0
        j = len(points) - 1
        for i in range(len(points)):
            ex, ey = points[j][0] - points[i][0], points[j][1] - points[i][1]
            wx, wy = x - points[i][0], y - points[i][1]
            h = max(0.0, min(1.0, (wx * ex + wy * ey) / (ex * ex + ey * ey)))
            bx, by = wx - ex * h, wy - ey * h
            d = min(d, bx * bx + by * by)
            c1, c2, c3 = y >= points[i][1], y < points[j][1], ex * wy > ey * wx
            if (c1 and c2 and c3) or (not c1 and not c2 and not c3):
                s = -s
            j = i
        return s * math.sqrt(d)
    return sdf


def arc(cx, cy, r, w, start, end):
    """Ring segment between angles (degrees, clockwise from +x, y down)."""
    a0, a1 = math.radians(start), math.radians(end)

    def sdf(x, y):
        angle = math.atan2(y - cy, x - cx) % (2 * math.pi)
        lo, hi = a0 % (2 * math.pi), a1 % (2 * math.pi)
        inside = lo <= angle <= hi if lo <= hi else angle >= lo or angle <= hi
        if inside:
            return abs(math.hypot(x - cx, y - cy) - r) - w / 2
        ends = [(cx + r * math.cos(a), cy + r * math.sin(a)) for a in (a0, a1)]
        return min(math.hypot(x - ex, y - ey) for ex, ey in ends) - w / 2
    return sdf


def ellipse_ring(cx, cy, rx, ry, w):
    """Approximate distance to an ellipse outline; exact enough for 1 px anti-aliasing."""
    def sdf(x, y):
        k = math.hypot((x - cx) / rx, (y - cy) / ry)
        return abs(k - 1.0) * min(rx, ry) - w / 2
    return sdf


def union(*shapes):
    return lambda x, y: min(s(x, y) for s in shapes)


def subtract(shape, cut):
    return lambda x, y: max(shape(x, y), -cut(x, y))


def chevron(cx, cy, size, w, direction):
    """An open arrowhead pointing up, down, left or right."""
    d = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}[direction]
    tip = (cx + d[0] * size / 2, cy + d[1] * size / 2)
    back = (cx - d[0] * size / 2, cy - d[1] * size / 2)
    side = (-d[1] * size, d[0] * size)
    return polyline([(back[0] + side[0], back[1] + side[1]), tip,
                     (back[0] - side[0], back[1] - side[1])], w)


def triangle(cx, cy, size, direction):
    d = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}[direction]
    tip = (cx + d[0] * size / 2, cy + d[1] * size / 2)
    back = (cx - d[0] * size / 2, cy - d[1] * size / 2)
    side = (-d[1] * size * 0.6, d[0] * size * 0.6)
    return polygon([tip, (back[0] + side[0], back[1] + side[1]),
                    (back[0] - side[0], back[1] - side[1])])


def gear(cx, cy, r, teeth=8):
    body = subtract(circle(cx, cy, r * 0.78), circle(cx, cy, r * 0.32))
    tooth = []
    for i in range(teeth):
        a = 2 * math.pi * i / teeth
        ox, oy = math.cos(a), math.sin(a)
        tooth.append(segment(cx + ox * r * 0.6, cy + oy * r * 0.6,
                             cx + ox * r, cy + oy * r, r * 0.28))
    return subtract(union(body, *tooth), circle(cx, cy, r * 0.32))


def crosshair(inner):
    parts = [ring(0.5, 0.5, 0.30, 0.07)]
    for x0, y0, x1, y1 in ((0.5, 0.10, 0.5, 0.26), (0.5, 0.74, 0.5, 0.90),
                           (0.10, 0.5, 0.26, 0.5), (0.74, 0.5, 0.90, 0.5)):
        parts.append(segment(x0, y0, x1, y1, 0.07))
    parts.append(inner)
    return union(*parts)


W = 0.09  # standard stroke

ICONS = {
    # A portal: the Portal layout tints it blue (+attack) and orange (+attack2).
    "portal": union(ellipse_ring(0.5, 0.5, 0.25, 0.38, 0.11), ellipse_ring(0.5, 0.5, 0.12, 0.23, 0.04)),
    # Generic primary/secondary fire: a filled and a hollow sight.
    "shoot": crosshair(circle(0.5, 0.5, 0.10)),
    "shoot_alt": crosshair(ring(0.5, 0.5, 0.12, 0.05)),
    "use": union(ring(0.5, 0.5, 0.34, W), circle(0.5, 0.5, 0.13)),
    "jump": union(chevron(0.5, 0.40, 0.36, W, "up"), segment(0.26, 0.78, 0.74, 0.78, W)),
    "crouch": union(chevron(0.5, 0.52, 0.36, W, "down"), segment(0.26, 0.22, 0.74, 0.22, W)),
    "tduck": union(chevron(0.5, 0.42, 0.34, W, "down"), segment(0.24, 0.78, 0.76, 0.78, W),
                   segment(0.24, 0.66, 0.24, 0.78, W), segment(0.76, 0.66, 0.76, 0.78, W)),
    "zoom": union(ring(0.42, 0.42, 0.24, W), segment(0.60, 0.60, 0.84, 0.84, W * 1.3),
                  segment(0.30, 0.42, 0.54, 0.42, 0.06), segment(0.42, 0.30, 0.42, 0.54, 0.06)),
    "speed": union(chevron(0.36, 0.5, 0.40, W, "right"), chevron(0.64, 0.5, 0.40, W, "right")),
    "reload": union(arc(0.5, 0.5, 0.30, W, 300, 240),
                    triangle(0.66, 0.24, 0.24, "right")),
    "flash_light_filled": union(box(0.14, 0.40, 0.50, 0.60, 0.04),
                                polygon([(0.48, 0.34), (0.66, 0.26), (0.66, 0.74), (0.48, 0.66)]),
                                segment(0.76, 0.30, 0.88, 0.22, 0.06),
                                segment(0.78, 0.50, 0.92, 0.50, 0.06),
                                segment(0.76, 0.70, 0.88, 0.78, 0.06)),
    "next_weap": union(triangle(0.58, 0.5, 0.40, "right"), segment(0.20, 0.5, 0.44, 0.5, W)),
    "prev_weap": union(triangle(0.42, 0.5, 0.40, "left"), segment(0.56, 0.5, 0.80, 0.5, W)),
    "save": subtract(union(box(0.20, 0.20, 0.80, 0.80, 0.05)),
                     union(box(0.32, 0.20, 0.64, 0.40), box(0.30, 0.54, 0.70, 0.80))),
    "load": union(segment(0.5, 0.16, 0.5, 0.56, W), chevron(0.5, 0.52, 0.26, W, "down"),
                  polyline([(0.20, 0.62), (0.20, 0.82), (0.80, 0.82), (0.80, 0.62)], W)),
    "settings": gear(0.5, 0.5, 0.36),
    "menu": union(segment(0.22, 0.30, 0.78, 0.30, W), segment(0.22, 0.50, 0.78, 0.50, W),
                  segment(0.22, 0.70, 0.78, 0.70, W)),
    "back": union(chevron(0.40, 0.5, 0.40, W, "left"), segment(0.34, 0.5, 0.80, 0.5, W)),
}


def render(shape):
    """RGBA8888 rows: white glyph over a translucent dark halo, 1 px anti-aliasing."""
    out = bytearray()
    for py in range(SIZE):
        for px in range(SIZE):
            d = shape((px + 0.5) / SIZE, (py + 0.5) / SIZE) * SIZE
            glyph = max(0.0, min(1.0, 0.5 - d))
            halo = max(0.0, min(1.0, 0.5 - (d - OUTLINE))) * HALO_ALPHA
            alpha = glyph + halo * (1.0 - glyph)
            value = glyph / alpha if alpha > 0 else 0.0
            level = int(round(255 * value))
            out += bytes((level, level, level, int(round(255 * alpha))))
    return bytes(out)


def vtf(rgba):
    """Uncompressed single-mip VTF 7.2, as public/vtf/vtf.h lays it out."""
    flags = 0x0004 | 0x0008 | 0x0100 | 0x0200 | 0x2000  # clamp S/T, no mip/lod, 8-bit alpha
    header = struct.pack(
        "<4s2iiHHIHH4s3f4sfiBiBBH",
        b"VTF\0", 7, 2, 80, SIZE, SIZE, flags, 1, 0, b"\0" * 4,
        0.5, 0.5, 0.5, b"\0" * 4, 1.0,
        0,           # IMAGE_FORMAT_RGBA8888
        1,           # mip levels
        -1, 0, 0,    # no low-resolution image
        1)           # depth
    return header.ljust(80, b"\0") + rgba


def main(argv):
    if len(argv) != 2:
        sys.exit(__doc__)
    out = Path(argv[1])
    target = out / INSTALL_DIR
    target.mkdir(parents=True, exist_ok=True)
    names = []
    for name, shape in sorted(ICONS.items()):
        (target / (name + ".vtf")).write_bytes(vtf(render(shape)))
        names.append("%s/%s.vtf" % (INSTALL_DIR, name))
    (out / MANIFEST).write_text("\n".join(names) + "\n")
    print("%d touch icons -> %s" % (len(names), out))


if __name__ == "__main__":
    main(sys.argv)
