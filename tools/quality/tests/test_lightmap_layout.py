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


if __name__ == "__main__":
    unittest.main()
