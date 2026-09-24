//========= Portal 2 reconstruction ============================================//
//
// Purpose: Community (Steam Workshop) co-op map flow; see c_community_coop.h.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). Retail builds this with the Puzzle
// Maker; it needs a match session (g_pMatchFramework), the Steam Workshop
// (WorkshopManager()) and BaseModUI's community map queue. Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_community_coop.h"

#if defined( PORTAL2_PUZZLEMAKER ) && !defined( NO_STEAM )

#include "matchmaking/imatchframework.h"
#include "utlqueue.h"	// ugc_file_info_manager.h uses CUtlQueue without including it
#include "ugc_file_info_manager.h"
#include "ugc_workshop_manager.h"
#include "gameui_interface.h"
#include "portal2/basemodpanel.h"
#include "portal2/uigamedata.h"
#include "portal2/vgenericwaitscreen.h"
#include "portal2_engine_compat.h"
#include "tier1/fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

ConVar cl_communitycoop_progress_throttle_rate( "cl_communitycoop_progress_throttle_rate", "0.3f", 0 );

extern ConVar cm_community_debug_spew;
extern Color rgbaCommunityDebug;

// Seconds to wait for the next map's file information before giving up
#define COMMUNITY_COOP_FILE_INFO_TIMEOUT	5.0f

// Most history entries sent in one session command
#define COMMUNITY_COOP_HISTORY_BATCH		10

C_CommunityCoopManager g_CommunityCoopManager;

static IMatchSession *GetCoopMatchSession( void )
{
	return g_pMatchFramework ? g_pMatchFramework->GetMatchSession() : NULL;
}

static bool IsCommunityCoopQueueMode( void )
{
	ECommunityMapQueueMode eMode = BASEMODPANEL_SINGLETON.GetCommunityMapQueueMode();
	return eMode == QUEUEMODE_USER_COOP_QUEUE || eMode == QUEUEMODE_COOP_QUICK_PLAY;
}

// A session command addressed to "all" or "host" from the first local player
static KeyValues *NewCoopCommand( const char *pszCommand, const char *pszRun )
{
	KeyValues *pCommand = new KeyValues( pszCommand );
	pCommand->SetString( "run", pszRun );
	return pCommand;
}

static void SetLocalXUID( KeyValues *pCommand )
{
	pCommand->SetUint64( "clxuid", g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( 0 )->GetXUID() );
}

//-----------------------------------------------------------------------------
// Download wait screen hooks
//-----------------------------------------------------------------------------
class CCM_WaitForDownloadOperation : public IMatchAsyncOperation
{
public:
	virtual bool IsFinished() { return false; }
	virtual AsyncOperationState_t GetState() { return AOS_RUNNING; }
	virtual uint64 GetResult() { return 0; }
	virtual void Abort() { g_CommunityCoopManager.CancelSkipMap(); }
	virtual void Release() {}
};

class CCM_WaitForDownloadMapCallback : public IWaitscreenCallbackInterface
{
public:
	CCM_WaitForDownloadMapCallback() : m_bDone( false ) {}

	virtual bool OnKeyCodePressed( vgui::KeyCode code ) { return true; }
	virtual void OnThink()
	{
		if ( !m_bDone )
		{
			m_bDone = g_CommunityCoopManager.GetState() != C_CommunityCoopManager::CCM_DOWNLOADING;
		}
	}

	bool m_bDone;
};

static CCM_WaitForDownloadOperation g_CCM_WaitForDownloadOperation;
static CCM_WaitForDownloadMapCallback g_CCM_WaitForDownloadMapCallback;

static void OpenDownloadWaitScreen( void )
{
	BASEMODPANEL_SINGLETON.CloseAllWindows( 0 );

	KeyValues *pSettings = new KeyValues( "WaitScreen" );
	KeyValues::AutoDelete autodelete( pSettings );
	pSettings->SetPtr( "options/asyncoperation", &g_CCM_WaitForDownloadOperation );
	g_CCM_WaitForDownloadMapCallback.m_bDone = false;
	pSettings->SetPtr( "options/waitscreencallback", &g_CCM_WaitForDownloadMapCallback );

	CUIGameData::Get()->CloseWaitScreen( NULL, NULL );
	CUIGameData::Get()->OpenWaitScreen( "#PORTAL2_WaitScreen_DownloadingPuzzle", 1.0f, pSettings );
}

