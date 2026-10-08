//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The material system's one IShaderDeviceMgr (RFC 0016 legacy device
//			facade, slice F1; user decision 2026-10-07). It answers adapter
//			enumeration and identity, the recommended configuration, the
//			video mode list and the current mode itself, from the render
//			core's adapter description (LegacyShaderServices::coreAdapter,
//			set by the composition root through the legacy frontend), the
//			backend's semantic adapter facts (describeAdapter), its hardware
//			config, the launcher's desktop display and dxsupport.cfg. The
//			linked backend keeps only what its device bring-up still owns
//			until slices F2-F4 move it: the app-system lifecycle, SetAdapter,
//			SetMode and the mode-change callbacks, which this facade forwards.
//
//=============================================================================//

#ifndef SHADER_DEVICE_FACADE_H
#define SHADER_DEVICE_FACADE_H

#include "render/legacy/stream_device.h"
#include "render/legacy_shader_provider.h"
#include "render/render_display_modes.h"
#include "shaderapi/IShaderDevice.h"

#include <vector>

class IFileSystem;
class ILauncherMgr;
class KeyValues;

class CShaderDeviceFacade final : public IShaderDeviceMgr
{
public:
	~CShaderDeviceFacade();

	// Takes the selected backend's services; services.manager must be the
	// backend's own manager. Rebinding releases the previous binding.
	void Bind( const render::LegacyShaderServices &services );
	void Unbind();
	bool IsBound() const { return m_bBound; }

	// Methods of IAppSystem: the backend's lifecycle, plus this facade's
	// optional services (the launcher's display, the file system).
	bool Connect( CreateInterfaceFn factory ) override;
	void Disconnect() override;
	void *QueryInterface( const char *pInterfaceName ) override;
	InitReturnVal_t Init() override;
	void Shutdown() override;

	// Methods of IShaderDeviceMgr answered here.
	int GetAdapterCount() const override;
	void GetAdapterInfo( int nAdapter, MaterialAdapterInfo_t &info ) const override;
	bool GetRecommendedConfigurationInfo(
	    int nAdapter, int nDXLevel, KeyValues *pConfiguration ) override;
	int GetModeCount( int nAdapter ) const override;
	void GetModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter, int nMode ) const override;
	void GetCurrentModeInfo( ShaderDisplayMode_t *pInfo, int nAdapter ) const override;

	// Has the render core create the device for the engine's window, before
	// the backend sets a video mode (CMaterialSystem::SetMode calls it ahead
	// of IShaderAPI::SetMode). True without a core device source.
	bool PrepareDevice( void *hWnd );

	// Methods of IShaderDeviceMgr the backend's device bring-up still owns.
	bool SetAdapter( int nAdapter, int nFlags ) override;
	CreateInterfaceFn SetMode( void *hWnd, int nAdapter, const ShaderDeviceInfo_t &mode ) override;
	void AddModeChangeCallback( ShaderModeChangeCallbackFunc_t func ) override;
	void RemoveModeChangeCallback( ShaderModeChangeCallbackFunc_t func ) override;

	// A desktop display source in place of the launcher's (tests); null
	// restores the launcher.
	typedef bool ( *DesktopSourceFn )( void *pContext, render::DisplayModeFacts *pDesktop );
	void SetDesktopSource( DesktopSourceFn fn, void *pContext )
	{
		m_pDesktopSource = fn;
		m_pDesktopContext = pContext;
	}

private:
	bool Describe( int nAdapter, render::RenderAdapterInfo *pInfo ) const;
	bool QueryDesktopDisplay( render::DisplayModeFacts *pDesktop ) const;
	void RefreshModeList() const;
	void ClampToCapabilities( KeyValues *pConfiguration ) const;

	bool m_bBound = false;
	render::LegacyShaderServices m_Services;
	ILauncherMgr *m_pLauncherMgr = NULL;
	DesktopSourceFn m_pDesktopSource = NULL;
	void *m_pDesktopContext = NULL;
	IFileSystem *m_pFileSystem = NULL;
	KeyValues *m_pDXSupport = NULL;
	bool m_bDXSupportRead = false;
	// Rebuilt by GetModeCount, which the engine calls before GetModeInfo.
	mutable std::vector<render::DisplayModeFacts> m_Modes;
	mutable bool m_bWarnedNoDisplay = false;
};

// The material system's one IShaderDevice (RFC 0016 legacy device facade,
// F4): whether a device presents, the back buffer, the window, samples,
// stencil and the gamma ramp from the backend's presentation source; the
// adapter from the core's adapter source; the legacy draw stream's resources
// and frame forwarded to the backend's ILegacyStreamDevice. No backend
// implements IShaderDevice.
class CShaderDeviceFacadeDevice final : public IShaderDevice
{
public:
	void Bind( const render::LegacyShaderServices &services );
	void Unbind();

