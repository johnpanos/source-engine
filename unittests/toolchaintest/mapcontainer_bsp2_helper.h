//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: C++11-compatible boundary of the fixture's C++20 BSP2 helper.
//
//=============================================================================//

#ifndef TOOLCHAINTEST_MAPCONTAINER_BSP2_HELPER_H
#define TOOLCHAINTEST_MAPCONTAINER_BSP2_HELPER_H

#include <cstddef>

// On success *ppBsp2 is provider-owned; release it with ToolchainFixtureFreeBsp2.
bool ToolchainFixtureConvertToBsp2( const unsigned char *pLegacy, size_t size,
    unsigned char **ppBsp2, size_t *pBsp2Size );
void ToolchainFixtureFreeBsp2( unsigned char *pBsp2 );

#endif // TOOLCHAINTEST_MAPCONTAINER_BSP2_HELPER_H
