//========= Copyright (c) 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=======================================================================================//

#include "vvideo.h"
#include "vfooterpanel.h"
#include "vdropdownmenu.h"
#include "vslidercontrol.h"
#include "vhybridbutton.h"
#include "engineinterface.h"
#include "IGameUIFuncs.h"
#include "gameui_util.h"
#include "vgui/ISurface.h"
#include "modes.h"
#include "videocfg/videocfg.h"
#include "vgenericconfirmation.h"
#include "render_stage_marks.h"

#include "materialsystem/materialsystem_config.h"

#ifdef _X360
#include "xbox/xbox_launch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "portal2_engine_compat.h"
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

#define VIDEO_ANTIALIAS_COMMAND_PREFIX "_antialias"
#define VIDEO_RESOLUTION_COMMAND_PREFIX "_res"
#define VIDEO_UISCALE_COMMAND_PREFIX "_uiscale"
#define VIDEO_TEMPORALSCALE_COMMAND_PREFIX "_temporalscale"

static const float s_TemporalScales[] = { 0.0f, 1.0f, 2.0f / 3.0f, 1.0f / 1.7f, 0.5f };
static const char *s_TemporalScaleNames[] = {
    "Off", "Native AA (100%)", "Quality (67%)", "Balanced (59%)", "Performance (50%)" };

// The UI scale row (ui_scale; 0 follows the display's scale). The shipped
// video.res has no such row, so PreApplyControlSettings adds it.
static const float s_UIScaleChoices[] = { 0.0f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f };

static void GetUIScaleChoiceName( int nChoice, char *pOut, int nOutSize )
{
	if ( s_UIScaleChoices[nChoice] <= 0.0f )
		V_strncpy( pOut, "Automatic", nOutSize );
	else
		V_snprintf( pOut, nOutSize, "%d%%", (int)( s_UIScaleChoices[nChoice] * 100.0f + 0.5f ) );
}

// The choice closest to a ui_scale value.
static int GetUIScaleChoice( float flScale )
{
	int nBest = 0;
	for ( int i = 1; i < ARRAYSIZE( s_UIScaleChoices ); ++i )
	{
		if ( fabsf( s_UIScaleChoices[i] - flScale ) < fabsf( s_UIScaleChoices[nBest] - flScale ) )
			nBest = i;
	}
	return nBest;
}

int GetScreenAspectMode( int width, int height );

Video::Video( Panel *parent, const char *panelName ):
BaseClass( parent, panelName ),
m_autodelete_pResourceLoadConditions( (KeyValues*) NULL )
{
	SetDeleteSelfOnClose( true );
	SetProportional( true );

	SetDialogTitle( "#GameUI_Video" );

	m_pResourceLoadConditions = new KeyValues( "video" );
	m_autodelete_pResourceLoadConditions.Assign( m_pResourceLoadConditions );

	// only gamma is not part of the apply logic
	// save off original value for discard logic
	CGameUIConVarRef mat_monitorgamma( "mat_monitorgamma" );
	m_flOriginalGamma = mat_monitorgamma.GetFloat();

	m_sldBrightness = NULL;
	m_drpAspectRatio = NULL;
	m_drpResolution = NULL;
	m_drpDisplayMode = NULL;
	m_drpPowerSavingsMode = NULL;
	m_drpSplitScreenDirection = NULL;
	m_drpUIScale = NULL;
	m_drpTemporalScale = NULL;
	m_flTemporalScale = 0.0f;
	m_flUIScale = 0.0f;
	m_btnAdvanced = NULL;

	m_bAcceptPowerSavingsWarning = false;

	m_nNumResolutionModes = 0;

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
	m_iCurrentResolutionWidth = config.m_VideoMode.m_Width;
	m_iCurrentResolutionHeight = config.m_VideoMode.m_Height;
	m_bCurrentWindowed = config.Windowed();
	gameui::GraphicsSettings current;
	current.width = config.m_VideoMode.m_Width;
	current.height = config.m_VideoMode.m_Height;
	current.windowed = config.Windowed();
#if defined( USE_SDL3 )
	CGameUIConVarRef borderless( "mat_borderless" );
	current.borderless = current.windowed && borderless.IsValid() && borderless.GetBool();
#elif !defined( POSIX )
	current.borderless = current.windowed && config.NoWindowBorder();
#endif
	CGameUIConVarRef uiScale( "ui_scale" );
	CGameUIConVarRef powerSaving( "mat_powersavingsmode" );
	current.uiScale = uiScale.IsValid() ? uiScale.GetFloat() : 0.0f;
	current.powerSaving = powerSaving.IsValid() ? clamp( powerSaving.GetInt(), 0, 1 ) : 0;
	CGameUIConVarRef temporalScale( "r_temporal_scale" );
	current.temporalScale = temporalScale.IsValid() ? temporalScale.GetFloat() : 1.0f;
	CGameUIConVarRef hdrMode( "mat_hdr_output" );
	CGameUIConVarRef hdrExposure( "mat_hdr_exposure" );
	CGameUIConVarRef hdrPeak( "mat_hdr_peak_nits" );
	current.hdr.automatic = hdrMode.IsValid() && hdrMode.GetBool();
	current.hdr.exposure = hdrExposure.IsValid() ? hdrExposure.GetFloat() : 1.0f;
	current.hdr.peakNits = hdrPeak.IsValid() ? hdrPeak.GetInt() : 1000;
	m_GraphicsSettings.Begin( current );

	GetRecommendedSettings();

	m_bPreferRecommendedResolution = false;
	m_bDirtyValues = false;
	m_bEnableApply = false;

	SetFooterEnabled( true );
	UpdateFooter();
}

Video::~Video()
{
}

void Video::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	if ( m_bCurrentWindowed )
	{
		m_pResourceLoadConditions->SetInt( "?windowed", 1 );
	}

	BaseClass::ApplySchemeSettings( pScheme );

	m_sldBrightness = dynamic_cast< SliderControl* >( FindChildByName( "SldBrightness" ) );
	m_drpAspectRatio = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpAspectRatio" ) );
	m_drpResolution = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpResolution" ) );
	m_drpDisplayMode = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpDisplayMode" ) );
	m_drpPowerSavingsMode = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpPowerSavingsMode" ) );
	m_drpSplitScreenDirection = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpSplitScreenDirection" ) );
	m_drpUIScale = dynamic_cast<BaseModHybridButton *>( FindChildByName( "DrpUIScale" ) );
	m_drpTemporalScale =
	    dynamic_cast<BaseModHybridButton *>( FindChildByName( "DrpTemporalScale" ) );

	SetupState( false );

	if ( m_sldBrightness )
	{
		m_sldBrightness->Reset();

		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		m_sldBrightness->NavigateTo();
	}

	if ( m_drpSplitScreenDirection )
	{
		const AspectRatioInfo_t &aspectRatioInfo = Portal2_GetAspectRatioInfo();
		bool bWidescreen = aspectRatioInfo.m_bIsWidescreen;

		if ( !bWidescreen )
		{
			m_drpSplitScreenDirection->SetEnabled( false );
			m_drpSplitScreenDirection->SetCurrentSelection( "#L4D360UI_SplitScreenDirection_Horizontal" );
		}
		else
		{
			CGameUIConVarRef ss_splitmode( "ss_splitmode" );
			int iSplitMode = ss_splitmode.GetInt();

			switch ( iSplitMode )
			{
			case 1:
				m_drpSplitScreenDirection->SetCurrentSelection( "#L4D360UI_SplitScreenDirection_Horizontal" );
				break;
			case 2:
				m_drpSplitScreenDirection->SetCurrentSelection( "#L4D360UI_SplitScreenDirection_Vertical" );
				break;
			default:
				m_drpSplitScreenDirection->SetCurrentSelection( "#L4D360UI_SplitScreenDirection_Default" );
			}
		}
	}

	UpdateFooter();
}

