#!/usr/bin/env python3
"""Tests for the retail mirror's private write directory (stage_portal2_runtime)."""

from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))

import stage_portal2_runtime  # noqa: E402


def fake_install(root):
    install = root / "Portal 2"
    update = install / stage_portal2_runtime.RETAIL_WRITE_DIR
    (update / "cfg").mkdir(parents=True)
    (update / "pak01_dir.vpk").write_bytes(b"vpk")
    (update / "cfg/config.cfg").write_text('hud_quickinfo "1"\n')
    return install


class PrivateRetailWriteDirTests(unittest.TestCase):
    def test_config_writes_stay_in_the_mirror(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            install, mirror = fake_install(root), root / "mirror"
            # A mirror made before the fix links update/ whole.
            mirror.mkdir()
            (mirror / "update").symlink_to(install / "update", target_is_directory=True)

            update = stage_portal2_runtime.private_retail_write_dir(install, mirror)

            self.assertFalse(update.is_symlink())
            self.assertTrue((update / "pak01_dir.vpk").is_symlink())
            self.assertEqual((update / "pak01_dir.vpk").resolve(),
                             (install / "update/pak01_dir.vpk").resolve())
            self.assertFalse((update / "cfg").is_symlink())
            self.assertEqual((update / "cfg/config.cfg").read_text(), 'hud_quickinfo "1"\n')
            # What the retail binary does on quit after a harness's hud_quickinfo 0.
            (update / "cfg/config.cfg").write_text('hud_quickinfo "0"\n')
            self.assertEqual((install / "update/cfg/config.cfg").read_text(),
                             'hud_quickinfo "1"\n')

    def test_repeat_keeps_the_mirror_config(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            install, mirror = fake_install(root), root / "mirror"
            update = stage_portal2_runtime.private_retail_write_dir(install, mirror)
            (update / "cfg/config.cfg").write_text('hud_quickinfo "0"\n')
            (install / "update/pak02_dir.vpk").write_bytes(b"vpk")

            stage_portal2_runtime.private_retail_write_dir(install, mirror)

            self.assertEqual((update / "cfg/config.cfg").read_text(), 'hud_quickinfo "0"\n')
            self.assertTrue((update / "pak02_dir.vpk").is_symlink())

    def test_linked_update_leaks_to_the_installation(self):
        # Control: the layout the helper replaces does write through.
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            install, mirror = fake_install(root), root / "mirror"
            mirror.mkdir()
            (mirror / "update").symlink_to(install / "update", target_is_directory=True)
            (mirror / "update/cfg/config.cfg").write_text('hud_quickinfo "0"\n')
            self.assertEqual((install / "update/cfg/config.cfg").read_text(),
                             'hud_quickinfo "0"\n')


def full_install(home):
    """An installation with the entries retail writes, under a fake home's
    Steam library."""
    install = home / ".local/share/Steam/steamapps/common/Portal 2"
    update = install / "update"
    for relative, text in (("update/cfg/config.cfg", 'hud_quickinfo "1"\n'),
                           ("update/glshaders.cfg", "shaders\n"),
                           ("update/pak01.vpk.sound.cache", "cache\n"),
                           ("update/sound/sound.cache", "cache\n"),
                           ("update/save/game_instructor_counts.txt", "player hints\n"),
                           ("update/resource/ui.res", "ui\n"),
                           ("portal2/SAVE/76561190000000000/steam_autocloud.vdf", "cloud\n"),
                           ("portal2/SAVE/76561190000000000/player.sav", "player save\n"),
                           ("portal2_linux", "ELF")):
        path = install / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
    (update / "pak01_dir.vpk").write_bytes(b"vpk")
    return install


def old_mirror(install, mirror):
    """portal2_audio's layout before: the executable, update/ and SAVE linked."""
    (mirror / "portal2").mkdir(parents=True)
    for name in ("portal2_linux", "update"):
        (mirror / name).symlink_to(install / name)
    (mirror / "portal2/SAVE").symlink_to(install / "portal2/SAVE", target_is_directory=True)


class RetailWriteTargetTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.home = Path(self.directory.name) / "home"
        self.install = full_install(self.home)
        self.mirror = Path(self.directory.name) / "mirror"
        old_mirror(self.install, self.mirror)

    def write_paths(self):
        return [self.mirror / relative for relative in stage_portal2_runtime.RETAIL_WRITE_PATHS]

    def test_saves_caches_and_the_executable_become_private(self):
        stage_portal2_runtime.private_retail_write_dir(self.install, self.mirror)
        update = self.mirror / "update"
        for name in ("glshaders.cfg", "pak01.vpk.sound.cache", "sound", "cfg"):
            self.assertFalse((update / name).is_symlink(), name)
            self.assertTrue((update / name).exists(), name)
        self.assertTrue((update / "pak01_dir.vpk").is_symlink())
        self.assertTrue((update / "resource").is_symlink())
        for relative in stage_portal2_runtime.RETAIL_EMPTY_WRITE_DIRS:
            path = self.mirror / relative
            self.assertTrue(path.is_dir() and not path.is_symlink(), relative)
            self.assertEqual(list(path.iterdir()), [], relative)
        executable = self.mirror / "portal2_linux"
        self.assertFalse(executable.is_symlink())
        self.assertEqual(executable.read_text(), "ELF")
        # What a harness's retail run writes stays in the mirror.
        (self.mirror / "portal2/SAVE/autosave.sav").write_text("harness save\n")
        (update / "glshaders.cfg").write_text("harness\n")
        (update / "save/game_instructor_counts.txt").write_text("harness hints\n")
        self.assertFalse((self.install / "portal2/SAVE/autosave.sav").exists())
        self.assertEqual((self.install / "update/glshaders.cfg").read_text(), "shaders\n")
        self.assertEqual((self.install / "update/save/game_instructor_counts.txt").read_text(),
                         "player hints\n")

    def test_linked_save_leaks_to_the_installation(self):
        # Control: the layout the helper replaces writes into the player's saves.
        (self.mirror / "portal2/SAVE/autosave.sav").write_text("harness save\n")
        self.assertTrue((self.install / "portal2/SAVE/autosave.sav").is_file())

    def test_launch_sandbox_refuses_the_old_mirror_and_accepts_the_new(self):
        import io
        import launch_sandbox
        environ = {"HOME": str(self.home)}
        with self.assertRaises(launch_sandbox.SandboxError):
            launch_sandbox.check_write_paths(self.write_paths(), environ=environ,
                                             repo=self.directory.name, log=io.StringIO())
        stage_portal2_runtime.private_retail_write_dir(self.install, self.mirror)
        checked = launch_sandbox.check_write_paths(self.write_paths(), environ=environ,
                                                   repo=self.directory.name, log=io.StringIO())
        self.assertIn(str(self.mirror / "portal2/SAVE"), checked)


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
