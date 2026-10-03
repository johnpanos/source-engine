"""Sensitivity of tools/vgui/vgui_fixture_content.py (RFC 0010 V0 fixture
materials): the unchanged set passes its check, and each seeded defect fails
it even when the manifest is re-recorded from the defective output, so the
probes and the blend rule catch it, not only the recorded hashes."""
import contextlib
import copy
import io
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent))
sys.path.insert(0, str(ROOT / "tools" / "vgui"))
import vgui_fixture_content as content  # noqa: E402
import vtf_write  # noqa: E402
from selftest import slow  # noqa: E402


def run_check(generated, recorded, **native):
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        status = content.check(generated, recorded, **native)
    return status, out.getvalue()


def recorded_from(generated):
    return content.manifest_of(generated)


def failing(output):
    return [line for line in output.splitlines() if line.startswith(("FAIL", "native FAIL"))]


class FixtureContentTest(unittest.TestCase):
    def test_unchanged_set_passes(self):
        generated = content.files()
        status, output = run_check(generated, recorded_from(generated))
        self.assertEqual(status, 0, failing(output))
        self.assertRegex(output, r"(?m)^CONFORMANCE \d+ 0$")

    def test_recorded_manifest_is_current(self):
        generated = content.files()
        recorded = content.json.loads((ROOT / content.MANIFEST).read_text())
        status, output = run_check(generated, recorded)
        self.assertEqual(status, 0, failing(output))

    def test_output_is_deterministic(self):
        self.assertEqual(content.files(), content.files())

    def test_every_cursor_the_surface_loads_is_in_the_set(self):
        source = (ROOT / "vguimatsurface" / "Cursor.cpp").read_text()
        loaded = sorted(set(content.re.findall(r'InitSoftwareCursorTexture\( "([^"]+)" \)', source)))
        self.assertTrue(loaded)
        names = {spec["name"] for spec in content.SPECS}
        self.assertEqual([name for name in loaded if name not in names], [])

    def seeded(self, patch):
        """Check a defective generator against a manifest recorded from it."""
        with patch:
            generated = content.files()
            status, output = run_check(generated, recorded_from(generated))
        self.assertEqual(status, 1, "seeded defect passed")
        return failing(output)

    def test_flipped_rows_fail_the_probes(self):
        real = content.texels

        def flipped(spec):
            rows = real(spec)
            stride = spec["width"] * 4
            frame = stride * spec["height"]
            out = bytearray()
            for f in range(spec["frames"]):
                image = rows[f * frame:(f + 1) * frame]
                for y in reversed(range(spec["height"])):
                    out += image[y * stride:(y + 1) * stride]
            return bytes(out)

        failures = self.seeded(mock.patch.object(content, "texels", flipped))
        self.assertTrue(any("quadrants frame 0 texel (0, 0)" in f for f in failures), failures)

    def test_swapped_channels_fail_the_probes(self):
        real = content.texels

        def bgra(spec):
            data = bytearray(real(spec))
            data[0::4], data[2::4] = data[2::4], data[0::4]
            return bytes(data)

        failures = self.seeded(mock.patch.object(content, "texels", bgra))
        self.assertTrue(any("quadrants" in f for f in failures), failures)

    def test_reversed_frames_fail_the_probes(self):
        real = content.texels

        def reversed_frames(spec):
            data = real(spec)
            size = spec["width"] * spec["height"] * 4
            frames = [data[i * size:(i + 1) * size] for i in range(spec["frames"])]
            return b"".join(reversed(frames))

        failures = self.seeded(mock.patch.object(content, "texels", reversed_frames))
        self.assertTrue(any("frames frame 0" in f for f in failures), failures)

    def test_a_blend_the_flags_do_not_give_fails(self):
        keys = copy.deepcopy(content.VMT_KEYS)
        keys["opaque"] = keys["opaque"] + [("$translucent", "1")]
        failures = self.seeded(mock.patch.object(content, "VMT_KEYS", keys))
        self.assertTrue(any("opaque: the surface derives blend alpha" in f for f in failures),
                        failures)

    def test_wrong_header_flags_fail(self):
        real = vtf_write.vtf

        def no_clamp(rgba, width, height, flags, frames=1):
            return real(rgba, width, height, flags & ~vtf_write.CLAMPS, frames)

        failures = self.seeded(mock.patch.object(content.vtf_write, "vtf", no_clamp))
        self.assertTrue(any(".vtf header" in f for f in failures), failures)

    def test_changed_bytes_fail_the_recorded_hashes(self):
        generated = content.files()
        recorded = recorded_from(generated)
        path = "materials/vgui/fixture/checker.vtf"
        changed = dict(generated)
        changed[path] = changed[path][:-1] + bytes([changed[path][-1] ^ 1])
        status, output = run_check(changed, recorded)
        self.assertEqual(status, 1)
        self.assertTrue(any(path in f for f in failing(output)))

    def test_a_missing_file_fails(self):
        generated = content.files()
        recorded = recorded_from(generated)
        missing = dict(generated)
        del missing["materials/vgui/cursors/hand.vmt"]
        status, output = run_check(missing, recorded)
        self.assertEqual(status, 1)
        self.assertTrue(any("vgui/cursors/hand.vmt is in the set" in f for f in failing(output)))


class NativeReaderTest(unittest.TestCase):
    """The engine's VTF reader (texturecontainer::vtf), built with the
    profile's flags. Slow tier: it compiles."""

    @slow  # slow tier: tools/quality/selftest.py --tier slow
    def test_native_reader_passes_and_catches_flipped_rows(self):
        generated = content.files()
        recorded = recorded_from(generated)
        cxx = os.environ.get("CXX", "g++")
        with tempfile.TemporaryDirectory() as work:
            status, output = run_check(generated, recorded, work=work, cxx=cxx)
            self.assertEqual(status, 0, failing(output))
            self.assertIn("the engine's VTF reader checked", output)
        real = content.texels

        def flipped_quadrants(spec):
            data = real(spec)
            if spec["name"] != "vgui/fixture/quadrants":
                return data
            stride = spec["width"] * 4
            return b"".join(data[y * stride:(y + 1) * stride]
                            for y in reversed(range(spec["height"])))

        with mock.patch.object(content, "texels", flipped_quadrants), \
                tempfile.TemporaryDirectory() as work:
            generated = content.files()
            # Python readers skipped by a matching manifest is not enough: the
            # native lines themselves must report the flipped texture.
            status, output = run_check(generated, recorded_from(generated), work=work, cxx=cxx)
        self.assertEqual(status, 1)
        self.assertTrue(any(line.startswith("native FAIL") and "quadrants" in line
                            for line in output.splitlines()), failing(output))


if __name__ == "__main__":
    unittest.main()
