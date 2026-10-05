//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5); see world_pass.h. Names no
//			material family: programs come from the one resolver.
//
//=============================================================================//

#include "render/pass/world/world_pass.h"
#include "group_resources.h"

#include "render/device/errors.h"
#include "render/frame/debug_specialization.h"
#include "render/material/draw_program.h"
#include "render/material/material_programs.h"
#include "render/material/program_resolver.h"
#include "render/material/scene_terms.h"
#include "render/material/surface_program.h"
#include "render/material/vmt_import.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <mutex>
#include <span>
#include <tuple>

namespace render::pass::world
{

namespace
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

bool SurfaceSelected(
    const std::optional<std::vector<std::uint32_t>> &selection, std::uint32_t surface )
{
	return !selection || std::binary_search( selection->begin(), selection->end(), surface );
}

bool ValidSurfaceSelection(
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
		std::string failure; // why, when failed
		// The material system handle of the view render target the program
		// reads through its view group (ResolvedProgram::viewInputs: the
		// water point's planar reflection), or 0.
		int viewInput = 0;
	};
	std::vector<Material> materials;
	std::vector<Material> modelMaterials;
	// Only snapshots used in the current recorded frame are retained. Replaced
	// groups retire behind the same submitted token as transient geometry.
	std::map<std::string, Material> dynamicMaterials;
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
	TextureId neutralArray; // two layers, for a binding read as an array
	TextureId neutralDepth; // far depth for an absent shadow atlas
	BufferId neutralStaging;
	bool uploaded = false;
	// A world stage's textures (WorldStage), by the names its groups use,
	// made at the first slot and updated in place; the stage revisions
	// they hold.
	std::map<std::string, TextureId> stageTextures;
	std::map<std::string, TextureDesc> stageDescs;
	bool stageMade = false;
	std::uint64_t stageLightmapRevision = 0;
	std::uint64_t stageProbeRevision = 0;
	std::uint64_t stageChangeRevision = 0;
};

// The names a world stage's groups use for its textures.
constexpr const char *kStageLightmap = "stage:lightmap";
constexpr const char *kStageGradient = "stage:lightmap-gradient";
constexpr const char *kStageIndirect = "stage:lightmap-indirect";
constexpr const char *kStageIndirectGradient = "stage:lightmap-indirect-gradient";
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
std::uint32_t StageTerms( const WorldStage &stage, bool runtimeDirect )
{
	material::SceneTermInputs inputs;
	inputs.runtimeDirect = runtimeDirect;
	inputs.indirectLightmap = !stage.indirect.empty();
	inputs.totalDirectionalLightmap = stage.lightmap.Directional();
	inputs.indirectDirectionalLightmap = !stage.indirectGradient.empty();
	inputs.probeVolume = stage.probes.has_value();
	inputs.probeBounce = stage.probes.has_value(); // the change atlas is always supplied
	inputs.reflectionProbes = !stage.reflectionProbes.empty();
	inputs.ambientOcclusion = true; // the neutral view binds one when AO is off
	return material::SceneTerms( inputs );
}

std::string Lower( std::string text )
{
	for ( char &c : text )
		c = char( std::tolower( static_cast<unsigned char>( c ) ) );
	return text;
}

foundation::Expected<Claimed, std::string> MapWorldMaterial( const WorldMaterial &source )
{
	if ( source.hasProxy )
		return foundation::MakeUnexpected(
		    std::string( "selected material needs a live proxy handoff" ) );
	Claimed claimed;
	claimed.blended = source.translucent;
	std::vector<material::VmtPair> variables;
	for ( const auto &[key, value] : source.variables )
		variables.push_back( { key, value } );
	auto mapped = material::MapVariables( source.shader, std::move( variables ), {} );
	if ( !mapped )
		return foundation::MakeUnexpected( "shader " + source.shader + " does not map" );
	claimed.desc = std::move( mapped ).Value();
	for ( const auto &[key, value] : source.defaults )
		claimed.desc.declaredDefaults.push_back( { key, value } );
	for ( const material::MaterialValue &value : claimed.desc.values )
	{
		if ( value.kind != material::ValueKind::kTexture )
			continue;
		int handle = 0;
		for ( const auto &[key, candidate] : source.textures )
		{
			if ( Lower( key ) == Lower( value.key ) )
			{
				handle = candidate;
				break;
			}
		}
		// Name every texture, even one the engine did not load (a model's lazy
		// bump map before its first draw) or whose handle is 0. A 0-handle
		// takes the group's neutral texture of that dimension instead of
		// failing the whole material by name, matching the world path's
		// "content lacks" fallback.
		claimed.handles[value.text] = handle;
	}
	return claimed;
}

std::string MaterialSnapshotKey( const WorldMaterial &source )
{
	std::string key;
	auto append = [&]( const std::string &value )
	{
		key += std::to_string( value.size() ) + ":" + value;
	};
	append( source.name );
	append( source.shader );
	key += source.mesh ? "M" : "W";
	key += source.translucent ? "T" : "O";
	key += source.hasProxy ? "P" : "-";
	for ( const auto *values : { &source.variables, &source.defaults } )
	{
		key += std::to_string( values->size() ) + ":";
		for ( const auto &[name, value] : *values )
		{
			append( name );
			append( value );
		}
	}
	for ( const auto &[name, handle] : source.textures )
	{
		append( name );
		append( std::to_string( handle ) );
	}
	return key;
}

// Whether serial a was issued before serial b (serials wrap within the
// tag's serial bits).
bool IssuedBefore( std::uint32_t a, std::uint32_t b )
{
	const std::uint32_t distance = ( b - a ) & kWorldSerialMask;
	return distance != 0 && distance < ( ( kWorldSerialMask + 1 ) >> 1 );
}

std::string PageName( int handle )
{
	return "lightmap-page:" + std::to_string( handle );
}

std::uint32_t StaticMaterial( const WorldData::StaticMesh &mesh,
    const WorldData::StaticInstance &instance, std::uint32_t surface )
{
	if ( surface >= mesh.surfaces.size() )
		return ~0u;
	if ( mesh.skinMaterials.empty() )
		return instance.skin == 0 ? mesh.surfaces[surface].material : ~0u;
	if ( instance.skin >= mesh.skinMaterials.size() ||
	     surface >= mesh.skinMaterials[instance.skin].size() )
		return ~0u;
	return mesh.skinMaterials[instance.skin][surface];
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
bool ModelLevelPinned( const WorldData::StaticMesh &mesh, std::uint32_t lod )
{
	return mesh.posed || lod + 1 >= mesh.lodCount();
}

// A surface's index range is its own level's, from zero: a surface no level
// lists, or one whose range leaves that level's indices, is not drawable.
bool LevelSurfaceDrawable(
    const WorldData::StaticMesh &mesh, std::uint32_t surface, const WorldSurface &range )
{
	const std::uint32_t lod = mesh.LodOfSurface( surface );
	if ( lod == ~0u || lod >= mesh.lodCount() || !mesh.lods[lod].Drawable() )
		return false;
	return range.firstIndex <= mesh.lods[lod].indexCount &&
	       range.indexCount <= mesh.lods[lod].indexCount - range.firstIndex;
}

} // namespace

LightmapPages SplitLightmapLayer(
    std::span<const std::byte> layer, std::uint32_t width, std::uint32_t height )
{
	constexpr std::size_t kTexel = 8; // RGBA16F
	LightmapPages pages;
	if ( layer.size() != std::size_t( width ) * height * kTexel )
		return pages; // no page: the caller reports it
	pages.height = height;
	if ( width != 2 * height )
	{
		pages.width = width;
		pages.flat.assign( layer.begin(), layer.end() );
		return pages;
	}
	pages.width = height;
	const std::size_t row = std::size_t( width ) * kTexel;
	const std::size_t half = std::size_t( pages.width ) * kTexel;
	pages.flat.resize( std::size_t( height ) * half );
	pages.gradient.resize( std::size_t( height ) * half );
	for ( std::uint32_t y = 0; y < height; ++y )
	{
		const std::byte *from = layer.data() + std::size_t( y ) * row;
		std::copy( from, from + half, pages.flat.data() + std::size_t( y ) * half );
		std::copy( from + half, from + row, pages.gradient.data() + std::size_t( y ) * half );
	}
	return pages;
}

// The vertices of one world surface that a mip footprint projects: each
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

struct WorldPass::State
{
	IRenderDevice2 *device = nullptr;              // the device the resources live on
	std::span<const std::uint32_t> fragmentModule; // SetSurfaceFragmentModule
	// The last screen output written on this sequence. Another camera or output
	// invalidates reuse even when a format's own prepass textures still exist.
	std::uint64_t screenFrame = 0;
	TextureId screenDepth;
	TextureId screenOutput;
	std::array<float, 48> screenInputs = {};

	IModelLevelSource *levelSource = nullptr; // SetModelLevelSource; null retains staging

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
	struct Queued
	{
		std::uint32_t serial = 0;
		std::uint64_t generation = 0; // the world the view was queued against
		std::uint64_t recordedStream = 0;
		WorldView view;
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
	// MapWorldMaterial of each dynamic draw's material, by MaterialSnapshotKey
	// (every input of the mapping). The claim (main thread) and the record
	// (render sequence) both read it; entries are immutable, so a cleared
	// table leaves borrowers their entry.
	using MappedMaterial = foundation::Expected<Claimed, std::string>;
	std::mutex mappedLock;
	std::map<std::string, std::shared_ptr<const MappedMaterial>> mapped;
	std::shared_ptr<const MappedMaterial> Mapped( const WorldMaterial &source, std::string &key );
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
	std::vector<std::pair<std::uint64_t, Group>> retiredGroups;
	GroupResources groupResources;
	std::vector<std::pair<std::uint64_t, TextureId>> retiredTextures;

