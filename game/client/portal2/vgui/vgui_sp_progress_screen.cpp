//========= Portal 2 reconstruction ============================================//
//
// Purpose: Single player elevator lightboard (vgui_screen panel
//          "sp_progress_sign"): chamber number, chapter progress and the test
//          icons listed for the map in sp_lightboard_icons.txt.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output) and the retail
// scripts/screens/sp_progress_sign.res. No original source or Steam2
// pseudocode exists for this file. Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "vgui_base_progress_screen.h"
#include <vgui_controls/Label.h>
#include "portal2_engine_compat.h"
#if defined( PORTAL2_PUZZLEMAKER )
#include "portal2/basemodpanel.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// Workshop tag to lightboard icon, for community maps.
struct LightboardTagIcon_t
{
	const char *m_pszTag;
	const char *m_pszIcon;
};

//-----------------------------------------------------------------------------
// Single player lightboard
//-----------------------------------------------------------------------------
class CVGUI_SP_ProgressSignScreen : public CVGUI_Base_ProgressSignScreen
{
	DECLARE_CLASS( CVGUI_SP_ProgressSignScreen, CVGUI_Base_ProgressSignScreen );

public:
	CVGUI_SP_ProgressSignScreen( vgui::Panel *pParent, const char *pPanelName );
	virtual ~CVGUI_SP_ProgressSignScreen();

	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );
	virtual void OnTick( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint( void );

private:
	void InitTagIcons( void );
	void UpdateLevelInfo( void );
	void SetupCommunityMapLightboard( void );

	vgui::Label *m_pLevelProgressLabel;
	CUtlVector< LightboardTagIcon_t > m_TagIcons;
	vgui::Label *m_pLevelNameLabel;
	int m_nNumIconTypes;
	bool m_bLabelsReady;
	char m_szMapName[128];
	int m_nLevelNumber;
	int m_nTotalLevels;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_SP_ProgressSignScreen, "sp_progress_sign" );

CVGUI_SP_ProgressSignScreen::CVGUI_SP_ProgressSignScreen( vgui::Panel *pParent, const char *pPanelName )
	: BaseClass( pParent, "CVGUI_SP_ProgressSignScreen" )
{
	m_pLevelProgressLabel = NULL;
	m_pLevelNameLabel = NULL;
	m_nNumIconTypes = 0;
	m_bLabelsReady = false;
	m_nLevelNumber = 0;
	m_nTotalLevels = 0;
	V_memset( m_szMapName, 0, sizeof( m_szMapName ) );

	InitTagIcons();
}

CVGUI_SP_ProgressSignScreen::~CVGUI_SP_ProgressSignScreen()
{
	m_TagIcons.Purge();
}

void CVGUI_SP_ProgressSignScreen::InitTagIcons( void )
{
	static const LightboardTagIcon_t s_TagIcons[] =
	{
		{ "Goo", "goop" },
		{ "Reflector Cube", "laser_cube" },
		{ "Turret", "turret" },
		{ "Faith Plate", "plate_fling" },
		{ "Light Bridge", "bridges" },
		{ "Tractor Beam", "tbeams" },
		{ "Bounce Gel", "paint_bounce" },
		{ "Speed Gel", "paint_speed" },
		{ "Laser Field", "danger_field" },
		{ "Weighted Cube Dropper", "cube_drop" },
		{ "Companion Cube Dropper", "cube_drop" },
		{ "Reflector Cube Dropper", "cube_drop" },
		{ "Edgeless Safety Cube Dropper", "cube_drop" },
		{ "Franken Cube Dropper", "cube_drop" },
		{ "Tractor Beam Polarity", "tbeam_polarity" },
		{ "Laser Catcher", "laser_power" },
		{ "Cube Button", "cube_button" },
	};

	for ( int i = 0; i < ARRAYSIZE( s_TagIcons ); ++i )
	{
		m_TagIcons.AddToTail( s_TagIcons[ i ] );
	}

	m_nNumIconTypes = 0;
	while ( g_pszLightboardIcons[ m_nNumIconTypes ] )
	{
		++m_nNumIconTypes;
	}
}

bool CVGUI_SP_ProgressSignScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	if ( !BaseClass::Init( pKeyValues, pInitData ) )
		return false;

	m_flDisabledIconAlpha = 112.0f;
	m_nBaseBrightness = 100;
	return true;
}

void CVGUI_SP_ProgressSignScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pLevelProgressLabel = dynamic_cast< vgui::Label * >( FindChildByName( "LevelProgressLabel" ) );
	if ( m_pLevelProgressLabel )
	{
		m_pLevelProgressLabel->SetFgColor( Color( 0, 0, 0, 255 ) );
	}

	m_pLevelNameLabel = dynamic_cast< vgui::Label * >( FindChildByName( "LevelNameLabel" ) );
	if ( m_pLevelNameLabel )
	{
		m_pLevelNameLabel->SetFgColor( Color( 0, 0, 0, 255 ) );
	}

	m_bLabelsReady = true;
	UpdateLevelInfo();
}

