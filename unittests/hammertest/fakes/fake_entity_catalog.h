//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: In-memory hammer::ports::IEntityCatalog for suites of modules that
//			consume the port (hammer.app, hammer.viewport, hammer.presenters).
//			Classes are stored already resolved (the fake has no base classes),
//			keyed by lower-cased name, so iteration order is the port's
//			case-insensitive order and Find's pointers stay stable while more
//			classes are added. Adding a class with an existing name (any case)
//			replaces it.
//
//			Usage:
//				hammertest::FakeEntityCatalog catalog;
//				using C = hammertest::FakeEntityCatalog;
//				catalog.AddPoint( "info_player_start", { C::Key( "angles", "angle", "0 0 0" ) } )
//				    .Box( { -16, -16, 0 }, { 16, 16, 72 } );
//				catalog.AddSolid( "func_door", { C::Key( "speed", "float", "100" ) },
//				    { C::Io( "Open" ) }, { C::Io( "OnOpen" ) } );
//
//			Links hammer/core/ports/entity_catalog.cpp (KeyTypeFromName, FindKey).
//
//=============================================================================//

#ifndef HAMMERTEST_FAKE_ENTITY_CATALOG_H
#define HAMMERTEST_FAKE_ENTITY_CATALOG_H

#include "hammer/ports/entity_catalog.h"

#include <cctype>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace hammertest
{

class FakeEntityCatalog final : public hammer::ports::IEntityCatalog
{
public:
	using EntityClassInfo = hammer::ports::EntityClassInfo;
	using EntityClassKind = hammer::ports::EntityClassKind;
	using KeyDefinition = hammer::ports::KeyDefinition;
	using IoDefinition = hammer::ports::IoDefinition;
	using Vec3d = mapgeometry::Vec3d;

	// A key of FGD type word 'typeName' ("string", "flags", "target_destination").
	static KeyDefinition Key(
	    std::string key, std::string typeName = "string", std::string defaultValue = std::string() )
	{
		KeyDefinition k;
		k.key = std::move( key );
		k.type = hammer::ports::KeyTypeFromName( typeName );
		k.typeName = std::move( typeName );
		k.defaultValue = std::move( defaultValue );
		return k;
	}

	static IoDefinition Io( std::string name, std::string type = "void" )
	{
		IoDefinition io;
		io.name = std::move( name );
		io.type = std::move( type );
		return io;
	}

	// Adds (or replaces) a class; later calls to Box/Color/Model/Sprite/
	// Description apply to it.
	FakeEntityCatalog &Add( EntityClassInfo info )
	{
		m_lastKey = Lower( info.name );
		m_classes[m_lastKey] = std::move( info );
		return *this;
	}

	FakeEntityCatalog &AddPoint( std::string name, std::vector<KeyDefinition> keys = {},
	    std::vector<IoDefinition> inputs = {}, std::vector<IoDefinition> outputs = {} )
	{
		return AddKind( EntityClassKind::Point, std::move( name ), std::move( keys ),
		    std::move( inputs ), std::move( outputs ) );
	}

	FakeEntityCatalog &AddSolid( std::string name, std::vector<KeyDefinition> keys = {},
	    std::vector<IoDefinition> inputs = {}, std::vector<IoDefinition> outputs = {} )
	{
		return AddKind( EntityClassKind::Solid, std::move( name ), std::move( keys ),
		    std::move( inputs ), std::move( outputs ) );
	}

	FakeEntityCatalog &AddKind( EntityClassKind kind, std::string name,
	    std::vector<KeyDefinition> keys = {}, std::vector<IoDefinition> inputs = {},
	    std::vector<IoDefinition> outputs = {} )
	{
		EntityClassInfo info;
		info.name = std::move( name );
		info.kind = kind;
		info.keys = std::move( keys );
		info.inputs = std::move( inputs );
		info.outputs = std::move( outputs );
		return Add( std::move( info ) );
	}

	FakeEntityCatalog &Box( Vec3d mins, Vec3d maxs )
	{
		Last().boxMins = mins;
		Last().boxMaxs = maxs;
		return *this;
	}
	FakeEntityCatalog &Color( Vec3d color )
	{
		Last().color = color;
		return *this;
	}
	FakeEntityCatalog &Model( std::string model )
	{
		Last().model = std::move( model );
		return *this;
	}
	FakeEntityCatalog &Sprite( std::string sprite )
	{
		Last().sprite = std::move( sprite );
		return *this;
	}
	FakeEntityCatalog &Description( std::string description )
	{
		Last().description = std::move( description );
		return *this;
	}

	const EntityClassInfo *Find( std::string_view name ) const override
	{
		const auto it = m_classes.find( Lower( name ) );
		return it == m_classes.end() ? nullptr : &it->second;
	}

	std::vector<std::string> ClassNames() const override
	{
		std::vector<std::string> names;
		for ( const auto &entry : m_classes )
		{
			names.push_back( entry.second.name );
		}
		return names;
	}

private:
	EntityClassInfo &Last() { return m_classes.at( m_lastKey ); } // throws before any Add

	static std::string Lower( std::string_view text )
	{
		std::string out( text );
		for ( char &c : out )
		{
			c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
		}
		return out;
	}

	std::map<std::string, EntityClassInfo> m_classes; // lower-cased name -> class
	std::string m_lastKey;                            // the class the fluent setters apply to
};

} // namespace hammertest

#endif // HAMMERTEST_FAKE_ENTITY_CATALOG_H
