"""Compiler statistics are static and incomplete dumps must fail."""
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import radv_shader_stats
import render_profile


class RadvShaderStatsTest(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.path = Path(directory.name) / "driver.log"
        values = {name: 0 for name in radv_shader_stats.REQUIRED}
        values.update({"Driver pipeline hash": 100, "Hash": 200, "VGPRs": 192,
                       "Instructions": 10400, "Code size": 56000, "Subgroups per SIMD": 8})
        self.text = ("ordinary log\nPixel Shader:\n*** SHADER STATS ***\n" +
                     "".join("%s: %s\n" % item for item in values.items()) +
                     "********************\n")
        self.path.write_text(self.text)

    def test_complete_dump_preserves_static_counts_and_units(self):
        report = radv_shader_stats.analyze(self.path)
        self.assertEqual(report["shaders"][0]["VGPRs"], 192)
        self.assertIn("not dynamic occupancy", report["semantics"])
        self.assertIn("scratch_bytes", radv_shader_stats.tsv(report))
        self.assertNotIn("mean_ms", radv_shader_stats.tsv(report))

    def test_repeated_compilations_are_not_draw_counts(self):
        self.path.write_text(self.text * 2)
        self.assertEqual(len(radv_shader_stats.analyze(self.path)["shaders"]), 2)

    def test_truncated_empty_or_incomplete_dump_fails(self):
        for text in ("ordinary log\n", self.text.split("********************")[0],
                     self.text.replace("VGPRs: 192\n", "")):
            self.path.write_text(text)
            with self.assertRaises(render_profile.ProfileError):
                radv_shader_stats.analyze(self.path)

    def test_duplicate_negative_or_unnamed_statistics_fail(self):
        for text in (self.text.replace("VGPRs: 192", "VGPRs: 192\nVGPRs: 192"),
                     self.text.replace("VGPRs: 192", "VGPRs: -1"),
                     self.text.replace("Pixel Shader:\n", "")):
            self.path.write_text(text)
            with self.assertRaises(render_profile.ProfileError):
                radv_shader_stats.analyze(self.path)


if __name__ == "__main__":
    unittest.main()
