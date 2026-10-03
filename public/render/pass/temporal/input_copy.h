//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_PASS_TEMPORAL_INPUT_COPY_H
#define RENDER_PASS_TEMPORAL_INPUT_COPY_H
#include "render/device/temporal.h"
#include <vector>
namespace render::pass::temporal
{
// Extract the rendered region before reconstruction. Host images return to
// their attachment usages; exact-size linear color/depth/motion are sampled.
class InputCopy
{
public:
	explicit InputCopy( device::IRenderDevice2 &device ) : m_Device( device ) {}
	~InputCopy();
	bool Record( device::CommandEncoder &encoder, const device::TemporalImages &source,
	    const device::TemporalImages &destination, device::TemporalExtent extent, int x, int y,
	    bool decodeSrgb, device::CompletionToken submitted );

private:
	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_Layout;
	device::PipelineId m_Pipeline;
	device::SamplerId m_Sampler;
	std::vector<device::BindGroupId> m_Pending;
	device::CompletionToken m_Last;
	bool Initialize();
};
}
#endif
