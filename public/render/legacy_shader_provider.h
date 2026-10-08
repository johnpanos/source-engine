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
namespace world_mesh_gpu
{
class IWorldMeshUpload;
}
namespace light_set
{
class ILightSetConsumer;
}
namespace gpu_compute
{
class IGpuCompute;
}
namespace render::legacy
{
class ICorePassSlots;
class ILegacyStreamDevice;
}
struct ShaderDeviceInfo_t;
typedef void *( *CreateInterfaceFn )( const char *pName, int *pReturnCode );

namespace render
{

// The presented frame as the presentation reports it (RFC 0016 legacy device
// facade, F4): what the material system's IShaderDevice answers.
struct LegacyPresentationFacts
{
	bool presenting = false; // a device presents to the window
	int backBufferWidth = 1024;
	int backBufferHeight = 768;
	int windowWidth = 0;
	int windowHeight = 0;
	int samples = 1;
	int stencilBits = 0;
};

struct LegacyShaderServices
{
	// The material system's IShaderDeviceMgr and IShaderDevice facades
	// (CShaderDeviceFacade, RFC 0016 legacy device facade): the material
	// system sets them when it binds the provider; a backend never does, and
	// implements neither interface (F4).
	IShaderDeviceMgr *manager = nullptr;
	IShaderDevice *device = nullptr;

	IShaderAPI *api = nullptr;
	// The legacy draw stream's resources and frame (render/legacy/stream_device.h).
	render::legacy::ILegacyStreamDevice *stream = nullptr;
	IShaderShadow *shadow = nullptr;
	IMaterialSystemHardwareConfig *hardware = nullptr;
	IDebugTextureInfo *debugTextures = nullptr;
	// Optional map-scoped upload capability. The engine reaches it through the
	// render core's legacy frontend (render/legacy/capabilities.h), which
	// orders its calls with the frame.
	world_mesh_gpu::IWorldMeshUpload *worldMeshUpload = nullptr;
	// Optional per-frame light set sink (render/light_set.h, RFC 0011), reached
	// the same way.
	light_set::ILightSetConsumer *lightSetConsumer = nullptr;
	// Optional compute service for engine-side GPU producers
	// (render/gpu_compute.h, RFC 0011 G6), reached the same way.
	gpu_compute::IGpuCompute *gpuCompute = nullptr;
	// Optional slots for core passes inside the legacy stream
	// (render/legacy/core_passes.h, RFC 0016 K5), reached the same way.
	render::legacy::ICorePassSlots *corePassSlots = nullptr;

	// Optional. Backend-owned facts the legacy MaterialAdapterInfo_t cannot carry:
	// semantic features, driverApi, memory and software status for an adapter the
	// manager enumerates. Valid once the manager is connected; returns false for
	// an index the manager does not enumerate. Without it an adapter reports no
	// semantic features. See LegacyRenderBackendProvider.
	bool ( *describeAdapter )( int adapter, RenderAdapterInfo *info ) = nullptr;

	// Optional (RFC 0016 legacy device facade, F1): the render core's
	// identity (name, vendor, device, driver, device memory) of the adapters
	// its device adapter creates the device on, set by the composition root
	// through the legacy frontend. When set, the material system's device
	// manager (CShaderDeviceFacade) takes identity from it and only the
	// semantic facts from describeAdapter; false for an index the core does
	// not enumerate.
	struct CoreAdapterSource
	{
		void *context = nullptr;
		bool ( *describe )( void *context, int adapter, RenderAdapterInfo *info ) = nullptr;
	} coreAdapter;

	// Optional (RFC 0016 legacy device facade, F2): the render core's owner
	// of the backend's device. The material system's device facade has it
	// create the device for the engine's window before the backend sets a
	// video mode (prepare: false with the reason, the mode then fails), and
	// release it after the backend shut down. A backend without a device
	// (the null one) leaves it empty.
	struct CoreDeviceSource
	{
		void *context = nullptr;
		bool ( *prepare )( void *context, void *window, char *error, size_t errorSize ) = nullptr;
		void ( *release )( void *context ) = nullptr;
	} coreDevice;

	// The backend's app-system lifecycle and video-mode bring-up, which the
	// material system's IShaderDeviceMgr facade forwards (F4).
	struct Lifecycle
	{
		void *context = nullptr;
		bool ( *connect )( void *context, CreateInterfaceFn factory ) = nullptr;
		void ( *disconnect )( void *context ) = nullptr;
		bool ( *init )( void *context ) = nullptr; // INIT_OK
		void ( *shutdown )( void *context ) = nullptr;
		bool ( *setAdapter )( void *context, int adapter, int flags ) = nullptr;
		CreateInterfaceFn ( *setMode )(
		    void *context, void *window, int adapter, const ShaderDeviceInfo_t &mode ) = nullptr;
	} lifecycle;

