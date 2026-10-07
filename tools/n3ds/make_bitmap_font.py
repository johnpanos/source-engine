#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Generate a tiny Source VGUI bitmap font for the 3DS: n3ds_small.

Output, for the name <font> (default n3ds_small):

  resource/<font>.vbf             the baked font (public/BitmapFontFile.h, version 3)
  materials/vgui/fonts/<font>.vtf the 128x64 glyph page, RGBA8888, white with alpha
  materials/vgui/fonts/<font>.vmt UnlitGeneric over that page (documentation only)

How the engine finds them (vgui2/vgui_surfacelib/BitmapFont.cpp): the scheme lists
the .vbf under BitmapFontFiles; CBitmapFont::Create reads that file through the
file system (path ID GAME) and takes the page from
FindTexture("vgui/fonts/<lower-case file base name>"), so the VTF path follows the
.vbf's base name. No VMT is read; the font texture cache builds its own
UnlitGeneric material around the texture name. The .vmt here is for tools.

    resource/clientscheme.res:
        BitmapFontFiles { "n3ds_small"  "resource/n3ds_small.vbf" }
        Fonts { "N3dsSmall" { "1" { "name" "n3ds_small" "bitmap" "1" } } }

The glyphs are a 5x7 pixel face (printable ASCII, 32-126) drawn for this tool and
embedded below as text. They are original work, released into the public domain
(CC0); they are not copied from any font file.

    python3 tools/n3ds/make_bitmap_font.py --out <dir> [--name n3ds_small]
"""

import argparse
import struct
import sys
from pathlib import Path

BITMAPFONT_ID = ord("T") << 24 | ord("N") << 16 | ord("F") << 8 | ord("V")
BITMAPFONT_VERSION = 3
PAGE_WIDTH, PAGE_HEIGHT = 128, 64
CELL_HEIGHT = 9          # 7 rows above the baseline, 2 for descenders
ASCENT = 7
ADVANCE_GAP = 1          # right bearing (c) of every glyph
SPACE_WIDTH = 3
STRIDE_X, STRIDE_Y = 7, 10   # one transparent pixel between glyph cells
FIRST, LAST = 32, 126

# VTF texture flags: point sample, clamp s/t, no mips, no lod, eight bit alpha.
VTF_FLAGS = 0x0001 | 0x0004 | 0x0008 | 0x0100 | 0x0200 | 0x2000

# '#' is a lit pixel. Seven rows, or nine for glyphs with descenders (g j p q y);
# lower-case letters sit in rows 2-6. Glyphs run from ASCII 32 (space) to 126 (~),
# separated by blank lines.
GLYPH_TEXT = """
.....
.....
.....
.....
.....
.....
.....

..#..
..#..
..#..
..#..
..#..
.....
..#..

.#.#.
.#.#.
.#.#.
.....
.....
.....
.....

.#.#.
.#.#.
#####
.#.#.
#####
.#.#.
.#.#.

..#..
.####
#.#..
.###.
..#.#
####.
..#..

##..#
##..#
...#.
..#..
.#...
#..##
#..##

.##..
#..#.
#.#..
.#...
#.#.#
#..#.
.##.#

..#..
..#..
.#...
.....
.....
.....
.....

...#.
..#..
.#...
.#...
.#...
..#..
...#.

.#...
..#..
...#.
...#.
...#.
..#..
.#...

.....
..#..
#.#.#
.###.
#.#.#
..#..
.....

.....
..#..
..#..
#####
..#..
..#..
.....

.....
.....
.....
.....
.##..
..#..
.#...

.....
.....
.....
#####
.....
.....
.....

.....
.....
.....
.....
.....
.##..
.##..

.....
....#
...#.
..#..
.#...
#....
.....

.###.
#...#
#..##
#.#.#
##..#
#...#
.###.

..#..
.##..
..#..
..#..
..#..
..#..
.###.

.###.
#...#
....#
...#.
..#..
.#...
#####

#####
...#.
..#..
...#.
....#
#...#
.###.

...#.
..##.
.#.#.
#..#.
#####
...#.
...#.

#####
#....
####.
....#
....#
#...#
.###.

..##.
.#...
#....
####.
#...#
#...#
.###.

#####
....#
...#.
..#..
.#...
.#...
.#...

.###.
#...#
#...#
.###.
#...#
#...#
.###.

.###.
#...#
#...#
.####
....#
...#.
.##..

.....
.##..
.##..
.....
.##..
.##..
.....

