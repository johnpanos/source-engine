"""Oracle fixtures for per-material pixel conformance, including seeded defects."""

import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


QUALITY = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("material_pixel_conformance",
                                              QUALITY / "material_pixel_conformance.py")
oracle = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(oracle)

REFERENCES = QUALITY.parents[1] / "quality" / "fixtures" / "material-pixels"


def capture(hdr, family="lightmap"):
    """The versioned D3D9 reference capture for an HDR mode."""
    return json.loads((REFERENCES / ("%s-dx9-%s.json" % (family, hdr))).read_text())


def exposure(hdr="integer"):
    return capture(hdr, "exposure")


def with_counts(report, counts):
    changed = copy.deepcopy(report)
    for bar, count in zip(changed["ranges"], counts):
        bar["pixels"] = count
    return changed


def with_pixels(report, name, pixels):
    changed = copy.deepcopy(report)
    for case in changed["cases"]:
        if case["name"] == name:
            case["pixels"] = pixels
    return changed


class ReferenceCaptureTest(unittest.TestCase):
    def test_d3d9_references_satisfy_their_own_oracle(self):
        for hdr in ("none", "integer"):
            report = capture(hdr)
            self.assertEqual(report["renderer"], "vulkan-compat")
            self.assertEqual(oracle.evaluate(report, hdr, report), [], hdr)

    def test_closed_form_matches_d3d9_integer_hdr(self):
        self.assertEqual(oracle.check_model(capture("integer")), [])

    def test_closed_form_is_not_claimed_for_ldr(self):
        # 8-bit LDR lightmaps quantize the dark end; LDR is held to the reference.
        self.assertNotEqual(oracle.check_model(capture("none")), [])


class SeededDefectTest(unittest.TestCase):
    def test_readback_that_returns_zeros_invalidates_the_run(self):
        report = copy.deepcopy(capture("none"))
        report["clear_probe"]["pixel"] = [0, 0, 0]
        failures = oracle.evaluate(report, "none")
        self.assertEqual(len(failures), 1)
        self.assertIn("readback", failures[0])

    def test_ignored_lightmap_is_detected(self):
        # The native path before lightmaps: every pixel is the base texture.
        report = copy.deepcopy(capture("none"))
        for case in report["cases"]:
            case["pixels"] = [list(case["base"]), list(case["base"])]
        failures = oracle.evaluate(report, "none")
        self.assertTrue(any("black lightmap" in failure for failure in failures))
        self.assertTrue(any("ramp" in failure for failure in failures))

    def test_swapped_red_and_blue_are_detected(self):
        report = copy.deepcopy(capture("none"))
        for case in report["cases"]:
            case["pixels"] = [[p[2], p[1], p[0]] for p in case["pixels"]]
        failures = oracle.evaluate(report, "none", capture("none"))
        self.assertTrue(any("red-only" in failure for failure in failures))
        self.assertTrue(any("base_color" in failure for failure in failures))

    def test_upside_down_frame_is_detected(self):
        # The native path before its viewport flip: Vulkan clip-space Y points down.
        report = copy.deepcopy(capture("none"))
        report["orientation"]["pixels"].reverse()
        failures = oracle.evaluate(report, "none")
        self.assertEqual(len(failures), 1)
        self.assertIn("upside down", failures[0])

    def test_missing_orientation_or_tone_capture_is_rejected(self):
        for capture_name in ("orientation", "tone_scale"):
            report = copy.deepcopy(capture("none"))
            del report[capture_name]
            with tempfile.TemporaryDirectory() as temp:
                path = Path(temp) / "pixels.json"
                path.write_text(json.dumps(report))
                with self.assertRaises(oracle.PixelsError):
                    oracle.read_pixels(path)

    def test_ignored_tone_scale_in_integer_hdr_is_detected(self):
        # Integer HDR without LINEAR_LIGHT_SCALE: the scaled draw equals the unscaled one.
        report = copy.deepcopy(capture("integer"))
        report["tone_scale"]["pixels"] = copy.deepcopy(report["cases"][2]["pixels"])
        failures = oracle.evaluate(report, "integer")
        self.assertTrue(failures and all(f.startswith("tone scale") for f in failures))

    def test_tone_scale_applied_without_hdr_is_detected(self):
        # D3D9 forces the scale to 1 without HDR.
        report = copy.deepcopy(capture("none"))
        report["tone_scale"]["pixels"] = [[188, 188, 188], [255, 255, 255]]
        self.assertTrue(any(f.startswith("tone scale") for f in oracle.evaluate(report, "none")))

    def test_non_monotonic_ramp_is_detected(self):
        report = with_pixels(capture("none"), "ramp_mid", [[60, 60, 60], [188, 188, 188]])
        self.assertTrue(any("strictly brighter" in failure
                            for failure in oracle.evaluate(report, "none")))

    def test_linear_output_is_caught_by_the_closed_form(self):
        # A backend that skips the sRGB encode writes linear light.
        report = with_pixels(capture("integer"), "ramp_mid", [[64, 64, 64], [128, 128, 128]])
        self.assertTrue(any(failure.startswith("ramp_mid")
                            for failure in oracle.check_model(report)))

    def test_unsupported_hdr_mode_fails_explicitly(self):
        report = copy.deepcopy(capture("none"))
        failures = oracle.evaluate(report, "integer", capture("integer"))
        self.assertEqual(failures, ["backend does not support HDR mode 'integer' "
                                    "(it reports HDR type 0)"])

    def test_reference_tolerance(self):
        reference = capture("none")
        near = with_pixels(reference, "ramp_mid", [[143, 141, 139], [188, 188, 188]])
        self.assertEqual(oracle.compare(near, reference), [])
        far = with_pixels(reference, "ramp_mid", [[150, 141, 141], [188, 188, 188]])
        self.assertEqual(len(oracle.compare(far, reference)), 1)

    def test_changed_inputs_are_not_compared_as_equal(self):
        reference = capture("none")
        report = copy.deepcopy(reference)
        report["cases"][2]["lightmap"][0] = [0.3, 0.3, 0.3]
        self.assertIn("inputs differ", oracle.compare(report, reference)[0])


