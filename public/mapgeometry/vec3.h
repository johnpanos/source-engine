//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Value arithmetic on the double-precision map vector Vec3d (RFC 0002,
//			world.map-geometry). Header-only and exact: no tolerance is hidden in
//			these operators. Tolerant comparisons name their epsilon explicitly.
//
//=============================================================================//

#ifndef MAPGEOMETRY_VEC3_H
#define MAPGEOMETRY_VEC3_H

#include "mapgeometry/brush.h"

#include <cmath>

namespace mapgeometry
{

constexpr Vec3d operator+( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x + b.x, a.y + b.y, a.z + b.z );
}

constexpr Vec3d operator-( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x - b.x, a.y - b.y, a.z - b.z );
}

constexpr Vec3d operator-( const Vec3d &a )
{
	return Vec3d( -a.x, -a.y, -a.z );
}

constexpr Vec3d operator*( const Vec3d &a, double s )
{
	return Vec3d( a.x * s, a.y * s, a.z * s );
}

constexpr Vec3d operator*( double s, const Vec3d &a )
{
	return a * s;
}

constexpr Vec3d operator/( const Vec3d &a, double s )
{
	return Vec3d( a.x / s, a.y / s, a.z / s );
}

constexpr Vec3d &operator+=( Vec3d &a, const Vec3d &b )
{
	a = a + b;
	return a;
}

constexpr Vec3d &operator-=( Vec3d &a, const Vec3d &b )
{
	a = a - b;
	return a;
}

constexpr double Dot( const Vec3d &a, const Vec3d &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr Vec3d Cross( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}

// Componentwise product and the component selected by axis index 0/1/2.
constexpr Vec3d Mul( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x * b.x, a.y * b.y, a.z * b.z );
}

constexpr double Component( const Vec3d &v, int axis )
{
	return axis == 0 ? v.x : ( axis == 1 ? v.y : v.z );
}

constexpr void SetComponent( Vec3d &v, int axis, double value )
{
	( axis == 0 ? v.x : ( axis == 1 ? v.y : v.z ) ) = value;
}

inline double Length( const Vec3d &a )
{
	return std::sqrt( Dot( a, a ) );
}

// The unit vector along 'a', or the zero vector when 'a' is (near) zero length.
inline Vec3d Normalize( const Vec3d &a )
{
	const double len = Length( a );
	return len > 1.0e-12 ? a / len : Vec3d();
}

// True when every component differs by at most 'epsilon'.
inline bool NearlyEqual( const Vec3d &a, const Vec3d &b, double epsilon )
{
	return std::fabs( a.x - b.x ) <= epsilon && std::fabs( a.y - b.y ) <= epsilon &&
	       std::fabs( a.z - b.z ) <= epsilon;
}

// Signed distance of 'p' from 'plane' (positive outside, see Plane).
inline double PlaneDistance( const Plane &plane, const Vec3d &p )
{
	return Dot( plane.normal, p ) - plane.dist;
}

} // namespace mapgeometry

#endif // MAPGEOMETRY_VEC3_H
