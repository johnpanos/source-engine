//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world's private declarations: the types its files
//			share and WorldPass::State. Included only by the pass's own files.
//
//=============================================================================//

#pragma once

#include "render/pass/world/world_pass.h"

#include "render/culling/cull.h"
#include "group_resources.h"

#include "render/device/errors.h"
#include "render/frame/debug_specialization.h"
#include "render/material/draw_program.h"
#include "render/material/material_programs.h"
#include "render/material/program_resolver.h"
#include "render/material/scene_terms.h"
#include "render/material/surface_program.h"
#include "render/material/vmt_import.h"
#include "render/math/matrix.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <mutex>
#include <span>
#include <string_view>
#include <tuple>
#include <unordered_map>

namespace render::pass::world
{

namespace detail
{

using namespace render::device;

// Diagnostic-only contiguous scopes. The guard closes failures/early returns;
// changing a label never changes draw order or splits an indexed draw run.
class RecordSection
{
public:
	explicit RecordSection( CommandEncoder &encoder ) : m_Encoder( encoder ) {}
	~RecordSection() { End(); }
	RecordSection( const RecordSection & ) = delete;
	RecordSection &operator=( const RecordSection & ) = delete;

	void Select( std::string_view prefix, std::string_view name = {} )
	{
		if ( !m_Encoder.LabelObserver() || ( m_Open && prefix == m_Prefix && name == m_Name ) )
			return;
		End();
		m_Prefix = prefix;
		m_Name = name;
		m_Encoder.BeginLabel( std::string( prefix ) + std::string( name ) );
		m_Open = true;
	}
	void End()
	{
		if ( m_Open )
			m_Encoder.EndLabel();
		m_Open = false;
	}

private:
	CommandEncoder &m_Encoder;
	std::string_view m_Prefix;
	std::string_view m_Name;
	bool m_Open = false;
};

inline bool SurfaceSelected(
    const std::optional<std::vector<std::uint32_t>> &selection, std::uint32_t surface )
{
	return !selection || std::binary_search( selection->begin(), selection->end(), surface );
}

inline bool ValidSurfaceSelection(
    const std::optional<std::vector<std::uint32_t>> &selection, std::size_t count )
{
	if ( !selection )
		return true;
	for ( std::size_t i = 0; i < selection->size(); ++i )
	{
		if ( ( *selection )[i] >= count || ( i && ( *selection )[i - 1] >= ( *selection )[i] ) )
			return false;
	}
	return true;
}

// What the main thread decided about a material.
struct Claimed
{
	bool draws = false;
	bool blended = false;
	bool opaqueBatch = false;
	material::MaterialDesc desc;
	std::map<std::string, int> handles; // the importer's texture name -> handle
};

// One opaque model draw of a recorded batch: which cohort's instance list it
// came from, whether it is a posed model, and its instance, mesh, level,
// surface and material. The screen passes' prepass lists hold these too (their
// cohort is 0: the prepass draws the world's instances, not a view's).
struct StaticDraw
{
	std::size_t cohort = 0;
	bool posed = false;
	std::uint32_t instance = 0;
	std::uint32_t mesh = 0;
	std::uint32_t lod = 0;
	std::uint32_t surface = 0;
	std::uint32_t material = 0;
	bool gpu = false; // drawn GPU-driven this view (the per-draw loops skip it)
};

// A resolved group: its device objects.
struct Group
{
	BindGroupId group;
	GroupResources::Buffer constants;
	std::vector<GroupResources::Buffer> storage;
	std::vector<SamplerId> samplers;
};

// The device objects of one world for one target format, made on the
// render sequence.
struct Resources
{
	Format colorFormat = Format::kUnknown;
	Format depthFormat = Format::kUnknown;
	std::uint32_t samples = 1;
	std::unique_ptr<material::ProgramResolver> resolver;
	std::unique_ptr<material::ProgramResolver> modelResolver;
	BufferId vertices;
	BufferId indices;
	struct Material
	{
		material::ResolvedProgram program;
		material::ProgramResolver *resolver = nullptr; // the one that resolved it
		Group group;
		bool ready = false;
		bool failed = false;
		std::string failure;        // why, when failed
		std::uint64_t lastUsed = 0; // the frame a dynamic snapshot last drew
		// The material system handle of the view render target the program
		// reads through its view group (ResolvedProgram::viewInputs: the
		// water point's planar reflection), or 0.
		int viewInput = 0;
		// The handle of its refraction target (ResolvedProgram::refractInput), or 0.
		int refractInput = 0;
	};
	std::vector<Material> materials;
	std::vector<Material> modelMaterials;
	// Snapshots used in the last kDynamicMaterialFrames recorded frames are
	// retained (a snapshot's key holds every value and texture handle it
	// binds, so a kept one is still exact); older ones and the overflow of
	// kMaxDynamicMaterials retire behind the submitted token, as transient
	// geometry does. Before 2026-10-05 every frame cleared the map, which
	// re-resolved each model material once per frame.
	std::unordered_map<std::string, Material> dynamicMaterials;
	std::uint64_t dynamicFrame = 0;
	// A world stage's depth-and-normal prepass (the screen passes' input): a
	// single-sample resolver of its own (its layouts, so its groups, differ
	// from the lit one's; the group maps below are keyed by layout), its
	// materials and targets.
	std::unique_ptr<material::ProgramResolver> prepassResolver;
	std::unique_ptr<material::ProgramResolver> prepassModelResolver;
	std::vector<Material> prepassModelMaterials;
	std::vector<Material> prepassMaterials;
	TextureId prepassDepth;
	TextureId prepassNormal;
	TextureDesc prepassDepthDesc;
	TextureDesc prepassNormalDesc;
	bool prepassUsed = false; // the targets rest in kSampled after their first view
	// A world stage's screen-pass prepass draw lists (the depth and normal
	// prepass that feeds ambient occlusion): the world's surfaces and its
	// instances' opaque PBR draws, built once instead of once per view per
	// frame. They are this resource set's, because they come from its own
	// resolvers, materials and groups (one set per target format), and they
	// depend on nothing in the view: the same entries are drawn into the
	// pass's own targets for every view that needs them. Only a complete
	// build is kept, so a material or group that was not ready is retried
	// rather than remembered as absent.
	struct PrepassLists
	{
		std::uint64_t generation = 0;
		std::uint64_t residencyRevision = 0;
		// The claims the world was set with, by pointer: SetWorld replaces the
		// vector, so its identity changes with the world's contents.
		const std::vector<Claimed> *claims = nullptr;
		std::vector<std::uint32_t> surfaces; // into WorldData::surfaces
		std::vector<StaticDraw> models;
		bool valid = false;
	};
	PrepassLists prepassLists;
	// Draw groups by (draw layout, lightmap page).
	// Draw groups by (draw layout, lightmap page, the program's draw inputs):
	// programs that share a layout may read different inputs (the pbr point
	// its three pages, unlit and lightmapped points the page alone).
	std::map<std::tuple<std::uint64_t, int, std::string>, Group> drawGroups;
	// Frame groups by frame layout; their constants are written per slot.
	std::map<std::uint64_t, Group> frameGroups;
	// The surface program's depth points of cutout materials (by material
	// index), for WorldTarget::cutoutShadows.
	std::map<std::uint32_t, PipelineId> cutoutPipelines;
	std::map<std::uint32_t, PipelineId> cutoutModelPipelines; // by model material
	// The programs' neutral view groups, by view layout (the world pass
	// supplies no clustered lights yet).
	std::map<std::uint64_t, Group> viewGroups;
	// Immutable lighting bindings shared by cohorts of one recording. The
	// snapshot is kept alive so pointer identity cannot be recycled underneath
	// the cache. Scene-color captures have ordered contents and stay transient.
	struct LitView
	{
		std::uint64_t layout = 0;
		std::shared_ptr<const StageViewLights> lights;
		TextureId shadowAtlas;
		TextureId occlusion;
		TextureId reflection;
		Group group;
	};
	std::deque<LitView> litViews; // stable addresses while preparing other layouts
	std::uint64_t litFrame = 0;
	std::uint64_t litStream = 0;
	// A 1x1 white texture for an absent input (a surface with no lightmap
	// page samples white: the input's neutral value), a 1x1 black cube for an
	// absent env map (its term is off, so it is never read), and their upload
	// buffer.
	TextureId neutralWhite;
	TextureId neutralCube;
	TextureId neutralCubeArray; // two cubes, for a binding read as a cube array
	TextureId neutralArray;     // two layers, for a binding read as an array
	// The map's RPRB probe buffer (WriteReflectionProbeBuffer words), shared
	// with every frame group request.
	std::shared_ptr<const std::vector<std::byte>> reflectionBuffer;
	TextureId neutralDepth; // far depth for an absent shadow atlas
	BufferId neutralStaging;
	bool uploaded = false;
	// A world stage's textures (WorldStage), by the names its groups use,
	// made at the first slot and updated in place; the stage revisions
	// they hold.
	std::map<std::string, TextureId> stageTextures;
	std::map<std::string, TextureDesc> stageDescs;
	bool stageMade = false;
	bool reflectionMade = false; // the map's reflection probes (WorldData::reflection)
	std::uint64_t stageLightmapRevision = 0;
	std::uint64_t stageProbeRevision = 0;
	std::uint64_t stageChangeRevision = 0;
};

// The names a world stage's groups use for its textures.
constexpr const char *kStageLightmap = "stage:lightmap";
constexpr const char *kStageGradient = "stage:lightmap-gradient";
constexpr const char *kStageIndirect = "stage:lightmap-indirect";
constexpr const char *kStageIndirectGradient = "stage:lightmap-indirect-gradient";
constexpr const char *kStageShadowMask = "stage:lightmap-shadow-mask";
constexpr const char *kStageProbeAtlas = "stage:probe-atlas";
constexpr const char *kStageProbeGrids = "stage:probe-grids";
constexpr const char *kStageChange = "stage:change";
constexpr const char *kStageReflection = "stage:reflection-probes";
constexpr const char *kStageSplitSum = "stage:split-sum";
constexpr const char *kStageLtc = "stage:ltc";
// Room in the grid table for the moving occluders' rows
// (world_mesh_gpu::kProbeVolumeMaxOccluders).
constexpr std::uint32_t kStageOccluderRows = 16;

// Adapt the product's stage to the surface program's shared scene policy.
// The view's occlusion is one where no screen pass recorded it.
inline std::uint32_t StageTerms(
    const WorldStage &stage, bool reflection, bool runtimeDirect, bool ambientOcclusion )
{
	material::SceneTermInputs inputs;
	inputs.runtimeDirect = runtimeDirect;
	inputs.indirectLightmap = !stage.indirect.flat.empty();
	inputs.totalDirectionalLightmap = stage.lightmap.Directional();
	inputs.indirectDirectionalLightmap = stage.indirect.Directional();
	inputs.probeVolume = stage.probes.has_value();
	inputs.probeBounce = stage.probes.has_value(); // the change atlas is always supplied
	inputs.reflectionProbes = reflection;
	// Off: compiled out of the programs (the neutral view still binds one).
	inputs.ambientOcclusion = ambientOcclusion;
	return material::SceneTerms( inputs );
}

// The scene terms a world's points draw with: its stage's, or, on a plain
// map, only the reflection probes its cubemaps supply.
inline std::uint32_t WorldTerms( const WorldData &world, bool runtimeDirect, bool ambientOcclusion )
{
	if ( world.stage )
		return StageTerms(
		    *world.stage, world.reflection.has_value(), runtimeDirect, ambientOcclusion );
	return world.reflection ? material::kSurfaceReflectionProbes : 0u;
}
foundation::Expected<Claimed, std::string> MapWorldMaterial( const WorldMaterial &source );

std::string MaterialSnapshotKey( const WorldMaterial &source );

// Whether serial a was issued before serial b (serials wrap within the
// tag's serial bits).
inline bool IssuedBefore( std::uint32_t a, std::uint32_t b )
{
	const std::uint32_t distance = ( b - a ) & kWorldSerialMask;
	return distance != 0 && distance < ( ( kWorldSerialMask + 1 ) >> 1 );
}

inline std::string PageName( int handle )
{
	return "lightmap-page:" + std::to_string( handle );
}

inline std::uint32_t StaticMaterial(
    const WorldData::StaticMesh &mesh, std::uint64_t skin, std::uint32_t surface )
{
	if ( surface >= mesh.surfaces.size() )
		return ~0u;
	if ( mesh.skinMaterials.empty() )
		return skin == 0 ? mesh.surfaces[surface].material : ~0u;
	if ( skin >= mesh.skinMaterials.size() || surface >= mesh.skinMaterials[skin].size() )
		return ~0u;
	return mesh.skinMaterials[skin][surface];
}

// Model geometry residency (RFC 0016). A level no view has selected for this
// many recorded frames is released, so a model only ever holds the levels its
// instances actually draw: level zero of a model every instance draws coarsely
// is not resident for the rest of the level's life. Re-upload costs one buffer
// creation and copy, which is why the window is long enough to keep a camera
// that crosses a level's switch point from thrashing.
constexpr std::uint32_t kModelLevelIdleFrames = 120;

// A pinned level is never released, so it needs no staging to come back from.
// The coarsest level is what every distance selects; a posed model is drawn at
// whichever level the host selects, at any distance.
inline bool ModelLevelPinned( const WorldData::StaticMesh &mesh, std::uint32_t lod )
{
	return mesh.posed || lod + 1 >= mesh.lodCount();
}

// A surface's index range is its own level's, from zero: a surface no level
// lists, or one whose range leaves that level's indices, is not drawable.
inline bool LevelSurfaceDrawable(
    const WorldData::StaticMesh &mesh, std::uint32_t surface, const WorldSurface &range )
{
	const std::uint32_t lod = mesh.LodOfSurface( surface );
	if ( lod == ~0u || lod >= mesh.lodCount() || !mesh.lods[lod].Drawable() )
		return false;
	return range.firstIndex <= mesh.lods[lod].indexCount &&
	       range.indexCount <= mesh.lods[lod].indexCount - range.firstIndex;
}

// referenced vertex once, in a contiguous array, with the UV bounds. A surface
// the footprint rejects (an index range or vertex outside the data, a
// non-finite UV) is invalid for every view.
struct SurfaceFootprintGeometry
{
	bool valid = false;
	float minU = 0.0f, minV = 0.0f, maxU = 0.0f, maxV = 0.0f;
	std::vector<std::array<float, 3>> positions;
};

// The object-space box and UV bounds of one model surface: a mip footprint
// projects the box's eight corners instead of every index.
struct SurfaceFootprintBounds
{
	bool valid = false;
	float lo[3] = {}, hi[3] = {};
	float minU = 0.0f, minV = 0.0f, maxU = 0.0f, maxV = 0.0f;
};

template <typename Vertices>
SurfaceFootprintBounds MeasureFootprintBounds( const WorldSurface &surface,
    const Vertices &vertices, const std::vector<std::uint32_t> &indices )
{
	SurfaceFootprintBounds out;
	if ( !surface.indexCount || surface.firstIndex > indices.size() ||
	     surface.indexCount > indices.size() - surface.firstIndex )
		return out;
	for ( int axis = 0; axis < 3; ++axis )
	{
		out.lo[axis] = std::numeric_limits<float>::max();
		out.hi[axis] = std::numeric_limits<float>::lowest();
	}
	out.minU = out.minV = std::numeric_limits<float>::max();
	out.maxU = out.maxV = std::numeric_limits<float>::lowest();
	for ( std::uint32_t k = 0; k < surface.indexCount; ++k )
	{
		const std::uint32_t vertexIndex = indices[surface.firstIndex + k];
		if ( vertexIndex >= vertices.size() )
			return {};
		const auto &vertex = vertices[vertexIndex];
		if ( !std::isfinite( vertex.uv[0] ) || !std::isfinite( vertex.uv[1] ) )
			return {};
		for ( int axis = 0; axis < 3; ++axis )
		{
			if ( !std::isfinite( vertex.position[axis] ) )
				return {};
			out.lo[axis] = std::min( out.lo[axis], vertex.position[axis] );
			out.hi[axis] = std::max( out.hi[axis], vertex.position[axis] );
		}
		out.minU = std::min( out.minU, vertex.uv[0] );
		out.minV = std::min( out.minV, vertex.uv[1] );
		out.maxU = std::max( out.maxU, vertex.uv[0] );
		out.maxV = std::max( out.maxV, vertex.uv[1] );
	}
	out.valid = true;
	return out;
}
} // namespace detail

using namespace detail;

struct WorldPass::State
{
	IRenderDevice2 *device = nullptr; // the device the resources live on
	std::span<const std::uint32_t> fragmentModule; // SetSurfaceFragmentModule
	// The last screen output written on this sequence. Another camera or output
	// invalidates reuse even when a format's own prepass textures still exist.
	std::uint64_t screenFrame = 0;
	TextureId screenDepth;
	TextureId screenOutput;
	std::array<float, 48> screenInputs = {};

