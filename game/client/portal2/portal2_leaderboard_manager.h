//========= Portal 2 reconstruction ============================================//
//
// Purpose: Owns the per-map challenge mode leaderboards and the challenge map
//          list (challenge_maplist.txt).
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PORTAL2_LEADERBOARD_MANAGER_H
#define PORTAL2_LEADERBOARD_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "matchmaking/imatchevents.h"
#include "portal2_leaderboard.h"
#include "tier1/utlsymbol.h"
#include "tier1/KeyValues.h"

class CPortalLeaderboardManager : public CAutoGameSystemPerFrame, public IMatchEventsSink
{
public:
	CPortalLeaderboardManager();
	virtual ~CPortalLeaderboardManager();

	// CAutoGameSystemPerFrame
	virtual char const *Name() { return "PortalLeaderboardManager"; }
	virtual bool Init();
	virtual void Shutdown();
	virtual void LevelShutdownPreEntity();
	virtual void Update( float frametime );

	// IMatchEventsSink
	virtual void OnEvent( KeyValues *pEvent );

	// The leaderboards for a map, created on first use. NULL while another
	// map's leaderboards are still querying or when Steam is unavailable.
	CPortalLeaderboard *GetLeaderboard( const char *pszMapName );

	// "chapter_<n>" from the "sp" or "coop" section of challenge_maplist.txt
	KeyValues *GetChallengeMapsFromChapter( int nChapter, bool bCoop );

	void CleanupLeaderboard( void );

	// The map whose scores the temporary score updates belong to
	static const char *GetTempScoresMap();
	static void SetTempScoresMap( const char *pszMapName );

private:
	bool LoadChallengeMapList( void );

	CUtlVector< CPortalLeaderboard * > m_Leaderboards;
	CUtlSymbolTable m_LeaderboardNames;
	CPortalLeaderboard *m_pCurrentLeaderboard;
	KeyValues *m_pChallengeMapList;
};

CPortalLeaderboardManager *PortalLeaderboardManager();

#endif // PORTAL2_LEADERBOARD_MANAGER_H
