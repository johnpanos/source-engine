// Self-test fixture modelling a well-formed "sensitivity" suite: it runs a
// contract predicate against a conforming provider (must pass) and a broken
// provider (must be caught), and exits 0 only when the oracle is non-vacuous.
#include "testing/conformance_result.h"
#include <cstdio>

namespace
{

// The contract clause: the provider must report a "touching" case as false.
bool Conforms( bool providerIsBuggy )
{
	const bool touches = providerIsBuggy; // buggy provider wrongly returns true
	return !touches;
}

} // namespace

int main()
{
	const bool realConforms = Conforms( false );
	const bool buggyConforms = Conforms( true );
	if ( !realConforms || buggyConforms )
	{
		std::printf( "FAIL: sensitivity oracle is vacuous\n" );
		return 1;
	}
	std::printf( "sensitivity ok: real passes, buggy caught\n" );
	return testing::ReportConformance( 2, 0 );
}
