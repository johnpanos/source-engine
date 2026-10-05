# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Sensitivity checks for the first game/lab image comparison."""

import json
from pathlib import Path
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import game_lab_compare as compare  # noqa: E402


class GameLabComparisonTests(unittest.TestCase):
    def test_neutral_frame_passes_and_dark_emitter_fails(self):
        lab = np.full((32, 32, 3), 80, dtype=np.uint8)
        game = lab.copy()
        game[:, :8] = 0
        neutral = compare.score(lab, lab)
        dark = compare.score(lab, game)
        self.assertTrue(all(neutral[key] <= limit for key, limit in compare.LIMITS.items()))
        self.assertGreater(dark["mean"], compare.LIMITS["mean"])
        self.assertGreater(dark["p99"], compare.LIMITS["p99"])
        self.assertGreater(dark["fraction_gt8"], compare.LIMITS["fraction_gt8"])

    def test_camera_oracle_refuses_wrong_view(self):
        fixture = {"cameras": {"overview": {"eye": [0.4, 3.0, 2.4],
                                             "forward": [0.9670745, 0.0, -0.2544933],
                                             "up": [0.0, 0.0, 1.0]}},
                   "horizontal_fov_degrees": 90.0}
        matching = {"origin": [15.7, 118.1, 94.5], "angles": [14.7, 0.0, 0.0],
                    "fov": 90.0}
        self.assertLess(compare.camera_errors(fixture, "overview", matching)["position_units"],
                        0.25)
        shifted = dict(matching, origin=[20.7, 118.1, 94.5])
        self.assertGreater(compare.camera_errors(fixture, "overview", shifted)["position_units"],
                           0.25)

    def test_view_oracle_requires_a_top_level_world_camera(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "view.jsonl"
            header = {"schema": "source-view-oracle/v1"}
            nested = {"event": "label_begin", "name": "vieworacle " + json.dumps(
                {"type": "3d", "stack": 2, "target": "backbuffer"})}
            path.write_text(json.dumps(header) + "\n" + json.dumps(nested) + "\n")
            with self.assertRaisesRegex(ValueError, "top-level world camera"):
                compare.camera_from_oracle(path)

    def test_nonfinite_lab_pixels_fail(self):
        with self.assertRaisesRegex(ValueError, "nonfinite"):
            compare.display_bytes(np.array([[[float("nan"), 0.0, 0.0]]]))

    def test_recorded_output_scale_matches_linear_surface_output(self):
        lab = np.full((2, 2, 3), 0.25, dtype=np.float32)
        scale = compare.output_scale({"tonemap_scale": 1.5})
        self.assertEqual(scale, 1.5)
        self.assertEqual(compare.score(compare.display_bytes(lab * scale),
                                       compare.display_bytes(np.full_like(lab, 0.375)))["mean"], 0)
        for invalid in (None, -1, float("nan"), "1.5", True):
            with self.assertRaisesRegex(ValueError, "tone-map scale"):
                compare.output_scale({"tonemap_scale": invalid})

    def test_content_comparison_requires_every_snapshot_asset(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "maps").mkdir()
            (root / "materials").mkdir()
            (root / "maps/example.bsp").write_bytes(b"map")
            (root / "materials/example.vmt").write_bytes(b"material")
            staged = {"maps/example.bsp": {"sha256": compare.sha256(
                root / "maps/example.bsp")}}
            self.assertEqual(compare.content_mismatches(root, staged),
                             ["materials/example.vmt"])
            staged["materials/example.vmt"] = {"sha256": "wrong"}
            self.assertEqual(compare.content_mismatches(root, staged),
                             ["materials/example.vmt"])
            staged["materials/example.vmt"]["sha256"] = compare.sha256(
                root / "materials/example.vmt")
            self.assertEqual(compare.content_mismatches(root, staged), [])

    def test_lab_frame_takes_the_games_output(self):
        # The game's frame is tone mapped by render.pass.output from its HDR
        # scene; a lab frame compared without the same output would score
        # every highlight as an error.
        seen = {}

        class Done:
            returncode = 0
            stdout = "render_lab: drawn"
            stderr = ""

        def run(command, **kwargs):
            seen["command"] = command
            Path(command[command.index("--out") + 1]).write_bytes(b"pfm")
            return Done()

        fixture = json.loads((compare.lf.ROOT /
                              "quality/fixtures/lighting/area-room/fixture.json").read_text())
        original = compare.subprocess.run
        compare.subprocess.run = run
        try:
            with tempfile.TemporaryDirectory() as directory:
                out = Path(directory) / "lab.pfm"
                compare.render_lab(Path(directory) / "render_lab", Path(directory), fixture,
                                   "overview", out)
        finally:
            compare.subprocess.run = original
        command = seen["command"]
        self.assertEqual(float(command[command.index("--output-peak") + 1]),
                         compare.GAME_SCENE_PEAK)


if __name__ == "__main__":
    unittest.main()
