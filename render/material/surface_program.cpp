//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The one surface program (RFC 0016 K11); see surface_program.h.
//
//=============================================================================//

#include "render/material/surface_program.h"

#include "render/device/errors.h"
#include "render/pbr_ltc_table.h"
#include "render/pbr_split_sum_table.h"
#include "render/shaderlib/core_artifacts.h"

#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>

namespace render::material
{

using namespace render::device;

namespace
{

// The material group's textures: binding, sampler binding after it.
constexpr std::uint32_t kMaterialTextures = 7;

// The program's stages in the core artifact store (RFC 0016 K10).
constexpr const char *kFragmentSource = "render/material/families/surface.frag";
// The same program with the SSR targets (kSurfaceSsrTargets).
constexpr const char *kSsrFragmentSource = "render/material/families/surface_ssr.frag";
constexpr const char *kShadowFragmentSource = "render/material/families/surface_shadow.frag";

const char *VertexSource( SurfaceVertexLayout layout, bool temporal = false, bool shadow = false,
    bool instanced = false )
{
	switch ( layout )
	{
	case SurfaceVertexLayout::kWorld:
		return shadow     ? "render/material/families/surface_world_shadow.vert"
		       : temporal ? "render/material/families/surface_world_temporal.vert"
		                  : "render/material/families/surface_world.vert";
	case SurfaceVertexLayout::kModel:
		return instanced  ? "render/material/families/surface_model_instanced.vert"
		       : shadow   ? "render/material/families/surface_model_shadow.vert"
		       : temporal ? "render/material/families/surface_model_temporal.vert"
		                  : "render/material/families/surface_model.vert";
	case SurfaceVertexLayout::kFlat:
		break;
	}
	return shadow     ? "render/material/families/surface_flat_shadow.vert"
	       : temporal ? "render/material/families/surface_flat_temporal.vert"
	                  : "render/material/families/surface_flat.vert";
}

} // namespace

std::uint32_t SurfaceVertexStride( SurfaceVertexLayout layout )
{
	switch ( layout )
	{
	case SurfaceVertexLayout::kWorld:
		return sizeof( SurfaceWorldVertex );
	case SurfaceVertexLayout::kModel:
		return sizeof( SurfaceModelVertex );
	case SurfaceVertexLayout::kFlat:
		break;
	}
	return sizeof( SurfaceFlatVertex );
}

std::uint32_t SurfaceDrawConstantBytes( SurfaceVertexLayout layout )
{
	return layout != SurfaceVertexLayout::kFlat ? sizeof( FamilyDrawConstants )
	                                            : sizeof( FamilyDrawConstants::toClip );
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

PbrSplitSumTable LtcTable()
{
	PbrSplitSumTable table;
	table.width = table.height = std::uint32_t( pbr::kLtcSize );
	table.texels.reserve( std::size( pbr::kLtcTable ) * 4 );
	for ( const pbr::LtcInverse &entry : pbr::kLtcTable )
	{
		table.texels.push_back( entry.m00 );
		table.texels.push_back( entry.m02 );
		table.texels.push_back( entry.m20 );
		table.texels.push_back( entry.m22 );
	}
	return table;
}

SurfaceLightGpu PackSurfaceLight( const light_set::RuntimeLight &light, int shadowTile,
    RuntimeShadowLayout shadowLayout, bool diffuseInBake, int maskId, bool moverInReach,
    std::uint32_t moverBits )
{
	SurfaceLightGpu packed;
	const bool spot = light.shape == light_set::LightShape::Spot;
	for ( int k = 0; k < 3; ++k )
	{
		packed.position[k] = light.position[k];
		packed.color[k] = light.color[k];
		packed.direction[k] = light.direction[k];
	}
	packed.position[3] = light.radius;
	packed.color[3] = light.minLight;
	packed.direction[3] = spot ? light.outerCos : -2.0f;
	packed.cone[0] = spot ? light.innerCos : 1.0f;
	packed.cone[1] = light.falloff == light_set::LightFalloff::InverseSquare ? 1.0f
	                 : light.falloff == light_set::LightFalloff::Attenuated  ? 2.0f
	                                                                         : 0.0f;
	for ( int k = 0; k < 3; ++k )
		packed.attenuation[k] = light.attenuation[k];
	packed.cone[2] = light.sourceRadius;
	packed.cone[3] = float( shadowTile );
	packed.spot[0] = light.spotExponent;
	packed.spot[1] = diffuseInBake ? 1.0f : 0.0f;
	packed.spot[2] = float( shadowLayout );
	if ( maskId > 0 )
		packed.spot[3] = float( moverInReach ? -maskId : maskId );
	if ( maskId > 0 && moverInReach )
		packed.attenuation[3] = std::bit_cast<float>( moverBits );
	return packed;
}

SurfaceAreaLight PackAreaLight(
    const area_light::AreaLight &light, bool diffuseInBake, int firstTile )
{
	SurfaceAreaLight packed;
	for ( int k = 0; k < 3; ++k )
	{
		packed.center[k] = light.rect.center[k];
		packed.halfU[k] = light.rect.halfU[k];
		packed.halfV[k] = light.rect.halfV[k];
		packed.radiance[k] = light.radiance[k];
	}
	packed.center[3] = light.rect.twoSided ? 1.0f : 0.0f;
	packed.halfU[3] = light.reach;
	packed.halfV[3] = diffuseInBake ? 1.0f : 0.0f;
	packed.radiance[3] = float( firstTile );
	return packed;
}

foundation::Expected<std::unique_ptr<SurfaceProgram>, SurfaceStatus> SurfaceProgram::Create(
    IRenderDevice2 &device, Format colorFormat, Format depthFormat, std::uint32_t sampleCount,
    std::span<const std::uint32_t> fragmentModule,
    std::span<const std::uint32_t> shadowFragmentModule )
{
	// The frame group reads the map's reflection probes as a cube array (RPRB
	// v8), so the program needs the device's cube arrays (clause D32).
	if ( !device.Facts().capabilities.Has( Capability::kCubeArrays ) )
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	std::unique_ptr<SurfaceProgram> program( new SurfaceProgram( device ) );
	program->m_FragmentModule = fragmentModule;
	program->m_ShadowFragmentModule = shadowFragmentModule;
	program->m_ColorFormat = colorFormat;
	program->m_DepthFormat = depthFormat;
	program->m_SampleCount = sampleCount;
	const BindingDesc frame[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } },
	    { 1, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 3, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 4, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 5, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 6, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 7, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 8, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 9, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 10, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 11, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 12, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 13, BindingKind::kStorageBuffer, 1, { ShaderStage::kFragment } } };
	std::vector<BindingDesc> material = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } } };
	for ( std::uint32_t texture = 0; texture < kMaterialTextures; ++texture )
	{
		material.push_back(
		    { 1 + texture * 2, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } } );
		material.push_back(
		    { 2 + texture * 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } );
	}
	const BindingDesc draw[] = { { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } },
	    { 3, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 4, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 5, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 6, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 7, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 8, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	const BindingDesc view[] = { { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kStorageBuffer, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kStorageBuffer, 1, { ShaderStage::kFragment } },
	    { 3, BindingKind::kStorageBuffer, 1, { ShaderStage::kFragment } },
	    { 4, BindingKind::kStorageBuffer, 1, { ShaderStage::kFragment } },
	    { 5, BindingKind::kStorageBuffer, 1, { ShaderStage::kFragment } },
	    { 6, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 7, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 8, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 9, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 10, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 11, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 12, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 13, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 14, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 15, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto frameLayout = device.CreateBindGroupLayout( { BindGroupRole::kFrame, frame } );
	auto viewLayout = device.CreateBindGroupLayout( { BindGroupRole::kView, view } );
	auto materialLayout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	auto drawLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( frameLayout )
		program->m_FrameLayout = frameLayout.Value();
	if ( viewLayout )
		program->m_ViewLayout = viewLayout.Value();
	if ( materialLayout )
		program->m_MaterialLayout = materialLayout.Value();
	if ( drawLayout )
		program->m_DrawLayout = drawLayout.Value();
	if ( !frameLayout || !viewLayout || !materialLayout || !drawLayout )
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	return program;
}

SurfaceProgram::~SurfaceProgram()
{
	for ( const auto &[key, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, CompletionToken() );
	for ( BindGroupLayoutId layout :
	    { m_DrawLayout, m_MaterialLayout, m_ViewLayout, m_FrameLayout } )
	{
		if ( layout.IsValid() )
			(void)m_Device.Release( layout, CompletionToken() );
	}
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::DebugPipeline(
    PipelineId shipped, const shaderlib::DebugSpecialization &debug )
{
	const auto found = m_Shipped.find( shipped.value );
	if ( found == m_Shipped.end() )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	return Pipeline( found->second, debug );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::Pipeline(
    const SurfaceVariant &variant, const shaderlib::DebugSpecialization &debug )
{
	m_PipelineFailure.clear();
	if ( variant.shadowDepth &&
	     ( variant.temporal || variant.blend != BlendMode::kOpaque || variant.portalMask ||
	         variant.decalModulate ||
	         ( variant.terms & ( kSurfaceTransmission | kSurfaceWater | kSurfaceDepthOnly ) ) ) )
	{
		m_PipelineFailure = "the point needs blending, transmission or a non-shadow depth effect";
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	}
	if ( variant.layout == SurfaceVertexLayout::kFlat && ( variant.terms & kSurfaceNormalTerms ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	// The energy point reads the tangent frame (flow coordinates, vortices,
	// opacity terms) and casts no shadow.
	if ( variant.wireframe && ( variant.shadowDepth || variant.temporal ) )
	{
		m_PipelineFailure = "a wireframe point casts no shadow and has no temporal variant";
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	}
	if ( variant.energy && ( variant.layout == SurfaceVertexLayout::kFlat || variant.shadowDepth ) )
	{
		m_PipelineFailure = "the energy point needs a tangent frame and casts no shadow";
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	}
	if ( variant.layout != SurfaceVertexLayout::kModel && ( variant.terms & kSurfaceModelTerms ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	// The map's probes are the pbr point's.
	if ( ( variant.terms & kSurfaceMapProbeTerms ) && !( variant.terms & kSurfacePbr ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	if ( ( variant.terms & kSurfaceTransmission ) && !( variant.terms & kSurfacePbr ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	if ( variant.treeSwayMode > 2 ||
	     ( variant.treeSwayMode && variant.layout == SurfaceVertexLayout::kFlat ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	// The lightmap basis is the pbr point's, on a world surface.
	if ( ( variant.terms & kSurfaceLightmapTerms ) &&
	     ( variant.layout != SurfaceVertexLayout::kWorld || !( variant.terms & kSurfacePbr ) ||
	         ( ( variant.terms & ( kSurfaceDirectionalLightmap | kSurfaceRuntimeDirect ) ) &&
	             !( variant.terms & kSurfaceBakedLightmap ) ) ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	if ( variant.instanced && ( variant.layout != SurfaceVertexLayout::kModel ||
	                              variant.temporal || variant.shadowDepth ) )
	{
		m_PipelineFailure = "only a model point that is not temporal or shadow depth instances";
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	}
	if ( !std::isfinite( variant.drawState.depthBiasConstant ) ||
	     !std::isfinite( variant.drawState.depthBiasSlope ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	const auto key = std::make_pair( variant, debug );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	// The program in the device's artifact format (RFC 0016 K10), with its
	// reflected bindings from the store; a suite's seeded fragment replaces
	// the core one, on a SPIR-V device only.
	const ArtifactFormat format = m_Device.Facts().artifactFormat;
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	const auto fragmentModule = variant.shadowDepth ? m_ShadowFragmentModule : m_FragmentModule;
	if ( !fragmentModule.empty() &&
	     !artifacts.ReplaceSpirv( variant.shadowDepth ? kShadowFragmentSource : kFragmentSource,
	         fragmentModule, format ) )
	{
		m_PipelineFailure = "the replacement fragment artifact is invalid";
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	}
	const bool withSsrTargets =
	    ( variant.terms & kSurfaceSsrTargets ) != 0 && !( variant.terms & kSurfaceDepthNormal );
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe(
	    { VertexSource( variant.layout, variant.temporal, variant.shadowDepth, variant.instanced ),
	        variant.shadowDepth ? kShadowFragmentSource
	        : variant.temporal  ? "render/material/families/surface_temporal.frag"
	        : withSsrTargets    ? kSsrFragmentSource
	                            : kFragmentSource } );
	recipe.debugName = "render.material.surface";
	auto resolved = shaderlib::Resolve( recipe, artifacts, format );
	if ( !resolved )
	{
		m_PipelineFailure = "the surface shader artifact was refused: status " +
		                    std::to_string( int( resolved.Error().status ) );
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	}
	const bool model = variant.layout == SurfaceVertexLayout::kModel;
	const bool alphaCoverage = !variant.shadowDepth && variant.alphaToCoverage && m_SampleCount > 1;
	const std::uint32_t drawConstantBytes = SurfaceDrawConstantBytes( variant.layout );
	std::vector<SpecializationConstant> constants = { { ShaderStage::kFragment, 0, variant.terms },
	    { ShaderStage::kFragment, 1, variant.detailMode },
	    { ShaderStage::kFragment, 2, variant.portalMask ? 1u : 0u },
	    { ShaderStage::kFragment, 3, variant.materialFeatures },
	    { ShaderStage::kFragment, 4, variant.viewFeatures },
	    { ShaderStage::kFragment, 5, alphaCoverage ? 1u : 0u },
	    { ShaderStage::kFragment, 6, variant.cable ? 1u : 0u },
	    { ShaderStage::kFragment, 7, variant.decalModulate ? 1u : 0u },
	    { ShaderStage::kFragment, 8, variant.shadowDepth ? 1u : 0u },
	    { ShaderStage::kFragment, 9, variant.staticVertexLight ? 1u : 0u },
	    { ShaderStage::kFragment, 11, variant.energy ? 1u : 0u } };
	// The model vertex reads the terms too (the vertexlit point's lighting),
	// the world vertex a static prop's baked vertex light.
	if ( model )
		constants.push_back( { ShaderStage::kVertex, 0, variant.terms } );
	if ( variant.layout == SurfaceVertexLayout::kWorld )
		constants.push_back( { ShaderStage::kVertex, 9, variant.staticVertexLight ? 1u : 0u } );
	if ( variant.layout != SurfaceVertexLayout::kFlat )
		constants.push_back( { ShaderStage::kVertex, 1, variant.treeSwayMode } );
	shaderlib::AppendDebugConstants( debug, ShaderStage::kFragment, constants );
	const VertexAttribute flatAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kFloat2, 20, 0 },
	    { 3, VertexFormat::kUnorm8x4, 28, 0 } };
	const VertexAttribute worldAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kFloat2, 20, 0 },
	    { 3, VertexFormat::kUnorm8x4, 28, 0 }, { 4, VertexFormat::kFloat3, 32, 0 },
	    { 5, VertexFormat::kFloat3, 44, 0 }, { 6, VertexFormat::kFloat4, 56, 0 } };
	const VertexAttribute modelAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat3, 12, 0 }, { 2, VertexFormat::kFloat4, 24, 0 },
	    { 3, VertexFormat::kFloat2, 40, 0 } };
	std::vector<VertexBufferLayout> buffers = { { SurfaceVertexStride( variant.layout ), false } };
	if ( variant.temporal )
		buffers.push_back( buffers.front() );
	if ( variant.instanced )
		buffers.push_back( { kSurfaceInstanceStride, true } );
	const BindGroupLayoutId layouts[] = {
	    m_FrameLayout, m_ViewLayout, m_MaterialLayout, m_DrawLayout };
	// The prepass writes the normal and roughness alone; the SSR targets are
	// three more attachments (RGBA16F) after the lit color.
	const bool prepass = ( variant.terms & ( kSurfaceDepthNormal | kSurfaceRsm ) ) != 0;
	const bool depthOnly = ( variant.terms & kSurfaceDepthOnly ) != 0;
	const bool ssrTargets = ( variant.terms & kSurfaceSsrTargets ) != 0 && !prepass;
	const std::uint8_t firstWrite =
	    variant.alphaWrite ? kColorWriteAll : std::uint8_t( kColorWriteAll & ~kColorWriteAlpha );
	const Format colors[] = { prepass && !depthOnly ? Format::kRGBA16Float : m_ColorFormat,
	    variant.temporal ? Format::kRG16Float : Format::kRGBA16Float,
	    variant.temporal ? Format::kR32Float : Format::kRGBA16Float, Format::kRGBA16Float };
	const BlendMode blends[] = { prepass ? BlendMode::kOpaque : variant.blend, BlendMode::kOpaque,
	    BlendMode::kOpaque, BlendMode::kOpaque };
	const std::uint8_t writes[] = { depthOnly ? std::uint8_t( 0 )
	                                : prepass
	                                    ? kColorWriteAll
	                                    : std::uint8_t( firstWrite & variant.drawState.colorWrite ),
	    std::uint8_t( variant.temporal && variant.drawState.colorWrite == 0 ? 0 : kColorWriteAll ),
	    std::uint8_t( variant.temporal && variant.drawState.colorWrite == 0 ? 0 : kColorWriteAll ),
	    kColorWriteAll };
	const std::size_t attachments = variant.shadowDepth ? 0
	                                : variant.temporal  ? 3
	                                : ssrTargets        ? 4
	                                                    : 1;
	PipelineDesc desc = resolved.Value().Desc();
	desc.layouts = layouts;
	desc.drawConstantBytes = drawConstantBytes;
	std::vector<VertexAttribute> attributes;
	if ( variant.layout == SurfaceVertexLayout::kFlat )
		attributes.assign( std::begin( flatAttributes ), std::end( flatAttributes ) );
	else if ( variant.layout == SurfaceVertexLayout::kWorld )
		attributes.assign( std::begin( worldAttributes ), std::end( worldAttributes ) );
	else
		attributes.assign( std::begin( modelAttributes ), std::end( modelAttributes ) );
	if ( variant.temporal )
		attributes.push_back( { 7, VertexFormat::kFloat3, 0, 1 } );
	// The instance record's eight rows: object-to-clip at 8-11, object-to-
	// world at 12-15 (surface_model_vertex.glsl's SURFACE_INSTANCED).
	if ( variant.instanced )
		for ( std::uint32_t row = 0; row < 8; ++row )
			attributes.push_back( { 8 + row, VertexFormat::kFloat4, row * 16, 1 } );
	desc.vertex = { attributes, buffers };
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = variant.drawState.cull;
	desc.raster.depthBiasConstant = variant.drawState.depthBiasConstant;
	desc.raster.depthBiasSlope = variant.drawState.depthBiasSlope;
	desc.raster.fill = variant.wireframe ? FillMode::kLines : FillMode::kSolid;
	const bool depth = m_DepthFormat != Format::kUnknown && !variant.ignoreDepth;
	desc.depthStencil = { depth,
	    depth && variant.blend == BlendMode::kOpaque &&
	        ( variant.terms & kSurfaceTransmission ) == 0,
	    CompareOp::kLessEqual };
	desc.depthStencil.stencil = variant.drawState.stencil;
	if ( variant.drawState.overrideDepth )
	{
		// APIs perform depth writes only with testing enabled. An always test
		// is the explicit write-without-comparison policy.
		desc.depthStencil.depthTest = variant.drawState.depthTest || variant.drawState.depthWrite;
		desc.depthStencil.depthWrite = variant.drawState.depthWrite;
		desc.depthStencil.compare =
		    variant.drawState.depthTest ? variant.drawState.depthCompare : CompareOp::kAlways;
	}
	desc.colorFormats = std::span<const Format>( colors, attachments );
	desc.blends = std::span<const BlendMode>( blends, attachments );
	desc.colorWriteMasks = std::span<const std::uint8_t>( writes, attachments );
	desc.depthFormat = m_DepthFormat;
	desc.sampleCount = m_SampleCount;
	desc.raster.alphaToCoverage = alphaCoverage;
	// The variant's kind in the name (captures, pipeline statistics, compile
	// hitch logs): layout, then the PBR point and the pass it serves.
	std::string debugName = std::string( "render.material.surface " ) +
	                        ( variant.layout == SurfaceVertexLayout::kWorld     ? "world"
	                            : variant.layout == SurfaceVertexLayout::kModel ? "model"
	                                                                            : "flat" ) +
	                        ( ( variant.terms & kSurfacePbr ) ? " pbr" : "" ) +
	                        ( depthOnly   ? " depth"
	                            : prepass ? " prepass"
	                                      : "" ) +
	                        ( variant.instanced ? " instanced" : "" ) +
	                        ( variant.staticVertexLight ? " static-light" : "" );
	desc.debugName = debugName;
	if ( variant.shadowDepth )
	{
		desc.depthFormat = Format::kD32Float;
		desc.sampleCount = 1;
		desc.depthStencil = { true, true, CompareOp::kLess };
		desc.raster.alphaToCoverage = false;
		desc.debugName = "render.material.surface-shadow";
	}
	desc.constants = constants;
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
	{
		const DeviceError &error = pipeline.Error();
		m_PipelineFailure = std::string( DescribeOperation( error.operation ) ) + ": " +
		                    DescribeStatus( error.status ) + " (native " +
		                    std::to_string( error.nativeCode ) + "), terms " +
		                    std::to_string( variant.terms ) + " layout " +
		                    std::to_string( int( variant.layout ) ) + " samples " +
		                    std::to_string( desc.sampleCount ) + " color format " +
		                    std::to_string( int( m_ColorFormat ) );
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	}
	m_Pipelines.emplace( key, pipeline.Value() );
	if ( debug.IsNeutral() )
	{
		m_Shipped.emplace( pipeline.Value().value, variant );
		if ( m_CreatedSink )
			m_CreatedSink( VariantKey( variant ) );
	}
	return pipeline.Value();
}

namespace
{
constexpr const char *kVariantKeyTag = "surface-v1";
} // namespace

std::string SurfaceProgram::VariantKey( const SurfaceVariant &v ) const
{
	const SurfaceDrawState &d = v.drawState;
	char line[512];
	std::snprintf( line, sizeof( line ),
	    "%s %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u "
	    "%u %u %u %u %u %u %08x %08x %u %u %u %u %u %u %u %u %u %u %u %u %u",
	    kVariantKeyTag, unsigned( m_ColorFormat ), unsigned( m_DepthFormat ), m_SampleCount,
	    unsigned( v.blend ), unsigned( v.alphaWrite ), v.terms, v.detailMode, unsigned( v.layout ),
	    unsigned( v.ignoreDepth ), unsigned( d.stencil.enabled ), unsigned( d.stencil.compare ),
	    unsigned( d.stencil.fail ), unsigned( d.stencil.depthFail ), unsigned( d.stencil.pass ),
	    unsigned( d.stencil.reference ), unsigned( d.stencil.readMask ),
	    unsigned( d.stencil.writeMask ), unsigned( d.cull ), unsigned( d.overrideDepth ),
	    unsigned( d.depthTest ), unsigned( d.depthWrite ), unsigned( d.depthCompare ),
	    unsigned( d.colorWrite ), std::bit_cast<std::uint32_t>( d.depthBiasConstant ),
	    std::bit_cast<std::uint32_t>( d.depthBiasSlope ), unsigned( v.portalMask ),
	    unsigned( v.temporal ), v.materialFeatures, v.viewFeatures, v.treeSwayMode,
	    unsigned( v.alphaToCoverage ), unsigned( v.decalModulate ), unsigned( v.cable ),
	    unsigned( v.shadowDepth ), unsigned( v.instanced ), unsigned( v.staticVertexLight ),
	    unsigned( v.energy ),
	    unsigned( v.wireframe ) );
	return line;
}

std::size_t SurfaceProgram::Prewarm( std::span<const std::string> keys )
{
	std::size_t created = 0;
	for ( const std::string &key : keys )
	{
		char tag[16] = {};
		unsigned f[38] = {};
		if ( std::sscanf( key.c_str(),
		         "%15s %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u "
		         "%u %u %u %u %u %u %x %x %u %u %u %u %u %u %u %u %u %u %u %u %u",
		         tag, &f[0], &f[1], &f[2], &f[3], &f[4], &f[5], &f[6], &f[7], &f[8], &f[9], &f[10],
		         &f[11], &f[12], &f[13], &f[14], &f[15], &f[16], &f[17], &f[18], &f[19], &f[20],
		         &f[21], &f[22], &f[23], &f[24], &f[25], &f[26], &f[27], &f[28], &f[29], &f[30],
		         &f[31], &f[32], &f[33], &f[34], &f[35], &f[36], &f[37] ) != 39 ||
		     std::strcmp( tag, kVariantKeyTag ) != 0 )
			continue;
		if ( f[0] != unsigned( m_ColorFormat ) || f[1] != unsigned( m_DepthFormat ) ||
		     f[2] != m_SampleCount || f[7] > unsigned( SurfaceVertexLayout::kModel ) )
			continue;
		SurfaceVariant v;
		SurfaceDrawState &d = v.drawState;
		v.blend = BlendMode( f[3] );
		v.alphaWrite = f[4] != 0;
		v.terms = f[5];
		v.detailMode = f[6];
		v.layout = SurfaceVertexLayout( f[7] );
		v.ignoreDepth = f[8] != 0;
		d.stencil.enabled = f[9] != 0;
		d.stencil.compare = CompareOp( f[10] );
		d.stencil.fail = StencilOp( f[11] );
		d.stencil.depthFail = StencilOp( f[12] );
		d.stencil.pass = StencilOp( f[13] );
		d.stencil.reference = std::uint8_t( f[14] );
		d.stencil.readMask = std::uint8_t( f[15] );
		d.stencil.writeMask = std::uint8_t( f[16] );
		d.cull = CullMode( f[17] );
		d.overrideDepth = f[18] != 0;
		d.depthTest = f[19] != 0;
		d.depthWrite = f[20] != 0;
		d.depthCompare = CompareOp( f[21] );
		d.colorWrite = std::uint8_t( f[22] );
		d.depthBiasConstant = std::bit_cast<float>( std::uint32_t( f[23] ) );
		d.depthBiasSlope = std::bit_cast<float>( std::uint32_t( f[24] ) );
		v.portalMask = f[25] != 0;
		v.temporal = f[26] != 0;
		v.materialFeatures = f[27];
		v.viewFeatures = f[28];
		v.treeSwayMode = f[29];
		v.alphaToCoverage = f[30] != 0;
		v.decalModulate = f[31] != 0;
		v.cable = f[32] != 0;
		v.shadowDepth = f[33] != 0;
		v.instanced = f[34] != 0;
		v.staticVertexLight = f[35] != 0;
		v.energy = f[36] != 0;
		v.wireframe = f[37] != 0;
		if ( Pipeline( v ) )
			++created;
	}
	return created;
}

std::uint32_t SurfaceMaterialFeatures( const SurfaceConstants &constants )
{
	return ( constants.flags[1] != 0.0f ? kSurfaceMaterialAlphaTest : 0u ) |
	       ( constants.flags[3] > 0.5f ? kSurfaceMaterialHalfLambert : 0u ) |
	       ( constants.meshModes[0] > 0.5f ? kSurfaceMaterialDiffuseWarp : 0u ) |
	       ( constants.meshProbeColor[2] > 0.5f ? kSurfaceMaterialSpecularWarp : 0u ) |
	       ( constants.meshModes[3] > 0.5f ? kSurfaceMaterialUnlitMesh : 0u );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::StatePipeline(
    PipelineId shipped, const SurfaceDrawState &state, const shaderlib::DebugSpecialization &debug )
{
	return ViewPipeline( shipped, state, kSurfaceAllViewFeatures, debug );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::ViewPipeline( PipelineId shipped,
    const SurfaceDrawState &state, std::uint32_t viewFeatures,
    const shaderlib::DebugSpecialization &debug )
{
	m_PipelineFailure.clear();
	const auto found = m_Shipped.find( shipped.value );
	if ( found == m_Shipped.end() || ( viewFeatures & ~kSurfaceAllViewFeatures ) != 0 )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	SurfaceVariant variant = found->second;
	variant.drawState = state;
	variant.viewFeatures = viewFeatures;
	return Pipeline( variant, debug );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::TemporalPipeline(
    PipelineId shipped, const SurfaceDrawState &state, std::uint32_t viewFeatures,
    const shaderlib::DebugSpecialization &debug )
{
	const auto found = m_Shipped.find( shipped.value );
	if ( found == m_Shipped.end() || ( found->second.terms & kSurfaceSsrTargets ) ||
	     m_SampleCount != 1 )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	SurfaceVariant variant = found->second;
	variant.drawState = state;
	variant.viewFeatures = viewFeatures;
	variant.temporal = true;
	return Pipeline( variant, debug );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::VariantPipeline(
    PipelineId shipped, std::uint32_t add, std::uint32_t remove )
{
	const auto found = m_Shipped.find( shipped.value );
	if ( found == m_Shipped.end() )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	SurfaceVariant variant = found->second;
	variant.terms = ( variant.terms | add ) & ~remove;
	return Pipeline( variant );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::StaticVertexLightPipeline(
    PipelineId shipped )
{
	const auto found = m_Shipped.find( shipped.value );
	if ( found == m_Shipped.end() || found->second.layout != SurfaceVertexLayout::kWorld )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	SurfaceVariant variant = found->second;
	variant.staticVertexLight = true;
	return Pipeline( variant );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::InstancedPipeline(
    PipelineId pipeline )
{
	const auto found = m_Shipped.find( pipeline.value );
	if ( found == m_Shipped.end() )
	{
		m_PipelineFailure = "the surface program did not create the requested point";
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	}
	SurfaceVariant variant = found->second;
	variant.instanced = true;
	return Pipeline( variant );
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::ShadowPipeline( PipelineId shipped )
{
	const auto found = m_Shipped.find( shipped.value );
	if ( found == m_Shipped.end() )
	{
		m_PipelineFailure = "the surface program did not create the requested point";
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	}
	SurfaceVariant variant = found->second;
	variant.shadowDepth = true;
	variant.temporal = false;
	variant.alphaToCoverage = false;
	variant.ignoreDepth = false;
	variant.terms &= ~( kSurfaceRsm | kSurfaceDepthNormal | kSurfaceSsrTargets );
	return Pipeline( variant );
}

foundation::Expected<ProgramRequest, SurfaceStatus> SurfaceProgram::Request(
    const SurfaceVariant &variant, const SurfaceConstants &constants,
    const SurfaceTextures &textures, const SamplerDesc &sampler )
{
	SurfaceVariant point = variant;
	if ( point.terms & kSurfacePbr )
	{
		const std::uint32_t features = SurfaceMaterialFeatures( constants );
		// Preserve the established control flow for authored warp lookups.
		// Specializing the portal gun's mipmapped warp changes edge pixels;
		// ordinary materials and view lighting can specialize independently.
		const bool warped =
		    ( features & ( kSurfaceMaterialDiffuseWarp | kSurfaceMaterialSpecularWarp ) ) != 0;
		point.materialFeatures = warped ? kSurfaceDynamicMaterialFeatures : features;
	}
	auto pipeline = Pipeline( point );
	if ( !pipeline )
		return foundation::MakeUnexpected( pipeline.Error() );
	ProgramRequest request;
	request.pipeline = pipeline.Value();
	request.vertexStride = SurfaceVertexStride( variant.layout );
	request.drawConstantBytes = SurfaceDrawConstantBytes( variant.layout );
	request.drawLayout = m_DrawLayout;
	request.frameLayout = m_FrameLayout;
	request.viewLayout = m_ViewLayout;
	request.neutralView = NeutralViewGroup();
	request.material.layout = m_MaterialLayout;
	request.material.constantsBinding = 0;
	SurfaceConstants packed = constants;
	packed.state[0] = variant.blend == BlendMode::kOpaque && variant.alphaWrite &&
	                          ( variant.terms & kSurfaceTransmission ) == 0
	                      ? 1.0f
	                      : 0.0f;
	const auto bytes = std::as_bytes( std::span( &packed, 1 ) );
	request.material.constants.assign( bytes.begin(), bytes.end() );
	request.material.textures.push_back(
	    { 1, textures.base, 2, sampler, textures.baseSrgb && !variant.decalModulate } );
	// The water point reads its env map without sRGB decoding, as
	// water_ps2x's sampler does.
	request.material.textures.push_back( { 3, textures.envmap, 4, sampler,
	    ( variant.terms & kSurfaceWater ) == 0, TextureDimension::kCube } );
	// The water and energy points read their flow map through the env map
	// mask's binding and their flow noise through MRAO's (data, like those;
	// they read neither). The energy point's details are sRGB images
	// (solidenergy_dx9_helper.cpp) at the detail and bump bindings, and its
	// flow bounds data at the emission binding.
	const bool water = ( variant.terms & kSurfaceWater ) != 0;
	const bool flow = water || variant.energy;
	request.material.textures.push_back(
	    { 5, flow ? textures.flowmap : textures.envmapMask, 6, sampler, false } );
	request.material.textures.push_back(
	    { 7, variant.energy ? textures.detail2 : textures.bump, 8, sampler, variant.energy } );
	request.material.textures.push_back( { 9, textures.detail, 10, sampler,
	    ( variant.terms & kSurfacePbr ) != 0 || variant.detailMode == 1 || variant.energy } );
	request.material.textures.push_back(
	    { 11, flow ? textures.flowNoise : textures.mrao, 12, sampler, false } );
	request.material.textures.push_back( { 13,
	    variant.energy ? textures.flowBounds : textures.emission, 14, sampler,
	    !variant.energy && ( variant.terms & kSurfaceSelfIllumMask ) == 0 } );
	return request;
}

GroupRequest SurfaceProgram::FrameGroup( const SurfaceFrame &frame, std::string splitSumTable,
    std::string ltcTable, const SurfaceMapTextures &map ) const
{
	GroupRequest request;
	request.layout = m_FrameLayout;
	request.constantsBinding = 0;
	const auto bytes = std::as_bytes( std::span( &frame, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	SamplerDesc clamped;
	clamped.address = AddressMode::kClampToEdge;
	request.textures.push_back( { 1, std::move( splitSumTable ), 2, clamped } );
	request.textures.push_back( { 3, std::move( ltcTable ), 4, clamped } );
	// The probe atlas is filtered, clamped to edge (probe_volume.glsl); its
	// grid table is fetched. The probe texture is fetched and filtered.
	request.textures.push_back( { 5, map.probeAtlas, 6, clamped } );
	request.textures.push_back( { 7, map.probeGrids, 8, clamped } );
	// The reflection probes' radiance: a cube array read by world direction
	// (a neutral cube array where a map has none), trilinear, clamped.
	ProgramTexture radiance;
	radiance.binding = 9;
	radiance.name = map.reflectionProbes;
	radiance.samplerBinding = 10;
	radiance.sampler = clamped;
	radiance.dimension = TextureDimension::kCube;
	radiance.array = true;
	request.textures.push_back( std::move( radiance ) );
	GroupBuffer probes;
	probes.binding = 13;
	if ( map.reflectionBuffer && !map.reflectionBuffer->empty() )
		probes.bytes = *map.reflectionBuffer;
	else
		probes.bytes.assign( 16, std::byte( 0 ) ); // count 0: no probes
	request.storage.push_back( std::move( probes ) );
	// The bounce atlas has the probe atlas's layout and is sampled with its
	// sampler (binding 6).
	ProgramTexture bounce;
	bounce.binding = 11;
	bounce.samplerBinding = 12;
	bounce.sampler = clamped;
	bounce.external = map.probeBounce;
	bounce.externalDesc = map.probeBounceDesc;
	request.textures.push_back( std::move( bounce ) );
	return request;
}

GroupRequest SurfaceProgram::ViewGroup( const SurfaceViewGpu &view,
    std::span<const std::byte> froxels, std::span<const std::byte> indices,
    std::span<const SurfaceLightGpu> lights, const SurfaceShadows &shadows,
    const SurfaceProjectors &projectors, const SurfaceScreenInputs &screen ) const
{
	GroupRequest request;
	request.layout = m_ViewLayout;
	request.constantsBinding = 0;
	SurfaceViewGpu captured = view;
	captured.depthAlpha[0] = screen.depthAlphaRange;
	captured.depthAlpha[1] =
	    screen.depthAlphaSourceWidth > 0 ? 1.0f / float( screen.depthAlphaSourceWidth ) : 0.0f;
	captured.depthAlpha[2] =
	    screen.depthAlphaSourceHeight > 0 ? 1.0f / float( screen.depthAlphaSourceHeight ) : 0.0f;
	const auto bytes = std::as_bytes( std::span( &captured, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	auto storage = [&]( std::uint32_t binding, std::span<const std::byte> data )
	{
		GroupBuffer buffer;
		buffer.binding = binding;
		buffer.bytes.assign( data.begin(), data.end() );
		if ( buffer.bytes.empty() )
			buffer.bytes.resize( 16 ); // a binding needs a size; the count says none
		request.storage.push_back( std::move( buffer ) );
	};
	storage( 1, froxels );
	storage( 2, indices );
	storage( 3, std::as_bytes( lights ) );
	storage( 4, std::as_bytes( shadows.tiles ) );
	storage( 5, std::as_bytes( projectors.lights ) );
	// Blocker search reads raw depth; visibility compares before filtering.
	ProgramTexture atlas;
	atlas.binding = 6;
	atlas.samplerBinding = 7;
	atlas.sampler.minFilter = atlas.sampler.magFilter = atlas.sampler.mipFilter =
	    Filter::kNearest;
	atlas.sampler.address = AddressMode::kClampToEdge;
	atlas.depth = true;
	atlas.external = shadows.atlas;
	atlas.externalDesc = shadows.atlasDesc;
	request.textures.push_back( std::move( atlas ) );
	SamplerDesc comparison;
	comparison.mipFilter = Filter::kNearest;
	comparison.address = AddressMode::kClampToEdge;
	comparison.comparison = CompareOp::kLessEqual;
	request.samplers.emplace_back( 11, comparison );
	// The cookies: a 2D array, filtered and clamped (the legacy flashlight
	// samples its cookie so).
	ProgramTexture cookies;
	cookies.binding = 8;
	cookies.samplerBinding = 9;
	cookies.sampler.address = AddressMode::kClampToEdge;
	cookies.external = projectors.cookies;
	cookies.externalDesc = projectors.cookiesDesc;
	cookies.array = true;
	request.textures.push_back( std::move( cookies ) );
	// The occlusion is fetched per pixel (texelFetch).
	ProgramTexture occlusion;
	occlusion.binding = 10;
	occlusion.samplerBinding = kNoSamplerBinding;
	occlusion.sampler.minFilter = occlusion.sampler.magFilter = occlusion.sampler.mipFilter =
	    Filter::kNearest;
	occlusion.sampler.address = AddressMode::kClampToEdge;
	occlusion.external = screen.ambientOcclusion;
	occlusion.externalDesc = screen.ambientOcclusionDesc;
	request.textures.push_back( std::move( occlusion ) );
	// The planar reflection: filtered, clamped (the water point offsets its
	// coordinates by the normal, and the legacy target clamps).
	ProgramTexture reflection;
	reflection.binding = 12;
	reflection.samplerBinding = 13;
	reflection.sampler.address = AddressMode::kClampToEdge;
	reflection.external = screen.planarReflection;
	reflection.externalDesc = screen.planarReflectionDesc;
	request.textures.push_back( std::move( reflection ) );
	ProgramTexture sceneColor;
	sceneColor.binding = 14;
	sceneColor.samplerBinding = 15;
	sceneColor.sampler.minFilter = sceneColor.sampler.magFilter = Filter::kLinear;
	sceneColor.sampler.mipFilter = Filter::kNearest;
	sceneColor.sampler.address = AddressMode::kClampToEdge;
	// Transmission and soft particles are different points of the surface
	// program. They share the scene snapshot binding, with per-point view groups.
	sceneColor.external = screen.depthAlpha.IsValid() ? screen.depthAlpha : screen.sceneColor;
	sceneColor.externalDesc =
	    screen.depthAlpha.IsValid() ? screen.depthAlphaDesc : screen.sceneColorDesc;
	request.textures.push_back( std::move( sceneColor ) );
	return request;
}

GroupRequest SurfaceProgram::NeutralViewGroup( const SurfaceScreenInputs &screen ) const
{
	// One froxel with no lights; no variant without kSurfaceClustered reads it.
	SurfaceViewGpu view;
	view.grid[0] = view.grid[1] = view.grid[2] = view.grid[3] = 1;
	const std::uint32_t froxel[2] = { 0, 0 };
	const std::uint32_t indices[4] = {};
	return ViewGroup( view, std::as_bytes( std::span( froxel ) ),
	    std::as_bytes( std::span( indices ) ), {}, {}, {}, screen );
}

GroupRequest SurfaceProgram::DrawGroup( std::string page, const ModelLighting &lighting,
    const SamplerDesc &sampler, std::string gradient, std::string indirect,
    std::string shadowMask ) const
{
	GroupRequest request;
	request.layout = m_DrawLayout;
	request.constantsBinding = 2;
	const auto bytes = std::as_bytes( std::span( &lighting, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	request.textures.push_back( { 0, std::move( page ), 1, sampler, true } );
	// The gradient page is signed linear data, filtered as the page is.
	request.textures.push_back( { 3, std::move( gradient ), 4, sampler } );
	request.textures.push_back( { 5, std::move( indirect ), 6, sampler } );
	// The baked shadow masks are visibility, filtered as the page is.
	request.textures.push_back( { 7, std::move( shadowMask ), 8, sampler } );
	return request;
}

} // namespace render::material
