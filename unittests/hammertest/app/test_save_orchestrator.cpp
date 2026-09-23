//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::app::SaveDocument (RFC 0002). Pins the
//			save-failure atomicity requirement: a temp-write failure or a commit
//			(rename) failure leaves the prior file intact, and a clean save
//			atomically replaces it.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/save_orchestrator.h"

#include "fake_file_store.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::app::SaveDocument;
using hammer::app::SaveStatus;
using hammertest::InMemoryFileStore;

namespace
{

int g_failures = 0;
int g_checks = 0;

void Check( bool condition, const char *expression, int line )
{
	++g_checks;
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_save_orchestrator.cpp", line, expression );
		++g_failures;
	}
}

std::string Read( InMemoryFileStore &store, const std::string &path )
{
	std::string out;
	store.Read( path, out );
	return out;
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

void TestCleanSaveReplaces()
{
	InMemoryFileStore store;
	store.files["map.vmf"] = "OLD";

	CHECK( SaveDocument( store, "map.vmf", "NEW" ) == SaveStatus::kOk );
	CHECK( Read( store, "map.vmf" ) == "NEW" );
	CHECK( !store.Exists( "map.vmf.savetmp" ) ); // temp cleaned via rename
}

void TestCreateNewFile()
{
	InMemoryFileStore store;
	CHECK( SaveDocument( store, "fresh.vmf", "DATA" ) == SaveStatus::kOk );
	CHECK( Read( store, "fresh.vmf" ) == "DATA" );
}

void TestTempWriteFailurePreservesPrior()
{
	InMemoryFileStore store;
	store.files["map.vmf"] = "OLD";
	store.failAllWrites = true;

	CHECK( SaveDocument( store, "map.vmf", "NEW" ) == SaveStatus::kTempWriteFailed );
	CHECK( Read( store, "map.vmf" ) == "OLD" ); // prior intact
	CHECK( !store.Exists( "map.vmf.savetmp" ) );
}

void TestCommitFailurePreservesPrior()
{
	InMemoryFileStore store;
	store.files["map.vmf"] = "OLD";
	store.failNextRename = true;

	CHECK( SaveDocument( store, "map.vmf", "NEW" ) == SaveStatus::kCommitFailed );
	CHECK( Read( store, "map.vmf" ) == "OLD" );  // prior intact
	CHECK( !store.Exists( "map.vmf.savetmp" ) ); // temp cleaned up
}

int main()
{
	TestCleanSaveReplaces();
	TestCreateNewFile();
	TestTempWriteFailurePreservesPrior();
	TestCommitFailurePreservesPrior();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.app SaveOrchestrator: %d check(s) FAILED\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "hammer.app SaveOrchestrator: all checks passed\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
