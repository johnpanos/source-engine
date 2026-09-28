//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for the shared entity catalog conformance
//			function (ports.entity_catalog.v1). A good fake passes it; each
//			deliberately bad catalog below breaks one clause of the port
//			contract and must be flagged (its private Checks record at least
//			one failure). Otherwise the port suite could certify a catalog
//			that misfinds classes, misorders keys or leaks base classes.
//
//=============================================================================//

#include "fakes/fake_entity_catalog.h"
#include "formats/entity_catalog_conformance.h"
#include "testing/checks.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using hammer::ports::EntityClassInfo;
using hammer::ports::EntityClassKind;
using hammer::ports::IEntityCatalog;
using hammertest::EntityCatalogExpectations;
using hammertest::FakeEntityCatalog;

namespace
{

using C = FakeEntityCatalog;

// A base class "Targetname" (targetname) under a point class with an own key
// and an override of the base key's default; plus a mixed-case class so that
// byte order and case-insensitive order differ.
FakeEntityCatalog GoodCatalog()
{
	FakeEntityCatalog f;
	f.AddPoint( "prop_thing",
	    { C::Key( "targetname", "target_source", "base" ), C::Key( "model", "studio", "m.mdl" ) },
	    { C::Io( "Kill" ) }, { C::Io( "OnUser1" ) } );
	f.AddPoint( "Light", { C::Key( "targetname", "target_source" ) } );
	f.AddSolid( "func_door", { C::Key( "speed", "float", "100" ) } );
	return f;
}

EntityCatalogExpectations Expect()
{
	EntityCatalogExpectations e;
	e.classNames = { "prop_thing", "Light", "func_door" };
	e.baseClassNames = { "Targetname" };
	e.classes = {
	    { "prop_thing", EntityClassKind::Point, { "targetname", "model" }, { "Kill" },
	        { "OnUser1" } },
	    { "Light", EntityClassKind::Point, { "targetname" }, {}, {} },
	    { "func_door", EntityClassKind::Solid, { "speed" }, {}, {} },
	};
	e.defaults = { { "prop_thing", "targetname", "base" } };
	return e;
}

// Forwards to a good fake unless a subclass overrides a call.
class Wrapper : public IEntityCatalog
{
public:
	const EntityClassInfo *Find( std::string_view name ) const override
	{
		return m_good.Find( name );
	}
	std::vector<std::string> ClassNames() const override { return m_good.ClassNames(); }

protected:
	FakeEntityCatalog m_good = GoodCatalog();
};

// Bad: exact-case lookup.
class CaseSensitiveFind final : public Wrapper
{
public:
	const EntityClassInfo *Find( std::string_view name ) const override
	{
		const EntityClassInfo *info = m_good.Find( name );
		return info && info->name == name ? info : nullptr;
	}
};

// Bad: names sorted by byte value ("Light" before "func_door").
class ByteSortedNames final : public Wrapper
{
public:
	std::vector<std::string> ClassNames() const override
	{
		std::vector<std::string> names = m_good.ClassNames();
		std::sort( names.begin(), names.end() );
		return names;
	}
};

// Bad: base classes are listed and found.
class ListsBaseClasses final : public Wrapper
{
public:
	ListsBaseClasses() { m_good.AddKind( EntityClassKind::Other, "Targetname" ); }
};

// Bad: a class drops out of the listing.
class MissingClass final : public Wrapper
{
public:
	std::vector<std::string> ClassNames() const override
	{
		std::vector<std::string> names = m_good.ClassNames();
		names.pop_back();
		return names;
	}
};

// Bad: a fresh copy per Find (results not stable for the catalog's lifetime).
class UnstableFind final : public Wrapper
{
public:
	const EntityClassInfo *Find( std::string_view name ) const override
	{
		const EntityClassInfo *info = m_good.Find( name );
		if ( info == nullptr )
		{
			return nullptr;
		}
		m_copies.push_back( std::make_unique<EntityClassInfo>( *info ) );
		return m_copies.back().get();
	}

private:
	mutable std::vector<std::unique_ptr<EntityClassInfo>> m_copies;
};

// Bad catalogs expressed as data on the fake.
FakeEntityCatalog BaseLastKeys()
{
	FakeEntityCatalog f = GoodCatalog();
	f.AddPoint( "prop_thing",
	    { C::Key( "model", "studio", "m.mdl" ), C::Key( "targetname", "target_source", "base" ) },
	    { C::Io( "Kill" ) }, { C::Io( "OnUser1" ) } );
	return f;
}

// The override appended instead of replacing the base key in place.
FakeEntityCatalog OverrideAppended()
{
	FakeEntityCatalog f = GoodCatalog();
	f.AddPoint( "prop_thing",
	    { C::Key( "targetname", "target_source", "inherited" ),
	        C::Key( "model", "studio", "m.mdl" ), C::Key( "TargetName", "target_source", "base" ) },
	    { C::Io( "Kill" ) }, { C::Io( "OnUser1" ) } );
	return f;
}

FakeEntityCatalog WrongKind()
{
	FakeEntityCatalog f = GoodCatalog();
	f.AddPoint( "func_door", { C::Key( "speed", "float", "100" ) } );
	return f;
}

FakeEntityCatalog MissingOutput()
{
	FakeEntityCatalog f = GoodCatalog();
	f.AddPoint( "prop_thing",
	    { C::Key( "targetname", "target_source", "base" ), C::Key( "model", "studio", "m.mdl" ) },
	    { C::Io( "Kill" ) } );
	return f;
}

// Runs the port suite privately; returns its failure count.
std::size_t Failures( const IEntityCatalog &catalog )
{
	std::FILE *sink = std::tmpfile();
	testing::Checks inner( sink != nullptr ? sink : stdout );
	hammertest::RunEntityCatalogConformance( inner, catalog, Expect() );
	if ( sink != nullptr )
	{
		std::fclose( sink );
	}
	return inner.Failures();
}

} // namespace

int main()
{
	testing::Checks checks;

	const FakeEntityCatalog good = GoodCatalog();
	checks.Equal( Failures( good ), std::size_t( 0 ), "good fake passes the port suite" );

	checks.That( Failures( CaseSensitiveFind() ) > 0, "case-sensitive Find flagged" );
	checks.That( Failures( ByteSortedNames() ) > 0, "byte-sorted ClassNames flagged" );
	checks.That( Failures( ListsBaseClasses() ) > 0, "listed base class flagged" );
	checks.That( Failures( MissingClass() ) > 0, "missing class flagged" );
	checks.That( Failures( UnstableFind() ) > 0, "unstable Find pointers flagged" );
	checks.That( Failures( BaseLastKeys() ) > 0, "base-last key order flagged" );
	checks.That( Failures( OverrideAppended() ) > 0, "appended override flagged" );
	checks.That( Failures( WrongKind() ) > 0, "wrong class kind flagged" );
	checks.That( Failures( MissingOutput() ) > 0, "missing output flagged" );

	return checks.Report();
}
