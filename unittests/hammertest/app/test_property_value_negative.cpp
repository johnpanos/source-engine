//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the empty-vs-unset property distinction
//			(RFC 0002/0005). A "helpful" aggregator that treats an empty string as
//			no contribution collapses Single("") into Unset -- exactly the loss the
//			RFC warns about. The oracle must detect it.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/property_value.h"

#include <cstdio>
#include <string>
#include <vector>

using hammer::app::PropertyState;
using hammer::app::PropertyValue;

namespace
{

// Correct aggregation via the real value type.
PropertyValue RealAggregate( const std::vector<std::string> &values )
{
	PropertyValue aggregate = PropertyValue::Unset();
	for ( const std::string &value : values )
	{
		aggregate = aggregate.AddContributor( value );
	}
	return aggregate;
}

// Broken aggregation: skips empty strings as if they were no contribution, so a
// selection whose only value is "" is reported Unset instead of Single("").
PropertyState BrokenAggregateState( const std::vector<std::string> &values )
{
	PropertyValue aggregate = PropertyValue::Unset();
	for ( const std::string &value : values )
	{
		if ( value.empty() )
		{
			continue; // the bug
		}
		aggregate = aggregate.AddContributor( value );
	}
	return aggregate.State();
}

// The empty-vs-unset clause: a single empty-string contributor must aggregate to
// Single(""), NOT Unset. Returns true when the state matches Single.
bool ConformsEmptyIsSingle( PropertyState state )
{
	return state == PropertyState::kSingle;
}

} // namespace

int main()
{
	const std::vector<std::string> onlyEmpty = { "" };

	const bool realConforms = ConformsEmptyIsSingle( RealAggregate( onlyEmpty ).State() );
	const bool brokenConforms = ConformsEmptyIsSingle( BrokenAggregateState( onlyEmpty ) );

	int failures = 0;

	if ( !realConforms )
	{
		std::printf( "FAIL: real PropertyValue collapsed empty string into unset\n" );
		++failures;
	}
	if ( brokenConforms )
	{
		std::printf( "FAIL: predicate did NOT detect the empty-as-unset aggregator\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.app PropertyValue negative: %d check(s) FAILED\n", failures );
		return 1;
	}
	std::printf( "hammer.app PropertyValue negative: oracle detects violations (real passes, "
	             "empty-as-unset caught)\n" );
	return 0;
}
