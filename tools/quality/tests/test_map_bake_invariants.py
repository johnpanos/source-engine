"""Production policy and compiled-data invariants; no reference-image comparisons."""
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import pbrt_map_build as pipeline
import pbrt_playable_content as content
import reflection_probe_set as rprb
import numpy as np


class ProductionPolicy(unittest.TestCase):
    def setUp(self):
        self.profile = pipeline.load_profile("source2")

    def test_quality_cannot_be_overridden(self):
        for key, value in (("lightmap", {"samples": 1}),
                           ("lightmap", {"device": "cpu"}),
                           ("probe_volume", {"fit_limit": True}),
                           ("radiosity", {"patch_size_m": 1}),
                           ("reflection_probe", None),
                           ("audit", {"require_directional": False})):
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                pipeline.with_defaults({key: value}, self.profile, key)

    def test_authored_locations_remain_content(self):
        value = pipeline.with_defaults({"reflection_probe": {"positions": [[1, 2, 3]]}},
                                       self.profile, "reflection_probe")
        self.assertEqual(value["positions"], [[1, 2, 3]])
        self.assertEqual(value["samples"], self.profile["reflection_probe"]["samples"])
        volumes = [{"capture": [1, 2, 3], "priority": 1}]
        value = pipeline.with_defaults({"reflection_probe": {"volumes": volumes}},
                                       self.profile, "reflection_probe")
        self.assertEqual(value["volumes"], volumes)

    def test_prbv_uses_the_authored_playable_envelope(self):
        build = object.__new__(pipeline.Pipeline)
        build.probe_volume = {}
        build.probe = {"volumes": [
            {"box_min": [-4, 2, -1], "box_max": [3, 6, 5]},
            {"box_min": [8, -2, 0], "box_max": [12, 4, 7]},
        ]}
        self.assertEqual(build.probe_volume_bounds(), [-4.0, -2.0, -1.0, 12.0, 6.0, 7.0])

        build.probe_volume = {"bounds_m": [-1, -1, -1, 1, 1, 1]}
        self.assertEqual(build.probe_volume_bounds(), [-1, -1, -1, 1, 1, 1])

    def test_reflection_and_prbv_share_the_playable_envelope(self):
        build = object.__new__(pipeline.Pipeline)
        build.paths = {"stage": "stage.usdc", "probe_placement": "placement.json"}
        build.profile = {"audit": {}}
        build.lightmap = {"device": "gpu", "seed": 0}
        build.probe_volume = {}
        build.probe = {"face_size": 256, "samples": 1024, "placement": {}, "volumes": [
            {"box_min": [-2, -3, -4], "box_max": [5, 6, 7]},
        ]}
        args = build.probe_arguments("scene.json", [])
        start = args.index("--bounds") + 1
        self.assertEqual(args[start:start + 6], ["-2.0", "-3.0", "-4.0", "5.0", "6.0", "7.0"])

    def test_retired_profile_rejected(self):
        with self.assertRaisesRegex(ValueError, "production map profile"):
            pipeline.load_profile("legacy-relight")

    def test_fixture_cannot_publish_or_mutate_output(self):
        with tempfile.TemporaryDirectory() as root:
            out = Path(root) / "untouched"
            with self.assertRaisesRegex(ValueError, "cannot publish"):
                pipeline.Pipeline({"quality": "gi-fixture"}, {}, out, None, False)
            self.assertFalse(out.exists())

    def test_authored_texture_resolution_preserved(self):
        self.assertEqual(content.target_size((4096, 2048)), (4096, 2048))


