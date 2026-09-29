//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's frame output (RFC 0016 "Output",
//			render.output.v1, K12 "Game output"): the legacy backend's present
//			stage asks the frontend's recorder for the frame's output, which
//			the composition records with render.pass.output, one renderer per
//			swapchain format on the backend's device. Private to the
//			composition.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_OUTPUT_H
#define RENDER_COMPOSITION_CORE_OUTPUT_H

#include "render/legacy/core_passes.h"
#include "render/pass/output/output.h"

#include <cstdint>
#include <map>
#include <memory>

namespace render::composition
{

class CoreOutput
{
public:
	// Records the output as a section; false when nothing was recorded (an
	// invalid target, a format without an encoding, parameters the pass
	// refuses), so the backend presents as before.
	bool Record( device::CommandEncoder &encoder, const legacy::CoreOutputTargets &targets );
	void ReleaseDevice( device::IRenderDevice2 &device );
	// Frames whose output the pass refused.
	std::uint64_t Failures() const { return m_Failures; }

private:
	device::IRenderDevice2 *m_Device = nullptr;
	std::map<device::Format, std::unique_ptr<pass::output::OutputRenderer>> m_Renderers;
	std::uint64_t m_Failures = 0;
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_OUTPUT_H
