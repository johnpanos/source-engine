//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5); see world_pass.h. Names no
//			material family: programs come from the one resolver.
//
//=============================================================================//

#include "render/pass/world/world_pass.h"

#include "render/material/draw_program.h"
#include "render/material/program_resolver.h"
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
	// A 1x1 white texture for an absent input (a surface with no lightmap
	// page samples white: the input's neutral value), and its upload buffer.
	TextureId neutralWhite;
	BufferId neutralStaging;
	bool uploaded = false;
};

std::string Lower( std::string text )
{
	for ( char &c : text )
		c = char( std::tolower( static_cast<unsigned char>( c ) ) );
	return text;
}

// Whether serial a was issued before serial b (serials wrap within the
// tag's low 31 bits).
bool IssuedBefore( std::uint32_t a, std::uint32_t b )
{
	const std::uint32_t distance = ( b - a ) & ~kWorldTag;
	return distance != 0 && distance < ( kWorldTag >> 1 );
}

std::string PageName( int handle )
{
	return "lightmap-page:" + std::to_string( handle );
}

} // namespace

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
			if ( old.neutralWhite.IsValid() )
				(void)device->Release( old.neutralWhite, after );
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
			auto blend = material::ClaimForDrawing( claimed.desc );
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
		std::string name = data.materials[m].name;
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
	s.nextSerial = ( s.nextSerial + 1 ) & ~kWorldTag;
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
	const std::uint32_t serial = tag & ~kWorldTag;
	std::shared_ptr<const WorldData> world;
	std::shared_ptr<const std::vector<Claimed>> claims;
	std::uint64_t generation = 0;
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
	if ( !r.resolver )
	{
		auto resolver = material::ProgramResolver::Create(
		    device, target.colorFormat, target.depthFormat, target.samples );
		if ( !resolver )
		{
			s.Fail( resolver.Error() );
			return;
		}
		r.resolver = std::move( resolver ).Value();
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

	// The neutral white texture, made on first use.
	auto neutralWhite = [&]() -> TextureId
	{
		if ( r.neutralWhite.IsValid() )
			return r.neutralWhite;
		TextureDesc desc;
		desc.format = Format::kRGBA8Unorm;
		desc.width = desc.height = 1;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		desc.debugName = "world neutral white";
		BufferDesc staging;
		staging.size = 4;
		staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
		auto texture = device.CreateTexture( desc );
		auto buffer = device.CreateBuffer( staging );
		if ( !texture || !buffer )
		{
			if ( texture )
				(void)device.Release( texture.Value(), CompletionToken() );
			if ( buffer )
				(void)device.Release( buffer.Value(), CompletionToken() );
			return {};
		}
		const std::byte white[4] = {
		    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, white );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		encoder.TransitionTexture(
		    texture.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.CopyBufferToTexture( buffer.Value(), texture.Value(), { 0, 0, 0, 1, 1 } );
		encoder.TransitionTexture(
		    texture.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
		r.neutralWhite = texture.Value();
		r.neutralStaging = buffer.Value();
		return r.neutralWhite;
	};

	// A group from its request: the constants uploaded here, the textures
	// imported by the names' handles, with the backend's samplers; a handle
	// of 0 (an absent input) takes the neutral white texture.
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
		for ( const material::ProgramTexture &texture : request.textures )
		{
			const auto handle = handles.find( texture.name );
			const bool absent = handle != handles.end() && handle->second == 0;
			const TextureId id = handle == handles.end() ? TextureId()
			                     : absent ? neutralWhite()
			                              : textures.Import( handle->second, texture.srgb );
			if ( !id.IsValid() )
			{
				*why = handle == handles.end()
				           ? "texture " + texture.name + " has no material system handle"
				           : "texture " + texture.name + " did not import" +
				                 ( texture.srgb ? " through an sRGB view" : "" );
				return false;
			}
			auto sampler =
			    device.CreateSampler( absent ? SamplerDesc() : textures.Sampler( handle->second ) );
			if ( !sampler )
			{
				*why = "a sampler was refused";
				return false;
			}
			out.samplers.push_back( sampler.Value() );
			entries.push_back( { texture.binding, {}, 0, 0, id, {} } );
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
		    return x.material != y.material ? x.material < y.material
		                                    : x.lightmapPage < y.lightmapPage;
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
	for ( const std::uint32_t index : order )
	{
		const WorldSurface &surface = world->surfaces[index];
		const Resources::Material &m = r.materials[surface.material];
		if ( surface.material != boundMaterial )
		{
			encoder.SetPipeline( m.program.request.pipeline );
			if ( m.program.request.frameLayout.IsValid() )
				encoder.SetBindGroup( BindGroupRole::kFrame,
				    r.frameGroups[m.program.request.frameLayout.value].group );
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
			const auto key =
			    std::make_pair( m.program.request.drawLayout.value, surface.lightmapPage );
			encoder.SetBindGroup( BindGroupRole::kDraw, r.drawGroups[key].group );
			boundPage = surface.lightmapPage;
			pageBound = true;
		}
		encoder.DrawIndexed( surface.indexCount, 1, surface.firstIndex, 0, 0 );
	}
	encoder.EndRendering();
	encoder.EndLabel();

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
