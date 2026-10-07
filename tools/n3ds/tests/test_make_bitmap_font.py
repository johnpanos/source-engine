#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""make_bitmap_font: the .vbf, its page texture, judged by an independent reader.

The reader follows public/BitmapFontFile.h and CBitmapFont (vgui2/vgui_surfacelib):
BitmapFont_t (pack 1: id, version, six shorts, glyph count, 256-byte translate
table) then BitmapGlyph_t records of seven shorts. It shares no code with the
generator; the glyph source is re-parsed here from the embedded text.

    python3 -m unittest discover -s tools/n3ds/tests -v
"""

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import make_bitmap_font as mbf  # noqa: E402


def read_vbf(data):
    """(font dict, glyph list) exactly as CBitmapFont::Create reads them."""
    if len(data) < 278:
        raise ValueError("short header")
    ident = data[0:4]
    version = struct.unpack_from("<i", data, 4)[0]
    if ident != b"VFNT" or version != 3:      # 'T'<<24|'N'<<16|'F'<<8|'V', little endian
        raise ValueError("bad id or version")
    page_w, page_h, max_w, max_h, flags, ascent, count = struct.unpack_from("<7h", data, 8)
    table = list(data[22:278])
    if len(data) != 278 + 14 * count:
        raise ValueError("size does not match the glyph count")
    glyphs = [struct.unpack_from("<7h", data, 278 + 14 * i) for i in range(count)]
    return {"page": (page_w, page_h), "max": (max_w, max_h), "flags": flags, "ascent": ascent,
            "table": table}, glyphs


def read_vtf_alpha(data):
    """(width, height, flags, [alpha rows]) of a one-mip RGBA8888 VTF."""
    if data[:4] != b"VTF\0" or struct.unpack_from("<2I", data, 4) != (7, 2):
        raise ValueError("not a VTF 7.2")
    header_size = struct.unpack_from("<I", data, 12)[0]
    width, height, flags, frames = struct.unpack_from("<HHIH", data, 16)
    fmt, mips, low_fmt, low_w, low_h = struct.unpack_from("<iBiBB", data, 52)
    if (fmt, mips, frames) != (0, 1, 1) or (low_fmt, low_w, low_h) != (-1, 0, 0):
        raise ValueError("expected RGBA8888, 1 mip, 1 frame, no thumbnail")
    if len(data) != header_size + 4 * width * height:
        raise ValueError("image size mismatch")
    pixels = data[header_size:]
    rows = [[pixels[4 * (y * width + x) + 3] for x in range(width)] for y in range(height)]
    for i in range(0, len(pixels), 4):
        if pixels[i:i + 3] != b"\xff\xff\xff":
            raise ValueError("glyph colour is not white")
    return width, height, flags, rows


def source_glyphs():
    """Re-parse the embedded glyph text: {ascii code: 9 rows of 5 characters}."""
    blocks = [b.split() for b in mbf.GLYPH_TEXT.strip().split("\n\n")]
    return {32 + i: rows + ["....."] * (9 - len(rows)) for i, rows in enumerate(blocks)}


def check(vbf, vtf):
    """Problems found in a (vbf, vtf) pair; empty when the font is sound."""
    problems = []
    try:
        font, glyphs = read_vbf(vbf)
        width, height, flags, alpha = read_vtf_alpha(vtf)
    except (ValueError, struct.error) as error:
        return ["unreadable: %s" % error]
    if font["page"] != (width, height):
        problems.append("page size differs from the texture")
    if flags & 0x100 == 0:
        problems.append("texture has mipmaps enabled")
    if len(glyphs) != 95:
        problems.append("glyph count")
        return problems
    source = source_glyphs()
    for code in range(32, 127):
        x, y, w, h, a, b, c = glyphs[code - 32]
        if w < 1 or h < 1 or x < 0 or y < 0 or x + w > width or y + h > height:
            problems.append("glyph %d rect off the page" % code)
            continue
        if b != w or a != 0 or c < 0:
            problems.append("glyph %d widths" % code)
        rows = source[code]
        ink = [i for row in rows for i, ch in enumerate(row) if ch == "#"]
        first = min(ink) if ink else 0
        for r in range(h):
            for i in range(w):
                lit = alpha[y + r][x + i] == 255
                expected = first + i < 5 and rows[r][first + i] == "#"
                if lit != expected:
                    problems.append("glyph %d pixel (%d,%d) differs from the source" % (code, i, r))
                    break
            else:
                continue
            break
        if any(alpha[y + r][x + i] not in (0, 255) for r in range(h) for i in range(w)):
            problems.append("glyph %d has partial alpha" % code)
    # No two glyph rects overlap.
    rects = [g[:4] for g in glyphs]
    for i, (x, y, w, h) in enumerate(rects):
        for x2, y2, w2, h2 in rects[i + 1:]:
            if x < x2 + w2 and x2 < x + w and y < y2 + h2 and y2 < y + h:
                problems.append("glyph rects overlap")
    table = font["table"]
    for code in range(256):
        if not 0 <= table[code] < 95:
            problems.append("translate entry %d out of range" % code)
        elif 32 <= code <= 126 and table[code] != code - 32:
            problems.append("translate entry %d" % code)
    if font["max"] != (max(g[5] + g[6] for g in glyphs), 9) or font["ascent"] != 7:
        problems.append("max size or ascent")
    return problems


class BitmapFont(unittest.TestCase):
    def setUp(self):
        files = mbf.build_font("n3ds_small")
        self.vbf = files["resource/n3ds_small.vbf"]
        self.vtf = files["materials/vgui/fonts/n3ds_small.vtf"]
        self.files = files

    def test_file_set_and_names(self):
        self.assertEqual(sorted(self.files), ["materials/vgui/fonts/n3ds_small.vmt",
                                              "materials/vgui/fonts/n3ds_small.vtf",
                                              "resource/n3ds_small.vbf"])
        self.assertIn(b"vgui/fonts/n3ds_small", self.files["materials/vgui/fonts/n3ds_small.vmt"])

    def test_font_is_sound(self):
        self.assertEqual(check(self.vbf, self.vtf), [])
        self.assertLess(len(self.vbf) + len(self.vtf), 40000)

    def test_a_few_glyphs_match_the_source(self):
        font, glyphs = read_vbf(self.vbf)
        _w, _h, _f, alpha = read_vtf_alpha(self.vtf)
        # 'I' is .###. / ..#.. x5 / .###.: 3 wide, rows 0 and 6 full, the stem in column 1.
        x, y, w, h, a, b, c = glyphs[ord("I") - 32]
        self.assertEqual((w, h, a, b, c), (3, 9, 0, 3, 1))
        self.assertEqual([alpha[y][x + i] for i in range(3)], [255, 255, 255])
        self.assertEqual([alpha[y + 3][x + i] for i in range(3)], [0, 255, 0])
        self.assertEqual([alpha[y + 8][x + i] for i in range(3)], [0, 0, 0])
        # '.' is a 2x2 dot in rows 5 and 6.
        x, y, w, h, *_ = glyphs[ord(".") - 32]
        self.assertEqual(w, 2)
        self.assertEqual([sum(1 for i in range(w) if alpha[y + r][x + i] == 255)
                          for r in range(9)], [0, 0, 0, 0, 0, 2, 2, 0, 0])
        # 'g' has a descender below the baseline (rows 7 and 8).
        x, y, w, h, *_ = glyphs[ord("g") - 32]
        self.assertTrue(any(alpha[y + 8][x + i] == 255 for i in range(w)))
        # space: blank, advance 3 + 1.
        x, y, w, h, a, b, c = glyphs[0]
        self.assertEqual((w, b, c), (3, 3, 1))
        self.assertTrue(all(alpha[y + r][x + i] == 0 for r in range(h) for i in range(w)))
        # Characters outside ASCII draw as '?', control characters as a space.
        self.assertEqual(font["table"][200], ord("?") - 32)
        self.assertEqual(font["table"][9], 0)

    def test_glyph_source_is_complete(self):
        glyphs = mbf.parse_glyphs()
        self.assertEqual(len(glyphs), 95)
        self.assertEqual(sorted(source_glyphs()), list(range(32, 127)))
        with self.assertRaises(ValueError):
            mbf.parse_glyphs(mbf.GLYPH_TEXT + "\n\n.....\n.....")

    def test_seeded_corruptions_are_caught(self):
        def patched(blob, index, value):
            out = bytearray(blob)
            out[index] = value
            return bytes(out)
        glyph_a = 278 + 14 * (ord("A") - 32)
        a_x = struct.unpack_from("<h", self.vbf, glyph_a)[0]
        a_y = struct.unpack_from("<h", self.vbf, glyph_a + 2)[0]
        pixel = 80 + 4 * (a_y * 128 + a_x) + 3
        cases = {
            "magic": (patched(self.vbf, 0, 0x58), self.vtf),
            "version": (patched(self.vbf, 4, 2), self.vtf),
            "glyph rect off page": (patched(self.vbf, glyph_a, 0x7F), self.vtf),
            "glyph wider": (patched(self.vbf, glyph_a + 4, 9), self.vtf),
            "translate": (patched(self.vbf, 22 + ord("A"), 3), self.vtf),
            "page size": (patched(self.vbf, 8, 64), self.vtf),
            "truncated vbf": (self.vbf[:-7], self.vtf),
            "pixel flipped": (self.vbf, patched(self.vtf, pixel, 255 - self.vtf[pixel])),
            "colour not white": (self.vbf, patched(self.vtf, pixel - 3, 0)),
            "texture truncated": (self.vbf, self.vtf[:-4]),
            "mips on": (self.vbf, patched(self.vtf, 21, 0)),
        }
        for name, (vbf, vtf) in cases.items():
            self.assertNotEqual(check(vbf, vtf), [], name)



SCHEME = ('"Scheme"\r\n{\r\n\tColors { "White" "255 255 255 255" }\r\n\tFonts\r\n\t{\r\n'
          '\t\t// comment { not a block\r\n'
          '\t\t"Default" [!$GAMECONSOLE]\r\n\t\t{\r\n\t\t\t"isproportional" "only"\r\n'
          '\t\t\t"1" { "name" "Verdana" "tall" "9" [!$GAMECONSOLE] "yres" "480 599" }\r\n'
          '\t\t\t"2" { "name" "Verdana" "tall" "12" }\r\n\t\t}\r\n'
          '\t\tMarlett { "1" { "name" "Marlett" "symbol" "1" } }\r\n\t}\r\n'
          '\tBorders { }\r\n'
          '\tCustomFontFiles\r\n\t{\r\n\t\t"1" "resource/TitilliumWeb.ttf"\r\n'
          '\t\t"2" { "font" "resource/x.ttf" "name" "X" }\r\n\t}\r\n}\r\n')


class SchemeHook(unittest.TestCase):
    def parse(self, text):
        tokens = list(mbf._kv_tokens(text))
        root, end = mbf._parse_block(tokens, 2)
        self.assertEqual(end, len(tokens))
        return {e[0]: e for e in root}

    def test_every_font_is_one_bitmap_set(self):
        out = mbf.hook_scheme(SCHEME, "n3ds_small")
        root = self.parse(out)
        files = [(e[0], e[2]) for e in root["BitmapFontFiles"][2]]
        self.assertEqual(files, [("n3ds_small", "resource/n3ds_small.vbf")])
        fonts = root["Fonts"][2]
        self.assertEqual([(f[0], f[1]) for f in fonts],
                         [("Default", "[!$GAMECONSOLE]"), ("Marlett", None)])
        for font in fonts:
            sets = [e for e in font[2] if isinstance(e[2], list)]
            self.assertEqual(len(sets), 1)
            fields = {e[0]: e[2] for e in sets[0][2]}
            self.assertEqual(fields, {"name": "n3ds_small", "bitmap": "1",
                                      "yres": mbf.SCHEME_YRES})
        self.assertEqual([(e[0], e[2]) for e in fonts[0][2] if not isinstance(e[2], list)],
                         [("isproportional", "only")])
        # Neighbouring blocks and line endings survive.
        self.assertIn("Colors", root)
        self.assertIn("Borders", root)
        self.assertNotIn("CustomFontFiles", root)
        self.assertNotIn(".ttf", out)
        self.assertNotIn("\n", out.replace("\r\n", ""))

    def test_yres_bypasses_proportional_scaling(self):
        lo, hi = map(int, mbf.SCHEME_YRES.split())
        self.assertTrue(lo <= 240 <= hi and lo > 0)

    def test_scheme_without_fonts_is_left_alone(self):
        self.assertIsNone(mbf.hook_scheme('"Layout" { "a" "b" }'))

if __name__ == "__main__":
    unittest.main()
