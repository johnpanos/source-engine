"""Material-import invariants, without renderer/image oracle comparisons."""
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import legacy_bsp_scene as scene
import source_content
import numpy as np


class MaterialImport(unittest.TestCase):
    def test_import_keeps_mip_zero(self):
        class Bsp:
            def pakfile(self):
                return None
        class Resolver:
            def read(self, name):
                return b"encoded", "fixture"
        image = np.zeros((1, 4096, 4), dtype=np.uint8)
        materials = scene.Materials(Bsp(), Resolver())
        with patch.object(scene.vtf_decode, "decode", return_value=(image, {"width": 4096})) as decode:
            result = materials.texture("wide")
            decode.assert_called_once_with(b"encoded", 0)
            self.assertEqual(result[0].shape[1], 4096)

    def test_explicit_mapping_keeps_namespace(self):
        class Bsp:
            def pakfile(self):
                return None
        class Resolver:
            def read(self, name):
                return (b'PBR { "$basetexture" "pbr/author/tile" }', "archive")
        materials = scene.Materials(Bsp(), Resolver(), material_overrides={"tile/original": "pbr/author/tile"})
        shader, params, _ = materials.vmt("tile/original")
        self.assertEqual(shader, "pbr")
        self.assertEqual(params[scene.BASE_KEY], "pbr/author/tile")

    def test_centered_uv_transform_has_fixed_center(self):
        scale, angle, translation = scene.usd_texture_transform(
            "center .5 .5 scale 4 4 rotate 0 translate 0 0")
        center = np.array((.5, .5)) * scale + translation
        self.assertTrue(bool(np.isfinite(center).all()))
        self.assertEqual(center.tolist(), [.5, .5])
        self.assertEqual(angle, 0)

    def test_invalid_uv_transform_rejected(self):
        for value in ("invalid", "center .5 .5 scale nan 1 rotate 0 translate 0 0"):
            with self.assertRaises(ValueError):
                scene.usd_texture_transform(value)

    def test_missing_archive_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "overrides.json"
            path.write_text(json.dumps({"schema": "source-material-overrides/v1",
                "archives": [{"path": "missing_dir.vpk"}],
                "materials": {"tile/a": "pbr/a"}}))
            with self.assertRaisesRegex(ValueError, "missing VPK"):
                source_content.material_overrides(path)


if __name__ == "__main__":
    unittest.main()
