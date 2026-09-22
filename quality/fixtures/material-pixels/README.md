# Material pixel references (`source-material-pixels/v1`)

D3D9 reference captures for `tools/quality/material_pixel_conformance.py`. Each
file is the output of `material_pixel_conformance` (the harness in
`unittests/shaderextensiontest/`) rendering real Source materials through the
real material system on the D3D9 backend.

| File | Family | HDR mode | Notes |
| --- | --- | --- | --- |
| `lightmap-dx9-none.json` | lightmap | `HDR_TYPE_NONE` | 8-bit sRGB lightmap pages, scale `2^2.2` |
| `lightmap-dx9-integer.json` | lightmap | `HDR_TYPE_INTEGER` | 16-bit lightmap pages, scale 16; also matches the closed-form model exactly |
| `exposure-dx9-none.json` | exposure | `HDR_TYPE_NONE` | luminance histogram (`dev/lumcompare` under occlusion queries) |
| `exposure-dx9-integer.json` | exposure | `HDR_TYPE_INTEGER` | as above; the path integer-HDR auto-exposure runs |

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

Recapture only when the harness cases or the D3D9 path change, and review the
differences before replacing a file. A reference that silently absorbs a D3D9
regression would pass every candidate that shares it.
