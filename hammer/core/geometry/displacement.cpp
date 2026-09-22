//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::geometry displacement parsing and surface
//			construction. See public/hammer/geometry/displacement.h.
//
//=============================================================================//

#include "hammer/geometry/displacement.h"

#include <cmath>
#include <cstddef>
#include <sstream>
#include <string>

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

// Parses a whitespace-separated list of doubles. Returns false if any token is
// not a number; a well-formed empty string yields an empty vector.
bool ParseDoubles( const std::string &text, std::vector<double> &out )
{
	out.clear();
	std::istringstream in( text );
	double value = 0.0;
	while ( in >> value )
	{
		out.push_back( value );
	}
	// Trailing non-numeric content (anything left that is not whitespace) is an error.
	if ( in.eof() )
	{
		return true;
	}
	// Clear the fail bit set by the last extraction and check for leftover tokens.
	in.clear();
	std::string leftover;
	in >> leftover;
	return leftover.empty();
}

// Finds a child block by name (first match), or nullptr.
const formats::KeyValueNode *FindChild( const formats::KeyValueNode &node, const char *name )
{
	for ( const formats::KeyValueNode &child : node.children )
	{
		if ( child.name == name )
		{
			return &child;
		}
	}
	return nullptr;
}

// Reads a "row0".."row{side-1}" grid of scalars into a flat row-major vector of
// length side*side. Returns false on a missing row, a wrong row count, or a row
// whose length != side.
bool ReadScalarGrid( const formats::KeyValueNode &block, int side, std::vector<double> &out )
{
	out.assign( static_cast<std::size_t>( side ) * side, 0.0 );
	int rowsSeen = 0;
	for ( int r = 0; r < side; ++r )
	{
		const std::string key = "row" + std::to_string( r );
		const std::string *value = block.Find( key );
		if ( value == nullptr )
		{
			return false;
		}
		std::vector<double> row;
		if ( !ParseDoubles( *value, row ) || static_cast<int>( row.size() ) != side )
		{
			return false;
		}
		for ( int c = 0; c < side; ++c )
		{
			out[static_cast<std::size_t>( r ) * side + c] = row[static_cast<std::size_t>( c )];
		}
		++rowsSeen;
	}
	// Reject an extra "row<side>" the grid should not have.
	if ( block.Find( "row" + std::to_string( side ) ) != nullptr )
	{
		return false;
	}
	return rowsSeen == side;
}

// Reads a "row0".. grid of Vec3d (three scalars per entry) into row-major order.
bool ReadVectorGrid( const formats::KeyValueNode &block, int side, std::vector<Vec3d> &out )
{
	out.assign( static_cast<std::size_t>( side ) * side, Vec3d() );
	for ( int r = 0; r < side; ++r )
	{
		const std::string key = "row" + std::to_string( r );
		const std::string *value = block.Find( key );
		if ( value == nullptr )
		{
			return false;
		}
		std::vector<double> row;
		if ( !ParseDoubles( *value, row ) || static_cast<int>( row.size() ) != side * 3 )
		{
			return false;
		}
		for ( int c = 0; c < side; ++c )
		{
			const std::size_t base = static_cast<std::size_t>( c ) * 3;
			out[static_cast<std::size_t>( r ) * side + c] =
			    Vec3d( row[base + 0], row[base + 1], row[base + 2] );
		}
	}
	if ( block.Find( "row" + std::to_string( side ) ) != nullptr )
	{
		return false;
	}
	return true;
}

// Reads the "triangle_tags" grid: (side-1) rows keyed row0..row{side-2}, each
// holding (side-1)*2 integer tags (two triangles per grid cell). Flattened
// row-major into 2*(side-1)^2 entries. Returns false on a missing/extra row or a
// wrong-length row.
bool ReadTagRows( const formats::KeyValueNode &block, int side, std::vector<int> &out )
{
	const int rows = side - 1;
	const int perRow = ( side - 1 ) * 2;
	out.assign( static_cast<std::size_t>( rows ) * perRow, 0 );
	for ( int r = 0; r < rows; ++r )
	{
		const std::string key = "row" + std::to_string( r );
		const std::string *value = block.Find( key );
		if ( value == nullptr )
		{
			return false;
		}
		std::vector<double> row;
		if ( !ParseDoubles( *value, row ) || static_cast<int>( row.size() ) != perRow )
		{
			return false;
		}
		for ( int c = 0; c < perRow; ++c )
		{
			out[static_cast<std::size_t>( r ) * perRow + c] = static_cast<int>( row[c] );
		}
	}
	if ( block.Find( "row" + std::to_string( rows ) ) != nullptr )
	{
		return false;
	}
	return true;
}

