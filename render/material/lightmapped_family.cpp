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
#include <string>
#include <vector>
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
using detail::TextureBound;

// The parameters the family draws, and the ones the caller owns.
constexpr std::array<std::string_view, 32> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "model",
    "nofog", "nocull", "bumpmap", "ssbump", "nodiffusebumplighting", "envmap", "envmapmask",
    "basealphaenvmapmask", "normalmapalphaenvmapmask", "envmaptint", "envmapcontrast",
    "envmapsaturation", "fresnelreflection", "detail", "detailscale", "detailblendmode",
    "detailblendfactor", "detailtint", "selfillum", "selfillumtint", "ssbumpmathfix",
    "envmaplightscale", "envmaplightscaleminmax" };

// The detail modes the port's combos draw: every TextureCombine mode but the
// self-illuminating ones (5, 6) without a bump map, and 0 and 1 with one
// (lightmappedgeneric_ps2_3_x.h's SKIP lines). 10 and 11 are the ssbump
// detail modes, which a detail texture's own flag selects.
bool DetailModeDrawn( int mode, bool bump )
{
	if ( bump )
		return mode == 0 || mode == 1;
	return mode == 0 || mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 7 ||
	       mode == 8 || mode == 9;
}

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
	// The terms, as lightmappedgeneric_dx9_helper.cpp sets the port's combos.
	const bool bump = TextureBound( block, "bumpmap" );
	const bool envmap = TextureBound( block, "envmap" );
	const bool selfIllum = ReadFlag( block, "selfillum" );
	if ( bump )
	{
		claim.terms |= ReadFlag( block, "ssbump" ) ? kLightmappedSsbump : kLightmappedBump;
		if ( !ReadFlag( block, "nodiffusebumplighting" ) )
			claim.terms |= kLightmappedDiffuseBump;
	}
	if ( envmap )
		claim.terms |= kLightmappedEnvmap;
	if ( TextureBound( block, "envmapmask" ) )
		claim.terms |= kLightmappedEnvmapMask;
	if ( ReadFlag( block, "basealphaenvmapmask" ) )
		claim.terms |= kLightmappedBaseAlphaEnvmapMask;
	if ( ReadFlag( block, "normalmapalphaenvmapmask" ) )
		claim.terms |= kLightmappedNormalMapAlphaEnvmapMask;
	if ( selfIllum )
		claim.terms |= kLightmappedSelfIllum;
	if ( TextureBound( block, "detail" ) )
	{
		const int mode = int( ReadParameter( block, "detailblendmode" ) );
		if ( !DetailModeDrawn( mode, bump ) )
		{
			claim.reason = "the family does not draw $detailblendmode " + std::to_string( mode ) +
			               ( bump ? " with a bump map" : "" );
			return claim;
		}
		claim.terms |= kLightmappedDetail;
		claim.detailMode = std::uint32_t( mode );
	}

	const bool alphaBlended = ReadFlag( block, "translucent" ) || ReadFlag( block, "vertexalpha" );
	claim.blend = alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque;
	claim.alphaWrite = !alphaBlended && !ReadFlag( block, "alphatest" );
	LightmappedConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
	{
#if defined( RENDER_MATERIAL_LIGHTMAPPED_SEEDED_GAMMA_COLOR )
		constants.tint[c] = detail::SourceGammaToLinear( ReadParameter( block, "color", c ) );
#else
		constants.tint[c] = ReadParameter( block, "color", c );
#endif
	}
	constants.tint[3] = ReadParameter( block, "alpha" );
	constants.flags[0] = ReadFlag( block, "vertexcolor" ) ? 1.0f : 0.0f;
	constants.flags[1] = ReadFlag( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[2] = detail::AlphaTestReference( block );

	// The env map's knobs as the port reads them. Its pixel fast path
	// (contrast 0 or 1, saturation 1, no fresnel, no self-illumination tint)
	// holds unless the material uses contrast with saturation, fresnel, or a
	// self-illumination tint; on it $envmapcontrast is 1 exactly when set to
	// 1 (FASTPATHENVMAPCONTRAST), and anything else is 0.
	float tint[3];
	float selfIllumTint[3];
	for ( int c = 0; c < 3; ++c )
	{
		tint[c] = ReadParameter( block, "envmaptint", c );
		selfIllumTint[c] = ReadParameter( block, "selfillumtint", c );
	}
	const float contrast = ReadParameter( block, "envmapcontrast" );
	const float saturation = ReadParameter( block, "envmapsaturation" );
	const float fresnel = ReadParameter( block, "fresnelreflection" );
	const bool usingContrast = envmap && contrast != 0.0f && contrast != 1.0f && saturation != 1.0f;
	const bool usingFresnel = envmap && fresnel != 1.0f;
	const bool usingSelfIllumTint =
	    selfIllum &&
	    ( selfIllumTint[0] != 1.0f || selfIllumTint[1] != 1.0f || selfIllumTint[2] != 1.0f );
	const bool fastPath = !usingContrast && !usingFresnel && !usingSelfIllumTint;
	for ( int c = 0; c < 3; ++c )
	{
		constants.envTint[c] = tint[c];
		constants.envContrast[c] = fastPath ? ( contrast == 1.0f ? 1.0f : 0.0f ) : contrast;
		constants.envSaturation[c] = fastPath ? 1.0f : saturation;
		constants.selfIllumTint[c] = fastPath ? 1.0f : selfIllumTint[c];
		constants.detailTint[c] = ReadParameter( block, "detailtint", c );
	}
	constants.envTint[3] = fastPath ? 1.0f : fresnel;
	constants.envContrast[3] = fastPath ? 0.0f : 1.0f - fresnel;
	constants.detailTint[3] = ReadParameter( block, "detailblendfactor" );
	constants.detailScale[0] = constants.detailScale[1] = ReadParameter( block, "detailscale" );
	constants.state[2] = ReadFlag( block, "ssbumpmathfix" ) ? 0.57735025882720947f : 1.0f;
	const float lightScaleMin = ReadParameter( block, "envmaplightscaleminmax", 0 );
	constants.envLightScale[0] = lightScaleMin;
	constants.envLightScale[1] =
	    ReadParameter( block, "envmaplightscaleminmax", 1 ) + lightScaleMin;
	constants.envLightScale[2] = ReadParameter( block, "envmaplightscale" );
	claim.claimed = true;
	return claim;
}