bool Video::GetRecommendedSettings( void )
{
	m_iRecommendedResolutionWidth = 640;
	m_iRecommendedResolutionHeight = 480; 
	m_iRecommendedAspectRatio = GetScreenAspectMode( m_iRecommendedResolutionWidth, m_iRecommendedResolutionHeight );
	m_bRecommendedWindowed = false;
	m_bRecommendedNoBorder = false;
	
	// Off by default since we aren't dynamically adjusting the state and don't want to get users into a hole.
	// In the future, we could set this via steamapicontext->SteamUtils()->GetCurrentBatteryPower() == 255 ? 0 : 1; // 255 indicates AC power, else battery
	m_nRecommendedPowerSavingsMode = 0;

#if !defined( _GAMECONSOLE )
	KeyValues *pConfigKeys = new KeyValues( "VideoConfig" );
	if ( !pConfigKeys )
		return false;

	if ( !ReadCurrentVideoConfig( pConfigKeys, true ) )
	{
		pConfigKeys->deleteThis();
		return false;
	}

	m_iRecommendedResolutionWidth = pConfigKeys->GetInt( "setting.defaultres", m_iRecommendedResolutionWidth );
	m_iRecommendedResolutionHeight = pConfigKeys->GetInt( "setting.defaultresheight", m_iRecommendedResolutionHeight );
	m_iRecommendedAspectRatio = GetScreenAspectMode( m_iRecommendedResolutionWidth, m_iRecommendedResolutionHeight );
	m_bRecommendedWindowed = !pConfigKeys->GetBool( "setting.fullscreen", !m_bRecommendedWindowed );
	m_bRecommendedNoBorder = pConfigKeys->GetBool( "setting.nowindowborder", m_bRecommendedNoBorder );

	pConfigKeys->deleteThis();
#endif

	return true;
}

void Video::Activate()
{
	BaseClass::Activate();

	UpdateFooter();
}

static void AcceptDefaultsOkCallback()
{
	Video *pSelf = 
		static_cast< Video* >( CBaseModPanel::GetSingleton().GetWindow( WT_VIDEO ) );
	if ( pSelf )
	{
		pSelf->SetDefaults();
	}
}

static void DiscardChangesOkCallback()
{
	Video *pSelf = 
		static_cast< Video* >( CBaseModPanel::GetSingleton().GetWindow( WT_VIDEO ) );
	if ( pSelf )
	{
		pSelf->DiscardChangesAndClose();
	}
}

void Video::DiscardChangesAndClose()
{
	m_GraphicsSettings.Cancel();
	if ( !m_bCurrentWindowed )
	{
		// the brightness slider is not part of apply, so need to restore when discarding
		CGameUIConVarRef mat_monitorgamma( "mat_monitorgamma" );
		if ( m_flOriginalGamma != mat_monitorgamma.GetFloat() )
		{
			mat_monitorgamma.SetValue( m_flOriginalGamma );
		}
	}

	BaseClass::OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
}

void Video::SetDefaults()
{
	SetupState( true );
}

void Video::SetupState( bool bUseRecommendedSettings )
{
	if ( !bUseRecommendedSettings )
	{
		const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
		m_iResolutionWidth = config.m_VideoMode.m_Width;
		m_iResolutionHeight = config.m_VideoMode.m_Height;
		m_iAspectRatio = GetScreenAspectMode( m_iResolutionWidth, m_iResolutionHeight );
		m_bWindowed = config.Windowed();
#if defined( USE_SDL3 )
		CGameUIConVarRef borderless( "mat_borderless" );
		m_bNoBorder = config.Windowed() && borderless.IsValid() && borderless.GetBool();
#elif !defined( POSIX )
		m_bNoBorder = config.Windowed() && config.NoWindowBorder();
#else
		m_bNoBorder = false;
#endif
		CGameUIConVarRef mat_powersavingsmode( "mat_powersavingsmode" );
		m_nPowerSavingsMode = clamp( mat_powersavingsmode.GetInt(), 0, 1 );
		CGameUIConVarRef ui_scale( "ui_scale" );
		m_flUIScale = ui_scale.IsValid() ? ui_scale.GetFloat() : 0.0f;
		CGameUIConVarRef temporalScale( "r_temporal_scale" );
		m_flTemporalScale = temporalScale.IsValid() ? temporalScale.GetFloat() : 1.0f;
	}
	else
	{
		m_iResolutionWidth = m_iRecommendedResolutionWidth;
		m_iResolutionHeight = m_iRecommendedResolutionHeight;
		m_iAspectRatio = m_iRecommendedAspectRatio;
		m_bWindowed = m_bRecommendedWindowed;
#if !defined( POSIX )
		m_bNoBorder = m_bRecommendedNoBorder;
#else
		m_bNoBorder = false;
#endif
		m_nPowerSavingsMode = m_nRecommendedPowerSavingsMode;
		m_flUIScale = 0.0f;
		m_flTemporalScale = 2.0f / 3.0f;

		m_bDirtyValues = true;
		m_bPreferRecommendedResolution = true;
	}

	PrepareResolutionList();

	SetControlEnabled( "SldBrightness", !m_bCurrentWindowed );

	if ( m_drpAspectRatio )
	{
		switch ( m_iAspectRatio )
		{
		default:
		case 0:
			m_drpAspectRatio->SetCurrentSelection( "#GameUI_AspectNormal" );
			break;
		case 1:
			m_drpAspectRatio->SetCurrentSelection( "#GameUI_AspectWide16x9" );
			break;
		case 2:
			m_drpAspectRatio->SetCurrentSelection( "#GameUI_AspectWide16x10" );
			break;
		}
	}

	if ( m_drpDisplayMode )
	{
		if ( m_bWindowed )
		{
#if defined( USE_SDL3 )
			if ( m_bNoBorder )
			{
				m_drpDisplayMode->SetCurrentSelection( "Borderless windowed" );
			}
			else
#elif !defined( POSIX )
			if ( m_bNoBorder )
			{
				m_drpDisplayMode->SetCurrentSelection( "#L4D360UI_VideoOptions_Windowed_NoBorder" );
			}
			else
#endif
			{
				m_drpDisplayMode->SetCurrentSelection( "#GameUI_Windowed" );
			}
		}
		else
		{
			m_drpDisplayMode->SetCurrentSelection( "#GameUI_Fullscreen" );
		}
	}

	SetPowerSavingsState();
	SetUIScaleState();
	SetTemporalScaleState();
}

