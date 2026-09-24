//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ====
//
// Purpose:		logic_eventlistener: fires OnEventFired for a named game event.
//
//
// Portal 2 port: from the CS:GO source drop; the repository's provenance and distribution warning applies.
// FetchEventData (script event tables) and logic_eventlistener_itemequip are
// CS:GO additions absent from retail Portal 2 and are not ported.
//
//=============================================================================

#include "cbase.h"
#include "logic_eventlistener.h"
#include "GameEventListener.h"
#include "igameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( logic_eventlistener, CLogicEventListener);

BEGIN_DATADESC( CLogicEventListener )

// Base
DEFINE_KEYFIELD( m_iszEventName, FIELD_STRING, "EventName" ),
DEFINE_KEYFIELD( m_bIsEnabled, FIELD_BOOLEAN, "IsEnabled" ),
DEFINE_KEYFIELD( m_nTeam, FIELD_INTEGER, "TeamNum" ),

// Inputs
DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

// Outputs
DEFINE_OUTPUT( m_OnEventFired, "OnEventFired" ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void CLogicEventListener::Spawn( void )
{
	BaseClass::Spawn();

	ListenForGameEvent( STRING(m_iszEventName) );
}

//-----------------------------------------------------------------------------
// Purpose: FireGameEvent
//-----------------------------------------------------------------------------
void CLogicEventListener::FireGameEvent( IGameEvent *event )
{
	if ( !m_bIsEnabled )
		return;

	const char *name = event->GetName();
	if ( Q_strcmp( name, STRING(m_iszEventName) ) == 0 )
	{
		if ( m_nTeam > 0 )
		{
			int nPlayerTeam = TEAM_INVALID;
			int	playerId = event->GetInt( "userid", 0 );
			for ( int i = 0; i <= MAX_PLAYERS; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
				if ( pPlayer && pPlayer->GetUserID() == playerId )
				{
					nPlayerTeam = pPlayer->GetTeamNumber();
					break;
				}
			}

			if ( nPlayerTeam != m_nTeam )
				return;
		}

		m_OnEventFired.FireOutput( NULL, NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CLogicEventListener::InputEnable( inputdata_t &inputdata )
{
	m_bIsEnabled = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CLogicEventListener::InputDisable( inputdata_t &inputdata )
{
	m_bIsEnabled = false;
}
