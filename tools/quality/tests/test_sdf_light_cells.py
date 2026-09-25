"""SDFV light cells (sdf_light_cells.py): conservative culling by range, side
and visibility, with a seeded wrong variant the property test must catch; and
the traced producers' probe focus (gi_focus.py)."""

import math
import sys
import unittest
from pathlib import Path
from unittest import mock

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import gi_focus as focus_module  # noqa: E402
import sdf_light_cells as cells  # noqa: E402
import sdf_volume  # noqa: E402


def sphere(centre, radiance, radius=2.0, style=-1):
    return {"kind": "sphere", "style": style, "rgb": (radiance,) * 3, "a": centre,
            "b": (radius, 0.0, 0.0)}


def spot(centre, normal, radiance, radius=2.0):
    return {"kind": "spot", "style": -1, "rgb": (radiance,) * 3, "a": centre, "b": normal,
            "c": (radius, 0.9, 0.8), "d": (1.0, 0.0)}


def lights_at(result, point):
    origin = np.asarray(result["origin"])
    index = np.clip(np.floor((np.asarray(point) - origin) / result["size"]).astype(int), 0,
                    np.asarray(result["dims"]) - 1)
    return result["lists"][index[0] + result["dims"][0] * (index[1] + result["dims"][1] *
                                                            index[2])]


def two_rooms():
    """Leaves: room 0 (x 0..100), room 1 (x 100..200), a solid slab at x 100
    between; the PVS keeps them apart."""
    contents = [0, 0, 1]
    clusters = [0, 1, -1]
    mins = [(0, 0, 0), (101, 0, 0), (99, 0, 0)]
    maxs = [(99, 100, 100), (200, 100, 100), (101, 100, 100)]
    return cells.Visibility(contents, clusters, mins, maxs, np.eye(2, dtype=bool))


class LightCellTests(unittest.TestCase):
    def test_range_cull_and_style_headroom(self):
        # L r^2 / d^2 = 4 x 4 / d^2 >= 0.01 -> d <= 40 from the sphere's surface.
        result, report = cells.build([sphere((0, 0, 0), 4.0)], (0, 0, 0), (400, 10, 10), 10.0,
                                     0.01)
        self.assertEqual(lights_at(result, (30, 5, 5)), [0])
        self.assertEqual(lights_at(result, (100, 5, 5)), [])
        self.assertGreater(report["culled"]["range"], 0)
        styled, _ = cells.build([sphere((0, 0, 0), 4.0, style=32)], (0, 0, 0), (400, 10, 10),
                                10.0, 0.01)
        # 2.1x brighter: d <= 40 sqrt(2.1) = 58 from the surface.
        self.assertEqual(lights_at(styled, (55, 5, 5)), [0])

    def test_global_lights_reach_every_cell(self):
        lights = [{"kind": "distant", "style": -1, "rgb": (1, 1, 1), "a": (0, 0, -1)},
                  {"kind": "dome", "style": -1, "rgb": (0.1, 0.1, 0.1)}]
        result, _ = cells.build(lights, (0, 0, 0), (100, 100, 100), 25.0, 1e-3, two_rooms())
        self.assertTrue(all(entry == [0, 1] for entry in result["lists"]))

    def test_one_sided_lights_skip_cells_behind_them(self):
        result, report = cells.build([spot((50, 50, 50), (0, 0, -1), 100.0)], (0, 0, 0),
                                     (100, 100, 100), 20.0, 1e-3)
        self.assertEqual(lights_at(result, (50, 50, 30)), [0])
        self.assertEqual(lights_at(result, (50, 50, 90)), [])
        self.assertGreater(report["culled"]["side"], 0)

    def test_visibility_keeps_rooms_apart(self):
        lights = [sphere((50, 50, 50), 100.0), sphere((150, 50, 50), 100.0)]
        result, report = cells.build(lights, (0, 0, 0), (200, 100, 100), 25.0, 1e-4, two_rooms())
        self.assertEqual(lights_at(result, (20, 50, 50)), [0])
        self.assertEqual(lights_at(result, (180, 50, 50)), [1])
        self.assertGreater(report["culled"]["visibility"], 0)
        # Without visibility both reach everywhere at this cutoff.
        blind, _ = cells.build(lights, (0, 0, 0), (200, 100, 100), 25.0, 1e-4)
        self.assertEqual(lights_at(blind, (180, 50, 50)), [0, 1])

    def test_a_buried_light_and_an_unclustered_leaf_see_everything(self):
        # A light inside the slab touches both rooms' leaves (their bounds
        # carry a margin), so both rooms see it.
        buried = sphere((100, 50, 50), 100.0, radius=0.5)
        result, _ = cells.build([buried], (0, 0, 0), (200, 100, 100), 25.0, 1e-4, two_rooms())
        self.assertEqual(lights_at(result, (20, 50, 50)), [0])
        self.assertEqual(lights_at(result, (180, 50, 50)), [0])
        # One touching no open leaf at all sees everything.
        closed = cells.Visibility([1], [-1], [(0, 0, 0)], [(10, 10, 10)], np.eye(1, dtype=bool))
        self.assertIsNone(closed.seen_from(np.zeros(3), np.ones(3)))
        # The slab leaf is solid and was dropped; a leaf with cluster -1 is
        # open to every light.
        free = cells.Visibility([0, 0], [0, -1], [(0, 0, 0), (101, 0, 0)],
                                [(99, 100, 100), (200, 100, 100)], np.eye(1, dtype=bool))
        result, _ = cells.build([sphere((50, 50, 50), 100.0)], (0, 0, 0), (200, 100, 100),
                                25.0, 1e-4, free)
        self.assertEqual(lights_at(result, (180, 50, 50)), [0])

    def conservative(self, seed=7):
        """Random lights and points: a light whose true light at a point
        reaches the cutoff must be listed in the point's cell."""
        rng = np.random.default_rng(seed)
        lights = [sphere(tuple(rng.uniform(0, 300, 3)), float(rng.uniform(1, 50)),
                         radius=float(rng.uniform(0.5, 4))) for _ in range(12)]
        result, _ = cells.build(lights, (0, 0, 0), (300, 300, 300), 40.0, 0.01)
        misses = 0
        for point in rng.uniform(0, 300, (4000, 3)):
            listed = set(lights_at(result, point))
            for index, light in enumerate(lights):
                distance = max(np.linalg.norm(point - light["a"]) - light["b"][0], 1e-6)
                if light["rgb"][0] * light["b"][0] ** 2 / distance ** 2 >= 0.01 and \
                        index not in listed:
                    misses += 1
        return misses

    def test_lists_are_conservative(self):
        self.assertEqual(self.conservative(), 0)

    def test_the_property_catches_culling_by_cell_centre(self):
        def centre_distance(point, lo, hi):
            return np.linalg.norm((lo + hi) / 2 - point, axis=1)
        with mock.patch.object(cells, "box_distance", centre_distance):
            self.assertGreater(self.conservative(), 0)

    def test_cells_round_trip_through_the_sdfv(self):
        lights = [sphere((50, 50, 50), 100.0), sphere((150, 50, 50), 100.0)]
        result, _ = cells.build(lights, (-8, -8, -8), (200, 100, 100), 25.0, 1e-4, two_rooms())
        shape = (3, 3, 3)
        data = sdf_volume.build([0.0, 0.0, 0.0], 8.0, [3, 3, 3], np.full(shape, 8.0),
                                np.zeros(shape + (3,)), np.zeros(shape + (3,)),
                                np.full(shape, sdf_volume.NO_SOURCE), lights, 16.0, result)
        volume = sdf_volume.Volume(data)
        for point in ((20, 50, 50), (180, 50, 50), (99, 10, 10)):
            self.assertEqual(volume.cell_lights(point), lights_at(result, point))

    def test_invalid_parameters(self):
        with self.assertRaises(ValueError):
            cells.build([], (0, 0, 0), (1, 1, 1), 0.0, 1e-3)
        self.assertTrue(math.isinf(cells.light_reach(
            {"kind": "dome", "rgb": (1, 1, 1)})[2]))




