#!/usr/bin/env python3
"""Randomized variants of the lightmap chart and seam invariants.

Each seed builds a planar wall: a random rigid placement, a grid of random
cell sizes with random quad diagonals, cut into random column blocks, each
block one planar chart packed into the atlas at one texel density with a
margin. Every variant must satisfy the invariants exactly; deliberately
broken variants (overlapping, touching, rescaled or escaped charts, a
mismatched seam pairing) must be caught. Pure numpy; no Blender or builds.
"""

import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import lightmap_seams as seams  # noqa: E402

SIZE = 128
DENSITY = 16.0          # texels per metre
MARGIN = 4              # texels between packed charts
SEEDS = range(24)


def random_rotation(rng):
    q, r = np.linalg.qr(rng.standard_normal((3, 3)))
    q *= np.sign(np.diag(r))
    if np.linalg.det(q) < 0:
        q[:, 0] = -q[:, 0]
    return q


class Wall:
    """A random planar wall and its packed charts."""

    def __init__(self, seed):
        rng = np.random.default_rng(seed)
        self.rng = rng
        columns, rows = rng.integers(2, 6), rng.integers(1, 4)
        self.xs = np.concatenate([[0], np.cumsum(rng.uniform(0.2, 0.6, columns))])
        self.ys = np.concatenate([[0], np.cumsum(rng.uniform(0.2, 0.6, rows))])
        # Random column cuts: block k spans columns cuts[k]..cuts[k+1].
        inner = sorted(rng.choice(np.arange(1, columns), rng.integers(0, columns),
                                  replace=False)) if columns > 1 else []
        self.cuts = [0] + list(inner) + [columns]
        self.rotation = random_rotation(rng)
        self.translation = rng.uniform(-50, 50, 3)
        local, block = [], []
        for i in range(columns):
            for j in range(rows):
                a = (self.xs[i], self.ys[j])
                b = (self.xs[i + 1], self.ys[j])
                c = (self.xs[i + 1], self.ys[j + 1])
                d = (self.xs[i], self.ys[j + 1])
                tris = [(a, b, c), (a, c, d)] if rng.random() < 0.5 else [(a, b, d), (b, c, d)]
                k = np.searchsorted(self.cuts, i, side="right") - 1
                for tri in tris:
                    local.append(tri)
                    block.append(k)
        self.local = np.array(local, dtype=np.float64)        # (n, 3, 2)
        self.block = np.array(block)
        # Pack blocks left to right: uv = local * scale + offset(block).
        scale = DENSITY / SIZE
        self.offsets = []
        cursor = MARGIN
        for k in range(len(self.cuts) - 1):
            x0 = self.xs[self.cuts[k]]
            self.offsets.append(np.array([cursor / SIZE - x0 * scale, MARGIN / SIZE]))
            cursor += (self.xs[self.cuts[k + 1]] - x0) * DENSITY + MARGIN
        self.scale = scale
        if cursor > SIZE or self.ys[-1] * DENSITY + 2 * MARGIN > SIZE:
            raise AssertionError("variant does not fit the atlas")

    def positions(self):
        flat = np.concatenate([self.local, np.zeros(self.local.shape[:2] + (1,))], axis=2)
        return flat @ self.rotation.T + self.translation

    def uvs(self):
        return np.stack([self.local[t] * self.scale + self.offsets[self.block[t]]
                         for t in range(len(self.local))])

    def cut_length(self):
        """Analytic seam length: every internal block cut spans the wall height."""
        return (len(self.cuts) - 2) * self.ys[-1]

    def local_of_texel(self, block, x, y):
        """Wall-local point whose lookup in `block`'s chart hits texel centre (x, y)."""
        u = (x + 0.5) / SIZE
        v = 1 - (y + 0.5) / SIZE
        return (np.stack([u, v], axis=-1) - self.offsets[block]) / self.scale


