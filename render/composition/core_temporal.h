//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_COMPOSITION_CORE_TEMPORAL_H
#define RENDER_COMPOSITION_CORE_TEMPORAL_H
#include "render/pass/temporal/input_copy.h"
#include "render/pass/output/output.h"
#include "render/legacy/core_passes.h"
namespace render::composition
{
struct TemporalRequest
{
	int x = 0, y = 0;
	device::TemporalExtent render, output;
	float jitterX = 0, jitterY = 0, deltaMilliseconds = 0;
	std::uint64_t generation = 0;
};
// Render-sequence state for the main game view; owns reconstruction images.
class CoreTemporal
{
public:
	CoreTemporal(
	    device::IRenderDevice2 &device, std::unique_ptr<device::ITemporalUpscaler> provider )
	    : m_Device( device ), m_Provider( std::move( provider ) ), m_Copy( device )
	{
	}
	~CoreTemporal();
	bool Record( device::CommandEncoder &encoder, const legacy::CorePassTarget &target,
	    device::TextureId motion, const TemporalRequest &request );

private:
	device::IRenderDevice2 &m_Device;
	std::unique_ptr<device::ITemporalUpscaler> m_Provider;
	pass::temporal::InputCopy m_Copy;
	std::unique_ptr<pass::output::OutputRenderer> m_Output;
	device::TemporalImages m_Images;
	device::TemporalExtent m_Render, m_Display;
	device::Format m_Format = device::Format::kUnknown;
	device::CompletionToken m_Last;
	std::uint64_t m_Generation = 0, m_Frame = 0;
	void ReleaseImages();
};
}
#endif