void Video::OnKeyCodePressed(KeyCode code)
{
	int joystick = GetJoystickForCode( code );
	int userId = CBaseModPanel::GetSingleton().GetLastActiveUserId();
	if ( joystick != userId || joystick < 0 )
	{	
		return;
	}

	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_A:
		// apply changes and close
		if ( ApplyChanges() )
			BaseClass::OnKeyCodePressed( ButtonCodeToJoystickButtonCode(
			    KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
		break;

	case KEY_XBUTTON_B:
		if ( m_bDirtyValues || m_GraphicsSettings.GetState() == gameui::GraphicsSettingsService::State::Editing ||
	     m_GraphicsSettings.GetState() == gameui::GraphicsSettingsService::State::Applied || ( m_sldBrightness && m_sldBrightness->IsDirty() ) )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

			GenericConfirmation *pConfirmation = 
				static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, true ) );

			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#PORTAL2_VideoSettingsConf";
			data.pMessageText = "#PORTAL2_VideoSettingsDiscardQ";
			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &DiscardChangesOkCallback;
			data.pOkButtonText = "#PORTAL2_ButtonAction_Discard";
			data.bCancelButtonEnabled = true;
			pConfirmation->SetUsageData( data );
		}
		else
		{
			// cancel
			BaseClass::OnKeyCodePressed( code );
		}
		break;

	case KEY_XBUTTON_X:
		// use defaults
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

			GenericConfirmation *pConfirmation = 
				static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, true ) );

			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#PORTAL2_VideoSettingsConf";
			data.pMessageText = "#PORTAL2_VideoSettingsUseDefaultsQ";
			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptDefaultsOkCallback;
			data.pOkButtonText = "#PORTAL2_ButtonAction_Reset";
			data.bCancelButtonEnabled = true;
			pConfirmation->SetUsageData( data );
		}
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

void Video::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "#GameUI_AspectNormal" ) )
	{
		m_iAspectRatio = 0;
		m_bDirtyValues = true;
		PrepareResolutionList();
	}
	else if ( !V_stricmp( command, "#GameUI_AspectWide16x9" ) )
	{
		m_iAspectRatio = 1;
		m_bDirtyValues = true;
		PrepareResolutionList();
	}
	else if ( !V_stricmp( command, "#GameUI_AspectWide16x10" ) )
	{
		m_iAspectRatio = 2;
		m_bDirtyValues = true;
		PrepareResolutionList();
	}
	else if ( StringHasPrefix( command, VIDEO_RESOLUTION_COMMAND_PREFIX ) )
	{
		int iCommandNumberPosition = Q_strlen( VIDEO_RESOLUTION_COMMAND_PREFIX );
		int iResolution = clamp( command[ iCommandNumberPosition ] - '0', 0, m_nNumResolutionModes - 1 );

		m_iResolutionWidth = m_nResolutionModes[iResolution].m_nWidth;
		m_iResolutionHeight = m_nResolutionModes[iResolution].m_nHeight;

		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "#GameUI_Windowed" ) )
	{
		m_bWindowed = true;
		m_bNoBorder = false;
		m_bDirtyValues = true;
		PrepareResolutionList();
	}
#if !defined( POSIX )
	else if ( !V_stricmp( command, "#L4D360UI_VideoOptions_Windowed_NoBorder" ) )
	{
		m_bWindowed = true;
		m_bNoBorder = true;
		m_bDirtyValues = true;
		PrepareResolutionList();
	}
#endif
#if defined( USE_SDL3 )
	else if ( !V_stricmp( command, "BorderlessWindowed" ) )
	{
		m_bWindowed = true;
		m_bNoBorder = true;
		m_bDirtyValues = true;
		PrepareResolutionList();
	}
