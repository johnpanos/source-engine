//========= Copyright (c) 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=======================================================================================//

#include "vadvancedvideo.h"
#include "vfooterpanel.h"
#include "vdropdownmenu.h"
#include "vslidercontrol.h"
#include "vhybridbutton.h"
#include "engineinterface.h"
#include "IGameUIFuncs.h"
#include "gameui_util.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "modes.h"
#include "videocfg/videocfg.h"
#include "vgenericconfirmation.h"
#include "render_stage_marks.h"

#include "materialsystem/materialsystem_config.h"
#include "tier1/fmtstr.h"
#include "vgui/ILocalize.h"

#ifdef _X360
#include "xbox/xbox_launch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

#define VIDEO_ANTIALIAS_COMMAND_PREFIX "_antialias"
#define VIDEO_CORE_AO_COMMAND_PREFIX "_coreao"
#define VIDEO_CORE_SHADOWS_COMMAND_PREFIX "_coreshadows"
#define VIDEO_CORE_DEPTH_COMMAND_PREFIX "_coredepth"
#define VIDEO_CORE_MOVERS_COMMAND_PREFIX "_coremovers"
#define VIDEO_CORE_DIRECT_COMMAND_PREFIX "_coredirect"

// The five RenderCoreWorldQuality settings (RFC 0016 K12). Each row's
// choice index is the ConVar value. Runtime direct light applies at map load.
static const char *const s_RenderCoreQualityNames[] = {
    "#GameUI_QualityOff",
    "#GameUI_Low",
    "#GameUI_Medium",
    "#GameUI_High",
    "#GameUI_QualityUltra",
};
static const int kRenderCoreAOChoices = 5;
static const int kRenderCoreShadowChoices = 4;
static const int kRenderCoreToggleChoices = 2;
static const char *const s_RenderCoreToggleNames[] = {
    "#GameUI_QualityOff",
    "#GameUI_QualityOn",
};

// The shipped localization has none of these tokens (and its GameUI_Ultra
// reads "Very High"), so the English text, in this dialog's title case, is
// added unless a loaded localization file already defines the token.
static void AddRenderCoreQualityStrings()
{
	static const struct
	{
		const char *token;
		const wchar_t *text;
	} kStrings[] = {
	    { "GameUI_AmbientOcclusion", L"Ambient Occlusion" },
	    { "GameUI_DynamicShadows", L"Dynamic Shadows" },
	    { "GameUI_CoreDepthPrepass", L"Depth Prepass" },
	    { "GameUI_CoreShadowMovers", L"Moving Object Shadows" },
	    { "GameUI_CoreRuntimeDirect", L"Runtime Direct Light (Next Map)" },
	    { "GameUI_QualityOn", L"On" },
	    { "GameUI_QualityOff", L"Off" },
	    { "GameUI_QualityUltra", L"Ultra" },
	    { "GameUI_AmbientOcclusion_Info",
	        L"Ambient Occlusion calculates contact shadows in corners and crevices, adding depth "
	        L"to surfaces. Higher settings improve shadow fidelity at the cost of GPU "
	        L"performance." },
	    { "GameUI_DynamicShadows_Info",
	        L"Dynamic Shadows controls the resolution and quality of real-time shadows cast by "
	        L"lights and objects. Higher settings produce sharper, more detailed shadows." },
	    { "GameUI_CoreDepthPrepass_Info",
	        L"Depth Prepass renders scene geometry into an early depth buffer to eliminate "
	        L"redundant pixel shading, improving rendering efficiency in complex scenes." },
	    { "GameUI_CoreShadowMovers_Info",
	        L"Moving Object Shadows enables real-time dynamic shadows cast by physics objects, "
	        L"moving panels, and characters." },
	    { "GameUI_CoreRuntimeDirect_Info",
	        L"Runtime Direct Light enables dynamic evaluation of direct light sources in real "
	        L"time. Changes take effect on the next map load." },
	};
	for ( const auto &entry : kStrings )
	{
		if ( !g_pVGuiLocalize->Find( entry.token ) )
			g_pVGuiLocalize->AddString( entry.token, const_cast<wchar_t *>( entry.text ), NULL );
	}
}

static bool HasRenderCoreQuality()
{
	return CGameUIConVarRef( "r_core_ao_quality" ).IsValid() &&
	       CGameUIConVarRef( "r_core_shadow_quality" ).IsValid() &&
	       CGameUIConVarRef( "r_core_depth_prepass" ).IsValid() &&
	       CGameUIConVarRef( "r_core_shadow_movers" ).IsValid() &&
	       CGameUIConVarRef( "r_core_runtime_direct" ).IsValid();
}

CAdvancedVideo::CAdvancedVideo(Panel *parent, const char *panelName):
BaseClass(parent, panelName)
{
	SetDeleteSelfOnClose( true );
	SetProportional( true );

	SetDialogTitle( "#GameUI_VideoAdvanced_Title" );

	m_drpModelDetail = NULL;
#ifndef POSIX	
	m_drpPagedPoolMem = NULL;
#endif
	m_drpQueuedMode = NULL;
	m_drpAntialias = NULL;
	m_drpFiltering = NULL;
	m_drpVSync = NULL;
	m_drpShaderDetail = NULL;
	m_drpCPUDetail = NULL;
	m_drpCoreAO = NULL;
	m_drpCoreShadows = NULL;
	m_drpCoreDepth = NULL;
	m_drpCoreMovers = NULL;
	m_drpCoreDirect = NULL;

	m_bDirtyValues = false;
	m_bEnableApply = false;

	// reset all warnings
	m_VideoWarning = VW_NONE;
	for ( int i = 0; i < VW_MAXWARNINGS; i++ )
	{
		m_bAcceptWarning[i] = false;
	}

	m_nNumAAModes = 0;

	m_iModelTextureDetail = 0;
	m_iPagedPoolMem = 0;
	m_nAASamples = 0;
	m_nAAQuality = 0;
	m_iFiltering = 1;
	m_bVSync = false;
	m_bTripleBuffered = false;
	m_iGPUDetail = 0;
	m_iCPUDetail = 0;
	m_iCoreAO = 0;
	m_iCoreShadows = 0;
	m_iCoreDepth = 0;
	m_iCoreMovers = 0;
	m_iCoreDirect = 0;
	m_iQueuedMode = -1;

	m_lblDescriptionTitle = NULL;
	m_lblDescription = NULL;
	m_pLastDescriptionControl = NULL;

	SetFooterEnabled( true );
	UpdateFooter();
}

CAdvancedVideo::~CAdvancedVideo()
{
}

