//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for adapters.source.vector_interop.v1.
//			Runs the shared adapter checks against the REAL seam
//			(hammer::adapters::source). Exit 0 iff every clause holds. Runs on the
//			MSVC/Wine parity lane (requires the Source Vector/BoundBox layer).
//
//=============================================================================//

#include "adapter_checks.h"

#include <cstdio>

int main()
{
	const int failures = hammer_adapter_checks::RunAdapterChecks<RealSourceAdapter>();
	if ( failures != 0 )
	{
		std::printf( "adapters.source.vector_interop: %d FAILURE(S)\n", failures );
		return 1;
	}
	std::printf( "adapters.source.vector_interop: seam preserves legacy geometry\n" );
	return 0;
}
