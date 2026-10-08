//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition (RFC 0016 A.7): assembles a complete render
//			core for an application root (the launcher, Hammer). It is the only
//			module that chooses a device adapter.
//
//			RenderCore_Create builds, in order: the device adapter, then the
//			legacy frontend around the root's legacy backend, then the renderer
//			with the configured features. It fails with a structured result,
//			naming what was missing, and leaves nothing behind.
//
//			The root then hands the binding out: the legacy provider to the
//			material system (MaterialSystem_BindShaderProvider), the renderer,
//			scene factory and stage markers to the engine
//			(Engine_BindRenderCore), and the stage markers to the client as the
//			RenderStageMarkers001 app system. The core outlives all of them;
//			the root destroys it last.
//
//			Engine-facing: no dual-ABI library type appears here.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_RENDER_CORE_H
#define RENDER_COMPOSITION_RENDER_CORE_H

#include "render/frame/renderer.h"
class IRenderStageMarkers;   // render/legacy/stage_markers.h (legacy-interop)
class IRenderMaterialBlocks; // render/legacy/material_blocks.h (legacy-interop)
#include "render/legacy/capabilities.h"
#include "render/legacy/core_passes.h"
#include "render/composition/render_core_panels.h"
#include "render/composition/render_core_world.h"
#include "render/legacy_shader_provider.h"
#include "render/scene/scene.h"

namespace jobsystem
{
class IWorkerBackend;
}

struct RenderCoreConfig
{
	// "null", "vulkan" or "gl"; the root takes it from -render-device, with
	// the product profile's default.
	const char *device = "null";
	// Comma-separated feature names from the product profile, in frame
	// order, e.g. "legacy-stream,present".
	const char *features = "legacy-stream,present";
	// Comma-separated "feature=fallback" substitutions the product profile
	// declares (RFC 0016 K10 "Capability negotiation"): a feature the device
	// cannot run is replaced by its declared fallback only when listed here,
	// and RenderCoreResult::substitutions names each one. The root takes it
	// from -render-fallbacks, with the product profile's default.
	const char *fallbacks = "";
	// Comma-separated capability names (render::device::CapabilityName) the
	// device must not claim: a profile takes the path that works without
	// them. An adapter that cannot mask fails composition by name.
	const char *maskedCapabilities = "";
	// The linked legacy backend the frontend wraps; may be null.
	const render::LegacyShaderProvider *legacyBackend = nullptr;
	bool validation = false;
	const char *temporalAssets = nullptr;
	bool temporal = false; // explicit experimental product selection; defaults unchanged
	// What the legacy frontend's core passes record until the world draws
	// from the scene (RFC 0016 K5 plan, step 3): null or "" marks no slot,
	// "empty" a label-only pass at each view's opaque stage, "seeded-clear"
	// its negative control. The root takes it from -render-core-passes.
	const char *corePasses = nullptr;
	// The root's compute workers, borrowed: they must outlive the core, and the
	// core starts no threads of its own. Null runs the core's compute work
	// (pooled culling) inline on the caller, the serial reference. Products
	// pass the process compute pool (CreateComputePoolWorkerBackend in
	// vstdlib/jobgraph_pool_bridge.h), never a pool the core's callers run on,
	// such as the material system's MatQueue pool: the core's pooled work would
	// then wait behind its own caller (RFC 0003's forbidden nested wait).
	jobsystem::IWorkerBackend *computeWorkers = nullptr;
};

enum RenderCoreStatus
{
	RENDER_CORE_OK = 0,
	RENDER_CORE_INVALID_CONFIG,
	RENDER_CORE_UNKNOWN_DEVICE,     // not an adapter this product links
	RENDER_CORE_DEVICE_FAILED,      // the adapter could not create a device
	RENDER_CORE_UNKNOWN_FEATURE,    // not a feature this product links
	RENDER_CORE_MISSING_CAPABILITY, // a feature needs what the device lacks, with no fallback
	RENDER_CORE_UNDECLARED_FALLBACK // a feature's fallback is not in RenderCoreConfig::fallbacks
};

