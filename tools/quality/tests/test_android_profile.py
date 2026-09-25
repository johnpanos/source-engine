# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Source-level guards for the Android build; no NDK, SDK or device required.

These catch regressions before a cross-build runs: the product profile and the
manifest template stay consistent with each other and with the APK verifier,
every touch-control texture the client names is generated, the generated icons
are deterministic and loadable, and the build script parses.
"""

import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ElementTree

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "android"))
import android_apk
import touch_icons

ANDROID_NS = "{http://schemas.android.com/apk/res/android}"


class ProfileTests(unittest.TestCase):
    def setUp(self):
        self.profile = json.loads(android_apk.DEFAULT_PROFILE.read_text())
        self.android = self.profile["android"]

    def test_every_declared_abi_is_mapped(self):
        abis = self.profile["target"]["abis"]
        self.assertTrue(abis)
        for abi in abis:
            self.assertIn(abi, android_apk.ABI_MACHINE)
            self.assertIn(abi, self.android["waf_arch"])
            self.assertIn(abi, self.android["ndk_triple"])

    def test_pins_are_complete(self):
        archives = {name: dependency for name, dependency in self.profile["dependencies"].items()
                    if "url" in dependency}
        self.assertEqual(sorted(archives), ["ktx_software", "ndk", "sdk_build_tools", "sdk_platform", "sdl3"])
        for name, dependency in archives.items():
            self.assertRegex(dependency["sha256"], r"^[0-9a-f]{64}$", name)
            self.assertGreater(dependency["archive_bytes"], 0, name)
            self.assertTrue(dependency["url"].startswith("https://"), name)
        self.assertRegex(self.android["compile_platform_jar_sha256"], r"^[0-9a-f]{64}$")
        self.assertLessEqual(self.android["min_sdk"], self.android["target_sdk"])
        self.assertEqual(self.android["page_size_alignment"], 16384)

    def test_package_facts_are_coherent(self):
        packaged = self.android["packaged_libraries"]
        self.assertEqual(len(packaged), len(set(packaged)))
        for required in (self.android["native_entry"]["library"], "libSDL3.so", "libc++_shared.so"):
            self.assertIn(required, packaged)
        self.assertFalse(set(packaged) & set(self.android["platform_libraries"]))
        for name in self.android["required_config_changes"]:
            self.assertIn(name, android_apk.CONFIG_CHANGES)


class DerivedProfileTests(unittest.TestCase):
    """Another game's profile extends the Portal profile instead of copying its pins."""

    PORTAL2 = ROOT / "quality/product_profiles/portal2-android-native-vulkan.json"

    def setUp(self):
        self.base = android_apk.load_profile(android_apk.DEFAULT_PROFILE)
        self.derived = android_apk.load_profile(self.PORTAL2)

    def test_pins_come_from_the_base(self):
        self.assertNotIn("dependencies", json.loads(self.PORTAL2.read_text()))
        self.assertEqual(self.derived["dependencies"], self.base["dependencies"])
        self.assertNotIn("extends", self.derived)
        for key in ("min_sdk", "target_sdk", "compile_platform", "waf_arch", "ndk_triple",
                    "permissions", "manifest_template"):
            self.assertEqual(self.derived["android"][key], self.base["android"][key], key)

    def test_products_do_not_collide(self):
        for key in ("application_id", "build_directory"):
            self.assertNotEqual(self.derived["android"][key], self.base["android"][key], key)
        self.assertEqual(self.derived["configure_options"]["build_games"], "portal2")
        self.assertIn("portal2", self.derived["content"]["directories"])
        self.assertIn("libvscript.so", self.derived["android"]["packaged_libraries"])
        self.assertEqual(sorted(set(self.derived["android"]["packaged_libraries"]) -
                                set(self.base["android"]["packaged_libraries"])),
                         ["libvscript.so"])

    def test_merge_rules(self):
        merged = android_apk.merge_profile({"a": {"x": 1, "y": [1]}, "b": 2},
                                           {"a": {"y": [2]}, "c": 3})
        self.assertEqual(merged, {"a": {"x": 1, "y": [2]}, "b": 2, "c": 3})

    def test_resolve_command_matches_loader(self):
        output = subprocess.run([sys.executable, str(ROOT / "tools/quality/android_apk.py"),
                                 "resolve", str(self.PORTAL2)],
                                check=True, capture_output=True, text=True).stdout
        self.assertEqual(json.loads(output), self.derived)


