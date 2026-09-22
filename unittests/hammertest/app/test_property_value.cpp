//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::app::PropertyValue (RFC 0002). Pins the
//			RFC requirement that multi-selection mixed values stay distinguishable
//			from empty strings and from unset properties.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/property_value.h"

#include <cstdio>
#include <string>
#include <vector>

using hammer::app::PropertyValue;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_property_value.cpp", line, expression );
		++g_failures;
	}
}

// Fold a selection's contributions for one property.
PropertyValue Aggregate( const std::vector<std::string> &values )
{
	PropertyValue aggregate = PropertyValue::Unset();
	for ( const std::string &value : values )
	{
		aggregate = aggregate.AddContributor( value );
	}
	return aggregate;
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

void TestThreeStatesAreDistinct()
{
	const PropertyValue unset = PropertyValue::Unset();
	const PropertyValue empty = PropertyValue::Single( "" );
	const PropertyValue mixed = PropertyValue::Mixed();

	CHECK( unset.IsUnset() );
	CHECK( empty.IsSingle() && empty.Value().empty() );
	CHECK( mixed.IsMixed() );

	// The whole point: empty-string, unset, and mixed are three different things.
	CHECK( !( unset == empty ) );
	CHECK( !( unset == mixed ) );
	CHECK( !( empty == mixed ) );
}

void TestAggregation()
{
	// No contributors -> unset.
	CHECK( Aggregate( {} ).IsUnset() );

	// One contributor -> single (even when empty).
	CHECK( Aggregate( { "wall" } ) == PropertyValue::Single( "wall" ) );
	CHECK( Aggregate( { "" } ) == PropertyValue::Single( "" ) );

	// Agreement stays single; an empty and a nonempty disagree -> mixed.
	CHECK( Aggregate( { "wall", "wall", "wall" } ) == PropertyValue::Single( "wall" ) );
	CHECK( Aggregate( { "", "" } ) == PropertyValue::Single( "" ) );
	CHECK( Aggregate( { "wall", "floor" } ).IsMixed() );
	CHECK( Aggregate( { "wall", "" } ).IsMixed() ); // empty differs from "wall"
	CHECK( Aggregate( { "", "floor" } ).IsMixed() );

	// Once mixed, always mixed.
	CHECK( Aggregate( { "a", "b", "a" } ).IsMixed() );
}

int main()
{
	TestThreeStatesAreDistinct();
	TestAggregation();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.app PropertyValue: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.app PropertyValue: all checks passed\n" );
	return 0;
}
