//========= Portal 2 reconstruction ============================================//
//
// Purpose: Challenge mode HUD (CHUDChallengeStats): portal count and elapsed
//          time for the current attempt, plus the +leaderboard / -leaderboard
//          commands that open the in-game challenge leaderboard.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output) and the portal2_dlc1 resource
// resource/ui/hud_challenge_stats_panel.res. No original source or Steam2
// pseudocode exists for this file. Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include "c_portal_player.h"
#include "portal_gamerules.h"
#include "portal_mp_gamerules.h"
#include "portal2_leaderboard_manager.h"
#include "portal2_engine_compat.h"
#include "gameui_interface.h"
#include "portal2/basemodpanel.h"
#include "portal2/vgenericconfirmation.h"
#include "portal2/vportalleaderboard.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

//-----------------------------------------------------------------------------
// Purpose: Portal count and time readout shown while playing a challenge map
//-----------------------------------------------------------------------------
class CHUDChallengeStats : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHUDChallengeStats, EditablePanel );

public:
	CHUDChallengeStats( const char *pElementName );

	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual bool ShouldDraw( void );

	virtual void OnTick( void );
	virtual void PerformLayout( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	void SetFontColor( Color fontColor ) { m_FontColor = fontColor; }
	void RequestLeaderboard( int nState, bool bCheated );

private:
	bool IsChallengeMap( void );
	void UpdateTime( C_Portal_Player *pPlayer );
	void OpenLeaderboard( void );

	Color m_FontColor;

	vgui::Label *m_pPortalsLabel;
	vgui::Label *m_pTimeLabel;
	vgui::Label *m_pPortalCountLabel;
	vgui::Label *m_pTimeCountLabel;

	// The map whose challenge state was last looked up
	char m_szMapName[128];
	bool m_bIsChallengeMap;

	bool m_bCheated;
	bool m_bLeaderboardPending;
	int m_nLeaderboardState;
};

DECLARE_HUDELEMENT( CHUDChallengeStats );

CHUDChallengeStats::CHUDChallengeStats( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "ChallengeStats" )
{
	m_FontColor = Color( 0, 0, 0, 0 );

	SetParent( GetClientMode()->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD );
	SetScheme( "basemodui_scheme" );

	m_pPortalsLabel = new vgui::Label( this, "PortalsLabel", "" );
	m_pTimeLabel = new vgui::Label( this, "TimeLabel", "" );
	m_pPortalCountLabel = new vgui::Label( this, "PortalCountLabel", "" );
	m_pTimeCountLabel = new vgui::Label( this, "TimeCountLabel", "" );

	V_strncpy( m_szMapName, "", sizeof( m_szMapName ) );
	m_bLeaderboardPending = false;
	m_bIsChallengeMap = false;
	m_bCheated = false;
	m_nLeaderboardState = STATE_PAUSE_MENU;
	m_FontColor = Color( 255, 255, 255, 255 );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 50 );
}

void CHUDChallengeStats::Init( void )
{
	Reset();
}

void CHUDChallengeStats::VidInit( void )
{
	Reset();
}

void CHUDChallengeStats::Reset( void )
{
	vgui::Label *pLabels[] = { m_pPortalsLabel, m_pTimeLabel, m_pPortalCountLabel, m_pTimeCountLabel };
	for ( int i = 0; i < ARRAYSIZE( pLabels ); ++i )
	{
		if ( pLabels[ i ] )
		{
			pLabels[ i ]->SetFgColor( m_FontColor );
			pLabels[ i ]->SetBgColor( Color( 0, 0, 0, 0 ) );
		}
	}
}

