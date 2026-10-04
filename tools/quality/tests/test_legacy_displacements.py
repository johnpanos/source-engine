"""Compiled displacement reading and finite surface-material composition."""

import math
import struct
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import legacy_bsp as bsp
import legacy_surface_textures as textures
from test_legacy_relight import legacy_map


def fixture(power=2, start_corner=0, removed=()):
    corners = np.array(((0, 0, 0), (0, 64, 0), (64, 64, 0), (64, 0, 0)), float)
    count = ((1 << power) + 1) ** 2
    info = bytearray(bsp.DISPINFO_BYTES)
    struct.pack_into("<3f3i", info, 0, *corners[start_corner], 0, 0, power)
    struct.pack_into("<H", info, 36, 7)
    verts = np.zeros((count, 5), dtype="<f4")
    verts[:, 2] = 1
    verts[:, 4] = np.linspace(0, 255, count)
    verts[count // 2, 3] = 8
    tags = np.ones(2 * (1 << power) ** 2, dtype="<u2")
    tags[list(removed)] |= bsp.DISPTRI_TAG_REMOVE
    data = legacy_map({bsp.LUMP_DISPINFO: (0, info), bsp.LUMP_DISP_VERTS: (0, verts.tobytes()),
                       bsp.LUMP_DISP_TRIS: (0, tags.tobytes())})
    face = {"index": 7, "dispinfo": 0, "points": corners, "plane_normal": np.array((0, 0, 1))}
    return bsp.LegacyBsp(data), face


class DisplacementReaderTests(unittest.TestCase):
    def test_full_grid_powers_start_rotation_deformation_and_fronts(self):
        for power in (2, 3, 4):
            for corner in range(4):
                with self.subTest(power=power, corner=corner):
                    source, face = fixture(power, corner)
                    result = source.displacement(face)
                    count = ((1 << power) + 1) ** 2
                    self.assertEqual(len(result["points"]), count)
                    self.assertEqual(len(result["triangles"]), 2 * (1 << power) ** 2)
                    np.testing.assert_array_equal(result["points"][0], face["points"][corner])
                    self.assertEqual(result["points"][count // 2, 2], 8)
                    self.assertEqual(result["flat_points"][count // 2, 2], 0)
                    self.assertEqual((result["alpha"][0], result["alpha"][-1]), (0, 1))
                    tri = result["points"][result["triangles"]]
                    fronts = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
                    self.assertTrue((fronts[:, 2] > 0).all())
                    np.testing.assert_allclose(np.linalg.norm(result["normals"], axis=1), 1)
                    self.assertTrue((result["normals"][:, 2] > 0).all())
                    self.assertGreater(np.linalg.norm(result["normals"][:, :2]), 0)

    def test_triangle_diagonals_alternate_in_compiled_tag_order(self):
        actual = bsp.displacement_triangles(2)
        # Independent first row, then the first cell in the next row (side 5).
        np.testing.assert_array_equal(actual[:4], ((0, 5, 6), (0, 6, 1),
                                                   (1, 6, 2), (2, 6, 7)))
        np.testing.assert_array_equal(actual[8:10], ((5, 10, 6), (6, 10, 11)))

    def test_removed_triangles_remain_holes(self):
        source, face = fixture(removed=(0, 17))
        result = source.displacement(face)
        self.assertEqual(len(result["triangles"]), 30)
        self.assertEqual(result["removed_triangles"], 2)
        self.assertFalse(any(set(ids) == {0, 5, 6} for ids in result["triangles"]))

    def test_malformed_ranges_parent_power_and_nonfinite_vertices_fail(self):
        source, face = fixture()
        for lump, offset, fmt, value in ((26, 12, "i", -1), (26, 16, "i", 200),
                                         (26, 20, "i", 12), (26, 36, "H", 8),
                                         (33, 0, "f", math.nan), (33, 16, "f", 300)):
            changed = bytearray(source.data)
            struct.pack_into("<" + fmt, changed, source.lumps[lump][0] + offset, value)
            with self.subTest(lump=lump, offset=offset), self.assertRaises(ValueError):
                bsp.LegacyBsp(changed).displacement(face)
        with self.assertRaisesRegex(ValueError, "missing displacement"):
            source.displacement(dict(face, dispinfo=100))
        with self.assertRaisesRegex(ValueError, "start"):
            source.displacement(dict(face, points=face["points"] + 20))


class SurfaceTextureTests(unittest.TestCase):
    def test_skewed_paint_quad_inverse_and_outside_coverage(self):
        quad = np.array(((0, 0), (-1, 2), (3, 3), (2, 0)), float)
        expected = np.array(((0.2, 0.3), (0.7, 0.8), (1.2, 0.5)))
        s, t = expected[:, 0, None], expected[:, 1, None]
        points = ((1 - s) * (1 - t) * quad[0] + (1 - s) * t * quad[1] +
                  s * t * quad[2] + s * (1 - t) * quad[3])
        actual, covered = textures.quad_coordinates(points, quad)
        np.testing.assert_allclose(actual, expected)
        np.testing.assert_array_equal(covered, (True, True, False))
        with self.assertRaisesRegex(ValueError, "degenerate"):
            textures.quad_coordinates(points, np.zeros((4, 2)))

    def test_static_paint_alpha_and_mod2x_have_distinct_neutral_values(self):
        base = np.array(((0.2, 0.4, 0.6, 1.), (0.2, 0.4, 0.6, 1.)))
        points = np.array(((0.5, 0.5, 0), (2, 2, 0)))
        paint = {"origin": np.zeros(3), "basis": np.array(((1, 0, 0), (0, 1, 0))),
                 "quad": np.array(((0, 0), (0, 1), (1, 1), (1, 0))),
                 "limits": (0, 1, 0, 1), "image": np.array([[(0.0, 1, 0, 0.5)]]),
                 "matrix": np.eye(2), "offset": np.zeros(2), "shader": "lightmappedgeneric"}
        result = textures.paint_color(base, points, [paint])
        np.testing.assert_allclose(result[0], (0.1, 0.7, 0.3, 1))
        np.testing.assert_array_equal(result[1], base[1])
        paint.update(shader="decalmodulate", image=np.array([[(0.5, 0.5, 0.5, 0.1)]]))
        np.testing.assert_allclose(textures.paint_color(base, points, [paint]), base)

    def test_static_brush_selection_rejects_runtime_states_case_insensitively(self):
        import legacy_bsp_scene as scene
        entity = {"classname": "func_brush", "model": "*1"}
        self.assertTrue(scene.static_brush_entity(entity))
        for change in ({"StartDisabled": "1"}, {"targetname": "door"}, {"parentname": "train"},
                       {"classname": "func_door"}, {"OnUse": "wall,Disable"},
                       {"renderamt": "0"}, {"rendermode": "2"}):
            with self.subTest(change=change):
                self.assertFalse(scene.static_brush_entity(dict(entity, **change)))

    def test_overlay_lump_decodes_projection_order_and_validates_indices(self):
        overlay = bytearray(352)
        struct.pack_into("<ihHi", overlay, 0, 0, 0, 1 | (2 << 14), 0)
        struct.pack_into("<4f", overlay, 264, 0, 1, 0, 1)
        struct.pack_into("<12f", overlay, 280, -1, -1, 1, -1, 1, 0, 1, 1, 0, 1, -1, 1)
        struct.pack_into("<6f", overlay, 328, 10, 20, 30, 0, 0, 1)
        data = legacy_map({45: (0, overlay), 6: (0, bytes(72)), 7: (0, bytes(bsp.FACE.size))})
        result = bsp.LegacyBsp(data).overlays()[0]
        self.assertEqual(result["order"], 2)
        np.testing.assert_array_equal(result["basis"], ((1, 0, 0), (0, -1, 0)))
        np.testing.assert_array_equal(result["origin"], (10, 20, 30))
        for offset, fmt, value in ((6, "H", 65), (8, "i", 5), (4, "h", -1), (280, "f", math.nan)):
            changed = bytearray(data)
            struct.pack_into("<" + fmt, changed, bsp.LegacyBsp(data).lumps[45][0] + offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                bsp.LegacyBsp(changed).overlays()

    def test_native_centered_uniform_scale_and_rotation(self):
        matrix, offset = textures.transform("center 0.5 0.5 scale 2 rotate 90 translate 1 2")
        np.testing.assert_allclose(matrix @ (0.5, 0.5) + offset, (1.5, 2.5))
        np.testing.assert_allclose(matrix @ (1, 0.5) + offset, (1.5, 3.5))
        for text in ("scale 2", "center nan 0 scale 1 1 rotate 0 translate 0 0",
                     textures.IDENTITY + " junk"):
            with self.assertRaises(ValueError):
                textures.transform(text)

    def test_modulation_endpoints_midpoint_and_zero_width(self):
        self.assertEqual(textures.modulation(np.array(0.5), np.array((0, 0.5))), 1)
        np.testing.assert_allclose(textures.modulation(np.array((0, 0.5, 1)),
                                                       np.array((0.25, 0.5))), (0, 0.5, 1))

    def test_actual_triangle_interpolation_not_bilinear_quad(self):
        uv = np.array(((0, 0), (1, 0), (1, 1), (0, 1)), float)
        weights, covered = textures.rasterize(uv, ((0, 1, 2), (0, 2, 3)),
                                             np.array((0, 0, 1, 0)), (0, 0), (1, 1), (4, 4))
        self.assertTrue(covered.all())
        np.testing.assert_allclose(weights[0, 0], 0.125)  # bilerp would give 0.015625
        np.testing.assert_allclose(weights[-1, -1], 0.875)

    def test_linear_color_blend_normal_layer_and_independent_transforms(self):
        uv = np.array(((0, 0), (1, 0), (1, 1), (0, 1)), float)
        red = np.tile((1., 0, 0, 1), (4, 4, 1))
        blue = np.tile((0., 0, 1, 1), (4, 4, 1))
        normals = np.tile((0., 0, 1), (4, 4, 1))
        layers = {"base": (red, np.eye(2), np.zeros(2)),
                  "base2": (blue, np.eye(2), np.zeros(2)),
                  "normal": (normals, np.eye(2), np.zeros(2)),
                  "normal2": (np.tile((0.6, 0, 0.8), (4, 4, 1)), np.eye(2), np.zeros(2))}
        with tempfile.TemporaryDirectory() as directory:
            record = {"params": {}}
            mapped, receipt = textures.compose(record, uv, ((0, 1, 2), (0, 2, 3)),
                                                np.full(4, 0.5), directory, "blend", layers)
            np.testing.assert_allclose(mapped, uv)
            self.assertEqual(receipt["size"], [4, 4])
            with Image.open(record["base"]["file"]) as image:
                self.assertEqual(image.getpixel((1, 1)), (188, 0, 188, 255))
            with Image.open(record["normal"]["file"]) as image:
                expected = np.array((0.3, 0, 0.9)) / math.sqrt(0.9)
                np.testing.assert_allclose(np.array(image.getpixel((1, 1))) / 255 * 2 - 1,
                                           expected, atol=1 / 127)
            # A layer's scaled lookup also raises the generated tile density.
            layers["base2"] = blue, np.eye(2) * 2, np.array((0.5, 0))
            _, receipt = textures.compose(record, uv, ((0, 1, 2), (0, 2, 3)),
                                          np.full(4, 1), directory, "scaled", layers)
            self.assertEqual(receipt["size"], [8, 8])


if __name__ == "__main__":
    unittest.main()