class FocusTests(unittest.TestCase):
    """gi_focus.py, the host ProbeFocus's independent implementation."""

    class Bsp:
        # Rooms 0 | 1 | 2 in a row (x 0..100, 101..200, 201..300) and room 3
        # apart (x 400..500); room 0 sees room 1, room 2 only touches room 1.
        contents = np.array([0, 0, 0, 0])
        clusters = np.array([0, 1, 2, 3])
        mins = np.array([(0, 0, 0), (101, 0, 0), (201, 0, 0), (400, 0, 0)], float)
        maxs = np.array([(99, 100, 100), (199, 100, 100), (300, 100, 100), (500, 100, 100)],
                        float)

        def __init__(self, vis=True):
            self.vis = vis

        def visibility(self):
            if not self.vis:
                return None
            pvs = np.eye(4, dtype=bool)
            pvs[0, 1] = pvs[1, 0] = True
            return pvs

        def leaves(self):
            return self.contents, self.clusters, self.mins, self.maxs

        def point_leaf(self, point):
            return int(np.flatnonzero((self.mins[:, 0] - 1 <= point[0]) &
                                      (point[0] <= self.maxs[:, 0] + 1))[0])

    def focus_at(self, eye, vis=True, radius=150.0):
        positions = np.array([(50, 50, 50), (150, 50, 50), (250, 50, 50), (450, 50, 50),
                              (460, 50, 50)], float)
        active = np.array([True, True, True, True, False])
        with mock.patch.object(focus_module, "probe_positions",
                               lambda volume: (positions, active)):
            return focus_module.focus(self.Bsp(vis), None, eye, radius)

    def test_visible_clusters_and_their_neighbours(self):
        # From room 0: rooms 0 and 1 are visible, room 2 touches room 1; room
        # 3 is neither. From room 3 only room 3.
        chosen, summary = self.focus_at((50, 50, 50))
        self.assertEqual((chosen, summary["mode"]), ([0, 1, 2], "visibility"))
        chosen, summary = self.focus_at((450, 50, 50))
        self.assertEqual((chosen, summary["mode"]), ([3], "visibility"))

    def test_the_fallback_uses_distance(self):
        # No visibility: the active probes within the radius.
        chosen, summary = self.focus_at((50, 50, 50), vis=False, radius=120.0)
        self.assertEqual((chosen, summary["mode"]), ([0, 1], "distance"))

    def test_neighbours_touch_within_a_unit(self):
        found = focus_module.neighbours(*self.Bsp().leaves())
        self.assertEqual([sorted(n) for n in found], [[1], [0, 2], [1], []])


if __name__ == "__main__":
    unittest.main()
