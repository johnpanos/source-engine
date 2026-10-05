//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.output (RFC 0016, render.output.v1); see output.h.
//
//=============================================================================//

#include "render/pass/output/output.h"

#include "render/shaderlib/core_artifacts.h"
#include "render/graph/executor.h"

#include <cmath>

namespace render::pass::output
{

namespace
{

using namespace render::device;

// The fragment stage's draw constants (48 bytes, D16).
struct Constants
{
	float params[4] = {};        // exposure, scene peak, headroom, 0
	std::uint32_t modes[4] = {}; // encoding, tone map (1) or a debug view (0), scaled (1)
	float extent[4] = {};        // the target's width and height, 1 to add the bloom
};
static_assert( sizeof( Constants ) == 48 );

Constants MakeConstants( const OutputParams &params, OutputEncoding encoding, std::uint32_t width,
    std::uint32_t height, bool scaled )
{
	Constants constants;
	constants.params[0] = params.exposure;
	constants.params[1] = params.scenePeak;
	constants.params[2] = params.headroom;
	constants.params[3] = params.linearScale;
	constants.modes[0] = static_cast<std::uint32_t>( encoding );
	constants.modes[1] = params.toneMap ? 1u : 0u;
	constants.modes[2] = scaled ? 1u : 0u;
	constants.extent[0] = float( width );
	constants.extent[1] = float( height );
	return constants;
}

bool ValidParams( const OutputParams &params, OutputEncoding encoding )
{
	if ( !std::isfinite( params.linearScale ) || params.linearScale <= 0.0f )
		return false;
	if ( !std::isfinite( params.exposure ) || params.exposure < 0.0f )
		return false;
	if ( !std::isfinite( params.scenePeak ) || params.scenePeak <= 0.0f ||
	     params.scenePeak > kMaxScenePeak )
		return false;
	if ( !std::isfinite( params.headroom ) || params.headroom < 1.0f )
		return false;
	// An 8-bit target shows the standard range.
	return encoding == OutputEncoding::kLinear || encoding == OutputEncoding::kPq ||
	       params.headroom == 1.0f;
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
	case Format::kRGB10A2Unorm:
		return OutputEncoding::kPq;
	default:
		return foundation::MakeUnexpected( OutputStatus::kInvalidTarget );
	}
}

foundation::Expected<std::unique_ptr<OutputRenderer>, OutputStatus> OutputRenderer::Create(
    IRenderDevice2 &device, Format targetFormat )
{
	return CreateWithFragment( device, targetFormat, {} );
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
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 3, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
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
	samplerDesc.minFilter = samplerDesc.magFilter = Filter::kLinear;
	auto linear = device.CreateSampler( samplerDesc );
	if ( !linear )
		return foundation::MakeUnexpected( OutputStatus::kDevice );
	renderer->m_LinearSampler = linear.Value();

	// The program in the device's artifact format (RFC 0016 K10); a suite's
	// seeded fragment replaces the core one, on a SPIR-V device only.
	constexpr const char *kFragment = "render/pass/output/output.frag";
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !fragmentSpirv.empty() &&
	     !artifacts.ReplaceSpirv( kFragment, fragmentSpirv, device.Facts().artifactFormat ) )
		return foundation::MakeUnexpected( OutputStatus::kDevice );
	shaderlib::PipelineRecipe recipe =
	    shaderlib::CoreRecipe( { "render/pass/output/output.vert", kFragment } );
	recipe.layouts = {
	    BindGroupLayoutId(), BindGroupLayoutId(), BindGroupLayoutId(), renderer->m_Layout };
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	recipe.colorFormats = { targetFormat };
	recipe.debugName = "render.pass.output";
	auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected( OutputStatus::kDevice );
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = sizeof( Constants );
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
	if ( m_LinearSampler.IsValid() )
		(void)m_Device.Release( m_LinearSampler, m_LastToken );
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
	const std::uint32_t sceneWidth = targets.sceneWidth ? targets.sceneWidth : targets.width;
	const std::uint32_t sceneHeight = targets.sceneHeight ? targets.sceneHeight : targets.height;
	if ( scene->width != sceneWidth || scene->height != sceneHeight ||
	     target->width != targets.width || target->height != targets.height || targets.width == 0 ||
	     targets.height == 0 )
		return foundation::MakeUnexpected( OutputStatus::kSizeMismatch );
	const bool scaled = sceneWidth != targets.width || sceneHeight != targets.height;
	const Constants constants =
	    MakeConstants( params, m_Encoding, targets.width, targets.height, scaled );

