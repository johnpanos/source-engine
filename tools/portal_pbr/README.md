# Portal 1 PBR texture staging

This workflow converts owned Portal 1 VPK content into editable PBR texture
sets and candidate `PBRMetalRough` VMT definitions. Keep generated game assets
outside this repository. The original VMT remains the compatibility reference;
the generated VMT is staging data until the material system implements and
validates the new family.

## Native Vulkan contract checked on 2026-09-22

The current native Vulkan shader is
[`pbr_direct.frag`](../../materialsystem/shaderapivulkan/shaders/pbr_direct.frag).
It samples **linear** base color at descriptor set 0, binding 0, and **linear**
MRAO at set 1, binding 0. MRAO R is metalness, G is perceptual roughness, and B
is AO. The shader clamps roughness to at least 0.02, forms dielectric F0 0.04
or metal F0 from base color, then evaluates GGX with `alpha = roughness²`,
height-correlated Smith visibility, and Schlick Fresnel. It uses dot products
and incident radiance passed as push constants. AO is unused in this direct
light term. The scalar reference is
[`pbr_brdf.h`](../../public/render/pbr_brdf.h).

[`vulkan_device.cpp`](../../materialsystem/shaderapivulkan/vulkan_device.cpp)
creates this as an optional dynamic shader pipeline. Its
`PbrDirectPipelineSupported()` flag means that the synthetic pipeline was
created, not that the renderer supports the full `material.pbr.v1` capability.
[`test_pbr_pixels.cpp`](../../unittests/shaderapivulkantest/test_pbr_pixels.cpp)
checks three rendered pixels against the scalar BRDF. The recorded Linux RADV
run passed 18 checks; the Vulkan validation layer was unavailable. See
[`RFC/0007-progress.md`](../../RFC/0007-progress.md) for the command and
evidence.

