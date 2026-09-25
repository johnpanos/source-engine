#!/usr/bin/env python3
"""Self-tests for the RFC 0011 GI tools: budgets, the EXR reader, region
masks and runtime capture decoding. Run:

    python3 -m unittest tools/quality/tests/test_gi_tools.py
"""

import copy
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
import gi_budgets  # noqa: E402
import gi_reference  # noqa: E402
import gi_runtime  # noqa: E402
import lightmap_layers  # noqa: E402


def exr_bytes(width, height, channels, compression=0, multipart=False):
    """A minimal scanline OpenEXR file with FLOAT channels {name: (H, W)}."""
    def attribute(name, kind, value):
        return name.encode() + b"\0" + kind.encode() + b"\0" + struct.pack("<i", len(value)) + value
    names = sorted(channels)
    chlist = b"".join(n.encode() + b"\0" + struct.pack("<iBxxxii", 2, 0, 1, 1) for n in names)
    chlist += b"\0"
    header = (attribute("channels", "chlist", chlist) +
              attribute("compression", "compression", bytes([compression])) +
              attribute("dataWindow", "box2i", struct.pack("<iiii", 0, 0, width - 1, height - 1)) +
              attribute("displayWindow", "box2i", struct.pack("<iiii", 0, 0, width - 1, height - 1)) +
              attribute("lineOrder", "lineOrder", b"\0"))
    if multipart:
        header += attribute("name", "string", b"Layer.Combined") + \
            attribute("type", "string", b"scanlineimage") + \
            attribute("chunkCount", "int", struct.pack("<i", height))
    header += b"\0"
    if multipart:
        header += b"\0"
    version = 2 | (0x1000 if multipart else 0)
    prefix = b"\x76\x2f\x31\x01" + struct.pack("<I", version) + header
    table_offset = len(prefix)
    rows = []
    offset = table_offset + 8 * height
    for y in range(height):
        data = b"".join(np.asarray(channels[n][y], "<f4").tobytes() for n in names)
        row = (struct.pack("<i", 0) if multipart else b"") + struct.pack("<ii", y, len(data)) + data
        rows.append((offset, row))
        offset += len(row)
    table = b"".join(struct.pack("<Q", o) for o, _ in rows)
    return prefix + table + b"".join(r for _, r in rows)


class BudgetTest(unittest.TestCase):
    def test_declared_budgets_are_complete(self):
        self.assertEqual(gi_budgets.check(gi_budgets.load()), [])

    def test_missing_owner_and_bad_limits_are_rejected(self):
        budgets = copy.deepcopy(gi_budgets.load())
        table = budgets["profiles"]["portal-linux-wayland-native-vulkan"]["budgets"]
        del table["radiosity"]["owner"]
        table["sdf"]["gpu_ms"] = -1
        del table["rayquery"]
        problems = gi_budgets.check(budgets)
        self.assertTrue(any("radiosity: budget has no owner" in p for p in problems))
        self.assertTrue(any("sdf: gpu_ms must be positive" in p for p in problems))
        self.assertTrue(any("no rayquery budget" in p for p in problems))

    def test_report_fails_over_budget_and_unsupported(self):
        budgets = gi_budgets.load()
        ok = gi_budgets.report(budgets, "portal-linux-wayland-native-vulkan", "radiosity",
                               {"cpu_ms_per_update": 0.5})
        self.assertEqual(ok["status"], "pass")
        over = gi_budgets.report(budgets, "portal-linux-wayland-native-vulkan", "radiosity",
                                 {"cpu_ms_per_update": 1.5})
        self.assertEqual(over["status"], "fail")
        inherited = gi_budgets.report(budgets, "ios-arm64-device", "sdf", {"gpu_ms": 0.1})
        self.assertEqual(inherited["status"], "fail")
        self.assertEqual(gi_budgets.report(budgets, "portal-linux-wayland-native-vulkan",
                                           "baked", {})["status"], "fail")


