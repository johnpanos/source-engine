//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.ssr (RFC 0016, render.ssr.v1); see
//			public/render/pass/ssr/ssr.h.
//
//=============================================================================//

#include "render/pass/ssr/ssr.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace render::pass::ssr
{

using namespace render::device;

namespace
{

constexpr const char *kPyramidSource = "render/pass/ssr/ssr_pyramid.comp";
constexpr const char *kTraceSource = "render/pass/ssr/ssr_trace.comp";
constexpr std::uint32_t kMaxLevels = 16;
constexpr std::uint32_t kGroup = 8;
constexpr std::uint32_t kDiagnosticBytes = 32; // two vec4 per pixel

// ssr_view.glsl's SsrView (std140).
struct ViewConstants
{
	float toClip[16];
	float fromClip[16];
	float eye[4];
	float extent[4];
	float params[4];
	float limits[4];
	std::uint32_t depthLevels[kMaxLevels][4];
	std::uint32_t colorLevels[kMaxLevels][4];
};
static_assert( sizeof( ViewConstants ) == 704, "ssr_view.glsl's SsrView" );

enum Binding : std::uint32_t
{
	kView = 0,
	kDepth = 1,
	kNormalRoughness = 2,
	kIblRadiance = 3,
	kSpecularWeight = 4,
	kLit = 5,
	kSampler = 6,
	kDepthPyramid = 7,
	kColorPyramid = 8,
	kOutput = 9,
	kDiagnostics = 10,
	kBindingCount = 11
};

void CopyMatrix( const math::float4x4 &m, float out[16] )
{
	for ( int r = 0; r < 4; ++r )
	{
		out[r * 4 + 0] = m.rows[r].x;
		out[r * 4 + 1] = m.rows[r].y;
		out[r * 4 + 2] = m.rows[r].z;
		out[r * 4 + 3] = m.rows[r].w;
	}
}

} // namespace

std::optional<std::string> ValidateParams( const SsrParams &params )
{
	if ( !( params.roughnessCutoff > 0.0f && params.roughnessCutoff <= 1.0f ) )
		return std::string( "the roughness cutoff is outside (0, 1]" );
	if ( !( params.roughnessFadeStart >= 0.0f && params.roughnessFadeStart < 1.0f ) )
		return std::string( "the roughness fade start is outside [0, 1)" );
	if ( !( params.thickness > 0.0f ) || !std::isfinite( params.thickness ) )
		return std::string( "the thickness is not positive and finite" );
	if ( !( params.edgeFade > 0.0f && params.edgeFade <= 0.5f ) )
		return std::string( "the edge fade is outside (0, 0.5]" );
	if ( params.maxSteps == 0 )
		return std::string( "a walk needs at least one step" );
	return std::nullopt;
}

foundation::Expected<std::unique_ptr<ScreenSpaceReflections>, SsrStatus>
ScreenSpaceReflections::Create( IRenderDevice2 &device, const SsrParams &params )
{
	return CreateWithTrace( device, params, {} );
}

foundation::Expected<std::unique_ptr<ScreenSpaceReflections>, SsrStatus>
ScreenSpaceReflections::CreateWithTrace(
    IRenderDevice2 &device, const SsrParams &params, std::span<const std::uint32_t> traceSpirv )
{
	if ( ValidateParams( params ) )
		return foundation::MakeUnexpected( SsrStatus::kInvalidParams );
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( SsrStatus::kDevice );
	std::unique_ptr<ScreenSpaceReflections> pass( new ScreenSpaceReflections( device ) );
	pass->m_Params = params;
	const ShaderStageSet compute{ ShaderStage::kCompute };
	const BindingDesc bindings[kBindingCount] = {
	    { kView, BindingKind::kUniformBuffer, 1, compute },
	    { kDepth, BindingKind::kSampledTexture, 1, compute },
	    { kNormalRoughness, BindingKind::kSampledTexture, 1, compute },
	    { kIblRadiance, BindingKind::kSampledTexture, 1, compute },
	    { kSpecularWeight, BindingKind::kSampledTexture, 1, compute },
	    { kLit, BindingKind::kSampledTexture, 1, compute },
	    { kSampler, BindingKind::kSampler, 1, compute },
	    { kDepthPyramid, BindingKind::kStorageBuffer, 1, compute },
	    { kColorPyramid, BindingKind::kStorageBuffer, 1, compute },
	    { kOutput, BindingKind::kStorageTexture, 1, compute },
	    { kDiagnostics, BindingKind::kStorageBuffer, 1, compute } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( SsrStatus::kDevice );
	pass->m_Layout = layout.Value();
	SamplerDesc point;
	point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
	point.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( point );
	if ( !sampler )
		return foundation::MakeUnexpected( SsrStatus::kDevice );
	pass->m_Sampler = sampler.Value();

	// The programs in the device's artifact format (RFC 0016 K10); a suite's
	// trace replaces the core one, on a SPIR-V device only.
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !traceSpirv.empty() &&
	     !artifacts.ReplaceSpirv( kTraceSource, traceSpirv, device.Facts().artifactFormat ) )
		return foundation::MakeUnexpected( SsrStatus::kDevice );
	for ( auto [source, out] : { std::pair{ kPyramidSource, &pass->m_Pyramid },
	          std::pair{ kTraceSource, &pass->m_Trace } } )
	{
		shaderlib::PipelineRecipe recipe =
		    shaderlib::CoreRecipe( { source }, PipelineKind::kCompute );
		recipe.layouts = { {}, {}, {}, pass->m_Layout };
		recipe.debugName = source;
		auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
		if ( !resolved )
			return foundation::MakeUnexpected( SsrStatus::kDevice );
		// The pyramid kernel reads its mode and level as draw constants.
		PipelineDesc desc = resolved.Value().Desc();
		if ( source == std::string_view( kPyramidSource ) )
			desc.drawConstantBytes = 16;
		auto pipeline = device.CreatePipeline( desc );
		if ( !pipeline )
			return foundation::MakeUnexpected( SsrStatus::kDevice );
		*out = pipeline.Value();
	}
	BufferDesc constants;
	constants.size = sizeof( ViewConstants );
	constants.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kUniform };
	auto constantsBuffer = device.CreateBuffer( constants );
	if ( !constantsBuffer )
		return foundation::MakeUnexpected( SsrStatus::kDevice );
	pass->m_Constants = constantsBuffer.Value();
	return pass;
}

