"""Fixtures for LMAP v3 (tools/quality/world_lightmap_v3.py)."""
from pathlib import Path
import struct
import sys
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "texture" / "tests"))
import world_lightmap_v3 as lmap  # noqa: E402
from test_bc_codec import TOOL  # noqa: E402


def pages(size=32, seed=3):
    rng = np.random.default_rng(seed)
    y, x = np.mgrid[0:size, 0:size].astype(np.float32) / size
    flat = (0.3 + 4 * np.exp(-((x - .5) ** 2 + (y - .5) ** 2) * 8))[..., None] * \
        np.array([1.0, 0.9, 0.7], np.float32)
    beta = np.stack([np.sin(x * 6) * .5, np.cos(y * 5) * .4, (x - y) * .3], -1).astype(np.float32)
    return flat, beta, (x > 0.4).astype(np.float32)


@unittest.skipIf(TOOL is None, "the pinned ktx tool is not built (build/toolchains)")
class LightmapV3Test(unittest.TestCase):
    def test_round_trip_keeps_light_gradient_and_sun(self):
        # 128^2: on tiny pages the sun's edge blocks dominate BC7's shared
        # endpoints (32^2: beta max 0.095); at 256^2 the sun costs nothing
        # measurable (max 0.026 with and without).
        flat, beta, sun = pages(128)
        indirect = (flat * 0.4, beta * 0.5)
        data, report = lmap.build(TOOL, [(flat, beta), indirect], sun)
        layout = lmap.read(data)
        self.assertEqual([l["role"] for l in layout["layers"]], ["total", "indirect"])
        self.assertTrue(layout["sun"])
        back, beta_back, sun_back = lmap.decode_layer(data, layout, 0)
        self.assertLess(np.abs(back - flat).max() / flat.max(), 0.05)
        self.assertLess(np.abs(beta_back - beta).max(), 0.1)  # 4 / 255 steps, BC7
        self.assertLess(np.abs(sun_back - sun).max(), 0.02)
        # Every layer carries the sun (the indirect layer's gradient serves
        # runtime direct light).
        self.assertLess(np.abs(lmap.decode_layer(data, layout, 1)[2] - sun).max(), 0.02)
        self.assertLess(report["bytes"], report["rgba16f_bytes"] / 5)

    def test_beta_beyond_one_is_clamped_and_reported(self):
        flat, beta, _ = pages()
        _, report = lmap.build(TOOL, [(flat, beta * 8)])
        self.assertGreater(report["layers"][0]["beta_clamped_share"], 0.0)

    def test_negative_and_overflowed_light_is_clamped_and_reported(self):
        flat, beta, _ = pages()
        flat = flat.copy()
        flat[0, 0] = -0.01
        flat[1, 1] = np.inf
        data, report = lmap.build(TOOL, [(flat, beta)])
        back = lmap.decode_layer(data, lmap.read(data), 0)[0]
        self.assertGreater(report["layers"][0]["irradiance_negative_share"], 0.0)
        self.assertGreater(report["layers"][0]["irradiance_overflow_share"], 0.0)
        self.assertTrue(np.isfinite(back).all() and back.min() >= 0.0)
        with self.assertRaises(lmap.LightmapError):
            flat[2, 2] = np.nan
            lmap.build(TOOL, [(flat, beta)])

    def test_malformed_lumps_are_refused(self):
        flat, beta, _ = pages()
        data, _ = lmap.build(TOOL, [(flat, beta)])
        for offset, fmt, value, code in ((0, "<I", 0, "BadIdentifier"),
                                         (4, "<I", 2, "UnsupportedVersion"),
                                         (20, "<I", 2, "UnsupportedVersion"),
                                         (24, "<I", 97, "UnsupportedFormat"),
                                         (16, "<I", 4, "InvalidLayerCount"),
                                         (8, "<I", 33, "InvalidDescriptor")):
            bad = bytearray(data)
            struct.pack_into(fmt, bad, offset, value)
            with self.assertRaises(lmap.LightmapError) as caught:
                lmap.read(bytes(bad))
            self.assertEqual(caught.exception.code, code, (offset, value))
        with self.assertRaises(lmap.LightmapError):
            lmap.read(data[:-1])


if __name__ == "__main__":
    unittest.main()
