//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The one surface program (RFC 0016 K11); see surface_program.h.
//
//=============================================================================//

#include "render/material/surface_program.h"

#include "render/pbr_split_sum_table.h"
#include "spv/families_spv.h"

#include <iterator>

namespace render::material
{

using namespace render::device;

namespace
{

// The material group's textures: binding, sampler binding after it.
constexpr std::uint32_t kMaterialTextures = 7;

std::span<const std::byte> VertexModule( SurfaceVertexLayout layout )
{
	switch ( layout )
	{
	case SurfaceVertexLayout::kWorld:
		return std::as_bytes( std::span( spirv::kSurfaceWorldVertex ) );
	case SurfaceVertexLayout::kModel:
		return std::as_bytes( std::span( spirv::kSurfaceModelVertex ) );
	case SurfaceVertexLayout::kFlat:
		break;
	}
	return std::as_bytes( std::span( spirv::kSurfaceFlatVertex ) );
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
	    { 2, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
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
	    { 2, BindingKind::kUniformBuffer, 1, { ShaderStage::kVertex, ShaderStage::kFragment } } };
	auto frameLayout = device.CreateBindGroupLayout( { BindGroupRole::kFrame, frame } );
	auto materialLayout = device.CreateBindGroupLayout( { BindGroupRole::kMaterial, material } );
	auto drawLayout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( frameLayout )
		program->m_FrameLayout = frameLayout.Value();
	if ( materialLayout )
		program->m_MaterialLayout = materialLayout.Value();
	if ( drawLayout )
		program->m_DrawLayout = drawLayout.Value();
	if ( !frameLayout || !materialLayout || !drawLayout )
		return foundation::MakeUnexpected( SurfaceStatus::kDevice );
	return program;
}

SurfaceProgram::~SurfaceProgram()
{
	for ( const auto &[key, pipeline] : m_Pipelines )
		(void)m_Device.Release( pipeline, CompletionToken() );
	for ( BindGroupLayoutId layout : { m_DrawLayout, m_MaterialLayout, m_FrameLayout } )
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
	const auto key = std::make_pair( variant, debug );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
		return found->second;
	std::vector<ReflectedBinding> fragmentBindings = { { 0, 0, BindingKind::kUniformBuffer },
	    { 0, 1, BindingKind::kSampledTexture }, { 0, 2, BindingKind::kSampler },
	    { 2, 0, BindingKind::kUniformBuffer }, { 3, 0, BindingKind::kSampledTexture },
	    { 3, 1, BindingKind::kSampler }, { 3, 2, BindingKind::kUniformBuffer } };
	for ( std::uint32_t texture = 0; texture < kMaterialTextures; ++texture )
	{
		fragmentBindings.push_back( { 2, 1 + texture * 2, BindingKind::kSampledTexture } );
		fragmentBindings.push_back( { 2, 2 + texture * 2, BindingKind::kSampler } );
	}
	// The flat and world vertices read the material block (the gamma vertex
	// color); the model vertex reads the draw's model lighting.
	const ReflectedBinding materialBinding[] = { { 2, 0, BindingKind::kUniformBuffer } };
	const ReflectedBinding lightingBinding[] = { { 3, 2, BindingKind::kUniformBuffer } };
	const bool model = variant.layout == SurfaceVertexLayout::kModel;
	const std::uint32_t drawConstantBytes = SurfaceDrawConstantBytes( variant.layout );
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv, VertexModule( variant.layout ), "main",
	        model ? std::span<const ReflectedBinding>( lightingBinding )
	              : std::span<const ReflectedBinding>( materialBinding ),
	        drawConstantBytes },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        m_FragmentModule.empty() ? std::as_bytes( std::span( spirv::kSurfaceFragment ) )
	                                 : std::as_bytes( m_FragmentModule ),
	        "main", fragmentBindings, 0 } };
	std::vector<SpecializationConstant> constants = { { ShaderStage::kFragment, 0, variant.terms },
	    { ShaderStage::kFragment, 1, variant.detailMode } };
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
	    m_FrameLayout, BindGroupLayoutId(), m_MaterialLayout, m_DrawLayout };
	const Format colors[] = { m_ColorFormat };
	const BlendMode blends[] = { variant.blend };
	const std::uint8_t writes[] = {
	    variant.alphaWrite ? kColorWriteAll : std::uint8_t( kColorWriteAll & ~kColorWriteAlpha ) };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
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
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.colorWriteMasks = writes;
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
	request.material.layout = m_MaterialLayout;
	request.material.constantsBinding = 0;
	SurfaceConstants packed = constants;
	packed.state[0] = variant.blend == BlendMode::kOpaque && variant.alphaWrite ? 1.0f : 0.0f;
	const auto bytes = std::as_bytes( std::span( &packed, 1 ) );
	request.material.constants.assign( bytes.begin(), bytes.end() );
	request.material.textures.push_back( { 1, textures.base, 2, sampler, true } );
	request.material.textures.push_back(
	    { 3, textures.envmap, 4, sampler, true, TextureDimension::kCube } );
	request.material.textures.push_back( { 5, textures.envmapMask, 6, sampler, false } );
	request.material.textures.push_back( { 7, textures.bump, 8, sampler, false } );
	request.material.textures.push_back(
	    { 9, textures.detail, 10, sampler, variant.detailMode == 1 } );
	request.material.textures.push_back( { 11, textures.mrao, 12, sampler, false } );
	request.material.textures.push_back( { 13, textures.emission, 14, sampler, true } );
	return request;
}

GroupRequest SurfaceProgram::FrameGroup(
    const SurfaceFrame &frame, std::string splitSumTable ) const
{
	GroupRequest request;
	request.layout = m_FrameLayout;
	request.constantsBinding = 0;
	const auto bytes = std::as_bytes( std::span( &frame, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	SamplerDesc clamped;
	clamped.address = AddressMode::kClampToEdge;
	request.textures.push_back( { 1, std::move( splitSumTable ), 2, clamped } );
	return request;
}

GroupRequest SurfaceProgram::DrawGroup(
    std::string page, const ModelLighting &lighting, const SamplerDesc &sampler ) const
{
	GroupRequest request;
	request.layout = m_DrawLayout;
	request.constantsBinding = 2;
	const auto bytes = std::as_bytes( std::span( &lighting, 1 ) );
	request.constants.assign( bytes.begin(), bytes.end() );
	request.textures.push_back( { 0, std::move( page ), 1, sampler, true } );
	return request;
}

} // namespace render::material
