# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Pinned dependency and extraction sensitivity tests; no network required."""

import copy
import hashlib
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import product_profile as profile_tools


class ProductProfileTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.profile = copy.deepcopy(profile_tools.load_profile())
        self.dependency = self.profile["dependencies"]["dxvk_native"]
        self.dependency.update(cache_archive="fixture.tar.gz", extracted_directory="fixture",
                               required_files=["include/fixture.h", "lib/libfixture.so"])
        self.archive = self.root / "fixture.tar.gz"
        self.entries = [
            ("usr", "directory", b""),
            ("usr/include/fixture.h", "file", b"fixture header"),
            ("usr/lib/libfixture.so.1", "file", b"fixture library"),
            ("usr/lib/libfixture.so", "symlink", "libfixture.so.1"),
        ]
        self.make_archive(self.entries)

    def make_archive(self, entries):
        with tarfile.open(self.archive, "w:gz") as archive:
            for name, kind, content in entries:
                member = tarfile.TarInfo(name)
                if kind == "directory":
                    member.type = tarfile.DIRTYPE
                elif kind == "symlink":
                    member.type = tarfile.SYMTYPE
                    member.linkname = content
                elif kind == "hardlink":
                    member.type = tarfile.LNKTYPE
                    member.linkname = content
                elif kind == "device":
                    member.type = tarfile.CHRTYPE
                elif kind == "fifo":
                    member.type = tarfile.FIFOTYPE
                else:
                    member.size = len(content)
                archive.addfile(member, io.BytesIO(content) if kind == "file" else None)
        self.dependency["sha256"] = hashlib.sha256(self.archive.read_bytes()).hexdigest()
        self.dependency["archive_bytes"] = self.archive.stat().st_size

    def extract(self):
        return profile_tools.fetch_dependency(self.profile, self.root)

    def test_exact_existing_archive_and_relative_soname_link_pass(self):
        prefix = self.extract()
        self.assertEqual((prefix / "lib/libfixture.so").read_bytes(), b"fixture library")
        self.assertEqual(prefix, self.extract())
        result = profile_tools.verify_dependency(self.profile, "dxvk_native", prefix)
        self.assertEqual(result["members"], 4)
        self.assertEqual(result["sha256"], self.dependency["sha256"])

    def test_symlink_chain_inside_archive_passes(self):
        self.make_archive(self.entries + [("usr/lib/alias.so", "symlink", "libfixture.so")])
        prefix = self.extract()
        self.assertEqual((prefix / "lib/alias.so").read_bytes(), b"fixture library")

    def test_changed_archive_hash_is_rejected_before_extraction(self):
        content = self.archive.read_bytes()
        self.archive.write_bytes(content[:-1] + bytes([content[-1] ^ 1]))
        with self.assertRaisesRegex(profile_tools.ProfileError, "SHA-256"):
            self.extract()
        self.assertFalse((self.root / "fixture").exists())

    def test_truncated_archive_is_rejected(self):
        self.archive.write_bytes(b"bad")
        with self.assertRaisesRegex(profile_tools.ProfileError, "size mismatch"):
            self.extract()

    def test_archive_symlink_is_rejected(self):
        backing = self.root / "backing.tar.gz"
        self.archive.rename(backing)
        self.archive.symlink_to(backing)
        with self.assertRaisesRegex(profile_tools.ProfileError, "symlink"):
            self.extract()

    def test_traversal_and_absolute_entries_are_rejected(self):
        for name in ("../escaped", "/tmp/source-profile-escaped", "usr/../../escaped", "usr\\escape"):
            with self.subTest(name=name):
                self.make_archive(self.entries + [(name, "file", b"bad")])
                with self.assertRaises(profile_tools.ProfileError):
                    self.extract()
                self.assertFalse((self.root / "fixture").exists())
                self.assertFalse((self.root / "escaped").exists())

    def test_unsafe_or_missing_symlink_target_is_rejected(self):
        for target in ("../../../escaped", "/tmp/escaped", "missing.so", "..\\escaped"):
            with self.subTest(target=target):
                self.make_archive(self.entries + [("usr/lib/evil.so", "symlink", target)])
                with self.assertRaises(profile_tools.ProfileError):
                    self.extract()
                self.assertFalse((self.root / "fixture").exists())

    def test_symlink_cycle_is_rejected(self):
        self.make_archive(self.entries + [("usr/a", "symlink", "b"), ("usr/b", "symlink", "a")])
        with self.assertRaisesRegex(profile_tools.ProfileError, "cycle"):
            self.extract()

    def test_symlink_directory_parent_is_rejected(self):
        self.make_archive(self.entries + [("usr/alias", "symlink", "lib/libfixture.so.1"),
                                         ("usr/alias/child", "file", b"bad")])
        with self.assertRaisesRegex(profile_tools.ProfileError, "non-directory parent"):
            self.extract()

    def test_special_and_hardlink_entries_are_rejected(self):
        for kind in ("hardlink", "device", "fifo"):
            with self.subTest(kind=kind):
                self.make_archive(self.entries + [("usr/evil", kind, "usr/include/fixture.h")])
                with self.assertRaisesRegex(profile_tools.ProfileError, "unsupported archive entry"):
                    self.extract()

    def test_duplicate_entry_is_rejected(self):
        self.make_archive(self.entries + [("usr/include/fixture.h", "file", b"replacement")])
        with self.assertRaisesRegex(profile_tools.ProfileError, "duplicate"):
            self.extract()

    def test_empty_archive_is_rejected(self):
        self.make_archive([])
        with self.assertRaisesRegex(profile_tools.ProfileError, "empty"):
            self.extract()

    def test_extraction_budget_is_enforced(self):
        with mock.patch.object(profile_tools, "MAX_EXPANDED_BYTES", 2):
            with self.assertRaisesRegex(profile_tools.ProfileError, "budget"):
                self.extract()
        self.assertFalse((self.root / "fixture").exists())
        self.assertFalse(list(self.root.glob(".dependency-*")))

    def test_existing_tree_is_never_replaced(self):
        destination = self.root / "existing"
        destination.mkdir()
        sentinel = destination / "keep"
        sentinel.write_text("user content")
        with self.assertRaisesRegex(profile_tools.ProfileError, "refusing to replace"):
            profile_tools.safe_extract(self.archive, destination)
        self.assertEqual(sentinel.read_text(), "user content")

    def test_modified_extracted_header_is_rejected(self):
        prefix = self.extract()
        header = prefix / "include/fixture.h"
        header.write_bytes(b"modified bytes")
        with self.assertRaisesRegex(profile_tools.ProfileError, "differs from pinned archive"):
            self.extract()
        self.assertEqual(header.read_bytes(), b"modified bytes")

    def test_substituted_regular_file_is_rejected(self):
        prefix = self.extract()
        header = prefix / "include/fixture.h"
        header.unlink()
        header.symlink_to("../lib/libfixture.so.1")
        with self.assertRaisesRegex(profile_tools.ProfileError, "substituted"):
            self.extract()

    def test_modified_soname_link_is_rejected(self):
        prefix = self.extract()
        link = prefix / "lib/libfixture.so"
        link.unlink()
        link.symlink_to("../include/fixture.h")
        with self.assertRaisesRegex(profile_tools.ProfileError, "symlink differs"):
            self.extract()

    def test_extra_extracted_file_is_rejected(self):
        prefix = self.extract()
        (prefix / "include/extra.h").write_text("unexpected shadow header")
        with self.assertRaisesRegex(profile_tools.ProfileError, "additional entries"):
            self.extract()

    def test_missing_required_file_is_rejected(self):
        self.dependency["required_files"].append("include/missing.h")
        with self.assertRaisesRegex(profile_tools.ProfileError, "required file missing"):
            self.extract()

    def test_failed_download_leaves_no_cache_entry(self):
        self.archive.unlink()
        opener = mock.Mock()
        opener.open.return_value = io.BytesIO(b"incorrect archive")
        with mock.patch.object(profile_tools.urllib.request, "build_opener", return_value=opener):
            with self.assertRaisesRegex(profile_tools.ProfileError, "size mismatch"):
                self.extract()
        self.assertFalse(self.archive.exists())
        self.assertFalse(list(self.root.glob(".download-*")))
        self.assertFalse((self.root / "fixture").exists())

    def test_valid_pinned_download_is_verified_then_extracted(self):
        content = self.archive.read_bytes()
        self.archive.unlink()
        opener = mock.Mock()
        opener.open.return_value = io.BytesIO(content)
        with mock.patch.object(profile_tools.urllib.request, "build_opener", return_value=opener):
            prefix = self.extract()
        self.assertEqual(self.archive.read_bytes(), content)
        self.assertTrue((prefix / "include/fixture.h").is_file())

    def test_http_redirect_is_rejected(self):
        handler = profile_tools._HTTPSRedirects()
        with self.assertRaisesRegex(profile_tools.ProfileError, "HTTPS"):
            handler.redirect_request(None, None, 302, "Found", {}, "http://example.test/archive")

    def test_profile_requires_https_hash_and_exact_versions(self):
        profile_path = self.root / "profile.json"
        for field, value in (("url", "http://example.test/archive"), ("sha256", "latest"),
                             ("cache_archive", "../escape.tar.gz")):
            with self.subTest(field=field):
                changed = copy.deepcopy(self.profile)
                changed["dependencies"]["dxvk_native"][field] = value
                profile_path.write_text(json.dumps(changed))
                with self.assertRaises(profile_tools.ProfileError):
                    profile_tools.load_profile(profile_path)

    def probe(self, command):
        if command[-1] == "-dumpfullversion":
            return self.profile["toolchain"]["version"]
        if command[-1] == "--version":
            return "g++ (GCC) " + self.profile["toolchain"]["version"]
        return self.profile["dependencies"]["pkg_config"][command[-1]]

    def test_expected_environment_reports_only_declared_observations(self):
        with mock.patch.object(profile_tools, "_run", side_effect=self.probe), \
             mock.patch.object(profile_tools.platform, "system", return_value="Linux"), \
             mock.patch.object(profile_tools.platform, "machine", return_value="x86_64"):
            observation = profile_tools.check_environment(self.profile)
        self.assertEqual(observation["packages"]["sdl3"], "3.4.16")
        self.assertEqual(set(observation), {"os", "architecture", "compiler", "packages"})

    def test_wrong_architecture_is_rejected(self):
        with mock.patch.object(profile_tools.platform, "machine", return_value="aarch64"):
            with self.assertRaisesRegex(profile_tools.ProfileError, "requires"):
                profile_tools.check_environment(self.profile)

    def test_wrong_sdl_version_is_rejected(self):
        def probe(command):
            return "3.4.99" if command[-1] == "sdl3" else self.probe(command)
        with mock.patch.object(profile_tools, "_run", side_effect=probe):
            with self.assertRaisesRegex(profile_tools.ProfileError, "requires sdl3"):
                profile_tools.check_environment(self.profile)

    def test_wrong_compiler_version_is_rejected(self):
        with mock.patch.object(profile_tools, "_run", return_value="99.1"):
            with self.assertRaisesRegex(profile_tools.ProfileError, "requires GCC"):
                profile_tools.check_environment(self.profile)


if __name__ == "__main__":
    unittest.main()
