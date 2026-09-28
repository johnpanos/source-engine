//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.math frustum tests (RFC 0016). Planes are extracted from a
//			view-projection matrix under the device conventions (clip depth 0
//			to 1), point inward and are normalized.
//
//			The tests are conservative: an Aabb or Sphere that intersects the
//			frustum is never reported outside. A volume near a frustum corner
//			may be reported inside although it is not; culling tolerates that.
//
//=============================================================================//

#ifndef RENDER_MATH_FRUSTUM_H
#define RENDER_MATH_FRUSTUM_H

#include "render/math/bounds.h"
#include "render/math/matrix.h"

namespace render::math
{

// ax + by + cz + d >= 0 is inside.
struct Plane
{
	float3 normal;
	float d = 0.0f;

	constexpr float Distance( const float3 &p ) const { return Dot( normal, p ) + d; }
};

enum class FrustumPlane : int
{
	kLeft,
	kRight,
	kBottom,
	kTop,
	kNear,
	kFar,
	kCount
};

struct Frustum
{
	Plane planes[static_cast<int>( FrustumPlane::kCount )];
};

Frustum ExtractFrustum( const float4x4 &viewProjection );
bool Intersects( const Frustum &frustum, const Aabb &box );
bool Intersects( const Frustum &frustum, const Sphere &sphere );
bool Contains( const Frustum &frustum, const float3 &point );

} // namespace render::math

#endif // RENDER_MATH_FRUSTUM_H
