#!/usr/bin/env python3
"""Invariants of the planar lightmap layout, over randomized scenes.

Each seed builds flat geometry the way a relit BSP arrives: triangle soups
(no shared vertices), random placement, walls made of several coplanar parts,
closed boxes, T-junctions and zero-area slivers. The layout must hold every
chart invariant exactly, split no flat region, keep the probe rows and the
parking texel free, and give the same UVs whatever the triangle order. Pure
numpy; no Blender, OpenUSD or map build.
"""

import math
import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lightmap_layout as layout  # noqa: E402
import lightmap_seams as seams  # noqa: E402

SIZE = 256
SEEDS = range(16)


def random_rotation(rng):
    q, r = np.linalg.qr(rng.standard_normal((3, 3)))
    q *= np.sign(np.diag(r))
    if np.linalg.det(q) < 0:
        q[:, 0] = -q[:, 0]
    return q


def grid(rng, columns, rows, width, height):
    xs = np.concatenate([[0], np.cumsum(rng.uniform(0.5, 1.5, columns))]) * 1.0
    ys = np.concatenate([[0], np.cumsum(rng.uniform(0.5, 1.5, rows))]) * 1.0
    xs *= width / xs[-1]
    ys *= height / ys[-1]
    tris = []
    for i in range(columns):
        for j in range(rows):
            a, b = (xs[i], ys[j], 0), (xs[i + 1], ys[j], 0)
            c, d = (xs[i + 1], ys[j + 1], 0), (xs[i], ys[j + 1], 0)
            tris += [(a, b, c), (a, c, d)] if rng.random() < 0.5 else [(a, b, d), (b, c, d)]
    return np.array(tris, dtype=np.float64)


def place(tris, rotation, translation):
    return tris @ rotation.T + translation


def scene(seed):
    """(triangles, expected chart count, zero-area triangle count)."""
    rng = np.random.default_rng(seed)
    parts, charts = [], 0
    # A wall of several coplanar parts (several meshes/materials): one chart.
    r, t = random_rotation(rng), rng.uniform(-20, 20, 3)
    width = 0.0
    for _ in range(int(rng.integers(1, 4))):
        w = rng.uniform(0.5, 2.0)
        parts.append(place(grid(rng, int(rng.integers(1, 4)), 2, w, 1.5) + [width, 0, 0], r, t))
        width += w
    charts += 1
    # T-junctions: a coarse and a fine half of one wall: one chart.
    r, t = random_rotation(rng), rng.uniform(-20, 20, 3)
    parts.append(place(np.concatenate([grid(rng, 1, 1, 1.0, 1.0),
                                       grid(rng, 2, 3, 1.0, 1.0) + [1.0, 0, 0]]), r, t))
    charts += 1
    # A closed box: six charts, one per face.
    r, t = random_rotation(rng), rng.uniform(-20, 20, 3)
    for axis, angle in (("x", 0), ("x", math.pi / 2), ("x", math.pi), ("x", -math.pi / 2),
                        ("y", math.pi / 2), ("y", -math.pi / 2)):
        c, s = math.cos(angle), math.sin(angle)
        turn = np.array([[1, 0, 0], [0, c, -s], [0, s, c]]) if axis == "x" else \
            np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]])
        face = grid(rng, 2, 2, 1.0, 1.0) - [0.5, 0.5, -0.5]
        parts.append(place(face @ turn.T, r, t))
    charts += 6
    # A zero-area sliver on the first wall's plane (a collinear BSP corner).
    first = parts[0][0]
    sliver = np.array([[first[0], first[1], (first[0] + first[1]) / 2]])
    parts.append(sliver)
    return np.concatenate(parts), charts, 1


def seam_count(positions, uvs):
    _, _, _, triangles, _ = seams.find_seams(positions, uvs, SIZE)
    return len(triangles)


