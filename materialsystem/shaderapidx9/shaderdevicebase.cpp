//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#define DISABLE_PROTECTED_THINGS
#include "togl/rendermechanism.h"
#include "shaderdevicebase.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include "tier1/utlbuffer.h"
#include "tier0/icommandline.h"
#include "tier2/tier2.h"
#include "filesystem.h"
#include "datacache/idatacache.h"
#include "shaderapi/ishaderutil.h"
#include "shaderapibase.h"
#include "shaderapi/ishadershadow.h"
#include "shaderapi_global.h"
#include "winutils.h"
#include "dxsupport_keyvalues.h"


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
IShaderUtil* g_pShaderUtil;		// The main shader utility interface
CShaderDeviceBase *g_pShaderDevice;
CShaderDeviceMgrBase *g_pShaderDeviceMgr;
CShaderAPIBase *g_pShaderAPI;
IShaderShadow *g_pShaderShadow;

bool g_bUseShaderMutex = false;	// Shader mutex globals
bool g_bShaderAccessDisallowed;
CShaderMutex g_ShaderMutex;

//-----------------------------------------------------------------------------
// FIXME: Hack related to setting command-line values for convars. Remove!!!
//-----------------------------------------------------------------------------
class CShaderAPIConVarAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase( ConCommandBase *pCommand )
	{
		// Link to engine's list instead
		g_pCVar->RegisterConCommand( pCommand );

		char const *pValue = g_pCVar->GetCommandLineValue( pCommand->GetName() );
		if( pValue && !pCommand->IsCommand() )
		{
			( ( ConVar * )pCommand )->SetValue( pValue );
		}
		return true;
	}
};

static void InitShaderAPICVars( )
{
	static CShaderAPIConVarAccessor g_ConVarAccessor;
	if ( g_pCVar )
	{
		ConVar_Register( FCVAR_MATERIAL_SYSTEM_THREAD, &g_ConVarAccessor );
	}
}



//-----------------------------------------------------------------------------
// Read dx support levels
//-----------------------------------------------------------------------------
#if defined( DX_TO_GL_ABSTRACTION )
	#if defined( OSX )
		// OSX
		#define SUPPORT_CFG_FILE "dxsupport_mac.cfg"
		// TODO: make this different for Mac?
		#define SUPPORT_CFG_OVERRIDE_FILE "dxsupport_override.cfg"
	#else
		// Linux/Win GL
		#define SUPPORT_CFG_FILE "dxsupport_linux.cfg"
		// TODO: make this different for Linux?
		#define SUPPORT_CFG_OVERRIDE_FILE "dxsupport_override.cfg"
	#endif
#else
	// D3D
	#define SUPPORT_CFG_FILE "dxsupport.cfg"
	#define SUPPORT_CFG_OVERRIDE_FILE "dxsupport_override.cfg"
#endif


//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
CShaderDeviceMgrBase::CShaderDeviceMgrBase()
{
	m_pDXSupport = NULL;
}

CShaderDeviceMgrBase::~CShaderDeviceMgrBase()
{
}


//-----------------------------------------------------------------------------
// Factory used to get at internal interfaces (used by shaderapi + shader dlls)
//-----------------------------------------------------------------------------
static CreateInterfaceFn s_TempFactory;
void *ShaderDeviceFactory( const char *pName, int *pReturnCode )
{
	if (pReturnCode)
	{
		*pReturnCode = IFACE_OK;
	}

	void *pInterface = s_TempFactory( pName, pReturnCode );
	if ( pInterface )
		return pInterface;

	pInterface = Sys_GetFactoryThis()( pName, pReturnCode );
	if ( pInterface )
		return pInterface;

	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;	
}

//-----------------------------------------------------------------------------
// Connect, disconnect
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrBase::Connect( CreateInterfaceFn factory )
{
	LOCK_SHADERAPI();

	Assert( !g_pShaderDeviceMgr );

	s_TempFactory = factory;

	// Connection/convar registration
	CreateInterfaceFn actualFactory = ShaderDeviceFactory;
	ConnectTier1Libraries( &actualFactory, 1 );
	InitShaderAPICVars();
	ConnectTier2Libraries( &actualFactory, 1 );
	g_pShaderUtil = (IShaderUtil*)ShaderDeviceFactory( SHADER_UTIL_INTERFACE_VERSION, NULL );
	g_pShaderDeviceMgr = this;

	s_TempFactory = NULL;

	if ( !g_pShaderUtil || !g_pFullFileSystem || !g_pShaderDeviceMgr )
	{
		Warning( "ShaderAPIDx10 was unable to access the required interfaces!\n" );
		return false;
	}

	// NOTE! : Overbright is 1.0 so that Hammer will work properly with the white bumped and unbumped lightmaps.
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	return true;
}

