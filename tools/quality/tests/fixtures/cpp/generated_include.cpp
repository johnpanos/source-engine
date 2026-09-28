// Self-test fixture: a suite whose header comes from its profile's generated
// include root (conformance.generated_include_roots); it passes only when the
// runner ran the generator and put its output on the include path.
#include "testing/conformance_result.h"
#include "selftest_gen/value.h"
#include <cstdio>

int main()
{
	std::printf( "generated value %d\n", SELFTEST_GENERATED_VALUE );
	return testing::ReportConformance( 1, SELFTEST_GENERATED_VALUE == 7 ? 0 : 1 );
}