struct RenderCoreResult
{
	RenderCoreStatus status = RENDER_CORE_OK;
	char message[256] = {};
	// Every substitution composition made, "feature -> fallback (lacks
	// capability)", comma-separated; empty when none. Set on success.
	char substitutions[256] = {};
	unsigned int substitutionCount = 0;
};

namespace render::device
{
class IRenderDevice2;
}

struct RenderCoreBinding
{
	// The core's device port (render.device.v2), for hosts that build their
	// own graphs on it (the Hammer viewports: offscreen passes, readback).
	// Owned by the core and valid until RenderCore_Destroy. Portable code
	// must not branch on which adapter it is (CAP011 rule 5; see deviceName).
	render::device::IRenderDevice2 *device = nullptr;
	render::frame::IRenderer *renderer = nullptr;
	render::scene::SceneFactory sceneFactory;
	IRenderStageMarkers *stageMarkers = nullptr;
	// RenderMaterialBlocks001, for the proxy corpus's frontend side.
	IRenderMaterialBlocks *materialBlocks = nullptr;
	// The legacy backend's world mesh, light set and compute capabilities, in
	// frame order (render/legacy/capabilities.h).
	render::legacy::ILegacyCapabilities *capabilities = nullptr;
	// The frontend's core-pass recorder, which the root binds into a legacy
	// backend that has slots (render/legacy/core_passes.h).
	render::legacy::ICorePassRecorder *corePasses = nullptr;
	// The BSP world drawn by the core; null when no legacy backend is composed.
	IRenderCoreWorld *world = nullptr;
	// In-world panels drawn by the core as emissive surfaces
	// (render_core_panels.h); null when no legacy backend is composed.
	IRenderCorePanels *panels = nullptr;
	// The renderer's compute service for the engine's indirect-light
	// producers (render/gpu_compute.h) on the core (RFC 0016 K12,
	// render.pass.indirect): it runs the SDF producer's program. Null when
	// the backend has no core frames to run it before. The ray-query
	// producer keeps the legacy backend's service (capabilities).
	gpu_compute::IGpuCompute *gpuCompute = nullptr;
	// For logs and evidence only (CAP011 rule 5): "null", "vulkan", "gl".
	const char *deviceName = nullptr;
	// Main-sequence mode selection through the owning core, without changing
	// IRenderCoreWorld's advertised vtable. Valid for this binding's lifetime.
	struct TemporalControl
	{
		void *context = nullptr;
		bool ( *setEnabled )( void *, bool ) = nullptr;
		bool ( *available )( const void * ) = nullptr;
	} temporal;
};

struct RenderCore;

extern "C" RenderCore *RenderCore_Create(
    const RenderCoreConfig *config, RenderCoreResult *result );
extern "C" void RenderCore_Destroy( RenderCore *core );
extern "C" const RenderCoreBinding *RenderCore_GetBinding( const RenderCore *core );
// The frontend's provider for MaterialSystem_BindShaderProvider; null when the
// config named no legacy backend.
extern "C" const render::LegacyShaderProvider *RenderCore_GetLegacyProvider(
    const RenderCore *core );
// How often the material system composed its backend through the frontend.
extern "C" unsigned int RenderCore_GetLegacyProviderCreates( const RenderCore *core );
// The material system's render call queue (MaterialSystem_RenderCallQueueHost),
// which the capabilities order their calls on; it must outlive the core.
extern "C" void RenderCore_BindRenderCallQueue(
    RenderCore *core, const render::legacy::RenderCallQueueHost *host );
// The adapter identity the material system's device facade reports for the
// backend composed through the frontend (RFC 0016 legacy device facade, F1);
// set before the material system creates the backend.
extern "C" void RenderCore_SetLegacyAdapterSource(
    RenderCore *core, const render::LegacyShaderServices::CoreAdapterSource *source );

#endif // RENDER_COMPOSITION_RENDER_CORE_H
