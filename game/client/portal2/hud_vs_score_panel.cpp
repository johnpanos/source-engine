//========= Portal 2 reconstruction ============================================//
//
// Purpose: Co-op versus score HUD (CHUDVSScorePanel): team scores, rounds,
//          round clock and the start-of-round countdown.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). No original source or Steam2
// pseudocode exists for this file. The retail VPKs ship no
// resource/ui/hud_vs_score_panel.res. C_InfoPortalScore drives the panel
// (c_info_portal_score.cpp); the winner, round-over and countdown methods have
// no callers in the retail binary. Not original Valve source; the
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
#include "hud_vs_score_panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar hud_score_panel_flash_time( "hud_score_panel_flash_time", "0.5f", FCVAR_DEVELOPMENTONLY, "The duration for each flash of the timer label." );
ConVar hud_score_panel_timer_base_color( "hud_score_panel_timer_base_color", "255 255 255 255", FCVAR_DEVELOPMENTONLY, "The color for the timer label when not flashing." );
ConVar hud_score_panel_timer_flash_color( "hud_score_panel_timer_flash_color", "255 0 0 255", FCVAR_DEVELOPMENTONLY, "The color for the timer label when it flashes towards the end." );

static const Color s_BlueTeamColor( 0, 128, 255, 255 );
static const Color s_OrangeTeamColor( 240, 110, 0, 255 );

// Portal 2 port: CS:GO-era ConVar::GetColor() is not in this tier1.
static Color GetConVarColor( const ConVar &var )
{
	int r = 255, g = 255, b = 255, a = 255;
	sscanf( var.GetString(), "%d %d %d %d", &r, &g, &b, &a );
	return Color( r, g, b, a );
}

DECLARE_HUDELEMENT( CHUDVSScorePanel );

CHUDVSScorePanel::CHUDVSScorePanel( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "VSScorePanel" )
{
	m_flRoundTime = 0.0f;
	m_flTimeRemaining = 0.0f;
	m_flLastTickTime = 0.0f;
	m_flLastFlashTime = 0.0f;
	m_flCountdownStartTime = 0.0f;
	m_bFlashOn = false;
	m_bCountingDown = false;
	m_nRoundsToWin = 0;
	m_bShowPanel = false;

	SetParent( GetClientMode()->GetViewport() );

	m_pBlueTeamLabel = new vgui::Label( this, "BlueTeamLabel", "" );
	m_pOrangeTeamLabel = new vgui::Label( this, "OrangeTeamLabel", "" );
	m_pBlueScoreLabel = new vgui::Label( this, "BlueScoreLabel", "" );
	m_pOrangeScoreLabel = new vgui::Label( this, "OrangeScoreLabel", "" );
	m_pBlueRoundsLabel = new vgui::Label( this, "BlueRoundsLabel", "" );
	m_pOrangeRoundsLabel = new vgui::Label( this, "OrangeRoundsLabel", "" );
	m_pWinnerLabel = new vgui::Label( this, "WinnerLabel", "" );
	m_pTimerLabel = new vgui::Label( this, "TimerLabel", "" );
	m_pGameDescriptionLabel = new vgui::Label( this, "GameDescriptionLabel", "" );
	m_pCountdownLabel = new vgui::Label( this, "CountdownLabel", "" );
}

bool CHUDVSScorePanel::ShouldDraw( void )
{
	return m_bShowPanel;
}

void CHUDVSScorePanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "resource/ui/hud_vs_score_panel.res" );
}

void CHUDVSScorePanel::Paint( void )
{
	BaseClass::Paint();
}

void CHUDVSScorePanel::PerformLayout( void )
{
	BaseClass::PerformLayout();

	m_pBlueTeamLabel->SetFgColor( s_BlueTeamColor );
	m_pBlueScoreLabel->SetFgColor( s_BlueTeamColor );
	m_pBlueRoundsLabel->SetFgColor( s_BlueTeamColor );
	m_pOrangeTeamLabel->SetFgColor( s_OrangeTeamColor );
	m_pOrangeScoreLabel->SetFgColor( s_OrangeTeamColor );
	m_pOrangeRoundsLabel->SetFgColor( s_OrangeTeamColor );

	// Centered text spans the whole screen width.
	vgui::Label *pWideLabels[] = { m_pGameDescriptionLabel, m_pCountdownLabel, m_pWinnerLabel };
	for ( int i = 0; i < ARRAYSIZE( pWideLabels ); ++i )
	{
		int x, y;
		pWideLabels[ i ]->GetPos( x, y );
		pWideLabels[ i ]->SetPos( 0, y );
		pWideLabels[ i ]->SetWide( ScreenWidth() );
	}

	ResetPanel();
}

