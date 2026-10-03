# RFC 0010 progress

Progress records for [RFC 0010](0010-portable-vgui-surface.md). The RFC owns
the semantics and acceptance rules; this file records what is installed, what
passed and what remains. The proposed rows (VG-A to VG-G) are unranked, and
AGENTS.md owns ranks and states.

## V0: VGUI ABI guard (`legacy.vgui-abi`), 2026-10-03

User direction (2026-10-03): guard VGUI's callers mechanically before
modernizing beneath them, then add the upload and draw counters. This slice
is the guard. It is the first part of V0 and does not close V0.

### Scope

The guard covers the 17 frozen VGUI interfaces that cross a module boundary:

- **Versioned interfaces** that game and tool modules obtain from vgui2 and
  vguimatsurface: `ISurface` (`VGUI_Surface030`), `IMatSystemSurface`
  (`MatSystemSurface008`), `IPanel` (`VGUI_Panel009`), `IVGui`
  (`VGUI_ivgui008`), `ISchemeManager` (`VGUI_Scheme010`), `IInput`
  (`VGUI_Input005`), `IInputInternal` (`VGUI_InputInternal001`), `ISystem`
  (`VGUI_System010`) and `ILocalize` (`VGUI_Localize005`).
- **Interfaces handed across the boundary** for the other side to call:
  `IClientPanel` (implemented by every `vgui_controls` panel and called by
  vgui2), `IScheme`, `IBorder`, `IImage`, `IHTML`, `IHTMLEvents`,
  `IVguiMatInfo` and `IVguiMatInfoVar`.

It does not cover `VGuiWorldPanelRecorder001`, a first-party interface added
for RFC 0016's in-world panels, or `vgui_controls`. `vgui_controls` is a
static library compiled into each consumer, so its classes are a source API,
not a cross-module ABI.

### What is installed

- **`quality/fixtures/vgui-abi/vgui_abi_v1.h`.** The recorded table
  `vgui-abi-v1`: 790 vtable slots (770 methods and 20 destructor entries) and 9 version strings, recorded from
  clang's vtable layout dump. It is never regenerated to make a check pass;
  `record --update` is a reviewed ABI decision.
- **`tools/vgui/vgui_abi.py`.** `record`, `check` and `sensitivity` for the
  table. It holds only the table's specification.
- **`tools/quality/abi_table.py`.** The shared table machinery, extracted from
  `tools/render/render_abi.py`, which now holds only the render table's
  specification. The render table is reproduced byte-identically, and its
  suites and unit tests pass unchanged. The extraction added three things:
  - namespaced interfaces;
  - global declaring classes written `::C`, so a suite resolving the table
    inside `namespace vgui` cannot bind to a namesake (`vgui::ILocalize`
    derives from `::ILocalize`);
  - an inheritance-aware sensitivity rule: a seed in a base may change
    exactly that base and the table interfaces derived from it
    (`ISurface` → `IMatSystemSurface`, `IInput` → `IInputInternal`).
- **`unittests/abitable/abi_table_check.h`.** The C++11 checker, shared by
  `legacy.render-abi` and `legacy.vgui-abi`.
- **`unittests/vguiabitest/vgui_abi_conformance.cpp`.** The suite, compiled
  in the exact legacy-cxx11 dialect as a prebuilt consumer would be.
- **`architecture/modules.json`.** The 16 interface headers and the table are
  added to `legacyAbi.paths`, so CAP010 keeps strict types out of them.
- **`tools/quality/tests/test_abi_table.py`.** Unit tests for the shared
  machinery and the VGUI specification. They run in `quality.selftest`.

### A defect found and fixed

An exact C++11 consumer could not compile `tier1/ilocalize.h`, and therefore
could not use `VGUI_Localize005`. `KeyValues::AutoDeleteInline` returns the
non-copyable `KeyValues::AutoDelete` by value. That is legal only under
C++17's guaranteed copy elision.

`AutoDelete` now has a public move constructor that transfers ownership, for
C++11 and later. It is inline and header-only, so no layout or vtable
changes. With the original header, g++ `-std=c++11` reports the private-copy
error. With the fix, the in-tree call pattern
(`Command( KeyValues::AutoDeleteInline( new KeyValues( "Start" ) ) )`)
compiles with g++ and clang++ in C++11, C++17 and C++20. g++ in plain C++20
still rejects `tier0/threadtools.h:1173`, a template-id used as a
constructor name. That happens with the original header too and is not
caused by this change.

### Evidence

Linux x86_64, g++ 13.3.0 and clang++ 18.1.3.

| Suite | Result |
| --- | --- |
| `legacy.vgui-abi` | 832 checks pass on g++ and on clang++ |
| `legacy.vgui-abi.table` | 18 of 18 (17 interface blocks and the preamble) |
| `legacy.vgui-abi.sensitivity` | 103 of 103: the control passes; for each of the 17 interfaces a slot reorder and an appended virtual are confirmed as real layout changes by clang, detected by the suite, and attributed to that interface (with its derived interfaces) and no other |
| `legacy.render-abi`, `.table`, `.sensitivity` | 574, 9 and 49 checks, unchanged after the extraction |
| `test_abi_table.py`, `test_render_abi.py` | 9 and 11 tests pass |
| archlint `check --all`, `baseline --verify` | pass |
| stylelint `--changed` (clang-format 22.1.8) | 0 failures |

