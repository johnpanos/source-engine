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
#include "core_temporal.h"
#include "mapcontainer/light_shadow_masks.h"
#include "mdl/studio_model.h"
#include "render/composition/render_core.h"
#include "render/legacy/core_backend.h"
#include "render/frame/renderer.h"
#include "render/graph/pass_timers.h"
#include "render/graph/scene_color.h"
#include "render/legacy/core_passes.h"
#include "render/pass/debug/debug_overlays.h"
#include "render/map_media/map_media.h"
#include "render/map_media/projector_cookies.h"
#include "render/pass/ao/ao.h"
#include "render/pass/output/output.h"
#include "render/pass/ssr/ssr.h"
#include "render/pass/volumetric/volumetric.h"
#include "render/pass/indirect/port_compute.h"
#include "render/pass/lights/map_lights.h"
#include "render/pass/lights/cluster_pass.h"
#include "render/resources/mip_feedback.h"
#include "render/pass/shadows/shadow_passes.h"
#include "render/pass/shadows/shadow_plan.h"
#include "render/pass/skinning/skinning.h"
#include "render/pass/world/world_pass.h"
#include "render/resources/mesh_cache.h"

#include <atomic>
#include <deque>
#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace render::composition
{

// The map media and cookie owner (render.map-media).
using map_media::CookieArray;
using map_media::CookieImages;
using map_media::DecodeCookies;
using map_media::FroxelLayoutOf;
using map_media::kFroxelFarZ;
using map_media::kFroxelNearZ;
using map_media::kFroxelSampling;
using map_media::kFroxelSliceMultiplier;
using map_media::kFroxelTileDivisor;
using map_media::MapMedia;
using map_media::MediaFromEntities;
using map_media::MediumLightsFrom;

class CoreWorld final : public IRenderCoreWorld,
                        public legacy::ICorePassRecorder,
                        public frame::IRenderStageHooks,
                        public pass::world::IWorldSceneColorCapture
{
public:
	~CoreWorld() { SavePipelineKeys(); }
	CoreWorld( legacy::ILegacyFrontend &frontend, const frame::IRenderer &renderer )
	    : m_Frontend( frontend ), m_Renderer( renderer )
	{
	}

	void BindHost( const legacy::RenderCallQueueHost *host ) { m_Host = host; }
	void EnableTemporal( bool enabled, const char *assets )
	{
		m_TemporalAvailable = enabled;
		m_TemporalAssets = assets ? assets : "";
	}
	bool TemporalAvailable() const { return m_TemporalAvailable; }
	bool SetTemporalEnabled( bool enabled )
	{
		if ( enabled && !m_TemporalAvailable )
			return false;
		if ( m_TemporalEnabled != enabled )
		{
			ResetTemporalHistory();
			m_TemporalEnabled = enabled;
		}
		return true;
	}

	// IRenderCoreWorld (the engine, main thread).
	void SetWorld( const RenderCoreWorldVertex *vertices, unsigned int vertexCount,
	    const unsigned int *indices, unsigned int indexCount,
	    const RenderCoreWorldSurface *surfaces, unsigned int surfaceCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) override;
	void SetWorldMesh( const void *wmsh, unsigned long long wmshBytes,
	    const RenderCoreWorldMeshlet *meshlets, unsigned int meshletCount,
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount,
	    const char *entities ) override;
	void SetStaticProps( const RenderCoreStaticModel *models, unsigned int modelCount,
	    const RenderCoreStaticProp *props, unsigned int propCount ) override;
	bool DrawsStaticProp( unsigned int prop, unsigned int lod = 0 ) const override;
	world_mesh_gpu::IWorldMeshUpload *StageUpload() override { return &m_Capture; }
	void ClearWorld() override
	{
		ResetTemporalHistory();
		m_QueuedLighting.clear();
		m_StageSet = false;
		m_StageWorld.reset();
		m_WorldCasters.reset();
		{
			std::lock_guard<std::mutex> guard( m_ShadowLock );
			m_Casters.reset();
		}
		m_StaticMeshes.clear();
		m_StaticInstances.clear();
		m_StaticCastsShadow.clear();
		m_StaticMaterials.clear();
		m_ModelPoseSources.clear();
		m_ModelBytes.clear();
		m_Pass.SetModelLevelSource( nullptr );
		SavePipelineKeys();
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
	    const float viewToClip[16], float waterZOffset, const RenderCoreStaticPropDraw *staticProps,
	    unsigned int staticPropCount, const RenderCorePosedModel *posedModels,
	    unsigned int posedModelCount ) override;
	void BeginFrame() override;
	void EndFrame() override;
	void SelectTemporalView( unsigned long long identity ) override { m_TemporalView = identity; }
	void ResetTemporalHistory() override;
	bool CaptureTemporalInputs( const char *prefix, bool afterReset ) override;
	void CommitTemporalFrame( bool submitted ) override;
	bool TemporalEnabled() const override { return m_TemporalEnabled; }
	void TemporalJitter( float *x, float *y ) const override
	{
		*x = m_JitterX;
		*y = m_JitterY;
	}
	bool ReconstructTemporal( int x, int y, int rw, int rh, int ow, int oh, float dt ) override;
	// frame::IRenderStageHooks (the main thread): the open views' depth, so a
	// queued view knows whether it is the frame's top level (not a portal,
	// mirror or monitor view inside another).
	void OnStage( frame::Stage stage, std::uint32_t depth ) override;
	unsigned long long Failures() const override;
	void GetStats( RenderCoreWorldStats *out ) const override;
	void SetGpuTimers( bool enabled ) override;
	void SetQuality( const RenderCoreWorldQuality &quality ) override;
	void SetFileSource( const RenderCoreFileSource &source ) override { m_Files = source; }
	void SetPipelineStore( const char *directory ) override;
	unsigned int TakeGpuTimes( char *out, unsigned int size ) override;
	void ReadCosts( RenderCoreCostReport *out ) override;

	// legacy::ICorePassRecorder (the backend, render sequence).
	std::uint32_t SlotStages() const override;
	bool AcceptsMeshes() const override { return m_DynamicDraws.load( std::memory_order_relaxed ); }
	IMaterial *NeutralMaterial( const char *shader ) override
	{
		return m_Host && m_Host->neutralMaterial ? m_Host->neutralMaterial( shader ) : nullptr;
	}

	std::uint32_t QueueMesh( const legacy::CoreMeshDraw &draw ) override;

	// The screen UI (RFC 0016 K8 UI cohort, render.ui-draw-list.v1), main
	// thread: the list as a view of dynamic draws of its materials, queued;
	// its tag, or 0 with why. With no level the pass holds an empty world
	// for it (a level's SetWorld replaces it).
	std::uint32_t QueueUiList( const ui_draw_list::ListView &list,
	    const RenderCoreWorldMaterial *materials, std::string &why );
	// Why a UI command of this material would be refused, or nullopt.
	std::optional<std::string> ClaimUiMaterial( const RenderCoreWorldMaterial &material );

	std::size_t RecordOpaqueBatch( std::span<const std::uint32_t> tags,
	    device::CommandEncoder &encoder, const legacy::CorePassTarget &target ) override;
	void RecordSlot( std::uint32_t tag, device::CommandEncoder &encoder,
	    const legacy::CorePassTarget &target ) override;
	std::optional<pass::world::WorldSceneColor> Capture( device::IRenderDevice2 &device,
	    device::CommandEncoder &encoder, device::TextureId source,
	    const device::TextureDesc &sourceDesc, std::uint64_t frame ) override;
	bool RecordOutput(
	    device::CommandEncoder &encoder, const legacy::CoreOutputTargets &targets ) override
	{
		return m_Output.Record( encoder, targets );
	}
	// The indirect-light producers' compute (render.pass.indirect), whose
	// device objects go with the backend's device.
	void BindCompute( pass::indirect::PortCompute *compute ) { m_Compute = compute; }
	void FrameSubmitted( device::CompletionToken token, bool submitted ) override
	{
		if ( m_Temporal )
			m_Temporal->CaptureSubmitted( token, submitted );
	}
	void ReleaseDevice( device::IRenderDevice2 &device ) override
	{
		if ( m_Compute )
			m_Compute->ReleaseDevice( device );
		m_Pass.ReleaseDevice( device );
		for ( auto &[frame, capture] : m_SceneCaptures )
			capture.Release( device, device::CompletionToken() );
		m_SceneCaptures.clear();
		m_Overlays.ReleaseDevice( device );
		m_Output.ReleaseDevice( device );
		m_Temporal.reset();
		for ( auto &[id, motion] : m_MotionTargets )
		{
			(void)device.Release( device::ResourceId( motion.image ), device::CompletionToken() );
			(void)device.Release( device::ResourceId( motion.depth ), device::CompletionToken() );
		}
		m_MotionTargets.clear();
		ReleaseShadows( device );
	}

private:
	void RecordWorldBatch( std::span<const std::uint32_t> tags, device::CommandEncoder &encoder,
	    const legacy::CorePassTarget &target );
	bool m_TemporalAvailable = false;
	bool m_TemporalEnabled = false; // main-sequence selection; queued views retain their choice
	std::string m_TemporalAssets;
	std::uint64_t m_TemporalGeneration = 1;
	std::uint32_t m_JitterSequence = 0;
	float m_JitterX = 0, m_JitterY = 0;
	std::uint32_t m_TemporalSerial = 0;
	std::mutex m_TemporalLock;
	std::map<std::uint32_t, TemporalRequest> m_TemporalRequests;
	std::string m_TemporalCapturePrefix;
	bool m_TemporalCaptureAfterReset = false;
	std::uint64_t m_TemporalCaptureGeneration = 0;
	// The host may replay one stream for a capture or swapchain recreation.
	std::map<std::uint32_t, TemporalRequest> m_RecordedTemporalRequests;
	std::uint64_t m_TemporalStream = 0;
	std::unique_ptr<CoreTemporal> m_Temporal;
	struct MotionTarget
	{
		device::TextureId image, depth;
		std::uint32_t width = 0, height = 0;
		std::uint64_t frame = 0, stream = 0;
	};
	std::map<std::uint64_t, MotionTarget> m_MotionTargets;
	// Render-sequence diagnostic state, bounded to 128 views of one recording.
	std::map<std::uint64_t, device::Viewport> m_TemporalRecordedViewports;
	std::uint64_t m_TemporalViewportFrame = 0, m_TemporalViewportStream = 0;

	std::uint64_t m_TemporalView = 0;
	struct MotionCamera
	{
		std::array<float, 16> toClip{};
		device::Viewport viewport;
	};
	struct MotionPose
	{
		std::uint32_t model, lod;
		int body;
		std::vector<material::SurfaceModelVertex> vertices;
		std::uint32_t skin = 0;
	};
	std::map<std::uint64_t, MotionCamera> m_PreviousCameras, m_PendingCameras;
	std::map<std::pair<std::uint64_t, std::uint64_t>, MotionPose> m_PreviousPoses, m_PendingPoses;

	// Sets the pass's world stage from m_StageWorld and the captured lighting.
	void SetStage();
	// Rebuilds the static shadow mesh from the stage and the core-claimed props.
	void SetStaticCasters();
	std::vector<pass::world::WorldData::StaticMesh> m_StaticMeshes;
	// Rises whenever the model geometry changes (SetStaticProps), so a world
	// republished with the same revision keeps its resident model levels.
	std::uint64_t m_ModelsRevision = 0;
	std::vector<pass::world::WorldData::StaticInstance> m_StaticInstances;
	std::vector<bool> m_StaticCastsShadow;
	std::vector<pass::world::WorldMaterial> m_StaticMaterials;
	struct ModelPoseSource
	{
		bool parsed = false;
		// Each hardware level's skinning source, in the same vertex order as
		// that level's WorldData::StaticMeshLod block. Empty for a model the
		// host declared static-only: nothing ever skins it.
		std::vector<std::vector<pass::skinning::SkinVertex>> vertices;
		std::vector<pass::skinning::BoneMatrix> poseToBone;
		std::vector<mdl::BodyPart> bodyParts;
		std::uint32_t lodCount = 0;
		// The per-frame source of one level's posed vertices; empty when the
		// model has no pose source.
		std::span<const pass::skinning::SkinVertex> LevelVertices( std::uint32_t lod ) const
		{
			return lod < vertices.size()
			           ? std::span<const pass::skinning::SkinVertex>( vertices[lod] )
			           : std::span<const pass::skinning::SkinVertex>();
		}
		struct SurfaceVariant
		{
			std::uint32_t part, model, lod;
		};
		std::vector<SurfaceVariant> surfaceBodies;
		std::vector<std::uint32_t> SelectedSurfaces( int body, std::uint32_t lod = 0 ) const
		{
			std::vector<std::uint32_t> selected;
			for ( std::uint32_t i = 0; i < surfaceBodies.size(); ++i )
			{
				const auto &variant = surfaceBodies[i];
				if ( variant.lod == lod &&
				     bodyParts[variant.part].SelectedModel( body ) == variant.model )
					selected.push_back( i );
			}
			return selected;
		}
	};
	std::vector<ModelPoseSource> m_ModelPoseSources;
	// A posed model's topology at one level and body: the selected surfaces,
	// the vertices they use with their skinning source, and the bones those
	// weight. A pure function of the model's data, built on first use and
	// kept until the models change (m_ModelsRevision); PoseModel had walked
	// every index of the level each frame.
	struct PoseTopology
	{
		bool valid = false; // false: a vertex names a bone the pose lacks
		std::vector<std::uint32_t> surfaces;
		std::vector<pass::skinning::SkinVertex> active;
		std::vector<std::uint32_t> activeIndices;
		std::vector<std::uint32_t> bones;
	};
	std::shared_ptr<const PoseTopology> PoseTopologyFor(
	    std::uint32_t model, std::uint32_t lod, int body ) const;
	mutable std::mutex m_PoseTopologyLock;
	mutable std::uint64_t m_PoseTopologyRevision = 0;
	mutable std::unordered_map<std::uint64_t, std::shared_ptr<const PoseTopology>> m_PoseTopology;
	bool PoseModel(
	    const RenderCorePosedModel &source, pass::world::WorldView::PosedModel &out ) const;
	std::vector<pass::world::WorldMaterial> WorldMaterials(
	    const RenderCoreWorldMaterial *materials, unsigned int materialCount ) const;
	// A UI material: as the world's, with its base texture at its $frame.
	pass::world::WorldMaterial UiMaterial( const RenderCoreWorldMaterial &material ) const;
	// Raw MDL/VVD/VTX bytes per model, kept for zero-staging resupply.
	struct ModelBytes
	{
		std::string mdl, vvd, vtx;
		unsigned int materialCount = 0;
		unsigned int materialLodCount = 0;
	};
	std::vector<ModelBytes> m_ModelBytes;
	class ModelLevelSource final : public pass::world::IModelLevelSource
	{
	public:
		explicit ModelLevelSource( CoreWorld &owner ) : m_Owner( owner ) {}
		std::optional<LevelGeometry> ResupplyLevel(
		    std::uint32_t mesh, std::uint32_t lod ) override;
		void PrefetchLevel( std::uint32_t mesh, std::uint32_t lod ) override;

	private:
		CoreWorld &m_Owner;
	};
	ModelLevelSource m_ModelLevelSource{ *this };

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

		pass::world::LightmapPages lightmap; // the total layer's pages
		pass::world::LightmapPages indirect; // the indirect layer's pages (flat empty: none)
		// The LMAP v3 lump the pages are blocks of (empty once they are
		// decoded for a region update), and whether it carries a sun mask.
		std::vector<std::byte> lmap;
		bool sunMask = false;
		// The static lights' baked shadow masks (LSMK; flat empty: none) and
		// their lights (origin and id).
		pass::world::LightmapPages shadowMask;
		std::shared_ptr<const std::vector<mapcontainer::LightShadowMaskRecord>> maskLights;
		// Block pages become RGBA16F pages (a region update needs texels).
		bool DecodePages();
		std::optional<pass::world::StageProbeVolume> probes; // the bake
		// The latest change from the bake (empty: none) and its grid table,
		// which a stage set later starts from.
		std::vector<std::byte> change;
		std::optional<pass::world::StageProbeVolume> table;
		// The map's RPRB v8 probes: the probe buffer and the cube data as stored.
		std::optional<pass::world::StageReflectionProbes> reflection;

	private:
		CoreWorld &m_Owner;
	};

	// The frame's light set, as the engine last published it (main thread).
	class LightSink final : public light_set::ILightSetConsumer
	{
	public:
		void PublishLightSet( const light_set::Snapshot &snapshot ) override
		{
			++revision;
			lights = snapshot.lights;
			areas = snapshot.areas;
			projected = snapshot.projected;
			occluders = snapshot.occluders;
			triangles = snapshot.coreTriangles;
		}
		std::uint64_t revision = 0; // each publication invalidates queued-view reuse
		std::vector<light_set::RuntimeLight> lights;
		std::vector<light_set::RuntimeAreaLight> areas;
		std::vector<light_set::RuntimeProjectedLight> projected;
		std::vector<dynamic_occlusion::TriangleOccluder> triangles;
		std::vector<light_set::RuntimeOccluder> occluders; // the moving objects
	};
	// The world stage's shadow atlas (per shadowed view of a frame): 4096
	// texels square, depth 32 (64 MB each) on the desktop profiles; a mobile
	// size is K12's per-profile atlas budget (not set yet).
	// The shadow atlas's texels by r_core_shadow_quality (0: unshadowed).
	static std::uint32_t ShadowAtlasFor( int quality )
	{
		return quality <= 0 ? 0u : quality == 1 ? 2048u : quality == 2 ? 4096u : 8192u;
	}
	// RenderCoreWorldQuality, set on the main thread, read where views are
	// queued and recorded.
	bool m_CoreOnly = false; // main thread, published by the frame slot
	std::atomic<int> m_AoQuality{ 3 };
	std::atomic<int> m_ShadowQuality{ 3 };
	std::atomic<bool> m_DepthPrepass{ true };
	std::atomic<int> m_GpuSubmission{ 0 }; // 0 off, 1 GPU culling, 2 with occlusion
	std::atomic<bool> m_ShadowMovers{ true };
	std::atomic<bool> m_ShadowPcss{ false };
	std::atomic<bool> m_AreaLightsOn{ false };
	std::atomic<bool> m_ProbeBounce{ false };
	// Runtime direct light on the next stage (RenderCoreWorldQuality), and
	// on the stage set.
	std::atomic<bool> m_RuntimeDirect{ true };
	std::atomic<bool> m_StageRuntimeDirect{ false };
	std::atomic<bool> m_StageHasIndirect{ false }; // the stage carries the indirect layer
	// A world stage view's work at its slot: the shadow plan's depth views,
	// drawn into an atlas (none without shadowed lights), and the view and
	// projection its screen passes (GTAO) reconstruct positions with.
	struct ShadowWork
	{
		std::vector<pass::shadows::ShadowPlanView> views;
		int sunFirst = -1;
		int sunCount = 0;
		std::uint32_t atlasSize = 0;
		std::uint32_t guardTexels = 0;
		math::float4x4 view;
		math::float4x4 projection;
		float eye[3] = {}; // the view's origin (the view matrix's inverse)
		// The frame's moving objects' boxes (with their pose versions):
		// casters drawn over the cached static tiles.
		std::vector<dynamic_occlusion::TriangleOccluder> triangles;
		std::vector<light_set::RuntimeOccluder> movers;
	};
	// The world stage's shadow casters (the stage mesh's positions and
	// indices; main thread writes at SetWorldMesh, the render sequence reads).
	struct Casters
	{
		std::vector<float> positions; // float3 per vertex
		// Grouped into spatial chunks, so each shadow view draws only the
		// chunks inside its frustum.
		std::vector<std::uint32_t> indices;
		struct Chunk
		{
			std::uint32_t firstIndex = 0;
			std::uint32_t indexCount = 0;
			float min[3] = {};
			float max[3] = {};
		};
		std::vector<Chunk> chunks;
		std::uint64_t generation = 0;
	};
	std::shared_ptr<const Casters> m_WorldCasters;
	// What a world stage view's lights are made from, taken when the main
	// thread queues it (the frame's lights change there), so the render
	// sequence clusters them and plans their shadows when it records the view.
	struct ViewLightInputs
	{
		bool stageWorld = false;
		float worldToView[16] = {};
		float viewToClip[16] = {};
		float viewport[6] = {};
		std::vector<light_set::RuntimeLight> lights; // the frame's, or the map's
		std::vector<area_light::AreaLight> areas;    // the map's then the frame's
		std::size_t mapAreas = 0;
		std::optional<pass::lights::MapSun> sun;
		bool sunMask = false;
		// The stage's baked shadow mask lights, or null.
		std::shared_ptr<const std::vector<mapcontainer::LightShadowMaskRecord>> maskLights;
		int shadowQuality = 0;
		std::vector<dynamic_occlusion::TriangleOccluder> triangles;
		std::vector<light_set::RuntimeOccluder> movers; // the frame's moving objects
		// The frame's projected lights with a cookie, each with its layer of
		// `cookies` (the frame's cookie set, decoded on the main thread).
		std::vector<projected_light::Light> projectors;
		std::vector<int> cookieLayers;
		std::shared_ptr<const CookieImages> cookies;
	};
	// A queued stage view keeps its CPU input snapshot across record-stream
	// replays. GPU light lists belong to one device submission frame: the
	// cluster kernel retires their buffers when the next frame starts.
	struct PendingView : pass::world::StageLightingInputs
	{
		struct FrameLighting
		{
			std::once_flag made;
			std::shared_ptr<const pass::world::StageViewLights> lights;
			std::shared_ptr<const ShadowWork> shadows;
			device::TextureId shadowAtlas;
			device::TextureDesc shadowAtlasDesc;
			// The view's medium is composited once per record frame.
			std::atomic<bool> fogged{ false };
			// The cutout casters are drawn into shadowAtlas once.
			std::atomic<bool> cutoutsDrawn{ false };
		};

		ViewLightInputs inputs;
		mutable std::mutex frameLock;
		mutable std::uint64_t recordFrame = 0;
		mutable std::shared_ptr<FrameLighting> frameLighting;

		std::shared_ptr<FrameLighting> ForFrame( std::uint64_t frame ) const
		{
			std::lock_guard<std::mutex> guard( frameLock );
			if ( !frameLighting || recordFrame != frame )
			{
				recordFrame = frame;
				frameLighting = std::make_shared<FrameLighting>();
			}
			return frameLighting;
		}
	};
	// Main-thread cache; shared PendingViews own immutable input snapshots.
	// The per-record-frame call_once publishes one GPU list to every same-view
	// cohort. Never reuse inputs across host frames or light publications.
	struct QueuedLighting
	{
		std::uint64_t frame = 0;
		std::uint64_t revision = 0;
		bool movers = false;
		std::shared_ptr<PendingView> pending;
	};
	std::vector<QueuedLighting> m_QueuedLighting;
	// A queued view's inputs, taken on the main thread.
	ViewLightInputs TakeViewLightInputs(
	    const float worldToView[16], const float viewToClip[16], const float viewport[6] ) const;
	// A world stage view's lights, clustered for it and planned into shadow
	// tiles; null without lights or matrices. Recorded on the render sequence
	// with the view consumer encoder.
	std::unique_ptr<pass::lights::ClusterKernel> m_ClusterKernel;
	std::uint64_t m_ClusterFrame = 0;
	std::atomic<std::uint64_t> m_LightingFailures{ 0 };
	std::shared_ptr<const pass::world::StageViewLights> StageViewLightsFor(
	    const ViewLightInputs &inputs, std::shared_ptr<const ShadowWork> *shadows,
	    device::CommandEncoder &encoder );
	// The view's area lights: the map's (baked light fixtures, a stage's
	// alone) then the frame's emitting surfaces, at most the surface
	// program's count. Each is packed with its tile from `areaTiles` (-1
	// past its end) and whether its diffuse light is in the bake.
	std::vector<area_light::AreaLight> ViewAreaLights( bool withMapAreas ) const;
	static void PackViewAreaLights( const std::vector<area_light::AreaLight> &areas,
	    std::size_t mapAreas, const std::vector<int> &areaTiles,
	    pass::world::StageViewLights &out );
	// A view of a world without a stage (the claimed BSP faces of a retail
	// map): its frame area lights alone, unshadowed (the core holds no
	// casters for it); null when the frame has none.
	void SetWorldCasters(
	    const pass::world::WorldData &data, std::span<const unsigned char> opaqueTriangles );
	// Render sequence: draws a view's shadow work into an atlas of this
	// frame's pool, as a submission of its own ahead of the frame's; the atlas
	// in kSampled, or invalid when it could not be drawn.
	device::TextureId DrawStageShadows( device::IRenderDevice2 &device, const ShadowWork &work,
	    device::CompletionToken submitted, std::uint64_t frame, device::TextureDesc *desc );
	void ReleaseShadows( device::IRenderDevice2 &device );
	// Render sequence: the stage's ambient occlusion target for a target
	// size (kSampled; made and first transitioned in `encoder`).
	bool EnsureOcclusion( device::IRenderDevice2 &device, device::CommandEncoder &encoder,
	    std::uint32_t width, std::uint32_t height, device::CompletionToken submitted );
	// Render sequence: the stage's objects live on this device (a new one
	// drops the old one's handles, which went with it).
	void BindStageDevice( device::IRenderDevice2 &device );
	// RFC 0014 D4: the timers of the slot's frame while they are on (render
	// sequence), else null.
	graph::GpuPassTimers *SlotTimers( const legacy::CorePassTarget &target );

	legacy::ILegacyFrontend &m_Frontend;
	const frame::IRenderer &m_Renderer;
	const legacy::RenderCallQueueHost *m_Host = nullptr;
	// The host's neutral default of a shader's variable (materialDefault),
	// kept once found: a found answer comes from a published neutral
	// material, whose variables never change. A missing one is asked again
	// (the neutral material may be published later). Looked up by
	// string_view, so a capture's per-variable lookup allocates nothing.
	const char *MaterialDefault( const char *shader, const char *key );
	struct ViewHash
	{
		using is_transparent = void;
		std::size_t operator()( std::string_view value ) const noexcept
		{
			return std::hash<std::string_view>{}( value );
		}
	};
	using DefaultsByKey = std::unordered_map<std::string, std::string, ViewHash, std::equal_to<>>;
	std::mutex m_DefaultsLock;
	std::unordered_map<std::string, DefaultsByKey, ViewHash, std::equal_to<>> m_MaterialDefaults;
	// The same answers by the capture's string addresses (symbol-table names
	// and shader names, stable for the process), each checked against its
	// text: an address that now holds other text takes the lookup above.
	struct DefaultByAddress
	{
		const std::string *shader = nullptr; // keys of m_MaterialDefaults and
		const std::string *key = nullptr;    // its DefaultsByKey: stable nodes
		const std::string *value = nullptr;
	};
	struct AddressPairHash
	{
		std::size_t operator()( const std::pair<const char *, const char *> &p ) const noexcept
		{
			return std::hash<const void *>{}( p.first ) * 31u ^
			       std::hash<const void *>{}( p.second );
		}
	};
	std::unordered_map<std::pair<const char *, const char *>, DefaultByAddress, AddressPairHash>
	    m_DefaultsByAddress;
	pass::world::WorldPass m_Pass;
	std::vector<std::pair<std::uint64_t, graph::InlineGraphResources>> m_SceneCaptures;
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
	// CPU mip demand aggregates all world views recorded for one host frame;
	// bounded replay history lets capture re-records reuse the same collector.
	std::map<std::uint64_t, std::unique_ptr<resources::MipFeedbackFrame>> m_MipFeedbackFrames;
	// Collection is off until a residency/streaming consumer reads
	// MipFeedbackFrame::Requests(); projecting footprints with no reader cost
	// ~12 ms of CPU per frame on sp_a1_intro4 (2026-10-05).
	bool m_MipFeedbackConsumer = false;
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
	// The stage map's participating media (its entity lump; render
	// sequence reads it only between SetWorldMesh calls, as m_MapLights).
	std::shared_ptr<const MapMedia> m_Media;
	// The stage's alpha-tested surfaces (WorldData::surfaces indices), left
	// out of the position-only casters and drawn by the world pass with
	// their materials' coverage (WorldTarget::cutoutShadows).
	std::vector<std::uint32_t> m_CutoutSurfaces;
	// The static props' alpha-tested surfaces (instance, surface), from
	// SetStaticCasters (main thread, at map load).
	std::vector<std::pair<std::uint32_t, std::uint32_t>> m_CutoutStaticSurfaces;
	// The game's files and the frame's projector cookies (main thread):
	// decoded again only when the set of cookie names changes. A set that
	// does not decode leaves the frame's projectors out, by name.
	RenderCoreFileSource m_Files;
	// The pipeline prewarm list (SetPipelineStore): its file, and the keys
	// read from it or saved to it. Main thread.
	void SavePipelineKeys();
	std::string m_PipelineKeysPath;
	std::set<std::string> m_PipelineKeys;
	std::vector<std::string> m_CookieNames;
	std::shared_ptr<const CookieImages> m_CookieImages;
	std::string m_CookieRefusal;
	unsigned int m_ProjectorsLit = 0;
	std::atomic<unsigned long long> m_ProjectorsRefused{ 0 };
	void RefreshCookies();
	// The uploaded cookie array (render sequence), of the images it holds.
	std::unique_ptr<CookieArray> m_Cookies;
	std::shared_ptr<const CookieImages> m_CookiesUploaded;
	bool m_StageSunMask = false;
	// The stage's baked shadow mask lights (LSMK), or null.
	std::shared_ptr<const std::vector<mapcontainer::LightShadowMaskRecord>> m_StageMaskLights;
	std::atomic<unsigned long long> m_StageLightingBuilds{ 0 };
	// Render sequence: views that read the frame's stage lighting instead of
	// planning their own lights, shadow atlas and occlusion.
	std::atomic<unsigned long long> m_SharedStageViews{ 0 };
	unsigned long long m_StageLitViews = 0; // main thread
	// Stream-view metadata and the stage's casters.
	std::mutex m_ShadowLock;
	// The material QueueMesh built for each nonzero
	// CoreMeshDraw::materialRevision (bounded; equal revisions, equal content).
	std::mutex m_MaterialLock;
	std::unordered_map<std::uint64_t, pass::world::WorldMaterial> m_RevisionMaterials;
	struct StreamView
	{
		std::array<float, 16> view;
		std::array<float, 16> projection;
		std::uint64_t recordedStream = 0;
	};
	std::map<std::uint32_t, StreamView> m_StreamViews; // under m_ShadowLock
	// Render sequence only: lighting from the most recent recorded scene view.
	StreamView m_StreamLightingView;
	std::uint64_t m_StreamLightingFrame = 0;
	pass::world::WorldTarget m_StreamLighting;

	std::shared_ptr<const Casters> m_Casters;
	std::uint64_t m_CasterGeneration = 0;
	// Render sequence: the depth renderer, the casters' mesh and the atlases
	// (one per shadowed view of a frame, reused from frame to frame).
	device::IRenderDevice2 *m_ShadowDevice = nullptr;
	std::unique_ptr<pass::shadows::ShadowDepthRenderer> m_ShadowRenderer;
	std::unique_ptr<resources::MeshCache> m_CasterMeshes;
	std::map<std::string, std::uint64_t> m_TriangleRevisions;
	std::uint64_t m_CastersStaged = 0;
	std::string m_CasterName;
	// An atlas keeps its tiles across frames (static casters): a frame
	// redraws only the views not already drawn into it as they are now.
	struct Atlas
	{
		device::TextureId texture;
		device::TextureDesc desc;
		device::ResourceUsage usage = device::ResourceUsage::kUndefined;
		std::vector<pass::shadows::ShadowPlanView> drawn; // what its tiles hold
		int sunFirst = -1;
		int sunCount = 0;
		std::uint64_t generation = 0;                     // of the casters drawn
		std::uint32_t guardTexels = 0;
		// The frame's atlas while movers cast: the static tiles restored,
		// the movers drawn over them. What its tiles were restored for, and
		// the tiles the movers were drawn in (restored again next frame).
		device::TextureId composite;
		device::ResourceUsage compositeUsage = device::ResourceUsage::kUndefined;
		std::vector<pass::shadows::ShadowPlanView> compositeHeld;
		std::uint64_t compositeGeneration = 0;
		std::uint32_t compositeGuardTexels = 0;
		// Each tile's movers as drawn in the frame's atlas (a hash of their
		// entities, parts and pose versions; 0: none): a tile is drawn again
		// only when they change.
		std::vector<std::pair<pass::shadows::ShadowTile, std::uint64_t>> moverTiles;
	};
	// Shadow tiles drawn and kept (RFC 0014 D4's report, render sequence).
	std::atomic<std::uint64_t> m_ShadowTilesDrawn{ 0 };
	std::atomic<std::uint64_t> m_ShadowTilesKept{ 0 };
	std::atomic<std::uint64_t> m_ShadowTilesShared{ 0 };
	std::atomic<std::uint64_t> m_ShadowTilesMoving{ 0 }; // tiles movers were drawn in
	// Shadowed lights a view's atlas plan left without tiles (view thread).
	std::atomic<std::uint64_t> m_ShadowLightsUnshadowed{ 0 };
	// Views' lights by baked shadow mask (LSMK): masked alone, masked with a
	// mover in reach, and without a mask.
	std::atomic<std::uint64_t> m_MaskedLights{ 0 };
	std::atomic<std::uint64_t> m_MaskedMoverLights{ 0 };
	std::atomic<std::uint64_t> m_UnmaskedLights{ 0 };
	std::atomic<std::uint64_t> m_UnmaskedWorldLights{ 0 }; // of which world lights
	const resources::MeshEntry *BoxCasterMesh();
	std::vector<Atlas> m_Atlases;
	std::size_t m_AtlasNext = 0;
	std::uint64_t m_AtlasFrame = 0;
	// Earlier per-view atlas slots used by this frame. Exact matching
	// non-sun tiles can seed a later view's atlas without rerasterizing casters.
	std::vector<std::size_t> m_FrameAtlasIndices;
	// The screen passes (render sequence): GTAO and its output.
	std::unique_ptr<pass::ao::AmbientOcclusion> m_Ao;
	// The volumetric term over a stage view (render sequence), made for the
	// format of the target it blends into.
	// render.ssr.v1 over a stage view (render sequence): the lit pass's three
	// targets, the reflections' output and its copy back into the frame, at
	// the target's extent.
	std::unique_ptr<pass::ssr::ScreenSpaceReflections> m_Ssr;
	std::unique_ptr<pass::output::OutputRenderer> m_SsrCopy;
	device::TextureId m_SsrTargets[3];
	device::TextureId m_SsrOutput;
	std::uint32_t m_SsrWidth = 0;
	std::uint32_t m_SsrHeight = 0;
	std::uint64_t m_SsrFrame = 0;
	bool m_SsrFresh = false; // new targets still in kUndefined
	std::atomic<bool> m_SsrOn{ true };
	std::atomic<unsigned long long> m_SsrViews{ 0 };
	std::atomic<unsigned long long> m_SsrRefused{ 0 };
	bool EnsureSsr( device::IRenderDevice2 &device, std::uint32_t width, std::uint32_t height,
	    device::CompletionToken submitted );
	void RecordSsr( device::CommandEncoder &encoder, const legacy::CorePassTarget &target,
	    const ShadowWork &work );
	void ReleaseSsr( device::IRenderDevice2 &device, device::CompletionToken token );
	std::unique_ptr<pass::volumetric::VolumetricRenderer> m_Volumetric;
	device::Format m_VolumetricFormat = device::Format::kUnknown;
	std::uint64_t m_VolumetricFrame = 0;
	std::atomic<bool> m_VolumetricOn{ true };
	std::atomic<unsigned long long> m_VolumetricViews{ 0 };
	std::atomic<unsigned long long> m_VolumetricRefused{ 0 };
	void RecordVolumetric( device::CommandEncoder &encoder, const legacy::CorePassTarget &target,
	    const ViewLightInputs &in );
	bool m_OcclusionNeutral = true; // m_Occlusion holds one (made so, or cleared since)
	// RFC 0014 D4: made and replaced on the render sequence; m_TimersLock
	// guards the pointer against the main thread's TakeGpuTimes.
	std::atomic<bool> m_DynamicDraws{ false };
	std::atomic<bool> m_GpuTimersOn{ false };
	std::mutex m_TimersLock;
	std::unique_ptr<graph::GpuPassTimers> m_Timers;
	graph::GpuPassTimers *m_SlotTimers = nullptr; // during a slot's recording
	std::uint64_t m_TimersFrame = 0;              // the frame the decision is for
	bool m_TimersThisFrame = false;
	std::uint64_t m_CostFrame = ~std::uint64_t( 0 ); // captured by the frame-start slot
	// The CPU time the timed views took to record (render sequence), in ns,
	// reported beside the GPU sections.
	std::atomic<std::uint64_t> m_RecordNs{ 0 };
	std::atomic<std::uint64_t> m_RecordViews{ 0 };
	device::TextureId m_Occlusion;
	device::TextureDesc m_OcclusionDesc;
	bool m_StageSet = false; // the pass holds a world stage
	// The stage's world without its lighting, kept to set the stage again
	// when the bake's probe volume arrives after it (main thread).
	std::shared_ptr<const pass::world::WorldData> m_StageWorld;
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_WORLD_H
