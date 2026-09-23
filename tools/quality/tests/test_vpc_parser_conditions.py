import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "scripts/waifulib"))
from vpc_parser import compute_statement, parse_vpcs  # noqa: E402


class VpcConditionTests(unittest.TestCase):
    def test_portal2_parenthesized_platform_condition(self):
        expression = "[ ($WIN32 || $OSXALL) && !$NO_STEAM ]"
        self.assertTrue(compute_statement(["WIN32"], expression))
        self.assertTrue(compute_statement(["OSXALL"], expression))
        self.assertFalse(compute_statement(["POSIX"], expression))
        self.assertFalse(compute_statement(["WIN32", "NO_STEAM"], expression))

    def test_not_and_or_precedence_and_define_values(self):
        self.assertTrue(compute_statement(["POSIX=1"], "[!$WIN32 && $POSIX]"))
        self.assertTrue(compute_statement(["WIN32"], "[$WIN32 || $OSXALL && $PS3]"))
        self.assertFalse(compute_statement([], "[$WIN32 || $OSXALL && $PS3]"))
        self.assertTrue(compute_statement([], "[1 && !0]"))

    def test_malformed_condition_fails_without_looping(self):
        for expression in ("[$WIN32 @ $POSIX]", "[($WIN32]", "[$WIN32 &&]"):
            with self.subTest(expression=expression), self.assertRaises(ValueError):
                compute_statement([], expression)

    def test_disabled_folder_excludes_its_sources(self):
        with tempfile.TemporaryDirectory() as directory:
            project = Path(directory)
            (project / "game.vpc").write_text(
                '$Configuration\n{\n\t$PreprocessorDefinitions "$BASE;PORTAL2"\n}\n'
                '$Project "Game"\n{\n'
                '\t$Folder "Economy" [($WIN32 || $OSXALL) && !$NO_STEAM]\n'
                '\t{\n\t\t$File "economy.cpp"\n\t}\n'
                '\t$File "core.cpp"\n}\n'
            )
            (project / "core.cpp").write_text("core")
            (project / "economy.cpp").write_text("economy")
            env = SimpleNamespace(SUBPROJECT_PATH=[str(project)], DEFINES=["POSIX"])
            sources = parse_vpcs(env, ["game.vpc"], ".")["sources"]
            self.assertEqual(["./core.cpp"], sources)

    def test_portal2_linux_selects_gameplay_without_desktop_economy(self):
        client = ROOT / "game/client"
        env = SimpleNamespace(SUBPROJECT_PATH=[str(client)], DEFINES=["POSIX=1", "LINUX=1"])
        sources = parse_vpcs(
            env, ["client_base.vpc", "client_portal_base.vpc", "client_portal2.vpc"], "../.."
        )["sources"]
        self.assertTrue(any(source.endswith("portal_gamerules.cpp") for source in sources))
        self.assertFalse(any(source.endswith("portal2_econ_ui.cpp") for source in sources))
        self.assertFalse(any(source.endswith("portal2_item_inventory.cpp") for source in sources))


if __name__ == "__main__":
    unittest.main()
