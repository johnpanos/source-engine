#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================

import argparse
import json
from pathlib import Path
import sys
import tempfile
import unittest

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import map_relight_diagnostics as diagnostics  # noqa: E402


class RelightDiagnosticsTest(unittest.TestCase):
    def test_roi_rejects_invalid_bounds(self):
        for value in ("wall:0,0,1", "wall:0.5,0,0.2,1", "wall:0,0,2,1",
                      "wall:nan,0,1,1"):
            with self.subTest(value=value):
                with self.assertRaises(argparse.ArgumentTypeError):
                    diagnostics.parse_roi(value)

    def test_roi_decodes_srgb_and_reports_clipping(self):
        image = Image.new("RGB", (2, 1))
        image.putdata([(128, 128, 128), (255, 0, 0)])
        sample = diagnostics.roi_metrics(image, (0, 0, 1, 1))
        self.assertAlmostEqual(sample["linearized_mean_rgb"][0], 0.60793, places=4)
        self.assertAlmostEqual(sample["clipped_pixel_fraction"], 0.5)
        self.assertGreater(sample["red_blue_ratio"], 1)

    def test_roi_excludes_not_applicable_hatch(self):
        image = Image.new("RGB", (2, 1))
        image.putdata([(188, 188, 188), (128, 64, 32)])
        sample = diagnostics.roi_metrics(image, (0, 0, 1, 1), exclude_hatch=True)
        self.assertEqual(sample["hatch_fraction"], 0.5)
        self.assertGreater(sample["red_blue_ratio"], 1)

    def test_probe_selection_decodes_global_rank_without_confusing_magenta(self):
        image = Image.new("RGB", (4, 1))
        image.putdata([(188, 0, 255), (188, 0, 255), (255, 0, 255),
                       (188, 188, 188)])
        sample = diagnostics.probe_selection_metrics(image, (0, 0, 1, 1))
        self.assertEqual(sample["rank_fraction"], {"15": 1.0})
        self.assertEqual(sample["sampled_pixels"], 2)
        self.assertAlmostEqual(sample["mean_first_weight"], 1.0)
        self.assertAlmostEqual(sample["fallback_fraction"], 1 / 3)

    def test_global_weight_includes_second_probe(self):
        # Local rank 0 at half weight, global rank 15 at the other half.
        image = Image.new("RGB", (1, 1), (49, 188, 188))
        sample = diagnostics.probe_selection_metrics(image, (0, 0, 1, 1), 15)
        self.assertAlmostEqual(sample["mean_global_weight"], 0.5, delta=0.01)
        self.assertEqual(sample["invalid_pixels"], 0)

    def test_runtime_checks_reject_bad_selection_and_missing_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            receipt = {"regions": {"stairs": (0, 0, 1, 1)},
                       "probe_layout": {"global_rank": 15},
                       "modes": {"probe-selection": {"image": "selection.png"}}}
            for color, expected in (((188, 0, 255), "fail"),  # global only
                                    ((49, 188, 188), "fail"),  # global second
                                    ((255, 0, 255), "fail"),  # invalid header
                                    ((188, 188, 188), "fail"),  # all hatch
                                    ((0, 0, 0), "fail"),  # no valid rank
                                    ((49, 0, 255), "pass")):  # local only
                with self.subTest(color=color):
                    Image.new("RGB", (2, 2), color).save(root / "selection.png")
                    result = diagnostics.check_probe_regions(receipt, root, {"stairs": 0.1})
                    self.assertEqual(result["status"], expected)
            receipt["probe_layout"] = None
            self.assertEqual(diagnostics.check_probe_regions(
                receipt, root, {"stairs": 0.1})["status"], "fail")
            self.assertEqual(diagnostics.check_probe_regions(
                receipt, root, {"missing": 0.1})["status"], "fail")
            self.assertEqual(diagnostics.check_probe_regions(
                {}, root, {"stairs": 0.1})["status"], "fail")

    def test_source_inventory_keeps_identity_without_claiming_contribution(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            content = root / "content"
            content.mkdir()
            receipt = root / "legacy-scene/scene-receipt.json"
            receipt.parent.mkdir()
            receipt.write_text(json.dumps({"lights": [
                {"world_light": 19, "hammerid": None, "type": "sky_ambient",
                 "radiance": [0.6, 0.4, 0.3]},
                {"world_light": 20, "type": "sky", "irradiance": [0.1, 0.1, 0.1]}]}))
            inventory = diagnostics.source_lights(content)
            self.assertEqual(inventory["lights"][0]["world_light"], 19)
            self.assertEqual(inventory["lights"][0]["red_blue_ratio"], 2)
            self.assertEqual(inventory["lights"][1]["type"], "sky")

    def test_probe_layout_labels_the_global_rank(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            content = root / "content"
            content.mkdir()
            receipt = root / "lighting/reflection_probes.rprb.json"
            receipt.parent.mkdir()
            receipt.write_text(json.dumps({"probes": 4, "global_index": 1,
                                           "fits": [{"capture": [0, 0, 0]},
                                                    {"capture": [1, 2, 3]}]}))
            layout = diagnostics.probe_layout(content)
            self.assertEqual(layout["global_rank"], 3)
            self.assertEqual(layout["global_capture"], [1, 2, 3])

    def test_capture_requires_one_recorded_exposure(self):
        with tempfile.TemporaryDirectory() as directory:
            boot = Path(directory)
            log = boot / "runtime/engine.log"
            log.parent.mkdir()
            log.write_text('[29.0] "mat_hdr_tonemapscale" = "4.000000" ( def. "1.0" )')
            self.assertEqual(diagnostics.captured_tonemap_scale(boot / "evidence.json"), 4)
            log.write_text("missing exposure")
            with self.assertRaises(ValueError):
                diagnostics.captured_tonemap_scale(boot / "evidence.json")


if __name__ == "__main__":
    unittest.main()