def affine_field(wall, seed):
    rng = np.random.default_rng(1000 + seed)
    gradient, base = rng.uniform(-0.5, 0.5, 2), rng.uniform(1.0, 2.0)
    return lambda p: base + p @ gradient


def baked_atlas(wall, field, bias=None, noise=0.0, seed=0):
    """Each block's texels (and its whole lookup footprint, as an ideal
    dilation) evaluate the field at the wall point that texel stands for,
    optionally with a per-block bias and noise, like separate bakes."""
    rng = np.random.default_rng(2000 + seed)
    labels = seams.charts(wall.positions(), wall.uvs(), SIZE)
    image = np.zeros((SIZE, SIZE, 4), dtype=np.float64)
    image[..., 3] = 1.0
    coverage = np.zeros((SIZE, SIZE), dtype=bool)
    ys, xs = np.mgrid[0:SIZE, 0:SIZE]
    for chart in range(labels.max() + 1):
        block = wall.block[np.flatnonzero(labels == chart)[0]]
        reads = seams.footprint(wall.uvs(), labels, chart, SIZE, SIZE)
        value = field(wall.local_of_texel(block, xs[reads], ys[reads]))
        if bias is not None:
            value = value * bias[block]
        value = value + noise * rng.standard_normal(value.shape)
        image[reads, :3] = value[:, None]
        count, _ = seams.rasterize(wall.uvs(), np.where(labels == chart, chart, -1),
                                   SIZE, SIZE)
        coverage |= count > 0
    return image, coverage


def wall_with_seams(start):
    """The first variant from seed `start` on that has at least two charts."""
    seed = start
    while len(Wall(seed).cuts) < 3:
        seed += 1
    return Wall(seed), seed


def found_seams(positions, uvs):
    uv_a, uv_b, weight, triangles, summary = seams.find_seams(positions, uvs, SIZE)
    return {"uv_a": uv_a, "uv_b": uv_b, "weight": weight, "triangles": triangles,
            "summary": summary, "size": SIZE}


