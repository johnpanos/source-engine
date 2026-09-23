//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.map_export.v1. A failure in a composed
//			stage (here: a func_instance pointing at a missing file) must PROPAGATE
//			as an error from PrepareForCompile, not be swallowed into a partial
//			compile document. A clean map must prepare successfully. Exit 0 when
//			both hold.
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/formats/map_export.h"
#include "testing/conformance_result.h"

#include "app/fake_file_store.h"

#include <cstdio>
#include <string>

using hammer::formats::CompileOptions;
using hammer::formats::CompilePrepResult;
using hammer::formats::ParseKeyValues;
using hammer::formats::PrepareForCompile;

namespace
{
int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *label )
{
	++g_checks;
	if ( !ok )
	{
		std::printf( "FAIL: %s\n", label );
		++g_failures;
	}
}

} // namespace

int main()
{
	// A func_instance referencing a file the store does not have must make the
	// whole prepare fail with a diagnostic (not a silent partial document).
	{
		hammertest::InMemoryFileStore store; // empty: child.vmf absent
		const char *map = "world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		                  "entity\n{\n\t\"classname\" \"func_instance\"\n\t\"file\" \"missing.vmf\""
		                  "\n\t\"origin\" \"0 0 0\"\n}\n";
		hammer::formats::ParseResult pr = ParseKeyValues( map );
		Check( pr.ok, "map parses" );
		CompilePrepResult r = PrepareForCompile( pr.root, store, CompileOptions{} );
		Check( !r.ok, "missing instance file fails the prepare" );
		Check( !r.error.empty(), "failure carries a diagnostic" );
	}

	// A clean map (no instances) prepares successfully.
	{
		hammertest::InMemoryFileStore store;
		const char *map = "world\n{\n\t\"classname\" \"worldspawn\"\n\tsolid\n\t{\n\t\t\"id\" "
		                  "\"1\"\n\t\tside\n\t\t{\n\t\t\t\"plane\" \"(0 0 0) (16 0 0) (16 16 0)\""
		                  "\n\t\t}\n\t}\n}\n";
		hammer::formats::ParseResult pr = ParseKeyValues( map );
		CompilePrepResult r = PrepareForCompile( pr.root, store, CompileOptions{} );
		Check( r.ok, "clean map prepares ok" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.map_export negative: ORACLE UNSOUND (%d)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.map_export negative: a bad func_instance fails the prepare (not a silent "
	             "partial); a clean map succeeds\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
