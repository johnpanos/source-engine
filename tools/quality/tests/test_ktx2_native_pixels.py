"""Sensitivity checks for the RFC 0008 native KTX2 pixel evidence runner."""

from pathlib import Path
from types import SimpleNamespace
import sys
import unittest


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))

import ktx2_native_pixels as gate  # noqa: E402


DEVICE = {"vendorID": 4098, "deviceID": 5510, "deviceName": "fixture GPU"}
PASS_OUTPUT = "KTX2_DEVICE vendorID=4098 deviceID=5510 name=fixture GPU\nCONFORMANCE 19 0\n"


class Ktx2NativePixelGateTests(unittest.TestCase):
    def run_result(self, stdout=PASS_OUTPUT, returncode=0):
        return SimpleNamespace(stdout=stdout, stderr="", returncode=returncode)

    def test_accepts_complete_matching_device(self):
        result = gate.classify(self.run_result(), {"device": DEVICE})
        self.assertEqual(result["checks"], 19)

    def test_rejects_different_device(self):
        with self.assertRaisesRegex(gate.GateFailure, "differs"):
            gate.classify(self.run_result(), {"device": {**DEVICE, "deviceID": 42}})

    def test_rejects_missing_or_zero_checks(self):
        for output in ("KTX2_DEVICE vendorID=4098 deviceID=5510 name=fixture GPU\n",
                       PASS_OUTPUT.replace("19 0", "0 0"),
                       PASS_OUTPUT + "CONFORMANCE 19 0\n"):
            with self.subTest(output=output):
                with self.assertRaisesRegex(gate.GateFailure, "incomplete"):
                    gate.classify(self.run_result(output), {"device": DEVICE})

    def test_rejects_skip_or_crash(self):
        for status in (77, -11):
            with self.subTest(status=status):
                with self.assertRaises(gate.GateFailure):
                    gate.classify(self.run_result(returncode=status), {"device": DEVICE})

    def test_rejects_stale_or_incomplete_selection_plan(self):
        product, host, product_path, host_path = gate.selector.load_profiles(
            gate.selector.DEFAULT_PRODUCT_PROFILE)
        plan = {"outcome": "pass", "missing_required_classes": [],
                "product_profile": product["id"],
                "product_profile_sha256": gate.selector.sha256(product_path),
                "host_tool_profile": host["id"],
                "host_tool_profile_sha256": gate.selector.sha256(host_path),
                "selections": {kind: {"selected": {"target": "fixture"}}
                               for kind in host["texture_classes"]}}
        gate.validate_plan(plan)
        plan["product_profile_sha256"] = "stale"
        with self.assertRaisesRegex(gate.GateFailure, "does not match"):
            gate.validate_plan(plan)
        plan["product_profile_sha256"] = gate.selector.sha256(product_path)
        plan["selections"].pop("hdr")
        with self.assertRaisesRegex(gate.GateFailure, "lacks"):
            gate.validate_plan(plan)


if __name__ == "__main__":
    unittest.main()
