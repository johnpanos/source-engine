//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The composition's in-world panels (RFC 0016 render.pass.panels):
//			IRenderCorePanels over the pass, and the router that sends each
//			forwarded slot to its owner (panel tags here, the rest to the
//			world).
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_CORE_PANELS_H
#define RENDER_COMPOSITION_CORE_PANELS_H

#include "core_luminance.h"
#include "render/composition/render_core_panels.h"
#include "render/frame/renderer.h"
#include "render/legacy/capabilities.h"
#include "render/legacy/core_backend.h"
#include "render/legacy/core_passes.h"
#include "render/pass/panels/panels.h"

#include <cstdint>
#include <mutex>
#include <string>

namespace render::composition
{

class CorePanels final : public IRenderCorePanels
{
public:
	// compute: whether the device has compute, which the pass needs for each
	// panel image's mip chain; without it every panel is refused (the
	// caller draws it) rather than taken and failed (RFC 0026: the PICA200).
	CorePanels(
	    legacy::ILegacyFrontend &frontend, const frame::IRenderer &renderer, bool compute )
	    : m_Frontend( frontend ), m_Renderer( renderer ), m_Compute( compute )
	{
	}

	void BindHost( const legacy::RenderCallQueueHost *host ) { m_Host = host; }

	// IRenderCorePanels (the engine, main thread).
	bool DrawPanel( const RenderCorePanel &panel, const float worldToClip[16],
	    const float viewport[6], unsigned long long hostFrame ) override;
	void RemovePanel( unsigned long long id ) override { m_Pass.Remove( id ); }
	unsigned long long Failures() const override { return m_Pass.Failures(); }
	void GetStats( RenderCorePanelStats *out ) const override;

	// The render sequence: a panel tag's slot.
	void RecordSlot(
	    std::uint32_t tag, device::CommandEncoder &encoder, const legacy::CorePassTarget &target );
	void ReleaseDevice( device::IRenderDevice2 &device ) { m_Pass.ReleaseDevice( device ); }

private:
	legacy::ILegacyFrontend &m_Frontend;
	const frame::IRenderer &m_Renderer;
	bool m_Compute = true;
	const legacy::RenderCallQueueHost *m_Host = nullptr;
	pass::panels::PanelPass m_Pass;
	mutable std::mutex m_Lock; // m_LastRefusal
	std::string m_LastRefusal;
};

// The frontend's forwarded recorder: panel tags to the panels, luminance tags
// to the luminance counts, every other forwarded slot (and the output) to the
// world, which owns the rest.
class ForwardedSlots final : public legacy::ICorePassRecorder
{
public:
	ForwardedSlots( legacy::ICorePassRecorder &world, CorePanels &panels, CoreLuminance &luminance )
	    : m_World( world ), m_Panels( panels ), m_Luminance( luminance )
	{
	}

	std::uint32_t SlotStages() const override { return m_World.SlotStages(); }
	bool AcceptsMeshes() const override { return m_World.AcceptsMeshes(); }
	std::uint32_t QueueMesh( const legacy::CoreMeshDraw &draw ) override
	{
		return m_World.QueueMesh( draw );
	}

	void RecordSlot( std::uint32_t tag, device::CommandEncoder &encoder,
	    const legacy::CorePassTarget &target ) override
	{
		if ( pass::panels::IsPanelTag( tag ) )
			m_Panels.RecordSlot( tag, encoder, target );
		else if ( pass::luminance::IsLuminanceTag( tag ) )
			m_Luminance.RecordSlot( tag, encoder, target );
		else
			m_World.RecordSlot( tag, encoder, target );
	}
	std::size_t RecordOpaqueBatch( std::span<const std::uint32_t> tags,
	    device::CommandEncoder &encoder, const legacy::CorePassTarget &target ) override
	{
		std::size_t count = 0;
		while ( count < tags.size() && !pass::panels::IsPanelTag( tags[count] ) &&
		        !pass::luminance::IsLuminanceTag( tags[count] ) )
			++count;
		return count ? m_World.RecordOpaqueBatch( tags.first( count ), encoder, target )
		             : ICorePassRecorder::RecordOpaqueBatch( tags, encoder, target );
	}
	bool RecordOutput(
	    device::CommandEncoder &encoder, const legacy::CoreOutputTargets &targets ) override
	{
		return m_World.RecordOutput( encoder, targets );
	}
	void FrameSubmitted( device::CompletionToken token, bool submitted ) override
	{
		m_Luminance.FrameSubmitted( token, submitted );
		m_World.FrameSubmitted( token, submitted );
	}
	void ReleaseDevice( device::IRenderDevice2 &device ) override
	{
		m_Luminance.ReleaseDevice( device );
		m_Panels.ReleaseDevice( device );
		m_World.ReleaseDevice( device );
	}

private:
	legacy::ICorePassRecorder &m_World;
	CorePanels &m_Panels;
	CoreLuminance &m_Luminance;
};

} // namespace render::composition

#endif // RENDER_COMPOSITION_CORE_PANELS_H
