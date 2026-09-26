// Self-test fixture: the test program of a suite with a shared-library unit.
// The runner defines CONFORMANCE_SHARED_FIXTURE_LIB as the library's path; the
// library must exist and must not be linked into this program.
#include "testing/conformance_result.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

extern "C" __attribute__( ( weak ) ) int SharedUnitFixtureValue();

int main()
{
	const char *path = CONFORMANCE_SHARED_FIXTURE_LIB;
	const std::size_t length = std::strlen( path );
	int failures = 0;
	failures += access( path, R_OK ) == 0 ? 0 : 1;
	failures += length > 3 && std::strcmp( path + length - 3, ".so" ) == 0 ? 0 : 1;
	failures += &SharedUnitFixtureValue == nullptr ? 0 : 1;
	std::printf( "shared unit %s: %d failure(s)\n", path, failures );
	return testing::ReportConformance( 3, failures );
}
