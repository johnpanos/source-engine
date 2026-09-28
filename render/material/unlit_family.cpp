//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `unlit` material family's program (RFC 0016 K4); see
//			unlit_family.h.
//
//=============================================================================//

#include "render/material/unlit_family.h"

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
using detail::SourceGammaToLinear;

// The parameters the family draws, and the ones the caller owns.
constexpr std::array<std::string_view, 12> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "additive",
    "model", "nofog", "nocull" };

} // namespace

UnlitClaim ClaimUnlit( const ParameterBlock &block )
{
	UnlitClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "unlit" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	const bool translucent = ReadFlag( block, "translucent" );
	const bool additive = ReadFlag( block, "additive" );
	if ( translucent && additive )
	{
		claim.reason = "$translucent with $additive blends src-alpha/one, which the port lacks";
		return claim;
	}
	const bool alphaBlended = translucent || ReadFlag( block, "vertexalpha" );
	claim.blend =
	    additive ? BlendMode::kAdditive : ( alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque );
	claim.alphaWrite = !alphaBlended && !ReadFlag( block, "alphatest" );
	UnlitConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
		constants.color[c] = SourceGammaToLinear( ReadParameter( block, "color", c ) );
	constants.color[3] = ReadParameter( block, "alpha" );
#if defined( RENDER_MATERIAL_UNLIT_SEEDED_IGNORE_VERTEX_COLOR )
	constants.flags[0] = 0.0f;
#else
	constants.flags[0] = ReadFlag( block, "vertexcolor" ) ? 1.0f : 0.0f;
#endif
	constants.flags[1] = ReadFlag( block, "vertexalpha" ) ? 1.0f : 0.0f;
	constants.flags[2] = ReadFlag( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[3] = ReadParameter( block, "alphatestreference" );
	claim.claimed = true;
	return claim;
}

foundation::Expected<std::unique_ptr<UnlitFamily>, UnlitStatus> UnlitFamily::Create(
    IRenderDevice2 &device, Format colorFormat, Format depthFormat )
{
	std::unique_ptr<UnlitFamily> family( new UnlitFamily( device ) );
	family->m_ColorFormat = colorFormat;
	family->m_DepthFormat = depthFormat;
	const BindingDesc bindings[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( UnlitStatus::kDevice );
	family->m_MaterialLayout = layout.Value();
	return family;
}

UnlitFamily::~UnlitFamily()
{
	for ( const auto &[blend, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, CompletionToken() );
	if ( m_MaterialLayout.IsValid() )
		(void)m_Device.Release( m_MaterialLayout, CompletionToken() );
}

foundation::Expected<PipelineId, UnlitStatus> UnlitFamily::Pipeline( const UnlitClaim &claim )
{
	const BlendMode blend = claim.blend;
	const auto key = std::make_pair( blend, claim.alphaWrite );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	const ReflectedBinding fragmentBindings[] = { { 2, 0, BindingKind::kUniformBuffer },
	    { 2, 1, BindingKind::kSampledTexture }, { 2, 2, BindingKind::kSampler } };
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	                                          std::as_bytes( std::span( spirv::kUnlitVertex ) ),
	                                          "main", {}, sizeof( UnlitDrawConstants ) },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kUnlitFragment ) ), "main", fragmentBindings, 0 } };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kUnorm8x4, 20, 0 } };
	const VertexBufferLayout buffers[] = { { sizeof( UnlitVertex ), false } };
	const BindGroupLayoutId layouts[] = {
	    BindGroupLayoutId(), BindGroupLayoutId(), m_MaterialLayout, BindGroupLayoutId() };
	const Format colors[] = { m_ColorFormat };
	const BlendMode blends[] = { blend };
	const std::uint8_t writes[] = {
	    claim.alphaWrite ? kColorWriteAll : std::uint8_t( kColorWriteAll & ~kColorWriteAlpha ) };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.drawConstantBytes = sizeof( UnlitDrawConstants );
	desc.vertex = { attributes, buffers };
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = CullMode::kNone;
	const bool depth = m_DepthFormat != Format::kUnknown;
	desc.depthStencil = { depth, depth && blend == BlendMode::kOpaque, CompareOp::kLessEqual };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.colorWriteMasks = writes;
	desc.depthFormat = m_DepthFormat;
	desc.debugName = "render.material.unlit";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( UnlitStatus::kDevice );
	m_Pipelines.emplace( key, pipeline.Value() );
	return pipeline.Value();
}

} // namespace render::material
