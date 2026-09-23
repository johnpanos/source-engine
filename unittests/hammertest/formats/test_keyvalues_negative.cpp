//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the semantic comparator (RFC 0002/0005). A
//			comparator that ignores values (or drops hard-to-compare fields) would
//			pass a round trip that lost data. The shared "data loss must be
//			detected" predicate runs against the real comparator (must catch a
//			changed value and a dropped block) and a permissive always-equal
//			comparator (must be caught).
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::formats::CompareKeyValues;
using hammer::formats::CompareResult;
using hammer::formats::KeyValueNode;
using hammer::formats::ParseKeyValues;

namespace
{

// The real comparator.
CompareResult RealCompare( const KeyValueNode &a, const KeyValueNode &b )
{
	return CompareKeyValues( a, b );
}

// A permissive comparator that always reports equal -- the exact failure mode the
// RFC warns about (a comparator that discards difficult fields to force green).
CompareResult PermissiveCompare( const KeyValueNode &, const KeyValueNode & )
{
	CompareResult result;
	result.equal = true;
	return result;
}

// The data-loss clause: a document with a changed material value and one with a
// dropped side block must both be reported NOT equal to the original. Returns
// true when the comparator detects both losses.
template <typename Compare> bool DetectsDataLoss( Compare compare )
{
	const char *original = "solid\n{\n\tside\n\t{\n\t\t\"material\" \"BRICK\"\n\t}\n\tside\n\t{\n"
	                       "\t\t\"material\" \"WOOD\"\n\t}\n}\n";
	const char *changedValue = "solid\n{\n\tside\n\t{\n\t\t\"material\" \"CONCRETE\"\n\t}\n\tside\n"
	                           "\t{\n\t\t\"material\" \"WOOD\"\n\t}\n}\n";
	const char *droppedBlock = "solid\n{\n\tside\n\t{\n\t\t\"material\" \"BRICK\"\n\t}\n}\n";

	const KeyValueNode a = ParseKeyValues( original ).root;
	const KeyValueNode b = ParseKeyValues( changedValue ).root;
	const KeyValueNode c = ParseKeyValues( droppedBlock ).root;

	const bool changeDetected = !compare( a, b ).equal;
	const bool dropDetected = !compare( a, c ).equal;
	return changeDetected && dropDetected;
}

} // namespace

int main()
{
	const bool realDetects = DetectsDataLoss( RealCompare );
	const bool permissiveDetects = DetectsDataLoss( PermissiveCompare );

	int checks = 0;
	int failures = 0;

	++checks;
	if ( !realDetects )
	{
		std::printf( "FAIL: real comparator missed a changed value or a dropped block\n" );
		++failures;
	}
	++checks;
	if ( permissiveDetects )
	{
		std::printf( "FAIL: predicate did NOT detect the permissive (always-equal) comparator\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.formats keyvalues negative: %d check(s) FAILED\n", failures );
		return testing::ReportConformance( checks, failures );
	}
	std::printf( "hammer.formats keyvalues negative: oracle detects violations (real passes, "
	             "permissive caught)\n" );
	return testing::ReportConformance( checks, failures );
}
