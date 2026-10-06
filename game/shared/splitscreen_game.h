//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Local split-screen in the game DLLs: how many local players a game supports,
//			and the active-slot scope macros (ported from the CS:GO game's cdll_client_int.h
//			and shareddefs.h). The engine owns the slots (public/isplitscreen.h).
//
//			Client DLL: the macros ask the engine, so a game with one local player iterates
//			slot 0 only. Server DLL: one slot, as every local player is a client of its own there.
//
//=============================================================================//

#ifndef SPLITSCREEN_GAME_H
#define SPLITSCREEN_GAME_H
#ifdef _WIN32
#pragma once
#endif

// A game opts in to more than one local player by defining this first (portal2_base_compat.h).
#ifndef MAX_SPLITSCREEN_PLAYERS
#define MAX_SPLITSCREEN_PLAYERS 1
#endif

#ifdef CLIENT_DLL

#include "isplitscreen.h"

class C_BaseEntity;

// The engine's slot service; NULL when the engine predates it (then there is one local player).
extern IEngineSplitScreen *g_pEngineSplitScreen;

// The split-screen slot of the local player entity (0 for any other entity).
int GetSplitScreenSlotForEntity( C_BaseEntity *pEntity );

inline int GameActiveSplitScreenSlot()
{
	return g_pEngineSplitScreen ? g_pEngineSplitScreen->GetActiveSplitScreenPlayerSlot() : 0;
}

inline int GameFirstValidSplitScreenSlot()
{
	return g_pEngineSplitScreen ? g_pEngineSplitScreen->FirstValidSplitScreenSlot() : 0;
}

inline int GameNextValidSplitScreenSlot( int nPrevious )
{
	return g_pEngineSplitScreen ? g_pEngineSplitScreen->NextValidSplitScreenSlot( nPrevious ) : -1;
}

inline bool GameIsValidSplitScreenSlot( int nSlot )
{
	return g_pEngineSplitScreen ? g_pEngineSplitScreen->IsValidSplitScreenSlot( nSlot ) : ( nSlot == 0 );
}

// Makes a slot the active one for a scope (the engine's CSetActiveSplitScreenPlayerGuard), and
// lets "the local player" be resolved inside it.
class CSetActiveSplitScreenPlayerGuard
{
public:
	CSetActiveSplitScreenPlayerGuard( char const *pchContext, int nLine, int slot )
	{
		Set( pchContext, nLine, slot );
	}
	CSetActiveSplitScreenPlayerGuard( char const *pchContext, int nLine, C_BaseEntity *pEntity )
	{
		Set( pchContext, nLine, GetSplitScreenSlotForEntity( pEntity ) );
	}
	~CSetActiveSplitScreenPlayerGuard()
	{
		if ( g_pEngineSplitScreen )
		{
			g_pEngineSplitScreen->SetActiveSplitScreenPlayerSlot( m_nSaveSlot );
			g_pEngineSplitScreen->SetLocalPlayerIsResolvable( m_pchContext, m_nLine, m_bResolvable );
		}
	}

private:
	void Set( char const *pchContext, int nLine, int slot )
	{
		m_pchContext = pchContext;
		m_nLine = nLine;
		m_nSaveSlot = 0;
		m_bResolvable = true;
		if ( g_pEngineSplitScreen )
		{
			m_nSaveSlot = g_pEngineSplitScreen->SetActiveSplitScreenPlayerSlot( slot );
			m_bResolvable = g_pEngineSplitScreen->SetLocalPlayerIsResolvable( pchContext, nLine, true );
		}
	}

	char const *m_pchContext;
	int m_nLine;
	int m_nSaveSlot;
	bool m_bResolvable;
};

#define ACTIVE_SPLITSCREEN_PLAYER_GUARD( slot ) CSetActiveSplitScreenPlayerGuard g_SSGuard( __FILE__, __LINE__, slot );
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT( entity ) CSetActiveSplitScreenPlayerGuard g_SSGuard( __FILE__, __LINE__, entity );
#define FORCE_DEFAULT_SPLITSCREEN_PLAYER_GUARD ACTIVE_SPLITSCREEN_PLAYER_GUARD( 0 )

#define FOR_EACH_VALID_SPLITSCREEN_PLAYER( iteratorName ) \
	for ( int iteratorName = GameFirstValidSplitScreenSlot(); iteratorName != -1; iteratorName = GameNextValidSplitScreenSlot( iteratorName ) )

#define GET_ACTIVE_SPLITSCREEN_SLOT() GameActiveSplitScreenSlot()
#define GET_NUM_SPLIT_SCREEN_PLAYERS() ( g_pEngineSplitScreen && g_pEngineSplitScreen->IsSplitScreenActive() ? 2 : 1 )
#define IS_VALID_SPLIT_SCREEN_SLOT( i ) GameIsValidSplitScreenSlot( i )
#define ASSERT_LOCAL_PLAYER_RESOLVABLE() Assert( !g_pEngineSplitScreen || g_pEngineSplitScreen->IsLocalPlayerResolvable() );
#define ASSERT_LOCAL_PLAYER_NOT_RESOLVABLE() Assert( !g_pEngineSplitScreen || !g_pEngineSplitScreen->IsLocalPlayerResolvable() );

#else // !CLIENT_DLL

#include "isplitscreen.h"

// The engine's view of which clients are local split-screen players; NULL when the engine predates it.
extern IEngineServerSplitScreen *g_pEngineServerSplitScreen;

#define ACTIVE_SPLITSCREEN_PLAYER_GUARD( slot )
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT( entity )
#define FORCE_DEFAULT_SPLITSCREEN_PLAYER_GUARD
#define FOR_EACH_VALID_SPLITSCREEN_PLAYER( iteratorName ) \
	for ( int iteratorName = 0; iteratorName == 0; ++iteratorName )
#define GET_ACTIVE_SPLITSCREEN_SLOT() ( 0 )
#define GET_NUM_SPLIT_SCREEN_PLAYERS() 1
#define IS_VALID_SPLIT_SCREEN_SLOT( i ) ( ( i ) == 0 )
#define ASSERT_LOCAL_PLAYER_RESOLVABLE()
#define ASSERT_LOCAL_PLAYER_NOT_RESOLVABLE()

#endif // CLIENT_DLL

// Screen-size and absolute-position scopes follow the viewport of a slot; until the client
// renders one viewport per local player they change nothing.
#define VGUI_SCREENSIZE_SPLITSCREEN_GUARD( slot )
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD_VGUI( slot )
#define ACTIVE_SPLITSCREEN_PLAYER_GUARD_ENT_VGUI( entity )
#define VGUI_ABSPOS_SPLITSCREEN_GUARD( slot )
#define VGUI_ABSPOS_SPLITSCREEN_GUARD_INVERT( slot )
#define HACK_GETLOCALPLAYER_GUARD( desc )

#endif // SPLITSCREEN_GAME_H
