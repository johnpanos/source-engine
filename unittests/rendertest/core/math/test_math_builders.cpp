//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.math's convention-free builders (RFC 0016 decision
//			"cameras"; contract render.math-builders.v1):
//
//			B1 LookBasis maps the eye to the view origin, forward to view -Z,
//			   right to +X and up to +Y, and a LookAt basis rebuilt through it
//			   gives LookAt's matrix;
//			B2 PixelToClip maps a target's top-left pixel corner to clip
//			   (-1, +1), its bottom-right to (+1, -1) and its centre to the
//			   origin, and passes z and w through;
//			B3 each builder's checks reject a seeded wrong builder (swapped
//			   basis rows, an unflipped y), so the checks can fail.
//
//=============================================================================//

#include "render/math/matrix.h"
#include "testing/checks.h"

#include <cmath>

namespace
{

using render::math::float3;
using render::math::float4;
using render::math::float4x4;

bool Near( const float3 &a, const float3 &b, float epsilon = 1.0e-5f )
{
	return std::fabs( a.x - b.x ) <= epsilon && std::fabs( a.y - b.y ) <= epsilon &&
	       std::fabs( a.z - b.z ) <= epsilon;
}

bool Near( const float4x4 &a, const float4x4 &b, float epsilon = 1.0e-5f )
{
	for ( int r = 0; r < 4; ++r )
	{
		const float4 &x = a.rows[r];
		const float4 &y = b.rows[r];
		if ( std::fabs( x.x - y.x ) > epsilon || std::fabs( x.y - y.y ) > epsilon ||
		     std::fabs( x.z - y.z ) > epsilon || std::fabs( x.w - y.w ) > epsilon )
			return false;
	}
	return true;
}

using BasisBuilder = float4x4 ( * )(
    const float3 &, const float3 &, const float3 &, const float3 & );
using PixelBuilder = float4x4 ( * )( float, float );

// B1 for one builder; true when every clause holds.
bool BasisHolds( BasisBuilder build )
{
	const float3 eye{ 10.0f, -20.0f, 30.0f };
	const float3 forward = render::math::Normalize( { 1.0f, 2.0f, -0.5f } );
	const float3 worldUp{ 0.0f, 0.0f, 1.0f };
	const float3 right = render::math::Normalize( render::math::Cross( forward, worldUp ) );
	const float3 up = render::math::Cross( right, forward );
	const float4x4 view = build( eye, forward, right, up );
	using render::math::TransformPoint;
	return Near( TransformPoint( view, eye ), { 0, 0, 0 } ) &&
	       Near( TransformPoint( view, eye + forward ), { 0, 0, -1 } ) &&
	       Near( TransformPoint( view, eye + right ), { 1, 0, 0 } ) &&
	       Near( TransformPoint( view, eye + up ), { 0, 1, 0 } ) &&
	       Near( view, render::math::LookAt( eye, eye + forward, worldUp ) );
}

// B2 for one builder.
bool PixelHolds( PixelBuilder build )
{
	const float4x4 m = build( 640.0f, 480.0f );
	using render::math::TransformPoint;
	const float4 passed = render::math::Transform( m, { 320.0f, 240.0f, 0.25f, 2.0f } );
	return Near( TransformPoint( m, { 0, 0, 0.5f } ), { -1, 1, 0.5f } ) &&
	       Near( TransformPoint( m, { 640, 480, 0 } ), { 1, -1, 0 } ) &&
	       Near( TransformPoint( m, { 320, 240, 0 } ), { 0, 0, 0 } ) && passed.z == 0.25f &&
	       passed.w == 2.0f;
}

// Seeded wrong builders (B3).
float4x4 SwappedBasis(
    const float3 &eye, const float3 &forward, const float3 &right, const float3 &up )
{
	return render::math::LookBasis( eye, forward, up, right );
}

float4x4 UnflippedPixels( float width, float height )
{
	float4x4 m = render::math::PixelToClip( width, height );
	m.rows[1] = { 0.0f, 2.0f / height, 0.0f, -1.0f };
	return m;
}

} // namespace

int main()
{
	testing::Checks checks;
	checks.That( BasisHolds( &render::math::LookBasis ), "B1.look-basis-maps-the-basis" );
	checks.That( PixelHolds( &render::math::PixelToClip ), "B2.pixel-to-clip-maps-the-corners" );
	checks.That( !BasisHolds( &SwappedBasis ), "B3.a-swapped-basis-is-rejected" );
	checks.That( !PixelHolds( &UnflippedPixels ), "B3.an-unflipped-y-is-rejected" );
	return checks.Report();
}
