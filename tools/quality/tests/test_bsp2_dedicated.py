#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Negative fixtures for the native BSP2 dedicated gate."""

import json
import hashlib
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import bsp2_dedicated  # noqa: E402


class Bsp2DedicatedTests(unittest.TestCase):
    def test_case_manifest_rejects_missing_and_unsafe_work(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "cases.json"
            for cases in ([], [17], [{"map": "../portal", "probes": [[0] * 6] * 3}],
                          [{"map": "example", "probes": [[0] * 6] * 2}],
                          [{"map": "example", "probes": [[0] * 6] * 2 + [[float("nan")] * 6]}],
                          [{"map": "example", "probes": [[0] * 6] * 3}] * 2):
                with self.subTest(cases=cases):
                    path.write_text(json.dumps({"schema": bsp2_dedicated.SCHEMA, "cases": cases}))
                    with self.assertRaises(ValueError):
                        bsp2_dedicated.load_cases(path)

    def test_missing_required_content_fails_with_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            result = bsp2_dedicated.main([
                "--runtime", str(root / "missing-runtime"),
                "--build", str(root / "missing-build"),
                "--tool", str(root / "missing-tool"),
                "--out", str(root / "evidence")])
            self.assertEqual(result, 1)
            evidence = json.loads((root / "evidence/evidence.json").read_text())
            self.assertEqual(evidence["status"], "fail")
            self.assertIn("required Portal runtime is missing", evidence["failures"])

    def test_failed_process_keeps_exit_and_diagnostic_streams(self):
        with tempfile.TemporaryDirectory() as directory:
            stage = Path(directory)
            launcher = stage / "dedicated_launcher"
            launcher.write_text("#!/bin/sh\necho launch-output\necho launch-error >&2\nexit 7\n")
            launcher.chmod(0o755)
            result = bsp2_dedicated.run_server(stage, "example", stage / "example.log", 5, 28040)
            self.assertEqual(result["exit_code"], 7)
            self.assertIn("launch-output", Path(result["stdout"]).read_text())
            self.assertIn("launch-error", Path(result["stderr"]).read_text())

    def test_partial_probe_output_and_wrong_version_fail(self):
        legacy = {"probes": ["a", "b", "c"], "legacy_version": 20,
                  "kind": "legacy-vbsp"}
        bsp2 = {"probes": ["a", "b", "c"], "legacy_version": 20,
                "kind": "bsp2"}
        with self.assertRaisesRegex(ValueError, "every declared probe"):
            bsp2_dedicated.compare_case_observations(4, 20, legacy, bsp2)
        with self.assertRaisesRegex(ValueError, "different legacy version"):
            bsp2_dedicated.compare_case_observations(3, 21, legacy, bsp2)

    def test_foreign_map_cannot_replace_staged_portal_map(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source_maps = root / "foreign"
            staged_maps = root / "stage/portal/maps"
            source_maps.mkdir()
            staged_maps.mkdir(parents=True)
            (source_maps / "example.bsp").write_bytes(b"VBSP" + (21).to_bytes(4, "little"))
            staged = staged_maps / "example.bsp"
            staged.write_bytes(b"original Portal map")
            with self.assertRaisesRegex(ValueError, "would replace staged Portal content"):
                bsp2_dedicated.run_case({"map": "example", "probes": [[0] * 6] * 3},
                                         source_maps, root / "stage", root / "tool", root,
                                         5, 28040, True, bsp2_dedicated.run_server, {})
            self.assertEqual(staged.read_bytes(), b"original Portal map")

    def test_client_capture_requires_active_player(self):
        with tempfile.TemporaryDirectory() as directory:
            log = Path(directory) / "client.log"
            log.write_text('map     : example at: 0 x, 0 y, 0 z\n# 1 "player" active\n')
            self.assertTrue(bsp2_dedicated.client_player_active(log))
            log.write_text('map     : example at: 0 x, 0 y, 0 z\n# 1 "player" spawning\n')
            self.assertFalse(bsp2_dedicated.client_player_active(log))

    def test_source_inventory_rejects_changed_map(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            map_path = root / "example.bsp"
            original = b"VBSP" + (21).to_bytes(4, "little")
            map_path.write_bytes(original)
            manifest = root / "inventory.json"
            manifest.write_text(json.dumps({"schema": "bsp2-corpus-inventory/v1",
                                            "maps": [{"name": map_path.name, "version": 21,
                                                      "sha256": hashlib.sha256(original).hexdigest()}]}))
            cases = [{"map": "example"}]
            self.assertEqual(bsp2_dedicated.check_case_sources(cases, root, manifest)["maps"], 1)
            map_path.write_bytes(original + b"changed")
            with self.assertRaisesRegex(ValueError, "hash or version differs"):
                bsp2_dedicated.check_case_sources(cases, root, manifest)


if __name__ == "__main__":
    unittest.main()
