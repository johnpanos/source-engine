//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world's WorldPass::Batch: one RecordBatch call's state
//			and steps (the RFC 0030 split, recorded in RFC/0016-progress.md).
//			Included only by the pass's own files.
//
//=============================================================================//

#pragma once

#include "world_pass_internal.h"

namespace render::pass::world
{

// A recording of one batch of queued views: RecordBatch takes the queue,
// then runs the steps in order. Each step returns false when the batch ends
// early (a failure already noted). An aggregate, built with designated
// initializers; its members are what the steps share.
struct WorldPass::Batch
{
	WorldPass &pass;
	std::span<const std::uint32_t> tags;
	CommandEncoder &encoder;
	const WorldTarget &target;
	RecordSection &preparation;
	State &s;
	std::vector<std::size_t> staticCohorts, posedCohorts;
	std::shared_ptr<const WorldData> world;
	std::shared_ptr<const std::vector<Claimed>> claims;
	std::shared_ptr<const std::vector<SurfaceFootprintGeometry>> footprintGeometry;
	std::shared_ptr<const std::vector<std::vector<SurfaceFootprintBounds>>> modelFootprintBounds;
	std::uint64_t generation = 0;
	WorldView view;
	// The slot's dynamic draws, shared with the queue (dynamic views never batch).
	std::shared_ptr<const State::QueuedDynamic> queuedDynamic;
	IRenderDevice2 &device;
	IWorldTextures &textures;
	Resources &r;

	// A surface's screen and UV extents depend on the view and the surface, not
	// on the pass that draws it: the prepass, depth and color passes (and a
	// model's depth and color draws) share one projection of its vertices.
	struct FootprintExtents
	{
		enum class State : std::uint8_t
		{
			kUnknown,
			kValid,
			kInvalid
		};
		State state = State::kUnknown;
		float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
		float minU = 0.0f, minV = 0.0f, maxU = 0.0f, maxV = 0.0f;
		// The material sets whose texture requests this surface has already made
		// this view (a repeat is a no-op: requests merge to the finest mip).
		const void *submitted[3] = {};
	};

	// A material's texture slots with their mip descriptions, resolved once
	// per view instead of once per surface.
	struct FootprintSlot
	{
		std::uint32_t slot, width, height, levels;
	};

	// GPU-driven submission (RFC 0016 S3/S4): outside a rendering, cull a
	// surface list on the GPU (and test it against this view's pyramid when
	// `occlude`), then compact it into one command list per bucket, a run of
	// the list with one material and lightmap page: the runs drawSurfaces
	// binds once each. No commands: the caller draws per surface.
	struct GpuBucket
	{
		std::uint32_t material = 0;
		int page = 0;
		std::uint32_t first = 0; // into the list
		std::uint32_t count = 0;
	};

	struct GpuList
	{
		std::vector<GpuBucket> buckets;
		BufferId commands;
	};

	// GPU-driven static models (RFC 0016 S3): a list's opaque static model
	// draws whose surface bounds are known, culled per draw (and occlusion-
	// tested with the world's surfaces when `occlude`) and compacted into one
	// command list per (material, mesh, level) run of the lit pass's order. A
	// command's first instance selects the draw's matrices in a per-instance
	// buffer, which the instanced point reads (SurfaceVariant::instanced).
	// Poses, temporal views, debug-specialized (with `viewDebug`), blended
	// and transmitting draws stay per draw. `taken` marks the list's entries
	// it draws (empty: none).
	struct GpuModelBucket
	{
		std::uint32_t material = 0;
		std::uint32_t mesh = 0;
		std::uint32_t lod = 0;
		std::uint32_t first = 0; // into draws
		std::uint32_t count = 0;
	};

	struct GpuModels
	{
		std::vector<GpuModelBucket> buckets;
		std::vector<StaticDraw> draws;
		std::vector<bool> taken;
		BufferId commands;
		BufferId records;
	};

	struct DynamicDraw
	{
		Resources::Material *material;
		BufferId vertices;
		BufferId indices;
		std::uint32_t count;
		int page;
		const WorldView::DynamicDraw *source;
		const Group *lit = nullptr; // the draw's own group, with its model lighting
		PipelineId pipeline;        // the program's, or its static vertex light variant
		// The draw's slices of the batch's buffers, in bytes.
		std::uint64_t vertexOffset = 0;
		std::uint64_t indexOffset = 0;
		IndexFormat indexFormat = IndexFormat::kUint32; // 16-bit: a lone draw's own
	};

