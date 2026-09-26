#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Device-free checks of the iOS conformance runner's selection rules."""

from pathlib import Path
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))

import ios_conformance  # noqa: E402


class UnsupportedReasonTests(unittest.TestCase):
    def setUp(self):
        self.profile = ios_conformance.load_profile()

    def test_shared_unit_is_unsupported(self):
        suite = {"id": "s", "units": [{"id": "lib", "link": "shared", "sources": []}]}
        self.assertEqual(ios_conformance.unsupported_reason(self.profile, suite),
                         self.profile["unsupported"]["shared-unit"])

    def test_sanitizer_is_unsupported(self):
        suite = {"id": "s", "extra_flags": ["-fsanitize=thread"]}
        self.assertEqual(ios_conformance.unsupported_reason(self.profile, suite),
                         self.profile["unsupported"]["sanitizer"])

    def test_dual_abi_seed_is_unsupported_only_for_sensitivity(self):
        units = [{"id": "a", "flags": ["-D_GLIBCXX_USE_CXX11_ABI=1"], "sources": []},
                 {"id": "b", "flags": ["-D_GLIBCXX_USE_CXX11_ABI=0"], "sources": []}]
        seeded = {"id": "s", "kind": "sensitivity", "units": units}
        positive = {"id": "p", "kind": "positive", "units": units}
        self.assertEqual(ios_conformance.unsupported_reason(self.profile, seeded),
                         self.profile["unsupported"]["libstdcxx-dual-abi"])
        self.assertIsNone(ios_conformance.unsupported_reason(self.profile, positive))

    def test_ordinary_suite_is_supported(self):
        self.assertIsNone(ios_conformance.unsupported_reason(
            self.profile, {"id": "s", "extra_flags": ["-pthread"]}))


class PlatformDefineTests(unittest.TestCase):
    def test_linux_defines_become_the_ios_product_defines(self):
        profile = ios_conformance.load_profile()
        suite = {"id": "s", "extra_flags": ["-DLINUX", "-D_LINUX", "-DPOSIX", "-pthread"],
                 "units": [{"id": "u", "flags": ["-DLINUX"], "sources": []}]}
        translated = ios_conformance.translate_platform(profile, suite)
        self.assertEqual(translated["extra_flags"],
                         ["-DOSX=1", "-D_OSX=1", "-DPLATFORM_IOS=1", "-DPOSIX", "-pthread"])
        self.assertEqual(translated["units"][0]["flags"],
                         ["-DOSX=1", "-D_OSX=1", "-DPLATFORM_IOS=1"])
        # The manifest row itself is untouched.
        self.assertEqual(suite["extra_flags"][0], "-DLINUX")


class MainSignatureTests(unittest.TestCase):
    def check(self, text):
        with tempfile.TemporaryDirectory(dir=ios_conformance.ROOT) as directory:
            path = Path(directory) / "suite.cpp"
            path.write_text(text)
            relative = str(path.relative_to(ios_conformance.ROOT))
            return ios_conformance.main_takes_arguments({"id": "s", "sources": [relative]})

    def test_main_with_arguments(self):
        self.assertTrue(self.check("int main( int argc, char **argv )\n{\n}\n"))

    def test_main_without_arguments(self):
        self.assertFalse(self.check("int main()\n{\n}\n"))
        self.assertFalse(self.check("int main( void )\n{\n}\n"))

    def test_no_main_is_an_error(self):
        with self.assertRaises(ios_conformance.conformance.ManifestError):
            self.check("int helper() { return 0; }\n")


if __name__ == "__main__":
    unittest.main()
