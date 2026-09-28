//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.math bounding volumes (RFC 0016).
//
//=============================================================================//

#ifndef RENDER_MATH_BOUNDS_H
#define RENDER_MATH_BOUNDS_H

#include "render/math/matrix.h"
#include "render/math/vector.h"

namespace render::math
{

// An axis-aligned box. A default box is empty (min above max) and contains
// nothing; growing it by a point makes it that point.
struct Aabb
{
	float3 min = { 1.0f, 1.0f, 1.0f };
	float3 max = { -1.0f, -1.0f, -1.0f };

	constexpr bool IsEmpty() const { return min.x > max.x || min.y > max.y || min.z > max.z; }
	constexpr float3 Center() const { return ( min + max ) * 0.5f; }
	constexpr float3 Extents() const { return ( max - min ) * 0.5f; }

	friend constexpr bool operator==( const Aabb &, const Aabb & ) = default;
};

struct Sphere
{
	float3 center;
	float radius = 0.0f;
};

Aabb Grow( const Aabb &box, const float3 &point );
Aabb Union( const Aabb &a, const Aabb &b );
bool Contains( const Aabb &box, const float3 &point );
bool Overlaps( const Aabb &a, const Aabb &b );
// The box that holds the transformed corners of box (exact for affine m).
Aabb TransformBounds( const float4x4 &m, const Aabb &box );
Sphere BoundingSphere( const Aabb &box );

} // namespace render::math

#endif // RENDER_MATH_BOUNDS_H
