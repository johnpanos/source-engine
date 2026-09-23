# Material pixel references (`source-material-pixels/v1`)

Backend reference captures for `tools/quality/material_pixel_conformance.py`.
Each file is the output of `material_pixel_conformance` (the harness in
`unittests/shaderextensiontest/`) rendering real Source materials through the
real material system. The backend is identified by each filename and report.

| File | Family | HDR mode | Notes |
| --- | --- | --- | --- |
| `lightmap-dx9-none.json` | lightmap | `HDR_TYPE_NONE` | 8-bit sRGB lightmap pages, scale `2^2.2` |
| `lightmap-dx9-integer.json` | lightmap | `HDR_TYPE_INTEGER` | 16-bit lightmap pages, scale 16; also matches the closed-form model exactly |
| `exposure-dx9-none.json` | exposure | `HDR_TYPE_NONE` | luminance histogram (`dev/lumcompare` under occlusion queries) |
| `exposure-dx9-integer.json` | exposure | `HDR_TYPE_INTEGER` | as above; the path integer-HDR auto-exposure runs |
| `portal-dx9-none.json` | portal | `HDR_TYPE_NONE` | stencil portal recursion (`PortalRefract` stages 0-2, depth 2); three 256x256 frames |
| `modellight-dx9-none.json` | modellight | `HDR_TYPE_NONE` | VertexLitGeneric lighting: ambient cube, point/spot/directional lights, half-Lambert, static color mesh, MODEL and skinned placement; eleven 256x256 frames |
| `modellight-dx9-integer.json` | modellight | `HDR_TYPE_INTEGER` | as above, linear tone-mapping scale 0.75 |
| `cable-dxvk-none.json` | cable | `HDR_TYPE_NONE` | Cable_DX9 normal-map half-Lambert captured through DXVK with front, side, back and diagonal normals |
| `sky-dxvk-none.json` | sky | `HDR_TYPE_NONE` | Sky_DX9 texture and tint, with translated texture coordinates sampled on both sides |
| `monitor-dxvk-none.json` | monitor | `HDR_TYPE_NONE` | MonitorScreen_DX9 base and second images, contrast, saturation, tint, and independent UV transforms |

`pbr-fallback-primary.vmt` and `pbr-fallback-legacy.vmt` are authored inputs
for the `pbr-fallback` family. The driver copies them into a private runtime
as `conformance/pbr_case.vmt` and `conformance/pbr_fallback.vmt`. A valid
primary patch must resolve through the same PBR root to the legacy fallback.
Twelve invalid VMTs omit a required field, name an absent or self fallback,
use a traversal path directly or through a nested fallback or primary patch,
name a missing primary patch include, reference an unsupported/PBR shader,
or reach a patch cycle.
Its oracle checks the resolved legacy shader and green rendered pixels for
both the direct PBR material and the valid primary patch, and
requires the material loader to reject each invalid VMT. It does not use a
D3D9 reference capture.

Lightmap pixels are held to these references within `PIXEL_TOLERANCE` (3 levels
per channel). Exposure counts are exact: every luminance range must equal the
geometric model and, per pixel, the reference. D3D9 ran with 4x MSAA
(`aa_samples` 4), so its queries count four samples per pixel. The oracle
measures that unit with the all-pixels range, as the client does.

Captured 2026-09-22, headless (SDL `offscreen` driver, `VK_EXT_headless_surface`,
drawable 256x256), with:

- source `a33a3b87` plus a dirty tree, `build-portal-vulkan` (DXVK);
- DXVK 2.7.1 on AMD Radeon 8060S (RADV 26.2.2).

The lightmap pixels are identical to the earlier captures made on a 1.5x Wayland
desktop (384x384).

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build-portal-vulkan --renderer vulkan-compat --hdr none \
    --family exposure --out OUT
```

Portal frames are held to the reference within 1 level per channel, with at
most 8 pixels beyond it (`material_pixel_portal.py`); native Vulkan measured one
channel of one pixel one level off. The harness pins the sampling configuration
on every backend (one sample per pixel, `mat_trilinear 1`, no forced
anisotropy) and the oracle rejects a capture without it. The D3D9 run needs
`DXVK_CONFIG="dxvk.enableGraphicsPipelineLibrary = False"`, which the runner sets
for this family: DXVK 2.7.1's fast-linked graphics-pipeline-library pipelines
ignore D3D9 user clip planes, so the recursion's clipped blocker renders
unclipped. Captured 2026-09-22 from source `3d3e5e68` plus a dirty tree, with the
toolchain above:

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build-portal-vulkan --renderer vulkan-compat --hdr none \
    --family portal --out OUT
```

Model lighting frames are held to an independent evaluation of
`common_vs_fxc.h` DoLighting (`material_pixel_modellight.py`), which D3D9 matches
within one level everywhere it judges, and to these references within 1 level
with no pixel beyond it. Captured 2026-09-22 from source `8c2c4268` plus a dirty
tree, with the toolchain above (`--family modellight`, both HDR modes).

Cable pixels are checked against the `cable_ps2x.fxc` half-Lambert equation
and the observed DXVK capture within 3 levels per channel. The capture was made
2026-09-23 UTC from source `fe0aff17` plus the recorded dirty tree, with DXVK
2.7.1 on AMD Radeon 8060S (RADV 26.2.2):

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime-dxvk \
    --build build-r03-portal-dxvk --renderer vulkan-compat --hdr none \
    --family cable --out OUT
```

Sky pixels are checked against sky_ps2x.fxc's sRGB texture, $color, and
linear-output equation, and against the DXVK capture within 3 levels per channel.
Two-sided samples detect wrong sky texture-transform registers. The capture was
made 2026-09-23 UTC from source fe0aff17 plus the recorded dirty tree with DXVK
2.7.1 on AMD Radeon 8060S (RADV 26.2.2):

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime-dxvk \
    --build build-r03-portal-dxvk --renderer vulkan-compat --hdr none \
    --family sky --out OUT
```

Monitor pixels are checked against monitorscreen_ps2x.fxc's two sRGB inputs,
contrast, saturation, and tint where the sample lies in a flat texel region.
The translated UV case uses the DXVK reference because the 4x4 textures are
filtered at those coordinates. Native and DXVK measured the same RGB bytes in
all four cases. Captured 2026-09-23 UTC with the native and DXVK product
profiles above:

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime-dxvk \
    --build build-r03-portal-dxvk --renderer vulkan-compat --hdr none \
    --family monitor --out OUT
```

Recapture only when the harness cases or the D3D9 path change, and review the
differences before replacing a file. A reference that silently absorbs a D3D9
regression would pass every candidate that shares it.
