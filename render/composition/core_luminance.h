//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The composition's luminance counts (RFC 0016 render.pass.luminance,
//			K8 post cohort): IRenderCoreLuminance over the pass, each query at
//			its slot of the frame's stream and read back when the native
//			host's submission token completes.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_LUMINANCE_H
#define RENDER_COMPOSITION_CORE_LUMINANCE_H

#include "render/composition/render_core_luminance.h"
#include "render/legacy/capabilities.h"
#include "render/legacy/core_backend.h"
#include "render/legacy/core_passes.h"
#include "render/pass/luminance/luminance.h"

#include <cstdint>

namespace render::composition
{

class CoreLuminance final : public IRenderCoreLuminance
{
public:
	explicit CoreLuminance( legacy::ILegacyFrontend &frontend ) : m_Frontend( frontend ) {}

	void BindHost( const legacy::RenderCallQueueHost *host ) { m_Host = host; }

	// IRenderCoreLuminance (the engine, main thread).
	unsigned Queue( ITexture *texture, int x0, int y0, int x1, int y1, float minimum, float maximum,
	    float scale ) override;
	int Result( unsigned id ) override;
	void GetStats( RenderCoreLuminanceStats *out ) const override;

	// The render sequence.
	void RecordSlot(
	    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target );
	void FrameSubmitted( device::CompletionToken token, bool submitted );
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	legacy::ILegacyFrontend &m_Frontend;
	const legacy::RenderCallQueueHost *m_Host = nullptr;
	pass::luminance::LuminanceCounter m_Counter;
	device::IRenderDevice2 *m_Device = nullptr; // render sequence: the slots' device
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_LUMINANCE_H
