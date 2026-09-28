//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `vertexlit` material family's program (RFC 0016 K4); see
//			vertexlit_family.h.
//
//=============================================================================//

#include "render/material/vertexlit_family.h"

#include "family_program.h"
#include "spv/families_spv.h"

#include <array>
#include <cmath>
#include <optional>
#include <span>
#include <string_view>

namespace render::material
{

namespace
{

using namespace render::device;
using detail::ReadFlag;
using detail::ReadParameter;
using detail::SourceGammaToLinear;

// The parameters the family draws, and the ones the caller owns or the port
// ignores ($vertexcolor, and $vertexalpha's vertex data).
constexpr std::array<std::string_view, 12> kClaimed = { "basetexture", "color", "alpha",
    "alphatest", "alphatestreference", "translucent", "halflambert", "vertexcolor", "vertexalpha",
    "model", "nofog", "nocull" };

} // namespace

VertexLitClaim ClaimVertexLit( const ParameterBlock &block )
{
	VertexLitClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "vertexlit" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	const float alpha = ReadParameter( block, "alpha" );
	const bool alphaTest = ReadFlag( block, "alphatest" );
	// EvaluateBlendRequirements: constant alpha modulation or vertex alpha.
	const bool alphaBlended =
	    ReadFlag( block, "translucent" ) || ReadFlag( block, "vertexalpha" ) || alpha < 1.0f;
	claim.blend = alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque;
	claim.alphaWrite = !alphaBlended && !alphaTest;
	VertexLitConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
		constants.color[c] = SourceGammaToLinear( ReadParameter( block, "color", c ) );
	constants.color[3] = alpha;
#if defined( RENDER_MATERIAL_VERTEXLIT_SEEDED_IGNORE_HALF_LAMBERT )
	constants.flags[0] = 0.0f;
#else
	constants.flags[0] = ReadFlag( block, "halflambert" ) ? 1.0f : 0.0f;
#endif
	constants.flags[1] = alphaTest ? 1.0f : 0.0f;
	// The port's alpha test holds its reference as a byte.
	constants.flags[2] =
	    std::floor( ReadParameter( block, "alphatestreference" ) * 255.0f ) / 255.0f;
	claim.claimed = true;
	return claim;
}

foundation::Expected<std::unique_ptr<VertexLitFamily>, VertexLitStatus> VertexLitFamily::Create(
    IRenderDevice2 &device, Format colorFormat, Format depthFormat )
{
	std::unique_ptr<VertexLitFamily> family( new VertexLitFamily( device ) );
	family->m_ColorFormat = colorFormat;
	family->m_DepthFormat = depthFormat;
	const BindingDesc material[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } },
	    { 1, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	const BindingDesc draw[] = { { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex } } };
	auto materialLayout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( !materialLayout )
		return foundation::MakeUnexpected( VertexLitStatus::kDevice );
	family->m_MaterialLayout = materialLayout.Value();
	auto drawLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !drawLayout )
		return foundation::MakeUnexpected( VertexLitStatus::kDevice );
	family->m_DrawLayout = drawLayout.Value();
	return family;
}

VertexLitFamily::~VertexLitFamily()
{
	for ( const auto &[key, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, CompletionToken() );
	for ( BindGroupLayoutId layout : { m_DrawLayout, m_MaterialLayout } )
	{
		if ( layout.IsValid() )
			(void)m_Device.Release( layout, CompletionToken() );
	}
}

foundation::Expected<PipelineId, VertexLitStatus> VertexLitFamily::Pipeline(
    const VertexLitClaim &claim )
{
	const BlendMode blend = claim.blend;
	const auto key = std::make_pair( blend, claim.alphaWrite );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	const ReflectedBinding vertexBindings[] = {
	    { 2, 0, BindingKind::kUniformBuffer }, { 3, 0, BindingKind::kUniformBuffer } };
	const ReflectedBinding fragmentBindings[] = { { 2, 0, BindingKind::kUniformBuffer },
	    { 2, 1, BindingKind::kSampledTexture }, { 2, 2, BindingKind::kSampler } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kVertexLitVertex ) ), "main", vertexBindings,
	        sizeof( VertexLitDrawConstants ) },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kVertexLitFragment ) ), "main", fragmentBindings,
	        0 } };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat3, 12, 0 }, { 2, VertexFormat::kFloat2, 24, 0 } };
	const VertexBufferLayout buffers[] = { { sizeof( VertexLitVertex ), false } };
	const BindGroupLayoutId layouts[] = {
	    BindGroupLayoutId(), BindGroupLayoutId(), m_MaterialLayout, m_DrawLayout };
	const Format colors[] = { m_ColorFormat };
	const BlendMode blends[] = { blend };
	const std::uint8_t writes[] = {
	    claim.alphaWrite ? kColorWriteAll : std::uint8_t( kColorWriteAll & ~kColorWriteAlpha ) };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.drawConstantBytes = sizeof( VertexLitDrawConstants );
	desc.vertex = { attributes, buffers };
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = CullMode::kNone;
	const bool depth = m_DepthFormat != Format::kUnknown;
	desc.depthStencil = { depth, depth && blend == BlendMode::kOpaque, CompareOp::kLessEqual };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.colorWriteMasks = writes;
	desc.depthFormat = m_DepthFormat;
	desc.debugName = "render.material.vertexlit";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( VertexLitStatus::kDevice );
	m_Pipelines.emplace( key, pipeline.Value() );
	return pipeline.Value();
}

foundation::Expected<ProgramRequest, VertexLitStatus> VertexLitFamily::Request(
    const VertexLitClaim &claim, std::string baseTexture, const SamplerDesc &sampler )
{
	auto pipeline = Pipeline( claim );
	if ( !pipeline )
		return foundation::MakeUnexpected( pipeline.Error() );
	ProgramRequest request;
	request.pipeline = pipeline.Value();
	request.vertexStride = sizeof( VertexLitVertex );
	request.drawConstantBytes = sizeof( VertexLitDrawConstants );
	request.drawLayout = m_DrawLayout;
	request.material.layout = m_MaterialLayout;
	request.material.constantsBinding = 0;
	const auto bytes = std::as_bytes( std::span( &claim.constants, 1 ) );
	request.material.constants.assign( bytes.begin(), bytes.end() );
	request.material.textures.push_back( { 1, std::move( baseTexture ), 2, sampler } );
	return request;
}

GroupRequest VertexLitFamily::LightingGroup( const VertexLitLighting &lighting ) const
{
	GroupRequest request;
	request.layout = m_DrawLayout;
	request.constantsBinding = 0;
	const auto bytes = std::as_bytes( std::span( &lighting, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	return request;
}

} // namespace render::material
