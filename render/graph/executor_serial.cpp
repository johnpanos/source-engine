//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 serial executor (RFC 0016); see executor.h.
//
//=============================================================================//

#include "execution_plan.h"

namespace render::graph
{

foundation::Expected<ExecuteResult, device::DeviceError> SerialGraphExecutor::Execute(
    const CompiledGraph &graph, device::IRenderDevice2 &device, const device::SubmitWaits &waits )
{
	auto plan = detail::Prepare( graph, device, m_Pool );
	if ( !plan )
		return foundation::MakeUnexpected( plan.Error() );
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

} // namespace render::graph
