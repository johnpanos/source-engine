"""Legacy-map relight: VTF decoding, BSP reading, light conversion, world-light
filtering and the gameplay-identity oracle, on synthetic inputs (no game
content, no pxr, no Blender)."""

import math
import struct
import sys
import unittest
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))

import bsp2_reader as br  # noqa: E402
import bsp_worldlights  # noqa: E402
import legacy_bsp  # noqa: E402
import legacy_bsp_relight  # noqa: E402
import legacy_bsp_scene as scene  # noqa: E402
import vtf_decode  # noqa: E402


def vtf(fmt, width, height, payload, minor=2, mips=1):
    """A VTF 7.<minor> (< 3) file: header, no thumbnail, then `payload`."""
    header = bytearray(80)
    header[:4] = b"VTF\0"
    struct.pack_into("<3I", header, 4, 7, minor, 80)
    struct.pack_into("<HHIHH", header, 16, width, height, 0, 1, 0)
    struct.pack_into("<3f", header, 32, 0.5, 0.5, 0.5)
    struct.pack_into("<f", header, 48, 1.0)
    struct.pack_into("<iBiBB", header, 52, fmt, mips, -1, 0, 0)
    struct.pack_into("<H", header, 63, 1)
    return bytes(header) + payload


def rgb565(r, g, b):
    return (r >> 3) << 11 | (g >> 2) << 5 | (b >> 3)


def legacy_map(lumps, version=20):
    """A legacy VBSP with the given {index: (version, payload)} lumps."""
    header = bytearray(br.LEGACY_HEADER_SIZE)
    header[:4] = br.LEGACY_IDENT
    struct.pack_into("<i", header, 4, version)
    body = bytearray(header)
    for index, (lump_version, payload) in sorted(lumps.items()):
        body.extend(b"\0" * (-len(body) % 4))
        struct.pack_into("<iiii", body, 8 + 16 * index, len(body), len(payload),
                         lump_version, 0)
        body.extend(payload)
    struct.pack_into("<i", body, 8 + 16 * br.LEGACY_LUMPS, 1)
    return bytes(body)


def world_light(style, intensity=(1.0, 1.0, 1.0), kind=legacy_bsp.EMIT_POINT):
    return legacy_bsp.WORLDLIGHT_V0.pack(0, 0, 0, *intensity, 0, 0, 1, 0, kind, style,
                                         0, 0, 0, 0, 0, 0, 1, 0, 0, 0)


class VtfDecodeTests(unittest.TestCase):
    def test_bgr888_channels_and_orientation(self):
        pixels = bytes([0, 0, 255, 0, 255, 0, 255, 0, 0, 10, 20, 30])  # BGR, 2x2
        image, info = vtf_decode.decode(vtf(3, 2, 2, pixels))
        self.assertEqual(image.shape, (2, 2, 4))
        self.assertEqual(image[0, 0].tolist(), [255, 0, 0, 255])
        self.assertEqual(image[0, 1].tolist(), [0, 255, 0, 255])
        self.assertEqual(image[1, 1].tolist(), [30, 20, 10, 255])
        self.assertEqual(info["reflectivity"], (0.5, 0.5, 0.5))

    def test_dxt1_block_palette_and_indices(self):
        c0, c1 = rgb565(248, 0, 0), rgb565(0, 0, 248)
        indices = 0
        for texel in range(16):
            indices |= (texel % 4) << (2 * texel)
        block = struct.pack("<HHI", c0, c1, indices)
        image, _ = vtf_decode.decode(vtf(13, 4, 4, block))
        self.assertEqual(image[0, 0, :3].tolist(), [255, 0, 0])
        self.assertEqual(image[0, 1, :3].tolist(), [0, 0, 255])
        self.assertEqual(image[0, 2, 0], 170)  # 2/3 c0 + 1/3 c1
        self.assertEqual(image[0, 3, 2], 170)
        self.assertTrue((image[..., 3] == 255).all())

    def test_dxt5_interpolated_alpha(self):
        alpha = bytes([255, 0]) + (0b001).to_bytes(6, "little")  # texel 0 index 1 -> a1
        colour = struct.pack("<HHI", rgb565(248, 248, 248), rgb565(248, 248, 248), 0)
        image, _ = vtf_decode.decode(vtf(15, 4, 4, alpha + colour))
        self.assertEqual(image[0, 0, 3], 0)
        self.assertEqual(image[0, 1, 3], 255)

    def test_mip_selection_skips_the_smaller_mips(self):
        small = bytes([1, 2, 3])  # 1x1 BGR, stored first
        large = bytes([9, 9, 9]) * 4
        image, _ = vtf_decode.decode(vtf(3, 2, 2, small + large, mips=2))
        self.assertEqual(image.shape[:2], (2, 2))
        self.assertEqual(image[0, 0, :3].tolist(), [9, 9, 9])
        tiny, _ = vtf_decode.decode(vtf(3, 2, 2, small + large, mips=2), mip=1)
        self.assertEqual(tiny[0, 0, :3].tolist(), [3, 2, 1])

    def test_truncated_and_unsupported_files_are_rejected(self):
        with self.assertRaises(ValueError):
            vtf_decode.decode(vtf(3, 2, 2, bytes(11)))
        with self.assertRaises(ValueError):
            vtf_decode.decode(vtf(24, 2, 2, bytes(64)))  # RGBA16161616F
        with self.assertRaises(ValueError):
            vtf_decode.decode(b"NOPE" + bytes(100))


