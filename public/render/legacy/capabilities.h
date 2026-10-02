//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy backend's optional capabilities, ordered by the
//			legacy frontend (RFC 0016 K3: the side channels leave the material
//			system).
//
//			The engine reaches the world mesh upload, the per-frame light set
//			sink and the compute service through the render core's binding
//			(RenderCoreBinding::capabilities), not through string lookups on
//			the material system. The frontend captures the backend's
//			capabilities when it creates the backend. World mesh and light set
//			calls come from the main thread while, under the queued material
//			system, the device belongs to the render thread: the frontend puts
//			each call, with a copy of every byte it borrows, on the material
//			system's render call queue through RenderCallQueueHost, so it
//			reaches the backend on the owning thread in the order the engine
//			made it. Without a queue it calls the backend directly.
//
//			A queued call cannot report the backend's result. A queued upload
//			returns true (accepted for the frame); a queued DrawBatch returns
//			whether the backend rejected the bound material the last time it ran
//			for it, so the engine's per-batch bookkeeping learns a rejection
//			one frame late.
//
//			ABI-facing: interfaces and C function pointers only.
//
//=============================================================================//

#ifndef RENDER_LEGACY_CAPABILITIES_H
#define RENDER_LEGACY_CAPABILITIES_H

#include "render/gpu_compute.h"
#include "render/light_set.h"
#include "render/world_mesh_upload.h"

class IMaterial;
class ITexture;

namespace render::legacy
{

// The material system's render call queue, as the frontend uses it.
struct RenderCallQueueHost
{
	// Whether the calling thread's render context queues its calls.
	bool ( *active )() = nullptr;
	// Queues call( payload ) on the calling thread's render call queue, in
	// order with the render-context calls queued before it. destroy( payload )
	// runs after the call, or when the queue is flushed unrun. False when
	// the context does not queue (active() is false): then nothing is queued,
	// destroy is not called, and the caller runs the call itself.
	bool ( *queue )( void ( *call )( void *payload ), void *payload,
	    void ( *destroy )( void *payload ) ) = nullptr;
	// The material the calling thread's render context has bound.
	IMaterial *( *boundMaterial )() = nullptr;
	// RFC 0016 K5: the shader API handle of a texture's first frame, and of a
	// lightmap page (render/legacy/core_passes.h ICoreTextures imports them);
	// 0 for none.
	int ( *textureHandle )( ITexture *texture ) = nullptr;
	int ( *lightmapPageHandle )( int page ) = nullptr;
	// Material-system owned, InitParams-initialized defaults. PrecacheVars
	// only: no neutral material uploads textures or enters the draw list.
	// Borrowed until the host shuts down after draining its render queue.
	IMaterial *( *neutralMaterial )( const char *shader ) = nullptr;
	// Lookup of an already prepared neutral's formatted value: never creates
	// a material or enters shader initialization (safe during draw capture).
	// Null when none was prepared; use CoreMeshVariable's declared default.
	// Temporary, copy before the next call.
	const char *( *materialDefault )( const char *shader, const char *key ) = nullptr;
};

class ILegacyCapabilities
{
public:
	// Null when the backend offers the capability not (or none was created).
	virtual world_mesh_gpu::IWorldMeshUpload *WorldMeshUpload() = 0;
	virtual light_set::ILightSetConsumer *LightSetConsumer() = 0;
	virtual gpu_compute::IGpuCompute *GpuCompute() = 0;

protected:
	~ILegacyCapabilities() = default;
};

} // namespace render::legacy

// The material system's render call queue (exported by the material system).
extern "C" const render::legacy::RenderCallQueueHost *MaterialSystem_RenderCallQueueHost();

#endif // RENDER_LEGACY_CAPABILITIES_H
