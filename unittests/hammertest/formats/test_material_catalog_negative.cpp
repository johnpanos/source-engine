//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.material_catalog.v1. Proves the catalog
//			fails cleanly (and caches the miss) for absent materials, materials
//			with no $basetexture, absent textures, and corrupt VTFs -- never a
//			fabricated image. Build/run via the conformance manifest.
//
//=============================================================================//

#include "formats/fake_asset_source.h"

#include "hammer/formats/material_catalog.h"

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

using namespace hammer::formats;

int main()
{
	hammertest::InMemoryAssetSource src;
	// Present material with no texture.
	src.assets["materials/tools/nodraw.vmt"] = "\"UnlitGeneric\" { \"%compiletools\" \"1\" }";
	// Material naming a texture that does not exist.
	src.assets["materials/broken/missingtex.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"broken/nope\" }";
	// Material naming a texture whose VTF is corrupt.
	src.assets["materials/broken/corrupt.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"broken/corrupt\" }";
	src.assets["materials/broken/corrupt.vtf"] = "not a real vtf file at all";

	MaterialCatalog catalog( src );

	// Absent material -> empty/nullptr, and the miss is cached (still empty/nullptr).
	CHECK(
	    catalog.ResolveBaseTexture( "does/not/exist" ).empty(), "absent material resolves empty" );
	CHECK(
	    catalog.BaseTextureImage( "does/not/exist" ) == nullptr, "absent material has no image" );
	CHECK( catalog.BaseTextureImage( "does/not/exist" ) == nullptr,
	    "absent material stays null (cached)" );

	// Present material, but no base texture -> empty/nullptr.
	CHECK( catalog.ResolveBaseTexture( "tools/nodraw" ).empty(),
	    "no-basetexture material resolves empty" );
	CHECK( catalog.BaseTextureImage( "tools/nodraw" ) == nullptr,
	    "no-basetexture material has no image" );

	// Base texture referenced but missing -> nullptr.
	CHECK(
	    catalog.BaseTextureImage( "broken/missingtex" ) == nullptr, "missing vtf yields no image" );

	// Base texture present but corrupt -> nullptr (decode rejected).
	CHECK( catalog.BaseTextureImage( "broken/corrupt" ) == nullptr, "corrupt vtf yields no image" );
	CHECK( catalog.BaseTextureImage( "broken/corrupt" ) == nullptr,
	    "corrupt vtf stays null (cached)" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.material_catalog.sensitivity: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.material_catalog.sensitivity: all malformed inputs handled cleanly\n" );
	return 0;
}
