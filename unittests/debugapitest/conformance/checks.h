//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: counted checks for the debug API conformance suites (checks-v1).
//
//===========================================================================//

#ifndef DEBUGAPITEST_CHECKS_H
#define DEBUGAPITEST_CHECKS_H

#include <cstdio>
#include <string>

namespace debugapitest
{

struct Results
{
	unsigned long checks = 0;
	unsigned long failures = 0;
	std::string scope;

	void Check( bool condition, const char *description )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			std::fprintf( stderr, "FAIL [%s]: %s\n", scope.c_str(), description );
		}
	}
};

} // namespace debugapitest

#endif // DEBUGAPITEST_CHECKS_H
