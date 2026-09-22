//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for formats.material_catalog.v1. Covers
//			material-name canonicalization, catalog enumeration, and the full
//			VMT -> $basetexture -> VTF -> RGBA resolution (including one-level patch
//			include) through an in-memory asset source, built on the shared VMT
//			parser (hammer::formats::ParseMaterial). Build/run via the conformance
//			manifest (linux-headless-core).
//
//=============================================================================//

#include "formats/fake_asset_source.h"
#include "formats/fake_vtf.h"

#include "hammer/formats/material_catalog.h"

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

using namespace hammer::formats;

int main()
{
	// --- Canonicalization --------------------------------------------------------
	CHECK( CanonicalizeMaterialName( "Concrete\\Floor001A.vmt" ) == "concrete/floor001a",
	    "canonicalize case+backslash+ext" );
	CHECK( CanonicalizeMaterialName( "/materials/metal/plate" ) == "metal/plate",
	    "canonicalize leading slash + materials prefix" );
	CHECK(
	    CanonicalizeMaterialName( "TILE/floor.VTF" ) == "tile/floor", "canonicalize .vtf strip" );

	// --- Catalog: enumeration + end-to-end resolution to RGBA --------------------
	hammertest::InMemoryAssetSource src;

	// A 1x1 BGR888 VTF whose only pixel is B=10 G=20 R=30 -> RGBA (30,20,10,255).
	std::string pixel;
	const unsigned char px[] = { 10, 20, 30 };
	pixel.assign( reinterpret_cast<const char *>( px ), sizeof( px ) );
	src.assets["materials/concrete/floor001a.vtf"] =
	    hammertest::BuildVtf( 1, 1, hammertest::VTF_FMT_BGR888, 1, { pixel }, false );
	src.assets["materials/concrete/floor001a.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"concrete/floor001a\" }";

	// A patch material that pulls its base texture from the included file.
	src.assets["materials/base/wall.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"concrete/floor001a\" }";
	src.assets["materials/custom/wall.vmt"] =
	    "\"patch\" { \"include\" \"materials/base/wall.vmt\" }";

	// A patch whose replace block overrides $basetexture: the parser's ResolvedParam
	// (via Material::BaseTexture) must supply it, without following the include.
	src.assets["materials/custom/override.vmt"] =
	    "\"patch\" { \"include\" \"materials/base/wall.vmt\" \"replace\" { \"$basetexture\" "
	    "\"concrete/floor001a\" } }";

	// A non-material asset that must not appear in the material listing.
	src.assets["materials/readme.txt"] = "not a material";

	MaterialCatalog catalog( src );

	const std::vector<std::string> &names = catalog.MaterialNames();
	CHECK( names.size() == 4, "four materials enumerated" );
	CHECK( std::find( names.begin(), names.end(), "concrete/floor001a" ) != names.end(),
	    "enumeration includes concrete/floor001a" );

	// Resolution accepts an authored (mixed-case, backslash) spelling.
	CHECK( catalog.ResolveBaseTexture( "Concrete\\Floor001A" ) == "concrete/floor001a",
	    "resolved base texture (authored spelling)" );

	const VtfImage *img = catalog.BaseTextureImage( "concrete/floor001a" );
	CHECK( img != nullptr, "base texture image decoded" );
	if ( img )
	{
		CHECK( img->width == 1 && img->height == 1, "image dimensions" );
		CHECK( img->rgba.size() == 4 && img->rgba[0] == 30 && img->rgba[1] == 20 &&
		           img->rgba[2] == 10 && img->rgba[3] == 255,
		    "image RGBA end-to-end" );
	}

	// Caching returns the same stable pointer.
	CHECK( catalog.BaseTextureImage( "concrete/floor001a" ) == img, "image cache stable pointer" );

	// Patch resolves its base texture through the include.
	CHECK( catalog.ResolveBaseTexture( "custom/wall" ) == "concrete/floor001a",
	    "patch resolves base texture via include" );
	CHECK( catalog.BaseTextureImage( "custom/wall" ) != nullptr, "patch base texture decodes" );

	// Patch with a replace-block override resolves via the parser's ResolvedParam.
	CHECK( catalog.ResolveBaseTexture( "custom/override" ) == "concrete/floor001a",
	    "patch replace override resolves via ResolvedParam" );

	if ( g_failures != 0 )
	{
		std::printf( "formats.material_catalog: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.material_catalog: all cases passed\n" );
	return 0;
}
