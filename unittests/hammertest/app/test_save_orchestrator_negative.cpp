//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for save-failure atomicity (RFC 0002/0005). A naive
//			saver that truncates the target before writing loses the prior file
//			when the write fails. The shared "prior preserved on failed write"
//			predicate must pass for the real orchestrator and be caught for the
//			naive one.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/save_orchestrator.h"

#include "fake_file_store.h"

#include <cstdio>
#include <string>

using hammer::app::SaveDocument;
using hammertest::InMemoryFileStore;

namespace
{

// Real saver: temp file + atomic rename.
void RealSave( InMemoryFileStore &store, const std::string &path, const std::string &content )
{
	SaveDocument( store, path, content );
}

// Broken saver: removes the target, then writes in place. If the write fails, the
// prior file is gone -- exactly the data loss the RFC's orchestration prevents.
void NaiveSave( InMemoryFileStore &store, const std::string &path, const std::string &content )
{
	store.Remove( path );
	store.Write( path, content );
}

// The prior-preserved-on-failed-write clause: with writes failing, a save attempt
// must leave the original content intact. Returns true when the saver preserves.
template <typename Saver> bool PreservesPriorOnFailedWrite( Saver saver )
{
	InMemoryFileStore store;
	store.files["map.vmf"] = "ORIGINAL";
	store.failAllWrites = true;

	saver( store, "map.vmf", "NEW" );

	std::string out;
	const bool present = store.Read( "map.vmf", out );
	return present && out == "ORIGINAL";
}

} // namespace

int main()
{
	const bool realPreserves = PreservesPriorOnFailedWrite( RealSave );
	const bool naivePreserves = PreservesPriorOnFailedWrite( NaiveSave );

	int failures = 0;

	if ( !realPreserves )
	{
		std::printf( "FAIL: real SaveDocument lost the prior file on a failed write\n" );
		++failures;
	}
	if ( naivePreserves )
	{
		std::printf( "FAIL: predicate did NOT detect the truncate-then-write saver\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.app SaveOrchestrator negative: %d check(s) FAILED\n", failures );
		return 1;
	}
	std::printf( "hammer.app SaveOrchestrator negative: oracle detects violations (real passes, "
	             "naive caught)\n" );
	return 0;
}
