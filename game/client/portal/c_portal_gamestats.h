//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side Portal 2 gamestats (configuration and survey data).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_PORTAL_GAMESTATS_H
#define C_PORTAL_GAMESTATS_H
#ifdef _WIN32
#pragma once
#endif

// Steam gamestats are not tracked on consoles or in builds without Steam; the
// consumers guard their calls the same way.
#if !defined( _GAMECONSOLE ) && !defined( NO_STEAM )

#include "gamestats.h"
#include "GameEventListener.h"
#include "igamesystem.h"
#include "portal_gamestats_shared.h"
#include "matchmaking/imatchframework.h"

struct CPortal_StuckPlayer;

typedef CUtlVector< CPortal_StuckPlayer* > VectorStuckPlayer;

//-----------------------------------------------------------------------------
// Purpose: Client configuration, reported once per session
//-----------------------------------------------------------------------------
struct SConfigStats : public BaseStatData
{
	SConfigStats()
	{
		m_TextLanguage[0] = '\0';
		m_AudioLanguage[0] = '\0';
		m_CountryCode[0] = '\0';
		m_ReverseMouse = 0;
		m_AllowConsole = 0;
		m_AllowGamepad = 0;
		m_OpenMic = 0;
		m_ColorBlind = 0;
		m_Captions = 0;
		m_LowViolence = 0;
	}

	char	m_TextLanguage[64];
	char	m_AudioLanguage[64];
	char	m_CountryCode[64];
	uint8	m_ReverseMouse;
	uint8	m_AllowConsole;
	uint8	m_AllowGamepad;
	uint8	m_OpenMic;
	uint8	m_ColorBlind;
	uint8	m_Captions;
	uint8	m_LowViolence;

	BEGIN_STAT_TABLE( "P2ConfigData" )
		REGISTER_STAT_STRING( m_TextLanguage )
		REGISTER_STAT_STRING( m_AudioLanguage )
		REGISTER_STAT_STRING( m_CountryCode )
		REGISTER_STAT( m_ReverseMouse )
		REGISTER_STAT( m_AllowConsole )
		REGISTER_STAT( m_AllowGamepad )
		REGISTER_STAT( m_OpenMic )
		REGISTER_STAT( m_ColorBlind )
		REGISTER_STAT( m_Captions )
		REGISTER_STAT( m_LowViolence )
	END_STAT_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: Answers to the standard playtest survey
//-----------------------------------------------------------------------------
struct SSurveyData : public BaseStatData
{
	SSurveyData( uint32 difficulty, uint32 enjoyment, const char *essay )
	{
		m_Difficulty = difficulty;
		m_Enjoyment = enjoyment;
		V_memset( m_Essay, 0, sizeof( m_Essay ) );
		if ( essay )
		{
			V_strncpy( m_Essay, essay, sizeof( m_Essay ) - 1 );
		}
	}

	uint32	m_Difficulty;
	uint32	m_Enjoyment;
	char	m_Essay[4097];

	BEGIN_STAT_TABLE( "P2SurveyData" )
		REGISTER_STAT( m_Difficulty )
		REGISTER_STAT( m_Enjoyment )
		REGISTER_STAT_STRING( m_Essay )
	END_STAT_TABLE()
};

typedef CUtlVector< SSurveyData* > VectorSurveyData;

// Reconstruction note: retail-only record. uigamedata_storage.cpp reports UI
// actions through C_CPortalGameStats::Event_UIEvent, which neither 2010 build
// has; the record follows the SSurveyData pattern with the reported fields.
struct SUIEventData : public BaseStatData
{
	SUIEventData( uint64 xuid, const char *pszAction, uint64 uiFlags, const char *pszMapName )
	{
		m_XUID = xuid;
		m_Flags = uiFlags;
		V_strncpy( m_Action, pszAction ? pszAction : "", sizeof( m_Action ) );
		V_strncpy( m_MapName, pszMapName ? pszMapName : "", sizeof( m_MapName ) );
	}

	uint64	m_XUID;
	uint64	m_Flags;
	char	m_Action[64];
	char	m_MapName[64];

	BEGIN_STAT_TABLE( "P2UIEventData" )
		REGISTER_STAT( m_XUID )
		REGISTER_STAT( m_Flags )
		REGISTER_STAT_STRING( m_Action )
		REGISTER_STAT_STRING( m_MapName )
	END_STAT_TABLE()
};

//-----------------------------------------------------------------------------
// Purpose: Client gamestats tracker
//-----------------------------------------------------------------------------
class C_CPortalGameStats : public CBaseGameStats, public CGameEventListener, public CAutoGameSystem, public IGameStatTracker, public IMatchEventsSink
{
	typedef CBaseGameStats BaseClass;

public:
	C_CPortalGameStats()
	{
		m_bSessionActive = false;
	}

	virtual ~C_CPortalGameStats()
	{
	}

	// IMatchEventsSink
	virtual void OnEvent( KeyValues *pEvent );

	// CBaseGameStats
	virtual void Event_LevelInit( void );
	virtual void Event_LevelShutdown( float flElapsed ) {}
	virtual bool ShouldSendDataOnLevelShutdown( void ) { return true; }
	virtual bool ShouldSendDataOnAppShutdown( void ) { return true; }
	virtual void FireGameEvent( IGameEvent *event ) {}
	virtual bool UseOldFormat( void ) { return false; }
	virtual bool AddDataForSend( KeyValues *pKV, StatSendType_t sendType );
	virtual void SubmitGameStats( KeyValues *pKV );
	void AddData( void );
	virtual void OnSessionStart( void );
	virtual void OnSessionEnd( void );

	virtual bool Init( void );
	virtual void Shutdown( void );
	virtual void Event_AddSurveyData( uint32 difficulty, uint32 enjoyment, const char *essay );
	virtual void Event_UIEvent( uint64 xuid, const char *szReportAction, uint64 uiFlags, const char *szMapName );

	virtual StatContainerList_t *GetStatContainerList( void ) { return s_StatLists; }

private:
	// Collects the client's configuration into m_ConfigData and queues it
	void GetConfigData( void );

	static StatContainerList_t *s_StatLists;

	VectorStuckPlayer	m_VectorStuckPlayer;
	SConfigStats		m_ConfigData;
	VectorSurveyData	m_SurveyData;
	bool				m_bSessionActive;
};

extern C_CPortalGameStats g_PortalGameStats;

#endif // !_GAMECONSOLE && !NO_STEAM

#endif // C_PORTAL_GAMESTATS_H
