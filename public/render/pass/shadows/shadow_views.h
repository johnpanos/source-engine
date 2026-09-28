//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.shadows shadow views (RFC 0016 K7, render.shadows.v1):
//			the view and projection a spot light, a flashlight or a cascade of
//			the sun's directional light renders its depth with. Matrices follow
//			render.math (clip depth 0 to 1, clip Y up, view looking down -Z).
//
//			Cascades split the camera's view up to a shadow distance with the
//			practical split scheme (a blend, by lambda, of logarithmic and
//			uniform splits). Each cascade is an orthographic box around the
//			bounding sphere of its slice of the view frustum. The sphere's
//			radius depends only on the projection and the splits, so the
//			cascade's texel size does not change as the camera turns, and the
//			box is moved in whole texels of a lattice fixed in the world, so
//			shadow edges do not crawl as the camera moves.
//
//=============================================================================//

#ifndef RENDER_PASS_SHADOWS_SHADOW_VIEWS_H
#define RENDER_PASS_SHADOWS_SHADOW_VIEWS_H

#include "foundation/expected.h"
#include "render/math/bounds.h"
#include "render/math/matrix.h"
#include "render/math/vector.h"

#include <array>
#include <cstdint>

namespace render::pass::shadows
{

enum class ShadowViewError : std::uint8_t
{
	kInvalidLight,      // non-finite values, a zero direction, or up along forward
	kConeTooWide,       // a spot wider than kMaxSpotHalfAngle (needs a cube shadow)
	kInvalidDepthRange, // near not above 0 or far not above near
	kInvalidCascades,   // count, lambda, resolution, field of view or distance out of range
};

// A spot's shadow frustum is a square pyramid around its cone; beyond this
// half-angle the pyramid degenerates.
inline constexpr float kMaxSpotHalfAngle = 1.4835298f; // 85 degrees

struct ShadowView
{
	math::float4x4 view;
	math::float4x4 projection;
	math::float4x4 viewProjection;
	float nearZ = 0.0f;
	float farZ = 0.0f;
};

struct SpotShadowDesc
{
	math::float3 position;
	math::float3 direction; // the cone's axis
	float outerCos = 1.0f;  // cosine of the cone's outer half-angle
	float nearZ = 1.0f;
	float range = 0.0f; // the far plane
};

[[nodiscard]] foundation::Expected<ShadowView, ShadowViewError> BuildSpotShadowView(
    const SpotShadowDesc &desc );

// A flashlight (a projected texture): its own orientation and horizontal and
// vertical fields of view, as the legacy FlashlightState_t carries them. Its
// up direction is the shadow map's up (clip +Y, texture row 0 side).
struct FlashlightShadowDesc
{
	math::float3 position;
	math::float3 forward;
	math::float3 up;
	float horizontalFovRadians = 0.0f;
	float verticalFovRadians = 0.0f;
	float nearZ = 0.0f;
	float farZ = 0.0f;
};

[[nodiscard]] foundation::Expected<ShadowView, ShadowViewError> BuildFlashlightShadowView(
    const FlashlightShadowDesc &desc );

inline constexpr std::uint32_t kMaxCascades = 4;

// Split distances 0..count: split i = lambda * near * ( far / near )^( i / count )
// + ( 1 - lambda ) * ( near + ( far - near ) * i / count ); split 0 is near
// and split count is far. Entries past count are far.
std::array<float, kMaxCascades + 1> PracticalSplits(
    float nearZ, float farZ, std::uint32_t count, float lambda );

struct CascadeDesc
{
	math::float4x4 cameraView; // world to view of the camera the cascades cover
	float verticalFovRadians = 0.0f;
	float aspect = 1.0f;
	float nearZ = 0.0f;
	float shadowDistance = 0.0f; // the view distance the last cascade reaches
	float lambda = 0.8f;
	std::uint32_t cascadeCount = kMaxCascades;
	std::uint32_t resolution = 1024; // texels across each cascade's viewport
	math::float3 lightDirection;     // the direction the sun's light travels
	// How far toward the light, beyond the cascade's sphere, casters are
	// kept in depth range.
	float casterDistance = 0.0f;
};

struct Cascade
{
	float splitNear = 0.0f;
	float splitFar = 0.0f;
	math::Sphere bounds;    // world sphere around the split's slice of the view frustum
	float texelSize = 0.0f; // world units per texel
	ShadowView view;        // orthographic, texel-snapped
};

struct CascadeSet
{
	std::uint32_t count = 0;
	std::array<Cascade, kMaxCascades> cascades;
};

[[nodiscard]] foundation::Expected<CascadeSet, ShadowViewError> BuildCascades(
    const CascadeDesc &desc );

} // namespace render::pass::shadows

#endif // RENDER_PASS_SHADOWS_SHADOW_VIEWS_H
