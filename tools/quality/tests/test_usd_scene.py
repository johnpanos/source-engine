"""Oracles for the USD scene front end (`usd_scene.py`); needs OpenUSD's `pxr`.

    PYTHONPATH=build/toolchains/openusd-25.11/lib/python \\
        /usr/bin/python3.12 -m unittest tools.quality.tests.test_usd_scene -v
"""

import math
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from pxr import Gf, Sdf, Usd, UsdGeom, UsdLux, UsdShade

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/quality"))
import usd_scene  # noqa: E402

FIXTURE = ROOT / "quality/fixtures/usd-maps/room/room.usda"


class FixtureTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        out = Path(cls.temporary.name)
        cls.model = usd_scene.extract(FIXTURE, out / "scene.json", out / "stage.usdc")
        cls.stage = Usd.Stage.Open(str(out / "stage.usdc"))
        cls.shapes = {shape["name"]: shape for shape in cls.model["shapes"]}

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def test_units_and_up_axis_become_z_up_meters(self):
        # The fixture is Y-up centimetres; its 600 x 300 x 800 cm interior.
        low, high = self.shapes["Floor"]["bounds"]
        self.assertAlmostEqual(high[2], 0.0, places=6)
        self.assertAlmostEqual(high[0] - low[0], 6.4, places=6)
        self.assertAlmostEqual(high[1] - low[1], 8.4, places=6)
        low, high = self.shapes["Ceiling"]["bounds"]
        self.assertAlmostEqual(low[2], 3.0, places=6)
        self.assertEqual(UsdGeom.GetStageUpAxis(self.stage), UsdGeom.Tokens.z)
        self.assertEqual(UsdGeom.GetStageMetersPerUnit(self.stage), 1.0)

    def test_subsets_instances_and_point_instancer_expand(self):
        self.assertEqual(self.shapes["Table_Top"]["material"], "woodfloor")
        self.assertEqual(self.shapes["Table_Legs"]["material"], "paintedsteel")
        self.assertEqual(len([n for n in self.shapes if n.startswith("Body")]), 2)
        self.assertEqual(len([n for n in self.shapes if n.startswith("Blocks_Block_")]), 3)
        # Hidden prototypes are not placed where they are authored.
        self.assertNotIn("Block", self.shapes)

    def test_material_channels(self):
        wood = self.model["materials"]["woodfloor"]
        self.assertEqual(sorted(wood["textures"]),
                         ["base", "metallic", "normal", "occlusion", "roughness"])
        self.assertEqual((wood["textures"]["roughness"]["channel"],
                          wood["textures"]["metallic"]["channel"]), ("g", "b"))
        self.assertEqual(wood["textures"]["normal"]["scale"][:3], [2.0, 2.0, 2.0])
        self.assertEqual(wood["st_transform"]["scale"], [0.5, 0.5])
        leaves = self.model["materials"]["leaves"]
        self.assertEqual(leaves["opacity_threshold"], 0.5)
        self.assertEqual(leaves["textures"]["opacity"]["channel"], "a")
        self.assertIn("emission", self.model["materials"]["screen"]["textures"])
        self.assertEqual(self.model["materials"]["brushedmetal"]["metallic"], 1.0)

    def test_st_transform_is_applied_to_stage_uvs(self):
        mesh = UsdGeom.Mesh(self.stage.GetPrimAtPath("/root/Floor/Floor"))
        st = np.asarray(UsdGeom.PrimvarsAPI(mesh).GetPrimvar("st").Get())
        # Floor st is metres (0.01 * cm), tiled at 0.5: the 6.4 m span becomes 3.2.
        self.assertAlmostEqual(float(st[:, 0].max() - st[:, 0].min()), 3.2, places=4)

    def test_lights_follow_usdlux_units(self):
        (panel,) = self.model["emitters"]
        self.assertAlmostEqual(panel["area_m2"], 0.64, places=6)
        # normalize off: radiance = intensity * color.
        np.testing.assert_allclose(panel["emission"]["radiance"], (6.0, 5.4, 4.5), rtol=1e-6)
        self.assertTrue(panel["emission"]["one_sided"])
        points = np.asarray(panel["points"])
        normal = np.cross(points[2] - points[0], points[1] - points[0])
        self.assertLess(normal[2], 0)  # rect light faces down (-Y in the Y-up source)
        (sun,) = self.model["distant_lights"]
        # normalize on: intensity is illuminance.
        np.testing.assert_allclose(sun["irradiance"], (4.0, 3.8, 3.44), rtol=1e-5)
        self.assertLess(sun["direction"][2], 0)
        self.assertEqual(self.model["environment"]["pole"], "Y")

    def test_camera(self):
        camera = self.model["camera"]
        self.assertEqual(camera["source"], "/Room/Camera")
        expected = math.degrees(2 * math.atan(20.25 / 40.0))
        self.assertAlmostEqual(camera["fov_degrees"], expected, places=4)
        np.testing.assert_allclose(camera["pose"]["eye"], (0.7, -7.6, 1.6), atol=1e-9)


