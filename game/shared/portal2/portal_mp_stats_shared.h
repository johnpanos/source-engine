//========= Portal 2 reconstruction ============================================//
//
// Purpose: Build-time interface for optional multiplayer statistics.
//
//=============================================================================//

#ifndef PORTAL_MP_STATS_SHARED_H
#define PORTAL_MP_STATS_SHARED_H

class CBaseEntity;

class CPortalMPStats
{
public:
	static void InitPortalMPStats() {}

	void ClearPerMapStats() {}
	void IncrementMapsCompleted() {}
	void IncrementPlayerDeathsMap( CBaseEntity *pPlayer ) { (void)pPlayer; }
	void IncrementPlayerPortals( CBaseEntity *pPlayer ) { (void)pPlayer; }
	void IncrementPlayerPortalsTraveled( CBaseEntity *pPlayer ) { (void)pPlayer; }
	void IncrementPlayerSteps( CBaseEntity *pPlayer ) { (void)pPlayer; }
	void IncrementPlayerTauntsUsedMap( CBaseEntity *pPlayer, int nTaunt )
	{
		(void)pPlayer;
		(void)nTaunt;
	}
	void RefreshStats( CBaseEntity *pPlayer, CBaseEntity *pLocalPlayer )
	{
		(void)pPlayer;
		(void)pLocalPlayer;
	}
	void SavePerMapStats( CBaseEntity *pPlayer, const char *pMapName )
	{
		(void)pPlayer;
		(void)pMapName;
	}
	void SaveStats( CBaseEntity *pPlayer ) { (void)pPlayer; }
	void SetStats( int nSteps, int nPlaced, int nTraveled, int nTaunts )
	{
		(void)nSteps;
		(void)nPlaced;
		(void)nTraveled;
		(void)nTaunts;
	}
	void SetTimeToCompleteMap( float flSeconds ) { (void)flSeconds; }
	void SubmitOGSEndOfMapStatsForPlayer( CBaseEntity *pPlayer, const char *pMapName )
	{
		(void)pPlayer;
		(void)pMapName;
	}
	void TeamTauntSuccess( const char *pTauntName ) { (void)pTauntName; }
};

CPortalMPStats *GetPortalMPStats();

#endif // PORTAL_MP_STATS_SHARED_H