class ExrReaderTest(unittest.TestCase):
    def test_single_and_multipart_uncompressed(self):
        values = {"R": np.arange(6, dtype=np.float32).reshape(2, 3),
                  "G": np.full((2, 3), 0.5, np.float32)}
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "a.exr"
            path.write_bytes(exr_bytes(3, 2, values))
            read = gi_reference.read_exr(path)
            np.testing.assert_array_equal(read["R"], values["R"])
            path.write_bytes(exr_bytes(3, 2, values, multipart=True))
            read = gi_reference.read_exr(path)
            np.testing.assert_array_equal(read["Layer.Combined.G"], values["G"])
            passes = gi_reference.render_passes(path)
            self.assertIn("Combined", passes)

    def test_compressed_and_foreign_files_are_rejected(self):
        values = {"R": np.zeros((2, 2), np.float32)}
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "z.exr"
            path.write_bytes(exr_bytes(2, 2, values, compression=3))
            with self.assertRaises(ValueError):
                gi_reference.read_exr(path)
            path.write_bytes(b"not an exr file at all")
            with self.assertRaises(ValueError):
                gi_reference.read_exr(path)


class RegionTest(unittest.TestCase):
    def test_masks_expand_models_and_erode_edges(self):
        index = np.zeros((20, 20), np.int32)
        index[2:18, 2:18] = 1
        index[8:12, 8:12] = 2
        scene = {"props": [{"name": "Probe", "shapes": ["ProbeShape"]}]}
        masks = gi_reference.region_masks(index, {"Wall": 1, "ProbeShape": 2},
                                          {"walls": ["Wall"], "model": ["Probe"]}, scene)
        self.assertEqual(int(masks["model"].sum()), 0)  # 4x4 eroded by 2 on each side
        self.assertTrue(masks["walls"][5, 5])
        self.assertFalse(masks["walls"][2, 2])
        with self.assertRaises(ValueError):
            gi_reference.region_masks(index, {"Wall": 1}, {"x": ["Missing"]})


class CaptureDecodeTest(unittest.TestCase):
    def test_srgb_decode_and_downsample(self):
        film = {"width": 4, "height": 3}
        encoded = np.full((12, 16, 3), 188, np.uint8)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "shot.tga"
            Image.fromarray(encoded).save(path)
            linear = gi_runtime.linear_capture(path, film, 1.0)
            self.assertEqual(linear.shape, (3, 4, 3))
            self.assertAlmostEqual(float(linear.mean()), 0.5029, places=3)
            halved = gi_runtime.linear_capture(path, film, 2.0)
            self.assertAlmostEqual(float(halved.mean()), 0.5029 / 2, places=3)



class LightmapLayersTest(unittest.TestCase):
    """The separated-bake oracle accepts consistent noisy layers and rejects
    the seeded defects."""

    def layers(self, seed=1):
        rng = np.random.default_rng(seed)
        shape = (64, 64, 3)
        direct = np.full(shape, 0.3) + rng.normal(0, 0.01, shape)
        indirect = np.full(shape, 0.45) + rng.normal(0, 0.01, shape)
        # An independent bake of the same texels: its own noise.
        total = np.full(shape, 0.75) + rng.normal(0, 0.014, shape)
        covered = np.ones(shape[:2], dtype=bool)
        return total, direct, indirect, covered

    def test_consistent_noisy_layers_pass(self):
        analytic = {"diffuse_light_direct": 0.3, "diffuse_light_indirect": 0.45}
        failures, summary = lightmap_layers.judge(*self.layers(), analytic)
        self.assertEqual(failures, [])
        self.assertEqual(summary["blocks"], 16)

    def test_doubled_indirect_fails_the_sum(self):
        total, direct, indirect, covered = self.layers()
        failures, _ = lightmap_layers.judge(total, direct, indirect * 2, covered)
        self.assertTrue(any("noise band" in f for f in failures))

    def test_swapped_layers_fail_the_analytic_check(self):
        total, direct, indirect, covered = self.layers()
        analytic = {"diffuse_light_direct": 0.3, "diffuse_light_indirect": 0.45}
        failures, _ = lightmap_layers.judge(total, indirect, direct, covered, analytic)
        self.assertEqual(len([f for f in failures if "analytic" in f]), 2)

    def test_a_small_bias_is_detected(self):
        total, direct, indirect, covered = self.layers()
        failures, _ = lightmap_layers.judge(total + 0.01, direct, indirect, covered)
        self.assertTrue(failures)


if __name__ == "__main__":
    unittest.main()