void CShaderDeviceMgrBase::Disconnect()
{
	LOCK_SHADERAPI();

	g_pShaderDeviceMgr = NULL;
	g_pShaderUtil = NULL;
	DisconnectTier2Libraries();
	ConVar_Unregister();
	DisconnectTier1Libraries();

	if ( m_pDXSupport )
	{
		m_pDXSupport->deleteThis();
		m_pDXSupport = NULL;
	}
}


//-----------------------------------------------------------------------------
// Query interface
//-----------------------------------------------------------------------------
void *CShaderDeviceMgrBase::QueryInterface( const char *pInterfaceName )
{
	if ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_MGR_INTERFACE_VERSION ) )
		return ( IShaderDeviceMgr* )this;
	if ( !Q_stricmp( pInterfaceName, MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION ) )
		return ( IMaterialSystemHardwareConfig* )g_pHardwareConfig;
	return NULL;
}


//-----------------------------------------------------------------------------
// Returns the hardware caps for a particular adapter
//-----------------------------------------------------------------------------
const HardwareCaps_t& CShaderDeviceMgrBase::GetHardwareCaps( int nAdapter ) const
{
	Assert( ( nAdapter >= 0 ) && ( nAdapter < GetAdapterCount() ) );
	return m_Adapters[nAdapter].m_ActualCaps;
}


//-----------------------------------------------------------------------------
// Utility methods for reading config scripts
//-----------------------------------------------------------------------------
static bool ReadBool( KeyValues *pGroup, const char *pKeyName, bool bDefault )
{
	int nVal = pGroup->GetInt( pKeyName, -1 );
	if ( nVal != -1 )
	{
		//		Warning( "\t%s = %s\n", pKeyName, (nVal != false) ? "true" : "false" );
		return (nVal != false);
	}
	return bDefault;
}

static void ReadInt( KeyValues *pGroup, const char *pKeyName, int nInvalidValue, int *pResult )
{
	int nVal = pGroup->GetInt( pKeyName, nInvalidValue );
	if ( nVal != nInvalidValue )
	{
		*pResult = nVal;
		//		Warning( "\t%s = %d\n", pKeyName, *pResult );
	}
}


//-----------------------------------------------------------------------------
// Reads in the dxsupport.cfg keyvalues (dxsupport_keyvalues.h)
//-----------------------------------------------------------------------------
KeyValues *CShaderDeviceMgrBase::ReadDXSupportKeyValues()
{
	if ( CommandLine()->CheckParm( "-ignoredxsupportcfg" ) )
		return NULL;

	if ( !m_pDXSupport )
		m_pDXSupport = dxsupport::ReadConfig( g_pFullFileSystem, SUPPORT_CFG_FILE, SUPPORT_CFG_OVERRIDE_FILE );
	return m_pDXSupport;
}



//-----------------------------------------------------------------------------
// Returns the max dx support level achievable with this board
//-----------------------------------------------------------------------------
void CShaderDeviceMgrBase::ReadDXSupportLevels( HardwareCaps_t &caps )
{
	// See if the file tells us otherwise
	KeyValues *pCfg = ReadDXSupportKeyValues();
	if ( !pCfg )
		return;

	KeyValues *pDeviceKeyValues = dxsupport::FindCardGroup( pCfg, caps.m_VendorID, caps.m_DeviceID );
	if ( pDeviceKeyValues )
	{
		// First, set the max dx level
		int nMaxDXSupportLevel = 0;
		ReadInt( pDeviceKeyValues, "MaxDXLevel", 0, &nMaxDXSupportLevel );
		if ( nMaxDXSupportLevel != 0 )
		{
			caps.m_nMaxDXSupportLevel = nMaxDXSupportLevel;
		}

		// Next, set the preferred dx level
		int nDXSupportLevel = 0;
		ReadInt( pDeviceKeyValues, "DXLevel", 0, &nDXSupportLevel );
		if ( nDXSupportLevel != 0 )
		{
			caps.m_nDXSupportLevel = nDXSupportLevel;
			// Don't slam up the dxlevel level to 92 on DX10 cards in OpenGL Linux/Win mode (otherwise Intel will get dxlevel 92 when we want 90)
			if ( !( IsOpenGL() && ( IsLinux() || IsWindows() ) ) )
			{
				if ( caps.m_bDX10Card )
				{
					caps.m_nDXSupportLevel = 92;
				}
			}
		}
		else
		{
			caps.m_nDXSupportLevel = caps.m_nMaxDXSupportLevel;
		}
	}
}