ScreenSpaceReflections::~ScreenSpaceReflections()
{
	(void)m_Device.WaitIdle();
	Collect( m_LastToken );
	for ( ResourceId id : { ResourceId( m_DepthPyramid ), ResourceId( m_ColorPyramid ),
	          ResourceId( m_Diagnostics ), ResourceId( m_Constants ), ResourceId( m_Pyramid ),
	          ResourceId( m_Trace ), ResourceId( m_Sampler ), ResourceId( m_Layout ) } )
	{
		if ( id.value != 0 )
			(void)m_Device.Release( id, m_LastToken );
	}
}

foundation::Expected<void, SsrStatus> ScreenSpaceReflections::Resize(
    std::uint32_t width, std::uint32_t height )
{
	if ( width == m_Width && height == m_Height && m_DepthPyramid.IsValid() )
		return {};
	for ( BufferId *buffer : { &m_DepthPyramid, &m_ColorPyramid, &m_Diagnostics } )
	{
		if ( buffer->IsValid() )
		{
			std::lock_guard<std::mutex> lock( m_PendingLock );
			m_Pending.push_back( *buffer );
		}
		*buffer = BufferId();
	}
	m_Width = width;
	m_Height = height;
	// The min-depth pyramid: sizes halved rounding up, down to 1 x 1.
	m_DepthLevels.clear();
	std::uint32_t first = 0;
	for ( std::uint32_t w = width, h = height; m_DepthLevels.size() < kMaxLevels; )
	{
		m_DepthLevels.push_back( { first, w, h } );
		first += w * h;
		if ( w == 1 && h == 1 )
			break;
		w = ( w + 1 ) / 2;
		h = ( h + 1 ) / 2;
	}
	const std::uint32_t depthCount = first;
	// The lit pyramid: sizes halved rounding down, at least 1, at most maxMip
	// levels past the first.
	m_ColorLevels.clear();
	first = 0;
	for ( std::uint32_t w = width, h = height; m_ColorLevels.size() < kMaxLevels; )
	{
		m_ColorLevels.push_back( { first, w, h } );
		first += w * h;
		if ( ( w == 1 && h == 1 ) || m_ColorLevels.size() > m_Params.maxMip )
			break;
		w = std::max( 1u, w / 2 );
		h = std::max( 1u, h / 2 );
	}
	const std::uint32_t colorCount = first;
	auto make = [&]( std::uint64_t size, std::initializer_list<ResourceUsage> usages ) -> BufferId
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = UsageSet( usages );
		auto buffer = m_Device.CreateBuffer( desc );
		return buffer ? buffer.Value() : BufferId();
	};
	m_DepthPyramid = make( std::uint64_t( depthCount ) * 4,
	    { ResourceUsage::kStorageRead, ResourceUsage::kStorageWrite } );
	m_ColorPyramid = make( std::uint64_t( colorCount ) * 16,
	    { ResourceUsage::kStorageRead, ResourceUsage::kStorageWrite } );
	m_Diagnostics = make( std::uint64_t( width ) * height * kDiagnosticBytes,
	    { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource } );
	if ( !m_DepthPyramid.IsValid() || !m_ColorPyramid.IsValid() || !m_Diagnostics.IsValid() )
		return foundation::MakeUnexpected( SsrStatus::kDevice );
	return {};
}

