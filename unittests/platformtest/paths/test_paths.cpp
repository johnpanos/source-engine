//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 platform-paths capability
//			(PLAT-PATHS-001, Q-FOUNDATION). Runs the shared suite against the
//			deterministic test backend on linux-headless-core.
//
//			Certifies contract semantics (normalization, encoding, explicit
//			availability, buffer-size handling) via a fake; NOT evidence of native
//			path resolution -- a real Win32/POSIX provider runs the same suite.
//
//			Build/run: tools/quality/conformance.py check --suite platform.paths
//
//=============================================================================//

#include "paths_conformance.h"
#include "fake_paths.h"

#include <cstdio>

int main()
{
	platformtest::CFakePlatformPaths paths;
	const int rc = platformtest::RunPathsPositive( "test_paths", paths );
	if ( rc == 0 )
	{
		std::printf( "ok test_paths: all checks passed\n" );
	}
	return rc;
}
