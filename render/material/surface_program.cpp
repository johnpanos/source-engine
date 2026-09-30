//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The one surface program (RFC 0016 K11); see surface_program.h.
//
//=============================================================================//

#include "render/material/surface_program.h"

#include "render/pbr_ltc_table.h"
#include "render/pbr_split_sum_table.h"
#include "render/shaderlib/core_artifacts.h"

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

const char *VertexSource( SurfaceVertexLayout layout )
{
	switch ( layout )
	{
	case SurfaceVertexLayout::kWorld:
		return "render/material/families/surface_world.vert";
	case SurfaceVertexLayout::kModel:
		return "render/material/families/surface_model.vert";
	case SurfaceVertexLayout::kFlat:
		break;
	}
	return "render/material/families/surface_flat.vert";
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
	return layout == SurfaceVertexLayout::kModel ? sizeof( FamilyDrawConstants )
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

SurfaceLightGpu PackSurfaceLight(
    const light_set::RuntimeLight &light, int shadowTile, int shadowTiles, bool diffuseInBake )
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
	packed.spot[2] = float( shadowTiles );
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
    std::span<const std::uint32_t> fragmentModule )
{
	std::unique_ptr<SurfaceProgram> program( new SurfaceProgram( device ) );
	program->m_FragmentModule = fragmentModule;
	program->m_ColorFormat = colorFormat;
	program->m_DepthFormat = depthFormat;
	program->m_SampleCount = sampleCount;
	const BindingDesc frame[] = { { 0, BindingKind::kUniformBuffer, 1, { ShaderStage::kFragment } },
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
	    { 12, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
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
	    { 6, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
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
	    { 13, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
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
	if ( variant.layout == SurfaceVertexLayout::kFlat && ( variant.terms & kSurfaceNormalTerms ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	if ( variant.layout != SurfaceVertexLayout::kModel && ( variant.terms & kSurfaceModelTerms ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	// The map's probes are the pbr point's.
	if ( ( variant.terms & kSurfaceMapProbeTerms ) && !( variant.terms & kSurfacePbr ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	// The lightmap basis is the pbr point's, on a world surface.
	if ( ( variant.terms & kSurfaceLightmapTerms ) &&
	     ( variant.layout != SurfaceVertexLayout::kWorld || !( variant.terms & kSurfacePbr ) ||
	         ( ( variant.terms & kSurfaceDirectionalLightmap ) &&
	             !( variant.terms & kSurfaceBakedLightmap ) ) ) )
		return foundation::MakeUnexpected( SurfaceStatus::kInvalidRequest );
	const auto key = std::make_pair( variant, debug );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	// The program in the device's artifact format (RFC 0016 K10), with its
	// reflected bindings from the store; a suite's seeded fragment replaces
	// the core one, on a SPIR-V device only.
	const ArtifactFormat format = m_Device.Facts().artifactFormat;
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !m_FragmentModule.empty() &&
	     !artifacts.ReplaceSpirv( kFragmentSource, m_FragmentModule, format ) )
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	const bool withSsrTargets =
	    ( variant.terms & kSurfaceSsrTargets ) != 0 && !( variant.terms & kSurfaceDepthNormal );
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe(
	    { VertexSource( variant.layout ), withSsrTargets ? kSsrFragmentSource : kFragmentSource } );
	recipe.debugName = "render.material.surface";
	auto resolved = shaderlib::Resolve( recipe, artifacts, format );
	if ( !resolved )
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	const bool model = variant.layout == SurfaceVertexLayout::kModel;
	const std::uint32_t drawConstantBytes = SurfaceDrawConstantBytes( variant.layout );
	std::vector<SpecializationConstant> constants = { { ShaderStage::kFragment, 0, variant.terms },
	    { ShaderStage::kFragment, 1, variant.detailMode } };
	// The model vertex reads the terms too (the vertexlit point's lighting).
	if ( model )
		constants.push_back( { ShaderStage::kVertex, 0, variant.terms } );
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
	const VertexBufferLayout buffers[] = { { SurfaceVertexStride( variant.layout ), false } };
	const BindGroupLayoutId layouts[] = {
	    m_FrameLayout, m_ViewLayout, m_MaterialLayout, m_DrawLayout };
	// The prepass writes the normal and roughness alone; the SSR targets are
	// three more attachments (RGBA16F) after the lit color.
	const bool prepass = ( variant.terms & ( kSurfaceDepthNormal | kSurfaceRsm ) ) != 0;
	const bool ssrTargets = ( variant.terms & kSurfaceSsrTargets ) != 0 && !prepass;
	const std::uint8_t firstWrite =
	    variant.alphaWrite ? kColorWriteAll : std::uint8_t( kColorWriteAll & ~kColorWriteAlpha );
	const Format colors[] = { prepass ? Format::kRGBA16Float : m_ColorFormat, Format::kRGBA16Float,
	    Format::kRGBA16Float, Format::kRGBA16Float };
	const BlendMode blends[] = { prepass ? BlendMode::kOpaque : variant.blend, BlendMode::kOpaque,
	    BlendMode::kOpaque, BlendMode::kOpaque };
	const std::uint8_t writes[] = { prepass ? kColorWriteAll : firstWrite, kColorWriteAll,
	    kColorWriteAll, kColorWriteAll };
	const std::size_t attachments = ssrTargets ? 4 : 1;
	PipelineDesc desc = resolved.Value().Desc();
	desc.layouts = layouts;
	desc.drawConstantBytes = drawConstantBytes;
	switch ( variant.layout )
	{
	case SurfaceVertexLayout::kFlat:
		desc.vertex = { flatAttributes, buffers };
		break;
	case SurfaceVertexLayout::kWorld:
		desc.vertex = { worldAttributes, buffers };
		break;
	case SurfaceVertexLayout::kModel:
		desc.vertex = { modelAttributes, buffers };
		break;
	}
	desc.topology = PrimitiveTopology::kTriangleList;
	desc.raster.cull = CullMode::kNone;
	const bool depth = m_DepthFormat != Format::kUnknown;
	desc.depthStencil = {
	    depth, depth && variant.blend == BlendMode::kOpaque, CompareOp::kLessEqual };
	desc.colorFormats = std::span<const Format>( colors, attachments );
	desc.blends = std::span<const BlendMode>( blends, attachments );
	desc.colorWriteMasks = std::span<const std::uint8_t>( writes, attachments );
	desc.depthFormat = m_DepthFormat;
	desc.sampleCount = m_SampleCount;
	desc.debugName = "render.material.surface";
	desc.constants = constants;
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	m_Pipelines.emplace( key, pipeline.Value() );
	if ( debug.IsNeutral() )
		m_Shipped.emplace( pipeline.Value().value, variant );
	return pipeline.Value();
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

foundation::Expected<ProgramRequest, SurfaceStatus> SurfaceProgram::Request(
    const SurfaceVariant &variant, const SurfaceConstants &constants,
    const SurfaceTextures &textures, const SamplerDesc &sampler )
{
	auto pipeline = Pipeline( variant );
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
	packed.state[0] = variant.blend == BlendMode::kOpaque && variant.alphaWrite ? 1.0f : 0.0f;
	const auto bytes = std::as_bytes( std::span( &packed, 1 ) );
	request.material.constants.assign( bytes.begin(), bytes.end() );
	request.material.textures.push_back( { 1, textures.base, 2, sampler, true } );
	// The water point reads its env map without sRGB decoding, as
	// water_ps2x's sampler does.
	request.material.textures.push_back( { 3, textures.envmap, 4, sampler,
	    ( variant.terms & kSurfaceWater ) == 0, TextureDimension::kCube } );
	// The water point reads its flow map through the env map mask's binding
	// and its flow noise through MRAO's (data, like those; it reads neither).
	const bool water = ( variant.terms & kSurfaceWater ) != 0;
	request.material.textures.push_back(
	    { 5, water ? textures.flowmap : textures.envmapMask, 6, sampler, false } );
	request.material.textures.push_back( { 7, textures.bump, 8, sampler, false } );
	request.material.textures.push_back(
	    { 9, textures.detail, 10, sampler, variant.detailMode == 1 } );
	request.material.textures.push_back(
	    { 11, water ? textures.flowNoise : textures.mrao, 12, sampler, false } );
	request.material.textures.push_back( { 13, textures.emission, 14, sampler, true } );
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
	request.textures.push_back( { 9, map.reflectionProbes, 10, clamped } );
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
	const auto bytes = std::as_bytes( std::span( &view, 1 ) );
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
	// The atlas is read with a point sampler: shadow_sample.glsl filters the
	// compare itself (the device port has no comparison samplers).
	ProgramTexture atlas;
	atlas.binding = 6;
	atlas.samplerBinding = 7;
	atlas.sampler.minFilter = atlas.sampler.magFilter = atlas.sampler.mipFilter =
	    Filter::kNearest;
	atlas.sampler.address = AddressMode::kClampToEdge;
	atlas.external = shadows.atlas;
	atlas.externalDesc = shadows.atlasDesc;
	request.textures.push_back( std::move( atlas ) );
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
	occlusion.samplerBinding = 11;
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
    const SamplerDesc &sampler, std::string gradient, std::string indirect ) const
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
	return request;
}

} // namespace render::material