// Parses "x y z" (VMF sometimes wraps in brackets). Returns false when not three
// numbers.
bool ParseVec3( const std::string &text, Vec3d &out )
{
	std::string cleaned = text;
	for ( char &ch : cleaned )
	{
		if ( ch == '[' || ch == ']' || ch == '(' || ch == ')' )
		{
			ch = ' ';
		}
	}
	std::vector<double> vals;
	if ( !ParseDoubles( cleaned, vals ) || vals.size() != 3 )
	{
		return false;
	}
	out = Vec3d( vals[0], vals[1], vals[2] );
	return true;
}

} // namespace

std::optional<DispInfo> ParseDispInfo( const formats::KeyValueNode &dispBlock )
{
	DispInfo info;

	const std::string *powerStr = dispBlock.Find( "power" );
	if ( powerStr == nullptr )
	{
		return std::nullopt;
	}
	{
		std::vector<double> vals;
		if ( !ParseDoubles( *powerStr, vals ) || vals.size() != 1 )
		{
			return std::nullopt;
		}
		info.power = static_cast<int>( vals[0] );
	}
	// Source supports powers 2..4; accept 1..4 so the smallest useful grid is legal.
	if ( info.power < 1 || info.power > 4 )
	{
		return std::nullopt;
	}
	const int side = info.Side();

	const std::string *startStr = dispBlock.Find( "startposition" );
	if ( startStr == nullptr || !ParseVec3( *startStr, info.startPosition ) )
	{
		return std::nullopt;
	}

	if ( const std::string *elev = dispBlock.Find( "elevation" ) )
	{
		std::vector<double> vals;
		if ( !ParseDoubles( *elev, vals ) || vals.size() != 1 )
		{
			return std::nullopt;
		}
		info.elevation = vals[0];
	}

	const formats::KeyValueNode *normals = FindChild( dispBlock, "normals" );
	const formats::KeyValueNode *distances = FindChild( dispBlock, "distances" );
	if ( normals == nullptr || distances == nullptr )
	{
		return std::nullopt;
	}
	if ( !ReadVectorGrid( *normals, side, info.normals ) )
	{
		return std::nullopt;
	}
	if ( !ReadScalarGrid( *distances, side, info.distances ) )
	{
		return std::nullopt;
	}

	// Offsets are optional; absent means all-zero.
	if ( const formats::KeyValueNode *offsets = FindChild( dispBlock, "offsets" ) )
	{
		if ( !ReadVectorGrid( *offsets, side, info.offsets ) )
		{
			return std::nullopt;
		}
	}
	else
	{
		info.offsets.assign( static_cast<std::size_t>( side ) * side, Vec3d() );
	}

	// Alphas are optional (per-vertex blend weight 0..255); absent means all-zero.
	if ( const formats::KeyValueNode *alphas = FindChild( dispBlock, "alphas" ) )
	{
		if ( !ReadScalarGrid( *alphas, side, info.alphas ) )
		{
			return std::nullopt;
		}
	}
	else
	{
		info.alphas.assign( static_cast<std::size_t>( side ) * side, 0.0 );
	}

	// subdiv flag is optional.
	if ( const std::string *sub = dispBlock.Find( "subdiv" ) )
	{
		std::vector<double> vals;
		if ( !ParseDoubles( *sub, vals ) || vals.size() != 1 )
		{
			return std::nullopt;
		}
		info.subdiv = static_cast<int>( vals[0] );
	}

	// triangle_tags is optional; when present it must be a full (side-1) x (side-1)*2 grid.
	if ( const formats::KeyValueNode *tags = FindChild( dispBlock, "triangle_tags" ) )
	{
		if ( !ReadTagRows( *tags, side, info.triangleTags ) )
		{
			return std::nullopt;
		}
	}

	return info;
}

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

	return surface;
}

} // namespace hammer::geometry
