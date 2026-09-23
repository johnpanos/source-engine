//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Engine-side (C++20) helper for the mapcontainer frozen-consumer
//          fixture: converts a legacy map to BSP2 with the real builder and
//          hands the bytes across a C++11-compatible function boundary. The
//          provider allocates and releases; the consumer only borrows.
//
//=============================================================================//

#include "mapcontainer_bsp2_helper.h"

#include "mapcontainer/map_container_builder.h"

#include <cstring>
#include <span>

#if __cplusplus < 202002L
#error "the engine-side helper must be compiled as C++20"
#endif

bool ToolchainFixtureConvertToBsp2( const unsigned char *pLegacy, size_t size,
    unsigned char **ppBsp2, size_t *pBsp2Size )
{
	*ppBsp2 = nullptr;
	*pBsp2Size = 0;
	auto converted = mapcontainer::ConvertLegacyToBsp2(
	    std::span<const std::byte>( reinterpret_cast<const std::byte *>( pLegacy ), size ) );
	if ( !converted )
		return false;
	const std::vector<std::byte> &bytes = converted.Value();
	*ppBsp2 = new unsigned char[bytes.size()];
	std::memcpy( *ppBsp2, bytes.data(), bytes.size() );
	*pBsp2Size = bytes.size();
	return true;
}

void ToolchainFixtureFreeBsp2( unsigned char *pBsp2 )
{
	delete[] pBsp2;
}
