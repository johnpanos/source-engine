//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Emitting version 2 sound entries (Portal 2 soundscripts with
//          "operator_stacks"). A separate, optional interface next to
//          IEngineSound, whose vtable stays unchanged: the engine exports it
//          beside IEngineSoundServer003 and IEngineSoundClient003. Game code
//          that cannot get it (an older engine) emits the entry's wave through
//          IEngineSound::EmitSound, without operator stacks.
//
//=============================================================================//

#ifndef IENGINESOUNDENTRY_H
#define IENGINESOUNDENTRY_H
#ifdef _WIN32
#pragma once
#endif

#include "engine/IEngineSound.h"

#define IENGINESOUNDENTRY_SERVER_INTERFACE_VERSION "IEngineSoundEntryServer001"
#define IENGINESOUNDENTRY_CLIENT_INTERFACE_VERSION "IEngineSoundEntryClient001"

abstract_class IEngineSoundEntry
{
public:
	// IEngineSound::EmitSound for a version 2 sound entry. pSample is the wave
	// the entry picked (it may be unprecached: the entry travels as its script
	// handle); nSoundEntryHash (ISoundEmitterSystemEntries::HashSoundName of the
	// entry name) lets the client run the entry's operator stacks.
	virtual void EmitSoundEntry( IRecipientFilter &filter, int iEntIndex, int iChannel,
	    unsigned int nSoundEntryHash, const char *pSample, float flVolume, soundlevel_t iSoundlevel,
	    int iFlags = 0, int iPitch = PITCH_NORM, int iSpecialDSP = 0, const Vector *pOrigin = NULL,
	    const Vector *pDirection = NULL, CUtlVector< Vector > *pUtlVecOrigins = NULL,
	    bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1 ) = 0;
};

#endif // IENGINESOUNDENTRY_H