	// Answered here.
	int GetCurrentAdapter() const override { return 0; }
	bool IsUsingGraphics() const override;
	void SpewDriverInfo() const override;
	// Every backend's back buffer is 8-bit RGB.
	ImageFormat GetBackBufferFormat() const override { return IMAGE_FORMAT_RGB888; }
	void GetBackBufferDimensions( int &width, int &height ) const override;
	int StencilBufferBits() const override;
	bool IsAAEnabled() const override;
	void GetWindowSize( int &width, int &height ) const override;
	void SetHardwareGammaRamp( float fGamma, float fGammaTVRangeMin, float fGammaTVRangeMax,
	    float fGammaTVExponent, bool bTVEnabled ) override;
	char *GetDisplayDeviceName() override { return const_cast<char *>( "" ); }

	// The legacy draw stream: the backend's.
	void Present() override { Stream().Present(); }
	void ReleaseResources() override { Stream().ReleaseResources(); }
	void ReacquireResources() override { Stream().ReacquireResources(); }
	bool AddView( void *hWnd ) override { return Stream().AddView( hWnd ); }
	void RemoveView( void *hWnd ) override { Stream().RemoveView( hWnd ); }
	void SetView( void *hWnd ) override { Stream().SetView( hWnd ); }
	IShaderBuffer *CompileShader(
	    const char *pProgram, size_t nBufLen, const char *pVersion ) override
	{
		return Stream().CompileShader( pProgram, nBufLen, pVersion );
	}
	VertexShaderHandle_t CreateVertexShader( IShaderBuffer *pBuffer ) override
	{
		return Stream().CreateVertexShader( pBuffer );
	}
	void DestroyVertexShader( VertexShaderHandle_t hShader ) override
	{
		Stream().DestroyVertexShader( hShader );
	}
	GeometryShaderHandle_t CreateGeometryShader( IShaderBuffer *pBuffer ) override
	{
		return Stream().CreateGeometryShader( pBuffer );
	}
	void DestroyGeometryShader( GeometryShaderHandle_t hShader ) override
	{
		Stream().DestroyGeometryShader( hShader );
	}
	PixelShaderHandle_t CreatePixelShader( IShaderBuffer *pBuffer ) override
	{
		return Stream().CreatePixelShader( pBuffer );
	}
	void DestroyPixelShader( PixelShaderHandle_t hShader ) override
	{
		Stream().DestroyPixelShader( hShader );
	}
	IMesh *CreateStaticMesh(
	    VertexFormat_t format, const char *pBudgetGroup, IMaterial *pMaterial = NULL ) override
	{
		return Stream().CreateStaticMesh( format, pBudgetGroup, pMaterial );
	}
	void DestroyStaticMesh( IMesh *pMesh ) override { Stream().DestroyStaticMesh( pMesh ); }
	IVertexBuffer *CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t format,
	    int nVertexCount, const char *pBudgetGroup ) override
	{
		return Stream().CreateVertexBuffer( type, format, nVertexCount, pBudgetGroup );
	}
	void DestroyVertexBuffer( IVertexBuffer *pBuffer ) override
	{
		Stream().DestroyVertexBuffer( pBuffer );
	}
	IIndexBuffer *CreateIndexBuffer( ShaderBufferType_t type, MaterialIndexFormat_t format,
	    int nIndexCount, const char *pBudgetGroup ) override
	{
		return Stream().CreateIndexBuffer( type, format, nIndexCount, pBudgetGroup );
	}
	void DestroyIndexBuffer( IIndexBuffer *pBuffer ) override
	{
		Stream().DestroyIndexBuffer( pBuffer );
	}
	IVertexBuffer *GetDynamicVertexBuffer(
	    int nStreamID, VertexFormat_t format, bool bBuffered = true ) override
	{
		return Stream().GetDynamicVertexBuffer( nStreamID, format, bBuffered );
	}
	IIndexBuffer *GetDynamicIndexBuffer(
	    MaterialIndexFormat_t format, bool bBuffered = true ) override
	{
		return Stream().GetDynamicIndexBuffer( format, bBuffered );
	}
	void EnableNonInteractiveMode(
	    MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo = NULL ) override
	{
		Stream().EnableNonInteractiveMode( mode, pInfo );
	}
	void RefreshFrontBufferNonInteractive() override
	{
		Stream().RefreshFrontBufferNonInteractive();
	}
	void HandleThreadEvent( uint32 threadEvent ) override
	{
		Stream().HandleThreadEvent( threadEvent );
	}

private:
	render::LegacyPresentationFacts Facts() const;
	// Bound before any use (CMaterialSystem binds it with the provider).
	render::legacy::ILegacyStreamDevice &Stream() const { return *m_Services.stream; }

	render::LegacyShaderServices m_Services;
};

#endif // SHADER_DEVICE_FACADE_H
