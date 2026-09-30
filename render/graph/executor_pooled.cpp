//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 pooled executor (RFC 0016); see executor.h.
//			Encoders are begun and submitted on the calling thread; only the
//			recording runs as jobs, one encoder per job (port clause D14).
//
//=============================================================================//

#include "execution_plan.h"

#include "jobsystem/job_graph.h"

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
	for ( std::size_t i = 0; i <= graph.order.size(); ++i )
	{
		auto encoder = device.BeginEncoder( device::QueueKind::kGraphics );
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
	auto token = device.Submit( device::QueueKind::kGraphics, encoders, waits );
	if ( !token )
	{
		detail::Finish( graph, plan.Value(), device, m_Pool, nullptr );
		return foundation::MakeUnexpected( token.Error() );
	}
	ExecuteResult result;
	result.token = token.Value();
	result.passes = static_cast<std::uint32_t>( graph.order.size() );
	result.transitions = detail::TransitionCount( graph );
	result.transients = shared.created;
	result.reused = shared.reused;
	result.encoders = static_cast<std::uint32_t>( encoders.size() );
	detail::Finish( graph, plan.Value(), device, m_Pool, &result.token );
	return result;
}

} // namespace render::graph
