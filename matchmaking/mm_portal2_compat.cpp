//===== Copyright 1996-2009, Valve Corporation, All rights reserved. ======//
//
// Purpose: Portal 2 port: this engine's answers to the CS:GO-era engine
//          calls the matchmaking framework makes (see mm_portal2_compat.h).
//
//===========================================================================//

#include "mm_framework.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace MMPortal2
{

int GetActiveSplitScreenPlayerSlot()
{
	return 0;
}

void SetActiveSplitScreenPlayerSlot( int nSlot )
{
	if ( nSlot != 0 )
		PORTAL2_UNSUPPORTED( "split-screen player slot selection" );
}

char const *GetLevelNameShort( IVEngineClient *pEngineClient )
{
	// GetLevelName() answers "maps/<name>.bsp", or "" when no map is loaded.
	static char s_szLevelNameShort[ MAX_PATH ];
	s_szLevelNameShort[0] = 0;
	if ( !pEngineClient )
		return s_szLevelNameShort;

	char const *szLevelName = pEngineClient->GetLevelName();
	if ( !szLevelName || !*szLevelName )
		return s_szLevelNameShort;

	if ( char const *szAfterMaps = StringAfterPrefix( szLevelName, "maps/" ) )
		szLevelName = szAfterMaps;
	Q_StripExtension( szLevelName, s_szLevelNameShort, sizeof( s_szLevelNameShort ) );
	return s_szLevelNameShort;
}

bool IsTransitioningToLoad()
{
	return false;
}

int GetEngineBuildNumber()
{
	if ( IVEngineClient *pEngineClient = g_pMatchExtensions->GetIVEngineClient() )
		return pEngineClient->GetEngineBuildNumber();

	// A dedicated server has no client interface; its build number is not
	// exposed to game modules.
	PORTAL2_UNSUPPORTED( "the engine build number on a dedicated server" );
	return 0;
}

void StartLoadingScreenForCommand( IVEngineClient *pEngineClient, char const *szCommand )
{
	if ( !pEngineClient )
	{
		Warning( "Matchmaking: cannot run \"%s\" without a client.\n", szCommand );
		return;
	}
	pEngineClient->ClientCmd_Unrestricted( szCommand );
}

}
