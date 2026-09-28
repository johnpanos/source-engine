//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared helpers of the entity catalog port. See
//			public/hammer/ports/entity_catalog.h.
//
//=============================================================================//

#include "hammer/ports/entity_catalog.h"

#include <cctype>

namespace hammer::ports
{

namespace
{

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	if ( a.size() != b.size() )
	{
		return false;
	}
	for ( std::size_t i = 0; i < a.size(); ++i )
	{
		if ( std::tolower( static_cast<unsigned char>( a[i] ) ) !=
		     std::tolower( static_cast<unsigned char>( b[i] ) ) )
		{
			return false;
		}
	}
	return true;
}

} // namespace

KeyType KeyTypeFromName( std::string_view typeName )
{
	struct Entry
	{
		const char *name;
		KeyType type;
	};
	static const Entry kTypes[] = {
	    { "string", KeyType::String },
	    { "integer", KeyType::Integer },
	    { "float", KeyType::Float },
	    { "boolean", KeyType::Boolean },
	    { "choices", KeyType::Choices },
	    { "flags", KeyType::Flags },
	    { "target_source", KeyType::TargetSource },
	    { "target_destination", KeyType::TargetDestination },
	    { "target_name_or_class", KeyType::TargetDestination },
	    { "color255", KeyType::Color255 },
	    { "color1", KeyType::Color1 },
	    { "angle", KeyType::Angle },
	    { "vector", KeyType::Vector },
	    { "origin", KeyType::Vector },
	    { "vecline", KeyType::Vector },
	    { "studio", KeyType::Studio },
	    { "sprite", KeyType::Sprite },
	    { "sound", KeyType::Sound },
	    { "material", KeyType::Material },
	    { "decal", KeyType::Decal },
	    { "scene", KeyType::Scene },
	    { "filterclass", KeyType::FilterClass },
	    { "sidelist", KeyType::SideList },
	};
	for ( const Entry &e : kTypes )
	{
		if ( EqualsNoCase( typeName, e.name ) )
		{
			return e.type;
		}
	}
	return KeyType::Other;
}

const KeyDefinition *EntityClassInfo::FindKey( std::string_view key ) const
{
	for ( const KeyDefinition &k : keys )
	{
		if ( EqualsNoCase( k.key, key ) )
		{
			return &k;
		}
	}
	return nullptr;
}

bool EntityClassInfo::HasInput( std::string_view name ) const
{
	for ( const IoDefinition &io : inputs )
	{
		if ( EqualsNoCase( io.name, name ) )
		{
			return true;
		}
	}
	return false;
}

bool EntityClassInfo::HasOutput( std::string_view name ) const
{
	for ( const IoDefinition &io : outputs )
	{
		if ( EqualsNoCase( io.name, name ) )
		{
			return true;
		}
	}
	return false;
}

} // namespace hammer::ports
