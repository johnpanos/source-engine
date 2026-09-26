# Native Vulkan legacy shader ports — progress

Scope: draw every stdshader_dx9 material shader that the native Vulkan shader
API (`materialsystem/shaderapivulkan`) can select. The shader API used to decline
these passes (`NoteDroppedMaterial`) or approximate them in a native family. Each
one now runs through a GLSL port of its D3D9 shader pair, and each port is
checked against that pair's compiled bytecode. Roadmap context: R32 (native
Vulkan functional MVP: "unsupported features fail explicitly"), tracked as
**R32-LEGACY-SHADERS**. This record closes no roadmap gate.

Status (2026-09-25): 84 ports cover every material shader of stdshader_dx9 that
this backend can reach at DX level 95 / shader model 3. The exceptions are listed
under "Not ported". The ports were written in the `source-engine-vkshaders`
worktree on `223f0ed8` (committed there as `15d60c4f`) and merged into
`subsystem-refactor` on 2026-09-25; see "Merge into subsystem-refactor" below for
what the merge kept native and its evidence. Evidence was taken on an AMD Radeon
8060S (RADV, Mesa), headless (`SDL_VIDEODRIVER=offscreen`), in build tree
`build-vk`.

## How it works

The porting guide is `materialsystem/shaderapivulkan/shaders/legacy/README.md`.
In short:

- **One generic pipeline family** (`kDynShaderLegacy`) runs every port.
  `TakeSnapshot` routes a pass by its exact pixel and vertex shader names
  (`FindLegacyProgram`) to `legacy#<program>#<ps static>#<vs static>`.
  - Set 0 holds up to 16 combined image samplers, allocated per draw from a
    per-frame pool.
  - Set 1 is the `LegacyConstants` block: pixel c0..c31, vertex c0..c63 and
    c217..c224, the combos, the boolean banks, i0, and the render target size.
  - The push block is the skin layout: viewProj, the alpha test, the manual
    sRGB decode mask (bits 0..15) and encode flag (bit 16), and the clip planes.
  - Two sets fit MoltenVK's limit of eight.
- **Derived registers follow D3D9's commit** (`LegacyTransformCommit`,
  mirroring `CShaderAPIDx8::CommitVertexShaderTransforms`). These are c4..c7
  and c12 cModelViewProj, c8..c11 and c13 cViewProj, and c58..c60 cModel[0].
  - A group is derived again only when its transforms changed. The shader API
    counts changes the way D3D9 flags them: every load counts, except
    LoadIdentity on an identity matrix.
  - A register the material wrote after its group's last derivation keeps the
    material's value. Compositor's `$textransform1` lives in c4..c5 this way.
