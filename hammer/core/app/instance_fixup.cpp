//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/instance_fixup.h.
//
//=============================================================================//

#include "hammer/app/instance_fixup.h"

#include <cctype>

namespace hammer::app
{

namespace
{

char LowerChar( char c )
{
	return static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
}

bool StartsWithNoCase( std::string_view text, std::string_view prefix )
{
	if ( text.size() < prefix.size() )
	{
		return false;
	}
	for ( std::size_t i = 0; i < prefix.size(); ++i )
	{
		if ( LowerChar( text[i] ) != LowerChar( prefix[i] ) )
		{
			return false;
		}
	}
	return true;
}

// The first case-insensitive occurrence of 'match' in 'text' at or after
// 'from' (V_stristr), or npos.
std::size_t FindNoCase( std::string_view text, std::string_view match, std::size_t from )
{
	if ( match.empty() || text.size() < match.size() )
	{
		return std::string_view::npos;
	}
	for ( std::size_t i = from; i + match.size() <= text.size(); ++i )
	{
		if ( StartsWithNoCase( text.substr( i ), match ) )
		{
			return i;
		}
	}
	return std::string_view::npos;
}

} // namespace

std::optional<InstanceFixupStyle> ParseFixupStyle( std::string_view value )
{
	if ( value.empty() || value == "0" )
	{
		return InstanceFixupStyle::Prefix;
	}
	if ( value == "1" )
	{
		return InstanceFixupStyle::Postfix;
	}
	if ( value == "2" )
	{
		return InstanceFixupStyle::None;
	}
	return std::nullopt;
}

bool IsGlobalInstanceName( std::string_view name )
{
	return name.empty() || name.front() == '@' || name.front() == '!';
}

std::string FixupInstanceName(
    std::string_view name, std::string_view fixup, InstanceFixupStyle style )
{
	if ( IsGlobalInstanceName( name ) )
	{
		return std::string( name );
	}
	switch ( style )
	{
	case InstanceFixupStyle::Prefix:
		return std::string( fixup ) + "-" + std::string( name );
	case InstanceFixupStyle::Postfix:
		return std::string( name ) + "-" + std::string( fixup );
	case InstanceFixupStyle::None:
		break;
	}
	return std::string( name );
}

std::vector<InstanceParameter> InstanceParameters( const std::vector<kvtext::KeyValue> &keys )
{
	std::vector<InstanceParameter> out;
	for ( const kvtext::KeyValue &kv : keys )
	{
		if ( !StartsWithNoCase( kv.key, "replace" ) )
		{
			continue;
		}
		const std::size_t space = kv.value.find( ' ' );
		if ( space == std::string::npos || space == 0 )
		{
			continue; // no value, or no variable to match
		}
		out.push_back( { kv.value.substr( 0, space ), kv.value.substr( space + 1 ) } );
	}
	return out;
}

std::string SubstituteInstanceParameters(
    std::string_view text, const std::vector<InstanceParameter> &parameters )
{
	std::string current( text );
	for ( const InstanceParameter &p : parameters )
	{
		std::string next;
		std::size_t from = 0;
		for ( ;; )
		{
			const std::size_t at = FindNoCase( current, p.variable, from );
			if ( at == std::string::npos )
			{
				next.append( current, from, std::string::npos );
				break;
			}
			next.append( current, from, at - from );
			next += p.value;
			from = at + p.variable.size();
		}
		current = std::move( next );
	}
	return current;
}

} // namespace hammer::app