	// A group from its request: the constants uploaded here, the textures
	// imported by the names' handles, with the backend's samplers; an input
	// the program names empty (a term that is off) or a handle of 0 (an
	// absent input) takes the neutral texture of its dimension.
	// Set by buildGroup when its failure is a texture the backend has made and
	// not filled yet; prepareMaterial retries such a material next frame
	// instead of failing its view.
	bool texturePending = false;
	// The view's first failure; the view counts once.
	std::string failure{};
	// Set by prepareMaterial when its material waits on a texture still being
	// filled (not a failure, so its caller notes nothing).
	bool lastMaterialPending = false;
	bool viewMaterialPending = false; // any in this view
	// The frame group of a program's layout, its terms written for this slot.
	material::FrameTerms terms{};
	// Presence comes from this slot's actual frame/view inputs, never a quality
	// setting. Uniform light data and all shadow filters remain unchanged.
	std::uint32_t viewFeatures{};
	std::map<std::uint64_t, bool> framesWritten{};
	// A program's view group: its neutral one (built once per layout).
	// A world stage's lit view: one group per view layout for this view, with
	// the view's clustered lights, retired behind this frame after drawing.
	std::map<std::uint64_t, Group *> litViews{};
	std::map<std::uint64_t, Group *> modelLitViews{};
	std::deque<Group> transientLitViews{};
	std::map<std::uint64_t, Group> sceneViews{};
	std::map<std::uint64_t, Group> depthViews{};
	// The view's ambient occlusion once the screen passes recorded it.
	TextureId viewOcclusion{};
	// The view's planar reflection (a program's view input, imported below
	// once the view's materials resolved), and the view groups that bind it
	// for programs without the view's lights, retired behind this frame.
	TextureId viewReflection{};
	// The view's water refraction (ResolvedProgram::refractInput), imported
	// as the reflection is; its view groups bind it at the scene color's slot.
	TextureId viewRefraction{};
	std::map<std::uint64_t, Group> refractViews{};
	TextureId viewSceneColor{};
	TextureDesc viewSceneColorDesc{};
	TextureId viewDepthAlpha{};
	std::map<std::uint64_t, Group> reflectViews{};
	// Resolve before rendering (uploads run outside it), then draw in
	// (material, page) order so binds change least.
	std::vector<std::uint32_t> order{};
	bool complete = true;
	std::vector<StaticDraw> staticDraws{};
	std::vector<BufferId> posedBuffers{};
	std::vector<BufferId> previousPosedBuffers{};
	// The view's draw constants. D3D9 puts pixel centers on integer
	// coordinates: a D3D9 transform is shifted right and down by half a pixel
	// of the viewport.
	material::FamilyDrawConstants constants{};
	std::span<const std::byte> constantBytes{};
	std::span<const std::byte> waterConstantBytes{};
	// The frame's debug controls (RFC 0014), as the view was queued: each
	// program's pipeline under its specialization (the shipped one when the
	// controls are neutral). A refused debug pipeline fails the view loudly.

	// Draws `list`'s surfaces, inside the caller's rendering, with their
	// materials in `materials` and each material's pipeline from
	// `pipelineOf` (none: the surfaces are not drawn and the view fails).
	// Surfaces of one binding whose index ranges touch draw as one range (a
	// world stage's meshlets, in the mesh's order).
	bool recordingTemporal = false;
	bool recordingSsr = false;
	std::map<std::pair<const void *, std::uint64_t>, std::vector<FootprintSlot>> footprintSlots{};
	FootprintExtents scratchFootprint{};
	std::vector<FootprintExtents> worldFootprints{};
	std::map<std::tuple<bool, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>,
	    FootprintExtents>
	    modelFootprints{};
	// Cutout casters (RFC 0016 K12): the stage's alpha-tested surfaces, each
	// through its own material's depth point, drawn over the atlas the
	// composition's other casters drew, before any view reads it.
	std::uint64_t cutoutDraws = 0;
	std::uint64_t cutoutIndirectDraws = 0;
	std::uint64_t cutoutRefused = 0;
	std::uint64_t cutoutNotResident = 0;
	std::array<float, 48> screenInputs{};
	// GPU occlusion (WorldTarget::gpuOcclusion): the pyramid comes from the
	// screen prepass's depth of the world surfaces, which covers the whole
	// target in clip depth. A surface it removes must fail the lit pass's
	// depth test at every pixel, so the view must test depth in the ordinary
	// direction (or equal, after the depth prepass) and must not change the
	// stencil where the test fails.
	const CapabilitySet &gpuCaps = device.Facts().capabilities;
	bool gpuDeviceCapable{};
	bool occlusionWanted{};
	std::vector<DynamicDraw> dynamicDraws{};
	// Draw groups holding one draw's model lighting: built per draw and
	// retired with the frame, never cached.
	std::deque<Group> litDrawGroups{};

