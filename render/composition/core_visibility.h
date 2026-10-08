//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The composition's pixel visibility counts (RFC 0016
//			render.pass.visibility, K8 sprites cohort): IRenderCoreVisibility
//			over the pass, each proxy at its slot of the frame's stream and
//			read back when the native host's submission token completes.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_VISIBILITY_H
#define RENDER_COMPOSITION_CORE_VISIBILITY_H

#include "render/composition/render_core_visibility.h"
#include "render/legacy/core_backend.h"
#include "render/legacy/core_passes.h"
#include "render/pass/visibility/visibility.h"

#include <cstdint>

namespace render::composition
{

class CoreVisibility final : public IRenderCoreVisibility
{
public:
	explicit CoreVisibility( legacy::ILegacyFrontend &frontend ) : m_Frontend( frontend ) {}

	unsigned Queue( const float points[5][4], const float viewport[6] ) override;
	int Result( unsigned id, long long *visible, long long *possible ) override;
	void GetStats( RenderCoreVisibilityStats *out ) const override;

	void RecordSlot(
	    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target );
	void FrameSubmitted( device::CompletionToken token, bool submitted );
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	legacy::ILegacyFrontend &m_Frontend;
	pass::visibility::VisibilityCounter m_Counter;
	device::IRenderDevice2 *m_Device = nullptr; // render sequence: the slots' device
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_VISIBILITY_H
