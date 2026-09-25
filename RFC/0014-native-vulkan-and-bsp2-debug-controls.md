# RFC 0014: Native Vulkan and BSP2 Debug Controls

- Status: Proposed (2026-09-25); no implementation gate complete
- Date: 2026-09-25
- Scope: Developer controls for the native Vulkan backend (`cl_vk_debug_*`) and
  for BSP2 maps as loaded (`cl_bsp2_*`). This covers debug views, draw
  identification and bisection, shader reload and capture, synchronization
  and resource inspection, and container and per-lump inspection. Every
  control has a tested oracle.
- Render backend: [RFC 0001](0001-capability-based-platform-architecture.md)
  owns the native Vulkan backend (R28/R32), its debug names and labels, and
  the queued material system (R32-QUEUED)
- Shading: [RFC 0007](0007-physically-based-lighting-pipeline.md) owns the PBR
  family and `pbr_brdf.glsl` (R47) and the reflection probes (R50)
- Map data: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md) owns
  the BSP2 container and its lumps (R53–R56)
- Indirect light: [RFC 0011](0011-runtime-indirect-lighting.md) owns the
  producers and `r_indirect_*`
- Antialiasing: [RFC 0012](0012-antialiasing-msaa-specular-alpha-coverage.md)
  owns MSAA and alpha to coverage (R65)
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)

## Decision and boundary

Native Vulkan bugs are found today with RenderDoc, environment variables,
launch switches and one-off views that each grew up next to a single feature.
This RFC replaces that with one family of runtime controls:

1. **One owner of debug-view state.** The backend device owns a validated
   `DebugControls` value. ConVars are parsed at one place and applied at a
   frame boundary on the thread that records the frame. No shader or engine
   module reads a debug ConVar directly.
2. **One view catalog.** Every pixel view is a numbered entry of
   `cl_vk_debug_view`, with a defined formula. `mat_indirect_view` and the
   BSP2 pixel views are ranges of that catalog, not separate state.
3. **Shipped shaders are unchanged.** Views compile into a separate
   `-DDEBUG_VIEW` variant of each family. The variant is selected only while a
   view is active. With every control at its default, the embedded module
   hashes, the pipelines and the pixels of the shipped variants are identical
   to the pre-change backend.
4. **Every control is tested.** Each view has an oracle and a negative
   control in the conformance manifest. Each behavior control has a test that
   shows it has the effect it claims, and that the default has none.
5. **Data versus runtime.** `cl_bsp2_*` shows what a map's lumps contain and
   how the loader used them. The runtime selections that already exist keep
   their owners: `mat_reflection_probes`, `mat_reflection_relight`,
   `r_indirect_*`, `r_probevolume*` and `r_worldmesh_*`.

The controls are development tools. They don't change content, formats,
gameplay or network state. They don't certify any other RFC's gate, though
those gates may use them as switches.

## Observed starting point (2026-09-25)

Observed by reading source at `e6ee16ba` plus the dirty tree. Nothing here
was measured for this RFC.

- **Launch switches** (`CommandLine()` in `shaderapivulkan`): `-vkvalidate`,
  `-vkdebuglabels`, `-novkdebuglabels`, `-vkpassmerge`, `-vkemitreuseverify`,
  `-vkframestats`, `-vkpipelinecache`, `-vkdxlevel`. Validation and the
  debug-utils extension are decided at instance creation
  (`CVulkanContext::CreateInstance`).
- **Environment variables:** `SOURCE_VK_SHADER_DIR` selects debug shader
  variants by 16-hex content hash (`vulkan_shader_library.cpp`, matched against
  `material_spv_index.h`). `VK_DEBUG_LIGHTMAPPED` logs lightmapped vertices
  and combos (`shaderapivulkan.cpp`, two sites).
- **`mat_indirect_view` 0–3** (cheat) with `mat_indirect_view_scale`. Modes 1
  and 2 are compiled variants (`world_pbr.frag -DINDIRECT_VIEW`, with
  `CLIP_PLANES` and `DELTA_VOLUME` combinations). Mode 3 is a flag in the
  direct-light constants. The model PBR path drops its environment map while a
  mode is set. The device validates the value in `SetIndirectLightView`.
  Callers: `gi_runtime.py`, `gi_temporal.py`, `gi_swing.py`, `gi_portal.py`.
- **`mat_reflection_probes` 0–3, +4 for blend weights** (cheat) already
  provides the RPRB blended, nearest and direction-only modes. Callers:
  `reflection_runtime.py`, `reflection_probe_set.py`.