.....
.##..
.##..
.....
.##..
..#..
.#...

...#.
..#..
.#...
#....
.#...
..#..
...#.

.....
.....
#####
.....
#####
.....
.....

.#...
..#..
...#.
....#
...#.
..#..
.#...

.###.
#...#
....#
...#.
..#..
.....
..#..

.###.
#...#
#.###
#.#.#
#.###
#....
.###.

.###.
#...#
#...#
#####
#...#
#...#
#...#

####.
#...#
#...#
####.
#...#
#...#
####.

.###.
#...#
#....
#....
#....
#...#
.###.

###..
#..#.
#...#
#...#
#...#
#..#.
###..

#####
#....
#....
####.
#....
#....
#####

#####
#....
#....
####.
#....
#....
#....

.###.
#...#
#....
#.###
#...#
#...#
.####

#...#
#...#
#...#
#####
#...#
#...#
#...#

.###.
..#..
..#..
..#..
..#..
..#..
.###.

..###
...#.
...#.
...#.
...#.
#..#.
.##..

#...#
#..#.
#.#..
##...
#.#..
#..#.
#...#

#....
#....
#....
#....
#....
#....
#####

#...#
##.##
#.#.#
#.#.#
#...#
#...#
#...#

#...#
#...#
##..#
#.#.#
#..##
#...#
#...#

.###.
#...#
#...#
#...#
#...#
#...#
.###.

####.
#...#
#...#
####.
#....
#....
#....

.###.
#...#
#...#
#...#
#.#.#
#..#.
.##.#

####.
#...#
#...#
####.
#.#..
#..#.
#...#

.####
#....
#....
.###.
....#
....#
####.

#####
..#..
..#..
..#..
..#..
..#..
..#..

#...#
#...#
#...#
#...#
#...#
#...#
.###.

#...#
#...#
#...#
#...#
#...#
.#.#.
..#..

#...#
#...#
#...#
#.#.#
#.#.#
##.##
#...#

#...#
#...#
.#.#.
..#..
.#.#.
#...#
#...#

#...#
#...#
.#.#.
..#..
..#..
..#..
..#..

#####
....#
...#.
..#..
.#...
#....
#####

.###.
.#...
.#...
.#...
.#...
.#...
.###.

.....
#....
.#...
..#..
...#.
....#
.....

.###.
...#.
...#.
...#.
...#.
...#.
.###.

..#..
.#.#.
#...#
.....
.....
.....
.....

.....
.....
.....
.....
.....
.....
#####

.#...
..#..
...#.
.....
.....
.....
.....

.....
.....
.###.
....#
.####
#...#
.####

#....
#....
#.##.
##..#
#...#
#...#
####.

.....
.....
.###.
#....
#....
#...#
.###.

....#
....#
.##.#
#..##
#...#
#...#
.####

.....
.....
.###.
#...#
#####
#....
.###.

..##.
.#..#
.#...
###..
.#...
.#...
.#...

.....
.....
.####
#...#
#...#
.####
....#
....#
.###.

#....
#....
#.##.
##..#
#...#
#...#
#...#

..#..
.....
.##..
..#..
..#..
..#..
.###.

...#.
.....
..##.
...#.
...#.
...#.
...#.
#..#.
.##..

#....
#....
#..#.
#.#..
##...
#.#..
#..#.

.##..
..#..
..#..
..#..
..#..
..#..
.###.

.....
.....
##.#.
#.#.#
#.#.#
#...#
#...#

.....
.....
#.##.
##..#
#...#
#...#
#...#

.....
.....
.###.
#...#
#...#
#...#
.###.

.....
.....
####.
#...#
#...#
####.
#....
#....
#....

.....
.....
.####
#...#
#...#
.####
....#
....#
....#

.....
.....
#.##.
##..#
#....
#....
#....

.....
.....
.####
#....
.###.
....#
####.

.#...
.#...
###..
.#...
.#...
.#..#
..##.

.....
.....
#...#
#...#
#...#
#..##
.##.#

.....
.....
#...#
#...#
#...#
.#.#.
..#..

.....
.....
#...#
#...#
#.#.#
#.#.#
.#.#.

.....
.....
#...#
.#.#.
..#..
.#.#.
#...#

.....
.....
#...#
#...#
#...#
.####
....#
....#
.###.

.....
.....
#####
...#.
..#..
.#...
#####

...##
..#..
..#..
.#...
..#..
..#..
...##

..#..
..#..
..#..
..#..
..#..
..#..
..#..

