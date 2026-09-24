//========= Portal 2 reconstruction ============================================//
//
// Purpose: HUD flash indicator and camera/picture control helper icons
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
#include <vgui_controls/EditablePanel.h>
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Briefly flashes an indicator icon in the center of the screen
//-----------------------------------------------------------------------------
class CHudIndicator : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudIndicator, vgui::EditablePanel );

public:
	CHudIndicator( const char *pElementName );

	void	MsgFunc_IndicatorFlash( bf_read &msg );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void Paint( void );
	virtual void Init( void );
	virtual bool ShouldDraw( void );
	virtual void Reset( void );

private:
	int		m_nTexture[2];
	int		m_nIndicatorType;

	float	m_flDisplayTime;
	float	m_flStartTime;
	float	m_flFadeInTime;
	float	m_flFadeOutTime;
};

DECLARE_HUD_MESSAGE( CHudIndicator, IndicatorFlash );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CHudIndicator::CHudIndicator( const char *pElementName ) : BaseClass( NULL, "HudIndicator" ), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	LoadControlSettings( "Resource/Indicator.res" );

	m_nTexture[0] = -1;
	m_nTexture[1] = -1;
	m_nIndicatorType = 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudIndicator::Init( void )
{
	HOOK_HUD_MESSAGE( CHudIndicator, IndicatorFlash );

	m_nTexture[0] = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTexture[0], "HUD/invalid", true, false );

	m_nTexture[1] = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTexture[1], "HUD/inv_full", true, false );

	m_flDisplayTime = 0.0f;
	m_flStartTime = 0.0f;
	m_flFadeInTime = 0.0f;
	m_flFadeOutTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudIndicator::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHudIndicator::ShouldDraw( void )
{
	return ( m_flDisplayTime > gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudIndicator::ApplySchemeSettings( vgui::IScheme *scheme )
{
	LoadControlSettings( "Resource/Indicator.res" );

	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );

	int screenWide, screenTall;
	GetHudSize( screenWide, screenTall );

	// Center a 256x256 panel on the screen
	int nCenterX = screenWide / 2;
	int nCenterY = screenTall / 2;
	SetBounds( nCenterX - 128, nCenterY - 128, 256, 256 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudIndicator::Paint( void )
{
	float flGlobalScale = 1.0f;
	float flGlobalAlpha = 128.0f;

	if ( gpGlobals->curtime < m_flFadeInTime )
	{
		flGlobalScale = SimpleSplineRemapValClamped( gpGlobals->curtime, m_flStartTime, m_flFadeInTime, 0.0f, 1.0f );
		flGlobalAlpha = flGlobalScale * 128.0f;
	}
	else if ( gpGlobals->curtime > m_flFadeOutTime )
	{
		flGlobalScale = SimpleSplineRemapValClamped( gpGlobals->curtime, m_flFadeOutTime, m_flDisplayTime, 1.0f, 0.0f );
		flGlobalAlpha = flGlobalScale * 128.0f;
	}

	// Pulse the icon's size while it's displayed
	float flBump = sin( gpGlobals->curtime * 10.0f ) * 16.0f * flGlobalScale;
	float flScale = 128.0f + ( flBump * flGlobalScale );
	const float flOffset = flScale * 0.5f;

	vgui::surface()->DrawSetTexture( m_nTexture[m_nIndicatorType] );

	float xLeft = ( GetWide() / 2 ) - flOffset;
	float xRight = ( GetWide() / 2 ) + flOffset;
	float yTop = ( GetTall() / 2 ) - flOffset;
	float yBottom = ( GetTall() / 2 ) + flOffset;

	// Drop shadow
	vgui::surface()->DrawSetColor( 0, 0, 0, flGlobalAlpha );
	const int nShadowDepth = 4;
	Vertex_t vert[4];
	vert[0].Init( Vector2D( xLeft + nShadowDepth, yTop + nShadowDepth ), Vector2D( 0, 0 ) );
	vert[1].Init( Vector2D( xRight + nShadowDepth, yTop + nShadowDepth ), Vector2D( 1, 0 ) );
	vert[2].Init( Vector2D( xRight + nShadowDepth, yBottom + nShadowDepth ), Vector2D( 1, 1 ) );
	vert[3].Init( Vector2D( xLeft + nShadowDepth, yBottom + nShadowDepth ), Vector2D( 0, 1 ) );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Icon
	vgui::surface()->DrawSetColor( 255, 255, 255, flGlobalScale * 255.0f );
	vert[0].Init( Vector2D( xLeft, yTop ), Vector2D( 0, 0 ) );
	vert[1].Init( Vector2D( xRight, yTop ), Vector2D( 1, 0 ) );
	vert[2].Init( Vector2D( xRight, yBottom ), Vector2D( 1, 1 ) );
	vert[3].Init( Vector2D( xLeft, yBottom ), Vector2D( 0, 1 ) );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Show an indicator for a set duration
//-----------------------------------------------------------------------------
void CHudIndicator::MsgFunc_IndicatorFlash( bf_read &msg )
{
	m_nIndicatorType = msg.ReadByte();

	m_flStartTime = gpGlobals->curtime;
	m_flDisplayTime = gpGlobals->curtime + msg.ReadFloat();
	m_flFadeInTime = gpGlobals->curtime + 0.2f;
	m_flFadeOutTime = m_flDisplayTime - 0.2f;
}

ConVar hud_show_control_helper( "hud_show_control_helper", "0" );

//-----------------------------------------------------------------------------
// Purpose: Shows the camera / neutral / picture control icons with the
//			selected one pulled to the front
//-----------------------------------------------------------------------------
class CHudControlHelper : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudControlHelper, vgui::EditablePanel );

public:
	CHudControlHelper( const char *pElementName );

	void	MsgFunc_ControlHelperAnimate( bf_read &msg );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void Paint( void );
	virtual void Init( void );
	virtual bool ShouldDraw( void );
	virtual void Reset( void );

private:
	void	DrawIcon( int x, int y, float flScale, float flAlpha, int nIconID, int nDepth );

	int		m_nTexture[3];
	int		m_nFrontIcon;

	float	m_flDisplayTime;
	float	m_flStartTime;
	float	m_flFadeInTime;
	float	m_flFadeOutTime;
	bool	m_bHoldIndefinitely;
};

DECLARE_HUD_MESSAGE( CHudControlHelper, ControlHelperAnimate );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CHudControlHelper::CHudControlHelper( const char *pElementName ) : BaseClass( NULL, "HudControlHelper" ), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	LoadControlSettings( "Resource/ControlHelper.res" );

	m_nTexture[0] = -1;
	m_nTexture[1] = -1;
	m_nTexture[2] = -1;
	m_nFrontIcon = 0;
	m_bHoldIndefinitely = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudControlHelper::Init( void )
{
	HOOK_HUD_MESSAGE( CHudControlHelper, ControlHelperAnimate );

	m_nTexture[0] = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTexture[0], "HUD/hud_icon_camera", true, false );

	m_nTexture[1] = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTexture[1], "HUD/hud_icon_neutral", true, false );

	m_nTexture[2] = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTexture[2], "HUD/hud_icon_picture", true, false );

	m_flDisplayTime = 0.0f;
	m_flStartTime = 0.0f;
	m_flFadeInTime = 0.0f;
	m_flFadeOutTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudControlHelper::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHudControlHelper::ShouldDraw( void )
{
	if ( hud_show_control_helper.GetBool() == false )
		return false;

	if ( m_flDisplayTime > gpGlobals->curtime )
		return true;

	return m_bHoldIndefinitely;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudControlHelper::ApplySchemeSettings( vgui::IScheme *scheme )
{
	LoadControlSettings( "Resource/ControlHelper.res" );

	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );

	// A full-width strip, 96 pixels tall, centered 80% of the way down the screen
	int nCenterY = ScreenHeight() * 0.8f;
	SetBounds( 0, nCenterY - 48, ScreenWidth(), 96 );
}

