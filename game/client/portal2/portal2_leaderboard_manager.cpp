//========= Portal 2 reconstruction ============================================//
//
// Purpose: Challenge mode leaderboard manager; see portal2_leaderboard_manager.h.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_leaderboard_manager.h"
#include "matchmaking/imatchframework.h"
#include "filesystem.h"
#include "tier1/fmtstr.h"
#if !defined( NO_STEAM )
#include "steam/steam_api.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CPortalLeaderboardManager g_PortalLeaderboardManager;

// The map the temporary score updates were made on
static char s_szTempScoresMap[ MAX_PATH ];

CPortalLeaderboardManager *PortalLeaderboardManager()
{
	return &g_PortalLeaderboardManager;
}

CPortalLeaderboardManager::CPortalLeaderboardManager()
	: CAutoGameSystemPerFrame( "PortalLeaderboardManager" ), m_LeaderboardNames( 0, 32, true )
{
	m_pCurrentLeaderboard = NULL;
	m_pChallengeMapList = NULL;
}

CPortalLeaderboardManager::~CPortalLeaderboardManager()
{
	CleanupLeaderboard();

	if ( m_pChallengeMapList )
	{
		m_pChallengeMapList->deleteThis();
		m_pChallengeMapList = NULL;
	}
}

bool CPortalLeaderboardManager::Init()
{
	if ( g_pMatchFramework )
	{
		g_pMatchFramework->GetEventsSubscription()->Subscribe( this );
	}

	LoadChallengeMapList();
	return true;
}

void CPortalLeaderboardManager::Shutdown()
{
	if ( g_pMatchFramework )
	{
		g_pMatchFramework->GetEventsSubscription()->Unsubscribe( this );
	}
}

bool CPortalLeaderboardManager::LoadChallengeMapList( void )
{
	m_pChallengeMapList = new KeyValues( "challenge_maplist" );
	if ( m_pChallengeMapList->LoadFromFile( g_pFullFileSystem, "challenge_maplist.txt", NULL ) )
		return true;

	m_pChallengeMapList->deleteThis();
	m_pChallengeMapList = NULL;
	return false;
}

KeyValues *CPortalLeaderboardManager::GetChallengeMapsFromChapter( int nChapter, bool bCoop )
{
	if ( !m_pChallengeMapList )
		return NULL;

	KeyValues *pMode = m_pChallengeMapList->FindKey( bCoop ? "coop" : "sp" );
	if ( !pMode )
		return NULL;

	return pMode->FindKey( CFmtStr( "chapter_%d", nChapter ) );
}

void CPortalLeaderboardManager::CleanupLeaderboard( void )
{
	for ( int i = 0; i < m_Leaderboards.Count(); ++i )
	{
		delete m_Leaderboards[ i ];
	}

	m_Leaderboards.Purge();
	m_LeaderboardNames.RemoveAll();
	m_pCurrentLeaderboard = NULL;
}

void CPortalLeaderboardManager::LevelShutdownPreEntity()
{
	CleanupLeaderboard();
}

void CPortalLeaderboardManager::Update( float frametime )
{
	if ( m_pCurrentLeaderboard )
	{
		m_pCurrentLeaderboard->ProcessRequests();
	}
}

void CPortalLeaderboardManager::OnEvent( KeyValues *pEvent )
{
	if ( V_stricmp( "OnProfilesChanged", pEvent->GetName() ) )
		return;

	CleanupLeaderboard();
	V_strncpy( s_szTempScoresMap, "", sizeof( s_szTempScoresMap ) );
}

CPortalLeaderboard *CPortalLeaderboardManager::GetLeaderboard( const char *pszMapName )
{
#if !defined( NO_STEAM )
	if ( !steamapicontext || !steamapicontext->SteamUser() || !steamapicontext->SteamUser()->BLoggedOn() || !steamapicontext->SteamUserStats() )
		return NULL;

	// Only one map's leaderboards query at a time.
	if ( m_pCurrentLeaderboard && m_pCurrentLeaderboard->IsQuerying() )
		return NULL;

	CUtlSymbol name = m_LeaderboardNames.Find( pszMapName );
	if ( !name.IsValid() )
	{
		name = m_LeaderboardNames.AddString( pszMapName );
		if ( m_Leaderboards.Count() <= (UtlSymId_t)name )
		{
			int nOldCount = m_Leaderboards.Count();
			m_Leaderboards.SetCount( (UtlSymId_t)name + 1 );
			for ( int i = nOldCount; i < m_Leaderboards.Count(); ++i )
			{
				m_Leaderboards[ i ] = NULL;
			}
		}

		m_Leaderboards[ (UtlSymId_t)name ] = new CPortalLeaderboard( pszMapName );
		m_Leaderboards[ (UtlSymId_t)name ]->Init();
	}

	m_pCurrentLeaderboard = m_Leaderboards[ (UtlSymId_t)name ];
	return m_pCurrentLeaderboard;
#else
	// Portal 2 port: leaderboards live in Steam UserStats; this build has no
	// Steam, so callers get no leaderboard, as retail does when offline.
	static bool s_bWarned = false;
	if ( !s_bWarned )
	{
		s_bWarned = true;
		DevWarning( "Portal 2: challenge mode leaderboards (Steam UserStats) are not supported by this build\n" );
	}
	return NULL;
#endif
}

const char *CPortalLeaderboardManager::GetTempScoresMap()
{
	return s_szTempScoresMap;
}

void CPortalLeaderboardManager::SetTempScoresMap( const char *pszMapName )
{
	V_strncpy( s_szTempScoresMap, pszMapName ? pszMapName : "", sizeof( s_szTempScoresMap ) );
}