//-----------------------------------------------------------------------------
// Loads the hardware caps, for cases in which the D3D caps lie or where we need to augment the caps
//-----------------------------------------------------------------------------
void CShaderDeviceMgrBase::LoadHardwareCaps( KeyValues *pGroup, HardwareCaps_t &caps )
{
	if( !pGroup )
		return;

	// don't just blanket kill clip planes on POSIX, only shoot them down if we're running ARB, or asked for nouserclipplanes.
	//FIXME need to take into account the caps bit that GLM can now provide, so NV can use normal clipping and ATI can fall back to fastclip.
	
	if ( CommandLine()->FindParm("-arbmode") || CommandLine()->CheckParm( "-nouserclip" ) )
	{
		caps.m_UseFastClipping = true;
	}
	else
	{
		caps.m_UseFastClipping = ReadBool( pGroup, "NoUserClipPlanes", caps.m_UseFastClipping );
	}

	caps.m_bNeedsATICentroidHack = ReadBool( pGroup, "CentroidHack", caps.m_bNeedsATICentroidHack );
	caps.m_bDisableShaderOptimizations = ReadBool( pGroup, "DisableShaderOptimizations", caps.m_bDisableShaderOptimizations );
}



//-----------------------------------------------------------------------------
// Reads in the hardware caps from the dxsupport.cfg file: the device groups
// in render.dxsupport-policy.v1 order.
//-----------------------------------------------------------------------------
void CShaderDeviceMgrBase::ReadHardwareCaps( HardwareCaps_t &caps, int nDxLevel )
{
	KeyValues *pCfg = ReadDXSupportKeyValues();
	if ( !pCfg )
		return;

	for ( KeyValues *pGroup : dxsupport::DeviceGroups( pCfg, nDxLevel, caps.m_VendorID, caps.m_DeviceID ) )
		LoadHardwareCaps( pGroup, caps );
}


//-----------------------------------------------------------------------------
// Gets the recommended configuration associated with a particular dx level
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrBase::GetRecommendedConfigurationInfo( int nAdapter, int nDXLevel, int nVendorID, int nDeviceID, KeyValues *pConfiguration ) 
{
	LOCK_SHADERAPI();

	const HardwareCaps_t& caps = GetHardwareCaps( nAdapter );
	if ( nDXLevel == 0 )
	{ 
		nDXLevel = caps.m_nDXSupportLevel;
	}
	nDXLevel = GetClosestActualDXLevel( nDXLevel );
	if ( nDXLevel > caps.m_nMaxDXSupportLevel )
		return false;

	KeyValues *pCfg = ReadDXSupportKeyValues();
	if ( !pCfg )
		return true;

	render::DxSupportQuery query;
	query.dxLevel = nDXLevel;
	query.maxDxLevel = caps.m_nMaxDXSupportLevel;
	query.vendorId = nVendorID;
	query.deviceId = nDeviceID;
	query.videoMemoryBytes = GetVidMemBytes( nAdapter );
	dxsupport::FillHostFacts( &query );
	dxsupport::ApplyRecommendedConfig( pCfg, query, pConfiguration );
	return true;
}


//-----------------------------------------------------------------------------
// Gets recommended congifuration for a particular adapter at a particular dx level
//-----------------------------------------------------------------------------
bool CShaderDeviceMgrBase::GetRecommendedConfigurationInfo( int nAdapter, int nDXLevel, KeyValues *pCongifuration )
{
	Assert( nAdapter >= 0 && nAdapter <= GetAdapterCount() );
	MaterialAdapterInfo_t info;
	GetAdapterInfo( nAdapter, info );
	return GetRecommendedConfigurationInfo( nAdapter, nDXLevel, info.m_VendorID, info.m_DeviceID, pCongifuration );
}


