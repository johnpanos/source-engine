"""Independent BSP2 reader: synthetic round trips, negative fixtures, and a
cross-implementation check against the C++ bsp2tool (RFC 0008 F1)."""

from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))

import bsp2_reader as br  # noqa: E402


def legacy_map(gap=True):
    header = bytearray(br.LEGACY_HEADER_SIZE)
    header[:4] = br.LEGACY_IDENT
    struct.pack_into("<i", header, 4, 20)
    for i in range(br.LEGACY_LUMPS):
        struct.pack_into("<i", header, 8 + 16 * i, 0x2000 + i)  # zero-length lumps, bogus offsets
    body = bytearray(header)
    for index, size in ((0, 37), (1, 200), (40, 513), (7, 56)):
        body.extend(b"\0" * (-len(body) % 4))
        if gap:
            body.extend(b"\xa1\xa2")
        offset = len(body)
        body.extend(bytes((index + k * 7) & 0xFF for k in range(size)))
        struct.pack_into("<iiii", body, 8 + 16 * index, offset, size, 1 if index == 40 else 0, 0)
    if gap:
        body.extend(b"\x5a\x00\x5a")
    struct.pack_into("<i", body, 8 + 16 * br.LEGACY_LUMPS, 77)
    return bytes(body)


def directory(data):
    offset, count = struct.unpack_from("<QI", data, 24)
    return offset, count


def entry_at(data, name):
    offset, count = directory(data)
    for i in range(count):
        at = offset + i * br.ENTRY_SIZE
        if struct.unpack_from("<I", data, at)[0] == br.fourcc(name):
            return at
    raise KeyError(name)


def rehash(data):
    offset, count = directory(data)
    data[48:64] = br.blake128(bytes(data[offset:offset + count * br.ENTRY_SIZE]))


class Bsp2ReaderTests(unittest.TestCase):
    def test_round_trip(self):
        for gap in (True, False):
            original = legacy_map(gap)
            converted = br.convert_legacy(original)
            parsed = br.Bsp2File(converted)
            parsed.check_legacy_structures()
            self.assertEqual(parsed.export_legacy(), original)
            self.assertEqual(br.LGAP in parsed.by_id, gap)
            for entry in parsed.entries:
                self.assertEqual(entry["offset"] % 16, 0)

    def test_negative_fixtures(self):
        good = bytearray(br.convert_legacy(legacy_map()))

        def flip_payload(d):
            d[struct.unpack_from("<Q", d, entry_at(d, "L001") + 16)[0]] ^= 1

        def misalign(d):
            at = entry_at(d, "L001") + 16
            struct.pack_into("<Q", d, at, struct.unpack_from("<Q", d, at)[0] + 4)
            rehash(d)

        def overlap(d):
            first = struct.unpack_from("<Q", d, entry_at(d, "L000") + 16)[0]
            struct.pack_into("<Q", d, entry_at(d, "L001") + 16, first)
            rehash(d)

        def required_unknown(d):
            struct.pack_into("<I", d, entry_at(d, "L007"), br.fourcc("ZZZZ"))
            rehash(d)

        def compressed(d):
            struct.pack_into("<I", d, entry_at(d, "L001") + 8, br.FLAG_REQUIRED | 1)
            rehash(d)

        def drop_lhdr(d):
            at = entry_at(d, "LHDR")
            struct.pack_into("<II", d, at, br.fourcc("OPT1"), 1)
            struct.pack_into("<I", d, at + 8, 0)
            rehash(d)

        cases = {
            "bad-magic": lambda d: d.__setitem__(0, ord("X")),
            "unsupported-container-version": lambda d: struct.pack_into("<I", d, 8, 2),
            "unsupported-hash": lambda d: struct.pack_into("<I", d, 36, 7),
            "directory-out-of-bounds": lambda d: d.pop(),
            "directory-hash-mismatch": lambda d: d.__setitem__(directory(d)[0] + 5, d[directory(d)[0] + 5] ^ 1),
            "content-hash-mismatch": flip_payload,
            "lump-misaligned": misalign,
            "lump-overlap": overlap,
            "unknown-required-lump": required_unknown,
            "unsupported-compression": compressed,
            "missing-legacy-header": drop_lhdr,
            "legacy-header-invalid": lambda d: struct.pack_into("<i", d, 40, 5),
        }
        for code, mutate in cases.items():
            with self.subTest(code=code):
                data = bytearray(good)
                mutate(data)
                with self.assertRaises(br.FormatError) as caught:
                    br.Bsp2File(bytes(data))
                self.assertEqual(caught.exception.code, code)

    def test_game_lump_offsets_checked_against_origin(self):
        # A game lump dictionary whose absolute offsets leave its lump is
        # rejected; relative to the legacy origin it is accepted.
        origin = 5000
        payload = struct.pack("<i", 1) + struct.pack("<iHHii", 1, 0, 1, origin + 20, 8) + b"\0" * 16
        br.check_legacy_structure(20, br.LUMP_GAME, payload, origin)
        with self.assertRaises(br.FormatError):
            br.check_legacy_structure(20, br.LUMP_GAME, payload, 64)

    def test_matches_cpp_implementation(self):
        compiler = shutil.which("c++")
        self.assertIsNotNone(compiler, "the required native C++ compiler is missing")
        with tempfile.TemporaryDirectory(prefix="bsp2-reader-") as scratch:
            scratch = Path(scratch)
            tool = scratch / "bsp2tool"
            build = subprocess.run(
                [compiler, "-std=c++20", "-O1", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT / "public"),
                 str(ROOT / "mapcontainer/blake2b.cpp"), str(ROOT / "mapcontainer/map_container.cpp"),
                 str(ROOT / "utils/bsp2tool/bsp2tool.cpp"), "-o", str(tool)],
                text=True, capture_output=True, timeout=120)
            self.assertEqual(build.returncode, 0, build.stdout + build.stderr)

            legacy = scratch / "map.bsp"
            legacy.write_bytes(legacy_map())
            cxx_out = scratch / "map.bsp2"
            run = subprocess.run([str(tool), "convert", str(legacy), str(cxx_out)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual(cxx_out.read_bytes(), br.convert_legacy(legacy.read_bytes()))

            # Both readers report the same error name for the same defect.
            bad = bytearray(cxx_out.read_bytes())
            bad[struct.unpack_from("<Q", bad, entry_at(bad, "L040") + 16)[0]] ^= 1
            bad_path = scratch / "bad.bsp2"
            bad_path.write_bytes(bytes(bad))
            run = subprocess.run([str(tool), "verify", str(bad_path)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 1)
            self.assertIn("content-hash-mismatch", run.stderr)
            with self.assertRaises(br.FormatError) as caught:
                br.Bsp2File(bytes(bad))
            self.assertEqual(caught.exception.code, "content-hash-mismatch")


if __name__ == "__main__":
    unittest.main()