void CAdvancedVideo::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_drpModelDetail = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpModelDetail" ) );
#ifndef POSIX
	m_drpPagedPoolMem = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpPagedPoolMem" ) );
#endif
	m_drpAntialias = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpAntialias" ) );
	m_drpFiltering = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpFiltering" ) );
	m_drpVSync = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpVSync" ) );
	m_drpQueuedMode = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpQueuedMode" ) );
	m_drpShaderDetail = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpShaderDetail" ) );
	m_drpCPUDetail = dynamic_cast< BaseModHybridButton* >( FindChildByName( "DrpCPUDetail" ) );
	m_drpCoreAO = dynamic_cast<BaseModHybridButton *>( FindChildByName( "DrpCoreAO" ) );
	m_drpCoreShadows = dynamic_cast<BaseModHybridButton *>( FindChildByName( "DrpCoreShadows" ) );
	m_drpCoreDepth = dynamic_cast<BaseModHybridButton *>( FindChildByName( "DrpCoreDepth" ) );
	m_drpCoreMovers = dynamic_cast<BaseModHybridButton *>( FindChildByName( "DrpCoreMovers" ) );
	m_drpCoreDirect = dynamic_cast<BaseModHybridButton *>( FindChildByName( "DrpCoreDirect" ) );
	m_lblDescriptionTitle = dynamic_cast<vgui::Label *>( FindChildByName( "LblDescriptionTitle" ) );
	m_lblDescription = dynamic_cast<vgui::Label *>( FindChildByName( "LblDescription" ) );

	SetupState( false );

	if ( m_drpModelDetail )
	{
		if ( m_ActiveControl )
			m_ActiveControl->NavigateFrom();
		m_drpModelDetail->NavigateTo();
		m_ActiveControl = m_drpModelDetail;
	}

	UpdateDescription( m_ActiveControl );

	UpdateFooter();
}

void AcceptWarningCallback()
{
	CAdvancedVideo *pSelf = static_cast< CAdvancedVideo* >( CBaseModPanel::GetSingleton().GetWindow( WT_ADVANCEDVIDEO ) );
	if ( pSelf )
	{
		pSelf->AcceptWarningCallback();
	}
}

void CAdvancedVideo::ShowWarning( VideoWarning_e videoWarning )
{
	GenericConfirmation* pConfirmation = 
		static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this ) );

	GenericConfirmation::Data_t data;

	switch ( videoWarning )
	{
	case VW_ANTIALIASING:
		data.pWindowTitle = "#L4D360UI_VideoOptions_Antialiasing";
		data.pMessageText = "#PORTAL2_VideoOptions_Antialiasing_Info";
		break;

	case VW_FILTERING:
		data.pWindowTitle = "#GameUI_Filtering_Mode";
		data.pMessageText = "#PORTAL2_VideoOptions_Filtering_Info";
		break;

	case VW_VSYNC:
		data.pWindowTitle = "#GameUI_Wait_For_VSync";
		data.pMessageText = "#PORTAL2_VideoOptions_WaitForVSync_Info";
		break;

	case VW_MULTICORE:
		data.pWindowTitle = "#L4D360UI_VideoOptions_Queued_Mode";
		data.pMessageText = "#PORTAL2_VideoOptions_QueuedMode_Info";
		break;

	case VW_SHADERDETAIL:
		data.pWindowTitle = "#GameUI_Shader_Detail";
		data.pMessageText = "#PORTAL2_VideoOptions_ShaderDetail_Info";
		break;

	case VW_CPUDETAIL:
		data.pWindowTitle = "#L4D360UI_VideoOptions_CPU_Detail";
		data.pMessageText = "#PORTAL2_VideoOptions_CPUDetail_Info";
		break;

	case VW_MODELDETAIL:
		data.pWindowTitle = "#L4D360UI_VideoOptions_Model_Texture_Detail";
		data.pMessageText = "#PORTAL2_VideoOptions_ModelDetail_Info";
		break;

	case VW_PAGEDPOOL:
		data.pWindowTitle = "#L4D360UI_VideoOptions_Paged_Pool_Mem";
		data.pMessageText = "#L4D360UI_VideoOptions_Paged_Pool_Mem_Info";
		break;

	default:
		return;
	}

	data.bOkButtonEnabled = true;
	data.pfnOkCallback = ::AcceptWarningCallback;

	m_VideoWarning = videoWarning;
	pConfirmation->SetUsageData( data );
}

void CAdvancedVideo::AcceptWarningCallback()
{
	m_bAcceptWarning[m_VideoWarning] = true;
	m_VideoWarning = VW_NONE;
}

void CAdvancedVideo::GetCurrentSettings( void )
{
	CGameUIConVarRef gpu_mem_level( "gpu_mem_level" );
	m_iModelTextureDetail = clamp( gpu_mem_level.GetInt(), 0, 2);

	CGameUIConVarRef mem_level( "mem_level" );
	m_iPagedPoolMem = clamp( mem_level.GetInt(), 0, 2);

	CGameUIConVarRef mat_antialias( "mat_antialias" );
	CGameUIConVarRef mat_aaquality( "mat_aaquality" );
	m_nAASamples = mat_antialias.GetInt();
	m_nAAQuality = mat_aaquality.GetInt();

	CGameUIConVarRef mat_forceaniso( "mat_forceaniso" );
	m_iFiltering = mat_forceaniso.GetInt();

	CGameUIConVarRef mat_vsync( "mat_vsync" );
	m_bVSync = mat_vsync.GetBool();

	CGameUIConVarRef mat_triplebuffered( "mat_triplebuffered" );
	m_bTripleBuffered = mat_triplebuffered.GetBool();

	CGameUIConVarRef mat_queue_mode( "mat_queue_mode" );
	m_iQueuedMode = mat_queue_mode.GetInt();

	CGameUIConVarRef gpu_level( "gpu_level" );
	m_iGPUDetail = clamp( gpu_level.GetInt(), 0, 3 );

	CGameUIConVarRef cpu_level( "cpu_level" );
	m_iCPUDetail = clamp( cpu_level.GetInt(), 0, 2 );

	if ( HasRenderCoreQuality() )
	{
		CGameUIConVarRef r_core_ao_quality( "r_core_ao_quality" );
		CGameUIConVarRef r_core_shadow_quality( "r_core_shadow_quality" );
		m_iCoreAO = clamp( r_core_ao_quality.GetInt(), 0, kRenderCoreAOChoices - 1 );
		m_iCoreShadows = clamp( r_core_shadow_quality.GetInt(), 0, kRenderCoreShadowChoices - 1 );
		m_iCoreDepth = clamp( CGameUIConVarRef( "r_core_depth_prepass" ).GetInt(), 0, 1 );
		m_iCoreMovers = clamp( CGameUIConVarRef( "r_core_shadow_movers" ).GetInt(), 0, 1 );
		m_iCoreDirect = clamp( CGameUIConVarRef( "r_core_runtime_direct" ).GetInt(), 0, 1 );
	}
}

