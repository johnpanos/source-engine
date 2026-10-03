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
        cls.data = rprb.build(probes, chains, candidates=True)
        cls.layout = rprb.read(cls.data)
        cls.grid = cls.layout["atlas_offset"] - rprb.CANDIDATE_BYTES

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

    def test_texture_contains_grid(self):
        texture = rprb.gpu_texture(self.layout, rprb.MODE_BLEND)
        self.assertTrue(bool((texture[0, 3] == (16, 16, 16, 1)).all()))
        start = struct.unpack("<I", bytes(int(x) for x in texture[0, 8]))[0]
        self.assertGreaterEqual(texture[start:].size, rprb.CANDIDATE_CELLS * 8)
        self.assertTrue(bool(rprb.np.isfinite(texture).all()))


if __name__ == "__main__":
    unittest.main()
