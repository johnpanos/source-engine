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

// True for an absolute engine path: "/..." or a drive root "C:/...".
inline bool IsAbsoluteEnginePath( const char *p )
{
	if ( p[0] == '/' )
	{
		return true;
	}
	const bool letter = ( p[0] >= 'A' && p[0] <= 'Z' ) || ( p[0] >= 'a' && p[0] <= 'z' );
	return letter && p[1] == ':' && p[2] == '/';
}

// True when no '/'-separated segment is "." or "..".
inline bool HasNoDotSegments( const char *p )
{
	const char *segment = p;
	for ( const char *c = p;; ++c )
	{
		if ( *c == '/' || *c == '\0' )
		{
			const long n = static_cast<long>( c - segment );
			if ( ( n == 1 && segment[0] == '.' ) ||
			     ( n == 2 && segment[0] == '.' && segment[1] == '.' ) )
			{
				return false;
			}
			if ( *c == '\0' )
			{
				return true;
			}
			segment = c + 1;
		}
	}
}

// True for well-formed UTF-8 (no overlong forms, surrogates or values past U+10FFFF).
inline bool IsValidUtf8( const char *text )
{
	const unsigned char *p = reinterpret_cast<const unsigned char *>( text );
	while ( *p != 0 )
	{
		const unsigned char c = *p;
		int extra = 0;
		unsigned int cp = 0;
		if ( c < 0x80 )
		{
			++p;
			continue;
		}
		else if ( c >= 0xc2 && c <= 0xdf )
		{
			extra = 1;
			cp = c & 0x1f;
		}
		else if ( c >= 0xe0 && c <= 0xef )
		{
			extra = 2;
			cp = c & 0x0f;
		}
		else if ( c >= 0xf0 && c <= 0xf4 )
		{
			extra = 3;
			cp = c & 0x07;
		}
		else
		{
			return false;
		}
		for ( int i = 1; i <= extra; ++i )
		{
			if ( ( p[i] & 0xc0 ) != 0x80 )
			{
				return false;
			}
			cp = ( cp << 6 ) | ( p[i] & 0x3f );
		}
		if ( ( extra == 2 && ( cp < 0x800 || ( cp >= 0xd800 && cp <= 0xdfff ) ) ) ||
		     ( extra == 3 && ( cp < 0x10000 || cp > 0x10ffff ) ) )
		{
			return false;
		}
		p += extra + 1;
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
				PP_CHECK( r, IsAbsoluteEnginePath( buf ) );
				PP_CHECK( r, HasNoDotSegments( buf ) );
				PP_CHECK( r, IsValidUtf8( buf ) );

				// Stable: a second call yields the identical string.
				const int len2 = paths.GetPath( id, buf2, static_cast<int>( sizeof( buf2 ) ) );
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

	// The executable directory, when both are available, is the file's parent.
	if ( paths.IsAvailable( PlatformPathId::kExecutableFile ) &&
	     paths.IsAvailable( PlatformPathId::kExecutableDir ) )
	{
		const int fileLen = paths.GetPath(
		    PlatformPathId::kExecutableFile, buf, static_cast<int>( sizeof( buf ) ) );
		const int dirLen = paths.GetPath(
		    PlatformPathId::kExecutableDir, buf2, static_cast<int>( sizeof( buf2 ) ) );
		const char *slash = fileLen > 0 ? std::strrchr( buf, '/' ) : nullptr;
		const int parentLen = slash == nullptr ? -1
		                      : slash == buf   ? 1
		                                       : static_cast<int>( slash - buf );
		PP_CHECK( r, dirLen > 0 && dirLen == parentLen &&
		                 std::strncmp( buf, buf2, static_cast<std::size_t>( dirLen ) ) == 0 );
	}

	return r;
}

inline int RunPathsPositive( const char *suiteName, const platform::IPlatformPaths &paths )
{
	PathsReport r = RunPlatformPathsConformance( paths );
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", suiteName, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", suiteName, r.checks );
	return 0;
}

} // namespace platformtest

#endif // PLATFORMTEST_PATHS_CONFORMANCE_H
