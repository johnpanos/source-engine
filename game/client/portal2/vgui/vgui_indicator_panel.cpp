//========= Portal 2 reconstruction ============================================//
//
// Purpose: VGUI screen that draws indicator panel checkmarks and countdown pies
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_vguiscreen.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "ienginevgui.h"
#include "c_indicator_panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Indicator panel screen
//-----------------------------------------------------------------------------
class CIndicatorScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CIndicatorScreen, CVGuiScreenPanel );

public:
	CIndicatorScreen( vgui::Panel *parent, const char *panelName );

	virtual void ApplySchemeSettings( IScheme *pScheme );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnTick();
	virtual void Paint( void );

private:
	void	PaintCountdownTimer( void );
	void	PaintIndicator( void );

	int		m_nCheckTextureID;
	int		m_nUncheckedTextureID;

	int		m_nXTextureID;
	int		m_nOTextureID;

	int		m_nTimerBackgroundTextureID;

	// Countdown pie
	int		m_nTimerBackgroundActiveTextureID;
	int		m_nTimerUpperSliceTextureID;
	int		m_nTimerLowerSliceTextureID;

	// Fade up when the countdown expires
	bool	m_bWasCounting;
	float	m_flFadeUpTime;

	// Entities
	CHandle<C_VGuiScreen>			m_hVGUIScreen;
	CHandle<C_LabIndicatorPanel>	m_hScreenEntity;
};

//-----------------------------------------------------------------------------
// Standard VGUI panel for objects
//-----------------------------------------------------------------------------
DECLARE_VGUI_SCREEN_FACTORY( CIndicatorScreen, "indicator_panel" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CIndicatorScreen::CIndicatorScreen( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CIndicatorScreen", vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/indicator_panel.res", "IndicatorPanelScreen" ) ),
	m_bWasCounting( false ),
	m_flFadeUpTime( 0.0f )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CIndicatorScreen::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	if ( !BaseClass::Init( pKeyValues, pInitData ) )
		return false;

	// Find the panel entity that owns our screen
	m_hVGUIScreen = dynamic_cast<C_VGuiScreen *>( GetEntity() );
	if ( m_hVGUIScreen != NULL )
	{
		m_hScreenEntity = dynamic_cast<C_LabIndicatorPanel *>( m_hVGUIScreen->GetOwnerEntity() );
	}

	//
	// Load our textures
	//

	// Countdown timer
	m_nTimerBackgroundTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTimerBackgroundTextureID, "signage/countdown_background", true, false );

	m_nTimerBackgroundActiveTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTimerBackgroundActiveTextureID, "signage/countdown_background_on", true, false );

	// Pie slices
	m_nTimerUpperSliceTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTimerUpperSliceTextureID, "signage/countdown_slice1", true, false );

	m_nTimerLowerSliceTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nTimerLowerSliceTextureID, "signage/countdown_slice2", true, false );

	// Indicator states
	m_nCheckTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nCheckTextureID, "signage/indicator_checked", true, false );

	m_nUncheckedTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nUncheckedTextureID, "signage/indicator_unchecked", true, false );

	m_nXTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nXTextureID, "signage/shape04", true, false );

	m_nOTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nOTextureID, "signage/shape05", true, false );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CIndicatorScreen::ApplySchemeSettings( IScheme *pScheme )
{
}

