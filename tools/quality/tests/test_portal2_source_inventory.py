import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "scripts/waifulib"))
from portal2_source_inventory import inventory  # noqa: E402


class Portal2InventoryTests(unittest.TestCase):
    def test_missing_sources_fail_and_case_only_spelling_resolves(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for side in ("client", "server"):
                project = root / "game" / side
                project.mkdir(parents=True)
                (project / f"{side}_portal_base.vpc").write_text(
                    '$File "portal\\Existing.cpp"\n'
                    '$File "portal\\missing.cpp"\n'
                )
                (project / f"{side}_portal2.vpc").write_text(
                    '$File "portal2\\first.cpp" \\\n'
                    '    "portal2\\second.cpp"\n'
                )
                (project / "portal").mkdir()
                (project / "portal/existing.cpp").write_text("source")
                (project / "portal2").mkdir()
                (project / "portal2/first.cpp").write_text("source")
            records = inventory(root)
            missing = {row["path"] for row in records if not row["present"]}
            self.assertEqual({
                "game/client/portal/missing.cpp",
                "game/client/portal2/second.cpp",
                "game/server/portal/missing.cpp",
                "game/server/portal2/second.cpp",
            }, missing)


if __name__ == "__main__":
    unittest.main()
