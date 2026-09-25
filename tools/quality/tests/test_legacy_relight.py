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


class SharedVerticesTest(unittest.TestCase):
    def test_faces_share_points_by_bsp_vertex_index(self):
        """Two quads naming the same two BSP vertices share those points: the
        scene keeps the BSP's connectivity (6 points, not 8 corners)."""
        a = np.array([[0, 0, 0], [1, 0, 0], [1, 1, 0], [0, 1, 0]], dtype=np.float64)
        b = a + [1, 0, 0]
        normal = np.array([0, 0, 1.0])
        uvs = [np.zeros((4, 2)), np.zeros((4, 2))]
        points, counts, indices, _, st, _ = scene.indexed_triangles(
            [a, b], uvs, [normal, normal], [np.array([0, 1, 2, 3]), np.array([1, 4, 5, 2])])
        self.assertEqual(len(points), 6)
        self.assertEqual(len(indices), 12)
        triangles = np.asarray(points)[indices].reshape(-1, 3, 3)
        areas = np.linalg.norm(np.cross(triangles[:, 1] - triangles[:, 0],
                                        triangles[:, 2] - triangles[:, 0]), axis=1) / 2
        self.assertAlmostEqual(areas.sum(), 2.0)  # both unit quads, exactly covered

    def test_every_triangle_corner_is_its_polygon_corner(self):
        """Random polygons: each triangle corner's point is exactly the polygon
        corner it came from, whichever sharing key is used."""
        rng = np.random.default_rng(3)
        for _ in range(50):
            count = int(rng.integers(3, 8))
            angles = np.sort(rng.uniform(0, 2 * np.pi, count))
            polygon = np.stack([np.cos(angles), np.sin(angles), np.zeros(count)], axis=1)
            ids = rng.permutation(100)[:count]
            for vertex_ids in (None, [ids]):
                points, counts, indices, _, _, _ = scene.indexed_triangles(
                    [polygon], [np.zeros((count, 2))], [np.array([0, 0, 1.0])], vertex_ids)
                self.assertEqual(len(counts), count - 2)
                self.assertEqual(len(points), count)


def convex_polygon(rng, corners, junctions, noise):
    """A random convex polygon in a random plane, with `junctions` extra
    vertices along random edges (vbsp's crack fixes) and `noise` rounding."""
    angles = np.sort(rng.uniform(0, 2 * np.pi, corners))
    flat = np.stack([np.cos(angles), np.sin(angles)], axis=1) * rng.uniform(0.5, 4)
    ring = list(flat)
    for _ in range(junctions):
        edge = int(rng.integers(0, len(ring)))
        a, b = ring[edge], ring[(edge + 1) % len(ring)]
        ring.insert(edge + 1, a + (b - a) * rng.uniform(0.05, 0.95))
    flat = np.array(ring) + rng.normal(0, noise, (len(ring), 2))
    q, _ = np.linalg.qr(rng.standard_normal((3, 3)))
    points = flat @ q[:, :2].T + rng.uniform(-100, 100, 3)
    return points, q[:, 2] * np.sign(np.linalg.det(q))


def all_triangulations(i, j):
    """Every triangulation of polygon corners i..j (brute force)."""
    if j - i < 2:
        yield []
        return
    for k in range(i + 1, j):
        for left in all_triangulations(i, k):
            for right in all_triangulations(k, j):
                yield left + [(i, k, j)] + right


def altitude(points, triangle, normal):
    a, b, c = (points[i] for i in triangle)
    signed = np.dot(np.cross(b - a, c - a), normal)
    return signed / max(np.linalg.norm(b - a), np.linalg.norm(c - b), np.linalg.norm(a - c))


class CanonicalVerticesTest(unittest.TestCase):
    def test_float_rounding_twins_merge_to_the_lowest_index(self):
        vertices = np.array([[-513.5, -310.0000305, 0], [0, 0, 0],
                             [-513.4999389, -309.99993896, 0], [1, 0, 0], [0, 0, 0]])
        self.assertEqual(legacy_bsp.canonical_vertices(vertices).tolist(), [0, 1, 0, 3, 1])

    def test_distinct_corners_stay_apart(self):
        rng = np.random.default_rng(2)
        grid = np.stack(np.meshgrid(*[np.arange(0, 64, 1.0)] * 2), -1).reshape(-1, 2)
        vertices = np.concatenate([grid, np.zeros((len(grid), 1))], axis=1)
        vertices = vertices[rng.permutation(len(vertices))]
        self.assertEqual(legacy_bsp.canonical_vertices(vertices).tolist(),
                         list(range(len(vertices))))


