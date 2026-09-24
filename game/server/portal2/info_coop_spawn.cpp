//========= Portal 2 reconstruction ============================================//
//
// Purpose: Player spawn points for cooperative mode
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "baseanimating.h"
#include "portal_mp_gamerules.h"
#include "utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_iPortalGunPlayerTeam;

class CInfo_Coop_Spawn : public CBaseAnimating
{
public:
	DECLARE_CLASS( CInfo_Coop_Spawn, CBaseAnimating );
	DECLARE_DATADESC();

	virtual void Activate( void );
	void InputEnable( inputdata_t &inputdata ) { m_bEnabled = true; }
	void InputDisable( inputdata_t &inputdata ) { m_bEnabled = false; }
	void InputSetAsActiveSpawn( inputdata_t &inputdata );
	virtual void UpdateOnRemove( void );
	// Reconstruction note: declared in the DWARF class but never emitted or
	// called in either build, so its definition is unknown.
	bool StringCompare( const char *pszA, const char *pszB );

	virtual int DrawDebugTextOverlays( void );

	bool	m_bEnabled;
	int		m_iStartingTeam;
	bool	m_bForceGunOnSpawn;
};

BEGIN_DATADESC( CInfo_Coop_Spawn )

	DEFINE_KEYFIELD( m_bEnabled, FIELD_BOOLEAN, "Enabled" ),
	DEFINE_KEYFIELD( m_iStartingTeam, FIELD_INTEGER, "StartingTeam" ),
	DEFINE_KEYFIELD( m_bForceGunOnSpawn, FIELD_BOOLEAN, "ForceGunOnSpawn" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetAsActiveSpawn", InputSetAsActiveSpawn ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( info_coop_spawn, CInfo_Coop_Spawn );

static CUtlVector<CInfo_Coop_Spawn *> s_SpawnLocations;

void CInfo_Coop_Spawn::Activate( void )
{
	s_SpawnLocations.AddToTail( this );

	// The spawn point's team is what GetCoopSpawnLocation() matches against
	ChangeTeam( m_iStartingTeam );

	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Enable this spawn and disable every other spawn for the same team
//-----------------------------------------------------------------------------
void CInfo_Coop_Spawn::InputSetAsActiveSpawn( inputdata_t &inputdata )
{
	int iKnownLocations = s_SpawnLocations.Count();
	CInfo_Coop_Spawn **pKnownLocations = s_SpawnLocations.Base();

	for ( int i = 0; i != iKnownLocations; ++i )
	{
		if ( pKnownLocations[i] == NULL )
		{
			Warning( "info_coop_spawn was removed but is still in known location list." );
			continue;
		}

		if ( pKnownLocations[i] == this || pKnownLocations[i]->m_iStartingTeam != m_iStartingTeam )
			continue;

		pKnownLocations[i]->m_bEnabled = false;

		if ( m_debugOverlays )
		{
			NDebugOverlay::Cross3D( pKnownLocations[i]->GetAbsOrigin(), 4.0f, 255, 0, 0, true, 2.0f );
		}
	}

	if ( m_debugOverlays )
	{
		NDebugOverlay::Cross3D( GetAbsOrigin(), 4.0f, 0, 255, 0, true, 2.0f );
	}

	m_bEnabled = true;
}

void CInfo_Coop_Spawn::UpdateOnRemove( void )
{
	s_SpawnLocations.FindAndFastRemove( this );
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Pick a random enabled spawn point for the given team (spawn points
//			on team 0 or 1 accept either team)
//-----------------------------------------------------------------------------
CBaseEntity *GetCoopSpawnLocation( int iTeam )
{
	int iKnownLocations = s_SpawnLocations.Count();
	CInfo_Coop_Spawn **pKnownLocations = s_SpawnLocations.Base();
	CInfo_Coop_Spawn **pValidLocations = (CInfo_Coop_Spawn **)stackalloc( sizeof( CInfo_Coop_Spawn * ) * iKnownLocations );
	int iValidLocations = 0;
	for ( int i = 0; i != iKnownLocations; ++i )
	{
		if ( !pKnownLocations[i]->m_bEnabled )
			continue;

		if ( pKnownLocations[i]->GetTeamNumber() == iTeam || pKnownLocations[i]->GetTeamNumber() <= TEAM_SPECTATOR )
		{
			pValidLocations[iValidLocations] = pKnownLocations[i];
			++iValidLocations;
		}
	}

	if ( iValidLocations == 0 )
		return NULL;

	CInfo_Coop_Spawn *pValidSpawn = pValidLocations[RandomInt( 0, iValidLocations - 1 )];
	if ( pValidSpawn->m_bForceGunOnSpawn && PortalMPGameRules()->Is2GunsCoOp() )
	{
		// Blue gets the portal gun, orange gets the paint gun
		g_iPortalGunPlayerTeam = TEAM_BLUE;
	}

	return pValidSpawn;
}

int CInfo_Coop_Spawn::DrawDebugTextOverlays( void )
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	char tempstr[255];
	V_snprintf( tempstr, sizeof( tempstr ), "%s", m_bEnabled ? "Enabled" : "Disabled" );
	EntityText( text_offset, tempstr, 0 );
	text_offset++;

	return text_offset;
}
