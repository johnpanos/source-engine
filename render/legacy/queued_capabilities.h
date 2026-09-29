//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frontend's frame-ordered capability adapters (RFC 0016 K3);
//			see public/render/legacy/capabilities.h.
//
//=============================================================================//

#ifndef RENDER_LEGACY_QUEUED_CAPABILITIES_H
#define RENDER_LEGACY_QUEUED_CAPABILITIES_H

#include "render/legacy/capabilities.h"
#include "render/legacy/core_passes.h"
#include "render/legacy_shader_provider.h"

#include <memory>

namespace render::legacy
{

class QueuedCapabilities final : public ILegacyCapabilities
{
public:
	QueuedCapabilities();
	~QueuedCapabilities();
	QueuedCapabilities( const QueuedCapabilities & ) = delete;
	QueuedCapabilities &operator=( const QueuedCapabilities & ) = delete;

	// The queue the adapters use; null calls the backend directly.
	void BindQueue( const RenderCallQueueHost *host );
	// The capabilities of a backend the frontend created (replacing any
	// earlier backend's).
	void Adopt( const LegacyShaderServices &services );

	world_mesh_gpu::IWorldMeshUpload *WorldMeshUpload() override;
	light_set::ILightSetConsumer *LightSetConsumer() override;
	gpu_compute::IGpuCompute *GpuCompute() override { return m_GpuCompute; }
	// The backend's core-pass slots in frame order (core_passes.h); null
	// when the backend has none.
	ICorePassSlots *CorePassSlots();

private:
	class WorldMesh;
	class LightSet;
	class CoreSlots;
	const RenderCallQueueHost *m_Host = nullptr;
	std::unique_ptr<WorldMesh> m_WorldMesh;
	std::unique_ptr<LightSet> m_LightSet;
	std::unique_ptr<CoreSlots> m_CoreSlots;
	gpu_compute::IGpuCompute *m_GpuCompute = nullptr;
};

} // namespace render::legacy

#endif // RENDER_LEGACY_QUEUED_CAPABILITIES_H