class ChartVariantsTest(unittest.TestCase):
    def test_valid_walls_satisfy_every_chart_invariant(self):
        for seed in SEEDS:
            with self.subTest(seed=seed):
                wall = Wall(seed)
                result = seams.chart_invariants(wall.positions(), wall.uvs(), SIZE)
                self.assertEqual(result["charts"], len(wall.cuts) - 1)
                self.assertEqual(result["out_of_bounds"], 0)
                self.assertEqual(result["overlap_texels"], 0)
                self.assertEqual(result["bleed_texels"], 0)
                self.assertAlmostEqual(result["density_spread"], 1.0, places=9)

    def test_overlapping_chart_is_caught(self):
        for seed in SEEDS:
            wall = Wall(seed)
            if len(wall.cuts) < 3:
                continue
            with self.subTest(seed=seed):
                # Map block 1's wall span onto block 0's texels (a copy of
                # block 0's offset alone would just continue block 0's chart).
                width = (wall.xs[wall.cuts[1]] - wall.xs[wall.cuts[0]]) * wall.scale
                wall.offsets[1] = wall.offsets[0] - [width, 0]
                result = seams.chart_invariants(wall.positions(), wall.uvs(), SIZE)
                self.assertGreater(result["overlap_texels"], 0)
                self.assertGreater(result["bleed_texels"], 0)

    def test_charts_one_texel_apart_bleed_without_overlapping(self):
        for seed in SEEDS:
            wall = Wall(seed)
            if len(wall.cuts) < 3:
                continue
            with self.subTest(seed=seed):
                # Slide block 1 left until its chart is one texel from block 0's.
                shift = (MARGIN - 1) / SIZE
                for k in range(1, len(wall.offsets)):
                    wall.offsets[k] = wall.offsets[k] - [shift * k, 0]
                result = seams.chart_invariants(wall.positions(), wall.uvs(), SIZE)
                self.assertEqual(result["overlap_texels"], 0)
                self.assertGreater(result["bleed_texels"], 0)

    def test_single_pass_counts_match_a_per_chart_oracle(self):
        """The single-pass overlap/bleed counts equal a brute-force per-chart
        computation, on valid, overlapping and touching variants."""
        def brute(positions, uvs):
            labels = seams.charts(positions, uvs, SIZE)
            charts = range(labels.max() + 1)
            centres = [seams.rasterize(uvs, np.where(labels == c, c, -1), SIZE, SIZE)[0] > 0
                       for c in charts]
            reads = [seams.footprint(uvs, labels, c, SIZE, SIZE) for c in charts]
            return int((np.sum(centres, axis=0) > 1).sum()), int((np.sum(reads, axis=0) > 1).sum())

        for seed in list(SEEDS)[:8]:
            wall = Wall(seed)
            variants = [wall.uvs()]
            if len(wall.cuts) >= 3:
                width = (wall.xs[wall.cuts[1]] - wall.xs[wall.cuts[0]]) * wall.scale
                moved = [o.copy() for o in wall.offsets]
                wall.offsets[1] = wall.offsets[0] - [width * 0.5, 0]
                variants.append(wall.uvs())
                wall.offsets = moved
            for uvs in variants:
                with self.subTest(seed=seed):
                    result = seams.chart_invariants(wall.positions(), uvs, SIZE)
                    self.assertEqual((result["overlap_texels"], result["bleed_texels"]),
                                     brute(wall.positions(), uvs))

    def test_rescaled_chart_breaks_density(self):
        wall = Wall(3)
        uvs = wall.uvs()
        first = wall.block == 0
        uvs[first] = (uvs[first] - uvs[first].min(axis=(0, 1))) * 1.8 + uvs[first].min(axis=(0, 1))
        result = seams.chart_invariants(wall.positions(), uvs, SIZE)
        self.assertEqual(result["density_violation"], 1)

    def test_escaped_uv_is_caught(self):
        wall = Wall(5)
        uvs = wall.uvs()
        uvs[0, 0] = [1.01, 0.5]
        self.assertGreater(seams.chart_invariants(wall.positions(), uvs, SIZE)["out_of_bounds"], 0)


