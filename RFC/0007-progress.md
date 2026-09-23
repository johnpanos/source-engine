# RFC 0007 progress

Updated: 2026-09-22. The full physically based lighting pipeline is not yet
implemented. This record separates installed evidence from the RFC's planned
interfaces and phases.

## R47: PBR material family core

The first headless BRDF slice is installed in
[`public/render/pbr_brdf.h`](../public/render/pbr_brdf.h). It defines GGX
distribution, height-correlated Smith visibility, Schlick Fresnel, and a
linear RGB specular BRDF. The independent
[`render.pbr-brdf`](../unittests/rendertest/contracts/render.pbr-brdf.v1.md)
conformance suite checks analytic values, normalized GGX distribution, and
white-furnace energy. The suite is in the shared RFC 0005 manifest and runs in
release mode as well as the default configuration.

Local evidence (2026-09-22):

```sh
python3 tools/quality/conformance.py check --rfc 0007 --config default
python3 tools/quality/conformance.py check --rfc 0007 --config release
python3 tools/stylelint/stylelint.py public/render/pbr_brdf.h unittests/rendertest/test_pbr_brdf.cpp --diff
```

All three RFC 0007 conformance suites passed in both configurations: the
positive suite passed 24 checks and the two sensitivity suites rejected their
seeded defects. Scoped
stylelint passed. The full architecture/baseline/inventory commands are not
green in the shared working tree: they report drift in modified window,
shader-device, and physics files and uninstrumented loader sites, none of
which is part of this BRDF slice. No architecture acceptance is claimed from
those runs.

The second R47 slice adds a native Vulkan direct-specular shader at
[`pbr_direct.frag`](../materialsystem/shaderapivulkan/shaders/pbr_direct.frag).
It samples an sRGB base color texture through the Vulkan sRGB view and a linear
metalness/roughness/AO texture;
the test supplies light/view angles and radiance directly. The native
[`pbr_native_pixel_conformance`](../unittests/shaderapivulkantest/test_pbr_pixels.cpp)
target renders three synthetic cases, captures actual pixels, compares them
with the headless BRDF, and rejects wrong roughness mapping and missing grazing
Fresnel. The shader is an optional native capability: failed pipeline creation
is reported and cleaned up without failing initialization of existing material
pipelines.

Native evidence (2026-09-22, Linux Wayland, AMD Radeon 8060S / RADV):

```sh
python3 waf configure -o build/pbr-native -T release --platform-provider=sdl3 --render-backend=native-vulkan --product-profile=quality/product_profiles/portal-linux-wayland-native-vulkan.json --build-games=portal
python3 waf build -o build/pbr-native --targets=pbr_native_pixel_conformance -j4
./build/pbr-native/unittests/shaderapivulkantest/pbr_native_pixel_conformance
python3 tools/quality/conformance.py check --rfc 0007 --config release
```

The native target built with host `glslc` shaderc v2026.1 and passed 18 checks.
Captured 8-bit RGB values were
`(128,128,128)`, `(162,41,21)`, and `(72,72,72)` against headless predictions
of `(127.9,127.9,127.9)`, `(162.3,40.6,20.7)`, and `(72.0,72.0,72.0)`.
The three release conformance suites matched their expected results. Vulkan
validation was requested but its layer was unavailable, so validation remains
unverified. The native test returns 77 when a display or Vulkan device is
unavailable; such a run does not certify a pixel gate. The isolated Waf output
avoided changing the shared checkout's existing build configuration.

R47 remains active. This direct-specular fixture is not yet a complete
`PBRMetalRough` material family. Diffuse lighting, baked SH/RNM and probes,
image-based lighting, Cycles reference fixtures, the full `pbr` pixel family,
material-system/baker/exporter adoption of the VMT schema, and validated
D3D9/DXVK runtime fallback are still required.
The native shader compiler is taken from the host path for this slice; the
shader artifact and compiler revision must be pinned in a product profile
before claiming reproducibility across machines.

