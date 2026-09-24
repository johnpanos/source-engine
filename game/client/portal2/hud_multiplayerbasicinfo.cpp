//========= Portal 2 reconstruction ============================================//
//
// Purpose: Team multiplayer score panel (player score, team score, other team)
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h"
#include "c_playerresource.h"
#include "c_team.h"
#include "portal_mp_gamerules.h"
#include <vgui/ISurface.h>
#include <vgui_controls/EditablePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


//-----------------------------------------------------------------------------
// Purpose: Shows the local player's score, their team's score and the other
//			team's score in non-cooperative multiplayer
//-----------------------------------------------------------------------------
class CHudMultiplayerBasicInfo : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudMultiplayerBasicInfo, vgui::EditablePanel );

public:
	CHudMultiplayerBasicInfo( const char *pElementName );

protected:

	virtual void Reset( void );
	virtual void Init( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
};

DECLARE_HUDELEMENT_DEPTH( CHudMultiplayerBasicInfo, 100 );


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudMultiplayerBasicInfo::CHudMultiplayerBasicInfo( const char *pElementName ) : BaseClass( NULL, "MultiplayerBasicInfo" ), CHudElement( pElementName )
{
	vgui::Panel *pParent = GetClientMode()->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	LoadControlSettings( "resource/multiplayerbasicinfo.res" );
}

void CHudMultiplayerBasicInfo::Reset( void )
{
	Init();
}

void CHudMultiplayerBasicInfo::Init( void )
{
	int screenWide, screenTall;
	GetHudSize( screenWide, screenTall );

	SetSize( 150, 80 );
	SetPos( screenWide - 200, screenTall - 130 );
}

void CHudMultiplayerBasicInfo::ApplySchemeSettings( vgui::IScheme *scheme )
{
	LoadControlSettings( "resource/multiplayerbasicinfo.res" );

	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( true );
	SetPaintBorderEnabled( false );
	SetPaintBackgroundType( 2 );
}

bool CHudMultiplayerBasicInfo::ShouldDraw( void )
{
	// Only for team versus multiplayer
	if ( g_pGameRules && g_pGameRules->IsMultiplayer() )
	{
		return !PortalMPGameRules()->IsCoOp();
	}

	return false;
}

void CHudMultiplayerBasicInfo::Paint( void )
{
	C_BaseHLPlayer *pPlayer = static_cast< C_BaseHLPlayer * >( C_BasePlayer::GetLocalPlayer() );
	if ( !pPlayer )
		return;

	if ( !pPlayer->GetTeam() )
		return;

	Color clrPanelBg;
	if ( pPlayer->GetTeam()->GetTeamNumber() == TEAM_RED )
	{
		clrPanelBg = Color( 255, 75, 20, 128 );
	}
	else if ( pPlayer->GetTeam()->GetTeamNumber() == TEAM_BLUE )
	{
		clrPanelBg = Color( 100, 175, 255, 128 );
	}
	else
	{
		clrPanelBg = Color( 0, 0, 0, 128 );
	}

	SetBgColor( clrPanelBg );

	Color clrText( 255, 255, 255, 255 );

	int iTextY = 10;

	// Our score
	{
		wchar_t text[ 32 ];
		V_snwprintf( text, ARRAYSIZE( text ), L"Your Score: %i", g_PR->GetPlayerScore( pPlayer->entindex() ) );

		vgui::surface()->DrawSetTextFont( m_hTextFont );
		vgui::surface()->DrawSetTextColor( clrText );
		vgui::surface()->DrawSetTextPos( 10, iTextY );
		vgui::surface()->DrawUnicodeString( text );

		iTextY += vgui::surface()->GetFontTall( m_hTextFont ) + 10;
	}

	// Our team's score
	{
		wchar_t text[ 32 ];
		V_snwprintf( text, ARRAYSIZE( text ), L"Team Score: %i", g_PR->GetTeamScore( pPlayer->GetTeamNumber() ) );

		vgui::surface()->DrawSetTextFont( m_hTextFont );
		vgui::surface()->DrawSetTextColor( clrText );
		vgui::surface()->DrawSetTextPos( 10, iTextY );
		vgui::surface()->DrawUnicodeString( text );

		iTextY += vgui::surface()->GetFontTall( m_hTextFont ) + 10;
	}

	// The other team's score, in their color
	{
		int iOtherTeam = ( pPlayer->GetTeamNumber() == TEAM_BLUE ) ? TEAM_RED : TEAM_BLUE;

		wchar_t text[ 32 ];
		V_snwprintf( text, ARRAYSIZE( text ), L"Other Team: %i", g_PR->GetTeamScore( iOtherTeam ) );

		Color clrEnemyTeam = ( iOtherTeam == TEAM_RED ) ? Color( 255, 0, 0, 255 ) : Color( 0, 0, 255, 255 );

		vgui::surface()->DrawSetTextFont( m_hTextFont );
		vgui::surface()->DrawSetTextColor( clrEnemyTeam );
		vgui::surface()->DrawSetTextPos( 10, iTextY );
		vgui::surface()->DrawUnicodeString( text );

		iTextY += vgui::surface()->GetFontTall( m_hTextFont ) + 10;
	}

	BaseClass::Paint();
}
