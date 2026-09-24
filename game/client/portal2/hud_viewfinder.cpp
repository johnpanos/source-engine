//========= Portal 2 reconstruction ============================================//
//
// Purpose: Camera viewfinder overlay for remote-view taunts, plus the screen
//			space bounds helpers shared with the radial menu
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "view_scene.h"
#include "c_portal_player.h"
#include "c_portal_playerlocaldata.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


//-----------------------------------------------------------------------------
// Purpose: Draws the camera viewfinder while the local player is in a
//			remote-view taunt
//
// Reconstruction note: neither 2010 build instantiates this element; there is
// no hud element factory or other constructor caller in either binary, so it is
// left unregistered here as well.
//-----------------------------------------------------------------------------
class CHudViewfinder : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudViewfinder, vgui::Panel );

public:
	CHudViewfinder( const char *pElementName );

	virtual void Init( void );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );

private:
	void PaintLocator( C_BaseEntity *pTarget );
	void PaintLocators( void );

	int m_iScopeTexture[ 4 ];
	int m_nChickenIcon;
	int m_nArrowIcon;
};


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudViewfinder::CHudViewfinder( const char *pElementName ) : BaseClass( NULL, "HudViewfinder" ), CHudElement( pElementName )
{
	vgui::Panel *pParent = GetClientMode()->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudViewfinder::Init( void )
{
	for ( int i = 0; i < 4; i++ )
	{
		m_iScopeTexture[ i ] = vgui::surface()->CreateNewTextureID();
	}

	vgui::surface()->DrawSetTextureFile( m_iScopeTexture[ 0 ], "HUD/camera_viewfinder_ul", true, false );
	vgui::surface()->DrawSetTextureFile( m_iScopeTexture[ 1 ], "HUD/camera_viewfinder_halfcircle", true, false );

	m_nChickenIcon = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nChickenIcon, "HUD/hud_loc_chicken", true, false );

	m_nArrowIcon = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nArrowIcon, "HUD/hud_loc_arrow", true, false );

	UnregisterForRenderGroup( "global" );
}

//-----------------------------------------------------------------------------
// Purpose: Sizes the panel to cover the whole HUD
//-----------------------------------------------------------------------------
void CHudViewfinder::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );

	int screenWide, screenTall;
	GetHudSize( screenWide, screenTall );
	SetBounds( 0, 0, screenWide, screenTall );
}

//-----------------------------------------------------------------------------
// Purpose: Only draw while the local player is taunting through a remote view
//-----------------------------------------------------------------------------
bool CHudViewfinder::ShouldDraw( void )
{
	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPlayer();

	return ( pPlayer && pPlayer->IsTaunting() && pPlayer->IsRemoteViewTaunt() );
}

