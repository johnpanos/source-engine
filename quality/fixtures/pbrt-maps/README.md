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
| `lightmap.device` | `auto` (default: GPU when Cycles finds HIP/CUDA/OptiX/oneAPI/Metal), `gpu` (fail without one) or `cpu`; the bake receipt records the device |
| `lightmap.exclude_materials` | extra materials that get no atlas space (transmissive and fully metallic ones never read the atlas and are always excluded) |
| `lightmap.denoise` | OpenImageDenoise `RTLightmap` pass on the atlas (default `true`; needs `libOpenImageDenoise.so.2`); the UV gutter fill still runs when this is `false` |
| `lightmap.preview_gain` | temporary display gain for the Source preview (default 1) |
| `world_mesh.weld_materials` / `weld_distance_source_units` | selected imported material names and a bounded Source-unit vertex weld to close measured near-coincident mesh seams (default: disabled); the pack receipt records affected corners and maximum movement |
| `reflection_probe` | optional map probe: `width` (equirect mip 0, default 512), `face_size`, `samples`, `position` (USD meters; default the scene's horizontal center at the reference eye height). The bake reserves `width/2` atlas rows; the probe's roughness mips and a marker texel are stored there and `world_pbr.frag` adds split-sum specular from them |
| `collision.envelope_meshes` | meshes whose bounds form the sealed shell (default: all meshes) |
| `collision.solid_materials` / `solid_meshes` | meshes that become solids: one convex 18-DOP brush per connected part, so separate stair treads or cushions stay separate |

5. Run the command above and inspect `<out>/reference/cycles.png` against the
   scene's reference image and `<out>/boot/` for the in-game frame. A failing
   pixel gate stops the build; `--keep-going` finishes the map anyway so you
   can inspect it, reports `gate-failed` in `<out>/build.json` and still
   exits nonzero. Do not loosen a gate to make a scene pass; record why it
   fails.

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

GPU baking is the practical path to noise-free lightmaps: the living room's
4096-sample 2048² bake takes about 6 minutes on a Radeon 8060S (HIP). The bake
merges the baked meshes into one bake-only object, because Blender otherwise
runs one render job per object and re-syncs the scene each time.

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
| Reflection probe faces / layout | `tools/quality/pbrt_reflection_probe.py` / `reflection_probe.py` (read by `world_pbr.frag`) |
| Collision shell, solids, spawn | `tools/quality/pbrt_collision_vmf.py` |
| USD → WMSH/BSP2 | `tools/quality/usd_worldmesh_pack.py` |
| VTF/VMT content | `tools/quality/pbrt_playable_content.py` (VTF helpers: `vtf_content.py`) |
| Reference, runtime, grain and mottle gates | `tools/quality/reference_compare.py` (tests: `tests/test_pbrt_gates.py`) |
| Collision drop test | `tools/quality/pbrt_traversal.py` (tests: `tests/test_pbrt_gates.py`) |
| Tool pins, provisioning, toolchain checks | `tools/quality/pbrt_map_toolchain.py` + `quality/product_profiles/pbrt-map-linux-tools.json` |
| Step order and caching | `tools/quality/pbrt_map_build.py` |
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
  not HDR). One reflection probe per map feeds mirrors and glossy floors:
  blurry mips rather than a GGX prefilter, and no parallax correction, so
  reflections are right near the probe and approximate elsewhere.
  Refraction and emissive WMSH batches are absent; glass is an alpha preview.
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
