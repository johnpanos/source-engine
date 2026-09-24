"""Tests for the shared PBRT scene reader used by the PBRT map pipeline."""

import math
from pathlib import Path
import struct
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import pbrt_scene

SCENE = """
Integrator "path" "integer maxdepth" [ 8 ]
Transform [ 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 ]
Film "rgb" "integer xresolution" [ 640 ] "integer yresolution" [ 360 ]
Camera "perspective" "float fov" [ 50 ]
WorldBegin
Texture "Wood" "spectrum" "imagemap" "string filename" [ "textures/wood.tga" ]
MakeNamedMaterial "Floor" "string type" [ "coateddiffuse" ]
    "bool remaproughness" [ false ] "float roughness" [ 0.09 ]
    "texture reflectance" [ "Wood" ]
MakeNamedMaterial "Silver" "string type" [ "conductor" ]
    "spectrum eta" [ "metal-Ag-eta" ] "spectrum k" [ "metal-Ag-k" ]
MakeNamedMaterial "Leaf" "string type" [ "diffusetransmission" ] "float scale" .5
NamedMaterial "Floor"
Shape "plymesh" "string filename" [ "models/Mesh000.ply" ]
AttributeBegin
    NamedMaterial "Silver"
    Transform [ 1 0 0 0 0 1 0 0 0 0 1 0 2 0 0 1 ]
    Shape "plymesh" "string filename" [ "models/Mesh001.ply" ]
AttributeEnd
Shape "plymesh" "string filename" [ "models/Mesh002.ply" ]
AttributeBegin
    AreaLightSource "diffuse" "rgb L" [ 2 2 2 ]
    Material "diffuse" "rgb reflectance" [ 0 0 0 ]
    Shape "disk" "float radius" [ 0.25 ]
AttributeEnd
"""


def write(directory, text):
    path = Path(directory) / "scene.pbrt"
    path.write_text(text)
    return path


class ParseTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.scene = pbrt_scene.parse(write(self.directory.name, SCENE))

    def tearDown(self):
        self.directory.cleanup()

    def test_attribute_stack_restores_material_and_transform(self):
        shapes = {Path(s["filename"]).stem: s for s in self.scene["shapes"]}
        self.assertEqual(shapes["Mesh001"]["material"], "Silver")
        self.assertEqual(shapes["Mesh001"]["world_from_object"][0][3], 2.0)
        self.assertEqual(shapes["Mesh002"]["material"], "Floor")
        self.assertEqual(shapes["Mesh002"]["world_from_object"], pbrt_scene.IDENTITY)

    def test_emitters_are_separate_from_meshes(self):
        self.assertEqual(len(self.scene["shapes"]), 3)
        self.assertEqual(len(self.scene["emitters"]), 1)
        emitter = self.scene["emitters"][0]
        self.assertEqual((emitter["kind"], emitter["radius"]), ("disk", 0.25))
        self.assertEqual(emitter["emission"]["radiance"], [2.0, 2.0, 2.0])
        self.assertTrue(emitter["material"].startswith("Anonymous"))

    def test_unbracketed_parameter_and_film(self):
        leaf = self.scene["materials"]["Leaf"]["parameters"]
        self.assertEqual(leaf["scale"]["value"], [0.5])
        self.assertEqual(self.scene["film"], {"width": 640, "height": 360})

    def test_material_translation(self):
        floor = pbrt_scene.material_summary(self.scene, "Floor")
        self.assertEqual(floor["base_texture"], "textures/wood.tga")
        self.assertAlmostEqual(floor["coat_roughness"], 0.3)
        silver = pbrt_scene.material_summary(self.scene, "Silver")
        self.assertEqual(silver["metallic"], 1.0)
        self.assertTrue(all(0.9 < value < 1.0 for value in silver["base_color"]))

    def test_camera_pose_converts_to_z_up(self):
        pose = pbrt_scene.camera_pose(self.scene)
        np.testing.assert_allclose(pose["forward"], (0, -1, 0), atol=1e-12)
        np.testing.assert_allclose(pose["up"], (0, 0, 1), atol=1e-12)


class RejectTest(unittest.TestCase):
    def check(self, text, message):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(ValueError, message):
                pbrt_scene.parse(write(directory, text))

    def test_unknown_directive(self):
        self.check(SCENE + 'Shape "sphere"\n', "unsupported PBRT shape")
        self.check(SCENE + 'ObjectBegin "x"\n', "unsupported PBRT directive")

    def test_undefined_material_and_unbalanced_stack(self):
        self.check(SCENE.replace('\nNamedMaterial "Floor"', '\nNamedMaterial "Nope"'),
                   "undefined material")
        self.check(SCENE + "AttributeBegin\n", "attribute stack")

    def test_duplicate_mesh_stems(self):
        self.check(SCENE + 'Shape "plymesh" "string filename" [ "other/Mesh000.ply" ]\n',
                   "unique")


class EnvironmentTest(unittest.TestCase):
    def test_equal_area_anchors(self):
        directions = np.array([(0, 0, 1), (0, 0, -1), (1, 0, 0), (0, 1, 0)], float)
        u, v = pbrt_scene.equal_area_sphere_to_square(directions)
        np.testing.assert_allclose(u, (0.5, 1.0, 1.0, 0.5), atol=1e-12)
        np.testing.assert_allclose(v, (0.5, 1.0, 0.5, 1.0), atol=1e-12)

    def test_pfm_rows_are_top_down(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sky.pfm"
            # File rows are bottom-up: the first stored row is the image bottom.
            rows = [[0.0] * 6, [1.0] * 6]
            path.write_bytes(b"PF\n2 2\n-1.0\n" +
                             struct.pack("<12f", *(rows[0] + rows[1])))
            pixels = pbrt_scene.read_pfm(path)
            self.assertEqual(pixels[0, 0, 0], 1.0)
            self.assertEqual(pixels[1, 0, 0], 0.0)

    def test_equirect_zenith_samples_light_up_direction(self):
        size = 64
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sky.pfm"
            image = np.repeat(np.arange(size, dtype="<f4")[:, None], size, axis=1)
            stored = np.repeat(image[::-1, :, None], 3, axis=2)
            path.write_bytes(b"PF\n%d %d\n-1.0\n" % (size, size) + stored.tobytes())
            scene = {"source": str(Path(directory) / "scene.pbrt"),
                     "environment": {"filename": "sky.pfm", "scale": 1.0,
                                     "world_from_light": pbrt_scene.IDENTITY}}
            equirect = pbrt_scene.environment_equirect(scene, 32)
        # USD up is PBRT +Y, which the equal-area square maps to v = 1 (last
        # row); the top equirect row sits half a texel below the pole.
        self.assertGreaterEqual(equirect[0, 0, 0], size - 2)
        # USD down is PBRT -Y (v = 0, first row).
        self.assertLessEqual(equirect[-1, 0, 0], 1)


if __name__ == "__main__":
    unittest.main()
