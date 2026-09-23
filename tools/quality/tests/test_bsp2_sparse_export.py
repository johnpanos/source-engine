#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Required-input and output-isolation checks for the sparse export lane."""

import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import bsp2_sparse_export  # noqa: E402


class SparseExportTests(unittest.TestCase):
    def test_changed_source_fails_before_tool_launch(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            maps = root / "maps"
            maps.mkdir()
            source = maps / "testchmb_a_01.bsp"
            source.write_bytes(b"VBSP" + struct.pack("<I", 20) + b"changed")
            tool = root / "tool"
            tool.write_bytes(b"not an executable")
            inventory = root / "inventory.json"
            inventory.write_text(json.dumps({"maps": [{"name": source.name,
                                                         "version": 20, "sha256": "0" * 64}]}))
            out = root / "evidence"
            result = bsp2_sparse_export.main([
                "--legacy-map", str(source), "--tool", str(tool),
                "--inventory", str(inventory), "--out", str(out)])
            self.assertEqual(result, 1)
            evidence = json.loads((out / "evidence.json").read_text())
            self.assertEqual(evidence["checks"], {})
            self.assertIn("hash or version differs", evidence["failures"][0])

    def test_missing_input_writes_failure_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            maps = root / "maps"
            maps.mkdir()
            inventory = root / "inventory.json"
            inventory.write_text(json.dumps({"maps": [{"name": "testchmb_a_01.bsp",
                                                         "version": 20, "sha256": "0" * 64}]}))
            out = root / "evidence"
            result = bsp2_sparse_export.main([
                "--legacy-map", str(maps / "testchmb_a_01.bsp"),
                "--tool", str(root / "missing-tool"),
                "--inventory", str(inventory), "--out", str(out)])
            self.assertEqual(result, 1)
            evidence = json.loads((out / "evidence.json").read_text())
            self.assertEqual(evidence["status"], "fail")
            self.assertTrue(any("missing" in item for item in evidence["failures"]))

    def test_output_cannot_enter_source_map_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            maps = root / "maps"
            maps.mkdir()
            out = maps / "evidence"
            result = bsp2_sparse_export.main([
                "--legacy-map", str(maps / "testchmb_a_01.bsp"),
                "--tool", str(root / "tool"),
                "--inventory", str(root / "inventory.json"), "--out", str(out)])
            self.assertEqual(result, 2)
            self.assertFalse(out.exists())

    def test_nonempty_output_is_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            maps = root / "maps"
            maps.mkdir()
            out = root / "evidence"
            out.mkdir()
            marker = out / "previous.txt"
            marker.write_text("keep")
            result = bsp2_sparse_export.main([
                "--legacy-map", str(maps / "testchmb_a_01.bsp"),
                "--tool", str(root / "tool"),
                "--inventory", str(root / "inventory.json"), "--out", str(out)])
            self.assertEqual(result, 2)
            self.assertEqual(marker.read_text(), "keep")


if __name__ == "__main__":
    unittest.main()
