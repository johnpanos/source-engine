//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy shader-API modules seen through the render.contracts
//			provider contract (RFC 0001 render migration steps 2-3, rank 8).
//
//			LegacyRenderBackendProvider adapts a bound LegacyShaderServices bundle
//			around its IShaderDeviceMgr: identity comes from the catalog entry,
//			adapter identity from IShaderDeviceMgr::GetAdapterInfo, and semantic
//			features from the backend's optional describeAdapter facts. Adapter
//			descriptions are captured once, as values, when the provider is built.
//
//			Claims are deliberately narrow. Legacy devices are still created by
//			IShaderDeviceMgr::SetMode with a native window inside the material
//			system, and draw work stays on the IShaderAPI command model (RFC 0001
//			step 9). The provider therefore advertises no offscreen device or
//			presentation: CreateDevice validates the adapter and required features
//			structurally and otherwise reports kNotAdvertised. The shared
//			render-backend suite certifies exactly those claims.
//
//=============================================================================//

#ifndef LEGACY_RENDER_BACKEND_PROVIDER_H
#define LEGACY_RENDER_BACKEND_PROVIDER_H

#include "render/legacy_shader_provider.h"
#include "render/render_backend.h"
#include "render/render_profile.h"

struct MaterialAdapterInfo_t;

namespace render
{

// The only part of IShaderDeviceMgr the provider uses: adapter enumeration.
// Production code views the bound manager through it; tests supply fakes.
class ILegacyAdapterSource
{
public:
	virtual int GetAdapterCount() const = 0;
	virtual void GetAdapterInfo( int adapter, MaterialAdapterInfo_t &info ) const = 0;

protected:
	~ILegacyAdapterSource() {}
};

typedef bool ( *LegacyDescribeAdapterFn )( int adapter, RenderAdapterInfo *info );

class LegacyRenderBackendProvider : public IRenderBackendProvider
{
public:
	static const int kMaxAdapters = 16;

	// Borrows nothing past construction: the adapter descriptions are copied.
	// 'services' must hold a connected manager.
	LegacyRenderBackendProvider(
	    const LegacyShaderProvider &provider, const LegacyShaderServices &services );
	// 'adapters' may be null (an invalid provider); 'describe' is optional.
	LegacyRenderBackendProvider( const LegacyShaderProvider &provider,
	    const ILegacyAdapterSource *adapters, LegacyDescribeAdapterFn describe );

	// False when the backend's facts contradict its manager (for example the
	// manager enumerates an adapter the describe hook rejects). An invalid
	// provider advertises no adapters.
	bool IsValid() const { return m_bValid; }
	const char *InvalidReason() const { return m_InvalidReason; }

	RenderBackendId GetBackendId() const override;
	RenderProviderCaps GetProviderCaps() const override;
	int GetAdapterCount() const override;
	bool GetAdapterInfo( int index, RenderAdapterInfo *out ) const override;
	IRenderDevice *CreateDevice(
	    const RenderDeviceRequest &request, RenderCreateError *error ) override;
	void DestroyDevice( IRenderDevice *device ) override;
	size_t GetLiveDeviceCount() const override;
	bool OwnsDevice( const IRenderDevice &device ) const override;

private:
	void Capture( const ILegacyAdapterSource *adapters, LegacyDescribeAdapterFn describe );
	void Invalidate( const char *reason );

	const char *m_pId;
	const char *m_pName;
	bool m_bValid;
	char m_InvalidReason[128];
	int m_nAdapters;
	RenderAdapterInfo m_Adapters[kMaxAdapters];
};

// The documented quirk table for the legacy providers. Static data; it outlives
// every profile selected from it.
const RenderQuirk *LegacyRenderQuirks( size_t *count );

// The single owner of "bound legacy provider + adapter + request -> profile".
// The material system calls it during Init; tests call it with real and fake
// services.
bool SelectLegacyRenderProfile( const LegacyShaderProvider &provider,
    const LegacyShaderServices &services, int adapter, const RenderProfileRequest &request,
    RenderFeatureProfile *profile, RenderProfileError *error );
bool SelectLegacyRenderProfile( const LegacyRenderBackendProvider &backend, int adapter,
    const RenderProfileRequest &request, RenderFeatureProfile *profile, RenderProfileError *error );

// Writes a one-line human description of a profile for logs and diagnostics.
void DescribeRenderProfile( const RenderFeatureProfile &profile, char *buffer, size_t size );

} // namespace render

#endif // LEGACY_RENDER_BACKEND_PROVIDER_H
