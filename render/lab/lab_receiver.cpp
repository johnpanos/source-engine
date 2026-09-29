//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's receiver scene (RFC 0016 K11); see lab_receiver.h.
//
//=============================================================================//

#include "lab_receiver.h"

#include "render/material/surface_program.h"

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

} // namespace render::lab
