#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Generator for the compile-equivalence fixture maps (RFC 0005 gap corpus,
# R48 legacy-lump gate). The VMF files next to this script are its checked-in
# output; `python3 make_maps.py --check` fails when they differ from what this
# script writes, `python3 make_maps.py` rewrites them. Every map is authored
# here from axis-aligned boxes and analytic displacement heights: no Valve or
# Portal content is copied. Python 3 standard library only.
#
# ============================================================================

import argparse
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
MAPS = os.path.join(HERE, "maps")

WALL = "CORPUS/WALL"
FLOOR = "CORPUS/FLOOR"
GROUND = "CORPUS/GROUND"
DETAIL = "CORPUS/DETAIL"
NODRAW = "TOOLS/TOOLSNODRAW"
SKY = "TOOLS/TOOLSSKYBOX"


def fmt(value):
    """Integers print bare; other values with at most three decimals."""
    value = round(float(value), 3)
    if value == int(value):
        return str(int(value))
    return ("%.3f" % value).rstrip("0").rstrip(".")


class Map:
    def __init__(self, name, comment):
        self.name = name
        self.comment = comment
        self.next_id = 1
        self.world = []
        self.entities = []

    def id(self):
        value = self.next_id
        self.next_id += 1
        return value

    def box(self, lo, hi, material, overrides=None, disp=None):
        """An axis-aligned brush. `overrides` maps a face name (top, bottom,
        left, right, back, front) to a material; `disp` maps a face name to
        (power, height(u, v)) for a displacement on that face."""
        (x1, y1, z1), (x2, y2, z2) = lo, hi
        faces = {
            "top": ((x1, y2, z2), (x2, y2, z2), (x2, y1, z2)),
            "bottom": ((x1, y1, z1), (x2, y1, z1), (x2, y2, z1)),
            "left": ((x1, y2, z2), (x1, y1, z2), (x1, y1, z1)),
            "right": ((x2, y2, z1), (x2, y1, z1), (x2, y1, z2)),
            "back": ((x2, y2, z2), (x1, y2, z2), (x1, y2, z1)),
            "front": ((x2, y1, z1), (x1, y1, z1), (x1, y1, z2)),
        }
        axes = {
            "top": ("[1 0 0 0] 0.25", "[0 -1 0 0] 0.25"),
            "bottom": ("[1 0 0 0] 0.25", "[0 -1 0 0] 0.25"),
            "left": ("[0 1 0 0] 0.25", "[0 0 -1 0] 0.25"),
            "right": ("[0 1 0 0] 0.25", "[0 0 -1 0] 0.25"),
            "back": ("[1 0 0 0] 0.25", "[0 0 -1 0] 0.25"),
            "front": ("[1 0 0 0] 0.25", "[0 0 -1 0] 0.25"),
        }
        lines = ["\tsolid", "\t{", '\t\t"id" "%d"' % self.id()]
        for face in ("top", "bottom", "left", "right", "back", "front"):
            points = " ".join("(%s)" % " ".join(fmt(c) for c in p) for p in faces[face])
            lines += ["\t\tside", "\t\t{", '\t\t\t"id" "%d"' % self.id(),
                      '\t\t\t"plane" "%s"' % points,
                      '\t\t\t"material" "%s"' % (overrides or {}).get(face, material),
                      '\t\t\t"uaxis" "%s"' % axes[face][0],
                      '\t\t\t"vaxis" "%s"' % axes[face][1],
                      '\t\t\t"rotation" "0"',
                      '\t\t\t"lightmapscale" "16"',
                      '\t\t\t"smoothing_groups" "0"']
            if disp and face in disp:
                if face != "top":
                    raise ValueError("displacements are generated on top faces only")
                power, height = disp[face]
                lines += dispinfo(power, (x1, y1, z2), height)
            lines.append("\t\t}")
        lines.append("\t}")
        return lines

    def add_world(self, *args, **kwargs):
        self.world += self.box(*args, **kwargs)

    def entity(self, classname, keys, brushes=()):
        lines = ["entity", "{", '\t"id" "%d"' % self.id(), '\t"classname" "%s"' % classname]
        for key, value in keys:
            lines.append('\t"%s" "%s"' % (key, value))
        for lo, hi, material in brushes:
            lines += self.box(lo, hi, material)
        lines.append("}")
        self.entities += lines

    def text(self):
        out = ["// %s" % line for line in self.comment.splitlines()]
        out += ["versioninfo", "{", '\t"editorversion" "400"', '\t"editorbuild" "8000"',
                '\t"mapversion" "1"', '\t"formatversion" "100"', '\t"prefab" "0"', "}",
                "visgroups", "{", "}", "viewsettings", "{", "}",
                "world", "{", '\t"id" "%d"' % 0, '\t"mapversion" "1"',
                '\t"classname" "worldspawn"', '\t"skyname" "corpus_sky"']
        out += self.world
        out.append("}")
        out += self.entities
        out += ["cameras", "{", '\t"activecamera" "-1"', "}",
                "cordon", "{", '\t"mins" "(-1024 -1024 -1024)"', '\t"maxs" "(1024 1024 1024)"',
                '\t"active" "0"', "}"]
        return "\n".join(out) + "\n"


