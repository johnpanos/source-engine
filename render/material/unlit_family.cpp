//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `unlit` material family's program (RFC 0016 K4); see
//			unlit_family.h.
//
//=============================================================================//

#include "render/material/unlit_family.h"

#include "spv/families_spv.h"

#include <array>
#include <cmath>
#include <cstring>
#include <span>
#include <string_view>

namespace render::material
{

namespace
{

using namespace render::device;

// The parameters the family draws, and the ones the caller owns.
constexpr std::array<std::string_view, 12> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "additive",
    "model", "nofog", "nocull" };

bool IsClaimed( std::string_view name )
{
	for ( std::string_view claimed : kClaimed )
	{
		if ( claimed == name )
			return true;
	}
	return false;
}

std::size_t ByteSize( ParameterType type )
{
	switch ( type )
	{
	case ParameterType::kFloat:
	case ParameterType::kInt:
		return 4;
	case ParameterType::kFloat2:
		return 8;
	case ParameterType::kFloat3:
		return 12;
	case ParameterType::kFloat4:
		return 16;
	case ParameterType::kTexture:
		return 0;
	}
	return 0;
}

// Whether a non-texture parameter holds its schema default, compared as the
// block stores it.
bool AtDefault( const FamilySchema &family, std::span<const std::byte> bytes, std::size_t index )
{
	const ParameterLayout &layout = family.layout[index];
	const ParameterDesc &parameter = family.desc.parameters[index];
	if ( layout.type == ParameterType::kInt )
	{
		const std::int32_t value = static_cast<std::int32_t>( parameter.defaults[0] );
		return std::memcmp( bytes.data() + layout.offset, &value, sizeof( value ) ) == 0;
	}
	return std::memcmp(
	           bytes.data() + layout.offset, parameter.defaults, ByteSize( layout.type ) ) == 0;
}

float ReadFloat( const ParameterBlock &block, std::string_view name, std::size_t component = 0 )
{
	const FamilySchema &family = block.Family();
	const std::optional<std::size_t> index = family.IndexOf( name );
	if ( !index )
		return 0.0f;
	const ParameterLayout &layout = family.layout[*index];
	if ( layout.type == ParameterType::kInt )
	{
		std::int32_t value = 0;
		std::memcpy( &value, block.Bytes().data() + layout.offset, sizeof( value ) );
		return static_cast<float>( value );
	}
	float value = 0.0f;
	std::memcpy( &value, block.Bytes().data() + layout.offset + component * sizeof( float ),
	    sizeof( value ) );
	return value;
}

// Source's GammaToLinear for a material color component (mathlib
// color_conversion.cpp, as the port's SetModulationPixelShaderDynamicState_
// LinearColorSpace applies it): values above one pass unchanged, values from
// 0.95 are one, the rest go through the 256-entry pow(2.2) table.
float SourceGammaToLinear( float gamma )
{
	if ( gamma > 1.0f )
		return gamma;
	if ( gamma < 0.0f )
		return 0.0f;
	if ( gamma >= 0.95f )
		return 1.0f;
	const int index = static_cast<int>( std::lround( gamma * 255.0f ) );
	return std::pow( static_cast<float>( index ) / 255.0f, 2.2f );
}

bool ReadBool( const ParameterBlock &block, std::string_view name )
{
	return ReadFloat( block, name ) != 0.0f;
}

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
	for ( std::size_t i = 0; i < family.desc.parameters.size(); ++i )
	{
		const std::string &name = family.desc.parameters[i].name;
		if ( IsClaimed( name ) )
			continue;
		const ParameterLayout &layout = family.layout[i];
		const bool set = layout.type == ParameterType::kTexture
		                     ? block.Textures()[layout.offset].IsValid()
		                     : !AtDefault( family, block.Bytes(), i );
		if ( set )
		{
			claim.reason = "the family does not draw $" + name;
			return claim;
		}
	}
	const bool translucent = ReadBool( block, "translucent" );
	const bool additive = ReadBool( block, "additive" );
	if ( translucent && additive )
	{
		claim.reason = "$translucent with $additive blends src-alpha/one, which the port lacks";
		return claim;
	}
	const bool alphaBlended = translucent || ReadBool( block, "vertexalpha" );
	claim.blend =
	    additive ? BlendMode::kAdditive : ( alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque );
	claim.alphaWrite = !alphaBlended && !ReadBool( block, "alphatest" );
	UnlitConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
		constants.color[c] = SourceGammaToLinear( ReadFloat( block, "color", c ) );
	constants.color[3] = ReadFloat( block, "alpha" );
#if defined( RENDER_MATERIAL_UNLIT_SEEDED_IGNORE_VERTEX_COLOR )
	constants.flags[0] = 0.0f;
#else
	constants.flags[0] = ReadBool( block, "vertexcolor" ) ? 1.0f : 0.0f;
#endif
	constants.flags[1] = ReadBool( block, "vertexalpha" ) ? 1.0f : 0.0f;
	constants.flags[2] = ReadBool( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[3] = ReadFloat( block, "alphatestreference" );
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