`legacy.vgui-abi` is in the default `linux-headless-core` plan, so the
baseline's `conformance.gcc` and `conformance.clang` checks gate it. The
`.table` and `.sensitivity` rows run on `linux-host-corpus`. They join a
baseline audit group when VG-A has a roadmap ID: baseline rows must be
`R<number>`.

Failures that predate this change, run against the same environment:

- `archlint inventory --verify` rejects `box3d/extern/` and
  `box3d/samples/`, because the `box3d` submodule is not checked out here.
- `quality.selftest` fails with 32 import errors (`numpy` is missing) and 9
  failures. A clean checkout of `HEAD` gives the same errors and 10 failures;
  the extra one is `test_waf_output_lock`, which passes on this branch's run.

### Reproduction

```sh
python3 tools/quality/conformance.py check --cxx g++ \
    --suite legacy.vgui-abi --suite legacy.vgui-abi.table \
    --suite legacy.vgui-abi.sensitivity \
    --build-dir build/quality/vgui-abi --out vgui-abi.json
python3 tools/vgui/vgui_abi.py check
python3 -m unittest tools/quality/tests/test_abi_table.py
```

### Not done

- The rest of V0: the upload and draw counters, the `ISurface` caller
  inventory, the direct-render panel inventory, the blend modes and
  primitives in use, and the fixed-screen corpus with its captures.
- No Windows or MSVC layout. The table is the Itanium (Linux x86_64) ABI, as
  `legacy.render-abi` is. The suite's member-pointer decoder also handles
  the ARM variant of the Itanium ABI, but no ARM run is recorded.
- No hosted CI run.

## V0: UI counters (`VGuiSurfaceStats001`), 2026-10-03

