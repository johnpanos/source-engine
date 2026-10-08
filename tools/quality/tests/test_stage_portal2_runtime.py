#!/usr/bin/env python3
"""Tests for stage_portal2_runtime's Workshop pack selection and mounts."""

from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))

import stage_portal2_runtime  # noqa: E402


class WorkshopSelectionTests(unittest.TestCase):
    PACKS = [
        {"id": "a", "include": ["materials/", "models/"],
         "exclude": ["models/npcs/core/", "materials/glass.vmt"]},
        {"id": "b", "include": ["materials/", "models/"]},
        {"id": "c", "include": ["materials/"]},
    ]

    def select(self, listings):
        return stage_portal2_runtime.select_workshop_files(self.PACKS, listings)

    def test_maps_and_scripts_are_never_taken(self):
        chosen, _ = self.select({"a": ["maps/sp_a2_core.bsp", "scripts/vscripts/x.nut",
                                       "materials/a.vmt"], "b": [], "c": []})
        self.assertEqual(list(chosen["a"]), ["materials/a.vmt"])

    def test_a_model_comes_whole_from_the_first_pack(self):
        chosen, _ = self.select({
            "a": ["models/boss.mdl", "models/boss.dx90.vtx"],
            "b": ["models/boss.mdl", "models/boss.vvd", "models/boss.phy", "models/cube.mdl"],
            "c": []})
        self.assertEqual(sorted(chosen["a"]), ["models/boss.dx90.vtx", "models/boss.mdl"])
        self.assertEqual(sorted(chosen["b"]), ["models/cube.mdl"])

    def test_excluded_paths_fall_to_the_next_pack(self):
        chosen, _ = self.select({
            "a": ["models/npcs/core/core.mdl", "materials/glass.vmt"],
            "b": ["models/npcs/core/core.mdl", "models/npcs/core/core.vvd"],
            "c": ["materials/glass.vmt", "models/ignored.mdl"]})
        self.assertEqual(chosen["a"], {})
        self.assertEqual(sorted(chosen["b"]),
                         ["models/npcs/core/core.mdl", "models/npcs/core/core.vvd"])
        self.assertEqual(list(chosen["c"]), ["materials/glass.vmt"])

    def test_missing_pack_is_reported(self):
        chosen, missing = self.select({"a": [], "b": None, "c": []})
        self.assertNotIn("b", chosen)
        self.assertEqual(missing, {"b": "not installed"})

    def test_mounts_precede_retail_in_manifest_order(self):
        contents = "\"GameInfo\"\n{\n\tFileSystem\n\t{\n\t\tSearchPaths\n\t\t{\n\t\t}\n\t}\n}\n"
        staged = stage_portal2_runtime.retail_search_paths(contents, workshop=("x", "y"))
        x, y = staged.find("../workshop/x"), staged.find("../workshop/y")
        retail = staged.find("../update/pak01_dir.vpk")
        self.assertTrue(0 <= x < y < retail)

    def test_collision_versions(self):
        import struct

        def solid(version):
            body = b"VPHY" + struct.pack("<hh", version, 0) + bytes(20)
            return struct.pack("<i", len(body)) + body

        data = struct.pack("<iii", 16, 0, 2) + bytes(4) + solid(0x100) + solid(0x101) + b"kv\0"
        self.assertEqual(stage_portal2_runtime.collision_versions(data), [0x100, 0x101])
        with self.assertRaises(ValueError):
            stage_portal2_runtime.collision_versions(data[:30])

    def test_namespace_moves_materials_and_the_model_directory(self):
        ns = {"from": "models/npcs/glados/", "to": "models/npcs/glalab/"}
        files = ["materials/models/npcs/glados/head.vmt", "materials/models/npcs/glados/head_hi.vtf"]
        path, data = stage_portal2_runtime.namespaced(
            files[0], b'"VertexLitGeneric" { "$basetexture" "models\\npcs\\glados\\head_hi" '
            b'"$phongexponenttexture" "models/npcs/glados/head_exponent" }', ns, files)
        self.assertEqual(path, "materials/models/npcs/glalab/head.vmt")
        self.assertIn(b'"models/npcs/glalab/head_hi"', data)
        # A retail texture the pack does not ship keeps its retail path.
        self.assertIn(b'"models/npcs/glados/head_exponent"', data)
        path, data = stage_portal2_runtime.namespaced(
            "models/npcs/glados/g.mdl", b"IDST\0npcs/glados/g.mdl\0models\\npcs\\glados\\\0", ns, files)
        self.assertEqual(path, "models/npcs/glados/g.mdl")
        self.assertEqual(data, b"IDST\0npcs/glados/g.mdl\0models\\npcs\\glalab\\\0")
        with self.assertRaises(ValueError):
            stage_portal2_runtime.namespaced("models/x.mdl", b"IDST", ns, files)

    def test_drop_keys_removes_only_the_listed_dead_key(self):
        vmt = b'"VertexlitGeneric"\n{\n\t"$selfillum" "1"\n                "$selfilumtint" "[5 0 0]"\n}\n'
        drops = [{"path": "materials/x.vmt", "key": "$selfilumtint"}]
        out = stage_portal2_runtime.drop_vmt_keys("materials/x.vmt", vmt, drops)
        self.assertNotIn(b"selfilumtint", out)
        self.assertIn(b'"$selfillum" "1"', out)
        self.assertEqual(stage_portal2_runtime.drop_vmt_keys("materials/y.vmt", vmt, drops), vmt)
        with self.assertRaises(ValueError):
            stage_portal2_runtime.drop_vmt_keys("materials/x.vmt", out, drops)

    def test_checked_in_manifest_takes_no_maps_or_scripts(self):
        import json
        manifest = json.loads(stage_portal2_runtime.WORKSHOP_MANIFEST.read_text())
        self.assertEqual(manifest["format"], "p2ce-workshop-mounts/v1")
        ids = [p["id"] for p in manifest["packs"]]
        self.assertEqual(len(ids), len(set(ids)))
        self.assertNotIn("3523648265", ids)  # November Doors, user decision
        for pack in manifest["packs"]:
            for prefix in pack["include"]:
                self.assertIn(prefix, ("materials/", "models/"))


if __name__ == "__main__":
    unittest.main()
