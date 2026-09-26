"""The legacy shader oracle (tools/quality/legacy_shader_oracle.py) on a recorded
native run (Modulate and MotionBlur passes, fixtures/legacy_oracle), and with
seeded defects it must detect. The replay needs the retail shaders in
hl2_misc_dir.vpk; those tests are skipped when it is absent."""

import copy
import json
import shutil
import tempfile
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "quality" / "tests"))

import legacy_shader_conformance as conformance  # noqa: E402
import legacy_shader_oracle as oracle  # noqa: E402
import source_vcs  # noqa: E402

FIXTURES = Path(__file__).resolve().parent / "fixtures" / "legacy_oracle"
VPK = source_vcs.find_default_vpk()


def load():
    report = json.loads((FIXTURES / "pixels.json").read_text())
    passes = oracle.load_capture(FIXTURES / "capture.jsonl")
    return report, passes


@unittest.skipIf(VPK is None, "no VPK with shaders/fxc")
class ReplayTest(unittest.TestCase):
    def setUp(self):
        self.report, self.passes = load()
        self.source = oracle.ShaderSource(str(VPK))

    def evaluate(self, report=None, passes=None, tolerance=2):
        return oracle.evaluate(report or self.report, self.passes if passes is None else passes,
                               self.source, tolerance)

    def test_recorded_run_matches(self):
        failures, details = self.evaluate()
        self.assertEqual(failures, [])
        self.assertEqual([d["name"] for d in details],
                         ["modulate", "modulate_mod2x_vertexcolor", "motion_blur_vector",
                          "motion_blur_still"])
        self.assertTrue(all(d["passes"] == 1 for d in details))
        self.assertLessEqual(max(p["error"] for d in details for p in d["pixels"]), 1)

    def test_wrong_native_pixel_is_detected(self):
        report = copy.deepcopy(self.report)
        pixel = report["cases"][2]["pixels"][1]
        pixel["rgba"][1] = (pixel["rgba"][1] + 6) % 256
        failures, _ = self.evaluate(report)
        self.assertEqual(len(failures), 1)
        self.assertIn("motion_blur_vector: pixel (%d, %d)" % (pixel["x"], pixel["y"]), failures[0])

    def test_error_at_the_tolerance_passes_and_beyond_fails(self):
        report = copy.deepcopy(self.report)
        pixel = report["cases"][0]["pixels"][0]
        pixel["rgba"][0] += 3
        self.assertEqual(self.evaluate(report, tolerance=4)[0], [])
        self.assertEqual(len(self.evaluate(report, tolerance=2)[0]), 1)

    def test_declined_pass_fails(self):
        passes = [p for p in self.passes if p["material"] != "conformance/legacy/modulate"]
        failures, _ = self.evaluate(passes=passes)
        self.assertEqual(failures, ["modulate: no legacy port drew the material (declined or "
                                    "another path)"])

    def test_wrong_combo_is_detected(self):
        # The still case recorded QUALITY 0; replaying QUALITY 1's bytecode (7
        # taps along a zero vector) is identical, so seed a blur vector too.
        passes = copy.deepcopy(self.passes)
        still = next(p for p in passes if p["material"].endswith("motion_blur_still"))
        still["ps_constants"][1] = [0.03, -0.02, 0.5, 0.2]
        still["ps_dynamic"] = 1
        failures, _ = self.evaluate(passes=passes)
        self.assertTrue(any(f.startswith("motion_blur_still: pixel") for f in failures))

    def test_wrong_register_is_detected(self):
        passes = copy.deepcopy(self.passes)
        modulate = next(p for p in passes if p["material"] == "conformance/legacy/modulate")
        modulate["ps_constants"][0] = [0.9, 0.9, 0.9, 1.0]  # g_WhiteGrayMix
        failures, _ = self.evaluate(passes=passes)
        self.assertTrue(any(f.startswith("modulate: pixel") for f in failures))

    def test_undefined_texture_fails_the_case(self):
        passes = copy.deepcopy(self.passes)
        passes[0]["textures"]["0"] = "conformance/legacy/not_defined"
        failures, _ = self.evaluate(passes=passes)
        self.assertEqual(len(failures), 1)
        self.assertIn("not_defined", failures[0])


def wine_fxc_available():
    try:
        oracle.shader_artifacts.load_compiler_profile()
    except (OSError, ValueError, KeyError):
        return False
    return shutil.which("wine") is not None and shutil.which("perl") is not None


