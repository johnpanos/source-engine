//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for formats.vpk_archive.v1. Builds VPK v1
//			and v2 fixtures with an independent serializer and proves the reader
//			resolves inline, preload-plus-chunk, external-archive, and root-level
//			entries, plus enumeration. Build/run via the conformance manifest
//			(linux-headless-core).
//
//=============================================================================//

#include "formats/fake_byte_store.h"

#include "hammer/formats/vpk_archive.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace
{
int g_failures = 0;
}
#define CHECK( cond, msg )                              \
	do                                                  \
	{                                                   \
		if ( !( cond ) )                                \
		{                                               \
			std::printf( "FAIL: %s\n", ( msg ) );       \
			++g_failures;                               \
		}                                               \
	} while ( 0 )

using hammer::formats::VpkArchive;

// Reads and compares a whole asset back to its original bytes.
static void ExpectContent( const VpkArchive &vpk, const std::string &path, const std::string &expected )
{
	std::string out;
	if ( !vpk.ReadAsset( path, out ) )
	{
		std::printf( "FAIL: ReadAsset('%s') returned false\n", path.c_str() );
		++g_failures;
		return;
	}
	if ( out != expected )
	{
		std::printf( "FAIL: content mismatch for '%s' (got %zu bytes, want %zu)\n",
		    path.c_str(), out.size(), expected.size() );
		++g_failures;
	}
}

static void RunForVersion( std::uint32_t version )
{
	const std::string label = "v" + std::to_string( version );

	// Mix of storage kinds:
	//  - inline chunk in the directory's own data section (default 0x7fff)
	//  - a preload-only file (all bytes inline in the tree, no chunk)
	//  - a preload + chunk file
	//  - a file in an external _000.vpk archive
	//  - a root-level file (no directory component)
	std::vector<hammertest::VpkBuildFile> files;
	files.push_back( { "materials/concrete/floor.vtf", std::string( 300, 'A' ), 0, 0x7fff } );
	files.push_back( { "materials/concrete/floor.vmt", std::string( "\"LightmappedGeneric\"{}" ), 21, 0x7fff } );
	files.push_back( { "materials/metal/plate.vtf", std::string( 500, 'B' ), 16, 0x7fff } );
	files.push_back( { "sound/ambient/hum.wav", std::string( 128, 'C' ), 0, 0 } ); // external archive 0
	files.push_back( { "readme.txt", std::string( "hello root file" ), 0, 0x7fff } );

	std::map<int, std::string> external;
	std::string dirBlob = hammertest::BuildVpk( files, external, version );

	hammertest::InMemoryByteStore store;
	store.files["game/pak_dir.vpk"] = dirBlob;
	for ( const auto &kv : external )
	{
		char name[64];
		std::snprintf( name, sizeof( name ), "game/pak_%03d.vpk", kv.first );
		store.files[name] = kv.second;
	}

	std::string error;
	std::unique_ptr<VpkArchive> vpk = VpkArchive::Open( store, "game/pak_dir.vpk", error );
	CHECK( vpk != nullptr, ( label + ": Open should succeed" ).c_str() );
	if ( !vpk )
		return;

	CHECK( vpk->Version() == version, ( label + ": version reported" ).c_str() );
	CHECK( vpk->Entries().size() == files.size(), ( label + ": entry count" ).c_str() );

	for ( const auto &f : files )
		ExpectContent( *vpk, f.path, f.data );

	// Find / HasAsset.
	CHECK( vpk->HasAsset( "materials/concrete/floor.vtf" ), ( label + ": HasAsset hit" ).c_str() );
	CHECK( !vpk->HasAsset( "materials/nope.vtf" ), ( label + ": HasAsset miss" ).c_str() );
	CHECK( vpk->Find( "readme.txt" ) != nullptr, ( label + ": root file Find" ).c_str() );

	// ListAssets by prefix + extension.
	std::vector<std::string> vmts;
	vpk->ListAssets( "materials/", ".vtf", vmts );
	CHECK( vmts.size() == 2, ( label + ": two .vtf under materials/" ).c_str() );

	std::vector<std::string> all;
	vpk->ListAssets( "", "", all );
	CHECK( all.size() == files.size(), ( label + ": ListAssets everything" ).c_str() );

	std::vector<std::string> sounds;
	vpk->ListAssets( "sound/", "", sounds );
	CHECK( sounds.size() == 1 && sounds[0] == "sound/ambient/hum.wav",
	    ( label + ": prefix-only listing" ).c_str() );
}

int main()
{
	RunForVersion( 1 );
	RunForVersion( 2 );

	// Empty archive (only the end-of-tree terminator) opens cleanly with no entries.
	{
		std::vector<hammertest::VpkBuildFile> none;
		std::map<int, std::string> external;
		hammertest::InMemoryByteStore store;
		store.files["e_dir.vpk"] = hammertest::BuildVpk( none, external, 2 );
		std::string error;
		auto vpk = VpkArchive::Open( store, "e_dir.vpk", error );
		CHECK( vpk != nullptr, "empty archive opens" );
		CHECK( vpk && vpk->Entries().empty(), "empty archive has no entries" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.vpk_archive: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.vpk_archive: all cases passed\n" );
	return 0;
}
