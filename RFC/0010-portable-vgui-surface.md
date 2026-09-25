# RFC 0010: Portable VGUI Surface, Text, and Input

- Status: Proposed (2026-09-23); no implementation gate complete. V1's UI
  scale owner is installed, but its exit gate is not met
  ([2026-09-25 update](#update-2026-09-25))
- Date: 2026-09-23
- Scope: The machinery beneath VGUI's frozen panel/control API: the 2D draw
  path in `vguimatsurface`, UI scale and coordinate spaces, font discovery and
  rasterization, VGUI input and text entry, optional VGUI capabilities (HTML,
  in-world panels), and VGUI's composition and product scope
- Platform, render and input seams: [RFC 0001](0001-capability-based-platform-architecture.md)
  owns render providers, presentation bridges, window/input contracts (rank 7,
  R14), and typed module composition (rank 18, R39)
- Editor UI: [RFC 0002](0002-hammer-responsibility-factorization.md) owns
  Hammer's GTK host; this RFC does not make VGUI an editor toolkit
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006](0006-modern-cpp-ownership-and-synchronization.md)

## Decision and boundary

VGUI stays the game's UI toolkit. Its public panel and control API, the
versioned interfaces game modules consume, and its authored data (`.res`
layouts, schemes, localization files) are preserved. Modernization happens
**beneath** that API, as a set of replaceable seams with a working consumer at
each step:

1. a recorded, backend-neutral 2D draw list between the surface and the
   renderer;
2. one owner for UI scale and the UI-unit/pixel mapping;
3. portable font discovery separated from shared FreeType rasterization;
4. VGUI input and text entry fed from normalized platform events;
5. optional capabilities (HTML, in-world panels) negotiated explicitly;
6. typed composition and a product-scoped set of VGUI modules.

This is not a rewrite and not a UI framework replacement. A different menu
technology may later become a second provider, as a separate product decision
after the seams exist (see [Alternatives](#alternatives-considered)).

## Observed starting point

Facts observed in the tree at 2026-09-23; they are not implementation claims.

- **Frozen API surface.** Game and tool modules reach VGUI through versioned
  interfaces: `VGUI_Surface030` (`public/vgui/ISurface.h`, about 148 virtual
  methods), `MatSystemSurface008`, `VGUI_Panel009`, `VGUI_ivgui008`,
  `VGUI_Scheme010`, `VGUI_Input005`, `VGUI_InputInternal001`, `VGUI_System010`
  and `VGUI_Localize005`. About 150 first-party `.cpp` files call
  `vgui::surface()`, `g_pVGuiSurface` or `g_pMatSystemSurface`; 113 of them are
  in `game/client`. `vgui_controls` (75 sources) is a static library linked into
  each consumer.
- **Immediate-mode drawing.** `CMatSystemSurface` (`vguimatsurface/`) draws each
  `DrawFilledRect`, `DrawOutlinedRect`, `DrawLine`, `DrawTexturedRect`,
  `DrawTexturedPolygon` and similar call by obtaining a dynamic `IMesh` for the
  current material (`InternalSetMaterial`), filling it with `CMeshBuilder`
  (`MATERIAL_QUADS`, `MATERIAL_LINES`, `MATERIAL_POLYGON`) and calling `Draw`.
  Text is the one batched path (`MAX_BATCHED_CHAR_VERTS`). `Clip2D` clips
  vertices on the CPU against a scissor rectangle. `StartDrawing`/
  `FinishDrawing` push the render context's matrices; `StartDrawingIn3DSpace`
  paints a panel tree into the world.
- **Textures are materials.** `TextureDictionary` maps VGUI texture IDs to
  `IMaterial`s, including procedural RGBA textures and named material files
  with proxies and frames (`DrawSetTextureFrame`, `IVguiMatInfo`).
- **Direct rendering inside `Paint`.** Some panels use the material system's
  render context directly while painting: at least 1 source in `vgui_controls`,
  4 in `matsys_controls` and 12 in `game/client` combine `Paint` with
  `CMatRenderContextPtr`/`GetRenderContext()` (a coarse grep; V0 owns the exact
  inventory).
- **Native Vulkan VGUI.** The native backend renders menus, console, HUD and
  fades through this path after four contract fixes, but
  [there is no D3D9 pixel capture of VGUI](0001-native-vulkan-progress.md#vgui-on-native-vulkan-menus-console-hud-and-fades-2026-09-22)
  and `MATERIAL_LINES` is still dropped by native, so `DrawLine`/`DrawPolyLine`
  would not draw there.
- **UI scale in progress.** Uncommitted work adds `vguimatsurface/UIScale.{h,cpp}`
  (back buffer pixels per UI unit from display scale, user `ui_scale`, back
  buffer and drawable sizes), UI-unit layout and cursor exchange in
  `CMatSystemSurface`, font raster scaling, and
  `ILauncherMgr::GetWindowDisplayScale`. The `vgui.ui_scale` and
  `vgui.ui_scale.sensitivity` suites are registered in
  `quality/conformance.manifest.json`. The back buffer is the video mode's size
  and is presented scaled ([record](0001-native-vulkan-progress.md#the-back-buffer-is-the-video-modes-size-presented-scaled-2026-09-22)).
- **Fonts.** `vgui2/vgui_surfacelib` holds `FontManager`, `FontAmalgam`,
  `BitmapFont`, `Win32Font` (GDI, Windows only) and `linuxfont` (FreeType plus
  fontconfig for every non-Windows target). `osxfont.cpp` is commented out of the
  wscript, so macOS would build the fontconfig path. On Android, `linuxfont`
  maps a few Windows face names and languages to files under
  `getenv("APP_DATA_PATH")/files`, falling back to named `/system/fonts` files.
- **Input.** The engine forwards `InputEvent_t` to
  `IMatSystemSurface::HandleInputEvent` (`engine/vgui_baseui_interface.cpp`,
  `engine/sys_mainwind.cpp`). `vguimatsurface/Input.cpp` also translates Win32
  messages, including IME composition (`WM_IME_*`) into VGUI event types.
  `TextEntry` calls `g_pInputSystem->StartTextInput()`; the SDL3 manager
  decides whether to show a screen keyboard. The client's touch controls
  (`game/client/touch.cpp`, `CTouchPanel`) are a gameplay VGUI panel, not a
  VGUI input mechanism.
- **HTML.** `vgui_controls/HTML.cpp` drives Steam's `ISteamHTMLSurface` (CEF
  hosted by the Steam client). Its game consumers are the team menu and the
  MOTD text window (`game/client/game_controls`). No CEF is built or linked by
  this tree; `devtools/bin/updatecef.*` are orphaned Valve scripts.
- **Composition.** `appframework/VguiMatSysApp.cpp` names `vguimatsurface.dll`
  and `vgui2.dll`; the engine loads GameUI as a module. Panorama exists only as
  headers under `public/panorama`; there is no implementation.
- **Tests.** Apart from the UI-scale suites there is no VGUI conformance suite,
  call-stream capture, or VGUI pixel fixture.

### Update (2026-09-25)

The list above is the 2026-09-23 starting point. Since then:

- The UI-scale work is committed. `ui_scale` is an archived engine ConVar
  (`engine/vgui_baseui_interface.cpp`). The
  [record](0001-native-vulkan-progress.md#vgui-and-hud-follow-the-sdl3-display-scale-2026-09-23)
  reports `vgui.ui_scale` (28 checks) and `vgui.ui_scale.sensitivity`
  (6 checks) passing, and isolated sway captures at 1.0, 1.5 and 2.0. It does
  not cover X11, macOS, iOS, Android, D3D9/DXVK or SDL2. V1 also needs pixel
  and hit-test round-trip fixtures at three scales, a mid-session scale
  change, and a check that no second px/unit conversion exists; none is
  recorded.
- Portal 2 retail fonts: `common/vgui_surfacelib/ValveFont.h` decodes `.vfont`
  files for `CMatSystemSurface`, with the `vgui.valvefont` suite (Q-CONTENT).
  `vgui2/src/Scheme.cpp` skips `"isproportional"` entries inside font blocks.
  This is a font format fix, not the V4 font source provider.
- Native Vulkan still drops `MATERIAL_LINES` and `MATERIAL_LINE_STRIP` draws
  (`shaderapivulkan.cpp`, "draw dropped: line/point topology").
- Native Vulkan applies `OverrideDepthEnable`, so in-world VGUI screens
  depth-test as on D3D9
  ([record](0001-native-vulkan-progress.md#forced-depth-test-for-vgui-screens-in-the-world-2026-09-24)).

## Goals

- Keep every existing VGUI consumer, layout and scheme working unchanged on the
  profiles that support them today.
- Make VGUI rendering backend-neutral and testable without a GPU, and let the
  native Vulkan backend render it through a declared contract rather than by
  emulating D3D9 shader state per draw.
- Batch 2D drawing with measured draw-call and frame-time improvements, without
  changing paint order or pixels on a given backend.
- Give UI scale, font discovery and text input one owner each, portable to
  Linux, macOS, iOS and Android.
- Make desktop-only or Steam-only VGUI behavior optional and explicit, so mobile
  products compose without hidden requirements.

## Non-goals

- Replacing VGUI, `vgui_controls`, `.res` layouts or schemes.
- Embedding CEF or any browser engine. A store-targeted mobile build must not
  depend on JIT or a downloaded runtime (AGENTS.md).
- Reimplementing Panorama.
- Using VGUI for Hammer (RFC 0002's GTK host owns editor UI).
- Complex text layout (bidirectional text, full OpenType shaping) in the first
  gate. It is an [open decision](#open-decisions-and-required-evidence).
- Rewriting callers of `vgui::surface()` for style. Callers migrate only when a
  named retirement condition needs it.

## Layers and owners

| Layer | Owner | Contract after this RFC |
| --- | --- | --- |
| Panel tree, controls, schemes, layouts | `vgui2/src`, `vgui2/vgui_controls` | Unchanged public API and data formats |
| Surface: paint traversal, state, clipping, recording | `vguimatsurface` (`CMatSystemSurface`) | Implements `ISurface`/`IMatSystemSurface`; records a UI draw list |
| UI scale and coordinate spaces | `vguimatsurface/UIScale` | Sole px/unit policy; surface and input route through it |
| UI draw-list execution | A UI draw consumer chosen by the composition root | `ui_draw_list` contract; null, material-system and (optionally) native consumers |
| UI textures and glyph atlas | Surface texture dictionary | Stable IDs; storage outlives any recorded list that references it |
| Font discovery | A per-profile font source provider | Face name, style and language to font bytes, or a structured failure |
| Font rasterization and metrics | `vgui_surfacelib` (FreeType) | Shared across non-Windows profiles; GDI retained for the Windows profile |
| Input and text entry | R14 normalized events; VGUI input translator | Pointer, key, text, composition and focus events in UI units |
| Optional capabilities | Named providers (HTML, in-world panels) | Negotiated at composition; absence is explicit |

## UI draw-list contract

`CMatSystemSurface` keeps its `ISurface` behavior but records drawing into a
**UI draw list** instead of issuing meshes directly. A consumer executes the
list. The contract is a narrow, value-type interface in a strict first-party
header; the surface and consumers may be migrated C++20 targets while the
public VGUI headers stay in their legacy dialect.

A list is an ordered sequence of commands, each with:

- a primitive: triangles (quads and polygons triangulated by the recorder),
  lines, or a line strip;
- vertices in back buffer pixels after UI scale (position, UV, vertex color as
  RGBA8 with a declared byte order);
- a clip rectangle, or the result of CPU clipping (see below);
- a UI texture reference (texture ID plus revision) or none;
- a blend mode from a closed set (opaque, alpha, additive, and the modes the V0
  inventory finds in use);
- a **legacy material** escape: an `IMaterial` reference for draws whose look
  depends on material behavior (proxies, frames, arbitrary shaders). Only a
  consumer that declares the material-system capability can execute it.

The recorder also emits **barriers**. A barrier forces execution of everything
recorded so far before code outside the surface touches the render context. It
is emitted around `StartDrawingIn3DSpace`, render-to-texture panels, and every
panel the V0 inventory identifies as rendering directly in `Paint`.

### Consumers

| Consumer | Purpose | Declares |
| --- | --- | --- |
| Recording (null) | Headless tests, call-stream and command capture | All primitives; legacy material recorded but not rendered |
| Material-system, immediate | Default until V3; reproduces today's behavior one command at a time | All primitives, legacy material |
| Material-system, batched | Coalesces adjacent commands with identical texture, blend, clip and primitive state | All primitives, legacy material |
| Native Vulkan UI (optional, V6) | Direct 2D pipeline on the native backend | Declared primitives and blend modes; legacy material only through the material-system path, or an explicit composition failure |

### Obligations (LSP)

Every consumer that claims the contract:

- executes commands in recorded order; batching may merge only adjacent
  commands with identical state and never reorders across a barrier;
- draws lines and line strips as lines (no silent drop);
- honors clip and blend exactly as declared;
- does not release or recycle a referenced texture, atlas page or vertex
  storage before its completion token allows it (RFC 0006 publication and GPU
  completion rules); the surface defers texture deletion accordingly;
- fails composition, not rendering, when a required primitive, blend mode or
  the legacy material capability is unavailable;
- accepts an empty list, a list of only barriers, and zero-area clip rectangles.

The shared suite runs against every consumer, including deliberately bad ones
that reorder, drop lines, ignore clip or blend, merge across a barrier, and free
a texture early. Each bad consumer must fail a named check.

### Clipping

`Clip2D` CPU clipping stays in the recorder for V2 and V3, so immediate and
batched modes see identical vertices. Replacing it with hardware scissor is a
later, separately measured change: it can alter rasterization at clip edges,
so its fixtures use a declared per-edge tolerance rather than exact equality.

## Scale and coordinate spaces

Three spaces are named and never mixed implicitly:

- **UI units**: where panels lay out, paint and receive cursor positions.
  Proportional layouts continue to scale from screen height; pixel-authored
  layouts grow with the UI scale.
- **Back buffer pixels**: where the draw list's vertices live. The back buffer
  is the video mode's size.
- **Drawable pixels**: the window's presented extent, owned by the presentation
  bridge (R16). The surface does not read it directly; it receives it through
  the scale inputs.

`uiscale::ComputeScale` is the only policy that maps between the first two;
glyph rasterization size, hit testing, texture subrect math and input
conversion route through it. Scale changes (display change, rotation, user
setting) are events: the surface re-evaluates the scale, invalidates the glyph
cache at the new raster size and notifies panels through the existing
screen-size-changed path. The installed `UIScale` owner is V1's starting point.

## Fonts

Split the current per-OS font classes into two roles:

- **Font source provider** (per profile): resolves a scheme's face name,
  weight, italic flag and current language to font bytes. Providers:
  - fontconfig (Linux desktop);
  - packaged fonts (Android, iOS, macOS; also usable on Linux for
    reproducible fixtures): a versioned manifest in game content mapping the
    face names schemes use (for example `Courier New`, `Tahoma`, `Verdana`) and
    language fallbacks (CJK, Thai) to bundled files, with licenses recorded;
  - GDI, retained unchanged for the Windows compatibility profile.
  Storage locations come from the platform paths provider (R11/R26), not from
  `getenv("APP_DATA_PATH")` or hardcoded `/system/fonts` names.
- **Rasterizer and metrics** (shared): FreeType renders glyphs at the pixel
  size (UI size times UI scale) and reports metrics in UI units. `FontEffects`
  (blur, outline, drop shadow, scanlines), `BitmapFont` and `FontAmalgam`
  ranges stay shared.

A missing face follows an explicit, recorded fallback chain. A product declares
its required faces; a package missing one fails its package check rather than
shipping with a silent substitute. Glyphs go into the existing font texture
cache, which becomes an ordinary UI texture in the draw list; sub-rectangle
uploads keep the native fixes already recorded.

## Input and text entry

- VGUI receives pointer, key, character, composition and focus events from
  R14's normalized event source. The translator converts pointer positions to
  UI units through `uiscale`.
- The primary touch in a menu context maps to pointer press, move and release,
  with no hover state while no touch is down. Multi-touch gestures stay with
  gameplay input (`CTouchPanel`) and product policy.
- Text entry uses the provider's text-input API: SDL3 text input and editing
  events map onto VGUI's existing IME event types
  (`IE_IMEStartComposition`/`IE_IMEComposition`/`IE_IMEEndComposition`), and
  `TextEntry` focus requests the screen keyboard through the provider. Win32
  `WM_*` translation remains private to the Windows profile.
- Backgrounding and focus loss release captured pointers and pressed keys and
  end any composition, without delivering synthetic clicks.

## Optional capabilities

- **HTML panels.** A capability with the Steam HTML surface as its first
  provider. Without a provider, `HTML` panels report unavailability through the
  existing load-error path, and products decide per consumer whether to hide
  the feature (MOTD) or fail composition (a required HTML feature). No product
  embeds a browser engine for VGUI.
- **In-world panels** (`StartDrawingIn3DSpace`). They require the legacy
  material capability until a consumer declares a world-space UI path.
- **Native window integration** in `ISurface` (title, tray notify icon, window
  flashing, cursor capture): routed to the window provider's optional
  capabilities, each explicitly supported or unsupported per profile.

## Composition and product scope

- `vgui2` and `vguimatsurface` are created through typed, linked factories
  under R39, so iOS links them statically. Filename lookup of VGUI modules
  (`VguiMatSysApp.cpp`) is retired for migrated products.
- `vgui::surface()`, `g_pVGuiSurface` and `g_pMatSystemSurface` remain as legacy
  accessors that the composition root assigns. They get no new consumers; new
  code receives what it uses.
- Tool-only libraries (`dme_controls`, `matsys_controls`, Perforce controls) are
  absent from client and mobile link maps; the dedicated server links no VGUI
  surface or font code (R12). The module manifest enforces this, with seeded
  violations rejected.
- The orphaned CEF update scripts are deleted; staged retail `libcef.so` and
  `chromehtml.so` are dropped from product runtimes once no consumer needs them.

## Compatibility

Preserved without a versioned decision:

- the interface versions and vtable layouts listed in the observed starting
  point;
- `.res`, scheme and localization formats, proportional layout semantics, and
  font effect flags;
- paint order, popup order, and the timing of `Paint`, `PaintBackground` and
  `OnThink` relative to engine rendering;
- the Windows (GDI) font path and Win32 input translation on the Windows profile.

Changes that alter pixels on an existing backend (hardware scissor, a native UI
pipeline, different glyph rasterization) need a fixture with a declared
tolerance and a recorded decision. The native backend's D3DCOLOR vertex-color
handling and the `vertexlit_and_unlit_generic` modulation rules remain the
reference for the material-system consumer.

## Validation and acceptance

Harness families: Q-PRESENTATION (draw list, pixels, scale, lifecycle),
Q-FOUNDATION (font source, input translator contracts), Q-ARCH (link maps,
module boundaries) and Q-PRODUCT (installed menus, console and HUD). Required
evidence:

1. **Fixed-screen corpus.** Main menu, options dialog, console with colored
   output, a text entry with composition, the HUD crosshair, an intro fade, a
   MOTD/HTML panel where supported, and a scaled layout at 1.0, 1.5 and 2.0.
   Each has a normalized `ISurface` call stream, a draw-list capture, and
   pixel captures on D3D9/DXVK (reference) and native Vulkan.
2. **Mode equivalence.** On a single backend, immediate and batched execution
   produce byte-identical pixels for the corpus, and identical draw lists.
   Cross-backend comparison uses declared per-screen tolerances; it does not
   demand identical pixels.
3. **Draw-list suite.** Shared positive checks per consumer and the bad
   consumers above; line primitives included.
4. **Scale.** The `vgui.ui_scale` suites plus pixel and hit-test round trips at
   each fixture scale, and a scale change during a session.
5. **Fonts.** Metric fixtures (tall, ascent, ABC widths, text extents) per font
   source provider with tolerances; missing-face and missing-required-font
   negative fixtures; glyph pages rebuilt after a scale change.
6. **Input.** Generated event sequences through a headless surface: mouse,
   keys, characters, composition start/update/commit/cancel, touch as pointer,
   focus loss and backgrounding mid-drag and mid-composition.
7. **Lifecycle.** Device or surface loss and recreation (Android, resize,
   minimize) restores UI textures and the glyph atlas; no referenced resource is
   freed before its completion token; shutdown with a recorded but unexecuted
   list is clean.
8. **Budgets.** Draw calls and CPU/GPU time for the UI on the corpus screens,
   set before optimization, per declared profile, including a low-end Android
   device.
9. **Composition.** Link-map evidence that mobile and dedicated products omit the
   declared modules; iOS startup with empty module-search locations.

Missing D3D9 reference captures, devices or profiles leave the corresponding
claim unverified under AGENTS.md.

## Delivery plan

| Phase | Deliverable | Exit gate |
| --- | --- | --- |
| V0 | Baseline: `ISurface` method inventory by caller and category (paint, texture, font, window, input, HTML, 3D); direct-render panel inventory; fixed-screen corpus with D3D9/DXVK and native captures; UI draw-call counts; native `MATERIAL_LINES` support | Captures reproducible from recorded commands; line fixture draws on native; a seeded missing-line or reordered-draw defect is detected |
| V1 | UI scale owner (the installed `UIScale` work) | Scale suites, hit-test and pixel round trips at three scales; scale change mid-session; no second px/unit conversion outside `uiscale` |
| V2 | Draw-list contract, recorder, recording and immediate material-system consumers, shared suite | Immediate mode is pixel-identical to the pre-V2 surface on the corpus; bad consumers fail; no `ISurface` change |
| V3 | Batched consumer and barrier audit | Byte-identical to immediate on each backend; measured draw-call and frame-time change against V0 budgets; default chosen from measurement with rollback |
| V4 | Font source providers and shared rasterizer; packaged-font manifest | Metric fixtures and negative fixtures pass on Linux (fontconfig and packaged) and Android; Apple profiles when available; GDI unchanged on Windows |
| V5 | Input and text entry on R14 events | Generated sequences, composition and screen keyboard on Linux and Android; Win32 translation confined to Windows |
| V6 | Optional native Vulkan UI consumer | Passes the shared suite and corpus tolerances, measured cheaper than the material-system path, or is not adopted |
| V7 | Composition and product scope: typed VGUI factories, HTML capability, tool-only exclusion, CEF leftovers removed | Link-map and empty-search-path evidence; HTML absent and present behaviors tested; no filename lookup for VGUI in migrated products |

V0 and V1 can proceed now. V2–V3 depend on R15's render seam. V5 depends on
R14's event contracts. V7 depends on R06 and proceeds with R39's cohorts. V4 can
proceed independently of the renderer work.

## Roadmap

Not yet ranked. AGENTS.md owns ranks, states and row IDs; this RFC proposes
the following rows for the user to rank. The first draft used R65–R69, but
AGENTS.md has since assigned R65–R66 to RFC 0012 and R67 to RFC 0013, so the
rows below use local labels until AGENTS.md assigns IDs:

| Proposed row | Scope | Prerequisites |
| --- | --- | --- |
| VG-A | VGUI baseline and UI scale owner (V0–V1) | R02, R15 |
| VG-B | UI draw list and batched execution (V2–V3; V6 optional) | VG-A |
| VG-C | Portable fonts (V4) | VG-A |
| VG-D | VGUI input and text entry (V5) | R14, VG-A |
| VG-E | VGUI composition, optional HTML and product scope (V7) | R06, VG-A |

Mobile evidence from VG-B to VG-E contributes to R29 and R36; none of these
rows closes a platform gate by itself.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| A panel renders directly in `Paint` and batching draws its surface content out of order | V0 inventory; barriers around identified panels; immediate mode remains available and is the oracle |
| Batching changes pixels through different vertex or index layout | Same-backend byte-identical gate; merge only identical adjacent state |
| Deferred texture deletion leaks or grows memory | Deletion keyed to completion tokens, with lifetime and leak checks in the suite |
| Packaged fonts change layout because metrics differ from Windows faces | Metric fixtures with tolerances; per-face manifest reviewed against corpus screens |
| Font licensing for bundled faces | License recorded per file in the manifest; no face ships without one |
| IME behavior differs across SDL3 backends | Composition sequences tested per profile; unsupported composition declared, not assumed |
| Native UI consumer diverges from D3D9 blending | Optional phase with tolerances; material-system path stays authoritative |

## Alternatives considered

- **Replace VGUI with a web UI (CEF, Ultralight).** Large binaries, JIT, store
  and packaging risk, and every `.res`/scheme and game panel would need
  rewriting. Rejected.
- **Adopt RmlUi (HTML/CSS-like, no browser engine) for game UI now.** Plausible
  as a later second menu provider once the draw-list and input seams exist; it
  would consume the same contracts. Not a replacement for VGUI's game-panel API.
- **Dear ImGui.** Suitable for developer overlays and debug tools, which may use
  the UI draw list. Not a game UI replacement.
- **Implement Panorama.** Only headers exist; the runtime would be a new UI
  framework with no content in this tree. Rejected.
- **Emulate everything in the native backend's material path.** This is today's
  working path and remains the compatibility consumer. On its own it keeps
  per-draw material overhead and gives no GPU-free oracle.

## Open decisions and required evidence

1. Which blend modes and primitives are in use (V0 inventory), and therefore the
   closed set the contract declares.
2. Whether batched mode becomes the default on every profile, decided from V3
   measurements.
3. Whether complex text layout (HarfBuzz shaping, bidirectional text) is a
   product requirement for any supported language.
4. Which faces and licenses the packaged-font manifest ships, and whether the
   Linux desktop profile defaults to packaged fonts for reproducibility.
5. Whether `osxfont.cpp` is revived for macOS or Apple profiles use packaged
   fonts through FreeType.
6. Whether the native UI consumer (V6) is worth adopting, decided by measurement.
7. Whether hardware scissor replaces CPU clipping, decided by edge-tolerance
   fixtures and measurement.

## Source references

- `public/vgui/ISurface.h`, `public/VGuiMatSurface/IMatSystemSurface.h`,
  `public/vgui/IPanel.h`, `public/vgui/IInputInternal.h`, `public/vgui/IHTML.h`
- `vguimatsurface/MatSystemSurface.{h,cpp}`, `Clip2D.cpp`, `TextureDictionary.cpp`,
  `FontTextureCache.cpp`, `Input.cpp`, `UIScale.{h,cpp}`
- `vgui2/vgui_surfacelib/` (`FontManager.cpp`, `linuxfont.cpp`, `Win32Font.cpp`,
  `osxfont.cpp`, `wscript`)
- `vgui2/vgui_controls/HTML.cpp`, `TextEntry.cpp`
- `engine/vgui_baseui_interface.cpp`, `engine/sys_mainwind.cpp`,
  `appframework/VguiMatSysApp.cpp`, `appframework/sdl3mgr.cpp`
- `game/client/touch.{h,cpp}`, `game/client/game_controls/vguitextwindow.cpp`
- `unittests/vguitest/`, `quality/conformance.manifest.json` (`vgui.ui_scale`)

## Proposed decision

Accept for planning. Start with **V0** (baseline, direct-render inventory, D3D9
reference captures, native line support) and **V1** (finish the gate of the
UI scale owner, which has landed with its suites). Neither changes VGUI's API or a renderer default. Commit to
batching as a default only after V3's measurements.
