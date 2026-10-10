//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5); see world_pass.h. Names no
//			material family: programs come from the one resolver.
//
//=============================================================================//

#include "world_pass_internal.h"

namespace render::pass::world
{

WorldPass::WorldPass() : m_State( std::make_unique<State>() )
{
}

// Without ReleaseDevice the device may be gone: the handles are dropped.
WorldPass::~WorldPass() = default;

void WorldPass::SetSurfaceFragmentModule( std::span<const std::uint32_t> module )
{
	m_State->fragmentModule = module;
}

void WorldPass::RemapLightmapPages( std::span<const int> handles )
{
	State &s = *m_State;
	if ( !s.world || handles.size() != s.world->surfaces.size() )
		return;
	bool changed = false;
	for ( std::size_t i = 0; i < handles.size() && !changed; ++i )
		changed = s.world->surfaces[i].lightmapPage != handles[i];
	if ( !changed )
		return;
	auto world = std::make_shared<WorldData>( *s.world );
	for ( std::size_t i = 0; i < handles.size(); ++i )
		world->surfaces[i].lightmapPage = handles[i];
	s.world = std::move( world );
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
			auto blend =
			    source.mesh
			        ? material::ClaimForMesh( claimed.desc, data.reflection.has_value(), true )
			        : material::ClaimForDrawing( claimed.desc, data.stage != nullptr, nullptr,
			              data.reflection.has_value() );
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

void WorldPass::SetPipelinePrewarm( std::vector<std::string> keys )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.pipelineKeyLock );
	s.prewarmKeys = std::move( keys );
}

std::vector<std::string> WorldPass::CreatedPipelineKeys() const
{
	const State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.pipelineKeyLock );
	return { s.createdKeys.begin(), s.createdKeys.end() };
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

void WorldPass::NoteRefusal( std::string reason )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	s.Refuse( std::move( reason ) );
}

