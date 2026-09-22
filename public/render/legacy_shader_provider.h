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

// The build links exactly one implementation. No filename, native handle, or
// untyped interface registry crosses this boundary.
extern "C" bool ShaderBackend_Create( render::LegacyShaderServices *services );
extern "C" const render::LegacyShaderProvider *ShaderBackend_Describe();
extern "C" const render::LegacyShaderProvider *NullShaderBackend_Describe();
extern "C" bool MaterialSystem_BindShaderProvider(
	IMaterialSystem *materialSystem, const render::LegacyShaderProvider *provider );

#endif // RENDER_LEGACY_SHADER_PROVIDER_H
