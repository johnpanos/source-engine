// Self-test fixture: passes only when the suite's @response-file flags were
// applied (the macro is defined solely in response_file.rsp).
#include "testing/conformance_result.h"
#include <cstdio>

int main()
{
#ifdef RESPONSE_FILE_FLAG
	std::printf( "self-test response file: applied\n" );
	return testing::ReportConformance( 1, 0 );
#else
	std::printf( "self-test response file: NOT applied\n" );
	return testing::ReportConformance( 1, 1 );
#endif
}
