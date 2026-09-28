//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/scene/map_objects.h.
//
//=============================================================================//

#include "hammer/scene/map_objects.h"

#include "mapgeometry/vec3.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace hammer::scene
{

using mapgeometry::Vec3d;

namespace
{

std::optional<double> ParseDouble( std::string_view text )
{
	std::string owned( text );
	const char *begin = owned.c_str();
	while ( *begin == ' ' || *begin == '\t' )
	{
		++begin;
	}
	if ( *begin == '\0' )
	{
		return std::nullopt;
	}
	char *end = nullptr;
	const double v = std::strtod( begin, &end );
	while ( end && ( *end == ' ' || *end == '\t' ) )
	{
		++end;
	}
	if ( !end || *end != '\0' || !std::isfinite( v ) )
	{
		return std::nullopt;
	}
	return v;
}

std::optional<Vec3d> ParseTriple( const std::string *text )
{
	if ( !text )
	{
		return std::nullopt;
	}
	double a = 0.0;
	double b = 0.0;
	double c = 0.0;
	char trailing = 0;
	if ( std::sscanf( text->c_str(), "%lf %lf %lf %c", &a, &b, &c, &trailing ) != 3 )
	{
		return std::nullopt;
	}
	if ( !std::isfinite( a ) || !std::isfinite( b ) || !std::isfinite( c ) )
	{
		return std::nullopt;
	}
	return Vec3d( a, b, c );
}

} // namespace

mapgeometry::Plane Side::Plane() const
{
	const Vec3d n = mapgeometry::Normalize(
	    mapgeometry::Cross( points[0] - points[1], points[2] - points[1] ) );
	mapgeometry::Plane plane;
	plane.normal = n;
	plane.dist = mapgeometry::Dot( points[0], n );
	return plane;
}

const Side *Solid::FindSide( std::uint32_t sideVmfId ) const
{
	for ( const Side &side : sides )
	{
		if ( side.vmfId == sideVmfId )
		{
			return &side;
		}
	}
	return nullptr;
}

Side *Solid::FindSide( std::uint32_t sideVmfId )
{
	return const_cast<Side *>( static_cast<const Solid *>( this )->FindSide( sideVmfId ) );
}

std::optional<Connection> ParseConnection( std::string_view output, std::string_view value )
{
	// The newer format separates with ESC (0x1B) so parameters may hold commas.
	const char separator = value.find( '\x1b' ) != std::string_view::npos ? '\x1b' : ',';
	std::vector<std::string_view> fields;
	std::size_t start = 0;
	for ( ;; )
	{
		const std::size_t at = value.find( separator, start );
		if ( at == std::string_view::npos )
		{
			fields.push_back( value.substr( start ) );
			break;
		}
		fields.push_back( value.substr( start, at - start ) );
		start = at + 1;
	}
	if ( fields.size() != 5 )
	{
		return std::nullopt;
	}
	const std::optional<double> delay = ParseDouble( fields[3] );
	const std::optional<double> times = ParseDouble( fields[4] );
	if ( !delay || !times || *times != std::floor( *times ) )
	{
		return std::nullopt;
	}
	Connection c;
	c.output = std::string( output );
	c.target = std::string( fields[0] );
	c.input = std::string( fields[1] );
	c.parameter = std::string( fields[2] );
	c.delay = *delay;
	c.timesToFire = static_cast<int>( *times );
	c.separator = separator;
	return c;
}

std::string FormatConnectionValue( const Connection &c )
{
	std::string out = c.target;
	out += c.separator;
	out += c.input;
	out += c.separator;
	out += c.parameter;
	out += c.separator;
	out += FormatNumber( c.delay );
	out += c.separator;
	out += std::to_string( c.timesToFire );
	return out;
}

const std::string *Entity::Key( std::string_view key ) const
{
	for ( const kvtext::KeyValue &kv : keys )
	{
		if ( kv.key == key )
		{
			return &kv.value;
		}
	}
	return nullptr;
}

bool Entity::SetKey( std::string_view key, std::string_view value )
{
	for ( kvtext::KeyValue &kv : keys )
	{
		if ( kv.key == key )
		{
			if ( kv.value == value )
			{
				return false;
			}
			kv.value = std::string( value );
			return true;
		}
	}
	keys.push_back( { std::string( key ), std::string( value ) } );
	return true;
}

bool Entity::RemoveKey( std::string_view key )
{
	const std::size_t before = keys.size();
	std::erase_if( keys, [&]( const kvtext::KeyValue &kv ) { return kv.key == key; } );
	return keys.size() != before;
}

std::optional<Vec3d> Entity::Origin() const
{
	return ParseTriple( Key( "origin" ) );
}

void Entity::SetOrigin( const Vec3d &origin )
{
	SetKey( "origin", FormatVec3( origin ) );
}

std::optional<Vec3d> Entity::Angles() const
{
	return ParseTriple( Key( "angles" ) );
}

void Entity::SetAngles( const Vec3d &pitchYawRoll )
{
	SetKey( "angles", FormatVec3( pitchYawRoll ) );
}

std::string_view Entity::Name() const
{
	const std::string *name = Key( "targetname" );
	return name ? std::string_view( *name ) : std::string_view();
}

std::string FormatNumber( double value )
{
	if ( value == 0.0 )
	{
		return "0"; // also folds negative zero
	}
	char buf[64];
	std::snprintf( buf, sizeof( buf ), "%.10g", value );
	return buf;
}

std::string FormatVec3( const Vec3d &v )
{
	return FormatNumber( v.x ) + " " + FormatNumber( v.y ) + " " + FormatNumber( v.z );
}

} // namespace hammer::scene
