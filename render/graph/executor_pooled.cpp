//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 pooled executor (RFC 0016); see executor.h.
//			Encoders are begun and submitted on the calling thread; only the
//			recording runs as jobs, one encoder per job (port clause D14).
//			A two-queue graph begins each pass's encoder on its own queue and
//			submits each run of same-queue passes with the compiled waits, as
//			the serial executor does (RFC 0016 S8).
//
//=============================================================================//

#include "execution_plan.h"

#include "jobsystem/job_graph.h"

#include <optional>
#include <vector>

namespace render::graph
{

foundation::Expected<ExecuteResult, device::DeviceError> PooledGraphExecutor::Execute(
    const CompiledGraph &graph, device::IRenderDevice2 &device, const device::SubmitWaits &waits )
{
	auto plan = detail::Prepare( graph, device, m_Pool );
	if ( !plan )
		return foundation::MakeUnexpected( plan.Error() );
	// One encoder per kept pass, and one for the final transitions.
	std::vector<device::CommandEncoder> encoders;
	encoders.reserve( graph.order.size() + 1 );
	// Entry graph.order.size() is the final transitions, always on graphics.
	auto queueOf = [&graph]( std::size_t i )
	{
		return i < graph.order.size() && graph.order[i].queue == Queue::kAsyncCompute
		           ? device::QueueKind::kCompute
		           : device::QueueKind::kGraphics;
	};
	for ( std::size_t i = 0; i <= graph.order.size(); ++i )
	{
		auto encoder = device.BeginEncoder( queueOf( i ) );
		if ( !encoder )
		{
			detail::Finish( graph, plan.Value(), device, m_Pool, nullptr );
			return foundation::MakeUnexpected( encoder.Error() );
		}
		encoders.push_back( std::move( encoder ).Value() );
		encoders.back().SetLabelObserver( m_Observer );
	}

	const detail::Plan &shared = plan.Value();
	jobsystem::JobGraphBuilder builder;
	for ( std::size_t i = 0; i < graph.order.size(); ++i )
	{
		jobsystem::JobDesc job;
		job.name = "render-graph-pass";
		job.function = [&graph, &shared, &encoders, i]( jobsystem::JobRunContext & )
		{
			detail::RecordPass( encoders[i], graph, shared, i );
		};
		builder.AddJob( job );
	}
	auto sealed = builder.Seal();
	bool recorded = sealed.HasValue();
	if ( recorded )
		recorded = m_Jobs.Execute( sealed.Value(), jobsystem::RunOptions() ).AllSucceeded();
	if ( !recorded )
	{
		detail::Finish( graph, plan.Value(), device, m_Pool, nullptr );
		return foundation::MakeUnexpected( device::DeviceError{
		    device::DeviceStatus::kInternal, device::DeviceOperation::kSubmit, 0 } );
	}
	detail::RecordFinal( encoders.back(), graph, shared );

	// One submission per run of consecutive same-queue encoders; a one-queue
	// graph is one graphics run, so one submission as before.
	using device::CompletionToken;
	const std::size_t total = encoders.size();
	std::vector<CompletionToken> tokenOf( total );
	std::optional<CompletionToken> lastCompute;
	std::optional<CompletionToken> last;
	std::uint32_t computeSubmissions = 0;
	bool firstOn[2] = { true, true };
	std::size_t begin = 0;
	while ( begin < total )
	{
		const device::QueueKind kind = queueOf( begin );
		const bool compute = kind == device::QueueKind::kCompute;
		std::size_t end = begin;
		while ( end < total && queueOf( end ) == kind )
			++end;
		std::vector<CompletionToken> segmentWaits;
		if ( firstOn[compute ? 1 : 0] )
			segmentWaits.assign( waits.tokens.begin(), waits.tokens.end() );
		firstOn[compute ? 1 : 0] = false;
		for ( std::size_t i = begin; i < end && i < graph.order.size(); ++i )
		{
			if ( graph.order[i].waitFor != UINT32_MAX )
				segmentWaits.push_back( tokenOf[graph.order[i].waitFor] );
		}
		if ( end == total && lastCompute )
			segmentWaits.push_back( *lastCompute );
		auto token = device.Submit( kind,
		    std::span<device::CommandEncoder>( encoders.data() + begin, end - begin ),
		    { segmentWaits } );
		if ( !token )
		{
			detail::Finish( graph, plan.Value(), device, m_Pool, last ? &*last : nullptr );
			return foundation::MakeUnexpected( token.Error() );
		}
		for ( std::size_t i = begin; i < end; ++i )
			tokenOf[i] = token.Value();
		( compute ? lastCompute : last ) = token.Value();
		computeSubmissions += compute;
		begin = end;
	}
	ExecuteResult result;
	result.token = *last;
	result.passes = static_cast<std::uint32_t>( graph.order.size() );
	result.transitions = detail::TransitionCount( graph );
	result.transients = shared.created;
	result.reused = shared.reused;
	result.encoders = static_cast<std::uint32_t>( encoders.size() );
	result.computeSubmissions = computeSubmissions;
	detail::Finish( graph, plan.Value(), device, m_Pool, &result.token );
	return result;
}

} // namespace render::graph
