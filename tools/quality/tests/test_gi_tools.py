#!/usr/bin/env python3
"""Self-tests for the RFC 0011 GI tools: budgets, the EXR reader, region
masks, the relational oracles, runtime capture decoding and the RTRN
radiosity transfer reader. Run:

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
import gi_oracles  # noqa: E402
import gi_reference  # noqa: E402
import gi_runtime  # noqa: E402
import lightmap_layers  # noqa: E402
import radiosity_transfer  # noqa: E402
import gi_soak  # noqa: E402
import gi_swing  # noqa: E402
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


class SwingScoreTest(unittest.TestCase):
    """RFC 0011 G9.4: a region's mean light per frame is smooth under the
    swing; flicker raises its second temporal difference."""

    def test_smooth_bounce_adds_little_and_flicker_adds_more_than_the_limit(self):
        t = np.arange(40, dtype=np.float64)
        # The control: a region lit by a light swinging past it (direct only).
        control = 0.3 + 0.1 * np.sin(t / 9.0)
        # A producer's run: the same plus a smooth bounce that follows it.
        smooth = control + 0.03 * np.sin(t / 9.0 + 0.4) + 0.05
        added = gi_swing.high_frequency(smooth) - gi_swing.high_frequency(control)
        self.assertLessEqual(added, gi_swing.HF_LIMIT)
        jitter = np.random.default_rng(3).normal(0.0, 0.01 * 0.35, t.shape)
        flicker = gi_swing.high_frequency(smooth + jitter) - gi_swing.high_frequency(control)
        self.assertGreater(flicker, gi_swing.HF_LIMIT)

    def test_region_curves_average_each_mask(self):
        frames = np.zeros((3, 8, 8))
        frames[:, :4] = 1.0
        masks = {region: np.zeros((2, 2), dtype=bool) for region in gi_swing.SWING_REGIONS}
        masks["floor"][0, :] = True
        masks["red_wall"][1, :] = True
        masks["back_wall"][:, 0] = True
        masks["east_wall"][:, 1] = True
        curves = gi_swing.region_curves(frames, masks)
        self.assertEqual(curves["floor"].tolist(), [1.0, 1.0, 1.0])
        self.assertEqual(curves["red_wall"].tolist(), [0.0, 0.0, 0.0])
        self.assertEqual(curves["back_wall"].tolist(), [0.5, 0.5, 0.5])


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


def fixture_with(oracle, states=("all", "red", "blue"), cameras=("c",),
                 regions=("left", "right", "floor")):
    return {"states": {s: {} for s in states},
            "cameras": {c: {} for c in cameras},
            "regions": {c: {r: [r] for r in regions} for c in cameras},
            "oracles": [oracle]}


def view(**regions):
    """{region: {light: rgb}} with the same rgb for every light."""
    return {r: {"total": list(v), "indirect": list(v), "direct": list(v)}
            for r, v in regions.items()}


class OracleTest(unittest.TestCase):
    """gi_oracles: every kind passes consistent statistics, fails a real
    defect, and its seeded control is rejected."""

    def evaluate(self, oracle, views, **fixture):
        [result] = gi_oracles.evaluate(fixture_with(oracle, **fixture), views)
        return result

    def mirror_views(self):
        return {("red", "c"): view(left=(0.4, 0, 0), right=(0.1, 0, 0), floor=(0.2, 0, 0)),
                ("blue", "c"): view(left=(0, 0, 0.1), right=(0, 0, 0.4), floor=(0, 0, 0.2)),
                ("all", "c"): view(left=(0.4, 0, 0.1), right=(0.1, 0, 0.4),
                                   floor=(0.2, 0, 0.2))}

    def test_superposition_passes_sums_and_rejects_a_dropped_light(self):
        oracle = {"kind": "superposition", "camera": "c", "sum": "all",
                  "parts": ["red", "blue"], "regions": ["left", "right", "floor"],
                  "lights": ["total", "indirect"]}
        result = self.evaluate(oracle, self.mirror_views())
        self.assertTrue(result["ok"])
        self.assertEqual(len(result["checks"]), 18)
        self.assertTrue(result["control"]["rejected"])
        views = self.mirror_views()
        views[("all", "c")]["floor"]["total"] = [0.2, 0.0, 0.23]
        self.assertFalse(self.evaluate(oracle, views)["ok"])

    def test_superposition_rejects_a_nonlinear_producer(self):
        oracle = {"kind": "superposition", "camera": "c", "sum": "all",
                  "parts": ["red", "blue"], "regions": ["floor"], "lights": ["total"]}
        views = {("red", "c"): view(floor=(0.2, 0.1, 0.1)),
                 ("blue", "c"): view(floor=(0.1, 0.1, 0.2))}
        views[("all", "c")] = view(floor=tuple((a + b) ** 1.1 for a, b in
                                              zip((0.2, 0.1, 0.1), (0.1, 0.1, 0.2))))
        self.assertFalse(self.evaluate(oracle, views)["ok"])

    def test_mirror_with_channel_swap(self):
        oracle = {"kind": "equal", "lights": ["total"],
                  "a": {"state": "all", "camera": "c", "region": "left", "channels": [0, 1, 2]},
                  "b": {"state": "all", "camera": "c", "region": "right", "channels": [2, 1, 0]}}
        result = self.evaluate(oracle, self.mirror_views())
        self.assertTrue(result["ok"])
        self.assertTrue(result["channel_mapping_observable"])
        self.assertTrue(result["control"]["rejected"])

    def test_self_mirror_control_biases_one_channel(self):
        oracle = {"kind": "equal", "lights": ["total"],
                  "a": {"state": "all", "camera": "c", "region": "floor", "channels": [0]},
                  "b": {"state": "all", "camera": "c", "region": "floor", "channels": [2]}}
        result = self.evaluate(oracle, self.mirror_views())
        self.assertTrue(result["ok"])
        self.assertTrue(result["control"]["rejected"])

    def test_zero_is_exact(self):
        oracle = {"kind": "zero", "state": "red", "camera": "c", "regions": ["left", "floor"],
                  "channels": [1, 2], "lights": ["total"]}
        result = self.evaluate(oracle, self.mirror_views())
        self.assertTrue(result["ok"])
        self.assertTrue(result["control"]["rejected"])
        views = self.mirror_views()
        views[("red", "c")]["floor"]["total"] = [0.2, 1e-4, 0.0]
        self.assertFalse(self.evaluate(oracle, views)["ok"])

    def test_value_and_its_bias_control(self):
        expected = {"total": [1.0, 0.6, 0.5], "indirect": [0.8, 0.3, 0.1]}
        oracle = {"kind": "value", "state": "all", "camera": "c", "regions": ["floor"],
                  "lights": ["indirect", "total"], "expected": expected}
        views = {("all", "c"): {"floor": {"total": [1.004, 0.598, 0.5],
                                          "indirect": [0.801, 0.3, 0.1]}}}
        result = self.evaluate(oracle, views)
        self.assertTrue(result["ok"])
        self.assertTrue(result["control"]["rejected"])
        views[("all", "c")]["floor"]["indirect"] = [0.8 * 0.97, 0.3, 0.1]
        self.assertFalse(self.evaluate(oracle, views)["ok"])

    def test_dominant_rejects_a_luminance_only_producer(self):
        oracle = {"kind": "dominant", "state": "all", "camera": "c", "region": "left",
                  "channel": 0, "over": [2], "factor": 2.0, "lights": ["indirect"]}
        result = self.evaluate(oracle, self.mirror_views())
        self.assertTrue(result["ok"])
        self.assertEqual(result["control"]["description"], "luminance-only producer")
        self.assertTrue(result["control"]["rejected"])

    def test_tint_order_and_ordered(self):
        entries = [{"state": "all", "camera": "c", "region": r}
                   for r in ("left", "floor", "right")]
        tint = {"kind": "tint_order", "entries": entries, "channel": 0, "over": 2,
                "factor": 1.5, "lights": ["total"]}
        result = self.evaluate(tint, self.mirror_views())
        self.assertTrue(result["ok"])
        self.assertTrue(result["control"]["rejected"])
        ordered = {"kind": "ordered", "entries": [entries[2], entries[1]], "factor": 1.0,
                   "lights": ["total"]}
        views = {("all", "c"): view(left=(0, 0, 0), right=(0.1, 0.1, 0.1),
                                    floor=(0.3, 0.3, 0.3))}
        result = self.evaluate(ordered, views)
        self.assertTrue(result["ok"])
        self.assertTrue(result["control"]["rejected"])

    def test_scale_chromaticity_and_uniform(self):
        views = {("all", "c"): view(left=(0.1, 0.05, 0.01), right=(0.1, 0.05, 0.01),
                                    floor=(0.1, 0.05, 0.01)),
                 ("red", "c"): view(left=(10.0, 5.0, 1.0), right=(10.0, 5.0, 1.0),
                                    floor=(10.0, 5.0, 1.0))}
        scale = {"kind": "scale", "camera": "c", "base": "all", "scaled": "red",
                 "factor": 100.0, "regions": ["left", "floor"], "lights": ["total"]}
        chroma = {"kind": "chromaticity", "state": "red", "camera": "c",
                  "regions": ["floor"], "expected": [1.0, 0.5, 0.1], "lights": ["total"]}
        uniform = {"kind": "uniform", "lights": ["total"],
                   "entries": [{"state": "all", "camera": "c", "region": r}
                               for r in ("left", "right", "floor")]}
        for oracle in (scale, chroma, uniform):
            result = self.evaluate(oracle, views)
            self.assertTrue(result["ok"], oracle["kind"])
            self.assertTrue(result["control"]["rejected"], oracle["kind"])
        views[("red", "c")]["floor"]["total"] = [10.0, 5.0, 1.3]
        self.assertFalse(self.evaluate(chroma, views)["ok"])

    def test_a_region_without_pixels_fails_instead_of_raising(self):
        oracle = {"kind": "zero", "state": "red", "camera": "c", "regions": ["floor"],
                  "channels": [1], "lights": ["direct"]}
        views = {("red", "c"): {"floor": {"pixels": 0}}}
        result = self.evaluate(oracle, views)
        self.assertFalse(result["ok"])
        self.assertIn("no direct light", result["checks"][0]["error"])

    def test_capture_oracles_keep_single_state_indirect_checks(self):
        fixture = {"oracles": [
            {"kind": "superposition", "camera": "c", "sum": "all", "parts": ["red"],
             "regions": ["floor"], "lights": ["indirect"]},
            {"kind": "zero", "state": "all", "camera": "c", "regions": ["floor"],
             "channels": [1], "lights": ["total", "indirect", "direct"]},
            {"kind": "zero", "state": "red", "camera": "c", "regions": ["floor"],
             "channels": [1], "lights": ["indirect"]},
            {"kind": "chromaticity", "state": "all", "camera": "c", "regions": ["floor"],
             "expected": [1.0, 0.0, 0.5], "lights": ["total"]},
            {"kind": "equal", "lights": ["total", "indirect"],
             "a": {"state": "all", "camera": "c", "region": "left"},
             "b": {"state": "all", "camera": "c", "region": "right"}}]}
        selected = gi_oracles.for_capture(fixture, "all", 0.1, 0.004)
        self.assertEqual([o["kind"] for o in selected], ["zero", "equal"])
        self.assertEqual(selected[0]["lights"], ["indirect"])
        self.assertEqual(selected[0]["absolute"], 0.004)
        self.assertEqual(selected[1]["tolerance"], 0.1)
        views = {("all", "c"): {"floor": {"indirect": [0.3, 0.003, 0.1]},
                                "left": {"indirect": [0.2, 0.1, 0.1]},
                                "right": {"indirect": [0.21, 0.1, 0.1]}}}
        results = gi_oracles.evaluate({"oracles": selected}, views)
        self.assertTrue(all(r["ok"] and r["control"]["rejected"] for r in results))

    def test_validation_names_unknown_references(self):
        oracle = {"kind": "zero", "state": "green", "camera": "c", "regions": ["ceiling"],
                  "channels": [0], "lights": ["glow"]}
        problems = gi_oracles.validate(fixture_with(oracle))
        self.assertTrue(any("unknown state green" in p for p in problems))
        self.assertTrue(any("no region ceiling" in p for p in problems))
        self.assertTrue(any("unknown light glow" in p for p in problems))
        self.assertTrue(gi_oracles.validate(fixture_with({"kind": "vibes"})))

    def test_checked_in_gallery_oracles_are_valid(self):
        gallery = 0
        for name in json.loads((FIXTURES / "index.json").read_text())["fixtures"]:
            fixture = json.loads((FIXTURES / name / "fixture.json").read_text())
            if "family" not in fixture:
                continue
            gallery += 1
            self.assertTrue(fixture["oracles"], name)
            self.assertEqual(gi_oracles.validate(fixture), [], name)
        self.assertGreaterEqual(gallery, 40)


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


class SoakAnalysisTest(unittest.TestCase):
    """gi_soak's gate on a synthetic engine log and memory series."""

    LOG = "\n".join(
        ["[1.0] indirect light: map 1, producer baked (offered: baked radiosity)"] +
        ["[%d.5] indirect light: switching to %s" % (t, p)
         for t, p in ((2, "radiosity"), (3, "baked"), (12, "radiosity"), (22, "baked"))] +
        ['[4.0] r_indirect_producer "sdf" unavailable (not-offered); keeping baked'] +
        ["[%d.0] gi_soak [%d.1] loop" % (t, t) for t in (10, 20, 30, 40)])

    def samples(self, leak=0.0):
        return [(t + 0.5, 500.0 + leak * t + (300.0 if t % 10 == 5 else 0.0))
                for t in range(0, 40)]

    def test_a_clean_run_passes(self):
        result = gi_soak.analyse(self.LOG, "", self.samples(), 0, False)
        self.assertEqual(result["status"], "pass", result["checks"])

    def test_load_spikes_are_not_a_leak(self):
        result = gi_soak.analyse(self.LOG, "", self.samples(), 0, False)
        self.assertLess(result["rss_growth_mb"], 1.0)

    def test_a_steady_leak_fails(self):
        result = gi_soak.analyse(self.LOG, "", self.samples(leak=4.0), 0, False)
        self.assertFalse(result["checks"]["no_leak"])

    def test_an_unoffered_producer_taken_fails(self):
        log = self.LOG + "\n[25.0] indirect light: switching to sdf"
        result = gi_soak.analyse(log, "", self.samples(), 0, False)
        self.assertFalse(result["checks"]["unoffered_never_taken"])

    def test_a_validation_message_fails(self):
        result = gi_soak.analyse(self.LOG, "Validation Error: [ VUID-x ]", self.samples(), 0,
                                 False)
        self.assertFalse(result["checks"]["no_validation_messages"])


if __name__ == "__main__":
    unittest.main()
