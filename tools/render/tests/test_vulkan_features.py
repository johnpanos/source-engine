"""Seeded faults for tools/render/vulkan_features.py check: each must fail."""
import copy
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import vulkan_features


def failures(record):
    return [name for name, ok, _ in vulkan_features.validate(record) if not ok]


class VulkanFeaturesTest(unittest.TestCase):
    def setUp(self):
        self.record = json.loads(vulkan_features.RECORD.read_text())

    def test_recorded_file_passes(self):
        self.assertEqual(failures(self.record), [])

    def test_feature_flipped_false_fails(self):
        for profile in vulkan_features.REQUIRED_PROFILES:
            for feature in vulkan_features.FEATURES:
                record = copy.deepcopy(self.record)
                record["profiles"][profile]["features"][feature]["supported"] = False
                self.assertEqual(failures(record), ["%s.%s" % (profile, feature)])

    def test_feature_neither_core_nor_extension_fails(self):
        record = copy.deepcopy(self.record)
        entry = record["profiles"]["linux-desktop"]["features"]["synchronization2"]
        entry["core"] = entry["extension"] = False
        self.assertEqual(failures(record), ["linux-desktop.synchronization2"])

    def test_required_profile_removed_fails(self):
        for profile in vulkan_features.REQUIRED_PROFILES:
            record = copy.deepcopy(self.record)
            del record["profiles"][profile]
            self.assertEqual(failures(record), ["%s.present" % profile])

    def test_required_profile_unavailable_fails(self):
        record = copy.deepcopy(self.record)
        record["profiles"]["android-fold7"] = {"required": True, "status": "unavailable",
                                               "reason": "not attached", "date": "2026-09-28"}
        self.assertEqual(failures(record), ["android-fold7.measured"])

    def test_required_flag_cleared_fails(self):
        record = copy.deepcopy(self.record)
        record["profiles"]["android-fold7"]["required"] = False
        self.assertEqual(failures(record), ["android-fold7.required"])

    def test_missing_evidence_fails(self):
        record = copy.deepcopy(self.record)
        del record["profiles"]["linux-desktop"]["evidence"]["probe_sha256"]
        self.assertEqual(failures(record), ["linux-desktop.evidence"])

    def test_optional_unavailable_needs_reason(self):
        record = copy.deepcopy(self.record)
        record["profiles"]["ios-iphone"]["reason"] = ""
        self.assertEqual(failures(record), ["ios-iphone.reason"])

    def test_wrong_schema_fails(self):
        record = copy.deepcopy(self.record)
        record["schema"] = "render-device-vulkan-features/v0"
        self.assertEqual(failures(record), ["schema"])

    def test_probe_entry_uses_the_selected_device(self):
        output = json.dumps({
            "probe": "p/v1", "loader_api": "1.4.0", "adapter_selects": 1,
            "devices": [
                {"index": 0, "name": "cpu", "type": "cpu", "vendor_id": "0x1", "device_id": "0x2",
                 "api_version": "1.3.0", "driver_name": "d", "driver_info": "i",
                 "portability_subset": False, "features": {},
                 "adapter": {"eligible": True, "path": "core13", "reason": ""}},
                {"index": 1, "name": "gpu", "type": "integrated", "vendor_id": "0x3",
                 "device_id": "0x4", "api_version": "1.3.0", "driver_name": "d",
                 "driver_info": "i", "portability_subset": False, "features": {"x": 1},
                 "adapter": {"eligible": True, "path": "core13", "reason": ""}}]})
        entry = vulkan_features.entry_from_probe(output, {"platform": "linux"}, "cmd", True)
        self.assertEqual(entry["device"]["name"], "gpu")
        self.assertEqual([d["selected"] for d in entry["all_devices"]], [False, True])

    def test_host_only_needs_timeline_and_synchronization2(self):
        for feature in vulkan_features.HOST_FEATURES:
            record = copy.deepcopy(self.record)
            record["profiles"]["android-tab-s8-ultra"]["features"][feature]["supported"] = False
            self.assertEqual(failures(record), ["android-tab-s8-ultra.%s" % feature])

    def test_host_only_required_profile_fails(self):
        record = copy.deepcopy(self.record)
        record["profiles"]["android-fold7"]["adapter_path"] = "host-only"
        self.assertEqual(failures(record), ["android-fold7.host-only"])

    def test_port_path_without_dynamic_rendering_fails(self):
        record = copy.deepcopy(self.record)
        record["profiles"]["android-tab-s8-ultra"]["adapter_path"] = "vulkan12+extensions"
        self.assertEqual(failures(record), ["android-tab-s8-ultra.dynamic_rendering"])

    def test_probe_falls_back_to_the_host_selection(self):
        device = {"index": 0, "name": "gpu", "type": "integrated", "vendor_id": "0x3",
                  "device_id": "0x4", "api_version": "1.1.128", "driver_name": "d",
                  "driver_info": "i", "portability_subset": False, "features": {},
                  "adapter": {"eligible": False, "path": "vulkan12+extensions",
                              "reason": "API below 1.2"}}
        output = json.dumps({"probe": "p/v2", "loader_api": "1.4.0", "adapter_selects": -1,
                             "host_selects": 0, "devices": [device]})
        entry = vulkan_features.entry_from_probe(output, {}, "cmd", False)
        self.assertEqual(entry["adapter_path"], "host-only")
        output = json.dumps({"probe": "p/v2", "loader_api": "1.4.0", "adapter_selects": -1,
                             "host_selects": -1, "devices": [device]})
        with self.assertRaises(vulkan_features.FeatureError):
            vulkan_features.entry_from_probe(output, {}, "cmd", False)

    def test_probe_selecting_nothing_is_an_error(self):
        output = json.dumps({"probe": "p/v1", "loader_api": "1.4.0", "adapter_selects": -1,
                             "devices": []})
        with self.assertRaises(vulkan_features.FeatureError):
            vulkan_features.entry_from_probe(output, {}, "cmd", True)


if __name__ == "__main__":
    unittest.main()