	void Fail( const std::string &why )
	{
		std::lock_guard<std::mutex> guard( lock );
		++stats.viewsFailed;
		stats.lastFailure = why;
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

WorldPass::WorldPass() : m_State( std::make_unique<State>() )
{
}

// Without ReleaseDevice the device may be gone: the handles are dropped.
WorldPass::~WorldPass() = default;

void WorldPass::SetSurfaceFragmentModule( std::span<const std::uint32_t> module )
{
	m_State->fragmentModule = module;
}

void WorldPass::SetWorld( WorldData data )
{
	State &s = *m_State;
	auto claims = std::make_shared<std::vector<Claimed>>();
	claims->reserve( data.materials.size() );
	std::map<std::string, std::uint32_t> gaps;
	WorldStats counts;
	for ( const WorldMaterial &source : data.materials )
	{
		Claimed claimed;
		std::string gap;
		auto mapped = MapWorldMaterial( source );
		if ( !mapped )
			gap = mapped.Error();
		else
		{
			claimed = std::move( mapped ).Value();
			// Static meshes use the same surface program with model vertices,
			// probes and clustered direct light instead of a lightmap page.
			auto blend = source.mesh
			                 ? material::ClaimForMesh( claimed.desc,
			                       data.stage && !data.stage->reflectionProbes.empty(),
			                       data.stage != nullptr )
			                 : material::ClaimForDrawing( claimed.desc, data.stage != nullptr,
			                       nullptr,
			                       data.stage && !data.stage->reflectionProbes.empty() );
			if ( !blend )
				gap = claimed.desc.family + ": " + blend.Error();
			else if ( !source.mesh && blend.Value() != BlendMode::kOpaque )
				gap = claimed.desc.family + ": blended (drawn in the translucent stage)";
			else
			{
				claimed.blended |= blend.Value() != BlendMode::kOpaque;
				claimed.draws = true;
				claimed.opaqueBatch =
				    !claimed.blended && material::SupportsOpaqueBatch( claimed.desc, source.mesh );
			}
		}
		if ( !gap.empty() )
		{
			++gaps["material " + source.name + ": " + gap];
		}
		counts.claimedMaterials += claimed.draws ? 1u : 0u;
		claims->push_back( std::move( claimed ) );
	}
	counts.materials = std::uint32_t( data.materials.size() );
	counts.surfaces = std::uint32_t( data.surfaces.size() );
	std::vector<std::uint32_t> perMaterial( data.materials.size(), 0 );
	for ( const WorldSurface &surface : data.surfaces )
	{
		if ( surface.material < claims->size() && ( *claims )[surface.material].draws )
		{
			++counts.claimedSurfaces;
			++perMaterial[surface.material];
		}
	}
	std::vector<std::pair<std::string, std::uint32_t>> claimedNames;
	for ( std::size_t m = 0; m < data.materials.size(); ++m )
	{
		if ( !( *claims )[m].draws )
			continue;
		// How many of a claimed material's keys the model does not read (each
		// at its neutral value, or the material would not be claimed).
		// The program that draws it first (cl_render_debug_view_program's
		// name for it): the family the resolver dispatches the material to.
		std::string name = ( *claims )[m].desc.family + " " + data.materials[m].name;
		if ( const std::size_t unread = ( *claims )[m].desc.unmapped.size() )
			name += " (" + std::to_string( unread ) + " unread keys at neutral)";
		claimedNames.emplace_back( std::move( name ), perMaterial[m] );
	}
	std::vector<std::pair<std::string, std::uint32_t>> ranked( gaps.begin(), gaps.end() );
	std::stable_sort( ranked.begin(), ranked.end(),
	    []( const auto &a, const auto &b )
	    {
		    return a.second > b.second;
	    } );
	auto footprints = std::make_shared<std::vector<SurfaceFootprintGeometry>>();
	footprints->resize( data.surfaces.size() );
	{
		std::vector<std::uint32_t> mark( data.vertices.size(), ~0u );
		for ( std::size_t i = 0; i < data.surfaces.size(); ++i )
		{
			const WorldSurface &surface = data.surfaces[i];
			SurfaceFootprintGeometry &out = ( *footprints )[i];
			if ( surface.firstIndex > data.indices.size() ||
			     surface.indexCount > data.indices.size() - surface.firstIndex )
				continue;
			bool valid = true;
			out.minU = out.minV = std::numeric_limits<float>::max();
			out.maxU = out.maxV = std::numeric_limits<float>::lowest();
			for ( std::uint32_t k = 0; k < surface.indexCount && valid; ++k )
			{
				const std::uint32_t vertexIndex = data.indices[surface.firstIndex + k];
				if ( vertexIndex >= data.vertices.size() )
				{
					valid = false;
					break;
				}
				const auto &vertex = data.vertices[vertexIndex];
				if ( !std::isfinite( vertex.uv[0] ) || !std::isfinite( vertex.uv[1] ) )
				{
					valid = false;
					break;
				}
				out.minU = std::min( out.minU, vertex.uv[0] );
				out.minV = std::min( out.minV, vertex.uv[1] );
				out.maxU = std::max( out.maxU, vertex.uv[0] );
				out.maxV = std::max( out.maxV, vertex.uv[1] );
				if ( mark[vertexIndex] != std::uint32_t( i ) )
				{
					mark[vertexIndex] = std::uint32_t( i );
					out.positions.push_back(
					    { vertex.position[0], vertex.position[1], vertex.position[2] } );
				}
			}
			out.valid = valid && surface.indexCount != 0;
			if ( !out.valid )
				out.positions.clear();
		}
	}
	auto modelBounds = std::make_shared<std::vector<std::vector<SurfaceFootprintBounds>>>(
	    data.staticMeshes.size() );
	for ( std::size_t meshId = 0; meshId < data.staticMeshes.size(); ++meshId )
	{
		const WorldData::StaticMesh &mesh = data.staticMeshes[meshId];
		auto &bounds = ( *modelBounds )[meshId];
		bounds.resize( mesh.surfaces.size() );
		for ( std::size_t i = 0; i < mesh.surfaces.size(); ++i )
		{
			if ( i >= mesh.surfaceLods.size() || mesh.surfaceLods[i] >= mesh.lods.size() )
				continue;
			const WorldData::StaticMeshLod &level = mesh.lods[mesh.surfaceLods[i]];
			if ( level.vertices && level.indices )
				bounds[i] =
				    MeasureFootprintBounds( mesh.surfaces[i], *level.vertices, *level.indices );
		}
	}
	std::lock_guard<std::mutex> guard( s.lock );
	s.world = std::make_shared<const WorldData>( std::move( data ) );
	s.claims = std::move( claims );
	s.footprintGeometry = std::move( footprints );
	s.modelFootprintBounds = std::move( modelBounds );
	++s.generation;
	// A new world starts from its stage's own lighting.
	s.stageLightmap.reset();
	s.stageLightmapPatches.clear();
	s.stageLightmapBaseRevision = s.stageLightmapRevision;
	s.stageProbeAtlas.reset();
	s.stageProbePatches.clear();
	s.stageProbeBaseRevision = s.stageProbeRevision;
	s.stageChangeBase.clear();
	s.stagePatches.clear();
	s.stageTable.reset();
	s.stageBaseRevision = ++s.stageChangeRevision;
	// Queued views stay with their world's generation: in queued mode the
	// slots of a frame that straddles a level change record after it, and
	// skip their views (an earlier world's) rather than fail.
	s.stats.materials = counts.materials;
	s.stats.claimedMaterials = counts.claimedMaterials;
	s.stats.surfaces = counts.surfaces;
	s.stats.claimedSurfaces = counts.claimedSurfaces;
	s.stats.gaps = std::move( ranked );
	s.stats.claimed = std::move( claimedNames );
}

void WorldPass::ClearWorld()
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	s.world.reset();
	s.claims.reset();
	s.footprintGeometry.reset();
	s.modelFootprintBounds.reset();
	++s.generation;
	// Queued views stay with their world's generation: in queued mode the
	// slots of a frame that straddles a level change record after it, and
	// skip their views (an earlier world's) rather than fail.
}

void WorldPass::SetModelLevelSource( IModelLevelSource *source )
{
	m_State->levelSource = source;
}

std::vector<std::pair<std::uint32_t, std::uint32_t>> WorldPass::DrainReleasedStaging()
{
	State &s = *m_State;
	std::vector<std::pair<std::uint32_t, std::uint32_t>> released;
	for ( std::uint32_t m = 0; m < s.models.models.size(); ++m )
	{
		auto &model = s.models.models[m];
		for ( std::uint32_t l = 0; l < model.size(); ++l )
		{
			if ( model[l].stagingReleased )
			{
				released.emplace_back( m, l );
				model[l].stagingReleased = false;
			}
		}
	}
	return released;
}

void WorldPass::SetStageLightmap( LightmapPages pages )
{
	State &s = *m_State;
	auto shared = std::make_shared<LightmapPages>( std::move( pages ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageLightmap = std::move( shared );
	s.stageLightmapPatches.clear();
	s.stageLightmapBaseRevision = ++s.stageLightmapRevision;
}

void WorldPass::SetStageProbeVolume(
    std::vector<std::byte> atlas, std::vector<std::byte> change, StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedAtlas = std::make_shared<std::vector<std::byte>>( std::move( atlas ) );
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageProbeAtlas = std::move( sharedAtlas );
	s.stageProbePatches.clear();
	s.stageProbeBaseRevision = ++s.stageProbeRevision;
	s.stageChangeBase = std::move( change );
	s.stagePatches.clear();
	s.stageTable = std::move( sharedTable );
	s.stageBaseRevision = ++s.stageChangeRevision;
}

void WorldPass::SetStageProbeRegions( std::vector<StageRegion> regions,
    std::vector<std::byte> atlasTexels, std::vector<std::byte> changeTexels,
    StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	if ( !atlasTexels.empty() )
		s.stageProbePatches.push_back(
		    { ++s.stageProbeRevision, regions, std::move( atlasTexels ) } );
	s.stageTable = std::move( sharedTable );
	if ( changeTexels.empty() )
		regions.clear();
	s.stagePatches.push_back(
	    { ++s.stageChangeRevision, std::move( regions ), std::move( changeTexels ) } );
}

void WorldPass::SetStageLightmapRegions(
    std::vector<StageRegion> regions, std::vector<std::byte> texels )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageLightmapPatches.push_back(
	    { ++s.stageLightmapRevision, std::move( regions ), std::move( texels ) } );
}

void WorldPass::SetStageChange( std::vector<std::byte> change, StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageChangeBase = std::move( change );
	s.stagePatches.clear();
	s.stageTable = std::move( sharedTable );
	s.stageBaseRevision = ++s.stageChangeRevision;
}

bool WorldPass::Draws( std::uint32_t material ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.claims && material < s.claims->size() && ( *s.claims )[material].draws;
}

bool WorldPass::DrawsStaticInstance( std::uint32_t instance ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( !s.world || !s.claims || instance >= s.world->staticInstances.size() )
		return false;
	const WorldData::StaticInstance &placement = s.world->staticInstances[instance];
	const std::uint32_t mesh = placement.mesh;
	if ( mesh >= s.world->staticMeshes.size() )
		return false;
	const WorldData::StaticMesh &data = s.world->staticMeshes[mesh];
	if ( !ValidSurfaceSelection( placement.surfaceSelection, data.surfaces.size() ) )
		return false;
	if ( placement.surfaceSelection && placement.surfaceSelection->empty() )
		return true;
	if ( data.surfaces.empty() )
		return false;
	for ( std::uint32_t i = 0; i < data.surfaces.size(); ++i )
	{
		if ( !SurfaceSelected( placement.surfaceSelection, i ) )
			continue;
		const WorldSurface &surface = data.surfaces[i];
		const std::uint32_t material = StaticMaterial( data, placement, i );
		if ( !LevelSurfaceDrawable( data, i, surface ) || material >= s.claims->size() ||
		     !( *s.claims )[material].draws || !s.world->materials[material].mesh )
			return false;
	}
	return true;
}

bool WorldPass::DrawsPosedModel( std::uint32_t meshId, std::uint32_t skin,
    RenderCoreDrawPhase phase,
    const std::optional<std::vector<std::uint32_t>> &surfaceSelection ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( !s.world || !s.claims || meshId >= s.world->staticMeshes.size() )
		return false;
	const WorldData::StaticMesh &mesh = s.world->staticMeshes[meshId];
	if ( !ValidSurfaceSelection( surfaceSelection, mesh.surfaces.size() ) )
		return false;
	if ( surfaceSelection && surfaceSelection->empty() )
		return true;
	if ( mesh.surfaces.empty() )
		return false;
	WorldData::StaticInstance instance;
	instance.mesh = meshId;
	instance.skin = skin;
	bool hasSurface = false;
	for ( std::uint32_t i = 0; i < mesh.surfaces.size(); ++i )
	{
		if ( !SurfaceSelected( surfaceSelection, i ) )
			continue;
		const WorldSurface &surface = mesh.surfaces[i];
		const std::uint32_t material = StaticMaterial( mesh, instance, i );
		if ( material >= s.claims->size() )
			return false;
		const bool blended = ( *s.claims )[material].blended;
		if ( ( phase == RenderCoreDrawPhase::kOpaque && blended ) ||
		     ( phase == RenderCoreDrawPhase::kBlended && !blended ) )
			continue;
		hasSurface = true;
		if ( !LevelSurfaceDrawable( mesh, i, surface ) || material >= s.claims->size() ||
		     !( *s.claims )[material].draws || !s.world->materials[material].mesh )
			return false;
	}
	return hasSurface || ( surfaceSelection && surfaceSelection->empty() );
}

std::uint32_t WorldPass::QueueView( WorldView view )
{
	State &s = *m_State;
	if ( view.surfaces.empty() && view.staticInstances.empty() && view.posedModels.empty() &&
	     view.dynamicDraws.empty() )
		return 0;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( !s.world )
		return 0;
	for ( WorldView::StaticInstance &draw : view.staticInstances )
	{
		if ( draw.instance >= s.world->staticInstances.size() )
		{
			s.stats.lastRefusal = "a static model names an instance the world does not have";
			return 0;
		}
		const WorldData::StaticInstance &instance = s.world->staticInstances[draw.instance];
		if ( !draw.surfaceSelection )
			draw.surfaceSelection = instance.surfaceSelection;
		if ( instance.mesh >= s.world->staticMeshes.size() ||
		     !ValidSurfaceSelection(
		         draw.surfaceSelection, s.world->staticMeshes[instance.mesh].surfaces.size() ) )
		{
			s.stats.lastRefusal = "a static model has an invalid surface selection";
			return 0;
		}
	}
	for ( const WorldView::PosedModel &pose : view.posedModels )
	{
		if ( pose.surfaceSelection && ( pose.mesh >= s.world->staticMeshes.size() ||
		                                  !ValidSurfaceSelection( pose.surfaceSelection,
		                                      s.world->staticMeshes[pose.mesh].surfaces.size() ) ) )
		{
			s.stats.lastRefusal = "a posed model has an invalid surface selection";
			return 0;
		}
	}
	// A successful tag promises that the core draws this snapshot. Refuse
	// unsupported inputs before publishing a slot or allocating GPU resources;
	// claimed inputs that later lose a texture still fail on the render sequence.
	for ( const WorldView::DynamicDraw &draw : view.dynamicDraws )
	{
		std::string key;
		const auto entry = s.Mapped( draw.material, key );
		const State::MappedMaterial &mapped = *entry;
		std::string why;
		if ( !mapped )
			why = mapped.Error();
		else
		{
			bool requiresDepthAlpha = false;
			auto claim = draw.material.mesh
			                 ? material::ClaimForMesh( mapped.Value().desc,
			                       s.world->stage && !s.world->stage->reflectionProbes.empty(),
			                       s.world->stage != nullptr )
			                 : material::ClaimForDrawing( mapped.Value().desc,
			                       s.world->stage != nullptr, &requiresDepthAlpha,
			                       s.world->stage && !s.world->stage->reflectionProbes.empty() );
			if ( !claim )
				why = claim.Error();
			else if ( requiresDepthAlpha &&
			          ( view.depthAlphaHandle <= 0 || !std::isfinite( view.depthAlphaRange ) ||
			              view.depthAlphaRange <= 0.0f ) )
				why = "$depthblend needs a captured depth-alpha texture and positive range";
		}
		if ( !why.empty() )
		{
			++s.stats.dynamicDrawsRefused;
			s.stats.lastRefusal = "material " + draw.material.name + ": " + why;
			auto gap = std::find_if( s.stats.gaps.begin(), s.stats.gaps.end(),
			    [&]( const auto &entry )
			    {
				    return entry.first == s.stats.lastRefusal;
			    } );
			if ( gap != s.stats.gaps.end() )
				++gap->second;
			else if ( s.stats.gaps.size() < 256 )
				s.stats.gaps.emplace_back( s.stats.lastRefusal, 1 );
			return 0;
		}
	}
	constexpr std::size_t kMaxQueued = 8192;
	if ( s.views.size() >= kMaxQueued )
	{
		++s.stats.viewsFailed;
		s.stats.lastFailure =
		    "the core view queue is full; no previously accepted work was discarded";
		return 0;
	}
	// Prefetch non-resident levels whose staging was released, so the
	// source can start rebuilding bytes before the render sequence needs
	// them. We don't know the exact LOD at queue time, so prefetch every
	// non-resident level of each referenced mesh.
	if ( s.levelSource )
	{
		auto prefetchMesh = [&]( std::uint32_t meshId )
		{
			if ( meshId >= s.models.models.size() )
				return;
			const auto &levels = s.models.models[meshId];
			for ( std::uint32_t lod = 0; lod < levels.size(); ++lod )
			{
				if ( levels[lod].resident )
					continue;
				const WorldData::StaticMeshLod &src = s.world->staticMeshes[meshId].lods[lod];
				if ( !src.vertices && !src.indices )
					s.levelSource->PrefetchLevel( meshId, lod );
			}
		};
		for ( const WorldView::StaticInstance &draw : view.staticInstances )
		{
			const WorldData::StaticInstance &inst = s.world->staticInstances[draw.instance];
			prefetchMesh( inst.mesh );
		}
		for ( const WorldView::PosedModel &pose : view.posedModels )
			prefetchMesh( pose.mesh );
	}
	s.stats.staticInstancesQueued += view.staticInstances.size();
	s.stats.posedModelsQueued += view.posedModels.size();
	const std::uint32_t serial = s.nextSerial;
	s.nextSerial = ( s.nextSerial + 1 ) & kWorldSerialMask;
	if ( s.nextSerial == 0 )
		s.nextSerial = 1;
	s.views.push_back( { serial, s.generation, 0, std::move( view ) } );
	++s.stats.viewsQueued;
	return kWorldTag | serial;
}

std::size_t WorldPass::OpaqueBatchSize(
    std::span<const std::uint32_t> tags, std::uint64_t streamEpoch ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.OpaqueBatchSize( tags, streamEpoch );
}

std::shared_ptr<const WorldPass::State::MappedMaterial> WorldPass::State::Mapped(
    const WorldMaterial &source, std::string &key )
{
	// Bounded: snapshot values that change every frame would otherwise grow it.
	constexpr std::size_t kMaxMapped = 4096;
	key = MaterialSnapshotKey( source );
	{
		std::lock_guard<std::mutex> guard( mappedLock );
		if ( auto found = mapped.find( key ); found != mapped.end() )
			return found->second;
	}
	auto entry = std::make_shared<const MappedMaterial>( MapWorldMaterial( source ) );
	std::lock_guard<std::mutex> guard( mappedLock );
	if ( mapped.size() >= kMaxMapped )
		mapped.clear();
	return mapped.emplace( key, entry ).first->second;
}

std::size_t WorldPass::State::OpaqueBatchSize(
    std::span<const std::uint32_t> tags, std::uint64_t streamEpoch ) const
{
	if ( tags.empty() )
		return 0;
	const State &s = *this;
	if ( !s.world || !s.world->stage || !s.claims )
		return 1;
	const auto lookup = [&]( std::uint32_t tag ) -> const WorldView *
	{
		if ( !IsWorldTag( tag ) || !( tag & kWorldSerialMask ) )
			return nullptr;
		const auto serial = tag & kWorldSerialMask;
		for ( const auto *queue : { &s.recorded, &s.views } )
		{
			for ( const auto &entry : *queue )
			{
				if ( entry.serial == serial && entry.generation == s.generation &&
				     ( queue == &s.views || !streamEpoch || entry.recordedStream == streamEpoch ) )
					return &entry.view;
			}
		}
		return nullptr;
	};
	const auto materialEligible = [&]( std::uint32_t id )
	{
		return id < s.claims->size() && ( *s.claims )[id].opaqueBatch;
	};
	const auto eligible = [&]( const WorldView &view )
	{
		// Debug draws retain their individual cohort identity and bisection.
		if ( !view.hostFrame || !view.dynamicDraws.empty() || view.debug.view ||
		     view.debug.legacy != frame::DebugLegacy::kOff )
			return false;
		for ( auto surface : view.surfaces )
			if ( surface >= s.world->surfaces.size() ||
			     !materialEligible( s.world->surfaces[surface].material ) )
				return false;
		const auto meshEligible = [&]( const WorldData::StaticInstance &instance,
		                              const auto &selection, RenderCoreDrawPhase phase )
		{
			if ( instance.mesh >= s.world->staticMeshes.size() )
				return false;
			const auto &mesh = s.world->staticMeshes[instance.mesh];
			for ( std::uint32_t surface = 0; surface < mesh.surfaces.size(); ++surface )
			{
				if ( !SurfaceSelected( selection, surface ) )
					continue;
				const auto material = StaticMaterial( mesh, instance, surface );
				if ( material >= s.claims->size() )
					return false;
				const bool blended = ( *s.claims )[material].blended;
				if ( phase == RenderCoreDrawPhase::kOpaque && blended )
					continue;
				if ( phase == RenderCoreDrawPhase::kBlended || !materialEligible( material ) )
					return false;
			}
			return true;
		};
		for ( const auto &draw : view.staticInstances )
			if ( draw.instance >= s.world->staticInstances.size() ||
			     !meshEligible( s.world->staticInstances[draw.instance], draw.surfaceSelection,
			         RenderCoreDrawPhase::kAll ) )
				return false;
		for ( const auto &pose : view.posedModels )
		{
			WorldData::StaticInstance instance;
			instance.mesh = pose.mesh;
			instance.skin = pose.skin;
			if ( !meshEligible( instance, pose.surfaceSelection, pose.phase ) )
				return false;
		}
		return true;
	};
	const WorldView *first = lookup( tags.front() );
	if ( !first || !eligible( *first ) )
		return 1;
	std::size_t count = 1;
	for ( ; count < tags.size(); ++count )
	{
		const WorldView *next = lookup( tags[count] );
		if ( !next || !next->surfaces.empty() || !eligible( *next ) ||
		     next->hostFrame != first->hostFrame || next->stageLighting != first->stageLighting ||
		     next->lights != first->lights || next->debug != first->debug ||
		     next->waterZOffset != first->waterZOffset ||
		     next->previousViewValid != first->previousViewValid ||
		     next->temporalView != first->temporalView ||
		     !std::equal( std::begin( first->motionToClip ), std::end( first->motionToClip ),
		         next->motionToClip ) ||
		     !std::equal( std::begin( first->previousToClip ), std::end( first->previousToClip ),
		         next->previousToClip ) ||
		     !std::equal( std::begin( first->toClip ), std::end( first->toClip ), next->toClip ) ||
		     !std::equal(
		         std::begin( first->viewRight ), std::end( first->viewRight ), next->viewRight ) ||
		     std::memcmp( &first->viewport, &next->viewport, sizeof( first->viewport ) ) != 0 ||
		     std::find( tags.begin(), tags.begin() + count, tags[count] ) != tags.begin() + count )
			break;
	}
	return count;
}

// Render sequence: one (model, level)'s buffers, uploaded from the world's
// staging on first use and again after a release. When a model level source
// is set, the staging shared_ptrs are released after the first upload and
// the source resupplies the bytes for a re-upload. False when the level has
// no geometry left to upload (a named failure; the caller draws nothing).
bool WorldPass::UploadModelLevel( State &state, device::IRenderDevice2 &device,
    device::CommandEncoder &encoder, const WorldData::StaticMesh &mesh, std::uint32_t meshId,
    std::uint32_t lod, std::uint64_t frame )
{
	if ( meshId >= state.models.models.size() || lod >= mesh.lodCount() )
		return false;
	const WorldData::StaticMeshLod &source = mesh.lods[lod];
	if ( !source.Drawable() )
		return false;
	State::ModelLevel &level = state.models.models[meshId][lod];
	level.lastUsedFrame = frame;
	if ( level.resident )
		return true;
	// The staging shared_ptrs: present on first upload, absent after the
	// composition released them (stagingReleased). A re-upload without
	// staging asks the source; without a source either, the level stays out.
	std::optional<IModelLevelSource::LevelGeometry> resupplied;
	std::span<const std::byte> vertexBytes;
	std::span<const std::byte> indexBytes;
	if ( source.vertices && source.indices )
	{
		vertexBytes = std::as_bytes( std::span( *source.vertices ) );
		indexBytes = std::as_bytes( std::span( *source.indices ) );
	}
	else if ( state.levelSource )
	{
		resupplied = state.levelSource->ResupplyLevel( meshId, lod );
		if ( !resupplied ||
		     resupplied->vertices.size() != source.vertexCount ||
		     resupplied->indices.size() != source.indexCount )
			return false;
		vertexBytes = std::as_bytes( std::span( resupplied->vertices ) );
		indexBytes = std::as_bytes( std::span( resupplied->indices ) );
	}
	else
	{
		return false;
	}
	if ( vertexBytes.size() !=
	         std::size_t( source.vertexCount ) * sizeof( material::SurfaceModelVertex ) ||
	     indexBytes.size() != std::size_t( source.indexCount ) * sizeof( std::uint32_t ) )
		return false;
	BufferDesc desc;
	desc.size = vertexBytes.size();
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
	desc.debugName = "model level vertices";
	auto vertices = device.CreateBuffer( desc );
	desc.size = indexBytes.size();
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndex };
	desc.debugName = "model level indices";
	auto indices = device.CreateBuffer( desc );
	if ( !vertices || !indices )
	{
		// A refused allocation keeps the level non-resident, so the next
		// frame that draws it tries again with no bytes dropped.
		if ( vertices )
			(void)device.Release( vertices.Value(), CompletionToken() );
		if ( indices )
			(void)device.Release( indices.Value(), CompletionToken() );
		return false;
	}
	level.vertices = vertices.Value();
	level.indices = indices.Value();
	level.resident = true;
	// The screen passes' prepass lists name the resident levels.
	++state.modelResidencyRevision;
	state.models.bytes += vertexBytes.size() + indexBytes.size();
	++state.stats.modelLevelUploads;
	encoder.TransitionBuffer(
	    level.vertices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( level.vertices, 0, vertexBytes );
	encoder.TransitionBuffer(
	    level.vertices, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
	encoder.TransitionBuffer(
	    level.indices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( level.indices, 0, indexBytes );
	encoder.TransitionBuffer(
	    level.indices, ResourceUsage::kCopyDestination, ResourceUsage::kIndex );
	// Mark the staging as releasable: the composition releases its own copies
	// on the next world publication. The source resupplies bytes for any
	// re-upload after that.
	if ( state.levelSource && !ModelLevelPinned( mesh, lod ) )
		level.stagingReleased = true;
	return true;
}

// Render sequence: adopt the world's model geometry (its (model, level)
// allocations survive a target format change and a stage republication), then
// release the levels no view has selected for kModelLevelIdleFrames, once per
// recorded frame.
void WorldPass::SweepModelResidency(
    State &state, const WorldData &world, const WorldTarget &target )
{
	// Reuse the resident levels only for a world that declares the same model
	// geometry revision: revision zero is "unversioned", so such a world (a lab
	// scene, a test fixture) uploads its own levels rather than inheriting
	// another world's buffers.
	bool sameGeometry = world.modelsRevision != 0 &&
	                    state.models.modelsRevision == world.modelsRevision &&
	                    state.models.models.size() == world.staticMeshes.size();
	for ( std::size_t mesh = 0; sameGeometry && mesh < world.staticMeshes.size(); ++mesh )
		sameGeometry = state.models.models[mesh].size() == world.staticMeshes[mesh].lodCount();
	if ( !sameGeometry )
	{
		// Another world's models: every level's buffers go with it, released at
		// a slot whose submitted token covers the frames that read them.
		for ( auto &model : state.models.models )
			for ( State::ModelLevel &level : model )
			{
				if ( level.vertices.IsValid() )
					state.retiredModelBuffers.push_back( { 0, level.vertices } );
				if ( level.indices.IsValid() )
					state.retiredModelBuffers.push_back( { 0, level.indices } );
			}
		state.models = State::ModelGeometry();
		// A new models revision replaces every level: the screen passes'
		// prepass lists are keyed by this too.
		++state.modelResidencyRevision;
		state.models.modelsRevision = world.modelsRevision;
		state.models.models.resize( world.staticMeshes.size() );
		for ( std::size_t mesh = 0; mesh < world.staticMeshes.size(); ++mesh )
			state.models.models[mesh].resize( world.staticMeshes[mesh].lodCount() );
	}
	if ( target.frame == 0 || state.residencyFrame == target.frame )
		return;
	state.residencyFrame = target.frame;
	for ( std::size_t mesh = 0; mesh < state.models.models.size(); ++mesh )
	{
		const WorldData::StaticMesh &data = world.staticMeshes[mesh];
		for ( std::size_t lod = 0; lod < state.models.models[mesh].size(); ++lod )
		{
			State::ModelLevel &level = state.models.models[mesh][lod];
			if ( !level.resident )
				continue;
			if ( !ModelLevelPinned( data, std::uint32_t( lod ) ) && level.lastUsedFrame != 0 &&
			     target.frame > level.lastUsedFrame + kModelLevelIdleFrames )
			{
				state.retiredModelBuffers.push_back( { target.frame, level.vertices } );
				state.retiredModelBuffers.push_back( { target.frame, level.indices } );
				state.models.bytes -=
				    std::size_t( data.lods[lod].vertexCount ) *
				        sizeof( material::SurfaceModelVertex ) +
				    std::size_t( data.lods[lod].indexCount ) * sizeof( std::uint32_t );
				++state.models.releasedLevels;
				// The screen passes' prepass lists name the resident levels.
				++state.modelResidencyRevision;
				level = State::ModelLevel();
			}
		}
	}
}

// Render sequence: the residency report the frame's views can read (the levels
// resident now, their bytes, and the levels released this world). Once per
// recorded frame, after this frame's uploads.
void WorldPass::PublishModelResidency( State &state )
{
	if ( state.residencyFrame == 0 || state.residencyReportedFrame == state.residencyFrame )
		return;
	state.residencyReportedFrame = state.residencyFrame;
	std::uint32_t resident = 0;
	std::uint64_t stagingBytes = 0;
	for ( std::uint32_t m = 0; m < state.models.models.size(); ++m )
	{
		const auto &model = state.models.models[m];
		for ( std::uint32_t l = 0; l < model.size(); ++l )
		{
			resident += model[l].resident ? 1u : 0u;
			if ( state.world && m < state.world->staticMeshes.size() )
			{
				const WorldData::StaticMeshLod &src = state.world->staticMeshes[m].lods[l];
				if ( src.vertices )
					stagingBytes += src.vertices->size() * sizeof( material::SurfaceModelVertex );
				if ( src.indices )
					stagingBytes += src.indices->size() * sizeof( std::uint32_t );
			}
		}
	}
	state.stats.modelLevelsResident = resident;
	state.stats.modelBufferBytes = state.models.bytes;
	state.stats.modelStagingBytes = stagingBytes;
	state.stats.modelLevelsReleased = state.models.releasedLevels > 0xfffffffeu
	                                      ? 0xfffffffeu
	                                      : std::uint32_t( state.models.releasedLevels );
}

void WorldPass::ReleaseDevice( IRenderDevice2 &device )
{
	State &s = *m_State;
	if ( s.device != &device )
		return;
	for ( Resources &variant : s.variants )
		s.Release( variant, CompletionToken() );
	s.variants.clear();
	for ( auto &[frame, old] : s.retired )
		s.Release( old, CompletionToken() );
	s.retired.clear();
	for ( auto &[frame, buffer] : s.retiredBuffers )
		(void)device.Release( buffer, CompletionToken() );
	s.retiredBuffers.clear();
	for ( const State::RetiredModelBuffer &retired : s.retiredModelBuffers )
		(void)device.Release( retired.buffer, CompletionToken() );
	s.retiredModelBuffers.clear();
	// The world's model geometry went with the device. A view that selects a
	// level again needs the world republished: nothing else can name its
	// staging.
	for ( const auto &model : s.models.models )
		for ( const State::ModelLevel &level : model )
		{
			if ( level.vertices.IsValid() )
				(void)device.Release( level.vertices, CompletionToken() );
			if ( level.indices.IsValid() )
				(void)device.Release( level.indices, CompletionToken() );
		}
	s.models = State::ModelGeometry();
	++s.modelResidencyRevision;
	for ( auto &[frame, group] : s.retiredGroups )
		s.ReleaseGroup( group, CompletionToken() );
	s.retiredGroups.clear();
	for ( auto &[frame, texture] : s.retiredTextures )
		(void)device.Release( texture, CompletionToken() );
	s.retiredTextures.clear();
	s.groupResources.Release( device );
	(void)device.Poll();
	s.device = nullptr;
}

WorldStats WorldPass::Stats() const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.stats;
}

std::shared_ptr<const StageLightingInputs> WorldPass::LightingInputs(
    std::uint32_t tag, std::uint64_t streamEpoch ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const std::uint32_t serial = tag & kWorldSerialMask;
	for ( auto kept = s.recorded.rbegin(); kept != s.recorded.rend(); ++kept )
	{
		if ( kept->serial == serial && ( streamEpoch == 0 || kept->recordedStream == streamEpoch ) )
			return kept->view.stageLighting;
	}
	for ( const State::Queued &queued : s.views )
	{
		if ( queued.serial == serial )
			return queued.view.stageLighting;
	}
	return {};
}

bool WorldPass::TemporalView( std::uint32_t tag, std::uint64_t streamEpoch ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const std::uint32_t serial = tag & kWorldSerialMask;
	for ( auto kept = s.recorded.rbegin(); kept != s.recorded.rend(); ++kept )
		if ( kept->serial == serial && ( streamEpoch == 0 || kept->recordedStream == streamEpoch ) )
			return kept->view.temporalView != 0;
	for ( const State::Queued &queued : s.views )
		if ( queued.serial == serial )
			return queued.view.temporalView != 0;
	return false;
}

bool WorldPass::ViewDrawsWorldGeometry( std::uint32_t tag, std::uint64_t streamEpoch ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const std::uint32_t serial = tag & kWorldSerialMask;
	for ( auto kept = s.recorded.rbegin(); kept != s.recorded.rend(); ++kept )
		if ( kept->serial == serial && ( streamEpoch == 0 || kept->recordedStream == streamEpoch ) )
			return kept->view.drawsWorldGeometry;
	for ( const State::Queued &queued : s.views )
		if ( queued.serial == serial )
			return queued.view.drawsWorldGeometry;
	return true;
}

std::uint64_t WorldPass::Failures() const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.stats.viewsFailed;
}

void WorldPass::Record( std::uint32_t tag, CommandEncoder &encoder, const WorldTarget &target )
{
	RecordBatch( std::span( &tag, 1 ), encoder, target );
}

void WorldPass::RecordBatch(
    std::span<const std::uint32_t> tags, CommandEncoder &encoder, const WorldTarget &target )
{
	if ( tags.empty() )
		return;
	RecordSection preparation( encoder );
	preparation.Select( "prepare world queue" );
	State &s = *m_State;
	std::vector<std::size_t> staticCohorts, posedCohorts;
	std::shared_ptr<const WorldData> world;
	std::shared_ptr<const std::vector<Claimed>> claims;
	std::shared_ptr<const std::vector<SurfaceFootprintGeometry>> footprintGeometry;
	std::shared_ptr<const std::vector<std::vector<SurfaceFootprintBounds>>> modelFootprintBounds;
	std::uint64_t generation = 0;
	WorldView view;
	bool found = false;
	bool earlierWorld = false; // the slot's view was queued against an earlier world
	{
		std::lock_guard<std::mutex> guard( s.lock );
		if ( tags.size() > 1 && s.OpaqueBatchSize( tags, target.streamEpoch ) != tags.size() )
		{
			++s.stats.viewsFailed;
			s.stats.lastFailure = "an opaque batch crosses a view or material ordering boundary";
			return;
		}
		for ( std::size_t cohortIndex = 0; cohortIndex < tags.size(); ++cohortIndex )
		{
			const std::uint32_t serial = tags[cohortIndex] & kWorldSerialMask;
			WorldView cohort;
			found = false;
			// A slot recorded again (the same stream for a capture) draws the
			// cohort it drew the first time; one of an earlier world draws nothing
			// and leaves the queue (the next frame's views) alone.
			bool again = false;
			if ( target.streamEpoch != 0 )
				std::erase_if( s.recorded,
				    [&]( const State::Queued &kept )
				    {
					    return kept.recordedStream != target.streamEpoch;
				    } );
			for ( auto kept = s.recorded.rbegin(); kept != s.recorded.rend() && !again; ++kept )
			{
				if ( kept->serial == serial )
				{
					again = true;
					found = kept->generation == s.generation;
					earlierWorld = !found;
					cohort = kept->view;
				}
			}
			// World views are issued in main-thread stream order. Dynamic tickets
			// are issued during render-call replay, possibly after the main thread
			// has queued another frame: their serials do not establish stream order.
			std::uint64_t recordingFrame = 0;
			bool dynamic = false;
			for ( const State::Queued &queued : s.views )
			{
				if ( queued.serial == serial )
				{
					recordingFrame = queued.view.hostFrame;
					dynamic = !queued.view.dynamicDraws.empty();
				}
			}
			while ( !again && !dynamic && !s.views.empty() &&
			        s.views.front().view.dynamicDraws.empty() &&
			        IssuedBefore( s.views.front().serial, serial ) )
			{
				s.Drop( s.views.front(), recordingFrame );
				s.views.pop_front();
			}
			auto queued = std::find_if( s.views.begin(), s.views.end(),
			    [&]( const State::Queued &entry )
			    {
				    return entry.serial == serial;
			    } );
			if ( !again && queued != s.views.end() )
			{
				// Only against the world it was queued for (SetWorld also clears
				// the queue; this holds if a slot records across the change).
				found = queued->generation == s.generation;
				earlierWorld = !found;
				cohort = queued->view;
				if ( cohort.hostFrame != 0 &&
				     ( s.recordedFrames.empty() || s.recordedFrames.back() != cohort.hostFrame ) )
				{
					s.recordedFrames.push_back( cohort.hostFrame );
					constexpr std::size_t kFramesKept = 64;
					while ( s.recordedFrames.size() > kFramesKept )
						s.recordedFrames.pop_front();
				}
				queued->recordedStream = target.streamEpoch;
				s.recorded.push_back( std::move( *queued ) );
				s.views.erase( queued );
				// The complete stream is replayable, including every dynamic slot.
				// The queue accepts at most this many slots before recording starts.
				constexpr std::size_t kRecordedKept = 8192;
				while ( s.recorded.size() > kRecordedKept )
					s.recorded.pop_front();
			}
			if ( !found )
				break;
			staticCohorts.insert( staticCohorts.end(), cohort.staticInstances.size(), cohortIndex );
			posedCohorts.insert( posedCohorts.end(), cohort.posedModels.size(), cohortIndex );
			if ( cohortIndex == 0 )
				view = std::move( cohort );
			else
			{
				view.staticInstances.insert( view.staticInstances.end(),
				    std::make_move_iterator( cohort.staticInstances.begin() ),
				    std::make_move_iterator( cohort.staticInstances.end() ) );
				view.posedModels.insert( view.posedModels.end(),
				    std::make_move_iterator( cohort.posedModels.begin() ),
				    std::make_move_iterator( cohort.posedModels.end() ) );
			}
		}
		world = s.world;
		claims = s.claims;
		footprintGeometry = s.footprintGeometry;
		modelFootprintBounds = s.modelFootprintBounds;
		generation = s.generation;
	}
	if ( earlierWorld )
	{
		// A frame recorded across a level change: its views were the earlier
		// world's, which is gone. Nothing to draw, and nothing failed.
		std::lock_guard<std::mutex> guard( s.lock );
		++s.stats.viewsSkipped;
		return;
	}
	if ( target.overrideDepthRange )
	{
		view.viewport.minDepth = target.minDepth;
		view.viewport.maxDepth = target.maxDepth;
	}
	// A stage view's lights made when its slot records (WorldTarget::lights).
	if ( target.lights )
		view.lights = target.lights;
	if ( !found || !world || !claims )
	{
		s.Fail( "a slot names no queued view of the current world" );
		return;
	}
	if ( view.stageLighting && !view.lights )
	{
		s.Fail( "a claimed stage view lost its queued lighting inputs" );
		return;
	}
	if ( !target.device || !target.color.IsValid() || !target.depth.IsValid() || !target.textures ||
	     ( target.motion.IsValid() && !target.motionDepth.IsValid() ) )
	{
		std::string missing;
		for ( const auto &[absent, what] :
		    { std::pair{ !target.device, "device" }, { !target.color.IsValid(), "color" },
		        { !target.depth.IsValid(), "depth" }, { !target.textures, "texture source" },
		        { target.motion.IsValid() && !target.motionDepth.IsValid(), "temporal depth" } } )
		{
			if ( absent )
				missing += missing.empty() ? what : std::string( ", " ) + what;
		}
		s.Fail( "the slot's target has no " + missing + " (a render-target texture)" );
		return;
	}
	if ( target.temporalViewport && view.temporalView )
		target.temporalViewport( view.temporalView, view.viewport );
	preparation.Select( "prepare world resources" );
	if ( s.device != target.device )
	{
		// A new backend device: the old one's objects went with it.
		s.variants.clear();
		s.retired.clear();
		s.retiredBuffers.clear();
		s.retiredGroups.clear();
		s.retiredTextures.clear();
		// The old device's model levels went with it; the world they belong to
		// publishes its staging again (SweepModelResidency adopts it).
		s.retiredModelBuffers.clear();
		s.models = State::ModelGeometry();
		++s.modelResidencyRevision;
		s.groupResources = GroupResources();
		s.device = target.device;
	}
	IRenderDevice2 &device = *s.device;
	IWorldTextures &textures = *target.textures;

	// The world's device objects for these formats. Objects of an earlier
	// world, or a set pushed out, retire with this frame: slots earlier in
	// it may have used them, so they are released at a later frame's slot,
	// whose submitted token covers this frame.
	constexpr std::size_t kMaxVariants = 4;
	if ( s.variantsGeneration != generation || s.variantsRuntimeDirect != target.runtimeDirect )
	{
		for ( Resources &variant : s.variants )
			s.retired.emplace_back( target.frame, std::move( variant ) );
		s.variants.clear();
		s.variantsGeneration = generation;
		s.variantsRuntimeDirect = target.runtimeDirect;
	}
	auto variant = std::find_if( s.variants.begin(), s.variants.end(),
	    [&]( const Resources &v )
	    {
		    return v.colorFormat == target.colorFormat && v.depthFormat == target.depthFormat &&
		           v.samples == target.samples;
	    } );
	if ( variant == s.variants.end() )
	{
		if ( s.variants.size() >= kMaxVariants )
		{
			s.retired.emplace_back( target.frame, std::move( s.variants.front() ) );
			s.variants.erase( s.variants.begin() );
		}
		Resources made;
		made.colorFormat = target.colorFormat;
		made.depthFormat = target.depthFormat;
		made.samples = target.samples;
		s.variants.push_back( std::move( made ) );
	}
	else if ( variant + 1 != s.variants.end() )
	{
		std::rotate( variant, variant + 1, s.variants.end() );
	}
	Resources &r = s.variants.back();
	if ( r.litFrame != target.frame || r.litStream != target.streamEpoch )
	{
		for ( auto &cached : r.litViews )
			s.retiredGroups.emplace_back( r.litFrame, std::move( cached.group ) );
		r.litViews.clear();
		r.litFrame = target.frame;
		r.litStream = target.streamEpoch;
	}
	if ( r.dynamicFrame != target.frame )
	{
		for ( auto &[key, m] : r.dynamicMaterials )
			s.retiredGroups.emplace_back( r.dynamicFrame, std::move( m.group ) );
		r.dynamicMaterials.clear();
		r.dynamicFrame = target.frame;
	}

	std::erase_if( s.retired,
	    [&]( std::pair<std::uint64_t, Resources> &old )
	    {
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    s.Release( old.second, target.submitted );
		    return true;
	    } );
	std::erase_if( s.retiredBuffers,
	    [&]( const std::pair<std::uint64_t, BufferId> &old )
	    {
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    (void)device.Release( old.second, target.submitted );
		    return true;
	    } );
	// A released model level's buffers: its last reader may be a submission
	// this token already covers.
	std::erase_if( s.retiredModelBuffers,
	    [&]( const State::RetiredModelBuffer &old )
	    {
		    if ( target.frame == 0 || old.frame >= target.frame )
			    return false;
		    (void)device.Release( old.buffer, target.submitted );
		    return true;
	    } );
	std::erase_if( s.retiredTextures,
	    [&]( const std::pair<std::uint64_t, TextureId> &old )
	    {
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    (void)device.Release( old.second, target.submitted );
		    return true;
	    } );
	std::erase_if( s.retiredGroups,
	    [&]( std::pair<std::uint64_t, Group> &old )
	    {
		    if ( target.frame == 0 || old.first == 0 || old.first >= target.frame )
			    return false;
		    s.ReleaseGroup( old.second, target.submitted, true );
		    return true;
	    } );
	if ( !r.resolver )
	{
		auto resolver = material::ProgramResolver::Create( device, target.colorFormat,
		    target.depthFormat, target.samples, material::VertexLayout::kSurface,
		    material::ProgramModules{ s.fragmentModule } );
		if ( !resolver )
		{
			s.Fail( resolver.Error() );
			return;
		}
		r.resolver = std::move( resolver ).Value();
		// A world stage draws with world pbr (the pbr point on the world
		// vertex) and the scene terms its data supports.
		if ( world->stage )
			r.resolver->SetWorldPbr( true, StageTerms( *world->stage, target.runtimeDirect ) );
		r.resolver->SetSceneColorAvailable( world->stage != nullptr );
		r.materials.resize( world->materials.size() );
	}
	if ( ( !view.staticInstances.empty() || !view.posedModels.empty() ) && !r.modelResolver )
	{
		auto resolver = material::ProgramResolver::Create( device, target.colorFormat,
		    target.depthFormat, target.samples, material::VertexLayout::kModel,
		    material::ProgramModules{ s.fragmentModule } );
		if ( !resolver )
		{
			s.Fail( "the static model resolver: " + resolver.Error() );
			return;
		}
		r.modelResolver = std::move( resolver ).Value();
		r.modelResolver->SetWorldPbr(
		    true, world->stage ? StageTerms( *world->stage, target.runtimeDirect ) : 0 );
		r.modelResolver->SetSceneColorAvailable( world->stage != nullptr );
		r.modelMaterials.resize( world->materials.size() );
	}
	if ( !r.uploaded )
	{
		const auto vertexBytes = std::as_bytes( std::span( world->vertices ) );
		const auto indexBytes = std::as_bytes( std::span( world->indices ) );
		if ( vertexBytes.empty() && indexBytes.empty() )
		{
			r.uploaded = true; // a model-only world has no BSP vertex/index buffers
		}
		else
		{
			BufferDesc desc;
			desc.size = vertexBytes.size();
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
			desc.debugName = "world vertices";
			auto vertices = device.CreateBuffer( desc );
			desc.size = indexBytes.size();
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndex };
			desc.debugName = "world indices";
			auto indices = device.CreateBuffer( desc );
			if ( !vertices || !indices || vertexBytes.empty() || indexBytes.empty() )
			{
				if ( vertices )
					(void)device.Release( vertices.Value(), CompletionToken() );
				if ( indices )
					(void)device.Release( indices.Value(), CompletionToken() );
				s.Fail( "the world's buffers were refused" );
				return;
			}
			r.vertices = vertices.Value();
			r.indices = indices.Value();
			encoder.TransitionBuffer(
			    r.vertices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( r.vertices, 0, vertexBytes );
			encoder.TransitionBuffer(
			    r.vertices, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
			encoder.TransitionBuffer(
			    r.indices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( r.indices, 0, indexBytes );
			encoder.TransitionBuffer(
			    r.indices, ResourceUsage::kCopyDestination, ResourceUsage::kIndex );
			r.uploaded = true;
		}
	}

	// A world stage's textures: made at the first slot from the stage, then
	// updated in place as its lighting changes (SetStageLightmap,
	// SetStageChange), so the groups that bind them stay valid. Each upload
	// has an initialized upload buffer of its own, released behind a later frame.
	auto stageUpload = [&]( TextureId texture, const TextureDesc &desc,
	                       std::span<const std::byte> bytes, ResourceUsage from ) -> bool
	{
		// D25: one initialized host-visible copy-source buffer; no device-local
		// staging buffer and no pass through the upload ring.
		auto buffer = device.CreateUploadBuffer( bytes );
		if ( !buffer )
			return false;
		s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
		encoder.TransitionTexture( texture, from, ResourceUsage::kCopyDestination );
		TextureBufferCopy copy;
		copy.width = desc.width;
		copy.height = desc.height;
		encoder.CopyBufferToTexture( buffer.Value(), texture, copy );
		encoder.TransitionTexture(
		    texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		return true;
	};
	// Rectangles of an RGBA16F stage texture from their packed texels.
	auto stageRegions = [&]( TextureId texture, const TextureDesc &desc,
	                        const std::vector<StageRegion> &regions,
	                        std::span<const std::byte> texels ) -> bool
	{
		if ( regions.empty() )
			return true;
		if ( texels.empty() )
			return false;
		auto buffer = device.CreateUploadBuffer( texels );
		if ( !buffer )
			return false;
		s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
		encoder.TransitionTexture(
		    texture, ResourceUsage::kSampled, ResourceUsage::kCopyDestination );
		std::uint64_t offset = 0;
		for ( const StageRegion &region : regions )
		{
			if ( region.x + region.width > desc.width || region.y + region.height > desc.height ||
			     offset + std::uint64_t( region.width ) * region.height * 8 > texels.size() )
				return false;
			TextureBufferCopy copy;
			copy.bufferOffset = offset;
			copy.x = region.x;
			copy.y = region.y;
			copy.width = region.width;
			copy.height = region.height;
			encoder.CopyBufferToTexture( buffer.Value(), texture, copy );
			offset += std::uint64_t( region.width ) * region.height * 8;
		}
		encoder.TransitionTexture(
		    texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		return true;
	};
	auto stageMake = [&]( const char *name, Format format, std::uint32_t width,
	                     std::uint32_t height, std::span<const std::byte> bytes ) -> bool
	{
		const std::size_t texel = format == Format::kRGBA32Float ? 16 : 8;
		if ( width == 0 || height == 0 || bytes.size() != std::size_t( width ) * height * texel )
			return false;
		TextureDesc desc;
		desc.format = format;
		desc.width = width;
		desc.height = height;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		desc.debugName = name;
		auto texture = device.CreateTexture( desc );
		if ( !texture )
			return false;
		r.stageTextures[name] = texture.Value();
		desc.debugName = {};
		r.stageDescs[name] = desc;
		return stageUpload( texture.Value(), desc, bytes, ResourceUsage::kUndefined );
	};
	// The grid table's rows, with room for the moving occluders' rows.
	auto paddedTable = []( const StageProbeVolume &table, std::uint32_t rows )
	{
		std::vector<float> padded( std::size_t( table.tableTexels ) * 4 * rows, 0.0f );
		std::copy_n(
		    table.table.begin(), std::min( padded.size(), table.table.size() ), padded.begin() );
		return padded;
	};
	if ( world->stage && !r.stageMade )
	{
		const WorldStage &stage = *world->stage;
		const LightmapPages &pages = stage.lightmap;
		bool made = stageMake(
		    kStageLightmap, Format::kRGBA16Float, pages.width, pages.height, pages.flat );
		if ( made && pages.Directional() )
			made = stageMake(
			    kStageGradient, Format::kRGBA16Float, pages.width, pages.height, pages.gradient );
		if ( made && !stage.indirect.empty() )
			made = stageMake(
			    kStageIndirect, Format::kRGBA16Float, pages.width, pages.height, stage.indirect );
		if ( made && !stage.indirectGradient.empty() )
			made = stageMake( kStageIndirectGradient, Format::kRGBA16Float, pages.width,
			    pages.height, stage.indirectGradient );
		if ( made && stage.probes )
		{
			const StageProbeVolume &probes = *stage.probes;
			const std::vector<std::byte> zero( probes.atlas.size(), std::byte( 0 ) );
			const std::vector<float> table =
			    paddedTable( probes, probes.rows + kStageOccluderRows );
			made = stageMake( kStageProbeAtlas, Format::kRGBA16Float, probes.atlasWidth,
			           probes.atlasHeight, probes.atlas ) &&
			       stageMake( kStageChange, Format::kRGBA16Float, probes.atlasWidth,
			           probes.atlasHeight, zero ) &&
			       stageMake( kStageProbeGrids, Format::kRGBA32Float, probes.tableTexels,
			           probes.rows + kStageOccluderRows, std::as_bytes( std::span( table ) ) );
		}
		if ( made && !stage.reflectionProbes.empty() )
			made = stageMake( kStageReflection, Format::kRGBA16Float, stage.reflectionWidth,
			    stage.reflectionHeight, stage.reflectionProbes );
		for ( const auto &[name, table] : { std::pair{ kStageSplitSum, material::SplitSumTable() },
		          std::pair{ kStageLtc, material::LtcTable() } } )
		{
			if ( made )
				made = stageMake( name, table.format, table.width, table.height,
				    std::as_bytes( std::span( table.texels ) ) );
		}
		if ( !made )
		{
			s.Fail( "the world stage's textures were refused" );
			return;
		}
		r.stageMade = true;
	}
	if ( world->stage )
	{
		// Under the lock: the page's base and parts are the state's (the
		// uploads copy what they read at once).
		std::unique_lock<std::mutex> guard( s.lock );
		if ( r.stageLightmapRevision != s.stageLightmapRevision )
		{
			const LightmapPages &own = world->stage->lightmap;
			const TextureDesc &desc = r.stageDescs[kStageLightmap];
			// A set behind the base takes it whole, then the parts after it.
			const bool whole = r.stageLightmapRevision < s.stageLightmapBaseRevision;
			const LightmapPages &base = s.stageLightmap ? *s.stageLightmap : own;
			bool fits = true;
			if ( whole )
				fits = base.width == desc.width && base.height == desc.height &&
				       base.Directional() == own.Directional() &&
				       stageUpload( r.stageTextures[kStageLightmap], desc, base.flat,
				           ResourceUsage::kSampled ) &&
				       ( !base.Directional() || stageUpload( r.stageTextures[kStageGradient], desc,
				                                    base.gradient, ResourceUsage::kSampled ) );
			for ( const State::StagePatch &patch : s.stageLightmapPatches )
			{
				if ( !fits || ( !whole && patch.revision <= r.stageLightmapRevision ) )
					continue;
				fits = stageRegions(
				    r.stageTextures[kStageLightmap], desc, patch.regions, patch.texels );
			}
			if ( !fits )
			{
				guard.unlock();
				s.Fail( "the world stage's lightmap update does not fit its pages" );
				return;
			}
			r.stageLightmapRevision = s.stageLightmapRevision;
			// The parts every resource set holds fold into the base.
			std::uint64_t applied = r.stageLightmapRevision;
			for ( const Resources &set : s.variants )
				if ( set.stageMade )
					applied = std::min( applied, set.stageLightmapRevision );
			while ( !s.stageLightmapPatches.empty() &&
			        s.stageLightmapPatches.front().revision <= applied )
			{
				const State::StagePatch &patch = s.stageLightmapPatches.front();
				if ( !s.stageLightmap )
					s.stageLightmap = std::make_shared<LightmapPages>( own );
				LightmapPages &pages = *s.stageLightmap;
				std::size_t offset = 0;
				for ( const StageRegion &region : patch.regions )
				{
					const std::size_t row = std::size_t( region.width ) * 8;
					for ( std::uint32_t y = 0; y < region.height; ++y )
					{
						const std::size_t at =
						    ( std::size_t( region.y + y ) * pages.width + region.x ) * 8;
						if ( at + row <= pages.flat.size() && offset + row <= patch.texels.size() )
							std::memcpy(
							    pages.flat.data() + at, patch.texels.data() + offset, row );
						offset += row;
					}
				}
				s.stageLightmapBaseRevision = patch.revision;
				s.stageLightmapPatches.pop_front();
			}
		}
	}
	if ( world->stage && world->stage->probes )
	{
		std::unique_lock<std::mutex> guard( s.lock );
		if ( r.stageProbeRevision != s.stageProbeRevision )
		{
			const StageProbeVolume &probes = *world->stage->probes;
			const TextureDesc &desc = r.stageDescs[kStageProbeAtlas];
			const bool whole = r.stageProbeRevision < s.stageProbeBaseRevision;
			const std::vector<std::byte> &base =
			    s.stageProbeAtlas ? *s.stageProbeAtlas : probes.atlas;
			bool fits = true;
			if ( whole )
				fits = base.size() == probes.atlas.size() &&
				       stageUpload(
				           r.stageTextures[kStageProbeAtlas], desc, base, ResourceUsage::kSampled );
			for ( const State::StagePatch &patch : s.stageProbePatches )
			{
				if ( !fits || ( !whole && patch.revision <= r.stageProbeRevision ) )
					continue;
				fits = stageRegions(
				    r.stageTextures[kStageProbeAtlas], desc, patch.regions, patch.texels );
			}
			if ( !fits )
			{
				guard.unlock();
				s.Fail( "the world stage's probe atlas update does not fit its volume" );
				return;
			}
			r.stageProbeRevision = s.stageProbeRevision;
			std::uint64_t applied = r.stageProbeRevision;
			for ( const Resources &set : s.variants )
				if ( set.stageMade )
					applied = std::min( applied, set.stageProbeRevision );
			while (
			    !s.stageProbePatches.empty() && s.stageProbePatches.front().revision <= applied )
			{
				const State::StagePatch &patch = s.stageProbePatches.front();
				if ( !s.stageProbeAtlas )
					s.stageProbeAtlas = std::make_shared<std::vector<std::byte>>( probes.atlas );
				std::size_t offset = 0;
				for ( const StageRegion &region : patch.regions )
				{
					const std::size_t row = std::size_t( region.width ) * 8;
					for ( std::uint32_t y = 0; y < region.height; ++y )
					{
						const std::size_t at =
						    ( std::size_t( region.y + y ) * probes.atlasWidth + region.x ) * 8;
						std::memcpy(
						    s.stageProbeAtlas->data() + at, patch.texels.data() + offset, row );
						offset += row;
					}
				}
				s.stageProbeBaseRevision = patch.revision;
				s.stageProbePatches.pop_front();
			}
		}
	}
	if ( world->stage && world->stage->probes )
	{
		// Under the lock: the change's base and parts are the state's (the
		// uploads copy what they read at once).
		std::unique_lock<std::mutex> guard( s.lock );
		if ( s.stageTable && r.stageChangeRevision != s.stageChangeRevision )
		{
			const StageProbeVolume &probes = *world->stage->probes;
			const std::uint32_t rows = probes.rows + kStageOccluderRows;
			const StageProbeVolume &stageTable = *s.stageTable;
			const bool whole = r.stageChangeRevision < s.stageBaseRevision;
			bool fits = stageTable.tableTexels == probes.tableTexels && stageTable.rows <= rows;
			if ( fits && whole )
			{
				const std::vector<std::byte> zero(
				    s.stageChangeBase.empty() ? probes.atlas.size() : 0, std::byte( 0 ) );
				const std::vector<std::byte> &change =
				    s.stageChangeBase.empty() ? zero : s.stageChangeBase;
				fits = change.size() == probes.atlas.size() &&
				       stageUpload( r.stageTextures[kStageChange], r.stageDescs[kStageChange],
				           change, ResourceUsage::kSampled );
			}
			for ( const State::StagePatch &patch : s.stagePatches )
			{
				if ( !fits || ( !whole && patch.revision <= r.stageChangeRevision ) )
					continue;
				fits = stageRegions( r.stageTextures[kStageChange], r.stageDescs[kStageChange],
				    patch.regions, patch.texels );
			}
			const std::vector<float> table = paddedTable( stageTable, rows );
			if ( !fits ||
			     !stageUpload( r.stageTextures[kStageProbeGrids], r.stageDescs[kStageProbeGrids],
			         std::as_bytes( std::span( table ) ), ResourceUsage::kSampled ) )
			{
				guard.unlock();
				s.Fail( "the world stage's probe change does not fit its volume" );
				return;
			}
			r.stageChangeRevision = s.stageChangeRevision;
			// The parts every resource set holds fold into the base.
			std::uint64_t applied = r.stageChangeRevision;
			for ( const Resources &set : s.variants )
				if ( set.stageMade )
					applied = std::min( applied, set.stageChangeRevision );
			while ( !s.stagePatches.empty() && s.stagePatches.front().revision <= applied )
			{
				const State::StagePatch &patch = s.stagePatches.front();
				if ( s.stageChangeBase.empty() )
					s.stageChangeBase.assign( probes.atlas.size(), std::byte( 0 ) );
				std::size_t offset = 0;
				for ( const StageRegion &region : patch.regions )
				{
					for ( std::uint32_t y = 0; y < region.height; ++y )
					{
						const std::size_t row = std::size_t( region.width ) * 8;
						const std::size_t at =
						    ( std::size_t( region.y + y ) * probes.atlasWidth + region.x ) * 8;
						if ( at + row <= s.stageChangeBase.size() &&
						     offset + row <= patch.texels.size() )
							std::memcpy(
							    s.stageChangeBase.data() + at, patch.texels.data() + offset, row );
						offset += row;
					}
				}
				s.stageBaseRevision = patch.revision;
				s.stagePatches.pop_front();
			}
		}
	}

	// Neutral inputs, made on first use: white color and far depth. The cube
	// term is off wherever its neutral texture is bound; filling it keeps it defined.
	auto neutral = [&]( TextureDimension dimension, bool array, bool depth ) -> TextureId
	{
		TextureId &slot = depth                                  ? r.neutralDepth
		                  : dimension == TextureDimension::kCube ? r.neutralCube
		                  : array                                ? r.neutralArray
		                                                         : r.neutralWhite;
		if ( slot.IsValid() )
			return slot;
		if ( !r.neutralStaging.IsValid() )
		{
			const std::uint32_t whiteAndDepth[] = { 0xFFFFFFFFu, 0x3F800000u };
			auto buffer = device.CreateUploadBuffer( std::as_bytes( std::span( whiteAndDepth ) ) );
			if ( !buffer )
				return {};
			r.neutralStaging = buffer.Value();
		}
		TextureDesc desc;
		desc.dimension = dimension;
		desc.format = depth ? Format::kD32Float : Format::kRGBA8Unorm;
		desc.width = desc.height = 1;
		desc.depthOrLayers = dimension == TextureDimension::kCube ? 6 : array ? 2 : 1;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		desc.debugName = dimension == TextureDimension::kCube ? "world neutral cube"
		                 : array                              ? "world neutral array"
		                                                      : "world neutral white";
		auto texture = device.CreateTexture( desc );
		if ( !texture )
			return {};
		encoder.TransitionTexture(
		    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		for ( std::uint32_t layer = 0; layer < desc.depthOrLayers; ++layer )
			encoder.CopyBufferToTexture(
			    r.neutralStaging, texture.Value(), { depth ? 4u : 0u, 0, layer, 1, 1 } );
		encoder.TransitionTexture(
		    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		slot = texture.Value();
		return slot;
	};

	// A group from its request: the constants uploaded here, the textures
	// imported by the names' handles, with the backend's samplers; an input
	// the program names empty (a term that is off) or a handle of 0 (an
	// absent input) takes the neutral texture of its dimension.
	auto buildGroup = [&]( const material::GroupRequest &request,
	                      const std::map<std::string, int> &handles, Group &out,
	                      std::string *why ) -> bool
	{
		std::vector<BindGroupEntry> entries;
		if ( !request.constants.empty() )
		{
			auto constants = s.groupResources.Acquire(
			    device, request.constants.size(), ResourceUsage::kUniform );
			if ( !constants )
			{
				*why = "a constants buffer was refused";
				return false;
			}
			out.constants = constants.Value();
			entries.push_back( { request.constantsBinding, out.constants.id, 0, 0, {}, {} } );
		}
		for ( const material::GroupBuffer &storage : request.storage )
		{
			if ( storage.external.IsValid() )
			{
				out.storage.push_back( { storage.external } );
				entries.push_back( { storage.binding, storage.external, 0, 0, {}, {} } );
				continue;
			}
			auto buffer = s.groupResources.Acquire( device,
			    std::max<std::uint64_t>( storage.bytes.size(), 4 ), ResourceUsage::kStorageRead );
			if ( !buffer )
			{
				*why = "a storage buffer was refused";
				return false;
			}
			out.storage.push_back( buffer.Value() );
			entries.push_back( { storage.binding, buffer.Value().id, 0, 0, {}, {} } );
		}
		for ( const material::ProgramTexture &texture : request.textures )
		{
			// A texture the group's owner made (the view's shadow atlas) is
			// bound as it is, with the request's own sampler.
			const bool external = texture.external.IsValid();
			// A world stage's own texture (its lightmap pages, probes and
			// tables), bound as it is.
			const auto stageTexture = r.stageTextures.find( texture.name );
			const bool staged =
			    !external && !texture.name.empty() && stageTexture != r.stageTextures.end();
			const auto handle = handles.find( texture.name );
			const bool absent =
			    !external && !staged &&
			    ( texture.name.empty() || ( handle != handles.end() && handle->second == 0 ) );
			const TextureId id =
			    external ? texture.external
			    : staged ? stageTexture->second
			    : absent ? neutral( texture.dimension, texture.array, texture.depth )
			    : handle == handles.end() ? TextureId()
			                              : textures.Import( handle->second, texture.srgb );
			if ( !id.IsValid() )
			{
				*why = handle == handles.end()
				           ? "texture " + texture.name + " has no material system handle"
				           : "texture " + texture.name + " did not import" +
				                 ( texture.srgb ? " through an sRGB view" : "" );
				return false;
			}
			entries.push_back( { texture.binding, {}, 0, 0, id, {} } );
			if ( texture.samplerBinding == material::kNoSamplerBinding )
				continue; // only fetched
			auto sampler = s.groupResources.AcquireSampler(
			    device, external || staged ? texture.sampler
			            : absent           ? texture.sampler
			                               : textures.Sampler( handle->second ) );
			if ( !sampler )
			{
				*why = "a sampler was refused";
				return false;
			}
			if ( sampler.Value().owned )
				out.samplers.push_back( sampler.Value().id );
			entries.push_back( { texture.samplerBinding, {}, 0, 0, {}, sampler.Value().id } );
		}
		for ( const auto &[binding, desc] : request.samplers )
		{
			auto sampler = s.groupResources.AcquireSampler( device, desc );
			if ( !sampler )
			{
				*why = "an additional sampler was refused";
				return false;
			}
			if ( sampler.Value().owned )
				out.samplers.push_back( sampler.Value().id );
			entries.push_back( { binding, {}, 0, 0, {}, sampler.Value().id } );
		}
		auto group = device.CreateBindGroup( { request.layout, entries } );
		if ( !group )
		{
			const DeviceError &error = group.Error();
			*why = "a bind group was refused: " + std::string( DescribeStatus( error.status ) ) +
			       " (native " + std::to_string( error.nativeCode ) + ")";
			return false;
		}
		out.group = group.Value();
		if ( out.constants.id.IsValid() )
		{
			encoder.TransitionBuffer(
			    out.constants.id, out.constants.before, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( out.constants.id, 0, request.constants );
			encoder.TransitionBuffer(
			    out.constants.id, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
			out.constants.before = ResourceUsage::kUniform;
		}
		for ( std::size_t i = 0; i < out.storage.size(); ++i )
		{
			if ( out.storage[i].size == 0 )
				continue;
			encoder.TransitionBuffer(
			    out.storage[i].id, out.storage[i].before, ResourceUsage::kCopyDestination );
			if ( !request.storage[i].bytes.empty() )
				encoder.WriteBuffer( out.storage[i].id, 0, request.storage[i].bytes );
			encoder.TransitionBuffer(
			    out.storage[i].id, ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead );
			out.storage[i].before = ResourceUsage::kStorageRead;
		}
		return true;
	};

	// The view's first failure; the view counts once.
	std::string failure;
	auto note = [&]( std::string why )
	{
		if ( failure.empty() )
			failure = std::move( why );
	};
	auto prepareMaterial = [&]( material::ProgramResolver &resolver, Resources::Material &m,
	                           const Claimed &claimed,
	                           const WorldMaterial &source ) -> Resources::Material *
	{
		if ( m.failed )
			note( m.failure );
		if ( m.ready || m.failed )
			return m.ready ? &m : nullptr;
		const std::string &name = source.name;
		const bool model =
		    &resolver == r.modelResolver.get() || &resolver == r.prepassModelResolver.get();
		auto program =
		    source.mesh ? resolver.ResolveMesh( claimed.desc ) : resolver.Resolve( claimed.desc );
		std::string why;
		if ( !program )
			why = program.Error();
		else if ( program.Value().request.vertexStride !=
		          ( model ? sizeof( material::SurfaceModelVertex ) : sizeof( WorldVertex ) ) )
			why = "its program reads another vertex than its mesh";
		else if ( !buildGroup( program.Value().request.material, claimed.handles, m.group, &why ) )
			s.ReleaseGroup( m.group, CompletionToken() );
		if ( !why.empty() )
		{
			m.failed = true;
			m.failure = "material " + name + ": " + why;
			note( m.failure );
			{
				std::lock_guard<std::mutex> guard( s.lock );
				auto gap = std::find_if( s.stats.gaps.begin(), s.stats.gaps.end(),
				    [&]( const auto &entry )
				    {
					    return entry.first == m.failure;
				    } );
				if ( gap == s.stats.gaps.end() )
					s.stats.gaps.emplace_back( m.failure, 1 );
				else
					++gap->second;
			}
			return nullptr;
		}
		m.program = std::move( program ).Value();
		m.resolver = &resolver;
		m.ready = true;
		m.viewInput = 0;
		for ( const std::string &input : m.program.viewInputs )
		{
			const auto handle = claimed.handles.find( input );
			if ( handle == claimed.handles.end() || handle->second == 0 )
			{
				m.ready = false;
				m.failed = true;
				m.failure = "material " + name + ": its view input " + input +
				            " has no material system handle";
				note( m.failure );
				return nullptr;
			}
			m.viewInput = handle->second;
		}
		return &m;
	};
	auto materialReadyIn = [&]( material::ProgramResolver &resolver,
	                           std::vector<Resources::Material> &materials, std::uint32_t index )
	{
		return prepareMaterial(
		    resolver, materials[index], ( *claims )[index], world->materials[index] );
	};

	auto materialReady = [&]( std::uint32_t index )
	{
		return materialReadyIn( *r.resolver, r.materials, index );
	};
	auto drawKey = []( const Resources::Material &m, int page, bool captured = false )
	{
		std::string inputs = captured ? "captured;" : "";
		for ( const std::string &input : m.program.drawInputs )
			inputs += input + ";";
		return std::make_tuple( m.program.request.drawLayout.value, page, std::move( inputs ) );
	};
	auto drawGroupReady = [&]( const Resources::Material &m, int page,
	                          bool captured = false ) -> const Group *
	{
		const auto key = drawKey( m, page, captured );
		if ( auto found = r.drawGroups.find( key ); found != r.drawGroups.end() )
			return found->second.group.IsValid() ? &found->second : nullptr;
		Group &group = r.drawGroups[key];
		std::vector<std::string> inputs;
		std::map<std::string, int> handles;
		for ( const std::string &input : m.program.drawInputs )
		{
			if ( world->stage && !captured )
			{
				// A world stage's pages; an input the stage lacks is off.
				const WorldStage &stage = *world->stage;
				if ( input == "lightmap" )
					inputs.push_back( kStageLightmap );
				else if ( input == "lightmap-gradient" )
					inputs.push_back( stage.lightmap.Directional() ? kStageGradient : "" );
				else if ( input == "lightmap-indirect" )
					inputs.push_back( stage.indirect.empty() ? "" : kStageIndirect );
				else if ( input == "lightmap-indirect-gradient" )
					inputs.push_back(
					    stage.indirectGradient.empty() ? "" : kStageIndirectGradient );
				else
				{
					note( "a program reads draw input " + input + ", which the world stage lacks" );
					return nullptr;
				}
				continue;
			}
			if ( input != "lightmap" )
			{
				note( "a program reads draw input " + input + ", which the world lacks" );
				return nullptr;
			}
			inputs.push_back( PageName( page ) );
			handles[PageName( page )] = page;
		}
		const std::optional<material::GroupRequest> request =
		    m.resolver->DrawGroup( m.program, inputs );
		std::string why;
		if ( !request || !buildGroup( *request, handles, group, &why ) )
		{
			s.ReleaseGroup( group, CompletionToken() );
			// Only successful bindings enter the cache. Retrying a missing
			// image must retain its named error and may succeed after upload.
			r.drawGroups.erase( key );
			note( "a draw group: " + ( why.empty() ? std::string( "not resolved" ) : why ) );
			return nullptr;
		}
		return &group;
	};

	// The frame group of a program's layout, its terms written for this slot.
	material::FrameTerms terms;
	std::copy_n( view.motionToClip, 16, terms.motionCurrentToClip );
	std::copy_n( view.previousToClip, 16, terms.motionPreviousToClip );
	terms.motionExtent[0] = view.viewport.width;
	terms.motionExtent[1] = view.viewport.height;
	terms.motionExtent[2] = view.previousViewValid ? 1.0f : 0.0f;
	std::memcpy( terms.clipPlanes, target.clipPlanes, sizeof( terms.clipPlanes ) );
	terms.lightmapScale = target.lightmapScale;
	terms.outputScale = target.outputScale;
	terms.encodeOutput = target.encodeOutput;
	terms.fogType = target.fogType;
	std::copy( target.fogColor, target.fogColor + 3, terms.fogColor );
	std::copy( target.fogParams, target.fogParams + 4, terms.fogParams );
	terms.fogEyeZ = target.fogEyeZ;
	std::copy( target.eye, target.eye + 3, terms.eye );
	terms.envmapScale = target.envmapScale;
	terms.specular = target.specular;
	terms.ssbumpNormalized = target.ssbumpNormalized;
	// The view's area lights (every point reads them).
	if ( view.lights )
		terms.areas = view.lights->areas;
	terms.time = target.time;
	std::memcpy( terms.foliage, target.foliage, sizeof( terms.foliage ) );
	terms.foliageAvailable = target.foliageAvailable;
	if ( !view.previousViewValid )
		std::copy_n( terms.foliage[0], 4, terms.foliage[1] );
	terms.waterReflectTintScale = target.waterReflectTintScale;
	std::copy( view.viewRight, view.viewRight + 2, terms.viewRight );
	if ( view.viewport.width > 0.0f && view.viewport.height > 0.0f )
	{
		terms.viewport[0] = view.viewport.x;
		terms.viewport[1] = view.viewport.y;
		terms.viewport[2] = 1.0f / view.viewport.width;
		terms.viewport[3] = 1.0f / view.viewport.height;
	}
	if ( world->stage )
	{
		// The stage's pages hold linear light (LMAP); the pbr point's tables
		// and the map's probes, with the host's change volume as the second
		// probe atlas (kSurfaceProbeBounce).
		terms.lightmapScale = 1.0f;
		terms.splitSumTable = kStageSplitSum;
		terms.ltcTable = kStageLtc;
		if ( world->stage->probes )
		{
			terms.map.probeAtlas = kStageProbeAtlas;
			terms.map.probeGrids = kStageProbeGrids;
			terms.map.probeBounce = r.stageTextures[kStageChange];
			terms.map.probeBounceDesc = r.stageDescs[kStageChange];
		}
		if ( !world->stage->reflectionProbes.empty() )
			terms.map.reflectionProbes = kStageReflection;
		// The view's sun.
		if ( view.lights )
		{
			std::copy(
			    view.lights->sunDirection, view.lights->sunDirection + 4, terms.sunDirection );
			std::copy( view.lights->sunColor, view.lights->sunColor + 4, terms.sunColor );
			std::copy( view.lights->sunShadow, view.lights->sunShadow + 4, terms.sunShadow );
		}
	}
	// Presence comes from this slot's actual frame/view inputs, never a quality
	// setting. Uniform light data and all shadow filters remain unchanged.
	std::uint32_t viewFeatures = terms.areas.empty() ? 0u : material::kSurfaceViewAreas;
	if ( terms.sunColor[0] != 0.0f || terms.sunColor[1] != 0.0f || terms.sunColor[2] != 0.0f )
		viewFeatures |= material::kSurfaceViewSun;
	if ( view.lights && view.lights->view.counts[0] > 0.0f )
		viewFeatures |= material::kSurfaceViewProjectors;
	std::map<std::uint64_t, bool> framesWritten;
	auto frameGroupReady = [&]( const Resources::Material &m ) -> const Group *
	{
		if ( auto error = material::FrameInputError( m.program, terms ) )
		{
			note( *error );
			return nullptr;
		}
		const std::uint64_t layout = m.program.request.frameLayout.value;
		if ( framesWritten[layout] )
			return &r.frameGroups[layout];
		const std::optional<material::GroupRequest> request =
		    m.resolver->FrameGroup( m.program, terms );
		if ( !request )
		{
			note( "a frame group was not resolved" );
			return nullptr;
		}
		Group &group = r.frameGroups[layout];
		if ( !group.group.IsValid() )
		{
			std::string why;
			if ( !buildGroup( *request, {}, group, &why ) )
			{
				s.ReleaseGroup( group, CompletionToken() );
				note( "a frame group: " + why );
				return nullptr;
			}
			framesWritten[layout] = true; // buildGroup wrote this slot's terms
		}
		else if ( !framesWritten[layout] )
		{
			encoder.TransitionBuffer(
			    group.constants.id, ResourceUsage::kUniform, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( group.constants.id, 0, request->constants );
			encoder.TransitionBuffer(
			    group.constants.id, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
			framesWritten[layout] = true;
		}
		return &group;
	};

	// A program's view group: its neutral one (built once per layout).
	// A world stage's lit view: one group per view layout for this view, with
	// the view's clustered lights, retired behind this frame after drawing.
	std::map<std::uint64_t, Group *> litViews;
	std::map<std::uint64_t, Group *> modelLitViews;
	std::deque<Group> transientLitViews;
	std::map<std::uint64_t, Group> sceneViews;
	std::map<std::uint64_t, Group> depthViews;
	// The view's ambient occlusion once the screen passes recorded it.
	TextureId viewOcclusion;
	// The view's planar reflection (a program's view input, imported below
	// once the view's materials resolved), and the view groups that bind it
	// for programs without the view's lights, retired behind this frame.
	TextureId viewReflection;
	TextureId viewSceneColor;
	TextureDesc viewSceneColorDesc;
	const TextureId viewDepthAlpha =
	    view.depthAlphaHandle > 0 ? textures.Import( view.depthAlphaHandle, false ) : TextureId();
	std::map<std::uint64_t, Group> reflectViews;
	auto viewGroupReady = [&]( const Resources::Material &m ) -> const Group *
	{
		const std::uint64_t layout = m.program.request.viewLayout.value;
		if ( m.program.depthBlend &&
		     ( !viewDepthAlpha.IsValid() || !std::isfinite( view.depthAlphaRange ) ||
		         view.depthAlphaRange <= 0.0f ) )
		{
			note( "$depthblend has no imported depth-alpha snapshot with a positive range" );
			return nullptr;
		}
		if ( m.program.sceneColor && !viewSceneColor.IsValid() )
		{
			note( "a transmitting program has no scene-color snapshot" );
			return nullptr;
		}
		if ( m.viewInput != 0 && !viewReflection.IsValid() )
		{
			note( "a program reads the view's planar reflection, which did not import" );
			return nullptr;
		}
		if ( view.lights )
		{
			const bool model =
			    m.resolver == r.modelResolver.get() || m.resolver == r.prepassModelResolver.get();
			Group *&slot = model ? modelLitViews[layout] : litViews[layout];
			Group *lit = m.program.depthBlend   ? &depthViews[layout]
			             : m.program.sceneColor ? &sceneViews[layout]
			                                    : slot;
			if ( lit && lit->group.IsValid() )
				return lit;
			if ( !m.program.sceneColor && !m.program.depthBlend )
			{
				const auto cached = std::find_if( r.litViews.begin(), r.litViews.end(),
				    [&]( const Resources::LitView &entry )
				    {
					    return entry.layout == layout && entry.lights == view.lights &&
					           entry.shadowAtlas == target.shadowAtlas &&
					           entry.occlusion == viewOcclusion &&
					           entry.reflection == viewReflection && entry.group.group.IsValid();
				    } );
				if ( cached != r.litViews.end() )
				{
					slot = &cached->group;
					return slot;
				}
				// A bound on retained bindings, never a draw limit. Unknown
				// frames and overflow keep the existing transient lifetime.
				constexpr std::size_t kMaxLitViews = 256;
				if ( target.frame != 0 && r.litViews.size() < kMaxLitViews )
				{
					r.litViews.push_back( { layout, view.lights, target.shadowAtlas, viewOcclusion,
					    viewReflection, {} } );
					lit = &r.litViews.back().group;
				}
				else
				{
					transientLitViews.emplace_back();
					lit = &transientLitViews.back();
				}
				slot = lit;
			}
			const StageViewLights &lights = *view.lights;
			// The lights' shadow tiles index the slot's atlas: tiles without
			// one would index nothing, which fails the view by name.
			material::SurfaceShadows shadows;
			if ( !lights.shadowTiles.empty() )
			{
				if ( !target.shadowAtlas.IsValid() )
				{
					note( "the view's lights have shadow tiles and the slot no atlas" );
					return nullptr;
				}
				shadows.atlas = target.shadowAtlas;
				shadows.atlasDesc = target.shadowAtlasDesc;
				shadows.tiles = lights.shadowTiles;
			}
			material::SurfaceScreenInputs screen;
			screen.depthAlpha = m.program.depthBlend ? viewDepthAlpha : TextureId();
			screen.depthAlphaRange = m.program.depthBlend ? view.depthAlphaRange : 0.0f;
			screen.depthAlphaSourceWidth = target.width;
			screen.depthAlphaSourceHeight = target.height;
			// The surface program fetches AO at the screen pixel. Models need
			// the same full-size view input as world surfaces.
			if ( viewOcclusion.IsValid() )
			{
				screen.ambientOcclusion = viewOcclusion;
				screen.ambientOcclusionDesc = target.ambientOcclusionDesc;
			}
			screen.planarReflection = viewReflection;
			if ( m.program.sceneColor )
			{
				screen.sceneColor = viewSceneColor;
				screen.sceneColorDesc = viewSceneColorDesc;
			}
			material::SurfaceProjectors projectors;
			projectors.lights = lights.projectors;
			projectors.cookies = lights.cookies;
			projectors.cookiesDesc = lights.cookiesDesc;
			material::GroupRequest request = m.resolver->Program().ViewGroup( lights.view,
			    lights.froxels, lights.indices, lights.lights, shadows, projectors, screen );
			for ( auto &buffer : request.storage )
			{
				if ( buffer.binding == 1 && lights.gpuFroxels.IsValid() )
				{
					buffer.external = lights.gpuFroxels;
					buffer.bytes.clear();
				}
				if ( buffer.binding == 2 && lights.gpuIndices.IsValid() )
				{
					buffer.external = lights.gpuIndices;
					buffer.bytes.clear();
				}
			}
			std::string why;
			if ( request.layout != m.program.request.viewLayout ||
			     !buildGroup( request, {}, *lit, &why ) )
			{
				s.ReleaseGroup( *lit, CompletionToken() );
				note( "the view's lights: " +
				      ( why.empty() ? std::string( "another view layout" ) : why ) );
				return nullptr;
			}
			return lit;
		}
		if ( m.viewInput != 0 || m.program.sceneColor || m.program.depthBlend )
		{
			Group &reflect = m.program.depthBlend   ? depthViews[layout]
			                 : m.program.sceneColor ? sceneViews[layout]
			                                        : reflectViews[layout];
			if ( reflect.group.IsValid() )
				return &reflect;
			material::SurfaceScreenInputs screen;
			screen.depthAlpha = m.program.depthBlend ? viewDepthAlpha : TextureId();
			screen.depthAlphaRange = m.program.depthBlend ? view.depthAlphaRange : 0.0f;
			screen.depthAlphaSourceWidth = target.width;
			screen.depthAlphaSourceHeight = target.height;
			screen.planarReflection = viewReflection;
			if ( m.program.sceneColor )
			{
				screen.sceneColor = viewSceneColor;
				screen.sceneColorDesc = viewSceneColorDesc;
			}
			const material::GroupRequest request = m.resolver->Program().NeutralViewGroup( screen );
			std::string why;
			if ( request.layout != m.program.request.viewLayout ||
			     !buildGroup( request, {}, reflect, &why ) )
			{
				s.ReleaseGroup( reflect, CompletionToken() );
				note( "the view's planar reflection: " +
				      ( why.empty() ? std::string( "another view layout" ) : why ) );
				return nullptr;
			}
			return &reflect;
		}
		Group &group = r.viewGroups[layout];
		if ( group.group.IsValid() )
			return &group;
		if ( !m.program.request.neutralView )
		{
			note( "a program reads a view group and names no neutral one" );
			return nullptr;
		}
		std::string why;
		if ( !buildGroup( *m.program.request.neutralView, {}, group, &why ) )
		{
			s.ReleaseGroup( group, CompletionToken() );
			note( "a view group: " + why );
			return nullptr;
		}
		return &group;
	};

	preparation.Select( "prepare world materials" );
	// Resolve before rendering (uploads run outside it), then draw in
	// (material, page) order so binds change least.
	std::vector<std::uint32_t> order;
	order.reserve( view.surfaces.size() );
	bool complete = true;
	for ( const std::uint32_t index : view.surfaces )
	{
		if ( index >= world->surfaces.size() )
		{
			note( "a view named a surface the world does not have" );
			complete = false;
			continue;
		}
		const WorldSurface &surface = world->surfaces[index];
		const Resources::Material *m =
		    surface.material < claims->size() && ( *claims )[surface.material].draws
		        ? materialReady( surface.material )
		        : nullptr;
		if ( !m && failure.empty() )
			note( "world surface " + std::to_string( index ) + " names unclaimed material " +
			      std::to_string( surface.material ) );
		if ( !m ||
		     ( m->program.request.drawLayout.IsValid() &&
		         !drawGroupReady( *m, surface.lightmapPage ) ) ||
		     ( m->program.request.frameLayout.IsValid() && !frameGroupReady( *m ) ) )
		{
			complete = false;
			continue;
		}
		order.push_back( index );
	}
	std::sort( order.begin(), order.end(),
	    [&]( std::uint32_t a, std::uint32_t b )
	    {
		    const WorldSurface &x = world->surfaces[a];
		    const WorldSurface &y = world->surfaces[b];
		    if ( x.material != y.material )
			    return x.material < y.material;
		    return x.lightmapPage != y.lightmapPage ? x.lightmapPage < y.lightmapPage
		                                            : x.firstIndex < y.firstIndex;
	    } );
	preparation.Select( "prepare model materials and meshes" );
	// Model geometry residency: adopt the world's levels, then release the ones
	// no view has selected for a while (once per recorded frame).
	SweepModelResidency( s, *world, target );
	std::vector<StaticDraw> staticDraws;
	std::vector<BufferId> posedBuffers( view.posedModels.size() );
	std::vector<BufferId> previousPosedBuffers( view.posedModels.size() );
	// One (model, level)'s buffers for this recording: uploaded from the
	// world's staging on first use, and again after the residency rule released
	// them. Null when the level has no geometry left to upload.
	auto modelLevel = [&]( std::uint32_t meshId, std::uint32_t lod ) -> const State::ModelLevel *
	{
		if ( meshId >= world->staticMeshes.size() || lod >= world->staticMeshes[meshId].lodCount() )
		{
			note( "a model names a level it does not have" );
			return nullptr;
		}
		const WorldData::StaticMesh &mesh = world->staticMeshes[meshId];
		if ( !UploadModelLevel( s, device, encoder, mesh, meshId, lod, target.frame ) )
		{
			note( "model " + std::to_string( meshId ) + " level " + std::to_string( lod ) +
			      " has no geometry to upload" );
			return nullptr;
		}
		return &s.models.models[meshId][lod];
	};
	for ( std::size_t staticIndex = 0; staticIndex < view.staticInstances.size(); ++staticIndex )
	{
		const auto &draw = view.staticInstances[staticIndex];
		const std::uint32_t instanceId = draw.instance;
		if ( instanceId >= world->staticInstances.size() )
		{
			note( "a view named a static model instance the world does not have" );
			complete = false;
			continue;
		}
		const WorldData::StaticInstance &instance = world->staticInstances[instanceId];
		if ( instance.mesh >= world->staticMeshes.size() )
		{
			note( "a static model instance names no mesh" );
			complete = false;
			continue;
		}
		const WorldData::StaticMesh &mesh = world->staticMeshes[instance.mesh];
		if ( !ValidSurfaceSelection( draw.surfaceSelection, mesh.surfaces.size() ) )
		{
			note( "a static model has an invalid surface selection" );
			complete = false;
			continue;
		}
		if ( draw.surfaceSelection && draw.surfaceSelection->empty() )
			continue;
		for ( std::uint32_t surfaceId = 0; surfaceId < mesh.surfaces.size(); ++surfaceId )
		{
			if ( !SurfaceSelected( draw.surfaceSelection, surfaceId ) )
				continue;
			const std::uint32_t materialId = StaticMaterial( mesh, instance, surfaceId );
			const Resources::Material *material =
			    materialId < claims->size() && ( *claims )[materialId].draws
			        ? materialReadyIn( *r.modelResolver, r.modelMaterials, materialId )
			        : nullptr;
			if ( !material && failure.empty() )
				note( "static model " + std::to_string( instance.mesh ) + " surface " +
				      std::to_string( surfaceId ) + " names unclaimed material " +
				      std::to_string( materialId ) );
			if ( !material ||
			     ( material->program.request.drawLayout.IsValid() &&
			         !drawGroupReady( *material, 0 ) ) ||
			     ( material->program.request.frameLayout.IsValid() &&
			         !frameGroupReady( *material ) ) )
			{
				complete = false;
				continue;
			}
			// Each surface's level is its own allocation; the first surface
			// that needs one uploads it and the rest of the level reuses it.
			const std::uint32_t lod = mesh.LodOfSurface( surfaceId );
			if ( lod == ~0u || !modelLevel( instance.mesh, lod ) )
			{
				complete = false;
				continue;
			}
			staticDraws.push_back( { staticCohorts[staticIndex], false, instanceId, instance.mesh,
			    lod, surfaceId, materialId } );
		}
	}
	for ( std::uint32_t poseId = 0; poseId < view.posedModels.size(); ++poseId )
	{
		const WorldView::PosedModel &pose = view.posedModels[poseId];
		if ( pose.mesh >= world->staticMeshes.size() )
		{
			note( "a posed model names no mesh" );
			complete = false;
			continue;
		}
		const WorldData::StaticMesh &mesh = world->staticMeshes[pose.mesh];
		if ( pose.surfaceSelection && pose.surfaceSelection->empty() )
			continue;
		// A posed model draws one hardware level: the selection names that
		// level's surfaces, its index buffer is that level's own, and the host's
		// vertices are the pose of that level.
		WorldData::StaticInstance instance;
		instance.mesh = pose.mesh;
		instance.skin = pose.skin;
		std::uint32_t level = ~0u;
		bool levelRefused = false;
		for ( std::uint32_t surfaceId = 0; surfaceId < mesh.surfaces.size(); ++surfaceId )
		{
			if ( !SurfaceSelected( pose.surfaceSelection, surfaceId ) )
				continue;
			const std::uint32_t lod = mesh.LodOfSurface( surfaceId );
			if ( lod == ~0u || !modelLevel( pose.mesh, lod ) )
			{
				levelRefused = true;
				continue;
			}
			if ( level != ~0u && level != lod )
			{
				note( "a posed model's selection spans two hardware levels" );
				complete = false;
				levelRefused = true;
				break;
			}
			if ( pose.vertices.size() != mesh.lods[lod].vertexCount )
			{
				note( "a posed model's vertices are not its selected level's" );
				complete = false;
				levelRefused = true;
				break;
			}
			level = lod;
			const std::uint32_t materialId = StaticMaterial( mesh, instance, surfaceId );
			if ( materialId >= claims->size() )
			{
				note( "posed model " + std::to_string( pose.mesh ) + " surface " +
				      std::to_string( surfaceId ) + " names missing material " +
				      std::to_string( materialId ) );
				complete = false;
				continue;
			}
			const bool blended = ( *claims )[materialId].blended;
			if ( ( pose.phase == RenderCoreDrawPhase::kOpaque && blended ) ||
			     ( pose.phase == RenderCoreDrawPhase::kBlended && !blended ) )
				continue;
			const Resources::Material *material =
			    materialId < claims->size() && ( *claims )[materialId].draws
			        ? materialReadyIn( *r.modelResolver, r.modelMaterials, materialId )
			        : nullptr;
			if ( !material && failure.empty() )
				note( "posed model " + std::to_string( pose.mesh ) + " surface " +
				      std::to_string( surfaceId ) + " names unclaimed material " +
				      std::to_string( materialId ) );
			if ( !material ||
			     ( material->program.request.drawLayout.IsValid() &&
			         !drawGroupReady( *material, 0 ) ) ||
			     ( material->program.request.frameLayout.IsValid() &&
			         !frameGroupReady( *material ) ) )
			{
				complete = false;
				continue;
			}
			staticDraws.push_back(
			    { posedCohorts[poseId], true, poseId, pose.mesh, lod, surfaceId, materialId } );
		}
		if ( levelRefused || level == ~0u )
			continue;
		// The pose's own vertices, in the level's vertex order, uploaded for
		// this frame: the host's pose changes every frame, so this buffer is
		// per frame while the level's index buffer is resident.
		const auto bytes = std::as_bytes( std::span( pose.vertices ) );
		BufferDesc desc;
		desc.size = bytes.size();
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
		desc.debugName = "posed model vertices";
		auto buffer = device.CreateBuffer( desc );
		if ( !buffer )
		{
			note( "a posed model's vertex buffer was refused" );
			complete = false;
			continue;
		}
		posedBuffers[poseId] = buffer.Value();
		s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
		if ( target.motion.IsValid() )
		{
			if ( pose.previousVertices.size() != pose.vertices.size() )
			{
				note( "temporal model has no previous correspondence buffer" );
				complete = false;
				continue;
			}
			auto previous = device.CreateBuffer( desc );
			if ( !previous )
			{
				complete = false;
				continue;
			}
			previousPosedBuffers[poseId] = previous.Value();
			encoder.TransitionBuffer(
			    previous.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer(
			    previous.Value(), 0, std::as_bytes( std::span( pose.previousVertices ) ) );
			encoder.TransitionBuffer(
			    previous.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
			s.retiredBuffers.emplace_back( target.frame, previous.Value() );
		}

		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, bytes );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
	}

	PublishModelResidency( s );
	preparation.Select( "prepare world view" );
	// The view's planar reflection: the render target the view's programs
	// name (one per view: the client draws one reflection view), imported
	// now, as the stream drew it before this slot (a resize replaces its
	// image, so it is not kept across views).
	int reflectionHandle = 0;
	for ( const std::uint32_t index : order )
	{
		const Resources::Material &m = r.materials[world->surfaces[index].material];
		if ( m.viewInput == 0 || m.viewInput == reflectionHandle )
			continue;
		if ( reflectionHandle != 0 )
			note( "the view's programs read two planar reflections" );
		else
			reflectionHandle = m.viewInput;
	}
	if ( reflectionHandle != 0 )
		viewReflection = textures.Import( reflectionHandle, true );

	// The view's draw constants. D3D9 puts pixel centers on integer
	// coordinates: a D3D9 transform is shifted right and down by half a pixel
	// of the viewport.
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, view.toClip, sizeof( constants.toClip ) );
	if ( view.viewport.width > 0.0f && view.viewport.height > 0.0f )
	{
		for ( int c = 0; c < 4; ++c )
		{
			constants.toClip[0 * 4 + c] += view.toClip[3 * 4 + c] / view.viewport.width;
			constants.toClip[1 * 4 + c] -= view.toClip[3 * 4 + c] / view.viewport.height;
		}
	}
	// The world is in world space: its object-to-world is the identity.
	for ( int i = 0; i < 4; ++i )
		constants.world[i * 5] = 1.0f;
	const auto constantBytes = std::as_bytes( std::span( &constants, 1 ) );
	// The water point's draws with the view's water plane moved (the
	// client's waterZAdjust): world-to-clip after a translation along z.
	material::FamilyDrawConstants waterConstants = constants;
	for ( int row = 0; row < 4; ++row )
		waterConstants.toClip[row * 4 + 3] += constants.toClip[row * 4 + 2] * view.waterZOffset;
	const auto waterConstantBytes = std::as_bytes( std::span( &waterConstants, 1 ) );
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
	auto statePipeline =
	    [&]( const Resources::Material &m, PipelineId base, const material::SurfaceDrawState &state,
	        const shaderlib::DebugSpecialization &debug = {} ) -> std::optional<PipelineId>
	{
		// The SSR targets' variant of the draw's point: three more outputs.
		if ( recordingSsr )
		{
			auto ssr = m.resolver->Program().VariantPipeline(
			    base, material::kSurfaceSsrTargets, 0 );
			if ( !ssr )
			{
				note( "the SSR targets' variant was refused for " + m.program.name );
				return std::nullopt;
			}
			base = ssr.Value();
		}
		auto result =
		    recordingTemporal
		        ? m.resolver->Program().TemporalPipeline( base, state, viewFeatures, debug )
		        : m.resolver->Program().ViewPipeline( base, state, viewFeatures, debug );
		if ( !result )
		{
			note( "view raster state: pipeline refused for " + m.program.name + " status " +
			      std::to_string( int( result.Error() ) ) + " depth format " +
			      std::to_string( int( target.depthFormat ) ) + " stencil " +
			      std::to_string( state.stencil.enabled ) + ": " +
			      m.resolver->Program().PipelineFailure() );
			return std::nullopt;
		}
		return result.Value();
	};
	// World and MDL-reader triangles face counterclockwise from outside.
	// Their owner applies authored culling, including on posed models and
	// private prepasses. Dynamic snapshots keep their captured raster state.
	auto surfaceStatePipeline = [&]( const Resources::Material &m, PipelineId base,
	                                material::SurfaceDrawState state,
	                                const shaderlib::DebugSpecialization &debug = {} )
	{
		state.cull = m.program.twoSided ? CullMode::kNone : CullMode::kBack;
		return statePipeline( m, base, state, debug );
	};
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
	std::map<std::pair<const void *, std::uint64_t>, std::vector<FootprintSlot>> footprintSlots;
	FootprintExtents scratchFootprint;
	std::vector<FootprintExtents> worldFootprints;
	if ( target.mipFeedback && world )
		worldFootprints.resize( world->surfaces.size() );
	std::map<std::tuple<bool, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>,
	    FootprintExtents>
	    modelFootprints;
	auto submitSurfaceFootprints =
	    [&]( const WorldSurface &surface, const std::vector<Resources::Material> &materials,
	        const auto &vertices, const auto &indices, FootprintExtents &extents,
	        const float *objectToWorld = nullptr, const SurfaceFootprintGeometry *compact = nullptr,
	        const SurfaceFootprintBounds *box = nullptr )
	{
		if ( !target.mipFeedback || !view.viewport.width || !view.viewport.height ||
		     surface.firstIndex > indices.size() ||
		     surface.indexCount > indices.size() - surface.firstIndex )
			return;
		// The box's eight corners bound every vertex's projection while all of
		// them are in front of the eye; a corner behind it falls back to the
		// surface's own vertices, which may still all be in front.
		bool boxResolved = false;
		if ( extents.state == FootprintExtents::State::kUnknown && box )
		{
			extents.state = FootprintExtents::State::kInvalid;
			boxResolved = true;
			if ( box->valid )
			{
				float toClip[16];
				for ( int row = 0; row < 4; ++row )
					for ( int col = 0; col < 4; ++col )
					{
						float value = 0.0f;
						for ( int k = 0; k < 4; ++k )
							value += view.toClip[row * 4 + k] *
							         ( objectToWorld ? objectToWorld[k * 4 + col]
							                         : ( k == col ? 1.0f : 0.0f ) );
						toClip[row * 4 + col] = value;
					}
				float minX = view.viewport.width, minY = view.viewport.height;
				float maxX = 0.0f, maxY = 0.0f;
				bool front = true;
				for ( int corner = 0; corner < 8 && front; ++corner )
				{
					const float p[3] = { ( corner & 1 ) ? box->hi[0] : box->lo[0],
					    ( corner & 2 ) ? box->hi[1] : box->lo[1],
					    ( corner & 4 ) ? box->hi[2] : box->lo[2] };
					float clip[4];
					for ( int row = 0; row < 4; ++row )
						clip[row] = toClip[row * 4 + 0] * p[0] + toClip[row * 4 + 1] * p[1] +
						            toClip[row * 4 + 2] * p[2] + toClip[row * 4 + 3];
					if ( !std::isfinite( clip[0] ) || !std::isfinite( clip[1] ) ||
					     !std::isfinite( clip[3] ) || clip[3] <= 1.0e-6f )
					{
						front = false;
						break;
					}
					const float x =
					    std::clamp( ( clip[0] / clip[3] * 0.5f + 0.5f ) * view.viewport.width, 0.0f,
					        view.viewport.width );
					const float y =
					    std::clamp( ( 0.5f - clip[1] / clip[3] * 0.5f ) * view.viewport.height,
					        0.0f, view.viewport.height );
					minX = std::min( minX, x );
					minY = std::min( minY, y );
					maxX = std::max( maxX, x );
					maxY = std::max( maxY, y );
				}
				if ( !front )
				{
					extents.state = FootprintExtents::State::kUnknown;
					boxResolved = false;
				}
				else if ( maxX > minX && maxY > minY && box->maxU >= box->minU &&
				          box->maxV >= box->minV )
					extents = { FootprintExtents::State::kValid, minX, minY, maxX, maxY, box->minU,
					    box->minV, box->maxU, box->maxV };
			}
		}
		if ( !boxResolved && extents.state == FootprintExtents::State::kUnknown )
		{
			extents.state = FootprintExtents::State::kInvalid;
			if ( compact )
			{
				if ( compact->valid )
				{
					float minX = view.viewport.width, minY = view.viewport.height;
					float maxX = 0.0f, maxY = 0.0f;
					bool valid = true;
					for ( const std::array<float, 3> &position : compact->positions )
					{
						float clip[4] = {};
						for ( int row = 0; row < 4; ++row )
							clip[row] = view.toClip[row * 4 + 0] * position[0] +
							            view.toClip[row * 4 + 1] * position[1] +
							            view.toClip[row * 4 + 2] * position[2] +
							            view.toClip[row * 4 + 3];
						if ( !std::isfinite( clip[0] ) || !std::isfinite( clip[1] ) ||
						     !std::isfinite( clip[3] ) || clip[3] <= 1.0e-6f )
						{
							valid = false;
							break;
						}
						const float x =
						    std::clamp( ( clip[0] / clip[3] * 0.5f + 0.5f ) * view.viewport.width,
						        0.0f, view.viewport.width );
						const float y =
						    std::clamp( ( 0.5f - clip[1] / clip[3] * 0.5f ) * view.viewport.height,
						        0.0f, view.viewport.height );
						minX = std::min( minX, x );
						minY = std::min( minY, y );
						maxX = std::max( maxX, x );
						maxY = std::max( maxY, y );
					}
					if ( valid && maxX > minX && maxY > minY && compact->maxU >= compact->minU &&
					     compact->maxV >= compact->minV )
						extents = { FootprintExtents::State::kValid, minX, minY, maxX, maxY,
						    compact->minU, compact->minV, compact->maxU, compact->maxV };
				}
			}
			else
			{
				float minX = view.viewport.width;
				float minY = view.viewport.height;
				float maxX = 0.0f;
				float maxY = 0.0f;
				float minU = std::numeric_limits<float>::max();
				float minV = std::numeric_limits<float>::max();
				float maxU = std::numeric_limits<float>::lowest();
				float maxV = std::numeric_limits<float>::lowest();
				bool valid = true;
				for ( std::uint32_t i = 0; i < surface.indexCount; ++i )
				{
					const std::uint32_t vertexIndex = indices[surface.firstIndex + i];
					if ( vertexIndex >= vertices.size() )
						valid = false;
					if ( !valid )
						break;
					const auto &vertex = vertices[vertexIndex];
					if ( !std::isfinite( vertex.uv[0] ) || !std::isfinite( vertex.uv[1] ) )
					{
						valid = false;
						break;
					}
					float position[4] = {
					    vertex.position[0], vertex.position[1], vertex.position[2], 1.0f };
					if ( objectToWorld )
					{
						float transformed[4] = {};
						for ( int row = 0; row < 4; ++row )
							for ( int col = 0; col < 4; ++col )
								transformed[row] += objectToWorld[row * 4 + col] * position[col];
						std::copy_n( transformed, 4, position );
					}
					float clip[4] = {};
					for ( int row = 0; row < 4; ++row )
						for ( int col = 0; col < 4; ++col )
							clip[row] += view.toClip[row * 4 + col] * position[col];
					if ( !std::isfinite( clip[0] ) || !std::isfinite( clip[1] ) ||
					     !std::isfinite( clip[3] ) || clip[3] <= 1.0e-6f )
					{
						valid = false;
						break;
					}
					const float x =
					    std::clamp( ( clip[0] / clip[3] * 0.5f + 0.5f ) * view.viewport.width, 0.0f,
					        view.viewport.width );
					const float y =
					    std::clamp( ( 0.5f - clip[1] / clip[3] * 0.5f ) * view.viewport.height,
					        0.0f, view.viewport.height );
					minX = std::min( minX, x );
					minY = std::min( minY, y );
					maxX = std::max( maxX, x );
					maxY = std::max( maxY, y );
					minU = std::min( minU, vertex.uv[0] );
					minV = std::min( minV, vertex.uv[1] );
					maxU = std::max( maxU, vertex.uv[0] );
					maxV = std::max( maxV, vertex.uv[1] );
				}
				if ( valid && maxX > minX && maxY > minY && maxU >= minU && maxV >= minV )
				{
					extents = { FootprintExtents::State::kValid, minX, minY, maxX, maxY, minU, minV,
					    maxU, maxV };
				}
			}
		}
		if ( extents.state != FootprintExtents::State::kValid )
			return;
		const float minX = extents.minX, minY = extents.minY, maxX = extents.maxX,
		            maxY = extents.maxY, minU = extents.minU, minV = extents.minV,
		            maxU = extents.maxU, maxV = extents.maxV;
		const void **seen = extents.submitted;
		const void **free = nullptr;
		for ( int i = 0; i < 3; ++i )
		{
			if ( seen[i] == &materials )
				return;
			if ( !seen[i] && !free )
				free = &seen[i];
		}
		if ( free )
			*free = &materials;
		auto slots = footprintSlots.find( { &materials, surface.material } );
		if ( slots == footprintSlots.end() )
		{
			std::vector<FootprintSlot> resolved;
			const auto &handles = ( *claims )[surface.material].handles;
			const auto &texturesUsed =
			    materials[surface.material].program.request.material.textures;
			for ( std::uint32_t slot = 0; slot < texturesUsed.size(); ++slot )
			{
				const auto handle = handles.find( texturesUsed[slot].name );
				if ( handle == handles.end() || handle->second <= 0 )
					continue;
				const auto info = target.textures->MipDescription( handle->second );
				if ( !info )
					continue;
				resolved.push_back( { slot, info->width, info->height, info->levels } );
			}
			slots = footprintSlots
			            .emplace(
			                std::pair{ static_cast<const void *>( &materials ), surface.material },
			                std::move( resolved ) )
			            .first;
		}
		for ( const FootprintSlot &slot : slots->second )
		{
			resources::VisibleTextureFootprint footprint;
			footprint.material = surface.material;
			footprint.textureSlot = slot.slot;
			footprint.textureWidth = slot.width;
			footprint.textureHeight = slot.height;
			footprint.mipLevels = slot.levels;
			footprint.screenWidth = maxX - minX;
			footprint.screenHeight = maxY - minY;
			footprint.uvWidth = maxU - minU;
			footprint.uvHeight = maxV - minV;
			target.mipFeedback->AddVisible( footprint );
		}
	};
	auto drawSurfaces =
	    [&]( const std::vector<std::uint32_t> &list,
	        const std::vector<Resources::Material> &materials,
	        const std::function<std::optional<PipelineId>( const Resources::Material & )>
	            &pipelineOf,
	        bool breakdown = false )
	{
		RecordSection family( encoder );
		std::uint32_t boundMaterial = ~0u;
		int boundPage = 0;
		bool pageBound = false;
		bool skipping = false;
		std::uint32_t runFirst = 0;
		std::uint32_t runCount = 0;
		auto flushRun = [&]()
		{
			if ( runCount )
				encoder.DrawIndexed( runCount, 1, runFirst, 0, 0 );
			runCount = 0;
		};
		for ( const std::uint32_t index : list )
		{
			const WorldSurface &surface = world->surfaces[index];
			const Resources::Material &m = materials[surface.material];
			if ( surface.material != boundMaterial )
			{
				flushRun();
				if ( breakdown )
					family.Select( "world / ", m.program.name );
				boundMaterial = surface.material;
				pageBound = false;
				const std::optional<PipelineId> pipeline = pipelineOf( m );
				skipping = !pipeline;
				if ( skipping )
				{
					complete = false;
					continue;
				}
				encoder.SetPipeline( *pipeline );
				if ( m.program.request.frameLayout.IsValid() )
					encoder.SetBindGroup( BindGroupRole::kFrame,
					    r.frameGroups[m.program.request.frameLayout.value].group );
				if ( m.program.request.viewLayout.IsValid() )
				{
					const std::uint64_t layout = m.program.request.viewLayout.value;
					const auto lit = litViews.find( layout );
					const auto reflect = reflectViews.find( layout );
					encoder.SetBindGroup(
					    BindGroupRole::kView, lit != litViews.end() ? lit->second->group
					                          : m.viewInput != 0 && reflect != reflectViews.end()
					                              ? reflect->second.group
					                              : r.viewGroups[layout].group );
				}
				encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
				encoder.SetVertexBuffer( 0, r.vertices, 0 );
				if ( recordingTemporal )
					encoder.SetVertexBuffer( 1, r.vertices, 0 );
				encoder.SetIndexBuffer( r.indices, 0, IndexFormat::kUint32 );
				encoder.SetDrawConstants(
				    0, ( m.program.name == "water" && view.waterZOffset != 0.0f ? waterConstantBytes
				                                                                : constantBytes )
				           .first( m.program.request.drawConstantBytes ) );
			}
			if ( skipping )
				continue;
			submitSurfaceFootprints( surface, materials, world->vertices, world->indices,
			    worldFootprints.empty() ? scratchFootprint : worldFootprints[index], nullptr,
			    footprintGeometry && index < footprintGeometry->size()
			        ? &( *footprintGeometry )[index]
			        : nullptr );
			if ( m.program.request.drawLayout.IsValid() &&
			     ( !pageBound || surface.lightmapPage != boundPage ) )
			{
				flushRun();
				encoder.SetBindGroup(
				    BindGroupRole::kDraw, r.drawGroups[drawKey( m, surface.lightmapPage )].group );
				boundPage = surface.lightmapPage;
				pageBound = true;
			}
			if ( runCount && surface.firstIndex == runFirst + runCount )
			{
				runCount += surface.indexCount;
			}
			else
			{
				flushRun();
				runFirst = surface.firstIndex;
				runCount = surface.indexCount;
			}
		}
		flushRun();
	};

	auto recordModel =
	    [&]( const StaticDraw &draw, const Resources::Material &m, PipelineId pipeline )
	{
		const WorldData::StaticInstance *instance =
		    draw.posed ? nullptr : &world->staticInstances[draw.instance];
		const WorldData::StaticMesh &mesh = world->staticMeshes[draw.mesh];
		const WorldSurface &surface = mesh.surfaces[draw.surface];
		encoder.SetPipeline( pipeline );
		if ( m.program.request.frameLayout.IsValid() )
			encoder.SetBindGroup(
			    BindGroupRole::kFrame, r.frameGroups[m.program.request.frameLayout.value].group );
		if ( m.program.request.viewLayout.IsValid() )
		{
			const std::uint64_t layout = m.program.request.viewLayout.value;
			const auto lit = modelLitViews.find( layout );
			const auto scene = sceneViews.find( layout );
			encoder.SetBindGroup( BindGroupRole::kView,
			    ( m.program.sceneColor || m.program.depthBlend ) && scene != sceneViews.end()
			        ? scene->second.group
			    : lit != modelLitViews.end() ? lit->second->group
			                                 : r.viewGroups[layout].group );
		}
		encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
		if ( m.program.request.drawLayout.IsValid() )
			encoder.SetBindGroup( BindGroupRole::kDraw, r.drawGroups[drawKey( m, 0 )].group );
		material::FamilyDrawConstants modelConstants;
		if ( instance )
			std::copy( instance->world, instance->world + 16, modelConstants.world );
		else
		{
			for ( int i = 0; i < 4; ++i )
				modelConstants.world[i * 5] = 1.0f;
		}
		for ( int row = 0; row < 4; ++row )
		{
			for ( int col = 0; col < 4; ++col )
			{
				float value = 0.0f;
				for ( int k = 0; k < 4; ++k )
					value += constants.toClip[row * 4 + k] * modelConstants.world[k * 4 + col];
				modelConstants.toClip[row * 4 + col] = value;
			}
		}
		encoder.SetDrawConstants( 0, std::as_bytes( std::span( &modelConstants, 1 ) )
		                                 .first( m.program.request.drawConstantBytes ) );
		const State::ModelLevel &buffers = s.models.models[draw.mesh][draw.lod];
		encoder.SetVertexBuffer( 0, draw.posed ? posedBuffers[draw.instance] : buffers.vertices );
		if ( recordingTemporal )
			encoder.SetVertexBuffer(
			    1, draw.posed ? previousPosedBuffers[draw.instance] : buffers.vertices );
		encoder.SetIndexBuffer( buffers.indices, 0, IndexFormat::kUint32 );
		WorldSurface feedbackSurface = surface;
		feedbackSurface.material = draw.material;
		// The level's own staging, whose indices the level's surface counts into.
		const WorldData::StaticMeshLod &level = mesh.lods[draw.lod];
		if ( level.vertices && level.indices && target.mipFeedback )
		{
			const auto &vertices =
			    draw.posed ? view.posedModels[draw.instance].vertices : *level.vertices;
			FootprintExtents &extents =
			    modelFootprints[{ draw.posed, draw.instance, draw.mesh, draw.lod, draw.surface }];
			// A static instance's box is the world's, built once; a pose's box
			// is measured once per view from its own vertices.
			SurfaceFootprintBounds posedBox;
			const SurfaceFootprintBounds *box = nullptr;
			if ( extents.state == FootprintExtents::State::kUnknown )
			{
				if ( draw.posed )
				{
					posedBox = MeasureFootprintBounds( surface, vertices, *level.indices );
					box = &posedBox;
				}
				else if ( modelFootprintBounds && draw.mesh < modelFootprintBounds->size() &&
				          draw.surface < ( *modelFootprintBounds )[draw.mesh].size() &&
				          draw.surface < mesh.surfaceLods.size() &&
				          mesh.surfaceLods[draw.surface] == draw.lod )
					box = &( *modelFootprintBounds )[draw.mesh][draw.surface];
			}
			submitSurfaceFootprints( feedbackSurface, r.modelMaterials, vertices, *level.indices,
			    extents, instance ? instance->world : nullptr, nullptr, box );
		}
		encoder.DrawIndexed( surface.indexCount, 1, surface.firstIndex, 0, 0 );
	};
	preparation.End();
	// Cutout casters (RFC 0016 K12): the stage's alpha-tested surfaces, each
	// through its own material's depth point, drawn over the atlas the
	// composition's other casters drew, before any view reads it.
	std::uint64_t cutoutDraws = 0;
	std::uint64_t cutoutRefused = 0;
	std::uint64_t cutoutNotResident = 0;
	if ( world->stage && target.cutoutShadows && target.cutoutShadows->atlas.IsValid() &&
	     !target.cutoutShadows->views.empty() )
	{
		const WorldCutoutShadows &cutouts = *target.cutoutShadows;
		struct CutoutDraw
		{
			PipelineId pipeline;
			const Resources::Material *material = nullptr;
			const Group *frame = nullptr;
			const Group *view = nullptr;
			const Group *draw = nullptr;
			const WorldSurface *surface = nullptr;
			// A static prop's: its level's buffers and its object-to-world.
			BufferId vertices;
			BufferId indices;
			const float *world = nullptr;
		};
		std::vector<CutoutDraw> draws;
		std::uint64_t notResident = 0;
		// The material's depth point, once per material; a blended or
		// transmitting point is no opaque caster (the program refuses it, and
		// it casts nothing, as the visible point lets light through).
		auto depthPoint = [&]( std::map<std::uint32_t, PipelineId> &cache, std::uint32_t index,
		                      const Resources::Material &m )
		{
			auto cached = cache.find( index );
			if ( cached == cache.end() )
			{
				auto pipeline = m.resolver->Program().ShadowPipeline( m.program.request.pipeline );
				cached = cache.emplace( index, pipeline ? pipeline.Value() : PipelineId() ).first;
			}
			return cached->second;
		};
		auto casterGroups = [&]( Resources::Material &m, CutoutDraw &cutout ) -> bool
		{
			if ( m.program.foliage )
			{
				note( "an animated ($treesway) cutout casts no cached shadow yet" );
				return false;
			}
			const Group *frame =
			    m.program.request.frameLayout.IsValid() ? frameGroupReady( m ) : nullptr;
			const Group *draw =
			    m.program.request.drawLayout.IsValid() ? drawGroupReady( m, 0 ) : nullptr;
			const Group *viewGroup = nullptr;
			if ( m.program.request.viewLayout.IsValid() )
			{
				Group &group = r.viewGroups[m.program.request.viewLayout.value];
				std::string why;
				if ( !group.group.IsValid() && m.program.request.neutralView &&
				     !buildGroup( *m.program.request.neutralView, {}, group, &why ) )
					s.ReleaseGroup( group, CompletionToken() );
				viewGroup = group.group.IsValid() ? &group : nullptr;
			}
			if ( ( m.program.request.frameLayout.IsValid() && !frame ) ||
			     ( m.program.request.drawLayout.IsValid() && !draw ) ||
			     ( m.program.request.viewLayout.IsValid() && !viewGroup ) )
			{
				note( "a cutout caster's groups are not ready" );
				return false;
			}
			cutout.material = &m;
			cutout.frame = frame;
			cutout.view = viewGroup;
			cutout.draw = draw;
			return true;
		};
		for ( const auto &[instanceIndex, surfaceIndex] : cutouts.staticSurfaces )
		{
			if ( instanceIndex >= world->staticInstances.size() )
				continue;
			const WorldData::StaticInstance &instance = world->staticInstances[instanceIndex];
			if ( instance.mesh >= world->staticMeshes.size() )
				continue;
			const WorldData::StaticMesh &mesh = world->staticMeshes[instance.mesh];
			if ( surfaceIndex >= mesh.surfaces.size() )
				continue;
			const std::uint32_t material = StaticMaterial( mesh, instance, surfaceIndex );
			const std::uint32_t lod = mesh.LodOfSurface( surfaceIndex );
			if ( lod == ~0u || instance.mesh >= s.models.models.size() ||
			     lod >= s.models.models[instance.mesh].size() ||
			     material >= r.modelMaterials.size() || material >= claims->size() ||
			     !( *claims )[material].draws )
				continue;
			State::ModelLevel &level = s.models.models[instance.mesh][lod];
			if ( !level.resident || !level.vertices.IsValid() || !level.indices.IsValid() )
			{
				++notResident;
				continue;
			}
			Resources::Material *m =
			    materialReadyIn( *r.modelResolver, r.modelMaterials, material );
			CutoutDraw cutout;
			if ( !m || !casterGroups( *m, cutout ) )
			{
				++cutoutRefused;
				continue;
			}
			cutout.pipeline = depthPoint( r.cutoutModelPipelines, material, *m );
			if ( !cutout.pipeline.IsValid() )
				continue;
			level.lastUsedFrame = std::max( level.lastUsedFrame, target.frame );
			cutout.surface = &mesh.surfaces[surfaceIndex];
			cutout.vertices = level.vertices;
			cutout.indices = level.indices;
			cutout.world = instance.world;
			draws.push_back( cutout );
		}
		for ( const std::uint32_t index : cutouts.surfaces )
		{
			if ( index >= world->surfaces.size() )
				continue;
			const WorldSurface &surface = world->surfaces[index];
			Resources::Material *m = materialReady( surface.material );
			if ( !m )
			{
				++cutoutRefused;
				continue;
			}
			if ( m->program.foliage )
			{
				++cutoutRefused;
				note( "an animated ($treesway) cutout casts no cached shadow yet" );
				continue;
			}
			auto cached = r.cutoutPipelines.find( surface.material );
			if ( cached == r.cutoutPipelines.end() )
				cached = r.cutoutPipelines
				             .emplace( surface.material,
				                 depthPoint( r.cutoutPipelines, surface.material, *m ) )
				             .first;
			if ( !cached->second.IsValid() )
				continue;
			const Group *frame =
			    m->program.request.frameLayout.IsValid() ? frameGroupReady( *m ) : nullptr;
			const Group *draw =
			    m->program.request.drawLayout.IsValid() ? drawGroupReady( *m, 0 ) : nullptr;
			const Group *viewGroup = nullptr;
			if ( m->program.request.viewLayout.IsValid() )
			{
				Group &group = r.viewGroups[m->program.request.viewLayout.value];
				std::string why;
				if ( !group.group.IsValid() && m->program.request.neutralView &&
				     !buildGroup( *m->program.request.neutralView, {}, group, &why ) )
					s.ReleaseGroup( group, CompletionToken() );
				viewGroup = group.group.IsValid() ? &group : nullptr;
			}
			if ( ( m->program.request.frameLayout.IsValid() && !frame ) ||
			     ( m->program.request.drawLayout.IsValid() && !draw ) ||
			     ( m->program.request.viewLayout.IsValid() && !viewGroup ) )
			{
				++cutoutRefused;
				note( "a cutout caster's groups are not ready" );
				continue;
			}
			draws.push_back( { cached->second, m, frame, viewGroup, draw, &surface, r.vertices,
			    r.indices, nullptr } );
		}
		if ( !draws.empty() )
		{
			preparation.Select( "cutout shadows" );
			encoder.TransitionTexture(
			    cutouts.atlas, ResourceUsage::kSampled, ResourceUsage::kDepthWrite );
			DepthAttachment depth;
			depth.texture = cutouts.atlas;
			depth.load = LoadOp::kLoad;
			RenderingDesc rendering;
			rendering.depth = depth;
			rendering.width = cutouts.atlasSize;
			rendering.height = cutouts.atlasSize;
			encoder.BeginRendering( rendering );
			for ( const WorldShadowView &shadowView : cutouts.views )
			{
				encoder.SetViewport( { float( shadowView.x ), float( shadowView.y ),
				    float( shadowView.size ), float( shadowView.size ), 0.0f, 1.0f } );
				for ( const CutoutDraw &cutout : draws )
				{
					// toClip = the view's matrix times the caster's
					// object-to-world (the identity for the world).
					material::FamilyDrawConstants shadowConstants;
					for ( int i = 0; i < 4; ++i )
						shadowConstants.world[i * 5] = 1.0f;
					if ( cutout.world )
						std::copy( cutout.world, cutout.world + 16, shadowConstants.world );
					for ( int row = 0; row < 4; ++row )
						for ( int col = 0; col < 4; ++col )
						{
							float value = 0.0f;
							for ( int k = 0; k < 4; ++k )
								value += shadowView.viewProjection[row * 4 + k] *
								         shadowConstants.world[k * 4 + col];
							shadowConstants.toClip[row * 4 + col] = value;
						}
					const auto shadowBytes = std::as_bytes( std::span( &shadowConstants, 1 ) );
					const auto &request = cutout.material->program.request;
					encoder.SetPipeline( cutout.pipeline );
					encoder.SetBindGroup( BindGroupRole::kMaterial, cutout.material->group.group );
					if ( cutout.frame )
						encoder.SetBindGroup( BindGroupRole::kFrame, cutout.frame->group );
					if ( cutout.view )
						encoder.SetBindGroup( BindGroupRole::kView, cutout.view->group );
					if ( cutout.draw )
						encoder.SetBindGroup( BindGroupRole::kDraw, cutout.draw->group );
					encoder.SetVertexBuffer( 0, cutout.vertices, 0 );
					encoder.SetIndexBuffer( cutout.indices, 0, IndexFormat::kUint32 );
					encoder.SetDrawConstants( 0, shadowBytes.first( request.drawConstantBytes ) );
					encoder.DrawIndexed(
					    cutout.surface->indexCount, 1, cutout.surface->firstIndex, 0, 0 );
					++cutoutDraws;
				}
			}
			encoder.EndRendering();
			encoder.TransitionTexture(
			    cutouts.atlas, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
			preparation.End();
		}
		cutoutNotResident = notResident;
	}
	// A world stage's screen passes (render_lab's order): the depth and
	// normal prepass into the pass's own single-sample targets, with a
	// resolver of their own, then the composition's ambient occlusion, which
	// the lit view groups read.
	auto opaquePbr = []( const Resources::Material &m )
	{
		return m.program.blend == BlendMode::kOpaque && !m.program.sceneColor &&
		       m.program.name == "pbr";
	};
	std::array<float, 48> screenInputs;
	std::copy_n( view.toClip, 16, screenInputs.begin() );
	screenInputs[16] = view.viewport.x;
	screenInputs[17] = view.viewport.y;
	screenInputs[18] = view.viewport.width;
	screenInputs[19] = view.viewport.height;
	std::memcpy( screenInputs.data() + 20, target.clipPlanes, sizeof( target.clipPlanes ) );
	std::copy_n( target.foliage[0], 4, screenInputs.begin() + 44 );
	const bool screenReusable =
	    target.frame != 0 && s.screenFrame == target.frame && s.screenDepth == r.prepassDepth &&
	    s.screenOutput == target.ambientOcclusion && s.screenInputs == screenInputs;
	if ( world->stage && target.screenPasses && target.ambientOcclusion.IsValid() &&
	     screenReusable )
		viewOcclusion = target.ambientOcclusion;
	if ( world->stage && target.screenPasses && target.ambientOcclusion.IsValid() &&
	     !screenReusable )
	{
		preparation.Select( "prepare world prepass" );
		if ( !r.prepassResolver )
		{
			auto resolver = material::ProgramResolver::Create( device, Format::kRGBA16Float,
			    Format::kD32Float, 1, material::VertexLayout::kSurface );
			if ( resolver )
			{
				r.prepassResolver = std::move( resolver ).Value();
				r.prepassResolver->SetWorldPbr(
				    true, StageTerms( *world->stage, target.runtimeDirect ) );
				r.prepassMaterials.resize( world->materials.size() );
			}
			else
			{
				note( "the prepass resolver: " + resolver.Error() );
			}
		}
		if ( !r.prepassModelResolver )
		{
			auto resolver = material::ProgramResolver::Create( device, Format::kRGBA16Float,
			    Format::kD32Float, 1, material::VertexLayout::kModel );
			if ( resolver )
			{
				r.prepassModelResolver = std::move( resolver ).Value();
				r.prepassModelResolver->SetWorldPbr(
				    true, StageTerms( *world->stage, target.runtimeDirect ) );
				r.prepassModelResolver->SetSceneColorAvailable( true );
				r.prepassModelMaterials.resize( world->materials.size() );
			}
			else
				note( "the model prepass resolver: " + resolver.Error() );
		}
		if ( r.prepassResolver && ( r.prepassDepthDesc.width != target.width ||
		                              r.prepassDepthDesc.height != target.height ) )
		{
			for ( TextureId old : { r.prepassDepth, r.prepassNormal } )
			{
				if ( old.IsValid() )
					s.retiredTextures.emplace_back( target.frame, old );
			}
			r.prepassDepth = r.prepassNormal = TextureId();
			TextureDesc depthDesc;
			depthDesc.format = Format::kD32Float;
			depthDesc.width = target.width;
			depthDesc.height = target.height;
			depthDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
			depthDesc.debugName = "world prepass depth";
			TextureDesc normalDesc = depthDesc;
			normalDesc.format = Format::kRGBA16Float;
			normalDesc.usages = { ResourceUsage::kColorAttachment, ResourceUsage::kSampled };
			normalDesc.debugName = "world prepass normal";
			auto depthTexture = device.CreateTexture( depthDesc );
			auto normalTexture = device.CreateTexture( normalDesc );
			if ( depthTexture && normalTexture )
			{
				r.prepassDepth = depthTexture.Value();
				r.prepassNormal = normalTexture.Value();
				depthDesc.debugName = normalDesc.debugName = {};
				r.prepassDepthDesc = depthDesc;
				r.prepassNormalDesc = normalDesc;
				r.prepassUsed = false;
			}
			else
			{
				for ( auto *made : { &depthTexture, &normalTexture } )
				{
					if ( *made )
						(void)device.Release( made->Value(), CompletionToken() );
				}
				r.prepassDepthDesc = TextureDesc();
				note( "the prepass targets were refused" );
			}
		}
		if ( r.prepassResolver && r.prepassDepth.IsValid() )
		{
			// The lists are the world's, not the view's: the same entries are
			// drawn into the pass's own targets whichever view needs them, and
			// they change only with the world, its claims and which model
			// levels are resident. Built once for this resource set, retried
			// while any material or group in them is not ready.
			Resources::PrepassLists &cached = r.prepassLists;
			const bool reusable = cached.valid && cached.generation == generation &&
			                      cached.residencyRevision == s.modelResidencyRevision &&
			                      cached.claims == claims.get();
			std::vector<std::uint32_t> prepassStorage;
			std::vector<StaticDraw> prepassModelStorage;
			// A material or group that was not ready leaves an entry out, so a
			// partial build is never remembered: the next view tries again.
			bool listsComplete = true;
			if ( !reusable )
			{
				cached = Resources::PrepassLists();
				cached.generation = generation;
				cached.residencyRevision = s.modelResidencyRevision;
				cached.claims = claims.get();
			}
			std::vector<std::uint32_t> &prepass = reusable ? cached.surfaces : prepassStorage;
			// Draw, frame and view groups for one prepass material. Only the
			// index list is cached between calls; the view group especially is
			// this call's state, so a reused list resolves them again below.
			auto prepassGroupsReady = [&]( const Resources::Material &m, int page ) -> bool
			{
				return ( !m.program.request.drawLayout.IsValid() || drawGroupReady( m, page ) ) &&
				       ( !m.program.request.frameLayout.IsValid() || frameGroupReady( m ) ) &&
				       ( !m.program.request.viewLayout.IsValid() || viewGroupReady( m ) );
			};
			for ( std::uint32_t index = 0;
			     index < world->surfaces.size() && !reusable; ++index )
			{
				const WorldSurface &surface = world->surfaces[index];
				if ( surface.material >= claims->size() || !( *claims )[surface.material].draws )
					continue;
				const Resources::Material *m =
				    materialReadyIn( *r.prepassResolver, r.prepassMaterials, surface.material );
				if ( !m || !prepassGroupsReady( *m, surface.lightmapPage ) )
				{
					complete = false;
					listsComplete = false;
					continue;
				}
				if ( opaquePbr( *m ) )
					prepass.push_back( index );
			}
			std::vector<StaticDraw> &prepassModels =
			    reusable ? cached.models : prepassModelStorage;
			if ( r.prepassModelResolver && !reusable )
			{
				// The prepass covers the world's instances, not this view's, so
				// it draws only the levels already resident: a released level is
				// uploaded when a view selects it, not by this pass over every
				// instance (which would keep every level resident forever).
				for ( std::uint32_t id = 0; id < world->staticInstances.size(); ++id )
				{
					const auto &instance = world->staticInstances[id];
					if ( instance.mesh >= world->staticMeshes.size() )
						continue;
					const auto &mesh = world->staticMeshes[instance.mesh];
					for ( std::uint32_t surface = 0; surface < mesh.surfaces.size(); ++surface )
					{
						if ( !SurfaceSelected( instance.surfaceSelection, surface ) )
							continue;
						const std::uint32_t material = StaticMaterial( mesh, instance, surface );
						if ( material >= claims->size() || !( *claims )[material].draws ||
						     !world->materials[material].mesh )
							continue;
						const auto *m = materialReadyIn(
						    *r.prepassModelResolver, r.prepassModelMaterials, material );
						if ( !m )
						{
							complete = false;
							listsComplete = false;
							continue;
						}
						if ( !opaquePbr( *m ) )
							continue;
						const std::uint32_t lod = mesh.LodOfSurface( surface );
						if ( lod == ~0u || lod >= s.models.models[instance.mesh].size() ||
						     !s.models.models[instance.mesh][lod].resident )
							continue;
						if ( !prepassGroupsReady( *m, 0 ) )
						{
							complete = false;
							listsComplete = false;
							continue;
						}
						prepassModels.push_back(
						    { 0, false, id, instance.mesh, lod, surface, material } );
					}
				}
			}
			// A reused list must still resolve the groups its entries bind: they
			// belong to this call, not to the cache. An entry whose groups are not
			// ready now is left out of this record without changing the cached
			// list, so a later call can draw it.
			std::vector<std::uint32_t> reusedPrepass;
			std::vector<StaticDraw> reusedPrepassModels;
			const std::vector<std::uint32_t> *drawnPrepass = &prepass;
			const std::vector<StaticDraw> *drawnPrepassModels = &prepassModels;
			if ( reusable )
			{
				reusedPrepass.reserve( prepass.size() );
				for ( const std::uint32_t index : prepass )
				{
					const WorldSurface &surface = world->surfaces[index];
					const Resources::Material *m =
					    surface.material < claims->size() && ( *claims )[surface.material].draws
					        ? materialReadyIn(
					              *r.prepassResolver, r.prepassMaterials, surface.material )
					        : nullptr;
					if ( m && prepassGroupsReady( *m, surface.lightmapPage ) )
						reusedPrepass.push_back( index );
					else
						complete = false;
				}
				drawnPrepass = &reusedPrepass;
				if ( r.prepassModelResolver )
				{
					reusedPrepassModels.reserve( prepassModels.size() );
					for ( const StaticDraw &draw : prepassModels )
					{
						const auto *m = materialReadyIn(
						    *r.prepassModelResolver, r.prepassModelMaterials, draw.material );
						if ( m && prepassGroupsReady( *m, 0 ) )
							reusedPrepassModels.push_back( draw );
						else
							complete = false;
					}
					drawnPrepassModels = &reusedPrepassModels;
				}
			}
			// The targets rest in kSampled between views (undefined before
			// their first); their contents are rewritten whole.
			const ResourceUsage rest =
			    r.prepassUsed ? ResourceUsage::kSampled : ResourceUsage::kUndefined;
			encoder.TransitionTexture( r.prepassNormal, rest, ResourceUsage::kColorAttachment );
			encoder.TransitionTexture( r.prepassDepth, rest, ResourceUsage::kDepthWrite );
			r.prepassUsed = true;
			const ColorAttachment normal[] = {
			    { r.prepassNormal, LoadOp::kClear, StoreOp::kStore, { 0, 0, 1, 0 }, {} } };
			RenderingDesc prepassRendering;
			prepassRendering.colors = normal;
			prepassRendering.depth =
			    DepthAttachment{ r.prepassDepth, LoadOp::kClear, StoreOp::kStore, 1.0f };
			prepassRendering.width = target.width;
			prepassRendering.height = target.height;
			preparation.End();
			encoder.BeginLabel( "core world prepass" );
			encoder.BeginRendering( prepassRendering );
			// This private D32 target has no stencil and encodes projection
			// depth for reconstruction. Portal masks and depth-range remapping
			// belong to the final target, not this offscreen view.
			Viewport prepassViewport = view.viewport;
			prepassViewport.minDepth = 0.0f;
			prepassViewport.maxDepth = 1.0f;
			encoder.SetViewport( prepassViewport );
			drawSurfaces( *drawnPrepass, r.prepassMaterials,
			    [&]( const Resources::Material &m ) -> std::optional<PipelineId>
			    {
				    auto variant = m.resolver->VariantPipeline(
				        m.program, material::kSurfaceDepthNormal, material::kSurfaceSsrTargets );
				    if ( !variant )
				    {
					    note( "the prepass: " + variant.Error() );
					    return std::nullopt;
				    }
				    return surfaceStatePipeline( m, variant.Value(), material::SurfaceDrawState() );
			    } );
			for ( const StaticDraw &draw : *drawnPrepassModels )
			{
				const auto &m = r.prepassModelMaterials[draw.material];
				auto variant = m.resolver->VariantPipeline(
				    m.program, material::kSurfaceDepthNormal, material::kSurfaceSsrTargets );
				if ( !variant )
				{
					complete = false;
					note( "the model prepass: " + variant.Error() );
					continue;
				}
				const auto pipeline =
				    surfaceStatePipeline( m, variant.Value(), material::SurfaceDrawState() );
				if ( pipeline )
					recordModel( draw, m, *pipeline );
				else
					complete = false;
			}
			encoder.EndRendering();
			encoder.EndLabel();
			encoder.TransitionTexture(
			    r.prepassNormal, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
			encoder.TransitionTexture(
			    r.prepassDepth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
			if ( target.screenPasses(
			         encoder, { r.prepassDepth, r.prepassNormal, target.width, target.height } ) )
			{
				viewOcclusion = target.ambientOcclusion;
				s.screenFrame = target.frame;
				s.screenDepth = r.prepassDepth;
				s.screenOutput = target.ambientOcclusion;
				s.screenInputs = screenInputs;
			}
			else
			{
				complete = false;
				note( "the camera's ambient occlusion was not recorded" );
			}
			// Keep a complete build for the next view and the next frame; the
			// aliases above are finished with.
			if ( !reusable && listsComplete )
			{
				cached.surfaces = std::move( prepassStorage );
				cached.models = std::move( prepassModelStorage );
				cached.valid = true;
				++s.stats.prepassListBuilds;
			}
			else if ( !reusable )
			{
				++s.stats.prepassListRetries;
			}
			else
			{
				++s.stats.prepassListReuses;
			}
		}
	}
	preparation.Select( "prepare lit view bindings" );
	// No screen passes (ambient occlusion off): the target holds the
	// neutral occlusion, one.
	if ( world->stage && !target.screenPasses && target.ambientOcclusion.IsValid() )
		viewOcclusion = target.ambientOcclusion;
	// The lit view groups, with the occlusion when it was recorded.
	std::erase_if( order,
	    [&]( std::uint32_t index )
	    {
		    const Resources::Material &m = r.materials[world->surfaces[index].material];
		    if ( m.program.request.viewLayout.IsValid() && !viewGroupReady( m ) )
		    {
			    complete = false;
			    return true;
		    }
		    return false;
	    } );
	std::erase_if( staticDraws,
	    [&]( const StaticDraw &draw )
	    {
		    const Resources::Material &m = r.modelMaterials[draw.material];
		    if ( m.program.sceneColor )
			    return false; // the snapshot is recorded after opaque draws
		    if ( m.program.request.viewLayout.IsValid() && !viewGroupReady( m ) )
		    {
			    complete = false;
			    return true;
		    }
		    return false;
	    } );

	preparation.Select( "prepare dynamic materials and uploads" );
	struct DynamicDraw
	{
		Resources::Material *material;
		BufferId vertices;
		BufferId indices;
		std::uint32_t count;
		int page;
		const WorldView::DynamicDraw *source;
	};
	std::vector<DynamicDraw> dynamicDraws;
	for ( const WorldView::DynamicDraw &draw : view.dynamicDraws )
	{
		std::string key;
		const auto entry = s.Mapped( draw.material, key );
		const State::MappedMaterial &mapped = *entry;
		if ( !mapped || draw.vertices.empty() || draw.indices.empty() ||
		     draw.indices.size() % 3 != 0 ||
		     std::any_of( draw.indices.begin(), draw.indices.end(),
		         [&]( std::uint32_t index )
		         {
			         return index >= draw.vertices.size();
		         } ) )
		{
			note( "dynamic material " + draw.material.name + ": " +
			      ( mapped ? "invalid triangle geometry" : mapped.Error() ) );
			complete = false;
			continue;
		}
		Resources::Material &cached = r.dynamicMaterials[key];
		Resources::Material *m =
		    prepareMaterial( *r.resolver, cached, mapped.Value(), draw.material );
		if ( !m || !drawGroupReady( *m, draw.lightmapPage, draw.capturedLightmap ) ||
		     !frameGroupReady( *m ) || ( !m->program.sceneColor && !viewGroupReady( *m ) ) )
		{
			complete = false;
			continue;
		}
		BufferDesc desc;
		desc.size = draw.vertices.size() * sizeof( WorldVertex );
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
		desc.debugName = "core dynamic vertices";
		auto vertices = device.CreateBuffer( desc );
		desc.size = draw.indices.size() * sizeof( std::uint32_t );
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndex };
		desc.debugName = "core dynamic indices";
		auto indices = device.CreateBuffer( desc );
		if ( !vertices || !indices )
		{
			for ( auto *buffer : { &vertices, &indices } )
			{
				if ( *buffer )
					(void)device.Release( buffer->Value(), CompletionToken() );
			}
			note( "dynamic geometry buffers were refused" );
			complete = false;
			continue;
		}
		for ( const auto &[buffer, bytes, usage] :
		    { std::tuple{ vertices.Value(), std::as_bytes( std::span( draw.vertices ) ),
		          ResourceUsage::kVertex },
		        std::tuple{ indices.Value(), std::as_bytes( std::span( draw.indices ) ),
		            ResourceUsage::kIndex } } )
		{
			encoder.TransitionBuffer(
			    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( buffer, 0, bytes );
			encoder.TransitionBuffer( buffer, ResourceUsage::kCopyDestination, usage );
			s.retiredBuffers.emplace_back( target.frame, buffer );
		}
		dynamicDraws.push_back( { m, vertices.Value(), indices.Value(),
		    std::uint32_t( draw.indices.size() ), draw.lightmapPage, &draw } );
	}

	preparation.End();
	std::vector<ColorAttachment> colors = {
	    { target.color, LoadOp::kLoad, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ target.depth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = target.width;
	rendering.height = target.height;
	const auto &stencil = target.drawState.stencil;
	const bool depthPrepassSafe =
	    target.depthPrepass && world->stage && !target.drawState.overrideDepth &&
	    ( !stencil.enabled || stencil.writeMask == 0 ||
	        ( stencil.fail == StencilOp::kKeep && stencil.depthFail == StencilOp::kKeep &&
	            stencil.pass == StencilOp::kKeep ) );
	auto prepassedState = [&]()
	{
		material::SurfaceDrawState state = target.drawState;
		state.overrideDepth = true;
		state.depthTest = true;
		state.depthWrite = false;
		state.depthCompare = CompareOp::kEqual;
		return state;
	};
	bool worldDepthReady = depthPrepassSafe;
	// The opaque (and alpha-tested) surfaces' depth first, into the target's
	// own depth with its color masked. Eligible PBR lighting reads that depth
	// with an equal test and no writes, allowing early rejection despite
	// the material shader's clipping/discard. A
	// translucent surface's depth would hide what the stream draws behind
	// it, so it is not drawn here.
	if ( target.depthPrepass && world->stage )
	{
		std::vector<std::uint32_t> opaque;
		opaque.reserve( order.size() );
		for ( const std::uint32_t index : order )
		{
			if ( r.materials[world->surfaces[index].material].program.blend == BlendMode::kOpaque )
				opaque.push_back( index );
		}
		if ( !opaque.empty() )
		{
			encoder.BeginLabel( "core world depth" );
			encoder.BeginRendering( rendering );
			encoder.SetViewport( view.viewport );
			drawSurfaces( opaque, r.materials,
			    [&]( const Resources::Material &m ) -> std::optional<PipelineId>
			    {
				    auto variant = m.resolver->VariantPipeline( m.program,
				        material::kSurfaceDepthNormal | material::kSurfaceDepthOnly,
				        material::kSurfaceSsrTargets );
				    if ( !variant )
				    {
					    worldDepthReady = false;
					    note( "the depth prepass: " + variant.Error() );
					    return std::nullopt;
				    }
				    const auto state = surfaceStatePipeline( m, variant.Value(), target.drawState );
				    if ( !state )
					    worldDepthReady = false;
				    return state;
			    } );
			encoder.EndRendering();
			encoder.EndLabel();
		}
	}
	// Opaque PBR models also occlude the world and each other. Record their
	// identical posed/static geometry before any expensive surface shading.
	// Stencil mutation and depth overrides are ordered effects, so those
	// views retain their existing stream ordering.
	const bool modelDepthPrepass = depthPrepassSafe;
	bool modelDepthReady = modelDepthPrepass;
	if ( modelDepthPrepass )
	{
		encoder.BeginLabel( "core model depth" );
		encoder.BeginRendering( rendering );
		encoder.SetViewport( view.viewport );
		for ( const StaticDraw &draw : staticDraws )
		{
			const Resources::Material &m = r.modelMaterials[draw.material];
			if ( !opaquePbr( m ) )
				continue;
			auto variant = m.resolver->VariantPipeline( m.program,
			    material::kSurfaceDepthNormal | material::kSurfaceDepthOnly,
			    material::kSurfaceSsrTargets );
			if ( !variant )
			{
				modelDepthReady = false;
				complete = false;
				note( "the model depth prepass: " + variant.Error() );
				continue;
			}
			const auto state = surfaceStatePipeline( m, variant.Value(), target.drawState );
			if ( !state )
			{
				modelDepthReady = false;
				complete = false;
				continue;
			}
			recordModel( draw, m, *state );
		}
		encoder.EndRendering();
		encoder.EndLabel();
	}
	encoder.BeginLabel( "core world" );
	recordingTemporal = target.motion.IsValid();
	if ( recordingTemporal )
	{
		colors.push_back( { target.motion, LoadOp::kLoad, StoreOp::kStore, {}, {} } );
		colors.push_back( { target.motionDepth, LoadOp::kLoad, StoreOp::kStore, {}, {} } );
		rendering.colors = colors;
	}
	// render.ssr.v1's inputs: three more attachments, cleared at the first
	// begin and kept across the scene-color captures' restarts.
	const TextureId ssrTargets[] = {
	    target.ssrNormalRoughness, target.ssrIblRadiance, target.ssrSpecularWeight };
	recordingSsr = world->stage && !recordingTemporal && target.samples == 1 &&
	               std::all_of( std::begin( ssrTargets ), std::end( ssrTargets ),
	                   []( TextureId t )
	                   {
		                   return t.IsValid();
	                   } );
	if ( recordingSsr )
	{
		const ClearColor clears[] = { { 0, 0, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
		for ( int i = 0; i < 3; ++i )
		{
			encoder.TransitionTexture(
			    ssrTargets[i], ResourceUsage::kSampled, ResourceUsage::kColorAttachment );
			colors.push_back( { ssrTargets[i], LoadOp::kClear, StoreOp::kStore, clears[i], {} } );
		}
		rendering.colors = colors;
	}
	encoder.BeginRendering( rendering );
	encoder.SetViewport( view.viewport );
	if ( recordingSsr )
		for ( std::size_t i = colors.size() - 3; i < colors.size(); ++i )
			colors[i].load = LoadOp::kLoad;
	RecordSection draws( encoder );
	draws.Select( "world surfaces" );
	drawSurfaces(
	    order, r.materials,
	    [&]( const Resources::Material &m ) -> std::optional<PipelineId>
	    {
		    return surfaceStatePipeline( m, m.program.request.pipeline,
		        worldDepthReady && opaquePbr( m ) ? prepassedState() : target.drawState,
		        frame::DebugSpecializationFor( view.debug, m.program.name ) );
	    },
	    true );
	draws.Select( "prepare model draw order" );
	// The model list is built in caller order. Opaque draws can be grouped by
	// material, while blended surfaces must stay after them and retain their
	// submitted order; their pipeline has depth writes disabled.
	const auto firstBlended = std::stable_partition( staticDraws.begin(), staticDraws.end(),
	    [&]( const StaticDraw &draw )
	    {
		    const material::ResolvedProgram &program = r.modelMaterials[draw.material].program;
		    return program.blend == BlendMode::kOpaque && !program.sceneColor;
	    } );
	std::sort( staticDraws.begin(), firstBlended,
	    [&]( const StaticDraw &a, const StaticDraw &b )
	    {
		    return std::tie( a.cohort, a.material, a.mesh, a.posed, a.instance, a.surface ) <
		           std::tie( b.cohort, b.material, b.mesh, b.posed, b.instance, b.surface );
	    } );
	draws.End();
	std::uint64_t drawnStatic = 0;
	std::uint64_t drawnPosed = 0;
	bool captureAttempted = false;
	bool sceneViewsPrepared = true;
	auto captureSceneColor = [&]() -> bool
	{
		if ( target.samples == 1 && !target.colorCopySource )
		{
			note( "the slot's color image does not support scene-color capture" );
			return false;
		}
		if ( !target.sceneColorCapture )
		{
			note( "the composition has no scene-color capture provider" );
			return false;
		}
		TextureDesc sourceDesc;
		sourceDesc.format = target.colorFormat;
		sourceDesc.width = target.width;
		sourceDesc.height = target.height;
		sourceDesc.sampleCount = target.samples;
		sourceDesc.usages = { ResourceUsage::kColorAttachment };
		if ( target.colorCopySource )
			sourceDesc.usages.Add( ResourceUsage::kCopySource );
		const std::optional<WorldSceneColor> captured = target.sceneColorCapture->Capture(
		    device, encoder, target.color, sourceDesc, target.frame );
		if ( !captured || !captured->texture.IsValid() )
		{
			note( "the scene-color capture resources were refused" );
			return false;
		}
		viewSceneColor = captured->texture;
		viewSceneColorDesc = captured->desc;
		return true;
	};
	for ( const StaticDraw &draw : staticDraws )
	{
		const Resources::Material &m = r.modelMaterials[draw.material];
		draws.Select( m.program.sceneColor ? "models transmitting / "
		              : draw.posed         ? "models posed / "
		                                   : "models static / ",
		    m.program.name );
		if ( m.program.sceneColor && !captureAttempted )
		{
			RecordSection capture( encoder );
			capture.Select( "model scene color capture and bindings" );
			encoder.EndRendering();
			captureAttempted = true;
			if ( !captureSceneColor() )
				complete = false;
			else
			{
				// View groups stage their constants and image bindings. Prepare
				// every transmitting layout while the host encoder is outside
				// rendering; uploads inside a render section are invalid.
				for ( const StaticDraw &candidate : staticDraws )
				{
					const Resources::Material &material = r.modelMaterials[candidate.material];
					if ( material.program.sceneColor &&
					     material.program.request.viewLayout.IsValid() &&
					     !viewGroupReady( material ) )
						sceneViewsPrepared = false;
				}
			}
			encoder.BeginRendering( rendering );
			encoder.SetViewport( view.viewport );
		}
		if ( m.program.sceneColor && ( !viewSceneColor.IsValid() || !sceneViewsPrepared ) )
		{
			complete = false;
			continue;
		}
		const auto state = surfaceStatePipeline( m, m.program.request.pipeline,
		    modelDepthReady && opaquePbr( m ) ? prepassedState() : target.drawState,
		    frame::DebugSpecializationFor( view.debug, m.program.name ) );
		if ( !state )
		{
			complete = false;
			continue;
		}
		recordModel( draw, m, *state );
		if ( draw.posed )
			++drawnPosed;
		else
			++drawnStatic;
	}
	draws.End();
	std::uint64_t drawnDynamic = 0;
	for ( const DynamicDraw &draw : dynamicDraws )
	{
		const Resources::Material &m = *draw.material;
		draws.Select( "dynamic / ", m.program.name );
		if ( m.program.sceneColor )
		{
			encoder.EndRendering();
			const bool captured = captureSceneColor();
			const Group *scene = captured ? viewGroupReady( m ) : nullptr;
			encoder.BeginRendering( rendering );
			encoder.SetViewport( view.viewport );
			if ( !scene )
			{
				complete = false;
				continue;
			}
		}
		const auto state = statePipeline( m, m.program.request.pipeline, target.drawState,
		    frame::DebugSpecializationFor( view.debug, m.program.name ) );
		if ( !state )
		{
			complete = false;
			continue;
		}
		encoder.SetPipeline( *state );
		encoder.SetBindGroup(
		    BindGroupRole::kFrame, r.frameGroups[m.program.request.frameLayout.value].group );
		const auto layout = m.program.request.viewLayout.value;
		const Group *group = m.program.depthBlend       ? &depthViews[layout]
		                     : m.program.sceneColor     ? &sceneViews[layout]
		                     : litViews.count( layout ) ? litViews[layout]
		                                                : &r.viewGroups[layout];
		encoder.SetBindGroup( BindGroupRole::kView, group->group );
		encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
		encoder.SetBindGroup( BindGroupRole::kDraw,
		    r.drawGroups[drawKey( m, draw.page, draw.source->capturedLightmap )].group );
		material::FamilyDrawConstants dynamicConstants = constants;
		std::copy_n( draw.source->modelToWorld, 16, dynamicConstants.world );
		encoder.SetDrawConstants( 0, std::as_bytes( std::span( &dynamicConstants, 1 ) )
		                                 .first( m.program.request.drawConstantBytes ) );
		encoder.SetVertexBuffer( 0, draw.vertices );
		if ( recordingTemporal )
			encoder.SetVertexBuffer( 1, draw.vertices );
		encoder.SetIndexBuffer( draw.indices, 0, IndexFormat::kUint32 );
		encoder.DrawIndexed( draw.count, 1, 0, 0, 0 );
		++drawnDynamic;
	}

	draws.End();
	encoder.EndRendering();
	if ( recordingSsr )
		for ( const TextureId t : ssrTargets )
			encoder.TransitionTexture( t, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	recordingSsr = false;
	encoder.EndLabel();
	preparation.Select( "retire world view resources" );
	for ( auto &group : transientLitViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : sceneViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : depthViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : reflectViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );

	std::lock_guard<std::mutex> guard( s.lock );
	s.stats.viewsDrawn += tags.size();
	s.stats.surfacesDrawn += order.size();
	s.stats.staticDrawsDrawn += drawnStatic;
	s.stats.posedDrawsDrawn += drawnPosed;
	s.stats.dynamicDrawsDrawn += drawnDynamic;
	s.stats.cutoutShadowDraws += cutoutDraws;
	s.stats.cutoutShadowRefused += cutoutRefused;
	s.stats.cutoutShadowNotResident += cutoutNotResident;
	if ( !complete )
	{
		++s.stats.viewsFailed;
		s.stats.lastFailure =
		    failure.empty() ? "a view named surfaces the pass does not draw" : failure;
	}
}

} // namespace render::pass::world
