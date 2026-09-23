import struct
import sys
import unittest
from pathlib import Path


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from legacy_lighting_audit import audit_lighting_lumps


def fixture():
    texinfo = bytearray(72)
    struct.pack_into("<i", texinfo, 64, 0x0800)  # SURF_BUMPLIGHT
    face = bytearray(56)
    struct.pack_into("<h", face, 10, 0)
    face[16:20] = bytes((0, 32, 255, 255))
    struct.pack_into("<i", face, 20, 8)  # Two average RGBExp32 values.
    struct.pack_into("<ii", face, 36, 1, 0)  # Two luxels.
    lighting = bytes(range(72))  # 8 average + 2 luxels × 2 styles × 4 bases × 4 bytes.
    return face, texinfo, lighting


class LegacyLightingAuditTest(unittest.TestCase):
    def test_bumped_multistyle_layout(self):
        face, texinfo, lighting = fixture()
        result = audit_lighting_lumps(face, texinfo, lighting)
        self.assertEqual(result["lit_faces"], 1)
        self.assertEqual(result["bumped_faces"], 1)
        self.assertEqual(result["styles"], [0, 32])

    def test_bad_offsets_and_sizes_fail(self):
        face, texinfo, lighting = fixture()
        for offset in (0, 4, 12):
            with self.subTest(offset=offset):
                changed = bytearray(face)
                struct.pack_into("<i", changed, 20, offset)
                with self.assertRaisesRegex(ValueError, "noncontiguous"):
                    audit_lighting_lumps(changed, texinfo, lighting)
        with self.assertRaisesRegex(ValueError, "extends past"):
            audit_lighting_lumps(face, texinfo, lighting[:-4])

    def test_missing_bump_basis_and_style_fail(self):
        face, texinfo, lighting = fixture()
        changed_texinfo = bytearray(texinfo)
        struct.pack_into("<i", changed_texinfo, 64, 0)
        with self.assertRaisesRegex(ValueError, "unreferenced"):
            audit_lighting_lumps(face, changed_texinfo, lighting)
        changed_face = bytearray(face)
        changed_face[17] = 255
        with self.assertRaisesRegex(ValueError, "noncontiguous"):
            audit_lighting_lumps(changed_face, texinfo, lighting)

    def test_style_after_sentinel_fails(self):
        face, texinfo, lighting = fixture()
        face[16:20] = bytes((0, 255, 32, 255))
        with self.assertRaisesRegex(ValueError, "after the sentinel"):
            audit_lighting_lumps(face, texinfo, lighting)


if __name__ == "__main__":
    unittest.main()
