# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Positive and negative fixtures for the host frame capture comparator."""

import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import host_frame_capture  # noqa: E402

CAPTURE = """host-frame-trace/v1
frame numticks=1 render=1 threaded=0
Input 1 | ht=1 cft=1 hft=1 hfc=0 svt=10 sg=10/1 cg=5/1 ins=1 ia=0.0000
Server 1 | ht=1 cft=1 hft=1 hfc=0 svt=10 sg=10/1 cg=5/1 ins=1 ia=0.0000
Client 1 | ht=1 cft=1 hft=1 hfc=0 svt=11 sg=10/1 cg=5/1 ins=1 ia=0.0000
SetClientInSimulation 0 | ht=1 cft=1 hft=1 hfc=0 svt=11 sg=10/1 cg=5/1 ins=1 ia=0.0000
Render 0 | ht=1 cft=1 hft=1 hfc=0 svt=11 sg=10/1 cg=5/1 ins=0 ia=0.3333
frame numticks=0 render=1 threaded=0
SetClientInSimulation 0 | ht=1 cft=0 hft=0 hfc=1 svt=11 sg=10/1 cg=5/1 ins=1 ia=0.0000
Render 0 | ht=1 cft=0 hft=0 hfc=1 svt=11 sg=10/1 cg=5/1 ins=0 ia=0.6667
"""


def run_main(expected_text, actual_text):
    with tempfile.TemporaryDirectory() as directory:
        expected = os.path.join(directory, "legacy.trace")
        actual = os.path.join(directory, "graph.trace")
        with open(expected, "w") as stream:
            stream.write(expected_text)
        with open(actual, "w") as stream:
            stream.write(actual_text)
        return host_frame_capture.main([expected, actual])


