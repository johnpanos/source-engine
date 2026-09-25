//========= Portal 2 reconstruction ============================================//
//
// Purpose: portal_stats_controller, the per-level entity that ends a level.
//
// The retail transition scripts (scripts/vscripts/transitions/
// sp_transition_list.nut and the co-op list) end a map with
// EntFire( "portal_stats_controller", "OnLevelEnd" ). The controller then runs
// RealTransitionFromMap() on the map's transition script, which issues the
// changelevel. No map places this entity; a game system creates one per level.
//
// Written from the retail Linux server.so: the CPortalStatsController RTTI and
// datadesc (inputs OnLevelStart and OnLevelEnd), its Spawn, Think and input
// handlers, and the game system that creates and removes it. No original
// source or pseudocode exists. Challenge-mode leaderboard requests are sent to
// the client in retail as KeyValues commands ("read_leaderboard",
// "write_leaderboard") and "+leaderboard"/"leaderboard_open" client commands;
// this engine has no server-to-client KeyValues command and this build defines
// NO_STEAM, so only the ScoreboardTempUpdate and ChallengeModeCheatSession
// user messages are sent. Not original Valve source; the repository's
// provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "eventqueue.h"
#include "portal2/portal/portal_player.h"
#include "portal_mp_gamerules.h"
#include "matchmaking/imatchframework.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CPortalStatsController;
static CPortalStatsController *g_pPortalStatsController = NULL;

// Retail tests the host player: the local player in single player, else the
// listen-server host (none on a dedicated server).
static CPortal_Player *GetStatsHostPlayer()
{
	CBasePlayer *pPlayer;
	if ( gpGlobals->maxClients < 2 )
		pPlayer = UTIL_GetLocalPlayer();
	else
		pPlayer = engine->IsDedicatedServer() ? NULL : UTIL_GetListenServerHost();
	return ToPortalPlayer( pPlayer );
}

static bool IsHostInChallengeMode( CPortal_Player *pHost )
{
	return pHost && pHost->GetBonusChallenge() > 0;
}

class CPortalStatsController : public CLogicalEntity
{
public:
	DECLARE_CLASS( CPortalStatsController, CLogicalEntity );
	DECLARE_DATADESC();

	CPortalStatsController();
	~CPortalStatsController();

	void Spawn() OVERRIDE;
	void Think() OVERRIDE;

	// A game system recreates the controller after every level load, so a
	// saved copy would duplicate it (and its transition) on restore.
	int ObjectCaps() OVERRIDE { return BaseClass::ObjectCaps() | FCAP_DONT_SAVE; }

	void InputOnLevelStart( inputdata_t &inputdata );
	void InputOnLevelEnd( inputdata_t &inputdata );

private:
	void SendChallengeScores();

	float m_flTransitionTime;		// curtime at which to transition; -1 when idle
	float m_flLeaderboardTime;		// retail: delay before "+leaderboard 4"; -1 when idle
};

LINK_ENTITY_TO_CLASS( portal_stats_controller, CPortalStatsController );

BEGIN_DATADESC( CPortalStatsController )
	DEFINE_INPUTFUNC( FIELD_FLOAT, "OnLevelStart", InputOnLevelStart ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "OnLevelEnd", InputOnLevelEnd ),
END_DATADESC()

CPortalStatsController::CPortalStatsController()
	: m_flTransitionTime( -1.0f ), m_flLeaderboardTime( -1.0f )
{
}

CPortalStatsController::~CPortalStatsController()
{
	if ( g_pPortalStatsController == this )
		g_pPortalStatsController = NULL;
}

void CPortalStatsController::Spawn()
{
	BaseClass::Spawn();
	m_flTransitionTime = -1.0f;

	// Retail starts these two maps itself three seconds after spawn.
	const char *pszMap = STRING( gpGlobals->mapname );
	if ( V_stricmp( pszMap, "sp_a2_intro" ) != 0 && V_stricmp( pszMap, "sp_a4_speed_tb_catch" ) != 0 )
		return;

	variant_t value;
	value.SetFloat( 3.0f );
	g_EventQueue.AddEvent( this, "OnLevelStart", value, 3.0f, this, this );
}

// The map's transition script entity: @transition_script, else the finale's
// script_check_finish_game, else any logic_script running transition_script.
static CBaseEntity *FindTransitionScript()
{
	CBaseEntity *pScript = gEntList.FindEntityByName( NULL, "@transition_script" );
	if ( !pScript )
		pScript = gEntList.FindEntityByName( NULL, "script_check_finish_game" );
	if ( pScript )
		return pScript;

	for ( CBaseEntity *pEnt = gEntList.FindEntityByClassname( NULL, "logic_script" ); pEnt;
		  pEnt = gEntList.FindEntityByClassname( pEnt, "logic_script" ) )
	{
		if ( V_stristr( STRING( pEnt->m_iszVScripts ), "transition_script" ) )
			return pEnt;
	}
	return NULL;
}

