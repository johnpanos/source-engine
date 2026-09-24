#ifndef PORTAL2_LEADERBOARD_MANAGER_H
#define PORTAL2_LEADERBOARD_MANAGER_H

#include "portal2_leaderboard.h"
#include "tier1/KeyValues.h"

class CPortalLeaderboardManager
{
public:
	CPortalLeaderboard *GetLeaderboard( const char * ) { return NULL; }
	KeyValues *GetChallengeMapsFromChapter( int, bool ) { return NULL; }
	void CleanupLeaderboard() {}
	static const char *GetTempScoresMap() { return ""; }
	static void SetTempScoresMap( const char * ) {}
};

inline CPortalLeaderboardManager *PortalLeaderboardManager()
{
	static CPortalLeaderboardManager manager;
	return &manager;
}

#endif
