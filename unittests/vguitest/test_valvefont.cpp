//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance of the .vfont decoder (common/vgui_surfacelib/ValveFont.h).
//          The encoder here is written from the later-branch format: payload
//          XOR-chained from a key seeded by salt bytes, then a salt count and
//          the "VFONT1" tag. Also covers untagged and malformed buffers.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "common/vgui_surfacelib/ValveFont.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace
{

unsigned long g_Checks = 0;
unsigned long g_Failures = 0;

void Check( bool condition, const char *what )
{
	++g_Checks;
	if ( !condition )
	{
		++g_Failures;
		std::printf( "FAIL: %s\n", what );
	}
}

// Encodes like the later branches: nSaltBytes includes the trailing count.
std::vector<unsigned char> Encode( const std::vector<unsigned char> &font, const std::vector<unsigned char> &salt )
{
	std::vector<unsigned char> out( font );
	unsigned char key = 0xA7;
	for ( unsigned char s : salt )
		key ^= (unsigned char)( s + 0xA7 );
	for ( unsigned char &b : out )
	{
		b ^= key;
		key = (unsigned char)( b + 0xA7 );
	}
	out.insert( out.end(), salt.begin(), salt.end() );
	out.push_back( (unsigned char)( salt.size() + 1 ) );
	const char tag[] = "VFONT1";
	out.insert( out.end(), tag, tag + 6 );
	return out;
}

std::vector<unsigned char> SampleFont( size_t size )
{
	// Starts like a TrueType file; the rest is deterministic filler.
	std::vector<unsigned char> font( size );
	for ( size_t i = 0; i < size; ++i )
		font[i] = (unsigned char)( ( i * 131 + 7 ) & 0xFF );
	if ( size >= 4 )
	{
		font[0] = 0x00;
		font[1] = 0x01;
		font[2] = 0x00;
		font[3] = 0x00;
	}
	return font;
}

void CheckRoundTrip( size_t fontSize, size_t saltSize )
{
	const std::vector<unsigned char> font = SampleFont( fontSize );
	std::vector<unsigned char> salt( saltSize );
	for ( size_t i = 0; i < saltSize; ++i )
		salt[i] = (unsigned char)( 0x5A ^ ( i * 29 ) );

	std::vector<unsigned char> buffer = Encode( font, salt );
	Check( fontSize == 0 || std::memcmp( buffer.data(), font.data(), fontSize ) != 0, "encoding changes the payload" );

	int nDecoded = -1;
	const ValveFont::DecodeResult_t result = ValveFont::DecodeFont( buffer.data(), (int)buffer.size(), nDecoded );
	Check( result == ValveFont::DECODE_OK, "tagged buffer decodes" );
	Check( nDecoded == (int)fontSize, "decoded size excludes salt, count and tag" );
	Check( nDecoded >= 0 && std::memcmp( buffer.data(), font.data(), fontSize ) == 0, "decoded bytes equal the font" );
}

} // namespace

int main()
{
	// Salt sizes the later-branch encoder produces (16..31) and the edges.
	CheckRoundTrip( 4096, 16 );
	CheckRoundTrip( 4096, 31 );
	CheckRoundTrip( 1, 0 );
	CheckRoundTrip( 257, 254 );

	// A plain TrueType file is left untouched.
	{
		std::vector<unsigned char> font = SampleFont( 512 );
		const std::vector<unsigned char> original = font;
		int nDecoded = -1;
		Check( ValveFont::DecodeFont( font.data(), (int)font.size(), nDecoded ) == ValveFont::DECODE_NOT_ENCODED,
			"untagged buffer is not encoded" );
		Check( nDecoded == 0 && font == original, "untagged buffer is unchanged" );
	}

	// The tag alone, a zero salt count and a count reaching the buffer start
	// are malformed rather than read out of bounds.
	{
		unsigned char tagOnly[] = { 'V', 'F', 'O', 'N', 'T', '1' };
		int nDecoded = -1;
		Check( ValveFont::DecodeFont( tagOnly, sizeof( tagOnly ), nDecoded ) == ValveFont::DECODE_NOT_ENCODED,
			"bare tag has no payload" );

		unsigned char zeroCount[] = { 1, 2, 3, 0, 'V', 'F', 'O', 'N', 'T', '1' };
		Check( ValveFont::DecodeFont( zeroCount, sizeof( zeroCount ), nDecoded ) == ValveFont::DECODE_MALFORMED,
			"zero salt count is malformed" );

		unsigned char oversized[] = { 1, 2, 3, 4, 'V', 'F', 'O', 'N', 'T', '1' };
		Check( ValveFont::DecodeFont( oversized, sizeof( oversized ), nDecoded ) == ValveFont::DECODE_MALFORMED,
			"salt count covering the whole payload is malformed" );
		Check( nDecoded == 0, "malformed buffer reports no font bytes" );

		unsigned char countOnly[] = { 1, 'V', 'F', 'O', 'N', 'T', '1' };
		Check( ValveFont::DecodeFont( countOnly, sizeof( countOnly ), nDecoded ) == ValveFont::DECODE_MALFORMED,
			"count byte without a payload is malformed" );
	}

	// Null input.
	{
		int nDecoded = -1;
		Check( ValveFont::DecodeFont( nullptr, 0, nDecoded ) == ValveFont::DECODE_NOT_ENCODED && nDecoded == 0,
			"null buffer is not encoded" );
	}

	return testing::ReportConformance( g_Checks, g_Failures );
}
