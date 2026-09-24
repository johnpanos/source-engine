//========= Portal 2 reconstruction ============================================//
//
// Purpose: Interactive polygon button for the cooperative radial menu.
//
// The retained retail client names CRadialButton, RadialButtonImage and
// SubmenuHotspot. The existing radial menu supplies the control contract.
// Not original Valve source; the repository's provenance and distribution
// warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "radialbutton.h"
#include "vgui/IImage.h"
#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRadialButton );

CRadialButton::CRadialButton( vgui::Panel *parent, const char *panelName )
    : BaseClass( parent, panelName ), m_pIcon( new vgui::ImagePanel( this, "RadialButtonImage" ) ),
      m_pSubmenuIndicator( new vgui::ImagePanel( this, "SubmenuHotspot" ) ), m_pPassthru( NULL ),
      m_bFakeArmed( false ), m_bChosen( false ), m_bPulse( false ), m_flMaxScale( 1.0f ),
      m_nGLaDOSResponse( 0 )
{
	m_pIcon->SetShouldScaleImage( true );
	m_pIcon->SetMouseInputEnabled( false );
	m_pSubmenuIndicator->SetMouseInputEnabled( false );
	m_pSubmenuIndicator->SetVisible( false );
}

void CRadialButton::SetImage( const char *pImageName )
{
	m_pIcon->SetImage( pImageName && pImageName[0] ? pImageName : NULL );
	InvalidateLayout();
}

void CRadialButton::SetMaxScale( float flScale )
{
	flScale = clamp( flScale, 0.0f, 1.0f );
	if ( m_flMaxScale != flScale )
	{
		m_flMaxScale = flScale;
		InvalidateLayout();
	}
}

void CRadialButton::ShowSubmenuIndicator( bool bShow )
{
	m_pSubmenuIndicator->SetVisible( bShow );
}

void CRadialButton::GetHotspotBounds( int *pMinX, int *pMinY, int *pMaxX, int *pMaxY ) const
{
	if ( pMinX )
		*pMinX = m_hotspotMins.x;
	if ( pMinY )
		*pMinY = m_hotspotMins.y;
	if ( pMaxX )
		*pMaxX = m_hotspotMaxs.x;
	if ( pMaxY )
		*pMaxY = m_hotspotMaxs.y;
}

void CRadialButton::PerformLayout()
{
	BaseClass::PerformLayout();

	vgui::IImage *pImage = m_pIcon->GetImage();
	if ( !pImage )
	{
		m_pIcon->SetSize( 0, 0 );
		return;
	}

	int nWide, nTall;
	pImage->GetSize( nWide, nTall );
	nWide *= m_flMaxScale;
	nTall *= m_flMaxScale;
	m_pIcon->SetSize( nWide, nTall );
	m_pIcon->SetPos( ( m_hotspotMins.x + m_hotspotMaxs.x - nWide ) / 2,
	    ( m_hotspotMins.y + m_hotspotMaxs.y - nTall ) / 2 );

	int nIndicatorWide, nIndicatorTall;
	m_pSubmenuIndicator->GetSize( nIndicatorWide, nIndicatorTall );
	m_pSubmenuIndicator->SetPos(
	    m_hotspotMaxs.x - nIndicatorWide, m_hotspotMaxs.y - nIndicatorTall );
}

void CRadialButton::OnThink()
{
	BaseClass::OnThink();
	int nAlpha = m_bChosen || m_bFakeArmed || IsArmed() ? 255 : 200;
	if ( m_bPulse && gpGlobals )
	{
		nAlpha = 200 + 55 * ( 0.5f + 0.5f * sinf( gpGlobals->curtime * 6.0f ) );
	}
	m_pIcon->SetDrawColor( Color( 255, 255, 255, nAlpha ) );
}
