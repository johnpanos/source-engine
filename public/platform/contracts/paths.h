//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Capability contract for platform path locations (RFC 0001 foundation
//			capability "Platform paths": executable, user-data, temporary, and
//			native-library paths).
//
//			Portable code asks for a location by role, not by branching on the OS.
//			A native provider resolves the real location; the deterministic test
//			provider returns fixed, normalized values. Returned paths use the
//			engine's virtual-path encoding: UTF-8, forward-slash separators (see
//			RFC 0001 "Path representation and encoding"); the native representation
//			is not exposed here. Path resolution/search policy for assets is a
//			separate concern (module resolver / filesystem); this contract only
//			reports well-known base locations.
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_PATHS_H
#define PLATFORM_CONTRACTS_PATHS_H

// Contract header: standard library only. No tier0/tier1, no native SDK, no
// OS-selection macros. Must compile under linux-headless-core.

namespace platform
{

// A well-known base location, named by role rather than by platform convention.
enum class PlatformPathId
{
	kExecutableFile = 0,  // the running executable's own path
	kExecutableDir,       // the directory containing the executable
	kUserData,            // per-user writable data directory
	kTemp,                // temporary directory
	kNativeLibraryDir,    // directory searched for native provider libraries
};

// Reports well-known platform path locations. All returned paths are normalized
// engine paths (UTF-8, '/'-separated, no "//", no trailing '/' except a lone
// root "/"). A location may be optional on some platforms; absence is reported
// explicitly through IsAvailable / a negative GetPath result, never emulated
// with an empty or bogus path.
class IPlatformPaths
{
public:
	virtual ~IPlatformPaths() = default;

	// True when this platform provides `id`. Optional locations report false
	// rather than returning a fabricated path.
	virtual bool IsAvailable( PlatformPathId id ) const = 0;

	// Writes the normalized path for `id` into `buffer` (NUL-terminated) and
	// returns the length written, excluding the NUL. Returns -1 without writing a
	// partial result when `id` is unavailable, `buffer` is null, `bufferSize` is
	// non-positive, or the path (plus NUL) does not fit. The result is stable:
	// repeated calls for the same `id` return the same string.
	virtual int GetPath( PlatformPathId id, char *buffer, int bufferSize ) const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_PATHS_H
