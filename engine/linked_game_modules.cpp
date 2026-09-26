//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The game modules a statically composed product linked; see
// public/engine/linked_game_modules.h.
//
//=============================================================================//

#include "linked_game_modules_internal.h"
#include "tier0/dbg.h"
#include "tier1/strtools.h"
#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if !defined( SWDS )
extern CSysModule *g_ClientDLLModule;
#endif
extern CSysModule *g_GameDLL;

static bool s_bBound = false;
static LinkedGameModules s_Modules;
static CUtlVector<LinkedGameAppSystem> s_AppSystems;

DLL_EXPORT bool Engine_BindLinkedGameModules( const LinkedGameModules *pModules )
{
	bool bLoaded = g_GameDLL != NULL;
#if !defined( SWDS )
	bLoaded = bLoaded || g_ClientDLLModule != NULL;
#endif
	if ( s_bBound || bLoaded )
	{
		Warning( "Engine_BindLinkedGameModules: the game modules are already bound or loaded.\n" );
		return false;
	}
	if ( !pModules || !pModules->server || ( pModules->appSystemCount && !pModules->appSystems ) )
	{
		Warning( "Engine_BindLinkedGameModules: a linked game needs at least a server module.\n" );
		return false;
	}
	for ( int i = 0; i < pModules->appSystemCount; ++i )
	{
		const LinkedGameAppSystem &system = pModules->appSystems[i];
		if ( !system.interfaceName || !system.interfaceName[0] || !system.system )
		{
			Warning( "Engine_BindLinkedGameModules: linked app system %d is incomplete.\n", i );
			return false;
		}
	}

	s_Modules = *pModules;
	s_AppSystems.CopyArray( pModules->appSystems, pModules->appSystemCount );
	s_Modules.appSystems = s_AppSystems.Base();
	s_bBound = true;
	return true;
}

const LinkedGameModules *Engine_GetLinkedGameModules()
{
	return s_bBound ? &s_Modules : NULL;
}

IAppSystem *Engine_FindLinkedGameAppSystem( const char *pInterfaceName )
{
	for ( int i = 0; i < s_AppSystems.Count(); ++i )
	{
		if ( !Q_strcmp( s_AppSystems[i].interfaceName, pInterfaceName ) )
			return s_AppSystems[i].system;
	}
	return NULL;
}
