//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's reference for render.pass.ssr (RFC 0016 K11,
//			render.ssr.v1): the pass's definition
//			(public/render/pass/ssr/ssr.h) evaluated on the CPU in double
//			precision, walking every texel the reflected ray crosses (no depth
//			pyramid), so it shares no code or acceleration with the GPU trace.
//			Private to render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_SSR_REFERENCE_H
#define RENDER_LAB_SSR_REFERENCE_H

#include "render/math/matrix.h"
#include "render/pass/ssr/ssr.h"

#include <cstdint>
#include <vector>

namespace render::lab
{

// A view's SSR inputs as the pass reads them, row 0 at the top. Each image
// holds width x height texels of four floats (the depth one float).
struct SsrReferenceInputs
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	math::float4x4 toClip;              // world to clip
	float eye[3] = { 0, 0, 0 };         // world position
	std::vector<float> depth;           // 0 near .. 1 far
	std::vector<float> normalRoughness; // octahedral normal xy, roughness
	std::vector<float> specularWeight;
	std::vector<float> imageSpecular;
	std::vector<float> lit;
};

struct SsrReferencePixel
{
	bool traced = false; // a surface under the cutoff: the ray was walked
	bool hit = false;
	double hitX = 0.0; // the hit's screen position, pixels from the top left
	double hitY = 0.0;
	std::uint32_t hitTexelX = 0; // the depth texel hit
	std::uint32_t hitTexelY = 0;
	double behind = 0.0; // view-space distance behind the surface at the hit
	double confidence = 0.0;
	double mip = 0.0;
	std::uint32_t steps = 0;          // texels walked
	float reflected[3] = { 0, 0, 0 }; // the lit pyramid at the hit
	float out[4] = { 0, 0, 0, 0 };    // the composited pixel
};

// The pass's result per pixel (row 0 at the top).
std::vector<SsrReferencePixel> ReferenceSsr(
    const SsrReferenceInputs &inputs, const pass::ssr::SsrParams &params );

// The lit pyramid the reference samples: mip k is a 2 x 2 box of mip k - 1,
// sizes halved and at least 1; level 0 is `lit`. Each level's RGBA texels.
struct SsrPyramidLevel
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<double> rgba;
};
std::vector<SsrPyramidLevel> ReferencePyramid(
    const SsrReferenceInputs &inputs, std::uint32_t maxMip );

} // namespace render::lab

#endif // RENDER_LAB_SSR_REFERENCE_H
