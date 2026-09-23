// Self-test fixture: a suite whose only check is assert(). Under NDEBUG the
// check disappears and the suite would report success, so the runner must
// reject it before building (invalid-oracle).
#include "testing/conformance_result.h"
#include <cassert>

int main()
{
	assert( 1 + 1 == 3 );
	return testing::ReportConformance( 1, 0 );
}
