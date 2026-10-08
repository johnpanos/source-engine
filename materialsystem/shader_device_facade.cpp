//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The material system's one IShaderDeviceMgr; see
//			shader_device_facade.h and RFC 0016's legacy device facade.
//
//=============================================================================//

#include "shader_device_facade.h"

#include "dxsupport_keyvalues.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "render/dxsupport_policy.h"
#include "render/render_backend.h"
#include "render/render_sample_count.h"
#include "shaderapi/ishaderapi.h"
#include "tier1/KeyValues.h"
#include "tier1/strtools.h"
#if defined( USE_SDL )
#include "appframework/ilaunchermgr.h"
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

namespace
{
// render::LegacyRenderBackendProvider's bound on enumerated adapters.
const int kMaxFacadeAdapters = 16;

void ToShaderDisplayMode( const render::DisplayModeFacts &mode, ShaderDisplayMode_t *pInfo )
{
	pInfo->m_nWidth = mode.width;
	pInfo->m_nHeight = mode.height;
	pInfo->m_Format = IMAGE_FORMAT_BGRA8888;
	pInfo->m_nRefreshRateNumerator = mode.refreshNumerator;
	pInfo->m_nRefreshRateDenominator = mode.refreshDenominator;
}
} // namespace

CShaderDeviceFacade::~CShaderDeviceFacade()
{
	Unbind();
}

void CShaderDeviceFacade::Bind( const render::LegacyShaderServices &services )
{
	Unbind();
	m_Services = services;
	m_pBackend = services.manager;
}

void CShaderDeviceFacade::Unbind()
{
	if ( m_pDXSupport )
		m_pDXSupport->deleteThis();
	m_pDXSupport = NULL;
	m_bDXSupportRead = false;
	m_Modes.clear();
	m_pBackend = NULL;
	m_Services = render::LegacyShaderServices();
}

bool CShaderDeviceFacade::Connect( CreateInterfaceFn factory )
{
	if ( !m_pBackend )
		return false;
#if defined( USE_SDL )
	// Optional: application roots without a launcher (tools, tests) get no modes.
	m_pLauncherMgr = (ILauncherMgr *)factory( SDLMGR_INTERFACE_VERSION, NULL );
#endif
	// Optional: without a file system there is no dxsupport.cfg to recommend from.
	m_pFileSystem = (IFileSystem *)factory( FILESYSTEM_INTERFACE_VERSION, NULL );
	return m_pBackend->Connect( factory );
}

void CShaderDeviceFacade::Disconnect()
{
	if ( m_pBackend )
		m_pBackend->Disconnect();
	if ( m_pDXSupport )
		m_pDXSupport->deleteThis();
	m_pDXSupport = NULL;
	m_bDXSupportRead = false;
	m_Modes.clear();
	m_pLauncherMgr = NULL;
	m_pFileSystem = NULL;
}

void *CShaderDeviceFacade::QueryInterface( const char *pInterfaceName )
{
	if ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_MGR_INTERFACE_VERSION ) )
		return static_cast<IShaderDeviceMgr *>( this );
	return m_pBackend ? m_pBackend->QueryInterface( pInterfaceName ) : NULL;
}

InitReturnVal_t CShaderDeviceFacade::Init()
{
	return m_pBackend ? m_pBackend->Init() : INIT_FAILED;
}

void CShaderDeviceFacade::Shutdown()
{
	if ( m_pBackend )
		m_pBackend->Shutdown();
}

// Identity (name, vendor, device, driver, memory) comes from the render
// core's adapter description when the root supplied one; the backend's
// describeAdapter supplies the semantic facts (features, driverApi, software)
// and, without the core, the identity too.
bool CShaderDeviceFacade::Describe( int nAdapter, render::RenderAdapterInfo *pInfo ) const
{
	*pInfo = render::RenderAdapterInfo();
	if ( !m_pBackend || nAdapter < 0 || nAdapter >= kMaxFacadeAdapters ||
	     !m_Services.describeAdapter || !m_Services.describeAdapter( nAdapter, pInfo ) )
		return false;
	const render::LegacyShaderServices::CoreAdapterSource &core = m_Services.coreAdapter;
	if ( core.describe )
	{
		render::RenderAdapterInfo identity;
		if ( !core.describe( core.context, nAdapter, &identity ) )
			return false;
		V_strncpy( pInfo->name, identity.name, sizeof( pInfo->name ) );
		pInfo->vendorId = identity.vendorId;
		pInfo->deviceId = identity.deviceId;
		pInfo->driverVersion = identity.driverVersion;
		pInfo->deviceMemoryBytes = identity.deviceMemoryBytes;
	}
	return true;
}

