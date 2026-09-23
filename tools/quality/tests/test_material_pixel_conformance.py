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


class SkinningTest(unittest.TestCase):
    CLEAR = [255, 0, 255]

    def skinning(self, hdr="none"):
        return capture(hdr, "skinning")

    def test_d3d9_references_satisfy_their_own_oracle(self):
        for hdr in ("none", "integer"):
            report = self.skinning(hdr)
            self.assertEqual(report["renderer"], "vulkan-compat")
            self.assertEqual(oracle.evaluate(report, hdr, report), [], hdr)

    def test_every_model_at_the_origin_is_detected(self):
        # The native backend before bone matrices: LoadBoneMatrix was a no-op, so
        # every model drew at the model-space origin, the middle of the frame.
        report = copy.deepcopy(self.skinning())
        color = report["color"]
        for case in report["cases"]:
            if case["name"] != "rigid_back_facing":
                case["pixels"] = [self.CLEAR, color, self.CLEAR]
        failures = oracle.evaluate(report, "none", self.skinning())
        for name in ("rigid_bone0", "one_bone", "high_index"):
            self.assertTrue(any(f.startswith(name + ": the model is not in") for f in failures))
        self.assertFalse(any(f.startswith("implicit_third_weight") for f in failures))

    def test_skinning_ignored_draws_at_the_decoy(self):
        # Drawing skinned vertices with bone 0 only (the MODEL matrix) puts them at
        # the decoy placement in the right third.
        report = copy.deepcopy(self.skinning())
        for case in report["cases"][1:5]:
            case["pixels"] = [self.CLEAR, self.CLEAR, report["color"]]
        failures = oracle.check_skinning(report)
        self.assertTrue(any(f.startswith("one_bone") for f in failures))
        self.assertTrue(any(f.startswith("blend_half") for f in failures))

    def test_unculled_back_face_is_detected(self):
        report = copy.deepcopy(self.skinning())
        report["cases"][-1]["pixels"][0] = report["color"]
        failures = oracle.check_skinning(report)
        self.assertEqual(len(failures), 1)
        self.assertIn("back face must be culled", failures[0])

    def test_missing_or_renamed_cases_are_rejected(self):
        report = copy.deepcopy(self.skinning())
        report["cases"].pop()
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "pixels.json"
            path.write_text(json.dumps(report))
            with self.assertRaises(oracle.PixelsError):
                oracle.read_pixels(path)


