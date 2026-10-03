//========= Copyright Valve Corporation, All rights reserved. ============//
#include "render/pass/temporal/input_copy.h"
#include "render/shaderlib/core_artifacts.h"
#include <cstdio>
namespace render::pass::temporal
{
using namespace device;
bool InputCopy::Initialize()
{
	if ( m_Pipeline.IsValid() )
		return true;
	const BindingDesc bindings[] = {
	    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 3, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	if ( !m_Layout.IsValid() )
	{
		auto layout = m_Device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
		if ( !layout )
			return false;
		m_Layout = layout.Value();
	}
	if ( !m_Sampler.IsValid() )
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = desc.mipFilter = Filter::kNearest;
		desc.address = AddressMode::kClampToEdge;
		auto sampler = m_Device.CreateSampler( desc );
		if ( !sampler )
			return false;
		m_Sampler = sampler.Value();
	}
	auto recipe = shaderlib::CoreRecipe(
	    { "render/pass/temporal/input_copy.vert", "render/pass/temporal/input_copy.frag" } );
	recipe.layouts = { BindGroupLayoutId(), BindGroupLayoutId(), BindGroupLayoutId(), m_Layout };
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	recipe.colorFormats = { Format::kRGBA16Float, Format::kRG16Float };
	recipe.debugName = "temporal inputs";
	auto resolved =
	    shaderlib::Resolve( recipe, shaderlib::CoreArtifacts(), m_Device.Facts().artifactFormat );
	if ( !resolved )
	{
		std::fprintf( stderr, "temporal inputs: shader artifacts unavailable (status %u)\n",
		    unsigned( resolved.Error().status ) );
		return false;
	}
	auto desc = resolved.Value().Desc();
	desc.drawConstantBytes = 16;
	desc.depthFormat = Format::kD32Float;
	desc.depthStencil.depthTest = true;
	desc.depthStencil.depthWrite = true;
	desc.depthStencil.compare = CompareOp::kAlways;
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
	{
		std::fprintf( stderr, "temporal inputs: pipeline refused (status %u native %d)\n",
		    unsigned( pipeline.Error().status ), int( pipeline.Error().nativeCode ) );
		return false;
	}
	m_Pipeline = pipeline.Value();
	return true;
}
InputCopy::~InputCopy()
{
	for ( auto group : m_Pending )
		(void)m_Device.Release( group, m_Last );
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_Last );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_Last );
	if ( m_Sampler.IsValid() )
		(void)m_Device.Release( m_Sampler, m_Last );
}
bool InputCopy::Record( CommandEncoder &encoder, const TemporalImages &source,
    const TemporalImages &destination, TemporalExtent extent, int x, int y, bool decodeSrgb,
    CompletionToken submitted, ResourceUsage sourceDepthUsage )
{
	if ( !extent.width || !extent.height || !Initialize() )
		return false;
	for ( auto group : m_Pending )
		(void)m_Device.Release( group, submitted );
	m_Pending.clear();
	m_Last = submitted;
	const BindGroupEntry entries[] = { { 0, {}, 0, 0, source.color, {} },
	    { 1, {}, 0, 0, source.depth, {} }, { 2, {}, 0, 0, source.motion, {} },
	    { 3, {}, 0, 0, {}, m_Sampler } };
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
		return false;
	m_Pending.push_back( group.Value() );
	encoder.TransitionTexture(
	    source.color, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	encoder.TransitionTexture( source.depth, sourceDepthUsage, ResourceUsage::kSampled );
	encoder.TransitionTexture(
	    source.motion, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	encoder.TransitionTexture(
	    destination.color, ResourceUsage::kSampled, ResourceUsage::kColorAttachment );
	encoder.TransitionTexture(
	    destination.depth, ResourceUsage::kSampled, ResourceUsage::kDepthWrite );
	encoder.TransitionTexture(
	    destination.motion, ResourceUsage::kSampled, ResourceUsage::kColorAttachment );
	const ColorAttachment colors[] = {
	    { destination.color, LoadOp::kDiscard, StoreOp::kStore, {}, {} },
	    { destination.motion, LoadOp::kDiscard, StoreOp::kStore, {}, {} } };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.depth = DepthAttachment{ destination.depth, LoadOp::kDiscard, StoreOp::kStore, 1.0f };
	rendering.width = extent.width;
	rendering.height = extent.height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport( { 0, 0, float( extent.width ), float( extent.height ), 0, 1 } );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	const int constants[] = { x, y, decodeSrgb ? 1 : 0, 0 };
	encoder.SetDrawConstants( 0, std::as_bytes( std::span( constants ) ) );
	encoder.Draw( 3 );
	encoder.EndRendering();
	encoder.TransitionTexture(
	    destination.color, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	encoder.TransitionTexture(
	    destination.depth, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
	encoder.TransitionTexture(
	    destination.motion, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	encoder.TransitionTexture(
	    source.color, ResourceUsage::kSampled, ResourceUsage::kColorAttachment );
	encoder.TransitionTexture( source.depth, ResourceUsage::kSampled, sourceDepthUsage );
	encoder.TransitionTexture(
	    source.motion, ResourceUsage::kSampled, ResourceUsage::kColorAttachment );
	return true;
}
}