class ExposureTest(unittest.TestCase):
    def test_d3d9_references_satisfy_their_own_oracle(self):
        for hdr in ("none", "integer"):
            report = exposure(hdr)
            self.assertEqual(report["renderer"], "vulkan-compat")
            self.assertEqual(oracle.evaluate(report, hdr, report), [], hdr)

    def test_d3d9_counts_every_sample_of_a_multisampled_target(self):
        # The reference ran with 4x MSAA: every count is the model times 4, which
        # the all-pixels range measures and the comparison divides out.
        report = exposure()
        self.assertEqual(report["aa_samples"], 4)
        area = report["query_rect"][2] * report["query_rect"][3]
        self.assertEqual(report["ranges"][-1]["pixels"], 4 * area)
        single = with_counts(report, [bar["pixels"] // 4 for bar in report["ranges"]])
        self.assertEqual(oracle.evaluate(single, "integer", report), [])

    def test_queries_without_results_are_incomplete(self):
        # The native backend before occlusion queries: every read stays pending.
        report = with_counts(exposure(), [-1] * oracle.LUMINANCE_RANGES)
        failures = oracle.evaluate(report, "integer", exposure())
        self.assertEqual(len(failures), 1)
        self.assertIn("no result", failures[0])

    def test_texel_edge_sampling_is_detected(self):
        # The native backend before the D3D9 half-pixel offset: screen-space
        # samples land on texel edges and blend regions into ranges they lack.
        # Measured headless (256x256) with the offset removed.
        counts = [5777, 0, 12375, 173, 160, 13831, 1, 109, 171, 0, 5559, 0, 6804, 0, 0, 5616,
                  50576]
        failures = oracle.evaluate(with_counts(exposure(), counts), "integer", exposure())
        self.assertTrue(any(f.startswith("luminance range 3 ") for f in failures))
        self.assertTrue(any("reference" in f for f in failures))

    def test_greater_equal_alpha_test_counts_everything(self):
        # dev/lumcompare's alpha test is GREATER 0; GREATEREQUAL keeps every pixel.
        report = exposure()
        everything = report["ranges"][-1]["pixels"]
        failures = oracle.check_exposure(with_counts(report, [everything] * len(report["ranges"])))
        self.assertTrue(failures)

    def test_color_writes_are_detected(self):
        report = copy.deepcopy(exposure())
        for region in report["regions"]:
            region["pixel"] = [255, 255, 255]
        failures = oracle.check_exposure(report)
        self.assertTrue(failures and all("must not write color" in f for f in failures))

    def test_ignored_srgb_read_is_detected(self):
        # Luminance of the gamma-encoded values instead of linear light.
        report = exposure()
        unit = report["ranges"][-1]["pixels"] // (report["query_rect"][2] * report["query_rect"][3])
        counts = []
        for bar in report["ranges"]:
            count = 0
            for region in report["regions"]:
                gamma = sum(w * c / 255.0 for w, c in zip(oracle.LUMINANCE_WEIGHTS, region["color"]))
                if bar["min"] <= gamma <= bar["max"]:
                    count += oracle._overlap(region["rect"], report["query_rect"])
            counts.append(count * unit)
        self.assertNotEqual(oracle.check_exposure(with_counts(report, counts)), [])

    def test_counts_that_are_not_whole_samples_are_rejected(self):
        report = exposure()
        counts = [bar["pixels"] for bar in report["ranges"]]
        counts[-1] += 1
        failures = oracle.check_exposure(with_counts(report, counts))
        self.assertEqual(len(failures), 1)
        self.assertIn("whole multiple", failures[0])

    def test_different_frame_size_is_not_compared(self):
        report = copy.deepcopy(exposure())
        report["frame"] = [384, 384]
        self.assertIn("differ from the reference", oracle.compare_exposure(report, exposure())[0])

    def test_family_mismatch_is_rejected(self):
        failures = oracle.evaluate(exposure(), "integer", capture("integer"))
        self.assertEqual(failures, ["reference holds family 'lightmap', capture 'exposure'"])

    def test_renamed_regions_or_missing_ranges_are_rejected(self):
        for mutate in (lambda r: r["regions"].pop(), lambda r: r["ranges"].pop()):
            report = copy.deepcopy(exposure())
            mutate(report)
            with tempfile.TemporaryDirectory() as temp:
                path = Path(temp) / "pixels.json"
                path.write_text(json.dumps(report))
                with self.assertRaises(oracle.PixelsError):
                    oracle.read_pixels(path)


class CaptureFormatTest(unittest.TestCase):
    def test_missing_or_renamed_cases_are_rejected(self):
        report = copy.deepcopy(capture("none"))
        report["cases"].pop()
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "pixels.json"
            path.write_text(json.dumps(report))
            with self.assertRaises(oracle.PixelsError):
                oracle.read_pixels(path)

    def test_exit_codes(self):
        with tempfile.TemporaryDirectory() as temp:
            good = Path(temp) / "good.json"
            good.write_text(json.dumps(capture("none")))
            bad = Path(temp) / "bad.json"
            bad.write_text(json.dumps(with_pixels(capture("none"), "black_lightmap",
                                                  [[40, 40, 40], [40, 40, 40]])))
            invalid = Path(temp) / "invalid.json"
            invalid.write_text("{}")
            reference = str(REFERENCES / "lightmap-dx9-none.json")
            self.assertEqual(oracle.main(["check", "--pixels", str(good), "--hdr", "none",
                                          "--reference", reference]), 0)
            self.assertEqual(oracle.main(["check", "--pixels", str(bad), "--hdr", "none"]), 1)
            self.assertEqual(oracle.main(["check", "--pixels", str(invalid), "--hdr", "none"]), 2)


if __name__ == "__main__":
    unittest.main()
