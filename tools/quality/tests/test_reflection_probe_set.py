"""Oracles for parallax-corrected, blended reflection probes (R50-PARALLAX).

Pure math, no Blender: the proxy-box fit, the corrected lookup, distance
roughness, the RPRB writer/reader, the blend and the placement are checked on
analytic box scenes (`reflection_probe_set.BoxScene`), where the true
reflection of every ray is known exactly.

    python3 -m unittest tools/quality/tests/test_reflection_probe_set.py -v
"""

import math
import struct
import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import reflection_probe  # noqa: E402
import reflection_probe_set as rps  # noqa: E402
import bc_codec  # noqa: E402  (reflection_probe_set puts tools/texture on the path)

ROOM = ((0.0, 0.0, 0.0), (6.0, 4.0, 3.0))


def unit(v):
    v = np.asarray(v, dtype=np.float64)
    return v / np.linalg.norm(v, axis=-1, keepdims=True)


def cube_depths(scene, capture, size=48):
    """Ray distances of a cube capture, as the depth pass yields them."""
    depths = {}
    for name in reflection_probe.FACES:
        forward, up, right = reflection_probe.face_basis(name)
        t = (np.arange(size) + 0.5) / size * 2 - 1
        x, y = np.meshgrid(t, -t)
        directions = unit(forward + x[..., None] * right + y[..., None] * up).reshape(-1, 3)
        distance, _ = scene(np.tile(capture, (len(directions), 1)), directions, 1e4)
        depths[name] = np.where(np.isfinite(distance), distance, 1e10).reshape(size, size)
    return depths


def fit(scene, capture, size=48):
    directions, distances, weights = reflection_probe.face_samples(cube_depths(scene, capture,
                                                                               size))
    return reflection_probe.fit_parallax_box(np.asarray(capture, float), directions, distances,
                                             weights)