def dispinfo(power, start, height):
    """A dispinfo block whose vertex (row r, column c) is raised by
    height(c / (n - 1), r / (n - 1)) along +z. The height functions used here
    are symmetric under u <-> v, so the row/column convention of the format
    cannot change the authored shape."""
    n = (1 << power) + 1
    rows = lambda f: ['\t\t\t\t"row%d" "%s"' % (r, " ".join(f(r, c) for c in range(n)))
                      for r in range(n)]
    lines = ["\t\t\tdispinfo", "\t\t\t{",
             '\t\t\t\t"power" "%d"' % power,
             '\t\t\t\t"startposition" "[%s]"' % " ".join(fmt(c) for c in start),
             '\t\t\t\t"flags" "0"', '\t\t\t\t"elevation" "0"', '\t\t\t\t"subdiv" "0"']
    blocks = [
        ("normals", lambda r, c: "0 0 1"),
        ("distances", lambda r, c: fmt(height(c / (n - 1), r / (n - 1)))),
        ("offsets", lambda r, c: "0 0 0"),
        ("offset_normals", lambda r, c: "0 0 1"),
        ("alphas", lambda r, c: "0"),
    ]
    for name, f in blocks:
        lines += ["\t\t\t\t%s" % name, "\t\t\t\t{"] + ["\t" + l for l in rows(f)] + ["\t\t\t\t}"]
    tags = " ".join(["9"] * (2 * (n - 1)))
    lines += ["\t\t\t\ttriangle_tags", "\t\t\t\t{"]
    lines += ['\t\t\t\t\t"row%d" "%s"' % (r, tags) for r in range(n - 1)]
    lines += ["\t\t\t\t}", "\t\t\t\tallowed_verts", "\t\t\t\t{",
              '\t\t\t\t\t"10" "%s"' % " ".join(["-1"] * 10), "\t\t\t\t}", "\t\t\t}"]
    return lines


def shell(m, lo, hi, thickness=16, material=WALL, floor=FLOOR, ceiling=WALL):
    """Six brushes sealing the interior box lo..hi."""
    (x1, y1, z1), (x2, y2, z2) = lo, hi
    t = thickness
    m.add_world((x1 - t, y1 - t, z1 - t), (x2 + t, y2 + t, z1), floor)
    m.add_world((x1 - t, y1 - t, z2), (x2 + t, y2 + t, z2 + t), ceiling)
    m.add_world((x1 - t, y1 - t, z1), (x1, y2 + t, z2), material)
    m.add_world((x2, y1 - t, z1), (x2 + t, y2 + t, z2), material)
    m.add_world((x1, y1 - t, z1), (x2, y1, z2), material)
    m.add_world((x1, y2, z1), (x2, y2 + t, z2), material)


