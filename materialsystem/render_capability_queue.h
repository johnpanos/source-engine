//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frame-ordered access to a render provider's optional capabilities
//          (world mesh upload and draw, the per-frame light set) under the
//          queued material system.
//
//          A provider's device belongs to the thread that owns it. With
//          mat_queue_mode 2 that is the material system's render thread,
//          while the engine calls these capabilities on the main thread, in
//          frame order among render-context calls that are themselves queued.
//          The adapters the material system hands out (QueryInterface) put
//          each call on the render call queue, with a copy of every byte it
//          borrows, so it reaches the provider on the owning thread in the
//          order the engine made it. Without a queue (single-threaded, or the
//          main thread holding the device through CMaterialSystem::Lock) they
//          call the provider directly.
//
//          A queued call cannot report the provider's result. A queued upload
//          returns true, meaning accepted for the frame; the provider reports
//          a rejection itself when the call runs. A queued DrawBatch returns
//          whether the provider rejected the bound material the last time it
//          ran for it (until then, accepted), so the engine's per-batch
//          bookkeeping learns a rejection one frame late.
//
//===========================================================================//

#ifndef RENDER_CAPABILITY_QUEUE_H
#define RENDER_CAPABILITY_QUEUE_H

#include "render/light_set.h"
#include "render/world_mesh_upload.h"

#include <functional>
#include <memory>
#include <mutex>
#include <set>

class CMatCallQueue;
class IMaterial;

// Where the adapters queue: the render call queue of the calling thread's
// current render context (null when calls go straight to the provider), and
// the material that context has bound.
struct RenderCapabilityQueueHost
{
	std::function<CMatCallQueue *()> renderCallQueue;
	std::function<IMaterial *()> boundMaterial;
};

class CQueuedWorldMeshUpload final : public world_mesh_gpu::IWorldMeshUpload
{
public:
	CQueuedWorldMeshUpload(
	    world_mesh_gpu::IWorldMeshUpload *provider, const RenderCapabilityQueueHost &host )
	    : m_provider( provider ), m_host( host )
	{
	}

	bool Upload( const world_mesh_gpu::WorldMeshUploadRequest &request ) override;
	bool UploadLightmap( const world_mesh_gpu::WorldLightmapUploadRequest &request ) override;
	bool UploadProbeVolume( const world_mesh_gpu::ProbeVolumeUploadRequest &request ) override;
	bool UploadShadowField( const world_mesh_gpu::ShadowFieldUploadRequest &request ) override;
	bool UploadReflectionProbes(
	    const world_mesh_gpu::ReflectionProbesUploadRequest &request ) override;
	bool DrawBatch( uint32_t firstIndex, uint32_t indexCount ) override;
	void Release() override;
	bool IsResident() const override { return m_provider->IsResident(); }

	// Runs on the render thread (queued DrawBatch).
	void ExecuteDrawBatch( IMaterial *material, uint32_t firstIndex, uint32_t indexCount );

private:
	void ForgetRejections();

	world_mesh_gpu::IWorldMeshUpload *m_provider;
	RenderCapabilityQueueHost m_host;
	// Materials whose queued batches the provider rejected, per map.
	std::mutex m_rejectedMutex;
	std::set<const IMaterial *> m_rejected;
};

class CQueuedLightSetConsumer final : public light_set::ILightSetConsumer
{
public:
	CQueuedLightSetConsumer(
	    light_set::ILightSetConsumer *provider, const RenderCapabilityQueueHost &host )
	    : m_provider( provider ), m_host( host )
	{
	}

	void PublishLightSet( const light_set::Snapshot &snapshot ) override;

private:
	light_set::ILightSetConsumer *m_provider;
	RenderCapabilityQueueHost m_host;
};

#endif // RENDER_CAPABILITY_QUEUE_H
