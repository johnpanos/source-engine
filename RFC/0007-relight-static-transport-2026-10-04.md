# Legacy relight static transport evidence — 2026-10-04

This is the bounded legacy producer slice described in
[RFC 0007 progress](0007-progress.md#legacy-relight-displacement-and-static-transport-slice-2026-10-04).
The source2 production quality profile is unchanged. No newly baked Intro4
package has been published, and no game-image or performance gate is claimed.

## Installed checks

From the repository root, with the installed map-tool profile:

```sh
PYTHONPATH=build/toolchains/openusd-25.11/lib/python OPENBLAS_NUM_THREADS=1 \
  /usr/bin/python3.12 -m unittest discover -s tools/quality/tests \
  -p test_legacy_displacements.py -v
PYTHONPATH=build/toolchains/openusd-25.11/lib/python OPENBLAS_NUM_THREADS=1 \
  /usr/bin/python3.12 -m unittest discover -s tools/quality/tests \
  -p test_legacy_relight_integration.py -v
```

The 14 unit tests cover powers/start corners, independent triangle topology,
holes, normals, vertex-alpha interpolation, modulation, sRGB blending, Source
UV transforms/lookups, static brush state selection, overlay binary parsing,
paint projection/density and malformed input. The six integration tests cover the real
BSP-to-USD lift, coverage semantics, charting, unpainted runtime albedo, native
VTEX clamp flags, WMSH deformation/UV round-trip, a nonzero Cycles bake with
preserved geometry, static versus triggered decals, and brush placement.
The integration fixture deliberately uses CPU Cycles at 256 pixels/16 samples;
production still requires source2's GPU and full samples/resolution. Blender
Python errors are fatal and missing tools are not skipped.

The existing legacy regression collection passed 102 tests before the final
additional transformed-lookup and paint-density unit tests. OpenUSD scene extraction passed 16,
bake invariants 18, map-build runner four, architecture fixtures 166 and style
fixtures 38. Architecture baseline and inventory verification pass. Full
architecture checking retains the existing CAP002 unresolved `charconv`
include in `public/gameui/graphics_settings_service.h`. Changed-line style and
`git diff --check` pass. Logs are retained under
`quality-results/ratman-floor/`.

## Shipped Intro4 content check

```sh
PYTHONPATH=build/toolchains/openusd-25.11/lib/python OPENBLAS_NUM_THREADS=1 \
  /usr/bin/python3.12 tools/quality/legacy_bsp_scene.py \
  --bsp "$HOME/.local/share/Steam/steamapps/common/Portal 2/portal2/maps/sp_a1_intro4.bsp" \
  --runtime run/runtime-p2-fsr --map-name sp_a1_intro4_relit \
  --model-tool build-p2-fsr/mdl/mdl_mesh_export \
  --out quality-results/ratman-floor/final-scene
PYTHONPATH=build/toolchains/openusd-25.11/lib/python OPENBLAS_NUM_THREADS=1 \
  /usr/bin/python3.12 tools/quality/usd_scene.py extract \
  --scene quality-results/ratman-floor/final-scene/scene.usda \
  --out-scene quality-results/ratman-floor/final-scene/scene.json \
  --out-stage quality-results/ratman-floor/final-scene/normalized.usdc
```

The export has 3076 lifted faces, 305 material records and 1961 solid nodraw
occluders. Only the 21 sky openings remain excluded world faces. Displacements
0/1 belong to faces 3093/3094: 81 vertices and 128 triangles each, retaining
both texture layers, normals, modulation and ssbump occlusion. Their material
tiles are 8192×4096 and 8192×8192 to retain authored texel density.

There are 56 authored overlays, with 202 painted receiver surfaces. Fifteen
proxy-driven overlays remain excluded; two selfillum signs contribute diffuse
paint with emission explicitly unbaked. Intro4 has no infodecal entities, so
the synthetic integration fixture exercises that cohort. Brush models *32/*36
contribute one transport face each. Metalgrate018/018b coverage joins transport
while its runtime draw remains separate.

The installed layout command passes at source2's 4096 atlas: 1170 charts,
23.499 texels/m, 86.923% atlas fill. The layout uses the existing source2
exclusions plus the receipt's static-prop/static-transport materials through
`map_scene.lightmap_exclusions`, exactly as `pbrt_map_build` does. The scene,
normalized stage, charted stage, layout receipt and independent WMSH check are
retained in `quality-results/ratman-floor/final-scene/`. A production rebake and
matched game capture remain separate acceptance work.
