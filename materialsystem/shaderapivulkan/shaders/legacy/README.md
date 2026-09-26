# Legacy shader ports

GLSL ports of `materialsystem/stdshaders` (stdshader_dx9) shader pairs that the
native Vulkan backend draws. A pass whose exact pixel and vertex shader names
have a port draws through it; any other pass is declined by name (the census
and `NoteDroppedMaterial` report it).

## How a pass reaches a port

1. The material's shader sets its shadow state (`SetPixelShader(name, static)`,
   `SetVertexShader(name, static)`, samplers, blend, sRGB). `TakeSnapshot`
   routes the pair to `legacy#<program>#<ps static>#<vs static>` when
   `FindLegacyProgram` knows it (`vulkan_legacy_programs.h`).
2. At draw time the shader API builds the pass's constants block
   (`shaderapivulkan_legacy.cpp`): pixel shader `c0..c31`, vertex shader
   `c0..c63` and `c217..c224`, the combo indices, the boolean constants and `i0`.
   It derives the registers D3D9's shader API derives: `c4..c7` cModelViewProj,
   `c8..c11` cViewProj, `c12`/`c13` their non-fast-clip z rows, `c58..c60`
   cModel[0], `c21..c26` the ambient cube, `c27..c46` cLightInfo, `b0..b3` and
   `i0` the enabled lights. `c0`/`c1` hold the math constants D3D9 writes at
   device creation. Like D3D9 (`CommitVertexShaderTransforms`), a derived group
   is written again only when its transforms changed; a register the material
   wrote after that keeps the material's value (Compositor's `$textransform1`
   in `c4..c5`).
3. The vertex record (`vulkan_legacy_programs.h`) carries world-space positions
   (skinned, or through the MODEL matrix), normals and tangents rotated the same
   way (not normalized), the raw vertex color and alpha (or the static-prop
   color mesh, D3D9's COLOR1, when the format has no vertex color),
   TEXCOORD0..2. Which slots hold the tangents and TEXCOORD2 follows the pass's
   vertex format, as D3D9's declaration does (`LegacyBrushTangents()`,
   `LegacyTexCoord2()`, `LegacyTangentT()` in `legacy_vs.glsl`).
4. `vulkan_legacy_pipeline.cpp` binds the port's samplers (set 0, binding N
   the Nth entry of `samplers=`; a set per draw from a per-frame pool), the
   constants (set 1) and the push block, and builds a pipeline per raster
   state. Two sets fit every device (MoltenVK binds at most eight).

## Writing a port

1. **Find the pair.** Read the shader's `.cpp` (and `*_helper.cpp`). Native
   reports DX level 95: `SupportsPixelShaders_2_b()` and
   `SupportsShaderModel_3_0()` are true, `NeedsShaderSRGBConversion()` is false
   (so `CONVERT_TO_SRGB` is 0), HDR is `none` or `integer`. Port the pair(s) the
   `.cpp` selects under those caps (usually `*_ps20b`, or `*_ps30`/`*_vs30` when
   the shader prefers shader model 3).
2. **Read the shipped code.** The strides of every combo are in
   `materialsystem/stdshaders/fxctmp9/<name>.inc` (`GetIndex()`). The compiled
   code is the reference, not the HLSL: disassemble a combo with
   `python3 tools/quality/source_vcs.py <name> --static <index> --dynamic <index> --disasm`.
   fxc folds and truncates (for example `mul( float4 uv, (float2x4)m )` is
   `u * m[0] + v * m[1]`, and a division by a folded zero disappears); port what
   it compiled.
