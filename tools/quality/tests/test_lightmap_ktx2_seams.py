#!/usr/bin/env python3
"""The LMAP package is continuous across chart seams, measured on the
encoded KTX2 texels the runtime samples.

A random wall (test_lightmap_variants.Wall) is baked with a different
exposure per chart, packaged by lightmap_ktx2.py with and without --seams,
and read back with `ktx extract`. Lookups use the runtime convention
(texel row r holds lightmap v = (r + 0.5) / size, top-left origin), which
is independent of the EXR orientation the stitcher works in, so a flip or
quantization error between stitching and encoding shows here. Needs the
pinned `ktx` tool from the map toolchain; no Blender or map build.
"""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent))
sys.path.insert(0, str(HERE))
import lightmap_denoise  # noqa: E402
import lightmap_seams as seams  # noqa: E402
import test_lightmap_variants as variants  # noqa: E402

SCOPE = "test-bake"


def ktx_tool():
    toolchain = ROOT / "build/toolchains/pbrt-map-toolchain.json"
    if not toolchain.is_file():
        return None
    path = Path(json.loads(toolchain.read_text())["ktx"])
    return path if path.is_file() else None


def runtime_lookup(page, uv):
    """Bilinear lookup in KTX/Vulkan convention: row r is v = (r + 0.5) / H."""
    height, width = page.shape[:2]
    x = uv[:, 0] * width - 0.5
    y = uv[:, 1] * height - 0.5
    x0, y0 = np.floor(x).astype(int), np.floor(y).astype(int)
    fx, fy = x - x0, y - y0
    x0, y0 = np.clip(x0, 0, width - 2), np.clip(y0, 0, height - 2)
    p = page.astype(np.float64)[..., :3] @ np.array([0.2126, 0.7152, 0.0722])
    return ((1 - fx) * (1 - fy) * p[y0, x0] + fx * (1 - fy) * p[y0, x0 + 1] +
            (1 - fx) * fy * p[y0 + 1, x0] + fx * fy * p[y0 + 1, x0 + 1])


@unittest.skipIf(ktx_tool() is None, "pinned ktx tool not provisioned (pbrt_map_toolchain.py)")
class Ktx2SeamTest(unittest.TestCase):
    def package(self, directory, stitch, extra=()):
        wall, seed = variants.wall_with_seams(2)
        bias = np.linspace(0.75, 1.25, len(wall.cuts) - 1)
        image, coverage = variants.baked_atlas(wall, variants.affine_field(wall, seed), bias,
                                               0.01, seed)
        atlas, cover = directory / "atlas.exr", directory / "coverage.exr"
        stage = directory / "stage.usda"
        stage.write_text("#usda 1.0\n")
        lightmap_denoise.write_linear_exr(atlas, image.astype(np.float32))
        mask = np.zeros_like(image, dtype=np.float32)
        mask[coverage] = 1.0
        lightmap_denoise.write_linear_exr(cover, mask)
        receipt = directory / "atlas.exr.json"
        receipt.write_text(json.dumps({
            "status": "pass", "scope": SCOPE, "size": variants.SIZE,
            "atlas_exr_sha256": seams.sha256(atlas), "lighting_stage_sha256": seams.sha256(stage)}))
        found = variants.found_seams(wall.positions(), wall.uvs())
        npz = directory / "seams.npz"
        with open(npz, "wb") as handle:
            np.savez(handle, schema=seams.SCHEMA, uv_a=found["uv_a"], uv_b=found["uv_b"],
                     weight=found["weight"], size=variants.SIZE)
        out = directory / "atlas.ktx2"
        command = [sys.executable, str(HERE.parent / "lightmap_ktx2.py"), "--exr", str(atlas),
                   "--bake-evidence", str(receipt), "--lighting-stage", str(stage),
                   "--ktx-tool", str(ktx_tool()), "--expected-scope", SCOPE, "--out", str(out)]
        if stitch:
            command += ["--seams", str(npz), "--coverage-exr", str(cover)] + list(extra)
        result = subprocess.run(command, capture_output=True, text=True)
        return result, out, found

    def decoded(self, out):
        raw = out.with_suffix(".rgba16f")
        subprocess.run([str(ktx_tool()), "extract", "--raw", str(out), str(raw)], check=True,
                       capture_output=True)
        return np.frombuffer(raw.read_bytes(), dtype="<f2").reshape(variants.SIZE,
                                                                   variants.SIZE, 4)

    def discontinuity(self, out, found):
        page = self.decoded(out)
        a, b = runtime_lookup(page, found["uv_a"]), runtime_lookup(page, found["uv_b"])
        return np.abs(a - b) / np.maximum((a + b) / 2, 1e-6)

    def test_stitched_package_is_continuous_where_the_unstitched_one_is_not(self):
        with tempfile.TemporaryDirectory() as name:
            directory = Path(name)
            result, out, found = self.package(directory, stitch=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertGreater(np.percentile(self.discontinuity(out, found), 99), 0.1)
            result, out, found = self.package(directory, stitch=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            relative = self.discontinuity(out, found)
            # Half-float storage adds about 2^-11 relative per lookup.
            self.assertLess(np.percentile(relative, 99), 0.002 + 2 ** -10)
            record = json.loads(out.with_name(out.name + ".json").read_text())["seams"]
            self.assertLessEqual(record["pages"]["total"]["after"]["p99"], record["gate"]["p99"])
            self.assertGreater(record["pages"]["total"]["before"]["p99"], 0.1)

    def test_gate_rejects_a_package_it_cannot_certify(self):
        with tempfile.TemporaryDirectory() as name:
            result, out, _ = self.package(Path(name), stitch=True, extra=["--max-seam", "0"])
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("exceeds the gate", result.stderr)
            self.assertFalse(out.exists())


if __name__ == "__main__":
    unittest.main()
