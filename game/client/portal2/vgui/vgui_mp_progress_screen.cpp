//========= Portal 2 reconstruction ============================================//
//
// Purpose: Co-op track lightboard (vgui_screen panel "mp_progress_sign"):
//          track name, level progress in the track and the icons listed in
//          coop_lightboard_icons.txt.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output) and the retail
// scripts/screens/mp_progress_sign.res. The 2010 build defined an older
// version of this class in vgui_mp_lobby_screen.cpp. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "vgui_base_progress_screen.h"
#include <vgui_controls/Label.h>
#include "portal_mp_gamerules.h"
#include "portal2_engine_compat.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Co-op lightboard
//-----------------------------------------------------------------------------
class CVGUI_MP_ProgressSignScreen : public CVGUI_Base_ProgressSignScreen
{
	DECLARE_CLASS( CVGUI_MP_ProgressSignScreen, CVGUI_Base_ProgressSignScreen );

public:
	CVGUI_MP_ProgressSignScreen( vgui::Panel *pParent, const char *pPanelName );

	virtual void OnTick( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint( void );
	virtual void PerformLayout( void );

	virtual void UpdateLevelInfo( void );

private:
	int m_nTrack;
	vgui::Label *m_pBranchNumLabel;
	vgui::Label *m_pLevelProgressNumLabel;
	bool m_bLabelsReady;
	int m_nLevel;
	int m_nTotalLevels;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_ProgressSignScreen, "mp_progress_sign" );

CVGUI_MP_ProgressSignScreen::CVGUI_MP_ProgressSignScreen( vgui::Panel *pParent, const char *pPanelName )
	: BaseClass( pParent, "CVGUI_MP_ProgressSignScreen" )
{
	m_nTrack = 1;
	m_pBranchNumLabel = NULL;
	m_pLevelProgressNumLabel = NULL;
	m_bLabelsReady = false;
	m_nLevel = 0;
	m_nTotalLevels = 0;
}

void CVGUI_MP_ProgressSignScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pBranchNumLabel = dynamic_cast< vgui::Label * >( FindChildByName( "BranchNumLabel" ) );
	m_pLevelProgressNumLabel = dynamic_cast< vgui::Label * >( FindChildByName( "LevelProgressNumLabel" ) );

	if ( m_pBranchNumLabel )
	{
		m_pBranchNumLabel->SetFgColor( Color( 0, 0, 0, 255 ) );
	}

	if ( m_pLevelProgressNumLabel )
	{
		m_pLevelProgressNumLabel->SetFgColor( Color( 0, 0, 0, 255 ) );
	}

	m_bLabelsReady = true;
	UpdateLevelInfo();
}

void CVGUI_MP_ProgressSignScreen::PerformLayout( void )
{
	BaseClass::PerformLayout();
}

void CVGUI_MP_ProgressSignScreen::OnTick( void )
{
	UpdateLevelInfo();
	BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: Refreshes the labels and icons when the co-op track or its current
//          level changes.
//-----------------------------------------------------------------------------
void CVGUI_MP_ProgressSignScreen::UpdateLevelInfo( void )
{
	if ( !g_pGameRules || !g_pGameRules->IsMultiplayer() )
		return;

	CPortalMPGameRules *pRules = PortalMPGameRules();
	if ( !pRules || !pRules->IsCoOp() )
		return;

	if ( !m_bLabelsReady )
		return;

	int nTrack = pRules->GetCoopSection();
	if ( nTrack < 1 )
		return;

	int nTotalLevels = pRules->GetBranchTotalLevelCount( nTrack - 1 );
	int nLevel = pRules->GetCoopBranchLevel( nTrack - 1 );
	if ( m_nTrack == nTrack && m_nLevel == nLevel )
		return;

	m_nLevel = nLevel;
	m_nTotalLevels = nTotalLevels;
	m_nTrack = nTrack;

	char szTrackName[128];
	V_snprintf( szTrackName, sizeof( szTrackName ), "#P2COOP_TrackName_%d", nTrack );
	char szProgress[128];
	V_snprintf( szProgress, sizeof( szProgress ), "%02d/%02d", m_nLevel, nTotalLevels );

	if ( m_pBranchNumLabel )
	{
		m_pBranchNumLabel->SetText( szTrackName );
	}

	if ( m_pLevelProgressNumLabel )
	{
		m_pLevelProgressNumLabel->SetText( szProgress );
	}

	KeyValues *pKeys = g_ProgressSignScreenKeys.GetCoopKeys();
	if ( !pKeys )
		return;

	KeyValues *pTracks = pKeys->FindKey( "Tracks" );
	KeyValues *pMaps = pKeys->FindKey( "Maps" );
	if ( !pTracks || !pMaps )
		return;

	KeyValues *pMap = pMaps->FindKey( Portal2Engine::GetLevelNameShort() );
	if ( !pMap )
		return;

	KeyValues *pValue = pMap->FindKey( "startup" );
	SetStartupSequence( pValue ? pValue->GetString() : NULL );
	SetDirtLevel( pMap->FindKey( "dirt" ) );

	// The track lists the icons shown; the map lists the ones lit.
	char szTrackKey[32];
	V_snprintf( szTrackKey, sizeof( szTrackKey ), "Track_%d", m_nTrack );
	KeyValues *pTrackIcons = pTracks->FindKey( szTrackKey );
	if ( !pTrackIcons )
		return;

	int nIcon = 0;
	for ( KeyValues *pIcon = pTrackIcons->GetFirstSubKey(); pIcon; pIcon = pIcon->GetNextKey() )
	{
		// Portal 2 port: retail writes an eleventh entry past the array
		// before warning; this stops at the array size instead.
		if ( nIcon == LIGHTBOARD_MAX_ICONS )
		{
			Warning( "Too many icons defined in the coop_lightboard_icons file" );
			return;
		}

		for ( int i = 0; g_pszLightboardIcons[ i ]; ++i )
		{
			if ( V_stricmp( pIcon->GetName(), g_pszLightboardIcons[ i ] ) != 0 || pIcon->GetInt() < 1 )
				continue;

			m_nIcons[ nIcon ] = i;
			for ( KeyValues *pMapIcon = pMap->GetFirstSubKey(); pMapIcon; pMapIcon = pMapIcon->GetNextKey() )
			{
				if ( !V_stricmp( pIcon->GetName(), pMapIcon->GetName() ) && pMapIcon->GetInt() > 0 )
				{
					m_bIconEnabled[ nIcon ] = true;
				}
			}
			break;
		}

		++nIcon;
	}
}

void CVGUI_MP_ProgressSignScreen::Paint( void )
{
	float flAlpha = UpdateFlicker();
	PaintBoardBackground();
	PaintLevelNumber( m_nLevel, flAlpha );
	PaintProgressBar( m_nLevel, m_nTotalLevels, flAlpha );
	PaintIcons();
	if ( m_nDirtLevel != -1 )
	{
		PaintDirt();
	}

	BaseClass::Paint();
}
