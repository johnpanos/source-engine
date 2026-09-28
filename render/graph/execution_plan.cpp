//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 execution shared by both executors and the
//			transient pool (RFC 0016); see execution_plan.h and executor.h.
//
//=============================================================================//

#include "execution_plan.h"

#include <utility>

namespace render::graph
{

device::TextureId RecordContext::Texture( ResourceRef resource ) const
{
	return resource.index < m_Textures.size() ? m_Textures[resource.index] : device::TextureId{};
}

device::BufferId RecordContext::Buffer( ResourceRef resource ) const
{
	return resource.index < m_Buffers.size() ? m_Buffers[resource.index] : device::BufferId{};
}

namespace
{

bool SameEntry( const PhysicalResource &a, const PhysicalResource &b )
{
	if ( a.isTexture != b.isTexture || !( a.usages == b.usages ) )
		return false;
	if ( a.isTexture )
		return a.texture.dimension == b.texture.dimension && a.texture.format == b.texture.format &&
		       a.texture.width == b.texture.width && a.texture.height == b.texture.height &&
		       a.texture.depthOrLayers == b.texture.depthOrLayers &&
		       a.texture.mipLevels == b.texture.mipLevels &&
		       a.texture.sampleCount == b.texture.sampleCount;
	return a.buffer.size == b.buffer.size && a.buffer.memory == b.buffer.memory;
}

void Release( device::IRenderDevice2 &device, const TransientPool::Entry &entry,
    device::CompletionToken token )
{
	if ( entry.texture.IsValid() )
		(void)device.Release( entry.texture, token );
	if ( entry.buffer.IsValid() )
		(void)device.Release( entry.buffer, token );
}

void RecordTransition( device::CommandEncoder &encoder, const CompiledGraph &graph,
    const detail::Plan &plan, const Transition &transition, device::ResourceUsage before )
{
	const std::uint32_t r = transition.resource.index;
	if ( graph.resources[r].isTexture )
		encoder.TransitionTexture( plan.textures[r], before, transition.after );
	else
		encoder.TransitionBuffer( plan.buffers[r], before, transition.after );
}

} // namespace

TransientPool::TransientPool( device::IRenderDevice2 &device, std::uint32_t idleExecutions )
    : m_Device( device ), m_IdleExecutions( idleExecutions )
{
}

TransientPool::~TransientPool()
{
	for ( const Entry &entry : m_Entries )
		Release( m_Device, entry, entry.token );
}

bool TransientPool::Take( const PhysicalResource &shape, Entry *out )
{
	for ( auto it = m_Entries.begin(); it != m_Entries.end(); ++it )
	{
		if ( SameEntry( it->shape, shape ) )
		{
			*out = std::move( *it );
			m_Entries.erase( it );
			return true;
		}
	}
	return false;
}

void TransientPool::Return( std::vector<Entry> &&entries )
{
	for ( auto it = m_Entries.begin(); it != m_Entries.end(); )
	{
		if ( ++it->idle > m_IdleExecutions )
		{
			Release( m_Device, *it, it->token );
			it = m_Entries.erase( it );
		}
		else
		{
			++it;
		}
	}
	for ( Entry &entry : entries )
	{
		entry.idle = 0;
		m_Entries.push_back( std::move( entry ) );
	}
}

namespace detail
{

std::uint32_t TransitionCount( const CompiledGraph &graph )
{
	std::size_t count = graph.finalTransitions.size();
	for ( const CompiledPass &pass : graph.order )
		count += pass.transitions.size();
	return static_cast<std::uint32_t>( count );
}

foundation::Expected<Plan, device::DeviceError> Prepare(
    const CompiledGraph &graph, device::IRenderDevice2 &device, TransientPool *pool )
{
	Plan plan;
	const std::size_t count = graph.resources.size();
	plan.textures.resize( count );
	plan.buffers.resize( count );
	for ( std::size_t r = 0; r < count; ++r )
	{
		plan.textures[r] = graph.resources[r].importedTexture;
		plan.buffers[r] = graph.resources[r].importedBuffer;
	}
	plan.physical.resize( graph.physical.size() );
	plan.fromPool.assign( graph.physical.size(), false );
	for ( std::size_t p = 0; p < graph.physical.size(); ++p )
	{
		const PhysicalResource &shape = graph.physical[p];
		TransientPool::Entry &entry = plan.physical[p];
		if ( pool && pool->Take( shape, &entry ) )
		{
			plan.fromPool[p] = true;
			++plan.reused;
			continue;
		}
		entry.shape = shape;
		const ResourceDecl &first = graph.resources[shape.members.front()];
		if ( shape.isTexture )
		{
			device::TextureDesc desc = shape.texture;
			desc.usages = shape.usages;
			desc.debugName = first.name;
			auto texture = device.CreateTexture( desc );
			if ( !texture )
			{
				Finish( graph, plan, device, pool, nullptr );
				return foundation::MakeUnexpected( texture.Error() );
			}
			entry.texture = texture.Value();
		}
		else
		{
			device::BufferDesc desc = shape.buffer;
			desc.usages = shape.usages;
			desc.debugName = first.name;
			auto buffer = device.CreateBuffer( desc );
			if ( !buffer )
			{
				Finish( graph, plan, device, pool, nullptr );
				return foundation::MakeUnexpected( buffer.Error() );
			}
			entry.buffer = buffer.Value();
		}
		++plan.created;
	}
	for ( std::size_t r = 0; r < count; ++r )
	{
		const std::int32_t p = graph.physicalOf[r];
		if ( p >= 0 )
		{
			plan.textures[r] = plan.physical[p].texture;
			plan.buffers[r] = plan.physical[p].buffer;
		}
	}

	// The transitions as recorded: a physical resource's first transition
	// starts from the usage the pool left it in, not from "undefined".
	std::vector<bool> seen( graph.physical.size(), false );
	std::vector<device::ResourceUsage> state( graph.physical.size() );
	for ( std::size_t p = 0; p < graph.physical.size(); ++p )
		state[p] = plan.physical[p].usage;
	plan.before.resize( graph.order.size() );
	for ( std::size_t i = 0; i < graph.order.size(); ++i )
	{
		for ( const Transition &transition : graph.order[i].transitions )
		{
			device::ResourceUsage before = transition.before;
			const std::int32_t p = graph.physicalOf[transition.resource.index];
			if ( p >= 0 )
			{
				if ( !seen[p] && before == device::ResourceUsage::kUndefined )
					before = state[p];
				seen[p] = true;
				state[p] = transition.after;
			}
			plan.before[i].push_back( before );
		}
	}
	for ( const Transition &transition : graph.finalTransitions )
		plan.finalBefore.push_back( transition.before );
	plan.exitUsage = std::move( state );
	return plan;
}

void RecordPass( device::CommandEncoder &encoder, const CompiledGraph &graph, const Plan &plan,
    std::size_t index )
{
	const CompiledPass &compiled = graph.order[index];
	const PassDecl &pass = graph.passes[compiled.declaration];
	encoder.BeginLabel( pass.name );
	for ( std::size_t t = 0; t < compiled.transitions.size(); ++t )
		RecordTransition( encoder, graph, plan, compiled.transitions[t], plan.before[index][t] );
	RecordContext context( encoder, plan.textures, plan.buffers );
	pass.execute( context );
	encoder.EndLabel();
}

void RecordFinal( device::CommandEncoder &encoder, const CompiledGraph &graph, const Plan &plan )
{
	for ( std::size_t t = 0; t < graph.finalTransitions.size(); ++t )
		RecordTransition( encoder, graph, plan, graph.finalTransitions[t], plan.finalBefore[t] );
}

void Finish( const CompiledGraph &graph, Plan &plan, device::IRenderDevice2 &device,
    TransientPool *pool, const device::CompletionToken *token )
{
	std::vector<TransientPool::Entry> keep;
	for ( std::size_t p = 0; p < plan.physical.size(); ++p )
	{
		TransientPool::Entry &entry = plan.physical[p];
		if ( !entry.texture.IsValid() && !entry.buffer.IsValid() )
			continue;
		if ( token )
		{
			entry.usage = plan.exitUsage.empty() ? entry.usage : plan.exitUsage[p];
			entry.token = *token;
		}
		if ( pool && ( token || plan.fromPool[p] ) )
			keep.push_back( std::move( entry ) );
		else
			Release( device, entry, token ? *token : device::CompletionToken{} );
	}
	(void)graph;
	if ( pool )
		pool->Return( std::move( keep ) );
}

} // namespace detail
} // namespace render::graph