class PortalTest(unittest.TestCase):
    """Stencil portal recursion (the portal family): the ray-traced scene model
    judges the structure, the D3D9 reference judges PortalRefract's pixels."""

    @classmethod
    def setUpClass(cls):
        cls.reference = capture("none", "portal")
        cls.portal = oracle.material_pixel_portal

    def report(self):
        return copy.deepcopy(self.reference)

    def repaint(self, report, name, paint):
        """Rewrites case `name`'s frame; paint(x, y, rgb) returns the new pixel
        or None to keep it."""
        case = next(c for c in report["cases"] if c["name"] == name)
        width, height = report["frame"]
        rgb = bytearray(self.portal.decode_frame(case, report["frame"]))
        for y in range(height):
            for x in range(width):
                index = (y * width + x) * 3
                pixel = paint(x, y, list(rgb[index:index + 3]))
                if pixel is not None:
                    rgb[index:index + 3] = bytes(pixel)
        case["frame"] = self.portal.encode_frame(rgb)
        return report

    def recursion_model(self, report, **scene):
        changed = copy.deepcopy(report)
        changed["scene"].update(scene)
        case = next(c for c in changed["cases"] if c["name"] == "recursion")
        return self.portal.SceneModel(changed, case)

    def test_d3d9_reference_satisfies_its_own_oracle(self):
        report = self.report()
        self.assertEqual(report["renderer"], "vulkan-compat")
        self.assertEqual(report["max_depth"], self.portal.PORTAL_STENCIL_DEPTH)
        self.assertEqual(oracle.evaluate(report, "none", self.reference), [])

    def test_backend_without_stencil_is_rejected(self):
        report = self.report()
        report["stencil_bits"] = 0
        failures = oracle.evaluate(report, "none", self.reference)
        self.assertEqual(len(failures), 1)
        self.assertIn("no stencil bits", failures[0])

    def test_shallow_recursion_is_rejected(self):
        report = self.report()
        report["max_depth"] = 1
        self.assertTrue(any("recursion depth 1" in f
                            for f in oracle.evaluate(report, "none", self.reference)))

    def test_unpinned_sampling_is_rejected(self):
        # D3D9's dxsupport defaults (4x MSAA, trilinear) differ from a backend
        # that reads no dxsupport.cfg; such frames are not comparable.
        for key, value in (("aa_samples", 4), ("force_trilinear", False),
                           ("force_anisotropy", 8)):
            report = self.report()
            report[key] = value
            failures = oracle.evaluate(report, "none", self.reference)
            self.assertTrue(any("not pinned" in f and key in f for f in failures), key)

    def test_ignored_recursion_is_detected(self):
        # Nested views drawn without their camera step show the outer level.
        report = self.report()
        model = self.recursion_model(report)
        level0 = model.color((0, "wall"))

        def flatten(x, y, rgb):
            label = model.label(x, y)
            return level0 if label is not None and label[0] > 0 else None
        failures = self.portal.check_model(self.repaint(report, "recursion", flatten))
        self.assertTrue(any(f.startswith("recursion:") and "disagree with the scene model" in f
                            for f in failures))

    def test_ignored_clip_plane_is_detected(self):
        # The exit portal's clip plane (DXVK's fast-linked pipelines drop it):
        # the blocker behind the exit portal shows in the first nested view.
        report = self.report()
        clipped = self.recursion_model(report)
        unclipped = self.recursion_model(report, exit_portal_distance=-1.0e9)
        color = clipped.color((1, "blocker"))
        seeded = []

        def unclip(x, y, rgb):
            label = clipped.label(x, y)
            if label not in (None, (1, "blocker")) and unclipped.label(x, y) == (1, "blocker"):
                seeded.append((x, y))
                return color
            return None
        failures = self.portal.check_model(self.repaint(report, "recursion", unclip))
        self.assertTrue(seeded)
        self.assertTrue(any(f.startswith("recursion: %d pixels disagree" % len(seeded))
                            for f in failures), failures)

    def test_reference_drift_is_detected(self):
        # Flames a few levels off (the DXT software-decode rounding the native
        # backend had) over more pixels than the allowance.
        report = self.repaint(self.report(), "static",
                              lambda x, y, rgb: [min(255, c + 2) for c in rgb]
                              if 100 <= x < 110 and 100 <= y < 110 else None)
        failures = self.portal.compare(report, self.reference)
        self.assertEqual(len(failures), 1)
        self.assertTrue(failures[0].startswith("static: 100 pixels differ"))

    def test_reference_tolerance(self):
        within = self.repaint(self.report(), "opening",
                              lambda x, y, rgb: [max(0, c - 1) for c in rgb])
        self.assertEqual(self.portal.compare(within, self.reference), [])
        few = self.repaint(self.report(), "opening",
                           lambda x, y, rgb: [0, 0, 0] if y == 0 and x < 8 else None)
        self.assertEqual(self.portal.compare(few, self.reference), [])

    def test_changed_scene_is_not_compared(self):
        report = self.report()
        report["scene"]["time"] += 1.0
        self.assertEqual(self.portal.compare(report, self.reference),
                         ["portal scene or case inputs differ from the reference capture"])

    def test_missing_cases_or_frames_are_rejected(self):
        truncated = self.report()
        truncated["cases"].pop()
        missing = self.report()
        del missing["cases"][0]["frame"]
        short = self.report()
        short["cases"][1]["frame"] = self.portal.encode_frame(b"\0" * 12)
        for report in (truncated, missing, short):
            with tempfile.TemporaryDirectory() as temp:
                path = Path(temp) / "pixels.json"
                path.write_text(json.dumps(report))
                with self.assertRaises(oracle.PixelsError):
                    oracle.read_pixels(path)


