//========= Portal 2 reconstruction ============================================//
//
// Purpose: Shared lightboard ("progress sign") vgui screen; see
//          vgui_base_progress_screen.h.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). Layout constants are the retail
// literals. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "vgui_base_progress_screen.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// Order matches the icon sheet; lightboard_startup_sequences.txt and the icon
// scripts name these strings.
const char *g_pszLightboardIcons[] =
{
	"dummy",
	"cube_drop",
	"cube_button",
	"cube_bonk",
	"drink_water",
	"goop",
	"crushers",
	"laser_cube",
	"turret",
	"turret_burn",
	"portal_fling",
	"plate_fling",
	"bridges",
	"bridge_block",
	"grinders",
	"tbeams",
	"paint_bounce",
	"paint_speed",
	"handoff",
	"button_stand",
	"laser_power",
	"portal_fling_2",
	"tbeam_polarity",
	"danger_field",
	NULL
};

static const char *s_pszDefaultStartupSequence = "default_flicker";

//-----------------------------------------------------------------------------
// CProgressSignScreenKeys
//-----------------------------------------------------------------------------
CProgressSignScreenKeys g_ProgressSignScreenKeys( "CProgressSignScreenKeys" );

CProgressSignScreenKeys::CProgressSignScreenKeys( const char *pszName ) : CAutoGameSystem( pszName )
{
	m_pSinglePlayerKeys = NULL;
	m_pCoopKeys = NULL;
	m_bStartupSequencesLoaded = false;
}

CProgressSignScreenKeys::~CProgressSignScreenKeys()
{
	m_StartupSequences.Purge();
}

bool CProgressSignScreenKeys::Init()
{
	if ( !m_pSinglePlayerKeys )
	{
		m_pSinglePlayerKeys = new KeyValues( "sp_lightboard_icons" );
		m_pSinglePlayerKeys->LoadFromFile( g_pFullFileSystem, "scripts/vscripts/transitions/sp_lightboard_icons.txt", NULL );
	}

	if ( !m_pCoopKeys )
	{
		m_pCoopKeys = new KeyValues( "coop_lightboard_icons" );
		m_pCoopKeys->LoadFromFile( g_pFullFileSystem, "scripts/vscripts/coop/coop_lightboard_icons.txt", NULL );
	}

	return LoadStartupSequences();
}

//-----------------------------------------------------------------------------
// Purpose: The built-in "default_flicker" comes first. A sequence that omits
//          a value keeps the value of the sequence before it.
//-----------------------------------------------------------------------------
bool CProgressSignScreenKeys::LoadStartupSequences()
{
	if ( m_bStartupSequencesLoaded )
		return false;

	LightboardStartupSequence_t sequence;
	V_strncpy( sequence.m_szName, s_pszDefaultStartupSequence, sizeof( sequence.m_szName ) );
	sequence.m_flFlickerRateMin = 0.2f;
	sequence.m_flFlickerRateMax = 0.65f;
	sequence.m_flFlickerQuickMin = 0.02f;
	sequence.m_flFlickerQuickMax = 0.12f;
	sequence.m_flBackgroundFlickerLength = 0.75f;
	sequence.m_flLevelNumberDelay = 0.8f;
	sequence.m_flProgressDelay = 1.65f;
	sequence.m_flIconDelay = 1.0f;
	m_StartupSequences.AddToTail( sequence );

	KeyValues *pKeys = new KeyValues( "lightboard_startup_sequences" );
	bool bLoaded = pKeys->LoadFromFile( g_pFullFileSystem, "scripts/vscripts/transitions/lightboard_startup_sequences.txt", NULL );
	if ( !bLoaded )
	{
		pKeys->deleteThis();
		return false;
	}

	KeyValues *pStartups = pKeys->FindKey( "Startups" );
	if ( pStartups )
	{
		for ( KeyValues *pSequence = pStartups->GetFirstSubKey(); pSequence; pSequence = pSequence->GetNextKey() )
		{
			V_strncpy( sequence.m_szName, pSequence->GetName(), sizeof( sequence.m_szName ) );

			struct
			{
				const char *m_pszKey;
				float *m_pflValue;
			} values[] =
			{
				{ "flicker_rate_min", &sequence.m_flFlickerRateMin },
				{ "flicker_rate_max", &sequence.m_flFlickerRateMax },
				{ "flicker_quick_min", &sequence.m_flFlickerQuickMin },
				{ "flicker_quick_max", &sequence.m_flFlickerQuickMax },
				{ "bg_flicker_length", &sequence.m_flBackgroundFlickerLength },
				{ "level_number_delay", &sequence.m_flLevelNumberDelay },
				{ "icon_delay", &sequence.m_flIconDelay },
				{ "progress_delay", &sequence.m_flProgressDelay },
			};

			for ( int i = 0; i < ARRAYSIZE( values ); ++i )
			{
				KeyValues *pValue = pSequence->FindKey( values[ i ].m_pszKey );
				if ( pValue )
				{
					*values[ i ].m_pflValue = pValue->GetFloat();
				}
			}

			m_StartupSequences.AddToTail( sequence );
		}
	}

	// Portal 2 port: retail keeps this KeyValues alive; nothing reads it again.
	pKeys->deleteThis();

	m_bStartupSequencesLoaded = true;
	return bLoaded;
}

