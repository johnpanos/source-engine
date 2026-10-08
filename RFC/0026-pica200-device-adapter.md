# RFC 0026: PICA200 Device Adapter for the Render Core

- Status: Proposed (2026-10-07); P0 to P2 and P6 done in Azahar, P4
  partial; P3 and P5 open. The 3DS client draws only through this adapter.
- Date: 2026-10-07
- User direction (2026-10-07): "start implementing the pica rendercore
  implementation".
- Render architecture: [RFC 0016](0016-render-core.md) owns
  `render.device.v2`, its conventions, capability negotiation, shader
  artifacts and the binding rules. This RFC adds an adapter, one artifact
  format, two capabilities (`kFloatTargets` and `kTextureCompressionETC1`,
  clauses D39 and D40) and the ETC1 formats (decision 4).
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  Q-PRESENTATION. The shared `render.device.v2` suite and its bad adapters
  are the oracle, as for Vulkan (K1), OpenGL (K10), Direct3D 12
  ([RFC 0024](0024-direct3d12-device-adapter.md)) and Metal
  ([RFC 0025](0025-metal-device-adapter.md)), restricted to the clauses the
  adapter's capabilities claim.
- Tracking: no ranked row. Extra product scope (the 3DS client,
  `--n3ds --render-backend=pica`), like TVOS-PROFILE; it adds no north-star
  target and no R29 criterion. Ranking it is the user's decision.

## Why

The 3DS client draws today through `materialsystem/shaderapipica`, a
fullbright legacy shader API over citro3d, with a null render-core device
beside it. That is a frozen legacy path (binding rule 1): new 3DS render work
cannot go there. The core needs a device adapter for the PICA200 so that
render-core passes, `render_lab` and, after R91, the whole frame can draw on
the 3DS through the same port as every other backend.

The PICA200 is not a shader-model GPU. Its vertex stage is programmable
(the PICA vertex shader ISA, assembled by `picasso`), but its fragment stage
is fixed function: six texture-environment (TEV) combiner stages, three
texture units sampled with the vertex shader's texture coordinates, a
combiner buffer, alpha test, stencil, depth and blend. It has no compute, no
storage buffers, no block compression the core uses (ETC1 only), no MSAA
render targets and at most 1024 x 1024 textures. So the adapter cannot accept
GLSL or SPIR-V; it accepts its own artifact form, and its capability set is
the smallest of any adapter.

## Decisions (agent decisions under the user's standing instruction)

1. **Module.** `render.device.pica` in `render/device/pica/` with
   `public/render/device/pica/provider.h`, provider name `"pica"`. The
   execution files (citro3d, libctru) build only for the 3DS
   (`env.N3DS`); the artifact readers and state tables are portable and
   build for every host so the host suite (`render.device.pica.portable`)
   checks them without a device.
2. **Artifact format `kPica`.** One new `ArtifactFormat` value
   (`render/device/pica/artifacts.h` owns both forms, byte for byte). A
   pipeline's two stages carry different forms under it:
   - vertex: `PVS1`, a header followed by the `picasso` DVLB the stage runs
     (one DVLE). The header maps the port's bindings onto float uniform
     registers: the D16 draw-constant block starts at a named register (four
     floats per register) and each uniform-buffer binding names its first
     register and count; the ranges may not overlap each other or the DVLE's
     own constants (checked when the DVLE is parsed). Vertex attribute
     location n is input register vn; `kUnorm8x4` arrives as 0 to 255. The
     PICA200 has no vertex index, so the header may name an input register
     the adapter feeds with it from its own attribute stream (first vertex,
     or index plus vertex offset).
   - fragment: `PFP1`, a combiner program: 1 to 6 TEV stages (sources,
     operands, combine functions, scales), the combiner buffer, an optional
     alpha test, up to three texture units (the port bindings of each
     texture and sampler), and a specialization table (D20: a fragment
     specialization constant with a given value makes a stage's constant a
     literal). A stage's constant is a literal, a vec4 of the D16 block or a
     vec4 of a bound uniform buffer, packed to RGBA8 when the draw is
     recorded: the PICA200's fragment stage reads no memory, so uniform
     buffers reach it through the CPU.
   The fragment artifact is data: the adapter validates it completely at
   `CreatePipeline` and refuses a malformed one with `kInvalidDescription`,
   never by drawing something else; every binding either form uses must be in
   its stage's reflection (`kLayoutMismatch`).
3. **Conventions (D13), measured in Azahar.** The port's conventions are
   corrected by the artifact compiler, as SPIRV-Cross corrects GLSL, and by
   the replay's register setup:
   - clip depth: a `PVS1` program writes the port's clip z negated (the GPU
     keeps `-w <= z <= 0`) and the device's depth map stores `-z/w`, scaled
     to the viewport's depth range: clip z 0.25 stores 0.25;
   - clip Y and rows: textures and render targets are stored with the
     port's row 0 first; the GPU writes a render target's top row there
     (clip +Y), so no flip is needed; it samples that row at t = 1, so a
     `PVS1` program writes t = 1 - v;
   - viewports and the render area: the GPU's window y runs up from the
     framebuffer's last stored row, so the replay converts the port's
     top-down viewport and scissor;
   - pixel centres are at half pixels, as the port's (an early half-pixel
     correction was a misreading of an emulator bug, below, and is gone).
   Screen rotation (the top screen is a 240 x 400 framebuffer scanned out
   sideways) belongs to the presentation bridge, not the device.
