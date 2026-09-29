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

## Revert and gated re-land (2026-09-25/26)

The merge below (`b5652908`) was reverted (`0559a463`) after the user played it:
in `./play` (testchmb_a_01, run.conf with `mat_queue_mode 2`, a 2880x1620
Wayland window) vault and window glass drew as opaque black diagonal bands, the
chamber sign and observation monitors broke, and the pause menu had no UI. The
headless spawn-view boots in the merge evidence did not show any of it.

The ports are back in the tree **off by default**, behind `-vklegacyports`
(`LegacyPortsEnabled`). Off, the backend routes and draws as before the ports:
no `legacy#` routes and the old skin route; Wireframe_DX9, DecalModulate_dx9,
WriteStencil_DX9 and DepthWrite are not drawn; PolyMode stays unimplemented;
`SetStandardVertexShaderConstants`, `SetDepthFeatheringPixelShaderConstant`,
the command-buffer depth-feathering constant and `GetAmbientLightCubeLuminance`
keep their old no-op behavior; UVWQ8888 uploads as UNORM. The vertex constant
file no longer seeds c0/c1 at construction (SetStandardVertexShaderConstants
writes them with the switch on). `legacy_shader_conformance.py` passes
`-vklegacyports` to the harness.

Evidence for the default: the `linux-native-vulkan-gpu` profile passes 15/15
(the re-land also fixed three suites missing the port sources and three with
them twice, which had left the host build unlinkable); the user played
`./play` with the ports off and confirmed it works.

**Root cause of the first merge's breakage** (found 2026-09-26 with the switch on,
headless, then RenderDoc pixel history on a band pixel): the band was the lab desk
model (`lab_desk05`, the native VertexLitGeneric path), drawn with a transform of
( 0, 1 / 2.2, 0, 0, 1, 2, ... ). `SetStandardVertexShaderConstants` wrote D3D9's
math constants into c0/c1 through `SetVertexShaderConstant`, which marks them
written, and `CommitDynamicVsConstants` reads a written c0..c3 as the native
backend's legacy model-view-projection alias when c4 is unwritten. Every native
draw on that fallback took the math constants as its transform: the black
diagonal bands, and plausibly the pause menu, sign and monitors. The fix stores
the constants (`StoreStandardVertexShaderConstants`, and the register file's
construction) without marking them written. The oracle never set up that
fallback, and the merge's spawn-view boots missed it.

Evidence after the fix (`build-ports`, private output of the shared checkout):

| Check | Result |
| --- | --- |
| testchmb_a_01 view set (8 yaws, 2 upward, pause menu), run.conf settings, switch on vs off | every view within 0.12 % of pixels (> 16/255); pause menu drawn |
| Switch off vs the earlier switch-off build | within 0.02 %; pause menu identical |
| Oracle, default cases, `--hdr none` | 267 / 267 |
| `linux-native-vulkan-gpu` profile | 15 / 15 |
| The user's `./play` with the switch off | confirmed working |

Open before the ports can default on: the user's `./play -vklegacyports` on
Wayland; a glass oracle (translucent env-mapped, refract and window glass must
show the scene behind them); the integer-HDR oracle run and the `forced/` files
after the fix; frame time with the ports.

## Default on (2026-09-26)

The ports are **on by default** by user decision, ahead of the open items above.
`LegacyPortsEnabled` now returns true unless `-novklegacyports` is passed, which
restores the switch-off behavior described above. `-vklegacyports` is still
accepted, so existing command lines and the oracle harness are unchanged.

Evidence (`build`, the shared native tree, default switch):

| Check | Result |
| --- | --- |
| Oracle, default cases, `--hdr none` | 267 / 267 |
| Oracle, default cases, `--hdr integer` | 267 / 267 (the open integer-HDR run) |
| Material pixel families, both HDR modes | 27 / 28; integer `sky` fails with magenta, identically with `-novklegacyports` (the existing failure) |
| `portal_boot.py` testchmb_a_01, headless, `mat_queue_mode 2`, Box3D | pass; window glass shows the room behind it |

The user then played `./play` on their Wayland session with the new default and
approved it: "glass works! approved" (2026-09-26).

### Glass oracle, `forced/` files and frame time (2026-09-26)

The `glass` family of `material_pixel_conformance`
(`unittests/shaderextensiontest/material_pixel_glass.cpp`) draws the glass
testchmb_a_01 uses over two walls through a perspective camera:
`glass/glasswindow_frosted` (LightmappedGeneric, translucent additive, env
mapped, on a real lightmap), `glass/glasswindow_refract01` (Refract, after the
engine's frame copy) and `models/props/box_dropper_tube` (VertexLitGeneric,
translucent additive, env mapped), plus an opaque control. The oracle
(`check_glass` in `tools/quality/material_pixel_conformance.py`) takes each
pane's transmission per channel from the two frames. Additive glass blends
into the sRGB frame in linear light, so it is measured on decoded values and
must be 1; Refract multiplies the copied frame by `$refracttint` on encoded
values; the control must pass nothing. Additive panes must also add light of
their own, so an undrawn pane does not pass as clear glass.

