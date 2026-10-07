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
// - The brackets and reticle are drawn as shapes rasterized at the screen's
//   pixel size instead of retail's 44x64 bitmaps, so they stay sharp at any
//   resolution and UI scale (user request, 2026-10-07). The bracket is the
//   construction recovered from sprites/hud/portal_crosshairs: a cut ring
//   between two ellipses (see kBracketOuter).
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
#include "cdll_util.h"
#include "utlvector.h"

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

	virtual ~CHUDQuickInfo();

private:
	enum BracketShape
	{
		BRACKET_OUTLINE,
		BRACKET_FILL,
		BRACKET_SHAPE_COUNT
	};

	void UpdateShapes();
	void DrawPixelQuad( int nTexture, float flLeftPx, float flTopPx, int nWide, int nTall,
	    float flMaxS, float flMaxT, bool bRotate180, const Color &clr );
	void DrawReticle( const Color &clr );
	void DrawBracket( BracketShape eShape, bool bRight, int x, int y, const Color &clr );

	float m_flPixelsPerUnit; // the scale the shapes were rasterized for; 0 before the first
	int m_nBracketTexture[BRACKET_SHAPE_COUNT];
	int m_nBracketWide; // bracket image size in pixels
	int m_nBracketTall;
	float m_flBracketMaxS; // the image's extent in its power-of-two texture
	float m_flBracketMaxT;
	int m_nWhiteTexture;

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

	m_flPixelsPerUnit = 0.0f;
	for ( int i = 0; i < BRACKET_SHAPE_COUNT; ++i )
		m_nBracketTexture[i] = -1;
	m_nBracketWide = m_nBracketTall = 0;
	m_flBracketMaxS = m_flBracketMaxT = 1.0f;
	m_nWhiteTexture = -1;
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

CHUDQuickInfo::~CHUDQuickInfo()
{
	if ( !vgui::surface() )
		return;
	for ( int i = 0; i < BRACKET_SHAPE_COUNT; ++i )
	{
		if ( m_nBracketTexture[i] != -1 )
			vgui::surface()->DeleteTextureByID( m_nBracketTexture[i] );
	}
	if ( m_nWhiteTexture != -1 )
		vgui::surface()->DeleteTextureByID( m_nWhiteTexture );
}

void CHUDQuickInfo::VidInit()
{
	// The shapes are rebuilt for the new mode on the next paint.
	m_flPixelsPerUnit = 0.0f;
}

//-----------------------------------------------------------------------------
// The left bracket in the units of retail's 44x64 bitmap: the ring between an
// upright ellipse and a smaller one turned 12.2 degrees, cut at its two ends,
// with a 0.75-unit outline centred just outside the edge and the fill inset
// inside it. The constants are the least-squares solution against both retail
// bitmaps (empty and full), to about 8 of 255 levels RMS; each edge alone fits
// an ellipse to 0.12 pixels. The right bracket is the left one turned 180
// degrees about the box centre.
//-----------------------------------------------------------------------------
struct BracketEllipse
{
	float cx, cy;    // centre
	float rx, ry;    // semi-axes before rotation
	float flDegrees; // rotation, clockwise on screen
};
struct BracketCut
{
	float x, y;      // a point on the line
	float flDegrees; // direction of the outward normal, clockwise from +x
};

static const float kBracketUnitsWide = 44.0f;
static const float kBracketUnitsTall = 64.0f;
static const BracketEllipse kBracketOuter = { 28.079f, 47.364f, 26.454f, 45.871f, -0.442f };
static const BracketEllipse kBracketInner = { 27.471f, 46.308f, 21.157f, 38.837f, 12.201f };
static const BracketCut kBracketCuts[2] = {
    { 42.512f, 12.0f, -1.471f }, // top end
    { 4.0f, 62.115f, 87.952f },  // bottom end
};
static const float kBracketOutlineUnits = 0.747f;
static const float kBracketOutlineOffsetUnits = 0.212f; // band centre, outside the edge
static const float kBracketFillInsetUnits = 1.443f;

//-----------------------------------------------------------------------------
// Signed distance to an ellipse, to first order (the implicit function over its
// gradient), negative inside: the measure the constants were solved with.
//-----------------------------------------------------------------------------
static float EllipseDistance( const BracketEllipse &e, float x, float y )
{
	float s, c;
	SinCos( DEG2RAD( e.flDegrees ), &s, &c );
	const float u = ( x - e.cx ) * c + ( y - e.cy ) * s;
	const float v = -( x - e.cx ) * s + ( y - e.cy ) * c;
	const float f = ( u * u ) / ( e.rx * e.rx ) + ( v * v ) / ( e.ry * e.ry ) - 1.0f;
	const float g =
	    2.0f * FastSqrt( Square( u / ( e.rx * e.rx ) ) + Square( v / ( e.ry * e.ry ) ) );
	return f / MAX( g, 1e-6f );
}

