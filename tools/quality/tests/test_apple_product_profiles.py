#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The derived Apple product profiles resolve onto the pins they extend."""

from pathlib import Path
import sys
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))

import profile_extends  # noqa: E402

PROFILES = QUALITY.parents[1] / "quality/product_profiles"


class Portal2IosProfileTests(unittest.TestCase):
    def setUp(self):
        self.base = profile_extends.load_profile(PROFILES / "portal-ios-native-vulkan.json")
        self.derived = profile_extends.load_profile(PROFILES / "portal2-ios-native-vulkan.json")

    def test_pins_come_from_the_portal_profile(self):
        for key in ("target", "sdk", "host_toolchain", "dependencies"):
            if key == "target":
                self.assertEqual({k: v for k, v in self.derived[key].items() if k != "product"},
                                 {k: v for k, v in self.base[key].items() if k != "product"})
            else:
                self.assertEqual(self.derived[key], self.base[key])
        self.assertEqual(self.derived["ios"]["system_frameworks"],
                         self.base["ios"]["system_frameworks"])

    def test_the_game_is_its_own_app(self):
        ios, base = self.derived["ios"], self.base["ios"]
        for key in ("bundle_id", "build_directory", "app_bundle"):
            self.assertNotEqual(ios[key], base[key], key)
        self.assertEqual(self.derived["configure_options"]["build_games"], "portal2")
        self.assertTrue(self.derived["configure_options"]["static_composition"])
        self.assertIn("vscript",
                      self.derived["static_composition"]["additional_required_modules"])

    def test_content_is_staged_outside_the_build_directory(self):
        content = self.derived["content"]
        self.assertNotEqual(content["stage_directory"], self.derived["ios"]["build_directory"])
        self.assertIn("portal2", content["directories"])
        self.assertNotIn("extends", self.derived)


if __name__ == "__main__":
    unittest.main()
