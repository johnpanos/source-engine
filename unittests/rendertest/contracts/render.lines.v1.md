# Contract: `render.lines.v1`

Module: `render.pass.lines` (RFC 0016 layer 6 feature; decision "lines and overlays")
Header: `public/render/pass/lines/lines.h`; shaders `render/pass/lines/lines.vert`, `lines.frag`
Suites: `unittests/rendertest/core/pass/lines/test_lines.cpp` (`render.lines.null`, headless)
and `test_lines_vulkan.cpp` (`render.lines`, `linux-native-vulkan-gpu`)
Rows: R86/R87 (the shared pass the Hammer editor's viewports and, later, the game's debug
overlay draw through)

| Clause | Obligation |
| --- | --- |
| L1 | One upload pass (the list's vertices into a transient buffer) and one render pass; resident batches draw first in the order given, then the list's world depth-tested filled, world depth-tested lines, world untested filled, world untested lines, screen lines and screen filled, one draw per non-empty batch, each after its 80-byte draw-constant write |
| L2 | Pipelines are made once per topology and depth mode and reused across frames |
| L3 | Invalid targets (no color, a zero size, depth present or absent against the renderer's depth format), depth-tested items or batches without a depth format, and a resident mesh not in the 16-byte `LineVertex` layout are refused before any pass is added |
| L4 | An empty list still clears the target: a render pass with no draw |
| P1 | Screen space is the target's logical pixels, origin top-left, y down: a quad covers its pixel rectangle and nothing outside it |
| P2 | World space goes through the view's world-to-clip: a world line lands on the row its y projects to |
| P3 | A depth-tested line behind a depth-tested face is hidden; an untested line draws over it |
| P4 | The view's depth bias pulls depth-tested lines (not triangles) toward the eye by `bias` in clip z |
| P5 | Screen filled items (handles) draw over screen lines |
| P6 | Colors are straight-alpha RGBA8, alpha-blended |
| P7 | Resident batches draw; the same inputs give byte-identical frames; the Khronos validation layer reports no message |
| P8 | Colors are display (sRGB-encoded) values: on an sRGB target the pass decodes them with the exact sRGB curve, so they read back as written within one level (the pass shares that target with the material families, which draw in linear light) |

Text labels are not drawn by this pass (RFC 0016 decision "lines and overlays": text waits for
a text pass on RFC 0010 fonts). The shaders are committed SPIR-V (`lines_spv.h`, an EMBEDDED row
of `tools/render/shader_toolchain.py`) until the K4 artifact build takes them over.
