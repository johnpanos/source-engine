//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for formats.search_path_assets.v1. Proves
//			the ordered composite resolves the FIRST provider that has an asset
//			(shadowing), lists the DE-DUPLICATED union across providers, and
//			ignores null providers. This is the multi-VPK mount behavior a game
//			relies on (a .vmt in one archive naming a .vtf in another). Build/run
//			via the conformance manifest (linux-headless-core).
//
//=============================================================================//

#include "formats/fake_asset_source.h"

#include "hammer/formats/search_path_assets.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace
{
int g_failures = 0;
}
#define CHECK( cond, msg )                                                                         \
	do                                                                                             \
	{                                                                                              \
		if ( !( cond ) )                                                                           \
		{                                                                                          \
			std::printf( "FAIL: %s\n", ( msg ) );                                                  \
			++g_failures;                                                                          \
		}                                                                                          \
	} while ( 0 )

int main()
{
	hammertest::InMemoryAssetSource high; // first in search order
	hammertest::InMemoryAssetSource low;  // fallback

	high.assets["materials/a.vmt"] = "HIGH-A";
	high.assets["materials/shared.vmt"] = "HIGH-SHARED";
	low.assets["materials/shared.vmt"] = "LOW-SHARED"; // shadowed by 'high'
	low.assets["materials/b.vmt"] = "LOW-B";
	low.assets["materials/tex.vtf"] = "LOW-TEX";

	hammer::formats::SearchPathAssets search;
	search.AddProvider( nullptr ); // ignored
	search.AddProvider( &high );
	search.AddProvider( &low );
	CHECK( search.ProviderCount() == 2, "null provider ignored" );

	// First-hit-wins for a shadowed path.
	std::string out;
	CHECK( search.ReadAsset( "materials/shared.vmt", out ) && out == "HIGH-SHARED",
	    "shadowed asset served from the first provider" );
	CHECK(
	    search.ReadAsset( "materials/b.vmt", out ) && out == "LOW-B", "fallback provider served" );
	CHECK( search.HasAsset( "materials/tex.vtf" ), "HasAsset across providers" );
	CHECK( !search.HasAsset( "materials/missing.vmt" ), "absent everywhere is a miss" );
	CHECK( !search.ReadAsset( "materials/missing.vmt", out ), "absent read fails" );

	// Union listing is de-duplicated: shared.vmt appears once.
	std::vector<std::string> vmts;
	search.ListAssets( "materials/", ".vmt", vmts );
	std::sort( vmts.begin(), vmts.end() );
	CHECK( vmts.size() == 3, "union of .vmt de-duplicated (a, b, shared)" );
	CHECK( std::find( vmts.begin(), vmts.end(), "materials/shared.vmt" ) != vmts.end() &&
	           std::count( vmts.begin(), vmts.end(), "materials/shared.vmt" ) == 1,
	    "shared listed exactly once" );

	std::vector<std::string> all;
	search.ListAssets( "", "", all );
	CHECK( all.size() == 4, "union of all assets de-duplicated (a, b, shared, tex)" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.search_path_assets: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.search_path_assets: all cases passed\n" );
	return 0;
}
