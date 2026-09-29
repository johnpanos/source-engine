//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `pbr` material family's program (RFC 0016 K4); see
//			pbr_family.h.
//
//=============================================================================//

#include "render/material/pbr_family.h"

#include "family_program.h"
#include "render/pbr_split_sum_table.h"
#include "spv/families_spv.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string_view>

namespace render::material
{

namespace
{

using namespace render::device;
using detail::ReadParameter;

constexpr float kPi = 3.14159265358979323846f;

// The parameters the family draws, and $fallbackmaterial, which only other
// profiles read.
constexpr std::array<std::string_view, 6> kClaimed = { "basetexture", "mraotexture", "bumpmap",
    "emissiontexture", "emissionscale", "fallbackmaterial" };

bool TextureBound( const ParameterBlock &block, std::string_view name )
{
	const FamilySchema &family = block.Family();
	const std::optional<std::size_t> index = family.IndexOf( name );
	if ( !index || family.layout[*index].type != ParameterType::kTexture )
		return false;
	return block.Textures()[family.layout[*index].offset].IsValid();
}

int TypeOrder( PbrLightType type )
{
	return type == PbrLightType::kSpot ? 0 : ( type == PbrLightType::kPoint ? 1 : 2 );
}

} // namespace

PbrModelLighting PackSourceModelLighting(
    const float eye[3], const float cube[6][3], std::span<const PbrLightDesc> lights )
{
	PbrModelLighting packed;
	for ( int c = 0; c < 3; ++c )
		packed.eye[c] = eye[c];
	for ( int face = 0; face < 6; ++face )
	{
		for ( int c = 0; c < 3; ++c )
			packed.cube[face][c] = cube[face][c];
	}
	// SortLights: an insertion sort by type, stable for equal types.
	std::array<const PbrLightDesc *, kPbrMaxLights> order = {};
	std::size_t count = 0;
	for ( const PbrLightDesc &light : lights )
	{
		if ( count == kPbrMaxLights )
			break;
		std::size_t j = count;
		while ( j > 0 && TypeOrder( order[j - 1]->type ) > TypeOrder( light.type ) )
		{
			order[j] = order[j - 1];
			--j;
		}
		order[j] = &light;
		++count;
	}
	packed.eye[3] = float( count );
	for ( std::size_t n = 0; n < count; ++n )
	{
		const PbrLightDesc &light = *order[n];
		PbrModelLighting::Light &out = packed.lights[n];
		const bool spot = light.type == PbrLightType::kSpot;
		for ( int c = 0; c < 3; ++c )
		{
			out.color[c] = light.color[c];
			out.direction[c] = light.direction[c];
			out.position[c] = light.position[c];
			out.attenuation[c] = light.attenuation[c];
		}
		out.color[3] = light.type == PbrLightType::kDirectional ? 1.0f : 0.0f;
		out.direction[3] = spot ? 1.0f : 0.0f;
		if ( spot )
		{
			// CShaderAPIDx8::SetLight's cone adjustment.
			const float phi = std::min( light.phi, kPi );
			const float theta = light.theta - phi > -1e-3f ? phi - 1e-3f : light.theta;
			const float stopdot = std::cos( theta * 0.5f );
			const float stopdot2 = std::cos( phi * 0.5f );
			out.spot[0] = light.falloff;
			out.spot[1] = stopdot;
			out.spot[2] = stopdot2;
			out.spot[3] = stopdot > stopdot2 ? 1.0f / ( stopdot - stopdot2 ) : 0.0f;
		}
		else
		{
			out.spot[1] = out.spot[2] = out.spot[3] = 1.0f;
		}
	}
	return packed;
}

PbrSplitSumTable SplitSumTable()
{
	PbrSplitSumTable table;
	table.width = table.height = std::uint32_t( pbr::kSplitSumSize );
	table.texels.reserve( std::size( pbr::kSplitSumTable ) * 4 );
	for ( const pbr::SplitSumCoefficients &coefficients : pbr::kSplitSumTable )
	{
		table.texels.push_back( coefficients.a );
		table.texels.push_back( coefficients.b );
		table.texels.push_back( 0.0f );
		table.texels.push_back( 1.0f );
	}
	return table;
}

PbrClaim ClaimPbr( const ParameterBlock &block )
{
	PbrClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "pbr" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	if ( !TextureBound( block, "basetexture" ) || !TextureBound( block, "mraotexture" ) )
	{
		claim.reason = "PBRMetalRough needs $basetexture and $mraotexture";
		return claim;
	}
	claim.normalMap = TextureBound( block, "bumpmap" );
	claim.emission = TextureBound( block, "emissiontexture" );
#if defined( RENDER_MATERIAL_PBR_SEEDED_IGNORE_NORMAL_MAP )
	claim.normalMap = false;
#endif
	claim.constants.flags[0] = claim.normalMap ? 1.0f : 0.0f;
	claim.constants.flags[1] = claim.emission ? 1.0f : 0.0f;
	claim.constants.flags[2] = ReadParameter( block, "emissionscale" );
	claim.claimed = true;
	return claim;
}

foundation::Expected<std::unique_ptr<PbrFamily>, PbrStatus> PbrFamily::Create(
    IRenderDevice2 &device, Format colorFormat, Format depthFormat )
{
	std::unique_ptr<PbrFamily> family( new PbrFamily( device ) );
	family->m_ColorFormat = colorFormat;
	family->m_DepthFormat = depthFormat;
	const BindingDesc frame[] = {
	    { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	const BindingDesc view[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } } };
	std::vector<BindingDesc> material = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	for ( std::uint32_t texture = 0; texture < 4; ++texture )
	{
		material.push_back(
		    { 1 + texture * 2, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } } );
		material.push_back(
		    { 2 + texture * 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } );
	}
	auto frameLayout = device.CreateBindGroupLayout( { BindGroupRole::kFrame, frame } );
	auto viewLayout = device.CreateBindGroupLayout( { BindGroupRole::kView, view } );
	auto materialLayout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	if ( frameLayout )
		family->m_FrameLayout = frameLayout.Value();
	if ( viewLayout )
		family->m_ViewLayout = viewLayout.Value();
	if ( materialLayout )
		family->m_MaterialLayout = materialLayout.Value();
	if ( !frameLayout || !viewLayout || !materialLayout )
		return foundation::MakeUnexpected( PbrStatus::kDevice );
	return family;
}

