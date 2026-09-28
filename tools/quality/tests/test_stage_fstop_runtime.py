import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import stage_fstop_runtime  # noqa: E402


PORTAL_GAMEINFO = """"GameInfo"
{
	game		"Portal"
	title 		"Portal"
	FileSystem
	{
		SteamAppId				400
		SearchPaths
		{
			game+mod		portal/custom/source-engine-shaders
			game+mod			portal/custom/*
			game+mod			portal/portal_pak.vpk
			game				|all_source_engine_paths|hl2/hl2_misc.vpk
			mod+mod_write+default_write_path		|gameinfo_path|.
			game+game_write		portal
			gamebin				portal/bin
			game				|all_source_engine_paths|hl2
			platform			|all_source_engine_paths|platform
		}
	}
}
"""


def paths(lines):
    return [tuple(line.split()) for line in lines]


class SearchPathTests(unittest.TestCase):
    def test_fstop_is_the_mod_and_valve_content_follows_portal(self):
        _, lines, tail = stage_fstop_runtime.search_paths(PORTAL_GAMEINFO)
        entries = paths(lines)
        self.assertEqual(entries[0], ("game", "portal/custom/source-engine-shaders"))
        self.assertEqual(entries[1], ("game+mod+mod_write+game_write+default_write_path",
                                      "|gameinfo_path|."))
        self.assertEqual(entries[2], ("gamebin", "|gameinfo_path|bin"))
        self.assertEqual(entries[-2:], [("game", "fstop_valve"),
                                        ("game", "fstop_valve_tempcontent")])
        self.assertGreater(entries.index(("game", "fstop_valve")),
                           entries.index(("game", "portal/portal_pak.vpk")))
        self.assertNotIn(("game", "fstop_content"), entries)
        self.assertTrue(tail.lstrip().startswith("}"))

    def test_portal_stays_read_only_and_its_gamebin_is_dropped(self):
        _, lines, _ = stage_fstop_runtime.search_paths(PORTAL_GAMEINFO)
        entries = paths(lines)
        self.assertIn(("game", "portal"), entries)
        self.assertNotIn(("gamebin", "portal/bin"), entries)
        writers = [entry for entry in entries if "write" in entry[0] or "mod" in entry[0]]
        self.assertEqual(writers, [("game+mod+mod_write+game_write+default_write_path",
                                    "|gameinfo_path|.")])


class LocalizationTests(unittest.TestCase):
    def test_adds_valve_fstop_and_authored_tokens_portal_lacks(self):
        portal = ('"lang"\n{\n\t"Language"\t"English"\n\t"Tokens"\n\t{\n'
                  '\t\t"Portal_Chapter1_Title"\t"Testchamber 00"\n'
                  '\t\t"FSTOP_Placement"\t"Kept"\n\t}\n}\n')
        valve = ('"lang"\n{\n\t"Tokens"\n\t{\n'
                 '\t\t"Portal_Chapter1_Title"\t"Replaced"\n'
                 '\t\t"fstop_hint_takephoto"\t"USE CAMERA"\n'
                 '\t\t"P2_Unrelated"\t"Not F-Stop"\n'
                 '\t\t"fstop_console"\t"PAD"\t[$X360]\n\t}\n}\n')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resource = root / "content"
            resource.mkdir()
            (resource / "Portal2_English.txt").write_bytes(b"\xff\xfe" + valve.encode("utf-16-le"))
            resolver = mock.Mock()
            resolver.read.return_value = (b"\xff\xfe" + portal.encode("utf-16-le"), "vpk")
            with mock.patch.object(stage_fstop_runtime.source_content, "ContentResolver",
                                   return_value=resolver):
                added = stage_fstop_runtime.write_localization(root, resource)
            text = (root / "fstop/resource/fstop_english.txt").read_bytes().decode("utf-16")
        self.assertEqual(added, 2)
        self.assertIn('"FSTOP_Camera"\t\t"CAMERA"', text)         # authored
        self.assertIn('"fstop_hint_takephoto"\t\t"USE CAMERA"', text)  # Valve's
        self.assertIn('"Testchamber 00"', text)
        self.assertIn('"Kept"', text)
        self.assertNotIn("Replaced", text)
        self.assertNotIn("Not F-Stop", text)
        self.assertNotIn("fstop_console", text)
        self.assertLess(text.index("FSTOP_Camera"), text.rindex("}", 0, text.rindex("}")))


class HudLayoutTests(unittest.TestCase):
    def test_adds_fstop_elements_with_content_layout_or_default(self):
        portal = '"Resource/HudLayout.res"\n{\n\tHudHealth\n\t{\n\t\t"fieldName" "HudHealth"\n\t}\n}\n'
        fstop = ('"Resource/HudLayout.res"\n{\n\tHudViewfinder\n\t{\n'
                 '\t\t"fieldName" "HudViewfinder"\n\t\t"wide"\t "320"\n\t}\n'
                 '\tHudHealth\n\t{\n\t\t"xpos" "99"\n\t}\n}\n')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            scripts = root / "content"
            scripts.mkdir()
            (scripts / "HudLayout.res").write_text(fstop)
            resolver = mock.Mock()
            resolver.read.return_value = (portal.encode(), "vpk")
            (root / "fstop/scripts").mkdir(parents=True)
            with mock.patch.object(stage_fstop_runtime.source_content, "ContentResolver",
                                   return_value=resolver):
                added = stage_fstop_runtime.write_hud_layout(root, scripts)
            text = (root / "fstop/scripts/hudlayout.res").read_text()
        self.assertEqual(added, len(stage_fstop_runtime.HUD_ELEMENTS))
        self.assertIn('"wide"\t "320"', text)       # taken from the content layout
        self.assertIn('"fieldName" "HudIndicator"', text)  # default block
        self.assertNotIn('"xpos" "99"', text)        # Portal's own elements are kept
        self.assertEqual(text.count("{"), text.count("}"))


