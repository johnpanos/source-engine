//===== Copyright 1996-2009, Valve Corporation, All rights reserved. ======//
//
// Purpose: Portal 2 port: private adaptation of the CS:GO-era matchmaking
//          framework to this engine. The framework was written against the
//          CS:GO engine interfaces (IVEngineClient015+, INetSupport,
//          IServerGameDLL::ApplyGameSettings); this engine's interfaces are
//          frozen, so calls to methods they lack are routed through here and
//          mapped to this engine's equivalents or to documented unsupported
//          results.
//
//===========================================================================//

#ifndef MM_PORTAL2_COMPAT_H
#define MM_PORTAL2_COMPAT_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/dbg.h"
#include "xbox/xboxstubs.h"

class IMatchFramework;

// The CS:GO tier2 held the process's matchmaking framework pointer; here each
// game module that uses the framework defines it (the Portal 2 server sets it
// to its framework, the client to the published one).
extern IMatchFramework *g_pMatchFramework;

//-----------------------------------------------------------------------------
// Console and platform declarations from the CS:GO xboxstubs.h / platform.h
// that this tree's SDK 2013 copies lack (values as in CS:GO). This engine
// builds no console targets.
//-----------------------------------------------------------------------------
#ifndef IsGameConsole
#define IsGameConsole() false
#endif
#ifndef IsPS3
#define IsPS3() false
#endif
#ifndef IsX360
#define IsX360() false
#endif
#ifndef FCVAR_RELEASE
#define FCVAR_RELEASE 0
#endif

#ifndef INVALID_XUID
#define INVALID_XUID ((XUID) 0)
#endif
#ifndef MAX_RICHPRESENCE_SIZE
#define MAX_RICHPRESENCE_SIZE 64
#endif
#ifndef XUSER_NAME_SIZE
#define XUSER_NAME_SIZE 16
#endif

#define XPROFILE_TITLE_SPECIFIC1    0x3FFF
#define XPROFILE_TITLE_SPECIFIC2    0x3FFE
#define XPROFILE_TITLE_SPECIFIC3    0x3FFD
#define XPROFILE_SETTING_MAX_SIZE 1000

#define X_CONTEXT_PRESENCE              0x00010001
#define X_CONTEXT_GAME_TYPE             0x0001000A
#define X_CONTEXT_GAME_MODE             0x0001000B

// Values for X_CONTEXT_GAME_TYPE
#define X_CONTEXT_GAME_TYPE_RANKED      0
#define X_CONTEXT_GAME_TYPE_STANDARD    1

// IGameEventListener2::GetEventDebugID was added after this engine's interface
// was frozen; listeners here keep the CS:GO value for their own bookkeeping.
#ifndef EVENT_DEBUG_ID_INIT
#define EVENT_DEBUG_ID_INIT 42
#endif

enum XUSER_SIGNIN_STATE
{
	eXUserSigninState_NotSignedIn,
	eXUserSigninState_SignedInLocally,
	eXUserSigninState_SignedInToLive,
};

FORCEINLINE int				XBX_GetSlotByUserId( int idx ) { return idx; }
FORCEINLINE void			XBX_SetUserId( int nSlot, int idx ) {}

class IVEngineClient;
class KeyValues;

//-----------------------------------------------------------------------------
// One-time report of a matchmaking feature this engine does not provide.
//-----------------------------------------------------------------------------
#define PORTAL2_UNSUPPORTED( feature ) \
	do \
	{ \
		static bool s_bWarned = false; \
		if ( !s_bWarned ) \
		{ \
			s_bWarned = true; \
			DevWarning( "Portal 2: " feature " is not supported by this engine\n" ); \
		} \
	} while ( 0 )

// MM_PORTAL2_ONLINE_SESSIONS is never defined for this engine. It marks the
// CS:GO framework code that serves online sessions (Steam lobbies, LAN
// discovery, dedicated-server reservations): those need INetSupport, the
// lobby API and engine reservation hooks that this engine does not have.
// Requests that would create such a session fail with a documented error.

namespace MMPortal2
{
	// This engine has one local player and no split-screen slots: the active
	// slot is always 0. Selecting another slot is reported as unsupported and
	// leaves slot 0 active.
	int GetActiveSplitScreenPlayerSlot();
	void SetActiveSplitScreenPlayerSlot( int nSlot );

	// CS:GO's GetLevelNameShort: the loaded map name without "maps/" and ".bsp".
	char const *GetLevelNameShort( IVEngineClient *pEngineClient );

	// CS:GO tracked a "transitioning to load" state between a load request and
	// the loading screen. This engine starts the loading screen synchronously,
	// so there is no such state.
	bool IsTransitioningToLoad();

	// Engine build number that CS:GO took from INetSupport.
	int GetEngineBuildNumber();

	// CS:GO showed the loading screen, then ran the command. This engine
	// shows the loading screen itself when the queued command connects or
	// loads a map.
	void StartLoadingScreenForCommand( IVEngineClient *pEngineClient, char const *szCommand );
}

#endif // MM_PORTAL2_COMPAT_H