class HostFrameCaptureTest(unittest.TestCase):
    def test_identical_captures_match(self):
        report = host_frame_capture.compare(host_frame_capture.parse(CAPTURE), host_frame_capture.parse(CAPTURE))
        self.assertTrue(report["identical"])
        self.assertEqual(report["compared_frames"], 2)
        self.assertEqual(report["compared_events"], 7)
        self.assertEqual(run_main(CAPTURE, CAPTURE), 0)

    def test_reordered_calls_are_detected(self):
        lines = CAPTURE.splitlines()
        lines[2], lines[3] = lines[3], lines[2]
        report = host_frame_capture.compare(host_frame_capture.parse(CAPTURE),
                                            host_frame_capture.parse("\n".join(lines)))
        self.assertFalse(report["identical"])
        self.assertEqual(report["first_divergence"]["frame"], 0)
        self.assertEqual(report["first_divergence"]["event"], 0)
        self.assertEqual(run_main(CAPTURE, "\n".join(lines)), 1)

    def test_changed_state_is_detected(self):
        changed = CAPTURE.replace("ins=0 ia=0.6667", "ins=0 ia=0.6666")
        report = host_frame_capture.compare(host_frame_capture.parse(CAPTURE), host_frame_capture.parse(changed))
        self.assertEqual(report["first_divergence"], {
            "frame": 1, "event": 1,
            "expected": "Render 0 | ht=1 cft=0 hft=0 hfc=1 svt=11 sg=10/1 cg=5/1 ins=0 ia=0.6667",
            "actual": "Render 0 | ht=1 cft=0 hft=0 hfc=1 svt=11 sg=10/1 cg=5/1 ins=0 ia=0.6666"})

    def test_missing_and_extra_calls_are_detected(self):
        dropped = CAPTURE.replace("Client 1 | ht=1 cft=1 hft=1 hfc=0 svt=11 sg=10/1 cg=5/1 ins=1 ia=0.0000\n", "")
        report = host_frame_capture.compare(host_frame_capture.parse(CAPTURE), host_frame_capture.parse(dropped))
        self.assertFalse(report["identical"])
        self.assertEqual(report["first_divergence"]["frame"], 0)
        extra = CAPTURE + "Sound 0 | ht=1 cft=0 hft=0 hfc=1 svt=11 sg=10/1 cg=5/1 ins=0 ia=0.6667\n"
        report = host_frame_capture.compare(host_frame_capture.parse(CAPTURE), host_frame_capture.parse(extra))
        self.assertEqual(report["first_divergence"]["event"], 2)
        self.assertEqual(report["first_divergence"]["expected"], "<end of frame>")

    def test_frame_shape_and_length_are_compared(self):
        shape = CAPTURE.replace("frame numticks=0", "frame numticks=2")
        self.assertIsNone(host_frame_capture.compare(
            host_frame_capture.parse(CAPTURE), host_frame_capture.parse(shape))["first_divergence"]["event"])
        truncated = CAPTURE.split("frame numticks=0")[0]
        report = host_frame_capture.compare(host_frame_capture.parse(CAPTURE), host_frame_capture.parse(truncated))
        self.assertFalse(report["identical"])
        self.assertEqual(report["first_divergence"]["actual"], "<end of capture>")

    def test_metadata_is_reported_not_compared(self):
        legacy = CAPTURE.replace("host-frame-trace/v1\n", "host-frame-trace/v1\n# host_frame_graph 0\n")
        graph = CAPTURE.replace("host-frame-trace/v1\n", "host-frame-trace/v1\n# host_frame_graph 1\n")
        metadata = []
        parsed = host_frame_capture.parse(graph, metadata=metadata)
        self.assertEqual(metadata, [{"frame": 0, "line": "host_frame_graph 1"}])
        self.assertTrue(host_frame_capture.compare(host_frame_capture.parse(legacy), parsed)["identical"])
        self.assertEqual(run_main(legacy, graph), 0)

    def test_tolerance_is_per_field_and_bounded(self):
        noisy = CAPTURE.replace("ia=0.6667", "ia=0.6666")
        expected, actual = host_frame_capture.parse(CAPTURE), host_frame_capture.parse(noisy)
        self.assertFalse(host_frame_capture.compare(expected, actual)["identical"])
        report = host_frame_capture.compare(expected, actual, tolerances={"ia": 0.0001})
        self.assertTrue(report["identical"])
        self.assertEqual(report["tolerated_differences"], {"ia": 1})
        # Outside the bound, in another field, or with a different call: divergent.
        far = CAPTURE.replace("ia=0.6667", "ia=0.6600")
        self.assertFalse(host_frame_capture.compare(
            expected, host_frame_capture.parse(far), tolerances={"ia": 0.0001})["identical"])
        tick = CAPTURE.replace("svt=11 sg=10/1 cg=5/1 ins=0 ia=0.6667", "svt=12 sg=10/1 cg=5/1 ins=0 ia=0.6667")
        self.assertFalse(host_frame_capture.compare(
            expected, host_frame_capture.parse(tick), tolerances={"ia": 0.0001})["identical"])
        renamed = noisy.replace("Render 0 | ht=1 cft=0", "Sound 0 | ht=1 cft=0")
        self.assertFalse(host_frame_capture.compare(
            expected, host_frame_capture.parse(renamed), tolerances={"ia": 0.0001})["identical"])

    def test_invalid_captures_are_rejected(self):
        with self.assertRaises(host_frame_capture.CaptureError):
            host_frame_capture.parse("frame numticks=1 render=1 threaded=0\n")
        with self.assertRaises(host_frame_capture.CaptureError):
            host_frame_capture.parse("host-frame-trace/v1\n")
        with self.assertRaises(host_frame_capture.CaptureError):
            host_frame_capture.parse("host-frame-trace/v1\nInput 1 | ht=0\n")
        with self.assertRaises(host_frame_capture.CaptureError):
            parsed = host_frame_capture.parse(CAPTURE)
            host_frame_capture.compare(parsed, parsed, skip_frames=2)
        self.assertEqual(run_main("", CAPTURE), 2)


if __name__ == "__main__":
    unittest.main()