- **The vertex record** carries skinned or world-space positions, normals and
  tangents, the vertex color (or the static color mesh, D3D9's COLOR1) and
  TEXCOORD0..2. Its tangent and TEXCOORD2 slots follow the pass's vertex format,
  as D3D9's declaration does.
- **Unbound samplers read (0, 0, 0, 1).** D3D9 sets no texture on a sampler the
  pass did not enable (`ApplyTextureEnable`), and the port reads that value there.
- **Partial routing where a native family already exists.** For
  VertexLitGeneric/UnlitGeneric (`VertexLitDrawsTextured`), skin
  (`SkinDrawsNative`) and LightmappedGeneric (`LightmappedDrawsNatively`), only
  the combos the native family cannot draw faithfully go to the port.
  - `-vklegacyvertexlit`, `-vklegacyskin` and `-vklegacylightmapped` send every
    combo to the port.
  - The cases for the combos the native families keep are in
    `quality/fixtures/legacy-shaders/forced/`.

Framework additions made along the way:

- `SetDepthFeatheringPixelShaderConstant` and `CBCMD_SET_DEPTH_FEATHERING_CONST`
  now behave as on D3D9. Both were no-ops.
- `PolyMode` works (Wireframe_DX9), through line fill mode when the device has
  `fillModeNonSolid`.
- `GetAmbientLightCubeLuminance` returns D3D9's value.
- `IMAGE_FORMAT_UVWQ8888` uploads as SNORM (the signed normalization cube).
- c0/c1 hold the math constants D3D9 writes at device creation.
- Refract_DX90 has a port. The merge keeps the native Refract path as the
  default (see below); `-vklegacyrefract` draws it through the port.

## Oracle

`tools/quality/legacy_shader_conformance.py` draws each case of
`quality/fixtures/legacy-shaders/*.vdf` through the real material system (the
`legacy` family of `material_pixel_conformance`). It records every pass
(`-vklegacycapture`), and `tools/quality/legacy_shader_oracle.py` replays each
pass on the D3D9 bytecode.

- **Reference bytecode.** By default the oracle compiles this tree's `.fxc` with
  the pinned FXC under Wine, one combo at a time (`--shaders retail` uses the
  shipped `.vcs`). The interpreter is `d3d9_shader_vm.py` and the texture
  sampler is `d3d9_texture.py`.
- **Replay.** Each pass goes through the vertex shader, D3D9 rasterization
  (integer pixel centers), the pixel shader, the alpha test, blending and the
  sRGB write. `dsx`/`dsy` are the quad differences over the triangle's plane.
- **Failures.** A case whose material no port drew fails.
- **Tolerance.** 2 of 255 by default.

Results at the end of this slice:

| Run | Cases | Result | Max error |
| --- | --- | --- | --- |
| all `*.vdf`, `--hdr none` (`qr/m11_none`) | 277 | pass | 1 |
| all `*.vdf`, `--hdr integer` (`qr/m11_integer`) | 277 | pass | 2 |
| `forced/vertexlitgeneric_textured_combos.vdf` with `-vklegacyvertexlit` | 6 | pass | 1 |
| `forced/skin_native_combos.vdf` with `-vklegacyskin` | 22 | pass | 1 |
| `forced/lightmappedgeneric_native_combos.vdf` with `-vklegacylightmapped` | 26 | pass | 1 |

Other checks:

- **`render.vulkan.legacy-constants`** (unit suite, 32 checks) covers:
  - register copies;
  - the matrix register contract, with rows-for-columns, fast-clip z and
    missing-row blocks rejected;
  - D3D9's commit, including a reload with the same value;
  - the program registry.
- **Python tests:** the oracle tests (15, including quad derivatives and a
  helper pixel), `test_shader_artifacts` (30) and `test_d3d9_shader_vm` all pass.
- **Existing native material pixel families** all pass. That is lightmap,
  exposure, skinning, modellight and bump in both HDR modes, plus portal, cable,
  sky, monitor, sprite, pbr-model, shadow and post.
- **GPU render suites:** world-pbr, model-pbr, compute, indirect-light.sdf,
  world-glass and zero-checks pass. indirect-switching and pbr-direct fail on a
  swapchain-semaphore validation message. That failure predates this work (the
  worktree base predates 41164e04, "per-image present semaphores").
- **Portal boots:** testchmb_a_00, testchmb_a_08, testchmb_a_15 and escape_02
  pass with no dropped draws. The escape_02 pipes and truss
  (VertexLitGeneric with an env map, static combo #132) used to be missing and
  now draw through the port. Skin env-map and light-warp combos draw through the
  port as well.
- **Style and architecture:** `regen_legacy_spv.py --check`, `stylelint
  --changed` (0 failures) and archlint (no findings in these files) all pass.

## Ports by material shader

- **Screen space and post:**
  - accumbuff4sample, accumbuff5sample, Bloom, ColorCorrection, Downsample,
    floatcombine(_autoexpose), floattoscreen(_vanilla), HSV,
    IntroScreenSpaceEffect;
  - FilmDust, FilmGrain, hsl_filmgrain passes 1 and 2;
  - Sample4x4 (all variants), sfm_integercombine, showz, DebugTextureView,
    DebugMRTTexture, color_projection;
  - warp, vr_distort_hud, vr_distort_texture, pyro_vision, Compositor, Bik;
  - screenspace_general's pixel shaders: constant_color, bloomadd,
    appchooser360movie, copy_fp_rt, lpreview_output, lpreview1, haloadd,
    haloadd1d, haloaddoutline, rendertargetblit;
  - HDRCombineTo16Bit and HDRSelectRange (first render target only);
  - MotionBlur, Modulate.
- **World and environment:**
  - LightmappedGeneric (the combos listed above), WorldTwoTextureBlend,
    DecalBaseTimesLightmapAlphaBlendSelfIllum, LightmappedReflective;
  - Water and WaterCheap, ShatteredGlass, WindowImposter, Cloud;
  - Sky and Sky_HDR (both compressed variants), TreeLeaf;
  - Portal, PortalStaticOverlay, VolumeClouds, Refract.
- **Models:**
  - VertexLitGeneric and UnlitGeneric (the combos listed above), with the bump
    and phong (skin) pairs;
  - VertexLitGeneric's blended passes: emissive scroll, flesh interior, weapon
    sheen, cloak;
  - Eyes, EyeRefract, EyeGlint, Teeth (plain, bump and flashlight);
  - Cloak, Core, VortWarp, Aftershock, ParticleSphere, ShadowModel, DepthWrite;
  - PBR (pbr_ps30).
- **Native paths:** Wireframe_DX9 and Eyeball draw UnlitGeneric's pair in line
  fill mode. WriteStencil_DX9 is vertex-only (WriteZ's native path).

## Not ported, or not verified

- **Flashlight passes.** No FLASHLIGHT or FLASHLIGHTSHADOWS combo is ever
  selected, because the backend reports no flashlight mode (the P7 track in the
  video-options record). The eyes and teeth flashlight pairs are ported but
  unverified.
- **DEPTHBLEND (depth feathering)** needs the frame's depth in destination alpha
  (WRITE_DEPTH_TO_DESTALPHA). Native does not write it
  (`ShouldWriteDepthToDestAlpha` is false), so feathering reads full depth. This
  affects the SpriteCard native family (still reported), VertexLitGeneric and
  ParticleSphere. It needs a scene-depth resolve, a render-target feature to
  build next on the existing scene capture.
- **SpriteCard ANIMBLEND on spline cards** is still reported. A concurrent
  session implemented it for sprite cards in the main tree.
- **Alpha to coverage** is render state, planned under R65.
- **Unreachable shaders:**
  - WorldVertexAlpha's fallback targets an unbuilt DX8 shader.
  - ParticleLitGeneric is dead code (no `.fxc`).
  - LIGHTING_PREVIEW 2 writes only its first render target.
- **Wrinkle maps** work at weight zero only. The flex stream's wrinkle weight
  has no slot in the record, and `SetFlexMesh` is a no-op.
- **Oracle limits:**
  - The pinned FXC (D3DX9 5.04.00.2904) miscompiles two shaders. One is
    pbr_ps30's parallax loop: its own `/Fc` listing swaps the step counter into
    the texture coordinate. That case is in
    `quality/fixtures/legacy-shaders/unverified/pbr_parallax.vdf`. The other is
    one skin RIMLIGHT combo with two lights, whose case keeps both lights facing
    the normals.
  - pbr_ps30 used `uint` for its light loop, which that compiler rejects, so
    PBR had no compiled shader at all. It is now `int`.
  - The oracle has no z clip and no depth test, so the case files avoid both.
  - Point-sampled cube maps are required: Vulkan filters cube maps seamlessly
    and D3D9 does not.

## Merge into subsystem-refactor (2026-09-25)

The ports (`15d60c4f`, based on `223f0ed8`) were merged onto `subsystem-refactor`
at `0934a720`. The branch had moved the native backend on since that base, so the
merge keeps every native path the branch had extended and gives the ports only
the passes it dropped or approximated.

- **Kept native by default** (`NativeKeepsLegacyPair`), each with a switch that
  sends it to its port:
  - `refract_ps20b` (`-vklegacyrefract`): the branch's Refract_DX90 path draws
    Portal 2's `$localrefract` and `$envmapsaturation`, which the port (built from
    this tree's `refract_ps2x.fxc`, with no LOCALREFRACT combo) does not. Refract
    variants the native path declines stay declined, as before.
  - `bloomadd_ps20b` (`-vklegacybloomadd`): the branch's native bloom add.
  - `sky_ps20b` (`-vklegacysky`): the port does not match D3D9 in integer HDR.
    The integer `sky` pixel family fails identically on the branch before the
    merge (`colored_sky` magenta), so this keeps that behavior unchanged. The
    compressed HDR sky pairs, which the branch did not draw, take their ports.
- **Restored native code the automatic merge removed:** refract's pixel-fog and
  tone-map classification, sampler 3/4 bindings, emit tangent frame and tint,
  fragment flags, frame-replay samplers, unsupported-material drop and
  draw-state recording.
- **The branch's mesh record** (`vulkan_mesh_layout.h`) keeps only the
  components a mesh's format declares, as D3D9's declaration does. The oracle's
  harness (`material_pixel_legacy.cpp`) now declares every component its quads
  write (`GetDynamicMeshEx`), as a model's vertex data carries them; before
  that, volume_clouds, aftershock and skin env-map cases read zero tangents.
- The refract, bloomadd and sky cases moved to `forced/`; run them with their
  switches.

Evidence at the merge (merged tree built in a private scratch worktree, same
`build-vk` profile; baseline = the branch at `6a85cfe4` built the same way):

| Check | Result |
| --- | --- |
| Oracle, default cases, `--hdr none` / `--hdr integer` | 267 / 267 pass each |
| Oracle, `forced/` files with their switches (refract, bloomadd, sky both HDR modes, skin, vertexlit, lightmapped) | all pass |
| Material pixel families, 14 x 2 HDR modes | all pass except integer `sky`, which fails identically on the baseline |
| `linux-native-vulkan-gpu` conformance profile | 15 / 15 |
| Headless `render.*` suites | 45 pass, 2 optional TSan lanes skipped |
| Python oracle, shader-artifact and D3D9 VM tests | 86 pass |
| stylelint `--changed`, archlint `check --all`, `regen_legacy_spv.py --check` | clean |

Portal boots, baseline against merged, with the harness settings and with
`run.conf`'s (`mat_queue_mode 2`, `mat_vk_emit_parallel 1`, the job graphs):

- testchmb_a_00, testchmb_a_08, testchmb_a_15, escape_02, gi_door,
  reflection_mirror_lamp and gi_probe_grid boot in both trees and both settings.
- Dropped material draws fall to 0 on every map except testchmb_a_15, which
  keeps `particle/warp1_warp` (SpriteCard DEPTHBLEND, open above); `dev/motion_blur`
  (about 340 a frame) now draws.
- The GI and PBR maps are pixel-identical to the baseline (mean difference 0.00,
  tone-map scale 1.0 in both).
- The Portal chambers differ systematically, and the merged build is
  deterministic (mean 0.04 between runs). Models whose skin and VertexLitGeneric
  env-map combos the native families drew without the env map now reflect it
  (the testchmb_a_08 elevator glass, the relaxation-vault glass, the portal gun),
  and auto-exposure follows the brighter scene (testchmb_a_08 tone-map scale
  0.89 to 1.05, testchmb_a_00 1.5 to 1.03). No D3D9 capture was taken for these
  frames (native-Vulkan focus); the ported combos are held to FXC bytecode by the
  oracle.

## Reproduce

```sh
python3 materialsystem/shaderapivulkan/shaders/regen_legacy_spv.py --check
WAFLOCK=.lock-waf-vk ./waf build -j 16
python3 tools/quality/legacy_shader_conformance.py --runtime <portal runtime> \
    --build build-vk --out qr/<new dir> [--hdr integer]
python3 tools/quality/legacy_shader_conformance.py --runtime <portal runtime> \
    --build build-vk --out qr/<new dir> \
    --cases quality/fixtures/legacy-shaders/forced/skin_native_combos.vdf \
    --extra-arg=-vklegacyskin
# likewise forced/refract.vdf (-vklegacyrefract), forced/bloomadd.vdf
# (-vklegacybloomadd) and forced/sky.vdf (-vklegacysky, both HDR modes)
python3 tools/quality/conformance.py check --suite render.vulkan.legacy-constants --out qr/<dir>
python3 -m unittest tools.quality.tests.test_legacy_shader_oracle \
    tools.quality.tests.test_shader_artifacts tools.quality.tests.test_d3d9_shader_vm
```
