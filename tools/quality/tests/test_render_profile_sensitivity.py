"""Sensitivity of the render.profile suite (RFC 0001 rank 8).

Each case compiles unittests/rendertest/test_render_profile.cpp against a copy of
public/render/render_profile.h with one seeded defect and requires the suite to
report at least one failure. The unmodified header must pass."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]
HEADER = ROOT / "public" / "render" / "render_profile.h"
SUITE = ROOT / "unittests" / "rendertest" / "test_render_profile.cpp"

MUTANTS = {
    "exclusive-device-range": (
        "adapter.deviceId >= quirk.deviceIdMin", "adapter.deviceId > quirk.deviceIdMin"),
    "no-table-validation": ("if ( !IsValidRenderQuirk( quirks[i] ) )", "if ( false )"),
    "removed-features-kept": ("& supported & ~removed", "& supported"),
    "required-not-checked": (
        "const uint32_t missing = request.required.bits & ~supported;",
        "const uint32_t missing = 0;"),
    "quirk-may-remove-required": (
        "const uint32_t removesRequired = quirk.removedFeatureBits & request.required.bits;",
        "const uint32_t removesRequired = 0;"),
    "driver-api-ignored": (
        "profile_detail::SelectorMatches( quirk.driverApi, adapter.driverApi ) &&", ""),
    "output-written-on-failure": (
        "\tconst uint32_t missing", "\t*out = RenderFeatureProfile();\n\tconst uint32_t missing"),
    "capacity-off-by-one": (
        "profile.appliedQuirkCount == kRenderMaxAppliedQuirks",
        "profile.appliedQuirkCount + 1 == kRenderMaxAppliedQuirks"),
}


def run_suite(header_text, scratch):
    include = scratch / "include"
    (include / "render").mkdir(parents=True, exist_ok=True)
    shutil.copy(ROOT / "public" / "render" / "render_backend.h", include / "render")
    (include / "render" / "render_profile.h").write_text(header_text)
    binary = scratch / "suite"
    build = subprocess.run(
        ["c++", "-std=c++20", "-w", "-I", str(include), "-I", str(ROOT / "public"), str(SUITE),
         "-o", str(binary)], capture_output=True, text=True, timeout=120)
    if build.returncode:
        raise AssertionError(build.stderr)
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
    match = re.search(r"^CONFORMANCE (\d+) (\d+)$", run.stdout, re.MULTILINE)
    if not match:
        raise AssertionError("suite printed no CONFORMANCE line:\n" + run.stdout)
    return run.returncode, int(match.group(1)), int(match.group(2))


class RenderProfileSensitivity(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(shutil.which("c++"), "the required native C++ compiler is missing")
        self.header = HEADER.read_text()

    def test_unmodified_header_passes(self):
        with tempfile.TemporaryDirectory(prefix="render-profile-") as scratch:
            code, checks, failures = run_suite(self.header, Path(scratch))
        self.assertEqual((code, failures), (0, 0))
        self.assertGreater(checks, 100)

    def test_every_seeded_defect_is_detected(self):
        for name, (original, replacement) in MUTANTS.items():
            with self.subTest(mutant=name):
                self.assertEqual(self.header.count(original), 1, "mutation site moved")
                mutated = self.header.replace(original, replacement)
                with tempfile.TemporaryDirectory(prefix="render-profile-") as scratch:
                    code, _, failures = run_suite(mutated, Path(scratch))
                self.assertNotEqual(code, 0)
                self.assertGreater(failures, 0)


if __name__ == "__main__":
    unittest.main()
