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
#include <cctype>
#include <cstddef>
#include <cstring>
#include <deque>
#include <functional>
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
	material::MaterialDesc desc;
	std::map<std::string, int> handles; // the importer's texture name -> handle
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
	struct StaticMeshBuffers
	{
		BufferId vertices;
		BufferId indices;
		bool uploaded = false;
	};
	std::vector<StaticMeshBuffers> staticMeshes;
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
	std::vector<Material> prepassMaterials;
	TextureId prepassDepth;
	TextureId prepassNormal;
	TextureDesc prepassDepthDesc;
	TextureDesc prepassNormalDesc;
	bool prepassUsed = false; // the targets rest in kSampled after their first view
	// Draw groups by (draw layout, lightmap page).
	// Draw groups by (draw layout, lightmap page, the program's draw inputs):
	// programs that share a layout may read different inputs (the pbr point
	// its three pages, unlit and lightmapped points the page alone).
	std::map<std::tuple<std::uint64_t, int, std::string>, Group> drawGroups;
	// Frame groups by frame layout; their constants are written per slot.
	std::map<std::uint64_t, Group> frameGroups;
	// The programs' neutral view groups, by view layout (the world pass
	// supplies no clustered lights yet).
	std::map<std::uint64_t, Group> viewGroups;
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
std::uint32_t StageTerms( const WorldStage &stage )
{
	material::SceneTermInputs inputs;
	inputs.runtimeDirect = stage.runtimeDirect;
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
		return foundation::MakeUnexpected( std::string( "selected material needs a live proxy handoff" ) );
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
		for ( const auto &[key, handle] : source.textures )
		{
			if ( Lower( key ) == Lower( value.key ) && handle != 0 )
				claimed.handles[value.text] = handle;
		}
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

struct WorldPass::State
{
	IRenderDevice2 *device = nullptr; // the device the resources live on

	// Guarded by lock: the world the main thread set and the queued views.
	mutable std::mutex lock;
	std::shared_ptr<const WorldData> world;
	std::shared_ptr<const std::vector<Claimed>> claims;
	std::uint64_t generation = 0;
	std::uint32_t nextSerial = 1;
	struct Queued
	{
		std::uint32_t serial = 0;
		std::uint64_t generation = 0; // the world the view was queued against
		std::uint64_t recordedStream = 0;
		WorldView view;
	};
	std::deque<Queued> views;
	// Views already recorded, newest last: the backend records a frame's
	// stream again for an on-demand capture (a screenshot), with the same
	// slots, which must draw the same views.
	std::deque<Queued> recorded;
	// Host frames with a recorded slot, newest last.
	std::deque<std::uint64_t> recordedFrames;
	WorldStats stats;
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
	std::vector<Resources> variants;
	std::vector<std::pair<std::uint64_t, Resources>> retired;
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
			for ( const Resources::StaticMeshBuffers &mesh : old.staticMeshes )
			{
				if ( mesh.vertices.IsValid() )
					(void)device->Release( mesh.vertices, after );
				if ( mesh.indices.IsValid() )
					(void)device->Release( mesh.indices, after );
			}
		}
		old = Resources();
	}
};

WorldPass::WorldPass() : m_State( std::make_unique<State>() )
{
}

