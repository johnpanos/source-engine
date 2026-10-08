//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): screen-space reflections at a slot.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

bool CoreWorld::EnsureSsr( device::IRenderDevice2 &device, std::uint32_t width,
    std::uint32_t height, device::CompletionToken submitted )
{
	using namespace render::device;
	BindStageDevice( device );
	if ( !m_Ssr )
	{
		auto created = pass::ssr::ScreenSpaceReflections::Create( device, {} );
		if ( !created )
			return false;
		m_Ssr = std::move( created ).Value();
	}
	if ( !m_SsrCopy )
	{
		auto created = pass::output::OutputRenderer::Create( device, Format::kRGBA16Float );
		if ( !created )
			return false;
		m_SsrCopy = std::move( created ).Value();
	}
	if ( m_SsrOutput.IsValid() && m_SsrWidth == width && m_SsrHeight == height )
		return true;
	// A resize: the old targets go behind the frames that used them.
	ReleaseSsr( device, submitted );
	auto make = [&]( std::initializer_list<ResourceUsage> usages, const char *name )
	{
		TextureDesc desc;
		desc.format = Format::kRGBA16Float;
		desc.width = width;
		desc.height = height;
		desc.usages = UsageSet( usages );
		desc.debugName = name;
		auto made = device.CreateTexture( desc );
		return made ? made.Value() : TextureId();
	};
	m_SsrTargets[0] = make( { ResourceUsage::kColorAttachment, ResourceUsage::kSampled },
	    "stage ssr normal roughness" );
	m_SsrTargets[1] =
	    make( { ResourceUsage::kColorAttachment, ResourceUsage::kSampled }, "stage ssr ibl" );
	m_SsrTargets[2] =
	    make( { ResourceUsage::kColorAttachment, ResourceUsage::kSampled }, "stage ssr weight" );
	m_SsrOutput =
	    make( { ResourceUsage::kStorageWrite, ResourceUsage::kSampled }, "stage ssr output" );
	m_SsrWidth = width;
	m_SsrHeight = height;
	m_SsrFresh = true;
	return m_SsrTargets[0].IsValid() && m_SsrTargets[1].IsValid() && m_SsrTargets[2].IsValid() &&
	       m_SsrOutput.IsValid();
}

void CoreWorld::ReleaseSsr( device::IRenderDevice2 &device, device::CompletionToken token )
{
	for ( device::TextureId &texture : m_SsrTargets )
	{
		if ( texture.IsValid() )
			(void)device.Release( texture, token );
		texture = device::TextureId();
	}
	if ( m_SsrOutput.IsValid() )
		(void)device.Release( m_SsrOutput, token );
	m_SsrOutput = device::TextureId();
	m_SsrWidth = m_SsrHeight = 0;
}

void CoreWorld::RecordSsr(
    device::CommandEncoder &encoder, const legacy::CorePassTarget &target, const ShadowWork &work )
{
	using namespace render::device;
	auto refuse = [&]( const char *why )
	{
		if ( m_SsrRefused.fetch_add( 1, std::memory_order_relaxed ) == 0 )
			std::fprintf( stderr, "render core: screen-space reflections refused: %s\n", why );
	};
	// The pass reads its inputs in kSampled; the frame's own depth and color
	// rest elsewhere between slots.
	encoder.TransitionTexture( target.depth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
	encoder.TransitionTexture(
	    target.color, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	pass::ssr::SsrDirectTargets targets;
	targets.depth = target.depth;
	targets.normalRoughness = m_SsrTargets[0];
	targets.iblRadiance = m_SsrTargets[1];
	targets.specularWeight = m_SsrTargets[2];
	targets.lit = target.color;
	targets.output = m_SsrOutput;
	targets.outputUsage = ResourceUsage::kStorageWrite;
	targets.width = target.width;
	targets.height = target.height;
	pass::ssr::SsrView view;
	// The projection the world pass rasterized with (its half-pixel shift).
	view.toClip = math::Multiply( work.projection, work.view );
	std::copy( work.eye, work.eye + 3, view.eye );
	bool traced = false;
	if ( !m_Ssr->Record( encoder, targets, view ) )
		refuse( "the pass did not record" );
	else
	{
		// The output replaces the frame (encoding alone: linear into the
		// float frame).
		encoder.TransitionTexture(
		    m_SsrOutput, ResourceUsage::kStorageWrite, ResourceUsage::kSampled );
		pass::output::OutputDirectTargets copy;
		copy.scene = m_SsrOutput;
		copy.sceneUsage = ResourceUsage::kSampled;
		copy.sceneWidth = target.width;
		copy.sceneHeight = target.height;
		copy.target = target.color;
		copy.targetUsage = ResourceUsage::kSampled;
		copy.width = target.width;
		copy.height = target.height;
		pass::output::OutputParams params;
		params.toneMap = false;
		traced = bool( m_SsrCopy->Record( encoder, copy, params ) );
		if ( !traced )
			refuse( "the copy back into the frame did not record" );
		encoder.TransitionTexture(
		    m_SsrOutput, ResourceUsage::kSampled, ResourceUsage::kStorageWrite );
	}
	encoder.TransitionTexture(
	    target.color, ResourceUsage::kSampled, ResourceUsage::kColorAttachment );
	encoder.TransitionTexture( target.depth, ResourceUsage::kSampled, ResourceUsage::kDepthWrite );
	if ( traced )
		m_SsrViews.fetch_add( 1, std::memory_order_relaxed );
}

} // namespace render::composition
