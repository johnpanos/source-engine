//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the entity property-sheet model. See
//			public/hammer/formats/entity_property_sheet.h.
//
//=============================================================================//

#include "hammer/formats/entity_property_sheet.h"

#include <cctype>
#include <string>

namespace hammer::formats
{

namespace
{

std::string Lower( std::string s )
{
	for ( char &c : s )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	return s;
}

// Case-insensitive lookup of a key's value among an entity's top-level pairs.
const std::string *FindKeyCI( const KeyValueNode &entity, const std::string &loweredKey )
{
	for ( const KeyValue &kv : entity.pairs )
	{
		if ( Lower( kv.key ) == loweredKey )
		{
			return &kv.value;
		}
	}
	return nullptr;
}

} // namespace

EntityPropertySheet BuildPropertySheet(
    const KeyValueNode &entity, const std::vector<EntityClass> &fgdClasses )
{
	EntityPropertySheet sheet;
	if ( const std::string *cls = entity.Find( "classname" ) )
	{
		sheet.classname = *cls;
	}

	std::vector<std::string> schemaKeysLower; // to skip these in the extra pass

	if ( auto resolved = ResolveClass( fgdClasses, sheet.classname ) )
	{
		sheet.classInFgd = true;
		for ( const FgdProperty &prop : resolved->properties )
		{
			PropertyRow row;
			row.key = prop.name;
			row.type = prop.type;
			row.displayName = prop.displayName.empty() ? prop.name : prop.displayName;
			row.choices = prop.choices;
			row.inSchema = true;

			const std::string *set = FindKeyCI( entity, Lower( prop.name ) );
			row.isSet = set != nullptr;
			row.value = set != nullptr ? *set : prop.defaultValue;

			schemaKeysLower.push_back( Lower( prop.name ) );
			sheet.rows.push_back( std::move( row ) );
		}
	}

	// Extra keys the entity carries that the schema does not declare. Meta keys
	// ("classname", "id") are not shown; the "editor" block is a child, not a pair.
	for ( const KeyValue &kv : entity.pairs )
	{
		const std::string lkey = Lower( kv.key );
		if ( lkey == "classname" || lkey == "id" )
		{
			continue;
		}
		bool inSchema = false;
		for ( const std::string &s : schemaKeysLower )
		{
			if ( s == lkey )
			{
				inSchema = true;
				break;
			}
		}
		if ( inSchema )
		{
			continue;
		}
		PropertyRow row;
		row.key = kv.key;
		row.type = "string";
		row.displayName = kv.key;
		row.value = kv.value;
		row.isSet = true;
		row.inSchema = false;
		sheet.rows.push_back( std::move( row ) );
	}

	return sheet;
}

} // namespace hammer::formats