foundation::Expected<std::unique_ptr<LightmappedFamily>, LightmappedStatus>
LightmappedFamily::Create( IRenderDevice2 &device, Format colorFormat, Format depthFormat,
    std::uint32_t sampleCount, std::span<const std::uint32_t> fragmentModule )
{
	std::unique_ptr<LightmappedFamily> family( new LightmappedFamily( device ) );
	family->m_FragmentModule = fragmentModule;
	family->m_ColorFormat = colorFormat;
	family->m_DepthFormat = depthFormat;
	family->m_SampleCount = sampleCount;
	std::vector<BindingDesc> material = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } } };
	for ( std::uint32_t binding = 1; binding <= 9; binding += 2 )
	{
		material.push_back(
		    { binding, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } } );
		material.push_back( { binding + 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } );
	}
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
	const BindingDesc frame[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	auto frameLayout = device.CreateBindGroupLayout( { BindGroupRole::kFrame, frame } );
	if ( !frameLayout )
		return foundation::MakeUnexpected( LightmappedStatus::kDevice );
	family->m_FrameLayout = frameLayout.Value();
	return family;
}

LightmappedFamily::~LightmappedFamily()
{
	for ( const auto &[key, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, CompletionToken() );
	for ( BindGroupLayoutId layout : { m_DrawLayout, m_MaterialLayout, m_FrameLayout } )
	{
		if ( layout.IsValid() )
			(void)m_Device.Release( layout, CompletionToken() );
	}
}

foundation::Expected<PipelineId, LightmappedStatus> LightmappedFamily::DebugPipeline(
    PipelineId shipped, const shaderlib::DebugSpecialization &debug )
{
	const auto found = m_Shipped.find( shipped.value );
	if ( found == m_Shipped.end() )
		return foundation::MakeUnexpected( LightmappedStatus::kInvalidRequest );
	return Pipeline( found->second.first, found->second.second, debug );
}

foundation::Expected<PipelineId, LightmappedStatus> LightmappedFamily::Pipeline(
    const LightmappedClaim &claim, LightmappedVertexLayout layout,
    const shaderlib::DebugSpecialization &debug )
{
	if ( layout == LightmappedVertexLayout::kFlat && ( claim.terms & kLightmappedSurfaceTerms ) )
		return foundation::MakeUnexpected( LightmappedStatus::kInvalidRequest );
	const BlendMode blend = claim.blend;
	const PipelineKey key =
	    std::make_tuple( blend, claim.alphaWrite, claim.terms, claim.detailMode, layout, debug );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	std::vector<ReflectedBinding> fragmentBindings = { { 0, 0, BindingKind::kUniformBuffer },
	    { 2, 0, BindingKind::kUniformBuffer }, { 3, 0, BindingKind::kSampledTexture },
	    { 3, 1, BindingKind::kSampler } };
	for ( std::uint32_t binding = 1; binding <= 9; binding += 2 )
	{
		fragmentBindings.push_back( { 2, binding, BindingKind::kSampledTexture } );
		fragmentBindings.push_back( { 2, binding + 1, BindingKind::kSampler } );
	}
	const ReflectedBinding vertexBindings[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const bool surface = layout == LightmappedVertexLayout::kSurface;
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        surface ? std::span<const std::byte>(
	                      std::as_bytes( std::span( spirv::kLightmappedSurfaceVertex ) ) )
	                : std::span<const std::byte>(
	                      std::as_bytes( std::span( spirv::kLightmappedVertex ) ) ),
	        "main", vertexBindings, sizeof( LightmappedDrawConstants ) },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        m_FragmentModule.empty() ? std::as_bytes( std::span( spirv::kLightmappedFragment ) )
	                                 : std::as_bytes( m_FragmentModule ),
	        "main", fragmentBindings, 0 } };
	std::vector<SpecializationConstant> constants = { { ShaderStage::kFragment, 0, claim.terms },
	    { ShaderStage::kFragment, 1, claim.detailMode } };
	shaderlib::AppendDebugConstants( debug, ShaderStage::kFragment, constants );
	const VertexAttribute flatAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kFloat2, 20, 0 },
	    { 3, VertexFormat::kUnorm8x4, 28, 0 } };
	const VertexAttribute surfaceAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kFloat2, 20, 0 },
	    { 3, VertexFormat::kUnorm8x4, 28, 0 }, { 4, VertexFormat::kFloat3, 32, 0 },
	    { 5, VertexFormat::kFloat3, 44, 0 }, { 6, VertexFormat::kFloat4, 56, 0 } };
	const VertexBufferLayout buffers[] = {
	    { surface ? std::uint32_t( sizeof( LightmappedSurfaceVertex ) )
	              : std::uint32_t( sizeof( LightmappedVertex ) ),
	        false } };
	const BindGroupLayoutId layouts[] = {
	    m_FrameLayout, BindGroupLayoutId(), m_MaterialLayout, m_DrawLayout };
	const Format colors[] = { m_ColorFormat };
	const BlendMode blends[] = { blend };
	const std::uint8_t writes[] = {
	    claim.alphaWrite ? kColorWriteAll : std::uint8_t( kColorWriteAll & ~kColorWriteAlpha ) };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.drawConstantBytes = sizeof( LightmappedDrawConstants );
	if ( surface )
		desc.vertex = { surfaceAttributes, buffers };
	else
		desc.vertex = { flatAttributes, buffers };
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = CullMode::kNone;
	const bool depth = m_DepthFormat != Format::kUnknown;
	desc.depthStencil = { depth, depth && blend == BlendMode::kOpaque, CompareOp::kLessEqual };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.colorWriteMasks = writes;
	desc.depthFormat = m_DepthFormat;
	desc.sampleCount = m_SampleCount;
	desc.debugName = "render.material.lightmapped";
	desc.constants = constants;
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( LightmappedStatus::kDevice );
	m_Pipelines.emplace( key, pipeline.Value() );
	if ( debug.IsNeutral() )
		m_Shipped.emplace( pipeline.Value().value, std::make_pair( claim, layout ) );
	return pipeline.Value();
}

