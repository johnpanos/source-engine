"""Oracles for LMAP v2 separated layers in `lightmap_ktx2.py`: the indirect
layer's own directional page (`--layer-directional indirect=EXR`, RFC 0016's
runtime direct light) lands in its gradient half, a layer without one keeps
a zero gradient half, and a beta page that is not the layer's is refused.

    python3 -m unittest tools.quality.tests.test_lightmap_ktx2_layers -v
"""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
sys.path.insert(0, str(HERE))
import lightmap_denoise  # noqa: E402
from test_lightmap_ktx2_seams import ktx_tool  # noqa: E402

SCOPE = "test-bake"
SIZE = 8


def sha256(path):
    import hashlib
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write(path, rgb):
    pixels = np.ones((SIZE, SIZE, 4), dtype=np.float32)
    pixels[..., :3] = rgb
    lightmap_denoise.write_linear_exr(path, pixels)
    return path


@unittest.skipIf(ktx_tool() is None, "pinned ktx tool not provisioned (pbrt_map_toolchain.py)")
class Ktx2LayerDirectionalTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        rows = np.linspace(0.2, 1.0, SIZE)[:, None, None]
        self.total = write(self.root / "atlas.exr", np.broadcast_to(rows * 2.0, (SIZE, SIZE, 3)))
        self.indirect = write(self.root / "indirect.exr",
                              np.broadcast_to(rows * 0.5, (SIZE, SIZE, 3)))
        self.stage = self.root / "stage.usda"
        self.stage.write_text("#usda 1.0\n")
        self.bake = self.root / "bake.json"
        self.bake.write_text("{}")
        self.receipt = self.root / "atlas.exr.json"
        self.receipt.write_text(json.dumps({
            "status": "pass", "scope": SCOPE, "size": SIZE,
            "atlas_exr_sha256": sha256(self.total),
            "lighting_stage_sha256": sha256(self.stage),
            "source_bake_evidence_sha256": sha256(self.bake)}))
        (self.root / "indirect.exr.json").write_text(json.dumps({
            "status": "pass", "layer": "indirect", "atlas_exr_sha256": sha256(self.indirect),
            "source_bake_evidence_sha256": sha256(self.bake)}))
        self.total_beta = self.beta("atlas-directional.exr", (0.3, 0.0, 0.1), self.total,
                                    "total")
        self.indirect_beta = self.beta("indirect-directional.exr", (0.0, -0.2, 0.05),
                                       self.indirect, "indirect")

    def tearDown(self):
        self.tmp.cleanup()

    def beta(self, name, value, flat, layer):
        path = write(self.root / name, np.broadcast_to(np.asarray(value), (SIZE, SIZE, 3)))
        path.with_name(path.name + ".json").write_text(json.dumps({
            "status": "pass", "layer": layer, "size": SIZE,
            "directional_exr_sha256": sha256(path), "flat_exr_sha256": sha256(flat)}))
        return path

    def package(self, extra, total_beta=True):
        out = self.root / "atlas.ktx2"
        command = [sys.executable, str(HERE.parent / "lightmap_ktx2.py"), "--exr",
                   str(self.total), "--bake-evidence", str(self.receipt), "--lighting-stage",
                   str(self.stage), "--ktx-tool", str(ktx_tool()), "--expected-scope", SCOPE] + \
            (["--directional-exr", str(self.total_beta)] if total_beta else []) + \
            ["--layer", "indirect=%s" % self.indirect, "--out", str(out)] + list(extra)
        return subprocess.run(command, capture_output=True, text=True), out

    def layer(self, out, index):
        raw = self.root / ("layer%d.rgba16f" % index)
        subprocess.run([str(ktx_tool()), "extract", "--raw", "--layer", str(index), str(out),
                        str(raw)], check=True, capture_output=True)
        return np.frombuffer(raw.read_bytes(), dtype="<f2").reshape(SIZE, 2 * SIZE, 4)

    def test_the_indirect_layer_carries_its_own_gradient(self):
        result, out = self.package(["--layer-directional",
                                    "indirect=%s" % self.indirect_beta])
        self.assertEqual(result.returncode, 0, result.stderr)
        indirect = self.layer(out, 1)
        np.testing.assert_allclose(indirect[:, SIZE:, :3].astype(np.float64),
                                   np.broadcast_to((0.0, -0.2, 0.05), (SIZE, SIZE, 3)),
                                   atol=1e-3)
        total = self.layer(out, 0)
        np.testing.assert_allclose(total[:, SIZE:, :3].astype(np.float64),
                                   np.broadcast_to((0.3, 0.0, 0.1), (SIZE, SIZE, 3)), atol=1e-3)
        record = json.loads(out.with_name(out.name + ".json").read_text())
        self.assertEqual(record["separated_layers"]["indirect"]["directional_exr_sha256"],
                         sha256(self.indirect_beta))

    def test_the_lmap_v3_lump_matches_the_master_pages(self):
        lump = self.root / "atlas.lmap"
        result, out = self.package(["--layer-directional", "indirect=%s" % self.indirect_beta,
                                    "--lmap-out", str(lump)])
        self.assertEqual(result.returncode, 0, result.stderr)
        sys.path.insert(0, str(HERE.parent))
        import world_lightmap_v3
        data = lump.read_bytes()
        layout = world_lightmap_v3.read(data)
        self.assertEqual([l["role"] for l in layout["layers"]], ["total", "indirect"])
        for index in (0, 1):
            master = self.layer(out, index).astype(np.float32)
            flat, beta, _ = world_lightmap_v3.decode_layer(data, layout, index)
            np.testing.assert_allclose(flat, master[:, :SIZE, :3], rtol=0.03, atol=1e-3)
            np.testing.assert_allclose(beta, master[:, SIZE:, :3], atol=0.01)
        receipt = json.loads(lump.with_name(lump.name + ".json").read_text())
        self.assertEqual(receipt["version"], 3)
        self.assertLess(receipt["bytes"], receipt["rgba16f_bytes"])

    def test_the_layers_gradient_alone_makes_a_directional_page(self):
        # Runtime direct light: the total page is flat (its gradient half
        # zero) and the indirect layer carries its own.
        result, out = self.package(["--layer-directional", "indirect=%s" % self.indirect_beta],
                                   total_beta=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(float(np.abs(self.layer(out, 0)[:, SIZE:, :3]).max()), 0.0)
        np.testing.assert_allclose(self.layer(out, 1)[:, SIZE:, :3].astype(np.float64),
                                   np.broadcast_to((0.0, -0.2, 0.05), (SIZE, SIZE, 3)),
                                   atol=1e-3)
        record = json.loads(out.with_name(out.name + ".json").read_text())
        self.assertEqual(record["layout"], "directional-2x1")

    def test_without_it_the_gradient_half_is_zero(self):
        result, out = self.package([])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(float(np.abs(self.layer(out, 1)[:, SIZE:, :3]).max()), 0.0)

    def test_the_totals_beta_is_refused_as_the_layers(self):
        # Negative control: the total's beta page names the total's flat page.
        result, out = self.package(["--layer-directional", "indirect=%s" % self.total_beta])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("differs from its receipt or layer", result.stderr)
        self.assertFalse(out.exists())


if __name__ == "__main__":
    unittest.main()
