//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.legacy-frontend's depth-alpha copy (depth_alpha.h).
//
//===========================================================================//

#include "render/legacy/depth_alpha.h"

#include "render/shaderlib/core_artifacts.h"

#include <cmath>

namespace render::legacy
{

using namespace render::device;

namespace
{

struct Constants
{
	float projection[4];
	float range[4];
};

constexpr const char *kVertex = "render/pass/output/output.vert";
constexpr const char *kFragment = "render/legacy/depth_alpha.frag";

} // namespace

std::unique_ptr<DepthAlphaPass> DepthAlphaPass::Create( IRenderDevice2 &device )
{
	return CreateWithFragment( device, {} );
}

std::unique_ptr<DepthAlphaPass> DepthAlphaPass::CreateWithFragment(
    IRenderDevice2 &device, std::span<const std::uint32_t> fragmentSpirv )
{
	std::unique_ptr<DepthAlphaPass> pass( new DepthAlphaPass( device ) );
	pass->m_Fragment.assign( fragmentSpirv.begin(), fragmentSpirv.end() );
	const BindingDesc draw[] = { { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !layout )
		return nullptr;
	pass->m_Layout = layout.Value();
	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter = Filter::kNearest;
	samplerDesc.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( samplerDesc );
	if ( !sampler )
		return nullptr;
	pass->m_Sampler = sampler.Value();
	return pass;
}

DepthAlphaPass::~DepthAlphaPass()
{
	for ( const PendingGroup &pending : m_Pending )
		(void)m_Device.Release( pending.group, m_LastToken );
	for ( const Pipeline &p : m_Pipelines )
		(void)m_Device.Release( p.pipeline, m_LastToken );
	if ( m_Sampler.IsValid() )
		(void)m_Device.Release( m_Sampler, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

PipelineId DepthAlphaPass::PipelineFor( Format format )
{
	for ( const Pipeline &p : m_Pipelines )
		if ( p.format == format )
			return p.pipeline;
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kVertex, kFragment } );
	recipe.layouts = { BindGroupLayoutId(), BindGroupLayoutId(), BindGroupLayoutId(), m_Layout };
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	recipe.colorFormats.assign( 1, format );
	recipe.debugName = "render.legacy.depth-alpha";
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !m_Fragment.empty() &&
	     !artifacts.ReplaceSpirv( kFragment, m_Fragment, m_Device.Facts().artifactFormat ) )
		return {};
	auto resolved = shaderlib::Resolve( recipe, artifacts, m_Device.Facts().artifactFormat );
	if ( !resolved )
		return {};
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = sizeof( Constants );
	const std::uint8_t alphaOnly[] = { kColorWriteAlpha };
	desc.colorWriteMasks = alphaOnly;
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return {};
	m_Pipelines.push_back( { format, pipeline.Value() } );
	return pipeline.Value();
}

bool DepthAlphaPass::Record( CommandEncoder &encoder, const DepthAlphaCopy &copy )
{
	if ( !copy.target.IsValid() || !copy.depth.IsValid() || copy.width == 0 ||
	     copy.height == 0 || !std::isfinite( copy.range ) || copy.range <= 0.0f ||
	     std::uint64_t( copy.x ) + copy.width > copy.targetWidth ||
	     std::uint64_t( copy.y ) + copy.height > copy.targetHeight )
		return false;
	const PipelineId pipeline = PipelineFor( copy.targetFormat );
	if ( !pipeline.IsValid() )
		return false;
	const BindGroupEntry entries[] = { { 0, {}, 0, 0, copy.depth, {} },
	    { 1, {}, 0, 0, {}, m_Sampler } };
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
		return false;
	m_Pending.push_back( { group.Value(), copy.recording } );
	Constants constants = {};
	for ( int i = 0; i < 4; ++i )
		constants.projection[i] = copy.projection[i];
	constants.range[0] = 1.0f / copy.range;

	encoder.TransitionTexture( copy.depth, copy.depthUsage, ResourceUsage::kSampled );
	encoder.TransitionTexture(
	    copy.target, ResourceUsage::kCopyDestination, ResourceUsage::kColorAttachment );
	ColorAttachment color;
	color.texture = copy.target;
	color.load = LoadOp::kLoad;
	const ColorAttachment colors[] = { color };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = copy.targetWidth;
	rendering.height = copy.targetHeight;
	encoder.BeginRendering( rendering );
	// The copied rectangle alone: a viewport over it bounds the triangle, and
	// the fragment reads the depth texel under its own pixel.
	encoder.SetViewport( { float( copy.x ), float( copy.y ), float( copy.width ),
	    float( copy.height ), 0.0f, 1.0f } );
	encoder.SetPipeline( pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.SetDrawConstants( 0, std::as_bytes( std::span<const Constants>( &constants, 1 ) ) );
	encoder.Draw( 3 );
	encoder.EndRendering();
	encoder.TransitionTexture(
	    copy.target, ResourceUsage::kColorAttachment, ResourceUsage::kCopyDestination );
	encoder.TransitionTexture( copy.depth, ResourceUsage::kSampled, copy.depthUsage );
	return true;
}

void DepthAlphaPass::Collect( CompletionToken token, std::uint64_t recording )
{
	// Groups the open recording binds are in its unsubmitted encoder: a release
	// behind the last submission would free them before that encoder submits.
	std::size_t kept = 0;
	for ( const PendingGroup &pending : m_Pending )
	{
		if ( pending.recording == recording )
			m_Pending[kept++] = pending;
		else
			(void)m_Device.Release( pending.group, token );
	}
	m_Pending.resize( kept );
	m_LastToken = token;
}

} // namespace render::legacy