PbrFamily::~PbrFamily()
{
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, CompletionToken() );
	for ( const auto &[debug, pipeline] : m_DebugPipelines )
		(void)m_Device.Release( pipeline, CompletionToken() );
	for ( BindGroupLayoutId layout : { m_MaterialLayout, m_ViewLayout, m_FrameLayout } )
	{
		if ( layout.IsValid() )
			(void)m_Device.Release( layout, CompletionToken() );
	}
}

std::optional<PipelineId> PbrFamily::DebugPipeline(
    PipelineId shipped, const shaderlib::DebugSpecialization &debug )
{
	if ( !m_Pipeline.IsValid() || shipped != m_Pipeline )
		return std::nullopt;
	auto pipeline = Pipeline( PbrClaim(), debug );
	if ( !pipeline )
		return std::nullopt;
	return pipeline.Value();
}

foundation::Expected<PipelineId, PbrStatus> PbrFamily::Pipeline(
    const PbrClaim &, const shaderlib::DebugSpecialization &debug )
{
	if ( debug.IsNeutral() && m_Pipeline.IsValid() )
		return m_Pipeline;
	if ( auto found = m_DebugPipelines.find( debug ); found != m_DebugPipelines.end() )
		return found->second;
	const ReflectedBinding vertexBindings[] = { { 1, 0, BindingKind::kUniformBuffer } };
	std::vector<ReflectedBinding> fragmentBindings = { { 0, 0, BindingKind::kSampledTexture },
	    { 0, 1, BindingKind::kSampler }, { 1, 0, BindingKind::kUniformBuffer },
	    { 2, 0, BindingKind::kUniformBuffer } };
	for ( std::uint32_t texture = 0; texture < 4; ++texture )
	{
		fragmentBindings.push_back( { 2, 1 + texture * 2, BindingKind::kSampledTexture } );
		fragmentBindings.push_back( { 2, 2 + texture * 2, BindingKind::kSampler } );
	}
	const ShaderArtifactView stages[] = { { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	                                          std::as_bytes( std::span( spirv::kPbrVertex ) ),
	                                          "main", vertexBindings, sizeof( PbrDrawConstants ) },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kPbrFragment ) ), "main", fragmentBindings, 0 } };
	const VertexAttribute attributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat3, 12, 0 }, { 2, VertexFormat::kFloat4, 24, 0 },
	    { 3, VertexFormat::kFloat2, 40, 0 } };
	const VertexBufferLayout buffers[] = { { sizeof( PbrVertex ), false } };
	const BindGroupLayoutId layouts[] = {
	    m_FrameLayout, m_ViewLayout, m_MaterialLayout, BindGroupLayoutId() };
	const Format colors[] = { m_ColorFormat };
	const BlendMode blends[] = { BlendMode::kOpaque };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.drawConstantBytes = sizeof( PbrDrawConstants );
	desc.vertex = { attributes, buffers };
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = CullMode::kNone;
	const bool depth = m_DepthFormat != Format::kUnknown;
	desc.depthStencil = { depth, depth, CompareOp::kLessEqual };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.depthFormat = m_DepthFormat;
	desc.debugName = "render.material.pbr";
	std::vector<SpecializationConstant> constants;
	shaderlib::AppendDebugConstants( debug, ShaderStage::kFragment, constants );
	desc.constants = constants;
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( PbrStatus::kDevice );
	if ( !debug.IsNeutral() )
	{
		m_DebugPipelines.emplace( debug, pipeline.Value() );
		return pipeline.Value();
	}
	m_Pipeline = pipeline.Value();
	return m_Pipeline;
}

