//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hammer catalog selects packaged KTX2 through the shared reader.
//
//=============================================================================//

#include "hammer/adapters/source/ktx2_preview.h"
#include "hammer/formats/material_catalog.h"
#include "testing/conformance_result.h"

#include "formats/fake_asset_source.h"
#include "formats/fake_vtf.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>

namespace
{

std::string ReadFile( const char *path )
{
	std::ifstream input( path, std::ios::binary );
	return std::string( std::istreambuf_iterator<char>{ input }, std::istreambuf_iterator<char>{} );
}

std::optional<hammer::formats::VtfImage> BadDecoder( const std::string &, std::string & )
{
	hammer::formats::VtfImage image;
	image.width = 8;
	image.height = 8;
	image.rgba.resize( 4 );
	return image;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc != 2 )
	{
		std::fprintf( stderr, "usage: hammer_ktx2_preview_conformance FIXTURE_DIRECTORY\n" );
		return 2;
	}
	int checks = 0;
	int failures = 0;
	const auto check = [&]( bool condition, const char *message )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			std::fprintf( stderr, "FAIL: %s\n", message );
		}
	};
	const std::string directory = argv[1];
	const std::string rgba = ReadFile( ( directory + "/red-8x8-rgba8-mips.ktx2" ).c_str() );
	const std::string bc7 = ReadFile( ( directory + "/red-8x8-bc7.ktx2" ).c_str() );
	check( !rgba.empty() && !bc7.empty(), "committed packages are present" );

	hammertest::InMemoryAssetSource assets;
	assets.assets["materials/test/mat.vmt"] =
	    "\"LightmappedGeneric\" { \"$basetexture\" \"test/red\" }";
	const std::string ktxPath = "materials/test/red.ktx2";
	assets.assets[ktxPath] = rgba;
	const unsigned char blue[] = { 255, 0, 0 };
	assets.assets["materials/test/red.vtf"] =
	    hammertest::BuildVtf( 1, 1, hammertest::VTF_FMT_BGR888, 1,
	        { std::string( reinterpret_cast<const char *>( blue ), sizeof( blue ) ) }, false );

	hammer::formats::MaterialCatalog ktxCatalog(
	    assets, &hammer::adapters::source::DecodeKtx2Preview );
	const hammer::formats::VtfImage *preview = ktxCatalog.BaseTextureImage( "test/mat" );
	check( preview && preview->width == 8 && preview->height == 8,
	    "Hammer catalog selects the packaged KTX2 image" );
	if ( preview )
	{
		check( preview->rgba.size() == 8 * 8 * 4 && preview->rgba[0] == 255 &&
		           preview->rgba[1] == 0 && preview->rgba[2] == 0 && preview->rgba[3] == 255,
		    "KTX2 preview pixels match the runtime reader" );
		check( ktxCatalog.BaseTextureImage( "test/mat" ) == preview,
		    "catalog caches the KTX2 preview" );
	}
	check( hammer::formats::CanonicalizeMaterialName( "TEST/RED.KTX2" ) == "test/red",
	    "canonicalizer strips KTX2 extension" );

	assets.assets[ktxPath] = bc7;
	hammer::formats::MaterialCatalog bc7Catalog(
	    assets, &hammer::adapters::source::DecodeKtx2Preview );
	check( bc7Catalog.BaseTextureImage( "test/mat" ) == nullptr,
	    "unpreviewable BC7 package does not silently select a VTF" );
	assets.assets[ktxPath] = "malformed";
	hammer::formats::MaterialCatalog badCatalog(
	    assets, &hammer::adapters::source::DecodeKtx2Preview );
	check( badCatalog.BaseTextureImage( "test/mat" ) == nullptr,
	    "malformed KTX2 package does not silently select a VTF" );
	hammer::formats::MaterialCatalog missingDecoder( assets );
	check( missingDecoder.BaseTextureImage( "test/mat" ) == nullptr,
	    "present KTX2 package requires a decoder" );
	hammer::formats::MaterialCatalog badDecoder( assets, &BadDecoder );
	check( badDecoder.BaseTextureImage( "test/mat" ) == nullptr,
	    "catalog rejects a decoder that returns incomplete RGBA pixels" );

	assets.assets.erase( ktxPath );
	hammer::formats::MaterialCatalog vtfCatalog(
	    assets, &hammer::adapters::source::DecodeKtx2Preview );
	const hammer::formats::VtfImage *vtf = vtfCatalog.BaseTextureImage( "test/mat" );
	check( vtf && vtf->width == 1 && vtf->height == 1 && vtf->rgba[2] == 255,
	    "legacy VTF still previews when no KTX2 package exists" );

	return testing::ReportConformance( checks, failures );
}
