//========= Portal 2 reconstruction ============================================//
//
// Purpose: One challenge mode map's leaderboards; see portal2_leaderboard.h.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). The Steam requests are compiled only
// without NO_STEAM; this build defines it, and CPortalLeaderboardManager then
// hands out no leaderboards. Not original Valve source; the repository's
// provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_leaderboard.h"
#include "portal2_leaderboard_bucketizer.h"
#include "tier1/KeyValues.h"
#include "tier1/utlbuffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Most recent challenge mode scores, set by the "ScoreboardTempUpdate" user
// message in BaseModUI (vportalleaderboard.cpp).
extern int g_nPortalScoreTempMostRecent;
extern int g_nTimeScoreTempMostRecent;

// Defined with BaseModUI::Leaderboard (vleaderboard.cpp); copies of each row.
extern ConVar leaderboard_duplicate_entries;
ConVar cl_steam_universe( "cl_steam_universe", "0", 0, "0:public, 1:beta, 2:dev" );

// Rows kept per leaderboard download
#define LEADERBOARD_MAX_ENTRIES		100

// A new time within this many hundredths of the next friend is "close"
#define LEADERBOARD_CLOSE_TIME		501
// A new portal count within this many portals of the next friend is "close"
#define LEADERBOARD_CLOSE_PORTALS	3

// Scores above these are left out of the friend graph's range
#define LEADERBOARD_GRAPH_MAX_PORTALS	40
#define LEADERBOARD_GRAPH_MAX_TIME		36000

// No local score to compare with
#define LEADERBOARD_NO_SCORE		( INT_MIN + 1 )

//-----------------------------------------------------------------------------
// GLaDOS' reactions to a challenge result, by category
//-----------------------------------------------------------------------------
enum GladosReaction_t
{
	GLADOS_REACTION_NO_IMPROVEMENT = 0,		// did not beat the player's own score
	GLADOS_REACTION_NO_FRIENDS,				// no comparable friend score
	GLADOS_REACTION_FAR_BEHIND,				// improved, still well behind the next friend
	GLADOS_REACTION_CLOSE_BEHIND,			// improved, close to the next friend
	GLADOS_REACTION_PASSED_FRIEND,			// passed the next friend
	GLADOS_REACTION_BEST_PORTALS,			// fewest portals among friends
	GLADOS_REACTION_BEST_TIME,				// best time among friends
	GLADOS_REACTION_COUNT
};

static CUtlVector< const char * > s_GladosLines[ GLADOS_REACTION_COUNT ];