// Without ReleaseDevice the device may be gone: the handles are dropped.
WorldPass::~WorldPass() = default;

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
			                 : material::ClaimForDrawing( claimed.desc, data.stage != nullptr );
			if ( !blend )
				gap = claimed.desc.family + ": " + blend.Error();
			else if ( !source.mesh && blend.Value() != BlendMode::kOpaque )
				gap = claimed.desc.family + ": blended (drawn in the translucent stage)";
			else
			{
				claimed.blended |= blend.Value() != BlendMode::kOpaque;
				claimed.draws = true;
			}
		}
		if ( !gap.empty() )
		{
			++gaps[gap];
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
	std::lock_guard<std::mutex> guard( s.lock );
	s.world = std::make_shared<const WorldData>( std::move( data ) );
	s.claims = std::move( claims );
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
	++s.generation;
	// Queued views stay with their world's generation: in queued mode the
	// slots of a frame that straddles a level change record after it, and
	// skip their views (an earlier world's) rather than fail.
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
	if ( data.vertices.empty() || data.indices.empty() || data.surfaces.empty() )
		return false;
	for ( std::uint32_t i = 0; i < data.surfaces.size(); ++i )
	{
		if ( !SurfaceSelected( placement.surfaceSelection, i ) )
			continue;
		const WorldSurface &surface = data.surfaces[i];
		const std::uint32_t material = StaticMaterial( data, placement, i );
		if ( material >= s.claims->size() || !( *s.claims )[material].draws ||
		     !s.world->materials[material].mesh || surface.firstIndex > data.indices.size() ||
		     surface.indexCount > data.indices.size() - surface.firstIndex )
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
	if ( mesh.vertices.empty() || mesh.indices.empty() || mesh.surfaces.empty() )
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
		if ( material >= s.claims->size() || !( *s.claims )[material].draws ||
		     !s.world->materials[material].mesh || surface.firstIndex > mesh.indices.size() ||
		     surface.indexCount > mesh.indices.size() - surface.firstIndex )
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
		auto mapped = MapWorldMaterial( draw.material );
		std::string why;
		if ( !mapped )
			why = mapped.Error();
		else
		{
			auto claim =
			    draw.material.mesh
			        ? material::ClaimForMesh( mapped.Value().desc,
			              s.world->stage && !s.world->stage->reflectionProbes.empty(),
			              s.world->stage != nullptr )
			        : material::ClaimForDrawing( mapped.Value().desc, s.world->stage != nullptr );
			if ( !claim )
				why = claim.Error();
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

std::uint64_t WorldPass::Failures() const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.stats.viewsFailed;
}

void WorldPass::Record( std::uint32_t tag, CommandEncoder &encoder, const WorldTarget &target )
{
	RecordSection preparation( encoder );
	preparation.Select( "prepare world queue" );
	State &s = *m_State;
	const std::uint32_t serial = tag & kWorldSerialMask;
	std::shared_ptr<const WorldData> world;
	std::shared_ptr<const std::vector<Claimed>> claims;
	std::uint64_t generation = 0;
	WorldView view;
	bool found = false;
	bool earlierWorld = false; // the slot's view was queued against an earlier world
	{
		std::lock_guard<std::mutex> guard( s.lock );
		// A slot recorded again (the same stream for a capture) draws the
		// view it drew the first time; one of an earlier world draws nothing
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
				view = kept->view;
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
			view = queued->view;
			if ( view.hostFrame != 0 &&
			     ( s.recordedFrames.empty() || s.recordedFrames.back() != view.hostFrame ) )
			{
				s.recordedFrames.push_back( view.hostFrame );
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
		world = s.world;
		claims = s.claims;
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
	if ( !target.device || !target.color.IsValid() || !target.depth.IsValid() || !target.textures )
	{
		std::string missing;
		for ( const auto &[absent, what] :
		    { std::pair{ !target.device, "device" }, { !target.color.IsValid(), "color" },
		        { !target.depth.IsValid(), "depth" }, { !target.textures, "texture source" } } )
		{
			if ( absent )
				missing += missing.empty() ? what : std::string( ", " ) + what;
		}
		s.Fail( "the slot's target has no " + missing + " (a render-target texture)" );
		return;
	}
	preparation.Select( "prepare world resources" );
	if ( s.device != target.device )
	{
		// A new backend device: the old one's objects went with it.
		s.variants.clear();
		s.retired.clear();
		s.retiredBuffers.clear();
		s.retiredGroups.clear();
		s.retiredTextures.clear();
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
	if ( s.variantsGeneration != generation )
	{
		for ( Resources &variant : s.variants )
			s.retired.emplace_back( target.frame, std::move( variant ) );
		s.variants.clear();
		s.variantsGeneration = generation;
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
		    target.depthFormat, target.samples, material::VertexLayout::kSurface );
		if ( !resolver )
		{
			s.Fail( resolver.Error() );
			return;
		}
		r.resolver = std::move( resolver ).Value();
		// A world stage draws with world pbr (the pbr point on the world
		// vertex) and the scene terms its data supports.
		if ( world->stage )
			r.resolver->SetWorldPbr( true, StageTerms( *world->stage ) );
		r.resolver->SetSceneColorAvailable( world->stage != nullptr );
		r.materials.resize( world->materials.size() );
	}
	if ( ( !view.staticInstances.empty() || !view.posedModels.empty() ) && !r.modelResolver )
	{
		auto resolver = material::ProgramResolver::Create( device, target.colorFormat,
		    target.depthFormat, target.samples, material::VertexLayout::kModel );
		if ( !resolver )
		{
			s.Fail( "the static model resolver: " + resolver.Error() );
			return;
		}
		r.modelResolver = std::move( resolver ).Value();
		r.modelResolver->SetWorldPbr( true, world->stage ? StageTerms( *world->stage ) : 0 );
		r.modelResolver->SetSceneColorAvailable( world->stage != nullptr );
		r.modelMaterials.resize( world->materials.size() );
		r.staticMeshes.resize( world->staticMeshes.size() );
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
	// has a staging buffer of its own, released behind a later frame.
	auto stageUpload = [&]( TextureId texture, const TextureDesc &desc,
	                       std::span<const std::byte> bytes, ResourceUsage from ) -> bool
	{
		BufferDesc staging;
		staging.size = bytes.size();
		staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
		staging.debugName = "world stage staging";
		auto buffer = device.CreateBuffer( staging );
		if ( !buffer )
			return false;
		s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, bytes );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
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
		BufferDesc staging;
		staging.size = texels.size();
		staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
		staging.debugName = "world stage region staging";
		auto buffer = device.CreateBuffer( staging );
		if ( !buffer )
			return false;
		s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, texels );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
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
			if ( !fits || !stageUpload( r.stageTextures[kStageProbeGrids],
			                  r.stageDescs[kStageProbeGrids],
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
							std::memcpy( s.stageChangeBase.data() + at,
							    patch.texels.data() + offset, row );
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
			BufferDesc staging;
			staging.size = 8;
			staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
			auto buffer = device.CreateBuffer( staging );
			if ( !buffer )
				return {};
			r.neutralStaging = buffer.Value();
			const std::uint32_t whiteAndDepth[] = { 0xFFFFFFFFu, 0x3F800000u };
			encoder.TransitionBuffer(
			    r.neutralStaging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( r.neutralStaging, 0, std::as_bytes( std::span( whiteAndDepth ) ) );
			encoder.TransitionBuffer(
			    r.neutralStaging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
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
		const bool model = &resolver == r.modelResolver.get();
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
	auto drawKey = []( const Resources::Material &m, int page )
	{
		std::string inputs;
		for ( const std::string &input : m.program.drawInputs )
			inputs += input + ";";
		return std::make_tuple( m.program.request.drawLayout.value, page, std::move( inputs ) );
	};
	auto drawGroupReady = [&]( const Resources::Material &m, int page ) -> const Group *
	{
		const auto key = drawKey( m, page );
		if ( auto found = r.drawGroups.find( key ); found != r.drawGroups.end() )
			return found->second.group.IsValid() ? &found->second : nullptr;
		Group &group = r.drawGroups[key];
		std::vector<std::string> inputs;
		std::map<std::string, int> handles;
		for ( const std::string &input : m.program.drawInputs )
		{
			if ( world->stage )
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
			note( "a draw group: " + ( why.empty() ? std::string( "not resolved" ) : why ) );
			return nullptr;
		}
		return &group;
	};

	// The frame group of a program's layout, its terms written for this slot.
	material::FrameTerms terms;
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
	std::map<std::uint64_t, bool> framesWritten;
	auto frameGroupReady = [&]( const Resources::Material &m ) -> const Group *
	{
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
	std::map<std::uint64_t, Group> litViews;
	std::map<std::uint64_t, Group> modelLitViews;
	std::map<std::uint64_t, Group> sceneViews;
	// The view's ambient occlusion once the screen passes recorded it.
	TextureId viewOcclusion;
	// The view's planar reflection (a program's view input, imported below
	// once the view's materials resolved), and the view groups that bind it
	// for programs without the view's lights, retired behind this frame.
	TextureId viewReflection;
	TextureId viewSceneColor;
	TextureDesc viewSceneColorDesc;
	std::map<std::uint64_t, Group> reflectViews;
	auto viewGroupReady = [&]( const Resources::Material &m ) -> const Group *
	{
		const std::uint64_t layout = m.program.request.viewLayout.value;
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
		if ( world->stage && view.lights )
		{
			const bool model = m.resolver == r.modelResolver.get();
			Group &lit = m.program.sceneColor ? sceneViews[layout]
			             : model              ? modelLitViews[layout]
			                                  : litViews[layout];
			if ( lit.group.IsValid() )
				return &lit;
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
			material::GroupRequest request = m.resolver->Program().ViewGroup(
			    lights.view, lights.froxels, lights.indices, lights.lights, shadows, {}, screen );
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
			     !buildGroup( request, {}, lit, &why ) )
			{
				s.ReleaseGroup( lit, CompletionToken() );
				note( "the view's lights: " +
				      ( why.empty() ? std::string( "another view layout" ) : why ) );
				return nullptr;
			}
			return &lit;
		}
		if ( m.viewInput != 0 || m.program.sceneColor )
		{
			Group &reflect = m.program.sceneColor ? sceneViews[layout] : reflectViews[layout];
			if ( reflect.group.IsValid() )
				return &reflect;
			material::SurfaceScreenInputs screen;
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
	struct StaticDraw
	{
		bool posed;
		std::uint32_t instance;
		std::uint32_t mesh;
		std::uint32_t surface;
		std::uint32_t material;
	};
	std::vector<StaticDraw> staticDraws;
	std::vector<BufferId> posedBuffers( view.posedModels.size() );
	auto uploadMesh = [&]( std::uint32_t meshId ) -> bool
	{
		Resources::StaticMeshBuffers &buffers = r.staticMeshes[meshId];
		if ( buffers.uploaded )
			return true;
		const WorldData::StaticMesh &mesh = world->staticMeshes[meshId];
		const auto vertexBytes = std::as_bytes( std::span( mesh.vertices ) );
		const auto indexBytes = std::as_bytes( std::span( mesh.indices ) );
		BufferDesc desc;
		desc.size = vertexBytes.size();
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
		desc.debugName = "model vertices";
		auto vertices = device.CreateBuffer( desc );
		desc.size = indexBytes.size();
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndex };
		desc.debugName = "model indices";
		auto indices = device.CreateBuffer( desc );
		if ( !vertices || !indices || vertexBytes.empty() || indexBytes.empty() )
		{
			if ( vertices )
				(void)device.Release( vertices.Value(), CompletionToken() );
			if ( indices )
				(void)device.Release( indices.Value(), CompletionToken() );
			note( "a model mesh's buffers were refused" );
			return false;
		}
		buffers.vertices = vertices.Value();
		buffers.indices = indices.Value();
		encoder.TransitionBuffer(
		    buffers.vertices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffers.vertices, 0, vertexBytes );
		encoder.TransitionBuffer(
		    buffers.vertices, ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
		encoder.TransitionBuffer(
		    buffers.indices, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffers.indices, 0, indexBytes );
		encoder.TransitionBuffer(
		    buffers.indices, ResourceUsage::kCopyDestination, ResourceUsage::kIndex );
		buffers.uploaded = true;
		return true;
	};
	for ( const WorldView::StaticInstance &draw : view.staticInstances )
	{
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
		if ( !uploadMesh( instance.mesh ) )
		{
			complete = false;
			continue;
		}
		for ( std::uint32_t surfaceId = 0; surfaceId < mesh.surfaces.size(); ++surfaceId )
		{
			if ( !SurfaceSelected( draw.surfaceSelection, surfaceId ) )
				continue;
			const std::uint32_t materialId = StaticMaterial( mesh, instance, surfaceId );
			const Resources::Material *material =
			    materialId < claims->size() && ( *claims )[materialId].draws
			        ? materialReadyIn( *r.modelResolver, r.modelMaterials, materialId )
			        : nullptr;
			if ( !material ||
			     ( material->program.request.drawLayout.IsValid() &&
			         !drawGroupReady( *material, 0 ) ) ||
			     ( material->program.request.frameLayout.IsValid() &&
			         !frameGroupReady( *material ) ) )
			{
				complete = false;
				continue;
			}
			staticDraws.push_back( { false, instanceId, instance.mesh, surfaceId, materialId } );
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
		if ( pose.vertices.size() != mesh.vertices.size() || !uploadMesh( pose.mesh ) )
		{
			note( "a posed model has the wrong vertex count or no mesh buffers" );
			complete = false;
			continue;
		}
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
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, bytes );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kVertex );
		s.retiredBuffers.emplace_back( target.frame, buffer.Value() );
		WorldData::StaticInstance instance;
		instance.mesh = pose.mesh;
		instance.skin = pose.skin;
		for ( std::uint32_t surfaceId = 0; surfaceId < mesh.surfaces.size(); ++surfaceId )
		{
			if ( !SurfaceSelected( pose.surfaceSelection, surfaceId ) )
				continue;
			const std::uint32_t materialId = StaticMaterial( mesh, instance, surfaceId );
			if ( materialId >= claims->size() )
			{
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
			if ( !material ||
			     ( material->program.request.drawLayout.IsValid() &&
			         !drawGroupReady( *material, 0 ) ) ||
			     ( material->program.request.frameLayout.IsValid() &&
			         !frameGroupReady( *material ) ) )
			{
				complete = false;
				continue;
			}
			staticDraws.push_back( { true, poseId, pose.mesh, surfaceId, materialId } );
		}
	}

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
	auto statePipeline =
	    [&]( const Resources::Material &m, PipelineId base, const material::SurfaceDrawState &state,
	        const shaderlib::DebugSpecialization &debug = {} ) -> std::optional<PipelineId>
	{
		auto result = m.resolver->Program().StatePipeline( base, state, debug );
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
	// World triangles use the surface vertex's counter-clockwise front face.
	// Keep authored two-sided materials, but do not draw an exit wall from the
	// virtual portal camera behind it. Clipping intentionally leaves a tolerance.
	auto worldStatePipeline = [&]( const Resources::Material &m, PipelineId base,
	                              material::SurfaceDrawState state,
	                              const shaderlib::DebugSpecialization &debug = {} )
	{
		state.cull = m.program.twoSided ? CullMode::kNone : CullMode::kBack;
		return statePipeline( m, base, state, debug );
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
					    BindGroupRole::kView, lit != litViews.end() ? lit->second.group
					                          : m.viewInput != 0 && reflect != reflectViews.end()
					                              ? reflect->second.group
					                              : r.viewGroups[layout].group );
				}
				encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
				encoder.SetVertexBuffer( 0, r.vertices, 0 );
				encoder.SetIndexBuffer( r.indices, 0, IndexFormat::kUint32 );
				encoder.SetDrawConstants(
				    0, ( m.program.name == "water" && view.waterZOffset != 0.0f ? waterConstantBytes
				                                                                : constantBytes )
				           .first( m.program.request.drawConstantBytes ) );
			}
			if ( skipping )
				continue;
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

	preparation.End();
	// A world stage's screen passes (render_lab's order): the depth and
	// normal prepass into the pass's own single-sample targets, with a
	// resolver of their own, then the composition's ambient occlusion, which
	// the lit view groups read.
	if ( world->stage && target.screenPasses && target.ambientOcclusion.IsValid() &&
	     !order.empty() )
	{
		preparation.Select( "prepare world prepass" );
		if ( !r.prepassResolver )
		{
			auto resolver = material::ProgramResolver::Create( device, Format::kRGBA16Float,
			    Format::kD32Float, 1, material::VertexLayout::kSurface );
			if ( resolver )
			{
				r.prepassResolver = std::move( resolver ).Value();
				r.prepassResolver->SetWorldPbr( true, StageTerms( *world->stage ) );
				r.prepassMaterials.resize( world->materials.size() );
			}
			else
			{
				note( "the prepass resolver: " + resolver.Error() );
			}
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
			std::vector<std::uint32_t> prepass;
			for ( const std::uint32_t index : order )
			{
				const WorldSurface &surface = world->surfaces[index];
				const Resources::Material *m =
				    materialReadyIn( *r.prepassResolver, r.prepassMaterials, surface.material );
				if ( !m ||
				     ( m->program.request.drawLayout.IsValid() &&
				         !drawGroupReady( *m, surface.lightmapPage ) ) ||
				     ( m->program.request.frameLayout.IsValid() && !frameGroupReady( *m ) ) ||
				     ( m->program.request.viewLayout.IsValid() && !viewGroupReady( *m ) ) )
				{
					complete = false;
					continue;
				}
				prepass.push_back( index );
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
			drawSurfaces( prepass, r.prepassMaterials,
			    [&]( const Resources::Material &m ) -> std::optional<PipelineId>
			    {
				    auto variant = m.resolver->VariantPipeline(
				        m.program, material::kSurfaceDepthNormal, material::kSurfaceSsrTargets );
				    if ( !variant )
				    {
					    note( "the prepass: " + variant.Error() );
					    return std::nullopt;
				    }
				    return worldStatePipeline( m, variant.Value(), material::SurfaceDrawState() );
			    } );
			encoder.EndRendering();
			encoder.EndLabel();
			encoder.TransitionTexture(
			    r.prepassNormal, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
			encoder.TransitionTexture(
			    r.prepassDepth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
			if ( target.screenPasses(
			         encoder, { r.prepassDepth, r.prepassNormal, target.width, target.height } ) )
				viewOcclusion = target.ambientOcclusion;
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
	};
	std::vector<DynamicDraw> dynamicDraws;
	for ( const WorldView::DynamicDraw &draw : view.dynamicDraws )
	{
		auto mapped = MapWorldMaterial( draw.material );
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
		Resources::Material &cached = r.dynamicMaterials[MaterialSnapshotKey( draw.material )];
		Resources::Material *m =
		    prepareMaterial( *r.resolver, cached, mapped.Value(), draw.material );
		if ( !m || !drawGroupReady( *m, draw.lightmapPage ) || !frameGroupReady( *m ) ||
		     ( !m->program.sceneColor && !viewGroupReady( *m ) ) )
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
		    std::uint32_t( draw.indices.size() ), draw.lightmapPage } );
	}

	preparation.End();
	const ColorAttachment colors[] = { { target.color, LoadOp::kLoad, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ target.depth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = target.width;
	rendering.height = target.height;
	// The opaque (and alpha-tested) surfaces' depth first, into the target's
	// own depth with its color masked: the lit pass's less-equal test then
	// rejects hidden fragments before shading them (Doom 2016's prepass). A
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
					    note( "the depth prepass: " + variant.Error() );
					    return std::nullopt;
				    }
				    return worldStatePipeline( m, variant.Value(), target.drawState );
			    } );
			encoder.EndRendering();
			encoder.EndLabel();
		}
	}
	encoder.BeginLabel( "core world" );
	encoder.BeginRendering( rendering );
	encoder.SetViewport( view.viewport );
	RecordSection draws( encoder );
	draws.Select( "world surfaces" );
	drawSurfaces(
	    order, r.materials,
	    [&]( const Resources::Material &m ) -> std::optional<PipelineId>
	    {
		    return worldStatePipeline( m, m.program.request.pipeline, target.drawState,
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
		    return std::tie( a.material, a.mesh, a.posed, a.instance, a.surface ) <
		           std::tie( b.material, b.mesh, b.posed, b.instance, b.surface );
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
		const WorldData::StaticInstance *instance =
		    draw.posed ? nullptr : &world->staticInstances[draw.instance];
		const WorldData::StaticMesh &mesh = world->staticMeshes[draw.mesh];
		const WorldSurface &surface = mesh.surfaces[draw.surface];
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
		const auto state = statePipeline( m, m.program.request.pipeline, target.drawState,
		    frame::DebugSpecializationFor( view.debug, m.program.name ) );
		if ( !state )
		{
			complete = false;
			continue;
		}
		encoder.SetPipeline( *state );
		if ( m.program.request.frameLayout.IsValid() )
			encoder.SetBindGroup(
			    BindGroupRole::kFrame, r.frameGroups[m.program.request.frameLayout.value].group );
		if ( m.program.request.viewLayout.IsValid() )
		{
			const std::uint64_t layout = m.program.request.viewLayout.value;
			const auto lit =
			    m.program.sceneColor ? sceneViews.find( layout ) : modelLitViews.find( layout );
			const auto end = m.program.sceneColor ? sceneViews.end() : modelLitViews.end();
			encoder.SetBindGroup(
			    BindGroupRole::kView, lit != end ? lit->second.group : r.viewGroups[layout].group );
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
		const Resources::StaticMeshBuffers &buffers = r.staticMeshes[draw.mesh];
		encoder.SetVertexBuffer( 0, draw.posed ? posedBuffers[draw.instance] : buffers.vertices );
		encoder.SetIndexBuffer( buffers.indices, 0, IndexFormat::kUint32 );
		encoder.DrawIndexed( surface.indexCount, 1, surface.firstIndex, 0, 0 );
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
		const auto state = statePipeline( m, m.program.request.pipeline, target.drawState );
		if ( !state )
		{
			complete = false;
			continue;
		}
		encoder.SetPipeline( *state );
		encoder.SetBindGroup(
		    BindGroupRole::kFrame, r.frameGroups[m.program.request.frameLayout.value].group );
		const auto layout = m.program.request.viewLayout.value;
		const Group *group = m.program.sceneColor       ? &sceneViews[layout]
		                     : litViews.count( layout ) ? &litViews[layout]
		                                                : &r.viewGroups[layout];
		encoder.SetBindGroup( BindGroupRole::kView, group->group );
		encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
		encoder.SetBindGroup( BindGroupRole::kDraw, r.drawGroups[drawKey( m, draw.page )].group );
		encoder.SetDrawConstants( 0, constantBytes.first( m.program.request.drawConstantBytes ) );
		encoder.SetVertexBuffer( 0, draw.vertices );
		encoder.SetIndexBuffer( draw.indices, 0, IndexFormat::kUint32 );
		encoder.DrawIndexed( draw.count, 1, 0, 0, 0 );
		++drawnDynamic;
	}

	draws.End();
	encoder.EndRendering();
	encoder.EndLabel();
	preparation.Select( "retire world view resources" );
	for ( auto &[layout, group] : litViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : modelLitViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : sceneViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );
	for ( auto &[layout, group] : reflectViews )
		s.retiredGroups.emplace_back( target.frame, std::move( group ) );

	std::lock_guard<std::mutex> guard( s.lock );
	++s.stats.viewsDrawn;
	s.stats.surfacesDrawn += order.size();
	s.stats.staticDrawsDrawn += drawnStatic;
	s.stats.posedDrawsDrawn += drawnPosed;
	s.stats.dynamicDrawsDrawn += drawnDynamic;
	if ( !complete )
	{
		++s.stats.viewsFailed;
		s.stats.lastFailure =
		    failure.empty() ? "a view named surfaces the pass does not draw" : failure;
	}
}

} // namespace render::pass::world
