import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import portal2_gi_chamber as chamber  # noqa: E402
import vmf_instances as vi  # noqa: E402

BOX = """
world
{
	"id" "1"
	"classname" "worldspawn"
	solid
	{
		"id" "2"
		side
		{
			"id" "3"
			"plane" "(0 0 16) (0 32 16) (32 32 16)"
			"material" "TILE/WHITE"
			"uaxis" "[1 0 0 0] 0.25"
			"vaxis" "[0 -1 0 0] 0.25"
		}
	}
}
entity
{
	"id" "4"
	"classname" "prop_dynamic"
	"targetname" "arm"
	"parentname" "@rail"
	"origin" "10 0 0"
	"angles" "0 0 0"
	connections
	{
		"OnAnimationDone" "arm\x1bSetAnimation\x1bidle\x1b0\x1b-1"
		"OnUser1" "!self,Kill,,0,-1"
	}
}
entity
{
	"id" "5"
	"classname" "func_instance_io_proxy"
	"targetname" "proxy"
}
"""

NESTED = """
entity
{
	"id" "1"
	"classname" "func_instance"
	"targetname" "inner"
	"file" "box.vmf"
	"origin" "0 0 100"
	"angles" "0 0 0"
	"fixup_style" "0"
}
entity
{
	"id" "2"
	"classname" "func_instance"
	"file" "box.vmf"
	"origin" "0 0 200"
	"angles" "0 0 0"
	"fixup_style" "2"
}
"""


class TransformTest(unittest.TestCase):
    def test_angles_round_trip(self):
        for angles in ((0, 90, 0), (90, 0, 0), (0, 270, 0), (30, 45, 10), (-25, 180, 0)):
            pitch, yaw, roll = vi.matrix_angles(vi.angle_matrix(*angles))
            again = vi.angle_matrix(pitch, yaw, roll)
            expected = vi.angle_matrix(*angles)
            for r in range(3):
                for c in range(3):
                    self.assertAlmostEqual(again[r][c], expected[r][c], places=6)

    def test_yaw_turns_x_toward_y(self):
        t = vi.Transform((100, 0, 0), (0, 90, 0))
        self.assertEqual([vi.clean(c) for c in t.point((1, 0, 0))], [100, 1, 0])
        # Pitch 90 points an instance's +x (a panel's facing) down, as a
        # ceiling light needs.
        down = vi.Transform((0, 0, 0), (90, 0, 0)).vector((1, 0, 0))
        self.assertEqual([vi.clean(c) for c in down], [0, 0, -1])


class LightAnglesTest(unittest.TestCase):
    def test_ceiling_spot_points_down(self):
        # The SDK light panel's spot faces the panel's +x ("pitch" 0, vrad's
        # convention); a panel turned onto a ceiling (pitch 90) must shine down.
        light = [("classname", "light_spot"), ("angles", "0 0 -180"), ("pitch", "0")]
        vi.transform_light_angles(light, vi.Transform((0, 0, 0), (90, 0, 0)))
        self.assertEqual(vi.value(light, "pitch"), "-90")
        self.assertEqual(vi.parse_vector(vi.value(light, "angles"))[0], -90.0)

    def test_yawed_instance_keeps_downward_tilt(self):
        light = [("classname", "light_spot"), ("angles", "-25 0 0"), ("pitch", "-25")]
        vi.transform_light_angles(light, vi.Transform((0, 0, 0), (0, 90, 0)))
        self.assertEqual(vi.value(light, "pitch"), "-25")
        self.assertAlmostEqual(vi.parse_vector(vi.value(light, "angles"))[1], 90.0)


