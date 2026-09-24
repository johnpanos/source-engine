#!/usr/bin/env python3
"""Small independent WMSH writer/reader contract and malformed-input probes."""

from pathlib import Path
import struct
import sys
import unittest


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from worldstage_mesh_compare import read_payload  # noqa: E402
from worldstage_mesh_pack import face_triangles, tangent_frame, write_payload  # noqa: E402


class WorldMeshPayloadTests(unittest.TestCase):
    def make_payload(self):
        points = [(0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0)]
        uv = [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0)]
        normal = (0.0, 0.0, 1.0)
        tangent, sign = tangent_frame(points, uv, normal)
        faces = {7: {"material": "test/é", "triangles": [
            (points, uv, uv, normal, tangent, sign)]}}
        return write_payload(faces, [{7}, set()])[0]

    def test_round_trip_unicode_material_and_leaf_references(self):
        decoded = read_payload(self.make_payload())
        self.assertEqual(decoded["materials"], ["test/é"])
        self.assertEqual(decoded["faces"], (7,))
        self.assertEqual(decoded["leaves"], [(0,), ()])
        self.assertEqual(decoded["counts"]["triangles"], 1)

    def test_truncation_and_bad_sections_are_rejected(self):
        payload = self.make_payload()
        cases = [payload[:-1], b"BAD!" + payload[4:]]
        changed = bytearray(payload)
        struct.pack_into("<Q", changed, 56, 144)
        cases.append(bytes(changed))
        for case in cases:
            with self.subTest(size=len(case)):
                with self.assertRaises(ValueError):
                    read_payload(case)

    def test_bad_index_and_leaf_reference_are_rejected(self):
        payload = self.make_payload()
        for offset_field, bad_value in ((64, 9), (104, 9)):
            changed = bytearray(payload)
            section = struct.unpack_from("<Q", changed, offset_field)[0]
            struct.pack_into("<I", changed, section, bad_value)
            with self.subTest(section=offset_field):
                with self.assertRaises(ValueError):
                    read_payload(bytes(changed))


class FaceTriangleTests(unittest.TestCase):
    # A unit square with a T-junction point (0.5, 0) on its first edge. Fanned
    # from corner 0, the triangle (0, 1, 2) lies on that edge and has no area.
    POINTS = [(0.0, 0.0, 0.0), (0.5, 0.0, 0.0), (1.0, 0.0, 0.0), (1.0, 1.0, 0.0),
              (0.0, 1.0, 0.0)]
    NORMAL = (0.0, 0.0, 1.0)

    def triangles(self, indices, uv_of=lambda point: point[:2]):
        uv = [uv_of(self.POINTS[index]) for index in indices]
        return face_triangles(self.POINTS, indices, [self.NORMAL] * (len(indices) // 3),
                              uv, uv)

    def test_collinear_fan_triangle_takes_the_face_frame(self):
        result = self.triangles([0, 1, 2, 0, 2, 3, 0, 3, 4])
        frames = {triangle[4:] for triangle in result}
        self.assertEqual(len(result), 3)
        self.assertEqual(frames, {((1.0, 0.0, 0.0), 1)})

    def test_collapsed_uv_on_a_real_triangle_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "degenerate material UV"):
            self.triangles([0, 2, 3], uv_of=lambda point: (point[0], 0.0))

    def test_face_without_area_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "no triangle with area"):
            self.triangles([0, 1, 2])


if __name__ == "__main__":
    unittest.main()
