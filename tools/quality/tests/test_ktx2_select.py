"""Negative controls for RFC 0008 product texture-format selection."""

import json
from pathlib import Path
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "texture"))

import ktx2_select as selector  # noqa: E402


FEATURES = sorted(selector.REQUIRED_FEATURES)


class Ktx2SelectTests(unittest.TestCase):
    def setUp(self):
        self.product, self.host, _, _ = selector.load_profiles(selector.DEFAULT_PRODUCT_PROFILE)
        self.report = {
            "$schema": "https://schema.khronos.org/vulkan/profiles-0.8-latest.json",
            "capabilities": {"device": {
                "properties": {"VkPhysicalDeviceProperties": {
                    "deviceName": "fixture GPU", "vendorID": 1, "deviceID": 2}},
                "formats": {}}}}
        for kind, targets in self.product["intent"]["texture_pipeline"]["format_preferences"].items():
            name = self.host["texture_classes"][kind]["formats"][targets[0]]
            self.report["capabilities"]["device"]["formats"][name] = {
                "VkFormatProperties": {"optimalTilingFeatures": list(FEATURES)}}

    def select(self):
        return selector.select(self.product, self.host, self.report, "product", "host", "device")

    def test_selects_first_supported_format(self):
        result = self.select()
        self.assertEqual(result["outcome"], "pass")
        self.assertEqual(result["selections"]["base-color"]["selected"]["target"], "bc7")

    def test_fallback_requires_explicit_profile_choice_and_device_support(self):
        formats = self.report["capabilities"]["device"]["formats"]
        formats.pop("VK_FORMAT_BC7_SRGB_BLOCK")
        formats["VK_FORMAT_ASTC_4x4_SRGB_BLOCK"] = {
            "VkFormatProperties": {"optimalTilingFeatures": list(FEATURES)}}
        self.assertEqual(self.select()["selections"]["base-color"]["selected"]["target"],
                         "astc")

    def test_missing_required_class_fails(self):
        self.report["capabilities"]["device"]["formats"].pop("VK_FORMAT_BC7_SRGB_BLOCK")
        result = self.select()
        self.assertEqual(result["outcome"], "fail")
        self.assertEqual(result["missing_required_classes"], ["base-color"])

    def test_missing_transfer_capability_fails(self):
        entry = self.report["capabilities"]["device"]["formats"]["VK_FORMAT_BC7_SRGB_BLOCK"]
        entry["VkFormatProperties"]["optimalTilingFeatures"].remove(
            "VK_FORMAT_FEATURE_TRANSFER_DST_BIT")
        self.assertEqual(self.select()["outcome"], "fail")

    def test_profile_rejects_undeclared_target(self):
        product = json.loads(selector.DEFAULT_PRODUCT_PROFILE.read_text())
        product["intent"]["texture_pipeline"]["format_preferences"]["hdr"] = ["bc7"]
        with tempfile.TemporaryDirectory() as scratch:
            path = Path(scratch) / "bad-profile.json"
            path.write_text(json.dumps(product))
            with self.assertRaisesRegex(selector.SelectionError, "invalid format preferences"):
                selector.load_profiles(path)


if __name__ == "__main__":
    unittest.main()
