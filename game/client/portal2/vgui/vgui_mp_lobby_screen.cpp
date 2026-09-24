//========= Portal 2 reconstruction ============================================//
//
// Purpose: Co-op lobby vgui screens: day select, progress sign, level select
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_vguiscreen.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/AnimationController.h"
#include <vgui/IVGui.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include "ienginevgui.h"
#include "c_portal_player.h"
#include "portal_mp_gamerules.h"
#include "c_mp_lobby_screen.h"
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Base class for all vgui screens attached to an owning object
//-----------------------------------------------------------------------------
class CObjectControlPanel : public CVGuiScreenPanel
{
	DECLARE_CLASS( CObjectControlPanel, CVGuiScreenPanel );
public:
	CObjectControlPanel( vgui::Panel *parent, const char *panelName );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnCommand( const char *command );
	virtual void OnTick();

protected:
	vgui::Panel *GetActivePanel() { return m_pActivePanel; }

	// Override these to deal with various controls in various states
	virtual void OnTickActive( C_BaseEntity *pObj, C_Portal_Player *pLocalPlayer );

	C_BaseEntity *GetOwningObject() const;

	// Selects the current panel
	virtual vgui::Panel *TickCurrentPanel();

	// Sends a message to the server object
	void SendToServerObject( const char *pMsg );

private:
	// The panel which is active while the owning object is valid
	vgui::EditablePanel *m_pActivePanel;

	vgui::Panel *m_pCurrentPanel;
};


//-----------------------------------------------------------------------------
// This is a panel that passes its commands on to its parent
//-----------------------------------------------------------------------------
class CCommandChainingPanel : public vgui::EditablePanel
{
	typedef vgui::EditablePanel BaseClass;

public:
	CCommandChainingPanel( vgui::Panel *parent, const char *panelName ) :
		BaseClass( parent, panelName )
	{
		SetPaintBackgroundEnabled( false );
	}

	void OnCommand( const char *command )
	{
		BaseClass::OnCommand( command );
		if ( GetParent() )
		{
			GetParent()->OnCommand( command );
		}
	}
};


DECLARE_VGUI_SCREEN_FACTORY( CObjectControlPanel, "object_control_panel" );


//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CObjectControlPanel::CObjectControlPanel( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName, 0 )
{
	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( true );

	// Make some high-level panels to group panels
	m_pActivePanel = new CCommandChainingPanel( this, "ActivePanel" );

	SetCursor( vgui::dc_arrow );

	m_pActivePanel->SetZPos( -1 );
}


//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CObjectControlPanel::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	if ( !BaseClass::Init( pKeyValues, pInitData ) )
		return false;

	SetCursor( vgui::dc_arrow );

	// Make the active panel fill the screen
	int x, y, w, h;
	GetBounds( x, y, w, h );
	m_pActivePanel->SetBounds( x, y, w, h );

	m_pActivePanel->SetVisible( false );
	m_pCurrentPanel = m_pActivePanel;

	return true;
}


//-----------------------------------------------------------------------------
// Returns the object it's attached to
//-----------------------------------------------------------------------------
C_BaseEntity *CObjectControlPanel::GetOwningObject() const
{
	C_BaseEntity *pScreenEnt = GetEntity();
	if ( !pScreenEnt )
		return NULL;

	C_BaseEntity *pObj = pScreenEnt->GetOwnerEntity();
	if ( !pObj )
		return NULL;

	return pObj;
}


//-----------------------------------------------------------------------------
// Ticks the active panel
//-----------------------------------------------------------------------------
void CObjectControlPanel::OnTickActive( C_BaseEntity *pObj, C_Portal_Player *pLocalPlayer )
{
}