	// Optional (F4): the presentation the backend's frames reach the window
	// through, which the material system's IShaderDevice facade answers from.
	// Without it the device presents nothing (the null backend).
	struct PresentationSource
	{
		void *context = nullptr;
		void ( *facts )( void *context, LegacyPresentationFacts *out ) = nullptr;
		// mat_monitorgamma's ramp, applied by presents; any thread.
		void ( *setGammaRamp )( void *context, float gamma, float tvRangeMin, float tvRangeMax,
		    float tvExponent, bool tvEnabled ) = nullptr;
		// The material system's mode-change callbacks.
		void ( *addModeChangeCallback )( void *context, void ( *callback )() ) = nullptr;
		void ( *removeModeChangeCallback )( void *context, void ( *callback )() ) = nullptr;
		// A display that has exactly one mode (a handheld's screen): its
		// width, height and refresh rate. Without it the launcher's desktop
		// display is enumerated.
		bool ( *fixedDisplay )( void *context, int *width, int *height, int *refreshHz ) = nullptr;
	} presentation;

	bool IsComplete() const
	{
		return lifecycle.connect && lifecycle.init && lifecycle.shutdown && lifecycle.setMode &&
		       api && stream && shadow && hardware;
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
	// The provider implements the queued material-system contract: meshes locked
	// on the main thread are described by IShaderAPI::ComputeVertexDescription and
	// their vertex bytes replayed into the provider's own mesh on the render
	// thread. A provider without it runs the material system single-threaded.
	bool supportsQueuedRendering;
	// Optional, for a provider with its own state (the render core's legacy
	// frontend, RFC 0016): when createFor is set the material system calls
	// createFor( context, services ) instead of create, so no global holds
	// the state. context is owned by the provider and outlives the binding.
	void *context = nullptr;
	bool ( *createFor )( void *context, LegacyShaderServices *services ) = nullptr;
};

} // namespace render

// A product may link several backends and let its composition root choose one at
// runtime, so every implementation exports its OWN named entry point. A shared
// `ShaderBackend_Describe` would collide across the linked modules and the
// dynamic linker would silently bind all callers to whichever module it resolved
// first, making the other backends unselectable. No filename, native handle, or
// untyped interface registry crosses this boundary.
extern "C" const render::LegacyShaderProvider *NativeVulkanShaderBackend_Describe();
extern "C" bool NativeVulkanShaderBackend_Create( render::LegacyShaderServices *services );
// The definitions export these from their module (DLL_EXPORT); MSVC requires
// the declarations to agree (C2375).
#if defined( _MSC_VER )
#define LEGACY_SHADER_PROVIDER_EXPORT __declspec( dllexport )
#else
#define LEGACY_SHADER_PROVIDER_EXPORT
#endif

extern "C" LEGACY_SHADER_PROVIDER_EXPORT const render::LegacyShaderProvider *
NullShaderBackend_Describe();
// The Nintendo 3DS fullbright backend (materialsystem/shaderapipica).
extern "C" LEGACY_SHADER_PROVIDER_EXPORT const render::LegacyShaderProvider *
PicaShaderBackend_Describe();
extern "C" LEGACY_SHADER_PROVIDER_EXPORT bool MaterialSystem_BindShaderProvider(
    IMaterialSystem *materialSystem, const render::LegacyShaderProvider *provider );

// The composition root's feature requirements for the bound provider. Accepted
// only before Connect; the material system selects the profile during Init from
// the selected adapter's facts and the documented quirk table, and fails Init if
// a required feature is unavailable. Roots that never call this get
// render::PreferAvailableRenderFeatures() (the legacy tool default).
extern "C" LEGACY_SHADER_PROVIDER_EXPORT bool MaterialSystem_SetRenderProfileRequest(
    IMaterialSystem *materialSystem, const render::RenderProfileRequest *request );
// Copies the profile selected by Init. Returns false before selection.
extern "C" LEGACY_SHADER_PROVIDER_EXPORT bool MaterialSystem_GetRenderProfile(
    IMaterialSystem *materialSystem, render::RenderFeatureProfile *profile );

#endif // RENDER_LEGACY_SHADER_PROVIDER_H