| Check | Result |
| --- | --- |
| `glass`, both HDR modes, ports on and `-novklegacyports` | pass; frosted 1.00, tube 1.00, refract (0.73, 0.81, 0.86) against the tint (0.72, 0.80, 0.85), control 0.00 |
| Oracle self-tests (`GlassTest`) | 12 / 12, including black, opaque and undrawn panes, an ignored tint, normal blending, a clear control, a missing wall and a fallback shader |
| `forced/` files with their switches, both HDR modes | 64 / 64 each; `sky` passes in integer HDR |
| `frame_pacing.py`, 6 interleaved rounds, `mat_queue_mode 2`, Box3D | warm median 4.04 ms on, 4.42 ms off (B/A 1.095); p99 6.39 vs 6.66 ms; rounds 1 and 4 on ran 5.7 ms (the host's slow mode) |

The first oracle draft measured additive glass on encoded values and failed
the frosted pane at 0.56; decoded, the same pixels give 1.00. A self-test keeps
that color-space choice load-bearing.

### View set at the new default (2026-09-26)

`tools/quality/legacy_ports_views.py` boots testchmb_a_01 headless three times
at run.conf's settings (1920x1080, `mat_queue_mode 2`, Box3D with shape
inertia, the job-graph and material settings): ports on, `-novklegacyports`,
and ports on again as the run-to-run noise measure. Each run captures eight
yaws and two upward views from the spawn point, then the pause menu. A view
passes when the fraction of pixels differing by more than 16/255 stays within
0.5 %, or three times its own noise; a comparison of two different views must
exceed 5 %, or the comparison could not see a change.

| View | On vs off | Noise |
| --- | --- | --- |
| yaw000 / yaw045 / yaw090 / yaw135 | 0.10 % / 0.01 % / 0.00 % / 0.01 % | 0.00-0.01 % |
| yaw180 / yaw225 / yaw270 / yaw315 | 0.01 % / 0.00 % / 0.00 % / 0.12 % | 0.00-0.01 % |
| up000 / up180 | 0.08 % / 0.00 % | 0.04 % / 0.00 % |
| pause menu | 0.00 % (drawn) | 0.00 % |
| Negative control (yaw000 on vs yaw090 off) | 65 % | |

The window glass in the yaw135 and yaw180 views shows the room behind it. With
the user's approval on Wayland, the glass oracle, the `forced/` files and the
frame time above, the default-on gate is met. The row stays `partial` for the
items under "Not ported, or not verified".

### Pixel fog on the native skin family (2026-09-28)

The user saw VertexLitGeneric `$phong` models in sp_a2_core (GLaDOS, her
cables) drawn without the chamber fog. The native skin family, which draws
most `skin_ps20b` combos, never applied FinalOutput's pixel fog: `skin.frag`
left it out, and `SnapshotPixelFog` did not list `skin_ps2*`, so its draws
carried no fog record. The `forced/skin_native_combos.vdf` fog cases passed
only because they run with `-vklegacyskin`, which judges the port, not the
native family.

- **Fix.** `SnapshotPixelFog` lists `skin_ps2*` (the PIXELFOGTYPE combo,
  `g_FogParams` c12, eye z in c11.z). The skin pipeline's vertex input takes
  the fog stream (`DrawFog`, vertex binding 1, locations 7..10, as the
  textured stage). `skin.vert` passes it on, and `skin.frag` applies
  `CalcPixelFogFactor`/`BlendPixelFog` after the tone-mapping scale and
  before the sRGB encode. Skin positions are already in world space, so
  `CommitPassFog` gives skin draws the identity world-z row.
- **Oracle.** `legacy_shader_conformance.py --native-against-port SWITCH`
  draws the cases with SWITCH (the port, judged against the bytecode), then
  without it (the native family). The native colour must equal the port's
  within the tolerance, and no port may draw a case in the native run
  (`legacy_shader_oracle.compare_native_family`). Alpha is not compared,
  because native passes do not write destination alpha. Six self-tests
  (`NativeFamilyTest`) cover an unfogged native family, the tolerance edge,
  a port drawing in the native run, and missing cases.

| Check | Result |
| --- | --- |
| `forced/skin_native_combos.vdf --native-against-port=-vklegacyskin`, both HDR modes | pass; 22 / 22 cases, maximum error 0 (the port run also passes the bytecode) |
| Negative control: the same cases with the `fog` blocks removed, native against the fogged port | `skin_native_fog` 41, `skin_native_fog_below` 43 levels; the other 20 cases 0 |
| `forced/vertexlitgeneric_textured_combos.vdf --native-against-port=-vklegacyvertexlit`, both HDR modes | pass |
| `forced/lightmappedgeneric_native_combos.vdf --native-against-port=-vklegacylightmapped` | fail, open: `lmg_native_ssbump_detail_ssbump_bump` 3-6 levels darker in the native family; the other 25 cases pass |
| Default run (all top-level `*.vdf`, `--hdr none`) | 258 of 273 cases pass; the 15 failures are all `flashlight.vdf` (flashlight is not ported) |
| `flex` and `pbr-model` pixel families (these share the skin vertex input) | pass |
| sp_a2_core, GLaDOS from two views, forced range fog, `mat_force_tonemap_scale 1`, native against `-vklegacyskin` | 0.001 % and 0.034 % of pixels beyond 16/255 (fog off: 0.031 %); fog on against fog off changes 91 % |

