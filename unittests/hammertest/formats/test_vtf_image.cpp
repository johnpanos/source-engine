//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for formats.vtf_image.v1. Decodes VTF
//			fixtures built by an independent serializer and checks exact RGBA
//			output for uncompressed and block-compressed formats, the
//			skip-to-mip-0 offset math across a mip chain, and both the 7.2 and
//			7.4 (resource dictionary) header layouts. Build/run via the
//			conformance manifest (linux-headless-core).
//
//=============================================================================//

#include "formats/fake_vtf.h"

#include "hammer/formats/vtf_image.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>

namespace
{
int g_checks = 0;
int g_failures = 0;
}
#define CHECK( cond, msg )                                                                         \
	do                                                                                             \
	{                                                                                              \
		++g_checks;                                                                                \
		if ( !( cond ) )                                                                           \
		{                                                                                          \
			std::printf( "FAIL: %s\n", ( msg ) );                                                  \
			++g_failures;                                                                          \
		}                                                                                          \
	} while ( 0 )

using hammer::formats::DecodeVtf;
using hammer::formats::VtfCompressionMethod;
using hammer::formats::VtfImage;

static bool DecodeFixtureMip( VtfCompressionMethod method, std::string_view encoded,
    std::span<std::uint8_t> decoded, std::string &error )
{
	if ( method != VtfCompressionMethod::Zstandard || encoded.size() != 1 )
	{
		error = "fixture decompressor received the wrong method or payload";
		return false;
	}
	if ( encoded[0] == 'R' )
	{
		constexpr std::array<std::uint8_t, 8> redBc1 = { 0x00, 0xf8, 0x00, 0xf8, 0, 0, 0, 0 };
		if ( decoded.size() % redBc1.size() != 0 )
			return false;
		for ( std::size_t offset = 0; offset < decoded.size(); offset += redBc1.size() )
			std::copy( redBc1.begin(), redBc1.end(), decoded.begin() + offset );
		return true;
	}
	if ( encoded[0] == 'B' )
	{
		// BC7 block copied from the independently generated red-8x8-bc7 KTX2
		// fixture. It decodes to opaque red in every texel.
		constexpr std::array<std::uint8_t, 16> redBc7 = {
		    0x20,
		    0xff,
		    0x3f,
		    0x00,
		    0x00,
		    0x00,
		    0xfc,
		    0xff,
		    0xaf,
		    0xaa,
		    0xaa,
		    0xaa,
		    0x00,
		    0x00,
		    0x00,
		    0x00,
		};
		if ( decoded.size() % redBc7.size() != 0 )
			return false;
		for ( std::size_t offset = 0; offset < decoded.size(); offset += redBc7.size() )
			std::copy( redBc7.begin(), redBc7.end(), decoded.begin() + offset );
		return true;
	}
	error = "fixture decompressor received an unknown payload";
	return false;
}

static void ExpectPixel(
    const VtfImage &img, int x, int y, int r, int g, int b, int a, const char *what )
{
	++g_checks;
	std::size_t i = ( std::size_t( y ) * img.width + x ) * 4;
	if ( i + 3 >= img.rgba.size() )
	{
		std::printf( "FAIL: %s pixel (%d,%d) out of range\n", what, x, y );
		++g_failures;
		return;
	}
	int gr = img.rgba[i], gg = img.rgba[i + 1], gb = img.rgba[i + 2], ga = img.rgba[i + 3];
	if ( gr != r || gg != g || gb != b || ga != a )
	{
		std::printf( "FAIL: %s pixel (%d,%d) = (%d,%d,%d,%d), want (%d,%d,%d,%d)\n", what, x, y, gr,
		    gg, gb, ga, r, g, b, a );
		++g_failures;
	}
}

