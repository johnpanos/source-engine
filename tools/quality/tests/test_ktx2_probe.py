"""Negative controls for the RFC 0008 pinned KTX2 feasibility probe."""

import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))

import ktx2_probe as probe  # noqa: E402


class Ktx2ProbeTests(unittest.TestCase):
    def test_ldr_comparator_detects_seeded_channel_error(self):
        self.assertEqual(probe.compare_bytes(b"\x00\x80\xff", b"\x00\x81\xff", 1), 1)
        with self.assertRaisesRegex(probe.ProbeFailure, "channel error"):
            probe.compare_bytes(b"\x00\x80\xff", b"\x00\x81\xff", 0)

    def test_hdr_comparator_detects_seeded_channel_error(self):
        original = struct.pack("<ee", 0.5, 4.0)
        altered = struct.pack("<ee", 0.5, 5.0)
        self.assertEqual(probe.compare_half(original, altered, 1.0), 1.0)
        with self.assertRaisesRegex(probe.ProbeFailure, "channel error"):
            probe.compare_half(original, altered, 0.0)
        with self.assertRaisesRegex(probe.ProbeFailure, "nonfinite"):
            probe.compare_half(original, struct.pack("<ee", 0.5, float("nan")), 1.0)

    def test_profile_rejects_non_exact_revision(self):
        profile = json.loads(probe.DEFAULT_PROFILE.read_text())
        profile["dependencies"]["ktx_software"]["revision"] = "main"
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad-profile.json"
            path.write_text(json.dumps(profile))
            with self.assertRaisesRegex(probe.ProbeFailure, "full Git SHA"):
                probe.profile_at(path)


if __name__ == "__main__":
    unittest.main()
