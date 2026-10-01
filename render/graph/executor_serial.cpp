//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 serial executor (RFC 0016); see executor.h.
//
//=============================================================================//

#include "execution_plan.h"

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
