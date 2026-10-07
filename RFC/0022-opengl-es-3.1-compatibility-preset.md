# RFC 0022: OpenGL ES 3.1 Compatibility Preset for the Render Core

- Status: Proposed (2026-10-05); implementation in progress (see
  [Progress](#progress)).
- Date: 2026-10-05
- User direction (2026-10-05): "Let's target OpenGLES 3.1, and track your
  progress in your RFC and implement it."
- Render architecture: [RFC 0016](0016-render-core.md) owns the device port
  (`render.device.v2`), conventions, capability negotiation, shader artifacts
  and the binding rules. This RFC adds a dialect to its K10 OpenGL adapter; it
  changes no port clause.
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  Q-PRESENTATION; the shared `render.device.v2` suite is the oracle.
- Tracking: no ranked row. It is a child of R92 (K10), which owns the GL
  adapter. Ranking it is the user's decision.

## Why

The render core's adapters are Vulkan and desktop OpenGL 4.5. The adapter
contract names no API, so a third target costs an adapter and shader
artifacts, not portable changes. OpenGL ES 3.1 is the lowest API with compute
shaders, storage buffers and image load/store. Lower GLES versions lack those
features, and the Forward+ core is built around them. ES 3.1 reaches Android
devices whose Vulkan drivers are missing or unusable, ANGLE-backed hosts and
embedded GPUs. It is also a check that the port really is API-neutral.

This is a compatibility preset, not a lower-quality profile. The binding rules
still hold: under RFC 0016 rule 7, nothing is cut to meet a budget. A device
that lacks a capability takes the negotiation path RFC 0016 already defines,
or composition fails by name.

## Decisions (agent decisions under the user's standing instruction)

1. **One adapter module, two dialects.** `render.device.gl` gains
   `GlAdapterOptions::api` (`kDesktop45`, `kEs31`) and a second provider
   descriptor, `"gles"`. The replay, validation, ring, program cache and
   resource model are shared. Desktop GL's direct-state-access entry points
   are filled, on an ES context, with bind-to-edit shims (`es_shims.cpp`).
   Where ES semantics really differ (texture clears, readback, enables,
   base instance), the adapter branches explicitly instead of hiding the
   difference in a shim.
   Why: DRY. A second adapter would copy about 4,000 lines of port logic,
   which then drift.
2. **A new artifact format, `ArtifactFormat::kGlslEs310`.** The same
   SPIR-V passes through the same `cross_compile` with
   `--version 310 --es`, with highp default precisions, and is validated by
   the pinned glslangValidator as ES. Every GLSL 4.50 header has an ES twin,
   `<stem>_gles.h` in `::gles`, and the core artifact store carries the third
   format. A row that does not cross-compile to ES is left out of its ES
   header and the store, with the reason in a comment. Its pipeline is then
   refused on an ES device (`kUnsupported`), never silently swapped.
3. **Required for an ES device** (composition fails with
   `kUnsupported` otherwise):
   - an ES 3.1 context;
   - `GL_EXT_clip_control`, for the port's conventions (clause D13). There
     is no shader-patching fallback, because it would move the convention
     into every artifact;
   - per-attachment blend and write mask (ES 3.2, `GL_OES_draw_buffers_indexed`
     or `GL_EXT_draw_buffers_indexed`; D17);
   - indexed draws with a base vertex (ES 3.2,
     `GL_OES_draw_elements_base_vertex` or `GL_EXT_draw_elements_base_vertex`);
   - the flat-slot counts the artifacts use. These are the same checks as
     desktop: 65 uniform-buffer bindings and 64 texture units.
4. **Emulated, not required:**
   - **Base instance.** The program's `SPIRV_Cross_BaseInstance` uniform
     carries `gl_InstanceIndex`'s base. Per-instance vertex buffers are bound
     at `offset + firstInstance * stride`. `GL_EXT_base_instance` is not used,
     so one path serves every driver.
   - **The upload ring.** With `GL_EXT_buffer_storage` it is persistent and
     coherent, as on desktop. Without it, the ring is CPU memory, copied into
     its destination with `glBufferSubData` when the submission is replayed.
     Ring ranges still retire only behind completion tokens (D10).
5. **Capabilities an ES device claims:**
   - `kCompute` and `kStorageBuffers` under the desktop rule: 64 storage and
     image slots, and storage blocks in compute, vertex and fragment stages.
   - `kTextureCompressionBC` is **not** claimed. ES cannot read a compressed
     texture back (D19's readback clause), and mobile parts lack BC. ETC2 and
     ASTC belong to R55.
   - `kTimestamps` is not claimed in the first slice.
     `GL_EXT_disjoint_timer_query` has no query-buffer write, so D23 needs a
     resolve step.
   - `kExternalImages` is not claimed.
6. **Formats ES does not have** fail texture creation with `kUnsupported`:
   BGRA without `GL_EXT_texture_format_BGRA8888`, sRGB BGRA, 16-bit unorm
   without `GL_EXT_texture_norm16`, and float color attachments without
   `GL_EXT_color_buffer_float`. Copies from depth formats to buffers are
   refused, because ES `ReadPixels` cannot read depth.
7. **Context.** EGL with `EGL_OPENGL_ES_API`, 3.1, robust when available.
   EGL keeps one current context per client API, so a `ContextScope` binds
   the API its context was made with.

## Gates

| Gate | Check | Passes when |
| --- | --- | --- |
| E0 Artifacts | `shader_artifacts.py build`; `render_spv` | every device-suite fixture has an ES artifact; each core program row is in `_gles.h` or named as left out |
| E1 Port suite | `render.device.v2.gles` on Mesa (radeonsi and llvmpipe) | every claimed clause of `render.device.v2` passes on an ES 3.1 context, with debug output silent; unclaimed capabilities are reported |
| E2 Sensitivity | `render.device.v2.gles.sensitivity` | each bad GL adapter fails its clause on the ES dialect too |
| E3 Core programs | `render.shaderlib.programs.gles` | every core program in the ES store links on the ES device; the rows left out are listed and stable |
| E4 Composition | `render.composition` with the `"gles"` provider | `render_lab` composes on the ES adapter, or fails by name for each missing capability |
| E5 Device | an Android ES 3.1 device (Fold7 with the Vulkan path disabled, or a device without Vulkan) | E1 and E4 pass on hardware; until then the preset is unverified on mobile |
| E6 Pixels | `render_lab` scenes on ES against the GL 4.5 adapter | within the cross-backend tolerance, as K10's pixel check |

## Gaps known before implementation

- **Slot compaction.** The artifacts use 16 flat slots per group per kind.
  ES 3.1's guaranteed minimums are smaller (48 texture units, 8 storage
  bindings, 4 image units), so some mobile drivers will refuse `kCompute`.
  That forces the negotiation path, which is correct behavior but loses
  features. A per-program compaction would need the artifact form to change,
  so it is a follow-up gate decided on device evidence.
- Storage images that are both read and written in an `rgba*` format are
  illegal in ES. Such rows fail E0 and are listed.
- Geometry, tessellation and layered rendering are not used by the core, so
  ES 3.1's lack of them costs nothing.
- **GPU-driven submission (planned, 2026-10-05).** RFC 0016's
  [phases S1–S8](0016-render-core.md#gpu-driven-submission-plan-2026-10-05-user-direction) need ES fallbacks: no bindless textures (per-material
  groups stay), single indirect draws only (one call per command), no
  indirect count (zero-instance commands), no lazy attachments or async
  compute. Each phase records the ES refusal by name.

## Progress

### Gate status

| Gate | State |
| --- | --- |
| E0 Artifacts | done on Linux: every device fixture has an ES artifact; 48 of 49 core rows do, and `kBounceCompute` is named as left out (restored 2026-10-06 after RPRB v8's cube array and the instanced vertex inputs had dropped the surface rows) |
| E1 Port suite | passes on Mesa radeonsi and llvmpipe, including the CPU ring and an ES 3.1-only context; hosted CI not run |
| E2 Sensitivity | passes: 8 of 8 bad adapters caught, each on its clause only |
| E3 Core programs | passes for the 12 programs the GL suite lists; a sweep over every store row is open |
| E4 Composition | the composition clauses pass on `"gles"`; `render_lab` is Vulkan-only, so its composition is open |
| E5 Device | open: no Android ES run |
| E6 Pixels | partial (2026-10-06): the five core family suites pass on ES and every ES frame is within the recorded cross-backend limit of Vulkan (`render.family.<f>.cross-backend-gles`, with a seeded-origin negative control); `render_lab` scenes on GL/ES remain open |

### 2026-10-06: family pixels on ES, and the render device as a video setting

User request: test the render core fully on OpenGL and OpenGL ES and compare
their frames with Vulkan; boot the game in all three modes, with the device
a video setting in the UI. Linux desktop (radeonsi, Radeon 8060S), g++.

- **Regressions found and fixed.** RPRB v8 (reflection probes as one cube
  array) put `samplerCubeArray` in every surface program; it is reserved in
  core `310 es`, so every surface row silently left the ES store and every
  ES family pipeline failed. The ES artifact now enables
  `GL_EXT_texture_cube_map_array` or `GL_OES_texture_cube_map_array`
  (`tools/render/shader_artifacts.py`), the extensions the adapter already
  claims `kCubeArrays` (D36) on. `surface_model_instanced.vert` had array
  vertex inputs, which ES forbids; they are now one `vec4` per row at the
  same locations, so the Vulkan vertex layout is unchanged. The same RPRB
  change had also left the family suites' frame group binding a 2D view at
  the cube-array binding 9 and nothing at binding 13, so every Vulkan
  cross-backend row failed validation; `SurfaceFrameGroup` now supplies a
  neutral cube array and a count-0 probe buffer.
- **Family pixels on ES.** `RENDERTEST_FAMILY_GLES` selects the GL adapter's
  ES dialect (the device's facts select the ES artifacts). New rows
  `render.family.<f>.gles` and `render.family.<f>.cross-backend-gles` for
  unlit, water, lightmapped, vertexlit and pbr, judged against the same port
  fixtures and `cross-backend-v1.vdf` limits, plus
  `render.family.unlit.cross-backend-gles.seeded-gl-lower-left`, which must
  fail. ES matches Vulkan as closely as desktop GL does: worst channel 28
  levels on the recorded PBR shading-edge case, within its outlier count.
  `render.shader-artifacts.gles` passes again.
- **Render device video setting.** `public/render/composition/render_device_setting.h`
  owns the file (`<game>/cfg/render_device.txt`), the choices (vulkan, gl,
  gles) and the parse (`render.device-setting`, 27 checks). The Portal 2
  Video menu has a "Render device (restart)" row; Apply saves the choice,
  and the launcher reads it before composing the core (`-render-device`
  overrides it). Switching in-process is R97. `play_p2` trees now link the
  GL adapter (`--render-core-gl`) and reconfigure an older tree once.
- **Game boots** (`portal2_map_views.py`, `sp_a1_intro4_relit`, offscreen,
  two cameras): `-render-device vulkan`, `gl` and `gles` each compose
  (`Render core: device <name>`), load the world stage and pass. Against
  the Vulkan run, GL and GLES frames differ by at most 1 level on one view
  and in 106/162 channels on the other; a second Vulkan run differs from the
  first in about 5,000 channels, so these are run-to-run noise. UI-driven:
  the menu row saved `gl` and `gles`, and a relaunch with no argument
  composed the saved device.
- **What this does not show.** In the game, the legacy stream still draws
  the frame through the native Vulkan backend; the GL/ES device runs the
  core's stage passes. The product boot with GL presenting the frame stays
  blocked on R91 and an SDL3–GL presentation bridge (K10 "Product boot"),
  so the identical game frames are expected, not evidence of GL pixels.

### 2026-10-05: slice 1 (artifacts, context, shims and the port suite)

**What changed**

- `ArtifactFormat::kGlslEs310` (`public/render/device/facts.h`).
- `tools/render/shader_artifacts.py`: `cross_compile(..., es=True)` runs
  SPIRV-Cross with `--version 310 --es`. It raises the default float and int
  precision to highp, because SPIRV-Cross defaults fragment floats to mediump
  and the port's math is 32-bit, then validates the result with the pinned
  glslangValidator as ES. A row whose ES artifact fails is left out of its ES
  header and the store, and the reason is written into the header.
- `tools/render/shader_toolchain.py`: `GLES_GENERATED`, an `_gles.h` twin of
  every GLSL 4.50 header. The core artifact store (`core_artifact_table.h`)
  carries the ES entries.
- `render.device.gl`:
  - `GlAdapterOptions::api` and the `"gles"` descriptor (`DescribeEs`).
  - An ES context in `context.cpp`. `ContextScope` binds the context's EGL
    client API and the per-thread ES state.
  - `es_shims.cpp`: the ES entry points and the DSA shims.
  - ES branches in `device.cpp` for requirements, enables, the ring, facts and
    format refusals.
  - ES branches in `execute.cpp`: texture clears through a scratch
    framebuffer; color readback with `glReadPixels`; depth readback with a
    compute copy, because ES `ReadPixels` cannot read depth; per-instance
    vertex buffers offset for the first instance; ring copies from CPU memory.
  - `pipelines.cpp`: an empty fragment stage for depth-only ES programs,
    because ES will not link a graphics program without one.
- The composition catalog (`render/composition/render_core.cpp`) lists
  `"gles"`, and capability masks apply to it.
- Setup now consumes stale GL errors. A limit query that a driver refuses
  falls back to single sampling and does not poison later calls.

**Evidence** (Linux, AMD Radeon 8060S on radeonsi and llvmpipe, Mesa 26.2.3,
g++ 16 and clang++ 22, `conformance.py check`)

- `render.device.v2.gles`: 981 checks pass, which is every claimed clause.
  Claimed: compute and storage buffers. Not claimed: block compression,
  timestamps, external images, aliasing, parallel recording, separate queues
  and ray query. GL debug output reported no messages.
- `render.device.v2.gles.sensitivity`: 19 checks. The control passes, and 8 of
  8 bad adapters fail only their own clause.
- `render.shader-artifacts.gles`: 19 checks. Every listed program links on ES,
  including the depth-only shadow program. A truncated ES artifact fails
  `kInvalidDescription`. The output, skinning and cluster passes create their
  programs.
- `render.composition.capabilities.gles`: 14 checks pass, as does the `gl`
  variant.
- Regressions: `render.device.v2.gl` (1,048), `.gl.sensitivity`,
  `render.shader-artifacts.gl`, `render.device.v2.vulkan` (1,107) and
  `render.device.v2.null` (549) all pass.
- Mesa overrides that check the fallback paths and refusals:
  - `-GL_EXT_buffer_storage`: the CPU ring passes 981 checks, on llvmpipe too.
  - `MESA_GLES_VERSION_OVERRIDE=3.1`: 981 checks pass.
  - With 3.1, removing either the indexed-blend or the base-vertex extensions
    makes the device refuse creation and name the missing requirement.
  - Removing `-GL_EXT_clip_control` is refused by name.
- The Waf build in `build-k10-gl` (`render_spv`, `render_device_gl`,
  `render_composition`) is clean under the strict flags, and stylelint passes
  on the changed lines.

**Findings**

- In Mesa's 3.1 override with `GL_OES_texture_storage_multisample_2d_array`
  removed, `GL_MAX_COLOR_TEXTURE_SAMPLES` is refused, although ES 3.1 core
  defines it. That error was poisoning every later call until the setup fix
  above. The suite now passes except for one debug message: an honest report
  of the driver's refusal.
- `MESA_EXTENSION_OVERRIDE=-GL_KHR_debug` cannot disable the extension. Mesa
  says so, and the attempt breaks depth uploads in a bare probe too. This is
  not an adapter defect.
- `render.composition.capabilities.gl` did not link before this slice. Its
  source list predated the volumetric pass and projector cookies (`1e660c940`),
  and the uncommitted `render/composition/map_media.cpp` work in the shared
  tree. The committed files are added to both variants. `map_media.cpp` is left
  to the session that owns it, and the composition evidence above used it from
  a temporary manifest.
- Three `tools/render/tests` failures predate this slice; they fail
  identically with HEAD's tool files. They are `check_inventory`'s
  `regenerators` argument, the regenerator usage message, and the seeded-byte
  count (41 against 3).

**Gaps and next slices**

1. **E6/E4: `render_lab` on the GL adapter** (both dialects), so ES frames
   can be compared with the GL 4.5 and Vulkan frames of the same scenes.
2. **`kBounceCompute`** reads and writes its `rgba16f` atlas in place, which
   ES forbids. The projected-light bounce pipeline is refused on ES. The fix
   is a pass change (ping-pong targets or an `r32ui` encoding), proven in
   `render_lab` first under the binding rules.
3. **An E3 sweep over every store row**, not only the 12 listed programs.
4. **E5**: run the suite on an Android ES 3.1 driver (Adreno or Mali), and
   record whether it claims `kCompute`. Slot compaction is decided on that
   evidence.
5. Readback of `RG16F`, `RGBA16F` and `R8` uses `glReadPixels` with the
   format's own type. Mesa accepts it; a stricter driver may need a
   readback through the `RGBA`/`FLOAT` pair with a CPU repack.
6. Depth upload into `D32FloatS8` (`DEPTH_COMPONENT` into a depth-stencil
   texture) is not valid ES. No suite uses it; it is a known gap.
