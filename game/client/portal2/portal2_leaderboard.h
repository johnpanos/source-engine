#ifndef PORTAL2_LEADERBOARD_H
#define PORTAL2_LEADERBOARD_H

#include "steam/isteamuserstats.h"
#include "steam/steamclientpublic.h"

class PortalLeaderboardItem_t
{
public:
	char m_szName[128];
	int m_iScore;
	CSteamID m_steamIDUser;
	uint64 m_xuid;
};

enum LeaderboardType
{
	LEADERBOARD_PORTAL = 0,
	LEADERBOARD_TIME,
	NUM_LEADERBOARDS
};

enum StatType_t
{
	STAT_TYPE_PORTAL,
	STAT_TYPE_TIME
};

class CPortalLeaderboard
{
public:
	SteamLeaderboard_t GetLeaderboardHandle( LeaderboardType ) const { return 0; }
	int GetCurrentPlayerIndex( LeaderboardType ) const { return -1; }
	const PortalLeaderboardItem_t *GetCurrentBest( LeaderboardType, int = 0 ) const { return NULL; }
	const PortalLeaderboardItem_t *GetPlayerAtIndex( int, LeaderboardType ) const { return NULL; }
	int GetNextTargetIndex( LeaderboardType ) const { return -1; }
	int GetWorstPlayerIndex( LeaderboardType ) const { return -1; }
	const char *GetMapName() const { return ""; }
	bool IsSinglePlayer() const { return true; }
	bool IsInvalid() const { return true; }
	bool IsQuerying() const { return false; }
	bool WasIOError() const { return false; }
	bool IsGladosAllowedToSpeak() const { return false; }
	void SetGladosIsAllowedToSpeak( bool ) {}
	void SetGladosHasSpoken() {}
	void DoGladosSpokenReaction( const char *, int * = NULL ) {}
	int NumXUIDs() const { return 0; }
	void UpdateXUIDs() {}
};

#endif