def light(m, origin, color="255 244 229 300"):
    m.entity("light", [("origin", origin), ("_light", color), ("_lightHDR", "-1 -1 -1 1"),
                       ("_lightscaleHDR", "1"), ("_quadratic_attn", "1"), ("style", "0")])


def player(m, origin):
    m.entity("info_player_start", [("origin", origin), ("angles", "0 0 0")])


def box_light():
    m = Map("box_light", "Corpus fixture: one sealed room, one point light (no detail, no displacement).")
    shell(m, (-256, -256, 0), (256, 256, 256))
    light(m, "0 0 192")
    player(m, "-128 -128 1")
    return m


def disp_occluder():
    m = Map("disp_occluder", "Corpus fixture: a power-3 displacement hump between a point light and a floor\n"
                            "made of two sewn power-2 displacements, so light rays cross displacements.")
    x1, x2, y1, y2, z2 = -512, 512, -256, 256, 320
    # Sealing shell; the floor slab is nodraw under the displacement strip.
    m.add_world((x1 - 16, y1 - 16, -32), (x2 + 16, y2 + 16, -16), NODRAW)
    m.add_world((x1, y1, -16), (160, y2, 0), FLOOR)
    m.add_world((x1 - 16, y1 - 16, z2), (x2 + 16, y2 + 16, z2 + 16), WALL)
    m.add_world((x1 - 16, y1 - 16, -16), (x1, y2 + 16, z2), WALL)
    m.add_world((x2, y1 - 16, -16), (x2 + 16, y2 + 16, z2), WALL)
    m.add_world((x1, y1 - 16, -16), (x2, y1, z2), WALL)
    m.add_world((x1, y2, -16), (x2, y2 + 16, z2), WALL)
    sides = {f: NODRAW for f in ("bottom", "left", "right", "back", "front")}
    hump = lambda u, v: 128.0 * math.sin(math.pi * u) * math.sin(math.pi * v)
    m.add_world((-128, y1, 0), (128, y2, 16), GROUND, overrides=sides, disp={"top": (3, hump)})
    bump_a = lambda u, v: 16.0 * math.sin(math.pi * u) * math.sin(math.pi * v)
    bump_b = lambda u, v: 12.0 * math.sin(math.pi * u) * math.sin(math.pi * v)
    m.add_world((160, y1, -16), (336, y2, 0), GROUND, overrides=sides, disp={"top": (2, bump_a)})
    m.add_world((336, y1, -16), (x2, y2, 0), GROUND, overrides=sides, disp={"top": (2, bump_b)})
    light(m, "-352 0 288", "255 250 240 500")
    player(m, "-448 -192 1")
    return m


# Cell grid for the three-room map: '#' solid, anything else open.
VIS_GRID = [
    "############",
    "#AAA########",
    "#AAA....BBB#",
    "#AAA####BBB#",
    "########BBB#",
    "##########.#",
    "######.....#",
    "######.#####",
    "####CCC#####",
    "####CCC#####",
    "####CCC#####",
    "############",
]


