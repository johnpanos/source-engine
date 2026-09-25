"""Negative fixtures for the VPhysics provider conformance runner."""

import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest

QUALITY = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(QUALITY))
SPEC = importlib.util.spec_from_file_location("physics_conformance", QUALITY / "physics_conformance.py")
physics = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(physics)

GOOD = """PASS boot module.load
FAIL gameplay trace.ray-hit: fraction 1.000
OBS surfaceprops.000.name x default
OBS vcollide.0.0.volume r0.05 100.0000
TIER boot 1 0
TIER gameplay 1 1
CONFORMANCE 2 1
"""


class ParseTests(unittest.TestCase):
    def test_parses_checks_observations_and_summary(self):
        parsed = physics.parse_output(GOOD)
        self.assertEqual(2, len(parsed["checks"]))
        self.assertEqual("fail", parsed["checks"][1]["status"])
        self.assertEqual("trace.ray-hit", parsed["checks"][1]["name"])
        self.assertEqual({"checks": 2, "failed": 1}, parsed["summary"])
        self.assertEqual(["default"], parsed["observations"]["surfaceprops.000.name"]["values"])

    def test_duplicate_summary_is_not_a_result(self):
        parsed = physics.parse_output(GOOD + "CONFORMANCE 2 1\n")
        self.assertEqual("duplicate", parsed["summary"])


class CompareTests(unittest.TestCase):
    def obs(self, tol, *values):
        return {"tolerance": tol, "values": list(values)}

    def test_exact_mismatch_diverges(self):
        result = physics.compare_observations({"k": self.obs("x", "default")}, {"k": self.obs("x", "metal")})
        self.assertEqual(1, result["divergent"])

    def test_absolute_tolerance(self):
        ref = {"k": self.obs("a0.5", "1.0", "2.0")}
        self.assertEqual(0, physics.compare_observations(ref, {"k": self.obs("a0.5", "1.4", "2.0")})["divergent"])
        self.assertEqual(1, physics.compare_observations(ref, {"k": self.obs("a0.5", "1.6", "2.0")})["divergent"])

    def test_relative_tolerance(self):
        ref = {"k": self.obs("r0.05", "100.0")}
        self.assertEqual(0, physics.compare_observations(ref, {"k": self.obs("r0.05", "104.0")})["divergent"])
        self.assertEqual(1, physics.compare_observations(ref, {"k": self.obs("r0.05", "106.0")})["divergent"])

    def test_missing_and_extra_keys_diverge(self):
        ref = {"k": self.obs("x", "1")}
        self.assertEqual(1, physics.compare_observations(ref, {})["divergent"])
        self.assertEqual(1, physics.compare_observations(ref, {"k": self.obs("x", "1"), "extra": self.obs("x", "1")})["divergent"])

    def test_value_count_mismatch_diverges(self):
        ref = {"k": self.obs("a1", "1", "2", "3")}
        self.assertEqual(1, physics.compare_observations(ref, {"k": self.obs("a1", "1", "2")})["divergent"])


class SensitivityTests(unittest.TestCase):
    def test_fault_requires_a_targeted_failure(self):
        parsed = physics.parse_output(GOOD)
        self.assertTrue(physics.fault_detected(parsed, ["trace."]))
        self.assertFalse(physics.fault_detected(parsed, ["module."]))

    def test_every_fault_names_targets(self):
        for fault, prefixes in physics.FAULTS.items():
            self.assertTrue(prefixes, fault)


class ReferenceDeficiencyTests(unittest.TestCase):
    DEFICIENT = {"gyro.free-energy-not-gained": "explicit gyroscopic integration"}

    def result(self, *checks):
        return {"outcome": "fail" if any(s == "fail" for _, s in checks) else "pass",
                "checks": [{"tier": "gameplay", "name": n, "status": s, "detail": ""} for n, s in checks]}

    def test_deficiency_failing_on_reference_passes_the_gate(self):
        result = self.result(("trace.ray-hit", "pass"), ("gyro.free-energy-not-gained", "fail"))
        self.assertEqual([], physics.judge_reference(result, self.DEFICIENT))

    def test_deficiency_passing_on_reference_fails_the_gate(self):
        result = self.result(("trace.ray-hit", "pass"), ("gyro.free-energy-not-gained", "pass"))
        failures = physics.judge_reference(result, self.DEFICIENT)
        self.assertEqual(1, len(failures))
        self.assertIn("not reproduced", failures[0])

    def test_missing_deficiency_fails_the_gate(self):
        failures = physics.judge_reference(self.result(("trace.ray-hit", "pass")), self.DEFICIENT)
        self.assertIn("not reproduced (missing)", failures[0])

    def test_other_reference_failure_still_fails(self):
        result = self.result(("trace.ray-hit", "fail"), ("gyro.free-energy-not-gained", "fail"))
        failures = physics.judge_reference(result, self.DEFICIENT)
        self.assertEqual(1, len(failures))
        self.assertIn("trace.ray-hit", failures[0])

    def test_crashed_reference_fails(self):
        result = self.result(("gyro.free-energy-not-gained", "fail"))
        result["outcome"] = "crash"
        self.assertIn("crash", physics.judge_reference(result, self.DEFICIENT)[0])

    def test_deficiency_is_not_fault_evidence(self):
        for name in physics.REFERENCE_DEFICIENCIES:
            parsed = self.result((name, "fail"))
            self.assertFalse(physics.fault_detected(parsed, ["gyro."]))

    def test_deficiencies_have_reasons(self):
        for name, reason in physics.REFERENCE_DEFICIENCIES.items():
            self.assertTrue(reason.strip(), name)


class VpkTests(unittest.TestCase):
    def write_vpk(self, directory, entries):
        tree = b""
        payload = b""
        for ext, folder, name, data in entries:
            tree += ext.encode() + b"\0" + folder.encode() + b"\0" + name.encode() + b"\0"
            tree += struct.pack("<IHHIIH", 0, 0, 0x7FFF, len(payload), len(data), 0xFFFF)
            # Close this entry's file list and folder list; the extension list
            # is closed once after all entries.
            tree += b"\0\0"
            payload += data
        tree += b"\0"
        path = Path(directory) / "test_dir.vpk"
        path.write_bytes(struct.pack("<III", 0x55AA1234, 1, len(tree)) + tree + payload)
        return str(path)

    def test_reads_embedded_entries(self):
        with tempfile.TemporaryDirectory() as directory:
            path = self.write_vpk(directory, [("txt", "scripts", "a", b"alpha"), ("phy", "models/props", "b", b"beta")])
            vpk = physics.VpkDirectory(path)
            self.assertEqual(b"alpha", vpk.read("scripts/a.txt"))
            self.assertEqual(b"beta", vpk.read("MODELS/props/b.phy"))
            self.assertIsNone(vpk.read("scripts/missing.txt"))

    def test_rejects_non_vpk(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad_dir.vpk"
            path.write_bytes(b"\0" * 32)
            with self.assertRaises(ValueError):
                physics.VpkDirectory(str(path))


if __name__ == "__main__":
    unittest.main()