The second part of V0, the counters the
[transfers and GPU caching](0010-portable-vgui-surface.md#transfers-and-gpu-caching)
work is judged against. This slice adds counting only. It changes no drawing,
upload or paint order, and does not close V0.

### What is counted

`public/VGuiMatSurface/IVGuiSurfaceStats.h` owns the definitions. The totals
are monotonic, and readers divide the difference between two snapshots by the
frames between them, so no reader resets another's view.

- **Frames.** `ISurface::RunFrame` calls, one per engine frame.
- **Paint passes and their main-thread time.** Top-level `PaintTraverseEx`
  calls (the engine makes one to three per frame), timed from
  `StartDrawing` through `FinishDrawing`. Under the queued material system,
  the render thread's replay is not included.
- **Draws.** `IMesh::Draw` calls from the surface's nine draw sites, now
  routed through one `SubmitMesh`, with text batches counted separately. Also
  counted: the vertices, indices and vertex bytes written into dynamic meshes,
  from `CMeshBuilder`'s own counts and vertex size.
- **Texture uploads.** `Download` calls for procedural textures and the bytes
  of their rectangles, with glyph uploads from the font cache counted
  separately.
- **CPU texel copies by the surface.** The caller's pixels into the backing
  copy, and the backing copy into the material system's image on
  regeneration (an upload or a restore). The backend's staging copy is outside
  the surface and is not counted.

`vguimatsurface/SurfaceStats.{h,cpp}` owns the counters for the module, as
relaxed atomics, because regeneration can run on the render thread. The
surface serves them through `QueryInterface`, as it serves
`VGuiWorldPanelRecorder001`. The interface is first-party and not part of the
frozen ABI table.

### Readers

- The RFC 0014 cost overlay (`cl_render_debug_cost 1`) shows two VGUI lines
  between its 4 Hz samples. They include the overlay's own drawing.
- `vgui_surface_stats` prints the per-frame means since its previous call,
  for measurements without the overlay.

Both use the one helper, `VGuiSurfaceStats_PerFrame`. It reports no rate
when no frame passed or any counter went backwards.

### Evidence

| Check | Result |
| --- | --- |
| `vgui.surface_stats` | 47 checks on g++ and clang++: exact means for every counter; no rate (and zeroed output) without frames or when any one of the 14 counters goes backwards; exact counts, no bytes for empty or negative sizes, the paint timer, and four threads of concurrent updates counted exactly |
| `vgui.surface_stats.sensitivity` | 6 of 6: five wrong helpers are each detected (dividing by passes, a counter missing from the backwards check, a rate without frames, 1000-byte KiB, stale output) and the real helper passes |
| ThreadSanitizer | the positive suite built with g++ `-fsanitize=thread`: 47 of 47, no reports. clang's TSan runtime is not installed here |
| Compile | `MatSystemSurface.cpp`, `TextureDictionary.cpp`, `FontTextureCache.cpp`, `SurfaceStats.cpp` and `engine/render_core_cost_panel.cpp` compile with clang++ 18 and the client's Linux defines and cxx20-permissive flags; the Waf product build below links them |
| `vgui.ui_scale`, `vgui.valvefont`, `legacy.vgui-abi` | unchanged, pass |
| archlint `check --all`, `baseline --verify`; stylelint `--changed` | pass; 0 failures |

### Product build (2026-10-03, after network access was granted)

- **Build.** Waf configure (`-T release --render-backend=legacy`, Linux
  x86_64, clang 18.1.3) and `waf build --targets=vguimatsurface,engine`
  succeed: 655 tasks, with `libvguimatsurface.so` and `libengine.so`
  linked. The libraries carry `VGuiSurfaceStats001`, the
  `vgui_surface_stats` command and the overlay's VGUI lines.
- **g++ 13.3 fails in `tier0`, before any changed file.** The cause is
  `public/tier0/threadtools.h:1173`, a template-id used as a constructor
  name, which C++20 rejects. That line is untouched here; the declared
  reference gcc is 16.2.1.
- **Pinned shader toolchain.** GitHub archive downloads are refused in this
  session, but the git proxy serves clones of public repositories. Each
  pinned commit of shaderc, glslang, SPIRV-Tools, SPIRV-Headers and
  SPIRV-Cross was cloned and its archive regenerated with `git archive
  --format=tar` and `gzip -n -6`. All five match their pinned SHA-256 and
  size byte for byte, and `shader_toolchain.py` verified them again before
  building. `shader.toolchain-pin` passes (186 checks). No pin was changed.
- **Submodules.** `ivp`, `box3d`, `thirdparty` and `lib` were not checked
  out in this container. Initializing them at their recorded commits fixed
  configure, and `archlint inventory --verify` now passes. The inventory
  failure recorded above for the ABI guard came from the missing `box3d`
  checkout, not from the tree.

### Unavailable here

- **No game content.** Portal and Portal 2 content is licensed and staged
  from a local Steam install (`quality/baseline.json` `content`), and none
  is present in this container. The counts therefore have not been observed
  in a running game, and no baseline numbers exist yet.
- **No budget rows.** The rows for `quality/budgets/render-v1.json` are set
  from the first measurement of the corpus screens. They are not invented
  here.

### Next

1. On a machine with the build and content, record the corpus screens' counts
   with `vgui_surface_stats`, on desktop and the Fold7.
2. Set the budget rows from those counts.
3. Then take the remaining V0 inventories: the `ISurface` callers, the
   direct-render panels, and the blend modes and primitives in use.

## V0: fixture materials (`vgui.fixture-materials`), 2026-10-03

User request: fixture materials to draw VGUI with, so the corpus does not
depend on licensed game content.

### What is installed

- **`tools/vgui/vgui_fixture_content.py`.** It draws 21 textures from
  closed-form rules (no image library, font or third-party art) and writes 43
  files (21 VTFs and 22 VMTs, about 380 KiB). The files are generated and
  never committed. The set:
  - nine `vgui/fixture/*` materials: quadrants, alpha ramp, additive, opaque
    with zero alpha, a point-sampled checker, a sub-rect atlas, a three-frame
    `$frame` texture, a non-square ramp, and a `$color`-tinted material for
    the draw list's legacy-material case;
  - the thirteen `vgui/cursors/*` textures the surface loads at start.

  The [fixture README](../quality/fixtures/vgui-surface/README.md) lists what
  each one exercises.
- **`quality/fixtures/vgui-surface/materials.json`.** The recorded manifest:
  each material's purpose, size, flags and blend, its probe texels, and every
  file's SHA-256 and size. `record` refuses to replace it without `--update`.
- **`tools/quality/vtf_write.py`.** The one VTF writer for generated art,
  moved out of `tools/android/touch_icons.py`. The touch icons it writes are
  byte-identical before and after the move (19 files), and the Android tests
  pass.
- **`unittests/vguitest/vgui_fixture_reader.cpp`.** It reads the probe
  texels through the engine's VTF container reader (`texturecontainer::vtf`,
  which `vtf/vtf.cpp` loads every game texture through).

### Evidence

| Check | Result |
| --- | --- |
| `vgui.fixture-materials` | 389 checks on g++ and clang++. The files regenerate byte-identically to the manifest. Every VMT, read by `tools/render/material_inventory.py`, names its texture and declares the blend the surface derives from its flags. Every VTF header is as specified, and all 78 probe texels read back through `tools/quality/vtf_decode.py` and through `texturecontainer::vtf` |
| `test_vgui_fixture_content.py` | 12 tests. With the manifest re-recorded from each defective output, so only the probes and the blend rule can catch it, these each fail: flipped rows, swapped channels, reversed frames, a blend the flags do not give, and wrong header flags. Changed bytes fail the hashes, and a missing file fails its presence check. The engine-reader test (slow tier) passes and reports a flipped texture itself |
| Determinism | two `write` runs are identical |
| stylelint `--changed`; archlint `check --all`, `inventory --verify` | pass |

### Not done

- **The materials have not been drawn.** Drawing them needs a host that
  composes the material system, vgui2 and vguimatsurface against a fixture
  game directory, with a fixture scheme and packaged fonts. That host is the
  next V0 step, and it is where `vgui_surface_stats` gets its first numbers.
- **No D3D9 or native Vulkan pixel captures.**
