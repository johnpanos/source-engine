//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `checks-v1` terminal result record of a standalone conformance
// suite, read by the shared runner (tools/quality/conformance.py, RFC 0005 Q1).
//
// Every suite the manifest expects to pass must end by returning
// ReportConformance( checks, failures ) from main. The runner accepts a run
// only when the process exits 0 AND stdout carries exactly one record with a
// nonzero check count and zero failures. A suite that returns early, crashes
// after printing progress, silently executes no checks, or prints a second
// record therefore fails even if its exit status is 0 (RFC 0005: zero
// discovery and incomplete results fail).
//
// Header-only and dialect-neutral (C++11 and later) so legacy-dialect units can
// use it. Checks must not be built on assert(): NDEBUG removes them, and the
// runner rejects suites that call it.
//
//=============================================================================//
#ifndef TESTING_CONFORMANCE_RESULT_H
#define TESTING_CONFORMANCE_RESULT_H

#include <cstdio>

namespace testing
{
// Prints the record and returns the matching process exit status.
inline int ReportConformance( unsigned long checks, unsigned long failures )
{
	std::printf( "CONFORMANCE %lu %lu\n", checks, failures );
	std::fflush( stdout );
	return ( checks > 0 && failures == 0 ) ? 0 : 1;
}
} // namespace testing

#endif // TESTING_CONFORMANCE_RESULT_H
