"""corpus/compile_equivalence.py: the independent v21 reader, the DISPINFO
don't-care mask, PVS decode/encode, and the fixture generator. No compiler
executables are needed; the suite itself runs them (corpus.compile.* rows)."""

import json
import os
import struct
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "quality" / "fixtures" / "corpus" / "compile"))

from corpus import compile_equivalence as ce  # noqa: E402
import make_maps  # noqa: E402


def dispinfo_record(edge_neighbors, corner_counts, fill=0xAB):
    """One 176-byte ddispinfo_t whose unset bytes hold `fill` garbage."""
    rec = bytearray([fill] * 176)
    struct.pack_into("<3fiiiifi", rec, 0, 1.0, 2.0, 3.0, 0, 0, 3, 0, 0.0, 1)
    struct.pack_into("<H", rec, 36, 7)
    struct.pack_into("<ii", rec, 40, 0, 0)
    for sub, neighbor in enumerate(edge_neighbors):
        at = 48 + 6 * sub
        struct.pack_into("<H", rec, at, neighbor)
        if neighbor != 0xFFFF:
            rec[at + 2:at + 5] = b"\x01\x02\x03"
    for corner, count in enumerate(corner_counts):
        at = 96 + 10 * corner
        for slot in range(count):
            struct.pack_into("<H", rec, at + 2 * slot, 40 + slot)
        rec[at + 8] = count
    for i in range(10):
        struct.pack_into("<I", rec, 136 + 4 * i, 0xFFFFFFFF)
    return bytes(rec)


class DispinfoMaskTest(unittest.TestCase):
    edges = [3, 0xFFFF, 0xFFFF, 0xFFFF, 2, 0xFFFF, 0xFFFF, 0xFFFF]
    corners = [0, 1, 2, 0]

    def test_garbage_in_unset_bytes_is_ignored(self):
        a = dispinfo_record(self.edges, self.corners, fill=0x11)
        b = dispinfo_record(self.edges, self.corners, fill=0xEE)
        self.assertNotEqual(a, b)
        self.assertEqual(ce.lump_digest(ce.L_DISPINFO, a), ce.lump_digest(ce.L_DISPINFO, b))

    def test_every_defined_byte_is_compared(self):
        a = dispinfo_record(self.edges, self.corners)
        mask = ce.dispinfo_mask(a)
        for offset in range(len(a)):
            if not mask[offset]:
                continue
            b = bytearray(a)
            b[offset] ^= 0x01
            with self.subTest(offset=offset):
                self.assertNotEqual(ce.lump_digest(ce.L_DISPINFO, a)[0],
                                    ce.lump_digest(ce.L_DISPINFO, bytes(b))[0])

    def test_mask_covers_exactly_the_documented_fields(self):
        mask = ce.dispinfo_mask(dispinfo_record(self.edges, self.corners))
        unset = [i for i, m in enumerate(mask) if not m]
        # 2 pad after m_iMapFace; 8 sub-neighbor pads; 6 invalid sub-neighbors x 3;
        # corner slots 4+3+2+4 = 13 x 2 bytes; 4 corner pads.
        self.assertEqual(len(unset), 2 + 8 + 18 + 26 + 4)
        self.assertNotIn(36, unset)                       # m_iMapFace
        self.assertNotIn(48 + 2, unset)                   # a valid neighbor's orientation
        self.assertIn(54 + 2, unset)                      # an invalid neighbor's orientation

    def test_malformed_lumps_are_rejected(self):
        with self.assertRaises(ValueError):
            ce.dispinfo_mask(b"\0" * 175)
        rec = bytearray(dispinfo_record(self.edges, self.corners))
        rec[96 + 8] = 5
        with self.assertRaises(ValueError):
            ce.dispinfo_mask(bytes(rec))


class PvsTest(unittest.TestCase):
    rows = [[0, 1, 2], [0, 1], [0, 2, 9], [3], [4], [5], [6], [7], [8], [2, 9]]

    def pas(self):
        # A PAS block with all-visible rows (0xFF bytes) to carry through.
        count = len(self.rows)
        width = (count + 7) // 8
        body = b"\xff" * width
        head = 4 + 8 * count
        out = bytearray(struct.pack("<i", count))
        for c in range(count):
            out += struct.pack("<ii", head, head + c * 0)
        return bytes(out + body)

    def test_round_trip(self):
        encoded = ce.encode_pvs(self.rows, self.pas())
        self.assertEqual(ce.decode_pvs(encoded), self.rows)

    def test_truncated_rows_are_rejected(self):
        encoded = ce.encode_pvs(self.rows, self.pas())
        with self.assertRaises(ValueError):
            ce.decode_pvs(encoded[:4 + 8 * len(self.rows) + 1])

    def test_empty_visibility_is_none(self):
        self.assertIsNone(ce.decode_pvs(b""))


class ReaderTest(unittest.TestCase):
    def bsp(self, lumps):
        header = bytearray(b"VBSP" + struct.pack("<i", 21))
        body = bytearray()
        for i in range(64):
            data = lumps.get(i, b"")
            header += struct.pack("<iiii", ce.HEADER_BYTES + len(body) if data else 0,
                                  len(data), 0, 0)
            body += data
        header += struct.pack("<i", 1)
        return bytes(header + body)

    def test_lumps_and_header(self):
        parsed = ce.read_bsp(self.bsp({0: b"entities", 8: b"\1\2\3\4"}))
        self.assertEqual((parsed["version"], parsed["revision"]), (21, 1))
        self.assertEqual(parsed["lumps"][8]["data"], b"\1\2\3\4")

    def test_out_of_bounds_lump_is_rejected(self):
        data = bytearray(self.bsp({8: b"\1\2\3\4"}))
        struct.pack_into("<i", data, 8 + 16 * 8 + 4, 400)
        with self.assertRaises(ValueError):
            ce.read_bsp(bytes(data))

    def test_lighting_diff_decodes_rgbe(self):
        a = bytes([10, 20, 30, 0])
        b = bytes([10, 20, 31, 1])
        diff = ce.lighting_diff(a, b)
        self.assertEqual(diff["differ"], 1)
        self.assertAlmostEqual(diff["max_abs"], 62 - 30)


class FixtureTest(unittest.TestCase):
    def test_checked_in_maps_match_the_generator(self):
        for make in make_maps.MAKERS:
            m = make()
            with self.subTest(map=m.name):
                path = ROOT / "quality/fixtures/corpus/compile/maps" / (m.name + ".vmf")
                self.assertEqual(path.read_text(encoding="utf-8"), m.text())

    def test_expected_records_every_map_and_lump(self):
        path = ROOT / "quality/fixtures/corpus/compile/expected.json"
        expected = json.loads(path.read_text(encoding="utf-8"))
        self.assertEqual(expected["schema"], ce.SCHEMA)
        self.assertEqual(sorted(expected["maps"]), sorted(ce.MAPS))
        for name, spec in expected["maps"].items():
            with self.subTest(map=name):
                self.assertEqual(sorted(spec["lumps"], key=int), [str(i) for i in range(64)])
                self.assertEqual(len(spec["pvs"]["rows"]), spec["pvs"]["clusters"])


if __name__ == "__main__":
    unittest.main()