bool CAdvancedVideo::GetRecommendedSettings( void )
{
#if !defined( _GAMECONSOLE )
	KeyValues *pConfigKeys = new KeyValues( "VideoConfig" );
	if ( !pConfigKeys )
		return false;

	if ( !ReadCurrentVideoConfig( pConfigKeys, true ) )
	{
		pConfigKeys->deleteThis();
		return false;
	}

	m_iModelTextureDetail = clamp( pConfigKeys->GetInt( "setting.gpu_mem_level", 0 ), 0, 2 );
	m_iPagedPoolMem = clamp( pConfigKeys->GetInt( "setting.mem_level", 0 ), 0, 2 );
	m_nAASamples = pConfigKeys->GetInt( "setting.mat_antialias", 0 );
	m_nAAQuality = pConfigKeys->GetInt( "setting.mat_aaquality", 0 );
	m_iFiltering = pConfigKeys->GetInt( "setting.mat_forceaniso", 1 );
	m_bVSync = pConfigKeys->GetBool( "setting.mat_vsync", true );
	m_bTripleBuffered = pConfigKeys->GetBool( "setting.mat_triplebuffered", false );
	m_iGPUDetail = pConfigKeys->GetInt( "setting.gpu_level", 0 );
	m_iCPUDetail = pConfigKeys->GetInt( "setting.cpu_level", 0 );
	m_iQueuedMode = pConfigKeys->GetInt( "setting.mat_queue_mode", -1 );

	pConfigKeys->deleteThis();

	// The render core's settings are not in the video config; their
	// defaults are the ConVars' defaults.
	if ( HasRenderCoreQuality() )
	{
		CGameUIConVarRef r_core_ao_quality( "r_core_ao_quality" );
		CGameUIConVarRef r_core_shadow_quality( "r_core_shadow_quality" );
		m_iCoreAO = clamp( atoi( r_core_ao_quality.GetDefault() ), 0, kRenderCoreAOChoices - 1 );
		m_iCoreShadows =
		    clamp( atoi( r_core_shadow_quality.GetDefault() ), 0, kRenderCoreShadowChoices - 1 );
		m_iCoreDepth =
		    clamp( atoi( CGameUIConVarRef( "r_core_depth_prepass" ).GetDefault() ), 0, 1 );
		m_iCoreMovers =
		    clamp( atoi( CGameUIConVarRef( "r_core_shadow_movers" ).GetDefault() ), 0, 1 );
		m_iCoreDirect =
		    clamp( atoi( CGameUIConVarRef( "r_core_runtime_direct" ).GetDefault() ), 0, 1 );
	}

	m_bDirtyValues = true;
#endif

	return true;
}

