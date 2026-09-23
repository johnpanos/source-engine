# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Sensitivity tests for the Android APK verifier (tools/quality/android_apk.py).

A synthetic APK built from the product profile must pass the package checks,
and each seeded defect -- a dropped or undeclared module, a wrong-architecture
or 4 KB-aligned library, an unresolved DT_NEEDED, text relocations, a missing
entry point, a misaligned stored library, a broken touch asset, and manifest
regressions -- must be reported. No NDK or SDK is required.
"""

import contextlib
import copy
import io
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools" / "android"))
import android_apk
import touch_icons

FIXTURES = Path(__file__).resolve().parent / "fixtures" / "android"
ARM64 = "arm64-v8a"


def make_elf(machine=183, needed=(), soname=None, exports=(), load_align=16384, textrel=False):
    """A minimal ELF64 shared object with the sections the verifier reads."""
    strings = bytearray(b"\0")

    def intern(name):
        offset = len(strings)
        strings.extend(name.encode() + b"\0")
        return offset

    dynamic = [(1, intern(name)) for name in needed]
    if soname:
        dynamic.append((14, intern(soname)))
    if textrel:
        dynamic.append((22, 0))
    dynamic.append((0, 0))
    symbols = [(intern(name), 0x12, 1) for name in exports]  # global function, defined
    symbols.append((intern("imported"), 0x12, 0))  # an undefined import
    dynstr = bytes(strings)

    dynamic_bytes = b"".join(struct.pack("<qQ", tag, value) for tag, value in dynamic)
    dynsym = bytes(24) + b"".join(struct.pack("<IBBHQQ", name, info, 0, shndx, 0, 0)
                                  for name, info, shndx in symbols)
    shstrtab = b"\0.dynstr\0.dynamic\0.dynsym\0.shstrtab\0"

    def pad(blob):
        return blob + bytes(-len(blob) % 8)

    phoff = 64
    dynstr_off = phoff + 2 * 56
    dynamic_off = dynstr_off + len(pad(dynstr))
    dynsym_off = dynamic_off + len(dynamic_bytes)
    shstr_off = dynsym_off + len(dynsym)
    shoff = shstr_off + len(pad(shstrtab))
    header = struct.pack("<4sBBBB8sHHIQQQIHHHHHH", b"\x7fELF", 2, 1, 1, 0, bytes(8),
                         3, machine, 1, 0, phoff, shoff, 0, 64, 56, 2, 64, 5, 4)
    phdrs = struct.pack("<IIQQQQQQ", 1, 5, 0, 0, 0, shoff, shoff, load_align)
    phdrs += struct.pack("<IIQQQQQQ", 2, 6, dynamic_off, dynamic_off, dynamic_off,
                         len(dynamic_bytes), len(dynamic_bytes), 8)
    sections = [
        (0, 0, 0, 0, 0, 0),
        (1, 3, dynstr_off, len(dynstr), 0, 0),
        (9, 6, dynamic_off, len(dynamic_bytes), 1, 16),
        (18, 11, dynsym_off, len(dynsym), 1, 24),
        (26, 3, shstr_off, len(shstrtab), 0, 0),
    ]
    shdrs = b"".join(struct.pack("<IIQQQQIIQQ", name, kind, 0, 0, offset, size, link, 0, 8, entsize)
                     for name, kind, offset, size, link, entsize in sections)
    return header + phdrs + pad(dynstr) + dynamic_bytes + dynsym + pad(shstrtab) + shdrs


class FixtureApk:
    """The entries of a package that satisfies the profile; tests edit them before writing."""

    def __init__(self, profile, abis=(ARM64,)):
        android = profile["android"]
        self.profile = profile
        self.entries = {
            "AndroidManifest.xml": b"binary manifest placeholder",
            "resources.arsc": b"resources placeholder",
            "classes.dex": b"dex\n035\0" + ("L%s;" % android["activity"].replace(".", "/")).encode(),
        }
        self.stored = {}
        for abi in abis:
            machine = android_apk.ABI_MACHINE[abi]
            for lib in android["packaged_libraries"]:
                exports = [android["native_entry"]["symbol"]] \
                    if lib == android["native_entry"]["library"] else []
                needed = ["libc.so", "liblog.so"]
                if lib == "libmain.so":
                    needed += ["liblauncher.so", "libSDL3.so", "libtier0.so"]
                self.entries["lib/%s/%s" % (abi, lib)] = make_elf(
                    machine, needed=needed, soname=lib, exports=exports)
        manifest = android["touch_assets"]
        base = manifest.rsplit("/", 1)[0]
        icon = touch_icons.vtf(bytes(touch_icons.SIZE * touch_icons.SIZE * 4))
        names = ["materials/vgui/touch/portal.vtf", "materials/vgui/touch/use.vtf"]
        for name in names:
            self.entries["%s/%s" % (base, name)] = icon
        self.entries[manifest] = ("\n".join(names) + "\n").encode()

    def write(self, path, align_stored=True):
        page = self.profile["android"]["page_size_alignment"]
        with open(path, "wb") as raw, zipfile.ZipFile(raw, "w") as apk:
            for name, data in self.entries.items():
                info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
                if name in self.stored:
                    info.compress_type = zipfile.ZIP_STORED
                    if self.stored[name] and align_stored:
                        # zipalign's padding record (id 0xd935) puts the data on a page.
                        start = raw.tell() + 30 + len(name.encode())
                        padding = -(start + 4) % page
                        info.extra = struct.pack("<HH", 0xD935, padding) + bytes(padding)
                else:
                    info.compress_type = zipfile.ZIP_DEFLATED
                apk.writestr(info, data)
        return path


class ElfReaderTests(unittest.TestCase):
    def test_reads_what_the_package_checks_need(self):
        elf = android_apk.parse_elf(make_elf(62, needed=["libc.so", "libm.so"], soname="libx.so",
                                             exports=["SDL_main"], load_align=4096))
        self.assertEqual(elf["machine"], 62)
        self.assertEqual(elf["needed"], ["libc.so", "libm.so"])
        self.assertEqual(elf["soname"], "libx.so")
        self.assertEqual(elf["load_aligns"], [4096])
        self.assertEqual(elf["exports"], {"SDL_main"})  # the undefined import is excluded
        self.assertFalse(elf["textrel"])
        self.assertTrue(android_apk.parse_elf(make_elf(textrel=True))["textrel"])

    def test_rejects_malformed_input(self):
        good = make_elf()
        for data in (b"", b"MZ" + bytes(100), good[:4] + b"\x01" + good[5:], good[:200]):
            with self.assertRaises((ValueError, struct.error)):
                android_apk.parse_elf(data)


class PackageContentTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.profile = json.loads(android_apk.DEFAULT_PROFILE.read_text())
        self.apk = FixtureApk(self.profile)

    def failures(self, abis=(ARM64,), **write):
        path = self.apk.write(Path(self.temporary.name) / "fixture.apk", **write)
        failures = android_apk.Failures()
        android_apk.check_contents(path, self.profile, list(abis), failures)
        return failures.items

    def assertReports(self, fragment, **kwargs):
        items = self.failures(**kwargs)
        self.assertTrue(any(fragment in item for item in items),
                        "expected a failure containing %r, got %r" % (fragment, items))

    def lib(self, name, abi=ARM64):
        return "lib/%s/%s" % (abi, name)

    def test_conforming_package_passes(self):
        self.assertEqual(self.failures(), [])

    def test_every_declared_abi_passes(self):
        self.apk = FixtureApk(self.profile, self.profile["target"]["abis"])
        self.assertEqual(self.failures(abis=self.profile["target"]["abis"]), [])

    def test_dropped_module_is_reported(self):
        del self.apk.entries[self.lib("libvphysics.so")]
        self.assertReports("libvphysics.so is not packaged")

    def test_undeclared_module_is_reported(self):
        self.apk.entries[self.lib("libstray.so")] = make_elf(soname="libstray.so")
        self.assertReports("libstray.so is packaged but not declared")

    def test_unexpected_abi_is_reported(self):
        self.apk.entries["lib/x86_64/libmain.so"] = make_elf(62)
        self.assertReports("native ABIs")

    def test_missing_abi_is_reported(self):
        self.assertReports("native ABIs", abis=(ARM64, "x86_64"))

    def test_undeclared_abi_is_reported(self):
        self.assertReports("not declared by the profile", abis=(ARM64, "armeabi-v7a"))

    def test_wrong_architecture_is_reported(self):
        self.apk.entries[self.lib("libengine.so")] = make_elf(62, soname="libengine.so")
        self.assertReports("libengine.so: ELF machine 62")

    def test_4k_page_alignment_is_reported(self):
        self.apk.entries[self.lib("libclient.so")] = make_elf(load_align=4096)
        self.assertReports("libclient.so: PT_LOAD aligned to 4096")

    def test_unresolved_dependency_is_reported(self):
        self.apk.entries[self.lib("libengine.so")] = make_elf(needed=["libcrypto.so"])
        self.assertReports("needs libcrypto.so")

    def test_text_relocations_are_reported(self):
        self.apk.entries[self.lib("libserver.so")] = make_elf(textrel=True)
        self.assertReports("libserver.so: has text relocations")

    def test_missing_entry_point_is_reported(self):
        self.apk.entries[self.lib("libmain.so")] = make_elf(needed=["libc.so"])
        self.assertReports("libmain.so: does not export SDL_main")

    def test_non_elf_library_is_reported(self):
        self.apk.entries[self.lib("libtier0.so")] = b"not a library"
        self.assertReports("libtier0.so: not an ELF file")

    def test_stored_library_must_start_on_a_page(self):
        name = self.lib("libvstdlib.so")
        self.apk.stored[name] = True
        self.assertEqual(self.failures(), [])
        self.assertReports("libvstdlib.so: stored uncompressed", align_stored=False)

    def test_activity_missing_from_dex_is_reported(self):
        self.apk.entries["classes.dex"] = b"dex\n035\0"
        self.assertReports("classes.dex does not define")

    def test_missing_touch_icon_is_reported(self):
        del self.apk.entries["assets/touch/materials/vgui/touch/use.vtf"]
        self.assertReports("use.vtf, which is not packaged")

    def test_invalid_touch_icon_is_reported(self):
        icon = bytearray(self.apk.entries["assets/touch/materials/vgui/touch/portal.vtf"])
        struct.pack_into("<i", icon, 52, 13)  # IMAGE_FORMAT_DXT1
        self.apk.entries["assets/touch/materials/vgui/touch/portal.vtf"] = bytes(icon)
        self.assertReports("portal.vtf: format 13")

    def test_unsafe_touch_entry_is_reported(self):
        self.apk.entries["assets/touch/touch_icons.txt"] = b"../../lib/arm64-v8a/libmain.so\n"
        self.assertReports("unsafe entry")

    def test_missing_manifest_is_reported(self):
        del self.apk.entries["AndroidManifest.xml"]
        self.assertReports("APK lacks AndroidManifest.xml")


class ManifestTests(unittest.TestCase):
    def setUp(self):
        self.profile = json.loads(android_apk.DEFAULT_PROFILE.read_text())
        self.badging = (FIXTURES / "badging.txt").read_text()
        self.tree = (FIXTURES / "manifest_tree.txt").read_text()

    def failures(self, badging=None, tree=None, profile=None, abis=(ARM64,)):
        failures = android_apk.Failures()
        android_apk.check_manifest(
            android_apk.parse_badging(self.badging if badging is None else badging),
            android_apk.parse_manifest_tree(self.tree if tree is None else tree),
            profile or self.profile, list(abis), failures)
        return failures.items

    def assertReports(self, fragment, **kwargs):
        items = self.failures(**kwargs)
        self.assertTrue(any(fragment in item for item in items),
                        "expected a failure containing %r, got %r" % (fragment, items))

    def test_recorded_package_manifest_passes(self):
        self.assertEqual(self.failures(), [])

    def test_sdk_level_change_is_reported(self):
        self.assertReports("minSdkVersion", badging=self.badging.replace(
            "minSdkVersion:'29'", "minSdkVersion:'24'"))

    def test_package_name_change_is_reported(self):
        profile = copy.deepcopy(self.profile)
        profile["android"]["application_id"] = "org.example.other"
        self.assertReports("manifest name", profile=profile)

    def test_extra_permission_is_reported(self):
        badging = self.badging + "uses-permission: name='android.permission.READ_EXTERNAL_STORAGE'\n"
        self.assertReports("manifest permissions", badging=badging)

    def test_missing_vulkan_requirement_is_reported(self):
        badging = "\n".join(line for line in self.badging.splitlines()
                            if "vulkan" not in line)
        self.assertReports("does not require android.hardware.vulkan.version", badging=badging)

    def test_native_code_mismatch_is_reported(self):
        self.assertReports("native-code", abis=(ARM64, "x86_64"))

    def test_compressed_only_native_libraries_are_reported(self):
        tree = self.tree.replace("extractNativeLibs(0x010104ea)=true", "extractNativeLibs(0x010104ea)=false")
        self.assertReports("extractNativeLibs", tree=tree)

    def test_activity_recreated_on_fold_is_reported(self):
        # Drop screenSize|smallestScreenSize (0x400|0x800) from the recorded flags.
        tree = self.tree.replace("configChanges(0x0101001f)=0x5000bff4",
                                 "configChanges(0x0101001f)=0x%x" % (0x5000bff4 & ~0xC00))
        items = self.failures(tree=tree)
        self.assertTrue(any("lacks screenSize" in item for item in items), items)
        self.assertTrue(any("lacks smallestScreenSize" in item for item in items), items)


class SdkToolTests(unittest.TestCase):
    def test_missing_build_tools_fail_rather_than_skip(self):
        profile = json.loads(android_apk.DEFAULT_PROFILE.read_text())
        with tempfile.TemporaryDirectory() as empty:
            failures = android_apk.Failures()
            android_apk.check_with_sdk(Path(empty) / "none.apk", Path(empty), profile, [ARM64], failures)
        self.assertTrue(failures)
        self.assertIn("missing", failures.items[0])

    def test_cli_exit_status_follows_the_result(self):
        profile = json.loads(android_apk.DEFAULT_PROFILE.read_text())
        with tempfile.TemporaryDirectory() as root:
            apk = FixtureApk(profile).write(Path(root) / "fixture.apk")
            report = Path(root) / "report.json"
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                status = android_apk.main(["check", str(apk), "--abi", ARM64,
                                           "--build-tools", root, "--report", str(report)])
            self.assertEqual(status, 1)  # no SDK tools in an empty directory
            self.assertEqual(json.loads(report.read_text())["result"], "fail")


if __name__ == "__main__":
    unittest.main()
