"""Round-trip captured VMT matrices and reject the old transpose/precision losses."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]
HEADER = ROOT / "public/render/material/vmt_matrix.h"
CASES = r'''
#include "candidate.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <sstream>
int main() {
    // Rotation, unequal scales, translation, and a sub-millipixel offset.
    const std::array<float,16> matrix = {
        0, -2, 0, .123456789f, 3, 0, 0, -.0001234567f,
        0, 0, 1, 0, 0, 0, 0, 1
    };
    std::istringstream text(RenderMaterialVmt::MatrixValue(matrix.data()));
    char open, close; text >> open;
    if (open != '[') std::abort();
    for (float expected : matrix) {
        float actual; text >> actual;
        if (!text || actual != expected) std::abort();
    }
    text >> close;
    if (close != ']') std::abort();
}
'''


class VmtMatrixCaptureTests(unittest.TestCase):
    def test_production_formatter_and_seeded_losses(self):
        source = HEADER.read_text()
        mutations = {
            "control": None,
            "column-major": ("rows[i]", "rows[(i % 4) * 4 + i / 4]"),
            "three-decimals": ("%.9g", "%.3f"),
        }
        with tempfile.TemporaryDirectory(prefix="vmt-matrix-") as directory:
            path = Path(directory)
            for name, mutation in mutations.items():
                with self.subTest(name=name):
                    candidate = source
                    if mutation:
                        self.assertIn(mutation[0], candidate)
                        candidate = candidate.replace(*mutation)
                    (path / "candidate.h").write_text(candidate)
                    (path / "case.cpp").write_text(CASES)
                    executable = path / name
                    subprocess.run([os.environ.get("CXX", "c++"), "-std=c++20", "-O2",
                                    str(path / "case.cpp"), "-o", str(executable)],
                                   check=True, capture_output=True, timeout=30)
                    result = subprocess.run([executable], timeout=5)
                    if mutation is None:
                        self.assertEqual(result.returncode, 0)
                    else:
                        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