4. **Capabilities and limits.** One optional capability, ETC1 (below), and
   one the port lacked: `Capability::kFloatTargets` (clause D39), float
   colour targets and float depth, which every other adapter claims (GL ES
   with `EXT_color_buffer_float`). The shared suite takes RGBA8 and D24S8
   equivalents where its oracle allows (D13 depth, D21/D17 blends within
   8-bit rounding, D37's second format), checks D27 refuses, skips D24's
   pixels, and a new clause checks the capability is claimed exactly when
   the formats are made. Limits: 1024 x 1024 textures, one colour
   attachment, 12 attribute loaders, one sample. Formats: `kRGBA8Unorm` and
   `kBGRA8Unorm` (stored as the GPU's RGBA8), `kR8Unorm` (stored RGBA8 so it
   samples (r, 0, 0, 1); not renderable) and `kD24UnormS8` (depth copies to
   and from buffers as the port's floats). ETC1 and ETC1A4 are port formats
   under a new `Capability::kTextureCompressionETC1` (clause D40, which the
   PICA claims): the 3DS's compressed textures, 4 and 8 bits per texel
   against RGBA8's 32, which its memory budget needs (source-engine-c2,
   2026-10-07); `FormatCapability` names every format's capability for every
   adapter. Named refusals: sRGB, BC, float
   and every other format; cube, 3D and array textures; MSAA; instancing;
   32-bit indices; line and point topologies; depth bias; alpha to
   coverage; storage bindings; compute; sampling a depth texture or with a
   comparison sampler (creating one is accepted); sampling a texture that
   is not a power of two of at least 8 on a side. Mip levels below 8x8 are
   stored and copied but never sampled (the GPU's smallest).
5. **State mapping.** Port enums map to the GPU's register codes through one
   table owned by the module (`render/device/pica/tables.h`); on the 3DS
   build each code is checked against libctru's `GPU_*` enums at compile
   time. Blend modes the GPU cannot express exactly (none of the seven
   today) would be refused by name.
6. **Execution and completion.** Buffers and textures live in linear memory.
   Encoders record CPU command lists (the shared `render/device/recording.h`
   of RFC 0025 decision 9, copied byte for byte from the main tree);
   `Submit` validates a submission and replays it at once: copies, writes
   and clears on the CPU, passes and draws into one citro3d frame. A CPU
   operation drains the GPU only when it touches a resource a draw since
   the last drain uses; uniform buffers never count, since draws capture
   them. `Submit` drains before it returns, so every token is complete when
   it is handed out; making submission asynchronous is a P5 frame-time
   decision. citro3d is one context per process: the first device
   initializes it and it stays, since citro3d
   does not survive `C3D_Fini` followed by `C3D_Init`; the texture-unit
   descriptions, the placeholder for unused units and the vertex-index
   stream live with it, and a pipeline erased while its program is bound is
   retired until another program is bound (citro3d reads the bound program
   again at the next bind).
7. **Texture layout.** `render/device/pica/texel_layout.*` owns the tiled
   layout and texel packing for the core. `materialsystem/shaderapipica/
   pica_texture.*` still holds the legacy shader API's copy; it is deleted
   with that API (decision 9) rather than migrated first, since the API
   itself retires.
8. **Families: a reduced 3DS material model (user decision, 2026-10-07:
   "yes we need to do a reduced material model").** The core's families
   reach the PICA200 through a reduced model of what its fixed-function
   fragment stage does: a base texture, a lightmap or vertex lighting, a
   vertex colour, a constant tint, alpha test and the port's blend modes,
   combined in at most six TEV stages. Each family declares its reduced
   form; terms the model lacks (per-pixel PBR lighting, normal and
   specular maps, reflections, probes, fog volumes, shadows) are dropped by
   a recorded rule per family, never silently, and a material the model
   cannot express is refused by name. The reduced forms are `kPica`
   artifacts the shader library holds beside the families' GLSL ones;
   judging them is matched game images on the 3DS against the same scenes'
   desktop images, with each dropped term listed, not pixel equality.

9. **Legacy retirement (user decision, 2026-10-07: "once we're at parity
   lets remove the legacy renderer"; then "we need rendercore at runtime
   on the 3ds as the only renderer").** The legacy renderer is the shader
   API's own citro3d drawing (`pica_renderer.cpp`, `pica_texture.cpp`'s
   tiled layouts, the fullbright program and `tools/n3ds/pica_lab`). It is
   deleted: `materialsystem/shaderapipica` keeps the material system's
   `IShaderAPI` as the game's frontend (RFC 0016's legacy frontend) and
   draws through this adapter, which owns citro3d; no borrowed context
   remains. The frontend's own retirement follows the reduced model's
   passes (decision 8, P3).

## Gates

| Gate | Done looks like | State |
| --- | --- | --- |
| P0 | `kPica`; module, provider and Waf target; `PVS1`/`PFP1` readers with full validation and the state tables, host suite with seeded malformed artifacts; the register codes checked against libctru; the `--n3ds` product links the adapter | done (2026-10-07) |
| P1 | Buffers, textures, samplers, bind groups, encoders, `Submit`, tokens, copies and readback; the shared suite's resource, copy, completion, lifetime and loss clauses pass in Azahar | done (2026-10-07; Azahar software renderer) |
| P2 | Pipelines and draws: the shared suite's conventions, D13, D16, D17, D20, D21 and sampled clauses, the adapter's raster checks, and seeded defects caught, in Azahar | done (2026-10-07): every check on Azahar's OpenGL renderer; all but texture LOD (which it lacks) on its software renderer |
| P3 | The unlit and lightmapped families compile `kPica` artifacts and match their Vulkan pixels within a recorded PICA tolerance; refused families named | partial (2026-10-07): the reduced model draws the game (lightmaps, model lighting, self-illumination) on Azahar with a matched desktop camera; the per-family Vulkan pixel tolerance is open |
| P4 | Top-screen presentation (rotation, display transfer) proven against the scanout; `render_lab` draws a BSP2 fixture on the 3DS | partial: the presenter passes in Azahar (2026-10-07); `render_lab` on the 3DS open |
| P5 | Shared suite and frame time on New 3DS hardware; asynchronous submission if frame time needs it | open |
| P6 | Legacy retirement (decision 9): the 3DS client draws only through this adapter, the legacy citro3d renderer and the borrowed context deleted | done (2026-10-07, Azahar): Portal 2's `intro4` demo on `sp_a1_intro4`; hardware and frame time stay P5's |

## Evidence

See the [progress section](#progress).

## Progress

- 2026-10-07: P0 to P2 (the same day).
  - Host: `render.device.pica.portable` (conformance manifest) passes 132
    checks with g++ and clang++: hand-built artifact bytes read back field
    for field and are reproduced by the writers; 30 seeded malformed
    artifacts are each refused by their named rule; blend factors are judged
    by evaluating the GPU's add equation against each port formula, compare
    and stencil codes by the GPU's documented semantics; specialization and
    constant packing; the texel layout (Morton order, padded extents, level
    offsets, sampled levels, copies through the stored form of each format).
    10 hand-seeded adapter defects each failed it (P0, not checked in).
  - Device: `tools/n3ds/build_device_suite.sh` builds the shared
    `render.device.v2` suite with the adapter as a 3DS program
    (`unittests/rendertest/core/device/test_device_pica.cpp`; the vertex
    fixtures are picasso programs in `shaders/pica/`, the fragment fixtures
    `PFP1` written there), plus the adapter's raster checks: indexed draws
    from vertex and index buffers, culling and winding both ways, vertex
    offset, strips, viewport offsets, the render area (scissor), depth
    ordering, a vertex-stage uniform buffer, `kUnorm8x4` vertex colours,
    two texture units modulated, the alpha test, and a vertex-buffer write
    between two draws in one submission. `tools/n3ds/run_device_suite.py`
    runs it headless in its own Azahar namespace (software renderer):
    347 checks, 0 failures, 1 unverified (2:1 minification must sample
    level 1; Azahar's software renderer has no texture LOD). Skipped by
    capability: D24 pixels (float depth), compute, 4x MSAA.
  - Sensitivity, checked in: seven seeded defects
    (`PicaAdapterOptions::Sensitivity`, one build each) are each caught by
    the clause they break: depth not negated (D13), draw constants dropped
    (D16), write masks ignored (D17), blends opaque (D21), uploads upside
    down (sampled), winding reversed (raster culling), CPU writes racing the
    GPU (raster ordering).
  - Found on the way: the local Azahar software framebuffer dropped every
    target's bottom row (an off-by-one in a bounds check added that day;
    fixed by source-engine-b3); CommandEncoder took the empty thread id as
    "no owner", which the 3DS main thread has, so D11 could not see a
    second thread (fixed in the port with an explicit flag);
    `recording.cpp` mixed `unsigned` and `uint32_t` in `std::max`, which
    devkitARM (`uint32_t` is `unsigned long`) rejects (fixed with explicit
    types; the main tree's copy needs the same fix).
  - Composition: `--render-core-device=pica` (3DS builds only) and
    `RenderCore_Create`'s catalog name the adapter; while the legacy PICA
    shader API is in the product the device borrows its citro3d context.
    The `--n3ds` product links the adapter (`./build-3ds.sh build`).
  - Port changes this needed, for the merge with the main tree:
    `Capability::kFloatTargets` (D39) and its claims in the Vulkan and GL
    adapters, the shared suite's D39 gates and clause, the 8x8 sampled
    fixture, the encoder owner flag, the `recording.cpp` types.
  - Host regressions: `render.device.v2.null`, `.sensitivity`, `.gl` and
    `.gles` pass after these changes; `render.device.v2.vulkan` does not
    build in this worktree (`vk_mem_alloc.h` is missing), unrelated.
  - P4, the top-screen presenter (`PresentTopScreen`): a 400x240 region of
    a device texture drawn turned onto the 240x400 screen framebuffer and
    display-transferred; the suite reads the real top-screen scanout (four
    quadrants and two corner pixels in place). The shared presentation
    contract (`render.presentation.v1`) is built on the legacy device; this
    pair-specific presenter is the PICA's until the core has one.
  - ETC1 and ETC1A4 (D40): the port formats, the PICA's tiled block layout
    (checked on the host against the 3DS's block order and byte order) and
    the suite's clause; in Azahar two mips of each round-trip and a
    specification block decodes exactly. 376 checks, 0 failures, 1
    unverified; eight seeded defects caught (the eighth keeps ETC1's byte
    order unconverted).
  - Azahar's OpenGL renderer (Mesa radeonsi): `run_device_suite.py
    --renderer opengl` runs it on a private headless mutter (Wayland, its
    D-Bus session the repository's private one, never the user's desktop):
    376 checks, 0 failures, 0 unverified, the texture-LOD check included;
    eight of eight seeds caught. Two independent PICA implementations (the
    software rasterizer and the OpenGL one) agree on every check.
  - Not run: New 3DS hardware; any family artifact (P3); frame time. RGBA4
    (16 bits per texel) is not a port format yet.
- 2026-10-07: P6, the 3DS client on the core (user direction: "we need
  rendercore at runtime on the 3ds as the only renderer").
  - `materialsystem/shaderapipica/pica_renderer.cpp` keeps its `pica::`
    API and draws through `render.device.pica`: a 512x256 RGBA8 target and
    D24S8 depth in screen orientation (the frame is its top-left 400x240),
    presented by `PresentTopScreen`; one encoder per frame; pipelines cached
    by draw state (the fragment program written per alpha test); D3D blend
    factor pairs mapped onto the port's blend modes, any other pair refused
    and counted; textures as RGBA8, `kETC1Rgb` and `kETC1A4` in the port's
    raster layout (`pica_texture.cpp` now encodes D40's blocks; the device
    tiles). Mesh memory is device upload buffers written in place through
    `MapUploadBuffer`/`FlushUploadBuffer` (provider.h, the frontend's
    bridge; submission is synchronous), with per-frame vertex and index
    rings; a mesh locked while a recorded draw reads it submits the frame
    first (`PrepareWrite`).
  - Deleted: the citro3d renderer, the GPU-tiled texture encoders, the
    legacy fullbright program's citro3d setup, `tools/n3ds/pica_lab`,
    `build_lab.sh` and `n3ds.py lab`, `PicaAdapterOptions::borrowContext`
    and `RENDER_CORE_PICA_BORROW`. The shader API's vertex program is
    assembled by the device's `pica_device_shaders` feature.
  - Fixed in the adapter: the replay split a frame on
    `C3D_GetCmdBufUsage()`, which lagged a game frame into overflowing the
    command list (`svcBreak` in `GPUCMD_AddInternal`); it now splits on the
    live offset (`GPUCMD_GetBuffer`) with room for the largest draw.
  - Evidence: `n3ds.py run --map sp_a1_intro4 --demo intro4 --headless`
    (Azahar, namespace `pica-p5`): verdict ok; frame 121 160 draws, 64,634
    triangles, frame 241 131 draws; 0 ring overflows, 0 refused blend
    pairs, one submit per frame. The same run with `-pica_texture_rgba8`
    gives the same image as ETC1. The user compared the image with the
    legacy renderer's ("yours looks so much better than the legacy
    renderer"); the last legacy capture showed garbled textures. The
    device suite still passes (376 checks, 0 failures, 8 of 8 seeds).
  - Open: New 3DS hardware and frame time (P5); RGBA4 and
    separate S/T wrap modes are not port features (textures the material
    system updates take RGBA8, mixed wrap is clamped).
- 2026-10-07: one device on the 3DS (user goal: "remove the null device from
  the 3ds build - use one device owned by the launcher and handed to the
  game").
  - The 3DS product links no null adapter (`RENDER_CORE_NO_NULL`; its
    `render.composition` has `pica` alone, and the configure step makes
    `pica` the 3DS's core device). `n3ds_main.cpp` no longer passes
    `-norendercore`.
  - The launcher's render core creates the device and hands it to the
    shader API before the material system starts
    (`PicaShaderBackend_BindDevice`, `public/render/device/pica/host_binding.h`);
    the shader API never creates one (Init fails by name without it) and
    releases everything it made on it at shutdown; the launcher unbinds it
    before destroying the core.
  - Evidence: the console logs `Render core: device pica, features
    legacy-stream,present`; `hl2_launcher` has no `render::device::null`
    symbol. `intro4` on `sp_a1_intro4` and the `n3ds_chamber` map run
    (verdict ok) on Azahar's OpenGL renderer, headless on a private
    compositor (`N3DS_RENDERER=opengl`, new), with clean textures through
    frame 481, also with the replay forced to split the command list
    before every draw. The core adds about 3.6 MB of heap (3.9 MB free at
    frame 121).
  - The device suite gained ETC1 and ETC1A4 mipmap sampling checks (level 1
    under 2:1 minification): 380 checks, 0 failures, 0 unverified on
    OpenGL.
- 2026-10-07: memory audit, the reduced model in the game, and two port
  gaps (user goal: "fix the texture corruption and audit all memory
  corruption", then P3, RGBA4 and separate wrap modes).
  - Audit (`92aaacbb6` and after): `Unlock` clamps to what `Lock`
    granted; wide texcoord 0 goes to a side array instead of past the
    vertex record; `ModifyBegin` prepares the write; Box3D allocates
    through tier0 (a null write on refusal); linear-memory guard bands
    (`guardLinearMemory`, on with validation) are checked at submit and
    name the first refused allocation; each texture level's FNV-1a is
    checked at upload (`corrupted` counter). Heap fixes found on the way:
    per-slot stream epochs (recorded views piled up), mid-frame
    `FlushRecording` past 1.5 MB of pending geometry, single-draw batches
    written in place, the material snapshot cache bounded at 64 on the
    3DS, and `CoreArtifacts()` holding no desktop formats in the 3DS build
    (copying them ran the core-only hatch out of memory at the matched
    camera).
  - P3: `EmitToCore` hands VertexLitGeneric meshes to the core; the
    reduced point `kWorldLit` lights a model handed over as world geometry
    per vertex (`surface_lit.v.pica`); material constants start at c8,
    clear of the draw block. `r_core_world 1` and `r_core_dynamic_draws 1`
    are the 3DS defaults.
  - Evidence (Azahar software renderer, headless): the whole `intro4` demo
    runs (verdict ok, 721 frames, 16,372 views drawn, 0 failed, 16,055
    dynamic draws, 0 refused; heap 71-84 MB used of 97 MB, linear free
    9.2-10.6 MB); the matched camera (`setpos -150 64 40; setang 55 0 0`)
    boots with 0 corrupted textures and compares with the desktop capture.
  - D41 (`SamplerDesc::addressV`, absent = `address`): every adapter
    honours it (Vulkan, GL, D3D12, Metal, PICA); the 3DS shader API keeps
    one sampler per S/T wrap pair instead of clamping when they differ.
    Creation and refusal are checked; no pixel check yet.
  - D42 (`Format::kRGBA4Unorm`, `Capability::kPackedRGBA4`, claimed by the
    PICA adapter alone): stored 2 bytes a texel; textures the material
    system updates in place use it (lightmap pages stay RGBA8;
    `-pica_texture_rgba8` restores RGBA8). Portable suite 146 checks,
    0 failures; a seeded 4-byte stride fails the D42 check; the texel code
    is checked against libctru's `GPU_RGBA4`. On the matched camera the
    in-place textures fall from 796 KB to 680 KB; console text draws
    correctly.
  - Open: the P3 per-family pixel tolerance against Vulkan; a D41 pixel
    check; New 3DS hardware (P5).
- 2026-10-07: first `intro4` timedemo on Azahar (user request: "perf test
  the intro4 demo"). No timing result.
  - Method: `n3ds.py --ns pica-p5 run --content
    build-3ds-content/sp_a1_intro4.p3 --map sp_a1_intro4 --timedemo intro4
    --headless --speed 0` (the probe's new `--timedemo` option; Source's
    timedemo reports frames, seconds, fps and ms/frame from
    `Plat_FloatTime`, which on the 3DS reads the emulated system tick).
    Build: the kiln tree `out/portal2-3ds/dev` at `623fc981e`, core world
    and dynamic draws on, software renderer.
  - Result: verdict `stall`. The harness saw no guest progress for 120 s
    after pica frame 121; the core had counted about 65 frames of model
    draws (roughly 2,000 core model draws by then). The timedemo never
    printed its result line, so there is no fps or ms/frame number. Heap
    at frame 121: 67.1 MB used of 97.5 MB, 4.4 MB free; linear free
    10.6 MB; 0 refused draws, 0 corrupted textures.
  - The same demo under `playdemo` ran to the end earlier the same day
    (721 frames, verdict ok), so the stall is specific to timedemo, which
    runs frames back to back. Not yet known: whether the guest hung or a
    timedemo frame took longer than the 120 s window under the software
    renderer.
  - Seen in the stall screenshot: the elevator's video screen shows texel
    noise. The 3DS build has no video provider (`--video-provider=none`),
    so the screen's texture may be shown without ever being written. Not
    yet checked.
  - Even when it completes, an Azahar timedemo approximates CPU cost only:
    the software renderer likely charges no emulated time for GPU work, so
    it is no stand-in for New 3DS hardware (P5).
- 2026-10-07: performance tooling, ARMv6K correctness and worker cores (user
  goals: "optimize Portal 2 on the 3DS emulator to a solid 20-30 FPS"; "find
  3DS bottlenecks almost instantly").
  - Guest-time profiler: the Azahar harness gains `profile start <us>` /
    `profile stop <path>` (a core-0 timing event that records the running
    thread, pc, lr and 64 stack words every N emulated microseconds, or
    idle). `tools/n3ds/guest_profile.py` reports idle share, threads, self
    time with each library function's nearest engine caller, and inclusive
    time counting only stack words that are return addresses (the previous
    instruction is BL/BLX). `n3ds.py profile` runs the intro4 demo and
    profiles it in about 60 s; `n3ds.py gprof` re-reports in about 7 s.
    The harness patch is tracked as `tools/n3ds/azahar-harness.patch`
    (`build_azahar.sh` applies it to a fresh checkout).
  - `n3ds.py --private`: runs inside a private headless mutter on its own
    D-Bus session, where the emulator renders with OpenGL instead of the
    software renderer. A 600-frame demo run went from about 45 min to
    118 s; nothing opens on the user's desktop.
  - The 3DS build was compiled for ARMv7-A with NEON and VFPv4 (a generic
    `-march=armv7-a -mfpu=neon-vfpv4` line in `wscript` overrode
    `N3DS_ARCH`), so the ELF held instructions the ARM11 cannot execute.
    Now ARMv6K/VFPv2 (`-mfpu=vfp`); `common/sse2scalar.h` implements the 81
    SSE intrinsics the engine uses for ARM without NEON (ssemath.h,
    vector4d.h, mathlib/sse.cpp). Only devkitPro's libjpeg-turbo NEON
    routines remain (runtime-dispatched, unused without NEON).
  - Worker threads: libctru's `pthread_create` makes every thread on core 0
    at priority 0x3F, below the main thread, so workers ran only when it
    blocked. `n3ds_pthread_create` (tier0's three thread-creation sites)
    places them on core 2, then core 1, at the creator's priority; the
    exheader allows cores 0-2 (`AffinityMask: 7`, `CanAccessCore2`).
    Verified in the harness's thread list; frame time unchanged, so the
    main thread is the bottleneck.
  - The bottom-screen text console is off by default (`-n3ds_console`
    restores it); `console.log` keeps every line.
  - Measurements (intro4 demo, emulated clock): 2-4 fps (270-620 ms per
    frame); core 0 idle 23 %, the main thread 76.5 %. Hottest by guest time:
    `memcpy` 12 % (a quarter from `std::to_chars`), `EmitToCore` 19.5 %
    inclusive, printf formatting 13.6 %, `WorldPass::State::Mapped` 8.2 %,
    the PICA texture-record map 8.1 %, `Replayer::BeginRendering` 5.6 % (CPU
    clears, one 4-byte copy per pixel). The 20-30 fps goal is open.
- 2026-10-07: the cheap fixes the guest profile named (user direction:
  "disable audio for now, and do the other cheap ones first").
  - Audio off by default on the 3DS (`-nosound` in `n3ds_main.cpp`).
  - Model draws keep each material's variables in the core's text form,
    rebuilt only when a raw signature of the values changes:
    `GetStringValue` formatted every float and vector parameter with
    snprintf on every draw. `CoreMeshDraw::materialRevision` (new, 0 =
    unknown) lets `CoreWorld::QueueMesh` reuse the `WorldMaterial` it built
    and `WorldPass::State::Mapped` reuse the snapshot key
    (`WorldMaterial::revision`), instead of copying every variable and
    formatting the key per draw.
  - Load clears fill tile-aligned regions one tile-row run at a time
    instead of one tiled `memcpy` per texel (`replay.cpp` `Fill`).
  - Effect on the intro4 demo, guest clock, matching 30-frame windows:
    421 -> 238 ms and 401 -> 211 ms per frame (about 1.8x); end-frame
    34 -> 15 ms. Printf formatting fell from 13.6 % to under 2 %. The
    matched camera's image is unchanged. Still 3-5 fps; the top remaining
    costs are `EmitToCore`'s CPU skinning (13.7 % self, plus 55 % of
    `memset`) and `StageUpload` copies.
  - `guest_profile.py --lines F`: the hottest source lines, or without
    line tables the hottest instructions, inside functions matching F.
- 2026-10-07: the portal lab (user goal: "in a separate binary w/ the 3ds
  rendercore figure out novel ways to render portals in high
  performance"). Lab scope only: no product or frontend change, no gate
  closed.
  - What: `render/lab/pica_portals/` (module `render.lab.pica-portals`, top
    render layer beside `render.lab`), a standalone 3DS program on this
    adapter alone. One test chamber (15,344 triangles in 133 chunks, one
    draw each), two facing portals (64 x 112, elliptical, so each shows the
    other), one camera path of five segments (approach, turn in place,
    strafe, close: 6 units from the portal, blue: the other portal), each
    technique over the same 120 frames. Build and run:
    `tools/n3ds/build_portal_lab.sh 24`, then
    `tools/n3ds/run_portal_lab.py --renderer opengl` (Azahar namespace
    `portal-lab`, private headless compositor; results in
    `build-3ds-portals/results/opengl/`: `report.txt`, `frames.csv`, the
    gallery as PNG).
  - Techniques. `stencil` is Portal's own (mask, depth reset, remote scene
    through an oblique near plane, seal) and the image reference.
    `stencil-crop` crops the remote projection to the portal's screen
    rectangle and sets the viewport to it, so the clipper drops everything
    outside it and culling uses the narrow frustum; `-cone` also culls by
    nine planes through the transferred eye and an octagon circumscribing
    the ellipse. `screen-rtt` draws the cropped view into a texture at 1:1
    and samples it with a CPU pre-projected ellipse (w = 1, so affine
    screen coordinates are exact without projective texturing, which the
    port does not have). `plane-rtt` uses Kooima's off-axis projection with
    the portal as the image plane: the texture is a function of the eye's
    position only (not its orientation), needs no oblique clipping (the near
    plane is the exit plane) and maps onto the ellipse with static
    coordinates. `plane-visible` crops that projection to the visible part
    of the portal at about one texel per pixel. `-cached` keeps the texture
    while the eye moves less than one texel's worth (with a guard band of a
    fifth on each side for the visible rectangle), `-recursive` shows a
    portal seen through a portal with its previous (ping-pong) texture, so
    recursion costs no extra pass. `plane-best` is visible + cached +
    recursive + cone; `plane-best-half` the same at half a texel per pixel.
    Every pass is cleared by a quad drawn first (colour, depth and stencil)
    except `stencil-cpu-clear`, which uses the load op. Controls: `none`
    (flat ellipses) and `seed-mirror` (the plane texture mirrored).
  - Oracles. Host: `portal_math_test.cpp`, 27 checks (the transfer, the
    three projections agreeing on 400 exit-room points seen through the
    portal, oblique clipping of the exit wall, the visible rectangle, the
    cone never culling what the ellipse shows, frustum culling, the scene);
    six seeded defects (mirrored texture, no oblique plane, no half turn,
    crop y swapped, visible rectangle cut short, inscribed octagon) each
    fail it; the build script requires all six. Device: 11 checks, all
    pass: `stencil-cpu-clear`, `stencil-crop`, `stencil-crop-cone` and
    `screen-rtt` match the stencil image (worst mean difference 0.001 of
    255); `plane-visible` and `plane-visible-cached` within resampling
    (worst raw mean 3.12, after a 4 x 4 low-pass 0.97, 0.14 % of pixels off
    by more than 48); both controls differ (low-pass 32.7 and 29.3); the
    cached techniques render at most twice while the eye only turns (1 of
    24 frames). The texture-based checks first used a raw mean < 3, which
    the close frame missed (3.02 to 3.12) with no bias (signed mean 0.04);
    the low-pass metric was added to separate resampling blur from
    geometric error, and the controls still fail it by 20 times.
  - Results (Azahar OpenGL renderer, 24 frames a segment, per frame over
    the whole path; CPU time is record + submit, which replays and drains;
    fragments are counted on 7 of 24 frames by drawing every draw again
    with an additive 1/255 and no tests, so they are what the GPU
    rasterizes before depth and stencil):

    | Technique | CPU ms | draws | indices | remote frags | stencil frags | total frags | turn CPU ms |
    | --- | --- | --- | --- | --- | --- | --- | --- |
    | none | 1.82 | 23 | 7,337 | 0 | 0 | 227,819 | 1.77 |
    | stencil-cpu-clear | 10.82 | 143 | 47,712 | 77,885 | 45,356 | 239,941 | 10.93 |
    | stencil | 8.69 | 144 | 47,718 | 77,885 | 45,356 | 335,941 | 8.79 |
    | stencil-crop | 5.20 | 81 | 25,967 | 24,758 | 45,356 | 282,814 | 4.88 |
    | stencil-crop-cone | 5.08 | 78 | 24,861 | 24,710 | 45,356 | 282,766 | 4.80 |
    | screen-rtt | 5.30 | 80 | 25,822 | 24,758 | 0 | 269,770 | 4.96 |
    | plane-visible | 5.27 | 80 | 25,621 | 25,346 | 0 | 270,973 | 4.96 |
    | plane-visible-cached | 4.71 | 70 | 22,610 | 25,089 | 0 | 271,506 | 2.06 |
    | plane-best | 4.66 | 68 | 21,920 | 25,060 | 0 | 271,477 | 2.06 |
    | plane-best-half | 4.66 | 68 | 21,920 | 7,676 | 0 | 241,245 | 2.06 |

    Total fragments include each pass's clear quad (96,000 for the main
    pass); `stencil-cpu-clear` has none because the CPU clears.
  - Findings.
    - The adapter's load-op clear is a CPU fill over every texel: 2.50 ms
      for the 400 x 240 colour and depth (median of 16, isolated), against
      0.40 ms of CPU for a quad that clears colour, depth and stencil on the
      GPU. In the full frame the quad saves 2.1 ms (`stencil` against
      `stencil-cpu-clear`); every pass of every other technique uses it. A
      GX memory fill in the adapter would serve the game too; not done here
      (the adapter is unchanged).
    - Cropping the remote view to the portal's rectangle is exact and halves
      the CPU cost of a portal (stencil 6.9 ms over `none`, cropped 3.4 ms):
      43 % fewer draws and 46 % fewer indices, 68 % fewer remote fragments.
      The cone culls 3 more draws a frame and stays exact.
    - The texture techniques drop the stencil passes (45,356 fragments a
      frame, three full ellipses; 166,277 at close range) for a clear of
      their region; overall fragment counts are within 5 % of the cropped
      stencil's at full density, and half density removes 69 % of the remote
      fragments for a low-pass error of 1.57.
    - The portal-plane texture is the one technique that survives camera
      rotation: while the eye only turns, `plane-best` renders the portal
      once in 24 frames and costs 0.29 ms over `none` (stencil 7.0 ms,
      cropped stencil 3.1 ms). Walking re-renders every frame (each step
      moves the eye by more than a texel's worth), so on the whole path the
      cache saves 19 % of the renders. Recursion through the previous
      texture adds no pass; its image is approximate (the inner view is the
      outer eye's), a raw mean difference 0.13 above the non-recursive one.
    - Rendering the whole portal at a fixed size (`plane-rtt`, LODs to
      256 x 512) blurs when the portal fills the screen; the visible
      rectangle at one texel per pixel fixes that at the same cost.
    - Not measured: GPU time. Azahar charges no emulated time for GPU work
      (the software and OpenGL renderers give identical timings), so the
      fragment and index counts stand for it. Azahar's software renderer
      samples level 0 only; image checks are judged on OpenGL. An earlier
      4-frame build measured 15 to 20 ms for the same CPU clear; neither the
      isolated benchmark nor later builds reproduce it, and it is not
      explained.
  - Open: New 3DS hardware (frame time, GPU time, fill rate: P5);
    stereoscopic rendering (one plane texture per eye, or one shared within
    a texel's parallax); moving objects in the remote room (the cache is
    keyed on the eye only); the game's portals (`r_core_world` frames).
- 2026-10-07: GPU skinning on the PICA200 (user direction: "work on that
  PICA vertex shader for skinning").
  - `kWorldLitSkinned` (`surface_lit_skinned.v.pica` + `skin.pica`): three
    bones per vertex from a 19-bone palette in c35-c91, after the draw's
    ModelLighting; the vertex is `SurfaceWorldVertex` with a bone meaning
    (bone-space position and normal, weights in `lightmapUv`, palette
    offsets in `tangentS`; `kMaxReducedBones`/`kReducedBoneFloats` in
    surface_program.h). `CoreMeshDraw::bonePalette` carries it through
    `QueueMesh` (`DynamicDraw::bonePalette`), the lit draw group appends it
    (`SurfaceProgram::DrawGroup`'s `bonePalette`), and
    `ProgramResolver::SkinnedPipeline` selects the variant (reduced programs
    only; `SurfaceVariant::skinned`, variant key `surface-v3`).
    `EmitToCore` sends bone-space vertices and the palette when the draw's
    bones fit (an unskinned mesh is one bone, its model matrix); more bones,
    or `-pica_cpu_skinning`, keep the CPU path.
  - The per-vertex model lighting now has one copy, `model_lighting.pica`
    (`.proc light`), linked into surface_model, surface_lit and the skinned
    program: pica_programs entries join files with '+', assembled as one
    source (one DVLE, the reader's rule, with every file's constants).
  - `CreateBuffer` zeroes new buffers only with guard bands (no clause
    promises zeroed contents), and `CheckGuards` returns at once without
    them.
  - Evidence: the matched camera renders pixel-identical with GPU and CPU
    skinning (0 of 96,000 pixels differ by more than 4). Guest profile:
    `EmitToCore` 13.9 % -> 7.6 % self; intro4 windows 230 -> 214 ms and
    299 -> 275 ms per frame (with the buffer fixes). An earlier experiment
    that skinned only each draw's referenced vertices was slower and was
    dropped.
- 2026-10-07: model draw copies (user direction: "optimize ... buffer copies
  in StageUpload"; research found each vertex copied about three more times
  after it is produced).
  - `CoreMeshDraw::takeVertices`/`takeIndices`: `QueueMesh` moves the
    frontend's arrays instead of copying them. The batch gather copy does not
    occur for model draws (one draw per slot, written from its own arrays).
    The copy into GPU-visible memory (`StageUpload`, about 5 % of core 0)
    remains; removing it needs an upload area in render.device.v2.
  - `EmitToCore` builds each GPU-skinned vertex once as a complete value
    instead of default-constructing the array and overwriting it.
  - Guest profile: `memcpy` 13.3 % -> 9.9 %, `memset` out of the top list;
    intro4 windows 214 -> 190 ms and 275 -> 245 ms per frame (best window
    18 fps). Matched camera: GPU vs CPU skinning and CPU vs the earlier
    capture differ in 0 of 96,000 pixels by more than 4.
  - Cumulative on the intro4 demo since the first guest profile: frame 181
    window 421 -> 190 ms, frame 331 window 401 -> 245 ms (about 1.6-2.2x).
- 2026-10-07: 16-bit indices for model draws (user direction: "let's do
  16bit"). `CoreMeshDraw::indices16`/`takeIndices16` and
  `WorldView::DynamicDraw::indices16`: `EmitToCore` builds 16-bit
  triangles (its meshes are at most 65535 vertices), the core takes them, and
  a lone dynamic draw uploads and binds them as `kUint16`, so the PICA
  replay no longer allocates, narrows and flushes a 16-bit copy per draw. A
  multi-draw batch widens into its 32-bit gather; other producers keep
  32-bit `indices`. Matched camera unchanged (0 of 96,000 pixels > 4).
  Guest time over the profiled demo stretch fell about 11 % (92,612 against
  104,053 samples at 500 us); windows 151 -> 127, 330 -> 276, 186 -> 166 ms,
  with two windows slower through single-frame spikes (up to 1.5 s) not yet
  explained.
- 2026-10-07: frame spikes found and fixed with the guest profiler.
  - Tooling: each guest-profile sample carries core 0's system tick, the
    game logs every frame over 400 ms (`pica: slow frame N ms ticks A B`),
    and `guest_profile.py --window A B` profiles that frame alone. With core
    0 idle the sampler records the application's main thread as it waits
    (thread id | 1 << 31), so the report says what core 0's idle time waits
    on; samples carry 160 stack words.
  - Finding: core 0 was idle 38-43 % of the demo, 28.7 % of the time with
    the main thread in file reads. A caller census traced them to
    `CPackedStoreReadCache::ReadCacheLine`: the VPK read cache fills a 1 MB
    line per miss and, since the memory audit, keeps one line on the 3DS, so
    the demo's interleaved small reads (texture bits, materials) re-read
    about 0.8 GB of the pack.
  - Fix: on the 3DS the packed store reads exactly the requested bytes
    (`packedstore.cpp`, ReadData), the path the cache falls back to.
  - Effect (intro4 demo, guest clock): the profiled stretch took 24 % less
    emulated time (70,833 against 92,612 samples at 500 us), core 0 idle
    38 % -> 19.7 %, file reads gone from the waits; the frame-211 window
    364 -> 186 ms (its 3.4 s spike now 0.6 s), windows 629 -> 299 and
    127 -> 81 ms. The matched camera's image is unchanged. Remaining waits:
    `GSPGPU_FlushDataCache` per replayed command, about 13 % of core 0.
- 2026-10-07: one data-cache flush per frame. The replay flushed the data
  cache after every CPU write the GPU would read (buffer writes and copies,
  texture uploads, clears and copies, narrowed indices), and
  `FlushUploadBuffer` flushed each upload range. The GPU runs a frame's
  commands only at `PicaDevice::Drain`, and its `C3D_FrameEnd( 0 )` already
  flushes the whole linear heap first, so those flushes were redundant;
  they are removed (`replay.cpp`, `pica_device.cpp`). Matched camera
  identical (0 of 192,000 pixels > 4). The profiled demo stretch took 15 %
  less emulated time (60,036 against 70,833 samples at 500 us), core 0 idle
  19.7 % -> 7.9 %, and the flush waits fell from about 13 % to 0.3 %.
  Remaining top costs: `memcpy` in `StageUpload` 14.5 %, `EmitToCore`
  15.9 %, string hash maps (`memcmp`, `_Hash_bytes`) about 9 %.
- 2026-10-07: model draws read their vertices in place (user direction:
  "have rendercore own it", applied to other backends where it makes sense).
  - Contract: `legacy::CoreMeshStreams` (core_passes.h) names a model draw's
    vertices in buffers of the core's device: the legacy 24-byte record,
    normals and, skinned, weights and palette slots (unorm8x4, slot x 3).
    `ICorePassRecorder::AcceptsMeshStreams` is true when the world pass's
    program reads them (`WorldPass::ReadsMeshStreams`, the reduced model);
    no portable code names the backend. The world pass binds the buffers
    as they are (`DynamicDraw::Streams`) and uploads indices alone; the
    surface program's `SurfaceVariant::meshStreams` selects
    `ReducedVertex::kStreamsSkinned` (the skinned program) or
    `kStreamsRigid` (new `surface_lit_rigid`, its body shared through
    `lit_bones_main.pica`). Variant keys are `surface-v4`.
  - Frontend: model-format static meshes (normals, no lightmap
    coordinates) keep one copy, an in-place buffer of the device the CPU
    writes (`pica::AllocLinear`'s inPlace), which legacy draws bind too
    (no resident copy) and the core reads (`DeviceBufferOf`); normals and
    weights moved there, and `SkinSlots` builds the slots once per content
    revision. Writes go through `PrepareWrite` (a recorded draw reading the
    memory is submitted first); read-only locks no longer count as writes.
    `-pica_no_mesh_streams` restores the built vertices (the A/B switch).
  - Memory: about 10 MB moved from the main heap to linear memory (the
    linear heap is 26 MB, 7.5 MB free in the intro4 demo; the main heap
    ~14 MB free of 89).
  - Effect (intro4 demo, same binary A/B): the profiled stretch took 22 %
    less emulated time (10,398 against 13,400 samples at 500 us);
    `EmitToCore` 12.3 % -> 4.2 % of core 0, `memcpy` 13.7 % -> 9.6 %.
    Matched camera: every pixel away from the physics cube equals the A/B
    reference (the debris props drawn by the rigid streams path among
    them); the cube's resting place and an error-material decal at its
    impact point follow frame timing.
  - Other backends: the contract, the world pass and the core composition
    are backend-neutral; the desktop programs refuse `meshStreams` by name
    (`MeshStreamsPipeline`, reduced programs only), and no desktop frontend
    writes in-place buffers yet (render.device.v2 has no persistent CPU
    mapping; the PICA provider's `MapUploadBuffer` is that bridge here).