void CPortalStatsController::Think()
{
	CPortal_Player *pHost = GetStatsHostPlayer();
	if ( !pHost )
		return;

	if ( !IsHostInChallengeMode( pHost ) && m_flTransitionTime != -1.0f && m_flTransitionTime < gpGlobals->curtime )
	{
		CBaseEntity *pScript = FindTransitionScript();

		// Without a transition script retail keeps thinking and retries.
		if ( pScript )
		{
			pScript->RunScript( "RealTransitionFromMap()", "CBaseEntity::RunScript" );
			m_flTransitionTime = -1.0f;
			UTIL_Remove( this );
			return;
		}
	}

	if ( m_flLeaderboardTime >= 0.0f && m_flLeaderboardTime < gpGlobals->curtime )
	{
		// Retail opens the leaderboard here for players that finished a
		// challenge; see the file header.
		m_flLeaderboardTime = -1.0f;
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CPortalStatsController::InputOnLevelStart( inputdata_t &inputdata )
{
	CPortal_Player *pHost = GetStatsHostPlayer();
	if ( IsHostInChallengeMode( pHost ) )
	{
		// Retail looks up only @transition_script here.
		CBaseEntity *pScript = gEntList.FindEntityByName( NULL, "@transition_script" );
		if ( pScript )
			pScript->RunScript( "TransitionReady ()", " TransitionReady" );
	}

	m_flLeaderboardTime = 3.5f;
	SetNextThink( gpGlobals->curtime + 0.1f );

	// Retail asks the game rules whether saving is allowed, which is false
	// exactly when the host is in challenge mode.
	if ( IsHostInChallengeMode( pHost ) )
	{
		static ConVarRef map_wants_save_disable( "map_wants_save_disable" );
		map_wants_save_disable.SetValue( 1 );
	}

	m_flTransitionTime = -1.0f;
}

void CPortalStatsController::SendChallengeScores()
{
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CPortal_Player *pPlayer = ToPortalPlayer( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer || !pPlayer->IsConnected() )
			continue;

		// Challenge time is in hundredths of a second. Co-op counts the
		// portals both players placed.
		int nTime = static_cast<int>( 100.0f * pPlayer->NumSecondsTaken() );
		int nPortals = PortalMPGameRules() ? PortalMPGameRules()->GetNumPortalsPlaced() : pPlayer->NumPortalsPlaced();

		CSingleUserRecipientFilter filter( pPlayer );
		filter.MakeReliable();
		UserMessageBegin( filter, "ScoreboardTempUpdate" );
			WRITE_LONG( nPortals );
			WRITE_LONG( nTime );
		MessageEnd();
	}
}

void CPortalStatsController::InputOnLevelEnd( inputdata_t &inputdata )
{
	float flDelay = inputdata.value.FieldType() == FIELD_FLOAT ? inputdata.value.Float() : 0.0f;

	CPortal_Player *pHost = GetStatsHostPlayer();
	if ( !IsHostInChallengeMode( pHost ) )
	{
		flDelay = 0.0f;
	}
	else
	{
		IMatchSession *pSession = g_pMatchFramework ? g_pMatchFramework->GetMatchSession() : NULL;
		KeyValues *pSettings = pSession ? pSession->GetSessionSettings() : NULL;
		if ( pSettings && pSettings->GetInt( "game/sv_cheats", 0 ) )
		{
			CReliableBroadcastRecipientFilter filter;
			UserMessageBegin( filter, "ChallengeModeCheatSession" );
			MessageEnd();
		}
		else
		{
			SendChallengeScores();
		}
	}

	m_flTransitionTime = gpGlobals->curtime + flDelay;
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Creates the controller after each level's entities spawn and removes it
// before they shut down, resetting every player's per-level statistics.
//-----------------------------------------------------------------------------
class CPortalStatsControllerSystem : public CAutoGameSystem
{
public:
	CPortalStatsControllerSystem() : CAutoGameSystem( "CPortalStatsControllerSystem" ) {}

	void LevelInitPostEntity() OVERRIDE
	{
		for ( int i = 1; i <= gpGlobals->maxClients; ++i )
		{
			CPortal_Player *pPlayer = ToPortalPlayer( UTIL_PlayerByIndex( i ) );
			if ( pPlayer && pPlayer->IsConnected() )
				pPlayer->ResetThisLevelStats();
		}

		if ( g_pPortalStatsController )
			return;

		CBaseEntity *pEnt = CreateEntityByName( "portal_stats_controller" );
		g_pPortalStatsController = dynamic_cast<CPortalStatsController *>( pEnt );
		if ( g_pPortalStatsController )
			DispatchSpawn( g_pPortalStatsController );
	}

	void LevelShutdownPreEntity() OVERRIDE
	{
		if ( g_pPortalStatsController )
		{
			UTIL_Remove( g_pPortalStatsController );
			g_pPortalStatsController = NULL;
		}
	}
};

static CPortalStatsControllerSystem g_PortalStatsControllerSystem;
