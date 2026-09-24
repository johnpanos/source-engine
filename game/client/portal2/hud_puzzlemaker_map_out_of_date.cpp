//========= Portal 2 reconstruction ============================================//
//
// Purpose: Warn when the playable chamber is older than the current puzzle.
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
#include <vgui_controls/Label.h>

#include "tier0/memdbgon.h"

class CHUDPuzzleMakerMapOutOfDate : public vgui::EditablePanel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHUDPuzzleMakerMapOutOfDate, vgui::EditablePanel );

public:
	CHUDPuzzleMakerMapOutOfDate( const char *pElementName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;
	bool ShouldDraw() OVERRIDE;

private:
	vgui::Label *m_pTitleLabel;
	vgui::Label *m_pDescriptionLabel;
};

DECLARE_HUDELEMENT( CHUDPuzzleMakerMapOutOfDate );

CHUDPuzzleMakerMapOutOfDate::CHUDPuzzleMakerMapOutOfDate( const char *pElementName )
    : BaseClass( NULL, "PuzzleMakerMapOutOfDate" ), CHudElement( pElementName )
{
	SetParent( g_pClientMode->GetViewport() );
	m_pTitleLabel = new vgui::Label( this, "PuzzleMakerMapOutOfDateTitleLabel", "" );
	m_pDescriptionLabel = new vgui::Label( this, "PuzzleMakerMapOutOfDateDescriptionLabel", "" );
}

void CHUDPuzzleMakerMapOutOfDate::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "resource/ui/hud_puzzlemaker_map_out_of_date.res" );
}

bool CHUDPuzzleMakerMapOutOfDate::ShouldDraw()
{
#if defined( PORTAL2_PUZZLEMAKER )
	return g_pPuzzleMaker && g_pPuzzleMaker->GetActive() && !g_pPuzzleMaker->IsVisible() &&
	       g_pPuzzleMaker->HasUncompiledChanges() && CHudElement::ShouldDraw();
#else
	return false;
#endif
}