//-----------------------------------------------------------------------------
// Purpose: Converts a world position to screen coordinates (0,0 if it's behind the view)
//-----------------------------------------------------------------------------
void UTIL_WorldToScreenCoords( const Vector &vecWorld, int *pScreenX, int *pScreenY )
{
	*pScreenX = 0;
	*pScreenY = 0;

	Vector vecTransform;
	if ( ScreenTransform( vecWorld, vecTransform ) == 0 )
	{
		*pScreenX = ( ScreenWidth() / 2 ) + ( 0.5f * vecTransform.x * ScreenWidth() ) + 0.5f;
		*pScreenY = ( ScreenHeight() / 2 ) + ( -0.5f * vecTransform.y * ScreenHeight() ) + 0.5f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Generates the eight corners of a box in world space
//-----------------------------------------------------------------------------
void UTIL_GenerateBoxVertices( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, Vector *pVerts )
{
	Vector vecPos;
	for ( int i = 0; i < 8; i++ )
	{
		vecPos[ 0 ] = ( i & 0x1 ) ? vMaxs[ 0 ] : vMins[ 0 ];
		vecPos[ 1 ] = ( i & 0x2 ) ? vMaxs[ 1 ] : vMins[ 1 ];
		vecPos[ 2 ] = ( i & 0x4 ) ? vMaxs[ 2 ] : vMins[ 2 ];

		pVerts[ i ] = vecPos + vOrigin;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finds the screen space rectangle that encloses a world space box
//-----------------------------------------------------------------------------
bool UTIL_WorldSpaceToScreensSpaceBounds( const Vector &vecCenter, const Vector &mins, const Vector &maxs, Vector2D *pMins, Vector2D *pMaxs )
{
	Vector vecBoxVerts[ 8 ];
	UTIL_GenerateBoxVertices( vecCenter, mins, maxs, vecBoxVerts );

	int nMaxX = 0;
	int nMinX = ScreenWidth();
	int nMaxY = 0;
	int nMinY = ScreenHeight();

	int nX, nY;
	for ( int i = 0; i < 8; i++ )
	{
		UTIL_WorldToScreenCoords( vecBoxVerts[ i ], &nX, &nY );

		if ( nX > nMaxX )
		{
			nMaxX = nX;
		}
		else if ( nX < nMinX )
		{
			nMinX = nX;
		}

		if ( nY > nMaxY )
		{
			nMaxY = nY;
		}
		else if ( nY < nMinY )
		{
			nMinY = nY;
		}
	}

	if ( pMins )
	{
		pMins->x = nMinX;
		pMins->y = nMinY;
	}

	if ( pMaxs )
	{
		pMaxs->x = nMaxX;
		pMaxs->y = nMaxY;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Finds the screen space rectangle that encloses an entity's bounds
//-----------------------------------------------------------------------------
bool UTIL_EntityBoundsToSizes( C_BaseEntity *pTarget, int *pMinX, int *pMinY, int *pMaxX, int *pMaxY )
{
	Vector vOBBMins, vOBBMaxs;
	pTarget->CollisionProp()->WorldSpaceSurroundingBounds( &vOBBMins, &vOBBMaxs );

	vOBBMins -= pTarget->GetAbsOrigin();
	vOBBMaxs -= pTarget->GetAbsOrigin();

	Vector2D mins, maxs;
	UTIL_WorldSpaceToScreensSpaceBounds( pTarget->GetAbsOrigin(), vOBBMins, vOBBMaxs, &mins, &maxs );

	if ( pMinX )
	{
		*pMinX = mins.x;
	}

	if ( pMinY )
	{
		*pMinY = mins.y;
	}

	if ( pMaxX )
	{
		*pMaxX = maxs.x;
	}

	if ( pMaxY )
	{
		*pMaxY = maxs.y;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draws a locator icon over a target, scaled down with distance and
//			with an arrow when the target is well above or below the player
//-----------------------------------------------------------------------------
void CHudViewfinder::PaintLocator( C_BaseEntity *pTarget )
{
	Vector vecScreen;
	if ( ScreenTransform( pTarget->WorldSpaceCenter(), vecScreen ) != 0 )
		return;

	float centerX = ( ScreenWidth() / 2 ) + ( 0.5 * vecScreen[ 0 ] * ScreenWidth() ) + 0.5;
	float centerY = ( ScreenHeight() / 2 ) - ( ( 0.5 * vecScreen[ 1 ] * ScreenHeight() ) + 0.5 );

	// Shrink the icon with distance
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	float flZDiff = pTarget->WorldSpaceCenter().z - pLocalPlayer->EyePosition().z;
	float flDistSqr = ( pTarget->WorldSpaceCenter() - pLocalPlayer->EyePosition() ).LengthSqr();
	float flDistScale = RemapValClamped( flDistSqr, ( 300.0f * 300.0f ), ( 3600.0f * 3600.0f ), 1.0f, 0.25f );

	float screenMax = 48.0f * flDistScale;

	vgui::surface()->DrawSetTexture( m_nChickenIcon );

	const int nDropShadowDepth = 4;

	// Drop shadow
	vgui::surface()->DrawSetColor( 0, 0, 0, (int)( 64.0f * flDistScale ) );
	vgui::surface()->DrawTexturedRect( centerX - screenMax + nDropShadowDepth, centerY - screenMax + nDropShadowDepth,
									   centerX + screenMax + nDropShadowDepth, centerY + screenMax + nDropShadowDepth );

	vgui::surface()->DrawSetColor( 255, 128, 0, (int)( 164.0f * flDistScale ) );
	vgui::surface()->DrawTexturedRect( centerX - screenMax, centerY - screenMax, centerX + screenMax, centerY + screenMax );

	// Point toward targets that are well above or below us
	if ( fabs( flZDiff ) > 120.0f )
	{
		vgui::surface()->DrawSetTexture( m_nArrowIcon );

		int nWidth = 8.0f * flDistScale;
		int nHeight = 8.0f * flDistScale;

		if ( flZDiff > 0.0f )
		{
			// Above: arrow over the icon
			vgui::surface()->DrawTexturedRect( centerX - nWidth, centerY - ( screenMax + nHeight ), centerX + nWidth, centerY - screenMax );
		}
		else
		{
			// Below: flipped arrow under the icon
			Vertex_t vert[ 4 ];

			vert[ 0 ].Init( Vector2D( centerX - nWidth, centerY + screenMax ), Vector2D( 0, 1 ) );
			vert[ 1 ].Init( Vector2D( centerX + nWidth, centerY + screenMax ), Vector2D( 1, 1 ) );
			vert[ 2 ].Init( Vector2D( centerX + nWidth, centerY + screenMax + nHeight ), Vector2D( 1, 0 ) );
			vert[ 3 ].Init( Vector2D( centerX - nWidth, centerY + screenMax + nHeight ), Vector2D( 0, 0 ) );

			vgui::surface()->DrawTexturedPolygon( 4, vert );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draws every locator the server has assigned to the local player
//-----------------------------------------------------------------------------
void CHudViewfinder::PaintLocators( void )
{
	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPlayer();

	for ( int i = 0; i < MAX_PORTAL_LOCATOR_ENTITIES; i++ )
	{
		int nEntityIndex = pPlayer->GetPortalPlayerLocalData().m_nLocatorEntityIndices[ i ];
		if ( nEntityIndex < 0 )
			continue;

		C_BaseEntity *pEntity = C_BaseEntity::Instance( nEntityIndex );
		if ( pEntity )
		{
			PaintLocator( pEntity );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draws the viewfinder: vignetted corners, focus circles, registration
//			marks and a blinking "REC"
//-----------------------------------------------------------------------------
void CHudViewfinder::Paint( void )
{
	PaintLocators();

	// The viewfinder is drawn over the refracted frame
	UpdateRefractTexture();

	int screenWide, screenTall;
	GetHudSize( screenWide, screenTall );

	// Corner vignettes, one quarter-texture per screen quadrant
	int xMid = screenWide / 2;
	int yMid = screenTall / 2;

	int wide = screenWide;

	int xLeft = xMid - wide / 2;
	int xRight = xMid + wide / 2;
	int yTop = 0;
	int yBottom = screenTall;

	float uv1 = 0.0f, uv2 = 1.0f;

	Vertex_t vert[ 4 ];

	Vector2D uv11( uv1, uv1 );
	Vector2D uv12( uv1, uv2 );
	Vector2D uv21( uv2, uv1 );
	Vector2D uv22( uv2, uv2 );

	vgui::surface()->DrawSetColor( 0, 0, 0, 255 );

	// Upper left
	vgui::surface()->DrawSetTexture( m_iScopeTexture[ 0 ] );
	vert[ 0 ].Init( Vector2D( xLeft, yTop ), uv11 );
	vert[ 1 ].Init( Vector2D( xMid, yTop ), uv21 );
	vert[ 2 ].Init( Vector2D( xMid, yMid ), uv22 );
	vert[ 3 ].Init( Vector2D( xLeft, yMid ), uv12 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Upper right
	vert[ 0 ].Init( Vector2D( xMid, yTop ), uv21 );
	vert[ 1 ].Init( Vector2D( xRight, yTop ), uv11 );
	vert[ 2 ].Init( Vector2D( xRight, yMid ), uv12 );
	vert[ 3 ].Init( Vector2D( xMid, yMid ), uv22 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Lower right
	vert[ 0 ].Init( Vector2D( xMid, yMid ), uv22 );
	vert[ 1 ].Init( Vector2D( xRight, yMid ), uv12 );
	vert[ 2 ].Init( Vector2D( xRight, yBottom ), uv11 );
	vert[ 3 ].Init( Vector2D( xMid, yBottom ), uv21 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Lower left
	vert[ 0 ].Init( Vector2D( xLeft, yMid ), uv12 );
	vert[ 1 ].Init( Vector2D( xMid, yMid ), uv22 );
	vert[ 2 ].Init( Vector2D( xMid, yBottom ), uv21 );
	vert[ 3 ].Init( Vector2D( xLeft, yBottom ), uv11 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Focus circles
	int tall;
	GetSize( wide, tall );

	int nInnerCircle = screenTall * 0.15f;
	int nOuterCircle = nInnerCircle * 1.25f;
	vgui::surface()->DrawOutlinedCircle( xMid, yMid, nInnerCircle, 64 );
	vgui::surface()->DrawOutlinedCircle( xMid, yMid, nOuterCircle, 64 );

	// Center reticle, drawn as two half circles
	vgui::surface()->DrawSetColor( 0, 0, 0, 64 );
	vgui::surface()->DrawSetTexture( m_iScopeTexture[ 1 ] );
	const float circleScale = screenTall * 0.05f;
	vert[ 0 ].Init( Vector2D( xMid - circleScale, yMid ), uv12 );
	vert[ 1 ].Init( Vector2D( xMid + circleScale, yMid ), uv22 );
	vert[ 2 ].Init( Vector2D( xMid + circleScale, yMid + circleScale ), uv21 );
	vert[ 3 ].Init( Vector2D( xMid - circleScale, yMid + circleScale ), uv11 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	vgui::surface()->DrawSetColor( 0, 0, 0, 32 );
	vert[ 0 ].Init( Vector2D( xMid - circleScale, yMid - circleScale ), uv21 );
	vert[ 1 ].Init( Vector2D( xMid + circleScale, yMid - circleScale ), uv11 );
	vert[ 2 ].Init( Vector2D( xMid + circleScale, yMid ), uv12 );
	vert[ 3 ].Init( Vector2D( xMid - circleScale, yMid ), uv22 );
	vgui::surface()->DrawTexturedPolygon( 4, vert );

	// Registration marks in each corner
	const float flRegisterWeight = 1.0f;
	const float flRegisterOffset = screenWide * 0.125f;
	const float flRegisterLength = screenWide * 0.1f;

	vgui::surface()->DrawSetColor( 0, 0, 0, 255 );

	// Upper left
	vgui::surface()->DrawFilledRect( flRegisterOffset, flRegisterOffset, flRegisterOffset + flRegisterLength, flRegisterOffset + flRegisterWeight );
	vgui::surface()->DrawFilledRect( flRegisterOffset, flRegisterOffset, flRegisterOffset + flRegisterWeight, flRegisterOffset + flRegisterLength );

	// Lower left
	vgui::surface()->DrawFilledRect( flRegisterOffset, screenTall - flRegisterOffset, flRegisterOffset + flRegisterLength, ( screenTall - flRegisterOffset ) + flRegisterWeight );
	vgui::surface()->DrawFilledRect( flRegisterOffset, screenTall - ( flRegisterOffset + flRegisterLength ), flRegisterOffset + flRegisterWeight, screenTall - flRegisterOffset );

	// Upper right
	vgui::surface()->DrawFilledRect( screenWide - ( flRegisterOffset + flRegisterLength ), flRegisterOffset, screenWide - flRegisterOffset, flRegisterOffset + flRegisterWeight );
	vgui::surface()->DrawFilledRect( screenWide - ( flRegisterOffset + flRegisterWeight ), flRegisterOffset, screenWide - flRegisterOffset, flRegisterOffset + flRegisterLength );

	// Lower right
	vgui::surface()->DrawFilledRect( screenWide - ( flRegisterOffset + flRegisterLength ), screenTall - ( flRegisterOffset + flRegisterWeight ), screenWide - flRegisterOffset, screenTall - flRegisterOffset );
	vgui::surface()->DrawFilledRect( screenWide - ( flRegisterOffset + flRegisterWeight ), screenTall - ( flRegisterOffset + flRegisterLength ), screenWide - flRegisterOffset, screenTall - flRegisterOffset );

	// Blinking "REC" inside the lower right registration mark
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	vgui::HFont hTFont = vgui::scheme()->GetIScheme( scheme )->GetFont( "InstructorTitle", true );

	int nAlpha = 0;
	float flAlpha = sinf( gpGlobals->curtime * 8.0f );
	if ( flAlpha > 0.25f )
	{
		nAlpha = 255;
	}

	vgui::surface()->DrawSetTextFont( hTFont );
	vgui::surface()->DrawSetTextColor( Color( 255, 0, 0, nAlpha ) );

	wchar_t unicode[ 256 ];
	g_pVGuiLocalize->ConvertANSIToUnicode( "REC", unicode, sizeof( unicode ) );

	int nWide, nTall;
	vgui::surface()->GetTextSize( hTFont, unicode, nWide, nTall );
	vgui::surface()->DrawSetTextPos( ( screenWide - flRegisterOffset ) - ( nWide + 32 ), ( screenTall - flRegisterOffset ) - ( nTall + 16 ) );
	vgui::surface()->DrawUnicodeString( unicode );
}
