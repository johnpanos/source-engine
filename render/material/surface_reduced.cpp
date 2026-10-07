//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The reduced 3DS material model (surface_reduced.h): its rules,
//			and the surface program's reduced layouts and pipelines.
//
//=============================================================================//

#include "surface_reduced.h"

#include "render/device/pica_codes.h"

#include <cmath>
#include <cstddef>
#include <cstring>

#if defined( __3DS__ )
extern "C" const unsigned char surface_flat_shbin[];
extern "C" const unsigned surface_flat_shbin_size;
extern "C" const unsigned char surface_static_shbin[];
extern "C" const unsigned surface_static_shbin_size;
extern "C" const unsigned char surface_model_shbin[];
extern "C" const unsigned surface_model_shbin_size;
extern "C" const unsigned char surface_lit_shbin[];
extern "C" const unsigned surface_lit_shbin_size;
#endif

namespace render::material
{

using namespace device;
namespace pf = device::pica_format;

namespace
{

// A combiner stage combining two sources into colour and alpha.
pf::CombinerStage Stage( std::uint8_t first, std::uint8_t second, std::uint8_t combine )
{
	pf::CombinerStage stage;
	stage.rgbSources = { first, second, pf::source::kPrimaryColor };
	stage.alphaSources = stage.rgbSources;
	stage.rgbCombine = stage.alphaCombine = combine;
	return stage;
}

pf::CombinerStage FromMaterial( pf::CombinerStage stage, std::uint32_t offset )
{
	stage.constantKind = pf::ConstantKind::kUniformBuffer;
	stage.constantGroup = std::uint8_t( BindGroupRole::kMaterial );
	stage.constantBinding = 0;
	stage.constantOffset = offset;
	return stage;
}

// The textures a reduced point samples: the material's base (binding 1, its
// sampler 2) and the draw's lightmap page (binding 0, its sampler 1).
constexpr pf::TextureUnit kBaseUnit{ 2, 1, 2, 2 };
constexpr pf::TextureUnit kLightmapUnit{ 3, 0, 3, 1 };

struct Dropped
{
	std::uint32_t term;
	std::string_view name;
};

// Terms the model loses, each by a recorded rule (RFC 0026 decision 8).
constexpr Dropped kDroppedTerms[] = { { kSurfaceDetail, "detail texture" },
    { kSurfaceBump, "bump and normal maps" }, { kSurfaceSsbump, "self-shadowed bump map" },
    { kSurfaceDiffuseBump, "bumped diffuse lighting (the flat lightmap page is used)" },
    { kSurfaceHalfLambert, "half-Lambert vertex lighting" }, { kSurfaceEnvmap, "environment map" },
    { kSurfaceEnvmapMask, "environment map mask" },
    { kSurfaceBaseAlphaEnvmapMask, "base-alpha environment mask" },
    { kSurfaceNormalMapAlphaEnvmapMask, "normal-alpha environment mask" },
    { kSurfaceEmissionTexture, "emission texture" },
    { kSurfaceSelfIllumMask, "self-illumination mask (self-illumination dropped with it)" },
    { kSurfaceClustered, "clustered runtime lights" },
    { kSurfaceRuntimeDirect, "runtime direct light" }, { kSurfaceProbeVolume, "probe volume" },
    { kSurfaceReflectionProbes, "reflection probes" },
    { kSurfaceAmbientOcclusion, "ambient occlusion" }, { kSurfaceProbeBounce, "probe bounce" },
    { kSurfaceMeshDirect, "mesh direct light" },
    { kSurfaceDirectionalLightmap, "directional lightmap (the flat page is used)" },
    { kSurfaceMraoTexture, "metalness, roughness and occlusion texture" },
    { kSurfacePhongExponentTexture, "Phong exponent texture" },
    { kSurfacePbr, "physically based shading (base colour kept)" } };

// Terms or points the model cannot stand in for: dropping them would change
// what the surface is, so the point is refused by name.
struct Refused
{
	std::uint32_t term;
	std::string_view reason;
};

constexpr Refused kRefusedTerms[] = {
    { kSurfaceWater, "water (no refraction or reflection on the PICA200)" },
    { kSurfaceTransmission, "transmission (no scene colour on the PICA200)" },
    { kSurfaceDepthNormal, "the depth-normal prepass (no such pass on the PICA200)" },
    { kSurfaceRsm, "reflective shadow maps" },
    { kSurfaceSsrTargets, "screen-space reflection targets" },
    { kSurfaceDepthOnly, "depth-only passes" } };

} // namespace

int ReducedAlphaTest( const SurfaceConstants &constants )
{
	if ( constants.flags[1] == 0.0f )
		return -1;
	const float reference = std::isnan( constants.flags[2] ) ? 0.0f : constants.flags[2];
	return int( std::lround( std::fmin( std::fmax( reference, 0.0f ), 1.0f ) * 255.0f ) );
}

ReducedPoint ReduceSurface( const SurfaceVariant &variant )
{
	ReducedPoint point;
	auto refuse = [&point]( std::string_view why )
	{
		point.refusal = std::string( why );
		return point;
	};
	if ( variant.shadowDepth )
		return refuse( "shadow depth (the PICA200 has no shadow atlas)" );
	if ( variant.temporal )
		return refuse( "temporal history (no motion vectors on the PICA200)" );
	if ( variant.energy )
		return refuse( "the energy point (flow fields need a programmable fragment stage)" );
	if ( variant.cable )
		return refuse( "the cable point (ribbon shading needs a programmable fragment stage)" );
	if ( variant.portalMask )
		return refuse( "portal masks (the stencil portal path is not reduced yet)" );
	if ( variant.instanced )
		return refuse( "instanced draws (the PICA200 draws no instances)" );
	for ( const Refused &refused : kRefusedTerms )
		if ( variant.terms & refused.term )
			return refuse( refused.reason );
	for ( const Dropped &dropped : kDroppedTerms )
		if ( variant.terms & dropped.term )
			point.dropped.push_back( dropped.name );
	// Foliage keeps its shape and loses its motion (the reduced vertex
	// programs deform nothing).
	if ( variant.treeSwayMode != 0 )
		point.dropped.push_back( "tree sway (vertex deformation)" );
	// The PICA200's fog unit is not driven: every point loses fog.
	point.dropped.push_back( "fog" );

	pf::FragmentProgram &f = point.fragment;
	f.textures.push_back( kBaseUnit );
	if ( variant.alphaTestReference >= 0 )
	{
		f.alphaTest = pf::test::kGreaterEqual;
		f.alphaReference = std::uint8_t( std::min<int>( variant.alphaTestReference, 255 ) );
	}
	// A modulating decal draws its factors unlit (the blend multiplies them).
	if ( variant.decalModulate )
	{
		f.stages.push_back(
		    Stage( pf::source::kTexture0, pf::source::kTexture0, pf::combine::kReplace ) );
		return point;
	}

	const bool unlit = ( variant.terms & kSurfaceUnlit ) != 0;
	// A model draw handed over as world geometry (the pbr point with no baked
	// lightmap, on a map without a stage) is lit by the draw's model lighting
	// like the model layout; every other world surface by its lightmap.
	const bool worldLit = variant.layout != SurfaceVertexLayout::kModel && !unlit &&
	                      !variant.staticVertexLight && ( variant.terms & kSurfacePbr ) &&
	                      !( variant.terms & kSurfaceBakedLightmap );
	const bool model = variant.layout == SurfaceVertexLayout::kModel || worldLit;
	const bool selfIllum = ( variant.terms & kSurfaceSelfIllum ) != 0 &&
	                       ( variant.terms & kSurfaceSelfIllumMask ) == 0 && !unlit;
	point.vertex = worldLit                 ? ReducedVertex::kWorldLit
	               : model                     ? ReducedVertex::kModel
	               : variant.staticVertexLight ? ReducedVertex::kStaticLight
	                                           : ReducedVertex::kFlat;
	// Self-illumination: base x $selfillumtint, kept in the combiner buffer.
	if ( selfIllum )
	{
		f.stages.push_back( FromMaterial(
		    Stage( pf::source::kTexture0, pf::source::kConstant, pf::combine::kModulate ),
		    kReducedSelfIllumTintOffset ) );
		f.bufferWrite = 0x11; // stage 0's colour and alpha
	}
	// base x tint
	f.stages.push_back(
	    FromMaterial( Stage( pf::source::kTexture0, pf::source::kConstant, pf::combine::kModulate ),
	        kReducedTintOffset ) );
	// x light: the vertex colour (unlit), or the light doubled (Source's
	// overbright: the lightmap's and the vertex programs' are halved).
	pf::CombinerStage light;
	if ( unlit || model || variant.staticVertexLight )
		light = Stage( pf::source::kPrevious, pf::source::kPrimaryColor, pf::combine::kModulate );
	else
	{
		f.textures.push_back( kLightmapUnit );
		light = Stage( pf::source::kPrevious, pf::source::kTexture1, pf::combine::kModulate );
	}
	if ( !unlit )
		light.rgbScale = pf::scale::k2;
	// The light colours the surface; its alpha stays the material's.
	light.alphaSources = { pf::source::kPrevious, pf::source::kPrevious, pf::source::kPrevious };
	light.alphaCombine = pf::combine::kReplace;
	f.stages.push_back( light );
	if ( selfIllum )
	{
		// lerp( lit, base x $selfillumtint, base alpha ): INTERPOLATE takes
		// source 0 x source 2 + source 1 x (1 - source 2).
		pf::CombinerStage mix;
		mix.rgbSources = {
		    pf::source::kPreviousBuffer, pf::source::kPrevious, pf::source::kTexture0 };
		mix.rgbOperands = {
		    pf::operand::kRgbColor, pf::operand::kRgbColor, pf::operand::kRgbAlpha };
		mix.rgbCombine = pf::combine::kInterpolate;
		mix.alphaSources = { pf::source::kPrevious, pf::source::kPrevious, pf::source::kPrevious };
		mix.alphaCombine = pf::combine::kReplace;
		f.stages.push_back( mix );
	}
	return point;
}

std::vector<std::byte> ReducedVertexArtifact( ReducedVertex vertex )
{
#if defined( __3DS__ )
	// families/pica/*.v.pica: the draw-constant block in c0-c7 (toClip, and
	// the world/model's object-to-world: FamilyDrawConstants is 128 bytes),
	// then the material constants' tint and flags (c8-c9) or the model's
	// lighting (c8 on, ModelLighting, 27 registers); picasso's own constants
	// follow, clear of the block.
	pf::VertexProgram program;
	program.drawConstantRegister = 0;
	const unsigned char *code = surface_flat_shbin;
	unsigned size = surface_flat_shbin_size;
	switch ( vertex )
	{
	case ReducedVertex::kFlat:
		program.uniforms = { { std::uint8_t( BindGroupRole::kMaterial ), 0, 8, 2 } };
		break;
	case ReducedVertex::kStaticLight:
		code = surface_static_shbin;
		size = surface_static_shbin_size;
		break;
	case ReducedVertex::kModel:
		code = surface_model_shbin;
		size = surface_model_shbin_size;
		program.uniforms = { { std::uint8_t( BindGroupRole::kDraw ), 2, 8, 27 } };
		break;
	case ReducedVertex::kWorldLit:
		code = surface_lit_shbin;
		size = surface_lit_shbin_size;
		program.uniforms = { { std::uint8_t( BindGroupRole::kDraw ), 2, 8, 27 } };
		break;
	}
	return pf::WriteVertexProgram( program, { reinterpret_cast<const std::byte *>( code ), size } );
#else
	(void)vertex;
	return {};
#endif
}

// The surface program's reduced mode ----------------------------------------------

bool SurfaceProgram::CreateReducedLayouts()
{
	const BindingDesc frame[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } } };
	const BindingDesc view[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } } };
	const BindingDesc material[] = {
	    { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } },
	    { 1, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	const BindingDesc draw[] = { { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } },
	    { 2, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } } };
	auto frameLayout = m_Device.CreateBindGroupLayout( { BindGroupRole::kFrame, frame } );
	auto viewLayout = m_Device.CreateBindGroupLayout( { BindGroupRole::kView, view } );
	auto materialLayout = m_Device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	auto drawLayout = m_Device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( frameLayout )
		m_FrameLayout = frameLayout.Value();
	if ( viewLayout )
		m_ViewLayout = viewLayout.Value();
	if ( materialLayout )
		m_MaterialLayout = materialLayout.Value();
	if ( drawLayout )
		m_DrawLayout = drawLayout.Value();
	return frameLayout && viewLayout && materialLayout && drawLayout;
}

