// Self-test fixture: reports the runner-provided seed and attempt, and fails
// on the attempt named by FAIL_ON_ATTEMPT (models a nondeterministic suite so
// repeats are proven to retain every failure rather than retrying to green).
#include "testing/conformance_result.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main()
{
	const char *seed = std::getenv( "CONFORMANCE_SEED" );
	const char *attempt = std::getenv( "CONFORMANCE_ATTEMPT" );
	const char *failOn = std::getenv( "FAIL_ON_ATTEMPT" );
	std::printf( "seed=%s attempt=%s\n", seed ? seed : "(unset)", attempt ? attempt : "(unset)" );
	unsigned long failures = ( !seed || !attempt ) ? 1 : 0;
	if ( failOn && attempt && std::strcmp( failOn, attempt ) == 0 )
	{
		std::printf( "FAIL environment.cpp: deliberate failure on attempt %s\n", attempt );
		++failures;
	}
	return testing::ReportConformance( 1, failures );
}