//-----------------------------------------------------------------------------
// Returns only valid dx levels
//-----------------------------------------------------------------------------
int CShaderDeviceMgrBase::GetClosestActualDXLevel( int nDxLevel ) const
{
	return render::ClosestActualDxLevel( nDxLevel, IsOpenGL(), ABSOLUTE_MINIMUM_DXLEVEL );
}


//-----------------------------------------------------------------------------
// Mode change callback
//-----------------------------------------------------------------------------
void CShaderDeviceMgrBase::AddModeChangeCallback( ShaderModeChangeCallbackFunc_t func )
{
	LOCK_SHADERAPI();
	Assert( func && m_ModeChangeCallbacks.Find( func ) < 0 );
	m_ModeChangeCallbacks.AddToTail( func );
}

void CShaderDeviceMgrBase::RemoveModeChangeCallback( ShaderModeChangeCallbackFunc_t func )
{
	LOCK_SHADERAPI();
	m_ModeChangeCallbacks.FindAndRemove( func );
}

void CShaderDeviceMgrBase::InvokeModeChangeCallbacks()
{
	int nCount = m_ModeChangeCallbacks.Count();
	for ( int i = 0; i < nCount; ++i )
	{
		m_ModeChangeCallbacks[i]();
	}
}


//-----------------------------------------------------------------------------
// Factory to return from SetMode
//-----------------------------------------------------------------------------
void* CShaderDeviceMgrBase::ShaderInterfaceFactory( const char *pInterfaceName, int *pReturnCode )
{
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_OK;
	}
	if ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_INTERFACE_VERSION ) )
		return static_cast< IShaderDevice* >( g_pShaderDevice );
	if ( !Q_stricmp( pInterfaceName, SHADERAPI_INTERFACE_VERSION ) )
		return static_cast< IShaderAPI* >( g_pShaderAPI );
	if ( !Q_stricmp( pInterfaceName, SHADERSHADOW_INTERFACE_VERSION ) )
		return static_cast< IShaderShadow* >( g_pShaderShadow );

	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;
}


//-----------------------------------------------------------------------------
//
// The Base implementation of the shader device
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
CShaderDeviceBase::CShaderDeviceBase()
{
	m_bInitialized = false;
	m_nAdapter = -1;
	m_hWnd = NULL;
	m_hWndCookie = NULL;
	m_dwThreadId = ThreadGetCurrentId();
}

CShaderDeviceBase::~CShaderDeviceBase()
{
}

void CShaderDeviceBase::SetCurrentThreadAsOwner()
{
	m_dwThreadId = ThreadGetCurrentId();
}

void CShaderDeviceBase::RemoveThreadOwner()
{
	m_dwThreadId = 0xFFFFFFFF;
}

bool CShaderDeviceBase::ThreadOwnsDevice()
{
	if ( ThreadGetCurrentId() == m_dwThreadId )
		return true;
	return false;
}


// Methods of IShaderDevice
ImageFormat CShaderDeviceBase::GetBackBufferFormat() const
{
	return IMAGE_FORMAT_UNKNOWN;
}

int CShaderDeviceBase::StencilBufferBits() const
{
	return 0;
}

bool CShaderDeviceBase::IsAAEnabled() const
{
	return false;
}


//-----------------------------------------------------------------------------
// Methods for interprocess communication to release resources
//-----------------------------------------------------------------------------
#define MATERIAL_SYSTEM_WINDOW_ID		0xFEEDDEAD

#ifdef USE_ACTUAL_DX
static VD3DHWND GetTopmostParentWindow( VD3DHWND hWnd )
{
	// Find the parent window...
	VD3DHWND hParent = GetParent( hWnd );
	while ( hParent )
	{
		hWnd = hParent;
		hParent = GetParent( hWnd );
	}

	return hWnd;
}

