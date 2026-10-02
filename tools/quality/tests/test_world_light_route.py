# ==== Copyright Valve Corporation, All rights reserved. ======================
import sys
from pathlib import Path
import struct
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import world_light_route as route


class RouteTests(unittest.TestCase):
    def slab(self):
        collision = route.Collision.__new__(route.Collision)
        # A slab only 0.254 mm thick: sampling endpoints would miss it.
        collision.planes = struct.pack("<4fi4fi", 1, 0, 0, .01, 0, 1, 0, 0, 0, 0)
        collision.nodes = struct.pack("<iii20xiii20x", 0, -1, 1, 1, -2, -1)
        collision.leaves = struct.pack("<i28xi28x", 0, 1)
        return collision

    def test_exact_trace_rejects_thin_wall(self):
        collision = self.slab()
        self.assertFalse(collision.clear((-.5, 0, 0), (.5, 0, 0)))
        self.assertTrue(collision.clear((.5, 0, 0), (1, 0, 0)))
        self.assertFalse(collision.sweep((.1, 0, 0), (.1, 1, 0), .2))

    def test_astar_cannot_connect_anchors_through_a_wall(self):
        points = [{"name": "a", "capture": [-.5, 0, 0]},
                  {"name": "b", "capture": [.5, 0, 0]}]
        with self.assertRaisesRegex(ValueError, "no BSP-clear route"):
            route.route(self.slab(), points, "a", "b")

    def test_astar_routes_around_obstacle(self):
        class Obstacle:
            def sweep(self, a, b, radius):
                return a[1] == 1 or b[1] == 1
        points = [{"name": name, "capture": point} for name, point in
                  (("start", [0, 0, 0]), ("corner", [1, 1, 0]), ("end", [2, 0, 0]))]
        path = route.route(Obstacle(), points, "start", "end")
        self.assertEqual(["start", "corner", "end"], [item["name"] for item in path])
        positions = route.samples(path, .2)
        self.assertEqual([0, 0, 0], positions[0])
        self.assertEqual([2, 0, 0], positions[-1])
        import math
        self.assertTrue(all(math.dist(a, b) <= .2 for a, b in zip(positions, positions[1:])))

    def test_layer_oracle_detects_seeded_loss(self):
        lit, missing = {"mean": .2, "p95": .4}, {"mean": 0, "p95": 0}
        self.assertGreater(route.layer_change(lit, missing, "p95"), .75)
        self.assertEqual(0, route.layer_change(lit, lit, "p95"))
        # An isolated not-applicable hatch can move without changing the field.
        isolated = {"mean": .8, "p95": .4}
        self.assertEqual(0, route.layer_change(lit, isolated, "p95"))

    def test_not_applicable_hatch_does_not_become_a_lighting_signal(self):
        import numpy as np
        from PIL import Image
        y, x = np.indices((64, 64))
        grey = np.where(((x + y) // 4) % 2 == 0, 137, 188).astype(np.uint8)
        pixels = np.repeat(grey[:, :, None], 3, axis=2)
        pixels[:, 24:40] = 120
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "layer.png"
            Image.fromarray(pixels).save(path)
            measured = route.metric(path)
        expected = ((120 / 255 + .055) / 1.055) ** 2.4
        self.assertGreater(measured["hatch_fraction"], .4)
        self.assertAlmostEqual(expected, measured["p95"], places=8)


if __name__ == "__main__":
    unittest.main()
