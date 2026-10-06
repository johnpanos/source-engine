//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A bound button's state, one per local split-screen player (ported from the
//			CS:GO client's kbutton.h).
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined( KBUTTON_H )
#define KBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include "splitscreen_game.h"

struct kbutton_t
{
	struct Split_t
	{
		// key nums holding it down
		int		down[ 2 ];		
		// low bit is down state
		int		state;			
	};

	// The state for the local player in nSlot, or in the active slot (-1)
	Split_t		&GetPerUser( int nSlot = -1 );

	Split_t		m_PerUser[ MAX_SPLITSCREEN_PLAYERS ];
};

#endif // KBUTTON_H