static BOOL CALLBACK EnumChildWindowsProc( VD3DHWND hWnd, LPARAM lParam )
{
	int windowId = GetWindowLongPtr( hWnd, GWLP_USERDATA );
	if (windowId == MATERIAL_SYSTEM_WINDOW_ID)
	{
		COPYDATASTRUCT copyData;
		copyData.dwData = lParam;
		copyData.cbData = 0;
		copyData.lpData = 0;

		SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&copyData);
	}
	return TRUE;
}

static BOOL CALLBACK EnumWindowsProc( VD3DHWND hWnd, LPARAM lParam )
{
	EnumChildWindows( hWnd, EnumChildWindowsProc, lParam );
	return TRUE;
}

static BOOL CALLBACK EnumWindowsProcNotThis( VD3DHWND hWnd, LPARAM lParam )
{
	if ( g_pShaderDevice && ( GetTopmostParentWindow( (VD3DHWND)g_pShaderDevice->GetIPCHWnd() ) == hWnd ) )
		return TRUE;

	EnumChildWindows( hWnd, EnumChildWindowsProc, lParam );
	return TRUE;
}
#endif

//-----------------------------------------------------------------------------
// Adds a hook to let us know when other instances are setting the mode
//-----------------------------------------------------------------------------

#ifdef STRICT
#define WINDOW_PROC WNDPROC
#else
#define WINDOW_PROC FARPROC
#endif