bool CHUDChallengeStats::ShouldDraw( void )
{
	if ( !IsChallengeMap() )
		return false;

	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: True when challenge mode is on and the current map is listed in
//          challenge_maplist.txt. The answer is cached per map name.
//-----------------------------------------------------------------------------
bool CHUDChallengeStats::IsChallengeMap( void )
{
	bool bMultiplayer = false;
	if ( g_pGameRules )
	{
		if ( g_pGameRules->IsMultiplayer() )
		{
			if ( PortalMPGameRules() && !PortalMPGameRules()->IsChallengeMode() )
				return false;

			bMultiplayer = true;
		}
		else
		{
			CPortalGameRules *pRules = dynamic_cast< CPortalGameRules * >( g_pGameRules );
			if ( pRules && !pRules->IsChallengeMode() )
				return false;
		}
	}

	const char *pMapName = Portal2Engine::GetLevelNameShort();
	if ( !pMapName )
		return false;

	if ( V_stricmp( pMapName, m_szMapName ) != 0 )
	{
		V_strncpy( m_szMapName, pMapName, sizeof( m_szMapName ) );
		m_bIsChallengeMap = false;

		int nChapter = BASEMODPANEL_SINGLETON.MapNameToChapter( pMapName, !bMultiplayer );
		KeyValues *pChallengeMaps = PortalLeaderboardManager()->GetChallengeMapsFromChapter( nChapter, bMultiplayer );
		if ( !pChallengeMaps )
			return false;

		for ( KeyValues *pMap = pChallengeMaps->GetFirstSubKey(); pMap; pMap = pMap->GetNextKey() )
		{
			if ( !V_stricmp( pMapName, pMap->GetString() ) )
			{
				m_bIsChallengeMap = true;
				return true;
			}
		}
	}

	return m_bIsChallengeMap;
}

void CHUDChallengeStats::UpdateTime( C_Portal_Player *pPlayer )
{
	float flSeconds = pPlayer->m_StatsThisLevel.fNumSecondsTaken;
	int nSeconds = (int)flSeconds;

	char szTime[32];
	if ( nSeconds < 600 )
	{
		V_snprintf( szTime, sizeof( szTime ), "%02d:%02d.%02d", nSeconds / 60, nSeconds % 60, (int)( ( flSeconds - (float)nSeconds ) * 100.0f ) );
	}
	else
	{
		V_snprintf( szTime, sizeof( szTime ), "%02d:%02d", nSeconds / 60, nSeconds % 60 );
	}

	m_pTimeCountLabel->SetText( szTime );
}

void CHUDChallengeStats::OnTick( void )
{
	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPortalPlayer();
	if ( pPlayer )
	{
		int nPortals = PortalMPGameRules() ? PortalMPGameRules()->GetNumPortalsPlaced() : pPlayer->m_StatsThisLevel.iNumPortalsPlaced.Get();

		char szPortals[32];
		V_snprintf( szPortals, sizeof( szPortals ), "%d", nPortals );
		m_pPortalCountLabel->SetText( szPortals );

		UpdateTime( pPlayer );
	}

	// Wait for any save in progress before taking over the game UI.
	if ( m_bLeaderboardPending && !engine->IsSaveInProgress() )
	{
		m_bLeaderboardPending = false;
		OpenLeaderboard();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Opens the challenge leaderboard over the in-game menu. A pending
//          confirmation or wait screen is dismissed first and the request is
//          retried on the next tick.
//-----------------------------------------------------------------------------
void CHUDChallengeStats::OpenLeaderboard( void )
{
	CBaseModFrame *pConfirmation = BASEMODPANEL_SINGLETON.GetWindow( WT_GENERICCONFIRMATION );
	CBaseModFrame *pWaitScreen = BASEMODPANEL_SINGLETON.GetWindow( WT_GENERICWAITSCREEN );
	if ( pConfirmation || pWaitScreen )
	{
		if ( pConfirmation )
		{
			const char *pTitle = static_cast< GenericConfirmation * >( pConfirmation )->GetUsageData().pWindowTitle;
			if ( pTitle && ( !V_strcmp( pTitle, "#PORTAL2_ChallengeMode_Cheat_Header" ) ||
				!V_strcmp( pTitle, "#L4D360UI_LeaveMultiplayerConf" ) ||
				!V_strcmp( pTitle, "#Portal2UI_GoToHubQ" ) ) )
			{
				pConfirmation->Close();
			}

			GameUI().AllowEngineHideGameUI();
			GameUI().HideGameUI();
			BASEMODPANEL_SINGLETON.CloseAllWindows( 0 );
		}

		m_bLeaderboardPending = true;
		return;
	}

	WINDOW_TYPE eLeaderboardWindow;
	const char *pStateKey;
	if ( IsChallengeMap() )
	{
		eLeaderboardWindow = WT_PORTALLEADERBOARDHUD;
		pStateKey = "LevelState";
	}
	else
	{
		if ( !V_stricmp( Portal2Engine::GetLevelNameShort(), "mp_coop_lobby_3" ) )
			return;

		// Portal 2 port: retail dereferences the co-op rules here without a
		// check; single player has none, so there is no leaderboard to open.
		if ( !PortalMPGameRules() || !PortalMPGameRules()->IsChallengeMode() )
			return;

		eLeaderboardWindow = WT_PORTALCOOPLEADERBOARD;
		pStateKey = "state";
	}

	engine->ExecuteClientCmd( "gameui_activate" );
	CBaseModFrame *pInGameMenu = BASEMODPANEL_SINGLETON.OpenWindow( WT_INGAMEMAINMENU, NULL, true );
	GameUI().PreventEngineHideGameUI();

	KeyValues *pLeaderboardValues = new KeyValues( "leaderboard" );
	KeyValues::AutoDelete autodelete( pLeaderboardValues );
	pLeaderboardValues->SetInt( pStateKey, m_nLeaderboardState );
	pLeaderboardValues->SetBool( "Cheated", m_bCheated );
	BASEMODPANEL_SINGLETON.OpenWindow( eLeaderboardWindow, pInGameMenu, true, pLeaderboardValues );
}

void CHUDChallengeStats::RequestLeaderboard( int nState, bool bCheated )
{
	m_nLeaderboardState = nState;
	m_bLeaderboardPending = true;
	m_bCheated = bCheated;
}

void CHUDChallengeStats::PerformLayout( void )
{
	BaseClass::PerformLayout();
}

void CHUDChallengeStats::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "resource/ui/hud_challenge_stats_panel.res" );
	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Commands
//-----------------------------------------------------------------------------
static void CC_HudSetChallengeFontColor( const CCommand &args )
{
	if ( args.ArgC() != 4 )
	{
		DevMsg( "hud_set_challenge_font_color r g b" );
		return;
	}

	int r = atoi( args.Arg( 1 ) );
	int g = atoi( args.Arg( 2 ) );
	int b = atoi( args.Arg( 3 ) );

	CHUDChallengeStats *pStats = GET_HUDELEMENT( CHUDChallengeStats );
	if ( pStats )
	{
		pStats->SetFontColor( Color( r, g, b, 255 ) );
		pStats->Reset();
	}
}
static ConCommand hud_set_challenge_font_color( "hud_set_challenge_font_color", CC_HudSetChallengeFontColor, "Set a new font color for challenge stats." );

// +leaderboard [state] [cheated]: state is a LeaderboardState_t (values 0-4;
// others, or no argument, become STATE_TRIGGERED). Only split-screen slot 0
// opens it.
static void CC_ShowLeaderboard( const CCommand &args )
{
	if ( Portal2Engine::GetActiveSplitScreenPlayerSlot() != 0 )
		return;

	int nState = STATE_TRIGGERED;
	bool bCheated = false;
	if ( args.ArgC() >= 2 )
	{
		unsigned int nRequested = (unsigned int)atoi( args.Arg( 1 ) );
		bCheated = atoi( args.Arg( 2 ) ) == 1;
		if ( nRequested < 5 )
			nState = (int)nRequested;
	}

	if ( nState != STATE_END_OF_LEVEL )
	{
		CPortalLeaderboardPanel::ResetTempScoreUpdates();
	}

	CHUDChallengeStats *pStats = GET_HUDELEMENT( CHUDChallengeStats );
	if ( pStats )
	{
		pStats->RequestLeaderboard( nState, bCheated );
	}
}
static ConCommand startleaderboard( "+leaderboard", CC_ShowLeaderboard, "Display in game leaderboard", FCVAR_SERVER_CAN_EXECUTE );

static void CC_HideLeaderboard( const CCommand &args )
{
	GameUI().AllowEngineHideGameUI();
	engine->ExecuteClientCmd( "gameui_hide" );
	BASEMODPANEL_SINGLETON.CloseAllWindows( 0 );
}
static ConCommand endleaderboard( "-leaderboard", CC_HideLeaderboard, "Hide in game leaderboard", FCVAR_SERVER_CAN_EXECUTE );