class GeometryTests(unittest.TestCase):
    def test_clipping_a_plane_by_a_cube_gives_its_face(self):
        planes = [(np.array(n, float), 16.0) for n in
                  ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))]
        winding = legacy_bsp.base_winding(*planes[4])
        for normal, dist in planes[:4]:
            winding = legacy_bsp.clip_winding(winding, normal, dist)
        self.assertEqual(len(winding), 4)
        self.assertAlmostEqual(legacy_bsp.polygon_area(winding), 32 * 32, places=3)
        self.assertTrue(np.allclose(winding[:, 2], 16.0))
        self.assertIsNone(legacy_bsp.clip_winding(winding, np.array((0.0, 0.0, 1.0)), 0.0))

    def test_point_in_polygon_for_either_winding(self):
        square = np.array([(0, 0, 0), (10, 0, 0), (10, 10, 0), (0, 10, 0)], float)
        normal = np.array((0.0, 0.0, 1.0))
        for polygon in (square, square[::-1]):
            self.assertTrue(scene.point_in_polygon(np.array((5.0, 5.0, 0.0)), polygon, normal,
                                                   0.5))
            self.assertTrue(scene.point_in_polygon(np.array((10.3, 5.0, 0.0)), polygon,
                                                   normal, 0.5))
            self.assertFalse(scene.point_in_polygon(np.array((11.0, 5.0, 0.0)), polygon,
                                                    normal, 0.5))

    def test_face_st_flips_v_to_usd(self):
        texinfo = {"vectors": np.array([(1.0, 0, 0, 0), (0, -1.0, 0, 0)])}
        st = scene.face_st(np.array([(0.0, 0, 0), (64.0, -32.0, 0)]), texinfo,
                           {"width": 64, "height": 64})
        self.assertEqual(st.tolist(), [[0.0, 1.0], [1.0, 0.5]])


class MaterialTests(unittest.TestCase):
    class Resolver:
        def __init__(self, files):
            self.files = files

        def read(self, relative):
            data = self.files.get(relative)
            return (data.encode(), "test:" + relative) if data else (None, None)

    class Bsp:
        def pakfile(self):
            return None

    def test_keyvalues_comments_nesting_and_bare_tokens(self):
        shader, params = scene.parse_keyvalues(
            '// header\n"LightmappedGeneric"\n{\n "$basetexture" "Concrete/Wall" // tail\n'
            ' $surfaceprop concrete\n "Proxies" { "Sine" { "resultVar" "$alpha" } }\n}\n')
        self.assertEqual(shader, "lightmappedgeneric")
        self.assertEqual(params["$basetexture"], "Concrete/Wall")
        self.assertEqual(params["$surfaceprop"], "concrete")
        self.assertIn("sine", params["proxies"])

    def test_cubemap_patch_renders_as_its_base(self):
        resolver = self.Resolver({
            "materials/concrete/wall.vmt": '"LightmappedGeneric" { "$basetexture" "c/w" }',
            "materials/maps/m/concrete/wall_1_2_3.vmt":
                '"patch" { "include" "materials/concrete/wall.vmt" '
                '"insert" { "$envmap" "maps/m/c_1_2_3" } }',
            "materials/maps/m/tinted.vmt":
                '"patch" { "include" "materials/concrete/wall.vmt" '
                '"replace" { "$basetexture" "c/tinted" } }'})
        materials = scene.Materials(self.Bsp(), resolver)
        shader, params, source = materials.vmt("maps/m/concrete/wall_1_2_3")
        self.assertEqual(shader, "lightmappedgeneric")
        self.assertEqual(params[scene.BASE_KEY], "concrete/wall")
        self.assertEqual(params["$envmap"], "maps/m/c_1_2_3")
        _, tinted, _ = materials.vmt("maps/m/tinted")
        self.assertEqual(tinted[scene.BASE_KEY], "maps/m/tinted")  # a real change keeps its name
        self.assertEqual(tinted["$basetexture"], "c/tinted")
        self.assertEqual(materials.vmt("missing/material")[0], None)