##...
..#..
..#..
...#.
..#..
..#..
##...

.....
.....
.#...
#.#.#
...#.
.....
.....
"""


def parse_glyphs(text=GLYPH_TEXT):
    """[rows] per ASCII code FIRST..LAST; each rows is CELL_HEIGHT strings of 5 characters."""
    blocks = [block.split() for block in text.strip().split("\n\n")]
    if len(blocks) != LAST - FIRST + 1:
        raise ValueError("expected %d glyphs, found %d" % (LAST - FIRST + 1, len(blocks)))
    result = []
    for code, rows in zip(range(FIRST, LAST + 1), blocks):
        if len(rows) not in (7, CELL_HEIGHT) or any(len(row) != 5 or set(row) - set("#.")
                                                    for row in rows):
            raise ValueError("glyph %d (%r) is malformed" % (code, chr(code)))
        result.append(rows + ["....."] * (CELL_HEIGHT - len(rows)))
    return result


def ink_columns(rows):
    """(first, last) lit column of a glyph, or None when it is blank."""
    lit = [x for row in rows for x, ch in enumerate(row) if ch == "#"]
    return (min(lit), max(lit)) if lit else None


def layout(glyphs):
    """[(x, y, w, h, a, b, c, first_column)] per glyph, packed on the page."""
    per_row = PAGE_WIDTH // STRIDE_X
    placed = []
    for index, rows in enumerate(glyphs):
        x = (index % per_row) * STRIDE_X
        y = (index // per_row) * STRIDE_Y
        if y + CELL_HEIGHT > PAGE_HEIGHT:
            raise ValueError("glyphs do not fit the page")
        span = ink_columns(rows)
        if span is None:
            width, first = SPACE_WIDTH, 0
        else:
            first, width = span[0], span[1] - span[0] + 1
        placed.append((x, y, width, CELL_HEIGHT, 0, width, ADVANCE_GAP, first))
    return placed


def build_page(glyphs, placed):
    """RGBA bytes of the page: lit pixels white and opaque, the rest white, alpha 0."""
    page = bytearray(b"\xff\xff\xff\x00" * (PAGE_WIDTH * PAGE_HEIGHT))
    for rows, (x, y, w, h, _a, _b, _c, first) in zip(glyphs, placed):
        for row, line in enumerate(rows):
            for column in range(w):
                if line[first + column] == "#":
                    page[4 * ((y + row) * PAGE_WIDTH + x + column) + 3] = 0xFF
    return bytes(page)


def build_vbf(placed):
    table = bytearray(256)
    question = ord("?") - FIRST
    for code in range(256):
        # Control characters draw as a space, anything outside ASCII as "?".
        table[code] = code - FIRST if FIRST <= code <= LAST else (0 if code < FIRST else question)
    header = struct.pack("<iihhhhhhh", BITMAPFONT_ID, BITMAPFONT_VERSION, PAGE_WIDTH, PAGE_HEIGHT,
                         max(p[2] + p[6] for p in placed), CELL_HEIGHT, 0, ASCENT, len(placed))
    body = b"".join(struct.pack("<7h", *p[:7]) for p in placed)
    return header + bytes(table) + body


def build_vtf(rgba):
    """VTF 7.2, RGBA8888, one mip, no thumbnail."""
    header = bytearray(80)
    header[0:4] = b"VTF\0"
    struct.pack_into("<3I", header, 4, 7, 2, 80)
    struct.pack_into("<HHIHH", header, 16, PAGE_WIDTH, PAGE_HEIGHT, VTF_FLAGS, 1, 0)
    struct.pack_into("<3f", header, 32, 1.0, 1.0, 1.0)
    struct.pack_into("<f", header, 48, 1.0)
    struct.pack_into("<iBiBB", header, 52, 0, 1, -1, 0, 0)
    struct.pack_into("<H", header, 63, 1)
    return bytes(header) + rgba


def build_vmt(name):
    return ('"UnlitGeneric"\n{\n\t"$basetexture" "vgui/fonts/%s"\n\t"$vertexcolor" 1\n'
            '\t"$vertexalpha" 1\n\t"$translucent" 1\n\t"$ignorez" 1\n\t"$nomip" 1\n}\n'
            % name).encode()


def build_font(name="n3ds_small"):
    """{logical path: bytes} of the font's three files."""
    glyphs = parse_glyphs()
    placed = layout(glyphs)
    return {
        "resource/%s.vbf" % name: build_vbf(placed),
        "materials/vgui/fonts/%s.vtf" % name: build_vtf(build_page(glyphs, placed)),
        "materials/vgui/fonts/%s.vmt" % name: build_vmt(name),
    }