class NegativeTest(unittest.TestCase):
    def stage(self):
        stage = Usd.Stage.CreateInMemory()
        UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
        mesh = UsdGeom.Mesh.Define(stage, "/World/Quad")
        mesh.CreatePointsAttr([Gf.Vec3f(0, 0, 0), Gf.Vec3f(1, 0, 0), Gf.Vec3f(1, 1, 0),
                               Gf.Vec3f(0, 1, 0)])
        mesh.CreateFaceVertexCountsAttr([4])
        mesh.CreateFaceVertexIndicesAttr([0, 1, 2, 3])
        return stage

    def test_unsupported_light_fails_loudly(self):
        stage = self.stage()
        UsdLux.CylinderLight.Define(stage, "/World/Tube")
        with self.assertRaises(ValueError):
            usd_scene.Extractor(stage, "memory").run()

    def test_unbound_mesh_uses_display_color_and_notes_missing_st(self):
        stage = self.stage()
        UsdGeom.Mesh(stage.GetPrimAtPath("/World/Quad")).CreateDisplayColorAttr(
            [Gf.Vec3f(0.2, 0.4, 0.6)])
        extractor = usd_scene.Extractor(stage, "memory")
        extractor.run()
        (name,) = extractor.summaries
        np.testing.assert_allclose(extractor.summaries[name]["base_color"], (0.2, 0.4, 0.6),
                                   rtol=1e-6)
        self.assertTrue(any("no st primvar" in note for note in extractor.notes))

    def test_non_preview_surface_is_recorded(self):
        stage = self.stage()
        material = UsdShade.Material.Define(stage, "/World/Look")
        shader = UsdShade.Shader.Define(stage, "/World/Look/Mdl")
        shader.SetSourceAsset(Sdf.AssetPath("OmniPBR.mdl"), "mdl")
        material.CreateSurfaceOutput("mdl").ConnectToSource(shader.ConnectableAPI(), "out")
        UsdShade.MaterialBindingAPI.Apply(stage.GetPrimAtPath("/World/Quad")).Bind(material)
        extractor = usd_scene.Extractor(stage, "memory")
        extractor.run()
        self.assertIn("unsupported", extractor.summaries["look"]["approximation"])

    def test_concave_polygon_is_ear_clipped(self):
        # An L-shaped hexagon: a fan from corner 0 would cover the notch.
        polygon = np.array(((0, 0, 0), (2, 0, 0), (2, 1, 0), (1, 1, 0), (1, 2, 0), (0, 2, 0)),
                           dtype=float)
        triangles, _ = usd_scene.triangulate(polygon, np.array([6]), np.arange(6))
        area = sum(0.5 * np.linalg.norm(np.cross(polygon[b] - polygon[a], polygon[c] - polygon[a]))
                   for a, b, c in triangles)
        self.assertEqual(len(triangles), 4)
        self.assertAlmostEqual(area, 3.0)


if __name__ == "__main__":
    unittest.main()
