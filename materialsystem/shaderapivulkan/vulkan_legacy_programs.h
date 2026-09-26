//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The registry of legacy shader ports: Source D3D9 shader pairs
//          (stdshader_dx9's pixel and vertex shaders) that the native Vulkan
//          backend draws through GLSL ports of their HLSL (shaders/legacy/).
//
//          Every port shares one pipeline layout and one set of inputs, so a
//          port is data: its SPIR-V stages, the D3D9 samplers it reads (one
//          descriptor set each) and how the shader API prepares its vertices.
//          The ports read the registers the material's shader wrote exactly as
//          the D3D9 shaders do, through two descriptor sets on every device:
//
//            set 0     the port's samplers, binding N the Nth LegacySamplerSlot
//                      (a set per draw, from a per-frame pool)
//            set 1     LegacyConstants: pixel shader c0..c31, vertex shader
//                      c0..c63 and c217..c224, the pass's combo indices and
//                      boolean constants (shaders/legacy/legacy_common.glsl)
//            push      the skin push block (viewProj, alpha test, the samplers
//                      the shader must decode from sRGB itself, the user clip
//                      planes)
//
//          A pass is drawn by a port only when its exact pixel and vertex
//          shader names are registered here; any other pass keeps being
//          declined by name.
//
//===========================================================================//

#ifndef VULKAN_LEGACY_PROGRAMS_H
#define VULKAN_LEGACY_PROGRAMS_H

#include <cstddef>
#include <cstdint>

namespace render_vulkan
{

enum
{
	// The samplers a port can read (bindings of set 0); the constants are set 1.
	// Sixteen is Vulkan's minimum maxPerStageDescriptorSamplers (skin_ps20b
	// reads twelve).
	kLegacySamplerSlots = 16,
	kLegacySamplerSet = 0,
	kLegacyConstantsSet = 1,
	// The vertex shader registers the constants carry: c0..c63, then c217..c224
	// (VERTEX_SHADER_SHADER_SPECIFIC_CONST_13..19 and _12).
	kLegacyVsLowRegisters = 64,
	kLegacyVsHighFirst = 217,
	kLegacyVsHighRegisters = 8,
	kLegacyVsRegisters = kLegacyVsLowRegisters + kLegacyVsHighRegisters,
};

// The image type a port declares for a sampler; an unbound or mismatched
// texture binds the backend's white image of that type.
enum LegacySamplerDim : uint8_t
{
	kLegacySampler2D = 0,
	kLegacySamplerCube,
	kLegacySamplerVolume,
};

struct LegacySamplerSlot
{
	int8_t sampler = -1; // the D3D9 sampler (s0..s15); -1 leaves the set unused
	LegacySamplerDim dim = kLegacySampler2D;
};

// How the shader API fills the vertex record for a port
// (CVulkanContext::kDynVertexFloats floats; shaders/legacy/legacy_vertex.glsl):
//   0..2   world position (skinned, or through the MODEL matrix), or with
//          kLegacyObjectPosition the POSITION stream itself
//   3..5   vertex color RGB (COLOR0); when the pass's format has no
//          VERTEX_COLOR, the static-prop color mesh's baked lighting, which
//          D3D9 binds as COLOR1 (the vertex shaders' vSpecular)
//   6..7   TEXCOORD0      8..9  TEXCOORD1
//   10..12 world normal (not normalized)
//   13..15 world tangent S: the TANGENTS stream when the pass's vertex format
//          has VERTEX_TANGENT_S (brushes), else the TANGENT (user data) stream,
//          as D3D9's declaration binds TANGENT
//   16     the TANGENT w sign; with VERTEX_TANGENT_S, TEXCOORD2.x
//   17     vertex color alpha
//   18..20 world tangent T (TANGENTT) with VERTEX_TANGENT_S, else TEXCOORD2.xyz
//          (z 0 unless the mesh's TEXCOORD2 has three components);
//          with kLegacyObjectPositionExtra the POSITION stream as the mesh
//          holds it (for vertex shaders that read v.vPos after skinning it)
//   21     TEXCOORD2.y with VERTEX_TANGENT_S, else 0
enum LegacyVertexFlags : uint32_t
{
	// The POSITION stream as the mesh holds it (no skinning, no MODEL matrix),
	// for vertex shaders that output it untransformed (screen-space passes).
	kLegacyObjectPosition = 1u << 0,
	// The POSITION stream as the mesh holds it in 18..20 (see the record).
	kLegacyObjectPositionExtra = 1u << 1,
};

// The per-draw constants block (std140 in the shaders).
struct LegacyConstants
{
	float ps[32][4];
	float vs[kLegacyVsRegisters][4];
	// Pixel shader static and dynamic combo index, vertex shader static and
	// dynamic combo index, as SetPixelShader/SetVertexShader and
	// SetPixelShaderIndex/SetVertexShaderIndex passed them.
	int32_t combos[4];
	// Pixel shader boolean constants b0..b15 as bits, the vertex shader's, the
	// D3D9 samplers read as sRGB (bits 0..15) and the samplers the pass enabled
	// (bits 16..31; the others read ( 0, 0, 0, 1 ), as D3D9 reads a sampler
	// with no texture set), and the pass's vertex format (VERTEX_* flags, low 32
	// bits), which decides the record's tangent and TEXCOORD2 slots.
	int32_t bools[4];
	// The vertex shader's integer constant i0 (the light loop: count, 0, 1, 0).
	int32_t vsLoop[4];
	// Render target width, height and their reciprocals (for VPOS).
	float target[4];
};

struct LegacyProgram
{
	const char *name;         // the port's name (diagnostics, pipeline store)
	const char *pixelShader;  // the D3D9 pixel shader file, exact (case-insensitive)
	const char *vertexShader; // the D3D9 vertex shader file, exact (case-insensitive)
	const uint32_t *vertSpv;
	size_t vertBytes;
	const uint32_t *fragSpv;
	size_t fragBytes;
	LegacySamplerSlot samplers[kLegacySamplerSlots];
	uint32_t vertexFlags;
};

// The port for a pass's shader pair, or -1 when none is registered.
int FindLegacyProgram( const char *pixelShader, const char *vertexShader );
int LegacyProgramCount();
const LegacyProgram &GetLegacyProgram( int id );
// Whether the port reads D3D9 sampler `sampler`.
bool LegacyProgramReadsSampler( const LegacyProgram &program, int sampler );

} // namespace render_vulkan

#endif // VULKAN_LEGACY_PROGRAMS_H