# Scheme hookup. Every glyph set of every font in a scheme's Fonts block becomes one
# bitmap set over this font, the scheme gains a BitmapFontFiles entry, and its
# CustomFontFiles block (TrueType files) is removed. The set
# carries "yres" "1 100000": CScheme::ReloadFontGlyphs scales scalex/scaley of
# proportional fonts that have no yres filter by screen height / 480, which at the
# 3DS's 240 lines would halve the 5x7 glyphs.
SCHEME_YRES = "1 100000"


def _kv_tokens(text):
    """(kind, value, start, end) tokens of KeyValues text: str, {, }, cond; comments skipped."""
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in " \t\r\n":
            i += 1
        elif text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif c in "{}":
            yield c, c, i, i + 1
            i += 1
        elif c == "[":
            j = text.index("]", i)
            yield "cond", text[i:j + 1], i, j + 1
            i = j + 1
        elif c == '"':
            j = text.index('"', i + 1)
            yield "str", text[i + 1:j], i, j + 1
            i = j + 1
        else:
            j = i
            while j < n and text[j] not in ' \t\r\n{}"[':
                j += 1
            yield "str", text[i:j], i, j
            i = j


def _parse_block(tokens, k):
    """Entries [(key, cond, value or list, start, end)] of the block whose '{' is tokens[k-1];
    returns (entries, index after the closing '}')."""
    entries = []
    while tokens[k][0] != "}":
        key, start = tokens[k][1], tokens[k][2]
        k += 1
        cond = None
        if tokens[k][0] == "cond":
            cond, k = tokens[k][1], k + 1
        if tokens[k][0] == "{":
            value, k = _parse_block(tokens, k + 1)
        else:
            value, k = tokens[k][1], k + 1
        if k < len(tokens) and tokens[k][0] == "cond":
            cond, k = tokens[k][1], k + 1
        entries.append((key, cond, value, start, tokens[k - 1][3]))
    return entries, k + 1


def hook_scheme(text, name="n3ds_small"):
    """A scheme's text with every font drawn by the bitmap font <name>; None if it has no Fonts."""
    tokens = list(_kv_tokens(text))
    if len(tokens) < 3 or tokens[1][0] != "{":
        return None
    root, _ = _parse_block(tokens, 2)
    fonts = [e for e in root if e[0].lower() == "fonts" and isinstance(e[2], list)]
    if not fonts:
        return None
    nl = "\r\n" if "\r\n" in text else "\n"
    out, last = [], 0
    # CustomFontFiles registers TrueType files with the surface (AddCustomFontFile reads
    # them into memory); no font names them any more, so the block goes.
    custom = [e for e in root if e[0].lower() == "customfontfiles"]
    for key, cond, entries, start, end in sorted(fonts + custom, key=lambda e: e[3]):
        if key.lower() == "customfontfiles":
            out.append(text[last:start].rstrip(" \t"))
            last = end
            continue
        lines = ["BitmapFontFiles", "\t{", '\t\t"%s"\t"resource/%s.vbf"' % (name, name), "\t}",
                 "\tFonts", "\t{"]
        for font, font_cond, sets, _, _ in entries:
            if not isinstance(sets, list):
                continue
            lines.append('\t\t"%s"%s' % (font, " " + font_cond if font_cond else ""))
            lines.append("\t\t{")
            for set_key, set_cond, value, _, _ in sets:
                if set_key.lower() == "isproportional" and not isinstance(value, list):
                    lines.append('\t\t\t"isproportional"\t"%s"%s'
                                 % (value, " " + set_cond if set_cond else ""))
            lines.append('\t\t\t"1" { "name" "%s" "bitmap" "1" "yres" "%s" }' % (name, SCHEME_YRES))
            lines.append("\t\t}")
        lines.append("\t}")
        out.append(text[last:start])
        out.append(nl.join(lines))
        last = end
    out.append(text[last:])
    return "".join(out)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, required=True, help="game directory to write into")
    parser.add_argument("--name", default="n3ds_small", help="font base name (lower case)")
    args = parser.parse_args(argv)
    if args.name != args.name.lower():
        parser.error("--name must be lower case: the engine looks the page up in lower case")
    for logical, data in build_font(args.name).items():
        destination = args.out / logical
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
        print("%7d  %s" % (len(data), destination))
    return 0


if __name__ == "__main__":
    sys.exit(main())