static void InitGladosLines( void )
{
	if ( s_GladosLines[ GLADOS_REACTION_NO_IMPROVEMENT ].Count() == 0 )
	{
		s_GladosLines[ GLADOS_REACTION_NO_IMPROVEMENT ].AddToTail( "vo/glados/dlc1_leaderboard21.wav" );
		s_GladosLines[ GLADOS_REACTION_NO_IMPROVEMENT ].AddToTail( "vo/glados/dlc1_leaderboard09.wav" );
	}
	if ( s_GladosLines[ GLADOS_REACTION_FAR_BEHIND ].Count() == 0 )
	{
		s_GladosLines[ GLADOS_REACTION_FAR_BEHIND ].AddToTail( "vo/glados/dlc1_leaderboard22.wav" );
		s_GladosLines[ GLADOS_REACTION_FAR_BEHIND ].AddToTail( "vo/glados/dlc1_leaderboard08.wav" );
	}
	if ( s_GladosLines[ GLADOS_REACTION_NO_FRIENDS ].Count() == 0 )
	{
		s_GladosLines[ GLADOS_REACTION_NO_FRIENDS ].AddToTail( "vo/glados/dlc1_leaderboard23.wav" );
		s_GladosLines[ GLADOS_REACTION_NO_FRIENDS ].AddToTail( "vo/glados/dlc1_leaderboard06.wav" );
		s_GladosLines[ GLADOS_REACTION_NO_FRIENDS ].AddToTail( "vo/glados/dlc1_leaderboard04.wav" );
		s_GladosLines[ GLADOS_REACTION_NO_FRIENDS ].AddToTail( "vo/glados/dlc1_leaderboard03.wav" );
		s_GladosLines[ GLADOS_REACTION_NO_FRIENDS ].AddToTail( "vo/glados/dlc1_leaderboard02.wav" );
		s_GladosLines[ GLADOS_REACTION_NO_FRIENDS ].AddToTail( "vo/glados/dlc1_leaderboard01.wav" );
	}
	if ( s_GladosLines[ GLADOS_REACTION_PASSED_FRIEND ].Count() == 0 )
	{
		s_GladosLines[ GLADOS_REACTION_PASSED_FRIEND ].AddToTail( "vo/glados/dlc1_leaderboard10.wav" );
		s_GladosLines[ GLADOS_REACTION_PASSED_FRIEND ].AddToTail( "vo/glados/dlc1_leaderboard11.wav" );
	}
	if ( s_GladosLines[ GLADOS_REACTION_CLOSE_BEHIND ].Count() == 0 )
	{
		s_GladosLines[ GLADOS_REACTION_CLOSE_BEHIND ].AddToTail( "vo/glados/dlc1_leaderboard15.wav" );
		s_GladosLines[ GLADOS_REACTION_CLOSE_BEHIND ].AddToTail( "vo/glados/dlc1_leaderboard14.wav" );
	}
	if ( s_GladosLines[ GLADOS_REACTION_BEST_PORTALS ].Count() == 0 )
	{
		s_GladosLines[ GLADOS_REACTION_BEST_PORTALS ].AddToTail( "vo/glados/dlc1_leaderboard19.wav" );
		s_GladosLines[ GLADOS_REACTION_BEST_PORTALS ].AddToTail( "vo/glados/dlc1_leaderboard18.wav" );
	}
	if ( s_GladosLines[ GLADOS_REACTION_BEST_TIME ].Count() == 0 )
	{
		s_GladosLines[ GLADOS_REACTION_BEST_TIME ].AddToTail( "vo/glados/dlc1_leaderboard19.wav" );
		s_GladosLines[ GLADOS_REACTION_BEST_TIME ].AddToTail( "vo/glados/dlc1_leaderboard17.wav" );
	}
}

//-----------------------------------------------------------------------------
// CPortalLeaderboard
//-----------------------------------------------------------------------------
CPortalLeaderboard::CPortalLeaderboard( const char *pszMapName )
{
	V_strncpy( m_szMapName, pszMapName, sizeof( m_szMapName ) );

	for ( int i = 0; i < NUM_LEADERBOARDS; ++i )
	{
		m_hLeaderboards[ i ] = 0;
		m_nCurrentPlayerIndex[ i ] = -1;
		m_pFriendsGraph[ i ] = NULL;
		m_pGlobalGraph[ i ] = NULL;
	}

	ResetCurrentRequest();
	m_bInvalid = false;
	m_bIOError = false;
	m_bGladosAllowedToSpeak = false;
	m_bGladosHasSpoken = false;

	InitGladosLines();
}

CPortalLeaderboard::~CPortalLeaderboard()
{
	ResetCurrentRequest();

	for ( int i = 0; i < NUM_LEADERBOARDS; ++i )
	{
		m_Entries[ i ].Purge();
		delete m_pFriendsGraph[ i ];
		m_pFriendsGraph[ i ] = NULL;
		delete m_pGlobalGraph[ i ];
		m_pGlobalGraph[ i ] = NULL;
	}

	m_Requests.Purge();
}

void CPortalLeaderboard::ResetCurrentRequest( void )
{
	m_CurrentRequest.m_eRequest = LEADERBOARD_REQUEST_NONE;
	m_CurrentRequest.m_eType = NUM_LEADERBOARDS;
	m_CurrentRequest.m_nParam = 0;
}

void CPortalLeaderboard::CancelRequests( void )
{
	ResetCurrentRequest();
	m_Requests.RemoveAll();
}

void CPortalLeaderboard::QueueRequest( int eRequest, LeaderboardType type )
{
	LeaderboardRequest_t request;
	request.m_eRequest = eRequest;
	request.m_eType = type;
	request.m_nParam = 0;
	m_Requests.AddToTail( request );
}

