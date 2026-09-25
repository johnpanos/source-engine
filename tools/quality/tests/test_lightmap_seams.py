#!/usr/bin/env python3
"""Invariants of lightmap seam finding, lookup, measurement and stitching.

Pure numpy geometry and images; no Blender, OpenUSD or map build. Each case
states the invariant it enforces and includes a deliberately bad input the
invariant must reject.
"""

import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lightmap_seams as seams  # noqa: E402

SIZE = 64


def quad(z=0.0, flip=False):
    """Unit square in the plane z, as two triangles sharing the diagonal."""
    p = np.array([[0, 0, z], [1, 0, z], [1, 1, z], [0, 1, z]], dtype=np.float64)
    tris = [(0, 1, 2), (0, 2, 3)]
    if flip:
        tris = [(0, 2, 1), (0, 3, 2)]
    return np.array([[p[i] for i in t] for t in tris])


def chart_uvs(positions, offset, scale=0.25):
    """Planar lightmap UVs: world xy scaled into the atlas at `offset`."""
    return positions[..., :2] * scale + np.asarray(offset, dtype=np.float64)


def run(positions, uvs):
    uv_a, uv_b, weight, triangles, summary = seams.find_seams(
        np.asarray(positions, dtype=np.float64), np.asarray(uvs, dtype=np.float64), SIZE)
    return {"uv_a": uv_a, "uv_b": uv_b, "weight": weight, "triangles": triangles,
            "summary": summary}


class FindSeamsTest(unittest.TestCase):
    def test_one_chart_has_no_seam(self):
        """Triangles continuous in the lightmap (shared edge, same UVs) are one chart."""
        positions = quad()
        result = run(positions, chart_uvs(positions, (0.1, 0.1)))
        self.assertEqual(result["summary"]["seam_edges"], 0)
        self.assertEqual(len(result["uv_a"]), 0)

    def test_unwelded_soup_with_matching_uvs_has_no_seam(self):
        """Duplicated but coincident corners with equal UVs are still one chart:
        continuity is a property of positions and UVs, not of vertex indices."""
        positions = quad().copy()  # each triangle already owns its corners
        result = run(positions, chart_uvs(positions, (0.1, 0.1)))
        self.assertEqual(result["summary"]["seam_edges"], 0)

    def test_split_charts_seam_the_whole_shared_edge(self):
        """Two charts meeting on a coplanar edge give samples along its full
        length, and each pair looks up the same world point on both sides."""
        positions = quad()
        uvs = chart_uvs(positions, (0.1, 0.1))
        uvs[1] = chart_uvs(positions[1], (0.6, 0.1))
        result = run(positions, uvs)
        diagonal = np.sqrt(2.0)
        self.assertEqual(result["summary"]["seam_edges"], 1)
        self.assertAlmostEqual(result["weight"].sum(), diagonal, places=9)
        texels = diagonal * 0.25 * SIZE
        self.assertGreaterEqual(len(result["uv_a"]), 2 * texels - 1)
        # Invert each side's planar chart back to world xy: same point.
        world_a = (result["uv_a"] - 0.1) / 0.25
        world_b = (result["uv_b"] - np.array([0.6, 0.1])) / 0.25
        a_first = result["triangles"][:, 0] == 0
        world_a, world_b = (np.where(a_first[:, None], world_a, (result["uv_a"] - [0.6, 0.1]) / 0.25),
                            np.where(a_first[:, None], world_b, (result["uv_b"] - 0.1) / 0.25))
        np.testing.assert_allclose(world_a, world_b, atol=1e-12)
        np.testing.assert_allclose(world_a[:, 0], world_a[:, 1], atol=1e-12)  # on x = y

    def test_crease_is_not_a_seam(self):
        """Across a 90 degree corner irradiance differs physically: no seam."""
        floor = quad()[:1]
        wall = np.array([[[0, 0, 0], [1, 1, 1], [1, 1, 0]]], dtype=np.float64)
        wall = np.array([[[0, 0, 0], [0, 0, 1], [1, 1, 0]]], dtype=np.float64)
        positions = np.concatenate([floor, wall])
        uvs = np.concatenate([chart_uvs(floor, (0.1, 0.1)),
                              chart_uvs(wall[..., [0, 2, 1]], (0.6, 0.6))])
        self.assertEqual(run(positions, uvs)["summary"]["seam_edges"], 0)

    def test_back_to_back_faces_are_not_a_seam(self):
        """Coplanar faces with opposite normals (both sides of a thin panel)."""
        top, bottom = quad()[:1], quad(flip=True)[1:]
        positions = np.concatenate([top, bottom])
        uvs = np.concatenate([chart_uvs(top, (0.1, 0.1)), chart_uvs(bottom, (0.6, 0.6))])
        self.assertEqual(run(positions, uvs)["summary"]["seam_edges"], 0)

    def test_t_junction_is_matched_along_its_full_length(self):
        """One long edge against two short edges (a T-junction) still pairs
        the whole length; exact edge matching alone would miss it."""
        big = np.array([[[0, 0, 0], [2, 0, 0], [1, 1, 0]]], dtype=np.float64)
        small = np.array([[[0, 0, 0], [1, -1, 0], [1, 0, 0]],
                          [[1, 0, 0], [1, -1, 0], [2, 0, 0]]], dtype=np.float64)
        positions = np.concatenate([big, small])
        uvs = np.concatenate([chart_uvs(big, (0.05, 0.6), 0.2),
                              chart_uvs(small, (0.05, 0.3), 0.2)])
        result = run(positions, uvs)
        self.assertEqual(result["summary"]["seam_edges"], 2)
        self.assertAlmostEqual(result["weight"].sum(), 2.0, places=9)

    def test_continuous_t_junction_is_one_chart_not_a_seam(self):
        """A T-junction inside one chart (UVs agree along the shared line) is
        interior: no seam samples, and the triangles label as one chart."""
        big = np.array([[[0, 0, 0], [2, 0, 0], [1, 1, 0]]], dtype=np.float64)
        small = np.array([[[0, 0, 0], [1, -1, 0], [1, 0, 0]],
                          [[1, 0, 0], [1, -1, 0], [2, 0, 0]]], dtype=np.float64)
        positions = np.concatenate([big, small])
        uvs = chart_uvs(positions, (0.3, 0.3), 0.2)
        result = run(positions, uvs)
        self.assertEqual(result["summary"]["seam_edges"], 0)
        self.assertEqual(result["summary"]["continuous_junctions"], 2)
        self.assertEqual(set(seams.charts(positions, uvs, SIZE)), {0})

    def test_uncharted_triangles_are_ignored(self):
        """Excluded materials keep a degenerate lightmap UV: never a seam side."""
        positions = quad()
        uvs = chart_uvs(positions, (0.1, 0.1))
        uvs[1] = 0.99
        result = run(positions, uvs)
        self.assertEqual(result["summary"]["charted_triangles"], 1)
        self.assertEqual(result["summary"]["seam_edges"], 0)


