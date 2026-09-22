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

using hammer::formats::DecodeVtf;
using hammer::formats::VtfImage;

static void ExpectPixel(
    const VtfImage &img, int x, int y, int r, int g, int b, int a, const char *what )
{
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
		return 1;
	}
	std::printf( "formats.vtf_image: all cases passed\n" );
	return 0;
}