class PlanarLayoutTest(unittest.TestCase):
    def test_every_chart_invariant_holds_exactly(self):
        for seed in SEEDS:
            with self.subTest(seed=seed):
                positions, charts, slivers = scene(seed)
                uvs, record = layout.planar_layout(positions, SIZE)
                self.assertEqual(record["charts"], charts)
                self.assertEqual(record["parked_triangles"], slivers)
                result = seams.chart_invariants(positions, uvs, SIZE)
                self.assertEqual(result["out_of_bounds"], 0)
                self.assertEqual(result["overlap_texels"], 0)
                self.assertEqual(result["bleed_texels"], 0)
                self.assertAlmostEqual(result["density_spread"], 1.0, places=9)

    def test_no_flat_region_is_split(self):
        """Coplanar parts, soups and T-junctions all chart as one piece: the
        seam finder finds no seam anywhere (a box's edges are creases)."""
        for seed in SEEDS:
            with self.subTest(seed=seed):
                positions, _, _ = scene(seed)
                uvs, _ = layout.planar_layout(positions, SIZE)
                self.assertEqual(seam_count(positions, uvs), 0)

    def test_charts_are_at_least_twice_the_margin_apart(self):
        """No texel centre of one chart lies within 2 x MARGIN texels of
        another chart's (so bake dilation and lookups stay inside a chart)."""
        from scipy import ndimage
        for seed in list(SEEDS)[:6]:
            with self.subTest(seed=seed):
                positions, _, _ = scene(seed)
                uvs, _ = layout.planar_layout(positions, SIZE)
                labels = seams.charts(positions, uvs, SIZE)
                reach = 2 * layout.MARGIN - 1
                owner = np.full((SIZE, SIZE), -1)
                for chart in range(labels.max() + 1):
                    covered = seams.footprint(uvs, labels, chart, SIZE, SIZE)
                    grown = ndimage.binary_dilation(covered, iterations=reach - 1)
                    self.assertFalse(((owner >= 0) & grown).any(), "chart %d" % chart)
                    owner[covered] = chart

    def test_projection_has_no_stretch(self):
        """Every charted triangle keeps its shape: lightmap-space edge lengths
        are world lengths times one density."""
        for seed in list(SEEDS)[:6]:
            positions, _, _ = scene(seed)
            uvs, record = layout.planar_layout(positions, SIZE)
            density = record["texels_per_metre"]
            live = uvs.reshape(len(uvs), -1).std(axis=1) > 0
            for k in range(3):
                world = np.linalg.norm(positions[live, (k + 1) % 3] - positions[live, k], axis=1)
                texel = np.linalg.norm(uvs[live, (k + 1) % 3] - uvs[live, k], axis=1) * SIZE
                np.testing.assert_allclose(texel, world * density, rtol=1e-9, atol=1e-9)

    def test_density_is_the_largest_the_packer_fits(self):
        for seed in list(SEEDS)[:6]:
            with self.subTest(seed=seed):
                positions, _, _ = scene(seed)
                _, record = layout.planar_layout(positions, SIZE)
                extents, keys = record["extents_m"], record["keys"]
                density = record["texels_per_metre"]
                args = (SIZE, layout.MARGIN, 0, SIZE - 1)
                self.assertIsNotNone(layout.fits(extents, keys, density, *args))
                self.assertIsNone(layout.fits(extents, keys, density * 1.05, *args))

    def test_probe_rows_and_parking_texel_stay_free(self):
        for seed in list(SEEDS)[:6]:
            with self.subTest(seed=seed):
                positions, _, _ = scene(seed)
                reserved = 40
                charted = np.ones(len(positions), bool)
                charted[:3] = False                           # an excluded mesh
                uvs, record = layout.planar_layout(positions, SIZE, charted,
                                                   reserved_rows=reserved)
                live = uvs.reshape(len(uvs), -1).std(axis=1) > 0
                self.assertGreaterEqual(uvs[live, :, 1].min() * SIZE, reserved)
                self.assertLessEqual(uvs[live].max() * SIZE, SIZE - 1)
                np.testing.assert_array_equal(uvs[:3], layout.parking_uv(SIZE))

    def test_triangle_order_and_corner_order_do_not_matter(self):
        for seed in list(SEEDS)[:8]:
            with self.subTest(seed=seed):
                positions, _, _ = scene(seed)
                uvs, _ = layout.planar_layout(positions, SIZE)
                rng = np.random.default_rng(seed)
                order = rng.permutation(len(positions))
                roll = int(rng.integers(0, 3))
                shuffled = np.roll(positions[order], roll, axis=1)
                again, _ = layout.planar_layout(shuffled, SIZE)
                back = np.empty_like(again)
                back[order] = np.roll(again, -roll, axis=1)
                np.testing.assert_array_equal(back, uvs)

    def test_same_input_same_bytes(self):
        positions, _, _ = scene(3)
        a, _ = layout.planar_layout(positions, SIZE)
        b, _ = layout.planar_layout(positions.copy(), SIZE)
        self.assertEqual(a.tobytes(), b.tobytes())

    def test_plane_ids_decide_exactly(self):
        """With plane ids, a sliver whose float normal is a fraction of a
        degree off still joins its plane's chart; without them it does not."""
        rng = np.random.default_rng(1)
        quad = grid(rng, 2, 1, 2.0, 1.0)
        # A sliver along the quad's right edge, tilted by float noise.
        sliver = np.array([[[2.0, 0, 0], [2.0005, 0.5, 2e-6], [2.0, 1.0, 0]]])
        positions = np.concatenate([quad, sliver])
        planes = np.array([4] * len(quad) + [4])
        normals = np.tile([0, 0, 1.0], (len(positions), 1))
        _, record = layout.planar_layout(positions, SIZE, planes=planes, plane_normals=normals)
        self.assertEqual(record["charts"], 1)
        _, record = layout.planar_layout(positions, SIZE)          # geometry alone
        self.assertEqual(record["charts"], 2)

    def test_nearly_parallel_planes_join_and_bends_do_not(self):
        """Two faces meeting on an edge: planes 0.03 degrees apart (vbsp's
        neighbouring brushes) chart together; a 0.5 degree bend does not."""
        rng = np.random.default_rng(2)
        for degrees, charts in ((0.03, 1), (0.5, 2)):
            angle = math.radians(degrees)
            bend = np.array([[math.cos(angle), 0, -math.sin(angle)], [0, 1, 0],
                             [math.sin(angle), 0, math.cos(angle)]])
            left = grid(rng, 1, 1, 1.0, 1.0)
            right = grid(rng, 1, 1, 1.0, 1.0) @ bend.T + [1.0, 0, 0]
            positions = np.concatenate([left, right])
            planes = np.array([1] * len(left) + [2] * len(right))
            normals = np.concatenate([np.tile([0, 0, 1.0], (len(left), 1)),
                                      np.tile(bend @ [0, 0, 1.0], (len(right), 1))])
            uvs, record = layout.planar_layout(positions, SIZE, planes=planes,
                                               plane_normals=normals)
            self.assertEqual(record["charts"], charts, degrees)
            self.assertLess(seams.chart_invariants(positions, uvs, SIZE)["density_spread"],
                            1.001)

    def test_a_face_far_off_its_plane_charts_by_its_corners(self):
        """vbsp can leave a millimetres-wide face's corners 60 degrees off its
        nominal plane; the renderer draws the corners, so the layout projects
        that face on its own plane (no foreshortening), not the nominal one."""
        rng = np.random.default_rng(3)
        wall = grid(rng, 1, 1, 1.0, 1.0)                        # z = 0 plane
        strip = np.array([[[1.0, 0, 0], [1.005, 0, 0], [1.0, 1.0, 0.0]]])
        strip[0, 1, 2] = 0.0087                                  # lifted 60 degrees
        positions = np.concatenate([wall, strip])
        planes = np.array([7] * len(positions))                  # all say plane 7
        normals = np.tile([0, 0, 1.0], (len(positions), 1))
        uvs, record = layout.planar_layout(positions, SIZE, planes=planes, plane_normals=normals)
        self.assertEqual(record["charts"], 2)
        self.assertLess(seams.chart_invariants(positions, uvs, SIZE)["density_spread"], 1.001)

    def test_a_bent_panel_is_two_charts(self):
        """Negative control: two quads meeting at 1 degree are not one plane.
        The layout keeps them apart; the invariant code sees the crease as a
        non-flat boundary, so no flat seam and no merge."""
        rng = np.random.default_rng(0)
        left = grid(rng, 1, 1, 1.0, 1.0)
        angle = math.radians(1.0)
        bend = np.array([[math.cos(angle), 0, -math.sin(angle)], [0, 1, 0],
                         [math.sin(angle), 0, math.cos(angle)]])
        right = (grid(rng, 1, 1, 1.0, 1.0)) @ bend.T + [1.0, 0, 0]
        positions = np.concatenate([left, right])
        uvs, record = layout.planar_layout(positions, SIZE)
        self.assertEqual(record["charts"], 2)
        self.assertEqual(seams.chart_invariants(positions, uvs, SIZE)["bleed_texels"], 0)


