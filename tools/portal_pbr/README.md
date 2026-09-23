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
