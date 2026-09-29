//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.output (RFC 0016, render.output.v1); see output.h.
//
//=============================================================================//

#include "render/pass/output/output.h"

#include "spv/output_spv.h"
#include "render/graph/executor.h"

#include <cmath>

namespace render::pass::output
{

namespace
{

using namespace render::device;

// The fragment stage's draw constants (32 bytes, D16).
struct Constants
{
	float params[4] = {};        // exposure, scene peak, headroom, 0
	std::uint32_t modes[4] = {}; // encoding, tone map (1) or a debug view (0)
};
static_assert( sizeof( Constants ) == 32 );

bool ValidParams( const OutputParams &params, OutputEncoding encoding )
{
	if ( !std::isfinite( params.exposure ) || params.exposure < 0.0f )
		return false;
	if ( !std::isfinite( params.scenePeak ) || params.scenePeak <= 0.0f ||
	     params.scenePeak > kMaxScenePeak )
		return false;
	if ( !std::isfinite( params.headroom ) || params.headroom < 1.0f )
		return false;
	// An 8-bit target shows the standard range.
	return encoding == OutputEncoding::kLinear || params.headroom == 1.0f;
}

} // namespace

foundation::Expected<OutputEncoding, OutputStatus> EncodingFor( Format target )
{
	switch ( target )
	{
	case Format::kRGBA8Unorm:
	case Format::kBGRA8Unorm:
		return OutputEncoding::kSrgb;
	case Format::kRGBA8Srgb:
	case Format::kBGRA8Srgb:
		return OutputEncoding::kHardware;
	case Format::kRGBA16Float:
		return OutputEncoding::kLinear;
	default:
		return foundation::MakeUnexpected( OutputStatus::kInvalidTarget );
	}
}

foundation::Expected<std::unique_ptr<OutputRenderer>, OutputStatus> OutputRenderer::Create(
    IRenderDevice2 &device, Format targetFormat )
{
	return CreateWithFragment( device, targetFormat, spirv::kOutputFragment );
}

foundation::Expected<std::unique_ptr<OutputRenderer>, OutputStatus>
OutputRenderer::CreateWithFragment(
    IRenderDevice2 &device, Format targetFormat, std::span<const std::uint32_t> fragmentSpirv )
{
	auto encoding = EncodingFor( targetFormat );
	if ( !encoding )
		return foundation::MakeUnexpected( encoding.Error() );
	std::unique_ptr<OutputRenderer> renderer( new OutputRenderer( device ) );
	renderer->m_TargetFormat = targetFormat;
	renderer->m_Encoding = encoding.Value();

	const BindingDesc draw[] = { { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !layout )
		return foundation::MakeUnexpected( OutputStatus::kDevice );
	renderer->m_Layout = layout.Value();
	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter = Filter::kNearest;
	samplerDesc.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( samplerDesc );
	if ( !sampler )
		return foundation::MakeUnexpected( OutputStatus::kDevice );
	renderer->m_Sampler = sampler.Value();

	const ReflectedBinding fragmentBindings[] = {
	    { 3, 0, BindingKind::kSampledTexture }, { 3, 1, BindingKind::kSampler } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kOutputVertex ) ), "main", {}, 0 },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv, std::as_bytes( fragmentSpirv ), "main",
	        fragmentBindings, sizeof( Constants ) } };
	const BindGroupLayoutId layouts[] = {
	    BindGroupLayoutId(), BindGroupLayoutId(), BindGroupLayoutId(), renderer->m_Layout };
	const Format colors[] = { targetFormat };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.drawConstantBytes = sizeof( Constants );
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = CullMode::kNone;
	desc.colorFormats = colors;
	desc.debugName = "render.pass.output";
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( OutputStatus::kDevice );
	renderer->m_Pipeline = pipeline.Value();
	return renderer;
}

OutputRenderer::~OutputRenderer()
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, m_LastToken );
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Sampler.IsValid() )
		(void)m_Device.Release( m_Sampler, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

foundation::Expected<void, OutputStatus> OutputRenderer::AddPass(
    graph::GraphBuilder &builder, const OutputTargets &targets, const OutputParams &params )
{
	if ( !ValidParams( params, m_Encoding ) )
		return foundation::MakeUnexpected( OutputStatus::kInvalidParams );
	const auto &resources = builder.Resources();
	auto extent = [&]( graph::ResourceRef ref ) -> const TextureDesc *
	{
		if ( !ref.IsValid() || ref.index >= resources.size() || !resources[ref.index].isTexture )
			return nullptr;
		return &resources[ref.index].texture;
	};
	const TextureDesc *scene = extent( targets.scene );
	const TextureDesc *target = extent( targets.target );
	if ( !scene || !target || target->format != m_TargetFormat )
		return foundation::MakeUnexpected( OutputStatus::kInvalidTarget );
	if ( scene->width != targets.width || scene->height != targets.height ||
	     target->width != targets.width || target->height != targets.height || targets.width == 0 ||
	     targets.height == 0 )
		return foundation::MakeUnexpected( OutputStatus::kSizeMismatch );

	Constants constants;
	constants.params[0] = params.exposure;
	constants.params[1] = params.scenePeak;
	constants.params[2] = params.headroom;
	constants.modes[0] = static_cast<std::uint32_t>( m_Encoding );
	constants.modes[1] = params.toneMap ? 1u : 0u;

	graph::PassBuilder pass = builder.AddPass( "output", graph::PassKind::kRender );
	pass.Read( targets.scene, ResourceUsage::kSampled )
	    .Write( targets.target, ResourceUsage::kColorAttachment );
	pass.Execute(
	    [this, targets, constants]( graph::RecordContext &context )
	    {
		    const BindGroupEntry entries[] = {
		        { 0, {}, 0, 0, context.Texture( targets.scene ), {} },
		        { 1, {}, 0, 0, {}, m_Sampler } };
		    auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
		    {
			    std::lock_guard<std::mutex> lock( m_PendingLock );
			    if ( !group )
			    {
				    ++m_RecordFailures;
				    return;
			    }
			    m_Pending.push_back( group.Value() );
		    }
		    CommandEncoder &encoder = context.Encoder();
		    ColorAttachment color;
		    color.texture = context.Texture( targets.target );
		    color.load = LoadOp::kDiscard;
		    const ColorAttachment colors[] = { color };
		    RenderingDesc rendering;
		    rendering.colors = colors;
		    rendering.width = targets.width;
		    rendering.height = targets.height;
		    encoder.BeginRendering( rendering );
		    encoder.SetViewport(
		        { 0.0f, 0.0f, float( targets.width ), float( targets.height ), 0.0f, 1.0f } );
		    encoder.SetPipeline( m_Pipeline );
		    encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
		    encoder.SetDrawConstants(
		        0, std::as_bytes( std::span<const Constants>( &constants, 1 ) ) );
		    encoder.Draw( 3 );
		    encoder.EndRendering();
	    } );
	return {};
}

void OutputRenderer::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

std::uint32_t OutputRenderer::RecordFailures() const
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	return m_RecordFailures;
}

} // namespace render::pass::output