def xatlas_tool():
    """The provisioned xatlas_chart (pbrt_map_toolchain provision --steps
    sources,xatlas), or None."""
    import pbrt_map_toolchain
    profile, _ = pbrt_map_toolchain.load_profiles()
    tool = pbrt_map_toolchain.absolute(profile["layout"]["xatlas_build"]) / \
        profile["dependencies"]["xatlas"]["build"]["binary"]
    return tool if tool.is_file() else None


def smooth_cylinder(rng, radius, height, segments, rings, faceted=False):
    """Triangles (n, 3, 3) and shading normals (n, 3, 3) of an open cylinder,
    randomly placed; smooth normals unless `faceted`."""
    rotation, offset = random_rotation(rng), rng.uniform(-3, 3, 3)
    tris, normals = [], []
    for i in range(segments):
        # The closing column reuses angle 0: 2 pi would not meet it exactly.
        a0, a1 = 2 * math.pi * i / segments, 2 * math.pi * ((i + 1) % segments) / segments
        mid = 2 * math.pi * (i + 0.5) / segments
        for j in range(rings):
            z0, z1 = height * j / rings, height * (j + 1) / rings

            def point(a, z):
                return (radius * math.cos(a), radius * math.sin(a), z)

            def normal(a):
                a = mid if faceted else a
                return (math.cos(a), math.sin(a), 0.0)
            tris += [(point(a0, z0), point(a1, z0), point(a1, z1)),
                     (point(a0, z0), point(a1, z1), point(a0, z1))]
            normals += [(normal(a0), normal(a1), normal(a1)), (normal(a0), normal(a1), normal(a0))]
    return (np.array(tris) @ rotation.T + offset, np.array(normals) @ rotation.T)


