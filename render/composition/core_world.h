//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): IRenderCoreWorld for the
//			engine over render.pass.world, and the legacy frontend's
//			forwarded recorder that draws its slots. It translates the engine's
//			plain arrays and material system textures (handles through the
//			render call queue host) into the pass's data, marks each view's
//			slot through the frontend's frame-ordered slots, and hands the
//			backend's slot targets and textures to the pass. Private to the
//			composition.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_WORLD_H
#define RENDER_COMPOSITION_CORE_WORLD_H

#include "core_output.h"
#include "render/composition/render_core.h"
#include "render/legacy/core_backend.h"
#include "render/frame/renderer.h"
#include "render/legacy/core_passes.h"
#include "render/pass/debug/debug_overlays.h"
#include "render/pass/world/world_pass.h"

#include <atomic>
#include <cstddef>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace render::composition
{

class CoreWorld final : public IRenderCoreWorld,
                        public legacy::ICorePassRecorder,
                        public frame::IRenderStageHooks
{
public:
	CoreWorld( legacy::ILegacyFrontend &frontend, const frame::IRenderer &renderer )
	    : m_Frontend( frontend ), m_Renderer( renderer )
	{
	}

	void BindHost( const legacy::RenderCallQueueHost *host ) { m_Host = host; }

	// IRenderCoreWorld (the engine, main thread).
	void SetWorld( const RenderCoreWorldVertex *vertices, unsigned int vertexCount,
	    const unsigned int *indices, unsigned int indexCount,
	    const RenderCoreWorldSurface *surfaces, unsigned int surfaceCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) override;
	void SetWorldMesh( const void *wmsh, unsigned long long wmshBytes,
	    const RenderCoreWorldMeshlet *meshlets, unsigned int meshletCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) override;
	world_mesh_gpu::IWorldMeshUpload *StageUpload() override { return &m_Capture; }
	void ClearWorld() override
	{
		m_StageSet = false;
		m_Pass.ClearWorld();
	}
	bool Draws( unsigned int material ) const override { return m_Pass.Draws( material ); }
	light_set::ILightSetConsumer *StageLights() override { return &m_Lights; }
	bool DrawView( const unsigned int *surfaces, unsigned int count, const float worldToClip[16],
	    const float viewport[6], unsigned long long hostFrame, const float worldToView[16],
	    const float viewToClip[16] ) override;
	void BeginFrame() override;
	void EndFrame() override;
	// frame::IRenderStageHooks (the main thread): the open views' depth, so a
	// queued view knows whether it is the frame's top level (not a portal,
	// mirror or monitor view inside another).
	void OnStage( frame::Stage stage, std::uint32_t depth ) override;
	unsigned long long Failures() const override;
	void GetStats( RenderCoreWorldStats *out ) const override;

	// legacy::ICorePassRecorder (the backend, render sequence).
	std::uint32_t SlotStages() const override { return 0; }
	void RecordSlot( std::uint32_t tag, device::CommandEncoder &encoder,
	    const legacy::CorePassTarget &target ) override;
	bool RecordOutput( device::CommandEncoder &encoder,
	    const legacy::CoreOutputTargets &targets ) override
	{
		return m_Output.Record( encoder, targets );
	}
	void ReleaseDevice( device::IRenderDevice2 &device ) override
	{
		m_Pass.ReleaseDevice( device );
		m_Overlays.ReleaseDevice( device );
		m_Output.ReleaseDevice( device );
	}

private:
	std::vector<pass::world::WorldMaterial> WorldMaterials(
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) const;

	// The engine's world mesh uploads, kept for the world stage (main
	// thread): the lightmap's pages, the baked probe volume and the
	// reflection probes a stage starts from; once a stage is set, the
	// lightmap and the probe volume's change go to the pass as they come.
	class StageCapture final : public world_mesh_gpu::IWorldMeshUpload
	{
	public:
		explicit StageCapture( CoreWorld &owner ) : m_Owner( owner ) {}
		bool Upload( const world_mesh_gpu::WorldMeshUploadRequest & ) override { return true; }
		bool UploadLightmap( const world_mesh_gpu::WorldLightmapUploadRequest &request ) override;
		bool UploadProbeVolume( const world_mesh_gpu::ProbeVolumeUploadRequest &request ) override;
		bool UploadShadowField( const world_mesh_gpu::ShadowFieldUploadRequest & ) override
		{
			return true;
		}
		bool UploadReflectionProbes(
		    const world_mesh_gpu::ReflectionProbesUploadRequest &request ) override;
		bool DrawBatch( std::uint32_t, std::uint32_t ) override { return false; }
		void Release() override;
		bool IsResident() const override { return true; }

		pass::world::LightmapPages lightmap;                 // the total layer's pages
		std::vector<std::byte> indirect;                     // the indirect layer's flat page
		std::optional<pass::world::StageProbeVolume> probes; // the bake
		std::uint32_t reflectionWidth = 0;
		std::uint32_t reflectionHeight = 0;
		std::vector<std::byte> reflection;

	private:
		CoreWorld &m_Owner;
	};

	// The frame's light set, as the engine last published it (main thread).
	class LightSink final : public light_set::ILightSetConsumer
	{
	public:
		void PublishLightSet( const light_set::Snapshot &snapshot ) override
		{
			lights = snapshot.lights;
		}
		std::vector<light_set::RuntimeLight> lights;
	};
	// A world stage view's lights, clustered for it; null without lights or
	// matrices.
	std::shared_ptr<const pass::world::StageViewLights> StageViewLightsFor(
	    const float worldToView[16], const float viewToClip[16], const float viewport[6] ) const;

	legacy::ILegacyFrontend &m_Frontend;
	const frame::IRenderer &m_Renderer;
	const legacy::RenderCallQueueHost *m_Host = nullptr;
	pass::world::WorldPass m_Pass;
	pass::debug::DebugOverlays m_Overlays;
	// cl_render_debug_legacy 1: the tags of top-level views (main thread
	// writes, render sequence reads), and per recorded frame serial the
	// top-level world slots it recorded, with their color targets (render
	// sequence). A frame recorded again for a capture records its slots
	// again, under the same or a higher serial, so nothing is consumed.
	std::uint32_t m_ViewDepth = 0;
	std::mutex m_TopLevelLock;
	std::set<std::uint32_t> m_TopLevel;
	std::map<std::uint64_t, std::vector<std::pair<std::uint32_t, device::TextureId>>> m_FrameViews;
	std::atomic<unsigned long long> m_Hatches{ 0 };
	std::atomic<unsigned long long> m_Tints{ 0 };
	std::atomic<unsigned long long> m_Redrawn{ 0 };
	CoreOutput m_Output;
	StageCapture m_Capture{ *this };
	LightSink m_Lights;
	unsigned long long m_StageLitViews = 0; // main thread
	bool m_StageSet = false; // the pass holds a world stage
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_WORLD_H