int main()
{
	// --- BGR888 2x2, single mip: channel swizzle correctness ---------------------
	{
		std::string mip0;
		const unsigned char px[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 };
		mip0.assign( reinterpret_cast<const char *>( px ), sizeof( px ) );
		std::string blob =
		    hammertest::BuildVtf( 2, 2, hammertest::VTF_FMT_BGR888, 1, { mip0 }, false );
		std::string err;
		auto img = DecodeVtf( blob, err );
		CHECK( img.has_value(), "BGR888 decode succeeds" );
		if ( img )
		{
			CHECK( img->width == 2 && img->height == 2, "BGR888 dimensions" );
			ExpectPixel( *img, 0, 0, 30, 20, 10, 255, "BGR888" );
			ExpectPixel( *img, 1, 0, 60, 50, 40, 255, "BGR888" );
			ExpectPixel( *img, 0, 1, 90, 80, 70, 255, "BGR888" );
			ExpectPixel( *img, 1, 1, 120, 110, 100, 255, "BGR888" );
		}
	}

	// --- BGRA8888 2x1: alpha preserved -------------------------------------------
	{
		std::string mip0;
		const unsigned char px[] = { 1, 2, 3, 4, 5, 6, 7, 8 }; // BGRA, BGRA
		mip0.assign( reinterpret_cast<const char *>( px ), sizeof( px ) );
		std::string blob =
		    hammertest::BuildVtf( 2, 1, hammertest::VTF_FMT_BGRA8888, 1, { mip0 }, false );
		std::string err;
		auto img = DecodeVtf( blob, err );
		CHECK( img.has_value(), "BGRA8888 decode succeeds" );
		if ( img )
		{
			ExpectPixel( *img, 0, 0, 3, 2, 1, 4, "BGRA8888" );
			ExpectPixel( *img, 1, 0, 7, 6, 5, 8, "BGRA8888" );
		}
	}

	// --- DXT1 4x4 single block, known endpoints and indices ----------------------
	{
		const unsigned char blk[] = {
		    0xff,
		    0xff, // c0 = white (RGB565 0xFFFF)
		    0x00,
		    0x00, // c1 = black
		    0xe4,
		    0x00,
		    0x00,
		    0x00, // indices: (0,0)=0 (1,0)=1 (2,0)=2 (3,0)=3, rest 0
		};
		std::string mip0( reinterpret_cast<const char *>( blk ), sizeof( blk ) );
		std::string blob =
		    hammertest::BuildVtf( 4, 4, hammertest::VTF_FMT_DXT1, 1, { mip0 }, false );
		std::string err;
		auto img = DecodeVtf( blob, err );
		CHECK( img.has_value(), "DXT1 decode succeeds" );
		if ( img )
		{
			ExpectPixel( *img, 0, 0, 255, 255, 255, 255, "DXT1 idx0" );
			ExpectPixel( *img, 1, 0, 0, 0, 0, 255, "DXT1 idx1" );
			ExpectPixel( *img, 2, 0, 170, 170, 170, 255, "DXT1 idx2 (2c0+c1)/3" );
			ExpectPixel( *img, 3, 0, 85, 85, 85, 255, "DXT1 idx3 (c0+2c1)/3" );
			ExpectPixel( *img, 0, 1, 255, 255, 255, 255, "DXT1 row1 idx0" );
		}
	}

	// --- DXT5 4x4: interpolated alpha decoded, color unaffected -------------------
	{
		const unsigned char blk[] = {
		    200,
		    100, // alpha endpoints a0>a1 -> 8-value mode
		    0,
		    0,
		    0,
		    0,
		    0,
		    0, // alpha indices all 0 -> alpha = a0 = 200
		    0xff,
		    0xff,
		    0x00,
		    0x00, // color c0=white c1=black
		    0xe4,
		    0x00,
		    0x00,
		    0x00, // color indices as above
		};
		std::string mip0( reinterpret_cast<const char *>( blk ), sizeof( blk ) );
		std::string blob =
		    hammertest::BuildVtf( 4, 4, hammertest::VTF_FMT_DXT5, 1, { mip0 }, false );
		std::string err;
		auto img = DecodeVtf( blob, err );
		CHECK( img.has_value(), "DXT5 decode succeeds" );
		if ( img )
		{
			ExpectPixel( *img, 0, 0, 255, 255, 255, 200, "DXT5 color+alpha" );
			ExpectPixel( *img, 1, 0, 0, 0, 0, 200, "DXT5 color+alpha idx1" );
		}
	}

	// --- Strata BC7 format 70: independently generated opaque-red block ----------
	{
		const unsigned char block[] = {
		    0x20,
		    0xff,
		    0x3f,
		    0x00,
		    0x00,
		    0x00,
		    0xfc,
		    0xff,
		    0xaf,
		    0xaa,
		    0xaa,
		    0xaa,
		    0x00,
		    0x00,
		    0x00,
		    0x00,
		};
		std::string mip0( reinterpret_cast<const char *>( block ), sizeof( block ) );
		std::string blob =
		    hammertest::BuildVtf( 4, 4, hammertest::VTF_FMT_STRATA_BC7, 1, { mip0 }, true );
		std::string err;
		auto img = DecodeVtf( blob, err );
		CHECK( img.has_value(), "Strata BC7 decode succeeds" );
		if ( img )
		{
			ExpectPixel( *img, 0, 0, 255, 0, 0, 255, "BC7 red first pixel" );
			ExpectPixel( *img, 3, 3, 255, 0, 0, 255, "BC7 red last pixel" );
		}
	}

	// --- Mip chain: mip 0 must be selected past the smaller-first mips -----------
	{
		// 8x8 BGR888, 4 mips (8,4,2,1). Smaller mips filled 0xEE; mip 0 has a known
		// first pixel and zeros elsewhere. A correct decoder reads mip 0's bytes.
		std::string mip0( std::size_t( 8 ) * 8 * 3, '\0' );
		mip0[0] = 1; // B
		mip0[1] = 2; // G
		mip0[2] = 3; // R
		std::string mip1( std::size_t( 4 ) * 4 * 3, char( 0xEE ) );
		std::string mip2( std::size_t( 2 ) * 2 * 3, char( 0xEE ) );
		std::string mip3( std::size_t( 1 ) * 1 * 3, char( 0xEE ) );
		std::string blob = hammertest::BuildVtf(
		    8, 8, hammertest::VTF_FMT_BGR888, 4, { mip0, mip1, mip2, mip3 }, false );
		std::string err;
		auto img = DecodeVtf( blob, err );
		CHECK( img.has_value(), "mip-chain decode succeeds" );
		if ( img )
		{
			CHECK( img->width == 8 && img->height == 8, "mip0 dimensions" );
			ExpectPixel( *img, 0, 0, 3, 2, 1, 255, "mip0 selected (not a smaller mip)" );
		}
	}

	// --- VTF 7.6 AXC: select and decompress only the mip-0 run --------------------
	{
		std::string blob = hammertest::BuildCompressedVtf( 8, 8, hammertest::VTF_FMT_DXT1, 4,
		    { "R", "ignored-one", "ignored-two", "ignored-three" } );
		std::string err;
		auto img = DecodeVtf( blob, err, &DecodeFixtureMip );
		CHECK( img.has_value(), "VTF 7.6 Zstandard mip decode succeeds" );
		if ( img )
		{
			CHECK( img->width == 8 && img->height == 8, "VTF 7.6 dimensions" );
			ExpectPixel( *img, 0, 0, 255, 0, 0, 255, "VTF 7.6 selected mip 0" );
			ExpectPixel( *img, 7, 7, 255, 0, 0, 255, "VTF 7.6 decompressed full mip" );
		}
	}

	// P2:CE's high-quality packs combine AXC compression with Strata BC7.
	{
		std::string blob =
		    hammertest::BuildCompressedVtf( 4, 4, hammertest::VTF_FMT_STRATA_BC7, 1, { "B" } );
		std::string err;
		auto img = DecodeVtf( blob, err, &DecodeFixtureMip );
		CHECK( img.has_value(), "VTF 7.6 compressed BC7 decode succeeds" );
		if ( img )
			ExpectPixel( *img, 2, 2, 255, 0, 0, 255, "VTF 7.6 compressed BC7 pixel" );
	}

	// --- 7.4 resource-dictionary layout decodes identically ----------------------
	{
		std::string mip0;
		const unsigned char px[] = { 10, 20, 30 };
		mip0.assign( reinterpret_cast<const char *>( px ), sizeof( px ) );
		std::string blob =
		    hammertest::BuildVtf( 1, 1, hammertest::VTF_FMT_BGR888, 1, { mip0 }, true );
		std::string err;
		auto img = DecodeVtf( blob, err );
		CHECK( img.has_value(), "7.4 resource-dict decode succeeds" );
		if ( img )
			ExpectPixel( *img, 0, 0, 30, 20, 10, 255, "7.4 resource-dict pixel" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.vtf_image: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "formats.vtf_image: all cases passed\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
