"""Sensitivity of the VGUI fixture host's oracle (tools/vgui/vgui_fixture_host.py,
RFC 0010 V0). A synthetic capture built from the oracle's own probe table must
pass, and each seeded defect must fail its named check. No host runs here;
the suite vgui.fixture-host runs the real one."""
import json
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / "tools" / "vgui"))
import vgui_fixture_host as host  # noqa: E402

W, H = 640, 480
CLEAR = (0, 0, 64, 255)


def counters(draws, text=0.0, frames=10, glyphs=0.0, uploads=0.0, copies=0.0):
    return {"frames": frames, "paintPasses": 1.0, "paintMilliseconds": 0.1, "draws": draws,
            "textDraws": text, "vertices": draws * 4, "indices": draws * 6, "vertexKiB": 1.0,
            "textureUploads": uploads, "textureUploadKiB": 0.0, "glyphUploads": glyphs,
            "glyphUploadKiB": 0.0, "cpuCopies": copies, "cpuCopyKiB": 0.0}


class Canvas:
    def __init__(self):
        self.pixels = bytearray(bytes(CLEAR) * (W * H))

    def set(self, x, y, rgb):
        i = (y * W + x) * 4
        self.pixels[i:i + 4] = bytes(rgb[:3]) + b"\xff"

    def fill(self, x0, y0, x1, y1, rgb, step=1):
        for y in range(y0, y1):
            for x in range(x0, x1, step):
                self.set(x, y, rgb)


def passing_capture():
    """{name: Canvas}, report, log: a capture the oracle accepts (native-vulkan)."""
    primitives = Canvas()
    for _, (point, rgb, _) in host.expected_probes(CLEAR).items():
        primitives.set(point[0], point[1], rgb)
    primitives.set(261, 52, (252, 252, 253))
    for y in range(100, 164):
        for x in range(500, 564):
            primitives.set(x, y, (255, 255, 255) if (x + y) % 2 == 0 else (0, 0, 0))
    primitives.fill(20, 220, 620, 240, (255, 255, 255), step=10)
    controls = Canvas()
    controls.fill(40, 40, 440, 340, (96, 96, 96))
    for _, (point, rgb) in host.CONTROLS_PROBES.items():
        controls.set(point[0], point[1], rgb)
    text = Canvas()
    text.fill(10, 10, 620, 300, (255, 255, 255), step=5)
    frames = {"primitives": primitives, "controls": controls, "text": text, "empty": Canvas()}
    report = {
        "schema": host.SCHEMA, "renderer": "native-vulkan", "width": W, "height": H,
        "clear": list(CLEAR), "warm_frames": 3, "steady_frames": 10,
        "fonts": {"Default": 19, "DefaultLarge": 33, "DefaultOutline": 24},
        "screens": [
            {"name": "primitives", "frame": "primitives.rgba", "frame_written": True,
             "have_first": True, "have_steady": True,
             "first": counters(24, 3, frames=1, glyphs=51, uploads=52, copies=104),
             "steady": counters(24, 3)},
            {"name": "controls", "frame": "controls.rgba", "frame_written": True,
             "have_first": True, "have_steady": True,
             "first": counters(24, 5, frames=1), "steady": counters(24, 5)},
            {"name": "text", "frame": "text.rgba", "frame_written": True,
             "have_first": True, "have_steady": True,
             "first": counters(14, 14, frames=1, glyphs=222, uploads=223, copies=446),
             "steady": counters(14, 14)},
            {"name": "empty", "frame": "empty.rgba", "frame_written": True,
             "have_first": True, "have_steady": True,
             "first": counters(0, frames=1), "steady": counters(0)},
        ]}
    log = "font fc: %s - /usr/share/fonts/x.ttc\n" % host.FOREIGN_FALLBACK
    return frames, report, log


def write(directory, frames, report, log):
    for name, canvas in frames.items():
        (directory / (name + ".rgba")).write_bytes(bytes(canvas.pixels))
    (directory / "frames.json").write_text(json.dumps(report))
    (directory / "host.log").write_text(log)


def failing(frames, report, log):
    with tempfile.TemporaryDirectory() as work:
        directory = Path(work)
        write(directory, frames, report, log)
        checks, _ = host.evaluate(directory)
    return [label for ok, label in checks if not ok], len(checks)


