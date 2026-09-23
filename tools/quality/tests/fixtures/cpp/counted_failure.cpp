// Self-test fixture: a counted check that fails. It must fail in the default
// AND the optimized NDEBUG release configuration, proving test assertions stay
// effective independently of engine debug assertions.
#include "testing/conformance_result.h"
#include <cstdio>

namespace
{
unsigned long g_checks = 0, g_failures = 0;
void Check( bool condition, const char *text )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL counted_failure.cpp: %s\n", text );
	}
}
} // namespace

int main( int argc, char ** )
{
	Check( argc > 0, "argc > 0" );
	Check( argc + 1 == 0, "argc + 1 == 0" );
	return testing::ReportConformance( g_checks, g_failures );
}
