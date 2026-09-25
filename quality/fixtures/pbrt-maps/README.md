# PBRT reference scenes → playable BSP2 maps

One command turns a PBRT-v4 scene (the format of Benedikt Bitterli's
[rendering resources](https://benedikt-bitterli.me/resources/)) into a Portal
BSP2 map with a USD-derived WMSH world mesh, a Cycles-baked KTX2 lightmap,
Source materials, collision and a spawn at the reference camera:

```sh
python3 tools/quality/pbrt_map_toolchain.py provision --jobs 16   # once per machine
python3 tools/quality/pbrt_map_build.py \
  --manifest quality/fixtures/pbrt-maps/living-room.json \
  --out quality-results/living-room-map --boot
```

`--boot` runs the map headless on the native Vulkan provider (no window on the
desktop) and saves a screenshot under `<out>/boot/`. It also runs the in-game
gates: a camera-matched frame at the PBRT reference eye compared with the
Cycles render (`<out>/camera-boot/`), and a gravity drop test onto the highest
walkable collision tops and the spawn floor (`<out>/traversal-boot/`). Re-running is incremental:
`<out>/steps.json` records input, settings and script digests per step, so a
collision edit does not repeat the bake. `--from STEP` forces a step and all
later ones. Logs are in `<out>/logs/<step>.log`; the failing step prints its
tail.

## Playing a map

A finished build is published to `run/maps/<map>/` (untracked), and every
`./play` mounts all published maps into its runtime as
`portal/custom/pbrt-<map>`, so this works immediately:

```sh
./play living_room        # also staircase2_pbrt, bedroom
python3 tools/quality/playable_maps.py list                 # what is published
python3 tools/quality/playable_maps.py publish quality-results/living-room-map
python3 tools/quality/playable_maps.py remove bedroom
```

A build that failed a gate under `--keep-going` is still published, and
`list` and `./play` show its failed gates. `--no-publish` skips publishing. The
engine draws a map's WMSH/LMAP world by default (`r_worldmesh_draw 2`, not a
cheat), so a published map also draws correctly when loaded from the console or
a menu; `r_worldmesh_draw 0` shows the legacy compile brushes for comparison. A
published map is not mounted when a shipped map has the same name.

The client must include the pinned KTX reader, or the log shows
`WMSH LMAP rejected` and the world is unlit. Enable it once per client tree:

```sh
python3 tools/quality/pbrt_map_toolchain.py provision --steps ktx-reader
python3 tools/quality/pbrt_map_toolchain.py configure-client --build build
```

## Adding a scene

1. Put the scene directory (`scene-v4.pbrt`, `models/`, `textures/`, license)
   at the repository root, like `living-room/` and `staircase2/`.
2. Check the scene reader accepts it and read its inventory (materials,
   meshes by material, camera, sky):
   `python3 tools/quality/pbrt_scene.py <dir>/scene-v4.pbrt`.
   Unsupported PBRT features fail loudly; extend `tools/quality/pbrt_scene.py`
   and its tests rather than special-casing a scene. A PLY placed more than
   once keeps its stem for the first placement and becomes `<stem>_i1`,
   `<stem>_i2`, ... for later ones; those are the names manifests use.
3. Choose collision meshes from the bounds table (USD Z-up meters, largest
   first, marking meshes that enclose the reference camera):
   `python3 tools/quality/pbrt_scene.py --meshes <dir>/scene-v4.pbrt`.
   Shell meshes often include wall thickness, so their bounds reach past the
   visible interior; prefer the floor plus an inner wall or ceiling mesh as
   `envelope_meshes`. If the shell still reaches below the walkable floor by
   less than a slab, the collision step raises the shell floor to it.
4. Copy a manifest. Only per-scene *decisions* belong in it:

| Field | Meaning |
| --- | --- |
| `map` | lowercase map and material namespace (`maps/<map>.bsp`, `materials/<map>/`) |
| `scene` | PBRT file, repository-relative |
| `reference.render` | optional Cycles render of the exported stage (`samples`, `scale`, `device`) for visual comparison with the scene's reference image |
| `reference.gate` | `max_mae`, `min_ssim`, `max_exposure_stops` for that render against the supplied image (scored through the reference's own fitted display curve) |
| `runtime_gate` | `max_mae`, `min_ssim`, `max_grain_ratio`, `max_mottle_ratio`, optional `min_edge_f1` and `min_fine_edge_precision` for the camera-matched game frame against the Cycles render; the receipt always records orientation-aware structural and fine edge parity |
| `lightmap.size` / `samples` | atlas edge and Cycles samples (2048 / 64 default; the shipped manifests use 4096 samples) |
| `lightmap.device` | `cpu` (default), `gpu` (HIP/CUDA/OptiX/oneAPI/Metal, failing without one; HIP on the AMD Fedora host) or `auto` (GPU when Cycles finds one, else CPU); the reflection-probe and probe-volume bakes use the same device, and each receipt records it |
| `lightmap.exclude_materials` | extra materials that get no atlas space (transmissive and fully metallic ones never read the atlas and are always excluded) |
| `lightmap.denoise` | OpenImageDenoise `RTLightmap` pass on the atlas (default `true`; needs `libOpenImageDenoise.so.2`); the UV gutter fill still runs when this is `false` |
| `lightmap.preview_gain` | temporary display gain for the Source preview (default 1) |
| `world_mesh.weld_materials` / `weld_distance_source_units` | selected imported material names and a bounded Source-unit vertex weld to close measured near-coincident mesh seams (default: disabled); the pack receipt records affected corners and maximum movement |
| `reflection_probe` | optional map reflection probes (R50-PARALLAX), packed as the RPRB lump: `width` (equirect mip 0 of every probe, default 512), `face_size`, `samples`, `light_paths` (Cycles policy of the probe renders, default `blender-default`; never the diffuse-only `lightmap`), `placement` (overrides of `reflection_probe_set.PLACEMENT_DEFAULTS`: `max_probes`, `spacing_m`, `eye_height_m`, `glossy_roughness`, `glossy_radius_m`, `fade_m`, ...), `bounds_m` (placement bounds, default the probe volume's or the meshes'), `position`/`positions` (USD meters: captures placed first, kept for scenes whose default placement a wall would spoil). Probes are placed per room (every eye-height floor point covered by a probe whose box contains and sees it) and near glossy surfaces, rendered with the depth pass, fitted to a parallax box, GGX-prefiltered and blended per pixel by `world_pbr.frag` |
| `collision.envelope_meshes` | meshes whose bounds form the sealed shell (default: all meshes) |
| `collision.solid_materials` / `solid_meshes` | meshes that become solids: one convex 18-DOP brush per connected part, so separate stair treads or cushions stay separate |

5. Run the command above and inspect `<out>/reference/cycles.png` against the
   scene's reference image and `<out>/boot/` for the in-game frame. A failing
   pixel gate stops the build; `--keep-going` finishes the map anyway so you
   can inspect it, reports `gate-failed` in `<out>/build.json` and still
   exits nonzero. Do not loosen a gate to make a scene pass; record why it
   fails.

## Relighting a shipped map

`legacy_bsp_relight.py` rebakes a compiled legacy map (v20/v21) with the new
lighting and keeps its gameplay:

```sh
python3 tools/quality/legacy_bsp_relight.py testchmb_a_00 --boot
./play testchmb_a_00_relit
```

It writes a `legacy_bsp` manifest to `<out>/manifest.json`
(`quality-results/relight/<map>_relit` by default) and runs the pipeline with
the `legacy-relight` profile:

1. `legacy-scene` (`legacy_bsp_scene.py`) turns the map into a USD scene:
   - the opaque world faces, with materials from their VMTs and VTFs;
   - the nodraw sides of the solid world brushes, as occluders;
   - vrad's compiled world lights, converted to physical lights.
   `legacy-scene/scene-receipt.json` lists every conversion and approximation.
2. The usual bakes run on that scene: direct and indirect LMAP layers, PRBV,
   RTRN and SDFV. With all of them, every indirect-light producer can run on
   the map, including `r_indirect_producer sdf`.
3. The BSP itself stands in for `collision` and `compile`.
4. `gameplay-identity.json` proves that every legacy lump is carried byte for
   byte, except two lighting-only kinds:
   - the leaf ambient lumps, rederived from the probe volume;
   - the world lights, where vrad's baked lights are removed so models are not
     lit twice.
   The run fails on any other difference.

The SDFV carries point lights as spheres and spot lights as disks with vrad's
cone, and its light cells use the map's PVS, so `r_indirect_producer sdf`
traces only the lights near each probe. The producer updates the probes
around the camera every update and the rest in turn (`r_indirect_focus`,
`r_indirect_probe_budget`). `--quality legacy-relight-preview` builds in
minutes instead of hours. Boot on a client with the RTRN/SDFV loaders, such
as `--build build`: the toolchain's `client_build` offers only `baked`.

The engine replaces only the opaque world with the relit world mesh. Brush
entities, displacements, water and translucent faces keep their vrad
lightmaps. Other current limits, each recorded in the receipt:

- constant and linear falloffs are matched at vrad's 100-unit normalization
  distance;
- named lights that start dark stay world lights and are not baked;
- static props do not occlude the bake.

## Hooking a regular compile into the pipeline

`vrad_cycles.py` is a drop-in for vrad. Anything that runs vrad can run it
with the same command line: Hammer's `$light_exe`, or a compile script that
runs vbsp, then vvis, then vrad.

```sh
T=build/toolchains/pbrt-map-tools
$T/vbsp -game "$GAME" maps/mymap
$T/vvis -game "$GAME" maps/mymap
tools/quality/vrad_cycles.py -game "$GAME" -final maps/mymap   # was: $T/vrad ...
./play mymap_relit
```

It runs the real vrad with every argument except its own `--cycles-*`
options. It then relights the BSP vrad wrote, the same way as a shipped map
(`legacy_bsp_relight.relight`, described above):

- vvis's visibility and vrad's lightmaps for brush entities, displacements
  and translucent faces are carried unchanged;
- Cycles bakes the opaque world, the PRBV, the RTRN and the SDFV from vrad's
  lights;
- the map is published as `<map>_relit`.

vrad still has to run in full: the relight converts its world lights, and
the legacy renderer uses its lightmaps for the faces the relight leaves out.

vrad's `-game` (or `-vproject`, or `$VPROJECT`) directory is searched for
the map's materials before the toolchain's game runtime, as vrad searched it
(manifest key `legacy_game`).

The relight never writes vrad's BSP, so a failed bake leaves the regular
compile's output usable, and the exit status is nonzero. If vrad fails, the
relight does not run and vrad's exit status is returned.

Options:

| Option | Default |
| --- | --- |
| `--cycles-quality NAME` | `legacy-relight`; `legacy-relight-preview` builds in minutes |
| `--cycles-map-name NAME` | `<map>_relit` |
| `--cycles-out DIR` | `quality-results/relight/<name>` |
| `--cycles-vrad PATH` | the toolchain's `vrad` |
| `--cycles-toolchain FILE` | the provisioned toolchain |
| `--cycles-from STEP` | none |
| `--cycles-device gpu\|cpu\|auto` | the profile's `cpu`; `gpu` and `auto` are explicit opt-ins |
| `--cycles-boot` | off |
| `--cycles-keep-going` | off |
| `--cycles-no-publish` | off |

The `legacy-relight` profiles exclude their occluder material only from maps
that have one. A map with no nodraw brush sides has no occluders. Exclusions
a manifest names must still exist.

## Toolchain

[`pbrt-map-linux-tools.json`](../../product_profiles/pbrt-map-linux-tools.json)
owns the pipeline's tool pins and install layout; OpenUSD/oneTBB and
KTX-Software pins stay in their own host profiles. `pbrt_map_toolchain.py
provision` clones those exact revisions (`--mirror NAME=PATH` fetches from a
local clone), builds OpenUSD and KTX, installs `vbsp2`/`vvis`/`vrad`/`vtex`/
`bsp2tool` from this checkout with a private Waf lock, and writes
`build/toolchains/pbrt-map-toolchain.json`, which `pbrt_map_build.py` uses by
default. `pbrt_map_toolchain.py check` (and `pbrt_map_build.py
--check-toolchain`) verify Blender and OIDN versions, the OpenUSD Python
version, the KTX revision, the compile tools and `bsp2tool pack-world-lit`.
`--toolchain FILE` still accepts a hand-written `pbrt-map-toolchain/v1` file,
checked the same way. The game runtime and client build are inputs from the
native Vulkan Portal profile, not provisioned here.

Every Cycles step bakes on the CPU (user decision, 2026-09-25;
[`cycles_device.py`](../../../tools/quality/cycles_device.py) owns the policy).
That day this host's HIP compute wedged (amdgpu "MES failed to respond to
msg=REMOVE_QUEUE"), and every Cycles GPU job hung until a reboot, while CPU bakes
kept working. The CPU is also the one device with bit-identical bakes and Cycles'
path guiding. On the GPU (HIP), the living room's 4096-sample 2048² bake took
about 6 minutes; its CPU time has not yet been measured. The bake merges the
baked meshes into one bake-only object, because Blender otherwise runs one
render job per object and re-syncs the scene each time.

On Fedora, Cycles HIP needs `rocm-hip` and `rocm-runtime` (Fedora's `blender`
package ships precompiled kernels, including `gfx1151`), plus membership of the
`render` and `video` groups for `/dev/kfd` and `/dev/dri/renderD*`. `rocminfo`
and `rocm-smi` are optional diagnostics; they matter only when a manifest opts in
with `lightmap.device: gpu`, which fails with "no Cycles GPU device is
available" on a host without one. Check `dmesg` for amdgpu MES errors before
blaming a stalled GPU bake on load.

Correctness checks also run on the CPU:
- the pipeline's reference render;
- `gi_reference.py`;
- the basis oracle and supplemental bakes;
- the `gi-fixture` export profile.

On this host a CPU bake is bit-identical across runs at the same seed, while
two HIP bakes of the same scene differ. CPU+GPU hybrid rendering was measured
about 24% slower than the GPU alone (a shared power budget), so it is not
offered.

## Owners

| Knowledge | Owner |
| --- | --- |
| PBRT parsing, axis conversion, camera pose, sky resampling, material translation policy | `tools/quality/pbrt_scene.py` (tests: `tools/quality/tests/test_pbrt_scene.py`) |
| Blender materials, emitters, sky, camera | `tools/quality/pbrt_blender.py` |
| PBRT → USD stage + reference render | `tools/quality/pbrt_usd_stage.py` |
| Lightmap UVs + Cycles irradiance and undilated coverage bakes | `tools/quality/pbrt_lightmap_bake.py` |
| Coverage-based gutter fill and atlas denoising | `tools/quality/lightmap_denoise.py` |
| Bounded mesh seam repair | `tools/quality/worldmesh_seam_weld.py` |
| KTX2 LMAP packaging | `tools/quality/lightmap_ktx2.py` (`--expected-scope`) |
| Sky dome render stage | `tools/quality/pbrt_sky_dome.py` |
| Reflection probe placement and faces (with depth) | `tools/quality/pbrt_reflection_probe.py` |
| Per-probe math: GGX chain, parallax-box fit, corrected lookup, distance roughness | `tools/quality/reflection_probe.py` (tests: `tests/test_reflection_probe_set.py`, `tests/test_pbrt_gates.py`) |
| RPRB encoding, blend, placement, packing | `tools/quality/reflection_probe_set.py` (C++ reader: `mapcontainer/reflection_probes.cpp`, suite `world.reflection-probes`; shader: `shaders/reflection_probes.glsl`, suite `render.reflection-probes.glsl`) |
| Reflection fixtures and in-game probe gates | `tools/quality/reflection_fixtures.py`, `tools/quality/reflection_runtime.py` |
| Collision shell, solids, spawn | `tools/quality/pbrt_collision_vmf.py` |
| USD → WMSH/BSP2 | `tools/quality/usd_worldmesh_pack.py` |
| VTF/VMT content | `tools/quality/pbrt_playable_content.py` (VTF helpers: `vtf_content.py`) |
| Reference, runtime, grain and mottle gates | `tools/quality/reference_compare.py` (tests: `tests/test_pbrt_gates.py`) |
| Collision drop test | `tools/quality/pbrt_traversal.py` (tests: `tests/test_pbrt_gates.py`) |
| Tool pins, provisioning, toolchain checks | `tools/quality/pbrt_map_toolchain.py` + `quality/product_profiles/pbrt-map-linux-tools.json` |
| Step order and caching | `tools/quality/pbrt_map_build.py` |
| Legacy BSP reading (faces, brushes, world lights) | `tools/quality/legacy_bsp.py` (tests: `tests/test_legacy_relight.py`) |
| Legacy map → relight scene, vrad light conversion | `tools/quality/legacy_bsp_scene.py` |
| VTF decoding | `tools/quality/vtf_decode.py` |
| Relight driver and gameplay-identity oracle | `tools/quality/legacy_bsp_relight.py` |
| vrad drop-in that relights a regular compile | `tools/quality/vrad_cycles.py` (tests: `tests/test_vrad_cycles.py`) |
| SDFV light cells (range, side, PVS culling) | `tools/quality/sdf_light_cells.py` (tests: `tests/test_sdf_light_cells.py`) |
| Traced producers' probe focus, independent of the engine host | `tools/quality/gi_focus.py` |
| Bake progress from Cycles' log | `tools/quality/bake_progress.py` (tests: `tests/test_bake_progress.py`) |
| Publishing to `./play` (store, mounts, launch arguments) | `tools/quality/playable_maps.py` (tests: `tests/test_playable_maps.py`) |

## Known limits (preview, not RFC 0008 acceptance)

- Collision is one convex 18-DOP per connected part plus extruded floor
  triangles: exact for treads and boxes, conservative for curved furniture.
- Visibility is conservative: each BSP leaf references the meshlets that can
  be seen through it (`tools/quality/worldmesh_leaf_visibility.py`). The
  engine culls groups of 32 meshlets to the view frustum, by WMSH v2
  front-face cones (materials without `$nocull`), and by software occlusion
  against the map's large opaque triangles (`engine/worldmesh_cull.h`).
  Furniture collision is nodraw `func_detail` (collision is not render
  geometry), so a single-room scene is one vis cluster: PVS culls nothing
  there. Finely tessellated furniture (cloth, carpet) has no triangle large
  enough to occlude, so occlusion mostly comes from walls, floors and flat
  furniture. Check a map with `tools/quality/worldmesh_visibility_oracle.py`:
  culled frames must be byte-identical to drawing every meshlet, and each
  stage's negative control must change a frame.
- Windows show the scene's sky through an unlit `SkyDome` (display-mapped,
  not HDR). Reflection probes are GGX-prefiltered and parallax-corrected
  against an axis-aligned box per probe, and blend between probes; a box
  only approximates a room with furniture or a non-rectangular shape, so a
  mirror shows walls flat and furniture misplaced (the audit gates each
  box's fit residual). Probes see no dynamic objects. Maps built before
  RPRB keep one direction-only probe in their LMAP band, which the shader
  still reads. Refraction and emissive WMSH batches are absent; glass is an
  alpha preview.
- PBRT coated, spectral and transmissive materials reduce to a single
  metal/roughness layer; each material's approximation is in the content
  receipt.
- The playable spawn uses the player's eye height; only the camera-matched
  gate boot places the eye at the reference camera (noclip).
- A mesh with more than 4096 separate parts (fur, grass, foliage cards) gets
  one planar lightmap chart of its footprint, so its parts share the light
  at their position rather than having their own charts. The bake receipt
  lists these under `projected_meshes`.
- PBRT `diffusetransmission` renders exactly in Cycles (diffuse +
  translucent lobes); the game material keeps only the reflected lobe.

## Retired staircase2 scripts

The first staircase2 slice used scene-specific scripts (`staircase2_usd_cycles`,
`_lightmap_bake`, `_collision_vmf`, `_playable_content`, `_compare`,
`_materials`, `_runtime_compare`, `_stage_equivalence`, `_grain_repro`). They
were retired on 2026-09-23 after `staircase2.json` regenerated their outputs
through this pipeline with every gate passing; their packager survives as
`lightmap_ktx2.py`. They remain in git history. Receipts they produced stay
valid as recorded evidence.