def stripes(points):
    """Scene radiance at surface points: bands 0.5 m wide on every wall, so a
    misplaced reflection is visible in every direction."""
    phase = np.floor(points[:, 0] / 0.5) + 2 * np.floor(points[:, 1] / 0.5) + \
        3 * np.floor(points[:, 2] / 0.5)
    return np.stack((0.2 + 0.8 * (phase % 2), 0.2 + 0.8 * ((phase // 2) % 2),
                     0.2 + 0.8 * ((phase // 4) % 2)), axis=1)


def probe_chain(scene, capture, width=256):
    """A probe's mirror mip (the only one a roughness-0 lookup reads)."""
    directions = reflection_probe.equirect_directions(width).reshape(-1, 3)
    distance, _ = scene(np.tile(capture, (len(directions), 1)), directions, 1e4)
    radiance = stripes(capture + distance[:, None] * directions)
    return [radiance.reshape(width // 2, width, 3)]


class ParallaxLookupTest(unittest.TestCase):
    def test_from_the_capture_point_the_lookup_is_the_ray(self):
        capture = np.array((2.0, 1.5, 1.6))
        rays = unit(np.random.default_rng(1).normal(size=(64, 3)))
        lookup, _, _, valid = reflection_probe.parallax_lookup(
            np.tile(capture, (64, 1)), rays, capture, np.array(ROOM[0]), np.array(ROOM[1]))
        self.assertTrue(valid.all())
        np.testing.assert_allclose(lookup, rays, atol=1e-9)

    def test_lookup_points_at_the_true_hit_when_the_box_is_the_room(self):
        scene = rps.BoxScene(rooms=[ROOM])
        capture = np.array((3.0, 2.0, 1.6))
        rng = np.random.default_rng(2)
        points = np.column_stack((rng.uniform(0.2, 5.8, 200), rng.uniform(0.2, 3.8, 200),
                                  np.zeros(200)))
        rays = unit(rng.normal(size=(200, 3)) * (1, 1, 0.3) + (0, 0, 1.0))
        distance, _ = scene(points, rays, 1e4)
        truth = unit(points + distance[:, None] * rays - capture)
        lookup, shaded, captured, valid = reflection_probe.parallax_lookup(
            points, rays, capture, np.array(ROOM[0]), np.array(ROOM[1]))
        self.assertTrue(valid.all())
        np.testing.assert_allclose(lookup, truth, atol=1e-9)
        np.testing.assert_allclose(shaded, distance, rtol=1e-9)
        # Direction-only sampling is off by tens of degrees for the same rays.
        error = np.degrees(np.arccos(np.clip(np.sum(rays * truth, axis=1), -1, 1)))
        self.assertGreater(error.max(), 30.0)

    def test_a_point_outside_whose_ray_misses_is_invalid_and_keeps_its_ray(self):
        lookup, _, _, valid = reflection_probe.parallax_lookup(
            np.array([(-1.0, 2.0, 1.0)]), np.array([(-1.0, 0.0, 0.0)]),
            np.array((3.0, 2.0, 1.5)), np.array(ROOM[0]), np.array(ROOM[1]))
        self.assertFalse(valid[0])
        np.testing.assert_allclose(lookup[0], (-1.0, 0.0, 0.0))


class DistanceRoughnessTest(unittest.TestCase):
    def test_frostbite_listing_25(self):
        f = reflection_probe.distance_roughness
        self.assertEqual(float(f(0.0, 1.0, 4.0)), 0.0)  # a mirror stays a mirror
        self.assertAlmostEqual(float(f(1.0, 0.1, 4.0)), 1.0)  # fully rough is unchanged
        # r = 0.4, hit 1 m from the point and 4 m from the capture:
        # clamp(0.25 * 0.4) = 0.1, lerp(0.1, 0.4, 0.4) = 0.22.
        self.assertAlmostEqual(float(f(0.4, 1.0, 4.0)), 0.22)
        # Never rougher than authored, even when the capture is nearer the hit.
        self.assertAlmostEqual(float(f(0.4, 4.0, 1.0)), 0.4)

    def test_direction_eases_to_the_ray_with_roughness(self):
        lookup, ray = np.array([(1.0, 0, 0)]), np.array([(0, 1.0, 0)])
        np.testing.assert_allclose(reflection_probe.corrected_direction(lookup, ray, 0.0), lookup)
        np.testing.assert_allclose(reflection_probe.corrected_direction(lookup, ray, 1.0), ray)


class BoxFitTest(unittest.TestCase):
    def assert_box(self, box, expected, tolerance=0.03):
        np.testing.assert_allclose(box[0], expected[0], atol=tolerance)
        np.testing.assert_allclose(box[1], expected[1], atol=tolerance)

    def test_empty_room_is_recovered_with_no_residual(self):
        box_min, box_max, report = fit(rps.BoxScene(rooms=[ROOM]), (2.0, 1.5, 1.6))
        self.assert_box((box_min, box_max), ROOM)
        self.assertLess(report["mean_relative_residual"], 1e-6)

    def test_furniture_in_front_of_a_wall_does_not_pull_the_plane(self):
        sofa = ((4.8, 0.5, 0.0), (5.6, 3.5, 0.9))
        box_min, box_max, report = fit(rps.BoxScene(rooms=[ROOM], solids=[sofa]),
                                       (2.0, 1.5, 1.6))
        self.assert_box((box_min, box_max), ROOM)
        self.assertGreater(report["mean_relative_residual"], 0.001)

    def test_a_doorway_into_a_deeper_room_does_not_pull_the_plane(self):
        # Room A, and room B behind A's +x wall seen through a 1 m doorway.
        scene = rps.BoxScene(rooms=[ROOM, ((6.0, 0.0, 0.0), (14.0, 4.0, 3.0))],
                             openings=[((5.9, 1.5, 0.0), (6.1, 2.5, 2.1))])
        box_min, box_max, _ = fit(scene, (2.0, 2.0, 1.6))
        self.assert_box((box_min, box_max), ROOM)

    def test_a_plain_percentile_would_have_gone_through_the_doorway(self):
        # The guard: the doorway's far wall is there to be mistaken.
        scene = rps.BoxScene(rooms=[ROOM, ((6.0, 0.0, 0.0), (14.0, 4.0, 3.0))],
                             openings=[((5.9, 1.0, 0.0), (6.1, 3.0, 2.5))])
        directions, distances, weights = reflection_probe.face_samples(
            cube_depths(scene, (2.0, 2.0, 1.6)))
        px = (np.argmax(np.abs(directions), axis=1) == 0) & (directions[:, 0] > 0)
        plane = distances[px] * directions[px, 0]
        self.assertGreater(np.percentile(plane, 95), 5.0)
        _, box_max, _ = fit(scene, (2.0, 2.0, 1.6))
        self.assertAlmostEqual(box_max[0], 6.0, delta=0.03)

    def test_a_face_that_sees_sky_stays_open(self):
        # No ceiling: the room is open upward.
        open_room = rps.BoxScene(rooms=[((0.0, 0.0, 0.0), (6.0, 4.0, 30.0))],
                                 openings=[((-1, -1, 2.9), (7, 5, 31))])
        box_min, box_max, report = fit(open_room, (2.0, 1.5, 1.6))
        self.assertEqual(box_max[2], 1.6 + reflection_probe.OPEN_EXTENT)
        self.assertTrue(report["faces"]["pz"]["open"])
        self.assertGreater(report["sky_fraction"], 0.1)

    def test_residual_scores_a_wrong_box(self):
        scene = rps.BoxScene(rooms=[ROOM])
        capture = np.array((2.0, 1.5, 1.6))
        directions, distances, weights = reflection_probe.face_samples(cube_depths(scene,
                                                                                   capture))
        good = reflection_probe.fit_residual(capture, np.array(ROOM[0]), np.array(ROOM[1]),
                                             directions, distances, weights)
        bad = reflection_probe.fit_residual(capture, np.array((1.0, 1.0, 1.0)),
                                            np.array((3.0, 2.0, 2.0)), directions, distances,
                                            weights)
        self.assertLess(good["mean_relative_residual"], 1e-6)
        self.assertGreater(bad["mean_relative_residual"], 0.5)


def two_probe_layout(scene=None):
    scene = scene or rps.BoxScene(rooms=[ROOM])
    probes = [
        {"capture": np.array((1.5, 2.0, 1.6)), "box_min": np.array(ROOM[0]),
         "box_max": np.array(ROOM[1]), "influence_min": np.array((-0.15, -0.15, -0.15)),
         "influence_max": np.array((3.0, 4.15, 3.15)), "fade": 0.5},
        {"capture": np.array((4.5, 2.0, 1.6)), "box_min": np.array(ROOM[0]),
         "box_max": np.array(ROOM[1]), "influence_min": np.array((-0.15, -0.15, -0.15)),
         "influence_max": np.array((6.15, 4.15, 3.15)), "fade": 0.5, "global": True}]
    chains = [probe_chain(scene, probe["capture"], 64) * 1 for probe in probes]
    chains = [reflection_probe.mip_chain(chain[0], samples=8) for chain in chains]
    return probes, chains


class RprbFormatTest(unittest.TestCase):
    def test_round_trip(self):
        probes, chains = two_probe_layout()
        layout = rps.read(rps.build(probes, chains))
        self.assertEqual((layout["count"], layout["width"]), (2, 64))
        self.assertEqual(layout["global_index"], 1)
        self.assertEqual([p["rank"] for p in layout["probes"]], [0, 1])
        np.testing.assert_allclose(layout["probes"][0]["capture"],
                                   probes[0]["capture"] * rps.SOURCE_UNITS_PER_METER, rtol=1e-6)
        # v7: the radiance chains are the BC6H blocks decoded (as halves);
        # this fixture's saturated checker is BC6H's worst case, so quality is
        # judged on smooth light below.
        data = rps.build(probes, chains)
        blocks = data[layout["atlas_offset"]:layout["atlas_offset"] +
                      rps.radiance_block_bytes(2, 64)]
        decoded = bc_codec.decode_bc6h(blocks, 128, 64).astype("<f2").astype(np.float64)
        for index, chain in enumerate(layout["chains"]):
            np.testing.assert_array_equal(chain[0], decoded[index * 32:(index + 1) * 32, :64])
        texture = rps.gpu_texture(layout)
        grid_rows = -(-rps.CANDIDATE_CELLS * 2 * rps.candidate_words(2) // 128)
        self.assertEqual(texture.shape, (1 + 2 + 2 * 32 + grid_rows, 128, 4))
        self.assertEqual(tuple(texture[0, 0]), (2, len(chains[0]), 64, rps.GPU_MARKER))
        self.assertEqual(texture[0, 1, 0], rps.MODE_BLEND)
        table = rps.texture_table(texture)
        self.assertEqual(table[1, 2, 3], 1.0)  # rank 1 is the global probe
        # hi + lo keeps positions to ~0.01 Source units.
        np.testing.assert_allclose(table[0, 0, :3], layout["probes"][0]["capture"], atol=0.02)
        np.testing.assert_allclose(table[1, 4, :3], layout["probes"][1]["influence_max"],
                                   atol=0.02)
        # The atlas follows the table, band by band.
        np.testing.assert_array_equal(texture[3:3 + 32, :64, :3],
                                      layout["chains"][0][0].astype(np.float16))

    def test_smooth_light_survives_the_block_encoding(self):
        probes, _ = two_probe_layout()
        y, x = np.mgrid[0:32, 0:64] / 64.0
        smooth = np.stack([0.5 + 4 * x, 0.3 + 2 * y, 0.2 + x * y], axis=2)
        chains = [reflection_probe.mip_chain(smooth * (1 + i)) for i in range(2)]
        layout = rps.read(rps.build(probes, chains))
        # A steep ramp is harsher than real light (a 512-wide Portal 2 probe:
        # 0.7-4.5% mean per mip, at most 0.6 stops at mip 0's hard edges;
        # RFC/0016-progress.md); this ramp reaches 13% on its 4 x 2 mip. The
        # bounds catch gross breakage (misplaced or unpadded blocks).
        for chain, back in zip(chains, layout["chains"]):
            for level, (mip, decoded) in enumerate(zip(chain, back)):
                error = bc_codec.hdr_error(mip, decoded)
                self.assertLess(error["relative_mean"], 0.15 if level == len(chain) - 1 else 0.06)
                self.assertLess(error["stops_max"], 0.3)

    def mutate(self, offset, fmt, value, code):
        probes, chains = two_probe_layout()
        data = bytearray(rps.build(probes, chains))
        struct.pack_into(fmt, data, offset, value)
        with self.assertRaises(rps.RprbError) as caught:
            rps.read(bytes(data))
        self.assertEqual(caught.exception.code, code)

    def test_malformed_payloads_are_rejected(self):
        for offset, fmt, value, code in rps.MALFORMATIONS:
            with self.subTest(code=code, offset=offset):
                self.mutate(offset, fmt, value, code)
        probes, chains = two_probe_layout()
        data = rps.build(probes, chains)
        with self.assertRaises(rps.RprbError):
            rps.read(data[:-2])                           # truncated atlas
        # Texel corruption is judged in the relight rows (RELIGHT_MALFORMATIONS):
        # any BC6H block decodes to finite, non-negative light.

    def test_writer_refuses_two_globals(self):
        probes, chains = two_probe_layout()
        probes[0]["global"] = True
        with self.assertRaises(rps.RprbError):
            rps.build(probes, chains)


class BlendTest(unittest.TestCase):
    def layout(self):
        probes, chains = two_probe_layout()
        return rps.read(rps.build(probes, chains, scale=1.0))

    def walk(self, count=241):
        x = np.linspace(0.2, 5.8, count)
        return np.column_stack((x, np.full(count, 2.0), np.zeros(count))), \
            np.tile((0.0, 0.0, 1.0), (count, 1))

    def test_weights_sum_to_one_with_at_most_two_probes(self):
        layout = self.layout()
        points, normals = self.walk()
        weights = rps.blend_weights(points, normals, layout["probes"])
        np.testing.assert_allclose(weights.sum(axis=1), 1.0)
        self.assertTrue(np.all((weights > 0).sum(axis=1) <= 2))

    def test_the_smaller_volume_wins_inside_it_and_fades_out_past_it(self):
        layout = self.layout()
        points = np.array([(1.0, 2.0, 0.0), (3.3, 2.0, 0.0), (4.0, 2.0, 0.0)])
        weights = rps.blend_weights(points, np.tile((0, 0, 1.0), (3, 1)), layout["probes"])
        self.assertEqual(weights[0, 0], 1.0)             # inside its influence
        self.assertTrue(0 < weights[1, 0] < 1)           # within the fade
        self.assertEqual(weights[2, 0], 0.0)             # past the fade: global only

    def test_a_capture_behind_the_surface_gets_no_weight(self):
        layout = self.layout()
        # A wall facing -x at x = 2.9 (inside probe 0's influence): its
        # capture at x = 1.5 is in front; one facing +x has it behind.
        weights = rps.blend_weights(np.array([(2.9, 2.0, 1.0), (2.9, 2.0, 1.0)]),
                                    np.array([(-1.0, 0, 0), (1.0, 0, 0)]), layout["probes"])
        self.assertEqual(weights[0, 0], 1.0)
        self.assertEqual(weights[1, 0], 0.0)

    def test_walking_across_the_boundary_is_continuous_and_snapping_is_not(self):
        layout = self.layout()
        points, normals = self.walk()
        reflected = np.tile(unit((0.3, 0.2, 1.0)), (len(points), 1))
        blended = rps.shade(points, normals, reflected, 0.0, layout)
        snapped = rps.shade(points, normals, reflected, 0.0, layout, rps.MODE_NEAREST)
        step = np.abs(np.diff(blended, axis=0)).max(axis=1)
        snap_step = np.abs(np.diff(snapped, axis=0)).max(axis=1)
        # The largest blended step is the stripes' own texture edges; the
        # snapped walk adds a jump where the nearest capture changes.
        weights = rps.blend_weights(points, normals, layout["probes"])
        # Continuous: no step exceeds the smoothstep's steepest slope (1.5 /
        # fade) over one walk step.
        walk_step = points[1, 0] - points[0, 0]
        self.assertLess(np.abs(np.diff(weights, axis=0)).max(), 1.5 / 0.5 * walk_step * 1.01)
        nearest = rps.blend_weights(points, normals, layout["probes"], rps.MODE_NEAREST)
        self.assertEqual(np.abs(np.diff(nearest, axis=0)).max(), 1.0)
        self.assertTrue(np.isfinite(step).all() and np.isfinite(snap_step).all())

    def test_three_equal_shares_do_not_divide_by_zero(self):
        probes = [{"capture": np.zeros(3), "fade": 1.0, "box_min": -np.ones(3),
                   "box_max": np.ones(3), "influence_min": -np.ones(3) * k,
                   "influence_max": np.ones(3) * k, "rank": r, "global": g}
                  for r, (k, g) in enumerate(((1.0, False), (1.0, False), (2.0, True)))]
        weights = rps.blend_weights(np.array([(0.0, 0, 0)]), np.array([(0, 0, 1.0)]), probes)
        np.testing.assert_allclose(weights.sum(), 1.0)


class MirrorFloorOracleTest(unittest.TestCase):
    """The reflection a mirror floor shows, from points away from the probe:
    ground truth by tracing the reflected ray in the scene, against the probe
    lookup the shader performs. The fitted box must beat direction-only
    sampling, and a deliberately wrong box must fail."""

    def setUp(self):
        self.scene = rps.BoxScene(rooms=[ROOM])
        self.capture = np.array((3.0, 2.0, 1.6))
        rng = np.random.default_rng(3)
        count = 400
        eye = np.array((0.8, 0.6, 1.6))  # a camera in a corner, far from the probe
        self.points = np.column_stack((rng.uniform(1.0, 5.8, count), rng.uniform(0.3, 3.7, count),
                                       np.zeros(count)))
        view = unit(self.points - eye)
        self.reflected = view * (1, 1, -1)
        distance, _ = self.scene(self.points, self.reflected, 1e4)
        self.truth = stripes(self.points + distance[:, None] * self.reflected)
        self.chain = probe_chain(self.scene, self.capture, 512)

    def error(self, box_min, box_max, parallax=True):
        probe = {"capture": self.capture, "box_min": np.asarray(box_min, float),
                 "box_max": np.asarray(box_max, float)}
        radiance = rps.probe_radiance(self.points, self.reflected, np.zeros(len(self.points)),
                                      probe, self.chain, parallax)
        return float(np.mean(np.abs(radiance - self.truth)))

    def test_fitted_box_beats_direction_only_and_a_wrong_box_fails(self):
        box_min, box_max, _ = fit(self.scene, self.capture)
        fitted = self.error(box_min, box_max)
        direction = self.error(box_min, box_max, parallax=False)
        wrong = self.error(np.array(ROOM[0]) + (1.0, 0.8, 0.0), np.array(ROOM[1]) + (1.0, 0.8, 0.6))
        self.assertLess(fitted, 0.06)
        self.assertGreater(direction, 3 * fitted)
        self.assertGreater(wrong, 3 * fitted)


class RelightOracleTest(unittest.TestCase):
    """R50-RELIGHT: a mirror floor reflecting a Lambertian room baked under a
    uniform diffuse light, after a point light is added. Ground truth traces
    the reflected ray and shades the hit analytically (albedo times the baked
    light plus the point light's irradiance / pi); the probe, relit through
    its relight bands by the same change, must match it to within the
    lookup's own error on the baked room (stripe edges under bilinear
    filtering), while the unrelit probe and one whose distances are halved
    (the point it saw misplaced) must not."""

    BAKED = 0.4
    LIGHT = np.array((4.2, 1.1, 2.2))
    INTENSITY = 1.5

    def albedo(self, points):
        return 0.8 * stripes(points)

    def change(self, points, normals):
        to_light = self.LIGHT - points
        distance2 = np.sum(to_light * to_light, axis=1)
        cosine = np.maximum(np.sum(normals * to_light, axis=1) / np.sqrt(distance2), 0.0)
        return np.repeat((self.INTENSITY * cosine / distance2)[:, None], 3, axis=1)

    def setUp(self):
        self.scene = rps.BoxScene(rooms=[ROOM])
        self.capture = np.array((3.0, 2.0, 1.6))
        rng = np.random.default_rng(5)
        count = 400
        eye = np.array((0.8, 0.6, 1.6))
        self.points = np.column_stack((rng.uniform(1.0, 5.8, count), rng.uniform(0.3, 3.7, count),
                                       np.zeros(count)))
        self.reflected = unit(self.points - eye) * (1, 1, -1)
        distance, normal = self.scene(self.points, self.reflected, 1e4)
        hits = self.points + distance[:, None] * self.reflected
        self.truth = self.albedo(hits) * (self.BAKED + self.change(hits, normal))
        width = 512
        directions = reflection_probe.equirect_directions(width).reshape(-1, 3)
        seen, normals = self.scene(np.tile(self.capture, (len(directions), 1)), directions, 1e4)
        points = self.capture + seen[:, None] * directions
        albedo = self.albedo(points)
        shape = (width // 2, width)
        self.chain = [(albedo * self.BAKED).reshape(*shape, 3)]
        first, second = rps.relight_chain(albedo.reshape(*shape, 3), normals.reshape(*shape, 3),
                                          seen.reshape(shape))
        self.bands = ([first[0]], [second[0]])
        self.probe = {"capture": self.capture, "box_min": np.array(ROOM[0], float),
                      "box_max": np.array(ROOM[1], float)}

    def light(self, points, normals):
        """The diffuse light now (baked plus the point light) and as baked."""
        baked = np.full((len(points), 3), self.BAKED)
        return baked + self.change(points, normals), baked

    def error(self, bands, light, truth=None):
        radiance = rps.probe_radiance(self.points, self.reflected, np.zeros(len(self.points)),
                                      self.probe, self.chain, bands=bands, light=light)
        truth = self.truth if truth is None else truth
        return float(np.mean(np.abs(radiance - truth)) / np.mean(truth))

    def test_relit_probe_matches_the_lit_room_and_the_controls_fail(self):
        distance, _ = self.scene(self.points, self.reflected, 1e4)
        hits = self.points + distance[:, None] * self.reflected
        baked = self.albedo(hits) * self.BAKED
        lookup = rps.probe_radiance(self.points, self.reflected, np.zeros(len(self.points)),
                                    self.probe, self.chain)
        floor = float(np.mean(np.abs(lookup - baked)) / np.mean(baked))
        relit = self.error(self.bands, self.light)
        unrelit = self.error(None, None)
        halved = [level * (1.0, 1.0, 1.0, 0.5) for level in self.bands[0]]
        misplaced = self.error((halved, self.bands[1]), self.light)
        self.assertLess(relit, floor + 0.02)
        self.assertGreater(unrelit, 4 * relit)
        self.assertGreater(misplaced, 2 * relit)

    def test_removed_light_scales_the_capture_and_the_additive_control_fails(self):
        """The capture holds a bright beam (a patch of direct light through a
        doorway) that the smooth diffuse-light field does not resolve. When
        the room's light drops to a tenth, the relit probe is the capture
        times the drop, beam included; adding albedo times the field's
        (negative) change instead leaves most of the beam lit."""
        width = 512
        directions = reflection_probe.equirect_directions(width).reshape(-1, 3)
        seen, _ = self.scene(np.tile(self.capture, (len(directions), 1)), directions, 1e4)
        points = self.capture + seen[:, None] * directions

        def beam(hits):
            return 1.0 + 4.0 * ((hits[:, 0] > 6.0 - 1e-6) & (hits[:, 2] < 1.5))[:, None]
        chain = [(self.albedo(points) * self.BAKED * beam(points)).reshape(width // 2, width, 3)]

        def dimmed(points, normals):
            baked = np.full((len(points), 3), self.BAKED)
            return 0.1 * baked, baked
        distance, _ = self.scene(self.points, self.reflected, 1e4)
        hits = self.points + distance[:, None] * self.reflected
        truth = 0.1 * self.albedo(hits) * self.BAKED * beam(hits)
        roughness = np.zeros(len(self.points))
        relit = rps.probe_radiance(self.points, self.reflected, roughness, self.probe, chain,
                                   bands=self.bands, light=dimmed)
        baked = rps.probe_radiance(self.points, self.reflected, roughness, self.probe, chain)
        albedo = self.albedo(hits)
        additive = np.maximum(baked - albedo * 0.9 * self.BAKED, 0.0)
        lit = beam(hits)[:, 0] > 1.0
        self.assertGreater(lit.sum(), 20)

        def error(radiance):
            return float(np.mean(np.abs(radiance - truth)) / np.mean(truth))
        self.assertLess(error(relit), 0.1)
        self.assertGreater(error(additive), 5 * error(relit))
        # The rule itself: removal is exactly relative, addition absolute.
        np.testing.assert_allclose(
            rps.relight(np.array([[0.2, 0.2, 0.2]]), np.array([[0.5, 0.5, 0.5]]),
                        np.array([[0.1, 0.4, 0.4]]), np.array([[0.4, 0.4, 0.2]])),
            [[0.05, 0.2, 0.3]])

    def test_an_occluder_hides_what_the_capture_saw(self):
        """A slab between the capture and the far wall: lookups whose seen
        point lies behind it take the slab's face (its reflectance times the
        light now there), the others are relit as without it."""
        slab = ((4.5, 0.0, 0.0), (4.7, 4.0, 3.0), 0.3)
        radiance = rps.probe_radiance(self.points, self.reflected, np.zeros(len(self.points)),
                                      self.probe, self.chain, bands=self.bands,
                                      light=self.light, occluders=[slab])
        free = rps.probe_radiance(self.points, self.reflected, np.zeros(len(self.points)),
                                  self.probe, self.chain, bands=self.bands, light=self.light)
        direction = rps.reflection_probe.corrected_direction(
            rps.reflection_probe.parallax_lookup(self.points, self.reflected, self.capture,
                                                 self.probe["box_min"],
                                                 self.probe["box_max"])[0],
            self.reflected, np.zeros(len(self.points)))
        albedo = rps.reflection_probe.sample_chain(self.bands[0], direction,
                                                   np.zeros(len(self.points)), channels=4)
        hidden, t, face, _ = rps.occluded_segment(self.capture, direction, albedo[:, 3], [slab])
        self.assertGreater(hidden.sum(), 20)
        self.assertGreater((~hidden).sum(), 20)
        np.testing.assert_array_equal(radiance[~hidden], free[~hidden])
        entry = self.capture + direction[hidden] * t[hidden, None]
        np.testing.assert_allclose(entry[:, 0], 4.5, atol=1e-9)
        np.testing.assert_allclose(face[hidden], np.tile((-1.0, 0.0, 0.0), (hidden.sum(), 1)))
        now, _ = self.light(entry, face[hidden])
        np.testing.assert_allclose(radiance[hidden], 0.3 * now)
        # Inside the slab the capture sees nothing else: the face is at t = 0.
        inside, t0, back, _ = rps.occluded_segment(np.array((4.6, 2.0, 1.0)),
                                                   np.array([[1.0, 0.0, 0.0]]), [5.0], [slab])
        self.assertTrue(inside[0])
        self.assertEqual(t0[0], 0.0)
        np.testing.assert_array_equal(back[0], (-1.0, 0.0, 0.0))

    def test_zero_change_is_the_baked_probe_exactly(self):
        zero = rps.probe_radiance(self.points, self.reflected, np.zeros(len(self.points)),
                                  self.probe, self.chain, bands=self.bands,
                                  light=lambda p, n: (np.full((len(p), 3), self.BAKED),
                                                      np.full((len(p), 3), self.BAKED)))
        baked = rps.probe_radiance(self.points, self.reflected, np.zeros(len(self.points)),
                                   self.probe, self.chain)
        np.testing.assert_array_equal(zero, baked)


class RelightFormatTest(unittest.TestCase):
    def test_v2_round_trip_and_its_corpus(self):
        probes, chains = rps.fixture_layout()
        data = rps.build(probes, chains, relight=rps.fixture_relight(probes))
        layout = rps.read(data)
        self.assertEqual(struct.unpack_from("<I", data, 4)[0], rps.VERSION)
        self.assertEqual(len(layout["relight"]), 2)
        texture = rps.gpu_texture(layout)
        self.assertEqual(float(texture[0, 1, 1]), 1.0)
        self.assertEqual(float(rps.gpu_texture(layout, relight=False)[0, 1, 1]), 0.0)
        for offset, fmt, value, code in rps.RELIGHT_MALFORMATIONS:
            edited = bytearray(data)
            struct.pack_into(fmt, edited, offset, value)
            with self.assertRaises(rps.RprbError) as caught:
                rps.read(bytes(edited))
            self.assertEqual(caught.exception.code, code, (offset, value))

    def test_v1_stays_readable_without_bands(self):
        probes, chains = rps.fixture_layout()
        layout = rps.read(rps.build(probes, chains))
        self.assertIsNone(layout["relight"])
        self.assertEqual(float(rps.gpu_texture(layout)[0, 1, 1]), 0.0)


class CapacityTest(unittest.TestCase):
    def test_priority_is_serialized_as_rank_and_global_remains_last(self):
        probes = [{"influence_min": np.zeros(3), "influence_max": np.ones(3) * size,
                   "priority": priority, "global": global_probe}
                  for size, priority, global_probe in [(1, 0, False), (2, 1, False),
                                                       (3, 99, True)]]
        self.assertEqual(rps.ranks(probes), [1, 0, 2])

    def test_tiled_gpu_contains_every_band_without_loss(self):
        for relight in (False, True):
            data = rps.capacity_fixture(relight)
            layout = rps.read(data)
            texture = rps.gpu_texture(layout)
            columns = rps.gpu_columns(layout)
            self.assertGreater(columns, 1)
            # Independently reconstruct the serialized atlas one band at a time.
            band, width = layout["width"] // 2, layout["atlas_width"]
            restored = np.concatenate([
                texture[65 + (i // columns) * band:65 + (i // columns + 1) * band,
                        (i % columns) * width:(i % columns + 1) * width]
                for i in range(layout["atlas_height"] // band)])
            self.assertEqual(restored.tobytes(), layout["atlas"].astype("<f2").tobytes())
            weights = rps.blend_weights(np.array([[1, 2, 0]]) * rps.SOURCE_UNITS_PER_METER,
                                       np.array([[0, 0, 1]]), layout["probes"])
            self.assertEqual(int(np.argmax(weights)), 62)
            self.assertEqual(float(weights[0, 62]), 1.0)
            self.assertEqual(layout["probes"][62]["rank"], 62)
            self.assertEqual(layout["probes"][63]["rank"], 63)
            table = rps.texture_table(texture)
            self.assertEqual(table[62, 1, 3], 62)  # band index, not a half-float row offset
            if relight:
                self.assertEqual(table[63, 3, 3], 190)


class VersionTest(unittest.TestCase):
    def test_only_v6_is_read(self):
        data = bytearray(rps.capacity_fixture())
        self.assertEqual(struct.unpack_from("<I", data, 4)[0], rps.VERSION)
        for version in range(1, 6):
            struct.pack_into("<I", data, 4, version)
            with self.assertRaises(rps.RprbError) as error:
                rps.read(bytes(data))
            self.assertEqual(error.exception.code, "UnsupportedVersion")

    def test_every_lump_has_a_grid_of_the_right_width(self):
        for count in (2, 64, 65, 256):
            self.assertEqual(rps.candidate_words(count), (count + 63) // 64)
        layout = rps.read(rps.capacity_fixture())
        self.assertEqual(layout["candidates"]["masks"].shape, (rps.CANDIDATE_CELLS, 1))


class RegridTest(unittest.TestCase):
    """A probe whose fitted box reaches past the world (a depth fit that saw
    the void) stretched the 16^3 candidate grid over the whole map."""

    def setUp(self):
        layout = rps.read(rps.capacity_fixture())
        probes = [dict(probe) for probe in layout["probes"]]
        local = next(i for i, probe in enumerate(probes) if not probe["global"] and probe["rank"] >= 2)
        probes[local]["influence_min"] = probes[local]["influence_min"] - 40000.0
        probes[local]["influence_max"] = probes[local]["influence_max"] + 40000.0
        self.data = rps.build(probes, layout["chains"], scale=1)
        others = [p for p in probes if not p["global"] and p is not probes[local]]
        self.bounds = (np.min([p["influence_min"] for p in others], axis=0),
                       np.max([p["influence_max"] for p in others], axis=0))

    def test_bounds_shrink_cells_and_keep_every_other_byte(self):
        out = rps.regrid(self.data, self.bounds)
        before, after = rps.read(self.data)["candidates"], rps.read(out)["candidates"]
        self.assertLess(after["step"], before["step"])
        self.assertLess(max(rps.candidates_per_cell(out)), max(rps.candidates_per_cell(self.data)))
        layout = rps.read(self.data)
        base = (rps.HEADER_BYTES + rps.RECORD_BYTES * layout["count"] + 15) & ~15
        self.assertEqual(len(out), len(self.data))
        self.assertEqual(out[:base], self.data[:base])
        self.assertEqual(out[layout["atlas_offset"]:], self.data[layout["atlas_offset"]:])

    def test_bounded_masks_hold_every_weighted_probe(self):
        out = rps.read(rps.regrid(self.data, self.bounds))
        grid, probes = out["candidates"], out["probes"]
        masks = grid["masks"].reshape(rps.CANDIDATE_CELLS, -1)
        rng = np.random.default_rng(7)
        # Points inside every probe's own reach (clipped to the bounds), so
        # each probe is weighted somewhere in the sample.
        points = np.concatenate([
            rng.uniform(np.maximum(p["influence_min"], self.bounds[0]),
                        np.minimum(p["influence_max"], self.bounds[1]), size=(6, 3))
            for p in probes if np.all(np.minimum(p["influence_max"], self.bounds[1]) >
                                      np.maximum(p["influence_min"], self.bounds[0]))])
        normals = rng.normal(size=points.shape)
        normals /= np.linalg.norm(normals, axis=1, keepdims=True)
        weights = rps.blend_weights(points, normals, probes)
        for point, row in zip(points, weights):
            cell = np.floor((point - grid["origin"]) / grid["step"]).astype(int)
            if np.any(cell < 0) or np.any(cell >= rps.CANDIDATE_DIM):
                continue  # outside the grid every rank is a candidate
            dim = rps.CANDIDATE_DIM
            mask = masks[cell[0] + dim * (cell[1] + dim * cell[2])]
            for index in np.nonzero(row)[0]:
                rank = probes[index]["rank"]
                self.assertTrue(int(mask[rank // 64]) >> (rank % 64) & 1,
                                "rank %d weighs %g at %s but is no candidate" % (rank, row[index], point))

    def test_the_bake_and_regrid_paths_agree(self):
        layout = rps.read(self.data)
        baked = rps.build(layout["probes"], layout["chains"], scale=1,
                          candidate_bounds=self.bounds)
        regridded = rps.regrid(self.data, self.bounds)
        # The same records and grid; the atlas is the input's own blocks (a
        # rebuild re-encodes decoded light, which BC6H does not reproduce).
        end = layout["atlas_offset"]
        self.assertEqual(baked[:end], regridded[:end])
        self.assertEqual(regridded[end:], self.data[end:])

    def test_bounds_missing_every_probe_are_refused(self):
        with self.assertRaises(rps.RprbError):
            rps.regrid(self.data, (np.full(3, 9e5), np.full(3, 9.5e5)))


class PlacementTest(unittest.TestCase):
    def authored(self):
        return {"capture": [3, 2, 1.6], "box_min": list(ROOM[0]), "box_max": list(ROOM[1]),
                "influence_min": list(ROOM[0]), "influence_max": list(ROOM[1]),
                "fade": 0.5, "priority": 2, "name": "room"}

    def test_authored_proxy_and_influence_survive_placement(self):
        volume = self.authored()
        probes, report = rps.place(rps.BoxScene(rooms=[ROOM]), (0, 0, -0.5), (6, 4, 2.9),
                                   params={"spacing_m": 1, "fit_rays": 64}, volumes=[volume])
        self.assertEqual(report["uncovered_walkable"], 0)
        self.assertEqual(len(probes), 1)
        self.assertEqual(probes[0]["priority"], 2)
        self.assertEqual(probes[0]["role"], "authored")
        for key in ("capture", "box_min", "box_max", "influence_min", "influence_max"):
            np.testing.assert_array_equal(probes[0][key], volume[key])

    def test_bad_authoring_fails_before_raycast(self):
        def no_rays(*args):
            self.fail("invalid authoring reached placement raycasts")
        for change in ({"capture": [99, 0, 0]}, {"box_min": [6, 4, 3]},
                       {"influence_max": [0, 0, 0]}, {"fade": float("nan")},
                       {"priority": 0.5}, {"global": "false"}):
            with self.subTest(change=change), self.assertRaises(ValueError):
                rps.place(no_rays, ROOM[0], ROOM[1], volumes=[dict(self.authored(), **change)])
        for cap in (0, 65):
            with self.assertRaises(ValueError):
                rps.place(no_rays, ROOM[0], ROOM[1], params={"max_probes": cap})
        with self.assertRaises(ValueError):
            rps.place(no_rays, ROOM[0], ROOM[1],
                      volumes=[dict(self.authored(), **{"global": True})] * 2)

    def two_rooms(self):
        # Room A 6 x 4, room B 5 x 4 beyond a wall at x = 6 with a doorway.
        return rps.BoxScene(rooms=[ROOM, ((6.2, 0.0, 0.0), (11.2, 4.0, 3.0))],
                            solids=[((6.0, -0.1, 0.0), (6.2, 4.1, 3.0))],
                            openings=[((5.9, 1.5, 0.0), (6.3, 2.5, 2.1))])

    def test_each_room_gets_a_probe_inside_it(self):
        scene = self.two_rooms()
        probes, report = rps.place(scene, (0.0, 0.0, -0.5), (11.2, 4.0, 2.9),
                                   params={"spacing_m": 0.5, "fit_rays": 512})
        rooms = [p for p in probes if p["role"] == "joint"]
        self.assertEqual(report["uncovered_walkable"], 0)
        self.assertEqual(len(rooms), 2)
        xs = sorted(p["capture"][0] for p in rooms)
        self.assertLess(xs[0], 6.0)
        self.assertGreater(xs[1], 6.2)
        self.assertEqual(sum(bool(p.get("global")) for p in probes), 1)
        for probe in rooms:
            self.assertAlmostEqual(probe["capture"][2], 1.63, places=6)

    def test_candidate_fit_batch_is_the_serial_oracle(self):
        class BatchedScene(rps.BoxScene):
            def __init__(self, **kwargs):
                super().__init__(**kwargs)
                self.batch_called = False

            def estimate_boxes(self, captures, directions):
                self.batch_called = True
                return [rps.estimate_box(self, capture, directions) for capture in captures]

        serial = self.two_rooms()
        batched = BatchedScene(rooms=serial.rooms, solids=serial.solids,
                               openings=serial.openings)
        bounds = (0.0, 0.0, -0.5), (11.2, 4.0, 2.9)
        settings = {"spacing_m": 1.0, "fit_rays": 128}
        expected, expected_report = rps.place(serial, *bounds, params=settings)
        actual, actual_report = rps.place(batched, *bounds, params=settings)
        self.assertTrue(batched.batch_called)
        self.assertEqual(actual_report, expected_report)
        self.assertEqual(len(actual), len(expected))
        for got, want in zip(actual, expected):
            self.assertEqual(set(got), set(want))
            for key in got:
                if isinstance(got[key], np.ndarray):
                    np.testing.assert_array_equal(got[key], want[key])
                else:
                    self.assertEqual(got[key], want[key])

    def test_an_open_side_ends_at_the_probes_own_room(self):
        # A 6 x 4 m room whose whole +x wall is open to the sky: the fit
        # reaches OPEN_EXTENT there, and placement must cut it back to the
        # floor the probe covers (plus the fade), not a kilometre away.
        scene = rps.BoxScene(rooms=[((0.0, 0.0, 0.0), (6.0, 4.0, 3.0))],
                             openings=[((5.9, -0.1, -0.1), (6.1, 4.1, 3.1))])
        probes, _ = rps.place(scene, (0.0, 0.0, -0.5), (6.0, 4.0, 2.9),
                              params={"spacing_m": 0.5, "fit_rays": 512})
        fade = rps.PLACEMENT_DEFAULTS["fade_m"]
        margin = rps.PLACEMENT_DEFAULTS["influence_margin_m"]
        placed = [p for p in probes if p["role"] == "joint"]
        self.assertTrue(placed)
        for probe in placed:
            self.assertLessEqual(probe["box_max"][0], 6.0 + fade + 1e-6)
            self.assertLessEqual(probe["influence_max"][0], 6.0 + fade + margin + 1e-6)
            self.assertEqual(probe["box_max"][2], 3.0)  # the closed ceiling is untouched

    def test_closed_faces_are_unchanged_by_the_room_bound(self):
        fit = {"faces": {"px": {"open": False}}}
        box = (np.array([0.0, 0.0, 0.0]), np.array([5.0, 4.0, 3.0]))
        out = rps.room_bounded(box, fit, np.array([[1.0, 1.0, 1.6]]), np.array([1.0, 1.0, 1.6]),
                               np.zeros(3), np.array([5.0, 4.0, 3.0]), 0.8)
        self.assertEqual(out[0].tolist(), box[0].tolist())
        self.assertEqual(out[1].tolist(), box[1].tolist())

    def test_no_probe_straddles_a_doorway(self):
        # The two-rooms fixture's geometry: 5 m rooms, a 0.2 m wall and a
        # 1.2 m doorway. A capture in front of the doorway fits a box through
        # it; placement must give each room a probe whose box is its room.
        a, b = ((0.0, 0.0, 0.0), (5.0, 4.0, 3.0)), ((5.2, 0.0, 0.0), (10.2, 4.0, 3.0))
        door = ((5.0, 1.4, 0.0), (5.2, 2.6, 2.2))
        scene = rps.BoxScene(rooms=[a, b, door],
                             openings=[((4.9, 1.4, -1.0), (5.3, 2.6, 2.2))])
        probes, report = rps.place(scene, (0.0, 0.0, -0.5), (10.2, 4.0, 2.9),
                                   params={"spacing_m": 0.75, "fit_rays": 512})
        self.assertEqual(report["uncovered_walkable"], 0)
        rooms = sorted((p for p in probes if p["role"] == "joint"),
                       key=lambda p: p["capture"][0])
        self.assertEqual(len(rooms), 2)
        self.assertLessEqual(rooms[0]["box_max"][0], 5.05)
        self.assertGreaterEqual(rooms[1]["box_min"][0], 5.15)
        for probe in rooms:
            self.assertGreaterEqual(min(probe["capture"][0] - probe["box_min"][0],
                                        probe["box_max"][0] - probe["capture"][0]), 0.5)

    def test_a_mirror_far_from_the_room_probe_gets_its_own(self):
        scene = rps.BoxScene(rooms=[((0.0, 0.0, 0.0), (12.0, 4.0, 3.0))])
        # A mirror on the -x wall's end, 1 x 1.5 m at eye height.
        mirror = np.array([[(0.001, 1.5, 0.8), (0.001, 2.5, 0.8), (0.001, 2.5, 2.3)],
                           [(0.001, 1.5, 0.8), (0.001, 2.5, 2.3), (0.001, 1.5, 2.3)]])
        points, normals = rps.glossy_samples(mirror, 64.0)
        normals = np.tile((1.0, 0, 0), (len(points), 1))
        probes, report = rps.place(scene, (0.0, 0.0, -0.5), (12.0, 4.0, 2.9),
                                   glossy=(points, normals),
                                   params={"spacing_m": 0.5, "fit_rays": 512})
        glossy = [p for p in probes if p["role"] == "glossy"]
        self.assertEqual(report["unserved_glossy"], 0)
        self.assertEqual(len(glossy), 1)
        self.assertLess(glossy[0]["capture"][0], 2.5)
        self.assertLess(glossy[0]["influence_max"][0], 0.5)

    def test_glossy_sampler_reports_its_source_triangle(self):
        triangles = np.array([
            [(0, 0, 0), (2, 0, 0), (0, 2, 0)],
            [(0, 0, 1), (0, 2, 1), (2, 0, 1)],
            [(0, 0, 2), (0, 0, 2), (0, 0, 2)],  # filtered zero area
        ], dtype=float)
        points, normals, indices = rps.glossy_samples(
            triangles, 8.0, seed=7, return_indices=True)
        self.assertEqual(len(points), len(normals))
        self.assertEqual(len(points), len(indices))
        self.assertTrue(set(indices) <= {0, 1})
        for normal, index in zip(normals, indices):
            expected = np.cross(triangles[index, 1] - triangles[index, 0],
                                triangles[index, 2] - triangles[index, 0])
            expected /= np.linalg.norm(expected)
            np.testing.assert_array_equal(normal, expected)

    def test_a_walkable_cap_widens_the_grid_and_still_covers_each_room(self):
        # sp_a2_core_relit (2026-09-29): a whole retail map at 0.75 m has tens
        # of thousands of walkable samples and placement ran for hours.
        scene = self.two_rooms()
        bounds = (0.0, 0.0, -0.5), (11.2, 4.0, 2.9)
        _, full = rps.place(scene, *bounds, params={"spacing_m": 0.25, "fit_rays": 256})
        probes, capped = rps.place(scene, *bounds, params={"spacing_m": 0.25, "fit_rays": 256,
                                                           "max_walkable": 60})
        self.assertGreater(full["walkable_samples"], 60)
        self.assertLessEqual(capped["walkable_samples"], 60)
        self.assertGreater(capped["walkable_spacing_m"], 0.25)
        self.assertEqual(full["walkable_spacing_m"], 0.25)
        self.assertEqual(capped["uncovered_walkable"], 0)
        xs = sorted(p["capture"][0] for p in probes if p["role"] == "joint")
        self.assertEqual(len(xs), 2)
        self.assertLess(xs[0], 6.0)
        self.assertGreater(xs[1], 6.2)


if __name__ == "__main__":
    unittest.main()
