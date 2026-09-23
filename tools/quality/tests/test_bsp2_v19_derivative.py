#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Check the v19 derivative's exact byte change and source pin."""

import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import bsp2_v19_derivative as derivative  # noqa: E402


class V19DerivativeTests(unittest.TestCase):
    def test_exact_header_change_and_pinned_source(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            maps = root / "maps"
            maps.mkdir()
            source = b"VBSP" + (20).to_bytes(4, "little") + b"payload"
            (maps / derivative.SOURCE_NAME).write_bytes(source)
            inventory = root / "inventory.json"
            inventory.write_text(json.dumps({"schema": "bsp2-corpus-inventory/v1",
                                             "maps": [{"name": derivative.SOURCE_NAME,
                                                       "version": 20,
                                                       "sha256": hashlib.sha256(source).hexdigest()}]}))
            output = root / "output"
            evidence = derivative.derive(maps, inventory, output)
            result = (output / derivative.OUTPUT_NAME).read_bytes()
            self.assertEqual(result, source[:4] + (19).to_bytes(4, "little") + source[8:])
            self.assertEqual(evidence["changed_bytes"], [4])
            self.assertEqual(json.loads((output / "inventory.json").read_text())["maps"][0]["version"], 19)
            with self.assertRaisesRegex(ValueError, "outside the source"):
                derivative.derive(maps, inventory, maps / "derivative")
            (maps / derivative.SOURCE_NAME).write_bytes(source + b"changed")
            with self.assertRaisesRegex(ValueError, "pinned inventory"):
                derivative.derive(maps, inventory, output)


if __name__ == "__main__":
    unittest.main()
