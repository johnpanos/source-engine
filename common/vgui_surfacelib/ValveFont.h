//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Decoder for Valve's obfuscated font files (.vfont).
//
// A .vfont is a TrueType/OpenType file, XOR-chained with a key that is seeded
// by "salt" bytes appended after the font data, followed by a count byte and
// the "VFONT1" tag:
//
//     encoded font | salt[n - 1] | n | "VFONT1"
//
// Later branches decode these in the surface before registering the font.
//=============================================================================//

#ifndef VGUI_SURFACELIB_VALVEFONT_H
#define VGUI_SURFACELIB_VALVEFONT_H
#ifdef _WIN32
#pragma once
#endif

#include <string.h>

namespace ValveFont
{

enum DecodeResult_t
{
	DECODE_NOT_ENCODED, // no VFONT1 tag: the buffer is left unchanged
	DECODE_OK,          // decoded in place; nDecodedBytes is the font size
	DECODE_MALFORMED,   // tagged, but the salt count does not fit the buffer
};

//-----------------------------------------------------------------------------
// Decodes pData in place. The decoded font occupies the first nDecodedBytes.
//-----------------------------------------------------------------------------
inline DecodeResult_t DecodeFont( unsigned char *pData, int nBytes, int &nDecodedBytes )
{
	static const char s_szTag[] = "VFONT1";
	const int nTagBytes = sizeof( s_szTag ) - 1;

	nDecodedBytes = 0;
	if ( !pData || nBytes <= nTagBytes || memcmp( pData + nBytes - nTagBytes, s_szTag, nTagBytes ) )
		return DECODE_NOT_ENCODED;

	const int nPayloadBytes = nBytes - nTagBytes;
	const int nSaltBytes = pData[nPayloadBytes - 1]; // includes the count byte itself
	if ( nSaltBytes < 1 || nSaltBytes >= nPayloadBytes )
		return DECODE_MALFORMED;

	const int nFontBytes = nPayloadBytes - nSaltBytes;
	unsigned char key = 0xA7;
	for ( int i = nFontBytes; i < nPayloadBytes - 1; ++i )
	{
		key ^= (unsigned char)( pData[i] + 0xA7 );
	}

	for ( int i = 0; i < nFontBytes; ++i )
	{
		const unsigned char encoded = pData[i];
		pData[i] = encoded ^ key;
		key = (unsigned char)( encoded + 0xA7 );
	}

	nDecodedBytes = nFontBytes;
	return DECODE_OK;
}

} // namespace ValveFont

#endif // VGUI_SURFACELIB_VALVEFONT_H