class LookupTest(unittest.TestCase):
    def test_texel_centre_is_one_texel_and_rows_run_top_down(self):
        """uv of texel (row 0, col 0)'s centre is (0.5/W, 1 - 0.5/H): imageio
        row 0 is the top of the atlas, v = 1 (Blender saves EXRs top first)."""
        taps, weights = seams.bilinear_taps(np.array([[0.5 / 8, 1 - 0.5 / 4]]), 4, 8)
        self.assertEqual(taps[0, np.argmax(weights[0])], 0)
        self.assertAlmostEqual(weights[0].max(), 1.0)

    def test_bilinear_reproduces_a_linear_ramp(self):
        """A bilinear lookup of an affine image is exact away from the border."""
        h, w = 16, 16
        y, x = np.mgrid[0:h, 0:w].astype(np.float64)
        image = np.dstack([0.3 * x + 0.7 * y, 2 * x, y])
        uv = np.random.default_rng(1).uniform(0.1, 0.9, (100, 2))
        taps, weights = seams.bilinear_taps(uv, h, w)
        values = seams.lookup(image, taps, weights)
        px, py = uv[:, 0] * w - 0.5, (1 - uv[:, 1]) * h - 0.5
        np.testing.assert_allclose(values[:, 1], 2 * px, atol=1e-9)
        np.testing.assert_allclose(values[:, 2], py, atol=1e-9)


def two_chart_atlas(left=1.0, right=2.0, noise=0.0, seed=3):
    """A 32 x 32 atlas: chart A in columns 2..9, chart B in 20..27, both rows
    4..27, whose facing borders (A's right edge, B's left edge) are one seam."""
    rng = np.random.default_rng(seed)
    h = w = 32
    image = np.zeros((h, w, 4), dtype=np.float32)
    coverage = np.zeros((h, w), dtype=bool)
    image[4:28, 2:10, :3] = left + noise * rng.standard_normal((24, 8, 3))
    image[4:28, 20:28, :3] = right + noise * rng.standard_normal((24, 8, 3))
    image[..., 3] = 1.0
    coverage[4:28, 2:10] = coverage[4:28, 20:28] = True
    # Gutter: nearest chart texel, as the denoise step's gutter fill does.
    image[4:28, 10, :3] = image[4:28, 9, :3]
    image[4:28, 19, :3] = image[4:28, 20, :3]
    rows = (np.arange(4 * 2, 28 * 2) + 0.5) / 2          # two samples per texel
    v = 1 - rows / h
    uv_a = np.stack([np.full_like(v, 10 / w), v], axis=1)   # A's right border
    uv_b = np.stack([np.full_like(v, 20 / w), v], axis=1)   # B's left border
    found = {"uv_a": uv_a, "uv_b": uv_b, "weight": np.full(len(v), 0.5), "size": w}
    return image, coverage, found


