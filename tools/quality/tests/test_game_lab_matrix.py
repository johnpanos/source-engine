#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Negative checks for the game/lab matrix runner's required cameras."""

from pathlib import Path
import json
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import game_lab_matrix as matrix  # noqa: E402


class MatrixFailuresTests(unittest.TestCase):
    def test_camera_setup_does_not_enable_core_after_level_init(self):
        fixture = matrix.lf.load_fixture("area-room")
        self.assertNotIn("r_core_world 1", matrix.camera_commands(fixture, "overview"))

    def test_scripted_portal_camera_is_released_for_fixture_pose(self):
        portal = matrix.lf.load_fixture("portal-chamber")
        command = matrix.camera_commands(portal, "room2")
        self.assertLess(command.index("point_viewcontrol Disable"), command.index("setpos"))
        other = matrix.lf.load_fixture("area-room")
        self.assertNotIn("point_viewcontrol", matrix.camera_commands(other, "overview"))

    def test_empty_and_incomplete_runs_fail(self):
        self.assertIn("no fixtures ran", matrix.matrix_failures({"fixtures": {}}))
        result = {"fixtures": {"room": {"cameras": {
            "left": {"boot_returncode": 0, "comparison": {"status": "diagnostic"}},
            "right": {"boot_returncode": 1}}}}}
        self.assertEqual(matrix.matrix_failures(result),
                         ["room/right: product boot failed"])
        result["fixtures"]["room"]["cameras"]["right"] = {"boot_returncode": 0}
        self.assertEqual(matrix.matrix_failures(result),
                         ["room/right: comparison did not complete"])

    def test_declared_pixel_failure_fails_while_diagnostic_completes(self):
        record = {"fixtures": {"room": {"cameras": {
            "left": {"boot_returncode": 0, "comparison": {"status": "diagnostic"}},
            "right": {"boot_returncode": 0, "comparison": {"status": "fail"}}}}}}
        self.assertEqual(matrix.matrix_failures(record),
                         ["room/right: comparison failed"])

    def test_zero_legacy_gate_requires_runtime_census(self):
        result = {"boot_returncode": 0, "comparison": {"status": "pass"},
                  "legacy_census": {"zero_legacy_stream": False}}
        record = {"fixtures": {"room": {"cameras": {"left": result}}}}
        self.assertEqual(matrix.matrix_failures(record, True),
                         ["room/left: legacy stream still drew"])
        result["legacy_census"]["zero_legacy_stream"] = True
        self.assertEqual(matrix.matrix_failures(record, True), [])

    def test_census_counts_settled_draws_and_rejects_missing_counters(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "frames.jsonl"
            header = {"schema": "vulkan-frame-stats/v1"}
            frames = [{"f": i, "legacy_stream_draws": 0,
                       "legacy_program_draws": 0} for i in range(32)]
            frames[0]["legacy_stream_draws"] = 99  # startup does not count
            path.write_text("\n".join(json.dumps(item) for item in [header] + frames))
            self.assertTrue(matrix.legacy_census(path)["zero_legacy_stream"])
            frames[-1]["legacy_stream_draws"] = 3
            frames[-1]["legacy_program_draws"] = 2
            path.write_text("\n".join(json.dumps(item) for item in [header] + frames))
            self.assertEqual(matrix.legacy_census(path)["stream_draws_max"], 3)
            self.assertFalse(matrix.legacy_census(path)["zero_legacy_stream"])
            del frames[-1]["legacy_program_draws"]
            path.write_text("\n".join(json.dumps(item) for item in [header] + frames))
            with self.assertRaisesRegex(ValueError, "invalid draw counts"):
                matrix.legacy_census(path)
            frames[-1]["legacy_program_draws"] = 2
            frames[-1]["f"] += 1
            path.write_text("\n".join(json.dumps(item) for item in [header] + frames))
            with self.assertRaisesRegex(ValueError, "missing or unordered frames"):
                matrix.legacy_census(path)


if __name__ == "__main__":
    unittest.main()
