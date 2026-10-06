//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 serial executor (RFC 0016); see executor.h.
//
//=============================================================================//

#include "execution_plan.h"

#include <optional>
#include <vector>

namespace render::graph
{

device::TextureId InlineGraphResources::Texture( ResourceRef ref ) const
{
	return ref.index < textures.size() ? textures[ref.index] : device::TextureId{};
}

void InlineGraphResources::Release( device::IRenderDevice2 &device, device::CompletionToken after )
{
	for ( device::TextureId texture : ownedTextures )
		(void)device.Release( texture, after );
	for ( device::BufferId buffer : ownedBuffers )
		(void)device.Release( buffer, after );
	ownedTextures.clear();
	ownedBuffers.clear();
	textures.clear();
	buffers.clear();
}

foundation::Expected<InlineGraphResources, device::DeviceError> RecordInline(
    const CompiledGraph &graph, device::IRenderDevice2 &device, device::CommandEncoder &encoder )
{
	auto prepared = detail::Prepare( graph, device, nullptr );
	if ( !prepared )
		return foundation::MakeUnexpected( prepared.Error() );
	detail::Plan &plan = prepared.Value();
	for ( std::size_t i = 0; i < graph.order.size(); ++i )
		detail::RecordPass( encoder, graph, plan, i );
	detail::RecordFinal( encoder, graph, plan );
	InlineGraphResources result;
	result.textures = std::move( plan.textures );
	result.buffers = std::move( plan.buffers );
	for ( const TransientPool::Entry &entry : plan.physical )
	{
		if ( entry.texture.IsValid() )
			result.ownedTextures.push_back( entry.texture );
		if ( entry.buffer.IsValid() )
			result.ownedBuffers.push_back( entry.buffer );
	}
	return result;
}

namespace
{

bool UsesTwoQueues( const CompiledGraph &graph )
{
	for ( const CompiledPass &pass : graph.order )
	{
		if ( pass.queue == Queue::kAsyncCompute )
			return true;
	}
	return false;
}

} // namespace

foundation::Expected<ExecuteResult, device::DeviceError> SerialGraphExecutor::Execute(
    const CompiledGraph &graph, device::IRenderDevice2 &device, const device::SubmitWaits &waits )
{
	auto plan = detail::Prepare( graph, device, m_Pool );
	if ( !plan )
		return foundation::MakeUnexpected( plan.Error() );
	if ( UsesTwoQueues( graph ) )
		return ExecuteTwoQueues( graph, device, waits, plan.Value() );
	auto encoder = device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !encoder )
	{
		detail::Finish( graph, plan.Value(), device, m_Pool, nullptr );
		return foundation::MakeUnexpected( encoder.Error() );
	}
	device::CommandEncoder &e = encoder.Value();
	e.SetLabelObserver( m_Observer );
	for ( std::size_t i = 0; i < graph.order.size(); ++i )
		detail::RecordPass( e, graph, plan.Value(), i );
	detail::RecordFinal( e, graph, plan.Value() );
	auto token = device.Submit( device::QueueKind::kGraphics, { &e, 1 }, waits );
	if ( !token )
	{
		detail::Finish( graph, plan.Value(), device, m_Pool, nullptr );
		return foundation::MakeUnexpected( token.Error() );
	}
	ExecuteResult result;
	result.token = token.Value();
	result.passes = static_cast<std::uint32_t>( graph.order.size() );
	result.transitions = detail::TransitionCount( graph );
	result.transients = plan.Value().created;
	result.reused = plan.Value().reused;
	result.encoders = 1;
	detail::Finish( graph, plan.Value(), device, m_Pool, &result.token );
	return result;
}

foundation::Expected<ExecuteResult, device::DeviceError> SerialGraphExecutor::ExecuteTwoQueues(
    const CompiledGraph &graph, device::IRenderDevice2 &device, const device::SubmitWaits &waits,
    detail::Plan &plan )
{
	using device::CompletionToken;
	using device::QueueKind;
	const std::size_t count = graph.order.size();
	std::vector<CompletionToken> tokenOf( count );
	std::optional<CompletionToken> lastCompute;
	std::optional<CompletionToken> last;
	bool firstOn[2] = { true, true };
	ExecuteResult result;
	// Releases the transients behind the latest submission, which a failed
	// device makes moot (a lost device frees everything).
	auto fail = [&]( const device::DeviceError &error )
	    -> foundation::Expected<ExecuteResult, device::DeviceError>
	{
		detail::Finish( graph, plan, device, m_Pool, last ? &*last : nullptr );
		return foundation::MakeUnexpected( error );
	};
	std::size_t begin = 0;
	while ( begin < count )
	{
		const Queue queue = graph.order[begin].queue;
		std::size_t end = begin;
		while ( end < count && graph.order[end].queue == queue )
			++end;
		const bool compute = queue == Queue::kAsyncCompute;
		const QueueKind kind = compute ? QueueKind::kCompute : QueueKind::kGraphics;
		const bool final = end == count && !compute;
		std::vector<CompletionToken> segmentWaits;
		if ( firstOn[compute ? 1 : 0] )
			segmentWaits.assign( waits.tokens.begin(), waits.tokens.end() );
		firstOn[compute ? 1 : 0] = false;
		for ( std::size_t i = begin; i < end; ++i )
		{
			if ( graph.order[i].waitFor != UINT32_MAX )
				segmentWaits.push_back( tokenOf[graph.order[i].waitFor] );
		}
		if ( final && lastCompute )
			segmentWaits.push_back( *lastCompute );
		auto encoder = device.BeginEncoder( kind );
		if ( !encoder )
			return fail( encoder.Error() );
		device::CommandEncoder &e = encoder.Value();
		e.SetLabelObserver( m_Observer );
		for ( std::size_t i = begin; i < end; ++i )
			detail::RecordPass( e, graph, plan, i );
		if ( final )
			detail::RecordFinal( e, graph, plan );
		auto token = device.Submit( kind, { &e, 1 }, { segmentWaits } );
		if ( !token )
			return fail( token.Error() );
		for ( std::size_t i = begin; i < end; ++i )
			tokenOf[i] = token.Value();
		( compute ? lastCompute : last ) = token.Value();
		++result.encoders;
		begin = end;
	}
	if ( graph.order.empty() || graph.order.back().queue == Queue::kAsyncCompute )
	{
		// The graph ends on the compute queue: a graphics submission records
		// the final transitions after it, so one token covers everything.
		auto encoder = device.BeginEncoder( QueueKind::kGraphics );
		if ( !encoder )
			return fail( encoder.Error() );
		detail::RecordFinal( encoder.Value(), graph, plan );
		std::vector<CompletionToken> join;
		if ( lastCompute )
			join.push_back( *lastCompute );
		auto token = device.Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, { join } );
		if ( !token )
			return fail( token.Error() );
		last = token.Value();
		++result.encoders;
	}
	result.token = *last;
	result.passes = static_cast<std::uint32_t>( count );
	result.transitions = detail::TransitionCount( graph );
	result.transients = plan.created;
	result.reused = plan.reused;
	detail::Finish( graph, plan, device, m_Pool, &result.token );
	return result;
}

} // namespace render::graph
