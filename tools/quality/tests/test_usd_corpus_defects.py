"""Known `usd_scene` defects exposed by the USD gap corpus (reported, not fixed).

Each test asserts the correct behaviour and fails today. The corpus runner
registers this module as an `expect: fail` row that names the first failure,
so a fix shows up as a mismatch; then move the test into
test_usd_corpus_rooms.py (and drop the mesh from KNOWN_NORMAL_DEFECTS there).

    python3 tools/quality/corpus/usd_runner.py tools.quality.tests.test_usd_corpus_defects
"""

import sys
import unittest
from pathlib import Path

try:
    from pxr import UsdGeom
except ImportError as error:  # no OpenUSD here; the corpus runner fails instead
    raise unittest.SkipTest("test_usd_corpus_defects needs OpenUSD's pxr: %s" % error)

sys.path.insert(0, str(Path(__file__).resolve().parent))
import test_usd_corpus_rooms as rooms  # noqa: E402


class SurfaceContextDefectTest(unittest.TestCase):
    """materials.usda /Room/Looks/Toon authors a custom shader id on the
    universal `outputs:surface` and nothing in any render context."""

    def test_universal_custom_surface_is_not_attributed_to_mtlx(self):
        temporary, model, _ = rooms.extract(rooms.fixture("materials"))
        try:
            approximation = model["materials"]["toon"]["approximation"] or ""
        finally:
            temporary.cleanup()
        # Root cause: preview_summary probes ComputeSurfaceSource("mtlx")
        # first, and UsdShade falls back to the universal output for a render
        # context that has none, so the first probed context is reported.
        self.assertNotIn("mtlx", approximation,
                         "Toon has no mtlx output but is recorded as %r" % approximation)


class LeftHandedNormalsDefectTest(unittest.TestCase):
    """box_room_in_zup.usda /Room/LeftHanded: orientation leftHanded, faces
    authored clockwise from outside, no authored normals."""

    def test_left_handed_stage_normals_agree_with_winding(self):
        temporary, _, stage = rooms.extract(rooms.fixture("box_in"))
        try:
            points, indices, prim = rooms.stage_mesh(stage, "LeftHanded")
            normals = [tuple(map(float, n)) for n in UsdGeom.Mesh(prim).GetNormalsAttr().Get()]
        finally:
            temporary.cleanup()
        # Root cause: smooth_vertex_normals derives normals from the authored
        # corner order by the right-hand rule, while add_mesh flips only the
        # triangles for leftHanded meshes; the normals are left inward.
        opposed = 0
        for t in range(0, len(indices), 3):
            a, b, c = (points[i] for i in indices[t:t + 3])
            winding = rooms.cross(rooms.sub(b, a), rooms.sub(c, a))
            opposed += sum(rooms.dot(winding, normals[t + k]) <= 0 for k in range(3))
        self.assertEqual(opposed, 0, "%d of %d LeftHanded corner normals oppose their winding"
                         % (opposed, len(indices)))


if __name__ == "__main__":
    unittest.main()
