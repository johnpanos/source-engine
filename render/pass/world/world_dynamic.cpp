//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): a batch's dynamic draws.
//
//=============================================================================//

#include "world_batch.h"

namespace render::pass::world
{

bool WorldPass::Batch::PrepareDynamicDraws()
{
	// Every dynamic draw's geometry in one vertex and one index buffer for the
	// batch, sliced by offset (a buffer pair per draw fragmented the PICA200's
	// linear memory until allocations failed with megabytes free).
	std::vector<WorldVertex> batchVertices;
	std::vector<std::uint32_t> batchIndices;
	std::size_t batchVertexCount = 0, batchIndexCount = 0;
	const std::size_t dynamicCount = queuedDynamic ? queuedDynamic->draws.size() : 0;
	for ( std::size_t dynamicIndex = 0; dynamicIndex < dynamicCount; ++dynamicIndex )
	{
		const WorldView::DynamicDraw &draw = queuedDynamic->draws[dynamicIndex];
		// The mapping and key decided when the view queued.
		const std::string &key = queuedDynamic->keys[dynamicIndex];
		const State::MappedMaterial &mapped = queuedDynamic->mapped[dynamicIndex]->material;
		const std::size_t drawIndexCount =
		    draw.indices16.empty() ? draw.indices.size() : draw.indices16.size();
		const std::size_t drawVertexCount = draw.vertices.size();
		const auto outOfRange = [&]( std::uint32_t index )
		{
			return index >= drawVertexCount;
		};
		if ( !mapped || drawVertexCount == 0 || drawIndexCount == 0 ||
		     drawIndexCount % 3 != 0 ||
		     std::any_of( draw.indices.begin(), draw.indices.end(), outOfRange ) ||
		     std::any_of( draw.indices16.begin(), draw.indices16.end(), outOfRange ) )
		{
			note( "dynamic material " + draw.Material().name + ": " +
			      ( mapped ? "invalid triangle geometry" : mapped.Error() ) );
			complete = false;
			continue;
		}
		Resources::Material &cached = r.dynamicMaterials[key];
		cached.lastUsed = target.frame;
		Resources::Material *m =
		    prepareMaterial( *r.resolver, cached, mapped.Value(), draw.Material() );
		if ( !m || !drawGroupReady( *m, draw.lightmapPage, draw.capturedLightmap ) ||
		     !frameGroupReady( *m ) || ( !m->program.sceneColor && !viewGroupReady( *m ) ) )
		{
			complete = false;
			continue;
		}
		// A mesh point without draw inputs reads the draw's model lighting.
		const Group *lit = nullptr;
		if ( draw.lighting && m->program.drawInputs.empty() )
		{
			const std::optional<material::GroupRequest> request =
			    m->resolver->DrawGroup( m->program, {}, &*draw.lighting );
			litDrawGroups.emplace_back();
			std::string why;
			if ( !request || !buildGroup( *request, {}, litDrawGroups.back(), &why ) )
			{
				s.ReleaseGroup( litDrawGroups.back(), CompletionToken() );
				litDrawGroups.pop_back();
				note( "a model-lighting draw group: " +
				      ( why.empty() ? std::string( "not resolved" ) : why ) );
				complete = false;
				continue;
			}
			lit = &litDrawGroups.back();
		}
		const std::uint64_t vertexOffset = batchVertexCount * sizeof( WorldVertex );
		const std::uint64_t indexOffset = batchIndexCount * sizeof( std::uint32_t );
		// A static prop's baked vertex lighting is a variant of its program.
		PipelineId pipeline = m->program.request.pipeline;
		if ( draw.staticVertexLight )
		{
			auto variant = m->resolver->StaticVertexLightPipeline( m->program );
			if ( !variant )
			{
				note( "material " + draw.Material().name +
				      ": its static vertex light variant: " + variant.Error() );
				complete = false;
				continue;
			}
			pipeline = variant.Value();
		}
		dynamicDraws.push_back( { m, BufferId{}, BufferId{}, std::uint32_t( drawIndexCount ),
		    draw.lightmapPage, &draw, lit, pipeline, vertexOffset, indexOffset } );
		batchVertexCount += draw.vertices.size();
		batchIndexCount += drawIndexCount;
	}
	if ( !dynamicDraws.empty() )
	{
		// One draw (the common case: a slot per draw) writes from its own
		// geometry, 16-bit indices as they are; several are gathered once,
		// with 32-bit indices.
		std::span<const WorldVertex> vertexBytes;
		std::span<const std::byte> indexBytes;
		if ( dynamicDraws.size() == 1 )
		{
			const WorldView::DynamicDraw &only = *dynamicDraws.front().source;
			vertexBytes = only.vertices;
			if ( only.indices16.empty() )
				indexBytes = std::as_bytes( std::span( only.indices ) );
			else
			{
				indexBytes = std::as_bytes( std::span( only.indices16 ) );
				dynamicDraws.front().indexFormat = IndexFormat::kUint16;
			}
		}
		else
		{
			batchVertices.reserve( batchVertexCount );
			batchIndices.reserve( batchIndexCount );
			for ( const DynamicDraw &draw : dynamicDraws )
			{
				batchVertices.insert( batchVertices.end(), draw.source->vertices.begin(),
				    draw.source->vertices.end() );
				batchIndices.insert(
				    batchIndices.end(), draw.source->indices.begin(), draw.source->indices.end() );
				batchIndices.insert( batchIndices.end(), draw.source->indices16.begin(),
				    draw.source->indices16.end() );
			}
			vertexBytes = batchVertices;
			indexBytes = std::as_bytes( std::span( batchIndices ) );
		}
		BufferDesc desc;
		desc.size = vertexBytes.size() * sizeof( WorldVertex );
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kVertex };
		desc.debugName = "core dynamic vertices";
		auto vertices = vertexBytes.empty() ? DeviceResult<BufferId>( BufferId{} )
		                                    : device.CreateBuffer( desc );
		desc.size = indexBytes.size();
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kIndex };
		desc.debugName = "core dynamic indices";
		auto indices = device.CreateBuffer( desc );
		if ( !vertices || !indices )
		{
			const DeviceError error = !vertices ? vertices.Error() : indices.Error();
			for ( auto *buffer : { &vertices, &indices } )
			{
				if ( *buffer && buffer->Value().IsValid() )
					(void)device.Release( buffer->Value(), CompletionToken() );
			}
			note( "dynamic geometry buffers were refused (" +
			      std::string( DescribeStatus( error.status ) ) + ", " +
			      std::to_string( vertexBytes.size() ) + " vertices)" );
			complete = false;
			dynamicDraws.clear();
		}
		else
		{
			for ( const auto &[buffer, bytes, usage] :
			    { std::tuple{
			          vertices.Value(), std::as_bytes( vertexBytes ), ResourceUsage::kVertex },
			        std::tuple{ indices.Value(), indexBytes, ResourceUsage::kIndex } } )
			{
				if ( !buffer.IsValid() )
					continue;
				encoder.TransitionBuffer(
				    buffer, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
				encoder.WriteBuffer( buffer, 0, bytes );
				encoder.TransitionBuffer( buffer, ResourceUsage::kCopyDestination, usage );
				s.retiredBuffers.emplace_back( target.frame, buffer );
			}
			for ( DynamicDraw &draw : dynamicDraws )
			{
				draw.vertices = vertices.Value();
				draw.indices = indices.Value();
			}
		}
		batchVertices.clear();
		batchVertices.shrink_to_fit();
		batchIndices.clear();
		batchIndices.shrink_to_fit();
	}
	preparation.End();
	return true;
}

} // namespace render::pass::world