class LightConversionTests(unittest.TestCase):
    def light(self, attenuation):
        return {"index": 0, "attenuation": attenuation}

    def test_inverse_square_light_is_unchanged(self):
        scale, approximated = scene.falloff_match(self.light((0.0, 0.0, 1.0)))
        self.assertEqual((scale, approximated), (1.0, False))

    def test_constant_falloff_matches_vrad_at_the_normalization_distance(self):
        attenuation = (200000.0, 0.0, 1.0)
        scale, approximated = scene.falloff_match(self.light(attenuation))
        self.assertTrue(approximated)
        d = scene.NORMALIZE_DISTANCE
        vrad = 1.0 / (attenuation[0] + d * d)
        self.assertAlmostEqual(scale / (d * d), vrad)

    def test_a_light_without_falloff_is_rejected(self):
        with self.assertRaises(ValueError):
            scene.falloff_match(self.light((0.0, 0.0, 0.0)))

    def test_only_switchable_styles_start_dark(self):
        lights = [{"index": i, "style": style} for i, style in enumerate((0, 1, 32, 33, 33))]
        owners = {0: {"spawnflags": "1"}, 1: {"spawnflags": "1"}, 2: {"spawnflags": "0"},
                  3: {}, 4: {"spawnflags": "1"}}
        # Preset style 1 ignores the flag; 33 is dark because one of its lights is.
        self.assertEqual(scene.starting_dark_styles(lights, owners), {33})

    def test_surface_lights_sum_onto_their_face(self):
        face = {"index": 7, "plane_normal": np.array((0.0, 0.0, -1.0)),
                "points": np.array([(0, 0, 100), (0, 32, 100), (32, 32, 100), (32, 0, 100)],
                                   float)}
        lights = [{"index": i, "origin": origin, "normal": (0.0, 0.0, -1.0),
                   "intensity": (2.0, 2.0, 2.0)}
                  for i, origin in enumerate(((8, 8, 100), (24, 24, 101), (8, 8, 300),
                                              (64, 64, 100)))]
        sums, unmatched = scene.match_surface_lights(lights, [face])
        self.assertEqual(sums[7].tolist(), [4.0, 4.0, 4.0])
        self.assertEqual(unmatched, [2, 3])
        # Radiance: pi x summed intensity / area (vrad's cos cos / d^2 kernel).
        self.assertAlmostEqual(math.pi * sums[7][0] / legacy_bsp.polygon_area(face["points"]),
                               math.pi * 4.0 / 1024.0)
        # A light facing the other way is not this face's.
        flipped = [dict(lights[0], normal=(0.0, 0.0, 1.0))]
        self.assertEqual(scene.match_surface_lights(flipped, [face])[1], [0])


class WorldLightTests(unittest.TestCase):
    def test_keep_only_removes_baked_lights_from_both_lumps(self):
        records = world_light(0) + world_light(32) + world_light(0, kind=legacy_bsp.EMIT_SURFACE)
        data = legacy_map({15: (0, records), 54: (0, records), 1: (0, b"planes!!")})
        out, removed = bsp_worldlights.keep_only(data, {32})
        self.assertEqual(removed, {15: 2, 54: 2})
        bsp = legacy_bsp.LegacyBsp(out)
        lights, lump = bsp.world_lights()
        self.assertEqual(lump, 54)
        self.assertEqual([(l["style"], l["type"]) for l in lights], [(32, "point")])
        self.assertEqual(bsp.lump(1), b"planes!!")

    def test_record_size_follows_the_lump_length(self):
        # Our compile tools write version-1 records under lump version 0.
        record = legacy_bsp.WORLDLIGHT_V1.pack(1, 2, 3, 4, 5, 6, 0, 0, 1, 0, 0, 0, 7,
                                               legacy_bsp.EMIT_POINT, 33, 0, 0, 0, 0, 0, 0,
                                               1, 0, 0, 0)
        lights, _ = legacy_bsp.LegacyBsp(legacy_map({15: (0, record * 3)})).world_lights()
        self.assertEqual([(l["style"], l["origin"]) for l in lights],
                         [(33, (1.0, 2.0, 3.0))] * 3)


class GameplayIdentityTests(unittest.TestCase):
    def source(self, changes=None):
        lumps = {0: (0, b'{ "classname" "worldspawn" }\0'), 1: (0, b"P" * 40),
                 15: (0, world_light(0)), 35: (0, b"G" * 24), 56: (1, b"A" * 28)}
        lumps.update(changes or {})
        return legacy_map(lumps)

    def test_unchanged_map_passes(self):
        data = self.source()
        result = legacy_bsp_relight.gameplay_identity(data, br.convert_legacy(data))
        self.assertEqual(result["status"], "pass")
        self.assertEqual(result["relit_lumps"], [])

    def test_relit_lighting_lumps_are_allowed(self):
        data = self.source()
        relit, _ = bsp_worldlights.keep_only(data, set())
        result = legacy_bsp_relight.gameplay_identity(data, br.convert_legacy(relit))
        self.assertEqual(result["status"], "pass")
        self.assertEqual([r["lump"] for r in result["relit_lumps"]], [15])

    def test_a_changed_gameplay_lump_fails(self):
        data = self.source()
        for index, payload in ((0, b'{ "classname" "worldspawn" "x" "1" }\0'),
                               (35, b"H" * 24), (1, b"P" * 39 + b"Q")):
            with self.subTest(lump=index):
                changed = self.source({index: (0, payload)})
                result = legacy_bsp_relight.gameplay_identity(data, br.convert_legacy(changed))
                self.assertEqual(result["status"], "fail")
                self.assertEqual([d["lump"] for d in result["differing_lumps"]], [index])


if __name__ == "__main__":
    unittest.main()
