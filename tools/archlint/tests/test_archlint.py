from __future__ import annotations

import importlib.util
import json
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


MANIFEST = {
    "legacyAbi": {"paths": ["legacy/interface.h"]},
    "loaderInventory": {
        "classifications": [
            {
                "name": "tool-indirection",
                "pathPrefixes": ["tools/"],
                "reason": "test tool",
            }
        ]
    },
}


class ArchlintTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        (self.root / "architecture").mkdir()
        (self.root / "architecture/modules.json").write_text(json.dumps(MANIFEST))

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def write(self, relative: str, contents: str) -> None:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents)

    def test_new_direct_load_is_not_in_empty_baseline(self) -> None:
        self.write("engine/new.cpp", 'void f() { Sys_LoadModule("new"); }\n')
        current = archlint.scan(self.root, MANIFEST)
        new, stale = archlint.compare_baseline(current, {"entries": []})
        self.assertEqual(["ARCH101"], [item.rule for item in new])
        self.assertEqual([], stale)

    def test_existing_occurrence_survives_line_movement(self) -> None:
        self.write("engine/old.cpp", 'void f() { Sys_GetFactory(module); }\n')
        original = archlint.scan(self.root, MANIFEST)
        baseline = archlint.baseline_document(original)
        self.write("engine/old.cpp", '\n\nvoid f() { Sys_GetFactory(module); }\n')
        new, stale = archlint.compare_baseline(archlint.scan(self.root, MANIFEST), baseline)
        self.assertEqual([], new)
        self.assertEqual([], stale)

    def test_changed_occurrence_is_new_and_old_one_is_stale(self) -> None:
        self.write("engine/old.cpp", 'void f() { Sys_LoadModule("old"); }\n')
        baseline = archlint.baseline_document(archlint.scan(self.root, MANIFEST))
        self.write("engine/old.cpp", 'void f() { Sys_LoadModule("replacement"); }\n')
        new, stale = archlint.compare_baseline(archlint.scan(self.root, MANIFEST), baseline)
        self.assertEqual(1, len(new))
        self.assertEqual(1, len(stale))

    def test_create_interface_fn_is_allowed_only_in_legacy_package(self) -> None:
        self.write("legacy/interface.h", "CreateInterfaceFn Sys_GetFactory();\n")
        self.write("engine/new.h", "CreateInterfaceFn NewFactory();\n")
        current = archlint.scan(self.root, MANIFEST)
        self.assertEqual(["engine/new.h"], [item.path for item in current if item.rule == "ARCH105"])

    def test_comments_and_strings_do_not_create_occurrences(self) -> None:
        self.write(
            "engine/clean.cpp",
            '// Sys_LoadModule("comment")\nconst char *s = "Sys_GetFactory(module)";\n',
        )
        self.assertEqual([], archlint.scan(self.root, MANIFEST))

    def test_inventory_assigns_explicit_classification(self) -> None:
        self.write("tools/example.cpp", 'void f() { Sys_LoadModule("tool"); }\n')
        inventory = archlint.inventory_document(self.root, MANIFEST)
        self.assertEqual("tool-indirection", inventory["sites"][0]["classification"])


if __name__ == "__main__":
    unittest.main()
