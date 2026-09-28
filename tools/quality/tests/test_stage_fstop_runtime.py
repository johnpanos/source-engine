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
    def test_fstop_is_the_mod_ahead_of_portal_content(self):
        _, lines, tail = stage_fstop_runtime.search_paths(PORTAL_GAMEINFO)
        entries = paths(lines)
        self.assertEqual(entries[0], ("game", "portal/custom/source-engine-shaders"))
        self.assertEqual(entries[1], ("game+mod+mod_write+game_write+default_write_path",
                                      "|gameinfo_path|."))
        self.assertEqual(entries[2], ("gamebin", "|gameinfo_path|bin"))
        self.assertEqual(entries[3:5], [("game", "fstop_content"), ("game", "fstop_imported")])
        self.assertLess(entries.index(("game", "fstop_imported")),
                        entries.index(("game", "portal/portal_pak.vpk")))
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
    def test_adds_only_missing_unconditional_tokens(self):
        portal = ('"lang"\n{\n\t"Language"\t"English"\n\t"Tokens"\n\t{\n'
                  '\t\t"Portal_Chapter1_Title"\t"Testchamber 00"\n\t}\n}\n')
        fstop = ('"lang"\n{\n\t"Tokens"\n\t{\n'
                 '\t\t"Portal_Chapter1_Title"\t"Replaced"\n'
                 '\t\t"FSTOP_Camera"\t"CAMERA"\n'
                 '\t\t"FSTOP_Console"\t"PAD"\t[$X360]\n\t}\n}\n')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            resource = root / "content"
            resource.mkdir()
            (resource / "FSTOP_English.txt").write_bytes(b"\xff\xfe" + fstop.encode("utf-16-le"))
            resolver = mock.Mock()
            resolver.read.return_value = (b"\xff\xfe" + portal.encode("utf-16-le"), "vpk")
            with mock.patch.object(stage_fstop_runtime.source_content, "ContentResolver",
                                   return_value=resolver):
                added = stage_fstop_runtime.write_localization(root, resource)
            text = (root / "fstop/resource/fstop_english.txt").read_bytes().decode("utf-16")
        self.assertEqual(added, 1)
        self.assertIn('"FSTOP_Camera"', text)
        self.assertIn('"Testchamber 00"', text)
        self.assertNotIn("Replaced", text)
        self.assertNotIn("FSTOP_Console", text)
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
