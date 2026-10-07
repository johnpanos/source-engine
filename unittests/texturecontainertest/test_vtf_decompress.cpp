//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: VTF 7.6 (P2:CE / Strata) textures in the game's legacy texture path:
//			the shared AXC decompressor, CVTFTexture's compressed BC7 runs and
//			the material system's BC7 decode for backends without BC7.
//=============================================================================//
#include "texturecontainer/vtf_decompress.h"
#include "testing/conformance_result.h"
#include "bitmap/imageformat.h"
#include "tier1/utlbuffer.h"
#include "vtf/vtf.h"

#define BCDEC_STATIC
#define BCDEC_IMPLEMENTATION
#include "bcdec.h"

#include <zlib.h>
#ifdef VTF_DECOMPRESS_ZSTD
#include <zstd.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace
{
unsigned checks = 0, failures = 0;
void Check( bool good, const char *what )
{
	++checks;
	if ( !good )
	{
		++failures;
		std::printf( "FAIL: %s\n", what );
	}
}

using texturecontainer::vtf::CompressionMethod;
using texturecontainer::vtf::DecompressInto;

std::string RandomBytes( std::size_t size, unsigned seed )
{
	std::mt19937 random( seed );
	std::string bytes( size, '\0' );
	for ( char &b : bytes )
		b = char( random() & 0xff );
	return bytes;
}

std::string Deflate( const std::string &raw )
{
	uLongf size = compressBound( uLong( raw.size() ) );
	std::string out( size, '\0' );
	compress2( reinterpret_cast<Bytef *>( out.data() ), &size,
	    reinterpret_cast<const Bytef *>( raw.data() ), uLong( raw.size() ), 6 );
	out.resize( size );
	return out;
}

#ifdef VTF_DECOMPRESS_ZSTD
std::string Zstd( const std::string &raw )
{
	std::string out( ZSTD_compressBound( raw.size() ), '\0' );
	out.resize( ZSTD_compress( out.data(), out.size(), raw.data(), raw.size(), 3 ) );
	return out;
}
#endif

void PutU32( std::string &b, std::size_t at, std::uint32_t v )
{
	for ( int i = 0; i < 4; ++i )
		b[at + std::size_t( i )] = char( ( v >> ( 8 * i ) ) & 0xff );
}

// An independent VTF 7.6 writer: a 0x60-byte header with two resources (the
// image and Strata's AXC table), the AXC table (size, level, method, one
// stored size per mip, smallest first), then the stored runs smallest first.
std::string Vtf76( int width, int height, int format,
    const std::vector<std::string> &storedLargestFirst, std::uint16_t method,
    std::uint32_t flags = 0 )
{
	const std::size_t mips = storedLargestFirst.size(), header = 0x60;
	const std::size_t axcBytes = ( mips + 2 ) * 4, image = header + axcBytes;
	std::string b( image, '\0' );
	std::memcpy( b.data(), "VTF", 4 );
	PutU32( b, 0x04, 7 );
	PutU32( b, 0x08, 6 );
	PutU32( b, 0x0c, std::uint32_t( header ) );
	b[0x10] = char( width & 0xff );
	b[0x11] = char( width >> 8 );
	b[0x12] = char( height & 0xff );
	b[0x13] = char( height >> 8 );
	PutU32( b, 0x14, flags );
	b[0x18] = 1; // frames
	PutU32( b, 0x34, std::uint32_t( format ) );
	b[0x38] = char( mips );
	PutU32( b, 0x39, 0xffffffffu ); // no thumbnail
	b[0x3f] = 1;                    // depth
	PutU32( b, 0x44, 2 );           // resources
	b[0x50] = char( 0x30 );         // image
	PutU32( b, 0x54, std::uint32_t( image ) );
	std::memcpy( b.data() + 0x58, "AXC", 3 );
	PutU32( b, 0x5c, std::uint32_t( header ) );
	PutU32( b, header, std::uint32_t( axcBytes - 4 ) );
	b[header + 4] = 1; // level
	b[header + 6] = char( method & 0xff );
	b[header + 7] = char( method >> 8 );
	for ( std::size_t i = 0; i < mips; ++i )
	{
		const std::string &run = storedLargestFirst[mips - 1 - i];
		PutU32( b, header + 8 + i * 4, std::uint32_t( run.size() ) );
		b += run;
	}
	return b;
}

void DecompressorCases()
{
	const std::string raw = RandomBytes( 4096, 1 ) + std::string( 4096, 'x' );
	std::vector<unsigned char> out( raw.size() );
	const std::string deflated = Deflate( raw );
	Check( !DecompressInto( CompressionMethod::Deflate, deflated.data(), deflated.size(),
	           out.data(), out.size() ) &&
	           std::memcmp( out.data(), raw.data(), raw.size() ) == 0,
	    "Deflate run round-trips" );
	Check( DecompressInto( CompressionMethod::Deflate, deflated.data(), deflated.size(), out.data(),
	           out.size() - 1 ) != nullptr,
	    "Deflate run larger than its image is refused" );
	std::vector<unsigned char> longer( raw.size() + 1 );
	Check( DecompressInto( CompressionMethod::Deflate, deflated.data(), deflated.size(),
	           longer.data(), longer.size() ) != nullptr,
	    "Deflate run shorter than its image is refused" );
	const std::string garbage = RandomBytes( 64, 2 );
	Check( DecompressInto( CompressionMethod::Deflate, garbage.data(), garbage.size(), out.data(),
	           out.size() ) != nullptr,
	    "corrupt Deflate run is refused" );
	Check( DecompressInto( CompressionMethod( 7 ), deflated.data(), deflated.size(), out.data(),
	           out.size() ) != nullptr,
	    "unknown method is refused" );
#ifdef VTF_DECOMPRESS_ZSTD
	Check( texturecontainer::vtf::SupportsDecompression( CompressionMethod::Zstandard ),
	    "this build decodes Zstandard" );
	const std::string zstd = Zstd( raw );
	std::fill( out.begin(), out.end(), 0 );
	Check( !DecompressInto(
	           CompressionMethod::Zstandard, zstd.data(), zstd.size(), out.data(), out.size() ) &&
	           std::memcmp( out.data(), raw.data(), raw.size() ) == 0,
	    "Zstandard run round-trips" );
	Check( DecompressInto( CompressionMethod::Zstandard, zstd.data(), zstd.size(), longer.data(),
	           longer.size() ) != nullptr,
	    "Zstandard run shorter than its image is refused" );
	Check( DecompressInto( CompressionMethod::Zstandard, garbage.data(), garbage.size(), out.data(),
	           out.size() ) != nullptr,
	    "corrupt Zstandard run is refused" );
#else
	Check( !texturecontainer::vtf::SupportsDecompression( CompressionMethod::Zstandard ) &&
	           DecompressInto( CompressionMethod::Zstandard, garbage.data(), garbage.size(),
	               out.data(), out.size() ) != nullptr,
	    "a build without libzstd refuses Zstandard by name" );
#endif
}

// An 8x8 BC7 texture with four mips (8x8, 4x4, 2x2, 1x1 = 64, 16, 16, 16 bytes),
// arbitrary blocks, every run compressed by `encode`.
template <typename Encode>
void GameLoadsCompressedBc7( const char *name, std::uint16_t method, Encode encode )
{
	const int sizes[4] = { 64, 16, 16, 16 };
	std::vector<std::string> raw, stored;
	for ( int mip = 0; mip < 4; ++mip )
	{
		raw.push_back( RandomBytes( std::size_t( sizes[mip] ), 10 + mip ) );
		stored.push_back( encode( raw.back() ) );
	}
	// Strata marks color textures sRGB with 0x00080000 (TEXTUREFLAGS_STAGING_MEMORY here).
	const std::string file = Vtf76( 8, 8, 70, stored, method, 0x00080000u | TEXTUREFLAGS_EIGHTBITALPHA );
	CUtlBuffer buffer( file.data(), int( file.size() ), CUtlBuffer::READ_ONLY );
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> texture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	const bool loaded = texture->Unserialize( buffer );
	std::string what = std::string( "game loads a VTF 7.6 BC7 texture with " ) + name + " runs";
	Check( loaded, what.c_str() );
	if ( !loaded )
		return;
	Check( texture->Format() == IMAGE_FORMAT_BC7, "VTF format 70 is IMAGE_FORMAT_BC7" );
	Check( !( texture->Flags() & TEXTUREFLAGS_STAGING_MEMORY ) &&
	           ( texture->Flags() & TEXTUREFLAGS_EIGHTBITALPHA ),
	    "a VTF 7.6 sRGB flag is not read as staging memory; other flags stay" );
	bool same = texture->MipCount() == 4;
	for ( int mip = 0; same && mip < 4; ++mip )
		same = std::memcmp( texture->ImageData( 0, 0, mip ), raw[std::size_t( mip )].data(),
		           std::size_t( sizes[mip] ) ) == 0;
	what = std::string( "every decompressed " ) + name + " mip equals its blocks";
	Check( same, what.c_str() );

	// One corrupt run fails the load instead of leaving stale texels.
	std::vector<std::string> corrupt = stored;
	corrupt[1] = RandomBytes( corrupt[1].size(), 99 );
	const std::string bad = Vtf76( 8, 8, 70, corrupt, method );
	CUtlBuffer badBuffer( bad.data(), int( bad.size() ), CUtlBuffer::READ_ONLY );
	std::unique_ptr<IVTFTexture, decltype( &DestroyVTFTexture )> badTexture(
	    CreateVTFTexture(), &DestroyVTFTexture );
	what = std::string( "a corrupt " ) + name + " run fails the load";
	Check( !badTexture->Unserialize( badBuffer ), what.c_str() );
}

void Bc7Decode()
{
	Check( ImageLoader::IsCompressed( IMAGE_FORMAT_BC7 ) &&
	           ImageLoader::GetMemRequired( 8, 8, 1, IMAGE_FORMAT_BC7, false ) == 64 &&
	           ImageLoader::GetMemRequired( 2, 2, 1, IMAGE_FORMAT_BC7, false ) == 16,
	    "BC7 is a 16-byte block format" );
	// 12x8: three blocks across, two down.
	const std::string blocks = RandomBytes( 6 * 16, 5 );
	const auto *src = reinterpret_cast<const uint8 *>( blocks.data() );
	std::vector<uint8> rgba( 12 * 8 * 4 ), bgra( 12 * 8 * 4 ), copy( blocks.size() );
	Check( ImageLoader::ConvertImageFormat(
	           src, IMAGE_FORMAT_BC7, rgba.data(), IMAGE_FORMAT_RGBA8888, 12, 8 ),
	    "BC7 decodes to RGBA8888" );
	Check( ImageLoader::ConvertImageFormat(
	           src, IMAGE_FORMAT_BC7, bgra.data(), IMAGE_FORMAT_BGRA8888, 12, 8 ),
	    "BC7 decodes to BGRA8888" );
	bool rgbaSame = true, bgraSame = true;
	for ( int block = 0; block < 6; ++block )
	{
		uint8 texels[4 * 4 * 4];
		bcdec_bc7( src + block * 16, texels, 16 );
		const int bx = block % 3, by = block / 3;
		for ( int y = 0; y < 4; ++y )
			for ( int x = 0; x < 4; ++x )
			{
				const uint8 *t = texels + ( y * 4 + x ) * 4;
				const std::size_t at = std::size_t( ( ( by * 4 + y ) * 12 + bx * 4 + x ) * 4 );
				rgbaSame = rgbaSame && std::memcmp( &rgba[at], t, 4 ) == 0;
				bgraSame = bgraSame && bgra[at] == t[2] && bgra[at + 1] == t[1] &&
				           bgra[at + 2] == t[0] && bgra[at + 3] == t[3];
			}
	}
	Check( rgbaSame, "RGBA texels sit at their blocks' positions" );
	Check( bgraSame, "BGRA swaps red and blue only" );
	Check( ImageLoader::ConvertImageFormat(
	           src, IMAGE_FORMAT_BC7, copy.data(), IMAGE_FORMAT_BC7, 12, 8 ) &&
	           std::memcmp( copy.data(), src, blocks.size() ) == 0,
	    "BC7 to BC7 copies the blocks" );
	std::vector<uint8> rgb( 12 * 8 * 3 );
	Check( !ImageLoader::ConvertImageFormat(
	           src, IMAGE_FORMAT_BC7, rgb.data(), IMAGE_FORMAT_RGB888, 12, 8 ),
	    "BC7 to an undeclared format is refused" );
}
} // namespace

int main()
{
	DecompressorCases();
	GameLoadsCompressedBc7( "Deflate", 8, Deflate );
#ifdef VTF_DECOMPRESS_ZSTD
	GameLoadsCompressedBc7( "Zstandard", 93, Zstd );
#endif
	Bc7Decode();
	return testing::ReportConformance( checks, failures );
}
