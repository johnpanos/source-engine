//========= Portal 2 reconstruction ============================================//
//
// Purpose: One challenge mode map's leaderboards (least portals and best
//          time): friend scores from Steam UserStats, the global score
//          histogram from the Steam Web API, and GLaDOS' reaction lines.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PORTAL2_LEADERBOARD_H
#define PORTAL2_LEADERBOARD_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "steam/steamclientpublic.h"
#include "steam/isteamuserstats.h"
#if !defined( NO_STEAM )
#include "steam/steam_api.h"
#include "steam/isteamhttp.h"
#endif

class CLeaderboardBucketizer;

// One downloaded leaderboard row
class PortalLeaderboardItem_t
{
public:
	CSteamID	m_steamIDUser;
	char		m_szName[32];
	int			m_iRank;
	int			m_iGlobalRank;
	uint64		m_xuid;
	int			m_iScore;
	bool		m_bLocalPlayer;
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
	// Queued work, processed one at a time
	enum LeaderboardRequestType_t
	{
		LEADERBOARD_REQUEST_NONE = 0,
		LEADERBOARD_REQUEST_WAIT = 1,
		LEADERBOARD_REQUEST_DOWNLOAD_FRIENDS = 2,
		LEADERBOARD_REQUEST_DOWNLOAD_GLOBAL_DATA = 3,
		LEADERBOARD_REQUEST_FIND = 4,
	};

	struct LeaderboardRequest_t
	{
		int				m_eRequest;
		LeaderboardType	m_eType;
		int				m_nParam;
	};

	explicit CPortalLeaderboard( const char *pszMapName );
	~CPortalLeaderboard();

	// Queues find/friends/global requests for both leaderboards.
	bool Init( void );

	// Starts the next queued request when none is running.
	void ProcessRequests( void );
	void CancelRequests( void );

	const char *GetMapName() const { return m_szMapName; }
	static const char *GetLeaderboardName( LeaderboardType type, const char *pszMapName );

	SteamLeaderboard_t GetLeaderboardHandle( LeaderboardType type ) const;
	void SetLeaderboardHandle( LeaderboardType type, SteamLeaderboard_t hLeaderboard );

	int GetEntryCount( LeaderboardType type ) const { return m_Entries[ type ].Count(); }
	const PortalLeaderboardItem_t *GetPlayerAtIndex( int nIndex, LeaderboardType type ) const;
	int GetCurrentPlayerIndex( LeaderboardType type ) const;
	int GetNextTargetIndex( LeaderboardType type ) const;
	int GetWorstPlayerIndex( LeaderboardType type ) const { return m_Entries[ type ].Count() - 1; }
	const PortalLeaderboardItem_t *GetNextTarget( LeaderboardType type ) const;
	const PortalLeaderboardItem_t *GetLocalPlayerEntry( LeaderboardType type ) const;
	const PortalLeaderboardItem_t *GetWorstPlayer( LeaderboardType type ) const;

	// nWhich 0: first place; 1: the entry ahead of the local player;
	// otherwise the local player's own entry.
	const PortalLeaderboardItem_t *GetCurrentBest( LeaderboardType type, int nWhich = 0 ) const;

	CLeaderboardBucketizer *GetFriendsGraphData( LeaderboardType type ) const { return m_pFriendsGraph[ type ]; }
	CLeaderboardBucketizer *GetGlobalGraphData( LeaderboardType type ) const { return m_pGlobalGraph[ type ]; }

	// Comparisons of the most recent challenge score with the downloaded rows
	bool IsNewBest( LeaderboardType type ) const;
	bool DidNotPassNextTarget( LeaderboardType type ) const;
	bool IsCloseToNextTarget( LeaderboardType type ) const;
	bool IsPersonalBest( LeaderboardType type ) const;
	bool IsFirstScore( void ) const;

	bool IsValidType( LeaderboardType type ) const { return (unsigned int)type < NUM_LEADERBOARDS; }
	bool IsRequestPending( void ) const { return m_CurrentRequest.m_eRequest != LEADERBOARD_REQUEST_NONE; }
	bool IsQuerying( void ) const;
	bool IsInvalid( void ) const { return m_bInvalid; }
	bool WasIOError( void ) const { return m_bIOError; }

	bool IsGladosAllowedToSpeak( void ) const { return m_bGladosAllowedToSpeak && !m_bGladosHasSpoken; }
	void SetGladosIsAllowedToSpeak( bool bAllowed ) { m_bGladosAllowedToSpeak = bAllowed; }
	void SetGladosHasSpoken( void ) { m_bGladosHasSpoken = true; }

	// Picks GLaDOS' line for the most recent challenge result and reports
	// which leaderboard improved most (LEADERBOARD_PORTAL or LEADERBOARD_TIME).
	void DoGladosSpokenReaction( char *pszLine, int *pnImprovedLeaderboardIndex = NULL );

	// Xbox 360 local profiles; none on this platform
	int NumXUIDs( void ) const { return 0; }
	bool UpdateXUIDs( void ) { return true; }

private:
	void ResetCurrentRequest( void );
	void QueueRequest( int eRequest, LeaderboardType type );
	void StartNextRequest( void );
	bool StartFindLeaderboard( void );
	bool StartDownloadFriends( void );
	bool StartDownloadGlobalData( void );
	void SetEntry( const LeaderboardEntry_t &entry, int nIndex );
	void BuildFriendsGraphData( void );

#if !defined( NO_STEAM )
	void OnFindLeaderboard( LeaderboardFindResult_t *pResult, bool bIOFailure );
	void OnScoresDownloaded( LeaderboardScoresDownloaded_t *pResult, bool bIOFailure );
	void OnGlobalDataDownloaded( HTTPRequestCompleted_t *pResult, bool bIOFailure );

	CCallResult< CPortalLeaderboard, LeaderboardFindResult_t > m_FindResult;
	CCallResult< CPortalLeaderboard, LeaderboardScoresDownloaded_t > m_DownloadResult;
	CCallResult< CPortalLeaderboard, HTTPRequestCompleted_t > m_HTTPResult;
#endif

	char m_szMapName[128];

	LeaderboardRequest_t m_CurrentRequest;
	CUtlVector< LeaderboardRequest_t > m_Requests;

	SteamLeaderboard_t m_hLeaderboards[ NUM_LEADERBOARDS ];
	CUtlVector< PortalLeaderboardItem_t > m_Entries[ NUM_LEADERBOARDS ];
	int m_nCurrentPlayerIndex[ NUM_LEADERBOARDS ];
	CLeaderboardBucketizer *m_pFriendsGraph[ NUM_LEADERBOARDS ];
	CLeaderboardBucketizer *m_pGlobalGraph[ NUM_LEADERBOARDS ];

	bool m_bInvalid;
	bool m_bIOError;
	bool m_bGladosAllowedToSpeak;
	bool m_bGladosHasSpoken;
};

#endif // PORTAL2_LEADERBOARD_H
