//========= Portal 2 reconstruction ============================================//
//
// Purpose: Button and icon state for the cooperative radial menu.
//
//=============================================================================//

#ifndef RADIALBUTTON_H
#define RADIALBUTTON_H

#include "polygonbutton.h"
#include <vgui_controls/ImagePanel.h>

class CRadialButton : public CPolygonButton
{
	DECLARE_CLASS_SIMPLE( CRadialButton, CPolygonButton );

public:
	CRadialButton( vgui::Panel *parent, const char *panelName );

	void SetImage( const char *pImageName );
	vgui::ImagePanel *GetIcon() const { return m_pIcon; }

	void SetPassthru( CRadialButton *pButton ) { m_pPassthru = pButton; }
	CRadialButton *GetPassthru() const { return m_pPassthru; }

	void SetFakeArmed( bool bArmed ) { m_bFakeArmed = bArmed; }
	bool IsFakeArmed() const { return m_bFakeArmed; }
	void SetChosen( bool bChosen ) { m_bChosen = bChosen; }
	void SetPulse( bool bPulse ) { m_bPulse = bPulse; }
	void SetMaxScale( float flScale );
	void SetGLaDOSResponse( int nResponse ) { m_nGLaDOSResponse = nResponse; }
	int GetGLaDOSResponse() const { return m_nGLaDOSResponse; }
	void ShowSubmenuIndicator( bool bShow );
	void GetHotspotBounds( int *pMinX, int *pMinY, int *pMaxX, int *pMaxY ) const;

	void PerformLayout() OVERRIDE;
	void OnThink() OVERRIDE;

private:
	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pSubmenuIndicator;
	CRadialButton *m_pPassthru;
	bool m_bFakeArmed;
	bool m_bChosen;
	bool m_bPulse;
	float m_flMaxScale;
	int m_nGLaDOSResponse;
};

#endif // RADIALBUTTON_H
