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
