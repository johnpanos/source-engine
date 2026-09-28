//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.math bounds and frustum tests (RFC 0016).
//
//=============================================================================//

#include "render/math/bounds.h"
#include "render/math/frustum.h"

#include <cmath>

namespace render::math
{

Aabb Grow( const Aabb &box, const float3 &point )
{
	if ( box.IsEmpty() )
		return { point, point };
	return { Min( box.min, point ), Max( box.max, point ) };
}

Aabb Union( const Aabb &a, const Aabb &b )
{
	if ( a.IsEmpty() )
		return b;
	if ( b.IsEmpty() )
		return a;
	return { Min( a.min, b.min ), Max( a.max, b.max ) };
}

bool Contains( const Aabb &box, const float3 &point )
{
	return !box.IsEmpty() && point.x >= box.min.x && point.x <= box.max.x && point.y >= box.min.y &&
	       point.y <= box.max.y && point.z >= box.min.z && point.z <= box.max.z;
}

bool Overlaps( const Aabb &a, const Aabb &b )
{
	return !a.IsEmpty() && !b.IsEmpty() && a.min.x <= b.max.x && a.max.x >= b.min.x &&
	       a.min.y <= b.max.y && a.max.y >= b.min.y && a.min.z <= b.max.z && a.max.z >= b.min.z;
}

Aabb TransformBounds( const float4x4 &m, const Aabb &box )
{
	if ( box.IsEmpty() )
		return box;
	Aabb result;
	for ( int corner = 0; corner < 8; ++corner )
	{
		const float3 p = { ( corner & 1 ) ? box.max.x : box.min.x,
		    ( corner & 2 ) ? box.max.y : box.min.y, ( corner & 4 ) ? box.max.z : box.min.z };
		result = Grow( result, TransformPoint( m, p ) );
	}
	return result;
}

Sphere BoundingSphere( const Aabb &box )
{
	if ( box.IsEmpty() )
		return {};
	return { box.Center(), Length( box.Extents() ) };
}

namespace
{

Plane Normalized( const float4 &p )
{
	const float length = std::sqrt( p.x * p.x + p.y * p.y + p.z * p.z );
	const float inverse = length > 0.0f ? 1.0f / length : 0.0f;
	return { { p.x * inverse, p.y * inverse, p.z * inverse }, p.w * inverse };
}

float4 Add( const float4 &a, const float4 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

float4 Subtract( const float4 &a, const float4 &b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

} // namespace

// Gribb and Hartmann's extraction for clip space -w <= x, y <= w and
// 0 <= z <= w.
Frustum ExtractFrustum( const float4x4 &m )
{
	Frustum frustum;
	const float4 &r0 = m.rows[0];
	const float4 &r1 = m.rows[1];
	const float4 &r2 = m.rows[2];
	const float4 &r3 = m.rows[3];
	frustum.planes[static_cast<int>( FrustumPlane::kLeft )] = Normalized( Add( r3, r0 ) );
	frustum.planes[static_cast<int>( FrustumPlane::kRight )] = Normalized( Subtract( r3, r0 ) );
	frustum.planes[static_cast<int>( FrustumPlane::kBottom )] = Normalized( Add( r3, r1 ) );
	frustum.planes[static_cast<int>( FrustumPlane::kTop )] = Normalized( Subtract( r3, r1 ) );
	frustum.planes[static_cast<int>( FrustumPlane::kNear )] = Normalized( r2 );
	frustum.planes[static_cast<int>( FrustumPlane::kFar )] = Normalized( Subtract( r3, r2 ) );
	return frustum;
}

bool Intersects( const Frustum &frustum, const Aabb &box )
{
	if ( box.IsEmpty() )
		return false;
	for ( const Plane &plane : frustum.planes )
	{
		// The corner furthest along the plane normal.
		const float3 positive = { plane.normal.x >= 0.0f ? box.max.x : box.min.x,
		    plane.normal.y >= 0.0f ? box.max.y : box.min.y,
		    plane.normal.z >= 0.0f ? box.max.z : box.min.z };
		if ( plane.Distance( positive ) < 0.0f )
			return false;
	}
	return true;
}

bool Intersects( const Frustum &frustum, const Sphere &sphere )
{
	for ( const Plane &plane : frustum.planes )
	{
		if ( plane.Distance( sphere.center ) < -sphere.radius )
			return false;
	}
	return true;
}

bool Contains( const Frustum &frustum, const float3 &point )
{
	for ( const Plane &plane : frustum.planes )
	{
		if ( plane.Distance( point ) < 0.0f )
			return false;
	}
	return true;
}

} // namespace render::math