#ifdef USE_ACTUAL_DX
static LRESULT CALLBACK ShaderDX8WndProc(VD3DHWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	// FIXME: Should these IPC messages tell when an app has focus or not?
	// If so, we'd want to totally disable the shader api layer when an app
	// doesn't have focus.

	// Look for the special IPC message that tells us we're trying to set
	// the mode....
	switch(msg)
	{
	case WM_COPYDATA:
		{
			if ( !g_pShaderDevice )
				break;

			COPYDATASTRUCT* pData = (COPYDATASTRUCT*)lParam;

			// that number is our magic cookie number
			if ( pData->dwData == CShaderDeviceBase::RELEASE_MESSAGE )
			{
				g_pShaderDevice->OtherAppInitializing(true);
			}
			else if ( pData->dwData == CShaderDeviceBase::REACQUIRE_MESSAGE )  
			{
				g_pShaderDevice->OtherAppInitializing(false);
			}
			else if ( pData->dwData == CShaderDeviceBase::EVICT_MESSAGE )  
			{
				g_pShaderDevice->EvictManagedResourcesInternal( );
			}
		}
		break;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}
#endif


//-----------------------------------------------------------------------------
// Install, remove ability to talk to other shaderapi apps
//-----------------------------------------------------------------------------
void CShaderDeviceBase::InstallWindowHook( void* hWnd )
{
	Assert( m_hWndCookie == NULL );
#ifdef USE_ACTUAL_DX
	VD3DHWND hParent = GetTopmostParentWindow( (VD3DHWND)hWnd );

	// Attach a child window to the parent; we're gonna store special info there
	// We can't use the USERDATA, cause other apps may want to use this.
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr( hParent, GWLP_HINSTANCE );
	WNDCLASS		wc;
	memset( &wc, 0, sizeof( wc ) );
	wc.style         = CS_NOCLOSE | CS_PARENTDC;
	wc.lpfnWndProc   = ShaderDX8WndProc;
	wc.hInstance     = hInst;
	wc.lpszClassName = "shaderdx8";

	// In case an old one is sitting around still...
	UnregisterClass( "shaderdx8", hInst );

	RegisterClass( &wc );

	// Create the window
	m_hWndCookie = CreateWindow( "shaderdx8", "shaderdx8", WS_CHILD, 
		0, 0, 0, 0, hParent, NULL, hInst, NULL );

	// Marks it as a material system window
	SetWindowLongPtr( (VD3DHWND)m_hWndCookie, GWLP_USERDATA, MATERIAL_SYSTEM_WINDOW_ID );
#endif
}

void CShaderDeviceBase::RemoveWindowHook( void* hWnd )
{
#ifdef USE_ACTUAL_DX
	if ( m_hWndCookie )
	{
		DestroyWindow( (VD3DHWND)m_hWndCookie ); 
		m_hWndCookie = 0;
	}

	VD3DHWND hParent = GetTopmostParentWindow( (VD3DHWND)hWnd );
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr( hParent, GWLP_HINSTANCE );
	UnregisterClass( "shaderdx8", hInst );
#endif
}


//-----------------------------------------------------------------------------
// Sends a message to other shaderapi applications
//-----------------------------------------------------------------------------
void CShaderDeviceBase::SendIPCMessage( IPCMessage_t msg )
{
#ifdef USE_ACTUAL_DX
	// Gotta send this to all windows, since we don't know which ones
	// are material system apps...
	if ( msg != EVICT_MESSAGE )
	{
		EnumWindows( EnumWindowsProc, (DWORD)msg );
	}
	else
	{
		EnumWindows( EnumWindowsProcNotThis, (DWORD)msg );
	}
#endif
}


//-----------------------------------------------------------------------------
// Find view
//-----------------------------------------------------------------------------
int CShaderDeviceBase::FindView( void* hWnd ) const
{
	/* FIXME: Is this necessary?
	// Look for the view in the list of views
	for (int i = m_Views.Count(); --i >= 0; )
	{
	if (m_Views[i].m_HWnd == (VD3DHWND)hwnd)
	return i;
	}
	*/
	return -1;
}

//-----------------------------------------------------------------------------
// Creates a child window
//-----------------------------------------------------------------------------
bool CShaderDeviceBase::AddView( void* hWnd )
{
	LOCK_SHADERAPI();
	/*
	// If we haven't created a device yet
	if (!Dx9Device())
		return false;

	// Make sure no duplicate hwnds...
	if (FindView(hwnd) >= 0)
		return false;

	// In this case, we need to create the device; this is our
	// default swap chain. This here says we're gonna use a part of the
	// existing buffer and just grab that.
	int view = m_Views.AddToTail();
	m_Views[view].m_HWnd = (VD3DHWND)hwnd;
	//	memcpy( &m_Views[view].m_PresentParamters, m_PresentParameters, sizeof(m_PresentParamters) );

	HRESULT hr;
	hr = Dx9Device()->CreateAdditionalSwapChain( &m_PresentParameters,
	&m_Views[view].m_pSwapChain );
	return !FAILED(hr);
	*/

	return true;
}

void CShaderDeviceBase::RemoveView( void* hWnd )
{
	LOCK_SHADERAPI();
	/*
	// Look for the view in the list of views
	int i = FindView(hwnd);
	if (i >= 0)
	{
	// FIXME		m_Views[i].m_pSwapChain->Release();
	m_Views.FastRemove(i);
	}
	*/
}

//-----------------------------------------------------------------------------
// Activates a child window
//-----------------------------------------------------------------------------
void CShaderDeviceBase::SetView( void* hWnd )
{
	LOCK_SHADERAPI();

	ShaderViewport_t viewport;
	g_pShaderAPI->GetViewports( &viewport, 1 );

	// Get the window (*not* client) rect of the view window
	m_ViewHWnd = (VD3DHWND)hWnd;
	GetWindowSize( m_nWindowWidth, m_nWindowHeight );

	// Reset the viewport (takes into account the view rect)
	// Don't need to set the viewport if it's not ready
	g_pShaderAPI->SetViewports( 1, &viewport );
}


//-----------------------------------------------------------------------------
// Gets the window size
//-----------------------------------------------------------------------------
void CShaderDeviceBase::GetWindowSize( int& nWidth, int& nHeight ) const
{
#if defined( USE_SDL )

	// this matches up to what the threaded material system does
	g_pShaderAPI->GetBackBufferDimensions( nWidth, nHeight );

#else

	// If the window was minimized last time swap buffers happened, or if it's iconic now, 
	// return 0 size
#ifdef _WIN32
	if ( !m_bIsMinimized && !IsIconic( ( HWND )m_hWnd ) )
#else
	if ( !m_bIsMinimized && !IsIconic( (VD3DHWND)m_hWnd ) )
#endif
	{
		// NOTE: Use the 'current view' (which may be the same as the main window) 
		RECT rect;
#ifdef _WIN32
		GetClientRect( ( HWND )m_ViewHWnd, &rect );
#else
		toglGetClientRect( (VD3DHWND)m_ViewHWnd, &rect );
#endif
		nWidth = rect.right - rect.left;
		nHeight = rect.bottom - rect.top;
	}
	else
	{
		nWidth = nHeight = 0;
	}

#endif
}


