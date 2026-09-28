//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/scene/displacement_geometry.h.
//
//=============================================================================//

#include "hammer/scene/displacement_geometry.h"

namespace hammer::scene
{

using mapgeometry::Vec3d;

std::optional<mapgeometry::DisplacementSurface> BuildDisplacement(
    const Solid &solid, std::size_t sideIndex, bool baseOnly )
{
	if ( sideIndex >= solid.sides.size() || !solid.sides[sideIndex].displacement )
	{
		return std::nullopt;
	}
	const Displacement &d = *solid.sides[sideIndex].displacement;
	if ( d.power < 2 || d.power > 4 )
	{
		return std::nullopt;
	}
	const std::optional<std::array<Vec3d, 4>> corners = QuadCorners( solid, sideIndex );
	if ( !corners )
	{
		return std::nullopt;
	}
	const std::size_t verts = static_cast<std::size_t>( d.VertsPerRow() * d.VertsPerRow() );
	const std::size_t tris = static_cast<std::size_t>( 2 * d.QuadsPerRow() * d.QuadsPerRow() );
	auto sized = [&]( const auto &array, std::size_t n )
	{
		return !array || array->size() == n;
	};
	if ( !sized( d.normals, verts ) || !sized( d.distances, verts ) || !sized( d.offsets, verts ) ||
	     !sized( d.alphas, verts ) || !sized( d.triangleTags, tris ) )
	{
		return std::nullopt;
	}
	mapgeometry::DispInfo info;
	info.power = d.power;
	info.subdiv = d.subdivided ? 1 : 0;
	info.startPosition = d.startPosition;
	info.elevation = baseOnly ? 0.0 : d.elevation;
	info.normals = d.normals ? *d.normals : std::vector<Vec3d>( verts );
	info.distances =
	    ( d.distances && !baseOnly ) ? *d.distances : std::vector<double>( verts, 0.0 );
	info.offsets = ( d.offsets && !baseOnly ) ? *d.offsets : std::vector<Vec3d>( verts );
	info.alphas = d.alphas ? *d.alphas : std::vector<double>( verts, 0.0 );
	info.triangleTags = d.triangleTags ? *d.triangleTags : std::vector<int>( tris, 0 );
	const Vec3d normal = solid.sides[sideIndex].Plane().normal;
	return mapgeometry::BuildDisplacementSurface( *corners, normal, info );
}

} // namespace hammer::scene