//-----------------------------------------------------------------------------
// Purpose: Hides every label and clears scores and the clock
//-----------------------------------------------------------------------------
void CHUDVSScorePanel::ResetPanel( void )
{
	m_pBlueTeamLabel->SetVisible( false );
	m_pBlueScoreLabel->SetVisible( false );
	m_pBlueRoundsLabel->SetVisible( false );
	m_pOrangeTeamLabel->SetVisible( false );
	m_pOrangeScoreLabel->SetVisible( false );
	m_pOrangeRoundsLabel->SetVisible( false );
	m_pTimerLabel->SetVisible( false );
	m_pWinnerLabel->SetVisible( false );
	m_pGameDescriptionLabel->SetVisible( false );
	m_pCountdownLabel->SetVisible( false );

	SetScores( 0, 0, 0, 0 );

	m_bFlashOn = false;
	m_bCountingDown = false;
	m_flTimeRemaining = 0.0f;
	m_flLastTickTime = 0.0f;
	m_flLastFlashTime = 0.0f;
	m_pTimerLabel->SetFgColor( GetConVarColor( hud_score_panel_timer_base_color ) );
}

void CHUDVSScorePanel::ShowScoreLabels( void )
{
	m_pBlueTeamLabel->SetVisible( true );
	m_pBlueScoreLabel->SetVisible( true );
	m_pBlueRoundsLabel->SetVisible( true );
	m_pOrangeTeamLabel->SetVisible( true );
	m_pOrangeScoreLabel->SetVisible( true );
	m_pOrangeRoundsLabel->SetVisible( true );
	m_pTimerLabel->SetVisible( true );
}

void CHUDVSScorePanel::SetShowPanel( bool bShow )
{
	m_bShowPanel = bShow;
	if ( bShow )
	{
		ResetPanel();
	}
	else
	{
		vgui::ivgui()->RemoveTickSignal( GetVPanel() );
	}
}

void CHUDVSScorePanel::SetGameInfo( float flRoundTime, const char *pDescription, int nRoundsToWin )
{
	m_flTimeRemaining = flRoundTime;
	m_flRoundTime = flRoundTime;
	UpdateTimerLabel();
	m_pGameDescriptionLabel->SetText( pDescription );
	m_nRoundsToWin = nRoundsToWin;
}

// Rounds are drawn as one "O" per round won and one "X" per remaining round.
static void BuildRoundsString( char *pBuffer, int nBufferSize, int nRoundsWon, int nRoundsToWin )
{
	V_snprintf( pBuffer, nBufferSize, "%i", nRoundsWon );
	for ( int i = 0; i < nRoundsToWin; ++i )
	{
		V_strncat( pBuffer, ( i < nRoundsWon ) ? "O" : "X", nBufferSize, COPY_ALL_CHARACTERS );
	}
}

void CHUDVSScorePanel::SetScores( int nBlueScore, int nOrangeScore, int nBlueRounds, int nOrangeRounds )
{
	char szText[32];
	V_snprintf( szText, sizeof( szText ), "Points: %d", nBlueScore );
	m_pBlueScoreLabel->SetText( szText );
	V_snprintf( szText, sizeof( szText ), "Points: %d", nOrangeScore );
	m_pOrangeScoreLabel->SetText( szText );

	BuildRoundsString( szText, sizeof( szText ), nBlueRounds, m_nRoundsToWin );
	m_pBlueRoundsLabel->SetText( szText );
	BuildRoundsString( szText, sizeof( szText ), nOrangeRounds, m_nRoundsToWin );
	m_pOrangeRoundsLabel->SetText( szText );
}

void CHUDVSScorePanel::SetWinner( int nWinner )
{
	if ( nWinner == VS_WINNER_ORANGE )
	{
		m_pWinnerLabel->SetText( "ORANGE WINS!!" );
		m_pWinnerLabel->SetFgColor( s_OrangeTeamColor );
	}
	else if ( nWinner == VS_WINNER_TIE )
	{
		m_pWinnerLabel->SetText( "TIE GAME" );
		m_pWinnerLabel->SetFgColor( Color( 255, 255, 255, 255 ) );
	}
	else if ( nWinner == VS_WINNER_BLUE )
	{
		m_pWinnerLabel->SetText( "BLUE WINS!!" );
		m_pWinnerLabel->SetFgColor( s_BlueTeamColor );
	}

	m_pWinnerLabel->SetVisible( true );
}