foundation::Expected<void, SsrStatus> ScreenSpaceReflections::Record(
    CommandEncoder &encoder, const SsrDirectTargets &targets, const SsrView &view )
{
	if ( !targets.depth.IsValid() || !targets.normalRoughness.IsValid() ||
	     !targets.iblRadiance.IsValid() || !targets.specularWeight.IsValid() ||
	     !targets.lit.IsValid() || !targets.output.IsValid() || targets.width == 0 ||
	     targets.height == 0 )
		return foundation::MakeUnexpected( SsrStatus::kInvalidTargets );
	if ( auto resized = Resize( targets.width, targets.height ); !resized )
		return resized;
	const auto fromClip = math::Inverse( view.toClip );
	if ( !fromClip )
		return foundation::MakeUnexpected( SsrStatus::kInvalidTargets );

	ViewConstants constants{};
	CopyMatrix( view.toClip, constants.toClip );
	CopyMatrix( *fromClip, constants.fromClip );
	std::copy( view.eye, view.eye + 3, constants.eye );
	constants.extent[0] = float( targets.width );
	constants.extent[1] = float( targets.height );
	constants.extent[2] = float( m_DepthLevels.size() );
	constants.extent[3] = float( m_ColorLevels.size() );
	constants.params[0] = m_Params.roughnessCutoff;
	constants.params[1] = m_Params.roughnessFadeStart * m_Params.roughnessCutoff;
	constants.params[2] = m_Params.thickness;
	constants.params[3] = m_Params.edgeFade;
	constants.limits[0] = float( m_ColorLevels.size() - 1 );
	constants.limits[1] = float( m_Params.maxSteps );
	for ( std::size_t i = 0; i < m_DepthLevels.size(); ++i )
	{
		constants.depthLevels[i][0] = m_DepthLevels[i].first;
		constants.depthLevels[i][1] = m_DepthLevels[i].width;
		constants.depthLevels[i][2] = m_DepthLevels[i].height;
	}
	for ( std::size_t i = 0; i < m_ColorLevels.size(); ++i )
	{
		constants.colorLevels[i][0] = m_ColorLevels[i].first;
		constants.colorLevels[i][1] = m_ColorLevels[i].width;
		constants.colorLevels[i][2] = m_ColorLevels[i].height;
	}

	const BindGroupEntry entries[kBindingCount] = {
	    { kView, m_Constants, 0, sizeof( ViewConstants ), {}, {} },
	    { kDepth, {}, 0, 0, targets.depth, {} },
	    { kNormalRoughness, {}, 0, 0, targets.normalRoughness, {} },
	    { kIblRadiance, {}, 0, 0, targets.iblRadiance, {} },
	    { kSpecularWeight, {}, 0, 0, targets.specularWeight, {} },
	    { kLit, {}, 0, 0, targets.lit, {} }, { kSampler, {}, 0, 0, {}, m_Sampler },
	    { kDepthPyramid, m_DepthPyramid, 0, 0, {}, {} },
	    { kColorPyramid, m_ColorPyramid, 0, 0, {}, {} }, { kOutput, {}, 0, 0, targets.output, {} },
	    { kDiagnostics, m_Diagnostics, 0, 0, {}, {} } };
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		++m_RecordFailures;
		return foundation::MakeUnexpected( SsrStatus::kDevice );
	}
	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		m_Pending.push_back( group.Value() );
	}

	encoder.TransitionBuffer(
	    m_Constants, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( m_Constants, 0, std::as_bytes( std::span( &constants, 1 ) ) );
	encoder.TransitionBuffer(
	    m_Constants, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
	for ( BufferId buffer : { m_DepthPyramid, m_ColorPyramid, m_Diagnostics } )
		encoder.TransitionBuffer( buffer, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	encoder.TransitionTexture( targets.output, targets.outputUsage, ResourceUsage::kStorageWrite );

	// The pyramids, one dispatch per level, each after the one it reads.
	encoder.SetPipeline( m_Pyramid );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	for ( std::uint32_t mode = 0; mode < 2; ++mode )
	{
		const std::vector<Level> &levels = mode == 0 ? m_DepthLevels : m_ColorLevels;
		const BufferId buffer = mode == 0 ? m_DepthPyramid : m_ColorPyramid;
		for ( std::uint32_t level = 0; level < levels.size(); ++level )
		{
			const std::uint32_t step[4] = { mode, level, 0, 0 };
			encoder.SetDrawConstants( 0, std::as_bytes( std::span( step ) ) );
			encoder.Dispatch( ( levels[level].width + kGroup - 1 ) / kGroup,
			    ( levels[level].height + kGroup - 1 ) / kGroup );
			encoder.TransitionBuffer(
			    buffer, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
		}
	}

	// The trace and the composite.
	encoder.SetPipeline( m_Trace );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch(
	    ( targets.width + kGroup - 1 ) / kGroup, ( targets.height + kGroup - 1 ) / kGroup );
	encoder.TransitionTexture( targets.output, ResourceUsage::kStorageWrite, targets.outputUsage );
	encoder.TransitionBuffer(
	    m_Diagnostics, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
	return {};
}

void ScreenSpaceReflections::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( ResourceId id : m_Pending )
		(void)m_Device.Release( id, token );
	m_Pending.clear();
	m_LastToken = token;
}

std::uint32_t ScreenSpaceReflections::RecordFailures() const
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	return m_RecordFailures;
}

} // namespace render::pass::ssr
