//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BLAKE2b (RFC 7693), unkeyed, variable digest length. Private to
//          mapcontainer; the BSP2 content hash is BLAKE2b with a 16-byte digest.
//
//=============================================================================//

#ifndef MAPCONTAINER_BLAKE2B_H
#define MAPCONTAINER_BLAKE2B_H

#include <cstddef>
#include <cstdint>

namespace mapcontainer::detail
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

} // namespace mapcontainer::detail

#endif // MAPCONTAINER_BLAKE2B_H
