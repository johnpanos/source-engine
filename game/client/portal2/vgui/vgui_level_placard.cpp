//========= Portal 2 reconstruction ============================================//
//
// Purpose: VGUI screen panel drawn on the level placard sign.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_vguiscreen.h"
#include "vgui_controls/Label.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "ienginevgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Control screen
//-----------------------------------------------------------------------------
class CLevelPlacardScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CLevelPlacardScreen, CVGuiScreenPanel );

public:
	CLevelPlacardScreen( vgui::Panel *parent, const char *panelName );
	virtual ~CLevelPlacardScreen();

	virtual void ApplySchemeSettings( IScheme *pScheme );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnTick();
	virtual void Paint();

private:
	bool	IsActive( void );

private:

	// The vgui screen entity that owns this panel
	CHandle<C_VGuiScreen>	m_hVGUIScreen;

	int		m_nTextureId;

	// Placard text
	vgui::Label *m_pDisplayTextLabel;
};


DECLARE_VGUI_SCREEN_FACTORY( CLevelPlacardScreen, "level_placard_screen" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CLevelPlacardScreen::CLevelPlacardScreen( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CLevelPlacardScreen", vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/LevelPlacardScreen.res", "LevelPlacardScreen" ) )
{
	m_pDisplayTextLabel = new vgui::Label( this, "NumberDisplay", "testing!" );
	m_nTextureId = -1;
}

//-----------------------------------------------------------------------------
// Destructor:
//-----------------------------------------------------------------------------
CLevelPlacardScreen::~CLevelPlacardScreen()
{
	g_pMatSystemSurface->DestroyTextureID( m_nTextureId );
}

void CLevelPlacardScreen::ApplySchemeSettings( IScheme *pScheme )
{
	m_pDisplayTextLabel->SetFgColor( Color( 255, 255, 255, 255 ) );
	m_pDisplayTextLabel->SetText( "" );
	// Reconstruction note: the third call is Panel vtable slot 34 with a false
	// argument; SetVisible( false ) is the plausible match for that slot.
	m_pDisplayTextLabel->SetVisible( false );
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CLevelPlacardScreen::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	if ( !BaseClass::Init( pKeyValues, pInitData ) )
		return false;

	// Save this for simplicity later on
	m_hVGUIScreen = dynamic_cast<C_VGuiScreen *>( GetEntity() );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the owning vgui screen is active
//-----------------------------------------------------------------------------
bool CLevelPlacardScreen::IsActive( void )
{
	bool bScreenActive = false;
	if ( m_hVGUIScreen != NULL )
	{
		bScreenActive = m_hVGUIScreen->IsActive();
	}

	return bScreenActive;
}

//-----------------------------------------------------------------------------
// Update the display string
//-----------------------------------------------------------------------------
void CLevelPlacardScreen::OnTick()
{
	BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: Clear the placard to black and draw the children over it
//-----------------------------------------------------------------------------
void CLevelPlacardScreen::Paint( void )
{
	vgui::surface()->DrawSetColor( 0, 0, 0, 255 );
	vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );

	BaseClass::Paint();
}
