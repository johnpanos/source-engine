//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's receiver scene (RFC 0016 K11); see lab_receiver.h.
//
//=============================================================================//

#include "lab_receiver.h"

#include "render/material/surface_program.h"
#include "render/pbr_brdf.h"

#include <cmath>

namespace render::lab
{

ReceiverView MakeReceiverView( math::float3 eye, math::float3 target, std::uint32_t size )
{
	ReceiverView view;
	view.eye = eye;
	view.size = size;
	view.view = math::LookAt( eye, target, math::float3{ 0, 0, 1 } );
	view.projection =
	    math::Perspective( 70.0f * 3.14159265f / 180.0f, 1.0f, view.nearZ, view.farZ );
	view.toClip = math::Multiply( view.projection, view.view );
	view.fromClip = *math::Inverse( view.toClip );
	return view;
}

std::optional<math::float3> ReceiverHit(
    const ReceiverView &view, std::uint32_t px, std::uint32_t py, float extent )
{
	const float nx = ( float( px ) + 0.5f ) / float( view.size ) * 2.0f - 1.0f;
	const float ny = 1.0f - ( float( py ) + 0.5f ) / float( view.size ) * 2.0f;
	const math::float4 a = math::Transform( view.fromClip, { nx, ny, 0.0f, 1.0f } );
	const math::float4 b = math::Transform( view.fromClip, { nx, ny, 1.0f, 1.0f } );
	const math::float3 near{ a.x / a.w, a.y / a.w, a.z / a.w };
	const math::float3 far{ b.x / b.w, b.y / b.w, b.z / b.w };
	if ( !( ( near.z > 0.0f ) != ( far.z > 0.0f ) ) )
		return std::nullopt;
	const float t = near.z / ( near.z - far.z );
	const math::float3 p{ near.x + ( far.x - near.x ) * t, near.y + ( far.y - near.y ) * t, 0.0f };
	if ( std::fabs( p.x ) > extent * 0.98f || std::fabs( p.y ) > extent * 0.98f )
		return std::nullopt;
	return p;
}

math::float3 ToEye( const ReceiverView &view, math::float3 p )
{
	return math::Normalize( { view.eye.x - p.x, view.eye.y - p.y, view.eye.z - p.z } );
}

std::vector<std::byte> ReceiverMesh( float extent )
{
	std::vector<std::byte> bytes;
	const float corners[4][2] = {
	    { -extent, -extent }, { extent, -extent }, { extent, extent }, { -extent, extent } };
	for ( int corner : { 0, 1, 2, 0, 2, 3 } )
	{
		material::SurfaceModelVertex vertex;
		vertex.position[0] = corners[corner][0];
		vertex.position[1] = corners[corner][1];
		vertex.normal[2] = 1.0f;
		vertex.tangent[0] = 1.0f;
		vertex.tangent[3] = 1.0f;
		const auto *raw = reinterpret_cast<const std::byte *>( &vertex );
		bytes.insert( bytes.end(), raw, raw + sizeof( vertex ) );
	}
	return bytes;
}

float RuntimeLightOracle( const light_set::RuntimeLight &light, const ReceiverMaterial &material,
    math::float3 p, math::float3 v )
{
	const math::float3 n{ 0, 0, 1 };
	const float normalDotView = std::max( math::Dot( n, v ), 0.0f );
	const float f0 = material.F0();
	const pbr::SplitSumCoefficients split =
	    pbr::SampleSplitSum( normalDotView, material.Roughness() );
	const float compensation = pbr::SpecularEnergyCompensation( f0, split );
	const float diffuseColor =
	    ( 1.0f - material.Metal() ) * ( 1.0f - pbr::SpecularDirectionalAlbedo( f0, split ) );
	const math::float3 toLight{
	    light.position[0] - p.x, light.position[1] - p.y, light.position[2] - p.z };
	const float distanceSquared = math::Dot( toLight, toLight );
	float falloff =
	    light.falloff == light_set::LightFalloff::Attenuated
	        ? light_set::AttenuatedFalloff( distanceSquared, light.radius, light.attenuation )
	    : light.falloff == light_set::LightFalloff::InverseSquare
	        ? light_set::InverseSquareFalloff( distanceSquared, light.radius, light.sourceRadius )
	        : light_set::Falloff( distanceSquared, light.radius, light.minLight );
	if ( !( falloff > 0.0f ) )
		return 0.0f;
	const math::float3 l = math::Normalize( toLight );
	if ( light.shape == light_set::LightShape::Spot )
	{
		const math::float3 axis{ light.direction[0], light.direction[1], light.direction[2] };
		falloff *= light_set::SpotFactor( -math::Dot( l, math::Normalize( axis ) ),
		    light.innerCos, light.outerCos, light.spotExponent );
	}
	const float normalDotLight = std::max( l.z, 0.0f );
	if ( !( falloff > 0.0f ) || !( normalDotLight > 0.0f ) )
		return 0.0f;
	const float incident = light.color[0] * falloff;
	const math::float3 h = math::Normalize( { l.x + v.x, l.y + v.y, l.z + v.z } );
	const pbr::Color specular = pbr::EvaluateSpecular( { f0, f0, f0 }, normalDotView,
	    normalDotLight, math::Dot( n, h ), math::Dot( v, h ), material.Roughness() );
	return diffuseColor * incident * normalDotLight +
	       pbr::kPi * incident * specular.red * compensation * normalDotLight;
}

} // namespace render::lab
