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
It consumes a linear base color texture and a metalness/roughness/AO texture;
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
versioned VMT schema, and validated D3D9/DXVK fallback are still required.
The native shader compiler is taken from the host path for this slice; the
shader artifact and compiler revision must be pinned in a product profile
before claiming reproducibility across machines.

Source review found an existing D3D9 `PBR` shader
(`materialsystem/stdshaders/pbr_dx9.cpp`, listed in its Waf source list) with different
parameters and no declared fallback. The new family is named `PBRMetalRough`
to preserve that shader and existing content. `CMaterial::InitializeShader`
uses a shader's `GetFallbackShader` before honoring `$fallbackmaterial`; an
unknown shader becomes the missing-shader material. The new fallback needs a
validated material-system path on compatibility providers, rather than merely
placing `$fallbackmaterial` in a VMT.

R48–R52 have no implementation evidence yet. In particular, vbsp/vvis/vrad
are not ported to Waf, the Cycles provider is not built or pinned, and RFC
0008's World Stage and canonical lighting formats are prerequisite work for
the later bake phases.
