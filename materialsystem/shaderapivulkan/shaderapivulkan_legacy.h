//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shader API's half of the legacy shader ports
//          (vulkan_legacy_programs.h): the constants block a pass's port reads,
//          built from the registers the material's shader wrote and the
//          registers D3D9's shader API derives from the transforms
//          (CShaderAPIDx8::SetVertexShaderViewProj and
//          SetVertexShaderModelViewProjAndModelView).
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_LEGACY_H
#define SHADERAPIVULKAN_LEGACY_H

#include "vulkan_legacy_programs.h"

#include <cstdint>
#include <cstdio>

namespace render_vulkan
{

// The inputs of one pass's constants. Matrices are the shader API's MATERIAL_*
// transforms as it stores them (row vectors: p' = p * M, element [row * 4 + col]).
struct LegacyPassInputs
{
	const float ( *ps )[4] = nullptr; // pixel shader c0..c31
	const float ( *vs )[4] = nullptr; // vertex shader c0..c255
	const int *psBools = nullptr;     // b0..b15
	const int *vsBools = nullptr;     // b0..b15
	int psStatic = 0;
	int psDynamic = 0;
	int vsStatic = 0;
	int vsDynamic = 0;
	unsigned srgbSamplers = 0; // D3D9 samplers read as sRGB (bit per sampler)
	const float *model = nullptr;
	const float *view = nullptr;
	const float *projection = nullptr;     // MATERIAL_PROJECTION
	const float *drawProjection = nullptr; // with the fast-clip plane, when enabled
	// With a commit: when the material last wrote each of c0..c255 (0 never),
	// and the latest such serial, from one counter that only increases.
	const uint64_t *vsWriteSerial = nullptr;
	uint64_t writeSerial = 0;
	// With a commit: the MODEL, VIEW and PROJECTION change counts (D3D9's
	// transform change flags: a matrix loaded again counts as changed even
	// when its value is the same).
	uint64_t modelGeneration = 0;
	uint64_t viewGeneration = 0;
	uint64_t projectionGeneration = 0;
};

// When the derived registers were last written, as D3D9 tracks them
// (CShaderAPIDx8::CommitVertexShaderTransforms): at a draw it rewrites
// c8..c11 and c13 when the view or projection changed since its last commit,
// c4..c7 and c12 when the model, view or projection changed, and c58..c60 when
// the model changed (a change count or the value, which covers the fast-clip
// projection). Otherwise the registers hold whatever was last written,
// which can be the material's own constants (Compositor's $textransform1 in
// c4..c5, which it writes after the transforms were committed).
struct LegacyTransformCommit
{
	bool valid = false;
	float model[16] = {};
	float view[16] = {};
	float projection[16] = {};
	float drawProjection[16] = {};
	uint64_t modelGeneration = 0;
	uint64_t viewGeneration = 0;
	uint64_t projectionGeneration = 0;
	// The write serial each group was derived at.
	uint64_t viewProjAt = 0;
	uint64_t modelViewProjAt = 0;
	uint64_t modelAt = 0;
};

// Fills `out` (except target, which the device fills per draw): the registers
// as written, then c4..c7 cModelViewProj, c8..c11 cViewProj, c12 and c13 the
// z columns of both without the fast-clip plane, and c58..c60 cModel[0]. With
// `commit`, as D3D9 commits them: a group is derived again only when its
// transforms changed since the commit (which it updates), and a register the
// material wrote after its group's derivation keeps the material's value.
void BuildLegacyConstants(
    const LegacyPassInputs &in, LegacyConstants *out, LegacyTransformCommit *commit = nullptr );

// One pass drawn by a legacy port, as -vklegacycapture <file> records it for the
// bytecode oracle (tools/quality/legacy_shader_oracle.py): everything the D3D9
// shaders read, and the fixed-function state around them. The raster values
// are the Vulkan enums the pass's D3D9 state became.
struct LegacyCaptureRecord
{
	const char *material = "";
	const char *pixelShader = "";
	const char *vertexShader = "";
	const LegacyConstants *constants = nullptr;
	const float ( *vs )[4] = nullptr;    // c0..c255 as the shader API holds them
	const int ( *vsInts )[4] = nullptr;  // i0..i15
	const int ( *psInts )[4] = nullptr;  // i0..i15
	unsigned long long vertexFormat = 0; // VertexFormat_t (VERTEX_* flags)
	const char *textures[16] = {};       // the texture bound to each sampler, or null
	float alphaRef = -1.0f;              // < 0: no alpha test
	bool alphaGreater = false;
	bool srgbWrite = false;
	bool blend = false;
	int srcBlend = 0; // VkBlendFactor
	int dstBlend = 0;
	bool colorWrite = true;
	bool alphaWrite = false;
	int cullMode = 0; // VkCullModeFlags
	// The MATERIAL_* transforms (row vectors) and the drawn geometry: a JSON
	// fragment of "primitive", "indices" and "vertices" members.
	const float *model = nullptr;
	const float *view = nullptr;
	const float *projection = nullptr;
	const float *drawProjection = nullptr;
	const char *geometry = "";
};
void WriteLegacyCapture( FILE *out, const LegacyCaptureRecord &record );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_LEGACY_H
