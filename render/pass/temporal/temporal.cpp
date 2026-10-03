//========= Copyright Valve Corporation, All rights reserved. ============//
#include "render/pass/temporal/temporal.h"
#include "render/graph/executor.h"
#include <cmath>
namespace render::pass::temporal
{
namespace
{
auto Invalid()
{
	return foundation::MakeUnexpected( device::DeviceError{
	    device::DeviceStatus::kInvalidDescription, device::DeviceOperation::kSubmit } );
}
bool Positive( device::TemporalExtent e )
{
	return e.width && e.height;
}
}
device::DeviceResult<device::TemporalDispatch> History::Prepare(
    const Frame &frame, std::uint32_t deviceEpoch )
{
	if ( m_Pending )
		return Invalid();
	if ( !frame.view.scene || !frame.view.generator || !frame.sequence || !deviceEpoch ||
	     !Positive( frame.render ) || !Positive( frame.output ) ||
	     frame.render.width > frame.output.width || frame.render.height > frame.output.height ||
	     !frame.motionComplete || !std::isfinite( frame.jitterX ) ||
	     !std::isfinite( frame.jitterY ) || !std::isfinite( frame.preExposure ) ||
	     frame.preExposure <= 0 || !std::isfinite( frame.deltaMilliseconds ) ||
	     frame.deltaMilliseconds <= 0 )
	{
		m_Invalid = true;
		return Invalid();
	}
	const bool sameView = m_Previous && m_Previous->view == frame.view;
	if ( sameView && m_Epoch == deviceEpoch && frame.sequence <= m_Previous->sequence )
		return Invalid();
	device::TemporalDispatch dispatch;
	dispatch.render = frame.render;
	dispatch.output = frame.output;
	dispatch.jitterX = frame.jitterX;
	dispatch.jitterY = frame.jitterY;
	dispatch.preExposure = frame.preExposure;
	dispatch.deltaMilliseconds = frame.deltaMilliseconds;
	dispatch.reset = m_Invalid || !sameView || m_Epoch != deviceEpoch || frame.discontinuity ||
	                 frame.sequence != m_Previous->sequence + 1 ||
	                 frame.render != m_Previous->render || frame.output != m_Previous->output ||
	                 frame.preExposure != m_Previous->preExposure;
	m_Pending = frame;
	m_PendingEpoch = deviceEpoch;
	return dispatch;
}
void History::Commit()
{
	if ( m_Pending )
	{
		m_Previous = m_Pending;
		m_Epoch = m_PendingEpoch;
		m_Invalid = false;
		m_Pending.reset();
	}
}
void History::Abort()
{
	m_Pending.reset();
	m_Invalid = true;
}

device::DeviceResult<void> AddPass( graph::GraphBuilder &graph, History &history,
    device::ITemporalUpscaler &provider, const Frame &frame, std::uint32_t deviceEpoch,
    const Inputs &inputs )
{
	using namespace device;
	const graph::ResourceRef refs[] = { inputs.color, inputs.depth, inputs.motion, inputs.output };
	const Format formats[] = {
	    Format::kRGBA16Float, Format::kD32Float, Format::kRG16Float, Format::kRGBA16Float };
	for ( unsigned i = 0; i < 4; ++i )
	{
		if ( refs[i].index >= graph.Resources().size() )
			return Invalid();
		for ( unsigned j = 0; j < i; ++j )
			if ( refs[i] == refs[j] )
				return Invalid();
		const auto &resource = graph.Resources()[refs[i].index];
		const auto extent = i == 3 ? frame.output : frame.render;
		if ( !resource.isTexture || resource.texture.width != extent.width ||
		     resource.texture.height != extent.height || resource.texture.sampleCount != 1 ||
		     resource.texture.format != formats[i] )
			return Invalid();
	}
	auto prepared = history.Prepare( frame, deviceEpoch );
	if ( !prepared )
		return foundation::MakeUnexpected( prepared.Error() );
	graph.AddPass( "temporal-upscale", graph::PassKind::kCompute )
	    .Read( inputs.color, ResourceUsage::kSampled )
	    .Read( inputs.depth, ResourceUsage::kSampled )
	    .Read( inputs.motion, ResourceUsage::kSampled )
	    .Write( inputs.output, ResourceUsage::kStorageWrite )
	    .SideEffect()
	    .Execute(
	        [dispatch = prepared.Value(), inputs, &provider](
	            graph::RecordContext &context ) mutable
	        {
		        dispatch.images = { context.Texture( inputs.color ),
		            context.Texture( inputs.depth ), context.Texture( inputs.motion ),
		            context.Texture( inputs.output ) };
		        // The provider invalidates the encoder on failure, so execution
		        // cannot publish a successful output or commit history.
		        if ( !provider.Record( context.Encoder(), dispatch ) )
			        (void)context.Encoder().TakeBackend();
	        } );
	return {};
}
}
