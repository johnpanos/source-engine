#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Required-input and evidence checks for the native F1 aggregate gate."""

import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import bsp2_f1_gate  # noqa: E402


class Bsp2F1GateTests(unittest.TestCase):
    def test_missing_content_and_products_fail_with_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            out = root / "evidence"
            result = bsp2_f1_gate.main([
                "--portal-runtime", str(root / "missing-portal"),
                "--portal2-runtime", str(root / "missing-portal2"),
                "--dedicated-build", str(root / "missing-dedicated"),
                "--client-build", str(root / "missing-client"),
                "--tool", str(root / "missing-tool"), "--out", str(out)])
            self.assertEqual(result, 1)
            evidence = json.loads((out / "evidence.json").read_text())
            self.assertEqual(evidence["status"], "fail")
            self.assertEqual(evidence["steps"], [])
            self.assertTrue(any("required Portal runtime" in item for item in evidence["failures"]))
            self.assertTrue(any("required Portal 2 runtime" in item for item in evidence["failures"]))

    def test_output_cannot_write_into_source_runtime(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            portal = root / "portal-runtime"
            portal.mkdir()
            out = portal / "evidence"
            result = bsp2_f1_gate.main([
                "--portal-runtime", str(portal), "--portal2-runtime", str(root / "portal2"),
                "--dedicated-build", str(root / "dedicated"),
                "--client-build", str(root / "client"),
                "--tool", str(root / "tool"), "--out", str(out)])
            self.assertEqual(result, 1)
            self.assertFalse(out.exists())

    def test_unexpected_step_error_writes_failure_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            portal = root / "portal-runtime"
            portal2 = root / "portal2-runtime"
            (portal / "portal").mkdir(parents=True)
            (portal2 / "portal2").mkdir(parents=True)
            (portal / "portal/gameinfo.txt").touch()
            (portal2 / "portal2/gameinfo.txt").touch()
            dedicated = root / "dedicated"
            client = root / "client"
            dedicated.mkdir()
            client.mkdir()
            tool = root / "bsp2tool"
            tool.write_text("#!/bin/sh\n")
            tool.chmod(0o755)
            out = root / "evidence"
            with mock.patch.object(bsp2_f1_gate.bsp2_reader, "main",
                                   side_effect=RuntimeError("seeded corpus crash")), \
                    mock.patch.object(bsp2_f1_gate.bsp2_dedicated, "main",
                                      side_effect=RuntimeError("seeded native crash")), \
                    mock.patch.object(bsp2_f1_gate.bsp2_sparse_export, "main",
                                      side_effect=RuntimeError("seeded sparse crash")), \
                    mock.patch.object(bsp2_f1_gate.bsp2_v19_derivative, "derive",
                                      side_effect=RuntimeError("seeded derivative crash")):
                result = bsp2_f1_gate.main([
                    "--portal-runtime", str(portal), "--portal2-runtime", str(portal2),
                    "--dedicated-build", str(dedicated), "--client-build", str(client),
                    "--tool", str(tool), "--out", str(out)])
            self.assertEqual(result, 1)
            evidence = json.loads((out / "evidence.json").read_text())
            self.assertEqual(evidence["status"], "fail")
            self.assertEqual(len(evidence["steps"]), 7)
            self.assertTrue(all(step["status"] == "fail" for step in evidence["steps"]))
            self.assertTrue(any("seeded corpus crash" in item for item in evidence["failures"]))


if __name__ == "__main__":
    unittest.main()
