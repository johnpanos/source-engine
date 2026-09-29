//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's analytic SSR scenes; see ssr_scene.h.
//
//=============================================================================//

#include "ssr_scene.h"

#include "render/math/matrix.h"

#include <cmath>
#include <limits>

namespace render::lab
{

namespace
{

using math::float3;

float Dot3( const float3 &a, const float3 &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float3 Add( const float3 &a, const float3 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

float3 Sub( const float3 &a, const float3 &b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

float3 Scale( const float3 &a, float s )
{
	return { a.x * s, a.y * s, a.z * s };
}

// The nearest rectangle along origin + t dir, t > minT; -1 when none.
int Nearest(
    const SsrScene &scene, const float3 &origin, const float3 &dir, float minT, float3 &point )
{
	int best = -1;
	float bestT = std::numeric_limits<float>::max();
	for ( std::size_t i = 0; i < scene.quads.size(); ++i )
	{
		const SsrQuad &q = scene.quads[i];
		const float denominator = Dot3( dir, q.normal );
		if ( std::fabs( denominator ) < 1e-9f )
			continue;
		const float t = Dot3( Sub( q.corner, origin ), q.normal ) / denominator;
		if ( !( t > minT ) || t >= bestT )
			continue;
		const float3 p = Add( origin, Scale( dir, t ) );
		const float3 local = Sub( p, q.corner );
		const float a = Dot3( local, q.u ) / Dot3( q.u, q.u );
		const float b = Dot3( local, q.v ) / Dot3( q.v, q.v );
		if ( a < 0.0f || a > 1.0f || b < 0.0f || b > 1.0f )
			continue;
		best = int( i );
		bestT = t;
		point = p;
	}
	return best;
}

} // namespace

SsrSceneImages RayCastScene( const SsrScene &scene )
{
	SsrSceneImages images;
	SsrReferenceInputs &in = images.inputs;
	const std::uint32_t W = scene.width, H = scene.height;
	in.width = W;
	in.height = H;
	in.eye[0] = scene.eye.x;
	in.eye[1] = scene.eye.y;
	in.eye[2] = scene.eye.z;
	in.toClip = math::Multiply( math::Perspective( scene.verticalFovDegrees * 3.14159265f / 180.0f,
	                                float( W ) / float( H ), scene.nearZ, scene.farZ ),
	    math::LookAt( scene.eye, scene.target, float3{ 0, 0, 1 } ) );
	const math::float4x4 fromClip = *math::Inverse( in.toClip );
	const std::size_t pixels = std::size_t( W ) * H;
	in.depth.assign( pixels, 1.0f );
	in.normalRoughness.assign( pixels * 4, 0.0f );
	in.specularWeight.assign( pixels * 4, 0.0f );
	in.imageSpecular.assign( pixels * 4, 0.0f );
	in.lit.assign( pixels * 4, 0.0f );
	images.truth.assign( pixels, SsrTruth() );
	std::vector<float3> points( pixels );

	for ( std::uint32_t y = 0; y < H; ++y )
	{
		for ( std::uint32_t x = 0; x < W; ++x )
		{
			const std::size_t i = std::size_t( y ) * W + x;
			const float nx = ( float( x ) + 0.5f ) / float( W ) * 2.0f - 1.0f;
			const float ny = 1.0f - ( float( y ) + 0.5f ) / float( H ) * 2.0f;
			const float3 nearPoint = math::TransformPoint( fromClip, { nx, ny, 0.0f } );
			const float3 farPoint = math::TransformPoint( fromClip, { nx, ny, 1.0f } );
			const float3 dir = Sub( farPoint, nearPoint );
			float3 p;
			const int q = Nearest( scene, nearPoint, dir, 0.0f, p );
			images.truth[i].quad = q;
			in.lit[i * 4 + 3] = 1.0f;
			if ( q < 0 )
				continue;
			const SsrQuad &quad = scene.quads[std::size_t( q )];
			points[i] = p;
			const math::float4 clip = math::Transform( in.toClip, { p.x, p.y, p.z, 1.0f } );
			in.depth[i] = clip.z / clip.w;
			const pass::ssr::Octahedral n =
			    pass::ssr::OctEncode( quad.normal.x, quad.normal.y, quad.normal.z );
			in.normalRoughness[i * 4 + 0] = n.x;
			in.normalRoughness[i * 4 + 1] = n.y;
			in.normalRoughness[i * 4 + 2] = quad.roughness;
			const std::array<float, 3> radiance = quad.radiance( p );
			for ( int c = 0; c < 3; ++c )
			{
				in.specularWeight[i * 4 + std::size_t( c )] = quad.weight[std::size_t( c )];
				in.imageSpecular[i * 4 + std::size_t( c )] = quad.imageSpecular[std::size_t( c )];
				in.lit[i * 4 + std::size_t( c )] =
				    radiance[std::size_t( c )] + quad.imageSpecular[std::size_t( c )];
			}
		}
	}

	// The true mirror reflection of each pixel's point.
	for ( std::uint32_t y = 0; y < H; ++y )
	{
		for ( std::uint32_t x = 0; x < W; ++x )
		{
			const std::size_t i = std::size_t( y ) * W + x;
			SsrTruth &truth = images.truth[i];
			if ( truth.quad < 0 )
				continue;
			const SsrQuad &quad = scene.quads[std::size_t( truth.quad )];
			const float3 view = Sub( scene.eye, points[i] );
			const float3 v = Scale( view, 1.0f / std::sqrt( Dot3( view, view ) ) );
			const float3 r = Sub( Scale( quad.normal, 2.0f * Dot3( quad.normal, v ) ), v );
			float3 hit;
			truth.reflectedQuad = Nearest( scene, points[i], r, 1e-3f, hit );
			if ( truth.reflectedQuad < 0 )
				continue;
			truth.reflectedPoint = hit;
			const math::float4 clip = math::Transform( in.toClip, { hit.x, hit.y, hit.z, 1.0f } );
			if ( !( clip.w > 0.0f ) )
				continue;
			truth.reflectedX = ( clip.x / clip.w + 1.0 ) * 0.5 * W;
			truth.reflectedY = ( 1.0 - clip.y / clip.w ) * 0.5 * H;
			truth.reflectedOnScreen = truth.reflectedX >= 0.0 && truth.reflectedX < W &&
			                          truth.reflectedY >= 0.0 && truth.reflectedY < H;
			if ( !truth.reflectedOnScreen )
				continue;
			const long cx = long( truth.reflectedX ), cy = long( truth.reflectedY );
			bool visible = true;
			for ( long dy = -2; dy <= 2 && visible; ++dy )
			{
				for ( long dx = -2; dx <= 2 && visible; ++dx )
				{
					const long sx = cx + dx, sy = cy + dy;
					visible = sx >= 0 && sy >= 0 && sx < long( W ) && sy < long( H ) &&
					          images.truth[std::size_t( sy ) * W + std::size_t( sx )].quad ==
					              truth.reflectedQuad;
				}
			}
			truth.reflectedVisible = visible;
		}
	}
	return images;
}

} // namespace render::lab