A subsequent native pixel check (2026-09-22, same Linux Wayland/RADV profile)
exercises the declared texture encodings. The CPU oracle independently decodes
the sRGB base texel; the GPU uses the sRGB image view. MRAO still uses its linear
view. Four cases passed 25 checks: the colored-metal pixel was `(123,7,2)`
against `(122.5,6.7,2.1)`, and the half-metal pixel was `(106,4,4)` against
`(105.9,4.0,4.0)`. Captured pixels also rejected the seeded wrong choices of
linear base sampling and sRGB decoding of metalness. The command is the native
`pbr_native_pixel_conformance` invocation above; Vulkan validation remained
unavailable. This adds a texture-encoding oracle to the synthetic specular
path, not runtime `PBRMetalRough` binding or the full PBR pixel gate.

The third R47 slice introduces the version 1
[`PBRMetalRough` parameter schema](../public/render/pbr_material_schema.h).
It owns the VMT names, required fields, scalar defaults, and texture color
encodings without changing existing `PBR` content. The
[`MaterialCatalog`](../public/hammer/formats/material_catalog.h) now validates
PBR definitions, follows primary and fallback VMT patch chains with path,
cycle, and depth limits, and asks an injected compatibility shader catalog
whether the resolved fallback shader is supported. The shared
[`render.pbr-material-schema`](../unittests/rendertest/contracts/render.pbr-material-schema.v1.md)
suite checks valid patches and a provider as well as missing fields, malformed
references, self/cyclic fallbacks, traversal, and a rejecting provider.

Local evidence (2026-09-22): the schema header compiled under C++11 for future
legacy material-system use. `python3 tools/quality/conformance.py check --rfc
0007 --config default` and the matching `release` command each passed all
four selected suites; the schema suite passed 25 checks in both configurations.
The added case rejects a patch chain beyond the legacy material loader's
ten-level bound.
The two existing Hammer material-catalog suites also passed in release (13
positive and 8 sensitivity checks). Changed-line style checking reports no
findings in this slice. The schema is not yet consumed by the runtime material
system, baker, or RFC 0008 world exporter. The editor result proves only
reference validity against the caller-supplied shader catalog; it is not
runtime fallback evidence.

Source review found an existing D3D9 `PBR` shader
(`materialsystem/stdshaders/pbr_dx9.cpp`, listed in its Waf source list) with different
parameters and no declared fallback. The new family is named `PBRMetalRough`
to preserve that shader and existing content. `CMaterial::InitializeShader`
uses a shader's `GetFallbackShader` before honoring `$fallbackmaterial`; an
unknown shader becomes the missing-shader material.

The fourth R47 slice registers `PBRMetalRough` as a fallback shader in the
standard shader provider. Its `GetFallbackShader` selects `Wireframe_DX9`;
the existing `CMaterial::InitializeShader` loop reads `$fallbackmaterial`
after that selection and loads the referenced VMT. This allows compatibility
materials to reach the runtime fallback path while the full native material
family is being built. The registration is deliberately temporary: replace
it when a `PBRMetalRough` shader implements the declared capability and keeps
the compatibility selection within the same validated material path.

Local evidence (2026-09-22): native Vulkan and DXVK Waf profile builds of
`builtin_shader_conformance` passed (`CONFORMANCE 302 0` for each). The test
enumerates both the existing `PBR` shader and the new registration, and
checks its exact fallback name. The DXVK build used
`WAFLOCK=.lock-waf-r03-portal-dxvk` and the native build used
`WAFLOCK=.lock-waf_linux_build`; both targeted
`builtin_shader_conformance`. Source review confirms the material loader's
subsequent `$fallbackmaterial` branch.

The fifth R47 slice adds a staged real-material pixel case to the shared
`material_pixel_conformance` harness. The primary `PBRMetalRough` VMT names
red primary data and a separate `UnlitGeneric` fallback VMT that names a green
procedural texture. The harness calls `FindMaterial`, records the resolved
shader, draws through the material system and DXVK, and samples the center and
an undrawn pixel. The oracle requires `UnlitGeneric`, no error material, green
at the center, and the unchanged magenta clear color outside. Seeded oracle
tests reject wrong shader selection, wrong pixels, and an incomplete capture.

Native evidence (2026-09-22, Linux headless surface, AMD Radeon 8060S / RADV):

