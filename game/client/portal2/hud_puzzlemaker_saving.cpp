//========= Portal 2 reconstruction ============================================//
//
// Purpose: Puzzle Maker save progress HUD.
//
// Reconstructed from the installed Portal 2 client binary. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#if defined( PORTAL2_PUZZLEMAKER )
#include "puzzlemaker/puzzlemaker.h"
#endif
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>

#include "tier0/memdbgon.h"

class CHUDPuzzleMakerSaving : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHUDPuzzleMakerSaving, vgui::EditablePanel );

public:
	CHUDPuzzleMakerSaving( const char *pElementName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;
	bool ShouldDraw() OVERRIDE;

private:
	vgui::ImagePanel *m_pSavingSpinner;
};

DECLARE_HUDELEMENT( CHUDPuzzleMakerSaving );

CHUDPuzzleMakerSaving::CHUDPuzzleMakerSaving( const char *pElementName )
    : BaseClass( NULL, "PuzzleMakerSaving" ), CHudElement( pElementName )
{
	SetParent( g_pClientMode->GetViewport() );
	m_pSavingSpinner = new vgui::ImagePanel( this, "SavingSpinner" );
}

void CHUDPuzzleMakerSaving::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "resource/ui/hud_puzzlemaker_saving.res" );
}

bool CHUDPuzzleMakerSaving::ShouldDraw()
{
#if defined( PORTAL2_PUZZLEMAKER )
	return g_pPuzzleMaker && g_pPuzzleMaker->IsSaving() && CHudElement::ShouldDraw();
#else
	return false;
#endif
}