	IModelLevelSource *levelSource = nullptr; // SetModelLevelSource; null retains staging

	// Pipeline prewarming: the keys to create when a resolver is made, and the
	// keys of the pipelines the resolvers created.
	mutable std::mutex pipelineKeyLock;
	std::vector<std::string> prewarmKeys;
	std::set<std::string> createdKeys;

	// Guarded by lock: the world the main thread set and the queued views.
	mutable std::mutex lock;
	std::shared_ptr<const WorldData> world;
	std::shared_ptr<const std::vector<Claimed>> claims;
	// Per world surface, the unique vertex positions and UV bounds its mip
	// footprint reads each frame (built once, with the world).
	std::shared_ptr<const std::vector<SurfaceFootprintGeometry>> footprintGeometry;
	// Per static mesh and surface, the object-space footprint bounds in the
	// surface's own level (built once, with the world; static instances reuse
	// them every frame).
	std::shared_ptr<const std::vector<std::vector<SurfaceFootprintBounds>>> modelFootprintBounds;
	std::uint64_t generation = 0;
	std::uint32_t nextSerial = 1;
	// MapWorldMaterial of a dynamic draw's material, and its claim as last
	// decided for the world's stage and reflection flags (the claim is a pure
	// function of the mapping and those flags).
	using MappedMaterial = foundation::Expected<Claimed, std::string>;
	struct MappedEntry
	{
		MappedMaterial material;
		bool claimStage = false;
		bool claimReflection = false;
		std::string claimError; // empty: claimed
		bool requiresDepthAlpha = false;
		// A claimed SpriteCard's card terms (render.sprite-card.v1).
		std::optional<sprite_card::Frame> cardTerms;
	};
	// A queued view's dynamic draws, with each draw's snapshot key and mapping
	// decided once when the view queued; shared, so recording a slot (and
	// recording it again for a capture) never copies the draws' geometry.
	struct QueuedDynamic
	{
		std::vector<WorldView::DynamicDraw> draws;
		std::vector<std::string> keys;
		std::vector<std::shared_ptr<const MappedEntry>> mapped;
	};
	struct Queued
	{
		std::uint32_t serial = 0;
		std::uint64_t generation = 0; // the world the view was queued against
		std::uint64_t recordedStream = 0;
		WorldView view; // its dynamicDraws moved into `dynamic`
		std::shared_ptr<const QueuedDynamic> dynamic;
	};
	std::size_t OpaqueBatchSize(
	    std::span<const std::uint32_t> tags, std::uint64_t streamEpoch ) const;
	std::deque<Queued> views;
	// Views already recorded, newest last: the backend records a frame's
	// stream again for an on-demand capture (a screenshot), with the same
	// slots, which must draw the same views.
	std::deque<Queued> recorded;
	// Host frames with a recorded slot, newest last.
	std::deque<std::uint64_t> recordedFrames;
	WorldStats stats;
	// MappedEntry of each dynamic draw's material, by MaterialSnapshotKey
	// (every input of the mapping). Entries are immutable, so a cleared table
	// leaves borrowers their entry.
	std::mutex mappedLock;
	std::unordered_map<std::string, std::shared_ptr<const MappedEntry>> mapped;
	// The snapshot key of each revisioned material (WorldMaterial::revision).
	std::unordered_map<std::uint64_t, std::string> revisionKeys;
	std::shared_ptr<const MappedEntry> Mapped(
	    const WorldMaterial &source, bool stage, bool reflection, std::string &key );
	// A world stage's lighting as it changes (SetStageLightmap,
	// SetStageProbeVolume, SetStageChange), with revisions that rise with each.
	// The total page: `stageLightmap` as of `stageLightmapBaseRevision`
	// (null: the stage's own pages), then the parts set since
	// (SetStageLightmapRegions), in revision order; a part every resource
	// set has applied folds into it.
	std::shared_ptr<LightmapPages> stageLightmap;
	std::uint64_t stageLightmapRevision = 0;
	std::uint64_t stageLightmapBaseRevision = 0;
	std::shared_ptr<std::vector<std::byte>> stageProbeAtlas;
	std::uint64_t stageProbeRevision = 0;
	std::uint64_t stageProbeBaseRevision = 0;
	std::shared_ptr<const StageProbeVolume> stageTable;
	std::uint64_t stageChangeRevision = 0;
	// The probe change: `stageChangeBase` as of `stageBaseRevision` (empty:
	// zero), then the parts set since, in revision order. A part every
	// resource set has applied folds into the base.
	struct StagePatch
	{
		std::uint64_t revision = 0;
		std::vector<WorldPass::StageRegion> regions;
		std::vector<std::byte> texels;
	};
	std::vector<std::byte> stageChangeBase;
	std::uint64_t stageBaseRevision = 0;
	std::deque<StagePatch> stagePatches;
	std::deque<StagePatch> stageLightmapPatches;
	std::deque<StagePatch> stageProbePatches;

