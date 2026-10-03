//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BLAKE2b (RFC 7693), unkeyed, variable digest length. Shared by block containers and asset identities; callers choose the digest length.
//
//=============================================================================//

#ifndef CONTENT_HASH_H
#define CONTENT_HASH_H

#include <cstddef>
#include <cstdint>

namespace content
{

class Blake2b
{
public:
	// digestSize is 1..64 bytes.
	explicit Blake2b( std::size_t digestSize ) noexcept;
	void Update( const void *pData, std::size_t size ) noexcept;
	void Final( uint8_t *pDigest ) noexcept;

private:
	void Compress( bool bLast ) noexcept;

	uint64_t m_State[8];
	uint64_t m_Counter[2];
	uint8_t m_Block[128];
	std::size_t m_BlockUsed;
	std::size_t m_DigestSize;
};

} // namespace content

#endif // CONTENT_HASH_H
