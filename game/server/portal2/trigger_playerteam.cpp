//========= Portal 2 reconstruction ============================================//
//
// Purpose: Trigger that fires separate outputs for the orange and blue players
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "triggers.h"
#include "portal_mp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTriggerPlayerTeam : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerPlayerTeam, CBaseTrigger );
	DECLARE_DATADESC();

	virtual void Spawn( void )
	{
		// Only players can use this trigger
		ClearSpawnFlags();
		AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS );

		BaseClass::Spawn();
		InitTrigger();
	}

	virtual void StartTouch( CBaseEntity *pOther )
	{
		if ( pOther == NULL || !pOther->IsPlayer() )
			return;

		// Filter by team if one was specified
		if ( m_nTargetTeam > 0 && pOther->GetTeamNumber() != m_nTargetTeam )
			return;

		BaseClass::StartTouch( pOther );

		if ( pOther->GetTeamNumber() == TEAM_RED )
		{
			m_OnStartTouchOrangePlayer.FireOutput( pOther, this );
		}
		else if ( pOther->GetTeamNumber() == TEAM_BLUE )
		{
			m_OnStartTouchBluePlayer.FireOutput( pOther, this );
		}

		if ( m_bTriggerOnce )
		{
			UTIL_Remove( this );
		}
	}

	virtual void EndTouch( CBaseEntity *pOther )
	{
		if ( pOther == NULL || !pOther->IsPlayer() )
			return;

		// Filter by team if one was specified
		if ( m_nTargetTeam > 0 && pOther->GetTeamNumber() != m_nTargetTeam )
			return;

		BaseClass::EndTouch( pOther );

		if ( pOther->GetTeamNumber() == TEAM_RED )
		{
			m_OnEndTouchOrangePlayer.FireOutput( pOther, this );
		}
		else if ( pOther->GetTeamNumber() == TEAM_BLUE )
		{
			m_OnEndTouchBluePlayer.FireOutput( pOther, this );
		}
	}

private:
	int		m_nTargetTeam;
	bool	m_bTriggerOnce;

	COutputEvent	m_OnStartTouchOrangePlayer;
	COutputEvent	m_OnEndTouchOrangePlayer;
	COutputEvent	m_OnStartTouchBluePlayer;
	COutputEvent	m_OnEndTouchBluePlayer;
};

BEGIN_DATADESC( CTriggerPlayerTeam )

	DEFINE_KEYFIELD( m_bTriggerOnce, FIELD_BOOLEAN, "trigger_once" ),
	DEFINE_KEYFIELD( m_nTargetTeam, FIELD_INTEGER, "target_team" ),

	DEFINE_OUTPUT( m_OnStartTouchOrangePlayer, "OnStartTouchOrangePlayer" ),
	DEFINE_OUTPUT( m_OnEndTouchOrangePlayer, "OnEndTouchOrangePlayer" ),
	DEFINE_OUTPUT( m_OnStartTouchBluePlayer, "OnStartTouchBluePlayer" ),
	DEFINE_OUTPUT( m_OnEndTouchBluePlayer, "OnEndTouchBluePlayer" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_playerteam, CTriggerPlayerTeam );