```sh
WAFLOCK=.lock-waf-r03-portal-dxvk python3 waf build --targets=material_pixel_conformance -j4
python3 tools/quality/material_pixel_conformance.py run \
  --runtime run/runtime-dxvk --build build-r03-portal-dxvk \
  --renderer vulkan-compat --hdr none --family pbr-fallback \
  --out /tmp/rfc0007-pbr-fallback-dxvk-20260923a
WAFLOCK=.lock-waf_linux_build python3 waf build --targets=material_pixel_conformance -j4
WAFLOCK=.lock-waf_linux_build python3 waf build --targets=hl2_launcher -j4
python3 tools/quality/material_pixel_conformance.py run \
  --runtime run/runtime-native --build build/pbr-native \
  --renderer native-vulkan --hdr none --family pbr-fallback \
  --out /tmp/rfc0007-pbr-fallback-native-20260923b
python3 -m unittest tools.quality.tests.test_material_pixel_conformance -v
```

Both backend captures passed with shader `UnlitGeneric`, center `(0,255,0)`,
outside `(255,0,255)`, and a matching clear probe. The 69 Python oracle tests
passed. This verifies the positive runtime fallback path on DXVK and native
Vulkan; it does not certify invalid/missing-fallback rejection, D3D9 without
DXVK, or native PBR shading. The runtime material system does not yet consume
the shared PBR schema, so the editor validator is the only installed
content-validation caller.

The sixth R47 slice makes the runtime VMT loader consume the same required-field
schema and fallback-reference syntax as the editor. Before returning a loaded
PBR VMT, `LoadVMTFile` rejects a missing `$basetexture`, `$mraotexture`, or
`$fallbackmaterial`, an unsafe path, a self reference, and an absent fallback
VMT. It inspects the fallback through the existing patch resolver, rejects a
remaining patch root or another PBR shader, and checks the selected shader
catalog when graphics is active. `FindMaterial` then returns the engine error
material instead of a PBR material that silently drops to wireframe. The
material pixel harness stages nine invalid PBR VMTs alongside the valid case
and requires all nine to be rejected. The positive fallback pixel remains part
of the same run.

Local evidence (2026-09-22): both DXVK and native Vulkan profile builds of
`material_pixel_conformance` succeeded. Staged `pbr-fallback` runs passed on
both profiles, including shader `UnlitGeneric`, green center pixel, clear
outside pixel, and all nine invalid-material rejection flags. The DXVK
capture is at `/tmp/rfc0007-pbr-validation-dxvk-20260923d` and the native
capture at `/tmp/rfc0007-pbr-validation-native-20260923c`. The shared RFC 0007
release conformance check passed four suites, including 26 schema checks; the
73 material-pixel Python oracle tests passed, including four focused PBR
fallback tests. The new schema header and test pass scoped style checking.
The global changed-line style check is currently stopped by unrelated Portal
client formatting and invalid UTF-8 in concurrently added Portal 2 source.
The full material-pixel Python test module currently also exercises a
concurrently added `sky` family whose reference file is absent, so that
module's complete run is not green.

This runtime preflight uses the legacy patch resolver, which can still follow
an unsafe nested `include` before the final root is checked. It has not proved
all malformed-VMT syntax fails, and no installed package-level content gate
runs the editor catalog against every shipped PBR material. Those cases and
the full native material path remain before fallback and R47 acceptance.

R48–R52 have no implementation evidence yet. In particular, vbsp/vvis/vrad
are not ported to Waf, the Cycles provider is not built or pinned, and RFC
0008's World Stage and canonical lighting formats are prerequisite work for
the later bake phases.

## Portal 1 texture staging (supporting R47)

The [Portal PBR workflow](../tools/portal_pbr/README.md) uses the existing
Hammer VPK/VMT/VTF codecs to inventory Portal 1 materials, stage editable PNG
base/normal/MRAO sets, and write `PBRMetalRough` candidate VMTs with separate
legacy fallback VMTs. Its exporter reads the versioned PBR parameter names
from `pbr_material_schema.h` and validates each generated definition. Generated
game content stays outside the repository.

Local run (2026-09-22): 748 Portal VMTs inventoried, 373 lit-material
candidates staged and schema-valid, 820 PNG assets accepted through the
ArmorPaint MCP bridge, and the `.arm` project reopened with 820 texture assets
reported. The manifest records 19 referenced VTFs unavailable or unsuitable
for a static 2D decode,
plus material-specific review reasons. Metalness 0, roughness 0.8, and AO 1
are documented placeholders where no authored PBR data exists. The converter
does not establish runtime `PBRMetalRough` support, KTX2 loading, correct
sRGB sampling, legacy fallback selection, or fidelity for the skipped shaders
and effects; R47 and the downstream content gates remain open.
