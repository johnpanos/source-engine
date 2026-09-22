# Material pixel references (`source-material-pixels/v1`)

D3D9 reference captures for `tools/quality/material_pixel_conformance.py`. Each
file is the output of `material_pixel_conformance` (the harness in
`unittests/shaderextensiontest/`) rendering `LightmappedGeneric` through the real
material system on the D3D9 backend. The oracle holds every other backend to
these pixels within `PIXEL_TOLERANCE` (3 levels per channel).

| File | HDR mode | Notes |
| --- | --- | --- |
| `lightmap-dx9-none.json` | `HDR_TYPE_NONE` | 8-bit sRGB lightmap pages, scale `2^2.2` |
| `lightmap-dx9-integer.json` | `HDR_TYPE_INTEGER` | 16-bit lightmap pages, scale 16; also matches the closed-form model exactly |

Captured 2026-09-22 with:

- source `15cf3d78` plus a dirty tree (digest `4598582a…`), `build-portal-vulkan` (DXVK);
- DXVK 2.7.1 on AMD Radeon 8060S (RADV 26.2.2);
- Wayland at 1.5× scale, so the drawable is 384×384.

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build-portal-vulkan --renderer vulkan-compat --hdr none --out OUT
```

Recapture only when the harness cases or the D3D9 path change, and review the
pixel differences before replacing a file. A reference that silently absorbs a
D3D9 regression would pass every candidate that shares it.
