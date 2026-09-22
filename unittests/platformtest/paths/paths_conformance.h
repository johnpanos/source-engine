//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for the RFC 0001 platform-paths capability
//			(platform::IPlatformPaths). Every provider that claims the contract --
//			the deterministic test backend here and the real Win32/POSIX backend
//			when it lands -- runs THIS predicate.
//
//			The predicate enforces the documented normalization/encoding rules and
//			the explicit-availability rule, without assuming any particular
//			location value (those are platform-specific). The negative test proves
//			it is not vacuous.
//
//=============================================================================//

#ifndef PLATFORMTEST_PATHS_CONFORMANCE_H
#define PLATFORMTEST_PATHS_CONFORMANCE_H

#include "platform/contracts/paths.h"

#include <cstdio>
#include <cstring>

namespace platformtest
{

struct PathsReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define PP_CHECK( report, cond ) ( report ).Record( ( cond ), #cond, __LINE__ )

// Returns true when `p` (length `len`) satisfies the contract's normalization and
// encoding rules: non-empty, UTF-8 with no embedded NUL, '/'-separated with no
// backslashes, no "//", and no trailing '/' unless the whole path is the root.
inline bool IsNormalizedEnginePath( const char *p, int len )
{
	if ( p == nullptr || len <= 0 )
	{
		return false;
	}
	if ( static_cast<int>( std::strlen( p ) ) != len )
	{
		return false; // embedded NUL or length mismatch
	}
	for ( int i = 0; i < len; ++i )
	{
		if ( p[i] == '\\' )
		{
			return false; // native separator leaked
		}
		if ( p[i] == '/' && i + 1 < len && p[i + 1] == '/' )
		{
			return false; // collapsed-separator rule
		}
	}
	if ( len > 1 && p[len - 1] == '/' )
	{
		return false; // trailing separator (root "/" is the only exception)
	}
	return true;
}

inline PathsReport RunPlatformPathsConformance( const platform::IPlatformPaths &paths )
{
	using platform::PlatformPathId;

	const PlatformPathId ids[] = {
		PlatformPathId::kExecutableFile,
		PlatformPathId::kExecutableDir,
		PlatformPathId::kUserData,
		PlatformPathId::kTemp,
		PlatformPathId::kNativeLibraryDir,
	};

	PathsReport r;
	char buf[1024];
	char buf2[1024];

	for ( const PlatformPathId id : ids )
	{
		const bool available = paths.IsAvailable( id );
		std::memset( buf, 0x7f, sizeof( buf ) );
		const int len = paths.GetPath( id, buf, static_cast<int>( sizeof( buf ) ) );

		if ( available )
		{
			// Available: a normalized, well-formed path is written and reported.
			PP_CHECK( r, len > 0 );
			if ( len > 0 )
			{
				PP_CHECK( r, IsNormalizedEnginePath( buf, len ) );

				// Stable: a second call yields the identical string.
				const int len2 =
					paths.GetPath( id, buf2, static_cast<int>( sizeof( buf2 ) ) );
				PP_CHECK( r, len2 == len );
				PP_CHECK( r, std::strcmp( buf, buf2 ) == 0 );
			}
		}
		else
		{
			// Unavailable: explicit negative result, never a fabricated path.
			PP_CHECK( r, len == -1 );
		}

		// Defensive-argument rules apply regardless of availability.
		PP_CHECK( r, paths.GetPath( id, nullptr, 16 ) == -1 );
		PP_CHECK( r, paths.GetPath( id, buf, 0 ) == -1 );

		// A one-byte buffer cannot hold any non-empty path plus NUL: expect -1
		// and no overflow (buf[0] must be untouched as a proxy for "no write").
		char tiny[1] = { '#' };
		const int tinyLen = paths.GetPath( id, tiny, 1 );
		if ( available )
		{
			PP_CHECK( r, tinyLen == -1 );
			PP_CHECK( r, tiny[0] == '#' );
		}
	}

	return r;
}

inline int RunPathsPositive( const char *suiteName, const platform::IPlatformPaths &paths )
{
	PathsReport r = RunPlatformPathsConformance( paths );
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n",
			suiteName, r.failures, r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", suiteName, r.checks );
	return 0;
}

} // namespace platformtest

#endif // PLATFORMTEST_PATHS_CONFORMANCE_H