@unittest.skipUnless(wine_fxc_available(), "no pinned FXC under Wine")
class SourceMatchedTest(unittest.TestCase):
    """The default reference: the recorded passes replayed on bytecode compiled
    from this tree's .fxc for exactly their combos."""

    def test_recorded_run_matches_source_matched_bytecode(self):
        report, passes = load()
        cache = Path(tempfile.mkdtemp(prefix="legacy-oracle-fxc-"))
        try:
            source = oracle.SourceMatchedShaders(cache)
            failures, details = oracle.evaluate(report, passes, source, 2)
            self.assertEqual(failures, [])
            self.assertEqual(len(details), 4)
            code = source.bytecode("modulate_ps20b", 0, 1)
            self.assertEqual(code[:4], bytes((0x01, 0x02, 0xFF, 0xFF)))  # ps_2_x
            with self.assertRaises(oracle.OracleError):
                source.bytecode("modulate_ps20b", 1, 0)  # not a static base
        finally:
            shutil.rmtree(cache, ignore_errors=True)


class InputTest(unittest.TestCase):
    def test_readback_self_check(self):
        report, passes = load()
        report["clear_probe"]["pixel"] = [0, 0, 0]
        failures, _ = oracle.evaluate(report, passes, oracle.ShaderSource("unused"))
        self.assertTrue(failures[0].startswith("readback self-check"))

    def test_other_family_is_rejected(self):
        report, passes = load()
        report["family"] = "post"
        failures, _ = oracle.evaluate(report, passes, oracle.ShaderSource("unused"))
        self.assertEqual(failures, ["pixels hold family 'post', not legacy"])

    def test_no_cases_fails(self):
        report, passes = load()
        report["cases"] = []
        failures, _ = oracle.evaluate(report, passes, oracle.ShaderSource("unused"))
        self.assertEqual(failures, ["no cases"])

    def test_capture_schema_is_checked(self):
        path = FIXTURES / "capture.jsonl"
        lines = path.read_text().splitlines()
        record = json.loads(lines[0])
        record["schema"] = "something-else"
        bad = Path(self.id().replace(".", "_") + ".jsonl")
        try:
            bad.write_text(json.dumps(record) + "\n")
            with self.assertRaises(oracle.OracleError):
                oracle.load_capture(bad)
        finally:
            bad.unlink(missing_ok=True)


class DerivativeTest(unittest.TestCase):
    """dsx/dsy as a pixel quad computes them: the value an instruction reads at
    the next pixel in x (y) minus its value here, over the triangle's plane."""

    def test_quad_derivatives_of_interpolants(self):
        import test_d3d9_shader_vm as vm_tests
        replay = oracle.PassReplay.__new__(oracle.PassReplay)
        replay.ps = vm_tests.assemble("""ps_3_0
            dcl_texcoord v0
            dsx r0, v0
            dsy r1, v0
            mul r1, r1, c0.x
            add oC0, r0, r1""")
        replay.ps_consts = {"c0": (10.0, 0.0, 0.0, 0.0)}
        # u rises 0.5 per pixel in x, v 0.25 per pixel in y (screen 4x4 triangle).
        outputs = [{"texcoord0": (0.0, 0.0, 0.0, 0.0)}, {"texcoord0": (2.0, 0.0, 0.0, 0.0)},
                   {"texcoord0": (0.0, 1.0, 0.0, 0.0)}]
        tri = ((0.0, 0.0, 0.5, 1.0), (4.0, 0.0, 0.5, 1.0), (0.0, 4.0, 0.5, 1.0))
        point = (1, 1)
        inputs = replay.inputs_at(outputs, tri, point)
        derivatives = replay.derivatives(outputs, tri, point, d3d9_texture_samplers())
        result = oracle.d3d9_shader_vm.run_pixel(replay.ps, inputs, replay.ps_consts, None,
                                                 derivatives)
        # dsx = ( 0.5, 0 ), dsy * 10 = ( 0, 2.5 ).
        self.assertEqual([round(v, 5) for v in result["oC0"][:2]], [0.5, 2.5])

    def test_helper_pixel_extrapolates_the_plane(self):
        tri = ((0.0, 0.0, 0.5, 1.0), (4.0, 0.0, 0.5, 1.0), (0.0, 4.0, 0.5, 1.0))
        self.assertIsNone(oracle.cover((5, 5), tri, oracle.VK_CULL_NONE))
        weights = oracle.cover((5, 5), tri, oracle.VK_CULL_NONE, extrapolate=True)
        self.assertAlmostEqual(sum(weights), 1.0)


def d3d9_texture_samplers():
    return oracle.d3d9_texture.SamplerSet({})


class MergeTest(unittest.TestCase):
    def test_case_files_merge_into_one_root(self):
        text = conformance.merge_case_files(sorted(conformance.CASE_DIRECTORY.glob("*.vdf")))
        self.assertTrue(text.startswith("\"LegacyShaderCases\"\n{"))
        self.assertEqual(text.count("\"LegacyShaderCases\""), 1)
        self.assertGreater(text.count("\"case\""), 0)


if __name__ == "__main__":
    unittest.main()