	graph::PassBuilder pass = builder.AddPass( "output", graph::PassKind::kRender );
	pass.Read( targets.scene, ResourceUsage::kSampled )
	    .Write( targets.target, ResourceUsage::kColorAttachment );
	pass.Execute(
	    [this, targets, constants, scaled]( graph::RecordContext &context )
	    {
		    (void)RecordDraw( context.Encoder(), context.Texture( targets.scene ), TextureId(),
		        context.Texture( targets.target ), targets.width, targets.height, scaled,
		        &constants );
	    } );
	return {};
}

foundation::Expected<void, OutputStatus> OutputRenderer::Record(
    CommandEncoder &encoder, const OutputDirectTargets &targets, const OutputParams &params )
{
	if ( !ValidParams( params, m_Encoding ) )
		return foundation::MakeUnexpected( OutputStatus::kInvalidParams );
	if ( !targets.scene.IsValid() || !targets.target.IsValid() )
		return foundation::MakeUnexpected( OutputStatus::kInvalidTarget );
	if ( targets.width == 0 || targets.height == 0 || targets.sceneWidth == 0 ||
	     targets.sceneHeight == 0 )
		return foundation::MakeUnexpected( OutputStatus::kSizeMismatch );
	const bool scaled =
	    targets.sceneWidth != targets.width || targets.sceneHeight != targets.height;
	Constants constants =
	    MakeConstants( params, m_Encoding, targets.width, targets.height, scaled );
	const bool bloom = targets.bloom.IsValid() && params.toneMap;
	constants.extent[2] = bloom ? 1.0f : 0.0f;
	if ( targets.sceneUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( targets.scene, targets.sceneUsage, ResourceUsage::kSampled );
	if ( targets.targetUsage != ResourceUsage::kColorAttachment )
		encoder.TransitionTexture(
		    targets.target, targets.targetUsage, ResourceUsage::kColorAttachment );
	const bool drawn = RecordDraw( encoder, targets.scene, bloom ? targets.bloom : TextureId(),
	    targets.target, targets.width, targets.height, scaled, &constants );
	if ( targets.sceneUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( targets.scene, ResourceUsage::kSampled, targets.sceneUsage );
	if ( targets.targetUsage != ResourceUsage::kColorAttachment )
		encoder.TransitionTexture(
		    targets.target, ResourceUsage::kColorAttachment, targets.targetUsage );
	if ( !drawn )
		return foundation::MakeUnexpected( OutputStatus::kDevice );
	return {};
}

bool OutputRenderer::RecordDraw( CommandEncoder &encoder, TextureId scene, TextureId bloom,
    TextureId target, std::uint32_t width, std::uint32_t height, bool scaled,
    const void *constants )
{
	// Without a bloom the scene fills its slot; the constants leave it unread.
	const BindGroupEntry entries[] = { { 0, {}, 0, 0, scene, {} },
	    { 1, {}, 0, 0, {}, scaled ? m_LinearSampler : m_Sampler },
	    { 2, {}, 0, 0, bloom.IsValid() ? bloom : scene, {} },
	    { 3, {}, 0, 0, {}, m_LinearSampler } };
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		if ( !group )
		{
			++m_RecordFailures;
			return false;
		}
		m_Pending.push_back( group.Value() );
	}
	ColorAttachment color;
	color.texture = target;
	color.load = LoadOp::kDiscard;
	const ColorAttachment colors[] = { color };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = width;
	rendering.height = height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport( { 0.0f, 0.0f, float( width ), float( height ), 0.0f, 1.0f } );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.SetDrawConstants( 0, std::as_bytes( std::span<const Constants>(
	                                 static_cast<const Constants *>( constants ), 1 ) ) );
	encoder.Draw( 3 );
	encoder.EndRendering();
	return true;
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