foundation::Expected<ProgramRequest, PbrStatus> PbrFamily::Request(
    const PbrClaim &claim, PbrTextures textures, const SamplerDesc &sampler )
{
	auto pipeline = Pipeline( claim );
	if ( !pipeline )
		return foundation::MakeUnexpected( pipeline.Error() );
	ProgramRequest request;
	request.pipeline = pipeline.Value();
	request.vertexStride = sizeof( PbrVertex );
	request.drawConstantBytes = sizeof( PbrDrawConstants );
	request.frameLayout = m_FrameLayout;
	request.viewLayout = m_ViewLayout;
	request.material.layout = m_MaterialLayout;
	request.material.constantsBinding = 0;
	const auto bytes = std::as_bytes( std::span( &claim.constants, 1 ) );
	request.material.constants.assign( bytes.begin(), bytes.end() );
	std::string names[] = { std::move( textures.base ), std::move( textures.mrao ),
	    claim.normalMap ? std::move( textures.normal ) : textures.placeholder,
	    claim.emission ? std::move( textures.emission ) : textures.placeholder };
	for ( std::uint32_t slot = 0; slot < 4; ++slot )
		request.material.textures.push_back(
		    { 1 + slot * 2, std::move( names[slot] ), 2 + slot * 2, sampler } );
	return request;
}

GroupRequest PbrFamily::FrameGroup( std::string table ) const
{
	GroupRequest request;
	request.layout = m_FrameLayout;
	SamplerDesc sampler;
	sampler.address = AddressMode::kClampToEdge;
	request.textures.push_back( { 0, std::move( table ), 1, sampler } );
	return request;
}

GroupRequest PbrFamily::ViewGroup( const PbrModelLighting &lighting ) const
{
	GroupRequest request;
	request.layout = m_ViewLayout;
	request.constantsBinding = 0;
	const auto bytes = std::as_bytes( std::span( &lighting, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	return request;
}

} // namespace render::material
