//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.shadows shadow views and cascades (RFC 0016 K7).
//
//=============================================================================//

#include "render/pass/shadows/shadow_views.h"

#include <algorithm>
#include <cmath>

namespace render::pass::shadows
{

namespace
{

using math::float3;
using math::float4x4;

bool Finite( float value )
{
	return std::isfinite( value );
}

bool Finite3( const float3 &v )
{
	return Finite( v.x ) && Finite( v.y ) && Finite( v.z );
}

// A stable up for a light basis: world +Z (Source's up) unless the axis is
// nearly vertical, then world +Y.
float3 UpFor( const float3 &axis )
{
	return std::fabs( axis.z ) < 0.99f ? float3{ 0.0f, 0.0f, 1.0f } : float3{ 0.0f, 1.0f, 0.0f };
}

// A view from an orthonormal basis (right, up, back) and eye position.
float4x4 BasisView( const float3 &eye, const float3 &right, const float3 &up, const float3 &back )
{
	float4x4 view;
	view.rows[0] = { right.x, right.y, right.z, -math::Dot( right, eye ) };
	view.rows[1] = { up.x, up.y, up.z, -math::Dot( up, eye ) };
	view.rows[2] = { back.x, back.y, back.z, -math::Dot( back, eye ) };
	view.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	return view;
}

ShadowView MakeView( const float4x4 &view, const float4x4 &projection, float nearZ, float farZ )
{
	ShadowView result;
	result.view = view;
	result.projection = projection;
	result.viewProjection = math::Multiply( projection, view );
	result.nearZ = nearZ;
	result.farZ = farZ;
	return result;
}

} // namespace

foundation::Expected<ShadowView, ShadowViewError> BuildSpotShadowView( const SpotShadowDesc &desc )
{
	using foundation::MakeUnexpected;
	if ( !Finite3( desc.position ) || !Finite3( desc.direction ) || !Finite( desc.outerCos ) ||
	     !( math::Dot( desc.direction, desc.direction ) > 0.0f ) )
		return MakeUnexpected( ShadowViewError::kInvalidLight );
	if ( !Finite( desc.nearZ ) || !Finite( desc.range ) || !( desc.nearZ > 0.0f ) ||
	     !( desc.range > desc.nearZ ) )
		return MakeUnexpected( ShadowViewError::kInvalidDepthRange );
	const float halfAngle = std::acos( std::clamp( desc.outerCos, -1.0f, 1.0f ) );
	if ( !( halfAngle <= kMaxSpotHalfAngle ) )
		return MakeUnexpected( ShadowViewError::kConeTooWide );

	const float3 forward = math::Normalize( desc.direction );
	const float4x4 view = math::LookAt( desc.position, desc.position + forward, UpFor( forward ) );
	// A square pyramid around the cone: its half-angle to the side planes is
	// the cone's (a zero-width cone still gets a texel's worth).
	const float fov = 2.0f * std::max( halfAngle, 1.0e-3f );
	const float4x4 projection = math::Perspective( fov, 1.0f, desc.nearZ, desc.range );
	return MakeView( view, projection, desc.nearZ, desc.range );
}

foundation::Expected<ShadowView, ShadowViewError> BuildFlashlightShadowView(
    const FlashlightShadowDesc &desc )
{
	using foundation::MakeUnexpected;
	if ( !Finite3( desc.position ) || !Finite3( desc.forward ) || !Finite3( desc.up ) ||
	     !( math::Dot( desc.forward, desc.forward ) > 0.0f ) )
		return MakeUnexpected( ShadowViewError::kInvalidLight );
	const float3 forward = math::Normalize( desc.forward );
	const float3 side = math::Cross( forward, desc.up );
	if ( !( math::Length( side ) > 1.0e-4f * math::Length( desc.up ) ) )
		return MakeUnexpected( ShadowViewError::kInvalidLight );
	const float limit = 3.1f; // below pi, where the projection degenerates
	if ( !( desc.horizontalFovRadians > 0.0f && desc.horizontalFovRadians < limit &&
	         desc.verticalFovRadians > 0.0f && desc.verticalFovRadians < limit ) )
		return MakeUnexpected( ShadowViewError::kInvalidLight );
	if ( !Finite( desc.nearZ ) || !Finite( desc.farZ ) || !( desc.nearZ > 0.0f ) ||
	     !( desc.farZ > desc.nearZ ) )
		return MakeUnexpected( ShadowViewError::kInvalidDepthRange );

	const float4x4 view = math::LookAt( desc.position, desc.position + forward, desc.up );
	const float aspect =
	    std::tan( desc.horizontalFovRadians * 0.5f ) / std::tan( desc.verticalFovRadians * 0.5f );
	const float4x4 projection =
	    math::Perspective( desc.verticalFovRadians, aspect, desc.nearZ, desc.farZ );
	return MakeView( view, projection, desc.nearZ, desc.farZ );
}

std::array<float, kMaxCascades + 1> PracticalSplits(
    float nearZ, float farZ, std::uint32_t count, float lambda )
{
	std::array<float, kMaxCascades + 1> splits;
	splits.fill( farZ );
	count = std::clamp<std::uint32_t>( count, 1, kMaxCascades );
	for ( std::uint32_t i = 0; i <= count; ++i )
	{
		const double t = double( i ) / double( count );
		const double logarithmic = double( nearZ ) * std::pow( double( farZ ) / nearZ, t );
		const double uniform = double( nearZ ) + ( double( farZ ) - nearZ ) * t;
		splits[i] = static_cast<float>( lambda * logarithmic + ( 1.0 - lambda ) * uniform );
	}
	splits[0] = nearZ;
	splits[count] = farZ;
	return splits;
}

foundation::Expected<CascadeSet, ShadowViewError> BuildCascades( const CascadeDesc &desc )
{
	using foundation::MakeUnexpected;
	if ( desc.cascadeCount == 0 || desc.cascadeCount > kMaxCascades || !Finite( desc.lambda ) ||
	     desc.lambda < 0.0f || desc.lambda > 1.0f || desc.resolution < 16 ||
	     !( desc.verticalFovRadians > 0.0f && desc.verticalFovRadians < 3.1f ) ||
	     !( desc.aspect > 0.0f ) || !Finite( desc.aspect ) || !Finite( desc.casterDistance ) ||
	     desc.casterDistance < 0.0f )
		return MakeUnexpected( ShadowViewError::kInvalidCascades );
	if ( !Finite( desc.nearZ ) || !Finite( desc.shadowDistance ) || !( desc.nearZ > 0.0f ) ||
	     !( desc.shadowDistance > desc.nearZ ) )
		return MakeUnexpected( ShadowViewError::kInvalidDepthRange );
	if ( !Finite3( desc.lightDirection ) ||
	     !( math::Dot( desc.lightDirection, desc.lightDirection ) > 0.0f ) )
		return MakeUnexpected( ShadowViewError::kInvalidLight );
	const std::optional<float4x4> cameraToWorld = math::Inverse( desc.cameraView );
	if ( !cameraToWorld )
		return MakeUnexpected( ShadowViewError::kInvalidCascades );

	// The light's basis is fixed in the world, so the snapping lattice is too.
	const float3 back = math::Normalize( desc.lightDirection ) * -1.0f;
	const float3 right = math::Normalize( math::Cross( UpFor( back ), back ) );
	const float3 up = math::Cross( back, right );

	// A corner of the frustum at view distance d lies k d from the axis.
	const double tanHalf = std::tan( double( desc.verticalFovRadians ) * 0.5 );
	const double k2 = tanHalf * tanHalf * ( 1.0 + double( desc.aspect ) * desc.aspect );

	const std::array<float, kMaxCascades + 1> splits =
	    PracticalSplits( desc.nearZ, desc.shadowDistance, desc.cascadeCount, desc.lambda );
	CascadeSet set;
	set.count = desc.cascadeCount;
	for ( std::uint32_t i = 0; i < desc.cascadeCount; ++i )
	{
		Cascade &cascade = set.cascades[i];
		const double n = splits[i];
		const double f = splits[i + 1];
		// The smallest sphere around the slice's eight corners has its center
		// on the axis where the near and far corners are equally far, or at
		// the far face when that point lies beyond it.
		double center = 0.5 * ( n + f ) * ( 1.0 + k2 );
		double radius;
		if ( center >= f )
		{
			center = f;
			radius = std::sqrt( k2 ) * f;
		}
		else
		{
			radius = std::sqrt( ( center - n ) * ( center - n ) + k2 * n * n );
		}
		// Rounding up to a multiple of a small quantum keeps the radius from
		// jittering in its last bits.
		radius = std::ceil( radius * 1.0e3 ) / 1.0e3;
		const float3 worldCenter =
		    math::TransformPoint( *cameraToWorld, { 0.0f, 0.0f, float( -center ) } );

		// The box is the sphere plus one texel, so moving it by less than a
		// texel keeps the sphere inside: texel = 2 r / ( resolution - 2 ).
		const double texel = 2.0 * radius / double( desc.resolution - 2 );
		const double halfExtent = radius + texel;
		const double x = std::floor( double( math::Dot( right, worldCenter ) ) / texel ) * texel;
		const double y = std::floor( double( math::Dot( up, worldCenter ) ) / texel ) * texel;
		const double z = double( math::Dot( back, worldCenter ) ) + radius + desc.casterDistance;
		const float depth = float( 2.0 * radius + desc.casterDistance );
		// The eye at right x + up y + back z; its translation is written from
		// the lattice coordinates directly, not from a rounded eye position.
		float4x4 view = BasisView( { 0.0f, 0.0f, 0.0f }, right, up, back );
		view.rows[0].w = float( -x );
		view.rows[1].w = float( -y );
		view.rows[2].w = float( -z );
		const float4x4 projection =
		    math::Orthographic( float( 2.0 * halfExtent ), float( 2.0 * halfExtent ), 0.0f, depth );

		cascade.splitNear = splits[i];
		cascade.splitFar = splits[i + 1];
		cascade.bounds = { worldCenter, float( radius ) };
		cascade.texelSize = float( texel );
		cascade.view = MakeView( view, projection, 0.0f, depth );
	}
	return set;
}

} // namespace render::pass::shadows
