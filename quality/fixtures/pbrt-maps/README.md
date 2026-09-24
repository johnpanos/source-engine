# PBRT reference scenes → playable BSP2 maps

One command turns a PBRT-v4 scene (the format of Benedikt Bitterli's
[rendering resources](https://benedikt-bitterli.me/resources/)) into a Portal
BSP2 map with a USD-derived WMSH world mesh, a Cycles-baked KTX2 lightmap,
Source materials, collision and a spawn at the reference camera:

```sh
python3 tools/quality/pbrt_map_build.py \
  --manifest quality/fixtures/pbrt-maps/living-room.json \
  --toolchain quality-results/pbrt-toolchain.json \
  --out quality-results/living-room-map --boot
```

`--boot` runs the map headless on the native Vulkan provider (no window on the
desktop) and saves a screenshot under `<out>/boot/`. Re-running is incremental:
`<out>/steps.json` records input, settings and script digests per step, so a
collision edit does not repeat the bake. `--from STEP` forces a step and all
later ones. Logs are in `<out>/logs/<step>.log`; the failing step prints its
tail.

## Adding a scene

1. Put the scene directory (`scene-v4.pbrt`, `models/`, `textures/`, license)
   at the repository root, like `living-room/` and `staircase2/`.
2. Check the scene reader accepts it and read its inventory (materials,
   meshes by material, camera, sky):
   `python3 tools/quality/pbrt_scene.py <dir>/scene-v4.pbrt`.
   Unsupported PBRT features fail loudly; extend `tools/quality/pbrt_scene.py`
   and its tests rather than special-casing a scene.
3. Copy a manifest. Only per-scene *decisions* belong in it:

| Field | Meaning |
| --- | --- |
| `map` | lowercase map and material namespace (`maps/<map>.bsp`, `materials/<map>/`) |
| `scene` | PBRT file, repository-relative |
| `reference.render` | optional Cycles render of the exported stage (`samples`, `scale`) for visual comparison with the scene's reference image |
| `lightmap.size` / `samples` | atlas edge and Cycles samples (2048 / 64 default) |
| `lightmap.exclude_materials` | extra materials that get no atlas space (transmissive and fully metallic ones never read the atlas and are always excluded) |
| `lightmap.denoise` | OpenImageDenoise `RTLightmap` pass on the atlas (default `true`; needs `libOpenImageDenoise.so.2`) |
| `lightmap.preview_gain` | temporary display gain for the Source preview (default 1) |
| `collision.envelope_meshes` | meshes whose bounds form the sealed shell (default: all meshes) |
| `collision.solid_materials` / `solid_meshes` | meshes that become axis-aligned solids |

4. Run the command above and inspect `<out>/reference/cycles.png` against the
   scene's reference image and `<out>/boot/` for the in-game frame.

## Toolchain file

Machine paths live outside the manifest (`pbrt-map-toolchain/v1`); relative
paths resolve from the repository root. `--check-toolchain` validates it
(executables exist, compile tools are complete, `bsp2tool` supports
`pack-world-lit`, `pxr` imports) and exits. The `/tmp` compile-tool install
predates `pack-world-lit`, so `bsp2tool` comes from the Waf tools tree.

```json
{
  "schema": "pbrt-map-toolchain/v1",
  "blender": "blender",
  "ocio": "quality/fixtures/staircase2-ocio/config.ocio",
  "usd_python": "/usr/bin/python3.12",
  "usd_pythonpath": "/tmp/rfc0008-openusd-install/lib/python",
  "compile_tools": "/tmp/rfc0008-worldstage-clean4-install",
  "bsp2tool": "build-rfc0008-tools-vbsp/utils/bsp2tool/bsp2tool",
  "ktx": "/tmp/rfc0008-ktx-pin/build-rfc0008/Release/ktx",
  "runtime": "run/runtime-native",
  "client_build": "build-rfc0008-staircase-lit-client"
}
```

These are the unpinned local installs used so far; RFC 0008 progress records
how they were built. They are not yet product-profile pinned dependencies.

## Owners

| Knowledge | Owner |
| --- | --- |
| PBRT parsing, axis conversion, camera pose, sky resampling, material translation policy | `tools/quality/pbrt_scene.py` (tests: `tools/quality/tests/test_pbrt_scene.py`) |
| Blender materials, emitters, sky, camera | `tools/quality/pbrt_blender.py` |
| PBRT → USD stage + reference render | `tools/quality/pbrt_usd_stage.py` |
| Lightmap UVs + Cycles irradiance bake | `tools/quality/pbrt_lightmap_bake.py` |
| Atlas denoising | `tools/quality/lightmap_denoise.py` |
| KTX2 LMAP packaging | `tools/quality/staircase2_lightmap_ktx2.py` (`--expected-scope`) |
| Collision shell, solids, spawn | `tools/quality/pbrt_collision_vmf.py` |
| USD → WMSH/BSP2 | `tools/quality/usd_worldmesh_pack.py` |
| VTF/VMT content | `tools/quality/pbrt_playable_content.py` |
| Step order, caching, toolchain | `tools/quality/pbrt_map_build.py` |

## Known limits (preview, not RFC 0008 acceptance)

- Collision is axis-aligned boxes; stairs and furniture are coarse blocks.
- Every WMSH leaf references every meshlet (no spatial visibility yet).
- Refraction, mirrors/reflection probes, emissive WMSH batches and sky
  rendering through windows are absent in game; glass is an alpha preview.
- PBRT coated, spectral and transmissive materials reduce to a single
  metal/roughness layer; each material's approximation is in the content
  receipt.
- The in-game eye height is the player's, not the reference camera's.
