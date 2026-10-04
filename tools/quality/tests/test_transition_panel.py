"""Compile production tile-marking methods against a small VGUI test double.

This checks startup ordering and rectangle coverage, not rendering. The real
menu captures in RFC/0016-transition-panel-fix.md cover the presentation path.
"""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]
SOURCE = ROOT / "game/client/portal2/gameui/portal2/transitionpanel.cpp"


def method(source, name, owner="CBaseModTransitionPanel"):
    start = source.index(owner + "::" + name + "(")
    start = source.rfind("\n", 0, start) + 1
    brace = source.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


# The double models only scheme availability, screen extent and grid storage.
# ApplySchemeSettings represents successful grid construction; all lookup and
# marking logic below is compiled from the production source, not reimplemented.
DOUBLE = r'''
#include <algorithm>
#include <cstdlib>
#include <set>
#define MIN std::min
#define MAX std::max
using WINDOW_TYPE = int;
struct Scheme {};
struct Surface {
    int wide = 1280, tall = 720;
    void GetScreenSize(int &w, int &h) { w = wide; h = tall; }
} screen;
Surface *surface() { return &screen; }
namespace vgui {
using IScheme = Scheme;
struct SchemeManager {
    Scheme value;
    bool available = true;
    Scheme *GetIScheme(int) { return available ? &value : nullptr; }
} manager;
SchemeManager *scheme() { return &manager; }
}
struct Tiles {
    int count = 0;
    int Count() const { return count; }
    bool IsValidIndex(int n) const { return n >= 0 && n < count; }
};
struct CBaseModTransitionPanel {
    int m_nTileWidth = 0, m_nTileHeight = 0;
    int m_nNumColumns = 0, m_nNumRows = 0;
    int m_nXOffset = 0, m_nYOffset = 0;
    int wide = 0, tall = 0, builds = 0;
    bool enabled = true;
    Tiles m_Tiles;
    std::set<int> touched;
    int GetWide() { return wide; }
    int GetTall() { return tall; }
    int GetScheme() { return 1; }
    bool IsEffectEnabled() { return enabled; }
    void ApplySchemeSettings(Scheme *) {
        ++builds;
        wide = screen.wide; tall = screen.tall;
        m_nTileWidth = 50; m_nTileHeight = 50;
        m_nNumColumns = (wide - m_nXOffset + 49) / 50;
        m_nNumRows = (tall - m_nYOffset + 49) / 50;
        m_Tiles.count = m_nNumColumns * m_nNumRows;
        touched.clear();
    }
    void TouchTile(int n, WINDOW_TYPE, bool) {
        if (m_Tiles.IsValidIndex(n)) touched.insert(n);
    }
    bool EnsureTileGrid();
    int GetTileIndex(int x, int y);
    void MarkTile(int x, int y, WINDOW_TYPE wt, bool bForce = false);
    void MarkTilesInRect(int x, int y, int w, int h, WINDOW_TYPE wt, bool bForce = false);
};
void check(bool value) { if (!value) std::exit(42); }
'''

CASES = r'''
int main(int argc, char **argv) {
    check(argc == 2);
    int scenario = std::atoi(argv[1]);
    CBaseModTransitionPanel p;
    if (scenario == 0) {
        p.MarkTilesInRect(0, 0, -1, -1, 1);
        check(p.builds == 1 && p.touched.size() == size_t(p.m_Tiles.Count()));
    } else if (scenario == 1) {
        vgui::manager.available = false;
        p.MarkTilesInRect(0, 0, -1, -1, 1);
        p.MarkTile(0, 0, 1);
        check(p.builds == 0 && p.touched.empty());
    } else if (scenario == 2) {
        screen.wide = screen.tall = 0;
        p.MarkTilesInRect(0, 0, -1, -1, 1);
        check(p.builds == 0 && p.touched.empty());
    } else if (scenario == 3) {
        p.MarkTile(0, 0, 1);
        p.MarkTile(1279, 719, 1);
        check(p.builds == 1 && p.touched == std::set<int>({0, 389}));
        check(p.GetTileIndex(1280, 0) == -1 && p.GetTileIndex(-1, 0) == -1);
        check(p.GetTileIndex(0, 720) == -1 && p.GetTileIndex(0, -1) == -1);
    } else if (scenario == 4) {
        p.MarkTilesInRect(0, 0, -1, -1, 1);
        screen.wide = 800; screen.tall = 600;
        p.MarkTilesInRect(0, 0, -1, -1, 1);
        check(p.builds == 2 && p.touched.size() == 192);
    } else if (scenario == 5) {
        p.enabled = false;
        p.MarkTilesInRect(0, 0, -1, -1, 1);
        check(p.builds == 0 && p.touched.empty());
    } else if (scenario == 6) {
        p.m_nXOffset = -20; p.m_nYOffset = -30;
        p.MarkTilesInRect(0, 0, -1, -1, 1);
        check(p.touched.size() == size_t(p.m_Tiles.Count()));
    } else if (scenario == 7) {
        // Exhaustive small-screen rectangles; the oracle intersects each
        // tile with the clipped inclusive coverage independently of indexing.
        screen.wide = 101; screen.tall = 83;
        for (int x = -60; x <= 160; x += 11)
        for (int y = -60; y <= 140; y += 13)
        for (int w : {0, 1, 49, 50, 200})
        for (int h : {0, 1, 49, 50, 200}) {
            p.touched.clear();
            p.MarkTilesInRect(x, y, w, h, 1);
            std::set<int> expected;
            if (w > 0 && h > 0 && x < 101 && y < 83 && x+w > 0 && y+h > 0) {
                for (int row = 0; row < 2; ++row)
                for (int col = 0; col < 3; ++col) {
                    if (col*50 <= std::min(x+w, 100) && (col+1)*50 > std::max(x, 0) &&
                        row*50 <= std::min(y+h, 82) && (row+1)*50 > std::max(y, 0))
                        expected.insert(row*3+col);
                }
            }
            check(p.touched == expected);
        }
    } else if (scenario == 8) {
        check(p.GetTileIndex(0, 0) == -1);
    } else std::exit(43);
}
'''