	// A queued view dropped because its slot never recorded (lock held): a
	// failure when a slot of its host frame recorded (or its frame is
	// unknown), else skipped with its frame.
	void Drop( const Queued &dropped, std::uint64_t recordingFrame )
	{
		// An earlier world's view: its level is gone, so is its slot.
		if ( dropped.generation != generation )
		{
			++stats.viewsSkipped;
			return;
		}
		const std::uint64_t frame = dropped.view.hostFrame;
		const bool frameRecorded = frame == 0 || frame == recordingFrame ||
		                           std::find( recordedFrames.begin(), recordedFrames.end(),
		                               frame ) != recordedFrames.end();
		if ( frameRecorded )
		{
			++stats.viewsFailed;
			stats.lastFailure = "a queued view's slot never recorded in a frame that recorded";
		}
		else
		{
			++stats.viewsSkipped;
		}
	}

	// Render sequence only: the current world's objects, one set per target
	// format (most recently used last), and earlier sets with the frame that
	// retired them.
	std::uint64_t variantsGeneration = 0;
	// The runtime direct light the variants' programs were resolved with
	// (WorldTarget::runtimeDirect).
	bool variantsRuntimeDirect = false;
	bool variantsAmbientOcclusion = true;
	std::vector<Resources> variants;
	std::vector<std::pair<std::uint64_t, Resources>> retired;
	// Render sequence only: the world's model geometry, one allocation pair per
	// (model, level). Model buffers are world data, not target state, so they
	// outlive a target format change and a stage republication (WorldData's
	// modelsRevision); they are released with the world, with the device, or by
	// the residency rule below, and a released level is uploaded again from
	// the world's staging when a view selects it.
	struct ModelLevel
	{
		BufferId vertices;
		BufferId indices;
		// The last recorded frame that drew or uploaded this level (0: never),
		// and whether its buffers are on the device now.
		std::uint64_t lastUsedFrame = 0;
		bool resident = false;
		// True after the first upload when a model level source is set: the
		// composition may release the staging shared_ptrs, and a re-upload
		// uses the source instead.
		bool stagingReleased = false;
	};
	struct ModelGeometry
	{
		std::uint64_t modelsRevision = 0; // WorldData::modelsRevision it belongs to
		std::vector<std::vector<ModelLevel>> models;
		std::uint64_t bytes = 0;          // resident vertex and index bytes
		std::uint64_t releasedLevels = 0; // levels the residency rule has released
	};
	ModelGeometry models;
	// Rises with every level that becomes resident or is released, and whenever
	// the model table is rebuilt (a new models revision, a new device). It is
	// monotone across those resets, unlike ModelGeometry itself. The screen
	// passes' prepass lists name the resident levels, so this is part of their
	// cache key.
	std::uint64_t modelResidencyRevision = 0;
	// The frame that last swept residency, and the frame the report was
	// published for, so each runs once per recorded frame.
	std::uint64_t residencyFrame = 0;
	std::uint64_t residencyReportedFrame = 0;
	// Model buffers released with the frame whose slot may still read them.
	struct RetiredModelBuffer
	{
		std::uint64_t frame = 0;
		BufferId buffer;
	};
	std::vector<RetiredModelBuffer> retiredModelBuffers;
	// Staging buffers of stage uploads, and a stage's per-view groups, with
	// the frame that recorded them.
	std::vector<std::pair<std::uint64_t, BufferId>> retiredBuffers;
	// GPU-driven submission: the kernels on this device, the per-surface
	// world bounds of the world they were built for, and the frame-retired
	// bind groups of recorded dispatches.
	std::unique_ptr<culling::CullKernel> gpuCull;
	std::unique_ptr<culling::CompactKernel> gpuCompact;
	// Occlusion (WorldTarget::gpuOcclusion): the kernels, their point
	// sampler, and the latest pyramid with the frame, prepass depth and
	// screen inputs it was built from (frame-retired like the cull buffers).
	std::unique_ptr<culling::OcclusionKernels> gpuOcclusion;
	SamplerId gpuPointSampler;
	struct GpuPyramid
	{
		BufferId pyramid;
		culling::OcclusionView view;
		std::uint64_t frame = 0;
		TextureId depth;
		std::array<float, 48> inputs{};
	} gpuPyramid;
	const WorldData *gpuBoundsWorld = nullptr;
	std::vector<culling::CullInstance> gpuBounds;
	std::vector<std::pair<std::uint64_t, BindGroupId>> retiredBindGroups;
	std::vector<std::pair<std::uint64_t, Group>> retiredGroups;
	GroupResources groupResources;
	std::vector<std::pair<std::uint64_t, TextureId>> retiredTextures;

