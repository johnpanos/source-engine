//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BLAKE2b following the RFC 7693 reference description.
//
//=============================================================================//

#include "blake2b.h"

#include <cstring>

namespace mapcontainer::detail
{

namespace
{
constexpr uint64_t kIv[8] = {
    0x6A09E667F3BCC908ull,
    0xBB67AE8584CAA73Bull,
    0x3C6EF372FE94F82Bull,
    0xA54FF53A5F1D36F1ull,
    0x510E527FADE682D1ull,
    0x9B05688C2B3E6C1Full,
    0x1F83D9ABFB41BD6Bull,
    0x5BE0CD19137E2179ull,
};

constexpr uint8_t kSigma[12][16] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
    { 11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
    { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
    { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
    { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
    { 12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
    { 13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
    { 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
    { 10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
};

constexpr uint64_t RotateRight( uint64_t value, int bits ) noexcept
{
	return ( value >> bits ) | ( value << ( 64 - bits ) );
}

uint64_t LoadLittle64( const uint8_t *p ) noexcept
{
	uint64_t value = 0;
	for ( int i = 7; i >= 0; --i )
		value = ( value << 8 ) | p[i];
	return value;
}

inline void Mix( uint64_t *v, int a, int b, int c, int d, uint64_t x, uint64_t y ) noexcept
{
	v[a] = v[a] + v[b] + x;
	v[d] = RotateRight( v[d] ^ v[a], 32 );
	v[c] = v[c] + v[d];
	v[b] = RotateRight( v[b] ^ v[c], 24 );
	v[a] = v[a] + v[b] + y;
	v[d] = RotateRight( v[d] ^ v[a], 16 );
	v[c] = v[c] + v[d];
	v[b] = RotateRight( v[b] ^ v[c], 63 );
}
} // namespace

Blake2b::Blake2b( std::size_t digestSize ) noexcept
    : m_Counter{ 0, 0 }, m_BlockUsed( 0 ), m_DigestSize( digestSize )
{
	for ( int i = 0; i < 8; ++i )
		m_State[i] = kIv[i];
	// Parameter block: digest length, no key, fanout 1, depth 1.
	m_State[0] ^= 0x01010000ull ^ static_cast<uint64_t>( digestSize );
	std::memset( m_Block, 0, sizeof( m_Block ) );
}

void Blake2b::Compress( bool bLast ) noexcept
{
	uint64_t v[16];
	uint64_t m[16];
	for ( int i = 0; i < 8; ++i )
	{
		v[i] = m_State[i];
		v[i + 8] = kIv[i];
	}
	v[12] ^= m_Counter[0];
	v[13] ^= m_Counter[1];
	if ( bLast )
		v[14] = ~v[14];
	for ( int i = 0; i < 16; ++i )
		m[i] = LoadLittle64( m_Block + 8 * i );

	for ( int round = 0; round < 12; ++round )
	{
		const uint8_t *s = kSigma[round];
		Mix( v, 0, 4, 8, 12, m[s[0]], m[s[1]] );
		Mix( v, 1, 5, 9, 13, m[s[2]], m[s[3]] );
		Mix( v, 2, 6, 10, 14, m[s[4]], m[s[5]] );
		Mix( v, 3, 7, 11, 15, m[s[6]], m[s[7]] );
		Mix( v, 0, 5, 10, 15, m[s[8]], m[s[9]] );
		Mix( v, 1, 6, 11, 12, m[s[10]], m[s[11]] );
		Mix( v, 2, 7, 8, 13, m[s[12]], m[s[13]] );
		Mix( v, 3, 4, 9, 14, m[s[14]], m[s[15]] );
	}

	for ( int i = 0; i < 8; ++i )
		m_State[i] ^= v[i] ^ v[i + 8];
}

void Blake2b::Update( const void *pData, std::size_t size ) noexcept
{
	const uint8_t *pBytes = static_cast<const uint8_t *>( pData );
	for ( std::size_t i = 0; i < size; ++i )
	{
		// The final block is compressed in Final, so a full buffer is only
		// flushed once more input arrives.
		if ( m_BlockUsed == sizeof( m_Block ) )
		{
			m_Counter[0] += m_BlockUsed;
			if ( m_Counter[0] < m_BlockUsed )
				++m_Counter[1];
			Compress( false );
			m_BlockUsed = 0;
		}
		m_Block[m_BlockUsed++] = pBytes[i];
	}
}

void Blake2b::Final( uint8_t *pDigest ) noexcept
{
	m_Counter[0] += m_BlockUsed;
	if ( m_Counter[0] < m_BlockUsed )
		++m_Counter[1];
	std::memset( m_Block + m_BlockUsed, 0, sizeof( m_Block ) - m_BlockUsed );
	Compress( true );
	for ( std::size_t i = 0; i < m_DigestSize; ++i )
		pDigest[i] = static_cast<uint8_t>( m_State[i >> 3] >> ( 8 * ( i & 7 ) ) );
}

} // namespace mapcontainer::detail
