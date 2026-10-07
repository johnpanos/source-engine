import io
import struct
import sys
import unittest
import zipfile
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import gameplay_identity  # noqa: E402
import legacy_bsp  # noqa: E402
import prop_vertex_light as P  # noqa: E402


def pak(entries):
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", zipfile.ZIP_STORED) as target:
        for name, data in entries.items():
            target.writestr(name, data)
    return buffer.getvalue()


def bsp_with_pak(blob):
    header = 8 + 64 * 16 + 4
    data = bytearray(header)
    struct.pack_into("<4si", data, 0, b"VBSP", 21)
    data += b"ABCD"  # a lump before the pak
    struct.pack_into("<ii", data, 8, header, 4)
    struct.pack_into("<ii", data, 8 + 16 * legacy_bsp.LUMP_PAKFILE, len(data), len(blob))
    data += blob
    return bytes(data)


class EncodingTest(unittest.TestCase):
    def test_round_trip_within_a_code(self):
        light = np.array([[0.0, 0.01, 0.1], [0.5, 1.0, 2.0], [4.0, 3.0, 0.2]])
        decoded = P.decode(P.encode(light))
        # One code step of c = 0.5 L^(1/2.2) in light.
        for value, back in zip(light.ravel(), decoded.ravel()):
            step = (2.0 * (0.5 * value ** (1 / 2.2) + 1 / 255.0)) ** 2.2 - value
            self.assertLessEqual(abs(back - value), abs(step) + 1e-9)

    def test_bright_colour_keeps_its_hue(self):
        light = np.array([[40.0, 20.0, 10.0]])
        bgra = P.encode(light)[0]
        self.assertEqual(bgra[2], 255)  # red is the peak, scaled to 1
        self.assertEqual(bgra[3], 255)
        ratio = (bgra[1] / 255.0) / 1.0
        expected = (20.0 / 40.0) ** (1 / 2.2)
        self.assertAlmostEqual(ratio, expected, delta=1 / 255.0)

    def test_byte_order_is_bgra(self):
        bgra = P.encode(np.array([[1.0, 0.0, 0.0]]))[0]
        self.assertEqual(bgra[0], 0)
        self.assertGreater(bgra[2], 0)

    def test_encoded_bytes_decode_back_to_themselves(self):
        codes = np.random.default_rng(3).integers(0, 256, (500, 4)).astype(np.uint8)
        codes[:, 3] = 255
        light = P.decode(codes)
        np.testing.assert_array_equal(P.encode(light), codes)


class ColourMeshTest(unittest.TestCase):
    def test_layout_round_trip_and_alignment(self):
        colours = np.arange(4 * 7, dtype=np.uint8).reshape(7, 4)
        data = P.vhv_file(-5, [(0, 4), (1, 3)], colours)
        self.assertEqual(len(data) % P.VHV_ALIGN, 0)
        checksum, groups, read = P.read_vhv(data)
        self.assertEqual(checksum, (-5) & 0xffffffff)
        self.assertEqual(groups, [(0, 4), (1, 3)])
        np.testing.assert_array_equal(read, colours)
        offset = struct.unpack_from("<III", data, struct.calcsize("<iIIIIi4I"))[2]
        self.assertEqual(offset, P.VHV_ALIGN)

    def test_count_mismatch_is_refused(self):
        with self.assertRaises(ValueError):
            P.vhv_file(1, [(0, 4)], np.zeros((3, 4), np.uint8))


class EnclosedTest(unittest.TestCase):
    def test_inside_solid_takes_nearest_open_sample_of_its_prop(self):
        positions = np.array([[0.0, 0, 0], [1, 0, 0], [10, 0, 0], [50, 0, 0]])
        light = np.array([[0.0, 0, 0], [0.2, 0.2, 0.2], [0.5, 0.5, 0.5], [0.0, 0, 0]])
        inside = np.array([True, False, False, True])
        filled, count = P.fill_enclosed(light, positions, np.array([0, 1, 2]), inside)
        self.assertEqual(count, 1)
        np.testing.assert_array_equal(filled[0], light[1])
        np.testing.assert_array_equal(filled[3], [0, 0, 0])  # another prop's sample

    def test_open_dark_samples_keep_their_darkness(self):
        positions = np.array([[0.0, 0, 0], [1, 0, 0]])
        light = np.array([[0.0, 0, 0], [0.5, 0.5, 0.5]])
        filled, count = P.fill_enclosed(light, positions, np.array([0, 1]),
                                        np.array([False, False]))
        self.assertEqual(count, 0)
        np.testing.assert_array_equal(filled, light)

    def test_prop_wholly_inside_is_unchanged(self):
        light = np.ones((2, 3))
        filled, count = P.fill_enclosed(light, np.zeros((2, 3)), np.array([0, 1]),
                                        np.array([True, True]))
        self.assertEqual(count, 0)


class PakTest(unittest.TestCase):
    def test_replace_pak_keeps_other_lumps(self):
        source = bsp_with_pak(pak({"a.txt": b"x"}))
        out = P.replace_pak(source, pak({"a.txt": b"x", "sp_0.vhv": b"new"}))
        self.assertEqual(out[8 + 64 * 16 + 4:8 + 64 * 16 + 8], b"ABCD")
        bsp = legacy_bsp.LegacyBsp(out)
        self.assertEqual(bsp.pakfile().read("sp_0.vhv"), b"new")

    def test_identity_accepts_only_colour_mesh_changes(self):
        base = pak({"materials/a.vmt": b"1", "sp_0.vhv": b"old", "sp_hdr_0.vhv": b"old"})
        relit = pak({"materials/a.vmt": b"1", "sp_0.vhv": b"new", "sp_hdr_0.vhv": b"new",
                     "sp_hdr_1.vhv": b"added"})
        self.assertTrue(gameplay_identity.pak_differs_only_in_colour_meshes(base, relit))
        # Seeded: another entry changed.
        changed = pak({"materials/a.vmt": b"2", "sp_0.vhv": b"new"})
        self.assertFalse(gameplay_identity.pak_differs_only_in_colour_meshes(base, changed))
        dropped = pak({"sp_0.vhv": b"new"})
        self.assertFalse(gameplay_identity.pak_differs_only_in_colour_meshes(base, dropped))


if __name__ == "__main__":
    unittest.main()