bool CPortalLeaderboard::Init( void )
{
	m_bInvalid = false;
	CancelRequests();
	for ( int i = 0; i < NUM_LEADERBOARDS; ++i )
	{
		m_hLeaderboards[ i ] = 0;
	}

	for ( int i = 0; i < NUM_LEADERBOARDS; ++i )
	{
		QueueRequest( LEADERBOARD_REQUEST_FIND, (LeaderboardType)i );
		QueueRequest( LEADERBOARD_REQUEST_DOWNLOAD_FRIENDS, (LeaderboardType)i );
		QueueRequest( LEADERBOARD_REQUEST_DOWNLOAD_GLOBAL_DATA, (LeaderboardType)i );
	}

	if ( m_Requests.Count() )
	{
		StartNextRequest();
	}

	return true;
}

void CPortalLeaderboard::ProcessRequests( void )
{
	if ( !IsRequestPending() && m_Requests.Count() )
	{
		StartNextRequest();
	}
}

bool CPortalLeaderboard::IsQuerying( void ) const
{
	return m_Requests.Count() != 0 || IsRequestPending();
}

void CPortalLeaderboard::StartNextRequest( void )
{
	m_CurrentRequest = m_Requests.Head();
	m_Requests.Remove( 0 );

	bool bStarted;
	switch ( m_CurrentRequest.m_eRequest )
	{
	case LEADERBOARD_REQUEST_WAIT:
		return;
	case LEADERBOARD_REQUEST_DOWNLOAD_FRIENDS:
		bStarted = StartDownloadFriends();
		break;
	case LEADERBOARD_REQUEST_DOWNLOAD_GLOBAL_DATA:
		bStarted = StartDownloadGlobalData();
		break;
	case LEADERBOARD_REQUEST_FIND:
		bStarted = StartFindLeaderboard();
		break;
	default:
		bStarted = false;
		break;
	}

	if ( !bStarted )
	{
		ResetCurrentRequest();
	}
}

const char *CPortalLeaderboard::GetLeaderboardName( LeaderboardType type, const char *pszMapName )
{
	static char s_szName[256];
	if ( type == LEADERBOARD_PORTAL )
	{
		V_snprintf( s_szName, sizeof( s_szName ), "challenge_portals_%s", pszMapName );
	}
	else if ( type == LEADERBOARD_TIME )
	{
		V_snprintf( s_szName, sizeof( s_szName ), "challenge_besttime_%s", pszMapName );
	}
	else
	{
		s_szName[ 0 ] = '\0';
	}
	return s_szName;
}

SteamLeaderboard_t CPortalLeaderboard::GetLeaderboardHandle( LeaderboardType type ) const
{
	if ( type == NUM_LEADERBOARDS )
	{
		type = m_CurrentRequest.m_eType;
	}

	if ( !IsValidType( type ) )
		return 0;

	return m_hLeaderboards[ type ];
}

void CPortalLeaderboard::SetLeaderboardHandle( LeaderboardType type, SteamLeaderboard_t hLeaderboard )
{
	if ( IsValidType( type ) )
	{
		m_hLeaderboards[ type ] = hLeaderboard;
	}
}

//-----------------------------------------------------------------------------
// Row queries
//-----------------------------------------------------------------------------
const PortalLeaderboardItem_t *CPortalLeaderboard::GetPlayerAtIndex( int nIndex, LeaderboardType type ) const
{
	if ( nIndex < 0 || nIndex >= m_Entries[ type ].Count() )
		return NULL;

	return &m_Entries[ type ][ nIndex ];
}