int CProgressSignScreenKeys::FindStartupSequence( const char *pszName ) const
{
	if ( pszName )
	{
		for ( int i = 0; i < m_StartupSequences.Count(); ++i )
		{
			if ( !V_stricmp( pszName, m_StartupSequences[ i ].m_szName ) )
				return i;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// CVGUI_Base_ProgressSignScreen
//-----------------------------------------------------------------------------
CVGUI_Base_ProgressSignScreen::CVGUI_Base_ProgressSignScreen( vgui::Panel *pParent, const char *pMetaClassName )
	: BaseClass( pParent, pMetaClassName )
{
	m_nStartupSequence = 0;
	m_bFlickering = false;
	m_nOverlayAlpha = 0;
	m_nDirtLevel = -1;
	m_nBoardTextureID = -1;
	m_nNumbersTextureID = -1;
	m_nBarTextureID = -1;
	m_nIconsTextureID = -1;
	m_flFlickerStartTime = 0.0f;
	m_flFlickerLength = 0.0f;
	m_nBrightness = 0;
	m_flIconAlphaScale = 0.0f;
	m_bWasEnabled = false;
	m_flNextFlickerDuration = 0.0f;
	m_flLastFlickerTime = 0.0f;
	m_flQuickFlickerDuration = 0.0f;
	m_bShownOnce = false;
	m_nBaseBrightness = 0;
	m_flDisabledIconAlpha = 0.0f;

	SetScheme( "basemodui_scheme" );

	for ( int i = 0; i < LIGHTBOARD_DIRT_LEVELS; ++i )
	{
		m_nDirtTextureIDs[ i ] = -1;
	}

	for ( int i = 0; i < LIGHTBOARD_MAX_ICONS; ++i )
	{
		m_nIcons[ i ] = -1;
		m_bIconEnabled[ i ] = false;
	}
}

CVGUI_Base_ProgressSignScreen::~CVGUI_Base_ProgressSignScreen()
{
	if ( vgui::surface() )
	{
		int nTextures[] = { m_nBoardTextureID, m_nNumbersTextureID, m_nBarTextureID, m_nIconsTextureID };
		for ( int i = 0; i < ARRAYSIZE( nTextures ); ++i )
		{
			if ( nTextures[ i ] != -1 )
				vgui::surface()->DestroyTextureID( nTextures[ i ] );
		}

		for ( int i = 0; i < LIGHTBOARD_DIRT_LEVELS; ++i )
		{
			if ( m_nDirtTextureIDs[ i ] != -1 )
				vgui::surface()->DestroyTextureID( m_nDirtTextureIDs[ i ] );
		}
	}
}

static void CreateLightboardTexture( int &nTextureID, const char *pszMaterial )
{
	if ( nTextureID == -1 )
	{
		nTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( nTextureID, pszMaterial, true, false );
	}
}

bool CVGUI_Base_ProgressSignScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	CreateLightboardTexture( m_nBoardTextureID, "vgui/screens/vgui_coop_progress_board" );
	CreateLightboardTexture( m_nNumbersTextureID, "vgui/screens/vgui_coop_progress_board_numbers" );
	CreateLightboardTexture( m_nBarTextureID, "vgui/screens/vgui_coop_progress_board_bar" );
	CreateLightboardTexture( m_nIconsTextureID, "vgui/screens/P2_lightboard_vgui" );
	CreateLightboardTexture( m_nDirtTextureIDs[ 0 ], "vgui/elevator_video_overlay1" );
	CreateLightboardTexture( m_nDirtTextureIDs[ 1 ], "vgui/elevator_video_overlay2" );
	CreateLightboardTexture( m_nDirtTextureIDs[ 2 ], "vgui/elevator_video_overlay3" );

	m_flDisabledIconAlpha = 134.0f;
	m_nBaseBrightness = 140;

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	// A screen that starts switched on does not flicker.
	if ( pInitData && pInitData->m_pEntity )
	{
		C_VGuiScreen *pScreen = dynamic_cast< C_VGuiScreen * >( pInitData->m_pEntity );
		if ( pScreen )
		{
			m_bWasEnabled = pScreen->IsActive();
		}
	}

	return BaseClass::Init( pKeyValues, pInitData );
}

void CVGUI_Base_ProgressSignScreen::OnTick( void )
{
	BaseClass::OnTick();

	// C_VGuiScreen enables the panel while the screen entity is active.
	if ( !m_bWasEnabled && IsEnabled() )
	{
		StartFlicker();
	}
	else if ( m_bFlickering && m_flFlickerStartTime + m_flFlickerLength < gpGlobals->curtime )
	{
		m_bFlickering = false;
	}

	m_bWasEnabled = IsEnabled();

	if ( !m_bShownOnce )
	{
		SetVisible( true );
		m_bShownOnce = true;
	}
}

void CVGUI_Base_ProgressSignScreen::ClearIcons( void )
{
	for ( int i = 0; i < LIGHTBOARD_MAX_ICONS; ++i )
	{
		m_nIcons[ i ] = -1;
		m_bIconEnabled[ i ] = false;
	}
}

void CVGUI_Base_ProgressSignScreen::SetStartupSequence( const char *pszName )
{
	m_nStartupSequence = g_ProgressSignScreenKeys.FindStartupSequence( pszName ? pszName : s_pszDefaultStartupSequence );
}

void CVGUI_Base_ProgressSignScreen::SetDirtLevel( KeyValues *pDirt )
{
	if ( pDirt )
	{
		m_nDirtLevel = clamp( pDirt->GetInt(), 0, LIGHTBOARD_DIRT_LEVELS - 1 );
	}
}

void CVGUI_Base_ProgressSignScreen::StartFlicker( void )
{
	m_bFlickering = true;
	m_flFlickerStartTime = gpGlobals->curtime;
	m_flLastFlickerTime = gpGlobals->curtime;

	const LightboardStartupSequence_t &sequence = g_ProgressSignScreenKeys.GetStartupSequence( m_nStartupSequence );
	m_flNextFlickerDuration = RandomFloat( sequence.m_flFlickerRateMin, sequence.m_flFlickerRateMax );
	// Retail draws the quick flicker from flicker_quick_min only.
	m_flQuickFlickerDuration = RandomFloat( sequence.m_flFlickerQuickMin, sequence.m_flFlickerQuickMin );

	m_flFlickerLength = MAX( MAX( sequence.m_flBackgroundFlickerLength, sequence.m_flProgressDelay ),
		MAX( sequence.m_flLevelNumberDelay, sequence.m_flIconDelay ) );
}

float CVGUI_Base_ProgressSignScreen::UpdateFlicker( void )
{
	m_flIconAlphaScale = 1.0f;

	if ( !m_bFlickering )
	{
		m_nOverlayAlpha = 255;
		m_nBrightness = m_nBaseBrightness;
		return 255.0f;
	}

	const LightboardStartupSequence_t &sequence = g_ProgressSignScreenKeys.GetStartupSequence( m_nStartupSequence );
	float flCurTime = gpGlobals->curtime;

	if ( flCurTime <= m_flFlickerStartTime + sequence.m_flBackgroundFlickerLength )
	{
		if ( m_flQuickFlickerDuration + m_flLastFlickerTime <= flCurTime )
		{
			m_nOverlayAlpha = 63;
			m_nBrightness = m_nBaseBrightness / 4;
		}

		if ( m_flNextFlickerDuration + m_flLastFlickerTime <= flCurTime )
		{
			m_nBrightness = ( m_nBrightness == m_nBaseBrightness ) ? m_nBaseBrightness / 4 : m_nBaseBrightness / 2;
			m_nOverlayAlpha = ( m_nOverlayAlpha != 255 ) ? 127 : 63;
			m_flLastFlickerTime = gpGlobals->curtime;
			m_flNextFlickerDuration = RandomFloat( sequence.m_flFlickerRateMin, sequence.m_flFlickerRateMax );
			m_flQuickFlickerDuration = RandomFloat( sequence.m_flFlickerQuickMin, sequence.m_flFlickerQuickMin );
		}
	}
	else
	{
		m_nOverlayAlpha = 255;
		m_nBrightness = m_nBaseBrightness;
	}

	float flAlpha = ( m_flFlickerStartTime + sequence.m_flLevelNumberDelay < flCurTime ) ? 255.0f : 2.0f;
	if ( flCurTime <= m_flFlickerStartTime + sequence.m_flIconDelay )
	{
		m_flIconAlphaScale = 0.1f;
	}

	return flAlpha;
}

void CVGUI_Base_ProgressSignScreen::PaintBoardBackground( void )
{
	int w, h;
	GetSize( w, h );

	vgui::surface()->DrawSetColor( m_nBrightness, m_nBrightness, m_nBrightness, 255 );
	vgui::surface()->DrawSetTexture( m_nBoardTextureID );
	vgui::surface()->DrawTexturedRect( 0, 0, w, h );
}

//-----------------------------------------------------------------------------
// Purpose: Two digits from a 6x2 sheet: 0-5 on the top row, 6-9 below.
//-----------------------------------------------------------------------------
void CVGUI_Base_ProgressSignScreen::PaintLevelNumber( int nLevel, float flAlpha )
{
	int nDigitTall = (int)( (float)GetTall() * 0.4f );

	vgui::surface()->DrawSetColor( 255, 255, 255, (int)flAlpha );
	vgui::surface()->DrawSetTexture( m_nNumbersTextureID );

	const float flCell = 1.0f / 6.0f;
	int nTens = nLevel / 10;
	int nOnes = nLevel % 10;

	float t0 = 0.0f, t1 = 0.5f;
	if ( nTens >= 6 )
	{
		nTens -= 6;
		t0 = 0.5f;
		t1 = 1.0f;
	}

	int y0 = nDigitTall / 4;
	int x1 = nDigitTall / 3 + 70;
	vgui::surface()->DrawTexturedSubRect( 70, y0, x1, nDigitTall + y0, (float)nTens * flCell, t0, flCell + (float)nTens * flCell, t1 );

	t0 = 0.0f;
	t1 = 0.5f;
	if ( nOnes >= 6 )
	{
		nOnes -= 6;
		t0 = 0.5f;
		t1 = 1.0f;
	}

	vgui::surface()->DrawSetTexture( m_nNumbersTextureID );
	vgui::surface()->DrawTexturedSubRect( x1, y0, ( nDigitTall / 3 ) * 2 + 70, nDigitTall + y0, (float)nOnes * flCell, t0, flCell + (float)nOnes * flCell, t1 );
}

void CVGUI_Base_ProgressSignScreen::PaintProgressBar( int nLevel, int nTotalLevels, float flAlpha )
{
	if ( nTotalLevels <= 0 )
		return;

	vgui::surface()->DrawSetColor( 255, 255, 255, (int)flAlpha );
	vgui::surface()->DrawSetTexture( m_nBarTextureID );

	int nBarWide = GetWide() - 162;
	float flFraction = ( nLevel > 0 ) ? (float)nLevel / (float)nTotalLevels : 1.0f;
	vgui::surface()->DrawTexturedSubRect( 81, 461, (int)( (float)nBarWide * flFraction + 81.0f ), 493,
		0.0f, 0.0f, 0.03125f * (float)nBarWide * flFraction, 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose: Up to ten icons from a 5x5 sheet, five per row.
//-----------------------------------------------------------------------------
void CVGUI_Base_ProgressSignScreen::PaintIcons( void )
{
	int x = 81;
	int y = 577;

	for ( int i = 0; i < LIGHTBOARD_MAX_ICONS; ++i )
	{
		if ( m_nIcons[ i ] == -1 )
			continue;

		int nCell = m_nIcons[ i ] - 1;
		float s = (float)( nCell % 5 ) * 0.2f;
		float t = (float)( nCell / 5 ) * 0.2f;

		int x1, xNext;
		if ( i == 5 )
		{
			y += 63;
			x = 81;
			x1 = 134;
			xNext = 144;
		}
		else
		{
			x1 = x + 53;
			xNext = x + 63;
		}

		float flAlpha = m_bIconEnabled[ i ] ? 255.0f : m_flDisabledIconAlpha;
		vgui::surface()->DrawSetColor( 0, 0, 0, (int)( m_flIconAlphaScale * flAlpha ) );
		vgui::surface()->DrawSetTexture( m_nIconsTextureID );
		vgui::surface()->DrawTexturedSubRect( x, y, x1, y + 53, s, t, s + 0.2f, t + 0.2f );

		x = xNext;
	}
}

void CVGUI_Base_ProgressSignScreen::PaintDirt( void )
{
	int w, h;
	GetSize( w, h );

	vgui::surface()->DrawSetColor( 255, 255, 255, m_nOverlayAlpha );
	vgui::surface()->DrawSetTexture( m_nDirtTextureIDs[ m_nDirtLevel ] );
	vgui::surface()->DrawTexturedRect( 0, 0, w, h );
}