void CHUDVSScorePanel::StartRound( bool bCountdown )
{
	m_bCountingDown = bCountdown;
	vgui::ivgui()->AddTickSignal( GetVPanel(), 10 );

	m_flCountdownStartTime = gpGlobals->curtime;
	m_flLastTickTime = gpGlobals->curtime;
	m_flTimeRemaining = m_flRoundTime;

	m_pWinnerLabel->SetVisible( false );
	m_pGameDescriptionLabel->SetVisible( true );
	m_pCountdownLabel->SetText( "Game starts in..." );

	if ( m_bCountingDown )
	{
		m_pCountdownLabel->SetVisible( true );
	}
	else
	{
		ShowScoreLabels();
	}
}

void CHUDVSScorePanel::RoundOver( int nWinner )
{
	m_flTimeRemaining = 0.0f;
	m_flLastTickTime = 0.0f;
	m_flLastFlashTime = 0.0f;

	char szTime[32];
	V_snprintf( szTime, sizeof( szTime ), "%02d:%02d:%02d", 0, 0, 0 );
	m_pTimerLabel->SetText( szTime );

	if ( nWinner == VS_WINNER_BLUE )
	{
		m_pWinnerLabel->SetText( "Blue wins the round" );
		m_pWinnerLabel->SetFgColor( s_BlueTeamColor );
	}
	else
	{
		m_pWinnerLabel->SetText( "Orange wins the round" );
		m_pWinnerLabel->SetFgColor( s_OrangeTeamColor );
	}

	m_pWinnerLabel->SetVisible( true );
}

void CHUDVSScorePanel::UpdateTimerLabel( void )
{
	int nSeconds = (int)m_flTimeRemaining;
	char szTime[32];
	V_snprintf( szTime, sizeof( szTime ), "%02d:%02d:%02d", nSeconds / 60, nSeconds % 60, (int)( ( m_flTimeRemaining - (float)nSeconds ) * 100.0f ) );
	m_pTimerLabel->SetText( szTime );
}

void CHUDVSScorePanel::OnTick( void )
{
	float flCurTime = gpGlobals->curtime;
	float flLastTickTime = m_flLastTickTime;
	m_flLastTickTime = flCurTime;

	float flCountdownElapsed = flCurTime - m_flCountdownStartTime;
	if ( m_bCountingDown )
	{
		if ( flCountdownElapsed <= 4.0f )
		{
			// "3", "2", "1" during the four second countdown
			const char *pCount = NULL;
			if ( flCountdownElapsed > 3.0f )
				pCount = "1";
			else if ( flCountdownElapsed > 2.0f )
				pCount = "2";
			else if ( flCountdownElapsed > 1.0f )
				pCount = "3";

			if ( pCount )
				m_pCountdownLabel->SetText( pCount );

			UpdateTimerLabel();
			return;
		}

		m_bCountingDown = false;
		m_pCountdownLabel->SetVisible( false );
		m_pGameDescriptionLabel->SetVisible( false );
		ShowScoreLabels();
		m_pGameDescriptionLabel->SetVisible( false );
	}
	else if ( flCountdownElapsed > 4.0f )
	{
		m_pGameDescriptionLabel->SetVisible( false );
	}

	m_flTimeRemaining = ( flLastTickTime + m_flTimeRemaining ) - flCurTime;
	if ( m_flTimeRemaining <= 0.0f )
	{
		m_pTimerLabel->SetFgColor( GetConVarColor( hud_score_panel_timer_base_color ) );
		m_flTimeRemaining = 0.0f;
		vgui::ivgui()->RemoveTickSignal( GetVPanel() );
		UpdateTimerLabel();
		return;
	}

	// Flash the clock during the last ten seconds.
	if ( m_flTimeRemaining < 10.0f )
	{
		if ( gpGlobals->curtime - m_flLastFlashTime > hud_score_panel_flash_time.GetFloat() )
		{
			m_bFlashOn = !m_bFlashOn;
			m_flLastFlashTime = gpGlobals->curtime;
		}

		m_pTimerLabel->SetFgColor( GetConVarColor( m_bFlashOn ? hud_score_panel_timer_flash_color : hud_score_panel_timer_base_color ) );
	}

	UpdateTimerLabel();
}
