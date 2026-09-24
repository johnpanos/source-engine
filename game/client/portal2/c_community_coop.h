//========= Portal 2 reconstruction ============================================//
//
// Purpose: Community (Steam Workshop) co-op map flow: exchanges the partners'
//          map histories, starts and tracks the map download on both
//          machines and moves the pair to the next map.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). The retail consumers that inline
// HasNextMap, HasPartnerHistory and Dev_SpewParterHistory are in BaseModUI.
// Not original Valve source; the repository's provenance and distribution
// warning applies.
//
//=============================================================================//

#ifndef C_COMMUNITY_COOP_H
#define C_COMMUNITY_COOP_H
#ifdef _WIN32
#pragma once
#endif

#if defined( PORTAL2_PUZZLEMAKER ) && !defined( NO_STEAM )

#include "igamesystem.h"
#include "matchmaking/imatchevents.h"
#include "steam/isteamremotestorage.h"
#include "utlvector.h"
#include "utlstring.h"

struct PublishedFileInfo_t;

class C_CommunityCoopManager : public CAutoGameSystemPerFrame, public IMatchEventsSink
{
public:
	// m_eState values
	enum CommunityCoopState_t
	{
		CCM_IDLE = 0,
		CCM_DOWNLOADING = 1,
		CCM_WAITING_FOR_VOTES = 2,
		CCM_LEVEL_TRANSITION = 3,
		CCM_WAITING_FOR_FILE_INFO = 5,
		CCM_SENDING_HISTORY = 6,
	};

	// Which dialog asked for the current download
	enum DownloadSource_t
	{
		CCM_DOWNLOAD_FROM_START = 0,
		CCM_DOWNLOAD_FROM_RATE_MAP = 1,
		CCM_DOWNLOAD_FROM_SKIP_MAP = 2,
	};

	struct PartnerHistoryEntry_t
	{
		PublishedFileId_t	m_nPublishedFileID;
		UGCHandle_t			m_hFile;
		UGCHandle_t			m_hPreviewFile;
	};

	C_CommunityCoopManager();
	virtual ~C_CommunityCoopManager();

	// CAutoGameSystemPerFrame
	virtual char const *Name() { return "C_CommunityCoopManager"; }
	virtual void PostInit();
	virtual void Shutdown();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreEntity();
	virtual void Update( float frametime );

	// IMatchEventsSink
	virtual void OnEvent( KeyValues *pEvent );

	// The client joined a community co-op session: send our map history to
	// the host once it has loaded.
	void OnClientReady( void );

	void BroadcastStartDownloadingMap( const PublishedFileInfo_t *pFileInfo, const char *pszFrom );
	void CancelSkipMap( void );

	// "workshop/<file handle>/<map name>", or empty when the map's
	// information is not known yet
	CUtlString GetCommunityMapName( PublishedFileId_t nMapID );

	bool HasNextMap( void ) const { return m_bHasNextMap; }
	bool HasPartnerHistory( void ) const { return m_bHasPartnerHistory; }
	bool IsInPartnerHistoryQueue( PublishedFileId_t nMapID ) const;
	void ClearPartnerHistory( void );

	void Dev_SpewParterHistory( void ) const
	{
		DevMsg( "THEIR HISTORY-----------\n" );
		for ( int i = 0; i < m_PartnerHistory.Count(); ++i )
		{
			DevMsg( "%llu\n", m_PartnerHistory[ i ].m_nPublishedFileID );
		}
		DevMsg( "-------END THEIR HISTORY\n" );
	}

	int GetState( void ) const { return m_eState; }

private:
	void ResetState( void );
	void SendStartCommand( void );
	void AbortToFrontScreen( void );
	void UpdateDownload( void );
	void UpdateVoting( void );
	void UpdateLevelTransition( void );
	void UpdateWaitingForFileInfo( void );

	PublishedFileId_t	m_nMapID;
	UGCHandle_t			m_hMapFile;
	UGCHandle_t			m_hMapPreviewFile;
	bool				m_bHasNextMap;
	bool				m_bHasPartnerHistory;
	float				m_flStateStartTime;
	float				m_flLastProgressTime;
	int					m_eState;
	int					m_eDownloadSource;

	CUtlVector< PartnerHistoryEntry_t > m_PartnerHistory;
};

extern C_CommunityCoopManager g_CommunityCoopManager;

#endif // PORTAL2_PUZZLEMAKER && !NO_STEAM

#endif // C_COMMUNITY_COOP_H