#endif
	else if ( !V_stricmp( command, "#GameUI_Fullscreen" ) )
	{
		m_bWindowed = false;
		m_bNoBorder = false;
		m_bDirtyValues = true;
		PrepareResolutionList();
	}
	else if ( StringHasPrefix( command, VIDEO_UISCALE_COMMAND_PREFIX ) )
	{
		const int nChoice = atoi( command + Q_strlen( VIDEO_UISCALE_COMMAND_PREFIX ) );
		if ( nChoice >= 0 && nChoice < ARRAYSIZE( s_UIScaleChoices ) )
		{
			m_flUIScale = s_UIScaleChoices[nChoice];
			m_bDirtyValues = true;
		}
	}
	else if ( StringHasPrefix( command, VIDEO_TEMPORALSCALE_COMMAND_PREFIX ) )
	{
		const int choice = atoi( command + Q_strlen( VIDEO_TEMPORALSCALE_COMMAND_PREFIX ) );
		if ( choice >= 0 && choice < ARRAYSIZE( s_TemporalScales ) && g_pRenderTemporalViews &&
		     g_pRenderTemporalViews->Available() )
		{
			m_flTemporalScale = s_TemporalScales[choice];
			m_bDirtyValues = true;
			SetTemporalScaleState();
		}
	}
	else if ( !V_stricmp( command, "ShowHDR" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_HDRVIDEO, this, true );
	}
	else if ( !V_stricmp( command, "ShowAdvanced" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_ADVANCEDVIDEO, this, true );
	}
	else if ( !V_stricmp( command, "PowerSavingsDisabled" ) )
	{
		m_nPowerSavingsMode = 0;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "PowerSavingsEnabled" ) )
	{
		m_nPowerSavingsMode = 1;
		m_bDirtyValues = true;
	}
	else if ( !Q_stricmp( command, "#L4D360UI_SplitScreenDirection_Default" ) )
	{
		if ( m_drpSplitScreenDirection && m_drpSplitScreenDirection->IsEnabled() )
		{
			CGameUIConVarRef ss_splitmode( "ss_splitmode" );
			ss_splitmode.SetValue( 0 );
			m_bDirtyValues = true;
		}
	}
	else if ( !Q_stricmp( command, "#L4D360UI_SplitScreenDirection_Horizontal" ) )
	{
		if ( m_drpSplitScreenDirection && m_drpSplitScreenDirection->IsEnabled() )
		{
			CGameUIConVarRef ss_splitmode( "ss_splitmode" );
			ss_splitmode.SetValue( 1 );
			m_bDirtyValues = true;
		}
	}
	else if ( !Q_stricmp( command, "#L4D360UI_SplitScreenDirection_Vertical" ) )
	{
		if ( m_drpSplitScreenDirection && m_drpSplitScreenDirection->IsEnabled() )
		{
			CGameUIConVarRef ss_splitmode( "ss_splitmode" );
			ss_splitmode.SetValue( 2 );
			m_bDirtyValues = true;
		}
	}
	else if ( !V_stricmp( "Cancel", command ) || !V_stricmp( "Back", command ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

static vmode_t s_pWindowedModes[] = 
{
	// NOTE: These must be sorted by ascending width, then ascending height
	{ 640, 480, 32, 60 },
	{ 852, 480, 32, 60 },
	{ 1280, 720, 32, 60 },
	{ 1920, 1080, 32, 60 },
};

static void GenerateWindowedModes( CUtlVector< vmode_t > &windowedModes, int nCount, vmode_t *pFullscreenModes )
{
	int nFSMode = 0;
	for ( int i = 0; i < ARRAYSIZE( s_pWindowedModes ); ++i )
	{
		while ( true )
		{
			if ( nFSMode >= nCount )
				break;

			if ( pFullscreenModes[nFSMode].width > s_pWindowedModes[i].width )
				break;

			if ( pFullscreenModes[nFSMode].width == s_pWindowedModes[i].width )
			{
				if ( pFullscreenModes[nFSMode].height > s_pWindowedModes[i].height )
					break;

				if ( pFullscreenModes[nFSMode].height == s_pWindowedModes[i].height )
				{
					// Don't add the matching fullscreen mode
					++nFSMode;
					break;
				}
			}

			windowedModes.AddToTail( pFullscreenModes[nFSMode] );
			++nFSMode;
		}

		windowedModes.AddToTail( s_pWindowedModes[i] );
	}

	for ( ; nFSMode < nCount; ++nFSMode )
	{
		windowedModes.AddToTail( pFullscreenModes[nFSMode] );
	}
}

static bool HasResolutionMode( const ResolutionMode_t * RESTRICT pModes, const int numModes, const vmode_t * RESTRICT pTestMode )
{
	for ( int i = 0 ; i < numModes ; ++i )
	{
		if ( pModes[ i ].m_nWidth  == pTestMode->width   &&
			 pModes[ i ].m_nHeight == pTestMode->height )
		{
			return true;
		}
	}

	return false;
}

void Video::GetResolutionName( vmode_t *pMode, char *pOutBuffer, int nOutBufferSize, bool &bIsNative )
{
	int desktopWidth, desktopHeight;
	gameuifuncs->GetDesktopResolution( desktopWidth, desktopHeight );
	V_snprintf( pOutBuffer, nOutBufferSize, "%i x %i", pMode->width, pMode->height );
	bIsNative = ( pMode->width == desktopWidth ) && ( pMode->height == desktopHeight );
}

void Video::PrepareResolutionList()
{
	if ( !m_drpResolution )
		return;

	int iOverflowPosition = 1;
	m_nNumResolutionModes = 0;

	// Set up the base string for each button command
	char szCurrentButton[ 32 ];
	V_strncpy( szCurrentButton, VIDEO_RESOLUTION_COMMAND_PREFIX, sizeof( szCurrentButton ) );

	int iCommandNumberPosition = V_strlen( szCurrentButton );
	szCurrentButton[ iCommandNumberPosition + 1 ] = '\0';

	// get full video mode list
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes( &plist, &count );

	int desktopWidth, desktopHeight;
	gameuifuncs->GetDesktopResolution( desktopWidth, desktopHeight );

	// Add some extra modes in if we're running windowed
	CUtlVector< vmode_t > windowedModes;
	if ( m_bWindowed )
	{
		GenerateWindowedModes( windowedModes, count, plist );
		count = windowedModes.Count();
		plist = windowedModes.Base();
	}

	if ( m_drpAspectRatio )
	{
		m_drpAspectRatio->EnableListItem( "#GameUI_AspectNormal", false );
		m_drpAspectRatio->EnableListItem( "#GameUI_AspectWide16x9", false );
		m_drpAspectRatio->EnableListItem( "#GameUI_AspectWide16x10", false );
	}

	// iterate all the video modes adding them to the dropdown
	for ( int i = 0; i < count; i++, plist++ )
	{
		// don't show modes bigger than the desktop for windowed mode
		if ( m_bWindowed && ( plist->width > desktopWidth || plist->height > desktopHeight ) )
			continue;

		// skip a mode if it is somehow already in the list (bug 30693)
		if ( HasResolutionMode( m_nResolutionModes, m_nNumResolutionModes, plist ) )
			continue;

		bool bIsNative;
		char szResolutionName[ 256 ];
		GetResolutionName( plist, szResolutionName, sizeof( szResolutionName ), bIsNative );

		int iAspectMode = GetScreenAspectMode( plist->width, plist->height );

		if ( iAspectMode >= 0 )
		{
			if ( m_drpAspectRatio )
			{
				switch ( iAspectMode )
				{
				default:
				case 0:
					m_drpAspectRatio->EnableListItem( "#GameUI_AspectNormal", true );
					break;
				case 1:
					m_drpAspectRatio->EnableListItem( "#GameUI_AspectWide16x9", true );
					break;
				case 2:
					m_drpAspectRatio->EnableListItem( "#GameUI_AspectWide16x10", true );
					break;
				}
			}
		}

		// filter the list for those matching the current aspect
		if ( iAspectMode == m_iAspectRatio )
		{
			if ( m_nNumResolutionModes == MAX_DYNAMIC_VIDEO_MODES )
			{
				// No more will fit in this drop down, and it's too late in the product to make this a scrollable list
				// Instead lets make the list less fine grained by removing middle entries
				if ( iOverflowPosition >= MAX_DYNAMIC_VIDEO_MODES )
				{
					// Wrap the second entry
					iOverflowPosition = 1;
				}

				int iShiftPosition = iOverflowPosition;

				while ( iShiftPosition < MAX_DYNAMIC_VIDEO_MODES - 1 )
				{
					// Copy the entry in front of us over this entry
					szCurrentButton[ iCommandNumberPosition ] = ( iShiftPosition + 1 ) + '0';

					char szResName[ 256 ];
					if ( m_drpResolution->GetListSelectionString( szCurrentButton, szResName, sizeof(szResName) ) )
					{
						szCurrentButton[ iCommandNumberPosition ] = iShiftPosition + '0';
						m_drpResolution->ModifySelectionString( szCurrentButton, szResName );
					}

					m_nResolutionModes[ iShiftPosition ].m_nWidth = m_nResolutionModes[ iShiftPosition + 1 ].m_nWidth;
					m_nResolutionModes[ iShiftPosition ].m_nHeight = m_nResolutionModes[ iShiftPosition + 1 ].m_nHeight;

					iShiftPosition++;
				}

				iOverflowPosition += 2;
				m_nNumResolutionModes--;
			}

			szCurrentButton[ iCommandNumberPosition ] = m_nNumResolutionModes + '0';

			if ( bIsNative )
			{
#if defined(POSIX)
				V_strncat( szResolutionName, " %S", sizeof( szResolutionName ) );
#else
				V_strncat( szResolutionName, " %s", sizeof( szResolutionName ) );
#endif
				m_drpResolution->ModifySelectionStringParms( szCurrentButton, "#L4D360UI_Native" );
			}

			m_drpResolution->ModifySelectionString( szCurrentButton, szResolutionName );

			m_nResolutionModes[ m_nNumResolutionModes ].m_nWidth = plist->width;
			m_nResolutionModes[ m_nNumResolutionModes ].m_nHeight = plist->height;

			m_nNumResolutionModes++;
		}
	}

	// Enable the valid possible choices
	for ( int i = 0; i < m_nNumResolutionModes; ++i )
	{
		szCurrentButton[ iCommandNumberPosition ] = i + '0';
		char szString[256];
		if ( m_drpResolution->GetListSelectionString( szCurrentButton, szString, sizeof( szString ) ) )
		{
			m_drpResolution->EnableListItem( szString, true );
		}
	}

	// Disable the remaining possible choices
	for ( int i = m_nNumResolutionModes; i < MAX_DYNAMIC_VIDEO_MODES; ++i )
	{
		szCurrentButton[ iCommandNumberPosition ] = i + '0';
		char szString[256];
		if ( m_drpResolution->GetListSelectionString( szCurrentButton, szString, sizeof( szString ) ) )
		{
			m_drpResolution->EnableListItem( szString, false );
		}
	}

	// Find closest selection
	int selectedItemID;
	for ( selectedItemID = 0; selectedItemID < m_nNumResolutionModes; ++selectedItemID )
	{
		if ( m_nResolutionModes[ selectedItemID ].m_nHeight > m_iResolutionHeight )
			break;

		if (( m_nResolutionModes[ selectedItemID ].m_nHeight == m_iResolutionHeight ) &&
			( m_nResolutionModes[ selectedItemID ].m_nWidth > m_iResolutionWidth ))
			break;
	}

	// Go back to the one that matched or was smaller (and prevent -1 when none are smaller)
	selectedItemID = MAX( selectedItemID - 1, 0 );

	if ( m_nResolutionModes[ selectedItemID ].m_nWidth != m_iResolutionWidth ||
		m_nResolutionModes[ selectedItemID ].m_nHeight != m_iResolutionHeight )
	{
		// not an exact match, try to find closest to current or recommended
		int nDesiredPixels = m_iCurrentResolutionWidth * m_iCurrentResolutionHeight;
		if ( m_bPreferRecommendedResolution )
		{
			nDesiredPixels = m_iRecommendedResolutionWidth * m_iRecommendedResolutionHeight;
		}
		int nBetterMode = -1;
		int nBest = INT_MAX;
		for ( int i = 0; i < m_nNumResolutionModes; ++i )
		{
			int nPixels = m_nResolutionModes[i].m_nWidth * m_nResolutionModes[i].m_nHeight;
			int nClosest = abs( nDesiredPixels - nPixels );
			if ( nBest > nClosest )
			{
				nBest = nClosest;
				nBetterMode = i;
			}
		}
			
		if ( nBetterMode != -1 )
		{
			selectedItemID = nBetterMode;
		}
	}

	// Select the currently set type
	szCurrentButton[ iCommandNumberPosition ] = selectedItemID + '0';
	char szString[256];
	if ( m_drpResolution->GetListSelectionString( szCurrentButton, szString, sizeof( szString ) ) )
	{
		m_drpResolution->SetCurrentSelection( szString );
	}

	m_iResolutionWidth = m_nResolutionModes[selectedItemID].m_nWidth;
	m_iResolutionHeight = m_nResolutionModes[selectedItemID].m_nHeight;
	SetTemporalScaleState();
}

// Portal's command adapter; the session owns draft/apply/save transitions.
class PortalGraphicsBackend final : public gameui::IGraphicsSettingsBackend
{
public:
	bool Apply( const gameui::GraphicsSettings &from, const gameui::GraphicsSettings &to ) override
	{
		CGameUIConVarRef powerSaving( "mat_powersavingsmode" );
		CGameUIConVarRef uiScale( "ui_scale" );
		CGameUIConVarRef temporalScale( "r_temporal_scale" );
		CGameUIConVarRef antialias( "mat_antialias" );
		CGameUIConVarRef hdrMode( "mat_hdr_output" );
		CGameUIConVarRef hdrExposure( "mat_hdr_exposure" );
		CGameUIConVarRef hdrPeak( "mat_hdr_peak_nits" );
		if ( from.hdr != to.hdr &&
		     ( !hdrMode.IsValid() || !hdrExposure.IsValid() || !hdrPeak.IsValid() ) )
			return false;
		if ( to.temporalScale != 0.0f && !antialias.IsValid() )
			return false;
		if ( from.temporalScale != to.temporalScale &&
		     ( !temporalScale.IsValid() || !g_pRenderTemporalViews ||
		         !g_pRenderTemporalViews->Available() ) )
			return false;
		if ( ( from.powerSaving != to.powerSaving && !powerSaving.IsValid() ) ||
		     ( from.uiScale != to.uiScale && !uiScale.IsValid() ) )
			return false;
		if ( to.borderless && !to.windowed )
			return false;
		if ( to.temporalScale != 0.0f && antialias.GetInt() != 0 )
			antialias.SetValue( 0 );
		if ( from.width != to.width || from.height != to.height || from.windowed != to.windowed ||
		     from.borderless != to.borderless )
		{
			char command[256];
			V_snprintf( command, sizeof( command ), "mat_setvideomode %d %d %d %d\n", to.width,
			    to.height, to.windowed ? 1 : 0, to.borderless ? 1 : 0 );
			engine->ClientCmd_Unrestricted( command );
		}
		if ( from.powerSaving != to.powerSaving )
			powerSaving.SetValue( to.powerSaving );
		if ( from.uiScale != to.uiScale )
			uiScale.SetValue( to.uiScale );
		if ( from.temporalScale != to.temporalScale )
			temporalScale.SetValue( to.temporalScale );
		if ( from.hdr != to.hdr )
		{
			hdrMode.SetValue( to.hdr.automatic ? 1 : 0 );
			hdrExposure.SetValue( to.hdr.exposure );
			hdrPeak.SetValue( to.hdr.peakNits );
		}
		return true;
	}

	bool Save() override
	{
		engine->ClientCmd_Unrestricted( "mat_savechanges\n" );
		engine->ClientCmd_Unrestricted(
		    VarArgs( "host_writeconfig_ss %d", XBX_GetPrimaryUserId() ) );
		return true;
	}
};

bool Video::ApplyChanges()
{
	if ( m_bDirtyValues || m_GraphicsSettings.GetState() == gameui::GraphicsSettingsService::State::Editing ||
	     m_GraphicsSettings.GetState() == gameui::GraphicsSettingsService::State::Applied || ( m_sldBrightness && m_sldBrightness->IsDirty() ) )
	{
		gameui::GraphicsSettings desired = m_GraphicsSettings.Draft();
		desired.width = m_iResolutionWidth;
		desired.height = m_iResolutionHeight;
		desired.windowed = m_bWindowed;
		desired.borderless = m_bNoBorder;
		desired.powerSaving = m_nPowerSavingsMode;
		if ( g_pRenderTemporalViews && g_pRenderTemporalViews->Available() )
			desired.temporalScale = m_flTemporalScale;
		// Keep an exact console value when the selected menu choice is its nearest label.
		if ( GetUIScaleChoice( desired.uiScale ) != GetUIScaleChoice( m_flUIScale ) )
			desired.uiScale = m_flUIScale;
		PortalGraphicsBackend backend;
		if ( !m_GraphicsSettings.Stage( desired ) || !m_GraphicsSettings.Apply( backend ) ||
		     !m_GraphicsSettings.Save( backend ) )
			return false;

		// Update the current video config file.
#if !defined( _GAMECONSOLE )
		int nAspectRatioMode = GetScreenAspectMode( desired.width, desired.height );
#if defined( USE_SDL3 ) || !defined( POSIX )
		UpdateCurrentVideoConfig( desired.width, desired.height, nAspectRatioMode,
		    !desired.windowed, desired.borderless );
#else
		// Portal 2 port: POSIX builds have no borderless window mode (as retail).
		UpdateCurrentVideoConfig(
		    desired.width, desired.height, nAspectRatioMode, !desired.windowed, false );
#endif
#endif

		m_bDirtyValues = false;
	}
	return true;
}

void Video::OnThink()
{
	BaseClass::OnThink();

	const bool dirty = m_bDirtyValues || m_GraphicsSettings.GetState() == gameui::GraphicsSettingsService::State::Editing ||
	    m_GraphicsSettings.GetState() == gameui::GraphicsSettingsService::State::Applied;
	if ( m_bEnableApply != dirty )
	{
		// enable the apply button
		m_bEnableApply = dirty;
		UpdateFooter();
	}
}

void Video::UpdateFooter()
{
	CBaseModFooterPanel *pFooter = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( pFooter )
	{
		int visibleButtons = FB_BBUTTON | FB_XBUTTON;
		if ( m_bEnableApply )
		{
			visibleButtons |= FB_ABUTTON;
		}

		pFooter->SetButtons( visibleButtons );
		pFooter->SetButtonText( FB_ABUTTON, "#GameUI_Apply" );
		pFooter->SetButtonText( FB_BBUTTON, "#L4D360UI_Back" );
		pFooter->SetButtonText( FB_XBUTTON, "#GameUI_UseDefaults" );
	}
}

void AcceptPowerSavingsWarningCallback()
{
	Video *pSelf = static_cast< Video* >( CBaseModPanel::GetSingleton().GetWindow( WT_VIDEO ) );
	if ( pSelf )
	{
		pSelf->AcceptPowerSavingsWarningCallback();
	}
}

void Video::ShowPowerSavingsWarning()
{
	GenericConfirmation* pConfirmation = 
		static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this ) );

	GenericConfirmation::Data_t data;

	data.pWindowTitle = "#GameUI_PowerSavingsMode";
	data.pMessageText = "#PORTAL2_VideoOptions_PowerSavings_Info";

	data.bOkButtonEnabled = true;
	data.pfnOkCallback = ::AcceptPowerSavingsWarningCallback;

	pConfirmation->SetUsageData( data );
}