COPY_DOUBLE = r"""
#include <cstdlib>
struct Rect_t { int x, y, width, height; };
using ShaderAPITextureHandle_t = int;
namespace render_vulkan {
struct CVulkanContext {
    struct DepthToAlpha { float projection[4]; float invRange; };
    bool lastDepth = false, succeeds = true;
    int copies = 0;
    bool QueueCopyToTexture(int, int *, int *, const DepthToAlpha *depth) {
        ++copies;
        lastDepth = depth != nullptr;
        return succeeds;
    }
};
}
render_vulkan::CVulkanContext g_VulkanContext;
struct { float destAlphaDepthRange = 400; } g_Fog;
int g_TargetCopies = 0, g_TargetCopiesDropped = 0;
float projection[16] = {};
const float *DrawProjection() { return projection; }
struct CommandLineDouble {
    bool disabled = false;
    int FindParm(const char *) { return disabled ? 1 : 0; }
} commands;
CommandLineDouble *CommandLine() { return &commands; }
void NoteDeviceUse(const char *) {}
#define VK_UNIMPLEMENTED() do {} while (false)
struct CShaderAPIVulkan {
    void CopyRenderTargetToTextureEx(ShaderAPITextureHandle_t, int, Rect_t *, Rect_t *);
};
void check(bool value) { if (!value) std::exit(42); }
"""
COPY_CASES = r"""
int main(int argc, char **argv) {
    check(argc == 2);
    if (std::atoi(argv[1]) == 1) commands.disabled = true;
    CShaderAPIVulkan api;
    // Orthographic UI projection: its alpha is opacity and must survive.
    projection[15] = 1;
    api.CopyRenderTargetToTextureEx(1, 0, nullptr, nullptr);
    check(!g_VulkanContext.lastDepth && g_TargetCopies == 1);
    // Perspective opaque scene: retain the soft-particle depth contract.
    projection[15] = 0;
    projection[11] = -1;
    api.CopyRenderTargetToTextureEx(1, 0, nullptr, nullptr);
    check(g_VulkanContext.lastDepth == !commands.disabled && g_TargetCopies == 2);
    // A following UI copy must not inherit the scene's policy.
    projection[15] = 1;
    projection[11] = 0;
    api.CopyRenderTargetToTextureEx(1, 0, nullptr, nullptr);
    check(!g_VulkanContext.lastDepth && g_TargetCopies == 3);
    api.CopyRenderTargetToTextureEx(1, 1, nullptr, nullptr);
    check(g_VulkanContext.copies == 3);
    g_VulkanContext.succeeds = false;
    api.CopyRenderTargetToTextureEx(1, 0, nullptr, nullptr);
    check(g_TargetCopies == 3 && g_TargetCopiesDropped == 1);
}
"""


class TransitionPanelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix="transition-panel-")
        cls.addClassCleanup(cls.directory.cleanup)
        source = SOURCE.read_text()
        cls.methods = "\n".join(method(source, name) for name in (
            "EnsureTileGrid", "GetTileIndex", "MarkTile", "MarkTilesInRect"))
        cls.executable = cls.compile(cls.methods, "current")

    @classmethod
    def compile(cls, methods, name, double=DOUBLE, cases=CASES):
        path = Path(cls.directory.name) / name
        path.with_suffix(".cpp").write_text(double + methods + cases)
        subprocess.run([os.environ.get("CXX", "c++"), "-std=c++20", "-O2",
                        str(path.with_suffix(".cpp")), "-o", str(path)],
                       check=True, capture_output=True, timeout=30)
        return path

    def test_production_marking(self):
        for case in range(9):
            with self.subTest(case=case):
                subprocess.run([self.executable, str(case)], check=True, timeout=5)

    def test_seeded_faults_are_detected(self):
        mutations = {
            "first-row-only": ("GetTileIndex( nRight, nBottom )", "-1", 0),
            "missing-last-row": ("nTile <= nEndTile", "nTile < nEndTile", 7),
            "missing-readiness": ("!IsEffectEnabled() || !EnsureTileGrid()",
                                  "!IsEffectEnabled()", 3),
        }
        for name, (old, new, case) in mutations.items():
            with self.subTest(fault=name):
                self.assertIn(old, self.methods)
                executable = self.compile(self.methods.replace(old, new), name)
                result = subprocess.run([executable, str(case)], timeout=5)
                self.assertNotEqual(result.returncode, 0)

    def test_production_copy_alpha_policy(self):
        source = (ROOT / "materialsystem/shaderapivulkan/shaderapivulkan.cpp").read_text()
        body = method(source, "CopyRenderTargetToTextureEx", "CShaderAPIVulkan")
        executable = self.compile(body, "copy-current", COPY_DOUBLE, COPY_CASES)
        for rollback in range(2):
            with self.subTest(depth_copy_disabled=rollback):
                subprocess.run([executable, str(rollback)], check=True, timeout=5)
        old = "s_depthAlpha && projection[2 * 4 + 3] != 0.0f"
        self.assertIn(old, body)
        broken = self.compile(body.replace(old, "s_depthAlpha"), "copy-ui-depth",
                              COPY_DOUBLE, COPY_CASES)
        self.assertNotEqual(subprocess.run([broken, "0"], timeout=5).returncode, 0)


if __name__ == "__main__":
    unittest.main()