class ModelLightTest(unittest.TestCase):
    """VertexLitGeneric model lighting (the modellight family): an independent
    evaluation of common_vs_fxc.h DoLighting judges every lit pixel, and the D3D9
    reference judges the whole frame."""

    @classmethod
    def setUpClass(cls):
        cls.references = {hdr: capture(hdr, "modellight") for hdr in ("none", "integer")}
        cls.light = oracle.material_pixel_modellight

    def only(self, name, hdr="none"):
        """The capture reduced to one case, so a seeded defect is judged alone."""
        report = copy.deepcopy(self.references[hdr])
        report["cases"] = [c for c in report["cases"] if c["name"] == name]
        return report

    def disagreements(self, report):
        return [f for f in self.light.check_model(report) if "disagree" in f]

    def test_d3d9_references_satisfy_their_own_oracle(self):
        for hdr, report in self.references.items():
            self.assertEqual(report["renderer"], "vulkan-compat")
            self.assertEqual(oracle.evaluate(report, hdr, report), [], hdr)

    def test_unlit_models_are_detected(self):
        # The native backend before model lighting: every quad showed the base
        # texture times the modulation.
        report = self.only("four_lights")
        case = report["cases"][0]
        width, height = report["frame"]
        model = self.light.CaseModel(report, case)
        rgb = bytearray(self.light.material_pixel_frames.decode_frame(case, report["frame"]))
        for y in range(height):
            for x in range(width):
                if model.expected(x, y) not in (None, self.light.CLEAR):
                    rgb[(y * width + x) * 3:(y * width + x) * 3 + 3] = bytes(report["base_color"])
        case["frame"] = self.light.material_pixel_frames.encode_frame(rgb)
        self.assertTrue(self.disagreements(report))

    def test_ambient_cube_faces_are_distinguished(self):
        # A backend that picks the wrong face for negative normal components.
        report = self.only("ambient_cube")
        cube = report["cases"][0]["cube"]
        cube[0], cube[1] = cube[1], cube[0]
        self.assertTrue(self.disagreements(report))

    def test_each_light_counts(self):
        report = self.only("four_lights")
        report["cases"][0]["lights"].pop()
        self.assertTrue(self.disagreements(report))

    def test_spot_cone_is_measured(self):
        report = self.only("spot")
        report["cases"][0]["lights"][0]["falloff"] = 1.0
        self.assertTrue(self.disagreements(report))

    def test_half_lambert_is_measured(self):
        report = self.only("half_lambert")
        report["cases"][0]["half_lambert"] = False
        self.assertTrue(self.disagreements(report))

    def test_static_color_mesh_is_measured(self):
        report = self.only("static_vertex")
        report["cases"][0]["static_color"] = False
        self.assertTrue(self.disagreements(report))

    def test_model_and_skinned_placement_turn_the_normals(self):
        identity = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0]]
        for name in ("model_transform", "skinned"):
            report = self.only(name)
            report["cases"][0]["model_matrix"] = identity
            self.assertTrue(self.disagreements(report), name)

    def test_ignored_tone_scale_is_detected(self):
        report = self.only("directional", "integer")
        report["tone_scale"] = 1.0
        self.assertTrue(self.disagreements(report))

    def test_too_few_lights_is_rejected(self):
        report = copy.deepcopy(self.references["none"])
        report["max_lights"] = 2
        failures = oracle.evaluate(report, "none", self.references["none"])
        self.assertEqual(len(failures), 1)
        self.assertIn("local lights", failures[0])

    def test_reference_tolerance_and_drift(self):
        reference = self.only("point")
        within = copy.deepcopy(reference)
        case = within["cases"][0]
        frames = self.light.material_pixel_frames
        rgb = frames.decode_frame(case, within["frame"])
        case["frame"] = frames.encode_frame(bytes(max(0, c - 1) for c in rgb))
        self.assertEqual(self.light.compare(within, reference), [])
        drift = copy.deepcopy(reference)
        rgb = bytearray(rgb)
        rgb[0:3] = bytes(c ^ 0x10 for c in rgb[0:3])
        drift["cases"][0]["frame"] = frames.encode_frame(rgb)
        failures = self.light.compare(drift, reference)
        self.assertEqual(len(failures), 1)
        self.assertTrue(failures[0].startswith("point: 1 pixels differ"))

    def test_changed_inputs_are_not_compared(self):
        reference = self.only("point")
        changed = copy.deepcopy(reference)
        changed["cases"][0]["lights"][0]["color"][0] += 0.5
        self.assertEqual(self.light.compare(changed, reference),
                         ["modellight inputs differ from the reference capture"])

    def test_phong_specular_is_measured(self):
        # A backend drawing $phong models with the vertex-lit shader (no
        # specular): the model's boost 0 predicts that, and D3D9 disagrees.
        report = self.only("phong")
        report["materials"][report["cases"][0]["material"]]["$phongboost"] = "0"
        self.assertTrue(self.disagreements(report))

    def test_phong_normal_map_is_measured(self):
        report = self.only("phong")
        report["textures"]["conformance/modellight_normal"]["row"][0][:3] = [128, 128, 255]
        self.assertTrue(self.disagreements(report))

    def test_phong_lightwarp_is_measured(self):
        report = self.only("phong_lightwarp")
        del report["materials"][report["cases"][0]["material"]]["$lightwarptexture"]
        self.assertTrue(self.disagreements(report))

    def test_phong_directional_lights_follow_the_lighting_origin(self):
        # CommitPixelShaderLighting places them 10000 units from the origin.
        report = self.only("phong")
        report["lighting_origin"] = [0, 0, 5000]
        self.assertTrue(self.disagreements(report))

    def test_phong_self_illumination_is_measured(self):
        report = self.only("phong_selfillum")
        del report["materials"][report["cases"][0]["material"]]["$selfillum"]
        self.assertTrue(self.disagreements(report))

    def test_missing_cases_are_rejected(self):
        report = copy.deepcopy(self.references["none"])
        report["cases"].pop()
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
