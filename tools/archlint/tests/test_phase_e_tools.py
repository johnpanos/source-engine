from __future__ import annotations

import copy
import contextlib
import importlib.util
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).resolve().parents[1] / "archlint.py"
SPEC = importlib.util.spec_from_file_location("archlint_phase_e", MODULE_PATH)
assert SPEC and SPEC.loader
archlint = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = archlint
SPEC.loader.exec_module(archlint)


DISPOSITIONS = {
    "phase-e-vrad": "child-process",
    "phase-e-vvis": "normal-executable",
    "phase-e-shadercompile": "child-process",
    "phase-e-texturecompile": "child-process",
    "phase-e-vtex-vtexconv": "linked-tool-library",
    "phase-e-vmpi-worker": "child-process",
    "phase-e-studiomdl": "normal-executable",
    "phase-e-tool-dictionaries": "tool-extension-host",
    "phase-e-tool-integrations": "tool-only-provider",
}


class PhaseEToolMigrationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        (self.root / "architecture").mkdir()
        self.source_by_cohort: dict[str, str] = {}
        for cohort_id in sorted(archlint.TOOL_MIGRATION_REQUIRED_COHORTS):
            relative = f"tools/{cohort_id.removeprefix('phase-e-')}.cpp"
            self.write(relative, "void PhaseEToolPlaceholder() {}\n")
            self.source_by_cohort[cohort_id] = relative

        self.write(
            "public/ilaunchabledll.h",
            '#define LAUNCHABLE_DLL_INTERFACE_VERSION "launchable_dll_1"\n'
            "class ILaunchableDLL {};\n",
        )
        vrad_path = self.source_by_cohort["phase-e-vrad"]
        self.write(
            vrad_path,
            "ILaunchableDLL *CreateLegacyTool( LAUNCHABLE_DLL_INTERFACE_VERSION );\n",
        )
        self.ledger = self.good_ledger()

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def write(self, relative: str, contents: str) -> None:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents, encoding="utf-8")

    def good_ledger(self) -> dict:
        cohorts = []
        for cohort_id, disposition in DISPOSITIONS.items():
            source = self.source_by_cohort[cohort_id]
            wrapper_sites = []
            if cohort_id == "phase-e-vrad":
                wrapper_sites.append(
                    {
                        "path": source,
                        "symbolCounts": {
                            "ILaunchableDLL": 1,
                            "LAUNCHABLE_DLL_INTERFACE_VERSION": 1,
                        },
                        "reason": "Frozen wrapper pending the child-process cutover.",
                    }
                )
            cohorts.append(
                {
                    "id": cohort_id,
                    "title": cohort_id,
                    "scope": f"Fixture scope for {cohort_id}",
                    "disposition": disposition,
                    "status": "inventoried",
                    "sourcePaths": [source],
                    "wrapperSites": wrapper_sites,
                    "deletionConditions": ["replacement workflow passes"],
                    "acceptanceEvidence": ["installed workflow result"],
                    "evidence": [],
                    "remainingWork": ["implement and verify the target disposition"],
                }
            )
        return {
            "version": 1,
            "schema": archlint.TOOL_MIGRATION_SCHEMA,
            "description": "Phase E fixture ledger.",
            "statuses": list(archlint.TOOL_MIGRATION_STATUSES),
            "dispositions": list(archlint.TOOL_MIGRATION_DISPOSITIONS),
            "abiDeclarations": [
                {
                    "path": "public/ilaunchabledll.h",
                    "symbolCounts": {
                        "ILaunchableDLL": 1,
                        "LAUNCHABLE_DLL_INTERFACE_VERSION": 1,
                    },
                    "reason": "Frozen legacy ABI declaration.",
                    "deleteWhen": ["all legacy consumers are retired"],
                }
            ],
            "cohorts": cohorts,
        }

    def cohort(self, ledger: dict, cohort_id: str) -> dict:
        return next(item for item in ledger["cohorts"] if item["id"] == cohort_id)

    def errors(self, ledger: dict | None = None) -> list[str]:
        return archlint.validate_tool_migrations(self.root, ledger or self.ledger)

    def assert_error_contains(self, needle: str, ledger: dict | None = None) -> None:
        errors = self.errors(ledger)
        self.assertTrue(
            any(needle in error for error in errors),
            f"expected {needle!r} in errors:\n" + "\n".join(errors),
        )

    def test_complete_fixture_ledger_and_exact_wrapper_ratchet_pass(self) -> None:
        self.assertEqual([], self.errors())

    def test_comments_and_literals_do_not_create_wrapper_uses(self) -> None:
        self.write(
            "tools/comments.cpp",
            '// ILaunchableDLL LAUNCHABLE_DLL_INTERFACE_VERSION\n'
            'const char *text = "ILaunchableDLL LAUNCHABLE_DLL_INTERFACE_VERSION";\n',
        )
        self.assertEqual([], self.errors())

    def test_unregistered_wrapper_use_is_rejected(self) -> None:
        self.write("tools/new_wrapper.cpp", "ILaunchableDLL *NewWrapper();\n")
        self.assert_error_contains("ARCH107 tools/new_wrapper.cpp:1: unregistered ILaunchableDLL")

    def test_extra_use_in_a_registered_file_is_rejected_by_exact_count(self) -> None:
        path = self.source_by_cohort["phase-e-vrad"]
        self.write(
            path,
            "ILaunchableDLL *First( LAUNCHABLE_DLL_INTERFACE_VERSION );\n"
            "ILaunchableDLL *Second();\n",
        )
        self.assert_error_contains("ledger expects 1 ILaunchableDLL occurrence(s), found 2")

    def test_stale_wrapper_claim_is_rejected(self) -> None:
        path = self.source_by_cohort["phase-e-vrad"]
        self.write(path, "void WrapperRemoved() {}\n")
        self.assert_error_contains("ledger expects 1 ILaunchableDLL occurrence(s), found 0")
        self.assert_error_contains(
            "ledger expects 1 LAUNCHABLE_DLL_INTERFACE_VERSION occurrence(s), found 0"
        )

    def test_missing_required_cohort_is_rejected(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        ledger["cohorts"] = [
            item for item in ledger["cohorts"] if item["id"] != "phase-e-studiomdl"
        ]
        self.assert_error_contains("required tool migration cohort 'phase-e-studiomdl' is missing", ledger)

    def test_duplicate_cohort_is_rejected(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        ledger["cohorts"].append(copy.deepcopy(ledger["cohorts"][0]))
        self.assert_error_contains("declared 2 times", ledger)

    def test_duplicate_symbol_claim_is_rejected(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        site = copy.deepcopy(self.cohort(ledger, "phase-e-vrad")["wrapperSites"][0])
        self.cohort(ledger, "phase-e-vvis")["sourcePaths"].append(site["path"])
        self.cohort(ledger, "phase-e-vvis")["wrapperSites"].append(site)
        self.assert_error_contains("is claimed by both", ledger)

    def test_unknown_status_and_disposition_are_rejected(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        cohort = self.cohort(ledger, "phase-e-vvis")
        cohort["status"] = "done"
        cohort["disposition"] = "load-whatever-dll"
        self.assert_error_contains("invalid status 'done'", ledger)
        self.assert_error_contains("invalid disposition 'load-whatever-dll'", ledger)

    def test_non_retired_cohort_must_state_remaining_work(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        self.cohort(ledger, "phase-e-vvis")["remainingWork"] = []
        self.assert_error_contains("non-retired cohort must state remainingWork", ledger)

    def test_retired_status_cannot_hide_wrappers_or_remaining_work(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        cohort = self.cohort(ledger, "phase-e-vrad")
        cohort["status"] = "retired"
        self.assert_error_contains("retired cohort still declares wrapperSites", ledger)
        self.assert_error_contains("retired cohort still declares remainingWork", ledger)
        self.assert_error_contains("retired cohort requires current acceptance evidence", ledger)

    def test_retired_status_with_no_debt_and_current_evidence_is_valid(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        cohort = self.cohort(ledger, "phase-e-vvis")
        cohort["status"] = "retired"
        cohort["remainingWork"] = []
        self.write("quality-results/vvis-installed.json", "{}\n")
        cohort["evidence"] = [
            {
                "path": "quality-results/vvis-installed.json",
                "sourceRevision": "fixture-revision",
                "profile": "fixture-profile",
                "testSelectors": ["vvis/installed-workflow"],
            }
        ]
        self.assertEqual([], self.errors(ledger))

    def test_missing_or_unidentified_evidence_cannot_retire_a_cohort(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        cohort = self.cohort(ledger, "phase-e-vvis")
        cohort["status"] = "retired"
        cohort["remainingWork"] = []
        cohort["evidence"] = [
            {
                "path": "quality-results/missing.json",
                "sourceRevision": "",
                "profile": "fixture-profile",
                "testSelectors": [],
            }
        ]
        self.assert_error_contains("missing non-empty 'sourceRevision'", ledger)
        self.assert_error_contains("testSelectors must be a non-empty string list", ledger)
        self.assert_error_contains("evidence path 'quality-results/missing.json' does not exist", ledger)
        self.assert_error_contains("retired cohort requires current acceptance evidence", ledger)

    def test_verified_status_requires_evidence(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        self.cohort(ledger, "phase-e-vvis")["status"] = "verified"
        self.assert_error_contains("verified cohort requires current acceptance evidence", ledger)

    def test_missing_source_and_wrapper_not_in_sources_are_rejected(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        cohort = self.cohort(ledger, "phase-e-vrad")
        cohort["sourcePaths"] = ["tools/missing.cpp"]
        self.assert_error_contains("source path 'tools/missing.cpp' does not exist", ledger)
        self.assert_error_contains("wrapper-site", ledger)
        self.assert_error_contains("is absent from sourcePaths", ledger)

    def test_abi_declaration_needs_a_deletion_condition(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        ledger["abiDeclarations"][0]["deleteWhen"] = []
        self.assert_error_contains("needs non-empty deleteWhen", ledger)

    def test_symbol_counts_reject_unknown_symbols_and_nonpositive_counts(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        counts = self.cohort(ledger, "phase-e-vrad")["wrapperSites"][0]["symbolCounts"]
        counts["CreateInterfaceFn"] = 1
        counts["ILaunchableDLL"] = 0
        self.assert_error_contains("names unknown symbol 'CreateInterfaceFn'", ledger)
        self.assert_error_contains("must be a positive integer", ledger)

    def test_schema_and_canonical_enums_are_versioned(self) -> None:
        ledger = copy.deepcopy(self.ledger)
        ledger["version"] = 2
        ledger["schema"] = "rfc0001-tool-migrations/v2"
        ledger["statuses"] = ["done"]
        ledger["dispositions"] = ["dll"]
        self.assert_error_contains("version must be 1", ledger)
        self.assert_error_contains("schema must be", ledger)
        self.assert_error_contains("statuses do not match", ledger)
        self.assert_error_contains("dispositions do not match", ledger)

    def test_tools_command_reads_the_versioned_ledger(self) -> None:
        path = self.root / "architecture/tool_migrations.json"
        path.write_text(json.dumps(self.ledger), encoding="utf-8")
        self.assertEqual(0, archlint.tool_migrations_command(self.root))

    def test_repository_check_runs_the_phase_e_ratchet(self) -> None:
        (self.root / "architecture/tool_migrations.json").write_text(
            json.dumps(self.ledger), encoding="utf-8"
        )
        (self.root / "architecture/modules.json").write_text(
            json.dumps(
                {
                    "legacyAbi": {"paths": []},
                    "loaderInventory": {"classifications": []},
                }
            ),
            encoding="utf-8",
        )
        (self.root / "architecture/baseline.json").write_text(
            json.dumps({"entries": []}), encoding="utf-8"
        )
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(0, archlint.main(["check", "--all"], self.root))
        self.write("tools/unregistered.cpp", "ILaunchableDLL *Unregistered();\n")
        with contextlib.redirect_stdout(io.StringIO()) as output:
            self.assertEqual(1, archlint.main(["check", "--all"], self.root))
        self.assertIn("ARCH107", output.getvalue())


if __name__ == "__main__":
    unittest.main()
