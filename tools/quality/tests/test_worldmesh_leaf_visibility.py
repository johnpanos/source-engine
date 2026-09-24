"""Tests for the conservative spatial leaf assignment of imported WMSH meshlets."""

from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import worldmesh_leaf_visibility as visibility

EMPTY, SOLID, DETAIL = 0, 1, 0x8000000

# Two rooms along x, separated and enclosed by solid (shell/void). Room A
# holds a structural furniture solid floating at x 10..20, y 10..50, z 10..50.
LEAVES = [
    (SOLID, (0, 0, 0), (0, 0, 0)),              # 0: BSP placeholder
    (EMPTY, (0, 0, 0), (10, 100, 100)),         # 1: room A, west
    (SOLID, (10, 10, 10), (20, 50, 50)),        # 2: furniture in room A
    (EMPTY, (10, 50, 0), (50, 100, 100)),       # 3: room A, north
    (EMPTY, (20, 0, 0), (50, 50, 100)),         # 4: room A, east
    (SOLID, (50, 0, 0), (70, 100, 100)),        # 5: wall between rooms (part of shell)
    (EMPTY, (70, 0, 0), (100, 100, 100)),       # 6: room B
    (SOLID, (-16, -16, -16), (116, 0, 116)),    # 7: shell south + void
    (SOLID, (-16, 100, -16), (116, 116, 116)),  # 8: shell north + void
    (SOLID, (-16, 0, -16), (0, 100, 116)),      # 9: shell west
    (SOLID, (100, 0, -16), (116, 100, 116)),    # 10: shell east
    (EMPTY, (10, 0, 0), (20, 10, 100)),         # 11: room A, south of furniture
    (EMPTY, (10, 10, 0), (20, 50, 10)),         # 12: room A, under furniture
    (EMPTY, (10, 10, 50), (20, 50, 100)),       # 13: room A, above furniture
]
SOLIDS = (0, 2, 5, 7, 8, 9, 10)


def refs(boxes):
    references, summary = visibility.assign(boxes, LEAVES)
    per_meshlet = [sorted(leaf for leaf, items in enumerate(references) if index in items)
                   for index in range(len(boxes))]
    return per_meshlet, summary


class AssignTest(unittest.TestCase):
    def test_open_space_meshlet_stays_in_its_room(self):
        (room_a,), _ = refs([((3, 60, 40), (7, 70, 45))])
        self.assertEqual(room_a, [1])
        (room_b,), _ = refs([((80, 40, 40), (90, 60, 60))])
        self.assertEqual(room_b, [6])

    def test_meshlet_inside_furniture_reaches_leaves_bordering_it(self):
        (inside,), summary = refs([((12, 20, 20), (18, 30, 30))])
        # Rays reach it through the open leaves around the furniture only.
        self.assertEqual(inside, [1, 3, 4, 11, 12, 13])
        self.assertEqual(summary["meshlets_through_solid"], 1)
        self.assertEqual(summary["solid_regions"], 2)

    def test_solids_touching_the_shell_form_one_region(self):
        leaves = list(LEAVES)
        leaves[2] = (SOLID, (10, 0, 0), (20, 50, 50))  # furniture against the south wall
        references, summary = visibility.assign([((12, 20, 20), (18, 30, 30))], leaves)
        inside = sorted(leaf for leaf, items in enumerate(references) if items)
        # A ray may enter the shell elsewhere and continue into the furniture.
        self.assertIn(6, inside)
        self.assertEqual(summary["solid_regions"], 1)

    def test_detail_solid_leaves_are_walked_and_referenced_directly(self):
        leaves = list(LEAVES)
        leaves[2] = (SOLID | DETAIL, (10, 10, 10), (20, 50, 50))
        references, summary = visibility.assign([((13, 20, 20), (17, 30, 30))], leaves)
        self.assertEqual([leaf for leaf, items in enumerate(references) if items], [2])
        self.assertEqual(summary["meshlets_through_solid"], 0)

    def test_geometry_in_the_shell_reaches_every_leaf_bordering_the_shell(self):
        (in_wall,), _ = refs([((55, 40, 40), (65, 60, 60))])
        self.assertIn(6, in_wall)
        self.assertIn(4, in_wall)
        # Touching no leaf at all: the every-leaf fallback, never dropped.
        (beyond,), summary = refs([((200, 200, 200), (210, 210, 210))])
        self.assertEqual(beyond, [1, 3, 4, 6, 11, 12, 13])
        self.assertEqual(summary["meshlets_in_every_leaf_fallback"], 1)

    def test_no_meshlet_is_left_unreferenced(self):
        boxes = [((x, y, 10), (x + 3, y + 3, 12)) for x in range(-20, 120, 7)
                 for y in range(-20, 120, 7)]
        per_meshlet, summary = refs(boxes)
        self.assertTrue(all(per_meshlet))
        self.assertTrue(all(leaf not in SOLIDS
                            for leaves in per_meshlet for leaf in leaves))
        self.assertEqual(summary["leaf_references"], sum(map(len, per_meshlet)))

    def test_every_open_leaf_touched_is_referenced(self):
        # A meshlet spanning both open leaves of room A is in both.
        (spanning,), _ = refs([((5, 60, 40), (30, 70, 45))])
        self.assertEqual(spanning, [1, 3])
        (touching,), _ = refs([((5, 60, 40), (8.5, 70, 45))])  # within the leaf padding
        self.assertEqual(touching, [1, 3])

    def test_bad_bsp_without_open_leaves_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "no non-solid leaves"):
            visibility.assign([((0, 0, 0), (1, 1, 1))], [(SOLID, (0, 0, 0), (9, 9, 9))])


class MortonTest(unittest.TestCase):
    def test_neighbors_stay_together(self):
        points = [(x, y, 0) for x in (0, 100) for y in (0, 100)]
        points = [(px + dx, py + dy, 0) for px, py, _ in points for dx in (0, 1) for dy in (0, 1)]
        order = visibility.morton_order(points)
        cells = [(points[i][0] // 50, points[i][1] // 50) for i in order]
        # Each 2x2 cluster is contiguous in the order.
        runs = [cells[i:i + 4] for i in range(0, 16, 4)]
        self.assertTrue(all(len(set(run)) == 1 for run in runs))
        self.assertEqual(sorted(order), list(range(16)))


if __name__ == "__main__":
    unittest.main()
