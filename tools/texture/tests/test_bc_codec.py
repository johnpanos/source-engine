"""Fixtures for tools/texture/bc_codec.py: round trips through the pinned ktx."""
from pathlib import Path
import sys
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import bc_codec



TOOL = bc_codec.default_tool()


@unittest.skipIf(TOOL is None, "the pinned ktx tool is not built (build/toolchains)")
class RoundTripTest(unittest.TestCase):
    def setUp(self):
        y, x = np.mgrid[0:64, 0:96].astype(np.float32) / 96

        self.x, self.y = x, y

    def test_bc6h_keeps_hdr_light_close(self):
        hdr = (0.2 + 20 * np.exp(-((self.x - .4) ** 2 + (self.y - .3) ** 2) * 30))[..., None] * \
            np.array([1.0, 0.8, 0.6], np.float32)
        blocks = bc_codec.encode_bc6h(TOOL, hdr)
        self.assertEqual(len(blocks), bc_codec.block_bytes("bc6hu", 96, 64))
        error = bc_codec.hdr_error(hdr, bc_codec.decode_bc6h(blocks, 96, 64))
        # A steep, narrow peak on a tiny page: a gross-breakage bound, not the
        # quality bar (real lightmaps set that; see the LMAP v3 receipt).
        self.assertLess(error["relative_mean"], 0.02)
        self.assertLess(error["stops_max"], 0.25)

    def test_bc7_and_bc4_keep_eight_bit_data_close(self):
        rgba = np.stack([self.x * 255, self.y * 255, (self.x + self.y) * 127, self.x * 0 + 255],
                        -1).astype(np.uint8)
        back = bc_codec.decode_bc7(bc_codec.encode_bc7(TOOL, rgba), 96, 64)
        self.assertLessEqual(np.abs(back.astype(int) - rgba.astype(int)).max(), 8)
        mask = ((self.x > 0.5) * 230 + 10).astype(np.uint8)
        back = bc_codec.decode_bc4(bc_codec.encode_bc4(TOOL, mask), 96, 64)
        self.assertLessEqual(np.abs(back.astype(int) - mask.astype(int)).max(), 2)

    def test_invalid_input_is_refused(self):
        with self.assertRaises(bc_codec.CodecError):
            bc_codec.encode_bc6h(TOOL, -np.ones((4, 4, 3), np.float32))
        with self.assertRaises(bc_codec.CodecError):
            bc_codec.decode_bc6h(b"\0" * 15, 4, 4)


class SizeTest(unittest.TestCase):
    def test_partial_blocks_are_padded(self):
        self.assertEqual(bc_codec.block_bytes("bc6hu", 5, 5), 4 * 16)
        self.assertEqual(bc_codec.block_bytes("bc4", 4, 4), 8)


if __name__ == "__main__":
    unittest.main()
