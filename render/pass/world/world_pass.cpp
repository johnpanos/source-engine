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
#include <map>
#include <mutex>
#include <span>

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
		Group group;
		bool ready = false;
		bool failed = false;
		std::string failure; // why, when failed
	};
	std::vector<Material> materials;
	// Draw groups by (draw layout, lightmap page).
	std::map<std::pair<std::uint64_t, int>, Group> drawGroups;
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
// them for the same map, less the view's shadows, occlusion and reflections,
// which the stage does not draw yet).
std::uint32_t StageTerms( const WorldStage &stage )
{
	// The view's runtime lights (a view without them binds the neutral view).
	std::uint32_t terms = material::kSurfaceClustered;
	if ( stage.lightmap.Directional() )
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
	std::shared_ptr<const LightmapPages> stageLightmap;
	std::uint64_t stageLightmapRevision = 0;
	std::shared_ptr<const std::vector<std::byte>> stageChange;
	std::shared_ptr<const StageProbeVolume> stageTable;
	std::uint64_t stageChangeRevision = 0;

	// A queued view dropped because its slot never recorded (lock held): a
	// failure when a slot of its host frame recorded (or its frame is
	// unknown), else skipped with its frame.
	void Drop( const Queued &dropped, std::uint64_t recordingFrame )
	{
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
	s.stageChange.reset();
	s.stageTable.reset();
	s.views.clear(); // recorded views stay: a re-recorded slot of theirs fails alone
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
	s.views.clear(); // recorded views stay: a re-recorded slot of theirs fails alone
}

void WorldPass::SetStageLightmap( LightmapPages pages )
{
	State &s = *m_State;
	auto shared = std::make_shared<const LightmapPages>( std::move( pages ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageLightmap = std::move( shared );
	++s.stageLightmapRevision;
}

void WorldPass::SetStageChange( std::vector<std::byte> change, StageProbeVolume table )
{
	State &s = *m_State;
	auto sharedChange = std::make_shared<const std::vector<std::byte>>( std::move( change ) );
	auto sharedTable = std::make_shared<const StageProbeVolume>( std::move( table ) );
	std::lock_guard<std::mutex> guard( s.lock );
	s.stageChange = std::move( sharedChange );
	s.stageTable = std::move( sharedTable );
	++s.stageChangeRevision;
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
	std::shared_ptr<const LightmapPages> stageLightmap;
	std::uint64_t stageLightmapRevision = 0;
	std::shared_ptr<const std::vector<std::byte>> stageChange;
	std::shared_ptr<const StageProbeVolume> stageTable;
	std::uint64_t stageChangeRevision = 0;
	WorldView view;
	bool found = false;
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
		stageLightmap = s.stageLightmap;
		stageLightmapRevision = s.stageLightmapRevision;
		stageChange = s.stageChange;
		stageTable = s.stageTable;
		stageChangeRevision = s.stageChangeRevision;
	}
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
	if ( world->stage && stageLightmap && r.stageLightmapRevision != stageLightmapRevision )
	{
		r.stageLightmapRevision = stageLightmapRevision;
		const TextureDesc &desc = r.stageDescs[kStageLightmap];
		const bool fits = stageLightmap->width == desc.width &&
		                  stageLightmap->height == desc.height &&
		                  stageLightmap->Directional() == world->stage->lightmap.Directional();
		if ( !fits ||
		     !stageUpload( r.stageTextures[kStageLightmap], desc, stageLightmap->flat,
		         ResourceUsage::kSampled ) ||
		     ( stageLightmap->Directional() &&
		         !stageUpload( r.stageTextures[kStageGradient], desc, stageLightmap->gradient,
		             ResourceUsage::kSampled ) ) )
		{
			s.Fail( "the world stage's lightmap update does not fit its pages" );
			return;
		}
	}
	if ( world->stage && world->stage->probes && stageTable &&
	     r.stageChangeRevision != stageChangeRevision )
	{
		r.stageChangeRevision = stageChangeRevision;
		const StageProbeVolume &probes = *world->stage->probes;
		const std::uint32_t rows = probes.rows + kStageOccluderRows;
		const std::vector<std::byte> zero(
		    stageChange && !stageChange->empty() ? 0 : probes.atlas.size(), std::byte( 0 ) );
		const std::vector<std::byte> &change =
		    stageChange && !stageChange->empty() ? *stageChange : zero;
		const std::vector<float> table = paddedTable( *stageTable, rows );
		if ( change.size() != probes.atlas.size() ||
		     stageTable->tableTexels != probes.tableTexels || stageTable->rows > rows ||
		     !stageUpload( r.stageTextures[kStageChange], r.stageDescs[kStageChange], change,
		         ResourceUsage::kSampled ) ||
		     !stageUpload( r.stageTextures[kStageProbeGrids], r.stageDescs[kStageProbeGrids],
		         std::as_bytes( std::span( table ) ), ResourceUsage::kSampled ) )
		{
			s.Fail( "the world stage's probe change does not fit its volume" );
			return;
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
	auto materialReady = [&]( std::uint32_t index ) -> Resources::Material *
	{
		Resources::Material &m = r.materials[index];
		if ( m.failed )
			note( m.failure );
		if ( m.ready || m.failed )
			return m.ready ? &m : nullptr;
		const Claimed &claimed = ( *claims )[index];
		const std::string &name = world->materials[index].name;
		auto program = r.resolver->Resolve( claimed.desc );
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
		m.ready = true;
		return &m;
	};
	auto drawGroupReady = [&]( const Resources::Material &m, int page ) -> const Group *
	{
		const auto key = std::make_pair( m.program.request.drawLayout.value, page );
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
		    r.resolver->DrawGroup( m.program, inputs );
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
	}
	std::map<std::uint64_t, bool> framesWritten;
	auto frameGroupReady = [&]( const Resources::Material &m ) -> const Group *
	{
		const std::uint64_t layout = m.program.request.frameLayout.value;
		if ( framesWritten[layout] )
			return &r.frameGroups[layout];
		const std::optional<material::GroupRequest> request =
		    r.resolver->FrameGroup( m.program, terms );
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
	auto viewGroupReady = [&]( const Resources::Material &m ) -> const Group *
	{
		const std::uint64_t layout = m.program.request.viewLayout.value;
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
			const material::GroupRequest request = r.resolver->Program().ViewGroup(
			    lights.view, lights.froxels, lights.indices, lights.lights, shadows );
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
		     ( m->program.request.frameLayout.IsValid() && !frameGroupReady( *m ) ) ||
		     ( m->program.request.viewLayout.IsValid() && !viewGroupReady( *m ) ) )
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

	const ColorAttachment colors[] = { { target.color, LoadOp::kLoad, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ target.depth, LoadOp::kLoad, StoreOp::kStore, 1.0f };
	rendering.width = target.width;
	rendering.height = target.height;
	encoder.BeginLabel( "core world" );
	encoder.BeginRendering( rendering );
	encoder.SetViewport( view.viewport );
	// D3D9 puts pixel centers on integer coordinates: a D3D9 transform is
	// shifted right and down by half a pixel of the viewport.
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
	std::uint32_t boundMaterial = ~0u;
	int boundPage = 0;
	bool pageBound = false;
	// The frame's debug controls (RFC 0014), as the view was queued: each
	// program's pipeline under its specialization (the shipped one when the
	// controls are neutral). A refused debug pipeline fails the view loudly.
	const bool debugNeutral = frame::DebugControlsNeutral( view.debug );
	// Surfaces of one binding whose index ranges touch draw as one range (a
	// world stage's meshlets, in the mesh's order).
	std::uint32_t runFirst = 0;
	std::uint32_t runCount = 0;
	auto flushRun = [&]()
	{
		if ( runCount )
			encoder.DrawIndexed( runCount, 1, runFirst, 0, 0 );
		runCount = 0;
	};
	for ( const std::uint32_t index : order )
	{
		const WorldSurface &surface = world->surfaces[index];
		const Resources::Material &m = r.materials[surface.material];
		if ( surface.material != boundMaterial )
		{
			flushRun();
			PipelineId pipeline = m.program.request.pipeline;
			if ( !debugNeutral )
			{
				auto debug = r.resolver->DebugPipeline(
				    m.program, frame::DebugSpecializationFor( view.debug, m.program.name ) );
				if ( !debug )
				{
					note( "debug view: " + debug.Error() );
					complete = false;
					continue;
				}
				pipeline = debug.Value();
			}
			encoder.SetPipeline( pipeline );
			if ( m.program.request.frameLayout.IsValid() )
				encoder.SetBindGroup( BindGroupRole::kFrame,
				    r.frameGroups[m.program.request.frameLayout.value].group );
			if ( m.program.request.viewLayout.IsValid() )
			{
				const std::uint64_t layout = m.program.request.viewLayout.value;
				const auto lit = litViews.find( layout );
				encoder.SetBindGroup( BindGroupRole::kView,
				    lit != litViews.end() ? lit->second.group : r.viewGroups[layout].group );
			}
			encoder.SetBindGroup( BindGroupRole::kMaterial, m.group.group );
			encoder.SetVertexBuffer( 0, r.vertices, 0 );
			encoder.SetIndexBuffer( r.indices, 0, IndexFormat::kUint32 );
			encoder.SetDrawConstants(
			    0, constantBytes.first( m.program.request.drawConstantBytes ) );
			boundMaterial = surface.material;
			pageBound = false;
		}
		if ( m.program.request.drawLayout.IsValid() &&
		     ( !pageBound || surface.lightmapPage != boundPage ) )
		{
			flushRun();
			const auto key =
			    std::make_pair( m.program.request.drawLayout.value, surface.lightmapPage );
			encoder.SetBindGroup( BindGroupRole::kDraw, r.drawGroups[key].group );
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
	encoder.EndRendering();
	encoder.EndLabel();
	for ( auto &[layout, group] : litViews )
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
