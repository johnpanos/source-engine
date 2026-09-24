import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from worldmesh_seam_weld import weld_material


def triangle(points):
    return (points, "material_uv", "lightmap_uv", "normal", "tangent", 1)


class WorldMeshSeamWeldTest(unittest.TestCase):
    def test_closes_near_coincident_wall_edge_without_touching_other_material(self):
        faces = {
            1: {"material": "map/wall", "triangles": [triangle(
                [(0, 0, 0), (10, 0, 0), (0, -10, 0)])]},
            2: {"material": "map/wall", "triangles": [triangle(
                [(0, 0.3, 0), (10, 0, 0), (10, 10, 0)])]},
            3: {"material": "map/wood", "triangles": [triangle(
                [(0, 0, 0), (10, 0, 0), (10, 10, 0)])]},
        }
        wood = faces[3]["triangles"][0]
        first_before = np.asarray(faces[1]["triangles"][0][0][0])
        second_before = np.asarray(faces[2]["triangles"][0][0][0])
        common = np.asarray(faces[1]["triangles"][0][0][1])
        self.assertGreater(np.linalg.norm(np.cross(
            common - first_before, second_before - first_before)) / 2, 1.0)
        result = weld_material(faces, "map/wall", 0.5)
        self.assertEqual(faces[1]["triangles"][0][0][0],
                         faces[2]["triangles"][0][0][0])
        self.assertEqual(faces[1]["triangles"][0][0][1],
                         faces[2]["triangles"][0][0][1])
        self.assertEqual(faces[3]["triangles"][0], wood)
        self.assertEqual(faces[1]["triangles"][0][1:], wood[1:])
        self.assertEqual(result["clusters"], 4)

    def test_rejects_chain_that_moves_farther_than_limit(self):
        faces = {1: {"material": "map/wall", "triangles": [triangle(
            [(0, 0, 0), (0.4, 0, 0), (0.8, 0, 0),
             (1.2, 0, 0), (1.6, 0, 0)])]}}
        before = faces[1]["triangles"][0]
        with self.assertRaisesRegex(ValueError, "chained"):
            weld_material(faces, "map/wall", 0.5)
        self.assertEqual(faces[1]["triangles"][0], before)


if __name__ == "__main__":
    unittest.main()