int CPortalLeaderboard::GetCurrentPlayerIndex( LeaderboardType type ) const
{
	for ( int i = 0; i < m_Entries[ type ].Count(); ++i )
	{
		if ( m_Entries[ type ][ i ].m_bLocalPlayer )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: The nearest row ahead of the local player with a strictly better
//          score; the top two rows are always targets.
//-----------------------------------------------------------------------------
int CPortalLeaderboard::GetNextTargetIndex( LeaderboardType type ) const
{
	const CUtlVector< PortalLeaderboardItem_t > &entries = m_Entries[ type ];
	int nCurrent = m_nCurrentPlayerIndex[ type ];
	if ( !entries.Count() || nCurrent < 0 )
		return -1;

	if ( nCurrent == 0 )
		return 0;

	int nTarget = nCurrent - 1;
	if ( nTarget < 2 )
		return nTarget;

	do
	{
		if ( entries[ nTarget ].m_iScore < entries[ nCurrent ].m_iScore )
			return nTarget;
		--nTarget;
	} while ( nTarget != 1 );

	return 1;
}

const PortalLeaderboardItem_t *CPortalLeaderboard::GetNextTarget( LeaderboardType type ) const
{
	int nCurrent = m_nCurrentPlayerIndex[ type ];
	if ( !m_Entries[ type ].Count() || nCurrent < 0 )
		return NULL;

	return &m_Entries[ type ][ nCurrent != 0 ? nCurrent - 1 : 0 ];
}

const PortalLeaderboardItem_t *CPortalLeaderboard::GetLocalPlayerEntry( LeaderboardType type ) const
{
	for ( int i = 0; i < m_Entries[ type ].Count(); ++i )
	{
		if ( m_Entries[ type ][ i ].m_bLocalPlayer )
			return &m_Entries[ type ][ i ];
	}

	return NULL;
}

const PortalLeaderboardItem_t *CPortalLeaderboard::GetWorstPlayer( LeaderboardType type ) const
{
	int nCount = m_Entries[ type ].Count();
	return nCount ? &m_Entries[ type ][ nCount - 1 ] : NULL;
}

const PortalLeaderboardItem_t *CPortalLeaderboard::GetCurrentBest( LeaderboardType type, int nWhich ) const
{
	if ( nWhich == 0 )
	{
		return m_Entries[ type ].Count() ? &m_Entries[ type ][ 0 ] : NULL;
	}

	if ( nWhich == 1 )
	{
		return GetNextTarget( type );
	}

	return GetLocalPlayerEntry( type );
}

static int GetMostRecentScore( LeaderboardType type )
{
	return ( type == LEADERBOARD_TIME ) ? g_nTimeScoreTempMostRecent : g_nPortalScoreTempMostRecent;
}

bool CPortalLeaderboard::IsNewBest( LeaderboardType type ) const
{
	if ( !m_Entries[ type ].Count() )
		return false;

	return GetMostRecentScore( type ) < m_Entries[ type ][ 0 ].m_iScore;
}

bool CPortalLeaderboard::DidNotPassNextTarget( LeaderboardType type ) const
{
	const PortalLeaderboardItem_t *pTarget = GetNextTarget( type );
	int nTargetScore = pTarget ? pTarget->m_iScore : LEADERBOARD_NO_SCORE;
	return GetMostRecentScore( type ) >= nTargetScore;
}

bool CPortalLeaderboard::IsCloseToNextTarget( LeaderboardType type ) const
{
	const PortalLeaderboardItem_t *pTarget = GetNextTarget( type );
	int nTargetScore = pTarget ? pTarget->m_iScore : LEADERBOARD_NO_SCORE;
	unsigned int nBehind = (unsigned int)( GetMostRecentScore( type ) - nTargetScore );
	return nBehind < (unsigned int)( type == LEADERBOARD_TIME ? LEADERBOARD_CLOSE_TIME : LEADERBOARD_CLOSE_PORTALS );
}

bool CPortalLeaderboard::IsPersonalBest( LeaderboardType type ) const
{
	const PortalLeaderboardItem_t *pLocal = GetLocalPlayerEntry( type );
	if ( !pLocal )
		return true;

	return GetMostRecentScore( type ) < pLocal->m_iScore;
}

bool CPortalLeaderboard::IsFirstScore( void ) const
{
	return GetLocalPlayerEntry( LEADERBOARD_TIME ) == NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Chooses the reaction category for each leaderboard, uses the
//          higher one (time wins ties) and picks a random line from it.
//-----------------------------------------------------------------------------
void CPortalLeaderboard::DoGladosSpokenReaction( char *pszLine, int *pnImprovedLeaderboardIndex )
{
	m_bGladosAllowedToSpeak = false;
	m_bGladosHasSpoken = true;

	const CUtlVector< PortalLeaderboardItem_t > &portals = m_Entries[ LEADERBOARD_PORTAL ];
	const CUtlVector< PortalLeaderboardItem_t > &times = m_Entries[ LEADERBOARD_TIME ];
	int nTime = g_nTimeScoreTempMostRecent;
	int nPortals = g_nPortalScoreTempMostRecent;

	// Time
	int eTimeReaction = GLADOS_REACTION_NO_FRIENDS;
	if ( times.Count() > 0 )
	{
		const PortalLeaderboardItem_t *pLocal = GetLocalPlayerEntry( LEADERBOARD_TIME );
		if ( pLocal && pLocal->m_iScore <= nTime )
		{
			eTimeReaction = GLADOS_REACTION_NO_IMPROVEMENT;
		}
		else if ( nTime < times[ 0 ].m_iScore )
		{
			eTimeReaction = GLADOS_REACTION_BEST_TIME;
		}
		else if ( pLocal )
		{
			const PortalLeaderboardItem_t *pTarget = GetNextTarget( LEADERBOARD_TIME );
			if ( pTarget && nTime <= pTarget->m_iScore )
			{
				eTimeReaction = GLADOS_REACTION_PASSED_FRIEND;
			}
			else if ( !pTarget && nTime < INT_MIN + 2 )
			{
				eTimeReaction = GLADOS_REACTION_PASSED_FRIEND;
			}
			else
			{
				int nAhead = pTarget ? pTarget->m_iScore : LEADERBOARD_NO_SCORE;
				eTimeReaction = ( (unsigned int)( nTime - nAhead ) < LEADERBOARD_CLOSE_TIME ) ? GLADOS_REACTION_CLOSE_BEHIND : GLADOS_REACTION_FAR_BEHIND;
			}
		}
	}

	// Portals. Retail looks for the local player's row in the time
	// leaderboard here, and an unimproved portal count yields the time line.
	int ePortalReaction = GLADOS_REACTION_NO_FRIENDS;
	bool bUseTime = false;
	const PortalLeaderboardItem_t *pLocalPortals = GetLocalPlayerEntry( LEADERBOARD_PORTAL );
	if ( portals.Count() > 0 && pLocalPortals && pLocalPortals->m_iScore <= nPortals )
	{
		bUseTime = true;
	}
	else if ( portals.Count() > 0 && nPortals < portals[ 0 ].m_iScore )
	{
		ePortalReaction = GLADOS_REACTION_BEST_PORTALS;
	}
	else if ( GetLocalPlayerEntry( LEADERBOARD_TIME ) )
	{
		const PortalLeaderboardItem_t *pTarget = portals.Count() ? GetNextTarget( LEADERBOARD_PORTAL ) : NULL;
		if ( pTarget && nPortals <= pTarget->m_iScore )
		{
			ePortalReaction = GLADOS_REACTION_PASSED_FRIEND;
		}
		else if ( !pTarget && nPortals < INT_MIN + 2 )
		{
			ePortalReaction = GLADOS_REACTION_PASSED_FRIEND;
		}
		else
		{
			int nAhead = pTarget ? pTarget->m_iScore : LEADERBOARD_NO_SCORE;
			ePortalReaction = ( (unsigned int)( nPortals - nAhead ) < LEADERBOARD_CLOSE_PORTALS ) ? GLADOS_REACTION_CLOSE_BEHIND : GLADOS_REACTION_FAR_BEHIND;
		}
	}

	int eReaction;
	int nImproved;
	if ( !bUseTime && eTimeReaction < ePortalReaction )
	{
		eReaction = ePortalReaction;
		nImproved = LEADERBOARD_PORTAL;
	}
	else
	{
		eReaction = eTimeReaction;
		nImproved = LEADERBOARD_TIME;
	}

	const CUtlVector< const char * > &lines = s_GladosLines[ eReaction ];
	V_strcpy( pszLine, lines[ RandomInt( 0, lines.Count() - 1 ) ] );

	if ( pnImprovedLeaderboardIndex )
	{
		*pnImprovedLeaderboardIndex = nImproved;
	}
}

//-----------------------------------------------------------------------------
// Purpose: The friend graph covers the downloaded scores, capped at 40
//          portals or six minutes.
//-----------------------------------------------------------------------------
void CPortalLeaderboard::BuildFriendsGraphData( void )
{
	LeaderboardType type = m_CurrentRequest.m_eType;
	delete m_pFriendsGraph[ type ];
	m_pFriendsGraph[ type ] = NULL;

	const CUtlVector< PortalLeaderboardItem_t > &entries = m_Entries[ type ];
	int nCount = entries.Count();
	if ( !nCount )
		return;

	int nCap = ( type == LEADERBOARD_PORTAL ) ? LEADERBOARD_GRAPH_MAX_PORTALS : LEADERBOARD_GRAPH_MAX_TIME;
	int nMin = MAX( MIN( entries[ 0 ].m_iScore, nCap ), 0 );
	int nMax = MAX( MIN( entries[ nCount - 1 ].m_iScore, nCap ), 0 );

	CLeaderboardBucketizer *pGraph = new CLeaderboardBucketizer;
	pGraph->SetEntryInfo( 0, nCount );
	pGraph->SetRange( nMin, nMax );
	for ( int i = 0; i < nCount; ++i )
	{
		pGraph->AddScore( entries[ i ].m_iScore );
	}

	m_pFriendsGraph[ type ] = pGraph;
}

void CPortalLeaderboard::SetEntry( const LeaderboardEntry_t &entry, int nIndex )
{
	PortalLeaderboardItem_t &item = m_Entries[ m_CurrentRequest.m_eType ][ nIndex ];
	item.m_iGlobalRank = entry.m_nGlobalRank;
	item.m_iRank = entry.m_nGlobalRank;
	item.m_xuid = 0;
	item.m_iScore = entry.m_nScore;
	item.m_steamIDUser = entry.m_steamIDUser;
	item.m_bLocalPlayer = false;
	item.m_szName[ 0 ] = '\0';

#if !defined( NO_STEAM )
	V_strncpy( item.m_szName, steamapicontext->SteamFriends()->GetFriendPersonaName( entry.m_steamIDUser ), sizeof( item.m_szName ) );
	item.m_bLocalPlayer = entry.m_steamIDUser.GetAccountID() == steamapicontext->SteamUser()->GetSteamID().GetAccountID();
	if ( item.m_bLocalPlayer )
	{
		m_nCurrentPlayerIndex[ m_CurrentRequest.m_eType ] = nIndex;
	}
#endif
}

#if !defined( NO_STEAM )

//-----------------------------------------------------------------------------
// Steam requests
//-----------------------------------------------------------------------------
bool CPortalLeaderboard::StartFindLeaderboard( void )
{
	LeaderboardType type = m_CurrentRequest.m_eType;
	if ( IsValidType( type ) && m_hLeaderboards[ type ] != 0 )
	{
		// Already found: move on to the next request.
		ResetCurrentRequest();
		if ( m_Requests.Count() )
		{
			StartNextRequest();
		}
		return true;
	}

	SteamAPICall_t hCall = steamapicontext->SteamUserStats()->FindLeaderboard( GetLeaderboardName( type, m_szMapName ) );
	if ( hCall == k_uAPICallInvalid )
		return false;

	m_FindResult.Set( hCall, this, &CPortalLeaderboard::OnFindLeaderboard );
	return true;
}

void CPortalLeaderboard::OnFindLeaderboard( LeaderboardFindResult_t *pResult, bool bIOFailure )
{
	m_bIOError = bIOFailure;
	if ( !pResult->m_bLeaderboardFound || bIOFailure )
	{
		CancelRequests();
		m_bInvalid = true;
		return;
	}

	const char *pszName = steamapicontext->SteamUserStats()->GetLeaderboardName( pResult->m_hSteamLeaderboard );
	if ( !V_strcmp( pszName, GetLeaderboardName( LEADERBOARD_PORTAL, m_szMapName ) ) )
	{
		m_hLeaderboards[ LEADERBOARD_PORTAL ] = pResult->m_hSteamLeaderboard;
	}
	if ( !V_strcmp( pszName, GetLeaderboardName( LEADERBOARD_TIME, m_szMapName ) ) )
	{
		m_hLeaderboards[ LEADERBOARD_TIME ] = pResult->m_hSteamLeaderboard;
	}

	ResetCurrentRequest();
	if ( m_Requests.Count() )
	{
		StartNextRequest();
	}
}

bool CPortalLeaderboard::StartDownloadFriends( void )
{
	LeaderboardType type = m_CurrentRequest.m_eType;
	if ( !IsValidType( type ) || !m_hLeaderboards[ type ] )
		return false;

	SteamAPICall_t hCall = steamapicontext->SteamUserStats()->DownloadLeaderboardEntries( m_hLeaderboards[ type ], k_ELeaderboardDataRequestFriends, 0, 0 );
	if ( hCall == k_uAPICallInvalid )
		return false;

	m_DownloadResult.Set( hCall, this, &CPortalLeaderboard::OnScoresDownloaded );
	return true;
}

void CPortalLeaderboard::OnScoresDownloaded( LeaderboardScoresDownloaded_t *pResult, bool bIOFailure )
{
	if ( !pResult || bIOFailure )
	{
		ResetCurrentRequest();
		m_bInvalid = true;
		return;
	}

	LeaderboardType type = m_CurrentRequest.m_eType;
	CUtlVector< PortalLeaderboardItem_t > &entries = m_Entries[ type ];
	entries.Purge();
	m_nCurrentPlayerIndex[ type ] = -1;

	int nEntries = MIN( pResult->m_cEntryCount, LEADERBOARD_MAX_ENTRIES );
	int nCopies = leaderboard_duplicate_entries.GetInt();
	int nRows = nEntries * nCopies;
	if ( nRows > 0 )
	{
		entries.SetCount( nRows );
		for ( int i = 0; i < nRows; ++i )
		{
			entries[ i ].m_steamIDUser = CSteamID();
		}
	}

	int nRow = 0;
	for ( int i = 0; i < nEntries; ++i )
	{
		LeaderboardEntry_t entry;
		if ( steamapicontext->SteamUserStats()->GetDownloadedLeaderboardEntry( pResult->m_hSteamLeaderboardEntries, i, &entry, NULL, 0 ) )
		{
			for ( int j = 0; j < nCopies; ++j )
			{
				SetEntry( entry, nRow + j );
			}
		}
		nRow += nCopies;
	}

	BuildFriendsGraphData();
	ResetCurrentRequest();
}

//-----------------------------------------------------------------------------
// Purpose: Requests the global score histogram from the Steam Web API.
//-----------------------------------------------------------------------------
bool CPortalLeaderboard::StartDownloadGlobalData( void )
{
	ISteamHTTP *pHTTP = steamapicontext->SteamHTTP();
	if ( !pHTTP )
	{
		Warning( "Can't get ISteamHTTP to get global graph data\n" );
		return false;
	}

	char szQuery[512];
	V_snprintf( szQuery, sizeof( szQuery ), "?leaderboardName=%s&%s", GetLeaderboardName( m_CurrentRequest.m_eType, m_szMapName ), "format=vdf" );

	int nAppID = engine->GetAppID();
	const char *pszFormat;
	if ( cl_steam_universe.GetInt() == 1 )
	{
		DevWarning( "cl_steam_universe not set to Public! DO NOT SHIP THIS WAY!!\n" );
		pszFormat = "http://api-beta.steampowered.com/%s_%d/%s/v0001/%s&appid=%d";
	}
	else if ( cl_steam_universe.GetInt() == 2 )
	{
		DevWarning( "cl_steam_universe not set to Public! DO NOT SHIP THIS WAY!!\n" );
		pszFormat = "http://localhost:8282/%s_%d/%s/v0001/%s&appid=%d";
	}
	else
	{
		pszFormat = "http://api.steampowered.com/%s_%d/%s/v0001/%s&appid=%d";
	}

	static char s_szURL[512];
	V_snprintf( s_szURL, sizeof( s_szURL ), pszFormat, "IPortal2Leaderboards", nAppID, "GetBucketizedData", szQuery, nAppID );

	HTTPRequestHandle hRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, s_szURL );
	SteamAPICall_t hCall;
	if ( !pHTTP->SendHTTPRequest( hRequest, &hCall ) )
	{
		Warning( "Failed to get global graph data from %s\n", s_szURL );
		return false;
	}

	m_HTTPResult.Set( hCall, this, &CPortalLeaderboard::OnGlobalDataDownloaded );
	return true;
}

void CPortalLeaderboard::OnGlobalDataDownloaded( HTTPRequestCompleted_t *pResult, bool bIOFailure )
{
	LeaderboardType type = m_CurrentRequest.m_eType;
	delete m_pGlobalGraph[ type ];
	m_pGlobalGraph[ type ] = NULL;

	ISteamHTTP *pHTTP = steamapicontext->SteamHTTP();
	if ( !pHTTP )
	{
		m_bInvalid = true;
		m_bIOError = true;
		return;
	}

	if ( pResult->m_eStatusCode == k_EHTTPStatusCode200OK && pResult->m_bRequestSuccessful && !bIOFailure )
	{
		uint32 nBodySize;
		if ( pHTTP->GetHTTPResponseBodySize( pResult->m_hRequest, &nBodySize ) )
		{
			CUtlBuffer body( 0, 0, CUtlBuffer::TEXT_BUFFER );
			body.EnsureCapacity( nBodySize + 1 );
			if ( pHTTP->GetHTTPResponseBodyData( pResult->m_hRequest, (uint8 *)body.Base(), nBodySize ) )
			{
				body.SeekPut( CUtlBuffer::SEEK_HEAD, nBodySize );

				KeyValues *pData = new KeyValues( "bucketizedData" );
				if ( !pData->LoadFromBuffer( NULL, body ) )
				{
					Warning( "Error parsing KeyValues for CPortalLeaderboard::OnLeaderboardDownloadGlobalData.\n" );
				}
				else
				{
					int nBuckets = pData->GetInt( "bucketCount", 0 );
					int nMin = pData->GetInt( "min", 0 );
					int nMax = pData->GetInt( "max", 0 );
					int nNumEntries = pData->GetInt( "numEntries", 0 );

					CUtlVector< int > buckets;
					if ( nBuckets > 0 )
					{
						buckets.SetCount( nBuckets );
						for ( int i = 0; i < nBuckets; ++i )
						{
							buckets[ i ] = nNumEntries;
						}
					}

					KeyValues *pBuckets = pData->FindKey( "buckets" );
					int nBucket = 0;
					for ( KeyValues *pBucket = pBuckets ? pBuckets->GetFirstSubKey() : NULL; nBucket < nBuckets && pBucket; pBucket = pBucket->GetNextKey() )
					{
						buckets[ nBucket++ ] = pBucket->GetInt( "playerCount", 0 );
					}

					CLeaderboardBucketizer *pGraph = new CLeaderboardBucketizer;
					pGraph->SetEntryInfo( 0, nNumEntries );
					pGraph->SetRange( nMin, nMax );
					pGraph->SetBuckets( buckets );
					m_pGlobalGraph[ type ] = pGraph;
				}
				pData->deleteThis();

				pHTTP->ReleaseHTTPRequest( pResult->m_hRequest );
				ResetCurrentRequest();
				return;
			}
		}
	}

	Warning( "Failed to get leaderboard global data: HTTP status %d\n", pResult->m_eStatusCode );
	pHTTP->ReleaseHTTPRequest( pResult->m_hRequest );
	ResetCurrentRequest();
	m_bInvalid = true;
	m_bIOError = true;
}

#else // NO_STEAM

// Portal 2 port: no Steam in this build; CPortalLeaderboardManager never
// creates a leaderboard, so these report that nothing started.
bool CPortalLeaderboard::StartFindLeaderboard( void ) { return false; }
bool CPortalLeaderboard::StartDownloadFriends( void ) { return false; }
bool CPortalLeaderboard::StartDownloadGlobalData( void ) { return false; }

#endif // NO_STEAM
