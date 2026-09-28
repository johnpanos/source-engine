//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.math matrix operations (RFC 0016).
//
//=============================================================================//

#include "render/math/matrix.h"

#include <cmath>

namespace render::math
{

namespace
{

float &At( float4x4 &m, int row, int column )
{
	return ( &m.rows[row].x )[column];
}

float At( const float4x4 &m, int row, int column )
{
	return ( &m.rows[row].x )[column];
}

} // namespace

float4x4 Multiply( const float4x4 &a, const float4x4 &b )
{
	float4x4 result;
	for ( int row = 0; row < 4; ++row )
	{
		for ( int column = 0; column < 4; ++column )
			At( result, row, column ) = Dot( a.rows[row], b.Column( column ) );
	}
	return result;
}

float4 Transform( const float4x4 &m, const float4 &v )
{
	return { Dot( m.rows[0], v ), Dot( m.rows[1], v ), Dot( m.rows[2], v ), Dot( m.rows[3], v ) };
}

float3 TransformPoint( const float4x4 &m, const float3 &p )
{
	const float4 h = Transform( m, { p.x, p.y, p.z, 1.0f } );
	const float inverse = h.w != 0.0f ? 1.0f / h.w : 1.0f;
	return { h.x * inverse, h.y * inverse, h.z * inverse };
}

float4x4 Transpose( const float4x4 &m )
{
	float4x4 result;
	for ( int row = 0; row < 4; ++row )
		result.rows[row] = m.Column( row );
	return result;
}

// Gauss-Jordan elimination with partial pivoting, in double precision.
std::optional<float4x4> Inverse( const float4x4 &m )
{
	double a[4][8];
	for ( int row = 0; row < 4; ++row )
	{
		for ( int column = 0; column < 4; ++column )
		{
			a[row][column] = At( m, row, column );
			a[row][column + 4] = row == column ? 1.0 : 0.0;
		}
	}
	for ( int column = 0; column < 4; ++column )
	{
		int pivot = column;
		for ( int row = column + 1; row < 4; ++row )
		{
			if ( std::fabs( a[row][column] ) > std::fabs( a[pivot][column] ) )
				pivot = row;
		}
		if ( std::fabs( a[pivot][column] ) < 1e-12 )
			return std::nullopt;
		if ( pivot != column )
		{
			for ( int k = 0; k < 8; ++k )
			{
				const double swap = a[column][k];
				a[column][k] = a[pivot][k];
				a[pivot][k] = swap;
			}
		}
		const double scale = 1.0 / a[column][column];
		for ( int k = 0; k < 8; ++k )
			a[column][k] *= scale;
		for ( int row = 0; row < 4; ++row )
		{
			if ( row == column || a[row][column] == 0.0 )
				continue;
			const double factor = a[row][column];
			for ( int k = 0; k < 8; ++k )
				a[row][k] -= factor * a[column][k];
		}
	}
	float4x4 result;
	for ( int row = 0; row < 4; ++row )
	{
		for ( int column = 0; column < 4; ++column )
			At( result, row, column ) = static_cast<float>( a[row][column + 4] );
	}
	return result;
}

float4x4 Translation( const float3 &offset )
{
	float4x4 result;
	result.rows[0].w = offset.x;
	result.rows[1].w = offset.y;
	result.rows[2].w = offset.z;
	return result;
}

float4x4 Scale( const float3 &factors )
{
	float4x4 result;
	result.rows[0].x = factors.x;
	result.rows[1].y = factors.y;
	result.rows[2].z = factors.z;
	return result;
}

float4x4 LookAt( const float3 &eye, const float3 &target, const float3 &up )
{
	const float3 back = Normalize( eye - target );
	const float3 right = Normalize( Cross( up, back ) );
	const float3 trueUp = Cross( back, right );
	float4x4 result;
	result.rows[0] = { right.x, right.y, right.z, -Dot( right, eye ) };
	result.rows[1] = { trueUp.x, trueUp.y, trueUp.z, -Dot( trueUp, eye ) };
	result.rows[2] = { back.x, back.y, back.z, -Dot( back, eye ) };
	result.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	return result;
}

float4x4 Perspective( float verticalFovRadians, float aspect, float nearZ, float farZ )
{
	const float y = 1.0f / std::tan( verticalFovRadians * 0.5f );
	const float x = y / aspect;
	// View-space z in [-near, -far] maps to depth [0, 1].
	const float range = farZ / ( nearZ - farZ );
	float4x4 result;
	result.rows[0] = { x, 0.0f, 0.0f, 0.0f };
	result.rows[1] = { 0.0f, y, 0.0f, 0.0f };
	result.rows[2] = { 0.0f, 0.0f, range, range * nearZ };
	result.rows[3] = { 0.0f, 0.0f, -1.0f, 0.0f };
	return result;
}

float4x4 Orthographic( float width, float height, float nearZ, float farZ )
{
	const float range = 1.0f / ( nearZ - farZ );
	float4x4 result;
	result.rows[0] = { 2.0f / width, 0.0f, 0.0f, 0.0f };
	result.rows[1] = { 0.0f, 2.0f / height, 0.0f, 0.0f };
	result.rows[2] = { 0.0f, 0.0f, range, range * nearZ };
	result.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	return result;
}

float4x4 LookBasis(
    const float3 &eye, const float3 &forward, const float3 &right, const float3 &up )
{
	float4x4 result;
	result.rows[0] = { right.x, right.y, right.z, -Dot( right, eye ) };
	result.rows[1] = { up.x, up.y, up.z, -Dot( up, eye ) };
	result.rows[2] = { -forward.x, -forward.y, -forward.z, Dot( forward, eye ) };
	result.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	return result;
}

float4x4 PixelToClip( float width, float height )
{
	float4x4 result;
	result.rows[0] = { 2.0f / width, 0.0f, 0.0f, -1.0f };
	result.rows[1] = { 0.0f, -2.0f / height, 0.0f, 1.0f };
	result.rows[2] = { 0.0f, 0.0f, 1.0f, 0.0f };
	result.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	return result;
}

} // namespace render::math