The material parameter names and encodings are now owned by
[`pbr_material_schema.h`](../../public/render/pbr_material_schema.h), which the
export helper reads directly. A candidate requires `$basetexture`,
`$mraotexture`, and `$fallbackmaterial`. The intended material behavior is in
[`RFC/0007`](../../RFC/0007-physically-based-lighting-pipeline.md#pbr-material-family):
`$basetexture` sRGB, `$mraotexture` linear R/G/B metalness/roughness/AO,
`$bumpmap` linear tangent normal, and optional sRGB emission. The runtime
material binding, sRGB image sampling, normal mapping, diffuse, world lights,
lightmaps, probes, IBL, KTX2 path, and validated D3D9/DXVK fallback are still
outstanding. In particular, the current pixel fixture supplies linear texels;
it does not establish how a Portal sRGB VTF will be sampled. Merely writing a
`PBRMetalRough` VMT does not make it render correctly in a game build. The
existing D3D9 `PBR` shader is a different family and must not be substituted.

## Conversion rules

- Use the existing `hammer::formats` VPK reader, VMT parser, and VTF decoder.
  VTF decoding currently covers mip 0, frame 0, face 0 only. Unsupported
  pixel formats, animated and cube/volume textures, and unreadable sources
  must be reported rather than silently replaced.
- Export albedo to PNG with its encoded color values preserved. The intended
  runtime slot is sRGB, while MRAO and normal slots are linear. ArmorPaint PNGs
  are editable intermediates; final runtime KTX2 encoding awaits RFC 0008.
- Reuse an authored `$bumpmap` where its VTF is supported. Do not infer a normal
  map from color. Mark Source-specific normal encodings such as SSBump for
  manual review.
- Begin with dielectric metalness 0, roughness 0.8 and AO 1. These are explicit
  placeholders, not recovered material properties. A numeric `$phongexponent`
  can provide an approximate roughness using `sqrt(2 / (exponent + 2))`;
  document that inference per material. Exponent textures and Fresnel ranges
  need review because a direct conversion is not specified.
- Carry alpha, surface property and useful legacy parameters into the staging
  report. Materials using proxies, refractive/water effects, additive blending,
  animation or other unhandled behavior require review. Preserve the original
  VMT in a separate fallback path. The schema requires that reference and this
  workflow parses the fallback VMT. Provider-specific fallback validation and
  runtime selection still need the material-system integration.

## Run the workflow

The host needs a C++20 compiler, `pkg-config`, libpng development files, and
Python Pillow. The script compiles its small host exporter under
`build/portal-pbr/`. It uses the repository's VPK, VMT and VTF code; it does
not need a second VPK or VTF decoder. Supply archives from a legally obtained
Portal 1 installation. Put output outside the source tree:

```sh
python3 tools/portal_pbr/workflow.py \
  --vpk /path/to/portal/portal_pak_dir.vpk \
  --asset-vpk /path/to/hl2/hl2_textures_dir.vpk \
  --out /path/to/portal-pbr-output
```

`--asset-vpk` is repeatable and supplies shared content after the Portal VPK.
Use `--only 'models/props/*'` or `--limit 10` for a review batch. To import
staged PNGs into the running ArmorPaint MCP bridge and save a new project:

```sh
python3 tools/portal_pbr/workflow.py \
  --vpk /path/to/portal/portal_pak_dir.vpk \
  --asset-vpk /path/to/hl2/hl2_textures_dir.vpk \
  --out /path/to/portal-pbr-output \
  --import-armorpaint --new-armorpaint-project \
  --armorpaint-project /path/to/portal-pbr-output/portal-pbr.arm
```

`--new-armorpaint-project` discards the live project; save work in ArmorPaint
first. Omit it to add assets to the current project. The MCP import replies
confirm that the bridge accepted paths. The script verifies the `.arm` write,
reopens it, and checks its asset count when it created a fresh project. The
bridge's live asset list can be null before the first save/reopen, and its name
listing is capped at 64 entries; the count after reopening is the useful
bulk-import check.

## Output and readiness

The converter writes `textures/` PNGs, `materials/portal_pbr/` candidate VMTs,
`materials/portal_pbr_fallback/` copies of source VMTs,
`legacy-materials/` original VMTs, and a machine-readable manifest with each
material's decisions and review reasons. The MCP import step adds selected PNG
assets to ArmorPaint; it cannot create a full engine shader through ArmorPaint.
Engine readiness requires the versioned VMT schema and real material binding,
the runtime texture format and color-space path, and a tested compatibility
fallback. Until then the candidate VMTs are reviewable definitions, not
installable replacements for Portal materials.

Local full-archive run on 2026-09-22: 748 Portal VMTs inventoried, 373
`LightmappedGeneric` or `VertexLitGeneric` PBR candidates staged and parsed
against the versioned required-field schema. The others are recorded as
skipped or needing review. The output occupies about 254 MB. ArmorPaint
accepted 820 PNGs, wrote the `.arm` project, reopened it, and reported 820
texture assets. These checks do not certify runtime rendering or fidelity to
every legacy material effect.

## High-resolution remaster

`remaster.py` takes the staged output above and writes a separate high-resolution
set. Its color pass uses the `realesrgan-x4plus` model from the official
[Real-ESRGAN ncnn Vulkan release](https://github.com/xinntao/Real-ESRGAN/releases/tag/v0.2.5.0).
The tested Ubuntu archive is `realesrgan-ncnn-vulkan-20220424-ubuntu.zip`, with
SHA-256 `e5aa6eb131234b87c0c51f82b89390f5e3e642b7b70f2b9bbe95b6a285a40c96`.
Install that archive in a local tool directory and pass its executable and
`models/` path explicitly. The host also needs a working Vulkan device and the
dependencies listed above. No game assets or model weights are committed to
this repository.

```sh
python3 tools/portal_pbr/remaster.py \
  --source /path/to/portal-pbr-output \
  --out /path/to/portal-pbr-remastered \
  --upscaler /path/to/realesrgan-ncnn-vulkan \
  --model-dir /path/to/models \
  --import-armorpaint --new-armorpaint-project \
  --armorpaint-project /path/to/portal-pbr-remastered/portal-pbr-remastered.arm
```

The default is a faithful 4× enlargement capped at 4096 pixels on the longest
edge; sources larger than 1024 pixels on one edge use 2×. Distinct source
images are sent to the color model only once. `remaster_channels.cpp` restores
authored alpha from the source, normalizes interpolated tangent normals, and
resizes MRAO as linear channel data. It does not invent roughness, metalness,
AO, or normals where the original staging run had only placeholders. Each
output set gets the candidate and original fallback VMT. The new manifest
records source and output hashes, dimensions, scale, model hashes, review notes,
and per-set QA. The script fails if an image is missing, has the wrong size, or
fails its alpha check. `--limit 2` provides a small pilot run.

These are editable high-resolution assets. A texture artist should inspect UV
islands, labels, tiling seams, alpha edges, and legacy material effects before
calling a particular material remastered. As with the lower-resolution output,
the VMTs are candidates until the native PBR material runtime and texture path
are implemented and validated.

## ArmorPaint PBR remaster (recipe pipeline)

`armorpaint_remaster.py` turns one Portal material into a high-resolution PBR set
authored in ArmorPaint through the armorpaint-mcp server. It goes beyond the
colour-only remaster above: it authors metalness, roughness, occlusion and normal
from legacy evidence. Each material has a recipe in `recipes/<material>.json`
that records **decisions only**. The facts come from the VPK: the VMT, its DirectX 9
fallback block (`portal_assets material` resolves `_HDR_DX9`/`_DX9`, where
Portal keeps `$bumpmap`/`$ssbump`), and the decoded VTF layers.

| Stage | What happens |
| --- | --- |
| resolve | Base, bump, SSBump flag, spec-mask source (`$normalmapalphaenvmapmask` → bump alpha, `$basealphaenvmapmask` → base alpha), Phong exponent map, `$selfillum`. The VMT governs where it disagrees with a VTF flag; unsupported effects stop the material. |
| prep | Real-ESRGAN colour, wrap-padded for tiling textures and edge-padded for model atlases. For tiling textures, a tileable Poisson height is integrated from the Source normal/SSBump (DirectX +Y; the integrability test must confirm it) and upscaled in float. Masks: R = cavity, G = roughness evidence, B = height. Then log-space de-lighting of colour baked from the normal, and a periodic noise image (ArmorPaint's `TEX_NOISE` is not periodic over UV 0–1). |
| author | One MCP session per material: new project, plane mesh, four imports, a named node graph (base/F0 mix, cavity metal mask, spec or exponent roughness with seam blend and noise, occlusion, DirectX-normal import), fill, texture export and `.arm` save. The full call transcript is kept. |
| package | `basecolor.png` (sRGB), `mrao.png` (R metal, G perceptual roughness, B AO), `normal.png` (OpenGL +Y) and `emission.png` (base × self-illum mask). A `PBRMetalRough` candidate VMT is validated against `pbr_material_schema.h`, with the original VMT as its fallback. VTFs are optional (`--vtex`). |
| qa | Size, normal tilt and convention (integrability), export-vs-input normal, roughness floor and variation, the wrap seam against the largest interior step and the source's own edge (tiling only), emission coverage, and advisory PBR ranges. |

Run it from the repository root, with output outside the tree:

```sh
python3 tools/portal_pbr/armorpaint_remaster.py $(find tools/portal_pbr/recipes -name '*.json') \
  --vpk /path/to/portal/portal_pak_dir.vpk --vpk /path/to/hl2/hl2_textures_dir.vpk \
  --out /path/to/portal-pbr-armorpaint \
  --upscaler build/portal-pbr/upscaler/unpacked/realesrgan-ncnn-vulkan \
  --model-dir build/portal-pbr/upscaler/unpacked/models \
  --vtex build/toolchains/pbrt-map-tools/vtex --launch-isolated
```

`--launch-isolated` starts ArmorPaint in a private headless sway/Xwayland
session. ArmorPaint reads its layer resolution only at startup, so the run
writes `layer_res` for the recipes' size, enables the bridge plugin, and restores
`config.json` afterwards. `--discard-armorpaint-project` drives an already running
ArmorPaint instead; each material starts a new project there, which discards the
open one. `--stage inspect` writes each material's source layers and resolved
facts; write a new recipe from those. The unit oracles, including negative
fixtures for DirectX exports, tilted normals, flat roughness, seams and black
emission, run with
`python3 -m unittest discover -s tools/portal_pbr/tests -p 'test_armorpaint*'`.

Recipe fields: `tiling` (world texture vs model atlas), `cavity.source`
(`base_alpha`, `height` with a range, or `none`), `delight` (strength and blur),
`metal` (`constant` or `cavity_range`), `f0` (value gain and saturation for metal
base colour), `roughness` (`constant`, `from_spec` [spec 0 → a, spec 1 → b] or
`from_exponent`, plus `seam_range`, `seam`, `noise_scale` in cycles and
`noise_amp`), `occlusion` (cavity range to AO range) and optional `emission`
(`selfillum` with a scale). `from_exponent` uses the engine's decode,
n = 1 + 149·R (`skin_ps20b.fxc`). Source's Phong is about Blinn-Phong 4n, so
alpha = √(2/(4n+2)), and the stored perceptual roughness is √alpha because the
native shader squares it.

Limits: ArmorPaint's `BUMP` node tilts a connected normal (measured mean XY
0.23), so micro-relief is not authored in the graph. De-lighting removes the
component that is linear in the normal; baked specular sheen can remain. Metal
F0 below bare steel is reported as an art-directed dark finish. The output is
a reviewed candidate: in-engine rendering is subject to the same
`PBRMetalRough` runtime caveats as above.
