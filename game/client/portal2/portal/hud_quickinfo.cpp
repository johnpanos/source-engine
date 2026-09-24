//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal 2 crosshair (the portal gun brackets and the centre reticle).
//
// Portal 2 reconstruction: retail builds this element from
// game/client/portal/hud_quickinfo.cpp, but that path holds the Portal 1
// version here, which needs the "crosshair" icon that Portal 2's
// hud_textures.txt comments out, so it never drew. Behavior follows the
// current retail linux32 client.so (decompiled CHUDQuickInfo::ShouldDraw and
// Paint); the Steam2 depot 852_3 client.dylib (DWARF) confirms the member and
// ConVar names. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
// Differences from retail:
// - Retail also swaps the brackets when C_Paint_Input reports swapped gun
//   buttons for the split-screen slot; this input has no such state.
// - Retail's third-person test is GetPlayerRenderMode(); the port stubs it,
//   so the local first-person view test is used instead.
// - Stereo apparent depth for the reticle is not supported by this surface.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "ivieweffects.h"
#include "ivrenderview.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "c_portal_player.h"
#include "c_weapon_portalgun.h"
#include "portal_util_shared.h"
#include "portal2/radialmenu.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_quickinfo( "hud_quickinfo", "1", FCVAR_ARCHIVE );
static ConVar hud_quickinfo_swap( "hud_quickinfo_swap", "0", FCVAR_ARCHIVE );

extern ConVar crosshair;

using namespace vgui;

class CHUDQuickInfo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDQuickInfo, vgui::Panel );

public:
	CHUDQuickInfo( const char *pElementName );
	virtual void VidInit();
	virtual bool ShouldDraw();
	virtual void Paint();
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	void DrawReticle( const Color &clr );
	void DrawBracket( CHudTexture *pIcon, int x, int y, const Color &clr );

	CHudTexture *m_icon_rbn; // right bracket, empty
	CHudTexture *m_icon_lbn; // left bracket, empty
	CHudTexture *m_icon_rb;  // right bracket, full
	CHudTexture *m_icon_lb;  // left bracket, full

	int m_nReticleSpread;  // distance of the four outer reticle dots
	int m_nBracketOffsetX; // bracket centre distance from the screen centre
	int m_nBracketOffsetY;
	float m_flBracketScale;
};

DECLARE_HUDELEMENT( CHUDQuickInfo );

CHUDQuickInfo::CHUDQuickInfo( const char *pElementName )
    : CHudElement( pElementName ), BaseClass( NULL, "HUDQuickInfo" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_CROSSHAIR );

	m_icon_rbn = m_icon_lbn = m_icon_rb = m_icon_lb = NULL;
	m_nReticleSpread = 10;
	m_nBracketOffsetX = 6;
	m_nBracketOffsetY = 10;
	m_flBracketScale = 1.0f;
}

void CHUDQuickInfo::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
}

void CHUDQuickInfo::VidInit()
{
	// Defined in scripts/mod_textures.txt.
	m_icon_rb = gHUD.GetIcon( "portal_crosshair_right_valid" );
	m_icon_lb = gHUD.GetIcon( "portal_crosshair_left_valid" );
	m_icon_rbn = gHUD.GetIcon( "portal_crosshair_right_invalid" );
	m_icon_lbn = gHUD.GetIcon( "portal_crosshair_left_invalid" );
}

bool CHUDQuickInfo::ShouldDraw()
{
	if ( !m_icon_rb || !m_icon_rbn || !m_icon_lb || !m_icon_lbn )
		return false;

	if ( !g_pClientMode->ShouldDrawCrosshair() )
		return false;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	if ( !crosshair.GetBool() || IsRadialMenuOpen() )
		return false;

	// Hidden in third person (taunts, deaths) and under a point_viewcontrol.
	if ( !C_BasePlayer::LocalPlayerInFirstPersonView() ||
	     pPlayer->entindex() != render->GetViewEntity() )
		return false;

	return CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage();
}

//-----------------------------------------------------------------------------
// Purpose: The centre dot and the four dots around it.
//-----------------------------------------------------------------------------
void CHUDQuickInfo::DrawReticle( const Color &clr )
{
	int x = ScreenWidth() / 2;
	int y = ScreenHeight() / 2;

	surface()->DrawSetColor( clr );
	surface()->DrawFilledRect( x, y, x + 1, y + 1 );
	surface()->DrawFilledRect( x + m_nReticleSpread, y, x + m_nReticleSpread + 1, y + 1 );
	surface()->DrawFilledRect( x - m_nReticleSpread, y, x - m_nReticleSpread + 1, y + 1 );
	surface()->DrawFilledRect( x, y + m_nReticleSpread, x + 1, y + m_nReticleSpread + 1 );
	surface()->DrawFilledRect( x, y - m_nReticleSpread, x + 1, y - m_nReticleSpread + 1 );
}