def smooth_sphere(rng, radius, segments, rings):
    rotation, offset = random_rotation(rng), rng.uniform(-3, 3, 3)
    tris, normals = [], []

    def direction(theta, phi):
        return np.array((math.sin(theta) * math.cos(phi), math.sin(theta) * math.sin(phi),
                         math.cos(theta)))
    for i in range(rings):
        for j in range(segments):
            t0, t1 = math.pi * i / rings, math.pi * (i + 1) / rings
            p0, p1 = 2 * math.pi * j / segments, 2 * math.pi * ((j + 1) % segments) / segments
            a, b, c, d = direction(t0, p0), direction(t1, p0), direction(t1, p1), direction(t0, p1)
            for tri in ((a, b, c), (a, c, d)):
                if np.linalg.norm(np.cross(tri[1] - tri[0], tri[2] - tri[0])) > 1e-9:
                    tris.append([radius * v for v in tri])
                    normals.append(list(tri))
    return (np.array(tris) @ rotation.T + offset, np.array(normals) @ rotation.T)


def closed_box(rng):
    """A randomly placed closed box of two triangles per face, (12, 3, 3)."""
    size = rng.uniform(0.5, 2.0, 3)
    rotation, offset = random_rotation(rng), rng.uniform(-3, 3, 3)
    tris = []
    for axis in range(3):
        for sign in (-1.0, 1.0):
            normal = np.zeros(3)
            normal[axis] = sign
            u = np.zeros(3)
            u[(axis + 1) % 3] = 1.0
            v = np.cross(normal, u)
            centre = normal * size / 2
            quad = [centre + (a * u + b * v) * size / 2
                    for a, b in ((-1, -1), (1, -1), (1, 1), (-1, 1))]
            tris += [(quad[0], quad[1], quad[2]), (quad[0], quad[2], quad[3])]
    return np.array(tris) @ rotation.T + offset