	// A world stage's textures: made at the first slot from the stage, then
	// updated in place as its lighting changes (SetStageLightmap,
	// SetStageChange), so the groups that bind them stay valid. Each upload
	// has an initialized upload buffer of its own, released behind a later frame.
	bool stageUpload( TextureId texture, const TextureDesc &desc, std::span<const std::byte> bytes,
	    ResourceUsage from );

	// Rectangles of an RGBA16F stage texture from their packed texels.
	bool stageRegions( TextureId texture, const TextureDesc &desc,
	    const std::vector<StageRegion> &regions, std::span<const std::byte> texels );

	bool stageMake( const char *name, Format format, std::uint32_t width, std::uint32_t height,
	    std::span<const std::byte> bytes );

	// The map's reflection probes (RPRB v8): the BC6H radiance cube array,
	// uploaded as the lump stores it (a copy per mip, probe and face, from one
	// initialized upload buffer), and the probe buffer's bytes for the frame
	// groups. A device without BC6H cube arrays refuses the stage by name.
	bool stageMakeReflection( const StageReflectionProbes &probes );

	// The grid table's rows, with room for the moving occluders' rows.
	std::vector<float> paddedTable( const StageProbeVolume &table, std::uint32_t rows );

	material::SurfaceDrawState prepassedState();

	std::uint64_t drawGpuModels(
	    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
	    const std::function<bool( const Resources::Material & )> &skip );

	bool captureSceneColor();

	// The view's opaque models culled on the GPU (RecordView), for drawGpuModels.
	GpuModels gpuViewModels{};

	void Run();
	bool PrepareResources();
	bool PrepareFrameTerms();
	bool PrepareViewGroups();
	bool PrepareSurfacesAndModels();
	bool PrepareDrawHelpers();
	bool RecordCutoutShadows();
	bool PrepareGpuSubmission();
	bool RecordScreenPasses();
	bool PrepareDynamicDraws();
	bool RecordView();

	// A new resolver (a map load): record the pipelines its program creates,
	// and create the previous runs' ones now, before any draw needs them, so
	// their driver compiles land in the load instead of mid-play.
	void prewarmResolver( material::ProgramResolver &resolver, const char *which );

	// Neutral inputs, made on first use: white color and far depth. The cube
	// term is off wherever its neutral texture is bound; filling it keeps it defined.
	TextureId neutral( TextureDimension dimension, bool array, bool depth );

	bool buildGroup( const material::GroupRequest &request,
	    const std::map<std::string, int> &handles, Group &out, std::string *why );

	void note( std::string why );

	Resources::Material *prepareMaterial( material::ProgramResolver &resolver,
	    Resources::Material &m, const Claimed &claimed, const WorldMaterial &source );

	Resources::Material *materialReadyIn( material::ProgramResolver &resolver,
	    std::vector<Resources::Material> &materials, std::uint32_t index );

	Resources::Material *materialReady( std::uint32_t index );

	std::tuple<std::uint64_t, int, std::string> drawKey(
	    const Resources::Material &m, int page, bool captured = false );

	const Group *drawGroupReady( const Resources::Material &m, int page, bool captured = false );

	const Group *frameGroupReady( const Resources::Material &m );

	const Group *viewGroupReady( const Resources::Material &m );

	std::optional<PipelineId> statePipeline( const Resources::Material &m, PipelineId base,
	    const material::SurfaceDrawState &state, const shaderlib::DebugSpecialization &debug = {} );

