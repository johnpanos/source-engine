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

    def test_require_all_targets_names_each_uncovered_policy_target(self):
        targets = sorted(self.policy["targets"])
        covered = next(t for t in targets
                       if policy_module.target_dialect(self.policy, t, "c++") == "cxx20")
        errors, summary = boundary.verify_invocations(
            ROOT, self.policy, self.record(self.cxx(target=covered,
                                                    arguments=["g++", "-std=c++20"])), "fixture")
        self.assertEqual([], errors)
        self.assertEqual([covered], summary["policy_targets_covered"])
        missing, seen = boundary.coverage_errors(self.policy, [summary])
        self.assertEqual([covered], seen)
        self.assertEqual(len(targets) - 1, len(missing))
        self.assertFalse(any(" %s " % covered in message for message in missing))
        self.assertTrue(all(message.startswith("TOOLCHAIN010") for message in missing))
        complete = {"policy_targets_covered": targets}
        self.assertEqual([], boundary.coverage_errors(self.policy, [summary, complete])[0])

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


@unittest.skipUnless(__import__("shutil").which("g++") and __import__("shutil").which("nm"), "needs g++ and nm")
class AbiIslandTests(unittest.TestCase):
    """TOOLCHAIN011 on real objects built with each dual-ABI value."""

    ISLAND = {"id": "fixture", "host": "tool", "members": ["island"], "value": "1"}

    def setUp(self):
        self.directory = __import__("tempfile").TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)

    def compile(self, name, abi, source):
        import subprocess
        src = os.path.join(self.directory.name, name + ".cpp")
        obj = os.path.join(self.directory.name, name + ".o")
        with open(src, "w") as stream:
            stream.write(source)
        subprocess.run(["g++", "-std=c++20", "-O1", "-c", "-D_GLIBCXX_USE_CXX11_ABI=%d" % abi, src, "-o", obj],
                       check=True)
        return obj

    def errors(self, island_source, host_source):
        island = self.compile("island", 1, island_source)
        host = self.compile("host", 0, host_source)
        return boundary.island_symbol_errors("fixture", self.ISLAND, [island], [host])

    def test_c_compatible_edge_passes(self):
        island = 'extern "C" int island_write(const char *path, char *error, unsigned long size) { return path != 0; }\n'
        host = 'extern "C" int island_write(const char *, char *, unsigned long);\nint main() { return island_write("x", 0, 0); }\n'
        self.assertEqual([], self.errors(island, host))

    def test_dual_abi_type_crossing_the_edge_fails(self):
        island = '#include <string>\nstd::string Describe(int value) { return std::to_string(value); }\n'
        # An ABI 0 caller would ask for the old-ABI symbol, which the island does not
        # define; a caller built with the island's ABI shows the tagged crossing.
        host_new_abi = '#include <string>\nstd::string Describe(int);\nint Use() { return (int)Describe(3).size(); }\n'
        island_obj = self.compile("island", 1, island)
        host_obj = self.compile("hostnew", 1, host_new_abi)
        errors = boundary.island_symbol_errors("fixture", self.ISLAND, [island_obj], [host_obj])
        self.assertTrue(any("dual-ABI type" in e and "Describe" in e for e in errors), errors)

    def test_shared_first_party_inline_code_fails(self):
        shared = ('#include <string>\nstruct Sink { std::string path; int count = 0;\n'
                  '  __attribute__((noinline)) int Size() const { return count + (int)path.size(); } };\n')
        island = shared + 'int IslandUse() { Sink s; return s.Size(); }\n'
        host = shared + 'int HostUse() { Sink s; return s.Size(); }\n'
        errors = self.errors(island, host)
        self.assertTrue(any("defined on both sides" in e and "Sink" in e for e in errors), errors)

    def test_typeinfo_of_a_shared_interface_is_abi_neutral(self):
        shared = ('struct Source { virtual ~Source(); virtual int Read() = 0; };\n'
                  'struct Impl : Source { int Read() override { return 1; } };\n')
        island = shared + 'int IslandMake() { Impl i; return i.Read(); }\n'
        host = shared + 'int HostMake() { Impl i; return i.Read(); }\n'
        self.assertFalse(any("_ZTI" in e or "_ZTS" in e for e in self.errors(island, host)))


class DeclaredIslandTests(unittest.TestCase):
    def test_only_an_exact_declared_split_is_sanctioned(self):
        policy = {"abi": {"islands": [{"id": "i", "host": "tool", "members": ["a", "b"], "value": "1"}]}}
        self.assertIsNotNone(boundary.declared_island(policy, "tool", {"1": ["a", "b"], "0": ["tool", "x"]}))
        self.assertIsNone(boundary.declared_island(policy, "tool", {"1": ["a", "b", "extra"], "0": ["tool"]}))
        self.assertIsNone(boundary.declared_island(policy, "tool", {"1": ["a"], "0": ["tool", "b"]}))
        self.assertIsNone(boundary.declared_island(policy, "other", {"1": ["a", "b"], "0": ["other"]}))

