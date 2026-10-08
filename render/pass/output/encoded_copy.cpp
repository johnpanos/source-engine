//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.output's encoded copy (encoded_copy.h).
//
//===========================================================================//

#include "render/pass/output/encoded_copy.h"

#include "render/shaderlib/core_artifacts.h"

namespace render::pass::output
{

using namespace render::device;

namespace
{

constexpr const char *kVertex = "render/pass/output/output.vert";
constexpr const char *kFragment = "render/pass/output/encoded_copy.frag";

} // namespace

std::unique_ptr<EncodedCopy> EncodedCopy::Create( IRenderDevice2 &device )
{
	return CreateWithFragment( device, {} );
}

std::unique_ptr<EncodedCopy> EncodedCopy::CreateWithFragment(
    IRenderDevice2 &device, std::span<const std::uint32_t> fragmentSpirv )
{
	std::unique_ptr<EncodedCopy> copy( new EncodedCopy( device ) );
	copy->m_Fragment.assign( fragmentSpirv.begin(), fragmentSpirv.end() );
	const BindingDesc draw[] = { { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !layout )
		return nullptr;
	copy->m_Layout = layout.Value();
	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter = Filter::kNearest;
	samplerDesc.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( samplerDesc );
	if ( !sampler )
		return nullptr;
	copy->m_Sampler = sampler.Value();
	return copy;
}

EncodedCopy::~EncodedCopy()
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, m_LastToken );
	for ( const Pipeline &p : m_Pipelines )
		(void)m_Device.Release( p.pipeline, m_LastToken );
	if ( m_Sampler.IsValid() )
		(void)m_Device.Release( m_Sampler, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

PipelineId EncodedCopy::PipelineFor( Format format )
{
	for ( const Pipeline &p : m_Pipelines )
		if ( p.format == format )
			return p.pipeline;
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kVertex, kFragment } );
	recipe.layouts = { BindGroupLayoutId(), BindGroupLayoutId(), BindGroupLayoutId(), m_Layout };
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	recipe.colorFormats.assign( 1, format );
	recipe.debugName = "render.pass.output.encoded-copy";
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !m_Fragment.empty() &&
	     !artifacts.ReplaceSpirv( kFragment, m_Fragment, m_Device.Facts().artifactFormat ) )
		return {};
	auto resolved = shaderlib::Resolve( recipe, artifacts, m_Device.Facts().artifactFormat );
	if ( !resolved )
		return {};
	auto pipeline = m_Device.CreatePipeline( resolved.Value().Desc() );
	if ( !pipeline )
		return {};
	m_Pipelines.push_back( { format, pipeline.Value() } );
	return pipeline.Value();
}

bool EncodedCopy::Record( CommandEncoder &encoder, const EncodedCopyTargets &targets )
{
	if ( !targets.source.IsValid() || !targets.target.IsValid() || targets.width == 0 ||
	     targets.height == 0 || targets.targetFormat == Format::kUnknown )
		return false;
	const PipelineId pipeline = PipelineFor( targets.targetFormat );
	if ( !pipeline.IsValid() )
		return false;
	const BindGroupEntry entries[] = { { 0, {}, 0, 0, targets.source, {} },
	    { 1, {}, 0, 0, {}, m_Sampler } };
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
		return false;
	// Held until Collect: the encoder that binds it is not submitted yet.
	m_Pending.push_back( group.Value() );

	if ( targets.sourceUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( targets.source, targets.sourceUsage, ResourceUsage::kSampled );
	if ( targets.targetUsage != ResourceUsage::kColorAttachment )
		encoder.TransitionTexture(
		    targets.target, targets.targetUsage, ResourceUsage::kColorAttachment );
	ColorAttachment color;
	color.texture = targets.target;
	color.load = LoadOp::kDiscard;
	const ColorAttachment colors[] = { color };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = targets.width;
	rendering.height = targets.height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport(
	    { 0.0f, 0.0f, float( targets.width ), float( targets.height ), 0.0f, 1.0f } );
	encoder.SetPipeline( pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Draw( 3 );
	encoder.EndRendering();
	if ( targets.targetUsage != ResourceUsage::kColorAttachment )
		encoder.TransitionTexture(
		    targets.target, ResourceUsage::kColorAttachment, targets.targetUsage );
	if ( targets.sourceUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( targets.source, ResourceUsage::kSampled, targets.sourceUsage );
	return true;
}

void EncodedCopy::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

} // namespace render::pass::output
