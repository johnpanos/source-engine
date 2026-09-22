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

    def test_each_frozen_loader_surface_is_detected(self) -> None:
        cases = {
            "ARCH101": 'Sys_LoadModule("module");',
            "ARCH102": "Sys_GetFactory(module);",
            "ARCH103": 'g_pFileSystem->LoadModule("module");',
            "ARCH104": 'CDllDemandLoader loader("module");',
        }
        for index, (expected_rule, statement) in enumerate(cases.items()):
            with self.subTest(rule=expected_rule):
                relative = f"engine/case_{index}.cpp"
                self.write(relative, f"void f() {{ {statement} }}\n")
                rules = [
                    item.rule
                    for item in archlint.scan_file(
                        self.root, self.root / relative, MANIFEST
                    )
                ]
                self.assertIn(expected_rule, rules)

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

    def test_factory_cannot_be_treated_as_an_app_system_module(self) -> None:
        cases = (
            "LoadModule( CreateInterfaceFn factory );",
            "LoadModule( Sys_GetFactoryThis() );",
            "LoadModule( VStdLib_GetICVarFactory() );",
            "LoadModule( FileSystemFactory );",
            "LoadModule( factory );",
        )
        for index, statement in enumerate(cases):
            with self.subTest(statement=statement):
                relative = f"app/case_{index}.cpp"
                self.write(relative, statement + "\n")
                rules = [
                    item.rule
                    for item in archlint.scan_file(
                        self.root, self.root / relative, MANIFEST
                    )
                ]
                self.assertIn("ARCH106", rules)

    def test_comments_and_strings_do_not_create_occurrences(self) -> None:
        self.write(
            "engine/clean.cpp",
            '// Sys_LoadModule("comment")\nconst char *s = "Sys_GetFactory(module)";\n',
        )
        self.assertEqual([], archlint.scan(self.root, MANIFEST))

    def test_escaped_literal_newline_does_not_shift_diagnostics(self) -> None:
        self.write(
            "engine/continued.cpp",
            'const char *s = "continued\\\ntext";\nvoid f() { Sys_LoadModule("module"); }\n',
        )
        occurrence = archlint.scan(self.root, MANIFEST)[0]
        self.assertEqual(3, occurrence.line)
        self.assertIn("Sys_LoadModule", occurrence.excerpt)

    def test_inventory_assigns_explicit_classification(self) -> None:
        self.write("tools/example.cpp", 'void f() { Sys_LoadModule("tool"); }\n')
        inventory = archlint.inventory_document(self.root, MANIFEST)
        self.assertEqual("tool-indirection", inventory["sites"][0]["classification"])

    def test_native_loader_inventory_requires_runtime_telemetry(self) -> None:
        self.write("engine/native.cpp", 'void f() { dlopen("provider.so", 1); }\n')
        inventory = archlint.inventory_document(self.root, MANIFEST)
        native = [
            site for site in inventory["sites"]
            if site["mechanism"] == "native-loader"
        ]
        self.assertEqual("missing", native[0]["telemetry"])
        self.assertEqual(
            {"covered": 0, "total": 1, "status": "partial"},
            inventory["nativeTelemetryCoverage"],
        )

    def test_native_loader_adapter_marks_runtime_coverage_complete(self) -> None:
        self.write(
            "engine/native.cpp",
            '#include "tier0/native_module_load_telemetry.h"\n'
            'void f() { dlopen("provider.so", 1); }\n',
        )
        inventory = archlint.inventory_document(self.root, MANIFEST)
        native = [
            site for site in inventory["sites"]
            if site["mechanism"] == "native-loader"
        ]
        self.assertEqual("tier0-native-adapter", native[0]["telemetry"])
        self.assertEqual(
            {"covered": 1, "total": 1, "status": "complete"},
            inventory["nativeTelemetryCoverage"],
        )

    def test_vpc_wide_loader_requires_matching_adapter_macro(self) -> None:
        provider = "external/vpc/public/tier0/native_module_load_telemetry.h"
        self.write(provider, "#define LoadLibraryA(path) Wrapped(path)\n")
        self.write(
            "external/vpc/tier0/native.cpp",
            'void f() { LoadLibraryW(L"provider.dll"); }\n',
        )
        inventory = archlint.inventory_document(self.root, MANIFEST)
        native = [
            site for site in inventory["sites"]
            if site["mechanism"] == "native-loader"
        ]
        self.assertEqual("missing", native[0]["telemetry"])

        self.write(
            provider,
            "#define LoadLibraryA(path) Wrapped(path)\n"
            "#define LoadLibraryW(path) WrappedWide(path)\n",
        )
        inventory = archlint.inventory_document(self.root, MANIFEST)
        native = [
            site for site in inventory["sites"]
            if site["mechanism"] == "native-loader"
        ]
        self.assertEqual("vpc-platform-native-adapter", native[0]["telemetry"])

    def test_moved_loader_file_is_triaged_as_a_relocation(self) -> None:
        # A frozen loader call that simply moves files is a relocation, not a new
        # architectural dependency: same rule + excerpt, different path.
        self.write("tier1/telemetry.cpp", 'void f() { Sys_LoadModule("mod"); }\n')
        baseline = archlint.baseline_document(archlint.scan(self.root, MANIFEST))
        (self.root / "tier1/telemetry.cpp").unlink()
        self.write("tier0/telemetry.cpp", 'void f() { Sys_LoadModule("mod"); }\n')
        new, stale = archlint.compare_baseline(archlint.scan(self.root, MANIFEST), baseline)
        relocations, genuinely_new, removed = archlint.classify_drift(new, stale)
        self.assertEqual(1, len(relocations))
        self.assertEqual([], genuinely_new)
        self.assertEqual([], removed)
        occurrence, entry = relocations[0]
        self.assertEqual("tier1/telemetry.cpp", entry["path"])
        self.assertEqual("tier0/telemetry.cpp", occurrence.path)

    def test_genuinely_new_call_is_not_a_relocation(self) -> None:
        self.write("tier1/telemetry.cpp", 'void f() { Sys_LoadModule("mod"); }\n')
        baseline = archlint.baseline_document(archlint.scan(self.root, MANIFEST))
        self.write("engine/other.cpp", 'void g() { Sys_LoadModule("different"); }\n')
        new, stale = archlint.compare_baseline(archlint.scan(self.root, MANIFEST), baseline)
        relocations, genuinely_new, removed = archlint.classify_drift(new, stale)
        self.assertEqual([], relocations)
        self.assertEqual(1, len(genuinely_new))
        self.assertEqual([], removed)


if __name__ == "__main__":
    unittest.main()
