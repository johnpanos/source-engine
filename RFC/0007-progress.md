# RFC 0007 progress

Updated: 2026-09-24. The full physically based lighting pipeline is not yet
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

The next R47 shading slice generates one 32×32 split-sum table with 4096
deterministic GGX importance samples per texel. The generator emits the C++
coefficients, which the Vulkan provider uploads as a linearly filtered float
texture; shader SPIR-V regeneration checks that the table matches the generator.
`EvaluateLayeredDirect` and the native fragment
shader now add Lambertian diffuse for the nonmetal fraction, weighted by
`1 − E_spec(N·V)` from that table. The MRAO AO channel remains reserved for
indirect light. This is a real BRDF term in the native draw path, while the
lighting inputs are still supplied by the synthetic test.

Local evidence (2026-09-22):

```sh
python3 materialsystem/shaderapivulkan/shaders/gen_pbr_split_sum.py --check
python3 tools/quality/conformance.py check --rfc 0007 --config release
python3 waf build -o build/pbr-native --targets=pbr_native_pixel_conformance -j4
python3 waf build -o build/pbr-native --targets=shaderapivulkan -j4
./build/pbr-native/unittests/shaderapivulkantest/pbr_native_pixel_conformance
```

All five release conformance suites passed. The positive BRDF suite passed 37
checks, including split-sum coefficients against an independent hemisphere
integral and layered white-furnace energy; the swapped-table negative control
failed its intended oracle. The native Vulkan suite passed 31 checks across
five material cases. Captured RGB matched headless predictions: white
dielectric `(73,73,73)` versus `(72.8,72.8,72.8)`, half-rough metal
`(160,160,160)` versus `(159.8,159.8,159.8)`, colored metal `(123,7,2)`
versus `(122.5,6.7,2.1)`, half metal `(88,1,1)` versus `(88.3,0.8,0.8)`,
and grazing dielectric `(91,91,91)` versus `(91.1,91.1,91.1)`. The native
suite rejects omitted diffuse as well as its earlier roughness, Fresnel, and
encoding defects. Vulkan validation remains unavailable on this runner. The
table now occupies a 32×32 RGBA32F texture rather than shader code. Its
interpolation error and startup cost still need evaluation against Cycles
references and target device profiles before this becomes the production IBL
representation.

After moving the table to a GPU texture, the native pixel suite again passed
all 31 checks with the same captured RGB values. The generated SPIR-V header
shrank from 708,813 to 212,923 bytes, and the incremental native build finished
in 10.5 seconds on this runner. The `shaderapivulkan` shared library also built
in the isolated profile. A fresh clean-build comparison and native validation-layer
run remain open. Architecture ownership now lists the generated table in the
render contract module; the strict checker no longer reports that include.
Repository-wide architecture checks still report 49 new and three stale legacy
baseline occurrences, plus ten uninstrumented loader sites elsewhere in the
shared tree. Changed-line style checking has no finding in this slice, but
reports concurrently edited files outside it.

The next R47 boundary is material-system selection of this shader for a real
`PBRMetalRough` VMT. That path must provide scene-derived view/light inputs,
bind the two authored textures, negotiate native capability, and preserve the
validated legacy fallback. The synthetic draw fixture does not supply those
runtime obligations.

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
material pixel harness stages ten invalid PBR VMTs alongside the valid case
and requires all ten to be rejected. The positive fallback pixel remains part
of the same run.

Local evidence (2026-09-22): both DXVK and native Vulkan profile builds of
`material_pixel_conformance` succeeded. Staged `pbr-fallback` runs passed on
both profiles, including shader `UnlitGeneric`, green center pixel, clear
outside pixel, and the original nine invalid-material rejection flags. The DXVK
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

The fallback patch resolver now checks each `include` path before opening it.
The tenth negative fixture points through `..` to an existing legacy VMT; the
loader rejects that path while the valid fallback still renders green. Both
the native Vulkan and DXVK staged runs passed with all ten rejection flags;
evidence is in `/tmp/rfc0007-pbr-nested-native-20260923a` and
`/tmp/rfc0007-pbr-nested-dxvk-20260923a`. The focused five Python fallback
oracle tests and the four RFC 0007 release conformance suites passed. This
protects the fallback patch chain.

A subsequent patch-resolution slice propagates primary patch failures to the
material loader, rejects a still-patched root after the recursion limit, and
records unsafe primary include paths during resolution. If the resulting root
is `PBRMetalRough`, an unsafe path rejects that material. A valid primary
patch remains accepted. The product harness now checks a valid primary patch,
an unsafe primary patch include, and a missing primary include. Native Vulkan
and DXVK staged runs passed with all 12 invalid-material flags. Both the
direct PBR VMT and the valid primary patch rendered green `UnlitGeneric`
pixels, with magenta clear color outside the draw (evidence:
`/tmp/rfc0007-primary-patch-native-20260923c` and
`/tmp/rfc0007-primary-patch-dxvk-20260923c`). The patch material is held by
the harness and cached before its pixel draw. The full 80-test material-pixel
Python oracle module passed, including seeded PBR patch defects. This check
rejects a PBR primary
patch after resolving its include chain; it does not prevent opening an unsafe
primary include whose eventual shader was unknown. Malformed VMT syntax is not
fully covered, and no installed package-level content gate runs the editor
catalog against every shipped PBR material. Those cases and the full native
material path remain before fallback and R47 acceptance.

The shared RFC 0007 release conformance run matched all five suites (37
positive BRDF checks, 26 schema checks, and three negative BRDF suites);
evidence is `quality-results/conformance.20260923T063624Z.json`. The global
architecture checker still reports the unrelated baseline drift above.

## R49 preparation: four-sample SH L1 feasibility

[`pbr_sh_l1.h`](../public/render/pbr_sh_l1.h) now fits each scalar irradiance
channel to `E(n) = constant + x*n.x + y*n.y + z*n.z` from four caller-supplied
directions. It has no copy of the production RNM basis or map layout; the
headless fixture uses the flat normal and the three values from
`mathlib/bumpvects.h`. The fit rejects singular and nonfinite inputs without
publishing a partial result. The shared
[`render.pbr-sh-l1`](../unittests/rendertest/contracts/render.pbr-sh-l1.v1.md)
suite checks affine recovery, uniform irradiance, failure behavior, and a
swapped-basis negative control.

Local evidence (2026-09-22): both default and release RFC 0007 conformance
runs passed all seven selected suites. The new positive suite passed 11 checks;
the negative suite detected its seeded basis swap. Evidence is
`quality-results/conformance.20260923T064643Z.json` (default) and
`quality-results/conformance.20260923T064737Z.json` (release). Scoped style
checking passed for the new header and test. Full archlint still reports the
same 49 new and three stale occurrences in the shared tree; no occurrence is
caused by this header after adding its owner to the module manifest.

The analytic grazing-light fixture gives a decisive limitation of the proposed
four-sample quality fit. A unit light along tangent X yields zero irradiance at
the perpendicular horizon normal, but the fitted L1 function evaluates to
`0.643950`. The suite records this as a measured approximation error, not a
passing quality tolerance. R49 must set a per-map directional-error budget and
test extra sampling or a least-squares fit against analytic lights and Cycles
before writing canonical SH L1 lighting. An L1 representation may still need a
declared quality limit for sharp or grazing light; the fitting function alone
does not satisfy the SH oracle in RFC 0007.

