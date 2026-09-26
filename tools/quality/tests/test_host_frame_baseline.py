#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Self-tests for tools/quality/host_frame_baseline.py (RFC 0003, R10).

The versioned legacy host-frame fixture is trusted only if it loads complete
(counts match its manifest), a copy of it matches itself, and every kind of
altered capture fails against it: a dropped event, two swapped events, a
changed tick field, and an ia change beyond the declared tolerance. An ia
change inside the tolerance must pass and be counted.
"""

import gzip
import json
import os
import re
import sys
import tempfile
import unittest
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import host_frame_baseline  # noqa: E402
import host_frame_capture  # noqa: E402


class FixtureTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest, cls.frames = host_frame_baseline.load_fixture(host_frame_baseline.FIXTURE)
        trace = host_frame_baseline.FIXTURE.parent / cls.manifest["trace"]
        cls.text = gzip.decompress(trace.read_bytes()).decode()

    def compare(self, text):
        path = Path(tempfile.mkdtemp(prefix="host-frame-")) / "h.t"
        path.write_text(text)
        return host_frame_baseline.compare_to_fixture(self.manifest, self.frames, path)

    def lines(self):
        return self.text.splitlines()

    def event_indices(self):
        return [i for i, line in enumerate(self.lines()) if " | " in line]

    def test_fixture_is_complete_and_nontrivial(self):
        self.assertEqual(host_frame_baseline.SCHEMA, self.manifest["schema"])
        self.assertGreater(self.manifest["frames"], 100)
        self.assertGreater(self.manifest["events"], 1000)

    def test_copy_matches(self):
        self.assertTrue(self.compare(self.text)["identical"])

    def test_dropped_event_fails(self):
        lines = self.lines()
        del lines[self.event_indices()[500]]
        self.assertFalse(self.compare("\n".join(lines) + "\n")["identical"])

    def test_swapped_events_fail(self):
        lines = self.lines()
        indices = self.event_indices()
        for a, b in zip(indices, indices[1:]):
            if lines[a].split(" | ")[0] != lines[b].split(" | ")[0] and b == a + 1:
                lines[a], lines[b] = lines[b], lines[a]
                break
        self.assertFalse(self.compare("\n".join(lines) + "\n")["identical"])

    def test_changed_tick_field_fails(self):
        lines = self.lines()
        index = next(i for i in self.event_indices() if re.search(r"\bht=[1-9]", lines[i]))
        lines[index] = re.sub(r"\bht=(\d+)", lambda m: "ht=%d" % (int(m.group(1)) + 1), lines[index], count=1)
        self.assertFalse(self.compare("\n".join(lines) + "\n")["identical"])

    def nudge_ia(self, delta):
        lines = self.lines()
        index = next(i for i in self.event_indices() if " ia=" in lines[i])
        lines[index] = re.sub(r"ia=(-?[\d.]+)", lambda m: "ia=%.4f" % (float(m.group(1)) + delta),
                              lines[index], count=1)
        return self.compare("\n".join(lines) + "\n")

    def test_ia_inside_tolerance_passes_and_is_counted(self):
        report = self.nudge_ia(0.0001)
        self.assertTrue(report["identical"])
        self.assertEqual(1, report["tolerated_differences"].get("ia"))

    def test_ia_beyond_tolerance_fails(self):
        self.assertFalse(self.nudge_ia(0.01)["identical"])

    def test_manifest_count_mismatch_is_rejected(self):
        directory = Path(tempfile.mkdtemp(prefix="host-frame-fixture-"))
        manifest = dict(self.manifest, events=self.manifest["events"] + 1)
        (directory / manifest["trace"]).write_bytes(
            (host_frame_baseline.FIXTURE.parent / manifest["trace"]).read_bytes())
        (directory / "fixture.json").write_text(json.dumps(manifest))
        with self.assertRaises(host_frame_capture.CaptureError):
            host_frame_baseline.load_fixture(directory / "fixture.json")


if __name__ == "__main__":
    unittest.main()