//-----------------------------------------------------------------------------
// C_CommunityCoopManager
//-----------------------------------------------------------------------------
C_CommunityCoopManager::C_CommunityCoopManager() : CAutoGameSystemPerFrame( NULL )
{
	m_nMapID = 0;
	m_hMapFile = 0;
	m_hMapPreviewFile = 0;
	m_flLastProgressTime = 0.0f;
	ResetState();
}

C_CommunityCoopManager::~C_CommunityCoopManager()
{
	m_PartnerHistory.Purge();
}

void C_CommunityCoopManager::ResetState( void )
{
	m_eDownloadSource = CCM_DOWNLOAD_FROM_START;
	m_eState = CCM_IDLE;
	m_bHasNextMap = false;
	m_bHasPartnerHistory = false;
	m_flStateStartTime = 0.0f;
	m_flLastProgressTime = 0.0f;
}

void C_CommunityCoopManager::PostInit()
{
	if ( g_pMatchFramework )
	{
		g_pMatchFramework->GetEventsSubscription()->Subscribe( this );
	}
}

void C_CommunityCoopManager::Shutdown()
{
	if ( g_pMatchFramework )
	{
		g_pMatchFramework->GetEventsSubscription()->Unsubscribe( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: A new map started: clear both machines' voting and download state.
//-----------------------------------------------------------------------------
void C_CommunityCoopManager::LevelInitPostEntity()
{
	IMatchSession *pSession = GetCoopMatchSession();
	if ( pSession && pSession->GetSessionSettings()->GetInt( "members/numMachines", 0 ) > 1 )
	{
		KeyValues *pCommand = NewCoopCommand( "Portal2::ClientCommunityVotingState", "all" );
		SetLocalXUID( pCommand );
		pCommand->SetString( "voting_state", "has_not_voted" );
		pSession->Command( pCommand );
		pCommand->deleteThis();

		pCommand = NewCoopCommand( "Portal2::ClientWorkshopDownloadProgress", "all" );
		SetLocalXUID( pCommand );
		pCommand->SetFloat( "workshop_map_progress", 0.0f );
		pSession->Command( pCommand );
		pCommand->deleteThis();
	}

	ResetState();
}

void C_CommunityCoopManager::LevelShutdownPreEntity()
{
	ResetState();
}

bool C_CommunityCoopManager::IsInPartnerHistoryQueue( PublishedFileId_t nMapID ) const
{
	for ( int i = 0; i < m_PartnerHistory.Count(); ++i )
	{
		if ( m_PartnerHistory[ i ].m_nPublishedFileID == nMapID )
			return true;
	}

	return false;
}

void C_CommunityCoopManager::ClearPartnerHistory( void )
{
	m_PartnerHistory.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Sends the co-op maps in our queue history to the host in batches,
//          then marks the history complete.
//-----------------------------------------------------------------------------
void C_CommunityCoopManager::OnClientReady( void )
{
	if ( m_eState != CCM_SENDING_HISTORY && cm_community_debug_spew.GetBool() )
	{
		ConColorMsg( rgbaCommunityDebug, "Begin sending client history\n" );
	}

	m_eState = CCM_SENDING_HISTORY;

	if ( !BASEMODPANEL_SINGLETON.QueueHistoryReady() )
		return;

	IMatchSession *pSession = GetCoopMatchSession();
	if ( !pSession )
		return;

	if ( cm_community_debug_spew.GetBool() )
	{
		ConColorMsg( rgbaCommunityDebug, "Queue history ready. Sending...\n" );
	}

	KeyValues *pHistory = NewCoopCommand( "CommunityCoopPartnerHistoryInfo", "host" );
	int nNumEntries = BASEMODPANEL_SINGLETON.GetNumQueueHistoryEntries();
	int nBatched = 0;
	for ( int i = 0; i < nNumEntries; ++i )
	{
		const PublishedFileInfo_t *pInfo = BASEMODPANEL_SINGLETON.GetQueueHistoryEntry( i );
		if ( !pInfo )
			continue;

		if ( pInfo->HasTag( "Cooperative" ) )
		{
			++nBatched;
			KeyValues *pEntry = new KeyValues( "PublishedFileInfo" );
			pEntry->SetUint64( "PublishedFileID", pInfo->m_nPublishedFileId );
			pEntry->SetUint64( "FileHandle", pInfo->m_hFile );
			pEntry->SetUint64( "PreviewFileHandle", pInfo->m_hPreviewFile );
			pHistory->AddSubKey( pEntry );
		}

		if ( nBatched == COMMUNITY_COOP_HISTORY_BATCH )
		{
			pSession->Command( pHistory );
			pHistory->deleteThis();
			pHistory = NewCoopCommand( "CommunityCoopPartnerHistoryInfo", "host" );
			nBatched = 0;
		}
	}

	if ( cm_community_debug_spew.GetBool() )
	{
		DevMsg( "History send complete\n" );
	}

	if ( nBatched != 0 )
	{
		pSession->Command( pHistory );
	}
	pHistory->deleteThis();

	KeyValues *pComplete = NewCoopCommand( "CommunityCoopPartnerHistoryInfoComplete", "host" );
	pSession->Command( pComplete );
	pComplete->deleteThis();

	m_eState = CCM_IDLE;
}

void C_CommunityCoopManager::BroadcastStartDownloadingMap( const PublishedFileInfo_t *pFileInfo, const char *pszFrom )
{
	if ( !pFileInfo )
	{
		DevMsg( "[CCommunityCoop] BroadcastStartDownloadingMap - Failed: no file info\n" );
		return;
	}

	IMatchSession *pSession = GetCoopMatchSession();
	if ( !pSession )
		return;

	KeyValues *pCommand = NewCoopCommand( "CommunityCoopStartDownloadingMap", "all" );
	pCommand->SetUint64( "PublishedFileID", pFileInfo->m_nPublishedFileId );
	pCommand->SetUint64( "FileHandle", pFileInfo->m_hFile );
	pCommand->SetUint64( "PreviewFileHandle", pFileInfo->m_hPreviewFile );
	if ( pszFrom )
	{
		pCommand->SetString( "from", pszFrom );
	}

	pSession->Command( pCommand );
	pCommand->deleteThis();
}

void C_CommunityCoopManager::SendStartCommand( void )
{
	KeyValues *pCommand = NewCoopCommand( "CommunityCoopStart", "all" );
	KeyValues::AutoDelete autodelete( pCommand );

	ECommunityMapQueueMode eMode = BASEMODPANEL_SINGLETON.GetCommunityMapQueueMode();
	if ( eMode == QUEUEMODE_USER_COOP_QUEUE )
	{
		pCommand->SetString( "mode", "coop" );
	}
	else if ( eMode == QUEUEMODE_COOP_QUICK_PLAY )
	{
		pCommand->SetString( "mode", "quickplay" );
	}
	else
	{
		return;
	}

	GetCoopMatchSession()->Command( pCommand );
}

void C_CommunityCoopManager::AbortToFrontScreen( void )
{
	if ( GetCoopMatchSession() )
	{
		g_pMatchFramework->CloseSession();
	}
	else
	{
		engine->ExecuteClientCmd( "disconnect" );
	}

	BASEMODPANEL_SINGLETON.CloseAllWindows( 0 );
	BASEMODPANEL_SINGLETON.OpenFrontScreen( false );
	ResetState();
}

void C_CommunityCoopManager::CancelSkipMap( void )
{
	m_eState = CCM_IDLE;
	BASEMODPANEL_SINGLETON.CloseAllWindows( 0 );
	GameUI().AllowEngineHideGameUI();
	GameUI().HideGameUI();

	IMatchSession *pSession = GetCoopMatchSession();
	if ( !pSession )
		return;

	KeyValues *pCommand = NewCoopCommand( "CancelSkipMap", "all" );
	pSession->Command( pCommand );
	pCommand->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Waits until every player has voted on the finished map, then
//          downloads the next one.
//-----------------------------------------------------------------------------
void C_CommunityCoopManager::UpdateVoting( void )
{
	KeyValues *pSettings = GetCoopMatchSession()->GetSessionSettings();
	int nNumMachines = pSettings->GetInt( "members/numMachines", 0 );
	if ( nNumMachines < 2 )
	{
		AbortToFrontScreen();
		return;
	}

	bool bSomeoneHasNotVoted = false;
	for ( int nMachine = 0; nMachine < nNumMachines; ++nMachine )
	{
		KeyValues *pMachine = pSettings->FindKey( CFmtStr( "members/machine%d", nMachine ) );
		int nNumPlayers = pMachine ? pMachine->GetInt( "numPlayers", 0 ) : 0;
		for ( int nPlayer = 0; nPlayer < nNumPlayers; ++nPlayer )
		{
			KeyValues *pPlayer = pMachine->FindKey( CFmtStr( "player%d", nPlayer ) );
			KeyValues *pVote = pPlayer ? pPlayer->FindKey( "game/voting_state" ) : NULL;
			if ( pVote && !V_stricmp( pVote->GetString(), "has_not_voted" ) )
			{
				bSomeoneHasNotVoted = true;
			}
		}
	}

	if ( bSomeoneHasNotVoted )
		return;

	m_eState = CCM_DOWNLOADING;
	OpenDownloadWaitScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Tracks the map download, shares our progress and moves on once
//          every player has the map.
//-----------------------------------------------------------------------------
void C_CommunityCoopManager::UpdateDownload( void )
{
	UGCFileRequestStatus_t eStatus = WorkshopManager().GetUGCFileRequestStatus( m_hMapFile );
	float flProgress = WorkshopManager().GetUGCFileDownloadProgress( m_hMapFile );

	if ( eStatus == UGCFILEREQUEST_READY )
	{
		WorkshopManager().PromoteUGCFileRequestToTop( m_hMapFile );
	}
	else if ( eStatus == UGCFILEREQUEST_FINISHED )
	{
		flProgress = 1.0f;
		if ( !Portal2Engine::IsClientLocalToActiveServer() )
		{
			BASEMODPANEL_SINGLETON.SetCurrentCommunityMapID( m_nMapID );
		}
	}
	else if ( eStatus == UGCFILEREQUEST_ERROR )
	{
		DevMsg( "[CCommunityCoop] OnWaitingForDownload - UGCFILEREQUEST_ERROR\n" );
		AbortToFrontScreen();
		return;
	}

	IMatchSession *pSession = GetCoopMatchSession();
	if ( gpGlobals->curtime - m_flLastProgressTime > cl_communitycoop_progress_throttle_rate.GetFloat() )
	{
		KeyValues *pCommand = NewCoopCommand( "Portal2::ClientWorkshopDownloadProgress", "all" );
		SetLocalXUID( pCommand );
		pCommand->SetFloat( "workshop_map_progress", flProgress );
		pSession->Command( pCommand );
		pCommand->deleteThis();
		m_flLastProgressTime = gpGlobals->curtime;
	}

	KeyValues *pSettings = pSession->GetSessionSettings();
	int nNumMachines = pSettings->GetInt( "members/numMachines", 0 );
	if ( nNumMachines < 2 )
	{
		DevMsg( "[CCommunityCoop] OnWaitingForDownload - numMachine < 2 somehow\n" );
		AbortToFrontScreen();
		return;
	}

	// The slowest player's progress drives the wait screen.
	float flLowestProgress = 1.0f;
	for ( int nMachine = 0; nMachine < nNumMachines; ++nMachine )
	{
		KeyValues *pMachine = pSettings->FindKey( CFmtStr( "members/machine%d", nMachine ) );
		int nNumPlayers = pMachine ? pMachine->GetInt( "numPlayers", 0 ) : 0;
		for ( int nPlayer = 0; nPlayer < nNumPlayers; ++nPlayer )
		{
			KeyValues *pPlayer = pMachine->FindKey( CFmtStr( "player%d", nPlayer ) );
			float flPlayerProgress = pPlayer ? pPlayer->GetFloat( "game/workshop_map_progress", 0.0f ) : 0.0f;
			flLowestProgress = MIN( flLowestProgress, flPlayerProgress );
		}
	}

	CUIGameData::Get()->UpdateWaitPanel( m_hMapFile, flLowestProgress );
	CUIGameData::Get()->UpdateWaitPanel( "#PORTAL2_WaitScreen_DownloadingPuzzle", 1.0f );

	if ( flLowestProgress == 1.0f )
	{
		m_eState = CCM_LEVEL_TRANSITION;
	}
}

CUtlString C_CommunityCoopManager::GetCommunityMapName( PublishedFileId_t nMapID )
{
	const PublishedFileInfo_t *pInfo = WorkshopManager().GetPublishedFileInfoByID( nMapID );
	if ( !pInfo )
		return CUtlString( "" );

	char szBaseName[128];
	V_FileBase( pInfo->m_pchFileName, szBaseName, sizeof( szBaseName ) );
	return CUtlString( CFmtStr( "workshop/%llu/%s", pInfo->m_hFile, szBaseName ) );
}

//-----------------------------------------------------------------------------
// Purpose: Everyone has the map: the host starts it (or the partner switches
//          to it after a skip/rate vote).
//-----------------------------------------------------------------------------
void C_CommunityCoopManager::UpdateLevelTransition( void )
{
	if ( m_eDownloadSource == CCM_DOWNLOAD_FROM_START )
	{
		IMatchSession *pSession = GetCoopMatchSession();
		if ( pSession )
		{
			const char *pszMap = pSession->GetSessionSettings()->GetString( "game/map", "" );
			bool bHost = !V_stricmp( "host", pSession->GetSessionSystemData()->GetString( "type", "host" ) );
			if ( !pszMap || !pszMap[ 0 ] )
			{
				if ( bHost )
				{
					CUtlString strMapName = GetCommunityMapName( BASEMODPANEL_SINGLETON.GetCurrentCommunityMapID() );
					DevMsg( "[CCommunityCoop] OnLevelTransition - host starting map [%s]\n", strMapName.Get() );

					KeyValues *pUpdate = new KeyValues( "update" );
					pUpdate->SetString( "update/game/map", strMapName.Get() );
					pSession->UpdateSessionSettings( pUpdate );
					if ( cm_community_debug_spew.GetBool() )
					{
						DevMsg( "Updating map to: %s", strMapName.Get() );
					}
					pUpdate->deleteThis();
				}
				else
				{
					DevMsg( "[CCommunityCoop] OnLevelTransition - client is ready to start [%s]\n", pszMap );
					BASEMODPANEL_SINGLETON.OnClientReady();
				}
			}
			else
			{
				DevMsg( "[CCommunityCoop] OnLevelTransition - no map name on the [%s]\n", bHost ? "host" : "client" );
			}
		}
	}
	else if ( m_eDownloadSource == CCM_DOWNLOAD_FROM_RATE_MAP || m_eDownloadSource == CCM_DOWNLOAD_FROM_SKIP_MAP )
	{
		engine->ExecuteClientCmd( "playvideo_end_level_transition p2_menu01 2" );
		if ( Portal2Engine::IsClientLocalToActiveServer() )
		{
			BASEMODPANEL_SINGLETON.SetCurrentCommunityMapID( m_nMapID );
			CUtlString strMapName = GetCommunityMapName( m_nMapID );
			char szCommand[256];
			V_snprintf( szCommand, sizeof( szCommand ), "mp_select_level %s", strMapName.Get() );
			if ( cm_community_debug_spew.GetBool() )
			{
				DevMsg( "Transitioning to map: %s\n", szCommand + V_strlen( "mp_select_level " ) );
			}
			engine->ServerCmd( szCommand, true );
		}
	}

	m_eState = CCM_IDLE;
}

void C_CommunityCoopManager::UpdateWaitingForFileInfo( void )
{
	PublishedFileId_t nMapID = BASEMODPANEL_SINGLETON.GetCurrentCommunityMapID();
	const PublishedFileInfo_t *pInfo = WorkshopManager().GetPublishedFileInfoByID( nMapID );
	if ( pInfo )
	{
		if ( cm_community_debug_spew.GetBool() )
		{
			ConColorMsg( rgbaCommunityDebug, "FileInfo recieved for %llu.  Starting download...\n", nMapID );
		}
		DevMsg( "[CCommunityCoop] CCM_WAITING_FOR_FILE_INFO - Success\n" );
		BroadcastStartDownloadingMap( pInfo, NULL );
		m_eState = CCM_DOWNLOADING;
		return;
	}

	if ( gpGlobals->curtime - m_flStateStartTime > COMMUNITY_COOP_FILE_INFO_TIMEOUT )
	{
		if ( cm_community_debug_spew.GetBool() )
		{
			ConColorMsg( rgbaCommunityDebug, "Info timed out waiting for ID %llu.  DISCONNECT!\n", nMapID );
		}

		g_pMatchFramework->GetEventsSubscription()->BroadcastEvent( new KeyValues( "OnMatchSessionUpdate", "state", "error", "error", "nopuzzle" ) );
		ResetState();
		DevMsg( "[CCommunityCoop] CCM_WAITING_FOR_FILE_INFO - Failed\n" );
	}
}

void C_CommunityCoopManager::Update( float frametime )
{
	if ( !IsCommunityCoopQueueMode() || !GetCoopMatchSession() )
		return;

	switch ( m_eState )
	{
	case CCM_DOWNLOADING:
		UpdateDownload();
		break;
	case CCM_WAITING_FOR_VOTES:
		UpdateVoting();
		break;
	case CCM_LEVEL_TRANSITION:
		UpdateLevelTransition();
		break;
	case CCM_WAITING_FOR_FILE_INFO:
		UpdateWaitingForFileInfo();
		break;
	case CCM_SENDING_HISTORY:
		DevMsg( "[CCommunityCoop] Sending Client History\n" );
		OnClientReady();
		break;
	}
}

void C_CommunityCoopManager::OnEvent( KeyValues *pEvent )
{
	ECommunityMapQueueMode eMode = BASEMODPANEL_SINGLETON.GetCommunityMapQueueMode();
	const char *pszEvent = pEvent->GetName();

	if ( !GetCoopMatchSession() )
		return;

	if ( !V_stricmp( pszEvent, "Command::CommunityCoopStart" ) )
	{
		BASEMODPANEL_SINGLETON.SetCommunityMapQueueMode(
			!V_stricmp( pEvent->GetString( "mode", "" ), "coop" ) ? QUEUEMODE_USER_COOP_QUEUE : QUEUEMODE_COOP_QUICK_PLAY );
		return;
	}

	if ( !V_stricmp( pszEvent, "Command::CommunityCoopPartnerHistoryInfo" ) )
	{
		DevMsg( "[CCommunityCoop] Receives Partner's History - In Progress\n" );
		for ( KeyValues *pEntry = pEvent->GetFirstTrueSubKey(); pEntry; pEntry = pEntry->GetNextTrueSubKey() )
		{
			if ( V_stricmp( pEntry->GetName(), "PublishedFileInfo" ) )
				continue;

			PartnerHistoryEntry_t &entry = m_PartnerHistory[ m_PartnerHistory.AddToTail() ];
			entry.m_nPublishedFileID = pEntry->GetUint64( "PublishedFileID" );
			entry.m_hFile = pEntry->GetUint64( "FileHandle" );
			entry.m_hPreviewFile = pEntry->GetUint64( "PreviewFileHandle" );
		}

		if ( cm_community_debug_spew.GetBool() )
		{
			ConColorMsg( rgbaCommunityDebug, "Client history info recieved.  History count at %d\n", m_PartnerHistory.Count() );
		}
		return;
	}

	if ( !V_stricmp( pszEvent, "Command::CommunityCoopPartnerHistoryInfoComplete" ) )
	{
		DevMsg( "[CCommunityCoop] Receives Partner's History - Completed\n" );
		if ( cm_community_debug_spew.GetBool() )
		{
			ConColorMsg( rgbaCommunityDebug, "Client history COMPLETE!\n" );
		}

		SendStartCommand();

		ECommunityMapQueueMode eCurrentMode = BASEMODPANEL_SINGLETON.GetCommunityMapQueueMode();
		if ( eCurrentMode == QUEUEMODE_USER_COOP_QUEUE )
		{
			PublishedFileId_t nMapID = BASEMODPANEL_SINGLETON.GetCurrentCommunityMapID();
			BroadcastStartDownloadingMap( BASEMODPANEL_SINGLETON.GetCommunityMapByFileID( nMapID ), NULL );
		}
		else if ( eCurrentMode == QUEUEMODE_COOP_QUICK_PLAY )
		{
			m_bHasPartnerHistory = true;
			m_eState = CCM_WAITING_FOR_FILE_INFO;
			m_flStateStartTime = gpGlobals->curtime;
			if ( cm_community_debug_spew.GetBool() )
			{
				ConColorMsg( rgbaCommunityDebug, "PartnerHistory acquired! Querying for quickplay maps.\n" );
			}
			BASEMODPANEL_SINGLETON.QueryForQuickPlayMaps();
			CUIGameData::Get()->OpenWaitScreen( "#PORTAL2_EditorMenu_WaitForQuickPlayResults", 1.0f, NULL );
		}
		return;
	}

	if ( V_stricmp( pszEvent, "Command::CommunityCoopStartDownloadingMap" ) )
	{
		bool bCommunityCoop = ( eMode == QUEUEMODE_USER_COOP_QUEUE || eMode == QUEUEMODE_COOP_QUICK_PLAY );
		if ( bCommunityCoop && !V_stricmp( pszEvent, "OnRequestMapRating" ) )
		{
			const PublishedFileInfo_t *pNextMap = BASEMODPANEL_SINGLETON.GetNextCommunityMapInQueueBasedOnQueueMode();

			KeyValues *pCommand = NewCoopCommand( "ActivateRateMapUI", "all" );
			pCommand->SetInt( "has_next_map", pNextMap != NULL );
			GetCoopMatchSession()->Command( pCommand );
			pCommand->deleteThis();

			BroadcastStartDownloadingMap( pNextMap, "RateMapDialog" );
			m_eState = CCM_WAITING_FOR_VOTES;
			return;
		}

		if ( !V_stricmp( pszEvent, "Command::ActivateRateMapUI" ) )
		{
			m_bHasNextMap = pEvent->GetInt( "has_next_map", 0 ) != 0;
			BASEMODPANEL_SINGLETON.CloseAllWindows( 0 );
			engine->ExecuteClientCmd( "gameui_activate" );
			GameUI().PreventEngineHideGameUI();

			KeyValues *pSettings = new KeyValues( "Settings" );
			KeyValues::AutoDelete autodelete( pSettings );
			pSettings->SetInt( "options/allowskiptonextlevel", 1 );
			BASEMODPANEL_SINGLETON.OpenWindow( WT_RATEMAP, NULL, true, pSettings );
			m_eState = CCM_WAITING_FOR_VOTES;
			return;
		}

		if ( !V_stricmp( pszEvent, "Command::CancelSkipMap" ) )
		{
			m_eState = CCM_IDLE;
		}
		return;
	}

	// Command::CommunityCoopStartDownloadingMap
	m_nMapID = pEvent->GetUint64( "PublishedFileID" );
	m_hMapFile = pEvent->GetUint64( "FileHandle" );
	m_hMapPreviewFile = pEvent->GetUint64( "PreviewFileHandle" );
	const char *pszFrom = pEvent->GetString( "from", "" );

	m_eDownloadSource = CCM_DOWNLOAD_FROM_START;
	m_eState = CCM_DOWNLOADING;
	if ( !V_stricmp( pszFrom, "SkipMapDialog" ) )
	{
		m_eDownloadSource = CCM_DOWNLOAD_FROM_SKIP_MAP;
		OpenDownloadWaitScreen();
	}
	else if ( !V_stricmp( pszFrom, "RateMapDialog" ) )
	{
		// The rate dialog stays up until every vote is in.
		m_eDownloadSource = CCM_DOWNLOAD_FROM_RATE_MAP;
		m_eState = CCM_WAITING_FOR_VOTES;
	}
	else
	{
		OpenDownloadWaitScreen();
	}

	if ( cm_community_debug_spew.GetBool() )
	{
		ConColorMsg( rgbaCommunityDebug, "Begin downloading map %llu\n", m_nMapID );
	}

	WorkshopManager().AddFileInfoQuery( new CCommunityMapCoopRequest( m_nMapID, m_hMapFile, m_hMapPreviewFile ), true );
}

#else // !PORTAL2_PUZZLEMAKER || NO_STEAM

// Portal 2 port: the Workshop co-op flow needs Steam and the Puzzle Maker.
// client_portal2.vpc enables PORTAL2_PUZZLEMAKER only for the Windows and macOS
// clients (the retail Linux client has it), and this build defines NO_STEAM.
// Every BaseModUI caller of g_CommunityCoopManager is compiled out with it, so
// this translation unit is empty here.

#endif // PORTAL2_PUZZLEMAKER && !NO_STEAM