foundation::Expected<ProgramRequest, LightmappedStatus> LightmappedFamily::Request(
    const LightmappedClaim &claim, const LightmappedTextures &textures,
    LightmappedVertexLayout layout, const SamplerDesc &sampler )
{
	auto pipeline = Pipeline( claim, layout );
	if ( !pipeline )
		return foundation::MakeUnexpected( pipeline.Error() );
	ProgramRequest request;
	request.pipeline = pipeline.Value();
	request.vertexStride = layout == LightmappedVertexLayout::kSurface
	                           ? std::uint32_t( sizeof( LightmappedSurfaceVertex ) )
	                           : std::uint32_t( sizeof( LightmappedVertex ) );
	request.drawConstantBytes = sizeof( LightmappedDrawConstants );
	request.drawLayout = m_DrawLayout;
	request.frameLayout = m_FrameLayout;
	request.material.layout = m_MaterialLayout;
	request.material.constantsBinding = 0;
	LightmappedConstants constants = claim.constants;
	constants.state[0] = claim.blend == BlendMode::kOpaque && claim.alphaWrite ? 1.0f : 0.0f;
	const auto bytes = std::as_bytes( std::span( &constants, 1 ) );
	request.material.constants.assign( bytes.begin(), bytes.end() );
	// The base texture and the env map are gamma images (sRGB views; an HDR
	// cube holds linear light and passes as it is); the mask and the bump map
	// are data. A detail texture reads through sRGB only in mode 1 (additive):
	// the other modes combine its gamma values as they are
	// (lightmappedgeneric_dx9_helper.cpp: EnableSRGBRead( SAMPLER12, mode == 1 )).
	request.material.textures.push_back( { 1, textures.base, 2, sampler, true } );
	request.material.textures.push_back(
	    { 3, textures.envmap, 4, sampler, true, TextureDimension::kCube } );
	request.material.textures.push_back( { 5, textures.envmapMask, 6, sampler, false } );
	request.material.textures.push_back( { 7, textures.bump, 8, sampler, false } );
	request.material.textures.push_back(
	    { 9, textures.detail, 10, sampler, claim.detailMode == 1 } );
	return request;
}

foundation::Expected<ProgramRequest, LightmappedStatus> LightmappedFamily::Request(
    const LightmappedClaim &claim, std::string baseTexture, const SamplerDesc &sampler )
{
	LightmappedTextures textures;
	textures.base = std::move( baseTexture );
	return Request( claim, textures, LightmappedVertexLayout::kFlat, sampler );
}

GroupRequest LightmappedFamily::LightmapGroup( std::string page, const SamplerDesc &sampler ) const
{
	GroupRequest request;
	request.layout = m_DrawLayout;
	request.textures.push_back( { 0, std::move( page ), 1, sampler, true } );
	return request;
}

GroupRequest LightmappedFamily::FrameGroup( const LightmappedFrame &frame ) const
{
	GroupRequest request;
	request.layout = m_FrameLayout;
	request.constantsBinding = 0;
	const auto bytes = std::as_bytes( std::span( &frame, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	return request;
}

} // namespace render::material
