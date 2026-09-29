//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Names of the NPCs that voice PotatOS. The client's LightedMouth
//          material proxy lights PotatOS on the portal gun from their mouths;
//          the server always networks them so the client has those mouths.
//
//=============================================================================//
#ifndef POTATOS_SPEAKER_H
#define POTATOS_SPEAKER_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/strtools.h"

inline bool IsPotatosSpeakerName( const char *pszName )
{
	return pszName &&
	       ( !V_stricmp( pszName, "@glados" ) || !V_stricmp( pszName, "@actor_potatos" ) );
}

#endif // POTATOS_SPEAKER_H
