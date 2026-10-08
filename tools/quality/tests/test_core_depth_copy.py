"""Compile the replay's depth-copy decisions and reject seeded packet losses."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_transition_panel import method

ROOT = Path(__file__).resolve().parents[3]
SOURCE = ROOT / "materialsystem/shaderapivulkan/vulkan_device.cpp"

DOUBLE = r'''
#include <cstring>
#include <cstdlib>
constexpr int VK_COMPARE_OP_ALWAYS = 7;
struct CVulkanContext {
    enum { kRecordDraw, kRecordClear, kRecordCopy, kRecordQueryBegin,
           kRecordQueryEnd, kRecordSceneCapture, kRecordCorePass };
    enum { kKeepNone = 0, kKeepDepth = 1, kKeepStencil = 2, kKeepDepthStencil = 3 };
    struct DynRasterState { bool depthTest = false, stencilEnable = false;
                            int depthCompare = 0, stencilCompare = 0; };
    struct DynDraw {
        int kind = kRecordCopy, target = -1, copyDst = 2;
        bool coreCustomEffect = false, corePortalCopy = false, queryInput = false;
        bool copyDepthToAlpha = false, clearDepth = false, clearStencil = false;
        DynRasterState raster;
        int copySrcRect[4] = {}, copyDstRect[4] = {};
        struct { float projection[4] = {1, -3, 1, 0}; float invRange = 1.f / 192; } copyDepth;
    };
    static bool SkipLegacyRecord(const DynDraw &, bool, bool);
    static int RecordBackBufferDepthReads(const DynDraw &);
    static bool Repeat(const DynDraw *, const DynDraw &, bool);
};
using Packet = CVulkanContext::DynDraw;
void check(bool value) { if (!value) std::abort(); }
'''
CASES = r'''
int main() {
    using C = CVulkanContext;
    Packet color, depth;
    depth.copyDepthToAlpha = true;
    check(C::SkipLegacyRecord(color, true, false));
    check(!C::SkipLegacyRecord(depth, true, false));
    check(!C::SkipLegacyRecord(color, false, false));
    check(!C::SkipLegacyRecord(color, true, true));
    color.corePortalCopy = true;
    check(!C::SkipLegacyRecord(color, true, false));
    check(C::RecordBackBufferDepthReads(color) == C::kKeepNone);
    check(C::RecordBackBufferDepthReads(depth) == C::kKeepDepth);
    depth.target = 3;
    check(C::RecordBackBufferDepthReads(depth) == C::kKeepNone);
    depth.target = -1;
    check(!C::Repeat(&color, depth, true));
    check(!C::Repeat(&depth, color, true));
    check(C::Repeat(&depth, depth, true));
    check(!C::Repeat(&depth, depth, false));
    check(!C::Repeat(nullptr, depth, true));
    Packet changed = depth;
    changed.copyDepth.invRange = 1.f / 384;
    check(!C::Repeat(&depth, changed, true));
    changed = depth;
    changed.copyDepth.projection[1] = -10;
    check(!C::Repeat(&depth, changed, true));
    changed = depth;
    changed.target = 3;
    check(!C::Repeat(&depth, changed, true));
    changed = depth;
    changed.copySrcRect[2] = 16;
    check(!C::Repeat(&depth, changed, true));
    changed = depth;
    changed.copyDstRect[2] = 32;
    check(!C::Repeat(&depth, changed, true));
    changed = depth;
    changed.copyDst = 7;
    check(!C::Repeat(&depth, changed, true));
}
'''


class CoreDepthCopyTests(unittest.TestCase):
    def test_production_replay_and_seeded_losses(self):
        source = SOURCE.read_text()
        bodies = "\n".join(method(source, name, "CVulkanContext") for name in
                           ("SkipLegacyRecord", "RecordBackBufferDepthReads"))
        start = source.index("const bool repeat =")
        expression = source[start:source.index(";", start) + 1]
        bodies += ("\nbool CVulkanContext::Repeat(const DynDraw *lastCopy, "
                   "const DynDraw &d, bool m_passMerging) { " + expression +
                   " return repeat; }")
        mutations = {
            "control": None,
            "discard-core-depth-input": (" && !d.copyDepthToAlpha", ""),
            "discard-depth-store": ("r.copyDepthToAlpha ? kKeepDepth : kKeepNone",
                                   "kKeepNone"),
            "merge-color-and-depth": ("lastCopy->copyDepthToAlpha == d.copyDepthToAlpha",
                                      "true"),
            "merge-different-depth-range":
                ("lastCopy->copyDepth.invRange == d.copyDepth.invRange", "true"),
            "merge-different-source": ("lastCopy->target == d.target", "true"),
        }
        with tempfile.TemporaryDirectory(prefix="core-depth-copy-") as directory:
            for name, mutation in mutations.items():
                with self.subTest(name=name):
                    candidate = bodies
                    if mutation is not None:
                        self.assertIn(mutation[0], candidate)
                        candidate = candidate.replace(*mutation)
                    executable = Path(directory) / name
                    executable.with_suffix(".cpp").write_text(DOUBLE + candidate + CASES)
                    subprocess.run([os.environ.get("CXX", "c++"), "-std=c++20", "-O2",
                                    str(executable.with_suffix(".cpp")), "-o", str(executable)],
                                   check=True, capture_output=True, timeout=30)
                    result = subprocess.run([executable], timeout=5)
                    if mutation is None:
                        self.assertEqual(result.returncode, 0)
                    else:
                        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