void CAdvancedVideo::ProcessAAList()
{
	if ( !m_drpAntialias )
		return;

	// We start with no entries
	m_nNumAAModes = 0;

	char szCurrentButton[ 32 ];
	V_strncpy( szCurrentButton, VIDEO_ANTIALIAS_COMMAND_PREFIX, sizeof( szCurrentButton ) );

	int iCommandNumberPosition = Q_strlen( szCurrentButton );
	szCurrentButton[ iCommandNumberPosition + 1 ] = '\0';

	// Always have the possibility of no AA
	Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
	szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
	m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_None" );

	m_nAAModes[m_nNumAAModes].m_nNumSamples = 1;
	m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
	m_nNumAAModes++;

	// Add other supported AA settings
	if ( materials->SupportsMSAAMode( 2 ) )
	{
		Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
		szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
		m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_2X" );

		m_nAAModes[m_nNumAAModes].m_nNumSamples = 2;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	if ( materials->SupportsMSAAMode( 4 ) )
	{
		Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
		szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
		m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_4X" );

		m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	if ( materials->SupportsMSAAMode( 6 ) )
	{
		Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
		szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
		m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_6X" );

		m_nAAModes[m_nNumAAModes].m_nNumSamples = 6;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	// nVidia CSAA "8x"
	if ( materials->SupportsCSAAMode( 4, 2 ) )							
	{
		Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
		szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
		m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_8X_CSAA" );

		m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
		m_nNumAAModes++;
	}

	// nVidia CSAA "16x"
	if ( materials->SupportsCSAAMode( 4, 4 ) )							
	{
		Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
		szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
		m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_16X_CSAA" );

		m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 4;
		m_nNumAAModes++;
	}

	if ( materials->SupportsMSAAMode( 8 ) )
	{
		Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
		szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
		m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_8X" );

		m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
		m_nNumAAModes++;
	}

	// nVidia CSAA "16xQ"
	if ( materials->SupportsCSAAMode( 8, 2 ) )							
	{
		Assert( m_nNumAAModes < MAX_DYNAMIC_AA_MODES );
		szCurrentButton[ iCommandNumberPosition ] = m_nNumAAModes + '0';
		m_drpAntialias->ModifySelectionString( szCurrentButton, "#GameUI_16XQ_CSAA" );

		m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
		m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
		m_nNumAAModes++;
	}

	// Enable the valid possible choices
	for ( int i = 0; i < m_nNumAAModes; ++i )
	{
		szCurrentButton[ iCommandNumberPosition ] = i + '0';
		char szString[256];
		if ( m_drpAntialias->GetListSelectionString( szCurrentButton, szString, sizeof( szString ) ) )
		{
			m_drpAntialias->EnableListItem( szString, true );
		}
	}
	
	// Disable the remaining possible choices
	for ( int i = m_nNumAAModes; i < MAX_DYNAMIC_AA_MODES; ++i )
	{
		szCurrentButton[ iCommandNumberPosition ] = i + '0';
		char szString[256];
		if ( m_drpAntialias->GetListSelectionString( szCurrentButton, szString, sizeof( szString ) ) )
		{
			m_drpAntialias->EnableListItem( szString, false );
		}
	}

	// Select the currently set type
	m_iAntiAlias = FindMSAAMode( m_nAASamples, m_nAAQuality );
}

void CAdvancedVideo::SetAntiAliasingState()
{
	char szCurrentButton[ 32 ];
	V_strncpy( szCurrentButton, VIDEO_ANTIALIAS_COMMAND_PREFIX, sizeof( szCurrentButton ) );

	int iCommandNumberPosition = Q_strlen( szCurrentButton );
	szCurrentButton[ iCommandNumberPosition + 1 ] = '\0';

	szCurrentButton[ iCommandNumberPosition ] = m_iAntiAlias + '0';
	char szString[256];
	if ( m_drpAntialias->GetListSelectionString( szCurrentButton, szString, sizeof( szString ) ) )
	{
		m_drpAntialias->SetCurrentSelection( szString );
	}
}

void CAdvancedVideo::SetFilteringState()
{
	if ( m_drpFiltering )
	{
		switch ( m_iFiltering )
		{
		case 0:
			m_drpFiltering->SetCurrentSelection( "#GameUI_Bilinear" );
			break;
		case 1:
			m_drpFiltering->SetCurrentSelection( "#GameUI_Trilinear" );
			break;
		case 2:
			m_drpFiltering->SetCurrentSelection( "#GameUI_Anisotropic2X" );
			break;
		case 4:
			m_drpFiltering->SetCurrentSelection( "#GameUI_Anisotropic4X" );
			break;
		case 8:
			m_drpFiltering->SetCurrentSelection( "#GameUI_Anisotropic8X" );
			break;
		case 16:
			m_drpFiltering->SetCurrentSelection( "#GameUI_Anisotropic16X" );
			break;
		default:
			m_drpFiltering->SetCurrentSelection( "#GameUI_Trilinear" );
			m_iFiltering = 1;
			break;
		}
	}
}

void CAdvancedVideo::SetVSyncState()
{
	if ( m_drpVSync )
	{
		if ( m_bVSync )
		{
			if ( m_bTripleBuffered )
			{
				m_drpVSync->SetCurrentSelection( "#L4D360UI_VideoOptions_VSync_TripleBuffered" );
			}
			else
			{
				m_drpVSync->SetCurrentSelection( "#L4D360UI_VideoOptions_VSync_DoubleBuffered" );
			}
		}
		else
		{
			m_drpVSync->SetCurrentSelection( "#L4D360UI_Disabled" );
		}
	}
}

void CAdvancedVideo::SetQueuedModeState()
{
	if ( m_drpQueuedMode )
	{
		// Only allow the options on multi-processor machines.
		if ( GetCPUInformation()->m_nPhysicalProcessors >= 2 )
		{
			if ( m_iQueuedMode != 0 )
			{
				m_drpQueuedMode->SetCurrentSelection( "#L4D360UI_Enabled" );
			}
			else
			{
				m_drpQueuedMode->SetCurrentSelection( "#L4D360UI_Disabled" );
			}
		}
		else
		{
			m_drpQueuedMode->SetEnabled( false );
		}
	}
}

void CAdvancedVideo::SetShaderDetailState()
{
	if ( m_drpShaderDetail )
	{
		switch ( m_iGPUDetail )
		{
		case 0:
			m_drpShaderDetail->SetCurrentSelection( "#GameUI_Low" );
			break;
		case 1:
			m_drpShaderDetail->SetCurrentSelection( "#GameUI_Medium" );
			break;
		case 2:
			m_drpShaderDetail->SetCurrentSelection( "#GameUI_High" );
			break;
		case 3:
			m_drpShaderDetail->SetCurrentSelection( "#GameUI_Ultra" );
			break;
		}
	}
}

void CAdvancedVideo::SetCPUDetailState()
{
	if ( m_drpCPUDetail )
	{
		switch ( m_iCPUDetail )
		{
		case 0:
			m_drpCPUDetail->SetCurrentSelection( "#GameUI_Low" );
			break;
		case 1:
			m_drpCPUDetail->SetCurrentSelection( "#GameUI_Medium" );
			break;
		case 2:
			m_drpCPUDetail->SetCurrentSelection( "#GameUI_High" );
			break;
		}
	}
}

void CAdvancedVideo::SetModelDetailState()
{
	if ( m_drpModelDetail )
	{
		switch ( m_iModelTextureDetail )
		{
		case 0:
			m_drpModelDetail->SetCurrentSelection( "#GameUI_Low" );
			break;
		case 1:
			m_drpModelDetail->SetCurrentSelection( "#GameUI_Medium" );
			break;
		case 2:
			m_drpModelDetail->SetCurrentSelection( "#GameUI_High" );
			break;
		}
	}
}

void CAdvancedVideo::SetRenderCoreQualityState()
{
	if ( m_drpCoreAO )
		m_drpCoreAO->SetCurrentSelection( s_RenderCoreQualityNames[m_iCoreAO] );
	if ( m_drpCoreShadows )
		m_drpCoreShadows->SetCurrentSelection( s_RenderCoreQualityNames[m_iCoreShadows] );
	if ( m_drpCoreDepth )
		m_drpCoreDepth->SetCurrentSelection( s_RenderCoreToggleNames[m_iCoreDepth] );
	if ( m_drpCoreMovers )
		m_drpCoreMovers->SetCurrentSelection( s_RenderCoreToggleNames[m_iCoreMovers] );
	if ( m_drpCoreDirect )
		m_drpCoreDirect->SetCurrentSelection( s_RenderCoreToggleNames[m_iCoreDirect] );
}

void CAdvancedVideo::SetPagedPoolState()
{
#ifndef POSIX
	if ( m_drpPagedPoolMem )
	{
		switch ( m_iPagedPoolMem )
		{
		case 0:
			m_drpPagedPoolMem->SetCurrentSelection( "#GameUI_Low" );
			break;
		case 1:
			m_drpPagedPoolMem->SetCurrentSelection( "#GameUI_Medium" );
			break;
		case 2:
			m_drpPagedPoolMem->SetCurrentSelection( "#GameUI_High" );
			break;
		}
	}
#endif
}

static void AcceptDefaultsOkCallback()
{
	CAdvancedVideo *pSelf = 
		static_cast< CAdvancedVideo* >( CBaseModPanel::GetSingleton().GetWindow( WT_ADVANCEDVIDEO ) );
	if ( pSelf )
	{
		pSelf->SetDefaults();
	}
}

static void DiscardChangesOkCallback()
{
	CAdvancedVideo *pSelf = 
		static_cast< CAdvancedVideo* >( CBaseModPanel::GetSingleton().GetWindow( WT_ADVANCEDVIDEO ) );
	if ( pSelf )
	{
		pSelf->DiscardChangesAndClose();
	}
}

void CAdvancedVideo::DiscardChangesAndClose()
{
	BaseClass::OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
}

void CAdvancedVideo::SetDefaults()
{
	SetupState( true );
}

void CAdvancedVideo::SetupState( bool bRecommendedSettings )
{
	if ( !bRecommendedSettings )
	{
		GetCurrentSettings();
	}
	else if ( !GetRecommendedSettings() )
	{
		return;
	}

	ProcessAAList();
	if ( m_drpAntialias )
		m_drpAntialias->SetEnabled(
		    !( g_pRenderTemporalViews && g_pRenderTemporalViews->Enabled() ) );

	SetAntiAliasingState();
	SetFilteringState();
	SetVSyncState();
	SetQueuedModeState();
	SetShaderDetailState();
	SetCPUDetailState();
	SetModelDetailState();
	SetPagedPoolState();
	SetRenderCoreQualityState();

	UpdateFooter();
}

void CAdvancedVideo::Activate()
{
	BaseClass::Activate();

	UpdateFooter();
}

void CAdvancedVideo::OnKeyCodePressed(KeyCode code)
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
		ApplyChanges();
		BaseClass::OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
		break;

	case KEY_XBUTTON_B:
		if ( m_bDirtyValues )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

			GenericConfirmation *pConfirmation = 
				static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, true ) );

			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#PORTAL2_AdvancedVideoConf";
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
			data.pWindowTitle = "#PORTAL2_AdvancedVideoConf";
			data.pMessageText = "#PORTAL2_VideoSettingsUseDefaultsQ";
			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptDefaultsOkCallback;
			data.pOkButtonText = "#PORTAL2_ButtonAction_Reset";
			data.bCancelButtonEnabled = true;
			pConfirmation->SetUsageData( data );
		}
		break;

	default:
		BaseClass::OnKeyCodePressed(code);
		break;
	}
}

