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

try:
    from pxr import Gf, Sdf, Usd, UsdGeom, UsdLux, UsdShade
except ImportError as error:  # system Python has no OpenUSD; see the docstring
    raise unittest.SkipTest("test_usd_scene needs OpenUSD's pxr: %s" % error)

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
        fixture = Usd.Stage.Open(str(FIXTURE))
        source = fixture.GetPrimAtPath("/Room/Shell/Floor")
        authored = np.asarray(UsdGeom.PrimvarsAPI(source).GetPrimvar("st").Get())
        # WoodFloor tiles at 0.5. The Floor is a box, so its side faces map z onto
        # u: the authored u span is 8.4 and the stage's must be 4.2.
        span = float(authored[:, 0].max() - authored[:, 0].min())
        self.assertAlmostEqual(span, 8.4, places=4)
        self.assertAlmostEqual(float(st[:, 0].max() - st[:, 0].min()), 0.5 * span, places=4)

    def test_stage_meshes_keep_the_source_vertex_sharing(self):
        """Stage meshes share vertices as their source meshes do (no triangle
        soup), and every corner still sits exactly where the model says."""
        shared = 0
        for name, shape in self.shapes.items():
            prim = self.stage.GetPrimAtPath("/root/%s/%s" % (name, name))
            if not prim:
                continue
            mesh = UsdGeom.Mesh(prim)
            points = np.asarray(mesh.GetPointsAttr().Get(), dtype=np.float64)
            indices = np.asarray(mesh.GetFaceVertexIndicesAttr().Get())
            self.assertEqual(len(indices), 3 * shape["triangles"])
            self.assertEqual(len(np.unique(indices)), len(points))  # every point used
            shared += len(points) < len(indices)
        self.assertGreater(shared, 0)

    def test_face_data_follows_each_triangle(self):
        """A uniform sourceEngine:* integer primvar reaches the stage with one
        value per triangle, the value of the source face it came from."""
        with tempfile.TemporaryDirectory() as name:
            out = Path(name)
            source = Usd.Stage.CreateNew(str(out / "quad.usda"))
            UsdGeom.SetStageUpAxis(source, UsdGeom.Tokens.z)
            UsdGeom.SetStageMetersPerUnit(source, 1.0)
            mesh = UsdGeom.Mesh.Define(source, "/World/Quads")
            mesh.CreatePointsAttr([Gf.Vec3f(x, y, 0) for x, y in
                                   ((0, 0), (1, 0), (1, 1), (0, 1), (2, 0), (2, 1))])
            mesh.CreateFaceVertexCountsAttr([4, 4])
            mesh.CreateFaceVertexIndicesAttr([0, 1, 2, 3, 1, 4, 5, 2])
            UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
                "st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.vertex).Set(
                [Gf.Vec2f(x, y) for x, y in ((0, 0), (1, 0), (1, 1), (0, 1), (2, 0), (2, 1))])
            UsdGeom.PrimvarsAPI(mesh).CreatePrimvar(
                "sourceEngine:plane", Sdf.ValueTypeNames.IntArray,
                UsdGeom.Tokens.uniform).Set([7, 9])
            source.GetRootLayer().Save()
            usd_scene.extract(out / "quad.usda", out / "scene.json", out / "stage.usdc")
            stage = Usd.Stage.Open(str(out / "stage.usdc"))
            meshes = [UsdGeom.Mesh(p) for p in stage.Traverse() if p.IsA(UsdGeom.Mesh)]
            self.assertEqual(len(meshes), 1)
            planes = UsdGeom.PrimvarsAPI(meshes[0]).GetPrimvar("sourceEngine:plane")
            self.assertEqual(planes.GetInterpolation(), UsdGeom.Tokens.uniform)
            points = np.asarray(meshes[0].GetPointsAttr().Get())
            corners = points[np.asarray(meshes[0].GetFaceVertexIndicesAttr().Get())]
            centres = corners.reshape(-1, 3, 3).mean(axis=1)
            # Each triangle carries its source quad's plane (quad 0: x < 1).
            expected = [7 if centre[0] < 1 else 9 for centre in centres]
            self.assertEqual(list(planes.Get()), expected)
            self.assertEqual(len(points), 6)  # the two quads still share an edge

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

    def grid_stage(self, cells):
        """A flat 1 m square tessellated into cells x cells quads."""
        stage = Usd.Stage.CreateInMemory()
        UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
        UsdGeom.SetStageMetersPerUnit(stage, 1.0)
        mesh = UsdGeom.Mesh.Define(stage, "/World/Bed/stone")
        step = 1.0 / cells
        mesh.CreatePointsAttr([Gf.Vec3f(x * step, y * step, 0) for y in range(cells + 1)
                               for x in range(cells + 1)])
        mesh.CreateFaceVertexCountsAttr([4] * cells * cells)
        mesh.CreateFaceVertexIndicesAttr([i for y in range(cells) for x in range(cells)
                                          for i in (y * (cells + 1) + x, y * (cells + 1) + x + 1,
                                                    (y + 1) * (cells + 1) + x + 1,
                                                    (y + 1) * (cells + 1) + x)])
        return stage

    def test_simplify_clusters_selected_meshes_and_keeps_extent(self):
        extractor = usd_scene.Extractor(self.grid_stage(40), "memory",
                                        [("/World/*/stone", 0.1)])
        extractor.run()
        (shape,) = extractor.shapes
        self.assertLess(len(shape["corner_triangles"]), 2 * 40 * 40 // 4)
        self.assertGreater(len(shape["corner_triangles"]), 0)
        points = usd_scene.shape_points(shape)
        np.testing.assert_allclose(points.min(axis=0), (0, 0, 0), atol=0.05)
        np.testing.assert_allclose(points.max(axis=0), (1, 1, 0), atol=0.05)
        # Area is preserved for a plane, and every triangle keeps its winding.
        corners = points.reshape(-1, 3, 3)
        cross = np.cross(corners[:, 1] - corners[:, 0], corners[:, 2] - corners[:, 0])
        self.assertTrue(np.all(cross[:, 2] > 0))
        self.assertAlmostEqual(0.5 * cross[:, 2].sum(), 1.0, delta=0.1)
        self.assertTrue(any("vertex clustering: 3200 ->" in note for note in extractor.notes))

    def test_camera_exposure_scales_lights_not_material_emission(self):
        stage = self.stage()
        UsdGeom.SetStageMetersPerUnit(stage, 1.0)
        camera = UsdGeom.Camera.Define(stage, "/World/Camera")
        camera.GetPrim().CreateAttribute("exposure", Sdf.ValueTypeNames.Float).Set(-10.0)
        dome = UsdLux.DomeLight.Define(stage, "/World/Dome")
        dome.CreateIntensityAttr(4096.0)
        rect = UsdLux.RectLight.Define(stage, "/World/Rect")
        rect.CreateIntensityAttr(2048.0)
        material = UsdShade.Material.Define(stage, "/World/Glow")
        shader = UsdShade.Shader.Define(stage, "/World/Glow/Surface")
        shader.CreateIdAttr("UsdPreviewSurface")
        shader.CreateInput("emissiveColor", Sdf.ValueTypeNames.Color3f).Set(Gf.Vec3f(1.5, 1, 0.5))
        material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
        UsdShade.MaterialBindingAPI.Apply(stage.GetPrimAtPath("/World/Quad")).Bind(material)
        extractor = usd_scene.Extractor(stage, "memory")
        extractor.run()
        self.assertEqual(extractor.exposure_scale, 2.0 ** -10)
        np.testing.assert_allclose(extractor.environment["radiance"], (4.0, 4.0, 4.0))
        np.testing.assert_allclose(extractor.emitters[0]["emission"]["radiance"], (2.0, 2.0, 2.0))
        np.testing.assert_allclose(extractor.summaries["glow"]["emission_color"], (1.5, 1, 0.5))

    def test_transparent_preview_surface_transmits_untinted(self):
        stage = self.stage()
        material = UsdShade.Material.Define(stage, "/World/Glass")
        shader = UsdShade.Shader.Define(stage, "/World/Glass/Surface")
        shader.CreateIdAttr("UsdPreviewSurface")
        shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(Gf.Vec3f(0, 0, 0))
        shader.CreateInput("opacity", Sdf.ValueTypeNames.Float).Set(0.0)
        material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
        UsdShade.MaterialBindingAPI.Apply(stage.GetPrimAtPath("/World/Quad")).Bind(material)
        extractor = usd_scene.Extractor(stage, "memory")
        extractor.run()
        glass = extractor.summaries["glass"]
        self.assertEqual(glass["transmission"], 1.0)
        self.assertEqual(glass["base_color"], (1.0, 1.0, 1.0))
        self.assertIn("untinted", glass["approximation"])

    def test_simplify_pattern_matching_nothing_fails(self):
        extractor = usd_scene.Extractor(self.grid_stage(2), "memory", [("/World/Rocks", 0.1)])
        with self.assertRaises(ValueError):
            extractor.run()


if __name__ == "__main__":
    unittest.main()