def three_rooms():
    m = Map("three_rooms", "Corpus fixture: three rooms joined by a dogleg corridor (rooms A and C are\n"
                          "mutually hidden), with func_detail crates, for real VVIS work.")
    cell, height = 128, 256
    rows = len(VIS_GRID)
    cols = len(VIS_GRID[0])
    # World y grows downward in the grid; row r spans y [(rows-1-r)*cell, (rows-r)*cell].
    m.add_world((0, 0, -16), (cols * cell, rows * cell, 0), FLOOR)
    m.add_world((0, 0, height), (cols * cell, rows * cell, height + 16), WALL)
    for r, line in enumerate(VIS_GRID):
        c = 0
        while c < cols:
            if line[c] != "#":
                c += 1
                continue
            start = c
            while c < cols and line[c] == "#":
                c += 1
            y = (rows - 1 - r) * cell
            m.add_world((start * cell, y, 0), (c * cell, y + cell, height), WALL)

    def center(row, col, z):
        return "%d %d %d" % (col * cell + cell // 2, (rows - 1 - row) * cell + cell // 2, z)

    m.entity("func_detail", [], [((200, 1184, 0), (264, 1248, 48), DETAIL)])
    m.entity("func_detail", [], [((1136, 1056, 0), (1200, 1120, 96), DETAIL)])
    m.entity("func_detail", [], [((656, 176, 0), (720, 240, 64), DETAIL)])
    light(m, center(2, 2, 192))
    light(m, center(3, 9, 192))
    light(m, center(6, 8, 160), "255 230 200 150")
    light(m, center(9, 5, 192))
    player(m, center(2, 2, 1))
    return m


def sky_spot():
    m = Map("sky_spot", "Corpus fixture: a room lit by a light_environment through a skybox chimney\n"
                       "and by a light_spot; exercises sun, sky ambient and cone falloff.")
    x1, x2, y1, y2, z2 = -256, 256, -256, 256, 256
    m.add_world((x1 - 16, y1 - 16, -16), (x2 + 16, y2 + 16, 0), FLOOR)
    m.add_world((x1 - 16, y1 - 16, 0), (x1, y2 + 16, z2 + 16), WALL)
    m.add_world((x2, y1 - 16, 0), (x2 + 16, y2 + 16, z2 + 16), WALL)
    m.add_world((x1, y1 - 16, 0), (x2, y1, z2 + 16), WALL)
    m.add_world((x1, y2, 0), (x2, y2 + 16, z2 + 16), WALL)
    # Ceiling with a 128x128 opening at the centre.
    m.add_world((x1, y1, z2), (-64, y2, z2 + 16), WALL)
    m.add_world((64, y1, z2), (x2, y2, z2 + 16), WALL)
    m.add_world((-64, y1, z2), (64, -64, z2 + 16), WALL)
    m.add_world((-64, 64, z2), (64, y2, z2 + 16), WALL)
    # Sky chimney above the opening.
    top = z2 + 16 + 128
    m.add_world((-80, -80, z2 + 16), (-64, 80, top), SKY)
    m.add_world((64, -80, z2 + 16), (80, 80, top), SKY)
    m.add_world((-64, -80, z2 + 16), (64, -64, top), SKY)
    m.add_world((-64, 64, z2 + 16), (64, 80, top), SKY)
    m.add_world((-80, -80, top), (80, 80, top + 16), SKY)
    m.entity("light_environment", [("origin", "0 0 200"), ("angles", "-70 30 0"), ("pitch", "-70"),
                                   ("_light", "255 248 230 400"), ("_lightHDR", "-1 -1 -1 1"),
                                   ("_lightscaleHDR", "1"),
                                   ("_ambient", "120 140 170 40"), ("_ambientHDR", "-1 -1 -1 1"),
                                   ("_AmbientScaleHDR", "1"), ("SunSpreadAngle", "0")])
    m.entity("light_spot", [("origin", "-224 -224 224"), ("angles", "-40 45 0"), ("pitch", "-40"),
                            ("_light", "255 200 150 600"), ("_lightHDR", "-1 -1 -1 1"),
                            ("_lightscaleHDR", "1"), ("_cone", "35"), ("_inner_cone", "20"),
                            ("_exponent", "1"), ("_quadratic_attn", "1"), ("style", "0")])
    player(m, "128 -128 1")
    return m


MAKERS = (box_light, disp_occluder, three_rooms, sky_spot)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="exit 1 when a checked-in VMF differs from the generator output")
    args = parser.parse_args()
    stale = []
    os.makedirs(MAPS, exist_ok=True)
    for make in MAKERS:
        m = make()
        path = os.path.join(MAPS, m.name + ".vmf")
        text = m.text()
        current = open(path, encoding="utf-8").read() if os.path.exists(path) else None
        if current != text:
            stale.append(m.name)
            if not args.check:
                with open(path, "w", encoding="utf-8", newline="\n") as stream:
                    stream.write(text)
    if args.check and stale:
        print("stale fixture maps: " + ", ".join(stale))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