foundation::Expected<PipelineId, SurfaceStatus> SurfaceProgram::ReducedPipeline(
    const SurfaceVariant &variant )
{
	const ReducedPoint point = ReduceSurface( variant );
	if ( !point.refusal.empty() )
	{
		m_PipelineFailure = "the reduced 3DS material model refuses " + point.refusal;
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	}
	const std::vector<std::byte> vertex = ReducedVertexArtifact( point.vertex );
	if ( vertex.empty() )
	{
		m_PipelineFailure = "the reduced vertex programs are built into 3DS builds only";
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	}
	const std::vector<std::byte> fragment = pf::WriteFragmentProgram( point.fragment );
	pf::VertexProgram vertexProgram;
	const std::uint32_t drawConstantBytes = SurfaceDrawConstantBytes( variant.layout );
	(void)pf::ReadVertexProgram( vertex, drawConstantBytes, vertexProgram );
	const auto vertexBindings = pf::BindingsOf( vertexProgram );
	const auto fragmentBindings = pf::BindingsOf( point.fragment );
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kPica, vertex, "main", vertexBindings },
	    { ShaderStage::kFragment, ArtifactFormat::kPica, fragment, "main", fragmentBindings } };
	const VertexAttribute flatAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat2, 12, 0 }, { 2, VertexFormat::kFloat2, 20, 0 },
	    { 3, VertexFormat::kUnorm8x4, 28, 0 } };
	const VertexAttribute modelAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat3, 12, 0 }, { 3, VertexFormat::kFloat2, 40, 0 } };
	// SurfaceWorldVertex's position, normal and uv.
	static_assert( offsetof( SurfaceWorldVertex, uv ) == 12 &&
	               offsetof( SurfaceWorldVertex, normal ) == 32 );
	const VertexAttribute litAttributes[] = { { 0, VertexFormat::kFloat3, 0, 0 },
	    { 1, VertexFormat::kFloat3, 32, 0 }, { 3, VertexFormat::kFloat2, 12, 0 } };
	const VertexBufferLayout buffers[] = { { SurfaceVertexStride( variant.layout ), false } };
	const BindGroupLayoutId layouts[] = {
	    m_FrameLayout, m_ViewLayout, m_MaterialLayout, m_DrawLayout };
	const bool model = variant.layout == SurfaceVertexLayout::kModel;
	PipelineDesc desc;
	desc.stages = stages;
	desc.layouts = layouts;
	desc.drawConstantBytes = drawConstantBytes;
	desc.vertex = { point.vertex == ReducedVertex::kWorldLit
	                    ? std::span<const VertexAttribute>( litAttributes )
	                : model ? std::span<const VertexAttribute>( modelAttributes )
	                        : std::span<const VertexAttribute>( flatAttributes ),
	    buffers };
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = variant.drawState.cull;
	const bool depth = m_DepthFormat != Format::kUnknown && !variant.ignoreDepth;
	desc.depthStencil = {
	    depth, depth && variant.blend == BlendMode::kOpaque, CompareOp::kLessEqual };
	desc.depthStencil.stencil = variant.drawState.stencil;
	if ( variant.drawState.overrideDepth )
	{
		desc.depthStencil.depthTest = variant.drawState.depthTest || variant.drawState.depthWrite;
		desc.depthStencil.depthWrite = variant.drawState.depthWrite;
		desc.depthStencil.compare =
		    variant.drawState.depthTest ? variant.drawState.depthCompare : CompareOp::kAlways;
	}
	const Format colors[] = { m_ColorFormat };
	const BlendMode blends[] = { variant.blend };
	const std::uint8_t writes[] = {
	    std::uint8_t( ( variant.alphaWrite ? kColorWriteAll : kColorWriteAll & ~kColorWriteAlpha ) &
	                  variant.drawState.colorWrite ) };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.colorWriteMasks = writes;
	desc.depthFormat = m_DepthFormat;
	desc.debugName =
	    model ? "render.material.surface reduced model" : "render.material.surface reduced";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
	{
		const DeviceError &error = pipeline.Error();
		m_PipelineFailure = std::string( "reduced " ) + DescribeOperation( error.operation ) +
		                    ": " + DescribeStatus( error.status );
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	}
	return pipeline.Value();
}

} // namespace render::material
