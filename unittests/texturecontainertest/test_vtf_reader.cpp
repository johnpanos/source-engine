//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0008 F3 VTF/KTX2 owned image description conformance.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "texturecontainer/texture_image.h"
#include "texturecontainer/vtf_image_reader.h"

#include "tier1/utlbuffer.h"
#include "vtf/vtf.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace
{

std::vector<std::byte> RedVtf( int frames )
{
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	if ( !texture ||
	     !texture->Init( 8, 8, 1, IMAGE_FORMAT_RGBA8888, TEXTUREFLAGS_SRGB, frames, 4 ) )
		return {};
	for ( int frame = 0; frame < frames; ++frame )
	{
		for ( int mip = 0; mip < 4; ++mip )
		{
			const int bytes = texture->ComputeMipSize( mip );
			unsigned char *pixels = texture->ImageData( frame, 0, mip );
			if ( !pixels || bytes <= 0 || bytes % 4 != 0 )
				return {};
			for ( int pixel = 0; pixel < bytes; pixel += 4 )
			{
				pixels[pixel] = 255;
				pixels[pixel + 1] = 0;
				pixels[pixel + 2] = 0;
				pixels[pixel + 3] = 255;
			}
		}
	}
	CUtlBuffer buffer;
	if ( !texture->Serialize( buffer ) || buffer.TellPut() <= 0 )
		return {};
	const auto *first = static_cast<const std::byte *>( buffer.Base() );
	return std::vector<std::byte>( first, first + buffer.TellPut() );
}

std::vector<std::byte> RedBc1Vtf()
{
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	if ( !texture || !texture->Init( 8, 8, 1, IMAGE_FORMAT_DXT1,
	                     TEXTUREFLAGS_SRGB | TEXTUREFLAGS_NOMIP, 1, 1 ) )
		return {};
	constexpr std::array<unsigned char, 8> block = { 0x00, 0xf8, 0x00, 0xf8, 0, 0, 0, 0 };
	unsigned char *pixels = texture->ImageData( 0, 0, 0 );
	if ( !pixels || texture->ComputeMipSize( 0 ) != 32 )
		return {};
	for ( int i = 0; i < 4; ++i )
		std::copy( block.begin(), block.end(), pixels + i * block.size() );
	CUtlBuffer buffer;
	if ( !texture->Serialize( buffer ) || buffer.TellPut() <= 0 )
		return {};
	const auto *first = static_cast<const std::byte *>( buffer.Base() );
	return std::vector<std::byte>( first, first + buffer.TellPut() );
}

std::vector<std::byte> LoadFixture( const std::string &path )
{
	std::ifstream file( path, std::ios::binary );
	std::vector<char> source(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
	std::vector<std::byte> result( source.size() );
	for ( std::size_t i = 0; i < source.size(); ++i )
		result[i] = std::byte{ static_cast<unsigned char>( source[i] ) };
	return result;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc != 2 )
		return 2;
	unsigned long checks = 0;
	unsigned long failures = 0;
	const auto check = [&]( bool condition, const char *message )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			std::fprintf( stderr, "FAIL: %s\n", message );
		}
	};
	std::vector<std::byte> vtf = RedVtf( 1 );
	check( !vtf.empty(), "legacy VTF writer produces the 2D fixture" );
	if ( vtf.empty() )
		return testing::ReportConformance( checks, failures );
	const auto legacy = texturecontainer::ReadVtfImage( vtf );
	check( legacy && legacy.Value().format == texturecontainer::PixelFormat::Rgba8Srgb &&
	           legacy.Value().levels.size() == 4,
	    "VTF reader produces owned sRGB levels" );
	const auto ktx = texturecontainer::ReadKtx2Image(
	    LoadFixture( std::string( argv[1] ) + "/red-8x8-rgba8-mips.ktx2" ) );
	check( ktx && ktx.Value().format == texturecontainer::PixelFormat::Rgba8Srgb &&
	           ktx.Value().levels.size() == 4,
	    "KTX2 reader produces the same format and level count" );
	if ( legacy && ktx && legacy.Value().levels.size() == ktx.Value().levels.size() )
	{
		bool equal = true;
		for ( std::size_t mip = 0; mip < legacy.Value().levels.size(); ++mip )
		{
			const auto &left = legacy.Value().levels[mip];
			const auto &right = ktx.Value().levels[mip];
			equal &= left.width == right.width && left.height == right.height &&
			         left.bytes == right.bytes;
		}
		check( equal, "VTF and KTX2 red textures have identical owned mip data" );
	}
	else
	{
		check( false, "VTF/KTX2 level comparison can run" );
	}
	std::fill( vtf.begin(), vtf.end(), std::byte{ 0 } );
	check( legacy && legacy.Value().levels[0].bytes[0] == std::byte{ 255 },
	    "VTF image data does not alias the encoded input" );
	const auto corrupt = texturecontainer::ReadVtfImage( vtf );
	check( !corrupt && corrupt.Error() == texturecontainer::ReadError::InvalidContainer,
	    "corrupt VTF signature fails" );
	const auto truncated = texturecontainer::ReadVtfImage( std::span( vtf ).first( 32 ) );
	check( !truncated && truncated.Error() == texturecontainer::ReadError::InvalidContainer,
	    "truncated VTF header fails" );
	const std::vector<std::byte> animated = RedVtf( 2 );
	const auto animation = texturecontainer::ReadVtfImage( animated );
	check( !animation && animation.Error() == texturecontainer::ReadError::UnsupportedTopology,
	    "animated VTF requests the legacy path explicitly" );
	const std::vector<std::byte> bc1Vtf = RedBc1Vtf();
	check( !bc1Vtf.empty(), "legacy writer produces the BC1 fixture" );
	if ( !bc1Vtf.empty() )
	{
		const auto legacyBc1 = texturecontainer::ReadVtfImage( bc1Vtf );
		const auto ktxBc1 = texturecontainer::ReadKtx2Image(
		    LoadFixture( std::string( argv[1] ) + "/red-8x8-bc1.ktx2" ) );
		check( legacyBc1 && ktxBc1 &&
		           legacyBc1.Value().format == texturecontainer::PixelFormat::Bc1Srgb &&
		           ktxBc1.Value().format == texturecontainer::PixelFormat::Bc1Srgb,
		    "VTF and KTX2 map BC1 sRGB to the same format" );
		check( legacyBc1 && ktxBc1 && legacyBc1.Value().levels.size() == 1 &&
		           ktxBc1.Value().levels.size() == 1 &&
		           legacyBc1.Value().levels[0].bytes == ktxBc1.Value().levels[0].bytes,
		    "VTF and KTX2 preserve the same BC1 blocks" );
	}
	return testing::ReportConformance( checks, failures );
}