	void Fail( const std::string &why )
	{
		std::lock_guard<std::mutex> guard( lock );
		++stats.viewsFailed;
		stats.lastFailure = why;
	}

	// A refused dynamic draw: counted, and named once per reason (the caller
	// holds the lock).
	void Refuse( std::string reason )
	{
		++stats.dynamicDrawsRefused;
		stats.lastRefusal = std::move( reason );
		auto gap = std::find_if( stats.gaps.begin(), stats.gaps.end(),
		    [&]( const auto &entry )
		    {
			    return entry.first == stats.lastRefusal;
		    } );
		if ( gap != stats.gaps.end() )
			++gap->second;
		else if ( stats.gaps.size() < 256 )
			stats.gaps.emplace_back( stats.lastRefusal, 1 );
	}

	void ReleaseGroup( Group &group, CompletionToken after, bool recycle = false )
	{
		if ( device )
		{
			if ( group.group.IsValid() )
				(void)device->Release( group.group, after );
			auto releaseBuffer = [&]( GroupResources::Buffer buffer )
			{
				if ( buffer.id.IsValid() && buffer.size != 0 )
				{
					if ( recycle )
						groupResources.Retire( *device, buffer, after );
					else
						(void)device->Release( buffer.id, after );
				}
			};
			releaseBuffer( group.constants );
			for ( const auto &buffer : group.storage )
				releaseBuffer( buffer );
			for ( SamplerId sampler : group.samplers )
				(void)device->Release( sampler, after );
		}
		group = Group();
	}