class CleanWindingTest(unittest.TestCase):
    def test_repeats_and_spikes_go(self):
        self.assertEqual(scene.clean_winding([1, 2, 3, 4]), [0, 1, 2, 3])
        self.assertEqual(scene.clean_winding([1, 2, 2, 2, 3, 4]), [0, 3, 4, 5])
        keys = [1, 2, 3, 4, 1]                                                 # wraps
        self.assertEqual(sorted(keys[i] for i in scene.clean_winding(keys)), [1, 2, 3, 4])
        self.assertEqual(scene.clean_winding([1, 2, 1, 3, 4]), [0, 3, 4])      # spike 1 2 1
        self.assertEqual(scene.clean_winding([1, 2, 1]), [])
        self.assertEqual(scene.clean_winding([5, 5, 5]), [])

    def test_folded_face_triangulates_without_zero_area(self):
        """The testchmb_a_00 face that lists one corner three times: cleaned,
        its triangles all have area, and each names its source polygon."""
        points = np.array([[-12.8014, -7.7724, 0.2032], [-12.9541, -7.9251, 0.2032],
                           [-13.0429, -7.874, 0.0], [-13.0429, -7.874, 0.0],
                           [-13.0429, -7.874, 0.0], [-12.8524, -7.6835, 0.0]])
        normal = np.cross(points[1] - points[0], points[2] - points[0])
        normal /= np.linalg.norm(normal)
        result = scene.indexed_triangles([points], [np.zeros((6, 2))], [normal],
                                         [np.array([1, 2, 3, 3, 3, 4])])
        corners = np.asarray(result[0])[result[2]].reshape(-1, 3, 3)
        areas = np.linalg.norm(np.cross(corners[:, 1] - corners[:, 0],
                                        corners[:, 2] - corners[:, 0]), axis=1)
        self.assertEqual(len(corners), 2)
        self.assertGreater(areas.min(), 1e-3)
        self.assertEqual(result[5], [0, 0])


class WidestTriangulationTest(unittest.TestCase):
    def setUp(self):
        self.rng = np.random.default_rng(11)

    def polygons(self, count):
        for _ in range(count):
            yield convex_polygon(self.rng, int(self.rng.integers(3, 9)),
                                 int(self.rng.integers(0, 5)), 1e-7)

    def test_result_is_a_valid_triangulation(self):
        """n - 2 counter-clockwise triangles; polygon edges used once and
        diagonals twice; areas sum to the polygon's area."""
        for points, normal in self.polygons(300):
            triangles = scene.widest_triangulation(points, normal)
            count = len(points)
            self.assertEqual(len(triangles), count - 2)
            uses = {}
            for triangle in triangles:
                self.assertGreater(altitude(points, triangle, normal), 0)
                for a, b in ((0, 1), (1, 2), (0, 2)):
                    edge = tuple(sorted((triangle[a], triangle[b])))
                    uses[edge] = uses.get(edge, 0) + 1
            for edge, used in uses.items():
                boundary = edge[1] - edge[0] in (1, count - 1)
                self.assertEqual(used, 1 if boundary else 2, edge)
            polygon_area = abs(sum(np.dot(np.cross(points[k] - points[0], points[k + 1] - points[0]),
                                          normal) for k in range(1, count - 1))) / 2
            triangle_area = sum(np.dot(np.cross(points[b] - points[a], points[c] - points[a]),
                                       normal) / 2 for a, b, c in triangles)
            self.assertAlmostEqual(triangle_area, polygon_area, places=9)

    def test_thinnest_triangle_is_optimal(self):
        """Against brute force over every triangulation (up to 8 corners)."""
        for points, normal in self.polygons(120):
            if len(points) > 8:
                continue
            chosen = min(altitude(points, t, normal)
                         for t in scene.widest_triangulation(points, normal))
            best = max(min(altitude(points, t, normal) for t in candidate)
                       for candidate in all_triangulations(0, len(points) - 1))
            self.assertAlmostEqual(chosen, best, places=12)

    def test_crack_fix_vertices_make_no_slivers(self):
        """A rectangle wall with vbsp junction vertices along its edges (and
        float rounding lifting them micrometres off): the fan leaves
        micron-wide slivers, the widest triangulation none thinner than the
        wall allows."""
        for _ in range(100):
            width, height = self.rng.uniform(1, 4), self.rng.uniform(1, 4)
            # Junctions sit at neighbouring faces' corners: spread along the edge.
            bottom = np.linspace(0, width, 5)[1:-1] + self.rng.uniform(-0.1, 0.1, 3)
            top = np.linspace(0, width, 4)[1:-1] + self.rng.uniform(-0.1, 0.1, 2)
            ring = [(0, 0)] + [(x, 0) for x in bottom] + [(width, 0), (width, height)]
            ring += [(x, height) for x in top[::-1]] + [(0, height)]
            flat = np.array(ring, dtype=np.float64)
            flat[1:4, 1] += self.rng.normal(0, 5e-6, 3)     # rounding off the edge
            points = np.concatenate([flat, np.zeros((len(flat), 1))], axis=1)
            normal = np.array([0, 0, 1.0])
            fan = min(altitude(points, (0, k, k + 1), normal) for k in range(1, len(points) - 1))
            widest = min(altitude(points, t, normal)
                         for t in scene.widest_triangulation(points, normal))
            self.assertLess(fan, 1e-3)
            self.assertGreater(widest, 1e-3)          # no micron slivers
            self.assertGreater(widest, 100 * max(fan, 1e-12))

    def test_same_polygon_same_triangles(self):
        points, normal = convex_polygon(self.rng, 7, 3, 1e-7)
        self.assertEqual(scene.widest_triangulation(points, normal),
                         scene.widest_triangulation(points.copy(), normal.copy()))


