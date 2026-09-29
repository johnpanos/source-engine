//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "c_neurotoxin_countdown.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CUtlVector< C_NeurotoxinCountdown* > g_NeurotoxinCountdowns;


IMPLEMENT_CLIENTCLASS_DT(C_NeurotoxinCountdown, DT_NeurotoxinCountdown, CNeurotoxinCountdown)

	RecvPropBool( RECVINFO(m_bEnabled) ),
#ifdef PORTAL2
	RecvPropFloat( RECVINFO(m_flCountdownTime) ),
#endif

END_RECV_TABLE()


C_NeurotoxinCountdown::C_NeurotoxinCountdown()
{
	g_NeurotoxinCountdowns.AddToTail( this );
}

C_NeurotoxinCountdown::~C_NeurotoxinCountdown()
{
	g_NeurotoxinCountdowns.FindAndRemove( this );
}

#ifdef PORTAL2
//-----------------------------------------------------------------------------
// Portal 2: the server runs m_flCountdownTime down; the client runs its copy
// down between updates so the hundredths move smoothly (2010 client.dylib).
//-----------------------------------------------------------------------------
void C_NeurotoxinCountdown::Spawn( void )
{
	BaseClass::Spawn();

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

void C_NeurotoxinCountdown::ClientThink( void )
{
	if ( m_bEnabled )
	{
		m_flCountdownTime -= gpGlobals->frametime;
	}
}

float C_NeurotoxinCountdown::GetCountdownTime( void )
{
	return m_flCountdownTime;
}
#endif

int C_NeurotoxinCountdown::GetMinutes( void )
{
	C_BasePlayer *player = UTIL_PlayerByIndex( 1 );
	if ( player )
		return player->GetBonusProgress() / 60;
	
	return 0.0f;
}

int C_NeurotoxinCountdown::GetSeconds( void )
{
	C_BasePlayer *player = UTIL_PlayerByIndex( 1 );
	if ( player )
		return player->GetBonusProgress() % 60;

	return 0.0f;
}

int C_NeurotoxinCountdown::GetMilliseconds( void )
{
	return static_cast<int>( gpGlobals->curtime * 100.0f ) % 100;;
}
