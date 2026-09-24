//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side Portal 2 gamestats (configuration and survey data).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_portal_gamestats.h"

#if !defined( _GAMECONSOLE ) && !defined( NO_STEAM )
#include "steam/steam_api.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if !defined( _GAMECONSOLE ) && !defined( NO_STEAM )

C_CPortalGameStats g_PortalGameStats;
IGameStatTracker::StatContainerList_t *C_CPortalGameStats::s_StatLists = new IGameStatTracker::StatContainerList_t();

//-----------------------------------------------------------------------------
// Purpose: Listen for match session events and become the active gamestats
//-----------------------------------------------------------------------------
bool C_CPortalGameStats::Init( void )
{
	if ( g_pMatchFramework && g_pMatchFramework->GetEventsSubscription() )
	{
		g_pMatchFramework->GetEventsSubscription()->Subscribe( this );
	}

	gamestats = this;

	return true;
}

void C_CPortalGameStats::Shutdown( void )
{
	if ( g_pMatchFramework && g_pMatchFramework->GetEventsSubscription() )
	{
		g_pMatchFramework->GetEventsSubscription()->Unsubscribe( this );
	}
}

void C_CPortalGameStats::Event_AddSurveyData( uint32 difficulty, uint32 enjoyment, const char *essay )
{
	m_SurveyData.AddToTail( new SSurveyData( difficulty, enjoyment, essay ) );
}

// Reconstruction note: retail-only (see SUIEventData in c_portal_gamestats.h).
void C_CPortalGameStats::Event_UIEvent( uint64 xuid, const char *szReportAction, uint64 uiFlags, const char *szMapName )
{
	SubmitStat( new SUIEventData( xuid, szReportAction, uiFlags, szMapName ) );
}

//-----------------------------------------------------------------------------
// Purpose: Send every tracked stat table and clear it
//-----------------------------------------------------------------------------
void C_CPortalGameStats::SubmitGameStats( KeyValues *pKV )
{
	int listCount = s_StatLists->Count();
	for ( int i = 0; i < listCount; ++i )
	{
		// Create a master key value that has stats everybody should share (map name, session ID, etc)
		(*s_StatLists)[i]->SendData( pKV );
		(*s_StatLists)[i]->Clear();
	}
}

bool C_CPortalGameStats::AddDataForSend( KeyValues *pKV, StatSendType_t sendType )
{
	AddData();

	if ( sendType == STATSEND_APPSHUTDOWN )
	{
		OnSessionEnd();
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Queue the accumulated survey data and hand everything to Steamworks
//-----------------------------------------------------------------------------
void C_CPortalGameStats::AddData( void )
{
	KeyValues *pKV = new KeyValues( "basedata" );
	if ( !pKV )
		return;

	const char *pzMapName = MapName();
	if ( !pzMapName )
	{
		pzMapName = "";
	}

	pKV->SetString( "MapID", pzMapName );

	for ( int k = 0; k < m_SurveyData.Count(); ++k )
	{
		SubmitStat( m_SurveyData[k] );
	}

	SubmitGameStats( pKV );

	// The stat lists own the submitted records now
	m_SurveyData.Purge();
	m_VectorStuckPlayer.Purge();

	pKV->deleteThis();
}

void C_CPortalGameStats::OnSessionStart( void )
{
	m_bSessionActive = true;
	GetConfigData();
	GetSteamWorksSGameStatsUploader().InitializeSystem();
}

void C_CPortalGameStats::OnSessionEnd( void )
{
	m_bSessionActive = false;
	GetSteamWorksSGameStatsUploader().SetExitStatus( k_EGameStatsExitDefaultValue );
	GetSteamWorksSGameStatsUploader().SendSessionStats();
}

//-----------------------------------------------------------------------------
// Purpose: A session covers real maps; menu background maps end it
//-----------------------------------------------------------------------------
void C_CPortalGameStats::Event_LevelInit( void )
{
	const char *name1 = V_stristr( MapName(), "background" );
	if ( !name1 && !m_bSessionActive )
	{
		OnSessionStart();
	}
	else if ( m_bSessionActive )
	{
		OnSessionEnd();
	}
}

void C_CPortalGameStats::GetConfigData( void )
{
	extern ConVar in_joystick;
	if ( in_joystick.GetInt() )
	{
		m_ConfigData.m_AllowGamepad = true;
	}

	if ( engine && steamapicontext && steamapicontext->SteamApps() && steamapicontext->SteamUtils() )
	{
		const char *countryCode = steamapicontext->SteamUtils()->GetIPCountry();
		if ( countryCode )
		{
			V_strncpy( m_ConfigData.m_CountryCode, countryCode, sizeof( m_ConfigData.m_CountryCode ) );
		}

		engine->GetUILanguage( m_ConfigData.m_TextLanguage, sizeof( m_ConfigData.m_TextLanguage ) );

		const char *audioLanguage = steamapicontext->SteamApps()->GetCurrentGameLanguage();
		if ( audioLanguage )
		{
			V_strncpy( m_ConfigData.m_AudioLanguage, audioLanguage, sizeof( m_ConfigData.m_AudioLanguage ) );
		}
		else
		{
			V_strncpy( m_ConfigData.m_AudioLanguage, "unknown", sizeof( m_ConfigData.m_AudioLanguage ) );
		}
	}
	else
	{
		V_strncpy( m_ConfigData.m_AudioLanguage, "unknown", sizeof( m_ConfigData.m_AudioLanguage ) );
		V_strncpy( m_ConfigData.m_TextLanguage, "unknown", sizeof( m_ConfigData.m_TextLanguage ) );
	}

	static ConVarRef m_pitch( "m_pitch" );
	static ConVarRef con_enable( "con_enable" );
	// Reconstruction note: both 2010 builds look up closecaption but never
	// read it, so m_Captions is always reported as 0.
	static ConVarRef closecaption( "closecaption" );
	static ConVarRef cl_colorblind( "cl_colorblind" );
	static ConVarRef voice_vox( "voice_vox" );

	if ( m_pitch.IsValid() && m_pitch.GetFloat() < 0.0f )
	{
		m_ConfigData.m_ReverseMouse = true;
	}

	if ( con_enable.IsValid() && con_enable.GetInt() )
	{
		m_ConfigData.m_AllowConsole = true;
	}

	if ( cl_colorblind.IsValid() )
	{
		m_ConfigData.m_ColorBlind = cl_colorblind.GetInt();
	}

	if ( voice_vox.IsValid() )
	{
		m_ConfigData.m_OpenMic = voice_vox.GetInt();
	}

	m_ConfigData.m_LowViolence = UTIL_IsLowViolence();

	SubmitStat( m_ConfigData );
}

//-----------------------------------------------------------------------------
// Purpose: Match framework events
//-----------------------------------------------------------------------------
void C_CPortalGameStats::OnEvent( KeyValues *pEvent )
{
	// Reconstruction note: both 2010 builds only compare the event name against
	// these three events and take no action for any of them.
	const char *szEvent = pEvent->GetName();
	if ( !V_stricmp( "OnEngineDisconnectReason", szEvent ) )
	{
	}
	else if ( !V_stricmp( "OnEngineEndGame", szEvent ) )
	{
	}
	else if ( !V_stricmp( "OnEngineLevelLoadingStarted", szEvent ) )
	{
	}
}

#endif // !_GAMECONSOLE && !NO_STEAM