	void Release( Resources &old, CompletionToken after )
	{
		if ( device )
		{
			for ( Resources::Material &m : old.materials )
				ReleaseGroup( m.group, after );
			for ( Resources::Material &m : old.modelMaterials )
				ReleaseGroup( m.group, after );
			for ( auto &[key, m] : old.dynamicMaterials )
				ReleaseGroup( m.group, after );
			for ( Resources::Material &m : old.prepassMaterials )
				ReleaseGroup( m.group, after );
			for ( Resources::Material &m : old.prepassModelMaterials )
				ReleaseGroup( m.group, after );
			for ( TextureId texture : { old.prepassDepth, old.prepassNormal } )
			{
				if ( texture.IsValid() )
					(void)device->Release( texture, after );
			}
			for ( auto &[key, group] : old.drawGroups )
				ReleaseGroup( group, after );
			for ( auto &[key, group] : old.frameGroups )
				ReleaseGroup( group, after );
			for ( auto &[key, group] : old.viewGroups )
				ReleaseGroup( group, after );
			for ( auto &view : old.litViews )
				ReleaseGroup( view.group, after );
			if ( old.neutralWhite.IsValid() )
				(void)device->Release( old.neutralWhite, after );
			if ( old.neutralCube.IsValid() )
				(void)device->Release( old.neutralCube, after );
			if ( old.neutralCubeArray.IsValid() )
				(void)device->Release( old.neutralCubeArray, after );
			if ( old.neutralDepth.IsValid() )
				(void)device->Release( old.neutralDepth, after );
			if ( old.neutralArray.IsValid() )
				(void)device->Release( old.neutralArray, after );
			for ( auto &[name, texture] : old.stageTextures )
				(void)device->Release( texture, after );
			if ( old.neutralStaging.IsValid() )
				(void)device->Release( old.neutralStaging, after );
			if ( old.vertices.IsValid() )
				(void)device->Release( old.vertices, after );
			if ( old.indices.IsValid() )
				(void)device->Release( old.indices, after );
		}
		old = Resources();
	}
};

} // namespace render::pass::world