int CShaderDeviceFacade::GetAdapterCount() const
{
	render::RenderAdapterInfo info;
	int count = 0;
	while ( count < kMaxFacadeAdapters && Describe( count, &info ) )
		++count;
	return count;
}

void CShaderDeviceFacade::GetAdapterInfo( int nAdapter, MaterialAdapterInfo_t &info ) const
{
	memset( &info, 0, sizeof( info ) );
	render::RenderAdapterInfo adapter;
	if ( !Describe( nAdapter, &adapter ) )
		return;
	V_strncpy( info.m_pDriverName, adapter.name, sizeof( info.m_pDriverName ) );
	info.m_VendorID = adapter.vendorId;
	info.m_DeviceID = adapter.deviceId;
	info.m_nDriverVersionHigh = static_cast<unsigned int>( adapter.driverVersion >> 32 );
	info.m_nDriverVersionLow = static_cast<unsigned int>( adapter.driverVersion & 0xffffffffu );
	const int level = m_Services.hardware ? m_Services.hardware->GetMaxDXSupportLevel() : 0;
	info.m_nDXSupportLevel = level;
	info.m_nMaxDXSupportLevel = level;
}

// dxsupport.cfg through the shared render.dxsupport-policy.v1 owner, for this
// adapter's identity, then held to what the backend supports: a
// recommendation never names a mode the Video options could not apply. A
// software adapter (the null backend) has no recommendation.
bool CShaderDeviceFacade::GetRecommendedConfigurationInfo(
    int nAdapter, int nDXLevel, KeyValues *pConfiguration )
{
	render::RenderAdapterInfo adapter;
	if ( !Describe( nAdapter, &adapter ) || !m_Services.hardware )
		return false;
	if ( adapter.isSoftware )
		return true;
	const int maxLevel = m_Services.hardware->GetMaxDXSupportLevel();
	const int nLevel = render::ClosestActualDxLevel(
	    nDXLevel != 0 ? nDXLevel : maxLevel, false, ABSOLUTE_MINIMUM_DXLEVEL );
	if ( nLevel > maxLevel )
		return false;

	if ( !m_pDXSupport && !m_bDXSupportRead )
	{
		m_bDXSupportRead = true;
		m_pDXSupport =
		    dxsupport::ReadConfig( m_pFileSystem, "dxsupport.cfg", "dxsupport_override.cfg" );
	}
	if ( !m_pDXSupport )
		return true;

	render::DxSupportQuery query;
	query.dxLevel = nLevel;
	query.maxDxLevel = maxLevel;
	query.vendorId = static_cast<int>( adapter.vendorId );
	query.deviceId = static_cast<int>( adapter.deviceId );
	query.videoMemoryBytes = m_Services.hardware->TextureMemorySize();
	dxsupport::FillHostFacts( &query );
	dxsupport::ApplyRecommendedConfig( m_pDXSupport, query, pConfiguration );
	ClampToCapabilities( pConfiguration );
	return true;
}

void CShaderDeviceFacade::ClampToCapabilities( KeyValues *pConfiguration ) const
{
	IShaderAPI *pApi = m_Services.api;
	if ( !pApi )
		return;
	uint32_t sampleMask = 1;
	for ( int samples = 2; samples <= render::kMaxSampleCount; samples *= 2 )
	{
		if ( pApi->SupportsMSAAMode( samples ) )
			sampleMask |= static_cast<uint32_t>( samples );
	}
	if ( KeyValues *pAA = pConfiguration->FindKey( "ConVar.mat_antialias" ) )
	{
		const int samples = render::ClampSampleCount( pAA->GetInt(), sampleMask );
		if ( samples != pAA->GetInt() && pAA->GetInt() > 1 )
		{
			pConfiguration->SetInt( "ConVar.mat_antialias", samples );
			pConfiguration->SetInt( "ConVar.mat_aaquality", 0 );
		}
	}
	if ( !pApi->SupportsShadowDepthTextures() )
		pConfiguration->SetInt( "ConVar.r_flashlightdepthtexture", 0 );
	if ( KeyValues *pHDR = pConfiguration->FindKey( "ConVar.mat_hdr_level" ) )
	{
		if ( pHDR->GetInt() > 2 )
			pConfiguration->SetInt( "ConVar.mat_hdr_level", 2 );
	}
}