class SeamVariantsTest(unittest.TestCase):
    def test_seam_length_is_the_analytic_cut_length(self):
        for seed in SEEDS:
            with self.subTest(seed=seed):
                wall = Wall(seed)
                result = found_seams(wall.positions(), wall.uvs())
                self.assertAlmostEqual(result["weight"].sum(), wall.cut_length(), places=6)

    def test_seams_are_invariant_under_placement_order_and_winding(self):
        """Rigid motion, triangle order, corner rotation within a triangle and
        a consistent winding flip leave the seam set unchanged."""
        for seed in SEEDS:
            with self.subTest(seed=seed):
                wall = Wall(seed)
                base = found_seams(wall.positions(), wall.uvs())
                rng = np.random.default_rng(seed)
                order = rng.permutation(len(wall.local))
                roll = rng.integers(0, 3)
                positions = np.roll(wall.positions()[order], roll, axis=1)
                uvs = np.roll(wall.uvs()[order], roll, axis=1)
                variants = [(positions, uvs), (positions[:, ::-1], uvs[:, ::-1]),
                            (positions @ random_rotation(rng).T + 7.0, uvs)]
                for p, u in variants:
                    other = found_seams(p, u)
                    self.assertAlmostEqual(other["weight"].sum(), base["weight"].sum(), places=6)
                    self.assertEqual(other["summary"]["seam_edges"],
                                     base["summary"]["seam_edges"])
                    pairs = lambda r: sorted(map(tuple, np.round(np.sort(
                        np.concatenate([r["uv_a"], r["uv_b"]], axis=1).reshape(-1, 2, 2),
                        axis=1).reshape(-1, 4) * 1e6)))
                    self.assertEqual(pairs(other), pairs(base))

    def test_correct_pairing_measures_zero_on_an_affine_field(self):
        """With ideal gutters, bilinear lookups of an affine field are exact,
        so paired samples of one world point agree to rounding."""
        for seed in SEEDS:
            wall = Wall(seed)
            if len(wall.cuts) < 3:
                continue
            with self.subTest(seed=seed):
                image, coverage = baked_atlas(wall, affine_field(wall, seed))
                result = found_seams(wall.positions(), wall.uvs())
                self.assertLess(seams.measure(image, result, coverage)["max"], 1e-9)

    def test_mismatched_pairing_is_caught(self):
        """Shifting side B by half the seam breaks the zero measurement: the
        measurement really compares the same world point."""
        caught = 0
        for seed in SEEDS:
            wall = Wall(seed)
            if len(wall.cuts) < 3:
                continue
            image, coverage = baked_atlas(wall, affine_field(wall, seed))
            result = found_seams(wall.positions(), wall.uvs())
            result["uv_b"] = np.roll(result["uv_b"], len(result["uv_b"]) // 2, axis=0)
            caught += seams.measure(image, result, coverage)["max"] > 1e-3
        self.assertGreater(caught, 0)

    def test_measure_is_symmetric(self):
        wall, seed = wall_with_seams(7)
        image, coverage = baked_atlas(wall, affine_field(wall, seed),
                                      bias=np.linspace(0.8, 1.2, len(wall.cuts) - 1))
        result = found_seams(wall.positions(), wall.uvs())
        swapped = dict(result, uv_a=result["uv_b"], uv_b=result["uv_a"])
        a, b = seams.measure(image, result, coverage), seams.measure(image, swapped, coverage)
        for key in ("mean", "p95", "p99", "max"):
            self.assertAlmostEqual(a[key], b[key], places=12)


class StitchVariantsTest(unittest.TestCase):
    def test_stitch_closes_biased_noisy_bakes(self):
        """Per-chart exposure bias (up to 20%) and noise: stitched seams agree."""
        for seed in SEEDS:
            wall = Wall(seed)
            if len(wall.cuts) < 3:
                continue
            with self.subTest(seed=seed):
                rng = np.random.default_rng(seed)
                bias = rng.uniform(0.8, 1.2, len(wall.cuts) - 1)
                image, coverage = baked_atlas(wall, affine_field(wall, seed), bias, 0.02, seed)
                result = found_seams(wall.positions(), wall.uvs())
                system = seams.stitch_system(result, coverage)
                stitched = seams.stitch(image, system)
                self.assertLess(seams.measure(stitched, result, coverage)["p99"], 1e-3)
                # Only seam-lookup texels change; alpha never does.
                untouched = np.ones(SIZE * SIZE, dtype=bool)
                untouched[system["used"]] = False
                np.testing.assert_array_equal(image.reshape(-1, 4)[untouched],
                                              stitched.reshape(-1, 4)[untouched])
                np.testing.assert_array_equal(image[..., 3], stitched[..., 3])

    def test_stitch_leaves_a_continuous_atlas_alone(self):
        for seed in SEEDS:
            wall = Wall(seed)
            if len(wall.cuts) < 3:
                continue
            with self.subTest(seed=seed):
                image, coverage = baked_atlas(wall, affine_field(wall, seed))
                result = found_seams(wall.positions(), wall.uvs())
                stitched = seams.stitch(image, seams.stitch_system(result, coverage))
                np.testing.assert_allclose(stitched, image, atol=1e-6)

    def test_stitch_again_changes_nothing_visible(self):
        """Stitching a stitched atlas is (nearly) a fixed point."""
        wall, seed = wall_with_seams(11)
        image, coverage = baked_atlas(wall, affine_field(wall, seed),
                                      np.linspace(0.8, 1.2, len(wall.cuts) - 1), 0.02, seed)
        result = found_seams(wall.positions(), wall.uvs())
        system = seams.stitch_system(result, coverage)
        once = seams.stitch(image, system)
        twice = seams.stitch(once, system)
        self.assertLess(np.abs(twice - once).max(), 1e-3 * np.abs(once).max())

    def test_buried_chart_moves_alone(self):
        """A chart the raw bake left black (buried under another surface) is
        free: stitching closes the seam by moving it, and the visible chart's
        covered texels keep their baked values."""
        for seed in SEEDS:
            wall = Wall(seed)
            if len(wall.cuts) < 3:
                continue
            with self.subTest(seed=seed):
                bias = np.ones(len(wall.cuts) - 1)
                bias[0] = 0.0                                    # block 0 buried
                image, coverage = baked_atlas(wall, affine_field(wall, seed), bias)
                result = found_seams(wall.positions(), wall.uvs())
                buried = seams.buried_texels(image, coverage)
                self.assertTrue(buried.any())
                stitched = seams.stitch(image, seams.stitch_system(result, coverage, buried))
                visible = coverage & ~buried
                self.assertLess(np.abs(stitched - image)[visible].max(), 1e-3 * image[visible].max())
                self.assertLess(seams.measure(stitched, result, coverage)["p99"], 1e-3)

    def test_layer_steps_are_measured_against_the_total(self):
        """A step in a dim indirect layer is judged by the total light it
        joins: large relative to the layer, small relative to the total."""
        wall, seed = wall_with_seams(3)
        bias = np.linspace(0.5, 1.5, len(wall.cuts) - 1)
        indirect, coverage = baked_atlas(wall, lambda p: 0.001 + 0 * p[..., 0], bias)
        total, _ = baked_atlas(wall, affine_field(wall, seed))
        result = found_seams(wall.positions(), wall.uvs())
        self.assertGreater(seams.measure(indirect, result, coverage)["p99"], 0.3)
        self.assertLess(seams.measure(indirect, result, coverage, reference=total)["p99"], 0.002)
        loud, _ = baked_atlas(wall, lambda p: 0.5 + 0 * p[..., 0], bias)
        self.assertGreater(seams.measure(loud, result, coverage, reference=total)["p99"], 0.1)

    def test_absolute_measure_reads_signed_pages(self):
        """The directional gradient is signed and near zero: its steps are
        absolute, largest over channels."""
        wall, seed = wall_with_seams(4)
        signed, coverage = baked_atlas(wall, lambda p: 0 * p[..., 0])
        result = found_seams(wall.positions(), wall.uvs())
        self.assertEqual(seams.measure(signed, result, coverage, absolute=True)["max"], 0.0)
        bias = np.linspace(-1, 1, len(wall.cuts) - 1)
        stepped, _ = baked_atlas(wall, lambda p: 0.05 + 0 * p[..., 0], bias)
        self.assertGreater(seams.measure(stepped, result, coverage, absolute=True)["max"], 0.04)

    def test_layers_stitch_to_the_stitched_total(self):
        """Linearity: stitch(direct) + stitch(indirect) = stitch(direct + indirect),
        so RFC 0011's separated layers still sum to the total after stitching."""
        wall, seed = wall_with_seams(13)
        direct, coverage = baked_atlas(wall, affine_field(wall, seed), None, 0.02, 1)
        indirect, _ = baked_atlas(wall, affine_field(wall, seed + 1), None, 0.02, 2)
        result = found_seams(wall.positions(), wall.uvs())
        system = seams.stitch_system(result, coverage)
        total = direct.copy()
        total[..., :3] += indirect[..., :3]
        summed = seams.stitch(direct, system)[..., :3] + seams.stitch(indirect, system)[..., :3]
        np.testing.assert_allclose(summed, seams.stitch(total, system)[..., :3], atol=1e-6)


if __name__ == "__main__":
    unittest.main()