class StitchTest(unittest.TestCase):
    def test_measure_reports_a_known_step(self):
        image, coverage, found = two_chart_atlas()
        result = seams.measure(image, found, coverage)
        self.assertAlmostEqual(result["p99"], 1.0 / 1.5, places=6)

    def test_measure_rejects_zero_samples(self):
        image, coverage, found = two_chart_atlas()
        empty = dict(found, uv_a=found["uv_a"][:0], uv_b=found["uv_b"][:0])
        with self.assertRaises(ValueError):
            seams.measure(image, empty, coverage)

    def test_stitched_seam_is_continuous(self):
        """After stitching, both sides' bilinear lookups agree at every sample
        (the gate), even from a 2x step with noise on both charts."""
        image, coverage, found = two_chart_atlas(noise=0.05)
        stitched = seams.stitch(image, seams.stitch_system(found, coverage))
        self.assertGreater(seams.measure(image, found, coverage)["p99"], 0.5)
        self.assertLess(seams.measure(stitched, found, coverage)["p99"], 1e-3)

    def test_stitch_changes_only_texels_a_seam_lookup_touches(self):
        image, coverage, found = two_chart_atlas(noise=0.05)
        system = seams.stitch_system(found, coverage)
        stitched = seams.stitch(image, system)
        untouched = np.ones(image.shape[0] * image.shape[1], dtype=bool)
        untouched[system["used"]] = False
        flat_in, flat_out = image.reshape(-1, 4), stitched.reshape(-1, 4)
        np.testing.assert_array_equal(flat_in[untouched], flat_out[untouched])
        np.testing.assert_array_equal(image[..., 3], stitched[..., 3])  # alpha kept

    def test_continuous_atlas_is_a_fixed_point(self):
        """Nothing to fix means nothing changes."""
        image, coverage, found = two_chart_atlas(left=1.5, right=1.5)
        stitched = seams.stitch(image, seams.stitch_system(found, coverage))
        np.testing.assert_allclose(stitched, image, atol=1e-6)

    def test_stitch_is_linear(self):
        """Stitching commutes with exposure: stitch(k x) = k stitch(x), so the
        separated layers stitch to a total that still sums."""
        image, coverage, found = two_chart_atlas(noise=0.05)
        system = seams.stitch_system(found, coverage)
        a = seams.stitch(image.astype(np.float64), system)
        b = seams.stitch(3.0 * image.astype(np.float64), system)
        np.testing.assert_allclose(b[..., :3], 3.0 * a[..., :3], rtol=1e-9, atol=1e-6)

    def test_covered_texels_move_less_than_gutter(self):
        """Covered texels are held near the bake; the free gutter absorbs most
        of the correction."""
        image, coverage, found = two_chart_atlas()
        stitched = seams.stitch(image, seams.stitch_system(found, coverage))
        change = np.abs(stitched - image)[..., :3].max(axis=2)
        self.assertLess(change[coverage].max(), change[~coverage].max())


class IndexedTrianglesTest(unittest.TestCase):
    def setUp(self):
        try:
            import legacy_bsp_scene
        except ImportError as error:  # needs OpenUSD's pxr
            self.skipTest("legacy_bsp_scene needs pxr: %s" % error)
        self.module = legacy_bsp_scene

    def test_coincident_corners_share_a_point(self):
        """Two quads sharing an edge: 6 unique points, 4 triangles, and every
        corner still carries its own UV and normal."""
        a = np.array([[0, 0, 0], [1, 0, 0], [1, 1, 0], [0, 1, 0]], dtype=np.float64)
        b = a + [1, 0, 0]
        normal = np.array([0, 0, 1.0])
        uvs = [np.arange(8).reshape(4, 2) * 0.1, np.arange(8).reshape(4, 2) * 0.2]
        points, counts, indices, normals, st = self.module.indexed_triangles(
            [a, b], uvs, [normal, normal])
        self.assertEqual(len(points), 6)
        self.assertEqual(counts, [3, 3, 3, 3])
        self.assertEqual(len(indices), 12)
        self.assertEqual(len(st), 12)
        corners = np.asarray(points)[indices]
        for tri in corners.reshape(-1, 3, 3):
            area = np.cross(tri[1] - tri[0], tri[2] - tri[0])
            self.assertGreater(np.dot(area, normal), 0)  # counter-clockwise front


if __name__ == "__main__":
    unittest.main()