// The desktop of the display the launcher selected (sdl_displayindex). The
// launcher owns display selection; false when there is none (tools, tests,
// the dedicated server).
bool CShaderDeviceFacade::QueryDesktopDisplay( render::DisplayModeFacts *pDesktop ) const
{
	*pDesktop = render::DisplayModeFacts();
	if ( m_pDesktopSource )
		m_pDesktopSource( m_pDesktopContext, pDesktop );
#if defined( USE_SDL )
	else if ( m_pLauncherMgr )
	{
		uint width = 0, height = 0, refreshHz = 0;
		m_pLauncherMgr->GetNativeDisplayInfo( -1, width, height, refreshHz );
		pDesktop->width = static_cast<int>( width );
		pDesktop->height = static_cast<int>( height );
		pDesktop->refreshNumerator = static_cast<int>( refreshHz );
		pDesktop->refreshDenominator = 1;
	}
#endif
	if ( pDesktop->width > 0 && pDesktop->height > 0 )
		return true;
	render::RenderAdapterInfo adapter;
	const bool software = Describe( 0, &adapter ) && adapter.isSoftware;
	if ( !software && !m_bWarnedNoDisplay )
	{
		Warning( "[ShaderDevice] no desktop display to enumerate video modes from\n" );
		m_bWarnedNoDisplay = true;
	}
	return false;
}

// SDL3 fullscreen covers the desktop and presentation scales the back buffer
// to it, so the modes are back-buffer sizes that fit the desktop
// (render.display-modes.v1), all at the desktop refresh rate.
void CShaderDeviceFacade::RefreshModeList() const
{
	render::DisplayModeFacts desktop;
	std::vector<render::DisplayModeFacts> modes = QueryDesktopDisplay( &desktop )
	                                                  ? render::BuildBackBufferModeList( desktop )
	                                                  : std::vector<render::DisplayModeFacts>();
	if ( !modes.empty() &&
	     ( modes.size() != m_Modes.size() || modes.back().width != m_Modes.back().width ||
	         modes.back().height != m_Modes.back().height ) )
		Msg( "[ShaderDevice] %d video modes for desktop %dx%d@%d (%dx%d .. %dx%d)\n",
		    static_cast<int>( modes.size() ), desktop.width, desktop.height,
		    desktop.refreshNumerator, modes.front().width, modes.front().height, modes.back().width,
		    modes.back().height );
	m_Modes = std::move( modes );
}

int CShaderDeviceFacade::GetModeCount( int nAdapter ) const
{
	RefreshModeList();
	return static_cast<int>( m_Modes.size() );
}

void CShaderDeviceFacade::GetModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter, int nMode ) const
{
	if ( !pInfo )
		return;
	if ( m_Modes.empty() )
		RefreshModeList();
	if ( nMode < 0 || nMode >= static_cast<int>( m_Modes.size() ) )
	{
		Warning( "[ShaderDevice] GetModeInfo: mode %d of %d requested\n", nMode,
		    static_cast<int>( m_Modes.size() ) );
		ToShaderDisplayMode( render::DisplayModeFacts(), pInfo );
		pInfo->m_nRefreshRateDenominator = 0;
		return;
	}
	ToShaderDisplayMode( m_Modes[nMode], pInfo );
}

// The display's current mode is the desktop's: fullscreen never changes it.
void CShaderDeviceFacade::GetCurrentModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter ) const
{
	if ( !pInfo )
		return;
	render::DisplayModeFacts desktop;
	QueryDesktopDisplay( &desktop );
	ToShaderDisplayMode( desktop, pInfo );
	if ( desktop.width <= 0 )
		pInfo->m_nRefreshRateDenominator = 0;
}

bool CShaderDeviceFacade::SetAdapter( int nAdapter, int nFlags )
{
	return m_pBackend && m_pBackend->SetAdapter( nAdapter, nFlags );
}

CreateInterfaceFn CShaderDeviceFacade::SetMode(
    void *hWnd, int nAdapter, const ShaderDeviceInfo_t &mode )
{
	return m_pBackend ? m_pBackend->SetMode( hWnd, nAdapter, mode ) : NULL;
}

void CShaderDeviceFacade::AddModeChangeCallback( ShaderModeChangeCallbackFunc_t func )
{
	if ( m_pBackend )
		m_pBackend->AddModeChangeCallback( func );
}

void CShaderDeviceFacade::RemoveModeChangeCallback( ShaderModeChangeCallbackFunc_t func )
{
	if ( m_pBackend )
		m_pBackend->RemoveModeChangeCallback( func );
}
