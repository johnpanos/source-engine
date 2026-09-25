//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::geometry displacement surface construction.
//			See public/hammer/geometry/displacement.h.
//
//=============================================================================//

#include "hammer/geometry/displacement.h"

#include <cmath>
#include <cstddef>

namespace hammer::geometry
{

namespace
{

// Local double-precision vector helpers. Vec3d (from brush.h) is a plain value
// type with no operators; the brush module keeps its own arithmetic private, so
// this module keeps its own too rather than widening a shared surface prematurely.
Vec3d Add( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x + b.x, a.y + b.y, a.z + b.z );
}

Vec3d Scale( const Vec3d &a, double s )
{
	return Vec3d( a.x * s, a.y * s, a.z * s );
}

double DistSq( const Vec3d &a, const Vec3d &b )
{
	const double dx = a.x - b.x;
	const double dy = a.y - b.y;
	const double dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}

Vec3d Sub( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x - b.x, a.y - b.y, a.z - b.z );
}

Vec3d Cross( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}

} // namespace

DisplacementSurface BuildDisplacementSurface(
    const std::array<Vec3d, 4> &corners, const Vec3d &faceNormal, const DispInfo &disp )
{
	DisplacementSurface surface;
	surface.power = disp.power;
	const int side = disp.Side();
	surface.side = side;

	// Choose the origin corner: the one nearest startPosition, matching VBSP/Hammer.
	int origin = 0;
	double best = DistSq( corners[0], disp.startPosition );
	for ( int i = 1; i < 4; ++i )
	{
		const double d = DistSq( corners[i], disp.startPosition );
		if ( d < best )
		{
			best = d;
			origin = i;
		}
	}
	// Rotate the winding so the origin corner is A, preserving winding direction.
	const Vec3d a = corners[static_cast<std::size_t>( origin )];
	const Vec3d b = corners[static_cast<std::size_t>( ( origin + 1 ) % 4 )];
	const Vec3d c = corners[static_cast<std::size_t>( ( origin + 2 ) % 4 )];
	const Vec3d d = corners[static_cast<std::size_t>( ( origin + 3 ) % 4 )];

	surface.vertices.resize( static_cast<std::size_t>( side ) * side );
	// Carry per-vertex blend weights straight through (row-major, parallel to
	// vertices); default to zero when the dispinfo carried no alphas grid.
	if ( static_cast<int>( disp.alphas.size() ) == side * side )
	{
		surface.vertexAlphas = disp.alphas;
	}
	else
	{
		surface.vertexAlphas.assign( static_cast<std::size_t>( side ) * side, 0.0 );
	}
	const double span = ( side > 1 ) ? static_cast<double>( side - 1 ) : 1.0;
	const Vec3d elevationPush = Scale( faceNormal, disp.elevation );

	for ( int r = 0; r < side; ++r )
	{
		const double t = static_cast<double>( r ) / span;
		for ( int col = 0; col < side; ++col )
		{
			const double s = static_cast<double>( col ) / span;
			// Bilinear interpolation across the (rotated) quad A,B,C,D.
			Vec3d base =
			    Add( Add( Scale( a, ( 1.0 - s ) * ( 1.0 - t ) ), Scale( b, s * ( 1.0 - t ) ) ),
			        Add( Scale( c, s * t ), Scale( d, ( 1.0 - s ) * t ) ) );

			const std::size_t idx = static_cast<std::size_t>( r ) * side + col;
			Vec3d v = Add( base, disp.offsets[idx] );
			v = Add( v, Scale( disp.normals[idx], disp.distances[idx] ) );
			v = Add( v, elevationPush );
			surface.vertices[idx] = v;
		}
	}

	// Two triangles per grid cell (row-major indices).
	surface.triangles.reserve( static_cast<std::size_t>( 2 ) * ( side - 1 ) * ( side - 1 ) );
	for ( int r = 0; r < side - 1; ++r )
	{
		for ( int col = 0; col < side - 1; ++col )
		{
			const int v00 = r * side + col;
			const int v10 = r * side + ( col + 1 );
			const int v01 = ( r + 1 ) * side + col;
			const int v11 = ( r + 1 ) * side + ( col + 1 );
			surface.triangles.push_back( { v00, v10, v11 } );
			surface.triangles.push_back( { v00, v11, v01 } );
		}
	}

	// Smooth per-vertex normals for lighting: accumulate each triangle's
	// (area-weighted) face normal into its three vertices, then normalize. Winding
	// follows the corner order passed in, so normals face the same way as the tris.
	surface.vertexNormals.assign( surface.vertices.size(), Vec3d() );
	for ( const std::array<int, 3> &tri : surface.triangles )
	{
		const Vec3d &p0 = surface.vertices[static_cast<std::size_t>( tri[0] )];
		const Vec3d &p1 = surface.vertices[static_cast<std::size_t>( tri[1] )];
		const Vec3d &p2 = surface.vertices[static_cast<std::size_t>( tri[2] )];
		const Vec3d faceN = Cross( Sub( p1, p0 ), Sub( p2, p0 ) );
		for ( int k = 0; k < 3; ++k )
		{
			const std::size_t idx = static_cast<std::size_t>( tri[static_cast<std::size_t>( k )] );
			surface.vertexNormals[idx] = Add( surface.vertexNormals[idx], faceN );
		}
	}
	for ( Vec3d &n : surface.vertexNormals )
	{
		const double len = std::sqrt( n.x * n.x + n.y * n.y + n.z * n.z );
		if ( len > 1e-12 )
		{
			n = Scale( n, 1.0 / len );
		}
	}

	return surface;
}

} // namespace hammer::geometry
