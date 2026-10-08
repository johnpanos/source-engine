"""Real BSP -> USD -> charted Cycles bake -> compiled WMSH integration.

Run on the installed map-tool profile (OpenUSD Python, xatlas and Blender).
Missing tools fail this required lane; the unit lane needs none of them.
"""

import json
import os
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parents[1]
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import legacy_bsp
import legacy_bsp_scene
import pbrt_playable_content
from test_legacy_displacements import fixture
from test_legacy_relight import legacy_map, vtf, world_light


def integration_bsp():
    displacement, _ = fixture()
    info = bytearray(displacement.lump(26))
    struct.pack_into("<H", info, 36, 0)
    points = np.array(((0, 0, 0), (0, 64, 0), (64, 64, 0), (64, 0, 0),
                       (0, 0, 32), (0, 64, 32), (64, 64, 32), (64, 0, 32)), dtype="<f4")
    faces = []
    for i in range(2):
        values = list(legacy_bsp.FACE.unpack(bytes(legacy_bsp.FACE.size)))
        values[:14] = [i, 0, 1, i * 4, 4, i * 2, 0 if i == 0 else -1,
                       -1, 0, 255, 255, 255, -1, 4096.0]
        faces.append(legacy_bsp.FACE.pack(*values))
    names = b"floor\0paint\0grate\0"
    texdata = b"".join(struct.pack("<3fi4i", 0.5, 0.5, 0.5, i, 4, 4, 4, 4)
                       for i in range(3))
    texinfo = b"".join(struct.pack("<16f2i", 1 / 16, 0, 0, 0, 0, 1 / 16, 0, 0,
                                    *([0.] * 8), 16 if i == 2 else 0, i) for i in range(3))
    edges = np.array(((0, 0), (0, 1), (1, 2), (2, 3), (3, 0),
                       (4, 5), (5, 6), (6, 7), (7, 4)), dtype="<u2")
    overlay = bytearray(352)
    struct.pack_into("<ihHi", overlay, 0, 0, 1, 1, 0)
    struct.pack_into("<4f", overlay, 264, 0, 1, 0, 1)
    struct.pack_into("<12f", overlay, 280, -16, -16, 1, -16, 16, 0,
                     16, 16, 0, 16, -16, 0)
    struct.pack_into("<6f", overlay, 328, 32, 32, 0, 0, 0, 1)
    leaf = bytearray(32)
    struct.pack_into("<6h", leaf, 8, -128, -128, -128, 128, 128, 128)
    sun = bytearray(world_light(0, intensity=(0.5, 0.5, 0.5), kind=legacy_bsp.EMIT_SKYLIGHT))
    struct.pack_into("<3f", sun, 24, 0, 0, -1)  # Incoming light travels toward the floor.
    lumps = {0: (0, b'{"classname" "worldspawn"}\0'), 1: (0, struct.pack("<4fi4fi", 0, 0, 1, 0, 2, 0, 0, 1, 32, 2)),
             2: (0, texdata), 3: (0, points.tobytes()), 6: (0, texinfo), 7: (0, b"".join(faces)),
             10: (1, leaf), 12: (0, edges.tobytes()), 13: (0, struct.pack("<8i", *range(1, 9))),
             14: (0, struct.pack("<9f3i", *([0.] * 9), -1, 0, 2)),
             15: (0, sun),
             26: (0, info), 33: (0, displacement.lump(33)), 48: (0, displacement.lump(48)),
             43: (0, names), 44: (0, struct.pack("<3i", 0, 6, 12)), 45: (0, overlay)}
    return legacy_bsp.LegacyBsp(legacy_map(lumps))


class Content:
    def read(self, name):
        definitions = {
            "materials/floor.vmt": b'WorldVertexTransition { "$basetexture" "red" "$basetexture2" "blue" }',
            "materials/paint.vmt": b'LightmappedGeneric { "$basetexture" "green" "$translucent" "1" }',
            "materials/grate.vmt": b'LightmappedGeneric { "$basetexture" "grate" "$translucent" "1" }',
        }
        colors = {"red": (255, 0, 0, 255), "blue": (0, 0, 255, 255),
                  "green": (0, 255, 0, 128), "grate": (255, 255, 255, 128)}
        if name in definitions:
            return definitions[name], "fixture"
        if name.startswith("materials/") and name.endswith(".vtf"):
            color = colors.get(name[10:-4])
            if color:
                return vtf(0, 4, 4, bytes(color) * 16), "fixture"
        return None, None


class RelightIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        try:
            from pxr import Usd, UsdGeom
        except ImportError as error:
            raise unittest.SkipTest("test_legacy_relight_integration needs OpenUSD's pxr: %s" % error)
        import lightmap_layout
        import usd_scene
        cls.temporary = tempfile.TemporaryDirectory(prefix="relight-displacement-")
        cls.out = Path(cls.temporary.name)
        cls.bsp = integration_bsp()
        cls.model, cls.receipt = legacy_bsp_scene.build_model(cls.bsp, Content(), cls.out / "textures")
        cls.scene = cls.out / "scene.usda"
        legacy_bsp_scene.write_usd(cls.model, cls.scene, "displacement_fixture")
        cls.json = cls.out / "scene.json"
        cls.normalized = cls.out / "normalized.usdc"
        cls.extracted = usd_scene.extract(cls.scene, cls.json, cls.normalized)
        cls.charted = cls.out / "charted.usdc"
        cls.hidden = cls.receipt["static_transport"]["materials"]
        cls.layout = lightmap_layout.author(cls.normalized, cls.charted, 256, 2, 0,
                                           cls.hidden, ROOT / "build/toolchains/xatlas-chart/xatlas_chart")
        cls.stage = Usd.Stage.Open(str(cls.charted))

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def test_lift_preserves_deformation_layer_weights_and_transport_coverage(self):
        from pxr import UsdGeom
        self.assertEqual(self.receipt["relit_faces"], 2)
        self.assertEqual(self.receipt["displacements"][0]["triangles"], 32)
        self.assertNotIn("displacement (legacy-lit)", self.receipt["excluded_faces"])
        self.assertEqual(len(self.hidden), 1)
        material = self.extracted["materials"][self.hidden[0]]
        self.assertEqual(material["opacity_threshold"], 0)
        self.assertEqual(material["transmission"], 0)
        self.assertIn("opacity", material["textures"])
        surface = next(p for p in self.stage.Traverse() if p.GetName() == "floor_disp0" and p.IsA(UsdGeom.Mesh))
        self.assertFalse(surface.GetAttribute("primvars:sourceEngine:plane"))
        self.assertGreater(max(p[2] for p in surface.GetAttribute("points").Get()), 0)
        lightmap = surface.GetAttribute("primvars:lightmap_st").Get()
        self.assertEqual(len(lightmap), 96)
        self.assertTrue(np.isfinite(np.asarray(lightmap)).all())

    def test_paint_changes_transport_but_compiled_runtime_base_remains_unpainted(self):
        self.assertEqual(len(self.receipt["authored_paint"]["surfaces"]), 1)
        summary = self.extracted["materials"]["floor_disp0"]
        with Image.open(summary["textures"]["base"]["file"]) as image:
            painted = np.asarray(image)
        image, _solid, _hashes = pbrt_playable_content.base_image(self.extracted, summary)
        runtime = np.asarray(image)
        self.assertTrue((painted[..., 1] > 100).any())
        self.assertTrue((runtime[..., 1] == 0).all())
        self.assertGreater(int(runtime[..., 0].max()), 200)
        self.assertGreater(int(runtime[..., 2].max()), 200)
        import vtf_content
        destination = self.out / "compiled_runtime_base"
        vtf_content.compile_texture(image, destination, ROOT / "build/toolchains/pbrt-map-tools/vtex", True)
        payload = destination.with_suffix(".vtf").read_bytes()
        self.assertEqual(struct.unpack_from("<I", payload, 20)[0] & 12, 12)

    def test_charted_displacement_round_trips_through_real_wmsh_writer_reader(self):
        from usd_worldmesh_pack import source_triangles
        from worldstage_mesh_pack import write_payload
        from worldstage_mesh_compare import read_payload
        faces, inventory = source_triangles(self.stage, "fixture", True, False, self.hidden)
        decoded = read_payload(write_payload(faces, [set(faces)])[0])
        self.assertEqual(decoded["counts"]["triangles"], 32)
        self.assertEqual(decoded["materials"], ["fixture/floor_disp0"])
        self.assertEqual(sum(r["triangles"] for r in inventory), 32)
        points = np.asarray([vertex[0] for vertex in decoded["vertices"]])
        self.assertAlmostEqual(points[:, 2].max(), 8, places=4)
        self.assertGreater(len(np.unique(points, axis=0)), 20)
        normals = np.asarray([vertex[1] for vertex in decoded["vertices"]])
        self.assertTrue((normals[:, 2] > 0).all())
        self.assertGreater(np.linalg.norm(normals[:, :2]), 0)
        self.assertTrue(np.isfinite(np.asarray([v[5] for v in decoded["vertices"]])).all())

    def test_real_cycles_bake_lights_the_displacement_and_preserves_its_geometry(self):
        # A small CPU fixture qualifies the data path; production retains its
        # source2 GPU profile and full samples/resolution.
        from pxr import Usd, UsdGeom
        exr, result = self.out / "bake.exr", self.out / "baked.usdc"
        log = self.out / "bake.log"
        args = ["blender", "--background", "--factory-startup", "--python-exit-code", "1", "--python",
                str(HERE / "pbrt_lightmap_bake.py"), "--", "--scene", str(self.json),
                "--stage", str(self.charted), "--out-stage", str(result), "--out-exr", str(exr),
                "--size", "256", "--samples", "16", "--device", "cpu", "--layout", "authored"]
        for material in self.hidden:
            args += ["--exclude-material", material]
        env = dict(os.environ, OCIO=str(ROOT / "quality/fixtures/staircase2-ocio/config.ocio"))
        with log.open("w") as handle:
            run = subprocess.run(args, stdout=handle, stderr=subprocess.STDOUT, env=env, timeout=120)
        self.assertEqual(run.returncode, 0, log.read_text()[-4000:])
        self.assertTrue(exr.is_file(), log.read_text()[-4000:])
        stats = self.out / "light-stats.json"
        inspect = ("import bpy,json,numpy as np; "
                   "im=bpy.data.images.load(" + repr(str(exr)) + "); "
                   "p=np.asarray(im.pixels[:]).reshape(-1,4)[:,:3]; "
                   "json.dump({'finite':bool(np.isfinite(p).all()),'peak':float(p.max()),"
                   "'lit':int((p.max(axis=1)>0.01).sum())},open(" + repr(str(stats)) + ",'w'))")
        with log.open("a") as handle:
            check = subprocess.run(["blender", "--background", "--factory-startup",
                                    "--python-exit-code", "1",
                                    "--python-expr", inspect], stdout=handle,
                                   stderr=subprocess.STDOUT, env=env, timeout=30)
        self.assertEqual(check.returncode, 0, log.read_text()[-4000:])
        light = json.loads(stats.read_text())
        self.assertTrue(light["finite"])
        self.assertGreater(light["peak"], 0.1)
        self.assertGreater(light["lit"], 100)
        baked = Usd.Stage.Open(str(result))
        before = next(p for p in self.stage.Traverse()
                      if p.IsA(UsdGeom.Mesh) and p.GetName() == "floor_disp0")
        after = next(p for p in baked.Traverse()
                     if p.IsA(UsdGeom.Mesh) and p.GetName() == before.GetName())
        self.assertTrue(after)
        np.testing.assert_array_equal(before.GetAttribute("points").Get(),
                                      after.GetAttribute("points").Get())
        np.testing.assert_array_equal(before.GetAttribute("faceVertexIndices").Get(),
                                      after.GetAttribute("faceVertexIndices").Get())

    def test_static_decal_is_lifted_and_triggered_decal_is_excluded(self):
        source = integration_bsp()
        lumps = {i: (entry[2], source.lump(i)) for i, entry in enumerate(source.lumps)}
        lumps[0] = (0, b'{"classname" "worldspawn"}\n'
                       b'{"classname" "infodecal" "texture" "paint" "origin" "8 8 0"}\n'
                       b'{"classname" "infodecal" "texture" "paint" "origin" "8 8 0" '
                       b'"targetname" "triggered"}\0')
        _model, receipt = legacy_bsp_scene.build_model(
            legacy_bsp.LegacyBsp(legacy_map(lumps)), Content(), self.out / "decal-textures")
        decals = [r for r in receipt["authored_paint"]["overlays"] if r["kind"] == "decal"]
        self.assertTrue(any(r.get("faces") == [0] and "excluded" not in r for r in decals))
        self.assertTrue(any(r.get("excluded") == "runtime-triggered or parented decal" for r in decals))

    def test_immutable_brush_is_transformed_and_kept_out_of_runtime_world_pack(self):
        source = integration_bsp()
        lumps = {i: (entry[2], source.lump(i)) for i, entry in enumerate(source.lumps)}
        lumps[0] = (0, b'{"classname" "worldspawn"}\n'
                       b'{"classname" "func_brush" "model" "*1" "origin" "128 0 0"}\0')
        lumps[14] = (0, source.lump(14) + struct.pack("<9f3i", *([0.] * 9), -1, 1, 1))
        model, receipt = legacy_bsp_scene.build_model(
            legacy_bsp.LegacyBsp(legacy_map(lumps)), Content(), self.out / "brush-textures")
        self.assertEqual(len(receipt["static_transport"]["brush_entities"]), 1)
        faces = [f for group in model["meshes"].values() for f in group if f.get("transport_only")]
        self.assertTrue(any(float(f["points"][:, 0].min()) == 128 for f in faces))
        lumps[0] = (0, lumps[0][1][:-1] +
                       b'\n{"classname" "logic_relay" "OnTrigger" "func_brush,Disable,,0,-1"}\0')
        _model, controlled = legacy_bsp_scene.build_model(
            legacy_bsp.LegacyBsp(legacy_map(lumps)), Content(), self.out / "controlled-brush-textures")
        self.assertEqual(controlled["static_transport"]["brush_entities"], [])


if __name__ == "__main__":
    unittest.main()