//-----------------------------------------------------------------------------
// Purpose: Draw a single 48x48 icon with a drop shadow
//-----------------------------------------------------------------------------
void CHudControlHelper::DrawIcon( int x, int y, float flScale, float flAlpha, int nIconID, int nDepth )
{
	vgui::surface()->DrawSetTexture( m_nTexture[nIconID] );

	const float flOffset = flScale * 0.5f;

	float xLeft = x - flOffset;
	float xRight = ( x + 48 ) + flOffset;
	float yTop = y - flOffset;
	float yBottom = ( y + 48 ) + flOffset;

	// Drop shadow
	vgui::surface()->DrawSetColor( 0, 0, 0, flAlpha * 127.0f );
	const int nShadowDepth = 6;
	Vertex_t vert[4];
	vert[0].Init( Vector2D( xLeft + nShadowDepth, yTop + nShadowDepth ), Vector2D( 0, 0 ) );
	vert[1].Init( Vector2D( xRight + nShadowDepth, yTop + nShadowDepth ), Vector2D( 1, 0 ) );
	vert[2].Init( Vector2D( xRight + nShadowDepth, yBottom + nShadowDepth ), Vector2D( 1, 1 ) );
	vert[3].Init( Vector2D( xLeft + nShadowDepth, yBottom + nShadowDepth ), Vector2D( 0, 1 ) );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Icons further back are drawn darker
	float flColor = ( 255 / nDepth );
	vgui::surface()->DrawSetColor( flColor, flColor, flColor, flAlpha * 255.0f );
	vert[0].Init( Vector2D( xLeft, yTop ), Vector2D( 0, 0 ) );
	vert[1].Init( Vector2D( xRight, yTop ), Vector2D( 1, 0 ) );
	vert[2].Init( Vector2D( xRight, yBottom ), Vector2D( 1, 1 ) );
	vert[3].Init( Vector2D( xLeft, yBottom ), Vector2D( 0, 1 ) );
	vgui::surface()->DrawTexturedPolygon( 4, vert );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudControlHelper::Paint( void )
{
	float flGlobalScale = 1.0f;
	float flGlobalAlpha = 1.0f;

	bool bScalingUp = false;
	bool bScalingDown = false;

	if ( gpGlobals->curtime < m_flFadeInTime )
	{
		// Grow the front icon in
		flGlobalScale = SimpleSplineRemapValClamped( gpGlobals->curtime, m_flStartTime, m_flFadeInTime, 0.0f, 1.0f );
		bScalingUp = true;
	}
	else if ( m_bHoldIndefinitely == false && gpGlobals->curtime > m_flFadeOutTime )
	{
		// Shrink and fade everything out
		flGlobalAlpha = SimpleSplineRemapValClamped( gpGlobals->curtime, m_flFadeOutTime, m_flDisplayTime, 1.0f, 0.0f );
		flGlobalScale = flGlobalAlpha;
		bScalingDown = true;
	}

	int iconCenterX = GetWide() * 0.5f;
	int iconY = GetTall() * 0.8f - 48.0f;

	// The side icons slide in toward the center as we fade out
	float flMoveScale = flGlobalAlpha;

	int nBaseOffset = iconCenterX - 24.0f;
	int xOffset[3] = { nBaseOffset + ( -68.0f * flMoveScale ), nBaseOffset, nBaseOffset + ( 68.0f * flMoveScale ) };

	// Draw the front icon last so it's on top
	int nCurIcon = ( m_nFrontIcon + 1 ) % ARRAYSIZE( m_nTexture );
	for ( int i = 0; i < 3; i++ )
	{
		float flScaleFactor = 1.0f;

		int nDepth = 2;

		int xPos = xOffset[nCurIcon];

		if ( nCurIcon == m_nFrontIcon )
		{
			nDepth = 1;

			// Front side icons are enlarged and pushed outward
			if ( nCurIcon != 1 )
			{
				flScaleFactor = 2.0f;

				if ( nCurIcon == 0 )
				{
					xPos += ( -10.0f * flGlobalScale );
				}
				else if ( nCurIcon == 2 )
				{
					xPos += ( 10.0f * flGlobalScale );
				}
			}
		}

		float flScale = ( bScalingDown || ( nCurIcon == m_nFrontIcon && bScalingUp ) ) ? ( 48.0f * flGlobalScale ) : 48.0f;

		DrawIcon( xPos, iconY, flScale * flScaleFactor, flGlobalAlpha, nCurIcon, nDepth );

		nCurIcon = ( nCurIcon + 1 ) % ARRAYSIZE( m_nTexture );
	}

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Bring an icon to the front, or clear the helper
//-----------------------------------------------------------------------------
void CHudControlHelper::MsgFunc_ControlHelperAnimate( bf_read &msg )
{
	bool bClear = ( msg.ReadByte() != 0 );
	if ( bClear )
	{
		m_flDisplayTime = gpGlobals->curtime + 0.1f;
		m_flStartTime = m_flFadeInTime = gpGlobals->curtime;
		m_flFadeOutTime = m_flDisplayTime;
		m_bHoldIndefinitely = false;
		return;
	}

	m_nFrontIcon = msg.ReadByte();

	// The neutral icon only shows briefly, the others stay up
	if ( m_nFrontIcon == 1 )
	{
		m_flDisplayTime = gpGlobals->curtime + 0.5f;
		m_bHoldIndefinitely = false;
	}
	else
	{
		m_bHoldIndefinitely = true;
	}

	m_flStartTime = gpGlobals->curtime;
	m_flFadeInTime = gpGlobals->curtime + 0.2f;
	m_flFadeOutTime = m_flDisplayTime - 0.1f;
}