void CVGUI_SP_ProgressSignScreen::OnTick( void )
{
	UpdateLevelInfo();
	BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: Reads the current map's entry from sp_lightboard_icons.txt once
//          per map.
//-----------------------------------------------------------------------------
void CVGUI_SP_ProgressSignScreen::UpdateLevelInfo( void )
{
	const char *pszLevelName = engine->GetLevelName();
	bool bCommunityMap = pszLevelName && V_stristr( pszLevelName, "puzzlemaker" ) != NULL;
#if defined( PORTAL2_PUZZLEMAKER )
	bCommunityMap = bCommunityMap || BASEMODPANEL_SINGLETON.GetCurrentCommunityMapID() != 0;
#endif

	if ( bCommunityMap )
	{
		if ( !m_bLabelsReady )
			return;

		const char *pszMapName = Portal2Engine::GetLevelNameShort();
		if ( !V_stricmp( m_szMapName, pszMapName ) )
			return;

		V_strncpy( m_szMapName, pszMapName, sizeof( m_szMapName ) );
		ClearIcons();
		SetupCommunityMapLightboard();
		return;
	}

	if ( !m_bLabelsReady )
		return;

	if ( g_pGameRules && g_pGameRules->IsMultiplayer() )
		return;

	const char *pszMapName = Portal2Engine::GetLevelNameShort();
	if ( !V_stricmp( m_szMapName, pszMapName ) )
		return;

	V_strncpy( m_szMapName, pszMapName, sizeof( m_szMapName ) );
	ClearIcons();

	if ( m_pLevelNameLabel )
	{
		m_pLevelNameLabel->SetVisible( false );
	}

	KeyValues *pKeys = g_ProgressSignScreenKeys.GetSinglePlayerKeys();
	if ( !pKeys )
		return;

	KeyValues *pMaps = pKeys->FindKey( "Maps" );
	if ( !pMaps )
		return;

	KeyValues *pMap = pMaps->FindKey( pszMapName );
	if ( !pMap )
		return;

	KeyValues *pValue = pMap->FindKey( "level_number" );
	if ( pValue )
	{
		m_nLevelNumber = pValue->GetInt();
	}

	pValue = pMap->FindKey( "total_levels" );
	if ( pValue )
	{
		m_nTotalLevels = pValue->GetInt();
	}

	char szProgress[128];
	V_snprintf( szProgress, sizeof( szProgress ), "%02d/%02d", m_nLevelNumber, m_nTotalLevels );
	if ( m_pLevelProgressLabel && m_bLabelsReady )
	{
		m_pLevelProgressLabel->SetText( szProgress );
	}

	pValue = pMap->FindKey( "startup" );
	SetStartupSequence( pValue ? pValue->GetString() : NULL );
	SetDirtLevel( pMap->FindKey( "dirt" ) );

	KeyValues *pIcons = pMap->FindKey( "icons" );
	if ( !pIcons )
		return;

	int nIcon = 0;
	for ( KeyValues *pIcon = pIcons->GetFirstSubKey(); pIcon; pIcon = pIcon->GetNextKey() )
	{
		// Portal 2 port: retail writes an eleventh entry past the array
		// before warning; this stops at the array size instead.
		if ( nIcon == LIGHTBOARD_MAX_ICONS )
		{
			DevWarning( "Too many lightboard icons defined for map %s\n", pszMapName );
			return;
		}

		for ( int i = 0; g_pszLightboardIcons[ i ]; ++i )
		{
			if ( !V_stricmp( pIcon->GetName(), g_pszLightboardIcons[ i ] ) )
			{
				m_nIcons[ nIcon ] = i;
				m_bIconEnabled[ nIcon ] = pIcon->GetInt() != 0;
				break;
			}
		}

		++nIcon;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Community and Puzzle Maker maps. The labels follow the retail
//          Puzzle Maker preview case ("Preview", no level count). Retail also
//          derives icons from the map's Steam Workshop or Puzzle Maker tags
//          (m_TagIcons) and fills the other slots with random disabled icons;
//          that needs the Workshop and Puzzle Maker, which this build lacks.
//-----------------------------------------------------------------------------
void CVGUI_SP_ProgressSignScreen::SetupCommunityMapLightboard( void )
{
	static bool s_bWarned = false;
	if ( !s_bWarned )
	{
		s_bWarned = true;
		DevWarning( "Portal 2: community map lightboards (Steam Workshop tags) are not supported by this build\n" );
	}

	SetStartupSequence( NULL );
	m_nDirtLevel = -1;
	m_nLevelNumber = 0;
	m_nTotalLevels = 0;

	if ( m_pLevelProgressLabel )
	{
		m_pLevelProgressLabel->SetText( "00/00" );
		m_pLevelProgressLabel->SetVisible( false );
	}

	if ( m_pLevelNameLabel )
	{
		m_pLevelNameLabel->SetText( "Preview" );
	}
}

void CVGUI_SP_ProgressSignScreen::Paint( void )
{
	float flAlpha = UpdateFlicker();
	PaintBoardBackground();
	PaintLevelNumber( m_nLevelNumber, flAlpha );
	if ( m_pLevelProgressLabel )
	{
		m_pLevelProgressLabel->SetFgColor( Color( 0, 0, 0, (int)flAlpha ) );
	}
	PaintProgressBar( m_nLevelNumber, m_nTotalLevels, flAlpha );
	PaintIcons();
	if ( m_nDirtLevel != -1 )
	{
		PaintDirt();
	}

	BaseClass::Paint();
}
