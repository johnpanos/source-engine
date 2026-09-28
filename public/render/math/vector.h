//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.math vectors (RFC 0016). Private to the render family:
//			the legacy frontend converts from Vector and VMatrix, and Hammer's
//			mapgeometry keeps its own double-precision tolerances.
//
//=============================================================================//

#ifndef RENDER_MATH_VECTOR_H
#define RENDER_MATH_VECTOR_H

#include <cmath>

namespace render::math
{

struct float2
{
	float x = 0.0f;
	float y = 0.0f;

	friend constexpr bool operator==( const float2 &, const float2 & ) = default;
};

struct float3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	friend constexpr bool operator==( const float3 &, const float3 & ) = default;
};

struct float4
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	friend constexpr bool operator==( const float4 &, const float4 & ) = default;
};

constexpr float3 operator+( const float3 &a, const float3 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

constexpr float3 operator-( const float3 &a, const float3 &b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

constexpr float3 operator*( const float3 &a, float s )
{
	return { a.x * s, a.y * s, a.z * s };
}

constexpr float Dot( const float3 &a, const float3 &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr float Dot( const float4 &a, const float4 &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

constexpr float3 Cross( const float3 &a, const float3 &b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

inline float Length( const float3 &v )
{
	return std::sqrt( Dot( v, v ) );
}

// A zero vector stays zero rather than producing NaNs.
inline float3 Normalize( const float3 &v )
{
	const float length = Length( v );
	return length > 0.0f ? v * ( 1.0f / length ) : v;
}

constexpr float3 Min( const float3 &a, const float3 &b )
{
	return { a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z };
}

constexpr float3 Max( const float3 &a, const float3 &b )
{
	return { a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z };
}

} // namespace render::math

#endif // RENDER_MATH_VECTOR_H