Local World Stage inspection evidence (2026-09-23): the Portal-remaster fixture
was baked in Blender Cycles at the flat and three exact RNM directions plus
eight versioned supplemental directions. The 12-direction least-squares SH L1
fit improves an independent held-out Cycles direction's chart-pixel mean
absolute error from 0.257 to 0.207 (19.5%) and the analytic direct-light
worst error from 0.845 to 0.500. Both the four- and 12-direction fits fail the
provisional 0.15 maximum analytic bound. Dense L1 least squares still has a
0.385 worst error on that analytic grid, exposing representational loss beyond
sample-count error. The exact RNM samples separately change pixels in the
playable DXVK map. [RFC 0008 progress](0008-progress.md#f2-supplemental-cycles-sh-fit-2026-09-23)
links the bake, independent BSP-direction checks, negative controls and
in-game evidence. The Python fit is an unencoded inspection artifact; the
R49 baker contract, C++ implementation, accepted quality tolerance, probes
and native SH runtime remain open.

A later RFC 0008 playable preview carries the same verified Cycles flat and
RNM samples in BSP2 beside the USD-derived WMSH. With identical WMSH,
upsampled Portal textures, client binaries and camera, changing only legacy
lighting from VRAD to Cycles changes 99.68% of pixels in the native Vulkan
WMSH view. A lightmap-UV-only negative map also changes the frame. The
[RFC 0008 integration receipt](0008-progress.md#f4-world-stage-geometry-and-cycles-light-in-the-playable-wmsh-view-2026-09-23)
establishes the compatibility bridge, not canonical SH/RNM lightmap assets or
the R49 baker gate.

The full R48–R52 gates remain open. VBSP has a Linux host-tool port and a
synthetic smoke; vvis/vrad and the legacy-output comparator remain. The Cycles
provider is not built or pinned, and RFC 0008's World Stage and canonical
lighting formats are prerequisite work for the later bake phases.

### R48 host compiler preparation (2026-09-23)

The FGD parser library used by vbsp now has an isolated Waf static target in
the host `--tools` product. `./waf configure -o
build-rfc0008-tools-fgdlib --tools -T release` and `./waf build -o
build-rfc0008-tools-fgdlib --targets=fgdlib -j4` built `libfgdlib.a` on
Linux x86_64. The port fixes case-sensitive includes, dependent-base template
lookup, integer formatting and a Windows-only file-existence check; the
token reader remains the authority for opening the file. This is a compile
and link check, not an FGD semantic corpus.

Using the Waf `vtex.cpp` compile flags with vbsp's include paths and
`MACRO_MATHLIB`, syntax checks pass for all 25 `utils/vbsp/*.cpp` files and
47 of 49 sources listed in `utils/vbsp/vbsp.vpc`. The two remaining source
failures are `utils/common/threads.cpp` and
`utils/common/tools_minidump.cpp`, both Windows implementations. No native
vbsp executable or byte-identical legacy-lump comparison is claimed. The
vvis/vrad tool ports and baker conformance remain outstanding.

An isolated Waf VBSP attempt compiles all 49 selected VBSP/common translation
units after adding Linux host-tool implementations of the legacy thread and
crash-handler entry points. The Linux thread implementation passes a standalone
4-worker, 4096-item exactly-once smoke test. A `pthread_create` failure
injected on the second worker reaches the error path with zero callbacks
started. At that point, linking failed because this tree had
declarations and callers for `EmitPhysCollision`,
`EmitWaterVolumesForBSP`, and `DumpCollideToGlView`, but no implementation of
those functions. Other unresolved host-tool dependencies included POSIX cmdlib
helpers, filesystem find wrappers, LZMA, and static library ordering. The Waf
VBSP target was kept out of the default `--tools` product at that point.

The next compiler slice restores Valve's `ivp.cpp` and `disp_ivp.cpp` plus
their headers from the pinned official Source SDK 2013 revision documented in
[`utils/vbsp/UPSTREAM.md`](../utils/vbsp/UPSTREAM.md). Only local include paths
and mechanical formatting changed. Both files pass the isolated Linux VBSP
syntax check. A Waf build with VBSP temporarily registered compiles all 210
tasks and reaches the link: the three previously missing collision symbols
are resolved. The remaining unresolved symbols are POSIX cmdlib/file search,
`ScratchPad3D_Create`, LZMA compression, shader-provider binding, and
`ImageLoader` symbols caused by archive order. This was a compiler inventory,
not Portal 2 semantic acceptance.

The installed Linux host-tool composition now builds VBSP, filesystem_stdio,
the null shader backend, standard material shader definitions, VPhysics, and
their dependencies through Waf. The port adds POSIX cmdlib output/cleanup,
links the existing filesystem search and LZMA sources, and repeats libbitmap
after libvtf to resolve its archive dependency. Its material-system bridge
supplies and owns the CVar registry alongside the filesystem factory. Optional
scratchpad debug support is excluded from this Linux tool profile; other host
tool products do not select the VBSP cohort. VBSP
requires a loaded surface-property manifest with a `default` property and
rejects missing data before map output.

Local evidence (2026-09-23, isolated release tools build):

```sh
./waf configure -o build-rfc0008-tools-vbsp --tools -T release --prefix=/tmp/rfc0008-vbsp-install
./waf build -o build-rfc0008-tools-vbsp -j4
./waf install -o build-rfc0008-tools-vbsp --targets=vbsp,bsp2tool,filesystem_stdio,vphysics -j4
python3 tools/quality/vbsp_host_smoke.py --vbsp /tmp/rfc0008-vbsp-install/vbsp --bsp2tool /tmp/rfc0008-vbsp-install/bsp2tool --out quality-results/rfc0008-vbsp-host-20260923/evidence.json
```

The full `--tools` build and installed-product smoke pass. The synthetic
Hammer room compiles to VBSP v21 with a 2,929-byte physics lump; the independent
BSP2 reader accepts its converted form, and export is byte-identical to the
compiled BSP. Missing surface-property manifest and missing `default` both
exit 1 before writing a BSP. The smoke records executable and output hashes
in ignored local evidence. Its fixture lacks texture pixels and a skybox, so
material warnings remain. R48 is partial: legacy executable
comparisons, baker conformance, and native content/profile coverage are still
required.

The Linux `--tools` product now also builds and installs local, non-MPI VVIS.
`tools/quality/vvis_host_smoke.py` compiles a sealed four-cluster room through
VBSP and VVIS, checks all four cluster PVS entries with an independent BSP
reader, compares one- and two-thread visibility bytes, rejects a missing portal
file and `-mpi`, and verifies byte-exact BSP2 convert/export. The pinned local
evidence is `quality-results/rfc0008-vvis-host-20260923/evidence.json`:
44 visibility bytes and SHA-256
`2c8c31960419645a5d58adac1f23fa7db2a7093c1f663dc8459197df7526b7d6`.
Substituting `/usr/bin/true` for VVIS makes the runner fail on the empty
visibility lump; the negative-control result is alongside the evidence.
This synthetic result does not establish legacy-output equivalence or the
serial/parallel job-graph gate.

VRAD's VPC product is an executable linked to `vrad_dll`. A Linux Waf product
now directly links that entry point and the original local solver, with a new
Waf `raytrace` archive and the existing material, physics, and BSP dependencies.
VMPI paths remain in Windows VPC builds; Linux rejects `-mpi` before mutation.
The port adapts case-sensitive includes, Linux temporary-file and executable
path handling, SIMD element access, and static-prop header/pointer use. The
normal direct-light path on the sealed room yields 37,120 LDR lighting bytes,
18,216 nonzero bytes, one world light, ambient data, and unchanged visibility
and physics. One and two worker threads produce byte-identical lighting lumps.
The installed-tool runner `tools/quality/vrad_host_smoke.py` also verifies a
byte-exact BSP2 convert/export after baking. Its ignored evidence is
`quality-results/rfc0008-vrad-host-20260923/evidence.json`; replacing VRAD
with `/usr/bin/true` fails on the empty lighting lump. This synthetic smoke
does not establish legacy executable identity, HDR, static-prop fidelity, or
the shared light-baker contract.

### R48 cross-version executable baseline (2026-09-23)

Locally installed Portal 2 Community Edition compiler executables under
`bin/win64` run in an isolated Wine 11 prefix. Their SHA-256 hashes are
`d2e0f2c09e752448b526b9ecf8501dbfbf6e13e299ba84ef72696f1fec19740b`
(VBSP), `3ed2fbd5e5441da695db58f16fba80af85802fb5ccc2a95130da259decdafa3a`
(VVIS), and `45eaf9ad11c6b0f30e90ed9e0f5c4c53a2034b083d5c1399e134854849ad1b46`
(VRAD). The installed Portal 2 executables also found locally are 32-bit but
cannot start under Wine because their installation lacks `tier0.dll` and
`vstdlib.dll`; no files were copied into the Steam installation.

Community Edition emits BSP v25 while this fork emits v21 from the same
`sealed_room.vmf` hash
`bd381a8c6f5662cf9fb6ec22b837af473063c8c564d7054620ef2fe04537eeeb`.
The independent
`tools/quality/compiler_cross_version_compare.py` reads both results and
requires byte-identical entities (336 bytes), visibility (44 bytes), HDR leaf
ambient samples (112 bytes), and the first 24 bytes of the HDR world light
(origin and intensity). Its local evidence is
`quality-results/rfc0008-compiler-cross-version-20260923/evidence.json`.
Five seeded changes to those promised fields and to HDR lighting all made the
comparator fail. Reference and candidate HDR lightmaps differ in size (74,112
versus 37,120 bytes), as do physics lumps (6,309 versus 2,537 bytes), so this
comparison does not certify the R48 same-revision byte-identity gate. The
Community Edition `-ldr` invocation still wrote HDR lighting on this fixture.

The independent v21 `legacy_lighting_audit.py` now checks every face against
its texinfo, style list, average-color prefix, luxel dimensions, and bumped
basis count. It requires contiguous spans that exactly fill the lighting lump.
The installed-tool smoke passes on the 16-face LDR room with 37,120 bytes and
styles 0 and 32; the v21 HDR cross-version candidate passes the same layout
check. Four unit tests cover a bumped, two-style face and reject wrong offsets,
truncated lighting, missing basis data, and a style after the sentinel. A
seeded offset change in the complete HDR BSP makes the cross-version comparator
fail. Local evidence: `quality-results/rfc0008-vrad-layout-20260923.json` and
`quality-results/rfc0008-compiler-cross-version-20260923/` (ignored). The audit
checks v21 storage structure, not radiometric values or v25 face semantics.
The pinned Portal 2 v21 inventory also passes this audit on all **106/106**
maps: 259,620 lit faces, 183,075 bumped faces, and styles 0 and 32–37. All
106 carry HDR lighting. The corpus runner rejects missing/extra maps and changed
source hashes before checking faces; ignored evidence is
`quality-results/rfc0008-v21-lighting-corpus-20260923.json`.

```sh
python3 tools/quality/legacy_lighting_audit.py \
  --source-root "$PORTAL2_MAP_ROOT" \
  --inventory quality/fixtures/bsp2-corpus-v21.json \
  --out quality-results/rfc0008-v21-lighting-corpus-20260923.json
```

Both the Community Edition VRAD and the Linux port emit all-zero lightmap
samples for this fixture in `-fast -bounce 0` mode; the Linux normal direct
bake remains nonzero. This removes the suspected fast-mode port regression
for this fixture, but does not prove fast-mode parity on the required corpus.

The runner's negative control used `/usr/bin/true` as a false-success converter:
it exited 1 and wrote `status: fail` evidence. The 72 archlint fixture tests
pass, including a new check that private VBSP factory exceptions do not extend
to sibling files. The reviewed manifest exception removes three old
`utilmatlib.cpp` ARCH105 baseline entries. The global architecture check still
reports 49 new and three stale occurrences elsewhere, and the loader inventory
reports 12 uninstrumented native sites in this shared worktree. Changed-line
stylelint reports one failure in the unrelated shader pixel conformance source.

## R48-BAKER: light-baker seam and pipeline consolidation

State: `planned` (recorded 2026-09-24 at the user's direction). Child of R48
(seam, legacy provider, shared suite); its Cycles provider feeds R49. Owner:
unassigned. Nothing below is installed.

### Observed starting point (read-only audit, 2026-09-24)

- There is no baker contract. `public/lighting/` and `utils/lighting/` do not
  exist; `ILightBaker`, `LightBakeScene` and `LightmapSampleLayout` appear only
  in this RFC.
- Every Cycles bake runs as a `blender -b` subprocess, contrary to the
  [Cycles provider](0007-physically-based-lighting-pipeline.md#cycles-provider)
  decision. Blender and OIDN are taken from `PATH` and checked by version string
  only (`quality/product_profiles/pbrt-map-linux-tools.json`,
  `tools/quality/pbrt_map_toolchain.py`).
- Two bake pipelines share no light-transport code: `pbrt_map_build.py`
  (PBRT/USD scene, Blender smart-project UVs, LMAP KTX2) and the
  `worldstage_cycles_*` scripts (VMF, vbsp2 charts, injection into the legacy
  lighting lump). `probe_volume_bake.py` is a third Cycles baker.
- Rules that need a single owner are duplicated:
  - The RNM basis is redefined in `pbrt_lightmap_bake.py`,
    `worldstage_cycles_bake_preview.py`, `worldstage_cycles_basis_oracle.py`,
    `worldstage_directional_bake_compare.py`, `worldstage_sh_l1_preview.py` and
    `bumpvects.h`.
  - Three 4-direction L1 fits exist: `public/render/pbr_sh_l1.h`,
    `worldstage_sh_l1_preview.py` and `lightmap_directional.py`.
  - There are two KTX2 lighting packers.
- The PBRT path's directional term is a luminance gradient
  (`lightmap_directional.py`), neither the canonical per-style SH L1 nor exact RNM.
- Cache keys miss inputs:
  - The `stage` step's inputs list the top-level scene file only
    (`pbrt_map_build.py:324`; `pbrt_scene.py:180` hashes that file). Edited
    PLY meshes or textures do not invalidate it.
  - Tool revisions and the resolved device are not keyed. `device: auto` can
    switch between CPU and GPU without a rebuild.
- A step deletes its previous outputs before running (`pbrt_map_build.py:250-256`),
  so a failed or interrupted bake leaves no usable package.
- Lightmap and reflection-probe bakes set no Cycles seed.
- Verification is incomplete:
  - The basis oracle tests one basis direction.
  - Both SH L1 analytic fits exceed the 0.15 bound (see
    [R49 preparation](#r49-preparation-four-sample-sh-l1-feasibility)).
  - The bake oracles are not in the conformance manifest.

### Device policy (installed 2026-09-24)

[`tools/quality/cycles_device.py`](../tools/quality/cycles_device.py) owns
the Cycles device choice for the existing Blender scripts, pending the
`CyclesBaker`:
- **Bakes and previews use the GPU** and fail if none is found.
- **Correctness checks use the CPU:**
  - the pipeline reference render;
  - `gi_reference.py`;
  - the basis oracle and supplemental bakes;
  - the `gi-fixture` export profile.

Measured on the Ryzen AI Max+ 395 / Radeon 8060S host with Blender 5.2.1:
- two CPU `DIFFUSE` bakes at the same seed were bit-identical;
- two HIP bakes of the same scene differed from each other;
- CPU+GPU hybrid baking took 32.2 s against 26.0 s for HIP alone, repeated
  twice on a 2048² 1024-spp bake. Hybrid is therefore not offered.

The existing G0 references were rendered on HIP. Re-rendering them on the CPU
would change their recorded hashes, and has not been done.

### Lightmap layout, seams and noise (installed 2026-09-25)

Blender no longer decides the lightmap layout: `lightmap.layout: planar` is
the default, and `"blender"` remains only as an explicit comparison option.
Blender runs Cycles on UVs the `layout` step writes into the stage
(`--layout authored`, checked unchanged after the round trip). On the PBRT
scenes:

- staircase2: 212 curved triangles in 42 xatlas charts, stretch 0.67 to 1.48.
  It passed a CPU bake and the `seams` gate: no overlap, bleed or escaped
  UVs, density spread 1.003 on flat charts, no flat seam.
- The bathroom's layout step passes on 590k triangles: 398k curved in 25k
  charts.

- **Topology** (`legacy_bsp.py`, `legacy_bsp_scene.py`, `usd_scene.py`):
  relit faces keep the BSP's shared vertices (merged within 0.01 units) and
  plane numbers (`primvars:sourceEngine:plane`). Faces are triangulated by
  the widest triangulation, which maximizes the thinnest triangle, instead of
  a fan that left micron-wide slivers at vbsp's crack-fix vertices.
- **Layout** ([`lightmap_layout.py`](../tools/quality/lightmap_layout.py)):
  - Flat triangles on one plane that meet along an edge or T-junction are one
    chart, projected onto that plane at one texel density.
  - Curved surfaces are triangles joined along smooth edges (continuous
    authored normals, non-coplanar). xatlas, pinned in the map tools profile
    and built by `pbrt_map_toolchain.py provision --steps sources,xatlas`,
    cuts and flattens them (`xatlas_chart.cpp`). Each chart is scaled to its
    surface area, and a chart stretching beyond 1.5x is charted again
    smaller until every triangle holds.
  - One deterministic packer places every chart with at least 4 texels
    between charts. The same triangles give the same bytes in any order.
  - The step fails on any overlap, bleed, escaped UV, flat-region split,
    off-density flat triangle or over-stretched curved triangle.
- **Seams** ([`lightmap_seams.py`](../tools/quality/lightmap_seams.py)): the
  `seams` step extracts the seams that remain and gates on chart invariants.
  `ktx2` stitches every page across them by least squares.
- **Lights**: a USD sphere, disk or rect light now lights the bake as the
  matching Cycles lamp. The tessellated mesh stays visible to the camera
  only. On the fixture (`tests/blender_emitter_lamps.py`) every lamp matches
  analytic irradiance within 0.35%, and the sphere lamp has 1/120 of the
  mesh's noise at equal samples.
- **Noise** ([`lightmap_noise.py`](../tools/quality/lightmap_noise.py)):
  - Every page is the mean of two half-sample bakes.
  - The total is the sum of the direct and indirect layers rather than a
    third bake.
  - The denoiser runs per chart, padded with the chart's own smoothed light
    so no chart reaches another.
  - The `noise` gate judges the halves' difference after that denoise.
  - testchmb_a_00 at 2048 texels:

    | Samples | Raw median | Denoised median / p99 |
    | --- | --- | --- |
    | 256 | 25% | 1.5% / 28% |
    | 1024 | 12.3% | 1.0% / 17% |

    The residual that remains sits at chart edges. `legacy-relight` bakes
    4096 samples with a 15% p99 target.
- Stalls: a Blender step that prints nothing for 30 minutes is stopped with
  its whole process group, and the step fails (`run_logged`,
  `tests/test_pbrt_map_build_run.py`). On 2026-09-25 at 02:06 the amdgpu
  driver logged "MES failed to respond to msg=REMOVE_QUEUE" and a page
  fault. The testchmb_a_00 relight bake then waited 4.5 hours on a lost HIP
  queue. Afterwards even a 64x64 Cycles HIP render would not finish; CPU
  bakes were unaffected. It needs a GPU reset or reboot. The full relight
  (4096 atlas, 4096 samples, 64-bounce paths) is therefore not yet built.
- Pre-existing, not caused by this work: the bathroom's reference gate fails
  on this host (mean error 10.874 against 10, SSIM 0.917). The 2026-09-24
  corpus build (`quality-results/rfc0011-corpus/bathroom`) recorded the same
  numbers. The cause is not established.

### Regular-compile hook: `vrad_cycles.py` (installed 2026-09-25)

At the user's direction, a regular vbsp/vvis/vrad compile can now feed the
Blender/Cycles pipeline without a manifest. See the
[pipeline guide](../quality/fixtures/pbrt-maps/README.md#hooking-a-regular-compile-into-the-pipeline).

- [`vrad_cycles.py`](../tools/quality/vrad_cycles.py) takes vrad's command
  line, so it can replace vrad as Hammer's `$light_exe` or as a script's light
  step.
  - It runs the real vrad without its own `--cycles-*` options.
  - It then passes vrad's BSP to `legacy_bsp_relight.relight`, the function
    `main` was split into. That is the path shipped maps already use.
  - vrad's BSP is never written. A vrad failure returns vrad's exit status
    and stops before the relight; a relight failure is nonzero.
- Two changes the fixture needed:
  - vrad's `-game` directory (or `-vproject`, `$VPROJECT`) becomes the
    manifest's `legacy_game`. `legacy_bsp_scene.GameDirectory` searches its
    `materials/` before the runtime, ignoring case, and those files are part
    of the `legacy-scene` step key. On the fixture, the relit scene without
    it took both base textures from the runtime's `hl2_textures_dir.vpk`.
    vbsp and vrad compiled against the fixture's untextured VMTs instead.
    With it, the scene uses those VMTs, and the base color is the texdata
    reflectivity vrad used.
  - The profile's `relight_occluder` exclusion made the bake fail on a map
    with no nodraw brush sides ("excluded materials are not in the PBRT
    scene"). `pbrt_map_build.applicable_exclusions` now applies a profile's
    exclusions only to scenes that have them. Exclusions a manifest names are
    still rejected when absent.
- The fixture run also found a hang in `probe_volume_bake.py`, whose forked
  trace workers came in with `a7928390` (2026-09-25):
  - A worker's `lobes @ distances` hung in libgomp's barrier.
    `texels @ rays.T` in the parent starts the OpenMP team of Blender's
    `libopenblaso` before the fork, and a forked child has none of its
    threads.
  - Setting OpenBLAS to one thread in the parent or in a worker initializer
    did not help. A 45 s Blender reproduction hung in all three variants.
  - The workers now use `np.einsum`, which does not call BLAS. In the same
    reproduction, pooled results were bitwise equal to serial and within
    2.4e-15 (relative) of BLAS.
  - Every probe-volume bake with more than one worker and more than one
    probe run was exposed to this hang, relights of shipped maps included.
- A GPU bake hung for 90 minutes in `hipStreamCreate` (HSA signal wait).
  The GPU's compute side had been wedged since 02:06:05, when the kernel
  logged `amdgpu: MES failed to respond to msg=REMOVE_QUEUE` and then a
  gfxhub page fault. Every HIP bake hangs until the GPU is reset or the
  host reboots. The evidence run below therefore used the CPU.
- `--cycles-device` (and `legacy_bsp_relight.py --device`) overrides the
  profile's device for every bake. `cpu` and `auto` stay explicit opt-ins
  under the device policy above.
- Evidence:
  - `tests/test_vrad_cycles.py` has 17 tests against a fake vrad and a
    recording relight. They cover argument splitting, vrad's map and `-game`
    rules, the failure paths, the game-directory layer, the exclusion policy,
    the relight manifest (`legacy_game`, device) and `legacy_game`
    validation.
  - Four of the five mutants were detected. The fifth was equivalent: it
    changed a branch `--cycles-*` options never reach.
  - `tests/test_legacy_relight.py` still passes (33).
  - End-to-end on `quality/fixtures/vbsp-host/sealed_room.vmf`:
    - The toolchain's `vbsp` and `vvis` ran first. Then
      `vrad_cycles.py -game game -bounce 2 --cycles-quality legacy-relight-preview --cycles-device cpu`
      ran on the pinned vrad, exiting 0.
    - `gameplay-identity.json` passed: 59 legacy lumps were identical and
      none differed. The world lights (vrad's 100 bytes; the named light
      became RTRN style 32) and the four leaf-ambient lumps were relit.
      LMAP, PRBV, RTRN, SDFV and WMSH were added.
    - vrad's BSP kept its pre-run SHA-256, and the BSP2 is `173e3581…`.
    - The pipeline steps took 548 s on the CPU, 424 s of them in the
      lightmap bake.
    - `--cycles-boot` booted the relit map headless on native Vulkan
      (`portal_boot` status pass), drawing the WMSH with the map's
      materials.
    - Unverified: a GPU bake (see above), a real Portal map through the
      hook, and Hammer's Run Map dialog.
- This is a hook, not the `ILightBaker` seam: Blender remains a subprocess,
  vrad must still run in full (the relight converts its world lights and
  keeps its lightmaps for brush entities, displacements and translucent
  faces), and Hammer under Wine cannot launch the Linux script. It closes no
  R48 criterion.

### Scope

1. `utils/lighting/` (strict C++20):
   - `LightmapSampleLayout` over stable charts keyed by authored surface ID, not
     BSP face ID, so native USD maps (R59) qualify;
   - a versioned `LightingPolicy`;
   - an immutable `LightBakeScene`;
   - `ILightBaker`, `BakeRequest`, `BakeResult` and `IBakeProgress` in
     `public/lighting/light_baker.h`, using the project `Expected`.
2. A shared suite enforcing the [obligations](0007-physically-based-lighting-pipeline.md#obligations-lsp).
   It runs against a fake and the five deliberately bad providers the RFC names.
3. `LegacyRadiosityBaker`: vrad behind the seam, with its legacy-lump output
   unchanged.
4. `CyclesBaker`: standalone Cycles and OIDN pinned by source revision in a
   declared profile. CPU is `Exact` per profile, GPU `Statistical`. `Final`
   bakes stay undenoised until OIDN determinism is measured.
5. One owner each for the RNM basis, SH L1 fit, KTX2 lighting packing, PRBV and
   reflection-probe encoding. The luminance-gradient format is retired, or kept
   only under a versioned compatibility decision.
6. Complete cache keys (all scene files, tool revisions, device class, seed) and
   atomic output replacement, so a failed or cancelled step leaves the previous
   package intact.
7. `pbrt_map_build.py`, the `worldstage_cycles_*` scripts and
   `probe_volume_bake.py` call the baker. Their Blender bake paths are deleted
   once fixture outputs match within declared tolerances.
8. Editor-facing requests (ahead of R52/R57):
   - the baker can be called in-process and over the R40 structured process
     protocol;
   - a `BakeRequest` can name a dirty subset (charts, a probe region, a light
     style) against a previous result;
   - providers may report intermediate sample counts for progressive display.

### Done looks like

- The shared suite passes for the fake, legacy and Cycles providers, and all
  five bad providers fail it. The suite is registered in
  `quality/conformance.manifest.json`.
- vrad legacy lumps are byte-identical before and after adaptation on the
  54-map BSP2 corpus.
- Cycles passes the RFC analytic oracles:
  - a light along each basis vector;
  - the white furnace;
  - point-light calibration;
  - SH L1 within its bound. The fit is fixed, or a reviewed bound is recorded.
- Two CPU bakes with the same input, seed and profile are bit-identical.
- The six RFC 0011 GI fixtures and the living-room PBRT map build through the
  baker, with no Blender process in the product bake path.
- No basis constant or L1 fit is defined outside its owner.
- Tests show that a one-chart edit re-bakes only that chart, and that a
  cancellation mid-bake leaves the package unchanged.

Non-goals: RFC 0011 G2+ runtime features, IBL (R50), changes to the D3D9 legacy
payload, GPU-exact determinism, and editor UI.

Open decision: reference renders (`gi_reference_blender.py`) should stay on an
independent scene-translation path (Blender or pbrt), declared as oracle
tooling. A reference built by the baker's own translation would share its bugs.
This needs to be recorded in the RFC before the Blender product paths are
removed.

Prerequisites: R03 (C++20 target profile), R48's Linux vrad build (installed),
and R54 charts (partial).

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

## Canonical PBR material on native Vulkan WMSH (R47 slice, 2026-09-23)

The native Vulkan standard-shader build now registers `PBRMetalRough` as a
material shader. It loads the required base color and MRAO textures with their
declared color encodings, binds optional normal data, requests sRGB output, and
feeds the BSP2 WMSH PBR pipeline through the existing Source material pass.
Missing required texture parameters issue a material warning; the shader and
API refuse to queue an invalid WMSH draw. Emissive, env-map and translucent
materials warn and decline the native draw until their feature paths exist.
DX9/DXVK keeps its established
`Wireframe_DX9` fallback registration and `$fallbackmaterial` behavior.
Existing `PBR` content is unchanged.

The canonical staircase package boots on native Vulkan with nine material
batches and 1,520 meshlets; the log identifies `staircase2/chrome shader
PBRMetalRough`. Native and DX9 built-in shader conformance each pass **302/0**;
the native WMSH pixel suite passes **25/0**, and the DXVK fallback material
pixel run passes. An unsupported env-map negative fixture warns once, drops
only its material batch, and leaves 1,500 other meshlets queued. The corrected
Cycles light bake,
sRGB output and atlas filtering are recorded in
[RFC 0008 progress](0008-progress.md#f4-canonical-native-pbr-material-and-linear-light-correction-2026-09-23).

This implementation covers BSP2 world batches. The older native
`pbr-fallback` screen-space fixture now fails because its expected legacy
fallback is no longer selected; that failure is retained in the RFC 0008
audit receipt. Dynamic meshes need a native PBR pass and a new positive pixel
oracle before the material family is complete. Reflections, transmission,
coat, KTX2 base textures, and cross-platform GPU evidence also remain open.

## PBRMetalRough on models, emission, $envmap and validation-clean GPU suites (R47 slice, 2026-09-24)

Scope (owner: session source-engine-79). This slice adds a native pass for dynamic
meshes, a positive oracle to replace the stale native `pbr-fallback`
expectation, and the emission and environment-map features that were declined
before.

- **Model pipeline.** `shaders/model_pbr.frag` (`vulkan_model_pbr.cpp`,
  `kDynShaderPbrModel`) draws PBRMetalRough on every non-WMSH mesh:
  - It runs on the skin pipeline's layout and vertex stage. The shader API
    already converts those vertices on the CPU: world-space position, normal,
    tangent (skinned or MODEL-transformed) and each vertex's attenuation of
    the four sorted lights.
  - The material's dynamic state commits the ambient cube (c4..c9) and the
    lights (c20..c25), as `skin_dx9_helper.cpp` does.
  - The BRDF is `pbr_brdf.h`'s layered GGX. Source's model-light units make a
    local light incident radiance π·color·attenuation, so a rough dielectric
    keeps VertexLitGeneric's brightness.
  - Specular image light comes from the `$envmap` cube, else the map's LMAP
    probe, else the ambient cube in the reflected direction.
  - A mesh without normals shades toward the eye instead of producing NaN.
  - A base stored in an sRGB format is never decoded twice.
- **Feature flags.** PBRMetalRough reports its features in c3.x
  (`render::pbr::NativeFeature`) and `$emissionscale` in c2.x. Pipelines
  decline, by name, only the features they lack: glass on models, and glass
  combined with emission or `$envmap`.
- **WMSH.** `world_pbr.frag` adds `$emissiontexture` (sRGB, decoded in the
  shader) × `$emissionscale` and the `$envmap` cube. The layout grows to 7
  sets, and `material.zw` carries the emission scale and cube mip count. A
  non-cube `$envmap` fails selection. Alpha-blended PBRMetalRough WMSH
  batches are no longer declined; they draw in the engine's back-to-front
  translucent WMSH pass.
- **Validation.** `CreateShaderModule` reflects each vertex module's SPIR-V
  input locations, and pipelines drop the attributes their vertex stage does
  not read. That removes every `WARNING-Shader-OutputNotConsumed`.

Evidence (RADV Radeon 8060S; private tree `build-vk79`; headless
`SDL_VIDEODRIVER=offscreen`). The Khronos validation layer was loaded from
Steam's runtime copy through a scratch manifest and `VK_ADD_LAYER_PATH`.

| Suite | Result | Validation messages |
| --- | --- | --- |
| `model_pbr_native_pixel_conformance` (new) | 30/0 | 0 |
| `world_pbr_native_pixel_conformance` (+ emission, `$envmap`, non-cube rejection) | 41/0 | 0 |
| `world_glass_native_pixel_conformance` | 36/0 | 0 |
| `pbr_native_pixel_conformance` | 32/0 (was 31/1 with validation on) | 0 |
| `ktx2_native_pixel_conformance` | 23/0 | 0 |
| `native_vulkan_bringup_conformance` | 99/0 (was 98/1 with validation on) | 0 |
| `material_equivalence_vulkan_conformance`, `material_facing_vulkan_conformance` | 64/0, 25/0 | layer not requested |
| `builtin_shader_conformance` | 306/0 | — |

- The model suite's CPU model uses `pbr_brdf.h`. Seven cases (ambient,
  lit dielectric, sRGB-stored base, lit metal, normal map, emission,
  `$envmap`) each match within 3 levels. Seeded defects are each separated by
  more than 6 levels: ignored occlusion, attenuation or Fresnel weighting,
  specular without π, MRAO decoded as sRGB, and emission bytes used undecoded.
- New material-system family `pbr-model`
  (`tools/quality/material_pixel_pbr_model.py`):
  - The modellight harness's quads, ambient cubes, point/spot/directional
    lights and rigid/skinned placements, drawn with four PBRMetalRough
    materials.
  - Judged per pixel against an independent layered-BRDF model that reads the
    generated split-sum table. It passes in `--hdr none` and `--hdr integer`
    over 57,976 judged pixels per case.
  - Pixels within 0.04 of N·V = 0 are not judged (`HORIZON_MARGIN`). At the
    horizon, both the GPU and the model switch terms, and the sub-pixel
    sample position decides the result, exactly as `EDGE_MARGIN` handles
    coverage.
  - Every seeded defect in `CONTROLS` must fail. The versioned native capture
    `quality/fixtures/material-pixels/pbr-model-native-vulkan-none.json`
    drives 6 oracle self-tests.
  - D3D9 draws the fallback, so this family has no cross-backend reference.
    A reference argument is refused.
- `pbr-fallback` now has a native contract: native resolves `PBRMetalRough`
  (including the valid primary patch), rejects every invalid definition, and
  leaves pixels to `pbr-model`. The D3D9/DXVK contract is unchanged. It
  passes on native, and the oracle suite passes 92/0 (3 new native-contract
  tests).
- Regression runs on native pass: modellight in both HDR modes against the
  D3D9 references, lightmap, skinning, portal, sprite, cable, sky and monitor.
- In-game:
  - `living_room` boots on `build-vk79` with WMSH PBR and the 2048² LMAP
    (`quality-results/vk79-boot-living3`).
  - A private staged Portal runtime mounted a PBRMetalRough override of
    `models/props/metal_box` through `portal/custom/` (searched before the
    VPKs). `prop_physics_create props/metal_box` draws it through shader 11,
    the model PBR pipeline, with its normal map
    (`quality-results/vk79-pbrbox3`). `portal_boot.py --content-root`
    deliberately cannot shadow installed content, so the relaunch used the
    recorded command.

Not covered:
- Static props' baked vertex lighting (color mesh) is not used by the model
  pipeline; it lights with the ambient cube and local lights.
- Clear coat, model glass, and flashlight/projected-texture passes for PBR
  models.
- Other GPUs and the Android/Apple profiles.

## Energy compensation, one GLSL BRDF, and grouped descriptor sets (R47 / R29 prep, 2026-09-25)

Scope, at the user's direction: three of the follow-ups from the comparison
with Google's Filament. Clean baseline before the change, at `7aca8408`
(archived tree): all 9 PBR suites passed.

- **Multiple-scattering energy compensation.**
  - `pbr_brdf.h` adds `SpecularEnergyCompensation`,
    `SpecularDirectionalAlbedo` and `EvaluateSpecularMultiScatter`, using
    Kulla–Conty in Filament's form `1 + F0·(1/(A+B) − 1)`.
  - `EvaluateLayeredDirect` now compensates the lobe and layers diffuse under
    the compensated albedo.
  - The split-sum table already carried the term (`A + B` is the F0 = 1
    albedo). Its axes were wrong for it, though: the last row sat at
    roughness 0.984, so roughness 1 was under-compensated by 6%.
  - The table now includes its end points (texel `i` at `i/(size−1)`). The
    C++ lookup, the GLSL lookup (`PbrSplitSum`) and the Python oracle
    (`material_pixel_pbr_model.py`) read it the same way.
  - White furnace, compensated: 0.9995–1.0004 for a white metal at roughness
    0.5–1. Single-scatter was 0.307 at roughness 1, N·V 1.
  - `render.pbr-brdf` adds the rough-metal and colored-metal furnace checks
    (55 checks). The new sensitivity suite
    `render.pbr-brdf.no-energy-compensation` must fail the rough-metal check.
- **One GLSL BRDF.**
  - `shaders/pbr_brdf.glsl` holds GGX, Smith, Schlick, the lobe, split-sum
    sampling, the compensation, the directional albedo and the clear coat.
  - `pbr_direct.frag`, `world_pbr.frag`, `world_pbr_glass.frag` and
    `model_pbr.frag` include it and define none of their own. Before this
    there were three GGX copies.
  - `EvaluateClearCoat` joins `pbr_brdf.h`, so the coat has a C++ reference.
  - The GGX denominator is evaluated as `(1−(N·H)²) + (N·H)²α²`, the same
    value algebraically. At roughness 0.02 and N·H = 1 the textbook form lost
    20% in float on the host; the new GPU check found it.
  - `render.pbr-brdf.glsl` (GPU) evaluates the library in a compute program
    (`pbr_brdf_check.comp`): 1,080 of 1,080 cases agree within
    `1e-6 + 3e-4·|C++|`. Seeded C++ models are rejected: no compensation
    (234 cases agree), α = roughness (180), and an unweighted coat (368).
  - `tools/quality/tests/test_pbr_shader_library.py` guards the source: every
    stage includes the library, no BRDF term is defined outside it, and no
    grouped stage binds a set above 2. It has 6 tests, 4 of them negative.
- **Descriptor sets by update frequency.**
  - `vulkan_descriptor_groups.{h,cpp}` (`CGroupedDescriptors`) gives the PBR
    and GI stages three sets instead of up to eleven:
    - set 0, the frame's: split sum, lightmap, indirect source, probe grid,
      shadow field;
    - set 1, the material's: base, MRAO, normal, emission or scene color,
      `$envmap` cube, scene depth;
    - set 2, the constants ring's dynamic block.
  - Sets 0 and 1 are written into per-frame-slot pools. A slot is reset only
    at its `BeginFrame`, after its fence. Equal requests in a slot share one
    set, a full pool grows the slot, and `Invalidate` follows an in-place
    sampler replacement.
  - The `maxBoundDescriptorSets` gates on WMSH PBR (7), direct lights and
    RuntimeIndirect (10), BakedPlusDelta (11), glass (7) and model
    probe-volume sampling (9) are gone.
  - `InitSkinPipeline` now creates the shared constants ring and vertex stage
    before its own 7-set gate, so only `$phong` needs seven sets. Model PBR
    has its own layout.
  - The white volume now lives beside the white cube, and
    `m_maxImageDimension3D` is set at device creation. Before this, a device
    without the post pipeline could not create any volume texture, the SDF
    shadow field included.
  - `VulkanContextConfig::descriptorSetLimit` lets suites act as a smaller
    device.

Evidence (RADV Radeon 8060S, headless, validation layer on):

| Suite | Result |
| --- | --- |
| `render.pbr-brdf` (+ 4 sensitivity suites) | 55/0, each seeded defect detected |
| `render.pbr-brdf.glsl` (new) | 9/0 |
| `render.grouped-descriptors` (new, seeded view-only key detected) | 15/0 |
| `render.world-pbr.native-pixels` / `.four-sets` | 94/0 / 94/0; PBR layouts use 3 sets |
| `render.model-pbr.native-pixels` / `.four-sets` | 67/0 / 67/0 |
| `render.world-glass.native-pixels` / `.four-sets` | 36/0 / 36/0 |
| `render.indirect-switching.native-pixels` / `.four-sets` | 83/0 / 83/0 |
| `render.pbr-direct.native-pixels` | 32/0 |
| `tools/quality/tests/test_material_pixel_conformance.py` | pass |

The `.four-sets` variants run the whole suite as a device that binds four
descriptor sets, the Vulkan minimum. Every PBR and GI variant is built and
draws the same checked pixels. LightmappedGeneric (9 sets) and `$phong`
(7 sets) are declined there, by name.

Pixel changes:
- Rough metals are brighter: up to 3.3× the old single-scatter specular at
  roughness 1.
- The `pbr-direct` colored-metal case's radiance went from 10 to 8 so the
  compensated red stays under the 8-bit ceiling.

Not covered:
- No suite draws BakedPlusDelta's pixels. The `.four-sets` run proves only
  that its pipelines build and validate at 4 sets.
- The legacy LightmappedGeneric and `$phong` layouts still need 9 and 7
  sets. A 4-set device keeps PBR and GI but not those legacy stages.
- Nothing ran on MoltenVK, Mali, the Fold7 or other GPUs, and no in-game
  capture was taken of the brighter rough metals.
- R29 platform evidence remains open.

## R50-PARALLAX: parallax-corrected, blended reflection probes (bounded R50 slice, 2026-09-25)

Scope, at the user's direction (2026-09-25): replace the scene-map
pipeline's single direction-only probe with automatically placed,
parallax-corrected, blended reflection probes carried by RFC 0008's `RPRB`
lump. This is a bounded slice of R50 (RFC 0007 F). It does not close R50: the
legacy runtime prefilter, IBL pixel fixtures across material families and
cache invalidation remain, and R50's prerequisites R47 and R56 are open.

### Why, and the sources

- **The problem.** The old pipeline rendered one probe at the scene's
  centre and sampled it by direction only. That assumes an infinitely
  distant scene, so the reflection does not move as the viewer does and is
  skewed everywhere except at the capture point. 3kliksphilip's "Advanced
  Reflections in CS:GO... and for Source 2?" (2019-12-01,
  https://www.youtube.com/watch?v=uX5krqI51hQ) shows each failure this slice
  fixes:
  - static reflections (01:49);
  - a proxy that is the wrong shape: a sphere at the world origin (03:19);
  - hand-tuned parallax per surface (04:15);
  - splay where the reflection meets the floor (04:26);
  - separate mirrors needing separate cubemaps (05:02);
  - Source 1 switching to the nearest cubemap instead of blending (06:13);
  - Source 2 apparently using parallax-corrected cubemaps for VR (06:52).

  The subtitles and a timestamped transcript are kept outside the repository
  (`~/Downloads/kliksphilip-cubemaps/`). The video cites Valve's developer-wiki
  page "Parallax Corrected Cubemaps"; that page could not be read here (a
  bot challenge), so nothing below relies on it.
- **Box proxy and blending.** Lagarde and Zanuttini, "Local Image-based
  Lighting With Parallax-corrected Cubemap", SIGGRAPH 2012
  (https://seblagarde.files.wordpress.com/2012/08/parallax_corrected_cubemap-siggraph2012.pdf):
  - intersect the reflected ray with a box proxy and sample toward the hit
    from the capture point; the box centre need not be the capture point;
  - influence volumes whose weights are 0% at the boundary and 100% inside
    an inner range, with a smaller volume inside a larger one taking
    precedence, normalized;
  - a limit on overlapping cubemaps (4 there) pops when a fifth overlaps;
  - glossy normals reaching the lower hemisphere can miss the box.
- **Distance roughness and easing.** Lagarde and de Rousiers, "Moving
  Frostbite to Physically Based Rendering 3.0", SIGGRAPH 2014 course notes
  (https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf):
  - distance-based roughness, `clamp(d_point / d_capture * r, 0, r)` eased
    back to `r` by `r` (Listing 25);
  - the corrected direction eased toward the reflected ray by roughness,
    and a smoothstep fade at the influence boundary (Listing F.1).

  Correction to the proposal: distance-based roughness is from the 2014
  notes, not the 2012 talk.

### Design and deviations from the sources

- **Proxy fit.** The capture's own depth pass fits the proxy: Cycles' Depth
  pass on the six cube faces the probe already renders, so no extra render.
  - Each box plane is the farthest strong planar peak of that face's
    solid-angle-weighted distance histogram, taken as a weighted median:
    peaks at least `PLANE_FRACTION` 0.25 of the strongest.
  - A plain high percentile is pulled through a doorway (the test
    `test_a_plain_percentile_would_have_gone_through_the_doorway` guards
    this) and is biased by the floor's continuous run of distances up to the
    wall.
  - A face that is mostly sky stays open (1000 m), which is right for
    windows.
  - The packer checks the depth convention (planar camera z) against BVH ray
    distances the renderer records at 64 texels per face before trusting it.
- **Placement.**
  - **Room probes.** Greedy cover of every eye-height point above a floor
    (1.63 m, 0.75 m grid) by a probe whose estimated box contains it and
    which sees it; each new probe is moved to the candidate nearest the
    middle of what it newly covers. Separate rooms therefore get separate
    probes.
  - **Glossy probes.** Every sample on a glossy surface (base or clear-coat
    roughness ≤ 0.35) needs a capture within 2.5 m in front of it that sees
    it. Uncovered ones get a probe whose influence is those samples' bounds,
    so a mirror gets its own probe.
  - Manifest `position(s)` seed captures. The largest room probe is global.
- **Blend** (our choices within Lagarde's rules):
  - Probes are visited by rank: influence volume ascending, the global probe
    last. Each takes its weight times the weight still unassigned; the
    global probe takes the rest.
  - A weight is 1 inside the influence box and falls to 0 at `fade`
    (0.5 m) outside it (a smoothstep). The influence box is the parallax box
    grown by 0.15 m, so the room's own walls get full weight.
  - Not in the sources: a facing term (smoothstep of the cosine over
    ±0.1) drops a probe whose capture lies behind the shaded surface, so a
    room's probe does not reflect onto the far side of its wall.
  - At most two probes are sampled per pixel: the two largest shares, each
    less the third largest, renormalized. This is continuous where the
    ranking changes, unlike Lagarde's fixed cap.
  - Weights are per shaded point, so a surface point's probe mix never
    depends on the camera. The Source 1 failure is therefore a spatial seam
    where one surface switches probe, and the walk gate measures that.
- **Interim step not built.** The proposal's interim carrier (the probe's
  position and box in extra LMAP marker texels until RPRB existed) was not
  built: RPRB landed in the same change, and a second carrier would have
  duplicated it.
- **Carrier.** `RPRB` v1 is raw RGBA16F like `PRBV`, not KTX2 (RFC 0008
  lists KTX2; BC6H/ASTC transcode is later work). Equirect mips use the
  legacy band's layout, and each probe has a band.
  - The GPU form is one texture: a record table with hi/lo half pairs
    (about 0.01-unit precision), header marker −3, then the atlas.
  - It binds in the frame descriptor set (vulkan_descriptor_groups.h) for
    world, glass and model PBR. Maps built before RPRB keep their LMAP band
    probe, which the shaders still read.
  - The scene sun's LMAP marker texels (no shader reader) are no longer
    written.
- **Modes.** `mat_reflection_probes`: 1 blend (default), 0 off, 2 nearest
  capture (Source 1's switch, a negative control), 3 direction-only (the
  old lookup, a negative control). +4 is the weight view: palette colour per
  rank times weight.

### Evidence

All on the Fedora host (RADV Strix Halo, Radeon 8060S), 2026-09-25:

- **Math and format** (`tools/quality/tests/test_reflection_probe_set.py`,
  23 tests).
  - The lookup is exact when the box is the room; a ray from the capture
    point is unchanged; a point outside whose ray misses keeps its ray.
  - Frostbite Listing 25 is reproduced to its worked values.
  - The box fit:
    - recovers an empty room exactly;
    - ignores a sofa;
    - ignores a doorway, where a plain percentile would have gone through
      it;
    - stays open to sky;
    - scores a wrong box.
  - RPRB round-trips, and each of the 20 malformations fails with its code.
  - Blending: weights sum to one on at most two probes, and a smaller volume
    wins. A walk is continuous within the smoothstep's slope, while the
    nearest-capture mode jumps.
  - Placement: each room of a two-room scene gets a probe whose box is that
    room, doorway geometry included, and a far mirror gets its own probe.
  - Analytic mirror-floor oracle, mean error against traced ground truth:
    fitted box 0.030, direction-only 0.39, wrong box 0.45.
- **C++ reader** (`world.reflection-probes`, 57 checks; clean under clang
  ASan/UBSan).
  - The fixture validates, and its GPU texture is byte-identical to the
    Python writer's.
  - Every corpus edit fails with the Python reader's error.
  - The reference blend reproduces the Python oracle's 192 samples (four
    modes) within 1e-6.
  - The continuity walk and the facing rule hold; 3,000 fuzzed mutants never
    crash.
- **GLSL** (`render.reflection-probes.glsl`, 30 checks):
  `shaders/reflection_probes.glsl` in a compute program on the device agrees
  with the C++ reference on 448 cases in each of four modes. Every other
  mode's reference is rejected (0 to 280 of 448 agree), and a texture
  without the marker carries no probes. No validation messages.
- **Existing GPU suites still pass** after the frame-set binding and shader
  change, including the `.four-sets` variants:
  - `render.world-pbr`, `model-pbr`, `world-glass` and
    `indirect-switching.native-pixels`;
  - `render.pbr-direct.native-pixels`;
  - `render.grouped-descriptors`.
- **Real scene: living room** (`scene-v4.pbrt` on its existing lighting
  stage).
  - Placement: two probes (the room, seeded at the manifest's position, and
    the window bay), all 36 walkable samples covered, global = the room.
  - The Depth pass's convention was verified as planar (median error 0 at
    the BVH samples).
  - Mean box residuals 0.050 and 0.067. 43 of 83 glossy samples are visible
    from eye height; 15 of those are unserved, none in a patch of 0.5 m².
  - In game (`build/`, native Vulkan, the map's BSP2 with the RPRB lump
    added): the engine logs "RPRB v1, 2 reflection probes".
    - With the old LMAP-band probe (`mat_reflection_probes 0`), the floor
      shows bright halos by the walls and sofa, and the mirror above the
      fireplace is black.
    - With the blend, the floor reflection sits in place and the mirror
      reflects the room.
    - The weight view shows the bay probe fading into the room's.
- **Mirror gate** (`reflection_runtime.py mirror`, record
  `quality-results/reflection-gates/mirror/mirror.json`): mirror-room, four
  probes (the room plus three glossy-floor probes, boxes within 0.0004 of the
  room), three cameras away from every capture. Floor error over wall error,
  per view:

  | Run | Floor/wall per view | Mean floor error |
  | --- | --- | --- |
  | Blended | 1.52–1.80 | 0.187 |
  | Direction-only | 7.6–8.2 | 0.883 |
  | Boxes shifted by (1.0, 0.8) m | 4.4–6.0 | 0.597 |

  Wall error is 0.10–0.12 in every run; the limit is 2.5 and the control
  margin 1.5×. PASS.
- **Walk gate** (`reflection_runtime.py walk`, record
  `quality-results/reflection-gates/walk/walk.json`): two-rooms, one probe
  per room (boxes exactly the rooms), 25 stations through the doorway.
  - Blended weight view: the largest red-share step between neighbouring
    floor pixels is 0.047 (limit 0.25), with the share running 0 to 1 over
    the walk.
  - Nearest-capture control: 1.0 (seam minimum 0.6). PASS.
- **Audit budgets.**
  - `reflection-fixture`: residual ≤ 0.15, no uncovered walkable sample,
    unserved glossy ≤ 0.1, budget not exhausted. Both fixture maps pass
    their audits and are published as playable maps (reflection_mirror_room,
    reflection_two_rooms).
  - `source2`: residual ≤ 0.25, uncovered walkable ≤ 0.1, unserved glossy
    ≤ 0.5, placement not stopped by `max_probes`. These are set from the
    living room (0.067, 0, 0.35), the one furnished scene measured.
- **Placement lessons.**
  - A capture in front of a doorway fits a box through it.
  - Candidates therefore need 0.5 m clearance and a 90th-percentile box
    residual of at most 0.35. A box through a doorway scores 0.4–0.6; the
    mean is diluted.
  - Each wall plane is bounded by how far the ceiling runs unbroken from the
    zenith. A room's ceiling reaches its walls whatever furniture stands on
    the floor, while the next room's ceiling seen under a lintel is a
    separate run.
  - The global probe is the room probe covering the most walkable space: an
    open box to the sky (a window bay) can be the largest by volume.
- **Pipeline fixes on the way.**
  - A stale source list in `test_bsp2_reader`'s bsp2tool build now compiles
    every `mapcontainer` source.
  - `FloatToHalf` has one owner (`mapcontainer`); `render/indirect_light.h`
    forwards to it.
  - Fixture bakes run on the CPU. On this host the GPU's HIP compute side
    wedged at 02:06 (the kernel logged "amdgpu: MES failed to respond to
    msg=REMOVE_QUEUE", then a page fault). Every later Cycles GPU job hung in
    `hipStreamCreate` (`hsa_signal_wait_scacquire`) until a GPU reset or
    reboot; CPU bakes were unaffected.

### Not done or not verified

- The rest of R50: the legacy maps' runtime `env_cubemap` prefilter, IBL
  pixel fixtures across the material families, and cache invalidation.
- A KTX2/BC6H RPRB payload; a probe-count or memory budget per mobile
  profile.
- Android, macOS and iOS runs. The RPRB lump needs no KTX reader, but it
  is untested on the Fold7.
- D3D9/DXVK, which ignores RPRB.
- Dynamic objects are not in the probes.
- No pixel oracle covers models on an RPRB map; only the shared shader path
  and the GLSL suite cover them.
- The source2 budgets rest on one furnished scene.
- Pre-existing, unrelated test failures seen here:
  - `test_android_profile.test_pins_are_complete`: a peer added
    `ktx_software`;
  - `test_usd_scene`: the system Python has no `pxr`.

## R50-RELIGHT: relightable reflection probes (bounded R50 slice, 2026-09-25)

Scope, at the user's direction (2026-09-25, a goal that also closed RFC 0011
decisions 4 and 6): make dynamic light reach specular. This is RFC 0011's
open decision 5, which the user placed as an amendment to this RFC's
image-based lighting. The amendment is in the RFC's
[image-based lighting](0007-physically-based-lighting-pipeline.md#image-based-lighting)
section. This builds on R50-PARALLAX and does not close R50.

### The problem and the source

- **The problem.** The world follows the runtime light set and the indirect
  producers (RFC 0011): unbaked lights with SDF shadows, and the change
  volume. The RPRB probes are fixed at bake time. A bulb switched on after
  the bake lights the west wall, but the mirror floor keeps reflecting the
  wall dark.
- **The source.** McAuley, "Rendering the World of Far Cry 4" (GDC 2015;
  transcript at archive.org, "GDC2015McAuley"):
  - it stores an albedo and a normal cubemap;
  - it relights them from the sun, the sky and the nearest probe's indirect
    light;
  - it GGX-prefilters the result at runtime (0.066 ms to light and 0.275 ms
    to filter a 128² cube);
  - it has no shadows, because it stores no depth.

### Design and deviations

- **Data (RPRB v2).**
  - Each probe gets two relight bands in its radiance chain's mip layout,
    box-filtered:
    - albedo RGB (Cycles' Diffuse Color pass) with the ray distance in
      alpha (the Depth pass the box fit already reads);
    - the world normal (the Normal pass).
  - Both come from the faces the probe already renders: no extra render.
  - `pbrt_reflection_probe.py --gbuffer` adds the two passes, enabled by
    `reflection_probe.relight` in the manifest or profile.
  - `reflection_probe_set.py` owns the encoding. The version is 2, and
    header flag 1 means relight. Each record's last word is its albedo
    band's row. v1 payloads still read.
  - The C++ reader (`mapcontainer/reflection_probes.{h,cpp}`) mirrors the
    Python reader, and the engine and bsp2tool carry the payload's version
    as the lump version.
  - The GPU table's field 3 w is the relight row. The mode texel's y is the
    switch `mat_reflection_relight` (cheat, default 1).
- **Shading** (`reflection_probes.glsl`). At a probe's lookup direction and
  lod, the seen point is capture + direction × distance, with the band's
  normal. The probe gains albedo × `ReflectionProbeDiffuseChange(point,
  normal)`, clamped at zero.
  - The includer supplies the change. `world_pbr.frag` supplies what the
    world itself adds in that variant: the change volume (`DELTA_VOLUME`)
    and the unbaked lights' SDF-shadowed direct light (`DIRECT_LIGHTS`).
    One owner per term: the same functions, now taking a position.
  - It is exact in the baked state and for a mirror of a Lambertian surface.
- **Deviations from the source.**
  - The stored distance lets the relit point be shadowed and lit by point
    lights.
  - The change is added at lookup time instead of relighting texels and
    re-prefiltering, so a rough lookup takes the change at its lobe's
    centre.
  - There is no pass, prefilter or per-probe runtime storage. The cost
    scales with pixels, not probes.
- **Cost policy** (`kRelightMinWeight`). The change is skipped for a lookup
  whose share of the pixel is below 0.1 (directional albedo luminance times
  the probe weight).
  - A matte dielectric's reflection carries about 4% of what it sees, and
    the pixel already shows the change diffusely.
  - Mirrors, metals and glossy floors at grazing angles stay relit.
- **Not relit.**
  - The glossy part of what a probe sees stays as baked, and so do its
    emitters and sky.
  - Under `RuntimeIndirect` without unbaked lights there is no change to
    apply (that variant has no change volume).
  - Glass and model PBR do not define the change, so their probe lookups
    stay as baked.

### Evidence

All on the Fedora host (RADV Strix Halo, Radeon 8060S), native Vulkan,
2026-09-25:

- **Oracles and parity.**
  - Python (`test_reflection_probe_set.py`, 27 tests, 4 new): a mirror
    floor reflecting a Lambertian room after a point light is added.
    - The relit probe is off by 6.6% of the level, within 2 points of the
      lookup's own 5.1% on the baked room (stripe edges).
    - Unrelit it is 33% off; with the distances halved (the seen point
      misplaced), 47%.
    - A zero change is the baked probe exactly.
    - The v2 round trip and its corpus pass, and v1 still reads.
  - C++ (`world.reflection-probes`, 81 checks, was 57):
    - the relight fixture validates, and its GPU texture is byte-identical
      to Python's;
    - all 11 relight malformations fail with the Python reader's codes;
    - the relit reference reproduces the Python oracle's 144 relit samples
      within 1e-6 (largest relight 0.58);
    - a zero change reproduces the baked radiance exactly;
    - a second fuzz pass runs over the v2 payload.
  - GLSL (`render.reflection-probes.glsl`, 52 checks, was 30): in each of
    blend, nearest and direction-only modes:
    - relit, all 448 cases match the relit reference;
    - the unrelit reference agrees on none;
    - with the switch off, all 448 match the unrelit reference;
    - no validation messages.
- **In-game gate** (`reflection_runtime.py relight`, record
  `quality-results/reflection-gates/relight/relight.json`).
  - The fixture is `mirror-lamp` (profile `reflection-relight-fixture`,
    5 probes, 1024 wide, max box residual 0.021):
    - the mirror room with a pillar;
    - a 2-unit bulb that the bake hides;
    - in game, an inverse-square `light_dynamic` (the new manifest
      `collision.dynamic_lights`);
    - Cycles references rendered with the bulb lit.
  - The SDF producer ran, warmed up and converged.
  - Scoring is `mirror`'s floor/wall ratio after the wall-gain fit. The floor
    pixels that mirror the bulb itself are left out (about 180–450 of
    19–23k), because the bulb is not in any probe.

  | Run | Floor/wall per view (east, far, south) | Mean floor error |
  | --- | --- | --- |
  | Relit | 1.34, 1.20, 1.61 (mean 1.38) | 0.369 |
  | As baked (`mat_reflection_relight 0`) | 2.88, 2.97, 3.21 (mean 3.02) | 0.806 |

  The limit is 2.5 per view, and the control must be at least 1.5× the
  relit mean (it is 2.2×). PASS. The engine logs "RPRB v2, 5 reflection
  probes, 9 mips from 1024 wide, relightable".
  - The wall error (0.26–0.29, against 0.10 in mirror-room) is reference
    noise. The bulb reaches the walls through the mirror, a caustic that
    shows as fireflies at 1024 samples, which the ratio normalizes.
  - By eye, relit, the floor mirrors the bulb-lit west wall and the pillar
    as the direct view shows them. As baked, it mirrors them dark.
- **Cost** (`reflection_runtime.py relight-cost`, record
  `quality-results/reflection-gates/relight-cost/relight-cost.json`). At
  1920 × 1080 from the east camera, SDF producer settled:
  - GPU frame median relit 1.195 and 1.201 ms, as baked 0.875 ms;
  - relight 0.323 ms against the declared 0.5 ms desktop budget. PASS.
  - Before the cost policy it was 0.678 ms and failed.
  - Attribution, measured before the policy: 0.591 ms with shadows off, and
    no measurable cost with the baked producer (no change volume). So the
    change volume's sampling, done per lookup, dominated, and the SDF
    shadow was about 0.09 ms.
- **No regressions.**
  - The R50-PARALLAX mirror gate on the rebuilt client gives the same
    numbers: blended 1.68 mean floor/wall, direction-only 7.95, wrong box
    5.36. PASS.
  - `render.world-pbr`, `model-pbr`, `world-glass`, `indirect-switching`
    (each with `.four-sets`), `pbr-direct` and `grouped-descriptors` pass.
  - `test_gi_tools`, `test_map_export` and `test_pbrt_gates` pass (73).
  - `reflection_fixtures.py --check` passes.

Reproduce:

```sh
python3 tools/quality/reflection_probe_set.py fixture
python3 tools/quality/conformance.py check --suite world.reflection-probes
python3 tools/quality/conformance.py check --runner gpu --suite render.reflection-probes.glsl
python3 tools/quality/pbrt_map_build.py --manifest quality/fixtures/reflection/mirror-lamp/map.json \
    --out quality-results/reflection-maps/mirror-lamp
python3 tools/quality/reflection_runtime.py references --fixture mirror-lamp --device cpu
python3 tools/quality/reflection_runtime.py relight --build build --out quality-results/reflection-gates/relight
python3 tools/quality/reflection_runtime.py relight-cost --build build \
    --out quality-results/reflection-gates/relight-cost
```

A runtime is staged once per output directory, and a rerun into the same
`--out` boots the old binaries. Give each build a fresh `--out`.

### Not done or not verified

- Glass and model PBR lookups are not relit. They do not bind the direct
  light block or define the change.
- Rough lookups take the change at the lobe's centre. No rough-surface
  pixel fixture measures that error.
- Baked light styles switched at runtime reach specular only through the
  producer's change volume, not per-style probe layers.
- Legacy `env_cubemap` probes carry no G-buffer.
- Android, macOS and iOS are not run; no mobile cost budget exists. The
  Fold7 has no SDF shadows (unshadowed relight) and would pay the per-lookup
  light loop.
- DXVK/D3D9 ignores RPRB.
- **An observed defect, not from this slice.** On `mirror-lamp` the bulb's
  pool of light on the west wall and ceiling shows faint concentric rings in
  the world's own direct light (G9's SDF shadow of an unbaked light), and
  the relit reflection reproduces them.
  - Hypothesis: the sphere trace's penumbra estimate bands near the
    surface it leaves. Not investigated.
  - Evidence: `quality-results/reflection-gates/relight/relit/`.
