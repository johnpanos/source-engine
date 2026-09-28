//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.math matrices (RFC 0016). float4x4 stores rows; vectors
//			are columns, so a point transforms as M * p and transforms compose
//			right to left (clip = projection * view * world * p).
//
//			Projections follow the device port's fixed conventions
//			(render/device/conventions.h): clip-space depth 0 to 1 and
//			clip-space Y up. View space looks down -Z.
//
//=============================================================================//

#ifndef RENDER_MATH_MATRIX_H
#define RENDER_MATH_MATRIX_H

#include "render/math/vector.h"

#include <optional>

namespace render::math
{

struct float4x4
{
	float4 rows[4] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

	static constexpr float4x4 Identity() { return {}; }

	constexpr float4 Column( int index ) const
	{
		const float *r0 = &rows[0].x;
		const float *r1 = &rows[1].x;
		const float *r2 = &rows[2].x;
		const float *r3 = &rows[3].x;
		return { r0[index], r1[index], r2[index], r3[index] };
	}

	friend constexpr bool operator==( const float4x4 &, const float4x4 & ) = default;
};

float4x4 Multiply( const float4x4 &a, const float4x4 &b );
float4 Transform( const float4x4 &m, const float4 &v );
// Transforms a point (w = 1) and divides by the resulting w.
float3 TransformPoint( const float4x4 &m, const float3 &p );
float4x4 Transpose( const float4x4 &m );
// The general inverse; empty for a singular matrix.
std::optional<float4x4> Inverse( const float4x4 &m );

float4x4 Translation( const float3 &offset );
float4x4 Scale( const float3 &factors );
// A right-handed view that looks from eye toward target with up as the
// approximate up direction.
float4x4 LookAt( const float3 &eye, const float3 &target, const float3 &up );
// Right-handed perspective: depth 0 at near, 1 at far, clip Y up.
float4x4 Perspective( float verticalFovRadians, float aspect, float nearZ, float farZ );
// Right-handed orthographic box with the same depth and Y conventions.
float4x4 Orthographic( float width, float height, float nearZ, float farZ );

} // namespace render::math

#endif // RENDER_MATH_MATRIX_H
