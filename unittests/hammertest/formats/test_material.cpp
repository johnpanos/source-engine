//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for formats.material.v1. Covers VMT parsing
//			(shader + $basetexture, case-insensitive keys, one-level patch/include),
//			material-name canonicalization, catalog enumeration, and the full
//			VMT -> $basetexture -> VTF -> RGBA resolution through an in-memory asset
//			source. Build/run via the conformance manifest (linux-headless-core).
//
//=============================================================================//

#include "formats/fake_asset_source.h"
#include "formats/fake_vtf.h"

#include "hammer/formats/material.h"

#include <algorithm>
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
	// --- Canonicalization --------------------------------------------------------
	CHECK( CanonicalizeMaterialName( "Concrete\\Floor001A.vmt" ) == "concrete/floor001a",
	    "canonicalize case+backslash+ext" );
	CHECK( CanonicalizeMaterialName( "/materials/metal/plate" ) == "metal/plate",
	    "canonicalize leading slash + materials prefix" );
	CHECK( CanonicalizeMaterialName( "TILE/floor.VTF" ) == "tile/floor", "canonicalize .vtf strip" );

	// --- ParseMaterial: basic ----------------------------------------------------
	{
		MaterialInfo info;
		std::string include, err;
		bool ok = ParseMaterial(
		    "\"LightmappedGeneric\"\n{\n\t\"$baseTexture\" \"Concrete/Floor001a\"\n}\n", info, include, err );
		CHECK( ok, "ParseMaterial ok" );
		CHECK( info.shader == "lightmappedgeneric", "shader lower-cased" );
		CHECK( info.baseTexture == "concrete/floor001a", "basetexture canonical (case-insensitive key)" );
		CHECK( include.empty(), "no include for a normal material" );
	}

	// --- ParseMaterial: patch/include --------------------------------------------
	{
		MaterialInfo info;
		std::string include, err;
		bool ok = ParseMaterial(
		    "patch\n{\n\tinclude \"materials/base/wall.vmt\"\n\treplace\n\t{\n\t\t\"$basetexture\" "
		    "\"custom/wall\"\n\t}\n}\n",
		    info, include, err );
		CHECK( ok, "ParseMaterial patch ok" );
		CHECK( info.shader == "patch", "patch shader" );
		CHECK( include == "base/wall", "patch include canonicalized" );
		CHECK( info.baseTexture == "custom/wall", "patch replace $basetexture found in child block" );
	}

	// --- Catalog: enumeration + end-to-end resolution to RGBA --------------------
	{
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
		    "patch { include \"materials/base/wall.vmt\" }";

		// A non-material asset that must not appear in the material listing.
		src.assets["materials/readme.txt"] = "not a material";

		MaterialCatalog catalog( src );

		const std::vector<std::string> &names = catalog.MaterialNames();
		CHECK( names.size() == 3, "three materials enumerated" );
		CHECK(
		    std::find( names.begin(), names.end(), "concrete/floor001a" ) != names.end(),
		    "enumeration includes concrete/floor001a" );

		const MaterialInfo *m = catalog.Material( "Concrete\\Floor001A" );
		CHECK( m != nullptr, "Material resolves an authored (mixed-case, backslash) name" );
		CHECK( m && m->baseTexture == "concrete/floor001a", "resolved base texture" );

		const VtfImage *img = catalog.BaseTextureImage( "concrete/floor001a" );
		CHECK( img != nullptr, "base texture image decoded" );
		if ( img )
		{
			CHECK( img->width == 1 && img->height == 1, "image dimensions" );
			CHECK( img->rgba.size() == 4 && img->rgba[0] == 30 && img->rgba[1] == 20
			        && img->rgba[2] == 10 && img->rgba[3] == 255,
			    "image RGBA end-to-end" );
		}

		// Caching returns the same stable pointer.
		CHECK( catalog.BaseTextureImage( "concrete/floor001a" ) == img, "image cache stable pointer" );

		// Patch resolves its base texture through the include.
		const MaterialInfo *patched = catalog.Material( "custom/wall" );
		CHECK( patched != nullptr && patched->baseTexture == "concrete/floor001a",
		    "patch resolves base texture via include" );
		CHECK( catalog.BaseTextureImage( "custom/wall" ) != nullptr, "patch base texture decodes" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.material: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.material: all cases passed\n" );
	return 0;
}
