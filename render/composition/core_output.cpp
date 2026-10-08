//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's frame output; see core_output.h.
//
//=============================================================================//

#include "core_output.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace render::composition
{

bool CoreOutput::Record( device::CommandEncoder &encoder, const legacy::CoreOutputTargets &targets )
{
	if ( !targets.device || !targets.scene.IsValid() || !targets.target.IsValid() )
		return false;
	if ( m_Device != targets.device )
	{
		m_Renderers.clear();
		m_Bloom.reset();
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
	// The frame's bloom: the whole legacy chain was claimed in its stream.
	FramePost post;
	{
		std::lock_guard<std::mutex> lock( m_PostLock );
		post = m_FramePost;
		m_FramePost = FramePost();
	}
	device::TextureId bloom;
	if ( post.downsample && post.blurX && post.blurY && post.add && post.enabled &&
	     targets.toneMap )
	{
		if ( !m_Bloom )
		{
			auto created = pass::post::BloomRenderer::Create( *m_Device );
			if ( created )
				m_Bloom = std::move( created ).Value();
		}
		if ( m_Bloom )
		{
			m_Bloom->Collect( targets.submitted );
			pass::post::BloomSource source;
			source.scene = targets.scene;
			source.sceneUsage = device::ResourceUsage::kColorAttachment;
			source.width = targets.sceneWidth;
			source.height = targets.sceneHeight;
			post.params.exposure = targets.exposure;
			encoder.BeginLabel( "bloom (render.pass.post)" );
			auto recorded = m_Bloom->Record( encoder, source, post.params );
			encoder.EndLabel();
			if ( recorded )
			{
				bloom = recorded.Value();
				++m_BloomFrames;
			}
		}
		if ( !bloom.IsValid() )
			++m_Failures;
	}
	pass::output::OutputDirectTargets direct;
	direct.scene = targets.scene;
	direct.sceneUsage = device::ResourceUsage::kColorAttachment;
	direct.sceneWidth = targets.sceneWidth;
	direct.sceneHeight = targets.sceneHeight;
	direct.target = targets.target;
	direct.targetUsage = device::ResourceUsage::kColorAttachment;
	direct.width = targets.width;
	direct.height = targets.height;
	direct.bloom = bloom;
	pass::output::OutputParams params;
	params.exposure = targets.exposure;
	params.scenePeak = targets.scenePeak;
	params.headroom = targets.headroom;
	params.linearScale = targets.linearScale;
	params.toneMap = targets.toneMap;
	std::copy( post.motionBlur, post.motionBlur + 4, params.motionBlur );
	params.motionBlurMax = post.motionBlurMax;
	encoder.BeginLabel( "output (render.pass.output)" );
	const bool recorded = renderer->Record( encoder, direct, params ).HasValue();
	encoder.EndLabel();
	if ( !recorded )
		++m_Failures;
	return recorded;
}

std::uint32_t CoreOutput::QueuePost( const legacy::CoreMeshDraw &draw )
{
	if ( !draw.shader || ( draw.variableCount && !draw.variables ) )
		return 0;
	std::vector<pass::post::PostDrawVariable> variables;
	variables.reserve( draw.variableCount );
	for ( std::uint32_t i = 0; i < draw.variableCount; ++i )
		if ( draw.variables[i].key && draw.variables[i].value )
			variables.push_back( { draw.variables[i].key, draw.variables[i].value } );
	auto claim = pass::post::ClaimPostDraw( draw.shader, variables );
	std::lock_guard<std::mutex> lock( m_PostLock );
	if ( !claim )
	{
		++m_PostRefused;
		if ( m_PostLastRefusal != claim.Error() )
			std::fprintf( stderr, "render.pass.post: refused %s (%s): %.*s\n",
			    draw.name ? draw.name : "?", draw.shader, int( claim.Error().size() ),
			    claim.Error().data() );
		m_PostLastRefusal = std::string( claim.Error() );
		return 0;
	}
	// A bounded queue: slots of a discarded stream never come back.
	if ( m_PostQueued.size() >= 256 )
		m_PostQueued.erase( m_PostQueued.begin() );
	const std::uint32_t tag = kPostTag | ( ++m_PostSerial & 0x00ffffffu );
	m_PostQueued[tag] = claim.Value();
	++m_PostClaimed;
	return tag;
}

void CoreOutput::RecordPost( std::uint32_t tag )
{
	std::lock_guard<std::mutex> lock( m_PostLock );
	const auto found = m_PostQueued.find( tag );
	if ( found == m_PostQueued.end() )
		return;
	const pass::post::PostClaim &claim = found->second;
	switch ( claim.role )
	{
	case pass::post::PostRole::kDownsample:
		m_FramePost.downsample = true;
		std::copy( claim.tint, claim.tint + 4, m_FramePost.params.tint );
		break;
	case pass::post::PostRole::kBlurX:
		m_FramePost.blurX = true;
		break;
	case pass::post::PostRole::kBlurY:
		m_FramePost.blurY = true;
		m_FramePost.params.bloomAmount = claim.bloomAmount;
		break;
	case pass::post::PostRole::kAdd:
		m_FramePost.add = true;
		m_FramePost.enabled = claim.bloomEnabled;
		break;
	case pass::post::PostRole::kMotionBlur:
		std::copy( claim.motionBlur, claim.motionBlur + 4, m_FramePost.motionBlur );
		m_FramePost.motionBlurMax = claim.motionBlurMax;
		break;
	}
	// Kept: a capture records the same stream again (the queue is bounded).
}

std::uint64_t CoreOutput::PostClaimed() const
{
	std::lock_guard<std::mutex> lock( m_PostLock );
	return m_PostClaimed;
}

std::uint64_t CoreOutput::PostRefused() const
{
	std::lock_guard<std::mutex> lock( m_PostLock );
	return m_PostRefused;
}

std::string CoreOutput::PostLastRefusal() const
{
	std::lock_guard<std::mutex> lock( m_PostLock );
	return m_PostLastRefusal;
}

void CoreOutput::ReleaseDevice( device::IRenderDevice2 &device )
{
	if ( m_Device == &device )
	{
		m_Bloom.reset();
		m_Renderers.clear();
		m_Device = nullptr;
	}
}

} // namespace render::composition