class FixtureHostOracleTest(unittest.TestCase):
    def test_passing_capture_passes(self):
        failures, count = failing(*passing_capture())
        self.assertEqual(failures, [])
        self.assertGreater(count, 60)

    def assertFails(self, needle, frames, report, log):
        failures, _ = failing(frames, report, log)
        self.assertTrue(any(needle in label for label in failures),
                        "expected a failure naming %r, got %s" % (needle, failures))

    def test_color_model_matches_the_measured_native_values(self):
        # The closed forms reproduce what native Vulkan drew (2026-10-03 run).
        probes = host.expected_probes(CLEAR)
        self.assertEqual(probes["filled rectangle"][1], (201, 35, 35))
        self.assertEqual(probes["additive"][1], (128, 64, 73))
        self.assertTrue(host.close((23, 147, 52), probes["alpha rectangle"][1], host.PIXEL_TOLERANCE))
        self.assertTrue(host.close((188, 188, 192), probes["alpha ramp column 128"][1],
                                   host.PIXEL_TOLERANCE))

    def test_flipped_texture_fails(self):
        frames, report, log = passing_capture()
        frames["primitives"].set(36, 116, (0, 0, 255))
        self.assertFails("quadrants top-left", frames, report, log)

    def test_gamma_space_blending_fails(self):
        frames, report, log = passing_capture()
        frames["primitives"].set(132, 52, (20, 100, 32))  # blended in gamma, not linear light
        self.assertFails("alpha rectangle", frames, report, log)

    def test_a_texture_whose_alpha_is_read_under_opaque_fails(self):
        frames, report, log = passing_capture()
        frames["primitives"].set(372, 132, CLEAR)
        self.assertFails("opaque ignores texture alpha", frames, report, log)

    def test_a_closed_known_gap_fails_until_the_list_is_updated(self):
        frames, report, log = passing_capture()
        frames["primitives"].set(500, 188, (255, 255, 0))
        self.assertFails("line: known gap still present", frames, report, log)

    def test_a_renderer_without_the_gap_must_draw_lines(self):
        frames, report, log = passing_capture()
        report["renderer"] = "dx9"
        self.assertFails("line is drawn", frames, report, log)

    def test_unread_frames_fail_the_clear_check(self):
        frames, report, log = passing_capture()
        frames["empty"].set(10, 10, (1, 0, 64))
        self.assertFails("every pixel is the clear color", frames, report, log)

    def test_uploads_after_warm_up_fail(self):
        frames, report, log = passing_capture()
        report["screens"][2]["steady"]["glyphUploads"] = 0.1
        self.assertFails("text: no glyphUploads after warm-up", frames, report, log)

    def test_uncounted_text_draws_fail(self):
        frames, report, log = passing_capture()
        report["screens"][2]["steady"]["textDraws"] = 0.0
        self.assertFails("text: every draw is a text draw", frames, report, log)

    def test_unstable_draw_counts_fail(self):
        frames, report, log = passing_capture()
        report["screens"][0]["steady"]["draws"] = 24.3
        self.assertFails("every steady frame submits the same", frames, report, log)

    def test_a_substituted_system_font_fails(self):
        frames, report, log = passing_capture()
        self.assertFails("fontconfig was asked only for the foreign fallback", frames, report,
                         log + "font fc: DejaVu Sans - /usr/share/fonts/DejaVuSans.ttf\n")

    def test_a_missing_material_fails(self):
        frames, report, log = passing_capture()
        self.assertFails("no VGUI material is missing", frames, report,
                         log + "--- Missing Vgui material vgui/hud/800corner1\n")

    def test_a_missing_screen_fails(self):
        frames, report, log = passing_capture()
        report["screens"] = report["screens"][:3]
        self.assertFails("the capture has the screens", frames, report, log)

    def test_a_short_frame_fails(self):
        frames, report, log = passing_capture()
        frames["controls"].pixels = frames["controls"].pixels[:-4]
        self.assertFails("controls: frame and first/steady counters captured", frames, report, log)


if __name__ == "__main__":
    unittest.main()
