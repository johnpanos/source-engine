//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's frame output; see core_output.h.
//
//=============================================================================//

#include "core_output.h"

namespace render::composition
{

bool CoreOutput::Record( device::CommandEncoder &encoder, const legacy::CoreOutputTargets &targets )
{
	if ( !targets.device || !targets.scene.IsValid() || !targets.target.IsValid() )
		return false;
	if ( m_Device != targets.device )
	{
		m_Renderers.clear();
		m_Device = targets.device;
	}
	std::unique_ptr<pass::output::OutputRenderer> &renderer = m_Renderers[targets.targetFormat];
	if ( !renderer )
	{
		auto created = pass::output::OutputRenderer::Create( *m_Device, targets.targetFormat );
		if ( !created )
		{
			m_Renderers.erase( targets.targetFormat );
			++m_Failures;
			return false;
		}
		renderer = std::move( created ).Value();
	}
	// Earlier frames' bind groups go behind every submission before this one.
	renderer->Collect( targets.submitted );
	pass::output::OutputDirectTargets direct;
	direct.scene = targets.scene;
	direct.sceneUsage = device::ResourceUsage::kColorAttachment;
	direct.sceneWidth = targets.sceneWidth;
	direct.sceneHeight = targets.sceneHeight;
	direct.target = targets.target;
	direct.targetUsage = device::ResourceUsage::kColorAttachment;
	direct.width = targets.width;
	direct.height = targets.height;
	pass::output::OutputParams params;
	params.exposure = targets.exposure;
	params.scenePeak = targets.scenePeak;
	params.headroom = targets.headroom;
	params.linearScale = targets.linearScale;
	params.toneMap = targets.toneMap;
	encoder.BeginLabel( "output (render.pass.output)" );
	const bool recorded = renderer->Record( encoder, direct, params ).HasValue();
	encoder.EndLabel();
	if ( !recorded )
		++m_Failures;
	return recorded;
}

void CoreOutput::ReleaseDevice( device::IRenderDevice2 &device )
{
	if ( m_Device == &device )
	{
		m_Renderers.clear();
		m_Device = nullptr;
	}
}

} // namespace render::composition
