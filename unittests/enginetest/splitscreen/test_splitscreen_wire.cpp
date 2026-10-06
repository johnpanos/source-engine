//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive suite for engine.splitscreen-wire.v1 against the real codecs.
//
//=============================================================================//

#include "splitscreen_wire_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>

int main()
{
	const enginetest::WireReport r = enginetest::RunSplitScreenWireConformance( enginetest::RealCodec() );
	if ( r.failures != 0 )
	{
		std::printf( "FAIL test_splitscreen_wire: %d/%d checks failed; first: %s (line %d)\n", r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
	}
	else
	{
		std::printf( "ok test_splitscreen_wire: %d checks passed\n", r.checks );
	}
	return testing::ReportConformance( r.checks, r.failures );
}
