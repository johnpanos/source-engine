//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): material claims, snapshot keys,
//			the draw predicates and opaque batching.
//
//=============================================================================//

#include "world_pass_internal.h"

namespace render::pass::world
{

namespace detail
{

namespace
{

std::string Lower( std::string text )
{
	for ( char &c : text )
		c = char( std::tolower( static_cast<unsigned char>( c ) ) );
	return text;
}

} // namespace

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
	// Length-prefixed fields, so no two inputs share a key. Built in one
	// reserved string: it runs for every dynamic draw.
	std::size_t size = source.name.size() + source.shader.size() + 64;
	for ( const auto *values : { &source.variables, &source.defaults } )
		for ( const auto &[name, value] : *values )
			size += name.size() + value.size() + 16;
	for ( const auto &[name, handle] : source.textures )
		size += name.size() + 32;
	std::string key;
	key.reserve( size );
	auto number = [&]( long long value )
	{
		char digits[24];
		const auto end = std::to_chars( digits, digits + sizeof( digits ), value ).ptr;
		key.append( digits, end );
	};
	auto append = [&]( std::string_view value )
	{
		number( static_cast<long long>( value.size() ) );
		key += ':';
		key += value;
	};
	append( source.name );
	append( source.shader );
	key += source.mesh ? 'M' : 'W';
	key += source.translucent ? 'T' : 'O';
	key += source.hasProxy ? 'P' : '-';
	for ( const auto *values : { &source.variables, &source.defaults } )
	{
		number( static_cast<long long>( values->size() ) );
		key += ':';
		for ( const auto &[name, value] : *values )
		{
			append( name );
			append( value );
		}
	}
	for ( const auto &[name, handle] : source.textures )
	{
		append( name );
		// The handle as its decimal text, length-prefixed (as before).
		char digits[24];
		const auto end = std::to_chars( digits, digits + sizeof( digits ), handle ).ptr;
		append( std::string_view( digits, std::size_t( end - digits ) ) );
	}
	return key;
}

} // namespace detail

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

std::size_t WorldPass::OpaqueBatchSize(
    std::span<const std::uint32_t> tags, std::uint64_t streamEpoch ) const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	return s.OpaqueBatchSize( tags, streamEpoch );
}

std::optional<std::string> WorldPass::DynamicClaim( const WorldMaterial &material )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const bool stage = s.world && s.world->stage != nullptr;
	const bool reflection = s.world && s.world->reflection.has_value();
	std::string key;
	const auto entry = s.Mapped( material, stage, reflection, key );
	if ( !entry->material )
		return entry->material.Error();
	if ( !entry->claimError.empty() )
		return entry->claimError;
	if ( entry->requiresDepthAlpha )
		return std::string( "$depthblend needs a captured depth-alpha texture" );
	return std::nullopt;
}

std::shared_ptr<const WorldPass::State::MappedEntry> WorldPass::State::Mapped(
    const WorldMaterial &source, bool stage, bool reflection, std::string &key )
{
	// Bounded: snapshot values that change every frame would otherwise grow it.
	// Each entry holds a parsed material description (tens of KB with its
	// variables).
	constexpr std::size_t kMaxMapped = 4096;
	// A revisioned material's key is built once (MaterialSnapshotKey formats
	// every variable, which per dynamic draw was a measured share of the frame).
	if ( source.revision )
	{
		std::lock_guard<std::mutex> guard( mappedLock );
		auto known = revisionKeys.find( source.revision );
		if ( known != revisionKeys.end() )
			key = known->second;
		else
		{
			key = MaterialSnapshotKey( source );
			if ( revisionKeys.size() >= 1024 )
				revisionKeys.clear();
			revisionKeys.emplace( source.revision, key );
		}
	}
	else
		key = MaterialSnapshotKey( source );
	std::shared_ptr<const MappedEntry> found;
	{
		std::lock_guard<std::mutex> guard( mappedLock );
		if ( auto at = mapped.find( key ); at != mapped.end() )
			found = at->second;
	}
	if ( found && found->claimStage == stage && found->claimReflection == reflection )
		return found;
	auto entry = std::make_shared<MappedEntry>(
	    MappedEntry{ found ? found->material : MapWorldMaterial( source ), stage, reflection, {},
	        false, {} } );
	if ( entry->material )
	{
		const material::MaterialDesc &desc = entry->material.Value().desc;
		// Scene color is the target's (the composition's capture).
		auto claim = source.mesh ? material::ClaimForMesh( desc, reflection, true )
		                         : material::ClaimForDrawing(
		                               desc, stage, &entry->requiresDepthAlpha, reflection );
		if ( !claim )
			entry->claimError = claim.Error();
		else
			entry->cardTerms = material::SpriteCardTermsFor( desc );
	}
	std::lock_guard<std::mutex> guard( mappedLock );
	if ( mapped.size() >= kMaxMapped )
		mapped.clear();
	auto &slot = mapped[key];
	slot = std::move( entry );
	return slot;
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
				// A view with dynamic draws never batches.
				if ( entry.serial == serial && entry.generation == s.generation &&
				     ( queue == &s.views || !streamEpoch || entry.recordedStream == streamEpoch ) )
					return entry.dynamic ? nullptr : &entry.view;
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

} // namespace render::pass::world
