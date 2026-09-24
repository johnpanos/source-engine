//========= Portal 2 reconstruction ============================================//
//
// Purpose: Point entity that listens for coop player pings and aims a
//			func_tank at the pinged location
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "GameEventListener.h"
#include "func_tank.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CInfo_Player_Ping_Detector : public CPointEntity, public CGameEventListener
{
public:
	DECLARE_CLASS( CInfo_Player_Ping_Detector, CPointEntity );

	CInfo_Player_Ping_Detector();
	virtual ~CInfo_Player_Ping_Detector();

	virtual void Spawn( void );
	virtual void UpdateOnRemove( void );
	virtual void FireGameEvent( IGameEvent *event );

	// Inputs
	void InputToggle( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

protected:
	void Toggle( void );
	void Enable( void );
	void Disable( void );

private:
	string_t m_iszFuncTankName;

	bool m_bEnabled;

	bool m_bDetectedNewPing;
	bool m_bLookAtPlayerPings;
	int m_nTeamToLookAt;

	Vector m_vecPingLocation;

	COutputEvent m_OnPingDetected;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CInfo_Player_Ping_Detector )

	DEFINE_OUTPUT( m_OnPingDetected, "OnPingDetected" ),

	DEFINE_FIELD( m_vecPingLocation, FIELD_VECTOR ),

	DEFINE_KEYFIELD( m_bEnabled, FIELD_BOOLEAN, "Enabled" ),
	DEFINE_KEYFIELD( m_iszFuncTankName, FIELD_STRING, "FuncTankName" ),
	DEFINE_KEYFIELD( m_bLookAtPlayerPings, FIELD_BOOLEAN, "LookAtPlayerPings" ),
	DEFINE_KEYFIELD( m_nTeamToLookAt, FIELD_INTEGER, "TeamToLookAt" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( info_player_ping_detector, CInfo_Player_Ping_Detector );

CInfo_Player_Ping_Detector::CInfo_Player_Ping_Detector()
{
	m_vecPingLocation = vec3_origin;
	m_bDetectedNewPing = false;
}

CInfo_Player_Ping_Detector::~CInfo_Player_Ping_Detector()
{
}

void CInfo_Player_Ping_Detector::Spawn( void )
{
	BaseClass::Spawn();

	if ( m_bEnabled )
	{
		ListenForGameEvent( "portal_player_ping" );
	}
}

void CInfo_Player_Ping_Detector::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();
}

void CInfo_Player_Ping_Detector::FireGameEvent( IGameEvent *event )
{
	if ( !m_bEnabled )
		return;

	const char *name = event->GetName();
	if ( V_strcmp( name, "portal_player_ping" ) == 0 )
	{
		int nPlayerTeam = -1;
		int playerId = event->GetInt( "userid" );

		Vector vecPingPos(
		    event->GetFloat( "ping_x" ), event->GetFloat( "ping_y" ), event->GetFloat( "ping_z" ) );

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

		// Team 1 means "both teams"
		if ( m_nTeamToLookAt == 1 || nPlayerTeam == m_nTeamToLookAt )
		{
			m_vecPingLocation = vecPingPos;
			m_bDetectedNewPing = true;

			// Point every named func_tank at the ping
			CBaseEntity *pNewEntity = NULL;
			while ( ( pNewEntity = gEntList.FindEntityByName(
			              pNewEntity, STRING( m_iszFuncTankName ) ) ) != NULL )
			{
				CFuncTank *pTank = dynamic_cast<CFuncTank *>( pNewEntity );
				if ( pTank )
				{
					pTank->SetTargetPosition( m_vecPingLocation );
				}
			}

			m_OnPingDetected.FireOutput( NULL, NULL );
		}
	}
}

void CInfo_Player_Ping_Detector::InputToggle( inputdata_t &inputdata )
{
	Toggle();
}

void CInfo_Player_Ping_Detector::InputEnable( inputdata_t &inputdata )
{
	Enable();
}

void CInfo_Player_Ping_Detector::InputDisable( inputdata_t &inputdata )
{
	Disable();
}

void CInfo_Player_Ping_Detector::Toggle( void )
{
	if ( m_bEnabled )
	{
		Disable();
	}
	else
	{
		Enable();
	}
}

void CInfo_Player_Ping_Detector::Enable( void )
{
	m_bEnabled = true;
	ListenForGameEvent( "portal_player_ping" );
}

void CInfo_Player_Ping_Detector::Disable( void )
{
	m_bEnabled = false;
	StopListeningForAllEvents();
}