//-----------------------------------------------------------------------------
// Purpose: Draws a bracket icon centred on ( x, y ).
//-----------------------------------------------------------------------------
void CHUDQuickInfo::DrawBracket( CHudTexture *pIcon, int x, int y, const Color &clr )
{
	int wide = pIcon->EffectiveWidth( m_flBracketScale );
	int tall = pIcon->EffectiveHeight( m_flBracketScale );
	pIcon->DrawSelf( x - wide / 2, y - tall / 2, wide, tall, clr );
}

//-----------------------------------------------------------------------------
// Purpose: Lightens a portal color channel toward white for the HUD.
//-----------------------------------------------------------------------------
static unsigned char BrightenPortalChannel( unsigned char c )
{
	if ( c <= 4 )
		return 5;

	float flValue = c + ( 1.0f - c * ( 1.0f / 255.0f ) ) * 64.0f;
	return ( flValue > 255.0f ) ? 255 : (unsigned char)flValue;
}

static Color HudPortalColor( int iPortal, int iTeamNumber, int nAlpha )
{
	Color clr = UTIL_Portal_Color( iPortal, iTeamNumber );
	return Color( BrightenPortalChannel( clr.r() ), BrightenPortalChannel( clr.g() ),
	    BrightenPortalChannel( clr.b() ), nAlpha );
}

void CHUDQuickInfo::Paint()
{
	C_Portal_Player *pPortalPlayer = ToPortalPlayer( C_BasePlayer::GetLocalPlayer() );
	if ( pPortalPlayer == NULL )
		return;

	C_BaseCombatWeapon *pWeapon = pPortalPlayer->GetActiveWeapon();

	// Everything fades with the screen fade.
	unsigned char fadeR, fadeG, fadeB, fadeA;
	bool bFadeBlend;
	vieweffects->GetFadeParams( &fadeR, &fadeG, &fadeB, &fadeA, &bFadeBlend );
	const int nFadeAlpha = fadeA;
	const int nOpaqueAlpha = 255 - nFadeAlpha;

	SetActive( true );

	DrawReticle( Color( 255, 255, 255, nOpaqueAlpha ) );

	if ( pWeapon == NULL || !hud_quickinfo.GetBool() )
		return;

	C_WeaponPortalgun *pPortalgun = dynamic_cast<C_WeaponPortalgun *>( pWeapon );
	if ( pPortalgun == NULL || ( !pPortalgun->CanFirePortal1() && !pPortalgun->CanFirePortal2() ) )
		return;

	const bool bCompleteWeapon = pPortalgun->CanFirePortal1() && pPortalgun->CanFirePortal2();
	const bool bSwap = hud_quickinfo_swap.GetBool();
	const int iTeamNumber = pPortalPlayer->GetTeamNumber();

	// The outlines of the left and right brackets.
	const int nOutlineAlpha = ( nFadeAlpha <= 150 ) ? 150 - nFadeAlpha : 0;
	const int iLeftPortal = bSwap ? 2 : 1;
	const int iRightPortal = bSwap ? 1 : 2;
	Color clrLeft = HudPortalColor( iLeftPortal, iTeamNumber, nOutlineAlpha );
	Color clrRight = HudPortalColor( iRightPortal, iTeamNumber, nOutlineAlpha );

	// A bracket fills while its portal is open.
	Color clrLeftFill = clrLeft;
	Color clrRightFill = clrRight;
	clrLeftFill[3] = 128;
	clrRightFill[3] = 128;
	if ( bCompleteWeapon )
	{
		const bool bBlueOpen = pPortalgun->GetBluePortalPosition() != vec3_invalid;
		const bool bOrangeOpen = pPortalgun->GetOrangePortalPosition() != vec3_invalid;
		const bool bLeftOpen = bSwap ? bOrangeOpen : bBlueOpen;
		const bool bRightOpen = bSwap ? bBlueOpen : bOrangeOpen;
		clrLeftFill[3] = bLeftOpen ? nOpaqueAlpha : 0;
		clrRightFill[3] = bRightOpen ? nOpaqueAlpha : 0;
	}

	// A gun with one portal draws both brackets in that portal's color.
	Color clrSingle = pPortalgun->CanFirePortal1() ? clrLeft : clrRight;
	clrSingle[3] = clamp( clrSingle.a() - nFadeAlpha, 0, 255 );

	const int xCenter = ScreenWidth() / 2;
	const int yCenter = ScreenHeight() / 2;
	const int xLeft = xCenter - m_nBracketOffsetX;
	const int yLeft = yCenter - m_nBracketOffsetY;
	const int xRight = xCenter + m_nBracketOffsetX;
	const int yRight = yCenter + m_nBracketOffsetY;

	DrawBracket( m_icon_lbn, xLeft, yLeft, bCompleteWeapon ? clrLeft : clrSingle );
	DrawBracket( m_icon_rbn, xRight, yRight, bCompleteWeapon ? clrRight : clrSingle );
	DrawBracket( m_icon_lb, xLeft, yLeft, bCompleteWeapon ? clrLeftFill : clrSingle );
	DrawBracket( m_icon_rb, xRight, yRight, bCompleteWeapon ? clrRightFill : clrSingle );
}