	// World and MDL-reader triangles face counterclockwise from outside.
	// Their owner applies authored culling, including on posed models and
	// private prepasses. Dynamic snapshots keep their captured raster state.
	std::optional<PipelineId> surfaceStatePipeline( const Resources::Material &m, PipelineId base,
	    material::SurfaceDrawState state, const shaderlib::DebugSpecialization &debug = {} );

	template <typename Vertices, typename Indices>
	void submitSurfaceFootprints( const WorldSurface &surface,
	    const std::vector<Resources::Material> &materials, const Vertices &vertices,
	    const Indices &indices, FootprintExtents &extents, const float *objectToWorld = nullptr,
	    const SurfaceFootprintGeometry *compact = nullptr,
	    const SurfaceFootprintBounds *box = nullptr );

	// A world material's pipeline, groups, the world's shared vertex and index
	// buffers, and the view's draw constants: what both the per-surface runs
	// and the GPU-driven buckets bind.
	void bindSurfaceMaterial( const Resources::Material &m, PipelineId pipeline );

	void drawSurfaces( const std::vector<std::uint32_t> &list,
	    const std::vector<Resources::Material> &materials,
	    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
	    bool breakdown = false );

	// A model draw's FamilyDrawConstants: its object-to-world (identity for
	// a pose, whose vertices are in world space) and object-to-clip.
	material::FamilyDrawConstants modelDrawConstants( const StaticDraw &draw );

	// Binds a model draw's pipeline, groups and buffers; with `instances`
	// (an instanced point's per-instance records) instead of its draw
	// constants.
	void bindModel( const StaticDraw &draw, const Resources::Material &m, PipelineId pipeline,
	    BufferId instances = {} );

	// A model draw's texture mip feedback (the per-surface footprint).
	void modelFeedback( const StaticDraw &draw );

	void recordModel( const StaticDraw &draw, const Resources::Material &m, PipelineId pipeline );

	// A world stage's screen passes (render_lab's order): the depth and
	// normal prepass into the pass's own single-sample targets, with a
	// resolver of their own, then the composition's ambient occlusion, which
	// the lit view groups read.
	bool opaquePbr( const Resources::Material &m );

	math::float4x4 viewToClip();

	// Records the pyramid of r.prepassDepth (in kDepthWrite, outside a
	// rendering) and leaves the depth in kDepthWrite again.
	bool buildPyramid();

	// The dispatches of GPU-driven submission (RFC 0016 S3/S4), outside a
	// rendering: cull `instances` against the view (and this view's pyramid
	// when `occlude`), then compact the kept ones' `templates` into one
	// command list per bucket (counts as words, culling::CommandsOffset).
	// The command buffer, invalid when the kernels or buffers were refused.
	BufferId gpuCompactCommands( const std::vector<culling::CullInstance> &instances,
	    const std::vector<culling::DrawTemplate> &templates,
	    const std::vector<culling::DrawBucket> &buckets, bool occlude );

	GpuList gpuCullList( const std::vector<std::uint32_t> &list,
	    const std::vector<Resources::Material> &materials, bool occlude );

	GpuModels gpuCullModels( const std::vector<StaticDraw> &list,
	    const std::vector<Resources::Material> &materials, bool occlude, bool viewDebug );

	// One indirect draw per bucket of `models` whose material `skip` does not
	// name, bound as recordModel binds its first draw, with the instanced
	// point of `pipelineOf`'s pipeline. The draws it covers (submitted; the
	// GPU culls some).
	std::uint64_t drawModelBuckets( const GpuModels &models,
	    const std::vector<Resources::Material> &materials,
	    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
	    const std::function<bool( const Resources::Material & )> &skip );

	// One indirect draw per bucket of `gpu`, bound as drawSurfaces binds the
	// same run; buckets whose material `skip` names are left out.
	void gpuDrawList( const GpuList &gpu, const std::vector<std::uint32_t> &list,
	    const std::vector<Resources::Material> &materials,
	    const std::function<std::optional<PipelineId>( const Resources::Material & )> &pipelineOf,
	    const std::function<bool( const Resources::Material & )> &skip = {} );
};

} // namespace render::pass::world