class ScriptTests(unittest.TestCase):
    def test_valve_sound_scripts_and_authored_ones_join_the_manifest(self):
        manifest = '"game_sounds_manifest"\n{\n\t"precache_file"\t"scripts/game_sounds.txt"\n}\n'
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            scripts = root / "content"
            scripts.mkdir()
            (scripts / "NPC_Sounds_Chicken.txt").write_text('"NPC_Chicken.Clucks" {}\n')
            (scripts / "weapon_portalgun.txt").write_text("WeaponData {}\n")
            resolver = mock.Mock()
            resolver.read.return_value = (manifest.encode(), "vpk")
            with mock.patch.object(stage_fstop_runtime.source_content, "ContentResolver",
                                   return_value=resolver):
                copied = stage_fstop_runtime.write_scripts(root, scripts)
            staged = root / "fstop/scripts"
            text = (staged / "game_sounds_manifest.txt").read_text()
            camera = (staged / "weapon_camera.txt").read_text()
            sounds = (staged / "game_sounds_fstop.txt").read_text()
            self.assertEqual(copied, ["npc_sounds_chicken.txt"])
            self.assertTrue((staged / "npc_sounds_chicken.txt").is_file())
            self.assertFalse((staged / "weapon_portalgun.txt").exists())
        self.assertIn('"scripts/npc_sounds_chicken.txt"', text)
        self.assertIn('"scripts/game_sounds_fstop.txt"', text)
        self.assertIn('"scripts/game_sounds.txt"', text)
        self.assertIn('"scripts/npc_sounds_zombie.txt"', text)
        self.assertEqual(text.count("{"), text.count("}"))
        self.assertIn('"models/weapons/v_cam.mdl"', camera)
        self.assertIn('"camera/snapshot.wav"', sounds)


class StageTests(unittest.TestCase):
    def test_rejects_a_content_root_that_is_not_valves_tree(self):
        with tempfile.TemporaryDirectory() as directory:
            remake = Path(directory) / "svn-master/game"
            (remake / "fstop").mkdir(parents=True)
            (remake / "imported_fstop").mkdir()
            with self.assertRaises(SystemExit) as raised:
                stage_fstop_runtime.stage(Path(directory) / "runtime", None, remake)
        self.assertIn("depot 852", str(raised.exception))


class LinkTests(unittest.TestCase):
    def test_mirrors_valve_assets_under_lowercase_paths(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "portal2"
            (source / "Materials/HUD").mkdir(parents=True)
            (source / "Materials/HUD/Inv_Photo1.vmt").write_text("a")
            (source / "Materials/hud").mkdir()
            (source / "Materials/hud/inv_photo1.vmt").write_text("b")
            (source / "maps").mkdir()
            (source / "maps/p2_lab.bsp").write_text("map")
            linked, shadowed = stage_fstop_runtime.link_assets(
                source, root / "fstop_valve", ("materials", "models"))
            mirrored = root / "fstop_valve/materials/hud/inv_photo1.vmt"
            self.assertEqual(linked, ["materials"])
            self.assertEqual(shadowed, 1)
            self.assertEqual(mirrored.read_text(), "a")
            self.assertTrue(mirrored.is_symlink())
            self.assertFalse((root / "fstop_valve/maps").exists())


class ParticleTests(unittest.TestCase):
    def test_appends_valve_fstop_particles_portal_lacks(self):
        manifest = ('particles_manifest\n{\n\t"file"\t"particles/portals.pcf"\n'
                    '\t"file"\t"!particles/fizzler.pcf"\n}\n')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resolver = mock.Mock()
            resolver.read.return_value = (manifest.encode(), "vpk")
            with mock.patch.object(stage_fstop_runtime.source_content, "ContentResolver",
                                   return_value=resolver):
                added = stage_fstop_runtime.write_particles(root)
            text = (root / "fstop/particles/particles_manifest.txt").read_text()
        self.assertEqual(added, ["airvents.pcf", "chicken.pcf", "geyser.pcf", "zombie.pcf"])
        self.assertIn('"particles/geyser.pcf"', text)
        self.assertIn('"particles/portals.pcf"', text)
        self.assertEqual(text.count("fizzler.pcf"), 1)
        self.assertEqual(text.count("{"), text.count("}"))


class BlobMaterialTests(unittest.TestCase):
    def test_writes_blue_portal2_blob_fallback(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            stage_fstop_runtime.write_blob_material(root)
            text = (root / "fstop/materials/fstop/blob_surface_bounce.vmt").read_text()
        self.assertIn('"VertexLitGeneric"', text)
        self.assertIn('"models/Weapons/V_physics_gun/glueblob"', text)
        self.assertIn('"[0.1 0.6 0.9]"', text)


if __name__ == "__main__":
    unittest.main()
