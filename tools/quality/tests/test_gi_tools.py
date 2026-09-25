#!/usr/bin/env python3
"""Self-tests for the RFC 0011 GI tools: budgets, the EXR reader, region
masks, runtime capture decoding and the RTRN radiosity transfer reader. Run:

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
import radiosity_transfer  # noqa: E402
import gi_temporal  # noqa: E402
import indirect_defaults  # noqa: E402
import sdf_volume  # noqa: E402

FIXTURES = HERE.parents[2] / "quality" / "fixtures" / "gi"


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




class RadiosityTransferTest(unittest.TestCase):
    """RTRN (RFC 0011 G4): the independent reader and reference solver."""

    def setUp(self):
        self.prbv = (FIXTURES / "prbv" / "contract.prbv").read_bytes()
        self.rtrn = (FIXTURES / "rtrn" / "contract.rtrn").read_bytes()

    def test_checked_in_fixture_is_the_generator_output(self):
        self.assertEqual(radiosity_transfer.fixture_contract(self.prbv), self.rtrn)

    def test_contract_is_the_furnace(self):
        transfer = radiosity_transfer.Transfer(self.rtrn, self.prbv)
        light, direct = transfer.solve()
        self.assertAlmostEqual(float(light[:, 0].mean()), 0.75, places=5)
        _, indirect = transfer.probe_light(light)
        lit = [i for i, row in enumerate(transfer.gather) if len(row[0])]
        self.assertAlmostEqual(float(indirect[lit].mean()), 0.45, places=5)
        # One bounce only: 0.3 + 0.6 * 0.3.
        self.assertAlmostEqual(float(transfer.solve(bounces=1)[0][:, 0].mean()), 0.48, places=5)

    def test_every_malformation_is_rejected(self):
        for name, variant in radiosity_transfer.malformations(self.rtrn):
            with self.subTest(name=name):
                with self.assertRaises(radiosity_transfer.TransferError):
                    radiosity_transfer.Transfer(variant, self.prbv)

    def test_a_transfer_is_bound_to_its_volume(self):
        other = bytearray(self.prbv)
        volume = radiosity_transfer.probe_volume.Volume(bytes(other))
        grid = volume.grids[0]
        offset = struct.unpack_from("<Q", other, 40)[0]
        x, y = grid.state_origin
        other[offset + (y * volume.width + x) * 8] ^= 1
        with self.assertRaises(radiosity_transfer.TransferError) as caught:
            radiosity_transfer.Transfer(self.rtrn, bytes(other))
        self.assertEqual(caught.exception.code, "topology-mismatch")

    def test_mutation_fuzzing_rejects_or_reads_in_range(self):
        counts = radiosity_transfer.fuzz(self.rtrn, self.prbv, iterations=400)
        self.assertGreater(counts["rejected"], 50)
        self.assertGreater(counts["accepted"], 10)


class SdfVolumeTest(unittest.TestCase):
    """SDFV (RFC 0011 G6): the independent reader and its fixture corpus."""

    def setUp(self):
        self.sdfv = (FIXTURES / "sdfv" / "contract.sdfv").read_bytes()

    def test_checked_in_fixture_is_the_generator_output(self):
        self.assertEqual(sdf_volume.fixture_contract(), self.sdfv)
        with tempfile.TemporaryDirectory() as out:
            sdf_volume.write_fixtures(out)
            for name in ("fixtures.json", "malformations.txt"):
                with self.subTest(file=name):
                    self.assertEqual((Path(out) / name).read_bytes(),
                                     (FIXTURES / "sdfv" / name).read_bytes())

    def test_every_malformation_is_rejected_with_its_code(self):
        codes = {line.split()[0]: line.split()[1] for line in
                 (FIXTURES / "sdfv" / "malformations.txt").read_text().splitlines()}
        for name, variant in sdf_volume.malformations(self.sdfv):
            with self.subTest(name=name):
                with self.assertRaises(sdf_volume.SdfError) as caught:
                    sdf_volume.Volume(variant)
                self.assertEqual(caught.exception.code, codes[name])

    def test_contract_scene(self):
        volume = sdf_volume.Volume(self.sdfv)
        self.assertEqual(volume.dims, [21, 13, 13])
        self.assertEqual([light["kind"] for light in volume.lights], ["dome"])
        self.assertGreater(volume.info()["emissive_voxels"], 0)


class TemporalStabilityTest(unittest.TestCase):
    """gi_temporal's score: a still burst passes at 0; flicker fails."""

    def burst(self, noise=0.0, seed=7):
        rng = np.random.default_rng(seed)
        base = np.full((12, 48, 64), 0.18)
        return base * (1.0 + noise * rng.standard_normal(base.shape))

    def test_a_still_burst_scores_zero(self):
        result = gi_temporal.score(self.burst())
        self.assertAlmostEqual(result["flicker_rms"], 0.0, places=12)
        self.assertEqual(result["status"], "pass")

    def test_seeded_flicker_fails(self):
        # One percent per-pixel noise, frame to frame: the SDF defect scored 2.6%.
        result = gi_temporal.score(self.burst(noise=0.01))
        self.assertEqual(result["status"], "fail")
        self.assertGreater(result["flicker_rms"], gi_temporal.FLICKER_RMS_LIMIT)

    def test_one_frame_pop_fails_the_step(self):
        frames = self.burst()
        frames[6] *= 1.01
        result = gi_temporal.score(frames)
        self.assertEqual(result["status"], "fail")
        self.assertGreater(result["step_max"], gi_temporal.STEP_LIMIT)

    def test_a_quantization_step_in_a_few_pixels_passes(self):
        frames = self.burst()
        frames[5, :4, :4] += 1.0 / 255.0
        self.assertEqual(gi_temporal.score(frames)["status"], "pass")


class IndirectDefaultsTest(unittest.TestCase):
    """The engine's default producers are the product profiles' (G8)."""

    def test_header_matches_the_profiles(self):
        self.assertEqual(indirect_defaults.HEADER.read_text(), indirect_defaults.render())

    def test_every_list_ends_with_baked(self):
        for _, profile in indirect_defaults.PLATFORMS:
            self.assertEqual(indirect_defaults.producers(profile)[-1], "baked")


if __name__ == "__main__":
    unittest.main()