class CompiledCandidates(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        probes, chains = rprb.fixture_layout()
        cls.data = rprb.build(probes, chains)
        cls.layout = rprb.read(cls.data)
        cls.grid = cls.layout["data_offset"] - rprb.candidate_bytes(cls.layout["count"])

    def reject(self, offset, fmt, value):
        data = bytearray(self.data)
        struct.pack_into(fmt, data, offset, value)
        with self.assertRaises(rprb.RprbError) as caught:
            rprb.read(data)
        self.assertEqual(caught.exception.code, "InvalidCandidates")

    def test_loader_requires_coverage(self):
        self.reject(self.grid + 32, "<Q", 0)

    def test_loader_rejects_out_of_range_rank(self):
        self.reject(self.grid + 32, "<Q", (1 << 64) - 1)

    def test_loader_rejects_invalid_bounds(self):
        for offset, value in ((0, float("nan")), (12, 0.0), (12, 3.0)):
            with self.subTest(offset=offset, value=value):
                self.reject(self.grid + offset, "<f", value)
        self.reject(self.grid + 16, "<I", 1)

    def test_buffer_contains_grid(self):
        buffer = rprb.gpu_buffer(self.layout, rprb.MODE_BLEND)
        self.assertEqual((int(buffer[6]), int(buffer[3])), (rprb.CANDIDATE_DIM, 1))
        self.assertEqual(buffer[rprb.GPU_MASKS_WORD:].size, rprb.CANDIDATE_CELLS * 2)


class WideCandidates(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = rprb.capacity_fixture(count=256)
        cls.layout = rprb.read(cls.data)

    def test_high_ranks_and_gpu_storage(self):
        self.assertEqual(self.layout["count"], 256)
        self.assertEqual(self.layout["candidates"]["masks"].shape, (rprb.CANDIDATE_CELLS, 4))
        buffer = rprb.gpu_buffer(self.layout)
        self.assertEqual(int(buffer[3]), 4)
        self.assertEqual(buffer[rprb.GPU_MASKS_WORD:].size, rprb.CANDIDATE_CELLS * 4 * 2)

    def test_missing_high_word_coverage_rejected(self):
        data = bytearray(self.data)
        grid = self.layout["data_offset"] - rprb.candidate_bytes(256)
        struct.pack_into("<Q", data, grid + 32 + 24, 0)
        with self.assertRaisesRegex(rprb.RprbError, "InvalidCandidates"):
            rprb.read(data)

    def test_older_versions_are_refused(self):
        data = bytearray(self.data)
        struct.pack_into("<I", data, 4, 5)
        with self.assertRaisesRegex(rprb.RprbError, "UnsupportedVersion"):
            rprb.read(data)

    def test_unused_high_words_rejected(self):
        data = bytearray(rprb.capacity_fixture(count=65))
        layout = rprb.read(data)
        grid = layout["data_offset"] - rprb.candidate_bytes(65)
        struct.pack_into("<Q", data, grid + 32 + 24, 1 << 63)
        with self.assertRaisesRegex(rprb.RprbError, "InvalidCandidates"):
            rprb.read(data)


class PlacementProgress(unittest.TestCase):
    def test_joint_candidate_preserves_space_for_both_obligations(self):
        room = np.array([[1, 1, 1, 0], [1, 1, 0, 0], [0, 0, 1, 1]], dtype=bool)
        glossy = np.array([[0, 0], [1, 0], [0, 1]], dtype=bool)
        uncovered, unserved = np.ones(4, dtype=bool), np.ones(2, dtype=bool)
        chosen, stop = rprb.select_coverage(room, glossy, uncovered, unserved,
                                           np.ones(3, dtype=bool), np.ones(3, dtype=bool),
                                           2, 0, 0)
        self.assertEqual(stop, "coverage_pass")
        self.assertEqual(len(chosen), 2)
        self.assertFalse(uncovered.any() or unserved.any())

    def test_exhaustion_is_failure_not_loop_or_false_success(self):
        for capacity, expected in ((0, "max_probes"), (3, "no_progress")):
            uncovered = np.ones(2, dtype=bool)
            _, stop = rprb.select_coverage(np.array([[1, 0]], dtype=bool),
                np.zeros((1, 0), dtype=bool), uncovered, np.zeros(0, dtype=bool),
                np.ones(1, dtype=bool), np.ones(1, dtype=bool), capacity, 0, 0)
            self.assertEqual(stop, expected)
            self.assertTrue(uncovered.any())

    def test_bad_proxy_fit_cannot_be_selected_for_coverage(self):
        uncovered = np.ones(2, dtype=bool)
        chosen, stop = rprb.select_coverage(
            np.array([[1, 1], [1, 0]], dtype=bool), np.zeros((2, 0), dtype=bool),
            uncovered, np.zeros(0, dtype=bool), np.array([False, True]),
            np.ones(2, dtype=bool), 2, 0, 0)
        self.assertEqual(chosen, [1])
        self.assertEqual(stop, "no_progress")
        self.assertTrue(uncovered[1])


if __name__ == "__main__":
    unittest.main()
