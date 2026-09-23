#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Declaration and failure sensitivity for the native Apple R03 probe."""

import copy
import json
from pathlib import Path
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import apple_toolchain as apple  # noqa: E402


class AppleToolchainTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.profile = json.loads(apple.PROFILE.read_text(encoding="utf-8"))

    def test_declared_profiles_are_complete(self):
        apple.check_profile(self.profile)

    def test_missing_sdk_fails(self):
        data = copy.deepcopy(self.profile)
        data["profiles"].pop()
        with self.assertRaisesRegex(apple.ProbeError, "three distinct targets"):
            apple.check_profile(data)

    def test_wrong_dialect_owner_fails(self):
        data = copy.deepcopy(self.profile)
        data["dialect_policy"] = "another-policy.json"
        with self.assertRaisesRegex(apple.ProbeError, "authoritative dialect policy"):
            apple.check_profile(data)

    def test_missing_native_runner_fails_before_compilation(self):
        with mock.patch.object(apple.platform, "system", return_value="Linux"), \
             mock.patch.object(apple, "command") as command:
            with self.assertRaisesRegex(apple.ProbeError, "arm64 macOS runner"):
                apple.check(self.profile, Path("/tmp"), {})
            command.assert_not_called()

    def test_nonzero_compiler_result_fails(self):
        failure = mock.Mock(returncode=1, stdout="", stderr="seeded compiler failure")
        with mock.patch.object(apple.subprocess, "run", return_value=failure):
            with self.assertRaisesRegex(apple.ProbeError, "seeded compiler failure"):
                apple.command(["clang++", "-std=c++20", "probe.cpp"], {})


if __name__ == "__main__":
    unittest.main()
