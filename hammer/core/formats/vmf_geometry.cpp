//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the hammer::formats VMF -> geometry decoders. See
//			public/hammer/formats/vmf_geometry.h.
//
//=============================================================================//

#include "hammer/formats/vmf_geometry.h"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace hammer::formats
{

namespace
{

using geometry::BrushFace;
using geometry::BrushSolid;
using geometry::DispInfo;
using geometry::DisplacementMesh;
using geometry::DisplacementSurface;
using geometry::Plane;
using geometry::SceneEntity;
using geometry::Vec3d;
using geometry::WorldScene;

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
const KeyValueNode *FindChild( const KeyValueNode &node, const char *name )
{
	for ( const KeyValueNode &child : node.children )
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
bool ReadScalarGrid( const KeyValueNode &block, int side, std::vector<double> &out )
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
bool ReadVectorGrid( const KeyValueNode &block, int side, std::vector<Vec3d> &out )
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
bool ReadTagRows( const KeyValueNode &block, int side, std::vector<int> &out )
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

// Parses a dispinfo "x y z" value (VMF sometimes wraps it in brackets). Returns
// false when not three numbers. Unlike geometry::ParseVec3 it tolerates bracket
// characters.
bool ParseBracketedVec3( const std::string &text, Vec3d &out )
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

void ImportSolids( const KeyValueNode &container, WorldScene &scene, std::size_t &solidCount )
{
	for ( const KeyValueNode &child : container.children )
	{
		if ( child.name != "solid" )
		{
			continue;
		}
		++solidCount;
		BrushSolid solid = BuildSolidFromBlock( child );
		if ( solid.faces.empty() )
		{
			continue;
		}
		scene.AddSolid( std::move( solid ) );
	}
}

// Imports any displaced (dispinfo) faces of the container's solids into the scene
// as renderable DisplacementMesh surfaces, via the hammer.geometry.displacement
// core. A displacement side's face must be a quad; malformed dispinfo is skipped.
void ImportDisplacements( const KeyValueNode &container, WorldScene &scene )
{
	for ( const KeyValueNode &solidBlock : container.children )
	{
		if ( solidBlock.name != "solid" )
		{
			continue;
		}
		// Cheap pre-check: does any side carry a dispinfo child block?
		bool anyDisp = false;
		for ( const KeyValueNode &side : solidBlock.children )
		{
			if ( side.name != "side" )
			{
				continue;
			}
			for ( const KeyValueNode &sc : side.children )
			{
				if ( sc.name == "dispinfo" )
				{
					anyDisp = true;
					break;
				}
			}
			if ( anyDisp )
			{
				break;
			}
		}
		if ( !anyDisp )
		{
			continue;
		}

		const BrushSolid solid = BuildSolidFromBlock( solidBlock );
		if ( solid.faces.empty() )
		{
			continue;
		}

		for ( const KeyValueNode &side : solidBlock.children )
		{
			if ( side.name != "side" )
			{
				continue;
			}
			const KeyValueNode *dispBlock = nullptr;
			for ( const KeyValueNode &sc : side.children )
			{
				if ( sc.name == "dispinfo" )
				{
					dispBlock = &sc;
					break;
				}
			}
			if ( !dispBlock )
			{
				continue;
			}
			const std::string *planeStr = side.Find( "plane" );
			if ( !planeStr )
			{
				continue;
			}
			const std::optional<std::array<Vec3d, 3>> pts = geometry::ParsePlanePoints( *planeStr );
			if ( !pts )
			{
				continue;
			}
			const std::optional<Plane> sidePlane =
			    geometry::PlaneFromPoints( ( *pts )[0], ( *pts )[1], ( *pts )[2] );
			if ( !sidePlane )
			{
				continue;
			}
			const BrushFace *face = geometry::FindFaceOnPlane( solid, *sidePlane );
			if ( !face || face->vertices.size() != 4 )
			{
				continue; // a displacement side must resolve to a quad face
			}
			const std::optional<DispInfo> disp = ParseDispInfo( *dispBlock );
			if ( !disp )
			{
				continue;
			}
			const std::array<Vec3d, 4> corners = {
			    face->vertices[0], face->vertices[1], face->vertices[2], face->vertices[3] };
			const DisplacementSurface surf =
			    geometry::BuildDisplacementSurface( corners, face->plane.normal, *disp );

			DisplacementMesh mesh;
			mesh.vertices = surf.vertices;
			mesh.alphas = surf.vertexAlphas;
			mesh.triangles = surf.triangles;
			scene.AddDisplacement( std::move( mesh ) );
		}
	}
}

} // namespace

BrushSolid BuildSolidFromBlock( const KeyValueNode &solidBlock )
{
	std::vector<Plane> planes;
	std::vector<std::string> materials;

	int id = 0;
	if ( const std::string *idStr = solidBlock.Find( "id" ) )
	{
		id = std::atoi( idStr->c_str() );
	}

	for ( const KeyValueNode &child : solidBlock.children )
	{
		if ( child.name != "side" )
		{
			continue;
		}
		const std::string *planeStr = child.Find( "plane" );
		if ( !planeStr )
		{
			continue;
		}
		const std::optional<std::array<Vec3d, 3>> pts = geometry::ParsePlanePoints( *planeStr );
		if ( !pts )
		{
			continue;
		}
		const std::optional<Plane> plane =
		    geometry::PlaneFromPoints( ( *pts )[0], ( *pts )[1], ( *pts )[2] );
		if ( !plane )
		{
			continue;
		}
		planes.push_back( *plane );
		const std::string *mat = child.Find( "material" );
		materials.push_back( mat ? *mat : std::string() );
	}

	return geometry::BuildSolidFromPlanes( planes, materials, id );
}

WorldScene BuildSceneFromDocument( const KeyValueNode &root )
{
	WorldScene scene;

	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name == "world" )
		{
			std::size_t ignored = 0;
			ImportSolids( block, scene, ignored );
			ImportDisplacements( block, scene );
		}
		else if ( block.name == "entity" )
		{
			SceneEntity entity;
			if ( const std::string *cls = block.Find( "classname" ) )
			{
				entity.classname = *cls;
			}
			if ( const std::string *name = block.Find( "targetname" ) )
			{
				entity.targetname = *name;
			}
			if ( const std::string *origin = block.Find( "origin" ) )
			{
				entity.origin = geometry::ParseVec3( *origin );
			}
			std::size_t solidCount = 0;
			ImportSolids( block, scene, solidCount );
			ImportDisplacements( block, scene );
			entity.solidCount = solidCount;
			scene.entities.push_back( std::move( entity ) );
		}
	}

	return scene;
}

std::optional<DispInfo> ParseDispInfo( const KeyValueNode &dispBlock )
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
	if ( startStr == nullptr || !ParseBracketedVec3( *startStr, info.startPosition ) )
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

	const KeyValueNode *normals = FindChild( dispBlock, "normals" );
	const KeyValueNode *distances = FindChild( dispBlock, "distances" );
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
	if ( const KeyValueNode *offsets = FindChild( dispBlock, "offsets" ) )
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
	if ( const KeyValueNode *alphas = FindChild( dispBlock, "alphas" ) )
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
	if ( const KeyValueNode *tags = FindChild( dispBlock, "triangle_tags" ) )
	{
		if ( !ReadTagRows( *tags, side, info.triangleTags ) )
		{
			return std::nullopt;
		}
	}

	return info;
}

} // namespace hammer::formats