//-----------------------------------------------------------------------------
// Update the display string
//-----------------------------------------------------------------------------
void CIndicatorScreen::OnTick()
{
	BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: Draw the countdown as eight pie sections, one per 1/8th of the
//			duration, sweeping counter-clockwise from the top.
//-----------------------------------------------------------------------------
void CIndicatorScreen::PaintCountdownTimer( void )
{
	const int NUM_PIE_SECTIONS = 8;

	// Figure out how many sections are lit and how far the current one has faded
	float flPerc = m_hScreenEntity->GetCountdownPercentage() * NUM_PIE_SECTIONS;
	int nWhole = ceil( flPerc );
	float flFraction = nWhole - flPerc;
	float flAlpha = RemapValClamped( flFraction, 0.0f, 1.0f, 255.0f, 0.0f );

	int cx = GetWide() / 2;
	int cy = GetTall() / 2;

	vgui::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );

	bool bTimedOut = ( flPerc <= FLT_EPSILON );

	if ( m_hScreenEntity->IsCountingDown() && bTimedOut == false )
	{
		vgui::surface()->DrawSetTexture( m_nTimerBackgroundActiveTextureID );
	}
	else
	{
		vgui::surface()->DrawSetTexture( m_nTimerBackgroundTextureID );
	}

	vgui::surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	float flGlobalAlpha = 255.0f;
	if ( bTimedOut )
	{
		// Restart our fade the moment the timer ran out
		if ( m_bWasCounting )
		{
			m_flFadeUpTime = gpGlobals->curtime;
			m_bWasCounting = false;
		}

		// Fade the whole pie back up once it's expired
		flGlobalAlpha = RemapValClamped( gpGlobals->curtime, m_flFadeUpTime, m_flFadeUpTime + 0.25f, 0.0f, 255.0f );

		// Reconstruction note: when timed out the binary draws every section with
		// the fade alpha, including the partial top-right one, so both alphas match.
		flAlpha = flGlobalAlpha;
		flPerc = NUM_PIE_SECTIONS;
	}
	else
	{
		m_bWasCounting = true;
	}

	// Section 8 (top right, upper)
	if ( flPerc > 7.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerUpperSliceTextureID );
		vgui::surface()->DrawTexturedRect( cx, 0, cx + GetWide() / 2, GetTall() / 2 );
	}

	// Section 7 (top right, lower)
	if ( flPerc > 6.0f && flPerc < 7.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedRect( cx, 0, cx + GetWide() / 2, GetTall() / 2 );
	}
	else if ( flPerc > 7.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flGlobalAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedRect( cx, 0, cx + GetWide() / 2, GetTall() / 2 );
	}

	// Section 6 (bottom right, upper)
	if ( flPerc > 5.0f && flPerc < 6.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( cx, cy, cx + GetWide() / 2, cy + GetTall() / 2, 0.0f, 1.0f, 1.0f, 0.0f );
	}
	else if ( flPerc > 6.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flGlobalAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( cx, cy, cx + GetWide() / 2, cy + GetTall() / 2, 0.0f, 1.0f, 1.0f, 0.0f );
	}

	// Section 5 (bottom right, lower)
	if ( flPerc > 4.0f && flPerc < 5.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerUpperSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( cx, cy, cx + GetWide() / 2, cy + GetTall() / 2, 0.0f, 1.0f, 1.0f, 0.0f );
	}
	else if ( flPerc > 5.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flGlobalAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerUpperSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( cx, cy, cx + GetWide() / 2, cy + GetTall() / 2, 0.0f, 1.0f, 1.0f, 0.0f );
	}

	// Section 4 (bottom left, lower)
	if ( flPerc > 3.0f && flPerc < 4.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerUpperSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, cy, cx, cy + GetTall() / 2, 1.0f, 1.0f, 0.0f, 0.0f );
	}
	else if ( flPerc > 4.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flGlobalAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerUpperSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, cy, cx, cy + GetTall() / 2, 1.0f, 1.0f, 0.0f, 0.0f );
	}

	// Section 3 (bottom left, upper)
	if ( flPerc > 2.0f && flPerc < 3.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, cy, cx, cy + GetTall() / 2, 1.0f, 1.0f, 0.0f, 0.0f );
	}
	else if ( flPerc > 3.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flGlobalAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, cy, cx, cy + GetTall() / 2, 1.0f, 1.0f, 0.0f, 0.0f );
	}

	// Section 2 (top left, lower)
	if ( flPerc > 1.0f && flPerc < 2.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, 0, cx, GetTall() / 2, 1.0f, 0.0f, 0.0f, 1.0f );
	}
	else if ( flPerc > 2.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flGlobalAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerLowerSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, 0, cx, GetTall() / 2, 1.0f, 0.0f, 0.0f, 1.0f );
	}

	// Section 1 (top left, upper)
	if ( flPerc > 0.0f && flPerc < 1.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerUpperSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, 0, cx, GetTall() / 2, 1.0f, 0.0f, 0.0f, 1.0f );
	}
	else if ( flPerc > 1.0f )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, flGlobalAlpha ) );
		vgui::surface()->DrawSetTexture( m_nTimerUpperSliceTextureID );
		vgui::surface()->DrawTexturedSubRect( 0, 0, cx, GetTall() / 2, 1.0f, 0.0f, 0.0f, 1.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw the checked / unchecked or tic-tac-toe state
//-----------------------------------------------------------------------------
void CIndicatorScreen::PaintIndicator( void )
{
	int nTextureID;

	switch ( m_hScreenEntity->GetPlayerPinged() )
	{
	case 0:
		// Unclaimed tic-tac-toe square
		vgui::surface()->DrawSetColor( Color( 40, 40, 40, 255 ) );
		vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
		return;

	case 1:
		nTextureID = m_nXTextureID;
		break;

	case 2:
		nTextureID = m_nOTextureID;
		break;

	default:
		nTextureID = ( m_hScreenEntity->IsChecked() ) ? m_nCheckTextureID : m_nUncheckedTextureID;
		break;
	}

	vgui::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
	vgui::surface()->DrawSetTexture( nTextureID );
	vgui::surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CIndicatorScreen::Paint( void )
{
	BaseClass::Paint();

	if ( m_hScreenEntity == NULL )
		return;

	if ( m_hScreenEntity->IsCountdownTimer() )
	{
		PaintCountdownTimer();
	}
	else
	{
		PaintIndicator();
	}
}
