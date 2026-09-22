//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats overlay (info_overlay) parsing. See
//			public/hammer/formats/overlay.h.
//
//=============================================================================//

#include "hammer/formats/overlay.h"

#include <sstream>
#include <string>
#include <vector>

namespace hammer::formats
{

namespace
{

using geometry::Vec3d;

bool ParseDoubles( const std::string &text, std::vector<double> &out )
{
	out.clear();
	std::string cleaned = text;
	for ( char &ch : cleaned )
	{
		if ( ch == '[' || ch == ']' || ch == '(' || ch == ')' )
		{
			ch = ' ';
		}
	}
	std::istringstream in( cleaned );
	double value = 0.0;
	while ( in >> value )
	{
		out.push_back( value );
	}
	return true;
}

bool ParseVec3( const std::string *text, Vec3d &out )
{
	if ( text == nullptr )
	{
		return false;
	}
	std::vector<double> vals;
	ParseDoubles( *text, vals );
	if ( vals.size() < 3 )
	{
		return false;
	}
	out = Vec3d( vals[0], vals[1], vals[2] );
	return true;
}

bool ParseScalar( const std::string *text, double &out )
{
	if ( text == nullptr )
	{
		return false;
	}
	std::vector<double> vals;
	ParseDoubles( *text, vals );
	if ( vals.size() != 1 )
	{
		return false;
	}
	out = vals[0];
	return true;
}

Vec3d Add( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.x + b.x, a.y + b.y, a.z + b.z );
}

Vec3d Scale( const Vec3d &a, double s )
{
	return Vec3d( a.x * s, a.y * s, a.z * s );
}

} // namespace

std::array<geometry::Vec3d, 4> Overlay::WorldCorners() const
{
	std::array<Vec3d, 4> corners;
	for ( int i = 0; i < 4; ++i )
	{
		const Vec3d &p = uv[static_cast<std::size_t>( i )];
		Vec3d world = Add( basisOrigin, Scale( basisU, p.x ) );
		world = Add( world, Scale( basisV, p.y ) );
		world = Add( world, Scale( basisNormal, p.z ) );
		corners[static_cast<std::size_t>( i )] = world;
	}
	return corners;
}

std::optional<Overlay> ParseOverlay( const KeyValueNode &entity )
{
	const std::string *cls = entity.Find( "classname" );
	if ( cls == nullptr || *cls != "info_overlay" )
	{
		return std::nullopt;
	}

	Overlay overlay;

	const std::string *material = entity.Find( "material" );
	if ( material == nullptr || material->empty() )
	{
		return std::nullopt;
	}
	overlay.material = *material;

	// "sides" is a space-separated list of face ids; at least one is required.
	const std::string *sides = entity.Find( "sides" );
	if ( sides == nullptr )
	{
		return std::nullopt;
	}
	{
		std::istringstream in( *sides );
		int id = 0;
		while ( in >> id )
		{
			overlay.sides.push_back( id );
		}
	}
	if ( overlay.sides.empty() )
	{
		return std::nullopt;
	}

	if ( !ParseVec3( entity.Find( "BasisOrigin" ), overlay.basisOrigin ) ||
	     !ParseVec3( entity.Find( "BasisU" ), overlay.basisU ) ||
	     !ParseVec3( entity.Find( "BasisV" ), overlay.basisV ) ||
	     !ParseVec3( entity.Find( "BasisNormal" ), overlay.basisNormal ) )
	{
		return std::nullopt;
	}

	const char *const uvKeys[4] = { "uv0", "uv1", "uv2", "uv3" };
	for ( int i = 0; i < 4; ++i )
	{
		if ( !ParseVec3( entity.Find( uvKeys[i] ), overlay.uv[static_cast<std::size_t>( i )] ) )
		{
			return std::nullopt;
		}
	}

	// Texture coordinate range is optional; defaults are 0..1.
	ParseScalar( entity.Find( "StartU" ), overlay.startU );
	ParseScalar( entity.Find( "EndU" ), overlay.endU );
	ParseScalar( entity.Find( "StartV" ), overlay.startV );
	ParseScalar( entity.Find( "EndV" ), overlay.endV );

	return overlay;
}

std::vector<Overlay> ExtractOverlays( const KeyValueNode &root )
{
	std::vector<Overlay> overlays;
	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name != "entity" )
		{
			continue;
		}
		if ( auto overlay = ParseOverlay( block ) )
		{
			overlays.push_back( std::move( *overlay ) );
		}
	}
	return overlays;
}

} // namespace hammer::formats
