//========= Portal 2 reconstruction ============================================//
//
// Purpose: Trigger volume that fires outputs when a coop player pings inside it
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "triggers.h"
#include "GameEventListener.h"
#include "portal_mp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTriggerPingDetector : public CBaseTrigger, public CGameEventListener
{
public:
	DECLARE_CLASS( CTriggerPingDetector, CBaseTrigger );
	DECLARE_DATADESC();

	virtual void Spawn( void )
	{
		ClearSpawnFlags();

		BaseClass::Spawn();
		InitTrigger();

		ListenForGameEvent( "portal_player_ping" );
	}

	virtual void FireGameEvent( IGameEvent *event )
	{
		const char *name = event->GetName();
		if ( V_strcmp( name, "portal_player_ping" ) == 0 )
		{
			int nPlayerTeam = -1;
			int playerId = event->GetInt( "userid" );

			Vector vecPingPos( event->GetFloat( "ping_x" ), event->GetFloat( "ping_y" ), event->GetFloat( "ping_z" ) );

			// Find the team of the player who pinged
			for ( int i = 1; i <= MAX_PLAYERS; ++i )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
				if ( pPlayer && pPlayer->GetUserID() == playerId )
				{
					nPlayerTeam = pPlayer->GetTeamNumber();
					break;
				}
			}

			// See if the ping location is inside this trigger
			Ray_t shotRay;
			shotRay.Init( vecPingPos, vecPingPos );
			trace_t tr;
			enginetrace->ClipRayToEntity( shotRay, MASK_ALL, this, &tr );

			if ( tr.DidHitNonWorldEntity() && tr.m_pEnt == this )
			{
				if ( nPlayerTeam == TEAM_RED )
				{
					m_OnOrangePlayerPinged.FireOutput( NULL, NULL );
				}
				else
				{
					m_OnBluePlayerPinged.FireOutput( NULL, NULL );
				}
			}
		}
	}

private:
	COutputEvent	m_OnOrangePlayerPinged;
	COutputEvent	m_OnBluePlayerPinged;
};

BEGIN_DATADESC( CTriggerPingDetector )
	DEFINE_OUTPUT( m_OnOrangePlayerPinged, "OnOrangePlayerPinged" ),
	DEFINE_OUTPUT( m_OnBluePlayerPinged, "OnBluePlayerPinged" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_ping_detector, CTriggerPingDetector );