void CAdvancedVideo::OnCommand(const char *command)
{
	Msg( "VIDEO COMMAND %s\n", command );
	if ( !V_stricmp( command, "ModelDetailHigh" ) )
	{
		m_iModelTextureDetail = 2;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "ModelDetailMedium" ) )
	{
		m_iModelTextureDetail = 1;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "ModelDetailLow" ) )
	{
		m_iModelTextureDetail = 0;
		m_bDirtyValues = true;
	}
#ifndef POSIX
	else if ( !V_stricmp( command, "PagedPoolMemHigh" ) )
	{
		m_iPagedPoolMem = 2;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "PagedPoolMemMedium" ) )
	{
		m_iPagedPoolMem = 1;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "PagedPoolMemLow" ) )
	{
		m_iPagedPoolMem = 0;
		m_bDirtyValues = true;
	}
#endif
	else if ( StringHasPrefix( command, VIDEO_ANTIALIAS_COMMAND_PREFIX ) )
	{
		if ( g_pRenderTemporalViews && g_pRenderTemporalViews->Enabled() )
			SetAntiAliasingState();
		else
		{
			int iCommandNumberPosition = Q_strlen( VIDEO_ANTIALIAS_COMMAND_PREFIX );
			m_iAntiAlias = clamp( command[ iCommandNumberPosition ] - '0', 0, m_nNumAAModes - 1 );
			m_bDirtyValues = true;
		}
	}
	else if ( !V_stricmp( command, "#GameUI_Bilinear" ) )
	{
		m_iFiltering = 0;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "#GameUI_Trilinear" ) )
	{
		m_iFiltering = 1;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "#GameUI_Anisotropic2X" ) )
	{
		m_iFiltering = 2;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "#GameUI_Anisotropic4X" ) )
	{
		m_iFiltering = 4;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "#GameUI_Anisotropic8X" ) )
	{
		m_iFiltering = 8;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "#GameUI_Anisotropic16X" ) )
	{
		m_iFiltering = 16;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "VSyncTripleBuffered" ) )
	{
		m_bVSync = true;
		m_bTripleBuffered = true;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "VSyncEnabled" ) )
	{
		m_bVSync = true;
		m_bTripleBuffered = false;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "VSyncDisabled" ) )
	{
		m_bVSync = false;
		m_bTripleBuffered = false;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "QueuedModeEnabled" ) )
	{
		m_iQueuedMode = -1;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "QueuedModeDisabled" ) )
	{
		m_iQueuedMode = 0;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "ShaderDetailVeryHigh" ) )
	{
		m_iGPUDetail = 3;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "ShaderDetailHigh" ) )
	{
		m_iGPUDetail = 2;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "ShaderDetailMedium" ) )
	{
		m_iGPUDetail = 1;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "ShaderDetailLow" ) )
	{
		m_iGPUDetail = 0;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "CPUDetailHigh" ) )
	{
		m_iCPUDetail = 2;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "CPUDetailMedium" ) )
	{
		m_iCPUDetail = 1;
		m_bDirtyValues = true;
	}
	else if ( !V_stricmp( command, "CPUDetailLow" ) )
	{
		m_iCPUDetail = 0;
		m_bDirtyValues = true;
	}
	else if ( StringHasPrefix( command, VIDEO_CORE_AO_COMMAND_PREFIX ) )
	{
		m_iCoreAO = clamp( atoi( command + V_strlen( VIDEO_CORE_AO_COMMAND_PREFIX ) ), 0,
		    kRenderCoreAOChoices - 1 );
		m_bDirtyValues = true;
	}
	else if ( StringHasPrefix( command, VIDEO_CORE_SHADOWS_COMMAND_PREFIX ) )
	{
		m_iCoreShadows = clamp( atoi( command + V_strlen( VIDEO_CORE_SHADOWS_COMMAND_PREFIX ) ), 0,
		    kRenderCoreShadowChoices - 1 );
		m_bDirtyValues = true;
	}
	else if ( StringHasPrefix( command, VIDEO_CORE_DEPTH_COMMAND_PREFIX ) )
	{
		m_iCoreDepth = clamp( atoi( command + V_strlen( VIDEO_CORE_DEPTH_COMMAND_PREFIX ) ), 0, 1 );
		m_bDirtyValues = true;
	}
	else if ( StringHasPrefix( command, VIDEO_CORE_MOVERS_COMMAND_PREFIX ) )
	{
		m_iCoreMovers =
		    clamp( atoi( command + V_strlen( VIDEO_CORE_MOVERS_COMMAND_PREFIX ) ), 0, 1 );
		m_bDirtyValues = true;
	}
	else if ( StringHasPrefix( command, VIDEO_CORE_DIRECT_COMMAND_PREFIX ) )
	{
		m_iCoreDirect =
		    clamp( atoi( command + V_strlen( VIDEO_CORE_DIRECT_COMMAND_PREFIX ) ), 0, 1 );
		m_bDirtyValues = true;
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

int CAdvancedVideo::FindMSAAMode( int nAASamples, int nAAQuality )
{
	// Run through the AA Modes supported by the device
	for ( int nAAMode = 0; nAAMode < m_nNumAAModes; nAAMode++ )
	{
		// If we found the mode that matches what we're looking for, return the index
		if ( ( m_nAAModes[nAAMode].m_nNumSamples == nAASamples) && ( m_nAAModes[nAAMode].m_nQualityLevel == nAAQuality) )
		{
			return nAAMode;
		}
	}

	return 0;	// Didn't find what we're looking for, so no AA
}

void CAdvancedVideo::ApplyChanges()
{
	if ( !m_bDirtyValues )
	{
		// No need to apply settings
		return;
	}

	CGameUIConVarRef gpu_mem_level( "gpu_mem_level" );
	gpu_mem_level.SetValue( m_iModelTextureDetail );

	CGameUIConVarRef mem_level( "mem_level" );
	mem_level.SetValue( m_iPagedPoolMem );

	CGameUIConVarRef mat_antialias( "mat_antialias" );
	CGameUIConVarRef mat_aaquality( "mat_aaquality" );
	const bool temporal = g_pRenderTemporalViews && g_pRenderTemporalViews->Enabled();
	mat_antialias.SetValue( temporal ? 0 : m_nAAModes[m_iAntiAlias].m_nNumSamples );
	mat_aaquality.SetValue( temporal ? 0 : m_nAAModes[m_iAntiAlias].m_nQualityLevel );

	CGameUIConVarRef mat_forceaniso( "mat_forceaniso" );
	mat_forceaniso.SetValue( m_iFiltering );

	CGameUIConVarRef mat_vsync( "mat_vsync" );
	mat_vsync.SetValue( m_bVSync );

	CGameUIConVarRef mat_triplebuffered( "mat_triplebuffered" );
	mat_triplebuffered.SetValue( m_bTripleBuffered );

	CGameUIConVarRef mat_queue_mode( "mat_queue_mode" );
	mat_queue_mode.SetValue( m_iQueuedMode );

	CGameUIConVarRef cpu_level( "cpu_level" );
	cpu_level.SetValue( m_iCPUDetail );

	CGameUIConVarRef gpu_level( "gpu_level" );
	gpu_level.SetValue( m_iGPUDetail );

	if ( HasRenderCoreQuality() )
	{
		CGameUIConVarRef r_core_ao_quality( "r_core_ao_quality" );
		CGameUIConVarRef r_core_shadow_quality( "r_core_shadow_quality" );
		r_core_ao_quality.SetValue( m_iCoreAO );
		r_core_shadow_quality.SetValue( m_iCoreShadows );
		CGameUIConVarRef( "r_core_depth_prepass" ).SetValue( m_iCoreDepth );
		CGameUIConVarRef( "r_core_shadow_movers" ).SetValue( m_iCoreMovers );
		CGameUIConVarRef( "r_core_runtime_direct" ).SetValue( m_iCoreDirect );
	}

	// apply changes
	engine->ClientCmd_Unrestricted( "mat_savechanges\n" );
	engine->ClientCmd_Unrestricted( VarArgs( "host_writeconfig_ss %d", XBX_GetPrimaryUserId() ) );

	m_bDirtyValues = false;
}

void CAdvancedVideo::OnThink()
{
	BaseClass::OnThink();

	if ( m_bEnableApply != m_bDirtyValues )
	{
		// enable the apply button
		m_bEnableApply = m_bDirtyValues;
		UpdateFooter();
	}

	Panel *pFocus = ipanel()->GetPanel( vgui::input()->GetFocus(), GetModuleName() );
	if ( pFocus && pFocus != m_pLastDescriptionControl && pFocus->GetParent() == this )
	{
		UpdateDescription( pFocus );
	}
}

void CAdvancedVideo::OnHybridButtonNavigatedTo( VPANEL button )
{
	Panel *panel = ipanel()->GetPanel( button, GetModuleName() );
	if ( panel )
	{
		m_ActiveControl = panel;
		UpdateDescription( panel );
	}
}

void CAdvancedVideo::NavigateToChild( Panel *pNavigateTo )
{
	BaseClass::NavigateToChild( pNavigateTo );
	m_ActiveControl = pNavigateTo;
	UpdateDescription( pNavigateTo );
}

void CAdvancedVideo::UpdateDescription( Panel *pControl )
{
	if ( !pControl )
		return;

	if ( !m_lblDescriptionTitle )
		m_lblDescriptionTitle =
		    dynamic_cast<vgui::Label *>( FindChildByName( "LblDescriptionTitle" ) );
	if ( !m_lblDescription )
		m_lblDescription = dynamic_cast<vgui::Label *>( FindChildByName( "LblDescription" ) );

	if ( !m_lblDescriptionTitle || !m_lblDescription )
		return;

	m_pLastDescriptionControl = pControl;

	const char *pName = pControl->GetName();
	const char *pTitle = NULL;
	const char *pDesc = NULL;

	if ( !V_stricmp( pName, "DrpAntialias" ) )
	{
		pTitle = "#L4D360UI_VideoOptions_Antialiasing";
		pDesc = "#PORTAL2_VideoOptions_Antialiasing_Info";
	}
	else if ( !V_stricmp( pName, "DrpFiltering" ) )
	{
		pTitle = "#GameUI_Filtering_Mode";
		pDesc = "#PORTAL2_VideoOptions_Filtering_Info";
	}
	else if ( !V_stricmp( pName, "DrpVSync" ) )
	{
		pTitle = "#GameUI_Wait_For_VSync";
		pDesc = "#PORTAL2_VideoOptions_WaitForVSync_Info";
	}
	else if ( !V_stricmp( pName, "DrpQueuedMode" ) )
	{
		pTitle = "#L4D360UI_VideoOptions_Queued_Mode";
		pDesc = "#PORTAL2_VideoOptions_QueuedMode_Info";
	}
	else if ( !V_stricmp( pName, "DrpShaderDetail" ) )
	{
		pTitle = "#GameUI_Shader_Detail";
		pDesc = "#PORTAL2_VideoOptions_ShaderDetail_Info";
	}
	else if ( !V_stricmp( pName, "DrpCPUDetail" ) )
	{
		pTitle = "#L4D360UI_VideoOptions_CPU_Detail";
		pDesc = "#PORTAL2_VideoOptions_CPUDetail_Info";
	}
	else if ( !V_stricmp( pName, "DrpModelDetail" ) )
	{
		pTitle = "#L4D360UI_VideoOptions_Model_Texture_Detail";
		pDesc = "#PORTAL2_VideoOptions_ModelDetail_Info";
	}
	else if ( !V_stricmp( pName, "DrpPagedPoolMem" ) )
	{
		pTitle = "#L4D360UI_VideoOptions_Paged_Pool_Mem";
		pDesc = "#L4D360UI_VideoOptions_Paged_Pool_Mem_Info";
	}
	else if ( !V_stricmp( pName, "DrpCoreAO" ) )
	{
		pTitle = "#GameUI_AmbientOcclusion";
		pDesc = "#GameUI_AmbientOcclusion_Info";
	}
	else if ( !V_stricmp( pName, "DrpCoreShadows" ) )
	{
		pTitle = "#GameUI_DynamicShadows";
		pDesc = "#GameUI_DynamicShadows_Info";
	}
	else if ( !V_stricmp( pName, "DrpCoreDepth" ) )
	{
		pTitle = "#GameUI_CoreDepthPrepass";
		pDesc = "#GameUI_CoreDepthPrepass_Info";
	}
	else if ( !V_stricmp( pName, "DrpCoreMovers" ) )
	{
		pTitle = "#GameUI_CoreShadowMovers";
		pDesc = "#GameUI_CoreShadowMovers_Info";
	}
	else if ( !V_stricmp( pName, "DrpCoreDirect" ) )
	{
		pTitle = "#GameUI_CoreRuntimeDirect";
		pDesc = "#GameUI_CoreRuntimeDirect_Info";
	}

	if ( pTitle )
		m_lblDescriptionTitle->SetText( pTitle );
	if ( pDesc )
		m_lblDescription->SetText( pDesc );
}

void CAdvancedVideo::UpdateFooter()
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

//-----------------------------------------------------------------------------
// Purpose: Adds the five render core quality rows below the last row:
//			copies of Model Detail's row, so they
//			look and navigate like the others. The shipped advancedvideo.res
//			predates them. The dialog grows to fit the added rows.
//-----------------------------------------------------------------------------
static KeyValues *AddRenderCoreQualityRow( KeyValues *pResourceData, KeyValues *pTemplate,
    const char *pName, const char *pLabel, const char *pCommandPrefix, int nChoices, int nY )
{
	KeyValues *pRow = pTemplate->MakeCopy();
	pRow->SetName( pName );
	pRow->SetString( "fieldName", pName );
	pRow->SetString( "labelText", pLabel );
	pRow->SetInt( "ypos", nY );
	pRow->SetInt( "visible", 1 );
	pRow->SetInt( "enabled", 1 );
	if ( KeyValues *pOldList = pRow->FindKey( "list" ) )
	{
		pRow->RemoveSubKey( pOldList );
		pOldList->deleteThis();
	}
	KeyValues *pList = pRow->FindKey( "list", true );
	for ( int i = 0; i < nChoices; ++i )
		pList->SetString( nChoices == kRenderCoreToggleChoices ? s_RenderCoreToggleNames[i]
		                                                       : s_RenderCoreQualityNames[i],
		    CFmtStr( "%s%d", pCommandPrefix, i ) );
	pResourceData->AddSubKey( pRow );
	return pRow;
}

void CAdvancedVideo::PreApplyControlSettings( KeyValues *pResourceData )
{
	if ( !pResourceData )
		return;

#ifdef POSIX
	// The paged pool is a Windows kernel resource; the shipped res hides its
	// row with [$OSX] conditionals only. POSIX builds remove the row here and
	// splice its navigation links, so the list never shows or focuses a
	// control whose commands are compiled out.
	if ( KeyValues *pPagedPool = pResourceData->FindKey( "DrpPagedPoolMem" ) )
	{
		const char *pNavUp = pPagedPool->GetString( "navUp" );
		const char *pNavDown = pPagedPool->GetString( "navDown" );
		if ( KeyValues *pAbove = pResourceData->FindKey( pNavUp ) )
			pAbove->SetString( "navDown", pNavDown );
		if ( KeyValues *pBelow = pResourceData->FindKey( pNavDown ) )
			pBelow->SetString( "navUp", pNavUp );
		pResourceData->RemoveSubKey( pPagedPool );
		pPagedPool->deleteThis();
	}
#endif

	if ( pResourceData->FindKey( "DrpCoreAO" ) || !HasRenderCoreQuality() )
		return;

	// The last row is the lowest list row; it navigates down to the first.
	KeyValues *pTemplate = pResourceData->FindKey( "DrpModelDetail" );
	KeyValues *pFrame = NULL;
	KeyValues *pLast = NULL;
	for ( KeyValues *pKey = pResourceData->GetFirstTrueSubKey(); pKey;
	    pKey = pKey->GetNextTrueSubKey() )
	{
		if ( !V_stricmp( pKey->GetString( "ControlName" ), "Frame" ) )
			pFrame = pKey;
		else if ( !V_stricmp( pKey->GetString( "style" ), "DialogListButton" ) &&
		          ( !pLast || pKey->GetInt( "ypos" ) > pLast->GetInt( "ypos" ) ) )
			pLast = pKey;
	}
	if ( !pTemplate || !pLast )
		return;
	AddRenderCoreQualityStrings();

	KeyValues *pAboveLast = pResourceData->FindKey( pLast->GetString( "navUp" ) );
	int nPitch = pAboveLast ? pLast->GetInt( "ypos" ) - pAboveLast->GetInt( "ypos" ) : 0;
	if ( nPitch <= 0 )
		nPitch = 25;
	const int nAOY = pLast->GetInt( "ypos" ) + nPitch;
	KeyValues *pAO = AddRenderCoreQualityRow( pResourceData, pTemplate, "DrpCoreAO",
	    "#GameUI_AmbientOcclusion", VIDEO_CORE_AO_COMMAND_PREFIX, kRenderCoreAOChoices, nAOY );
	KeyValues *pShadows = AddRenderCoreQualityRow( pResourceData, pTemplate, "DrpCoreShadows",
	    "#GameUI_DynamicShadows", VIDEO_CORE_SHADOWS_COMMAND_PREFIX, kRenderCoreShadowChoices,
	    nAOY + nPitch );
	KeyValues *pDepth = AddRenderCoreQualityRow( pResourceData, pTemplate, "DrpCoreDepth",
	    "#GameUI_CoreDepthPrepass", VIDEO_CORE_DEPTH_COMMAND_PREFIX, kRenderCoreToggleChoices,
	    nAOY + 2 * nPitch );
	KeyValues *pMovers = AddRenderCoreQualityRow( pResourceData, pTemplate, "DrpCoreMovers",
	    "#GameUI_CoreShadowMovers", VIDEO_CORE_MOVERS_COMMAND_PREFIX, kRenderCoreToggleChoices,
	    nAOY + 3 * nPitch );
	KeyValues *pDirect = AddRenderCoreQualityRow( pResourceData, pTemplate, "DrpCoreDirect",
	    "#GameUI_CoreRuntimeDirect", VIDEO_CORE_DIRECT_COMMAND_PREFIX, kRenderCoreToggleChoices,
	    nAOY + 4 * nPitch );

	const char *pFirst = pLast->GetString( "navDown" );
	if ( KeyValues *pFirstRow = pResourceData->FindKey( pFirst ) )
		pFirstRow->SetString( "navUp", "DrpCoreDirect" );
	pDirect->SetString( "navDown", pFirst );
	pDirect->SetString( "navUp", "DrpCoreMovers" );
	pMovers->SetString( "navDown", "DrpCoreDirect" );
	pMovers->SetString( "navUp", "DrpCoreDepth" );
	pDepth->SetString( "navDown", "DrpCoreMovers" );
	pDepth->SetString( "navUp", "DrpCoreShadows" );
	pShadows->SetString( "navDown", "DrpCoreDepth" );
	pShadows->SetString( "navUp", "DrpCoreAO" );
	pAO->SetString( "navDown", "DrpCoreShadows" );
	pAO->SetString( "navUp", pLast->GetName() );
	pLast->SetString( "navDown", "DrpCoreAO" );

	// The frame's size is in dialog tiles (Dialog.TileHeight, the same
	// proportional units as the rows).
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
	const int nTileTall = pScheme ? atoi( pScheme->GetResourceString( "Dialog.TileHeight" ) ) : 0;
	if ( pFrame && nTileTall > 0 )
	{
		const int nBottom = pDirect->GetInt( "ypos" ) + pDirect->GetInt( "tall" );
		const int nTiles = ( nBottom + nTileTall - 1 ) / nTileTall;
		if ( nTiles > pFrame->GetInt( "tall" ) )
			pFrame->SetInt( "tall", nTiles );
	}
}

bool CAdvancedVideo::DescribeRenderCoreQuality( char *pOut, int nOutSize )
{
	if ( !m_drpCoreAO || !m_drpCoreShadows || !m_drpCoreDepth || !m_drpCoreMovers ||
	     !m_drpCoreDirect )
		return false;
	const char *szAO = m_drpCoreAO->GetCurrentSelection();
	const char *szShadows = m_drpCoreShadows->GetCurrentSelection();
	if ( !szAO )
		szAO = "";
	if ( !szShadows )
		szShadows = "";
	// The localized text as the list shows it; a missing token reads "?".
	const wchar_t *wszAO = g_pVGuiLocalize->Find( szAO );
	const wchar_t *wszShadows = g_pVGuiLocalize->Find( szShadows );
	char szAOText[64], szShadowsText[64];
	g_pVGuiLocalize->ConvertUnicodeToANSI( wszAO ? wszAO : L"?", szAOText, sizeof( szAOText ) );
	g_pVGuiLocalize->ConvertUnicodeToANSI(
	    wszShadows ? wszShadows : L"?", szShadowsText, sizeof( szShadowsText ) );
	V_snprintf( pOut, nOutSize,
	    "ambient occlusion \"%s\" (%s), dynamic shadows \"%s\" (%s), "
	    "depth prepass %d, moving shadows %d, runtime direct %d",
	    szAOText, szAO, szShadowsText, szShadows, m_iCoreDepth, m_iCoreMovers, m_iCoreDirect );
	return true;
}

void CAdvancedVideo::ApplyRenderCoreQualityChoice(
    int nAO, int nShadows, int nDepth, int nMovers, int nDirect )
{
	OnCommand( CFmtStr( "%s%d", VIDEO_CORE_AO_COMMAND_PREFIX, nAO ) );
	OnCommand( CFmtStr( "%s%d", VIDEO_CORE_SHADOWS_COMMAND_PREFIX, nShadows ) );
	if ( nDepth >= 0 )
		OnCommand( CFmtStr( "%s%d", VIDEO_CORE_DEPTH_COMMAND_PREFIX, nDepth ) );
	if ( nMovers >= 0 )
		OnCommand( CFmtStr( "%s%d", VIDEO_CORE_MOVERS_COMMAND_PREFIX, nMovers ) );
	if ( nDirect >= 0 )
		OnCommand( CFmtStr( "%s%d", VIDEO_CORE_DIRECT_COMMAND_PREFIX, nDirect ) );
	SetRenderCoreQualityState();
	ApplyChanges();
}

// Developer check (RFC 0016 K12): opens the advanced video dialog and prints
// the render core quality it shows. Optional values select all five rows
// and apply them as the A button does. The dialog lays out on its first frame, so run it again after a wait.
CON_COMMAND_F( ui_show_video_advanced,
    "Opens the advanced video dialog and prints its render core quality; "
    "[ao shadows [depth movers direct]] selects and applies those rows",
    FCVAR_CHEAT )
{
	CBaseModPanel &panel = CBaseModPanel::GetSingleton();
	CAdvancedVideo *pDialog = static_cast<CAdvancedVideo *>( panel.GetWindow( WT_ADVANCEDVIDEO ) );
	if ( !pDialog )
	{
		pDialog = static_cast<CAdvancedVideo *>(
		    panel.OpenWindow( WT_ADVANCEDVIDEO, panel.GetWindow( panel.GetActiveWindowType() ) ) );
	}
	if ( !pDialog )
		return;
	char szQuality[256];
	if ( !pDialog->DescribeRenderCoreQuality( szQuality, sizeof( szQuality ) ) )
	{
		Msg( "advanced video: render core quality: not laid out yet\n" );
		return;
	}
	if ( args.ArgC() >= 3 )
	{
		pDialog->ApplyRenderCoreQualityChoice( atoi( args[1] ), atoi( args[2] ),
		    args.ArgC() >= 6 ? atoi( args[3] ) : -1, args.ArgC() >= 6 ? atoi( args[4] ) : -1,
		    args.ArgC() >= 6 ? atoi( args[5] ) : -1 );
		pDialog->DescribeRenderCoreQuality( szQuality, sizeof( szQuality ) );
	}
	Msg( "advanced video: render core quality: %s\n", szQuality );
}
