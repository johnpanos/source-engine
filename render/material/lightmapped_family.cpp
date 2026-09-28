//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `lightmapped` material family's program (RFC 0016 K4); see
//			lightmapped_family.h.
//
//=============================================================================//

#include "render/material/lightmapped_family.h"

#include "family_program.h"
#include "spv/families_spv.h"

#include <array>
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

// The parameters the family draws, and the ones the caller owns.
constexpr std::array<std::string_view, 11> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "model",
    "nofog", "nocull" };

} // namespace

LightmappedClaim ClaimLightmapped( const ParameterBlock &block )
{
	LightmappedClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "lightmapped" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	const bool alphaBlended = ReadFlag( block, "translucent" ) || ReadFlag( block, "vertexalpha" );
	claim.blend = alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque;
	claim.alphaWrite = !alphaBlended && !ReadFlag( block, "alphatest" );
	LightmappedConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
	{
#if defined( RENDER_MATERIAL_LIGHTMAPPED_SEEDED_GAMMA_COLOR )
		constants.tint[c] = detail::SourceGammaToLinear( ReadParameter( block, "color", c ) ) *
		                    kLightmapScaleLinear;
#else
		constants.tint[c] = ReadParameter( block, "color", c ) * kLightmapScaleLinear;
#endif
	}
	constants.tint[3] = ReadParameter( block, "alpha" );
	constants.flags[0] = ReadFlag( block, "vertexcolor" ) ? 1.0f : 0.0f;
	constants.flags[1] = ReadFlag( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[2] = ReadParameter( block, "alphatestreference" );
	claim.claimed = true;
	return claim;
}

foundation::Expected<std::unique_ptr<LightmappedFamily>, LightmappedStatus>
LightmappedFamily::Create( IRenderDevice2 &device, Format colorFormat, Format depthFormat )
{
	std::unique_ptr<LightmappedFamily> family( new LightmappedFamily( device ) );
	family->m_ColorFormat = colorFormat;
	family->m_DepthFormat = depthFormat;
	const BindingDesc material[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	const BindingDesc draw[] = { { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto materialLayout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( !materialLayout )
		return foundation::MakeUnexpected( LightmappedStatus::kDevice );
	family->m_MaterialLayout = materialLayout.Value();
	auto drawLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !drawLayout )
		return foundation::MakeUnexpected( LightmappedStatus::kDevice );
	family->m_DrawLayout = drawLayout.Value();
	return family;
}

LightmappedFamily::~LightmappedFamily()
{
	for ( const auto &[key, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, CompletionToken() );
	for ( BindGroupLayoutId layout : { m_DrawLayout, m_MaterialLayout } )
	{
		if ( layout.IsValid() )
			(void)m_Device.Release( layout, CompletionToken() );
	}
}

foundation::Expected<PipelineId, LightmappedStatus> LightmappedFamily::Pipeline(
    const LightmappedClaim &claim )
{
	const BlendMode blend = claim.blend;
	const auto key = std::make_pair( blend, claim.alphaWrite );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	const ReflectedBinding fragmentBindings[] = { { 2, 0, BindingKind::kUniformBuffer },
	    { 2, 1, BindingKind::kSampledTexture }, { 2, 2, BindingKind::kSampler },
	    { 3, 0, BindingKind::kSampledTexture }, { 3, 1, BindingKind::kSampler } };
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kLightmappedVertex ) ), "main", {},
	        sizeof( LightmappedDrawConstants ) },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kLightmappedFragment ) ), "main", fragmentBindings,
	        0 } };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kFloat2, 20, 0 },
	    { 3, VertexFormat::kUnorm8x4, 28, 0 } };
	const VertexBufferLayout buffers[] = { { sizeof( LightmappedVertex ), false } };
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
	desc.drawConstantBytes = sizeof( LightmappedDrawConstants );
	desc.vertex = { attributes, buffers };
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = CullMode::kNone;
	const bool depth = m_DepthFormat != Format::kUnknown;
	desc.depthStencil = { depth, depth && blend == BlendMode::kOpaque, CompareOp::kLessEqual };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.colorWriteMasks = writes;
	desc.depthFormat = m_DepthFormat;
	desc.debugName = "render.material.lightmapped";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( LightmappedStatus::kDevice );
	m_Pipelines.emplace( key, pipeline.Value() );
	return pipeline.Value();
}

} // namespace render::material
