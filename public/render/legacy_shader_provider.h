//========= Copyright Valve Corporation, All rights reserved. ============//
//
// A temporary, typed boundary between the material system and its linked
// renderer. These are existing Source contracts, not native graphics SDK types.
// The selected provider owns every borrowed service through Disconnect. This
// bundle is private to composition and the material compatibility adapter; new
// render consumers use render_backend.h instead.
//
//=============================================================================//

#ifndef RENDER_LEGACY_SHADER_PROVIDER_H
#define RENDER_LEGACY_SHADER_PROVIDER_H

#include "render/render_profile.h"

class IShaderDeviceMgr;
class IShaderAPI;
class IShaderDevice;
class IShaderShadow;
class IMaterialSystemHardwareConfig;
class IDebugTextureInfo;
class IMaterialSystem;

namespace render
{

struct LegacyShaderServices
{
	IShaderDeviceMgr *manager = nullptr;
	IShaderAPI *api = nullptr;
	IShaderDevice *device = nullptr;
	IShaderShadow *shadow = nullptr;
	IMaterialSystemHardwareConfig *hardware = nullptr;
	IDebugTextureInfo *debugTextures = nullptr;

	// Optional. Backend-owned facts the legacy MaterialAdapterInfo_t cannot carry:
	// semantic features, driverApi, memory and software status for an adapter the
	// manager enumerates. Valid once the manager is connected; returns false for
	// an index the manager does not enumerate. Without it an adapter reports no
	// semantic features. See LegacyRenderBackendProvider.
	bool ( *describeAdapter )( int adapter, RenderAdapterInfo *info ) = nullptr;

	bool IsComplete() const
	{
		return manager && api && device && shadow && hardware;
	}
};

// A build-selected entry in the composition root's render catalog. The factory
// supplies borrowed Source services; their linked module outlives consumers.
// legacyModuleName is used only by the old SetShaderAPI compatibility entry.
struct LegacyShaderProvider
{
	const char *id;
	const char *legacyModuleName;
	bool ( *create )( LegacyShaderServices *services );
};

} // namespace render

// A product may link several backends and let its composition root choose one at
// runtime, so every implementation exports its OWN named entry point. A shared
// `ShaderBackend_Describe` would collide across the linked modules and the
// dynamic linker would silently bind all callers to whichever module it resolved
// first, making the other backends unselectable. No filename, native handle, or
// untyped interface registry crosses this boundary.
extern "C" const render::LegacyShaderProvider *Dx9ShaderBackend_Describe();
extern "C" bool Dx9ShaderBackend_Create( render::LegacyShaderServices *services );
extern "C" const render::LegacyShaderProvider *NativeVulkanShaderBackend_Describe();
extern "C" bool NativeVulkanShaderBackend_Create( render::LegacyShaderServices *services );
extern "C" const render::LegacyShaderProvider *NullShaderBackend_Describe();
extern "C" bool MaterialSystem_BindShaderProvider(
	IMaterialSystem *materialSystem, const render::LegacyShaderProvider *provider );

// The composition root's feature requirements for the bound provider. Accepted
// only before Connect; the material system selects the profile during Init from
// the selected adapter's facts and the documented quirk table, and fails Init if
// a required feature is unavailable. Roots that never call this get
// render::PreferAvailableRenderFeatures() (the legacy tool default).
extern "C" bool MaterialSystem_SetRenderProfileRequest(
    IMaterialSystem *materialSystem, const render::RenderProfileRequest *request );
// Copies the profile selected by Init. Returns false before selection.
extern "C" bool MaterialSystem_GetRenderProfile(
    IMaterialSystem *materialSystem, render::RenderFeatureProfile *profile );

#endif // RENDER_LEGACY_SHADER_PROVIDER_H
