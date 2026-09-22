//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.material.v1. Proves ParseMaterial rejects
//			non-keyvalues text and empty documents, that a material with no
//			$basetexture yields an empty base texture (not a fabricated one), and
//			that the catalog fails cleanly (and caches the miss) for absent
//			materials, absent textures, and corrupt VTFs. Build/run via the
//			conformance manifest.
//
//=============================================================================//

#include "formats/fake_asset_source.h"
#include "formats/fake_vtf.h"

#include "hammer/formats/material.h"

#include <cstdio>
#include <string>

namespace
{
int g_failures = 0;
}
#define CHECK( cond, msg )                        \
	do                                            \
	{                                             \
		if ( !( cond ) )                          \
		{                                         \
			std::printf( "FAIL: %s\n", ( msg ) ); \
			++g_failures;                         \
		}                                         \
	} while ( 0 )

using namespace hammer::formats;

int main()
{
	// Unparseable keyvalues (unbalanced brace) is rejected with a diagnostic.
	{
		MaterialInfo info;
		std::string include, err;
		bool ok = ParseMaterial( "\"Foo\" { \"$basetexture\" \"x\" ", info, include, err );
		CHECK( !ok, "unbalanced braces rejected" );
		CHECK( !err.empty(), "parse error carries a diagnostic" );
	}

	// Empty document (no top block) is rejected.
	{
		MaterialInfo info;
		std::string include, err;
		CHECK( !ParseMaterial( "   \n  // just a comment\n", info, include, err ), "empty vmt rejected" );
	}

	// A material with no $basetexture parses but has an empty base texture.
	{
		MaterialInfo info;
		std::string include, err;
		bool ok = ParseMaterial( "\"UnlitGeneric\" { \"$color\" \"[1 0 0]\" }", info, include, err );
		CHECK( ok, "material without basetexture still parses" );
		CHECK( info.baseTexture.empty(), "absent $basetexture yields empty, not fabricated" );
	}

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

	// Absent material -> nullptr, and the miss is cached (still nullptr).
	CHECK( catalog.Material( "does/not/exist" ) == nullptr, "absent material is nullptr" );
	CHECK( catalog.Material( "does/not/exist" ) == nullptr, "absent material stays nullptr (cached)" );

	// Present material, but no base texture -> image nullptr.
	CHECK( catalog.BaseTextureImage( "tools/nodraw" ) == nullptr, "no-basetexture material has no image" );

	// Base texture referenced but missing -> nullptr.
	CHECK( catalog.BaseTextureImage( "broken/missingtex" ) == nullptr, "missing vtf yields no image" );

	// Base texture present but corrupt -> nullptr (decode rejected).
	CHECK( catalog.BaseTextureImage( "broken/corrupt" ) == nullptr, "corrupt vtf yields no image" );
	CHECK( catalog.BaseTextureImage( "broken/corrupt" ) == nullptr, "corrupt vtf stays nullptr (cached)" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.material.sensitivity: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.material.sensitivity: all malformed inputs handled cleanly\n" );
	return 0;
}
