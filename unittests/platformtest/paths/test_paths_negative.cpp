//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity (negative-provider) check for the RFC 0001 platform-paths
//			conformance suite (PLAT-PATHS-001, Q-FOUNDATION). Feeds the SAME shared
//			predicate deliberately-broken providers -- each violating one clause of
//			the normalization / explicit-availability contract -- and asserts every
//			one is caught while the conforming backend passes.
//
//			Passes (exit 0) when the oracle distinguishes conforming from broken.
//
//			Build/run: tools/quality/conformance.py check --suite platform.paths.sensitivity
//
//=============================================================================//

#include "paths_conformance.h"
#include "fake_paths.h"

#include "platform/contracts/paths.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace
{

enum class Defect
{
	kBackslashSeparator,    // returns a native-separator path
	kTrailingSlash,         // returns a path with a trailing '/'
	kDoubleSlash,           // returns a path containing "//"
	kFabricatedUnavailable, // IsAvailable() false but GetPath returns a real path
	kRelative,              // returns a relative path
	kDotSegment,            // returns a path with a ".." segment
	kInvalidUtf8,           // returns bytes that are not UTF-8
	kDirNotParent           // the executable directory is not the file's parent
};

class CBrokenPaths : public platform::IPlatformPaths
{
public:
	explicit CBrokenPaths( Defect defect ) : m_defect( defect ) {}

	bool IsAvailable( platform::PlatformPathId id ) const override
	{
		if ( m_defect == Defect::kFabricatedUnavailable &&
		     id == platform::PlatformPathId::kNativeLibraryDir )
		{
			return false; // claims unavailable...
		}
		return Value( id ) != nullptr;
	}

	int GetPath( platform::PlatformPathId id, char *buffer, int bufferSize ) const override
	{
		const std::string owned = Value( id ) != nullptr ? Value( id ) : "";
		const char *value = Value( id ) != nullptr ? owned.c_str() : nullptr;
		// ...but the fabricated-unavailable defect still hands back a path here.
		if ( m_defect == Defect::kFabricatedUnavailable &&
		     id == platform::PlatformPathId::kNativeLibraryDir )
		{
			value = "/opt/game/lib";
		}
		if ( value == nullptr || buffer == nullptr || bufferSize <= 0 )
		{
			return -1;
		}
		const int len = static_cast<int>( std::strlen( value ) );
		if ( len + 1 > bufferSize )
		{
			return -1;
		}
		std::memcpy( buffer, value, static_cast<std::size_t>( len ) + 1 );
		return len;
	}

private:
	// The executable file is the directory value plus "/engine", so only the
	// kDirNotParent defect breaks the parent clause.
	const char *Value( platform::PlatformPathId id ) const
	{
		if ( id == platform::PlatformPathId::kExecutableFile && m_defect != Defect::kDirNotParent )
		{
			m_file = std::string( Base() ) + "/engine";
			return m_file.c_str();
		}
		return Base( id );
	}

	const char *Base( platform::PlatformPathId id = platform::PlatformPathId::kExecutableDir ) const
	{
		if ( id == platform::PlatformPathId::kNativeLibraryDir )
		{
			return nullptr; // unavailable in the base fake
		}
		// Inject the string defect on every available id.
		switch ( m_defect )
		{
		case Defect::kBackslashSeparator:
			return "\\opt\\game\\bin";
		case Defect::kTrailingSlash:
			return "/opt/game/bin/";
		case Defect::kDoubleSlash:
			return "/opt//game/bin";
		case Defect::kFabricatedUnavailable:
			return "/opt/game/bin"; // normalized; the defect is in availability
		case Defect::kRelative:
			return "opt/game/bin";
		case Defect::kDotSegment:
			return "/opt/game/../bin";
		case Defect::kInvalidUtf8:
			return "/opt/g\xc3me/bin";
		case Defect::kDirNotParent:
			return id == platform::PlatformPathId::kExecutableFile ? "/opt/game/bin/engine"
			                                                       : "/opt/elsewhere";
		}
		return "/opt/game/bin";
	}

	Defect m_defect;
	mutable std::string m_file;
};

struct Case
{
	Defect defect;
	const char *name;
};

bool Caught( const platform::IPlatformPaths &paths )
{
	platformtest::PathsReport r = platformtest::RunPlatformPathsConformance( paths );
	return r.failures > 0;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;

	// 1) The conforming test backend must PASS.
	{
		platformtest::CFakePlatformPaths good;
		platformtest::PathsReport r = platformtest::RunPlatformPathsConformance( good );
		++checks;
		if ( r.failures != 0 )
		{
			std::printf( "FAIL: conforming paths backend rejected by suite (%d/%d); "
			             "first: %s (line %d)\n",
			    r.failures, r.checks, r.firstFailure, r.firstFailureLine );
			++failures;
		}
	}

	// 2) Every broken provider must be CAUGHT.
	const Case cases[] = {
	    { Defect::kBackslashSeparator, "backslash-separator" },
	    { Defect::kTrailingSlash, "trailing-slash" },
	    { Defect::kDoubleSlash, "double-slash" },
	    { Defect::kFabricatedUnavailable, "fabricated-unavailable-path" },
	    { Defect::kRelative, "relative-path" },
	    { Defect::kDotSegment, "dot-segment" },
	    { Defect::kInvalidUtf8, "invalid-utf8" },
	    { Defect::kDirNotParent, "executable-dir-not-parent" },
	};
	for ( const Case &c : cases )
	{
		CBrokenPaths bad( c.defect );
		++checks;
		if ( !Caught( bad ) )
		{
			std::printf( "FAIL: broken paths provider '%s' was NOT caught\n", c.name );
			++failures;
		}
	}

	if ( failures == 0 )
	{
		std::printf( "ok test_paths_negative: suite accepts conforming and rejects "
		             "all %zu broken providers\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
