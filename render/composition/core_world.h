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
#include "render/pass/ao/ao.h"
#include "render/pass/indirect/port_compute.h"
#include "render/pass/lights/map_lights.h"
#include "render/pass/shadows/shadow_passes.h"
#include "render/pass/shadows/shadow_plan.h"
#include "render/pass/world/world_pass.h"
#include "render/resources/mesh_cache.h"

#include <atomic>
#include <deque>
#include <cstddef>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
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
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount,
	    const char *entities ) override;
	world_mesh_gpu::IWorldMeshUpload *StageUpload() override { return &m_Capture; }
	void ClearWorld() override
	{
		m_StageSet = false;
		m_StageWorld.reset();
		m_Pass.ClearWorld();
	}
	bool Draws( unsigned int material ) const override { return m_Pass.Draws( material ); }
	light_set::ILightSetConsumer *StageLights() override { return &m_Lights; }
	bool TextureResident( ITexture *texture ) const override
	{
		return m_Host && m_Host->textureHandle && m_Host->textureHandle( texture ) != 0;
	}
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
	// The indirect-light producers' compute (render.pass.indirect), whose
	// device objects go with the backend's device.
	void BindCompute( pass::indirect::PortCompute *compute ) { m_Compute = compute; }
	void ReleaseDevice( device::IRenderDevice2 &device ) override
	{
		if ( m_Compute )
			m_Compute->ReleaseDevice( device );
		m_Pass.ReleaseDevice( device );
		m_Overlays.ReleaseDevice( device );
		m_Output.ReleaseDevice( device );
		ReleaseShadows( device );
	}

private:
	// Sets the pass's world stage from m_StageWorld and the captured lighting.
	void SetStage();
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
		// The latest change from the bake (empty: none) and its grid table,
		// which a stage set later starts from.
		std::vector<std::byte> change;
		std::optional<pass::world::StageProbeVolume> table;
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
			areas = snapshot.areas;
		}
		std::vector<light_set::RuntimeLight> lights;
		std::vector<light_set::RuntimeAreaLight> areas;
	};
	// The world stage's shadow atlas (per shadowed view of a frame): 4096
	// texels square, depth 32 (64 MB each) on the desktop profiles; a mobile
	// size is K12's per-profile atlas budget (not set yet).
	static constexpr std::uint32_t kStageShadowAtlas = 4096;
	// A world stage view's work at its slot: the shadow plan's depth views,
	// drawn into an atlas (none without shadowed lights), and the view and
	// projection its screen passes (GTAO) reconstruct positions with.
	struct ShadowWork
	{
		std::vector<pass::shadows::ShadowPlanView> views;
		std::uint32_t atlasSize = 0;
		std::uint32_t guardTexels = 0;
		math::float4x4 view;
		math::float4x4 projection;
		float eye[3] = {}; // the view's origin (the view matrix's inverse)
	};
	// The world stage's shadow casters (the stage mesh's positions and
	// indices; main thread writes at SetWorldMesh, the render sequence reads).
	struct Casters
	{
		std::vector<float> positions; // float3 per vertex
		std::vector<std::uint32_t> indices;
		std::uint64_t generation = 0;
	};
	// A world stage view's lights, clustered for it and planned into shadow
	// tiles; null without lights or matrices.
	std::shared_ptr<const pass::world::StageViewLights> StageViewLightsFor(
	    const float worldToView[16], const float viewToClip[16], const float viewport[6],
	    std::shared_ptr<const ShadowWork> *shadows ) const;
	// The view's area lights: the map's (baked light fixtures, a stage's
	// alone) then the frame's emitting surfaces, at most the surface
	// program's count. Each is packed with its tile from `areaTiles` (-1
	// past its end) and whether its diffuse light is in the bake.
	std::vector<area_light::AreaLight> ViewAreaLights( bool withMapAreas ) const;
	void PackViewAreaLights( const std::vector<area_light::AreaLight> &areas, std::size_t mapAreas,
	    const std::vector<int> &areaTiles, pass::world::StageViewLights &out ) const;
	// A view of a world without a stage (the claimed BSP faces of a retail
	// map): its frame area lights alone, unshadowed (the core holds no
	// casters for it); null when the frame has none.
	std::shared_ptr<const pass::world::StageViewLights> AreaViewLights() const;
	// Render sequence: draws a view's shadow work into an atlas of this
	// frame's pool, as a submission of its own ahead of the frame's; the atlas
	// in kSampled, or invalid when it could not be drawn.
	device::TextureId DrawStageShadows( device::IRenderDevice2 &device, const ShadowWork &work,
	    std::uint64_t frame, device::TextureDesc *desc );
	void ReleaseShadows( device::IRenderDevice2 &device );
	// Render sequence: the stage's ambient occlusion target for a target
	// size (kSampled; made and first transitioned in `encoder`).
	bool EnsureOcclusion( device::IRenderDevice2 &device, device::CommandEncoder &encoder,
	    std::uint32_t width, std::uint32_t height, device::CompletionToken submitted );
	// Render sequence: the stage's objects live on this device (a new one
	// drops the old one's handles, which went with it).
	void BindStageDevice( device::IRenderDevice2 &device );

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
	pass::indirect::PortCompute *m_Compute = nullptr;
	LightSink m_Lights;
	// The stage map's authored lights (its entity lump), and whether its
	// lightmap packs the sun's baked visibility (the total page's alpha).
	pass::lights::MapLights m_MapLights;
	bool m_StageSunMask = false;
	unsigned long long m_StageLitViews = 0; // main thread
	// Shadow work by world tag (DrawView writes, RecordSlot reads), and the
	// stage's casters.
	std::mutex m_ShadowLock;
	std::deque<std::pair<std::uint32_t, std::shared_ptr<const ShadowWork>>> m_ShadowWork;
	std::shared_ptr<const Casters> m_Casters;
	std::uint64_t m_CasterGeneration = 0;
	// Render sequence: the depth renderer, the casters' mesh and the atlases
	// (one per shadowed view of a frame, reused from frame to frame).
	device::IRenderDevice2 *m_ShadowDevice = nullptr;
	std::unique_ptr<pass::shadows::ShadowDepthRenderer> m_ShadowRenderer;
	std::unique_ptr<resources::MeshCache> m_CasterMeshes;
	std::uint64_t m_CastersStaged = 0;
	std::string m_CasterName;
	struct Atlas
	{
		device::TextureId texture;
		device::TextureDesc desc;
		device::ResourceUsage usage = device::ResourceUsage::kUndefined;
	};
	std::vector<Atlas> m_Atlases;
	std::size_t m_AtlasNext = 0;
	std::uint64_t m_AtlasFrame = 0;
	// The screen passes (render sequence): GTAO and its output.
	std::unique_ptr<pass::ao::AmbientOcclusion> m_Ao;
	device::TextureId m_Occlusion;
	device::TextureDesc m_OcclusionDesc;
	bool m_StageSet = false; // the pass holds a world stage
	// The stage's world without its lighting, kept to set the stage again
	// when the bake's probe volume arrives after it (main thread).
	std::shared_ptr<const pass::world::WorldData> m_StageWorld;
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_WORLD_H
