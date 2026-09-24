//========= Portal 2 reconstruction ============================================//
//
// Purpose: Off-screen arrow pointing toward the partner's co-op ping location
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
#include "view_scene.h"
#include "c_portal_player.h"
#include "portal_util_shared.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sv_portal_coop_ping_hud_indicitator_duration;

ConVar cl_coop_ping_indicator_scale( "cl_coop_ping_indicator_scale", "64" );


class CHudCoopPingIndicator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudCoopPingIndicator, vgui::Panel );

public:
	CHudCoopPingIndicator( const char *pElementName );
	virtual void Init( void );
	virtual void LevelInit( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );

	void MsgFunc_HudPingIndicator( bf_read &msg );

private:
	void DrawIndicatorHint( void );

	float m_flStartDisplayTime;
	Vector m_vecPingLocation;

	int m_nArrowTexture;
};


DECLARE_HUDELEMENT( CHudCoopPingIndicator );
DECLARE_HUD_MESSAGE( CHudCoopPingIndicator, HudPingIndicator );


CHudCoopPingIndicator::CHudCoopPingIndicator( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "CHudCoopPingIndicator" )
{
	vgui::Panel *pParent = GetClientMode()->GetViewport();
	SetParent( pParent );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudCoopPingIndicator::Init( void )
{
	m_nArrowTexture = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nArrowTexture, "HUD/hud_icon_arrow", true, false );

	HOOK_HUD_MESSAGE( CHudCoopPingIndicator, HudPingIndicator );
}

//-----------------------------------------------------------------------------
// Purpose: Start the level with the indicator already timed out
//-----------------------------------------------------------------------------
void CHudCoopPingIndicator::LevelInit( void )
{
	m_flStartDisplayTime = -1.0f - sv_portal_coop_ping_hud_indicitator_duration.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudCoopPingIndicator::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: Only draw in multiplayer while a recent ping is being displayed
//-----------------------------------------------------------------------------
bool CHudCoopPingIndicator::ShouldDraw( void )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return false;

	if ( !GameRules()->IsMultiplayer() )
		return false;

	if ( gpGlobals->curtime > m_flStartDisplayTime + sv_portal_coop_ping_hud_indicitator_duration.GetFloat() )
		return false;

	return ( CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudCoopPingIndicator::Paint( void )
{
	DrawIndicatorHint();
}

//-----------------------------------------------------------------------------
// Purpose: Draw an arrow between the screen center and the ping, pinned inside
//			a circle around the center and fading in as it nears the circle
//-----------------------------------------------------------------------------
void CHudCoopPingIndicator::DrawIndicatorHint( void )
{
	if ( m_vecPingLocation == vec3_invalid )
		return;

	Vector vecScreen;
	ScreenTransform( m_vecPingLocation, vecScreen );

	float xCenter = ScreenWidth() / 2;
	float yCenter = ScreenHeight() / 2;

	float x = xCenter + ( 0.5 * vecScreen[ 0 ] * ScreenWidth() + 0.5 );
	float y = yCenter - ( 0.5 * vecScreen[ 1 ] * ScreenHeight() + 0.5 );

	// Offset from the screen center
	x -= xCenter;
	y -= yCenter;

	float flDist = sqrt( x * x + y * y );

	// Keep the arrow within a circle around the center of the screen
	const float flInnerCircle = 0.5f * ( ScreenWidth() < ScreenHeight() ? ScreenWidth() : ScreenHeight() ) * 0.9f;
	float flAng = atan2( y, x );

	float sa, ca;
	SinCos( flAng, &sa, &ca );

	float flRadius = 0.5f * flDist;
	if ( flRadius >= flInnerCircle )
	{
		flRadius = flInnerCircle;
	}

	const float flScale = cl_coop_ping_indicator_scale.GetFloat();

	vgui::Vertex_t verts[ 4 ];

	Vector2D center( ScreenWidth() / 2 + ca * flRadius, ScreenHeight() / 2 + sa * flRadius );

	// Arrow points along the direction to the ping
	Vector2D axis[ 2 ];
	axis[ 0 ].Init( ca, sa );
	axis[ 1 ].Init( -sa, ca );

	verts[ 0 ].m_TexCoord.Init( 0, 0 );
	Vector2DMA( center, -0.5f * flScale, axis[ 0 ], verts[ 0 ].m_Position );
	Vector2DMA( verts[ 0 ].m_Position, -0.5f * flScale, axis[ 1 ], verts[ 0 ].m_Position );

	verts[ 1 ].m_TexCoord.Init( 1, 0 );
	Vector2DMA( verts[ 0 ].m_Position, flScale, axis[ 0 ], verts[ 1 ].m_Position );

	verts[ 2 ].m_TexCoord.Init( 1, 1 );
	Vector2DMA( verts[ 1 ].m_Position, flScale, axis[ 1 ], verts[ 2 ].m_Position );

	verts[ 3 ].m_TexCoord.Init( 0, 1 );
	Vector2DMA( verts[ 0 ].m_Position, flScale, axis[ 1 ], verts[ 3 ].m_Position );

	// Fade in as the ping moves out toward the edge of the circle
	Color color = UTIL_Portal_Color( 0, 0 );
	color[ 3 ] = RemapValClamped( flDist, flInnerCircle * 0.5f, flInnerCircle, 0.0f, 128.0f );

	vgui::surface()->DrawSetColor( color );
	vgui::surface()->DrawSetTexture( m_nArrowTexture );
	vgui::surface()->DrawTexturedPolygon( 4, verts );
}

//-----------------------------------------------------------------------------
// Purpose: Our partner pinged a location
//-----------------------------------------------------------------------------
void CHudCoopPingIndicator::MsgFunc_HudPingIndicator( bf_read &msg )
{
	C_Portal_Player *pPlayer = C_Portal_Player::GetLocalPortalPlayer();
	if ( !pPlayer )
		return;

	m_flStartDisplayTime = gpGlobals->curtime;

	m_vecPingLocation.x = msg.ReadFloat();
	m_vecPingLocation.y = msg.ReadFloat();
	m_vecPingLocation.z = msg.ReadFloat();
}