Open: the native skin family does not write the height-fog factor to
destination alpha (`WRITEWATERFOGTODESTALPHA`), and the frame copy's
depth-to-alpha does not rebuild it, so skin models under water fog are
missing from the water's dest-alpha fog.

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
- **DEPTHBLEND (depth feathering)**, done 2026-09-26. Native passes still do not
  write depth into destination alpha (`ShouldWriteDepthToDestAlpha` stays
  false); instead every frame copy (`CopyRenderTargetToTextureEx`) gets the
  source's projected z over the dest-alpha range written into its alpha from
  the copied depth (`RecordDepthToAlpha`, `shaders/depth_to_alpha.frag`, the
  draw projection's z column), which is what D3D9 PC's copies hold. The native
  SpriteCard stage now feathers the vertex alpha as `spritecard_ps2x` does
  (`kFragmentSpriteDepthBlend`, sampler 2 in set 3, c2.x in the modulation's
  alpha, the screen position divided per pixel); the VertexLitGeneric and
  ParticleSphere ports read the same alpha. `-novkdepthalpha` rolls back to a
  plain color copy. Evidence: the `softparticle` pixel family (cards 5, 25 and
  80 units in front of a wall and in front of a far wall, after the engine's
  `_rt_FullFrameDepth` copy) matches DepthFeathering with the copy's 8-bit
  alpha within 0.04 in both HDR modes, and fails with `-novkdepthalpha`; every
  other family passes as before (integer `sky` still fails, as it did).
- **SpriteCard ANIMBLEND on spline cards** is still reported. A concurrent
  session implemented it for sprite cards in the main tree.
- **Alpha to coverage** is render state, planned under R65.
- **Unreachable shaders:**
  - WorldVertexAlpha's fallback targets an unbuilt DX8 shader.
  - ParticleLitGeneric is dead code (no `.fxc`).
  - LIGHTING_PREVIEW 2 writes only its first render target.
- **Wrinkle maps**, done 2026-09-26. `SupportsStreamOffset()` is true, so
  studiorender draws delta-flexed groups statically and binds their flex stream
  (`CEmptyMesh::SetFlexMesh`, routed through `OnSetFlexMesh` for the queued
  material system). The stream has D3D9's stream-2 layout
  (`MeshFormatIsFlexStream`: position delta, wrinkle, normal delta); emit adds
  the deltas to the record before skinning (the normal delta to the tangent
  too, as `ApplyMorph` does), keeps SEAMLESS's object position raw, puts the
  wrinkle weight in a model record's slot 21 for `skin_vs20`, and keys the
  geometry-reuse cache on the flex stream. Evidence: the `flex` pixel family
  (wrinkle weights 0, ±1, 0.5, -0.25, a position delta and an unbound draw)
  matches skin_ps20b's blend and the moved face in both HDR modes, with 8
  seeded-defect self-tests; the queued mesh contract passes 158 checks;
  every other family and the oracle pass as before.
- **Oracle limits:**
  - The pinned FXC (D3DX9 5.04.00.2904) miscompiled two shaders. One was
    pbr_ps30's parallax loop: its own `/Fc` listing swapped the step counter
    into the texture coordinate, so the shipped D3D9 bytecode was wrong too.
    Fixed 2026-09-26 in the HLSL (`parallaxCorrect` runs a fixed trip count
    with a found flag instead of exiting by writing the counter); the case,
    `quality/fixtures/legacy-shaders/pbr_parallax.vdf`, fails on the old loop
    (one displaced pixel 4 levels off) and passes on the new one in both HDR
    modes, and is in the default run. The other is one skin RIMLIGHT combo
    with two lights, whose case keeps both lights facing the normals.
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
# The native families against their ports (skin, vertexlit, lightmapped):
python3 tools/quality/legacy_shader_conformance.py --runtime <portal runtime> \
    --build build-vk --out qr/<new dir> [--hdr integer] \
    --cases quality/fixtures/legacy-shaders/forced/skin_native_combos.vdf \
    --native-against-port=-vklegacyskin
python3 tools/quality/conformance.py check --suite render.vulkan.legacy-constants --out qr/<dir>
python3 -m unittest tools.quality.tests.test_legacy_shader_oracle \
    tools.quality.tests.test_shader_artifacts tools.quality.tests.test_d3d9_shader_vm
```
