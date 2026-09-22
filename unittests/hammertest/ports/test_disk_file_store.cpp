//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for ports.file_store.v1. Runs the shared
//			IFileStore contract against BOTH the real DiskFileStore (over a unique
//			temp directory) and the InMemoryFileStore fake, proving they satisfy
//			the same behavioral contract (Liskov). Build/run via the conformance
//			manifest (linux-headless-core).
//
//=============================================================================//

#include "file_store_contract.h"

#include "app/fake_file_store.h"
#include "hammer/adapters/platform/disk_file_store.h"

#include <cstdio>
#include <filesystem>
#include <random>
#include <string>

int main()
{
	namespace fs = std::filesystem;

	int failures = 0;

	// Real provider over a unique temp directory.
	std::error_code ec;
	fs::path dir = fs::temp_directory_path( ec ) /
				   ( "hammer_disk_store_" + std::to_string( std::random_device{}() ) );
	fs::create_directories( dir, ec );
	if ( ec )
	{
		std::printf( "FAIL: could not create temp dir %s\n", dir.string().c_str() );
		return 1;
	}

	{
		hammer::adapters::platform::DiskFileStore disk;
		failures += hammertest::RunFileStoreContract( disk, ( dir / "d_" ).string(), "disk" );
	}

	// In-memory fake, same contract.
	{
		hammertest::InMemoryFileStore mem;
		failures += hammertest::RunFileStoreContract( mem, "mem/", "memory" );
	}

	// Clean up the temp directory (best effort).
	fs::remove_all( dir, ec );

	if ( failures != 0 )
	{
		std::printf( "ports.file_store: %d FAILURE(S)\n", failures );
		return 1;
	}
	std::printf( "ports.file_store: DiskFileStore and InMemoryFileStore both conform\n" );
	return 0;
}