- **`r_worldmesh_draw` 0/1/2** compares uploaded WMSH batches with legacy
  surfaces; `r_worldmesh_cull*` controls WMSH culling and reports.
- **Descriptor sets:** the PBR and GI stages bind set 0 (frame, six combined
  image samplers), set 1 (material) and set 2 (a dynamic uniform block owned
  by the caller) (`vulkan_descriptor_groups.h`). The frame set has no uniform
  binding. The legacy material pipelines share a push-constant block
  `{ mat4 mvp; vec4 color; }`.
- **Families in tree** (`material_spv_index.h`): `world_pbr` (16 variants),
  `world_pbr_glass`, `model_pbr` (6), `pbr_direct`, `lightmapped`, `skin`,
  `solidenergy`, `paintblob`, `portal_refract`, `screenspace_post`,
  `present_gamma` and the generic textured path (`demo_dyn_tex`). The 84
  stdshader ports (R32-LEGACY-SHADERS) are in a separate, unmerged tree.
- **Debug names and labels** exist (`vulkan_debug_utils.h`): object names for
  images, managed textures (`NameManagedTexture`) and MSAA targets, and pass
  and frame labels, replayed in order under the queued material system.
  `tools/renderdoc/rdc.py` reads them.
- **Map container:** `IMapContainer::VerifyContent` checks every lump's content
  hash (`public/mapcontainer/map_container.h`). Lumps in use: `LHDR`, `LGAP`,
  legacy `L###`, `WMSH`, `LMAP`, `RPRB`, `PRBV`, `RTRN`, `SDFV`. The
  independent reader is `tools/quality/bsp2_reader.py`.
- **No** GPU timestamp queries, draw bisection, per-draw pick, shader hot
  reload, RenderDoc in-app trigger, overdraw, mip or MSAA views exist.

## Goals

- Answer "what is this pixel made of" and "which draw is wrong" in game,
  without a capture, on every native profile (Linux, Android, and Apple once
  R29 lands).
- Separate shader bugs from synchronization and reuse bugs with one switch.
- Make the shader edit loop seconds long: edit GLSL, reload, and see the
  result in the same session.
- Show what a BSP2 map carries, and test each optional lump's fallback by
  withholding it.
- Keep harnesses and people on the same switches, with one owner per switch.

## Non-goals

- D3D9, DXVK or togl support. Native Vulkan is the target (user decision
  2026-09-24). The controls report "unsupported" on other backends.
- Enabling validation or the debug-utils extension at runtime. Both remain
  instance-time launch switches.