namespace
{

// A dynamic draw's SpriteCard records as world-space vertices: each corner
// from render.sprite-card.v1 with the material's card terms, its position
// through the card's model matrix, the frame's coordinate as the base uv, the
// next frame's in the lightmap-uv slot and the blend in the lightmap offset
// (surface_program.glsl's SpriteCardSurface reads them there).
void ExpandCards( sprite_card::Frame frame, WorldView::DynamicDraw &draw )
{
	std::copy_n( draw.cardModel, 16, frame.model );
	std::copy_n( draw.cardView, 16, frame.view );
	frame.splineRange = draw.cardSplineRange;
	// splinecard_vs20 faces the camera; Portal 2's spline cards with end
	// normals (TEXCOORD6/7) turn toward them.
	if ( frame.kind == sprite_card::Kind::kSpline )
		frame.orientation = draw.cardSplineNormals ? 3 : 0;
	sprite_card::Prepare( frame );
	draw.vertices.resize( draw.cards.size() );
	for ( std::size_t i = 0; i < draw.cards.size(); ++i )
	{
		const sprite_card::Corner corner = sprite_card::Expand( frame, draw.cards[i] );
		WorldVertex &vertex = draw.vertices[i];
		vertex = {};
		for ( int j = 0; j < 3; ++j )
			vertex.position[j] = corner.position[0] * frame.model[j] +
			                     corner.position[1] * frame.model[4 + j] +
			                     corner.position[2] * frame.model[8 + j] + frame.model[12 + j];
		vertex.uv[0] = corner.uv[0];
		vertex.uv[1] = corner.uv[1];
		vertex.lightmapUv[0] = corner.uv2[0];
		vertex.lightmapUv[1] = corner.uv2[1];
		vertex.lightmapOffset = corner.blend;
		for ( int c = 0; c < 4; ++c )
			vertex.color[c] =
			    std::uint8_t( std::clamp( corner.color[c], 0.0f, 1.0f ) * 255.0f + 0.5f );
	}
	draw.cards.clear();
	draw.cards.shrink_to_fit();
}

} // namespace

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
	std::shared_ptr<State::QueuedDynamic> dynamic;
	if ( !view.dynamicDraws.empty() )
	{
		dynamic = std::make_shared<State::QueuedDynamic>();
		dynamic->keys.reserve( view.dynamicDraws.size() );
		dynamic->mapped.reserve( view.dynamicDraws.size() );
	}
	const bool stage = s.world->stage != nullptr;
	const bool reflection = s.world->reflection.has_value();
	for ( WorldView::DynamicDraw &draw : view.dynamicDraws )
	{
		std::string key;
		const auto entry = s.Mapped( draw.Material(), stage, reflection, key );
		std::string why;
		if ( !entry->material )
			why = entry->material.Error();
		else if ( !entry->claimError.empty() )
			why = entry->claimError;
		else if ( !draw.cards.empty() && !entry->cardTerms )
			why = "card records for a material that is not a SpriteCard";
		else if ( entry->requiresDepthAlpha &&
		          ( view.depthAlphaHandle <= 0 || !std::isfinite( view.depthAlphaRange ) ||
		              view.depthAlphaRange <= 0.0f ) )
			why = "$depthblend needs a captured depth-alpha texture and positive range";
		if ( why.empty() && !draw.cards.empty() )
			ExpandCards( *entry->cardTerms, draw );
		dynamic->keys.push_back( std::move( key ) );
		dynamic->mapped.push_back( entry );
		if ( !why.empty() )
		{
			s.Refuse( "material " + draw.Material().name + ": " + why );
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
	if ( dynamic )
	{
		dynamic->draws = std::move( view.dynamicDraws );
		view.dynamicDraws.clear();
	}
	s.views.push_back( { serial, s.generation, 0, std::move( view ), std::move( dynamic ) } );
	++s.stats.viewsQueued;
	return kWorldTag | serial;
}

bool WorldPass::HasWorld() const
{
	std::lock_guard<std::mutex> guard( m_State->lock );
	return m_State->world != nullptr;
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
	for ( auto &[frame, group] : s.retiredBindGroups )
		(void)device.Release( group, CompletionToken() );
	s.retiredBindGroups.clear();
	s.gpuCull.reset();
	s.gpuCompact.reset();
	s.gpuOcclusion.reset();
	if ( s.gpuPointSampler.IsValid() )
		(void)device.Release( s.gpuPointSampler, CompletionToken() );
	s.gpuPointSampler = SamplerId();
	s.gpuPyramid = State::GpuPyramid();
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

std::optional<std::string> UiListView(
    const ui_draw_list::ListView &list, std::span<const WorldMaterial> materials, WorldView &out )
{
	if ( const char *why = ui_draw_list::Validate( list ) )
		return std::string( why );
	if ( list.materialCount != materials.size() )
		return std::string( "the list names " ) + std::to_string( list.materialCount ) +
		       " materials but " + std::to_string( materials.size() ) + " were given";
	out = WorldView();
	out.drawsWorldGeometry = false;
	out.viewport = { float( list.viewport[0] ), float( list.viewport[1] ),
	    float( list.viewport[2] ), float( list.viewport[3] ), 0.0f, 1.0f };
	const math::float4x4 toClip =
	    math::PixelToClip( float( list.viewport[2] ), float( list.viewport[3] ) );
	for ( int r = 0; r < 4; ++r )
	{
		out.toClip[r * 4 + 0] = toClip.rows[r].x;
		out.toClip[r * 4 + 1] = toClip.rows[r].y;
		out.toClip[r * 4 + 2] = toClip.rows[r].z;
		out.toClip[r * 4 + 3] = toClip.rows[r].w;
	}
	out.dynamicDraws.reserve( list.commandCount );
	for ( std::uint32_t i = 0; i < list.commandCount; ++i )
	{
		const ui_draw_list::Command &command = list.commands[i];
		if ( command.vertexCount == 0 )
			continue;
		WorldView::DynamicDraw draw;
		draw.material = materials[command.material];
		draw.vertices.resize( command.vertexCount );
		draw.indices.resize( command.vertexCount );
		for ( std::uint32_t v = 0; v < command.vertexCount; ++v )
		{
			const ui_draw_list::Vertex &source = list.vertices[command.firstVertex + v];
			WorldVertex &vertex = draw.vertices[v];
			// The port's pixel, less the half pixel the view's D3D9
			// transform is shifted by when it draws.
			vertex.position[0] =
			    ui_draw_list::ToPixel( source.x, list.scale, list.offset[0] ) - 0.5f;
			vertex.position[1] =
			    ui_draw_list::ToPixel( source.y, list.scale, list.offset[1] ) - 0.5f;
			vertex.position[2] = 0.5f;
			vertex.uv[0] = source.s;
			vertex.uv[1] = source.t;
			std::copy_n( source.color, 4, vertex.color );
			draw.indices[v] = v;
		}
		out.dynamicDraws.push_back( std::move( draw ) );
	}
	return std::nullopt;
}

} // namespace render::pass::world
