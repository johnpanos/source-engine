from __future__ import annotations

import copy
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).resolve().parents[1] / "archlint.py"
SPEC = importlib.util.spec_from_file_location("archlint", MODULE_PATH)
assert SPEC and SPEC.loader
archlint = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = archlint
SPEC.loader.exec_module(archlint)


MODULE_BLOCK = {
    "strictIncludeRoots": ["public/hammer/", "hammer/core/"],
    "legacyHost": "hammer.adapters.mfc",
    "modules": [
        {"id": "hammer.geometry", "strict": True, "allowedEdges": []},
        {"id": "hammer.scene", "strict": True, "allowedEdges": ["hammer.geometry"]},
        {"id": "hammer.app", "strict": True, "allowedEdges": ["hammer.geometry", "hammer.scene"]},
        {"id": "hammer.adapters.mfc", "strict": False, "allowedEdges": ["hammer.app", "hammer.scene"]},
    ],
    "nativeTokens": ["afxwin.h", "CWnd", "GtkWidget", "windows.h"],
}


def good_inventory(root: Path) -> dict:
    (root / "hammer").mkdir(parents=True, exist_ok=True)
    (root / "hammer/boundbox.h").write_text("class BoundBox {};\n")
    return {
        "coverage": {"status": "partial", "totalHammerSourceFiles": 452, "filesClassified": 1},
        "files": [
            {
                "path": "hammer/boundbox.h",
                "currentOwner": "hammer.adapters.mfc",
                "destinationModule": "hammer.geometry",
                "responsibility": "geometry",
                "effects": ["pure-computation"],
                "state": "authoritative-document-data",
                "factorization": "extract",
                "evidence": "observed",
            }
        ],
    }


def good_migrations() -> dict:
    return {
        "statuses": archlint.HAM_STATUSES,
        "migrations": [
            {
                "id": "HAM-BUILD-001",
                "title": "t",
                "phase": "H0",
                "responsibility": "composition",
                "evidenceKind": "observed",
                "status": "inventoried",
                "authority": "legacy",
                "dependsOn": [],
                "sources": [],
            },
            {
                "id": "HAM-GEOMETRY-001",
                "title": "t",
                "phase": "H1",
                "responsibility": "geometry",
                "evidenceKind": "observed",
                "status": "inventoried",
                "authority": "legacy",
                "destinationModule": "hammer.geometry",
                "dependsOn": ["HAM-BUILD-001"],
                "sources": [],
            },
        ],
    }


class ModuleGraphTests(unittest.TestCase):
    def test_valid_graph_has_no_errors(self) -> None:
        self.assertEqual([], archlint.validate_module_graph(MODULE_BLOCK))

    def test_unknown_edge_is_rejected(self) -> None:
        block = copy.deepcopy(MODULE_BLOCK)
        block["modules"][1]["allowedEdges"].append("hammer.ghost")
        errors = archlint.validate_module_graph(block)
        self.assertTrue(any("hammer.ghost" in message for message in errors))

    def test_cycle_is_rejected(self) -> None:
        block = copy.deepcopy(MODULE_BLOCK)
        block["modules"][0]["allowedEdges"].append("hammer.scene")  # geometry <-> scene
        errors = archlint.validate_module_graph(block)
        self.assertTrue(any("cycle" in message for message in errors))


class InventoryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_valid_inventory_passes(self) -> None:
        inventory = good_inventory(self.root)
        self.assertEqual([], archlint.validate_inventory(self.root, MODULE_BLOCK, inventory))

    def test_unknown_owner_is_rejected(self) -> None:
        inventory = good_inventory(self.root)
        inventory["files"][0]["destinationModule"] = "hammer.nowhere"
        errors = archlint.validate_inventory(self.root, MODULE_BLOCK, inventory)
        self.assertTrue(any("hammer.nowhere" in message for message in errors))

    def test_duplicate_file_is_rejected(self) -> None:
        inventory = good_inventory(self.root)
        inventory["files"].append(copy.deepcopy(inventory["files"][0]))
        inventory["coverage"]["filesClassified"] = 2
        errors = archlint.validate_inventory(self.root, MODULE_BLOCK, inventory)
        self.assertTrue(any("duplicate" in message for message in errors))

    def test_missing_source_file_is_rejected(self) -> None:
        inventory = good_inventory(self.root)
        inventory["files"][0]["path"] = "hammer/does_not_exist.h"
        errors = archlint.validate_inventory(self.root, MODULE_BLOCK, inventory)
        self.assertTrue(any("does not correspond" in message for message in errors))

    def test_bad_vocabulary_is_rejected(self) -> None:
        inventory = good_inventory(self.root)
        inventory["files"][0]["responsibility"] = "wizardry"
        inventory["files"][0]["state"] = "vibes"
        errors = archlint.validate_inventory(self.root, MODULE_BLOCK, inventory)
        self.assertEqual(2, len(errors))

    def test_complete_coverage_requires_all_files(self) -> None:
        inventory = good_inventory(self.root)
        inventory["coverage"]["status"] = "complete"
        errors = archlint.validate_inventory(self.root, MODULE_BLOCK, inventory)
        self.assertTrue(any("complete coverage" in message for message in errors))

    def test_coverage_count_must_match(self) -> None:
        inventory = good_inventory(self.root)
        inventory["coverage"]["filesClassified"] = 99
        errors = archlint.validate_inventory(self.root, MODULE_BLOCK, inventory)
        self.assertTrue(any("filesClassified" in message for message in errors))


class MigrationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_valid_ledger_passes(self) -> None:
        self.assertEqual([], archlint.validate_migrations(self.root, MODULE_BLOCK, good_migrations()))

    def test_unknown_dependency_is_rejected(self) -> None:
        ledger = good_migrations()
        ledger["migrations"][1]["dependsOn"] = ["HAM-GHOST-001"]
        errors = archlint.validate_migrations(self.root, MODULE_BLOCK, ledger)
        self.assertTrue(any("HAM-GHOST-001" in message for message in errors))

    def test_dependency_cycle_is_rejected(self) -> None:
        ledger = good_migrations()
        ledger["migrations"][0]["dependsOn"] = ["HAM-GEOMETRY-001"]
        errors = archlint.validate_migrations(self.root, MODULE_BLOCK, ledger)
        self.assertTrue(any("cycle" in message for message in errors))

    def test_invalid_status_is_rejected(self) -> None:
        ledger = good_migrations()
        ledger["migrations"][0]["status"] = "almost-done"
        errors = archlint.validate_migrations(self.root, MODULE_BLOCK, ledger)
        self.assertTrue(any("invalid status" in message for message in errors))

    def test_unknown_destination_module_is_rejected(self) -> None:
        ledger = good_migrations()
        ledger["migrations"][1]["destinationModule"] = "hammer.void"
        errors = archlint.validate_migrations(self.root, MODULE_BLOCK, ledger)
        self.assertTrue(any("hammer.void" in message for message in errors))


class NativeTokenScanTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def write(self, relative: str, contents: str) -> None:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents)

    def test_native_token_in_strict_root_is_detected(self) -> None:
        self.write("public/hammer/geometry/boundbox.h", "#include <afxwin.h>\nclass CWnd;\n")
        occurrences = archlint.scan_native_tokens(
            self.root, MODULE_BLOCK["strictIncludeRoots"], MODULE_BLOCK["nativeTokens"]
        )
        self.assertEqual(2, len(occurrences))
        self.assertTrue(all(item.rule == "HAM003" for item in occurrences))

    def test_native_token_outside_strict_root_is_ignored(self) -> None:
        self.write("hammer/mainfrm.cpp", "#include <afxwin.h>\n")
        occurrences = archlint.scan_native_tokens(
            self.root, MODULE_BLOCK["strictIncludeRoots"], MODULE_BLOCK["nativeTokens"]
        )
        self.assertEqual([], occurrences)

    def test_token_in_comment_is_ignored(self) -> None:
        self.write("hammer/core/geometry/clean.h", "// CWnd is not used here\nint clean = 1;\n")
        occurrences = archlint.scan_native_tokens(
            self.root, MODULE_BLOCK["strictIncludeRoots"], MODULE_BLOCK["nativeTokens"]
        )
        self.assertEqual([], occurrences)


if __name__ == "__main__":
    unittest.main()
