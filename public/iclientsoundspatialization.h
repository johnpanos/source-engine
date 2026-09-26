//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game specific, non-entity sound positioning for the engine's sound
//          operator system (source_info "game_multi_origin"). An optional
//          interface the client DLL may export beside IBaseClientDLL, whose
//          vtable (VClient017) stays unchanged. Without it the engine hears a
//          sound only at its own origin.
//
//=============================================================================//

#ifndef ICLIENTSOUNDSPATIALIZATION_H
#define ICLIENTSOUNDSPATIALIZATION_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

struct SpatializationInfo_t;

#define CLIENTSOUNDSPATIALIZATION_INTERFACE_VERSION "IClientSoundSpatialization001"

abstract_class IClientSoundSpatialization
{
public:
	// Fills info.m_pUtlVecMultiOrigins (when non-NULL) with the extra places
	// the sound at *info.pOrigin is heard from (Portal 2: through each portal
	// pair). Returns false when there is nothing to add.
	virtual bool GetSoundSpatialization( SpatializationInfo_t &info ) = 0;
};

#endif // ICLIENTSOUNDSPATIALIZATION_H
