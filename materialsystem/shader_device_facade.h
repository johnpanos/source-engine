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
	bool IsBound() const { return m_pBackend != NULL; }

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

	IShaderDeviceMgr *m_pBackend = NULL;
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

#endif // SHADER_DEVICE_FACADE_H
