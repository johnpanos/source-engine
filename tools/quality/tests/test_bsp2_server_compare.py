#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Negative controls for the dedicated-server VBSP/BSP2 oracle."""

from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import bsp2_server_compare  # noqa: E402


def capture(kind="legacy-vbsp", fraction="0.25", pvs="12345678"):
    return (f"map_container_info: maps/example.bsp kind={kind} "
            "legacy_version=20 revision=1 lumps=64\n"
            "map_container_info: gamelump sprp v5 size=4 loaded=1 crc=abcdef12\n"
            f"map_container_probe: leaf=1 cluster=2 contents=0 pvs_crc={pvs} "
            f"fraction={fraction} startsolid=0 allsolid=0 hit_contents=1\n"
            "map_container_probe: leaf=2 cluster=-1 contents=1 pvs_crc=87654321 "
            "fraction=1 startsolid=0 allsolid=0 hit_contents=0\n"
            "map_container_probe: leaf=3 cluster=3 contents=0 pvs_crc=11111111 "
            "fraction=1 startsolid=0 allsolid=0 hit_contents=0\n"
            "map     : example at: 0 x, 0 y, 0 z\n"
            "edicts  : 12 used of 2048 max\n")


class Bsp2ServerCompareTests(unittest.TestCase):
    def setUp(self):
        self.scratch = tempfile.TemporaryDirectory()
        self.addCleanup(self.scratch.cleanup)

    def read(self, text):
        path = Path(self.scratch.name) / "capture.log"
        path.write_text(text)
        return bsp2_server_compare.read_capture(path)

    def test_equal_server_observations(self):
        legacy = self.read(capture())
        bsp2 = self.read(capture(kind="bsp2"))
        self.assertEqual(bsp2_server_compare.compare(legacy, bsp2), [])

    def test_changed_visibility_and_trace_are_detected(self):
        legacy = self.read(capture())
        self.assertIn("probes differs", bsp2_server_compare.compare(
            legacy, self.read(capture(kind="bsp2", pvs="deadbeef"))))
        self.assertIn("probes differs", bsp2_server_compare.compare(
            legacy, self.read(capture(kind="bsp2", fraction="0.5"))))

    def test_missing_and_vacuous_probes_fail(self):
        with self.assertRaisesRegex(ValueError, "fewer than three"):
            self.read(capture().replace(
                "map_container_probe: leaf=3 cluster=3 contents=0 pvs_crc=11111111 "
                "fraction=1 startsolid=0 allsolid=0 hit_contents=0\n", ""))
        with self.assertRaisesRegex(ValueError, "no probe hit world"):
            self.read(capture(fraction="1"))
        with self.assertRaisesRegex(ValueError, "different maps"):
            self.read(capture().replace("map     : example", "map     : other"))


if __name__ == "__main__":
    unittest.main()