static float CutDistance( const BracketCut &cut, float x, float y )
{
	float s, c;
	SinCos( DEG2RAD( cut.flDegrees ), &s, &c );
	return ( x - cut.x ) * c + ( y - cut.y ) * s;
}

// Signed distance in units to the left bracket's edge, negative inside.
static float BracketDistance( float x, float y )
{
	float d =
	    MAX( EllipseDistance( kBracketOuter, x, y ), -EllipseDistance( kBracketInner, x, y ) );
	for ( int i = 0; i < ARRAYSIZE( kBracketCuts ); ++i )
		d = MAX( d, CutDistance( kBracketCuts[i], x, y ) );
	return d;
}

static unsigned char Coverage( float flValue )
{
	return (unsigned char)( clamp( flValue, 0.0f, 1.0f ) * 255.0f + 0.5f );
}

//-----------------------------------------------------------------------------
// Purpose: Rasterizes the bracket shapes for the current UI scale. Coverage
// comes from the signed distance to the bracket edge, evaluated per screen pixel, so
// the edges are antialiased at the size they are drawn.
//-----------------------------------------------------------------------------
void CHUDQuickInfo::UpdateShapes()
{
	float flUnitsPerPixelX, flUnitsPerPixelY;
	GetVGuiUnitsPerPixel( flUnitsPerPixelX, flUnitsPerPixelY );
	const float flScale = flUnitsPerPixelY > 0.0f ? 1.0f / flUnitsPerPixelY : 1.0f;
	if ( m_flPixelsPerUnit == flScale && m_nWhiteTexture != -1 )
		return;
	m_flPixelsPerUnit = flScale;

	if ( m_nWhiteTexture == -1 )
	{
		unsigned char white[4 * 4 * 4];
		V_memset( white, 255, sizeof( white ) );
		m_nWhiteTexture = vgui::surface()->CreateNewTextureID( true );
		vgui::surface()->DrawSetTextureRGBA( m_nWhiteTexture, white, 4, 4, 0, true );
	}

	// One transparent pixel of border so edge texels never clamp onto ink.
	const float flShapeScale = flScale * m_flBracketScale; // pixels per bitmap unit
	m_nBracketWide = (int)ceilf( kBracketUnitsWide * flShapeScale ) + 2;
	m_nBracketTall = (int)ceilf( kBracketUnitsTall * flShapeScale ) + 2;
	const float flOriginX = 0.5f * ( m_nBracketWide - kBracketUnitsWide * flShapeScale );
	const float flOriginY = 0.5f * ( m_nBracketTall - kBracketUnitsTall * flShapeScale );

	const float flOutlinePx = kBracketOutlineUnits * flShapeScale;
	const float flOutlineOffsetPx = kBracketOutlineOffsetUnits * flShapeScale;
	const float flFillInsetPx = kBracketFillInsetUnits * flShapeScale;

	// The surface stores procedural textures at power-of-two sizes and maps
	// texture coordinates over the whole of them; upload the padded image.
	int nTextureWide = 1, nTextureTall = 1;
	while ( nTextureWide < m_nBracketWide )
		nTextureWide <<= 1;
	while ( nTextureTall < m_nBracketTall )
		nTextureTall <<= 1;
	m_flBracketMaxS = (float)m_nBracketWide / nTextureWide;
	m_flBracketMaxT = (float)m_nBracketTall / nTextureTall;

	CUtlVector<unsigned char> outline, fill;
	outline.SetCount( nTextureWide * nTextureTall * 4 );
	fill.SetCount( nTextureWide * nTextureTall * 4 );
	V_memset( outline.Base(), 0, outline.Count() );
	V_memset( fill.Base(), 0, fill.Count() );
	for ( int y = 0; y < m_nBracketTall; ++y )
	{
		for ( int x = 0; x < m_nBracketWide; ++x )
		{
			// Signed distance in pixels at the pixel centre; coverage is a
			// one-pixel ramp across it.
			const float flUnitX = ( x + 0.5f - flOriginX ) / flShapeScale;
			const float flUnitY = ( y + 0.5f - flOriginY ) / flShapeScale;
			const float flSigned = flShapeScale * BracketDistance( flUnitX, flUnitY );

			// The art is cropped to its cell; the ring's far side reaches past it.
			const float flOutsideCell =
			    flShapeScale * MAX( MAX( -flUnitX, flUnitX - kBracketUnitsWide ),
			                       MAX( -flUnitY, flUnitY - kBracketUnitsTall ) );
			const float flCellCoverage = clamp( 0.5f - flOutsideCell, 0.0f, 1.0f );

			const int n = ( y * nTextureWide + x ) * 4;
			const float flBandCentre = flOutlineOffsetPx;
			outline[n + 0] = outline[n + 1] = outline[n + 2] = 255;
			outline[n + 3] = Coverage(
			    flCellCoverage *
			    clamp( 0.5f * flOutlinePx - fabsf( flSigned - flBandCentre ) + 0.5f, 0.0f, 1.0f ) );
			fill[n + 0] = fill[n + 1] = fill[n + 2] = 255;
			fill[n + 3] = Coverage(
			    flCellCoverage * clamp( -( flSigned + flFillInsetPx ) + 0.5f, 0.0f, 1.0f ) );
		}
	}

	const unsigned char *pShapes[BRACKET_SHAPE_COUNT] = { outline.Base(), fill.Base() };
	for ( int i = 0; i < BRACKET_SHAPE_COUNT; ++i )
	{
		// A procedural texture keeps its first size, so a new size needs a new one.
		if ( m_nBracketTexture[i] != -1 )
			vgui::surface()->DeleteTextureByID( m_nBracketTexture[i] );
		m_nBracketTexture[i] = vgui::surface()->CreateNewTextureID( true );
		vgui::surface()->DrawSetTextureRGBA(
		    m_nBracketTexture[i], pShapes[i], nTextureWide, nTextureTall, 0, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draws a texture one texel per pixel at a pixel position, so the
// shapes keep the sharpness they were rasterized with.
//-----------------------------------------------------------------------------
void CHUDQuickInfo::DrawPixelQuad( int nTexture, float flLeftPx, float flTopPx, int nWide,
    int nTall, float flMaxS, float flMaxT, bool bRotate180, const Color &clr )
{
	const float flUnits = 1.0f / m_flPixelsPerUnit;
	const float x0 = flLeftPx * flUnits, y0 = flTopPx * flUnits;
	const float x1 = ( flLeftPx + nWide ) * flUnits, y1 = ( flTopPx + nTall ) * flUnits;
	const float s0 = bRotate180 ? flMaxS : 0.0f, s1 = flMaxS - s0;
	const float t0 = bRotate180 ? flMaxT : 0.0f, t1 = flMaxT - t0;
	vgui::Vertex_t verts[4];
	verts[0].Init( Vector2D( x0, y0 ), Vector2D( s0, t0 ) );
	verts[1].Init( Vector2D( x1, y0 ), Vector2D( s1, t0 ) );
	verts[2].Init( Vector2D( x1, y1 ), Vector2D( s1, t1 ) );
	verts[3].Init( Vector2D( x0, y1 ), Vector2D( s0, t1 ) );
	vgui::surface()->DrawSetColor( clr );
	vgui::surface()->DrawSetTexture( nTexture );
	vgui::surface()->DrawTexturedPolygon( 4, verts );
}

bool CHUDQuickInfo::ShouldDraw()
{
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
	// Square dots of whole pixels around the centre pixel.
	const float flScale = m_flPixelsPerUnit;
	const int nDot = MAX( 1, (int)( flScale + 0.5f ) );
	const int nSpread = (int)( m_nReticleSpread * flScale + 0.5f );
	const float x = floorf( ScreenWidth() / 2 * flScale ) - ( nDot - 1 ) / 2;
	const float y = floorf( ScreenHeight() / 2 * flScale ) - ( nDot - 1 ) / 2;

	static const int kDots[5][2] = { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for ( int i = 0; i < 5; ++i )
	{
		DrawPixelQuad( m_nWhiteTexture, x + kDots[i][0] * nSpread, y + kDots[i][1] * nSpread, nDot,
		    nDot, 1.0f, 1.0f, false, clr );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draws a bracket shape centred on ( x, y ) in UI units.
//-----------------------------------------------------------------------------
void CHUDQuickInfo::DrawBracket( BracketShape eShape, bool bRight, int x, int y, const Color &clr )
{
	const float flLeft = floorf( x * m_flPixelsPerUnit - 0.5f * m_nBracketWide + 0.5f );
	const float flTop = floorf( y * m_flPixelsPerUnit - 0.5f * m_nBracketTall + 0.5f );
	DrawPixelQuad( m_nBracketTexture[eShape], flLeft, flTop, m_nBracketWide, m_nBracketTall,
	    m_flBracketMaxS, m_flBracketMaxT, bRight, clr );
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

	UpdateShapes();
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

	// Retail draws the full bitmaps (outline plus fill) over the empty ones.
	DrawBracket( BRACKET_OUTLINE, false, xLeft, yLeft, bCompleteWeapon ? clrLeft : clrSingle );
	DrawBracket( BRACKET_OUTLINE, true, xRight, yRight, bCompleteWeapon ? clrRight : clrSingle );
	DrawBracket( BRACKET_OUTLINE, false, xLeft, yLeft, bCompleteWeapon ? clrLeftFill : clrSingle );
	DrawBracket(
	    BRACKET_OUTLINE, true, xRight, yRight, bCompleteWeapon ? clrRightFill : clrSingle );
	DrawBracket( BRACKET_FILL, false, xLeft, yLeft, bCompleteWeapon ? clrLeftFill : clrSingle );
	DrawBracket( BRACKET_FILL, true, xRight, yRight, bCompleteWeapon ? clrRightFill : clrSingle );
}
