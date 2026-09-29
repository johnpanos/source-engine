# RFC 0014: Render Core and BSP2 Debug Controls

- Status: Proposed (2026-09-25); retargeted to the render core (2026-09-28);
  no implementation gate complete
- Date: 2026-09-25, amended 2026-09-28
- Scope: developer controls for the RFC 0016 render core
  (`cl_render_debug_*`) and for BSP2 maps as loaded (`cl_bsp2_*`). They cover
  debug views, draw identification and bisection, shader reload and capture,
  graph synchronization and resource inspection, and container and per-lump
  inspection. Every control has a tested oracle.
- Renderer: [RFC 0016](0016-render-core.md) owns the core: frame description,
  render graph, scene, draw lists, shader library, device port and adapters
- Shading: [RFC 0007](0007-physically-based-lighting-pipeline.md) owns the BRDF
  (`pbr_brdf.h` and its GLSL mirror) and the reflection probes
- Map data: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md) owns
  the BSP2 container and its lumps
- Indirect light: [RFC 0011](0011-runtime-indirect-lighting.md) owns the
  producers and `r_indirect_*`
- Antialiasing: [RFC 0012](0012-antialiasing-msaa-specular-alpha-coverage.md)
  owns MSAA and alpha to coverage
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)

**Binding render rules (user decision, 2026-09-28).** All rendering, lighting and
material implementation for this RFC is bound by
[RFC 0016's binding rules](0016-render-core.md#binding-rules-for-all-render-work-user-decision-2026-09-28).
The legacy render paths are frozen. New work lands on the render core,
is proven in `render_lab` before any integration, and deletes the old copy
in the change that replaces it. These rules are not negotiable, and only the
user can change them.
Performance gates never block work (rule 7): the effect is made to look
right first, then optimized. Every time, cost or budget check in this RFC's
gates is measured and recorded, and a miss becomes an optimization item,
never a blocker and never a reason to cut an effect.

## Retargeted to the render core (user decision, 2026-09-28)

The first version of this RFC (2026-09-25) put every control in the native
Vulkan backend (`CVulkanContext`, `materialsystem/shaderapivulkan/`), under
the prefix `cl_vk_debug_*`. That backend is now a frozen path (RFC 0016
binding rule 1), and the user decided that the debug controls must be on the
core. Nothing of the first version was implemented. This version replaces
it, and nothing in it is built on a frozen path:

- **State.** The debug state is part of the core's frame description
  (`render.frame.v1`'s `FrameDesc` policy), not device state.
- **Views.** Views are a specialization constant of the core's programs, not
  separate `-DDEBUG_VIEW` modules of backend shaders.
- **Draws.** Draw records, pick and bisection work on the core's draw lists
  and graph passes.
- **Tooling.** Reload, timers, resource views and synchronization checks are
  shader-library, graph and device-port features, implemented by adapters.
- **Names.** The prefix is `cl_render_debug_*`. The core names no graphics
  API, so the `vk` is dropped; the user's `cl_` prefix is kept.
- **Legacy stream.** Draws that still go through the legacy stream (RFC
  0016's legacy stream passes, drawn by the frozen backend) are not
  instrumented. The controls treat each legacy stream pass as one opaque unit
  and can hide it. They add no instrumentation inside the frozen backend.

## Decision and boundary

Render bugs are found today with RenderDoc, environment variables, launch
switches and one-off views that each grew up next to a single feature. This
RFC replaces that with one family of runtime controls on the render core:

1. **One owner of debug state.** A validated `DebugControls` value is part
   of the `FrameDesc` policy. The engine parses the ConVars in one place, in
   the render core host, and puts the value into each frame's description.
   The renderer validates it when the frame is built and applies it to the
   whole frame. No shader, pass or engine module reads a debug ConVar
   directly.
2. **One view catalog.** Every pixel view is a numbered entry of
   `cl_render_debug_view`, with a defined formula. The indirect views and the
   BSP2 pixel views are ranges of that catalog, not separate state.
3. **Shipped pipelines are unchanged.** The view is one specialization
   constant, `kDebugView` (RFC 0016 port clause D20), of every core program.
   Its neutral value is 0, and a view-0 pipeline is the shipped pipeline:
   same module, same constants, same pixels. Pipelines for other views are
   created on first use.
4. **Every control is tested.** Each view has an oracle and a negative
   control in the conformance manifest. Each behavior control has a test that
   shows it has the effect it claims, and that its default has none.
5. **Proven in the lab first.** Every view and every draw control first
   passes in `render_lab` (RFC 0016 K11), with no engine in the process. Only
   then is it wired into the product.
6. **Data versus runtime.** `cl_bsp2_*` shows what a map's lumps contain and
   how the loader used them. The runtime selections that exist keep their
   owners until the core replaces them: `mat_reflection_probes`,
   `mat_reflection_relight`, `r_indirect_*` and `r_probevolume*`.

The controls are development tools. They change no content, format, gameplay
or network state, and they certify no other RFC's gate, though those gates
may use them as switches. They also serve as the tooling of the lighting
proof: the term views (albedo, normal, direct, indirect, specular) are how
`render_lab` isolates the lighting-model terms of RFC 0016 K11.

## Observed starting point (2026-09-25, updated 2026-09-28)

Observed by reading source; nothing was measured for this RFC.

- **Frozen backend tools** (`materialsystem/shaderapivulkan/`):
  - launch switches: `-vkvalidate`, `-vkdebuglabels`, `-vkpassmerge`,
    `-vkemitreuseverify`, `-vkframestats`, `-vkgputimers`,
    `-vkpipelinecache`;
  - the environment variables `SOURCE_VK_SHADER_DIR` and
    `VK_DEBUG_LIGHTMAPPED`;
  - `mat_indirect_view` 0–3 (`world_pbr.frag -DINDIRECT_VIEW`);
  - `mat_reflection_probes` 0–3.

  Under RFC 0016's binding rule 1 these stay as they are. They are deleted
  with the code they belong to, and they gain no new controls.
- **Render core** (RFC 0016, installed):
  - the device port with Vulkan and null adapters, and specialization
    constants (D20);
  - the render graph with `ValidateCompiledGraph`, the pooled executor and
    the `TransientPool`;
  - the frame graph with stage passes and legacy stream passes (K3);
  - the families `unlit`, `lightmapped`, `vertexlit` and `pbr`;
  - `render.pass.world`, with `r_core_world`, `r_core_world_stats`,
    `r_core_world_isolate` and `r_core_world_strict`;
  - `render.pass.lines`, the scene and culling, and the generated SPIR-V
    headers from the pinned shader toolchain.
- **Map container:** `IMapContainer::VerifyContent` checks every lump's
  content hash (`public/mapcontainer/map_container.h`). The independent reader
  is `tools/quality/bsp2_reader.py`.
- **None exist:** a debug view catalog on the core, per-pass GPU timers on
  the graph, draw pick or bisection on draw lists, shader reload in the
  shader library, or resource overlays.

## Goals

- Answer "what is this pixel made of" and "which draw is wrong" without a
  capture, in `render_lab` and in game, on every native profile.
- Separate shader bugs from synchronization and reuse bugs with one switch.
- Make the shader edit loop seconds long: edit GLSL, rebuild with the pinned
  toolchain, reload, and see the result in the same session.
- Show what a BSP2 map carries, and test each optional lump's fallback by
  withholding it.
- Show at a glance what the core draws and what still goes through the
  legacy stream.
- Keep harnesses and people on the same switches, with one owner per switch.

## Non-goals

- Controls on the frozen paths (the native backend, D3D9, DXVK, ToGL, the
  CPU lighting path). They report nothing and gain nothing.
- Per-draw instrumentation inside legacy stream passes.
- Enabling validation or a debug-utils layer at runtime. Both stay
  adapter-creation options.
- New content formats or lump versions. `cl_bsp2_*` reads what exists.
- Shipping debug controls to players. The controls are cheat-protected.
  Whether store builds exclude the debug pipelines and draw records is a
  profile decision (see [Open decisions](#open-decisions-and-required-evidence)).
- Performance tuning. Timers and counters report cost; budgets stay with
  their owning rows.

## Layers and owners

| Layer | Owner | Holds |
|---|---|---|
| ConVar registration and parsing | the engine's render core host (`cl_render_debug_*`); the engine client (`cl_bsp2_*`) | Names, help, flags and ranges, parsed once per frame into one `DebugControls` request in the `FrameDesc` |
| Validated debug state | `render.frame` (`FrameDesc` policy), validated by `render.renderer` | The applied `DebugControls` and the view catalog. Each program declares which views it supports. An invalid value is rejected with a message, and the previous state is kept |
| Frame application | the render sequence, per frame | The value travels in the frame description, so the main thread and the render sequence cannot disagree within a frame, in either queued mode |
| Shader side | `render/shaders/common/debug_view.glsl` | The one include with every view's formula. Programs include it and branch on `kDebugView` at their single output point |
| Draw records, pick, bisection | `render.scene` draw lists and `render.graph` | Per-draw records for core draws, one record per legacy stream pass |
| Reload, timers, resources, sync checks | `render.shader-library`, `render.graph`, `render.device.v2` (adapters) | Portable requests; each adapter implements them or reports them unsupported by name |
| Map-side data | the engine's BSP2 loader and lump consumers | The per-lump load records, consumed or ignored state and fallbacks that `cl_bsp2_*` reports |
| Oracles | the `render.debug-*` and `world.bsp2-debug*` suites, run in `render_lab` first | CPU formulas, pixel suites and negative controls |

## Common rules

- **Flags.** Anything that changes pixels or behavior is `FCVAR_CHEAT`.
  Commands and reports that only print are not.
- **Invalid input fails visibly.** An unknown program name, an out-of-range
  view or an unsupported control prints why and changes nothing.
- **Not-applicable pattern.** A program that can't produce a requested view
  (for example roughness on a lightmapped legacy point) draws a fixed grey
  diagonal hatch (period 8 pixels, 25% and 50% grey). It never draws its
  normal shading, which would look like a valid answer.
- **Legacy stream passes under a view.** While a pixel view is active,
  legacy stream passes are not recorded. The view's targets are cleared to
  the hatch first, so every pixel the core does not draw shows the hatch.
  Deciding which passes run is core plumbing; nothing changes inside the
  frozen backend.
- **Post-processing is bypassed** while a pixel view is active: no tone map,
  bloom, color correction or gamma ramp. Radiometric views output a linear
  value times `cl_render_debug_view_scale` (default 1), encoded as the
  target requires. A view's output is its formula, not the formula after
  tone mapping.
- **Default identity.** With every control at default, the shipped
  pipelines' modules and constants and the pixel suites are unchanged. The
  frame time is measured by an interleaved A/B and recorded; it is a
  performance check and does not block (RFC 0016 binding rule 7).
- **Both threading modes.** Every product control is tested with the render
  sequence on the main thread and off it (`mat_queue_mode 0` and `2`, and
  their successors).

## Debug views (`render.debug-views.v1`)

### Catalog

`cl_render_debug_view <n>` (cheat, default 0). The formula column is the
oracle. **S** is any program with a lighting-model term set (the surface
program and the families); **all** is every core program.

| n | View | Output | Programs |
|---|---|---|---|
| 0 | Off | Normal shading | all |
| 1 | Albedo | Linear base color after texture and modulation, no lighting | S |
| 2 | World normal | `n * 0.5 + 0.5`, the shading normal after normal mapping | S with a normal |
| 3 | Normal map | The decoded tangent-space sample `t * 0.5 + 0.5` | S with a normal map |
| 4 | Roughness | Material perceptual roughness as grey | S with a roughness term |
| 5 | Filtered roughness | RFC 0012's filtered roughness as grey | S with a roughness term |
| 6 | Metalness | Material metalness as grey | S with a metalness term |
| 7 | AO | Material AO times screen-space AO as grey | S |
| 8 | Baked light | Lightmap basis irradiance with albedo 1, times the lightmap scale | S lightmapped |
| 9 | Direct light | The direct-light terms only: clustered, sun, area, projected, with their visibility | S lit |
| 10 | Image-based specular | The probe specular term only | S with a specular image |
| 11 | Screen-space reflections | The SSR contribution only, and its confidence in alpha | S with SSR |
| 12 | Emissive | The emission term | S with emission |
| 13 | UV0 checker | `(floor(8 u) + floor(8 v)) mod 2` as black and white | all with UV0 |
| 14 | Vertex color | Vertex color RGB | all with a color stream |
| 15 | Linear depth | View-space depth / `cl_render_debug_view_range` (default 4096 units), clamped | all |
| 16 | NaN, Inf, negative | Final color before tone mapping: NaN magenta, ±Inf cyan, any negative channel yellow, else luminance × 0.5 as grey | all |
| 17 | Over-range | Luminance before tone mapping above `cl_render_debug_view_threshold` (default 1) in red, else luminance × 0.5 as grey | all |
| 18 | Indirect irradiance | Indirect diffuse irradiance / π (the formula of `mat_indirect_view 1`) | S |
| 19 | Indirect radiance | Indirect diffuse radiance (the formula of `mat_indirect_view 2`) | S |
| 20 | All diffuse | Bake, producer change and unbaked direct light, irradiance / π (the formula of `mat_indirect_view 3`) | S |
| 21 | Shadow visibility | Per-light visibility of the brightest light at the pixel, as grey | S lit |
| 22 | Cluster load | Light count of the pixel's froxel as a heat ramp over 0..`cl_render_debug_view_range` | S lit |
| 23 | Volumetric transmittance | The camera-to-pixel transmittance of the volumetric fog, as grey | all |
| 32 | World batch | A hashed color per world draw batch | world |
| 33 | World material | A hashed color per world material index | world |
| 34 | Lightmap chart | A hashed color per lightmap chart | lightmapped world |
| 35 | Lightmap texel density | Checker at lightmap texel resolution | lightmapped world |
| 36 | Lightmap chart borders | Texels within one texel of a chart border in red; seam bleed shows as red leaking | lightmapped world |

Numbers 24–31 and 37 and above are reserved. Views 18–20 keep the RFC 0011
formulas and oracles of `mat_indirect_view`; they arrive with the indirect
terms on the core (RFC 0016 K11, then K12).

### Modifiers

| ConVar | Default | Effect |
|---|---|---|
| `cl_render_debug_view_program <name>` | empty | Applies the view to one program only. Others draw flat 18% grey and keep their depth. Names come from the program resolver; `cl_render_debug_view_program ?` lists them. Also applies to `cl_render_debug_brdf` and `cl_render_debug_heatmap`. |
| `cl_render_debug_view_scale` | 1 | Linear exposure for the radiometric views (8–12, 18–20) |
| `cl_render_debug_view_range` | 4096 | Divisor for view 15; count range for views 22 and overdraw |
| `cl_render_debug_view_threshold` | 1 | Threshold for view 17 |

### Lighting-model controls

These isolate the RFC 0016 lighting-model terms. They are how `render_lab`
proves each term on its own.

| ConVar | Effect | Oracle |
|---|---|---|
| `cl_render_debug_brdf <n>` | 0 full, 1 diffuse lobe only, 2 specular lobe only, 3 energy compensation off, 4 the split-sum table sample as RG | Each mode equals the matching `pbr_brdf.h` CPU term on the `render.pbr-brdf.glsl` cases; mode 3 reproduces the `no-energy-compensation` negative's furnace loss |
| `cl_render_debug_furnace 1` | Albedo 1 and a uniform environment of radiance 1 replace probes, image-based light and lightmaps; direct lights off | An energy-compensated white metal sphere reads 1 within the R47 furnace tolerance at every roughness; with `cl_render_debug_brdf 3` rough spheres read below it |
| `cl_render_debug_term <name…>` | Turns off the named lighting-model terms: `clustered`, `sun`, `area`, `projected`, `baked`, `probes`, `ibl`, `ssr`, `ao`, `specular_occlusion`, `emission`, `volumetric` | Turning a term off gives exactly that term's neutral value: the frame equals the frame of the same scene without that input, bitwise |
| `cl_render_debug_force_roughness`, `cl_render_debug_force_metalness` | −1 off; otherwise the value, clamped to [0, 1], replaces the material's | The pixel equals the same material authored with that value |

### What the core does not draw

| ConVar | Effect |
|---|---|
| `cl_render_debug_legacy <n>` | 1 draws every legacy stream pass's pixels magenta over the frame (the pass runs, then its written area is tinted by a stencil the core sets for that pass); 2 skips legacy stream passes, leaving the hatch |
| `cl_render_debug_claims` (command) | Prints, per program and per material, what the core claims and each named gap, as `r_core_world_stats` does for the world; it replaces `VK_DEBUG_LIGHTMAPPED`, whose two frozen-backend sites are deleted when this lands |

The magenta mode needs a stencil reference that the core sets around a legacy
stream pass. That is core plumbing: the frozen backend still records the pass
unchanged.

### Overdraw, mip and MSAA views

| ConVar | Effect |
|---|---|
| `cl_render_debug_overdraw <n>` | 1 counts fragments that pass the depth test; 2 counts every rasterized fragment (depth test off). Counts go into an integer graph target through debug pipelines with additive state, then are colorized in one pass over 0..`cl_render_debug_view_range` (default 8 for this view). Core draws only. |
| `cl_render_debug_mip <n>` | 1 base-texture mip level as a fixed color ramp; 2 texel density: texels per pixel below 0.5 blue, 0.5–2 green, above 2 red |
| `cl_render_debug_msaa <n>` | 1 pixels whose samples differ (edge pixels) in white; 2 alpha-to-coverage sample count per pixel (RFC 0012 A1 on the core). Both report unsupported at 1 sample. |

### Implementation

- `render/shaders/common/debug_view.glsl` holds every formula. A program
  includes it and, when `kDebugView` is not 0, calls `DebugViewOutput(inputs)`
  at its single output point. The inputs are a struct of the program's
  lighting-model terms, with a mask of which ones it has; the mask drives the
  not-applicable pattern.
- `kDebugView` is a specialization constant with neutral value 0. The other
  parameters (program filter result, BRDF mode, term mask, overrides, scale,
  range and threshold) travel in a debug block in the frame bind group. It is
  bound only in pipelines whose `kDebugView` is not 0, so the layouts of
  shipped pipelines don't change. The render-core session owns the program
  layouts; the debug block's binding is agreed with it.
- Debug pipelines are created on first use and kept for the map. Their
  creation is excluded from the pipeline-miss log.

### Implementation decisions (D0, 2026-09-28)

Agent decisions by the render-core owner, under the user's standing
instruction. The [progress record](0016-progress.md#rfc-0014-d0-in-the-lab-the-view-catalog-on-the-core-2026-09-28)
has the evidence.

- **Every debug parameter is a specialization constant.** The block in the
  frame bind group described above is replaced. The ids are 100–108: view,
  BRDF mode, terms off, flags (furnace, filtered out), scale, range,
  threshold, forced roughness and forced metalness.
  `public/render/shaderlib/debug_view.h` owns them together with the view
  catalog. With no debug block there is no layout variant. A program whose
  specialization is neutral receives no debug constant, so its pipeline is
  the shipped one.
  - A view's parameters are set only for the views that read them: the
    scale for views 8–12, the range for view 15 and the threshold for view
    17. That keeps one pipeline per view.
- **Default identity.** Every view uses the same module. That is binding
  rule 2 (one program), and it rules out a separate `-DDEBUG_VIEW` module.
  "Modules and constants unchanged" therefore means:
  - a shipped pipeline receives exactly its own constants;
  - the family pixel suites report the same worst differences as before
    the change.
- **Formulas.** Luminance uses the Rec. 709 weights. Linear depth is
  `1 / gl_FragCoord.w`, the clip-space w, which is the view-space depth.
- **Programs.** The views are in `lightmapped` (which `unlit` and the
  editor's `preview` draw through), `pbr`, `vertexlit` and `lines`. The
  shadow receiver pass (`render.pass.shadows`) is an oracle pass whose
  output encodes visibility, not a shading program, and takes no views.
- **Terms on the programs that exist today.**
  - `pbr`: its four model lights answer to `clustered`, its ambient cube
    to `probes`, and its reflected-cube image light to `ibl`. `vertexlit`
    mixes the cube and the lights in its vertex stage, so no light term is
    separable there, and views 9 and 18–20 draw the hatch.
  - On the `lightmapped` legacy point:
    - `baked` off is the frame of a zero lightmap page;
    - `emission` off is the frame of `$selfillumtint 0`;
    - `ibl` off is the frame without `$envmap`;
    - `cl_render_debug_brdf 1` and `2` drop the env map's specular term and
      the diffuse term.
- **Cube textures.** `render.resources`' `TextureCache` stages all six
  faces of a cube. The lab needs this for its env maps and probes.

## What is this pixel, and which draw is broken (`render.debug-draws.v1`)

### Frame draw records

While any control in this section is active, the core records one entry per
draw in recording order:

- the draw index, the graph pass and view, and the target;
- the material, its program and its non-neutral terms (the permutation);
- the pipeline key, the module hashes and the vertex layout;
- the bind groups and the debug names of their resources.

A legacy stream pass is one entry, carrying its stage and its count of legacy
draws. The core does not see inside it. The records live with the frame and
are released when the frame's completion token signals. They cost nothing
while no control needs them.

### Controls

| Control | Effect |
|---|---|
| `cl_render_debug_pick` (command) | Records the next frame with a 1×1 scissor at the crosshair and one occlusion query per core draw and per legacy stream pass; that frame is not presented. Prints every entry that wrote the pixel, in submission order, and marks the last opaque one. It also names the draw in the adapter's debug labels (`pick: draw <i>`), so `rdc.py` can find it in a capture. `cl_render_debug_pick x y` picks a window coordinate instead. |
| `cl_render_debug_draw_isolate <i>` | −1 off. Draws only entry `i`; the frame's clears and passes still run. |
| `cl_render_debug_draw_skip <i>` | −1 off. Skips entry `i`. With `cl_render_debug_draw_skip_count <n>` (default 1) it skips `[i, i+n)`, for bisection. |
| `cl_render_debug_material_isolate <substr>` | Draws only core draws whose material name contains `substr` (case-insensitive). |
| `cl_render_debug_heatmap <n>` | 1 hue by pipeline key, 2 by program, 3 by permutation. Hues are a stable hash, so a color means the same thing across frames and runs. |

Draw indices are stable only for a fixed camera and scene. The pick output
prints the frame number and the camera, so a report can be reproduced.

The pick uses occlusion queries, so it needs no shader change. A draw that
discards at the pixel records zero samples, which is the correct answer.

## Iteration and capture (`render.debug-iteration.v1`)

| Control | Effect |
|---|---|
| `cl_render_debug_reload_shaders` (command) | Reloads SPIR-V (and later per-target artifacts) that the pinned toolchain built into the debug shader directory (`tools/render/shader_toolchain.py build --out <dir>`), for every module whose file changed, and prints what changed. It runs in `render.shader-library` at a frame boundary. Old modules and pipelines are retired behind the completion token of the last frame that used them. A module that fails to load or build keeps the previous one and prints the reason, so the reload is all-or-nothing per module. The engine never compiles GLSL at runtime. |
| `cl_render_debug_shader_dir <path>` | Sets the debug shader directory; changing it performs a reload. Empty means the built-in artifacts only. |
| `cl_render_debug_capture` (command) | Captures the next frame through RenderDoc's in-app API when RenderDoc has already injected itself into the process. It is a device-port request; the Vulkan adapter finds the API only through an already-loaded module (`RTLD_NOLOAD`) and never loads RenderDoc itself. `cl_render_debug_capture <n>` captures n frames. |
| `cl_render_debug_capture_on_error 1` | On the first validation error of a map, captures the following frame and prints the capture path |
| `cl_render_debug_validation_break 1` | On a validation error, breaks into the debugger when `Plat_IsInDebugSession()`; otherwise prints a stack trace. Only meaningful when the adapter was created with validation. |
| `cl_render_debug_validation_mute <ids…>` | Space-separated message IDs to suppress. Muted counts are still reported at map end. |

The RenderDoc header is vendored at a pinned version, private to the adapters
that use it. Capture controls exist only in desktop profiles that allow
runtime-injected tools. The iOS static composition reports them unsupported.
An adapter that can't provide a control (the null adapter, or the OpenGL
adapter for a Vulkan message ID) reports it unsupported by name.

## Graph synchronization and resources (`render.debug-sync.v1`)

| Control | Effect |
|---|---|
| `cl_render_debug_sync_paranoid <n>` | 1: the graph compiler emits a full barrier (all stages, all accesses) between every pair of passes and after every copy and dispatch. 2: also, each frame waits on the previous frame's completion token before recording. If an artifact disappears at 1 or 2, it is a hazard, not a shader bug. |
| `cl_render_debug_poison_reuse 1` | When the transient pool aliases memory, or an upload ring or uniform slot is recycled after its completion token, fills it with a NaN pattern (`0x7FC00000` per word) first. With correct synchronization the frame is unchanged; a visible NaN (view 16 finds it) means early reuse. |
| `cl_render_debug_graph` (command) | Prints the compiled frame graph: passes, their accesses, transitions, aliasing, merges, and each pass's recording worker |
| `cl_render_debug_rt_list` (command) | Lists graph resources and imported textures by debug name, with format, size, mips, layers and memory |
| `cl_render_debug_rt_view <name>` | Draws that resource in a corner overlay (`cl_render_debug_rt_view_size`, default 25% of the width) as a debug pass after the frame, before present. Depth is shown as linear depth, and float formats are scaled by `cl_render_debug_view_scale`. 3D textures show slice `cl_render_debug_rt_slice`, cubes an unfolded cross, and the shadow atlas its tiles with their light keys. |
| `cl_render_debug_stats 1` | Prints the core's frame counters (passes, draws, pipelines bound, bind-group writes, pipeline misses, upload bytes, legacy stream draws) on the engine's notify lines |
| `cl_render_debug_gpu_timers 1` | Timestamps around every graph pass, read after the frame's completion token, reported per pass with `cl_render_debug_stats`. It supersedes the frozen backend's `-vkgputimers`, which is deleted with that backend. Unsupported when the adapter has no timestamp support. |
| `cl_render_debug_pipeline_miss_log 1` | Logs every pipeline created after the map's prewarm finished, with its program, permutation and key. Debug pipelines are excluded. |
| `cl_render_debug_pass_merge <n>` | −1 the graph's measured default; 0 no merging; 1 merge wherever legal. The graph compiler owns the value. |

## BSP2 container (`world.bsp2-debug.v1`)

The engine records one load entry per lump when a BSP2 map loads: the FourCC,
the version, the offset and size, the content hash, whether a consumer read it,
the consumer's name, and the load time and memory the consumer reports.

| Control | Effect |
|---|---|
| `cl_bsp2_info` (command) | Prints the load entries, plus the `LHDR` legacy file size and the `LGAP` record count. Lumps no consumer read are marked `unknown`. On a legacy BSP it prints that the map is not BSP2. |
| `cl_bsp2_verify` (command) | Calls `IMapContainer::VerifyContent` on the loaded file and reports each lump's result. It also rebuilds the legacy byte stream from `LHDR`, `LGAP` and the `L###` lumps and compares its hash with the recorded one, through the container library's own reconstruction. |
| `cl_bsp2_ignore_lumps "<FourCC…>"` | Latched: applies at the next map load. The loader withholds the named optional lumps, as if the map lacked them, and prints each consumer's existing fallback for a map without that lump (for example `RPRB withheld: using legacy cubemaps`). Naming a required lump (`LHDR`, `L###`) is refused. It adds no fallback code to any consumer. |
| `cl_bsp2_load_report 1` | Prints the load entries' time and memory at every map load |

The container controls read the map, not a render path, so they are not bound
by the freeze. They need no core gate.

## BSP2 per-lump views and overlays (`world.bsp2-views.v1`)

Pixel views are catalog entries (32–36) of the core's world pass. The
`cl_bsp2_*_view` names are commands that set `cl_render_debug_view`, so there
is one state owner:

| Command | Sets |
|---|---|
| `cl_bsp2_world_view batch\|material\|off` | View 32, 33 or 0 |
| `cl_bsp2_lmap_view charts\|density\|borders\|lightmap\|off` | View 34, 35, 36, 8 or 0 |

Other controls. Overlays draw through the core's `render.pass.lines` and
small core passes, never through the legacy material system:

| Lump | Control | Effect |
|---|---|---|
| world | `cl_render_debug_wipe <x>` | 0 off; otherwise a vertical split at fraction `x` of the width: on the left the core draws its claimed surfaces, on the right legacy draws the same surfaces (RFC 0016's isolate plumbing). This is the migration comparison. |
| world | `cl_render_debug_cull_freeze 1` | Freezes the scene's culling camera. The view camera moves on, and the frozen frustum is drawn as lines. |
| LMAP | `cl_bsp2_lmap_style <n>` | −1 all styles; otherwise only style `n` contributes |
| RPRB | `cl_bsp2_rprb_draw <n>` | 1 probe centers and capture spheres, 2 adds parallax boxes, 3 adds influence volumes |
| RPRB | `cl_bsp2_rprb_force <i>` | −1 off; otherwise every core surface uses probe `i` alone |
| RPRB | `cl_bsp2_rprb_band <n>` | 0 off; 1 albedo, 2 distance, 3 normal relight band of the dominant probe (RPRB v2 only) |
| PRBV | `cl_bsp2_prbv_draw <n>` | 1 probe positions, colored by validity; 2 spheres shaded by each probe's irradiance |
| SDFV | `cl_bsp2_sdf_view <n>` | 1 distance slice on the plane through the crosshair; 2 camera ray-march step count as a heat ramp; 3 light-cell bounds |
| RTRN | `cl_bsp2_rtrn_draw 1` | The transfer links of the patch under the crosshair, weighted by line brightness |
| all | `cl_bsp2_pick` (command) | At the crosshair: leaf and cluster, world batch, LMAP chart and texel, styles, the nearest PRBV probes and their weights, the SDF distance and the RPRB blend. `cl_render_debug_pick` prints this block too when the map is BSP2. |

Views and controls that read what the frozen backend draws today (its WMSH
batches, its probe sampling) apply once the core draws that content. Until
then they report unsupported for the frozen draws. They are not added to the
frozen backend.

## Delivery plan and gates

Each phase lands with its tests. A phase closes only when:
- its checks pass in `render_lab` first (binding rule 3);
- then in the product on native Vulkan Linux, with the render sequence on
  and off the main thread;
- and the default-identity check passes.

The Fold7 run is required for D0 and D4. Apple runs are optional.

### D0: State owner, catalog and term views

- `DebugControls` in `FrameDesc`, validation, and the host's ConVar parsing.
- `debug_view.glsl` and `kDebugView` in every core program.
- Views 1–17, and 21–23 as their terms reach the core; the program filter;
  the hatch clear for legacy stream passes.
- Tests:
  - `render.debug-views` in `render_lab`: a pixel suite with a fixture per
    view on analytic inputs (a plane with a known normal, a UV-mapped quad,
    known material constants, a known lightmap). Each view's output matches
    its formula within one 8-bit step.
  - Negative programs for the suite: a swapped normal encoding, a view that
    tone maps, a NaN view that misses NaN (an injected `0/0` input), and a
    program that draws normal shading instead of the hatch.
  - Default identity: shipped pipelines' modules and constants and every
    `render.*` pixel suite unchanged. An interleaved A/B frame time on
    Portal is recorded, and it does not block (binding rule 7).

### D1: Lighting-model controls, claims, legacy view

- `cl_render_debug_brdf`, `_furnace`, `_term`, `_force_*`, `_legacy`,
  `_claims`. `VK_DEBUG_LIGHTMAPPED`'s two sites deleted in the same change.
- Views 18–20 when the indirect terms reach the core. `mat_indirect_view`
  and its `-DINDIRECT_VIEW` variants are deleted in the change that makes the
  core own the indirect terms (binding rule 4). The `gi_*.py` callers move to
  `cl_render_debug_view` in that change.
- Tests:
  - the BRDF modes against `pbr_brdf.h`;
  - a furnace fixture in `render_lab` that passes with compensation and fails
    with `cl_render_debug_brdf 3`;
  - each `cl_render_debug_term` equals the frame without that input,
    bitwise;
  - a scene with one unclaimed material shows it magenta under
    `cl_render_debug_legacy 1`, and a claimed one isn't;
  - an unknown program name is rejected.

### D2: Draw records, pick and bisection

- Frame draw records, `cl_render_debug_pick`, `_draw_isolate`, `_draw_skip`,
  `_material_isolate`, `_heatmap`.
- Tests:
  - a scripted scene with a known draw order: the pick names the known
    draw, and every overlapping translucent draw in order;
  - isolate and skip change exactly the expected pixels;
  - a discarding draw at the pixel isn't reported;
  - a legacy stream pass is reported as one entry;
  - the pick label appears in an `rdc.py` capture;
  - records are released after the completion token, with no growth over
    1,000 frames.

### D3: Reload and capture

- `cl_render_debug_reload_shaders`, `_shader_dir`, the capture and
  validation controls.
- Tests:
  - a reload with one changed module changes its pixels and leaves the
    others;
  - a broken module keeps the old one;
  - retired modules are destroyed only after their completion token (a
    delayed-token fixture, as in R16);
  - validation mute and break on a seeded validation error;
  - capture under RenderDoc through `rdc.py` in an isolated compositor.

### D4: Graph synchronization and resources

- `cl_render_debug_sync_paranoid`, `_poison_reuse`, `_graph`, `_rt_list`,
  `_rt_view`, `_stats`, `_gpu_timers`, `_pipeline_miss_log`, `_pass_merge`.
- Tests:
  - with poisoning on, frames match frames with poisoning off;
  - a seeded early-reuse defect (a recycle one frame early) shows NaN under
    view 16;
  - a seeded missing barrier fixes itself under `sync_paranoid 1`;
  - the timers are monotonic, and their sum is within the frame's measured
    GPU time;
  - `_pass_merge 0` and `1` give byte-identical pixels.

### D5: Overdraw, mip, MSAA

- `cl_render_debug_overdraw`, `_mip`, `_msaa` (mode 2 after RFC 0012 A1 on
  the core).
- Tests: stacked quads with known counts; a textured quad at known distances
  maps to the expected mip; edge pixels of a known triangle at 4x.

### D6: BSP2 container

- Load entries, `cl_bsp2_info`, `_verify`, `_ignore_lumps`, `_load_report`.
- Tests:
  - `cl_bsp2_info` matches `bsp2_reader.py` on the 54-map corpus;
  - a corrupted lump fails `cl_bsp2_verify`;
  - withholding each optional lump in turn boots Portal with the stated
    fallback message and no errors;
  - naming `LHDR` is refused.

### D7: BSP2 views and overlays

- Views 32–36 on the core's world pass, the `cl_bsp2_*` view commands, the
  overlays through `render.pass.lines`, `cl_render_debug_wipe`,
  `cl_render_debug_cull_freeze`, and `cl_bsp2_pick`.
- Tests:
  - the chart and batch views match the Python reader's assignment at
    sampled pixels;
  - the border view marks a seeded seam-bleed fixture;
  - `cl_bsp2_rprb_force` matches the probe set's nearest-probe mode where
    probe `i` is nearest;
  - `cl_bsp2_pick` matches the reader's leaf and chart at the crosshair;
  - under `cl_render_debug_wipe 0.5` the two halves agree within the K5
    isolate tolerance.

**Dependencies.**
- D0 needs RFC 0016 K4 and `render_lab`'s composition check (K11). It is the
  first debug work, because K11's term proofs use it.
- D1 grows with the lighting-model terms (K11, then K12).
- D2 needs K5's draw lists.
- D3 and D4 need K2, which is done.
- D5 needs K5.
- D6 needs nothing from the core.
- D7 needs K5's world pass and, for the probe and volume overlays, their
  terms on the core.

## Roadmap

Tracked as `R95-DEBUG-CONTROLS`, a child of R95 (RFC 0016 K11), renamed from
`R32-DEBUG-CONTROLS` on 2026-09-28 when the controls moved to the core. D0
and D1 are part of how K11 proves the lighting terms. D2, D4 and D5 also serve
R89 and R90, and D6–D7 also serve R53, R54 and R56. The controls close no
other row's criterion by existing; those gates may use them as switches.

## Risks and mitigations

| Risk | Mitigation |
|---|---|
| Debug pipelines raise the pipeline count | Created on first use only; counts recorded per profile in D0; mobile profiles may exclude them (open decision) |
| A view's formula drifts from the shipped program | Views read the program's own terms through one call at the output point; D0 checks that view 0 through the debug path is the shipped pipeline, byte for byte |
| Debug state differs between threads | It travels in the frame description; tested with the render sequence on and off the main thread |
| Draw records cost memory on large frames | Recorded only while a control needs them; released at the completion token; D2 checks growth |
| Reload destroys a module that a frame still uses | Retirement behind completion tokens, tested with a delayed token |
| Pressure to add a control to the frozen backend "for now" | Refused by RFC 0016 binding rule 1. The control goes on the core, or waits until the core draws that content |

## Alternatives considered

- **Keep the controls in the native backend** (this RFC's first version).
  Rejected by the user on 2026-09-28. The backend is frozen, and controls
  built there would be rebuilt on the core.
- **Separate `-DDEBUG_VIEW` modules per family** (the first version's
  choice). Rejected on the core: port clause D20 gives specialization
  constants, and a neutral constant is the shipped pipeline, so a second
  module set would only duplicate artifacts.
- **A uniform branch in every shipped shader.** Rejected: it adds per-pixel
  cost to normal frames.
- **One ConVar per view.** Rejected: views are mutually exclusive, and
  separate flags would need a precedence rule and a second owner.
- **An ID attachment for pick.** Rejected for now: it needs a second color
  output in every program. Occlusion queries give the same answer for one
  pixel with no shader change.
- **Instrumenting draws inside legacy stream passes.** Rejected: that is
  new work on a frozen path. Legacy passes are one entry each, and migrating
  their content to the core is the fix.

## Open decisions and required evidence

- Whether store-targeted Android and iOS builds exclude the debug pipelines
  and draw records. This needs D0's size and memory measurement.
- The debug block's binding slot. The render-core session owns the program
  layouts and decides it.
- Whether `cl_bsp2_lmap_source` is needed once R49 decides how Cycles atlases
  are carried.

## Source references

- `public/render/frame/`, `render/renderer/`, `render/graph/`,
  `render/shaders/common/`, `render/material/families/`,
  `render/pass/world/`, `render/pass/lines/`
- `tools/render/shader_toolchain.py`, `tools/renderdoc/rdc.py`
- `public/mapcontainer/map_container.h`, `map_container_format.h`,
  `tools/quality/bsp2_reader.py`
- Frozen, for the controls being replaced:
  `materialsystem/shaderapivulkan/shaderapivulkan.cpp` (`mat_indirect_view`,
  `VK_DEBUG_LIGHTMAPPED`), `vulkan_world_pbr.cpp` (`SetIndirectLightView`)

## Proposed decision

Adopt one frame-owned debug state with a numbered view catalog on the render
core, expressed as a neutral specialization constant, so the shipped
pipelines are unchanged. Add the draw, iteration, graph and BSP2 controls
above in phases D0–D7. Each phase is proven in `render_lab` first and has
oracles and negative controls. Build nothing on a frozen path. Delete
`mat_indirect_view` and `VK_DEBUG_LIGHTMAPPED` in the changes that replace
them.