void Video::AcceptPowerSavingsWarningCallback()
{
	m_bAcceptPowerSavingsWarning = true;
}

void Video::SetTemporalScaleState()
{
	if ( !m_drpTemporalScale )
		return;
	const bool available = g_pRenderTemporalViews && g_pRenderTemporalViews->Available();
	m_drpTemporalScale->SetEnabled( available );
	if ( !available )
	{
		m_drpTemporalScale->ModifySelectionString(
		    "_temporalscale0", "Unavailable in this session" );
		m_drpTemporalScale->SetCurrentSelection( "Unavailable in this session" );
		return;
	}
	int selected = 0;
	for ( int i = 1; i < ARRAYSIZE( s_TemporalScales ); ++i )
		if ( fabsf( m_flTemporalScale - s_TemporalScales[i] ) <
		     fabsf( m_flTemporalScale - s_TemporalScales[selected] ) )
			selected = i;
	for ( int i = 0; i < ARRAYSIZE( s_TemporalScales ); ++i )
	{
		char text[96];
		const float scale = i == selected ? m_flTemporalScale : s_TemporalScales[i];
		if ( scale == 0.0f )
			V_strncpy( text, s_TemporalScaleNames[i], sizeof( text ) );
		else
			V_snprintf( text, sizeof( text ), "%s: %d x %d", s_TemporalScaleNames[i],
			    int( m_iResolutionWidth * scale ), int( m_iResolutionHeight * scale ) );
		m_drpTemporalScale->ModifySelectionString(
		    CFmtStr( "%s%d", VIDEO_TEMPORALSCALE_COMMAND_PREFIX, i ), text );
		if ( i == selected )
			m_drpTemporalScale->SetCurrentSelection( text );
	}
}

