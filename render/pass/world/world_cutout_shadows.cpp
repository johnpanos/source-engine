//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): a batch's cutout shadow casters.
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

bool WorldPass::Batch::RecordCutoutShadows()
{
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
			// Multi-draw indirect: a run's first world surface issues its
			// records (indirectCount from indirectFirst); the rest are covered.
			std::uint32_t indirectFirst = 0;
			std::uint32_t indirectCount = 0;
			bool covered = false;
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
			if ( instanceIndex >= world->PropCount() )
				continue;
			const auto &instance = world->Prop( instanceIndex );
			if ( instance.mesh >= world->staticMeshes.size() )
				continue;
			const WorldData::StaticMesh &mesh = world->staticMeshes[instance.mesh];
			if ( surfaceIndex >= mesh.surfaces.size() )
				continue;
			const std::uint32_t material = StaticMaterial( mesh, instance.material, surfaceIndex );
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
			cutout.world = &instance.world.rows[0].x;
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
		// World surfaces (RFC 0016 S2): their draws differ only in index
		// range within a material, so each material's run is one multi-draw
		// whose records serve every view (the atlas depth does not depend on
		// their order). Props keep one draw each (their matrices differ).
		BufferId cutoutCommands;
		if ( !draws.empty() && device.Facts().capabilities.Has( Capability::kMultiDrawIndirect ) )
		{
			const auto plain = std::stable_partition( draws.begin(), draws.end(),
			    []( const CutoutDraw &cutout )
			    {
				    return cutout.world == nullptr;
			    } );
			auto key = []( const CutoutDraw &cutout )
			{
				return std::make_pair( cutout.pipeline.value, cutout.material );
			};
			std::stable_sort( draws.begin(), plain,
			    [&]( const CutoutDraw &a, const CutoutDraw &b )
			    {
				    return key( a ) < key( b );
			    } );
			std::vector<DrawIndexedIndirectCommand> commands;
			for ( auto run = draws.begin(); run != plain; )
			{
				auto end = run;
				while ( end != plain && key( *end ) == key( *run ) )
				{
					commands.push_back(
					    { end->surface->indexCount, 1, end->surface->firstIndex, 0, 0 } );
					end->covered = end != run;
					++end;
				}
				run->indirectCount = static_cast<std::uint32_t>( end - run );
				run->indirectFirst =
				    static_cast<std::uint32_t>( commands.size() ) - run->indirectCount;
				run = end;
			}
			BufferDesc desc;
			desc.size = std::max<std::uint64_t>(
			    commands.size() * sizeof( DrawIndexedIndirectCommand ), 20 );
			desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndirect };
			if ( auto made =
			         commands.empty() ? std::nullopt : std::optional( device.CreateBuffer( desc ) );
			    made && *made )
			{
				cutoutCommands = made->Value();
				s.retiredBuffers.emplace_back( target.frame, cutoutCommands );
				encoder.TransitionBuffer(
				    cutoutCommands, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
				encoder.WriteBuffer( cutoutCommands, 0, std::as_bytes( std::span( commands ) ) );
				encoder.TransitionBuffer(
				    cutoutCommands, ResourceUsage::kCopyDestination, ResourceUsage::kIndirect );
			}
			else
			{
				for ( CutoutDraw &cutout : draws )
				{
					cutout.indirectCount = 0;
					cutout.covered = false;
				}
			}
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
					if ( cutout.covered )
						continue;
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
					if ( cutout.indirectCount )
					{
						encoder.DrawIndexedIndirect( cutoutCommands,
						    std::uint64_t( cutout.indirectFirst ) *
						        sizeof( DrawIndexedIndirectCommand ),
						    cutout.indirectCount, sizeof( DrawIndexedIndirectCommand ) );
						cutoutDraws += cutout.indirectCount;
						++cutoutIndirectDraws;
					}
					else
					{
						encoder.DrawIndexed(
						    cutout.surface->indexCount, 1, cutout.surface->firstIndex, 0, 0 );
						++cutoutDraws;
					}
				}
			}
			encoder.EndRendering();
			encoder.TransitionTexture(
			    cutouts.atlas, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
			preparation.End();
		}
		cutoutNotResident = notResident;
	}
	return true;
}

} // namespace render::pass::world
