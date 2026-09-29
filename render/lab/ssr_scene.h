//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's analytic SSR scenes (RFC 0016 K11,
//			render.ssr.v1): rectangles with a lit colour that is a smooth
//			function of the point, ray-cast per pixel into the pass's inputs,
//			with each pixel's true mirror reflection (the nearest rectangle
//			along the reflected ray) for judging a trace. Private to
//			render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_SSR_SCENE_H
#define RENDER_LAB_SSR_SCENE_H

#include "ssr_reference.h"

#include "render/math/vector.h"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace render::lab
{

struct SsrQuad
{
	math::float3 corner; // one corner; the rectangle is corner + a U + b V, a, b in [0, 1]
	math::float3 u;
	math::float3 v;
	math::float3 normal; // unit, facing the camera side
	float roughness = 1.0f;
	std::array<float, 3> weight = { 1.0f, 1.0f, 1.0f };      // specular weight w
	std::array<float, 3> iblRadiance = { 0.0f, 0.0f, 0.0f }; // lit holds w times it
	// The light leaving the rectangle towards the eye (rgb), before the image
	// specular.
	std::function<std::array<float, 3>( const math::float3 & )> radiance;
};

struct SsrScene
{
	std::uint32_t width = 256;
	std::uint32_t height = 192;
	math::float3 eye;
	math::float3 target;
	float verticalFovDegrees = 60.0f;
	float nearZ = 1.0f;
	float farZ = 8192.0f;
	std::vector<SsrQuad> quads;
};

// Per pixel: the rectangle seen (-1: none) and where its true reflection
// lands (the nearest rectangle along the mirror ray from the pixel's
// point): its index (-1: none), the point, and its screen position.
struct SsrTruth
{
	int quad = -1;
	int reflectedQuad = -1;
	math::float3 reflectedPoint;
	double reflectedX = 0.0;
	double reflectedY = 0.0;
	bool reflectedOnScreen = false;
	// The reflected point is what the camera sees there: the same rectangle
	// over a 5 x 5 pixel neighbourhood, at its depth.
	bool reflectedVisible = false;
};

struct SsrSceneImages
{
	SsrReferenceInputs inputs;
	std::vector<SsrTruth> truth;
	std::vector<math::float3> points; // each pixel's surface point
};

SsrSceneImages RayCastScene( const SsrScene &scene );

// The rectangle the camera sees through a screen point (pixels from the top
// left) and where; -1 when none.
int CastCamera( const SsrScene &scene, const SsrReferenceInputs &inputs, double sx, double sy,
    math::float3 &point );
// The same with the inverse of inputs.toClip given.
int CastCamera( const SsrScene &scene, const SsrReferenceInputs &inputs,
    const math::float4x4 &fromClip, double sx, double sy, math::float3 &point );

} // namespace render::lab

#endif // RENDER_LAB_SSR_SCENE_H
