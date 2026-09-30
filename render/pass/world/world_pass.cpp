//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5); see world_pass.h. Names no
//			material family: programs come from the one resolver.
//
//=============================================================================//

#include "render/pass/world/world_pass.h"

#include "render/frame/debug_specialization.h"
#include "render/material/draw_program.h"
#include "render/material/material_programs.h"
#include "render/material/program_resolver.h"
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

// What the main thread decided about a material.
struct Claimed
{
	bool draws = false;
	material::MaterialDesc desc;
	std::map<std::string, int> handles; // the importer's texture name -> handle
};

// A resolved group: its device objects.
struct Group
{
	BindGroupId group;
	BufferId constants;
	std::vector<BufferId> storage;
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
	BufferId neutralStaging;
	bool uploaded = false;
	// A world stage's textures (WorldStage), by the names its groups use,
	// made at the first slot and updated in place; the stage revisions
	// they hold.
	std::map<std::string, TextureId> stageTextures;
	std::map<std::string, TextureDesc> stageDescs;
	bool stageMade = false;
	std::uint64_t stageLightmapRevision = 0;
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

// The pbr point's scene terms a stage's data supports (as render_lab sets
// them for the same map, less the screen-space reflections, which the stage
// does not draw yet). The view's occlusion is one where no screen pass
// recorded it.
std::uint32_t StageTerms( const WorldStage &stage )
{
	// The view's runtime lights (a view without them binds the neutral view).
	std::uint32_t terms = material::kSurfaceClustered | material::kSurfaceAmbientOcclusion;
	// Runtime direct light reads the indirect layer, directional when the
	// bake wrote its own gradient page; else the total layer's pages.
	const bool runtimeDirect = stage.runtimeDirect && !stage.indirect.empty();
	if ( runtimeDirect )
		terms |= material::kSurfaceRuntimeDirect;
	if ( runtimeDirect ? !stage.indirectGradient.empty() : stage.lightmap.Directional() )
		terms |= material::kSurfaceDirectionalLightmap;
	if ( stage.probes )
		terms |= material::kSurfaceProbeVolume | material::kSurfaceProbeBounce;
	if ( !stage.reflectionProbes.empty() )
		terms |= material::kSurfaceReflectionProbes;
	return terms;
}

std::string Lower( std::string text )
{
	for ( char &c : text )
		c = char( std::tolower( static_cast<unsigned char>( c ) ) );
	return text;
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
	// SetStageChange), with revisions that rise with each.
	// The total page: `stageLightmap` as of `stageLightmapBaseRevision`
	// (null: the stage's own pages), then the parts set since
	// (SetStageLightmapRegions), in revision order; a part every resource
	// set has applied folds into it.
	std::shared_ptr<LightmapPages> stageLightmap;
	std::uint64_t stageLightmapRevision = 0;
	std::uint64_t stageLightmapBaseRevision = 0;
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
	std::vector<std::pair<std::uint64_t, TextureId>> retiredTextures;

	void Fail( const std::string &why )
	{
		std::lock_guard<std::mutex> guard( lock );
		++stats.viewsFailed;
		stats.lastFailure = why;
	}

	void ReleaseGroup( Group &group, CompletionToken after )
	{
		if ( device )
		{
			if ( group.group.IsValid() )
				(void)device->Release( group.group, after );
			if ( group.constants.IsValid() )
				(void)device->Release( group.constants, after );
			for ( BufferId buffer : group.storage )
				(void)device->Release( buffer, after );
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
		std::vector<material::VmtPair> variables;
		for ( const auto &[key, value] : source.variables )
			variables.push_back( { key, value } );
		auto mapped = material::MapVariables( source.shader, std::move( variables ), {} );
		std::string gap;
		if ( !mapped )
		{
			gap = "shader " + source.shader + " does not map";
		}
		else
		{
			claimed.desc = std::move( mapped ).Value();
			for ( const auto &[key, value] : source.defaults )
				claimed.desc.declaredDefaults.push_back( { key, value } );
			// Texture handles by the importer's names, matched by variable key.
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
			// A world stage's materials resolve with world pbr.
			auto blend = material::ClaimForDrawing( claimed.desc, data.stage != nullptr );
			if ( !blend )
				gap = claimed.desc.family + ": " + blend.Error();
			else if ( blend.Value() != BlendMode::kOpaque )
				gap = claimed.desc.family + ": blended (drawn in the translucent stage)";
			else
				claimed.draws = true;
		}
		if ( !gap.empty() )
			++gaps[gap];
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

void WorldPass::SetStageChangeRegions(
    std::vector<StageRegion> regions, std::vector<std::byte> texels, StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageTable = std::move( sharedTable );
	s.stagePatches.push_back( { ++s.stageChangeRevision, std::move( regions ), std::move( texels ) } );
}

bool WorldPass::Draws( std::uint32_t material ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.claims && material < s.claims->size() && ( *s.claims )[material].draws;
}

std::uint32_t WorldPass::QueueView( WorldView view )
{
	State &s = *m_State;
	if ( view.surfaces.empty() )
		return 0;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( !s.world )
		return 0;
	const std::uint32_t serial = s.nextSerial;
	s.nextSerial = ( s.nextSerial + 1 ) & kWorldSerialMask;
	if ( s.nextSerial == 0 )
		s.nextSerial = 1;
	// A view whose slot never records (a skipped frame) is dropped when a
	// later one records; the queue stays bounded meanwhile.
	constexpr std::size_t kMaxQueued = 256;
	if ( s.views.size() >= kMaxQueued )
	{
		s.Drop( s.views.front(), 0 );
		s.views.pop_front();
	}
	s.views.push_back( { serial, s.generation, std::move( view ) } );
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
	(void)device.Poll();
	s.device = nullptr;
}

WorldStats WorldPass::Stats() const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.stats;
}

std::uint64_t WorldPass::Failures() const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.stats.viewsFailed;
}

void WorldPass::Record( std::uint32_t tag, CommandEncoder &encoder, const WorldTarget &target )
{
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
		// Views are queued in stream order: earlier ones whose slots did not
		// record are dropped; a slot older than every queued view takes none.
		std::uint64_t recordingFrame = 0;
		for ( const State::Queued &queued : s.views )
		{
			if ( queued.serial == serial )
				recordingFrame = queued.view.hostFrame;
		}
		while ( !again && !s.views.empty() && IssuedBefore( s.views.front().serial, serial ) )
		{
			s.Drop( s.views.front(), recordingFrame );
			s.views.pop_front();
		}
		if ( !again && !s.views.empty() && s.views.front().serial == serial )
		{
			// Only against the world it was queued for (SetWorld also clears
			// the queue; this holds if a slot records across the change).
			found = s.views.front().generation == s.generation;
			earlierWorld = !found;
			view = s.views.front().view;
			if ( view.hostFrame != 0 &&
			     ( s.recordedFrames.empty() || s.recordedFrames.back() != view.hostFrame ) )
			{
				s.recordedFrames.push_back( view.hostFrame );
				constexpr std::size_t kFramesKept = 64;
				while ( s.recordedFrames.size() > kFramesKept )
					s.recordedFrames.pop_front();
			}
			s.recorded.push_back( std::move( s.views.front() ) );
			s.views.pop_front();
			constexpr std::size_t kRecordedKept = 64; // a frame's views, with room
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
	// A stage view's lights made when its slot records (WorldTarget::lights).
	if ( target.lights )
		view.lights = target.lights;
	if ( !found || !world || !claims )
	{
		s.Fail( "a slot names no queued view of the current world" );
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
	if ( s.device != target.device )
	{
		// A new backend device: the old one's objects went with it.
		s.variants.clear();
		s.retired.clear();
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
		    if ( target.frame == 0 || old.first >= target.frame )
			    return false;
		    s.ReleaseGroup( old.second, target.submitted );
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
		r.materials.resize( world->materials.size() );
	}
	if ( !r.uploaded )
	{
		const auto vertexBytes = std::as_bytes( std::span( world->vertices ) );
		const auto indexBytes = std::as_bytes( std::span( world->indices ) );
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

	// The neutral textures, made on first use: a white 2D texture and a
	// black cube, filled from one 4-byte white upload (the cube's term is off
	// wherever it is bound; filling it keeps it defined).
	auto neutral = [&]( TextureDimension dimension, bool array ) -> TextureId
	{
		TextureId &slot = dimension == TextureDimension::kCube ? r.neutralCube
		                  : array                              ? r.neutralArray
		                                                       : r.neutralWhite;
		if ( slot.IsValid() )
			return slot;
		if ( !r.neutralStaging.IsValid() )
		{
			BufferDesc staging;
			staging.size = 4;
			staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
			auto buffer = device.CreateBuffer( staging );
			if ( !buffer )
				return {};
			r.neutralStaging = buffer.Value();
			const std::byte white[4] = {
			    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
			encoder.TransitionBuffer(
			    r.neutralStaging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( r.neutralStaging, 0, white );
			encoder.TransitionBuffer(
			    r.neutralStaging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		}
		TextureDesc desc;
		desc.dimension = dimension;
		desc.format = Format::kRGBA8Unorm;
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
			encoder.CopyBufferToTexture( r.neutralStaging, texture.Value(), { 0, 0, layer, 1, 1 } );
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
			BufferDesc desc;
			desc.size = request.constants.size();
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kUniform };
			desc.debugName = "world group constants";
			auto constants = device.CreateBuffer( desc );
			if ( !constants )
			{
				*why = "a constants buffer was refused";
				return false;
			}
			out.constants = constants.Value();
			entries.push_back( { request.constantsBinding, out.constants, 0, 0, {}, {} } );
		}
		for ( const material::GroupBuffer &storage : request.storage )
		{
			BufferDesc desc;
			desc.size = std::max<std::uint64_t>( storage.bytes.size(), 4 );
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead };
			desc.debugName = "world group storage";
			auto buffer = device.CreateBuffer( desc );
			if ( !buffer )
			{
				*why = "a storage buffer was refused";
				return false;
			}
			out.storage.push_back( buffer.Value() );
			entries.push_back( { storage.binding, buffer.Value(), 0, 0, {}, {} } );
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
			const TextureId id = external ? texture.external
			                     : staged ? stageTexture->second
			                     : absent ? neutral( texture.dimension, texture.array )
			                     : handle == handles.end()
			                         ? TextureId()
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
			auto sampler = device.CreateSampler( external || staged ? texture.sampler
			                                     : absent ? SamplerDesc()
			                                              : textures.Sampler( handle->second ) );
			if ( !sampler )
			{
				*why = "a sampler was refused";
				return false;
			}
			out.samplers.push_back( sampler.Value() );
			entries.push_back( { texture.samplerBinding, {}, 0, 0, {}, sampler.Value() } );
		}
		auto group = device.CreateBindGroup( { request.layout, entries } );
		if ( !group )
		{
			*why = "a bind group was refused";
			return false;
		}
		out.group = group.Value();
		if ( out.constants.IsValid() )
		{
			encoder.TransitionBuffer(
			    out.constants, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( out.constants, 0, request.constants );
			encoder.TransitionBuffer(
			    out.constants, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
		}
		for ( std::size_t i = 0; i < out.storage.size(); ++i )
		{
			encoder.TransitionBuffer(
			    out.storage[i], ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			if ( !request.storage[i].bytes.empty() )
				encoder.WriteBuffer( out.storage[i], 0, request.storage[i].bytes );
			encoder.TransitionBuffer(
			    out.storage[i], ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead );
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
	auto materialReadyIn = [&]( material::ProgramResolver &resolver,
	                           std::vector<Resources::Material> &materials,
	                           std::uint32_t index ) -> Resources::Material *
	{
		Resources::Material &m = materials[index];
		if ( m.failed )
			note( m.failure );
		if ( m.ready || m.failed )
			return m.ready ? &m : nullptr;
		const Claimed &claimed = ( *claims )[index];
		const std::string &name = world->materials[index].name;
		auto program = resolver.Resolve( claimed.desc );
		std::string why;
		if ( !program )
			why = program.Error();
		else if ( program.Value().request.vertexStride != sizeof( WorldVertex ) )
			why = "its program reads another vertex than the world's";
		else if ( !buildGroup( program.Value().request.material, claimed.handles, m.group, &why ) )
			s.ReleaseGroup( m.group, CompletionToken() );
		if ( !why.empty() )
		{
			m.failed = true;
			m.failure = "material " + name + ": " + why;
			note( m.failure );
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
			    group.constants, ResourceUsage::kUniform, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( group.constants, 0, request->constants );
			encoder.TransitionBuffer(
			    group.constants, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
			framesWritten[layout] = true;
		}
		return &group;
	};

	// A program's view group: its neutral one (built once per layout).
	// A world stage's lit view: one group per view layout for this view, with
	// the view's clustered lights, retired behind this frame after drawing.
	std::map<std::uint64_t, Group> litViews;
	// The view's ambient occlusion once the screen passes recorded it.
	TextureId viewOcclusion;
	// The view's planar reflection (a program's view input, imported below
	// once the view's materials resolved), and the view groups that bind it
	// for programs without the view's lights, retired behind this frame.
	TextureId viewReflection;
	std::map<std::uint64_t, Group> reflectViews;
	auto viewGroupReady = [&]( const Resources::Material &m ) -> const Group *
	{
		const std::uint64_t layout = m.program.request.viewLayout.value;
		if ( m.viewInput != 0 && !viewReflection.IsValid() )
		{
			note( "a program reads the view's planar reflection, which did not import" );
			return nullptr;
		}
		if ( world->stage && view.lights )
		{
			Group &lit = litViews[layout];
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
			if ( viewOcclusion.IsValid() )
			{
				screen.ambientOcclusion = viewOcclusion;
				screen.ambientOcclusionDesc = target.ambientOcclusionDesc;
			}
			screen.planarReflection = viewReflection;
			const material::GroupRequest request = m.resolver->Program().ViewGroup(
			    lights.view, lights.froxels, lights.indices, lights.lights, shadows, {}, screen );
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
		if ( m.viewInput != 0 )
		{
			Group &reflect = reflectViews[layout];
			if ( reflect.group.IsValid() )
				return &reflect;
			material::SurfaceScreenInputs screen;
			screen.planarReflection = viewReflection;
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
	const bool debugNeutral = frame::DebugControlsNeutral( view.debug );

	// Draws `list`'s surfaces, inside the caller's rendering, with their
	// materials in `materials` and each material's pipeline from
	// `pipelineOf` (none: the surfaces are not drawn and the view fails).
	// Surfaces of one binding whose index ranges touch draw as one range (a
	// world stage's meshlets, in the mesh's order).
	auto drawSurfaces =
	    [&]( const std::vector<std::uint32_t> &list,
	        const std::vector<Resources::Material> &materials,
	        const std::function<std::optional<PipelineId>( const Resources::Material & )>
	            &pipelineOf )
	{
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

	// A world stage's screen passes (render_lab's order): the depth and
	// normal prepass into the pass's own single-sample targets, with a
	// resolver of their own, then the composition's ambient occlusion, which
	// the lit view groups read.
	if ( world->stage && target.screenPasses && target.ambientOcclusion.IsValid() &&
	     !order.empty() )
	{
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
			encoder.BeginLabel( "core world prepass" );
			encoder.BeginRendering( prepassRendering );
			encoder.SetViewport( view.viewport );
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
				    return variant.Value();
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
				    return variant.Value();
			    } );
			encoder.EndRendering();
			encoder.EndLabel();
		}
	}
	encoder.BeginLabel( "core world" );
	encoder.BeginRendering( rendering );
	encoder.SetViewport( view.viewport );
	drawSurfaces( order, r.materials,
	    [&]( const Resources::Material &m ) -> std::optional<PipelineId>
	    {
		    if ( debugNeutral )
			    return m.program.request.pipeline;
		    auto debug = m.resolver->DebugPipeline(
		        m.program, frame::DebugSpecializationFor( view.debug, m.program.name ) );
		    if ( !debug )
		    {
			    note( "debug view: " + debug.Error() );
			    return std::nullopt;
		    }
		    return debug.Value();
	    } );
	encoder.EndRendering();
	encoder.EndLabel();
	for ( auto &[layout, group] : litViews )
		s.retiredGroups.emplace_back( target.frame, group );
	for ( auto &[layout, group] : reflectViews )
		s.retiredGroups.emplace_back( target.frame, group );

	std::lock_guard<std::mutex> guard( s.lock );
	++s.stats.viewsDrawn;
	s.stats.surfacesDrawn += order.size();
	if ( !complete )
	{
		++s.stats.viewsFailed;
		s.stats.lastFailure =
		    failure.empty() ? "a view named surfaces the pass does not draw" : failure;
	}
}

} // namespace render::pass::world