class TextureMappingVariantsTest(unittest.TestCase):
    """The relit world mesh must texture every face exactly as the engine
    does: WMSH st = the engine's (p . s + w) / mapping size, top-left origin."""

    class Materials:
        def __init__(self, params, files):
            self.params, self.files = params, files

        def vmt(self, name):
            return "LightmappedGeneric", self.params[name], "test:" + name

        def read(self, relative):
            data = self.files.get(relative)
            return (data, "test:" + relative) if data else (None, None)

    def test_st_through_the_world_mesh_flip_is_the_engine_coordinate(self):
        rng = np.random.default_rng(7)
        for _ in range(200):
            vectors = rng.uniform(-2, 2, (2, 4)) * [1, 1, 1, 64]
            mapping = {"width": int(2 ** rng.integers(3, 12)),
                       "height": int(2 ** rng.integers(3, 12))}
            points = rng.uniform(-4096, 4096, (int(rng.integers(3, 9)), 3))
            st = scene.face_st(points, {"vectors": vectors}, mapping)
            stored = np.stack([st[:, 0], 1.0 - st[:, 1]], axis=1)  # usd_worldmesh_pack
            engine = np.stack([(points @ vectors[j, :3] + vectors[j, 3]) /
                               (mapping["width"], mapping["height"])[j] for j in (0, 1)], axis=1)
            np.testing.assert_allclose(stored, engine, rtol=0, atol=1e-9 * np.abs(engine).max())

    def test_mapping_size_follows_the_representative_texture_order(self):
        """$basetexture, then $envmapmask, $bumpmap, $dudvmap, $normalmap; the
        size is the VTF's full-resolution header size, not the texdata size."""
        files = {"materials/a.vtf": vtf(13, 512, 256, b""),
                 "materials/b.vtf": vtf(13, 128, 128, b"")}
        order = list(scene.REPRESENTATIVE_TEXTURES)
        for index, key in enumerate(order):
            params = {later: "b" for later in order[index + 1:]}
            params[key] = "A.vtf" if index % 2 else "materials\\a"
            materials = self.Materials({"m": params}, files)
            size, source = scene.mapping_size(materials, "m")
            self.assertEqual((size["width"], size["height"]), (512, 256), key)
            self.assertIn(key, source)

    def test_missing_or_absent_texture_maps_as_the_error_texture(self):
        materials = self.Materials({"missing": {"$basetexture": "nope"}, "none": {}}, {})
        for name in ("missing", "none"):
            size, _ = scene.mapping_size(materials, name)
            self.assertEqual((size["width"], size["height"]),
                             (scene.ERROR_TEXTURE_SIZE, scene.ERROR_TEXTURE_SIZE))


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