void Video::SetUIScaleState()
{
	if ( m_drpUIScale )
	{
		char szName[32];
		GetUIScaleChoiceName( GetUIScaleChoice( m_flUIScale ), szName, sizeof( szName ) );
		m_drpUIScale->SetCurrentSelection( szName );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Places the UI scale row where the Advanced button was and moves the
//			button down a row, in the base settings and in each conditional
//			block ("?windowed" moves the rows up when brightness is hidden).
//			Returns the Advanced button's new base ypos.
//-----------------------------------------------------------------------------
static int MoveRowBelow( KeyValues *pAbove, KeyValues *pScale, KeyValues *pAdvanced )
{
	const int nRowY = pAdvanced->GetInt( "ypos" );
	int nPitch = nRowY - pAbove->GetInt( "ypos" );
	if ( nPitch <= 0 )
		nPitch = 25;

	for ( KeyValues *pCondition = pAdvanced->GetFirstTrueSubKey(); pCondition;
	    pCondition = pCondition->GetNextTrueSubKey() )
	{
		if ( pCondition->GetName()[0] != '?' || !pCondition->FindKey( "ypos" ) )
			continue;
		KeyValues *pAboveCondition = pAbove->FindKey( pCondition->GetName() );
		const int nConditionRowY = pCondition->GetInt( "ypos" );
		int nConditionPitch =
		    nConditionRowY - ( pAboveCondition && pAboveCondition->FindKey( "ypos" )
		                             ? pAboveCondition->GetInt( "ypos" )
		                             : pAbove->GetInt( "ypos" ) );
		if ( nConditionPitch <= 0 )
			nConditionPitch = nPitch;
		pScale->FindKey( pCondition->GetName(), true )->SetInt( "ypos", nConditionRowY );
		pCondition->SetInt( "ypos", nConditionRowY + nConditionPitch );
	}

	pScale->SetInt( "ypos", nRowY );
	pAdvanced->SetInt( "ypos", nRowY + nPitch );
	return nRowY + nPitch;
}

//-----------------------------------------------------------------------------
// Purpose: Adds the UI scale row above the Advanced button: a copy of the row
//			above it (so it looks and navigates like the others) with the
//			ui_scale choices. The rows below it move down one row, and the
//			dialog grows by a tile when they no longer fit.
//-----------------------------------------------------------------------------
void Video::PreApplyControlSettings( KeyValues *pResourceData )
{
#if defined( USE_SDL3 )
	if ( pResourceData )
	{
		KeyValues *pDisplay = pResourceData->FindKey( "DrpDisplayMode" );
		if ( pDisplay )
			pDisplay->FindKey( "list", true )
			    ->SetString( "Borderless windowed", "BorderlessWindowed" );
	}
#endif
	if ( !pResourceData || pResourceData->FindKey( "DrpUIScale" ) )
		return;

	KeyValues *pAdvanced = pResourceData->FindKey( "BtnAdvanced" );
	KeyValues *pAbove =
	    pAdvanced ? pResourceData->FindKey( pAdvanced->GetString( "navUp" ) ) : NULL;
	if ( !pAbove || V_stricmp( pAbove->GetString( "style" ), "DialogListButton" ) )
		return;

	KeyValues *pScale = pAbove->MakeCopy();
	pScale->SetName( "DrpUIScale" );
	pScale->SetString( "fieldName", "DrpUIScale" );
	pScale->SetString( "labelText", "UI Scale" );
	pScale->SetString( "navUp", pAbove->GetName() );
	pScale->SetString( "navDown", "BtnAdvanced" );
	if ( KeyValues *pOldList = pScale->FindKey( "list" ) )
	{
		pScale->RemoveSubKey( pOldList );
		pOldList->deleteThis();
	}
	KeyValues *pList = pScale->FindKey( "list", true );
	for ( int i = 0; i < ARRAYSIZE( s_UIScaleChoices ); ++i )
	{
		char szName[32];
		GetUIScaleChoiceName( i, szName, sizeof( szName ) );
		pList->SetString( szName, CFmtStr( "%s%d", VIDEO_UISCALE_COMMAND_PREFIX, i ) );
	}
	pResourceData->AddSubKey( pScale );

	pAbove->SetString( "navDown", "DrpUIScale" );
	pAdvanced->SetString( "navUp", "DrpUIScale" );
	MoveRowBelow( pAbove, pScale, pAdvanced );

	KeyValues *pTemporal = pScale->MakeCopy();
	pTemporal->SetName( "DrpTemporalScale" );
	pTemporal->SetString( "fieldName", "DrpTemporalScale" );
	pTemporal->SetString( "labelText", "FSR render scale" );
	pTemporal->SetString( "navUp", "DrpUIScale" );
	KeyValues *pTemporalList = pTemporal->FindKey( "list" );
	pTemporalList->Clear();
	for ( int i = 0; i < ARRAYSIZE( s_TemporalScales ); ++i )
		pTemporalList->SetString(
		    s_TemporalScaleNames[i], CFmtStr( "%s%d", VIDEO_TEMPORALSCALE_COMMAND_PREFIX, i ) );
	pResourceData->AddSubKey( pTemporal );
	pScale->SetString( "navDown", "DrpTemporalScale" );
	pAdvanced->SetString( "navUp", "DrpTemporalScale" );
	MoveRowBelow( pScale, pTemporal, pAdvanced );
	KeyValues *hdr = pAdvanced->MakeCopy();
	hdr->SetName( "BtnHDR" );
	hdr->SetString( "fieldName", "BtnHDR" );
	hdr->SetString( "labelText", "HDR" );
	hdr->SetString( "command", "ShowHDR" );
	hdr->SetString( "navUp", "DrpTemporalScale" );
	hdr->SetString( "navDown", "BtnAdvanced" );
	pTemporal->SetString( "navDown", "BtnHDR" );
	pAdvanced->SetString( "navUp", "BtnHDR" );
	pResourceData->AddSubKey( hdr );
	const int nAdvancedY = MoveRowBelow( pTemporal, hdr, pAdvanced );

	// The frame's size is in dialog tiles (Dialog.TileHeight, the same
	// proportional units as the rows).
	KeyValues *pFrame = NULL;
	for ( KeyValues *pKey = pResourceData->GetFirstTrueSubKey(); pKey;
	    pKey = pKey->GetNextTrueSubKey() )
	{
		if ( !V_stricmp( pKey->GetString( "ControlName" ), "Frame" ) )
		{
			pFrame = pKey;
			break;
		}
	}
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
	const int nTileTall = pScheme ? atoi( pScheme->GetResourceString( "Dialog.TileHeight" ) ) : 0;
	if ( pFrame && nTileTall > 0 )
	{
		const int nBottom = nAdvancedY + pAdvanced->GetInt( "tall" );
		const int nTiles = ( nBottom + nTileTall - 1 ) / nTileTall;
		if ( nTiles > pFrame->GetInt( "tall" ) )
			pFrame->SetInt( "tall", nTiles );
	}
}

bool Video::CheckTemporalScale( int choice )
{
	if ( !m_drpTemporalScale )
		return false;
	if ( choice >= 0 )
	{
		OnCommand( CFmtStr( "%s%d", VIDEO_TEMPORALSCALE_COMMAND_PREFIX, choice ) );
		if ( !ApplyChanges() )
			return false;
	}
	Msg( "video temporal scale: enabled=%d, selected=%s, applied=%.8f\n",
	    int( m_drpTemporalScale->IsEnabled() ), m_drpTemporalScale->GetCurrentSelection(),
	    m_GraphicsSettings.Applied().temporalScale );
	return true;
}

CON_COMMAND_F( ui_show_video, "Open video settings; [0..4] selects and applies the FSR scale row",
    FCVAR_CHEAT )
{
	CBaseModPanel &panel = CBaseModPanel::GetSingleton();
	Video *dialog = static_cast<Video *>( panel.GetWindow( WT_VIDEO ) );
	if ( !dialog )
		dialog = static_cast<Video *>(
		    panel.OpenWindow( WT_VIDEO, panel.GetWindow( panel.GetActiveWindowType() ) ) );
	if ( !dialog || !dialog->CheckTemporalScale( args.ArgC() > 1 ? atoi( args[1] ) : -1 ) )
		Msg( "video temporal scale: not laid out or apply failed; retry after a wait\n" );
}

void Video::SetPowerSavingsState()
{
	if ( m_drpPowerSavingsMode )
	{
		m_drpPowerSavingsMode->SetCurrentSelection( m_nPowerSavingsMode != 0 ? "#L4D360UI_Enabled" : "#L4D360UI_Disabled" );	
	}
}

// The HDR page uses the shipped Video dialog's row styles and tile layout.
// Values stay local until Apply; Back discards them. The renderer owns negotiation.
HdrVideo::HdrVideo( Panel *parent, const char *panelName, Video &video )
    : BaseClass( parent, panelName ), m_Menu( video.GraphicsSession() )
{
	m_Video = &video;
	SetDeleteSelfOnClose( true );
	SetProportional( true );
	V_strncpy( m_ResourceName, "Resource/UI/BaseModUI/Video.res", sizeof( m_ResourceName ) );
	SetDialogTitle( "HDR" );
	SetFooterEnabled( true );
}

HdrVideo::~HdrVideo()
{
	if ( m_Video.Get() )
		m_Menu.Cancel();
}

void HdrVideo::PreApplyControlSettings( KeyValues *resource )
{
	if ( !resource )
		return;
	KeyValues *source = resource->FindKey( "DrpDisplayMode" );
	KeyValues *button = resource->FindKey( "BtnAdvanced" );
	if ( !source || !button )
		return;
	KeyValues *row = source->MakeCopy();
	KeyValues *apply = button->MakeCopy();
	// Video's conditional positions belong to its original rows. Applying
	// them to every clone would stack all three HDR rows at the same y.
	for ( KeyValues *settings : { row, apply } )
	{
		for ( KeyValues *key = settings->GetFirstTrueSubKey(); key; )
		{
			KeyValues *next = key->GetNextTrueSubKey();
			if ( key->GetName()[0] == '?' )
			{
				settings->RemoveSubKey( key );
				key->deleteThis();
			}
			key = next;
		}
	}
	for ( KeyValues *control = resource->GetFirstTrueSubKey(); control; )
	{
		KeyValues *next = control->GetNextTrueSubKey();
		if ( !V_stricmp( control->GetString( "ControlName" ), "Frame" ) )
		{
			control->SetName( GetName() );
			control->SetString( "fieldName", GetName() );
			control->SetInt( "tall", 6 );
		}
		else
		{
			resource->RemoveSubKey( control );
			control->deleteThis();
		}
		control = next;
	}
	const char *names[] = { "DrpHdrMode", "DrpHdrExposure", "DrpHdrPeak" };
	const char *labels[] = { "Output", "Exposure", "Peak brightness" };
	for ( int i = 0; i < 3; ++i )
	{
		KeyValues *control = row->MakeCopy();
		control->SetName( names[i] );
		control->SetString( "fieldName", names[i] );
		control->SetString( "labelText", labels[i] );
		control->SetInt( "ypos", i * 25 );
		control->SetString( "navUp", i == 0 ? "BtnHdrApply" : names[i - 1] );
		control->SetString( "navDown", i == 2 ? "BtnHdrApply" : names[i + 1] );
		KeyValues *list = control->FindKey( "list", true );
		list->Clear();
		if ( i == 0 )
		{
			list->SetString( "Automatic (HDR when available)", "HdrMode1" );
			list->SetString( "SDR", "HdrMode0" );
		}
		else if ( i == 1 )
		{
			for ( int percent : { 25, 50, 75, 100, 125, 150, 200, 400 } )
				list->SetString( CFmtStr( "%d%%", percent ), CFmtStr( "HdrExposure%d", percent ) );
		}
		else
		{
			for ( int nits : { 203, 400, 600, 1000, 1600, 2000, 4000 } )
				list->SetString( CFmtStr( "%d nits", nits ), CFmtStr( "HdrPeak%d", nits ) );
			// Keep the measured calibration selectable even between presets.
			const int calibrated = m_Menu.Draft().peakNits;
			list->SetString( CFmtStr( "%d nits", calibrated ), CFmtStr( "HdrPeak%d", calibrated ) );
		}
		resource->AddSubKey( control );
	}
	row->deleteThis();
	apply->SetName( "BtnHdrApply" );
	apply->SetString( "fieldName", "BtnHdrApply" );
	apply->SetString( "labelText", "#L4D360UI_Apply" );
	apply->SetString( "command", "ApplyHDR" );
	apply->SetString( "navUp", "DrpHdrPeak" );
	apply->SetString( "navDown", "DrpHdrMode" );
	apply->SetInt( "ypos", 75 );
	resource->AddSubKey( apply );
	KeyValues *status = new KeyValues( "LblHdrStatus" );
	status->SetString( "ControlName", "Label" );
	status->SetString( "fieldName", "LblHdrStatus" );
	status->SetInt( "xpos", 8 );
	status->SetInt( "ypos", 110 );
	status->SetInt( "wide", 430 );
	status->SetInt( "tall", 50 );
	status->SetString( "font", "Default" );
	status->SetInt( "wrap", 1 );
	status->SetString( "labelText", "" );
	resource->AddSubKey( status );
}

void HdrVideo::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	UpdateState();
}

void HdrVideo::UpdateState()
{
	if ( !m_Video.Get() )
		return;
	CGameUIConVarRef mode( "mat_hdr_output" );
	const bool available = mode.IsValid();
	const char *names[] = { "DrpHdrMode", "DrpHdrExposure", "DrpHdrPeak" };
	const CFmtStr exposureText( "%d%%", int( m_Menu.Draft().exposure * 100.0f + 0.5f ) );
	const CFmtStr peakText( "%d nits", m_Menu.Draft().peakNits );
	const char *values[] = { m_Menu.Draft().automatic ? "Automatic (HDR when available)" : "SDR",
	    exposureText, peakText };
	for ( int i = 0; i < 3; ++i )
	{
		if ( auto *row = dynamic_cast<BaseModHybridButton *>( FindChildByName( names[i] ) ) )
		{
			row->SetEnabled( available );
			row->SetCurrentSelection( values[i] );
		}
	}
	if ( auto *status = dynamic_cast<vgui::Label *>( FindChildByName( "LblHdrStatus" ) ) )
	{
		CGameUIConVarRef active( "mat_hdr_output_active" );
		const bool hdr = active.IsValid() && active.GetBool();
		status->SetText( !available ? "HDR rendering is unavailable in this session."
		                            : hdr ? "HDR output active. Set peak brightness to your display's rating."
		                                  : "SDR output active. HDR scene lighting is tone mapped to SDR." );
	}
	if ( auto *footer = CBaseModPanel::GetSingleton().GetFooterPanel() )
	{
		footer->SetButtons( FB_ABUTTON | FB_BBUTTON );
		footer->SetButtonText( FB_ABUTTON, "#L4D360UI_Apply" );
		footer->SetButtonText( FB_BBUTTON, "#L4D360UI_Cancel" );
	}
}

void HdrVideo::OnCommand( const char *command )
{
	if ( !m_Video.Get() )
		return;
	switch ( m_Menu.Command( command ) )
	{
	case gameui::HdrSettingsMenu::Action::Apply:
		ApplyChanges();
		break;
	case gameui::HdrSettingsMenu::Action::Cancel:
		NavigateBack();
		break;
	case gameui::HdrSettingsMenu::Action::Invalid:
		BaseClass::OnCommand( command );
		break;
	case gameui::HdrSettingsMenu::Action::Changed:
		break;
	}
	UpdateState();
}

void HdrVideo::OnKeyCodePressed( KeyCode code )
{
	const KeyCode key = GetBaseButtonCode( code );
	if ( key == KEY_XBUTTON_A || key == KEY_ENTER )
		ApplyChanges();
	else if ( key == KEY_XBUTTON_B || key == KEY_ESCAPE )
		OnCommand( "Cancel" );
	else
		BaseClass::OnKeyCodePressed( code );
}

void HdrVideo::ApplyChanges()
{
	if ( m_Video.Get() )
	{
		(void)m_Video->ApplyHdrChanges();
		m_Menu.Commit();
	}
}

void HdrVideo::OnThink()
{
	BaseClass::OnThink();
	UpdateState();
}

bool HdrVideo::CheckSettings( int mode )
{
	InvalidateLayout( true, true );
	for ( const char *name : { "DrpHdrMode", "DrpHdrExposure", "DrpHdrPeak", "BtnHdrApply", "LblHdrStatus" } )
	{
		auto *row = FindChildByName( name );
		if ( !row )
			return false;
		int x, y, width, height;
		row->GetBounds( x, y, width, height );
		Msg( "HDR row %s: %d,%d %dx%d visible=%d\n", name, x, y, width, height, row->IsVisible() );
	}
	if ( !FindChildByName( "DrpHdrMode" ) || !FindChildByName( "BtnHdrApply" ) )
		return false;
	if ( mode >= 0 )
	{
		OnCommand( mode ? "HdrMode1" : "HdrMode0" );
		OnCommand( "ApplyHDR" );
	}
	CGameUIConVarRef selected( "mat_hdr_output" );
	return selected.IsValid() && ( mode < 0 || selected.GetInt() == mode );
}

CON_COMMAND_F( ui_show_video_hdr, "Open Video > HDR; optional 0 SDR / 1 automatic selection",
    FCVAR_DONTRECORD )
{
	CBaseModPanel &panel = CBaseModPanel::GetSingleton();
	auto *video = static_cast<Video *>( panel.GetWindow( WT_VIDEO ) );
	if ( !video )
		video = static_cast<Video *>( panel.OpenWindow( WT_VIDEO, panel.GetWindow( panel.GetActiveWindowType() ) ) );
	auto *hdr = static_cast<HdrVideo *>( panel.OpenWindow( WT_HDRVIDEO, video, true ) );
	if ( hdr )
		Msg( "HDR menu: %s\n", hdr->CheckSettings( args.ArgC() > 1 ? atoi( args[1] ) : -1 ) ? "pass" : "fail" );
}
