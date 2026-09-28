//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the hammer::ports::IEntityCatalog
//			contract (ports.entity_catalog.v1). Every catalog that claims the
//			port runs this one function with the expectations its fixture
//			declares: the FGD-backed catalog and the test fake in
//			test_fgd_entity_catalog.cpp, and deliberately bad catalogs in
//			test_entity_catalog_negative.cpp, which it must flag.
//
//			Clauses checked:
//			  C1 ClassNames lists exactly the expected non-base classes, sorted
//			     case-insensitively without duplicates, and is repeatable.
//			  C2 Find is case-insensitive, returns the class of that name, and
//			     returns the same pointer every time (stable results).
//			  C3 Base classes and unknown names are neither listed nor found.
//			  C4 Each expected class has its kind and its resolved keys in order
//			     (bases first, an override keeping the base key's place), with
//			     no duplicate key and the expected (overriding) defaults.
//			  C5 FindKey, HasInput and HasOutput are case-insensitive and reject
//			     unknown names; inputs and outputs are exactly the expected set.
//
//			Case-insensitive comparison here is written independently of the
//			catalogs under test.
//
//=============================================================================//

#ifndef HAMMERTEST_ENTITY_CATALOG_CONFORMANCE_H
#define HAMMERTEST_ENTITY_CATALOG_CONFORMANCE_H

#include "hammer/ports/entity_catalog.h"
#include "testing/checks.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace hammertest
{

struct EntityCatalogExpectations
{
	struct ClassShape
	{
		std::string name;
		hammer::ports::EntityClassKind kind = hammer::ports::EntityClassKind::Point;
		std::vector<std::string> keys; // resolved order
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
	};
	struct KeyDefault
	{
		std::string className;
		std::string key;
		std::string defaultValue;
	};

	std::vector<std::string> classNames; // every non-base class, any order
	std::vector<std::string> baseClassNames;
	std::vector<std::string> unknownNames;
	std::vector<ClassShape> classes;
	std::vector<KeyDefault> defaults;
};

namespace conformance_detail
{

inline std::string Fold( std::string_view text, bool upper )
{
	std::string out( text );
	for ( char &c : out )
	{
		const unsigned char u = static_cast<unsigned char>( c );
		c = static_cast<char>( upper ? std::toupper( u ) : std::tolower( u ) );
	}
	return out;
}

inline bool SameNoCase( std::string_view a, std::string_view b )
{
	return Fold( a, false ) == Fold( b, false );
}

inline bool Before( std::string_view a, std::string_view b )
{
	return Fold( a, false ) < Fold( b, false );
}

inline std::vector<std::string> FoldedSorted( std::vector<std::string> names )
{
	for ( std::string &name : names )
	{
		name = Fold( name, false );
	}
	std::sort( names.begin(), names.end() );
	return names;
}

} // namespace conformance_detail

inline void RunEntityCatalogConformance( testing::Checks &checks,
    const hammer::ports::IEntityCatalog &catalog, const EntityCatalogExpectations &expect )
{
	using namespace conformance_detail;
	using hammer::ports::EntityClassInfo;
	using hammer::ports::KeyDefinition;

	// C1: the listing.
	const std::vector<std::string> names = catalog.ClassNames();
	bool sorted = true;
	for ( std::size_t i = 1; i < names.size(); ++i )
	{
		sorted = sorted && Before( names[i - 1], names[i] );
	}
	checks.That( sorted, "C1 ClassNames sorted case-insensitively, no duplicates" );
	checks.That( FoldedSorted( names ) == FoldedSorted( expect.classNames ),
	    "C1 ClassNames lists exactly the expected non-base classes" );
	checks.That( catalog.ClassNames() == names, "C1 ClassNames repeatable" );

	// C2: lookup of every listed class.
	for ( const std::string &name : names )
	{
		const EntityClassInfo *info = catalog.Find( name );
		checks.That( info != nullptr, "C2 listed class is found: " + name );
		if ( info == nullptr )
		{
			continue;
		}
		checks.That( SameNoCase( info->name, name ), "C2 Find returns the named class: " + name );
		checks.That( catalog.Find( Fold( name, true ) ) == info &&
		                 catalog.Find( Fold( name, false ) ) == info,
		    "C2 Find is case-insensitive and stable: " + name );
		checks.That( catalog.Find( name ) == info, "C2 repeated Find returns the same pointer" );
	}

	// C3: base classes and unknown names.
	for ( const std::string &base : expect.baseClassNames )
	{
		checks.That(
		    catalog.Find( base ) == nullptr && catalog.Find( Fold( base, true ) ) == nullptr,
		    "C3 base class not found: " + base );
		checks.That( std::none_of( names.begin(), names.end(),
		                 [&]( const std::string &n )
		                 {
			                 return SameNoCase( n, base );
		                 } ),
		    "C3 base class not listed: " + base );
	}
	std::vector<std::string> unknown = expect.unknownNames;
	unknown.push_back( "__no_such_entity_class__" );
	for ( const std::string &name : unknown )
	{
		checks.That( catalog.Find( name ) == nullptr, "C3 unknown name not found: " + name );
	}

	// C4 and C5: class shapes.
	for ( const EntityCatalogExpectations::ClassShape &shape : expect.classes )
	{
		const EntityClassInfo *info = catalog.Find( shape.name );
		if ( !checks.That( info != nullptr, "C4 expected class found: " + shape.name ) )
		{
			continue;
		}
		checks.That( info->kind == shape.kind, "C4 kind: " + shape.name );
		std::vector<std::string> keys;
		for ( const KeyDefinition &key : info->keys )
		{
			keys.push_back( key.key );
		}
		checks.That( keys == shape.keys, "C4 resolved keys in base-first order: " + shape.name );
		bool unique = true;
		for ( std::size_t i = 0; i < keys.size(); ++i )
		{
			for ( std::size_t j = i + 1; j < keys.size(); ++j )
			{
				unique = unique && !SameNoCase( keys[i], keys[j] );
			}
		}
		checks.That( unique, "C4 no duplicate resolved key: " + shape.name );

		bool findKey = true;
		for ( const KeyDefinition &key : info->keys )
		{
			findKey = findKey && info->FindKey( Fold( key.key, true ) ) == &key &&
			          info->FindKey( Fold( key.key, false ) ) == &key;
		}
		checks.That( findKey && info->FindKey( "__no_such_key__" ) == nullptr,
		    "C5 FindKey case-insensitive: " + shape.name );

		bool io = info->inputs.size() == shape.inputs.size() &&
		          info->outputs.size() == shape.outputs.size();
		for ( const std::string &name : shape.inputs )
		{
			io =
			    io && info->HasInput( Fold( name, true ) ) && info->HasInput( Fold( name, false ) );
		}
		for ( const std::string &name : shape.outputs )
		{
			io = io && info->HasOutput( Fold( name, true ) ) &&
			     info->HasOutput( Fold( name, false ) );
		}
		checks.That( io && !info->HasInput( "__no_such_input__" ) &&
		                 !info->HasOutput( "__no_such_output__" ),
		    "C5 inputs and outputs exact and case-insensitive: " + shape.name );
	}

	for ( const EntityCatalogExpectations::KeyDefault &value : expect.defaults )
	{
		const EntityClassInfo *info = catalog.Find( value.className );
		const KeyDefinition *key = info ? info->FindKey( value.key ) : nullptr;
		checks.That( key != nullptr && key->defaultValue == value.defaultValue,
		    "C4 resolved default (override wins): " + value.className + "." + value.key );
	}
}

} // namespace hammertest

#endif // HAMMERTEST_ENTITY_CATALOG_CONFORMANCE_H
