//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance suite for platform.file-probe.v1 (R11). A native
//			test builds a directory of known entries and describes it in a
//			fixture; every probe claiming the contract runs THIS predicate on it.
//			Includes the encoding cases the platform allows (bytes that are not
//			UTF-8 on POSIX; non-BMP and unpaired-surrogate names on Windows).
//
//=============================================================================//

#ifndef PLATFORMTEST_FILE_PROBE_CONFORMANCE_H
#define PLATFORMTEST_FILE_PROBE_CONFORMANCE_H

#include "platform/contracts/module_resolver.h"

#include <functional>
#include <string>
#include <vector>

namespace platformtest
{

struct FileProbeReport
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

struct FileProbeCase
{
	platform::NativePath path;
	platform::FileKind expected;
	const char *what;
};

struct FileProbeFixture
{
	std::vector<FileProbeCase> cases;
	// A path of the other flavor, which this platform's probe must not read.
	platform::NativePath foreignFlavor;
	// Counts the fixture directory's entries, to show probing changes nothing.
	std::function<int()> countEntries;
};

inline FileProbeReport RunFileProbeConformance(
    const platform::IFileProbe &probe, const FileProbeFixture &fixture )
{
	FileProbeReport r;
	const int before = fixture.countEntries ? fixture.countEntries() : 0;
	for ( int pass = 0; pass < 2; ++pass ) // stable on repeat
	{
		for ( const FileProbeCase &c : fixture.cases )
		{
			r.Record( probe.Probe( c.path ) == c.expected, c.what, __LINE__ );
		}
	}
	r.Record( probe.Probe( platform::NativePath() ) == platform::FileKind::kMissing, "empty path",
	    __LINE__ );
	if ( !fixture.foreignFlavor.IsEmpty() )
	{
		r.Record( probe.Probe( fixture.foreignFlavor ) == platform::FileKind::kMissing,
		    "other flavor is missing", __LINE__ );
	}
	if ( fixture.countEntries )
	{
		r.Record( fixture.countEntries() == before, "probing creates nothing", __LINE__ );
	}
	return r;
}

} // namespace platformtest

#endif // PLATFORMTEST_FILE_PROBE_CONFORMANCE_H
