//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for formats.vtf_image.v1. Proves the decoder rejects
//			bad signatures, unsupported versions/formats, zero dimensions, and
//			truncated image data with a diagnostic rather than returning a wrong or
//			partial image. Build/run via the conformance manifest.
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

static void ExpectRejected( const std::string &blob, const char *what )
{
	std::string error;
	auto img = DecodeVtf( blob, error );
	if ( img.has_value() )
	{
		std::printf( "FAIL: %s should be rejected but decode succeeded\n", what );
		++g_failures;
	}
	else if ( error.empty() )
	{
		std::printf( "FAIL: %s rejected without a diagnostic\n", what );
		++g_failures;
	}
}

int main()
{
	// A valid 2x2 BGR888 to mutate.
	std::string good = hammertest::BuildVtf(
	    2, 2, hammertest::VTF_FMT_BGR888, 1, { std::string( 12, '\1' ) }, false );

	// Bad signature.
	{
		std::string b = good;
		b[0] = 'X';
		ExpectRejected( b, "bad signature" );
	}
	// Unsupported major version.
	{
		std::string b = good;
		b[4] = 9; // major version 9
		ExpectRejected( b, "unsupported major version" );
	}
	// Unsupported image format.
	{
		std::string b = good;
		b[0x34] = 99; // no such ImageFormat we support
		ExpectRejected( b, "unsupported image format" );
	}
	// Zero width.
	{
		std::string b = good;
		b[0x10] = 0;
		b[0x11] = 0;
		ExpectRejected( b, "zero width" );
	}
	// Truncated file (smaller than a header).
	ExpectRejected( good.substr( 0, 0x20 ), "short header" );
	// Header valid, but image data missing (claims a mip larger than the bytes).
	{
		// Build a header for an 8x8 BGR888 with 1 mip, then chop off the pixels.
		std::string big = hammertest::BuildVtf(
		    8, 8, hammertest::VTF_FMT_BGR888, 1, { std::string( 8 * 8 * 3, '\2' ) }, false );
		ExpectRejected( big.substr( 0, big.size() - 10 ), "truncated image data" );
	}

	if ( g_failures != 0 )
	{
		std::printf( "formats.vtf_image.sensitivity: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "formats.vtf_image.sensitivity: all malformed inputs rejected\n" );
	return 0;
}