def flat_normals(positions):
    normals, _ = seams.triangle_normals(positions)
    return np.repeat(normals[:, None], 3, axis=1)


@unittest.skipIf(xatlas_tool() is None, "xatlas_chart is not provisioned "
                 "(pbrt_map_toolchain.py provision --steps sources,xatlas)")
class CurvedLayoutTest(unittest.TestCase):
    """Curved surfaces through xatlas: invariants, stretch and determinism."""

    def scene(self, seed):
        rng = np.random.default_rng(seed)
        cylinder = smooth_cylinder(rng, rng.uniform(0.3, 1.0), rng.uniform(1, 3),
                                   int(rng.integers(12, 32)), int(rng.integers(2, 8)))
        sphere = smooth_sphere(rng, rng.uniform(0.2, 0.8), int(rng.integers(12, 24)),
                               int(rng.integers(6, 12)))
        box = closed_box(rng)
        positions = np.concatenate([cylinder[0], sphere[0], box])
        normals = np.concatenate([cylinder[1], sphere[1], flat_normals(box)])
        curved = np.zeros(len(positions), bool)
        curved[:len(cylinder[0]) + len(sphere[0])] = True
        return positions, normals, curved

    def test_invariants_and_stretch(self):
        for seed in range(6):
            with self.subTest(seed=seed):
                positions, normals, curved = self.scene(seed)
                uvs, record = layout.planar_layout(positions, SIZE, corner_normals=normals,
                                                   xatlas=xatlas_tool())
                np.testing.assert_array_equal(record["curved_mask"], curved)
                invariants = seams.chart_invariants(positions, uvs, SIZE,
                                                    layout.MAX_STRETCH ** 2)
                for key in ("out_of_bounds", "overlap_texels", "bleed_texels"):
                    self.assertEqual(invariants[key], 0, key)
                low, high = layout.stretch(positions, uvs * SIZE, record["texels_per_metre"])
                self.assertGreaterEqual(low[curved].min(), 1 / layout.MAX_STRETCH - 1e-6)
                self.assertLessEqual(high[curved].max(), layout.MAX_STRETCH + 1e-6)
                # Flat triangles keep the exact atlas density.
                self.assertLess(np.abs(low[~curved] - 1).max(), 1e-6)
                self.assertLess(np.abs(high[~curved] - 1).max(), 1e-6)

    def test_curved_chart_area_matches_surface_area(self):
        positions, normals, curved = self.scene(3)
        uvs, record = layout.planar_layout(positions, SIZE, corner_normals=normals,
                                           xatlas=xatlas_tool())
        density = record["texels_per_metre"]
        texels = uvs * SIZE
        uv_area = layout.triangle_areas(texels)
        area = layout.triangle_areas(positions)
        # Summed over every curved chart, the UV area is the surface area at
        # the atlas density (each chart is scaled to it on its own).
        self.assertAlmostEqual(uv_area[curved].sum() / (area[curved].sum() * density ** 2), 1.0,
                               delta=1e-6)

    def test_same_uvs_in_any_triangle_and_corner_order(self):
        for seed in (0, 4):
            with self.subTest(seed=seed):
                positions, normals, _ = self.scene(seed)
                uvs, _ = layout.planar_layout(positions, SIZE, corner_normals=normals,
                                              xatlas=xatlas_tool())
                rng = np.random.default_rng(100 + seed)
                order = rng.permutation(len(positions))
                turn = rng.integers(0, 3, len(positions))
                shuffled = np.array([np.roll(positions[i], -r, axis=0)
                                     for i, r in zip(order, turn)])
                shuffled_normals = np.array([np.roll(normals[i], -r, axis=0)
                                             for i, r in zip(order, turn)])
                again, _ = layout.planar_layout(shuffled, SIZE, corner_normals=shuffled_normals,
                                                xatlas=xatlas_tool())
                expected = np.array([np.roll(uvs[i], -r, axis=0) for i, r in zip(order, turn)])
                self.assertEqual(expected.tobytes(), again.tobytes())

    def test_only_smooth_edges_make_a_surface_curved(self):
        rng = np.random.default_rng(8)
        smooth = smooth_cylinder(rng, 0.5, 2.0, 16, 3)
        faceted = smooth_cylinder(rng, 0.5, 2.0, 16, 3, faceted=True)
        box = closed_box(rng)
        live = np.ones(len(smooth[0]), bool)
        self.assertTrue(layout.curved_triangles(smooth[0], smooth[1], live).all())
        self.assertFalse(layout.curved_triangles(faceted[0], faceted[1], live).any())
        # A BSP's world faces carry their plane's normal: never curved.
        self.assertFalse(layout.curved_triangles(box, flat_normals(box),
                                                 np.ones(len(box), bool)).any())
        # Faceted surfaces chart as planar faces and need no xatlas.
        uvs, record = layout.planar_layout(faceted[0], SIZE, corner_normals=faceted[1])
        self.assertEqual(record["curved_triangles"], 0)
        self.assertEqual(record["charts"], 16)

    def test_stretch_limit_is_what_refines_charts(self):
        """Negative control: a charter whose first answer stretches every
        chart 2:1. With the limit lifted that stretch comes through; with it,
        the over-stretched charts are charted again (smaller) until they hold."""
        positions, normals = smooth_sphere(np.random.default_rng(2), 0.4, 24, 12)
        real = layout.xatlas_charts
        calls = []

        def stretched_first(positions, corner_normals, members, tool, max_area=0.0):
            labels, flat = real(positions, corner_normals, members, tool, max_area)
            calls.append(max_area)
            if max_area == 0.0:
                flat = flat * np.array([2.0, 1.0])
            return labels, flat

        limit = layout.MAX_STRETCH
        layout.xatlas_charts = stretched_first
        try:
            layout.MAX_STRETCH = 1e9
            uvs, record = layout.planar_layout(positions, SIZE, corner_normals=normals,
                                               xatlas=xatlas_tool())
            low, high = layout.stretch(positions, uvs * SIZE, record["texels_per_metre"])
            self.assertGreater(high.max() / low.min(), 2 * limit / 1.5)
            layout.MAX_STRETCH = limit
            calls.clear()
            uvs, record = layout.planar_layout(positions, SIZE, corner_normals=normals,
                                               xatlas=xatlas_tool())
        finally:
            layout.xatlas_charts = real
            layout.MAX_STRETCH = limit
        self.assertTrue(any(area > 0 for area in calls))
        low, high = layout.stretch(positions, uvs * SIZE, record["texels_per_metre"])
        self.assertLessEqual(high.max(), limit + 1e-6)
        self.assertGreaterEqual(low.min(), 1 / limit - 1e-6)

    def test_curved_surfaces_need_xatlas(self):
        positions, normals, _ = self.scene(1)
        with self.assertRaisesRegex(ValueError, "curved surfaces"):
            layout.planar_layout(positions, SIZE, corner_normals=normals)


if __name__ == "__main__":
    unittest.main()