3. **Write the stages.** `shaders/legacy/<pixel shader>.frag`, and a
   `<vertex shader>.vert` unless an existing one is the same vertex shader.
   Conventions:
   - `#include "legacy_ps.glsl"` / `"legacy_vs.glsl"`.
   - Varyings by semantic: `TEXCOORDn` at location `n`, `COLORn` at `8 + n`, so
     any vertex port links with any pixel port of the same interface.
   - Registers under their HLSL names: `#define g_FogParams PS_C( PSREG_FOG_PARAMS )`,
     `#define cBaseTextureTransform_0 VS_C( 48 )`.
   - Combos with their `.inc` strides: `STATIC_PS_COMBO( stride, count )`,
     `DYNAMIC_PS_COMBO`, `STATIC_VS_COMBO`, `DYNAMIC_VS_COMBO`.
   - Samplers: `layout( set = 0, binding = N ) uniform sampler2D|samplerCube|sampler3D`,
     N being the sampler's position in the `samplers=` list; read them with
     `tex2D( N, sampler, uv )`, `texCUBE`, `tex3D`, `tex2Dproj`, `tex2Dlod`,
     which decode sRGB when the hardware view could not.
   - The vertex stage writes `gl_Position` through `LegacyProject( worldPos )`
     (cViewProj and the user clip planes); positions are already in world space.
   - The pixel stage ends with `LegacyWrite( FinalOutput( ... ) )` (tone map,
     fog, then D3D9's alpha test and the sRGB write).
   - Shared helpers (one owner each; extend them rather than copying):
     `legacy_vs_lighting.glsl` (common_vs_fxc.h vertex lighting, including
     GetVertexAttenForLight), `legacy_ps_lighting.glsl`
     (common_vertexlitgeneric_dx9.h pixel lighting, Fresnel, tangent-space
     helpers, tex1D), `legacy_color.glsl` (HlslPow, scalar gamma, sRGB curves,
     HSL), `legacy_bumpbasis.glsl`, `legacy_combine.glsl` (detail blend modes,
     depth feathering), `legacy_screen_vs.glsl`, `legacy_vs_bumped_model.glsl`,
     `legacy_blended_pass_vs.glsl`, `legacy_vortwarp_vs.glsl`,
     `legacy_flashlight*.glsl`. `GammaToLinear`/`LinearToGamma` (vec3) are in
     `legacy_common.glsl`; `LegacyTangentSign()` is TANGENT's w (1 for the
     brush streams, as D3D9 binds a three-component TANGENTS).
4. **Register it** with an `@legacy` line in the pixel stage:

       // @legacy program=<name> ps=<pixel shader> vs=<vertex shader> vert=<stage>
       //         samplers=0:2d,1:2d,2:cube [flags=object_position]

   `flags`: `object_position` passes POSITION untransformed (screen-space
   vertex shaders that output it as is); `object_position_extra` adds the
   untransformed POSITION in `inExtra.xyz` (vertex shaders that read `v.vPos`
   besides the skinned position, e.g. seamless mapping), in place of TEXCOORD2.
   A D3D9 sampler may appear twice with different types (`1:cube,1:2d`): the
   slot whose type does not match the bound texture reads white.

   When a native family already draws some combos of the same pair, route only
   the combos it cannot draw to the port in `SnapshotShaderRoute`
   (`shaderapivulkan.cpp`), with a `-vklegacy<name>` switch that sends every
   combo to the port; cases for the native combos go in `forced/` and run with
   `--extra-arg=-vklegacy<name>` (see `VertexLitDrawsTextured`).
5. **Regenerate and build.**
   `python3 materialsystem/shaderapivulkan/shaders/regen_legacy_spv.py` writes
   `legacy_spv.h` (SPIR-V and the program table); `--check` fails on a stale
   header. Then build the tree.
6. **Prove it.** Add `quality/fixtures/legacy-shaders/<shader>.vdf` (the case
   format is documented in `unittests/shaderextensiontest/material_pixel_legacy.cpp`)
   with cases that exercise each combo and input the port reads, and run

       python3 tools/quality/legacy_shader_conformance.py --runtime <portal runtime> \
           --build <tree> --out <dir> --cases quality/fixtures/legacy-shaders/<shader>.vdf

   It draws every case through the material system, records each pass
   (`-vklegacycapture`), replays the pass on bytecode compiled from this tree's
   `.fxc` with the pinned FXC (`--shaders retail`: the shipped `.vcs`;
   `tools/quality/legacy_shader_oracle.py`) and compares pixels. A case whose
   material no port drew fails, so a declined pass cannot pass. Cases the
   default run cannot judge go in `forced/` (combos a native family keeps; run
   with the family's `-vklegacy<name>`) or `unverified/` (with the reason, e.g.
   a reference compiler miscompile).
