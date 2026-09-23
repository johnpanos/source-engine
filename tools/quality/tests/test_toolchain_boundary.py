#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Negative fixtures for the R03 final-command and link-ABI verifier."""

import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools", "quality"))

import toolchain_boundary as boundary  # noqa: E402
import toolchain_policy as policy_module  # noqa: E402


class ToolchainBoundaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.policy = policy_module.load_policy(ROOT)

    def record(self, *entries):
        return {
            "schema": boundary.INVOCATIONS_SCHEMA,
            "policy_sha256": boundary.policy_digest(ROOT),
            "toolchain": {"compiler_cxx": "g++", "compiler_cc": "gcc"},
            "entries": list(entries),
        }

    def cxx(self, target="consumer", arguments=None, use=None, features=None):
        return {
            "target": target,
            "source": "fixture.cpp",
            "language": "c++",
            "dialect": policy_module.target_dialect(self.policy, target, "c++"),
            "arguments": arguments or ["g++", "-std=c++20", "-fpermissive"],
            "use": use or [],
            "features": features or ["cxxstlib"],
        }

    def verify(self, record):
        return boundary.verify_invocations(ROOT, self.policy, record, "fixture")[0]

    def test_normal_cxx20_command_passes(self):
        self.assertEqual([], self.verify(self.record(self.cxx())))

    def test_cxx11_escape_is_rejected(self):
        entry = self.cxx(arguments=["g++", "-std=c++11", "-fpermissive"])
        self.assertIn("TOOLCHAIN002", " ".join(self.verify(self.record(entry))))

    def test_duplicate_standard_flag_is_rejected(self):
        entry = self.cxx(arguments=["g++", "-std=c++11", "-std=c++20", "-fpermissive"])
        self.assertIn("TOOLCHAIN002", " ".join(self.verify(self.record(entry))))

    def test_strict_target_rejects_permissive_flag(self):
        entry = self.cxx(target="jobsystem", arguments=["g++", "-std=c++20", "-fpermissive"])
        self.assertIn("TOOLCHAIN004", " ".join(self.verify(self.record(entry))))

    def test_box3d_fast_math_is_rejected(self):
        entry = {
            "target": "box3d_private", "source": "box3d.c", "language": "c",
            "dialect": "box3d-c17", "features": ["cstlib"], "use": [],
            "arguments": ["gcc", "-std=gnu17", "-ffp-contract=off", "-fno-fast-math",
                          "-fvisibility=hidden", "-fPIC", "-ffast-math"],
        }
        self.assertIn("TOOLCHAIN004", " ".join(self.verify(self.record(entry))))

    def test_recorded_dialect_must_match_policy(self):
        entry = self.cxx()
        entry["dialect"] = "legacy-cxx11"
        self.assertIn("TOOLCHAIN006", " ".join(self.verify(self.record(entry))))

    def test_link_closure_rejects_dual_abi_mismatch(self):
        provider = self.cxx(target="provider", arguments=["g++", "-std=c++20",
                                                           "-fpermissive", "-D_GLIBCXX_USE_CXX11_ABI=0"])
        consumer = self.cxx(target="consumer", use=["provider"], features=["cxxprogram"])
        self.assertIn("TOOLCHAIN007", " ".join(self.verify(self.record(provider, consumer))))

    def test_stale_policy_and_zero_invocations_fail(self):
        record = self.record(self.cxx())
        record["policy_sha256"] = "stale"
        self.assertIn("stale evidence", " ".join(self.verify(record)))
        record["entries"] = []
        self.assertIn("zero compile invocations", " ".join(self.verify(record)))

    def test_msvc_cxx20_requires_true_language_macro(self):
        flags = policy_module.dialect_flags(self.policy, "cxx20-permissive", "msvc")
        self.assertEqual([], policy_module.check_flags(self.policy, "cxx20-permissive",
                                                        "msvc", flags))
        flags.remove("/Zc:__cplusplus")
        self.assertIn("TOOLCHAIN003", " ".join(policy_module.check_flags(
            self.policy, "cxx20-permissive", "msvc", flags)))

    def test_exact_cxx11_fixture_has_no_msvc_mapping(self):
        with self.assertRaisesRegex(policy_module.PolicyError, "no mapping"):
            policy_module.dialect_mapping(self.policy, "legacy-cxx11", "msvc")


if __name__ == "__main__":
    unittest.main()
