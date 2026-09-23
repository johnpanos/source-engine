//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Experimental World Stage geometry output for the vbsp2 host tool.
//
//=============================================================================//

#ifndef VBSP_WORLDSTAGE_H
#define VBSP_WORLDSTAGE_H

#include <cstddef>

// C-compatible seam: the legacy VBSP target uses libstdc++'s old string ABI,
// while the pinned OpenUSD host profile uses the current one.
bool WriteWorldStageGeometry( const char *bspPath, char *error, std::size_t errorBytes );

#endif // VBSP_WORLDSTAGE_H
