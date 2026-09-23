//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0008 F3 packaged KTX2 reader and ownership conformance.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "texturecontainer/texture_image.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <vector>

namespace
{

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
	if ( argc != 2 && argc != 4 )
	{
		std::fprintf(
		    stderr, "usage: ktx2_reader_conformance FIXTURE_DIRECTORY [HDR_KTX2 HDR_RAW]\n" );
		return 2;
	}
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

	const std::string directory = argv[1];
	std::vector<std::byte> encoded = LoadFixture( directory + "/red-8x8-bc7.ktx2" );
	check( !encoded.empty(), "BC7 fixture exists" );
	if ( encoded.empty() )
		return testing::ReportConformance( checks, failures );

	auto image = texturecontainer::ReadKtx2Image( encoded );
	check( image.HasValue(), "packaged BC7 KTX2 reads" );
	if ( image )
	{
		check( image.Value().format == texturecontainer::PixelFormat::Bc7Srgb,
		    "BC7 sRGB maps to the portable format" );
		check( image.Value().levels.size() == 1 && image.Value().levels[0].width == 8 &&
		           image.Value().levels[0].height == 8 &&
		           image.Value().levels[0].bytes.size() == 64,
		    "2D level dimensions and block size are exact" );
		constexpr std::array<std::uint8_t, 16> redBlock = { 0x20, 0xff, 0x3f, 0x00, 0x00, 0x00,
		    0xfc, 0xff, 0xaf, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00 };
		bool exactBlocks = image.Value().levels[0].bytes.size() == 64;
		for ( std::size_t i = 0; exactBlocks && i < 64; ++i )
			exactBlocks = image.Value().levels[0].bytes[i] == std::byte{ redBlock[i % 16] };
		check( exactBlocks, "reader preserves exact packaged BC7 blocks" );
		std::fill( encoded.begin(), encoded.end(), std::byte{ 0 } );
		check( image.Value().levels[0].bytes[0] == std::byte{ redBlock[0] },
		    "mip storage outlives and does not alias encoded input" );
	}
	std::vector<std::byte> mipPackage = LoadFixture( directory + "/red-8x8-rgba8-mips.ktx2" );
	const auto mips = texturecontainer::ReadKtx2Image( mipPackage );
	check( mips && mips.Value().format == texturecontainer::PixelFormat::Rgba8Srgb &&
	           mips.Value().levels.size() == 4,
	    "RGBA8 fallback reads all authored mip levels" );
	if ( mips && mips.Value().levels.size() == 4 )
	{
		bool exactMips = true;
		for ( std::size_t level = 0; level < 4; ++level )
		{
			const std::uint32_t side = 8U >> level;
			const auto &imageLevel = mips.Value().levels[level];
			exactMips &= imageLevel.width == side && imageLevel.height == side &&
			             imageLevel.bytes.size() == side * side * 4;
			for ( std::size_t pixel = 0; pixel + 3 < imageLevel.bytes.size(); pixel += 4 )
				exactMips &= imageLevel.bytes[pixel] == std::byte{ 255 } &&
				             imageLevel.bytes[pixel + 1] == std::byte{ 0 } &&
				             imageLevel.bytes[pixel + 2] == std::byte{ 0 } &&
				             imageLevel.bytes[pixel + 3] == std::byte{ 255 };
		}
		check( exactMips, "mip offsets, extents and pixels are exact at every level" );
	}

	const std::vector<std::byte> master = LoadFixture( directory + "/red-8x8-uastc-master.ktx2" );
	check( !master.empty(), "UASTC master fixture exists" );
	if ( !master.empty() )
	{
		const auto unsupported = texturecontainer::ReadKtx2Image( master );
		check(
		    !unsupported && unsupported.Error() == texturecontainer::ReadError::UnsupportedFormat,
		    "UASTC master is rejected before runtime upload" );
	}
	const auto empty = texturecontainer::ReadKtx2Image( {} );
	check( !empty && empty.Error() == texturecontainer::ReadError::InvalidContainer,
	    "empty input fails" );
	std::vector<std::byte> damaged = LoadFixture( directory + "/red-8x8-bc7.ktx2" );
	damaged[0] = std::byte{ 0 };
	const auto badMagic = texturecontainer::ReadKtx2Image( damaged );
	check( !badMagic && badMagic.Error() == texturecontainer::ReadError::InvalidContainer,
	    "corrupt identifier fails" );
	damaged = LoadFixture( directory + "/red-8x8-bc7.ktx2" );
	damaged.resize( 120 );
	const auto truncated = texturecontainer::ReadKtx2Image( damaged );
	check( !truncated && truncated.Error() == texturecontainer::ReadError::InvalidContainer,
	    "truncated payload fails" );
	damaged = LoadFixture( directory + "/red-8x8-bc7.ktx2" );
	damaged[44] = std::byte{ 2 };
	const auto compressed = texturecontainer::ReadKtx2Image( damaged );
	check( !compressed &&
	           compressed.Error() == texturecontainer::ReadError::UnsupportedSupercompression,
	    "declared supercompression fails before image data allocation" );
	damaged = LoadFixture( directory + "/red-8x8-bc7.ktx2" );
	for ( std::size_t byte = 88; byte < 96; ++byte )
		damaged[byte] = std::byte{ 255 };
	const auto oversized = texturecontainer::ReadKtx2Image( damaged );
	check( !oversized && oversized.Error() == texturecontainer::ReadError::TooLarge,
	    "oversized declared mip fails before image data allocation" );
	if ( argc == 4 )
	{
		std::vector<std::byte> atlasPackage = LoadFixture( argv[2] );
		const std::vector<std::byte> atlasPixels = LoadFixture( argv[3] );
		check( !atlasPackage.empty() && !atlasPixels.empty(),
		    "HDR atlas package and independent raw extraction exist" );
		if ( !atlasPackage.empty() && !atlasPixels.empty() )
		{
			const auto atlas = texturecontainer::ReadKtx2Image( atlasPackage );
			check( atlas.HasValue(), "World Stage HDR atlas reads" );
			if ( atlas )
			{
				check( atlas.Value().format == texturecontainer::PixelFormat::Rgba16Float &&
				           atlas.Value().levels.size() == 1 &&
				           atlas.Value().levels[0].width == 4096 &&
				           atlas.Value().levels[0].height == 256,
				    "World Stage atlas format, dimensions and mip count are exact" );
				check( atlas.Value().levels[0].bytes == atlasPixels,
				    "reader pixels match the independent KTX extraction exactly" );
				std::fill( atlasPackage.begin(), atlasPackage.end(), std::byte{ 0 } );
				check( atlas.Value().levels[0].bytes == atlasPixels,
				    "HDR atlas pixels outlive encoded input" );
			}
		}
	}
	return testing::ReportConformance( checks, failures );
}
