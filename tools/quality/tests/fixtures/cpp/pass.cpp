// Self-test fixture: a suite that builds, runs its checks and reports a
// complete checks-v1 result through the shared header (exit 0).
#include "testing/conformance_result.h"
#include <cstdio>

int main()
{
	std::printf( "self-test pass: ok\n" );
	return testing::ReportConformance( 1, 0 );
}