class CollapseTest(unittest.TestCase):
    def setUp(self):
        self.root = Path(tempfile.mkdtemp())
        (self.root / "box.vmf").write_text(BOX)
        (self.root / "nested.vmf").write_text(NESTED)

    def test_moves_geometry_and_keeps_texture_alignment(self):
        out = vi.collapse("box.vmf", self.root, (64, 0, 8), (0, 90, 0), name="door")
        side = vi.children(out.world[0], "side")[0]
        points = [tuple(float(c) for c in m) for m in vi.PLANE.findall(vi.value(side, "plane"))]
        self.assertEqual(points[1], (32.0, 0.0, 24.0))
        # The u axis turned with the instance; its shift keeps the texel that
        # was at the instance origin at the moved origin.
        self.assertEqual(vi.value(side, "uaxis"), "[0 1 0 0] 0.25")
        self.assertEqual(vi.value(side, "vaxis"), "[1 0 0 -256] 0.25")

    def test_prefixes_names_and_connections(self):
        out = vi.collapse("box.vmf", self.root, (0, 0, 0), (0, 0, 0), name="door")
        (arm,) = out.entities
        self.assertEqual(vi.value(arm, "targetname"), "door-arm")
        self.assertEqual(vi.value(arm, "parentname"), "@rail")
        connections = dict(vi.children(arm, "connections")[0])
        self.assertEqual(connections["OnAnimationDone"], "door-arm,SetAnimation,idle,0,-1")
        self.assertEqual(connections["OnUser1"], "!self,Kill,,0,-1")

    def test_style_two_keeps_names_and_drops_proxies(self):
        out = vi.collapse("box.vmf", self.root, (0, 0, 0), (0, 0, 0), name="", fixup_style=2)
        self.assertEqual([vi.value(e, "classname") for e in out.entities], ["prop_dynamic"])
        self.assertEqual(vi.value(out.entities[0], "targetname"), "arm")

    def test_nested_instances_compose(self):
        out = vi.collapse("nested.vmf", self.root, (0, 0, 0), (0, 0, 0), name="outer")
        names = sorted(vi.value(e, "targetname") for e in out.entities)
        self.assertEqual(names, ["outer-arm", "outer-inner-arm"])
        heights = sorted(vi.parse_vector(vi.value(e, "origin"))[2] for e in out.entities)
        self.assertEqual(heights, [100.0, 200.0])

    def test_serialize_round_trips(self):
        tree = vi.read(self.root / "box.vmf")
        self.assertEqual(vi.parse(vi.serialize(tree)), tree)


class ChamberGeometryTest(unittest.TestCase):
    def area(self, rects):
        return sum((a1 - a0) * (b1 - b0) for (a0, a1), (b0, b1) in rects)

    def test_subtract_covers_the_rest_without_overlap(self):
        rect = ((0, 100), (0, 50))
        holes = [((10, 20), (10, 20)), ((60, 90), (0, 50)), ((15, 30), (15, 40))]
        pieces = chamber.subtract(rect, holes)
        union_holes = 10 * 10 + 30 * 50 + 15 * 25 - 5 * 5
        self.assertEqual(self.area(pieces), 100 * 50 - union_holes)
        for i, a in enumerate(pieces):
            for b in pieces[i + 1:]:
                self.assertIsNone(chamber.intersect(a, b))
            for hole in holes:
                self.assertIsNone(chamber.intersect(a, hole))

    def test_box_faces_point_outward(self):
        m = chamber.Chamber(Path("."))
        solid = m.solid((0, 0, 0), (10, 20, 30), "X")
        for side in vi.children(solid, "side"):
            p0, p1, p2 = [tuple(float(c) for c in m_) for m_ in
                          vi.PLANE.findall(vi.value(side, "plane"))]
            a = [p0[i] - p1[i] for i in range(3)]
            b = [p2[i] - p1[i] for i in range(3)]
            normal = (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                      a[0] * b[1] - a[1] * b[0])
            centre = (5, 10, 15)
            outward = [p1[i] - centre[i] for i in range(3)]
            self.assertGreater(sum(n * o for n, o in zip(normal, outward)), 0)


if __name__ == "__main__":
    unittest.main()