- New content formats or lump versions. `cl_bsp2_*` reads what exists.
- Shipping debug controls to players. The controls are cheat-protected.
  Whether store builds compile out the `DEBUG_VIEW` variants is a profile
  decision (see [Open decisions](#open-decisions-and-required-evidence)).
- Performance tuning. Timers and counters report cost; budgets stay with
  their owning rows.

## Layers and owners

| Layer | Owner | Holds |
|---|---|---|
| ConVar registration and parsing | `shaderapivulkan` (`cl_vk_debug_*`); engine client (`cl_bsp2_*`) | Names, help, flags, ranges. Parsed each frame into one `DebugControls` request. |
| Validated debug state | `CVulkanContext` (`vulkan_debug_controls.{h,cpp}`) | The applied `DebugControls`, the view catalog, per-family support masks. Rejects invalid values with a console message and keeps the previous state. |
| Frame application | Render thread, at `BeginFrame` | Applies a new request only between frames. Under `mat_queue_mode 2` the request travels through the existing frame-ordered queue adapter, so the main and render threads never disagree mid-frame. |
| Shader side | `shaders/debug_view.glsl` | The one include with every view's formula. Families include it under `DEBUG_VIEW`. |
| Map-side data | Engine BSP2 loader and lump consumers | The per-lump load records, consumed/ignored state and fallbacks that `cl_bsp2_*` reports. |
| Oracles | `render.debug-*` and `world.bsp2-debug*` suites | CPU formulas, pixel suites, negative controls. |

The `cl_` prefix is kept as the user chose it, although these controls live in
`shaderapivulkan` and the engine rather than `client.dll`. Existing `r_`,
`mat_` and `-vk*` names are not renamed; where their meaning overlaps, this RFC
names one owner and a migration.

## Common rules

- **Flags.** Anything that changes pixels or behavior is `FCVAR_CHEAT`.
  Commands and reports that only print are not.
- **Invalid input fails visibly.** An unknown family name, out-of-range view
  or unsupported control prints why and changes nothing.
- **Not-applicable pattern.** A family that can't produce a requested view
  (for example roughness on `lightmapped`) draws a fixed grey diagonal hatch
  (period 8 pixels, 25%/50% grey). It never draws its normal shading, which
  would look like a valid answer.
- **Post-processing is bypassed** while a pixel view is active: no tonemap,
  bloom, color correction or gamma ramp. Radiometric views output a linear
  value times `cl_vk_debug_view_scale` (default 1), sRGB-encoded as the back
  buffer requires. A view's output is therefore its formula, not the
  formula after tonemapping.
- **Default identity.** With every control at default, embedded module hashes,
  created pipelines and pixel suites are unchanged, and frame time is equal
  within the measured noise of an interleaved A/B.
- **Both threading modes.** Every control is tested under `mat_queue_mode 0`
  and `2`.

## Debug views (`render.debug-views.v1`)

### Catalog

`cl_vk_debug_view <n>` (cheat, default 0). The formula column is the oracle.
Families: **P** = PBR (`world_pbr`, `world_pbr_glass`, `model_pbr`,
`pbr_direct`), **L** = legacy natives (`lightmapped`, `skin`, generic
textured, and the stdshader ports when merged).

| n | View | Output | Families |
|---|---|---|---|
| 0 | Off | Normal shading | all |
| 1 | Albedo | Linear base color after texture and modulation, no lighting | P, L |
| 2 | World normal | `n * 0.5 + 0.5`, the shading normal after normal mapping | P, L with a normal |
| 3 | Normal map | The decoded tangent-space sample `t * 0.5 + 0.5` | P, L with a bump map |
| 4 | Roughness | Material perceptual roughness as grey (before R65 specular AA) | P |
| 5 | Metalness | Material metalness as grey | P |
| 6 | AO | Material ambient occlusion as grey | P |
| 7 | Baked lightmap | Lightmap irradiance with albedo 1, times the lightmap scale | P, L lightmapped |
| 8 | Direct light | Only the direct-light terms (the frame's light set), with the material | P, L lit |
| 9 | Specular / IBL | Only the environment and reflection-probe specular term | P, L with `$envmap` |
| 10 | Emissive | The emissive term | P, L with self-illum |
| 11 | UV0 checker | `(floor(8 u) + floor(8 v)) mod 2` as black/white | all with UV0 |
| 12 | Vertex color | Vertex color RGB | all with a color stream |
| 13 | Linear depth | View-space depth / `cl_vk_debug_view_range` (default 4096 units), clamped | all |
| 14 | NaN/Inf/negative | Final pre-tonemap color: NaN magenta, ±Inf cyan, any negative channel yellow, else luminance × 0.5 as grey | all |
| 15 | Over-range | Pre-tonemap luminance > `cl_vk_debug_view_threshold` (default 1) red, else luminance × 0.5 as grey | all |
| 16 | Indirect irradiance | Was `mat_indirect_view 1`: indirect diffuse irradiance / π | P world |
| 17 | Indirect radiance | Was `mat_indirect_view 2` | P world |
| 18 | All diffuse | Was `mat_indirect_view 3`: bake, producer change and unbaked direct light, irradiance / π | P world |
| 32 | WMSH batch | A hashed color per uploaded WMSH batch | WMSH world |
| 33 | WMSH material | A hashed color per WMSH material index | WMSH world |
| 34 | LMAP chart | A hashed color per lightmap chart | lightmapped world |
| 35 | LMAP texel density | Checker at lightmap texel resolution | lightmapped world |
| 36 | LMAP chart borders | Texels within one texel of a chart border in red; seam bleed shows as red leaking | lightmapped world |

Numbers 19–31 and 37 and above are reserved. Views 16–18 keep their existing
formulas and oracles (RFC 0011); they move owners, not behavior.

### Modifiers

| ConVar | Default | Effect |
|---|---|---|
| `cl_vk_debug_view_family <name>` | empty | Applies the view to one family only. Others draw flat 18% grey, keeping depth. Names come from the backend's family registry; `cl_vk_debug_view_family ?` lists them. Also applies to `cl_vk_debug_brdf` and `cl_vk_debug_heatmap`. |
| `cl_vk_debug_view_scale` | 1 | Linear exposure for radiometric views (7–10, 16–18). Replaces `mat_indirect_view_scale`. |
| `cl_vk_debug_view_range` | 4096 | Divisor for view 13; count range for overdraw. |
| `cl_vk_debug_view_threshold` | 1 | Threshold for view 15. |

### PBR controls

| ConVar | Effect | Oracle |
|---|---|---|
| `cl_vk_debug_brdf <n>` | 0 full, 1 diffuse lobe only, 2 specular lobe only, 3 energy compensation off, 4 the split-sum LUT sample as RG | Each mode equals the matching `pbr_brdf.h` CPU term on the `render.pbr-brdf.glsl` cases. Mode 3 must reproduce the `no-energy-compensation` negative's furnace loss. |
| `cl_vk_debug_furnace 1` | Albedo 1 and a uniform environment of radiance 1 replace probes, IBL and lightmaps; direct lights off | In game, an energy-compensated white metal sphere reads 1 within the R47 furnace tolerance at every roughness; with `cl_vk_debug_brdf 3` rough spheres read below it. |
| `cl_vk_debug_force_roughness`, `cl_vk_debug_force_metalness` | −1 off, else the value clamped to [0, 1] replaces the material's | Pixel equals the same material authored with that value. |

These affect P families only. L families show the not-applicable pattern when
`cl_vk_debug_brdf` is nonzero and `cl_vk_debug_view_family` doesn't exclude them.

### Fallback and unported draws

`cl_vk_debug_fallback 1` draws magenta for draws the backend resolves to no
native family, to an unported combo, or to the built-in fallback texture
("built-in fallback texture" in the debug names). The backend already makes
these decisions (`*DrawsNative`, `BindStandardTexture` fallbacks); this control
exposes them and doesn't make a second decision.

`cl_vk_debug_unported_report 1` logs each (shader, static combo, reason) once
per map. It replaces `VK_DEBUG_LIGHTMAPPED`. Deletion condition: both
`VK_DEBUG_LIGHTMAPPED` sites are removed when this control lands, and the
lightmapped vertex dump moves into `cl_vk_debug_pick`.

### Overdraw, mip and MSAA views

| ConVar | Effect |
|---|---|
| `cl_vk_debug_overdraw <n>` | 1 counts fragments that pass the depth test, 2 counts every rasterized fragment (depth test off). Counted into an `R16_UINT` target by debug pipelines with additive state, then colorized in one pass over 0..`cl_vk_debug_view_range` (default 8 for this view). |
| `cl_vk_debug_mip <n>` | 1 base-texture mip level from `textureQueryLod` as a fixed color ramp; 2 texel density: texels per pixel below 0.5 blue, 0.5–2 green, above 2 red. |
| `cl_vk_debug_msaa <n>` | 1 pixels whose samples differ (edge pixels) in white; 2 alpha-to-coverage sample count per pixel. Mode 2 reports unsupported until R65 A1 replaces the `VK_UNIMPLEMENTED` stub. Both report unsupported at 1 sample. |

### Implementation

- `debug_view.glsl` holds every formula. A family includes it and calls
  `DebugViewOutput(inputs)` at its single output point under `#ifdef
  DEBUG_VIEW`. The inputs are a struct of the family's intermediate terms, with
  a mask of which ones it has; the mask drives the not-applicable pattern.
- The view number, family filter result, BRDF mode, overrides, scale, range and
  threshold travel in a 32-byte debug block. PBR stages append it to their
  existing set 2 dynamic uniform block. Legacy stages extend their push-constant
  range by 16 bytes, but only in `DEBUG_VIEW` pipeline layouts. Shipped layouts
  are unchanged.
- `DEBUG_VIEW` subsumes `INDIRECT_VIEW`. The `-DINDIRECT_VIEW` variants are
  deleted when views 16–18 pass their existing pixel suites through the new
  variant.
- `regen_material_spv.py` generates the debug variants and records them in
  `material_spv_index.h`, so `SOURCE_VK_SHADER_DIR` can override them too.
- Pipelines for debug variants are created on first use and kept for the map.
  Their creation is excluded from the pipeline-miss log (below).

### Migration of existing views

`mat_indirect_view` becomes a deprecated alias. Setting it to 1–3 sets
`cl_vk_debug_view` to 16–18, and 0 sets it to 0; it stores nothing itself. The
four `gi_*.py` callers move to `cl_vk_debug_view`. The alias is deleted when no
tool or config in the tree names it. `mat_indirect_view_scale` gets the same
treatment toward `cl_vk_debug_view_scale`.

`mat_reflection_probes` keeps ownership of the RPRB runtime modes. This RFC
adds no `cl_bsp2_rprb_mode`.

## What is this pixel, and which draw is broken (`render.debug-draws.v1`)

### Frame draw records

While any control in this section is active, the backend records one entry per
draw in recording order (the replay order under `mat_queue_mode 2`):

- the draw index, the render pass label and the target;
- the material name (from `IShaderAPI::Bind`) and the shader family;
- the static and dynamic combo;
- the pipeline state key, the vertex and fragment module hashes (the 16-hex
  names `SOURCE_VK_SHADER_DIR` uses) and the vertex layout
  (`vulkan_mesh_layout`);
- the bound descriptor sets and their images' debug names.

The records live in the frame slot and are released when that slot's fence
signals. They cost nothing while no control needs them.

### Controls

| Control | Effect |
|---|---|
| `cl_vk_debug_pick` (command) | Records the next frame with a 1×1 scissor at the crosshair and one occlusion query per draw; that frame is not presented. Prints every draw that wrote that pixel in submission order and marks the last opaque one. Each entry prints the frame draw record. It also inserts a debug label `pick: draw <i>` so `rdc.py` can find the draw in a capture of that frame. `cl_vk_debug_pick x y` picks a window coordinate instead. |
| `cl_vk_debug_draw_isolate <i>` | −1 off. Draws only draw `i`; the frame's clears and passes still run. |
| `cl_vk_debug_draw_skip <i>` | −1 off. Skips draw `i` (its state is still bound, so later draws are unaffected). With `cl_vk_debug_draw_skip_count <n>` (default 1) skips `[i, i+n)`, for bisection. |
| `cl_vk_debug_material_isolate <substr>` | Draws only draws whose material name contains `substr` (case-insensitive). |
| `cl_vk_debug_heatmap <n>` | 1 hue by pipeline state key, 2 by shader family, 3 by combo. Hues are a stable hash, so a color means the same thing across frames and runs. Uses the `DEBUG_VIEW` variants. |

Draw indices are stable only for a fixed camera and scene. The pick output
prints the frame number and the camera, so a report can be reproduced.

The pick uses occlusion queries so it needs no shader change and works for
every family, including unported ones. A draw that discards at the pixel
records zero samples, which is the correct answer.

## Iteration and capture (`render.debug-iteration.v1`)

| Control | Effect |
|---|---|
| `cl_vk_debug_reload_shaders` (command) | Rescans the debug shader directory, rebuilds modules and pipelines for every hash whose file changed, and prints what changed. Old modules and pipelines are retired behind the completion token of the last frame that used them, not destroyed at once. It runs at a frame boundary on the recording thread. A variant that fails to load or build keeps the previous one and prints the reason, so the reload is all-or-nothing per hash. |
| `cl_vk_debug_shader_dir <path>` | Sets the debug-variant directory. `SOURCE_VK_SHADER_DIR` only sets its starting value. Changing it performs a reload. Empty means embedded shaders only. |
| `cl_vk_debug_capture` (command) | Captures the next frame through RenderDoc's in-app API, when RenderDoc already injected itself into the process. The backend finds the API only through an already-loaded module (`RTLD_NOLOAD`) and never loads RenderDoc itself. With `cl_vk_debug_capture <n>`, captures n frames. |
| `cl_vk_debug_capture_on_error 1` | On the first validation error of a map, captures the following frame and prints the capture path. The erroring frame itself was already submitted. |
| `cl_vk_debug_validation_break 1` | On a validation error, breaks into the debugger when `Plat_IsInDebugSession()`; otherwise prints a stack trace. Only meaningful under `-vkvalidate`. |
| `cl_vk_debug_validation_mute <ids…>` | Space-separated VUIDs or message ID numbers to suppress. Muted counts are still reported at map end. |

The RenderDoc header is vendored at a pinned version. The capture controls are
compiled only into desktop profiles that allow runtime-injected tools. The iOS
static composition has no module lookup and reports them unsupported.

## Synchronization and resources (`render.debug-sync.v1`)

| Control | Effect |
|---|---|
| `cl_vk_debug_sync_paranoid <n>` | 1 inserts a full memory barrier (all commands, all reads and writes) after every render pass, transfer and compute dispatch. 2 also waits for the queue to go idle after every submit. If an artifact disappears at 1 or 2, it is a hazard, not a shader bug. |
| `cl_vk_debug_poison_reuse 1` | When a stream, upload-ring or uniform slot is recycled after its completion token, fills it with a NaN pattern (`0x7FC00000` per word) before reuse. With correct synchronization the frame is unchanged; a visible NaN (view 14 finds it) means early reuse. |
| `cl_vk_debug_rt_list` (command) | Lists managed textures and render targets by debug name, with format, size, mips, layers and memory. |
| `cl_vk_debug_rt_view <name>` | Draws that target in a corner overlay (`cl_vk_debug_rt_view_size`, default 25% of the width) after the frame, before present. Depth is shown as linear depth; float formats are scaled by `cl_vk_debug_view_scale`; 3D textures show slice `cl_vk_debug_rt_slice`; cubes show an unfolded cross. Uses the existing debug names (RPRB atlas, LMAP atlas, SDF field, shadow depth, bloom). |
| `cl_vk_debug_stats 1` | Prints the `-vkframestats` counters (draws, pipelines bound, descriptor writes, render passes, pipeline-cache misses, upload bytes) through the engine's on-screen notify lines. |
| `cl_vk_debug_gpu_timers 1` | Writes timestamps around every labeled region into a per-slot query pool, reads them after the slot's fence, and reports GPU time per region with `cl_vk_debug_stats`. Unsupported when the graphics queue's `timestampValidBits` is 0. |
| `cl_vk_debug_pipeline_miss_log 1` | Logs every pipeline created after the map's prewarm finished, with its family and state key. Debug-variant pipelines are excluded. |
| `cl_vk_debug_passmerge`, `cl_vk_debug_emit_reuse_verify` | Runtime versions of `-vkpassmerge` and `-vkemitreuseverify`. The ConVar owns the value; the switch sets its starting value. Applied at a frame boundary. |

## BSP2 container (`world.bsp2-debug.v1`)

The engine records one load entry per lump when a BSP2 map loads: the FourCC,
the version, the offset and size, the content hash, whether a consumer read it,
the consumer's name, and the load time and memory the consumer reports.

| Control | Effect |
|---|---|
| `cl_bsp2_info` (command) | Prints the load entries, plus the `LHDR` legacy file size and the `LGAP` record count. Lumps no consumer read are marked `unknown`. On a legacy BSP it prints that the map is not BSP2. |
| `cl_bsp2_verify` (command) | Calls `IMapContainer::VerifyContent` on the loaded file and reports each lump's result. It also rebuilds the legacy byte stream from `LHDR`, `LGAP` and the `L###` lumps and compares its hash with the recorded one, through the container library's own reconstruction. |
| `cl_bsp2_ignore_lumps "<FourCC…>"` | Latched: applies at the next map load. Withholds the named optional lumps from their consumers. Each consumer must take its declared fallback and say so in the console, for example `RPRB withheld: using legacy cubemaps`. Naming a required lump (`LHDR`, `L###`) is refused. |
| `cl_bsp2_load_report 1` | Prints the load entries' time and memory at every map load. |

## BSP2 per-lump views and overlays (`world.bsp2-views.v1`)

Pixel views are catalog entries (32–36). The `cl_bsp2_*_view` names are
commands that set `cl_vk_debug_view`, so there is one state owner:

| Command | Sets |
|---|---|
| `cl_bsp2_wmsh_view batch\|material\|off` | View 32, 33 or 0 |
| `cl_bsp2_lmap_view charts\|density\|borders\|lightmap\|off` | View 34, 35, 36, 7 or 0 |

Other controls:

| Lump | Control | Effect |
|---|---|---|
| WMSH | `cl_bsp2_wmsh_wipe <x>` | 0 off; else a vertical split at fraction `x` of the width: WMSH batches left, legacy surfaces right. Built on `r_worldmesh_draw`'s two paths in one frame. |
| WMSH | `cl_bsp2_wmsh_cull_freeze 1` | Freezes the WMSH culling camera. The view camera moves on; the frozen frustum is drawn as lines. |
| LMAP | `cl_bsp2_lmap_style <n>` | −1 all styles; else only style `n` contributes (for example the Cycles preview's style 32). |
| RPRB | `cl_bsp2_rprb_draw <n>` | 1 probe centers and capture spheres, 2 adds parallax boxes, 3 adds influence volumes. |
| RPRB | `cl_bsp2_rprb_force <i>` | −1 off; else every surface uses probe `i` alone. |
| RPRB | `cl_bsp2_rprb_band <n>` | 0 off; 1 albedo, 2 distance, 3 normal relight band of the dominant probe (RPRB v2 only; unsupported on v1). |
| PRBV | `cl_bsp2_prbv_draw <n>` | 1 probe positions, colored by validity after `r_probevolume_visibility`; 2 spheres shaded by each probe's irradiance. |
| SDFV | `cl_bsp2_sdf_view <n>` | 1 distance slice on the plane through the crosshair, facing the camera; 2 camera ray-march step count as a heat ramp; 3 light-cell bounds. |
| RTRN | `cl_bsp2_rtrn_draw 1` | The transfer links of the patch under the crosshair, weighted by line brightness. |
| all | `cl_bsp2_pick` (command) | At the crosshair: leaf and cluster, WMSH batch, LMAP chart and texel, styles, the nearest PRBV probes and their weights, the SDF distance and the RPRB blend. `cl_vk_debug_pick` prints this block too when the map is BSP2. |

Overlays use the engine's debug overlay for lines and boxes. The shaded PRBV
spheres and the SDF views are small dedicated pipelines, not `DEBUG_VIEW`
variants, because they draw the lump's data, not a surface.

`cl_bsp2_lmap_source` (choosing a vrad or Cycles atlas) is deferred. Today the
Cycles preview is a style layer, which `cl_bsp2_lmap_style` covers. It is
revisited when a map carries both sources as separate atlases (R49).

## Delivery plan and gates

Each phase lands with its tests. A phase closes only on native Vulkan Linux
evidence under `mat_queue_mode 0` and `2`, plus the default-identity check. The
Android run (Fold7) is required for D0 and D4; Apple runs wait for R29.

### D0: State owner, catalog and core views

- `DebugControls`, the frame-boundary application and the queued adapter.
- `debug_view.glsl`, `DEBUG_VIEW` variants for the P families and
  `lightmapped`, `skin` and the generic textured path.
- Views 1–18. `mat_indirect_view` alias; the `gi_*.py` callers migrated;
  `INDIRECT_VIEW` variants deleted.
- Tests:
  - `render.debug-views`: a GPU pixel suite with per-view fixtures on
    analytic inputs (a plane with a known normal, a UV-mapped quad, known
    material constants, a known lightmap). Each view's output must match its
    formula within one 8-bit step.
  - Negative providers for the suite: a swapped normal encoding, a view that
    tonemaps, a NaN view that misses NaN (an injected `0/0` input), and a family
    that draws normal shading instead of the hatch.
  - `.four-sets` device variant.
  - Default identity: the module hashes of shipped variants and every existing
    `render.*` pixel suite are unchanged, and an interleaved A/B frame-time run
    on Portal shows no difference beyond noise.

### D1: Family filter, PBR controls, fallback

- `cl_vk_debug_view_family`, `cl_vk_debug_brdf`, `cl_vk_debug_furnace`,
  `cl_vk_debug_force_*`, `cl_vk_debug_fallback`,
  `cl_vk_debug_unported_report`. `VK_DEBUG_LIGHTMAPPED` removed.
- Tests: BRDF modes against `pbr_brdf.h` on the `render.pbr-brdf.glsl`
  cases; an in-game furnace fixture (spheres of roughness 0.05–1, white metal)
  passes with compensation and fails with `cl_vk_debug_brdf 3`; a fixture with
  one unported combo is magenta and a ported one isn't; an unknown family name
  is rejected.

### D2: Draw records, pick and bisection

- Frame draw records, `cl_vk_debug_pick`, `_draw_isolate`, `_draw_skip`,
  `_material_isolate`, `_heatmap`.
- Tests: a scripted scene with known draw order: the pick names the known
  draw, and all overlapping translucent draws in order; isolate and skip
  change exactly the expected pixels; a discarding draw at the pixel isn't
  reported; the pick label appears in an `rdc.py` capture; records are
  released after the slot fence (no growth over 1,000 frames).

### D3: Reload and capture

- `cl_vk_debug_reload_shaders`, `cl_vk_debug_shader_dir`, the capture and
  validation controls.
- Tests: reload with a changed debug variant changes the pixels and leaves
  the others; a broken variant keeps the old one; retired modules are destroyed
  only after their completion token (a delayed-fence fixture, as in R16);
  validation mute and break on a seeded validation error under `-vkvalidate`;
  capture under RenderDoc via `rdc.py` in an isolated compositor.

### D4: Synchronization and resources

- `cl_vk_debug_sync_paranoid`, `_poison_reuse`, `_rt_list`, `_rt_view`,
  `_stats`, `_gpu_timers`, `_pipeline_miss_log`, `_passmerge`,
  `_emit_reuse_verify`.
- Tests: with poisoning on, Portal frames match with poisoning off; a seeded
  early-reuse defect (recycle one frame early) shows NaN under view 14;
  a seeded missing barrier fixes itself under `sync_paranoid 1`; timers are
  monotonic and their sum is within the frame's measured GPU time; the runtime
  twins match their launch switches' behavior.

### D5: Overdraw, mip, MSAA

- `cl_vk_debug_overdraw`, `_mip`, `_msaa` (mode 2 after R65 A1).
- Tests: stacked quads with known counts; a textured quad at known
  distances maps to the expected mip; edge pixels of a known triangle at 4x.

### D6: BSP2 container

- Load entries, `cl_bsp2_info`, `_verify`, `_ignore_lumps`, `_load_report`.
- Tests: `cl_bsp2_info` matches `bsp2_reader.py` on the 54-map corpus; a
  corrupted lump fails `cl_bsp2_verify`; withholding each optional lump in turn
  (`WMSH`, `LMAP`, `RPRB`, `PRBV`, `RTRN`, `SDFV`) boots Portal with the stated
  fallback message and no errors; naming `LHDR` is refused.

### D7: BSP2 views and overlays

- Views 32–36, the `cl_bsp2_*` view commands and overlays, `cl_bsp2_pick`.
- Tests: chart and batch views match the Python reader's chart and batch
  assignment at sampled pixels; the border view marks a seeded seam-bleed
  fixture; `cl_bsp2_rprb_force` matches `mat_reflection_probes 2` where probe
  `i` is the nearest; `cl_bsp2_pick` matches the reader's leaf and chart at the
  crosshair.

## Roadmap

Tracked as `R32-DEBUG-CONTROLS`, a child of R32 in AGENTS.md. D6–D7 also
support R53, R54 and R56 debugging. The controls close no R32, R47, R50,
R53 or R65 criterion by existing; those gates may use them as switches.

## Risks and mitigations

| Risk | Mitigation |
|---|---|
| Debug variants double SPIR-V size and pipeline count | Created on first use only; size recorded per profile in D0; mobile profiles may exclude them (open decision). |
| A view's formula drifts from the shipped shader | Views read the family's own intermediate terms through one call at the output point; D0 compares view 0 through the debug variant with the shipped variant, byte for byte. |
| Debug state read mid-frame under the queued material system | One owner, applied at `BeginFrame` through the frame-ordered adapter; tested in mode 2. |
| Draw records cost memory on large frames | Recorded only while a control needs them; released at the slot fence; D2 checks growth. |
| Hot reload destroys a module a frame still uses | Retirement behind completion tokens, tested with a delayed fence. |
| `ignore_lumps` leaves partial state | Each consumer's fallback is explicit and tested per lump; required lumps are refused. |

## Alternatives considered

- **A uniform branch in every shipped shader.** Rejected: it changes every
  shipped module and its pixel-suite identity, and adds per-pixel cost to
  normal frames.
- **Specialization constants per view.** Rejected: they multiply pipelines by
  view count and still recompile shipped pipelines.
- **One ConVar per view.** Rejected: views are mutually exclusive, and separate
  flags would need a precedence rule and a second owner.
- **An ID attachment for pick.** Rejected for now: it needs a second color
  output in every family. Occlusion queries give the same answer for one pixel
  with no shader change.
- **Keeping `mat_indirect_view` as a separate owner.** Rejected: two view
  switches that can both be set need a precedence rule.

## Open decisions and required evidence

- Whether store-targeted Android and iOS builds compile out `DEBUG_VIEW`
  variants and the draw records. This needs D0's size and memory measurement.
- The exact R65 alpha-to-coverage hook for `cl_vk_debug_msaa 2`.
- Whether `cl_bsp2_lmap_source` is needed once R49 decides how Cycles atlases
  are carried.
- The stdshader ports (R32-LEGACY-SHADERS) gain `DEBUG_VIEW` variants when they
  merge; until then, D0 covers the in-tree families only.

## Source references

- `materialsystem/shaderapivulkan/shaderapivulkan.cpp` (`mat_indirect_view`,
  `mat_reflection_probes`, `VK_DEBUG_LIGHTMAPPED`)
- `materialsystem/shaderapivulkan/vulkan_world_pbr.cpp`
  (`SetIndirectLightView`, indirect-view pipelines)
- `materialsystem/shaderapivulkan/vulkan_descriptor_groups.h`
- `materialsystem/shaderapivulkan/vulkan_debug_utils.h`,
  `vulkan_shader_library.cpp`, `material_spv_index.h`
- `public/mapcontainer/map_container.h`, `map_container_format.h`
- `engine/gl_rsurf.cpp` (`r_worldmesh_*`), `engine/lightcache.cpp`
- `tools/renderdoc/rdc.py`, `tools/quality/bsp2_reader.py`

## Proposed decision

Adopt one device-owned debug state with a numbered view catalog, compiled into
separate `DEBUG_VIEW` variants so shipped shaders are unchanged. Add the draw,
iteration, synchronization and BSP2 controls above in phases D0–D7, each with
oracles and negative controls. Migrate `mat_indirect_view` and
`VK_DEBUG_LIGHTMAPPED` into the new owner and delete them when their callers
have moved.