class ManifestTemplateTests(unittest.TestCase):
    """The source manifest must satisfy what the verifier requires of the packaged one."""

    def setUp(self):
        self.profile = json.loads(android_apk.DEFAULT_PROFILE.read_text())
        self.android = self.profile["android"]
        template = (ROOT / self.android["manifest_template"]).read_text()
        self.assertIn("@PACKAGE@", template)
        self.assertIn("@APP_LABEL@", template)
        text = template.replace("@PACKAGE@", self.android["application_id"]).replace(
            "@APP_LABEL@", self.android["application_label"])
        markup = re.sub(r"<!--.*?-->", "", text, flags=re.S)
        self.assertEqual(re.findall(r"@[A-Z_]+@", markup), [])  # every token substituted
        self.manifest = ElementTree.fromstring(text)

    def attribute(self, element, name):
        return element.get(ANDROID_NS + name)

    def test_permissions_are_exactly_the_profiles(self):
        declared = [self.attribute(e, "name") for e in self.manifest.iter("uses-permission")]
        self.assertEqual(sorted(declared), sorted(self.android["permissions"]))

    def test_required_features(self):
        required = {self.attribute(e, "name") for e in self.manifest.iter("uses-feature")
                    if self.attribute(e, "required") != "false"}
        for feature in self.android["required_features"]:
            self.assertIn(feature, required)

    def test_application_extracts_native_libraries(self):
        application = self.manifest.find("application")
        self.assertEqual(self.attribute(application, "extractNativeLibs"), "true")

    def test_activity_survives_rotation_and_folding(self):
        activities = list(self.manifest.iter("activity"))
        self.assertEqual([self.attribute(a, "name") for a in activities], [self.android["activity"]])
        changes = set(self.attribute(activities[0], "configChanges").split("|"))
        for name in changes:
            self.assertIn(name, android_apk.CONFIG_CHANGES)
        for name in self.android["required_config_changes"]:
            self.assertIn(name, changes)
        actions = [self.attribute(e, "name") for e in activities[0].iter("action")]
        self.assertIn("android.intent.action.MAIN", actions)


class TouchIconTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        cls.first = Path(cls.temporary.name) / "first"
        cls.second = Path(cls.temporary.name) / "second"
        for out in (cls.first, cls.second):
            subprocess.run([sys.executable, str(ROOT / "tools/android/touch_icons.py"), str(out)],
                           check=True, stdout=subprocess.DEVNULL)

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def test_every_texture_the_client_names_is_generated(self):
        source = (ROOT / "game/client/touch.cpp").read_text()
        referenced = set(re.findall(r'"vgui/touch/([A-Za-z0-9_]+)"', source))
        self.assertIn("portal", referenced)  # the pattern still matches the layout
        self.assertEqual(sorted(referenced - set(touch_icons.ICONS)), [])

    def test_icons_are_valid_and_listed(self):
        listed = (self.first / touch_icons.MANIFEST).read_text().split()
        self.assertEqual(sorted(listed), sorted("%s/%s.vtf" % (touch_icons.INSTALL_DIR, name)
                                                for name in touch_icons.ICONS))
        for entry in listed:
            self.assertIsNone(android_apk.check_vtf((self.first / entry).read_bytes()), entry)

    def test_output_is_deterministic(self):
        files = sorted(p.relative_to(self.first) for p in self.first.rglob("*") if p.is_file())
        self.assertTrue(files)
        for relative in files:
            self.assertEqual((self.first / relative).read_bytes(),
                             (self.second / relative).read_bytes(), str(relative))


class BuildScriptTests(unittest.TestCase):
    def test_script_parses(self):
        for script in ("build-android-apk.sh", "build-android-portal2-apk.sh"):
            subprocess.run(["bash", "-n", str(ROOT / script)], check=True)

    def test_portal2_script_uses_its_profile(self):
        script = (ROOT / "build-android-portal2-apk.sh").read_text()
        self.assertIn("portal2-android-native-vulkan.json", script)
        self.assertIn("stage_portal2_runtime.py\" --mount-custom", script)

    def test_script_verifies_the_package(self):
        script = (ROOT / "build-android-apk.sh").read_text()
        self.assertIn("tools/quality/android_apk.py\" check", script)


if __name__ == "__main__":
    unittest.main()