//-----------------------------------------------------------------------------
// Selects the current panel
//-----------------------------------------------------------------------------
vgui::Panel *CObjectControlPanel::TickCurrentPanel()
{
	C_Portal_Player *pLocalPlayer = C_Portal_Player::GetLocalPortalPlayer();
	C_BaseEntity *pObj = GetOwningObject();

	m_pCurrentPanel = GetActivePanel();
	m_pCurrentPanel->SetCursor( vgui::dc_arrow );
	OnTickActive( pObj, pLocalPlayer );
	return m_pCurrentPanel;
}

//-----------------------------------------------------------------------------
// Sends a message to the server object
//-----------------------------------------------------------------------------
void CObjectControlPanel::SendToServerObject( const char *pMsg )
{
	C_BaseEntity *pObj = GetOwningObject();
	if ( pObj )
	{
		engine->ServerCmd( pMsg );
	}
}

//-----------------------------------------------------------------------------
// Frame-based update
//-----------------------------------------------------------------------------
void CObjectControlPanel::OnTick()
{
	BaseClass::OnTick();

	C_BaseEntity *pObj = GetOwningObject();
	if ( !pObj )
		return;

	Assert( m_pCurrentPanel );

	m_pCurrentPanel->SetVisible( false );
	m_pCurrentPanel = TickCurrentPanel();
	m_pCurrentPanel->SetVisible( true );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CObjectControlPanel::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );
}


//-----------------------------------------------------------------------------
// Co-op lobby day select screen
//-----------------------------------------------------------------------------
class CVGUI_MP_LobbyScreen : public CObjectControlPanel
{
	DECLARE_CLASS( CVGUI_MP_LobbyScreen, CObjectControlPanel );

public:
	CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName );

	virtual void ApplySchemeSettings( IScheme *pScheme );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnTick();
	virtual void OnCommand( const char *pCommand );

private:
	void Update( C_MP_LobbyScreen *pLobbyScreen );
	void UpdateDayLabel( void );

	void NextDay( void );
	void PreviousDay( void );
	void SelectDay( void );

private:
	vgui::ImagePanel	*m_pBackgroundImage;
	vgui::Label			*m_pDayTextLabel;

	Color	m_cDefault;
	Color	m_cInvisible;

	bool	bIsAlreadyVisible;

	// Days are 1-based; the last one is the experimental course
	int		m_nStartDay;
	int		m_nCurrentDay;
	int		m_nMaxDays;
};


DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LobbyScreen, "mp_lobby_screen" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CVGUI_MP_LobbyScreen::CVGUI_MP_LobbyScreen( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CVGUI_MP_LobbyScreen" ), m_cDefault( 0, 0, 0, 0 ), m_cInvisible( 0, 0, 0, 0 ),
	  m_nStartDay( 4 ), m_nCurrentDay( 4 ), m_nMaxDays( 4 )
{
}

//-----------------------------------------------------------------------------
// Purpose: Show the currently selected day
//-----------------------------------------------------------------------------
void CVGUI_MP_LobbyScreen::UpdateDayLabel( void )
{
	char szDayName[128];
	if ( g_pGameRules->IsMultiplayer() && PortalMPGameRules()->IsCoOp() )
	{
		// Reconstruction note: the 2010 build reads m_nCoopSectionIndex directly; the
		// imported game rules keep it private behind GetCoopSection().
		m_nCurrentDay = PortalMPGameRules()->GetCoopSection();
	}

	if ( m_nCurrentDay == 4 )
	{
		V_snprintf( szDayName, sizeof( szDayName ), "Experimental" );
	}
	else
	{
		V_snprintf( szDayName, sizeof( szDayName ), "Day %d", m_nCurrentDay );
	}

	if ( m_pDayTextLabel )
	{
		m_pDayTextLabel->SetText( szDayName );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CVGUI_MP_LobbyScreen::ApplySchemeSettings( IScheme *pScheme )
{
	assert( pScheme );

	m_cDefault = pScheme->GetColor( "CVGUI_MP_LobbyScreen_Default", GetFgColor() );
	m_cInvisible = Color( 0, 0, 0, 0 );

	m_pDayTextLabel = (vgui::Label *)FindChildByName( "DayTitle" );
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CVGUI_MP_LobbyScreen::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	return BaseClass::Init( pKeyValues, pInitData );
}

//-----------------------------------------------------------------------------
// Update the display string
//-----------------------------------------------------------------------------
void CVGUI_MP_LobbyScreen::OnTick()
{
	BaseClass::OnTick();

	if ( !bIsAlreadyVisible )
	{
		SetVisible( true );
		bIsAlreadyVisible = true;
	}

	UpdateDayLabel();
}

void CVGUI_MP_LobbyScreen::Update( C_MP_LobbyScreen *pLobbyScreen )
{
}

void CVGUI_MP_LobbyScreen::NextDay( void )
{
	m_nCurrentDay++;
	if ( m_nCurrentDay > m_nMaxDays )
	{
		m_nCurrentDay = 1;
	}

	UpdateDayLabel();
}

void CVGUI_MP_LobbyScreen::PreviousDay( void )
{
	m_nCurrentDay--;
	if ( m_nCurrentDay < 1 )
	{
		m_nCurrentDay = m_nMaxDays;
	}

	UpdateDayLabel();
}

void CVGUI_MP_LobbyScreen::SelectDay( void )
{
	char szServerMsg[32];
	V_snprintf( szServerMsg, sizeof( szServerMsg ), "lobby_select_day %d", m_nCurrentDay );
	SendToServerObject( szServerMsg );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CVGUI_MP_LobbyScreen::OnCommand( const char *pCommand )
{
	if ( !Q_stricmp( pCommand, "DayNext" ) )
	{
		NextDay();
		SelectDay();
	}
	else if ( !Q_stricmp( pCommand, "DayPrev" ) )
	{
		PreviousDay();
		SelectDay();
	}

	BaseClass::OnCommand( pCommand );
}


#define MP_PROGRESS_ICON_COUNT	16

//-----------------------------------------------------------------------------
// Co-op progress sign: shows the current track and level in that track
//-----------------------------------------------------------------------------
class CVGUI_MP_ProgressSignScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CVGUI_MP_ProgressSignScreen, CVGuiScreenPanel );

public:
	CVGUI_MP_ProgressSignScreen( vgui::Panel *parent, const char *panelName );

	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void PerformLayout( void );
	virtual void Paint( void );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnTick();

protected:
	virtual void UpdateLevelLabel( void );

	int m_nDay;

private:
	vgui::Label			*m_pBranchNumLabel;
	vgui::Label			*m_pLevelProgressNumLabel;
	vgui::ImagePanel	*m_pLevelIconPanels[MP_PROGRESS_ICON_COUNT];
	bool	bIsAlreadyVisible;
	bool	m_bInitializedLevelLabel;
	int		m_nBackgroundTextureID;
	int		m_nNumberTextureID;

	int		m_nCurrentLevel;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_ProgressSignScreen, "mp_progress_sign" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CVGUI_MP_ProgressSignScreen" ), m_nDay( 1 ), m_bInitializedLevelLabel( false ), m_nCurrentLevel( 0 )
{
	for ( int i = 0; i < MP_PROGRESS_ICON_COUNT; i++ )
	{
		m_pLevelIconPanels[i] = new vgui::ImagePanel( this, VarArgs( "LevelIcon%d", i ) );
	}
}

void CVGUI_MP_ProgressSignScreen::Paint( void )
{
	int w, h;
	GetSize( w, h );

	// Background
	vgui::surface()->DrawSetColor( Color( 140, 140, 140, 255 ) );
	vgui::surface()->DrawSetTexture( m_nBackgroundTextureID );
	vgui::surface()->DrawTexturedRect( 0, 0, w, h );

	int nNumHeight = h * 0.4f;
	int nNumWidth = nNumHeight / 3;

	const int nNumPerLine = 6;
	const float flNumWidthFrac = 1.0f / nNumPerLine;

	// Tens digit (always zero)
	vgui::surface()->DrawSetTexture( m_nNumberTextureID );
	vgui::surface()->DrawTexturedSubRect( ( w / 2 ) - nNumWidth, nNumHeight / 4, w / 2, ( nNumHeight / 4 ) + nNumHeight, 0.0f, 0.0f, flNumWidthFrac, 0.5f );

	// Ones digit
	// Reconstruction note: the binary moves to the second row of the number sheet for
	// levels above 4 while indexing that row from nNumPerLine; kept as compiled.
	float flNumY = 0.0f;
	int nMod = 0;
	if ( m_nCurrentLevel > 4 )
	{
		flNumY = 0.5f;
		nMod = nNumPerLine;
	}

	float flNumX = ( m_nCurrentLevel - nMod ) * flNumWidthFrac;
	vgui::surface()->DrawSetTexture( m_nNumberTextureID );
	vgui::surface()->DrawTexturedSubRect( w / 2, nNumHeight / 4, ( w / 2 ) + nNumWidth, ( nNumHeight / 4 ) + nNumHeight, flNumX, flNumY, flNumX + flNumWidthFrac, flNumY + 0.5f );

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Update the track name, progress and level icons
//-----------------------------------------------------------------------------
void CVGUI_MP_ProgressSignScreen::UpdateLevelLabel( void )
{
	if ( g_pGameRules->IsMultiplayer() && PortalMPGameRules()->IsCoOp() && m_bInitializedLevelLabel )
	{
		int nDay = PortalMPGameRules()->GetCoopSection();
		if ( nDay <= 0 )
			return;

		int nTotalLevels = PortalMPGameRules()->GetBranchTotalLevelCount( nDay - 1 );

		int nLevel = PortalMPGameRules()->GetCoopBranchLevel( nDay - 1 );
		if ( nLevel == m_nCurrentLevel )
		{
			m_nDay = 1;
			return;
		}

		m_nDay = nDay;
		m_nCurrentLevel = nLevel;

		char szBranchNumber[128];
		V_snprintf( szBranchNumber, sizeof( szBranchNumber ), "#P2COOP_TrackName_%d", nDay );

		char szProgressNumbers[128];
		V_snprintf( szProgressNumbers, sizeof( szProgressNumbers ), "%d/%d", m_nCurrentLevel, nTotalLevels );

		if ( m_pBranchNumLabel && m_bInitializedLevelLabel )
			m_pBranchNumLabel->SetText( szBranchNumber );

		if ( m_pLevelProgressNumLabel && m_bInitializedLevelLabel )
			m_pLevelProgressNumLabel->SetText( szProgressNumbers );

		int w, h;
		GetSize( w, h );

		for ( int i = 0; i < nTotalLevels; i++ )
		{
			int nBlockSize = 26;
			int nSelectExtra = 6;

			bool bSelected = ( i == ( m_nCurrentLevel - 1 ) );

			// Reconstruction note: the 2010 build reads a per-branch completion table;
			// the imported game rules key completion by player too. Player 0 is used here.
			bool bCompleted = PortalMPGameRules()->IsPlayerLevelInBranchComplete( 0, m_nDay - 1, i );

			if ( bSelected )
			{
				m_pLevelIconPanels[i]->SetImage( "screens/vgui_level_select_icon" );
				m_pLevelIconPanels[i]->SetDrawColor( bCompleted ? Color( 255, 200, 0, 255 ) : Color( 0, 200, 255, 255 ) );
			}
			else
			{
				m_pLevelIconPanels[i]->SetDrawColor( Color( 255, 255, 255, 255 ) );
				if ( bCompleted )
				{
					m_pLevelIconPanels[i]->SetImage( "screens/vgui_level_completed_icon" );
				}
				else
				{
					m_pLevelIconPanels[i]->SetImage( "screens/vgui_level_icon" );
				}
			}

			int nSize = bSelected ? ( nBlockSize + nSelectExtra ) : nBlockSize;
			m_pLevelIconPanels[i]->SetSize( nSize, nSize );

			int x, y;
			m_pLevelIconPanels[i]->GetPos( x, y );
			m_pLevelIconPanels[i]->SetPos( x, bSelected ? 462 - ( nSelectExtra / 2 ) : 462 );

			int new_x = ( w / 2 ) - ( nSize / 2 ) + ( ( i - ( m_nCurrentLevel - 1 ) ) * ( nBlockSize + nSelectExtra ) );
			vgui::GetAnimationController()->RunAnimationCommand( m_pLevelIconPanels[i], "xpos", new_x, 0.0f, 0.25f, vgui::AnimationController::INTERPOLATOR_DEACCEL );
			m_pLevelIconPanels[i]->SetPaintBackgroundEnabled( true );
			m_pLevelIconPanels[i]->SetVisible( true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CVGUI_MP_ProgressSignScreen::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pBranchNumLabel = (vgui::Label *)FindChildByName( "BranchNumLabel" );
	m_pLevelProgressNumLabel = (vgui::Label *)FindChildByName( "LevelProgressNumLabel" );

	if ( m_pBranchNumLabel )
		m_pBranchNumLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	if ( m_pLevelProgressNumLabel )
		m_pLevelProgressNumLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	for ( int i = 0; i < MP_PROGRESS_ICON_COUNT; i++ )
	{
		m_pLevelIconPanels[i]->SetDrawColor( Color( 255, 255, 255, 255 ) );
		m_pLevelIconPanels[i]->SetVisible( false );
	}

	m_bInitializedLevelLabel = true;
}

void CVGUI_MP_ProgressSignScreen::PerformLayout( void )
{
	BaseClass::PerformLayout();

	for ( int i = 0; i < MP_PROGRESS_ICON_COUNT; i++ )
	{
		m_pLevelIconPanels[i]->SetShouldScaleImage( true );
	}
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CVGUI_MP_ProgressSignScreen::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	m_nBackgroundTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nBackgroundTextureID, "vgui/screens/vgui_coop_progress_board", true, false );

	m_nNumberTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_nNumberTextureID, "vgui/screens/vgui_coop_progress_board_numbers", true, false );

	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	return BaseClass::Init( pKeyValues, pInitData );
}

//-----------------------------------------------------------------------------
// Update the display string
//-----------------------------------------------------------------------------
void CVGUI_MP_ProgressSignScreen::OnTick()
{
	BaseClass::OnTick();

	if ( !bIsAlreadyVisible )
	{
		SetVisible( true );
		bIsAlreadyVisible = true;
	}

	UpdateLevelLabel();
}


//-----------------------------------------------------------------------------
// Co-op level select screen for one track (day)
//-----------------------------------------------------------------------------
class CVGUI_MP_LevelSelectScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CVGUI_MP_LevelSelectScreen, CVGuiScreenPanel );

public:
	CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName );

	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void PerformLayout( void );

	virtual bool Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData );
	virtual void OnTick();

protected:
	virtual void UpdateLevelLabel( void );

	int m_nDay;

private:
	vgui::Label			*m_pBranchTitleLabel;
	vgui::Label			*m_pSelectLevelLabel;
	vgui::Label			*m_pLevelNumLabel;
	vgui::Label			*m_pLevelTextLabel;
	vgui::ImagePanel	*m_pLevelIconPanels[MP_PROGRESS_ICON_COUNT];
	bool	bIsAlreadyVisible;
	bool	m_bInitializedLevelLabel;

	int		m_nCurrentLevel;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LevelSelectScreen, "mp_select_screen_branch_1" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CVGUI_MP_LevelSelectScreen::CVGUI_MP_LevelSelectScreen( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CVGUI_MP_LevelSelectScreen" ), m_nDay( 1 ), m_bInitializedLevelLabel( false ), m_nCurrentLevel( 0 )
{
	for ( int i = 0; i < MP_PROGRESS_ICON_COUNT; i++ )
	{
		m_pLevelIconPanels[i] = new vgui::ImagePanel( this, VarArgs( "LevelIcon%d", i ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update the level number, name and level icons for this track
//-----------------------------------------------------------------------------
void CVGUI_MP_LevelSelectScreen::UpdateLevelLabel( void )
{
	if ( g_pGameRules->IsMultiplayer() && PortalMPGameRules()->IsCoOp() && m_bInitializedLevelLabel )
	{
		int nThisDay = m_nDay - 1;
		int nTotalLevels = PortalMPGameRules()->GetBranchTotalLevelCount( nThisDay );

		int nLevel = PortalMPGameRules()->GetCoopBranchLevel( nThisDay );
		if ( nLevel == m_nCurrentLevel )
			return;

		m_nCurrentLevel = nLevel;

		char szLevelNumber[128];
		V_snprintf( szLevelNumber, sizeof( szLevelNumber ), "Level %d", nLevel );
		char szLevelName[128];
		V_snprintf( szLevelName, sizeof( szLevelName ), "(%s)", PortalMPGameRules()->GetBranchLevelName( nThisDay, m_nCurrentLevel - 1 ) );

		if ( m_pLevelNumLabel && m_bInitializedLevelLabel )
			m_pLevelNumLabel->SetText( szLevelNumber );

		if ( m_pLevelTextLabel && m_bInitializedLevelLabel )
			m_pLevelTextLabel->SetText( szLevelName );

		int w, h;
		GetSize( w, h );

		for ( int i = 0; i < nTotalLevels; i++ )
		{
			int nBlockSize = 120;
			int nSelectExtra = 30;

			bool bSelected = ( i == ( m_nCurrentLevel - 1 ) );

			// Reconstruction note: see CVGUI_MP_ProgressSignScreen::UpdateLevelLabel.
			bool bCompleted = PortalMPGameRules()->IsPlayerLevelInBranchComplete( 0, m_nDay - 1, i );

			if ( bSelected )
			{
				m_pLevelIconPanels[i]->SetImage( "screens/vgui_level_select_icon" );
				m_pLevelIconPanels[i]->SetDrawColor( bCompleted ? Color( 255, 200, 0, 255 ) : Color( 0, 200, 255, 255 ) );
			}
			else
			{
				m_pLevelIconPanels[i]->SetDrawColor( Color( 255, 255, 255, 255 ) );
				if ( bCompleted )
				{
					m_pLevelIconPanels[i]->SetImage( "screens/vgui_level_completed_icon" );
				}
				else
				{
					m_pLevelIconPanels[i]->SetImage( "screens/vgui_level_icon" );
				}
			}

			int nSize = bSelected ? ( nBlockSize + nSelectExtra ) : nBlockSize;
			m_pLevelIconPanels[i]->SetSize( nSize, nSize );

			int x, y;
			m_pLevelIconPanels[i]->GetPos( x, y );
			m_pLevelIconPanels[i]->SetPos( x, bSelected ? 635 - ( nSelectExtra / 2 ) : 635 );

			int new_x = ( w / 2 ) - ( nSize / 2 ) + ( ( i - ( m_nCurrentLevel - 1 ) ) * ( nBlockSize + nSelectExtra ) );
			vgui::GetAnimationController()->RunAnimationCommand( m_pLevelIconPanels[i], "xpos", new_x, 0.0f, 0.25f, vgui::AnimationController::INTERPOLATOR_DEACCEL );
			m_pLevelIconPanels[i]->SetPaintBackgroundEnabled( true );
			m_pLevelIconPanels[i]->SetZPos( m_pBranchTitleLabel->GetZPos() );
			m_pLevelIconPanels[i]->SetVisible( true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CVGUI_MP_LevelSelectScreen::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pBranchTitleLabel = (vgui::Label *)FindChildByName( "BranchTitle" );
	m_pSelectLevelLabel = (vgui::Label *)FindChildByName( "SelectLevelTitle" );
	m_pLevelNumLabel = (vgui::Label *)FindChildByName( "LevelNumberTitle" );
	m_pLevelTextLabel = (vgui::Label *)FindChildByName( "LevelNameTitle" );

	if ( m_pBranchTitleLabel )
		m_pBranchTitleLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	if ( m_pSelectLevelLabel )
		m_pSelectLevelLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	if ( m_pLevelNumLabel )
		m_pLevelNumLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	if ( m_pLevelTextLabel )
		m_pLevelTextLabel->SetFgColor( Color( 0, 0, 0, 255 ) );

	for ( int i = 0; i < MP_PROGRESS_ICON_COUNT; i++ )
	{
		m_pLevelIconPanels[i]->SetDrawColor( Color( 255, 255, 255, 255 ) );
		m_pLevelIconPanels[i]->SetVisible( false );
	}

	m_bInitializedLevelLabel = true;
}

void CVGUI_MP_LevelSelectScreen::PerformLayout( void )
{
	BaseClass::PerformLayout();

	for ( int i = 0; i < MP_PROGRESS_ICON_COUNT; i++ )
	{
		m_pLevelIconPanels[i]->SetShouldScaleImage( true );
	}
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CVGUI_MP_LevelSelectScreen::Init( KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData )
{
	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	return BaseClass::Init( pKeyValues, pInitData );
}

//-----------------------------------------------------------------------------
// Update the display string
//-----------------------------------------------------------------------------
void CVGUI_MP_LevelSelectScreen::OnTick()
{
	BaseClass::OnTick();

	if ( !bIsAlreadyVisible )
	{
		SetVisible( true );
		bIsAlreadyVisible = true;
	}

	UpdateLevelLabel();
}


//-----------------------------------------------------------------------------
// Level select screens for the remaining tracks
//-----------------------------------------------------------------------------
class CVGUI_MP_LevelSelectScreen2 : public CVGUI_MP_LevelSelectScreen
{
	DECLARE_CLASS( CVGUI_MP_LevelSelectScreen2, CVGUI_MP_LevelSelectScreen );
public:
	CVGUI_MP_LevelSelectScreen2( vgui::Panel *parent, const char *panelName );
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LevelSelectScreen2, "mp_select_screen_branch_2" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CVGUI_MP_LevelSelectScreen2::CVGUI_MP_LevelSelectScreen2( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CVGUI_MP_LevelSelectScreen2" )
{
	m_nDay = 2;
}


class CVGUI_MP_LevelSelectScreen3 : public CVGUI_MP_LevelSelectScreen
{
	DECLARE_CLASS( CVGUI_MP_LevelSelectScreen3, CVGUI_MP_LevelSelectScreen );
public:
	CVGUI_MP_LevelSelectScreen3( vgui::Panel *parent, const char *panelName );
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LevelSelectScreen3, "mp_select_screen_branch_3" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CVGUI_MP_LevelSelectScreen3::CVGUI_MP_LevelSelectScreen3( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CVGUI_MP_LevelSelectScreen3" )
{
	m_nDay = 3;
}


class CVGUI_MP_LevelSelectScreen4 : public CVGUI_MP_LevelSelectScreen
{
	DECLARE_CLASS( CVGUI_MP_LevelSelectScreen4, CVGUI_MP_LevelSelectScreen );
public:
	CVGUI_MP_LevelSelectScreen4( vgui::Panel *parent, const char *panelName );
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_LevelSelectScreen4, "mp_select_screen_branch_4" );

//-----------------------------------------------------------------------------
// Constructor:
//-----------------------------------------------------------------------------
CVGUI_MP_LevelSelectScreen4::CVGUI_MP_LevelSelectScreen4( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, "CVGUI_MP_LevelSelectScreen4" )
{
	m_nDay = 4;
}
