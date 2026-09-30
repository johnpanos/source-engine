//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.ao: ground-truth ambient occlusion; see
//			public/render/pass/ao/ao.h.
//
//=============================================================================//

#include "render/pass/ao/ao.h"

#include "render/shaderlib/core_artifacts.h"

#include <cmath>
#include <span>

namespace render::pass::ao
{

using namespace render::device;

namespace
{

constexpr const char *kSource = "render/pass/ao/gtao.comp";
constexpr std::uint32_t kGroup = 8;

struct ViewConstants
{
	float view[16];
	float fromClip[16];
	float eye[4];
	float extent[4];
	float params[4];
	float blur[4];
};
static_assert( sizeof( ViewConstants ) == 192, "gtao.comp's AoView" );

enum Binding : std::uint32_t
{
	kView = 0,
	kDepth = 1,
	kNormal = 2,
	kSampler = 3,
	kScratch = 4,
	kOutput = 5,
	kBindingCount = 6
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

foundation::Expected<std::unique_ptr<AmbientOcclusion>, AoStatus> AmbientOcclusion::Create(
    IRenderDevice2 &device, const AoParams &params )
{
	if ( !( params.radius > 0.0f ) || !( params.falloff >= 0.0f && params.falloff < 1.0f ) ||
	     params.slices == 0 || params.steps == 0 )
		return foundation::MakeUnexpected( AoStatus::kInvalidParams );
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( AoStatus::kDevice );
	std::unique_ptr<AmbientOcclusion> pass( new AmbientOcclusion( device ) );
	pass->m_Params = params;
	const ShaderStageSet compute{ ShaderStage::kCompute };
	const BindingDesc bindings[kBindingCount] = {
	    { kView, BindingKind::kUniformBuffer, 1, compute },
	    { kDepth, BindingKind::kSampledTexture, 1, compute },
	    { kNormal, BindingKind::kSampledTexture, 1, compute },
	    { kSampler, BindingKind::kSampler, 1, compute },
	    { kScratch, BindingKind::kStorageBuffer, 1, compute },
	    { kOutput, BindingKind::kStorageTexture, 1, compute } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( AoStatus::kDevice );
	pass->m_Layout = layout.Value();
	SamplerDesc point;
	point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
	point.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( point );
	if ( !sampler )
		return foundation::MakeUnexpected( AoStatus::kDevice );
	pass->m_Sampler = sampler.Value();
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kSource }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, pass->m_Layout };
	recipe.debugName = kSource;
	auto resolved =
	    shaderlib::Resolve( recipe, shaderlib::CoreArtifacts(), device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected( AoStatus::kDevice );
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = 16;
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( AoStatus::kDevice );
	pass->m_Pipeline = pipeline.Value();
	BufferDesc constants;
	constants.size = sizeof( ViewConstants );
	constants.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kUniform };
	auto constantsBuffer = device.CreateBuffer( constants );
	if ( !constantsBuffer )
		return foundation::MakeUnexpected( AoStatus::kDevice );
	pass->m_Constants = constantsBuffer.Value();
	return pass;
}

AmbientOcclusion::~AmbientOcclusion()
{
	(void)m_Device.WaitIdle();
	Collect( m_LastToken );
	for ( ResourceId id : { ResourceId( m_Scratch ), ResourceId( m_Constants ),
	          ResourceId( m_Pipeline ), ResourceId( m_Sampler ), ResourceId( m_Layout ) } )
	{
		if ( id.value != 0 )
			(void)m_Device.Release( id, m_LastToken );
	}
}

foundation::Expected<void, AoStatus> AmbientOcclusion::Record(
    CommandEncoder &encoder, const AoTargets &targets, const AoView &view )
{
	if ( !targets.depth.IsValid() || !targets.normalRoughness.IsValid() ||
	     !targets.output.IsValid() || targets.width == 0 || targets.height == 0 )
		return foundation::MakeUnexpected( AoStatus::kInvalidTargets );
	const auto fromClip = math::Inverse( math::Multiply( view.projection, view.view ) );
	if ( !fromClip )
		return foundation::MakeUnexpected( AoStatus::kInvalidTargets );
	if ( targets.width != m_Width || targets.height != m_Height || !m_Scratch.IsValid() )
	{
		if ( m_Scratch.IsValid() )
		{
			std::lock_guard<std::mutex> lock( m_PendingLock );
			m_Pending.push_back( m_Scratch );
		}
		BufferDesc desc;
		desc.size = std::uint64_t( targets.width ) * targets.height * 4;
		desc.usages = { ResourceUsage::kStorageRead, ResourceUsage::kStorageWrite };
		auto scratch = m_Device.CreateBuffer( desc );
		if ( !scratch )
			return foundation::MakeUnexpected( AoStatus::kDevice );
		m_Scratch = scratch.Value();
		m_Width = targets.width;
		m_Height = targets.height;
	}
	ViewConstants constants{};
	CopyMatrix( view.view, constants.view );
	CopyMatrix( *fromClip, constants.fromClip );
	constants.eye[0] = view.eye[0];
	constants.eye[1] = view.eye[1];
	constants.eye[2] = view.eye[2];
	constants.extent[0] = float( targets.width );
	constants.extent[1] = float( targets.height );
	// Pixels per world unit at view distance 1: half the height times the
	// projection's y scale.
	constants.extent[2] = 0.5f * float( targets.height ) * view.projection.rows[1].y;
	constants.params[0] = m_Params.radius;
	constants.params[1] = m_Params.falloff;
	constants.params[2] = float( m_Params.slices );
	constants.params[3] = float( m_Params.steps );
	constants.blur[0] = float( m_Params.blurRadius );

	const BindGroupEntry entries[kBindingCount] = {
	    { kView, m_Constants, 0, sizeof( ViewConstants ), {}, {} },
	    { kDepth, {}, 0, 0, targets.depth, {} }, { kNormal, {}, 0, 0, targets.normalRoughness, {} },
	    { kSampler, {}, 0, 0, {}, m_Sampler }, { kScratch, m_Scratch, 0, 0, {}, {} },
	    { kOutput, {}, 0, 0, targets.output, {} } };
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
		return foundation::MakeUnexpected( AoStatus::kDevice );
	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		m_Pending.push_back( group.Value() );
	}
	encoder.TransitionBuffer(
	    m_Constants, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( m_Constants, 0, std::as_bytes( std::span( &constants, 1 ) ) );
	encoder.TransitionBuffer(
	    m_Constants, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
	encoder.TransitionBuffer( m_Scratch, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	encoder.TransitionTexture( targets.output, targets.outputUsage, ResourceUsage::kStorageWrite );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	const std::uint32_t groupsX = ( targets.width + kGroup - 1 ) / kGroup;
	const std::uint32_t groupsY = ( targets.height + kGroup - 1 ) / kGroup;
	const std::uint32_t integrate[4] = { 0, 0, 0, 0 };
	encoder.SetDrawConstants( 0, std::as_bytes( std::span( integrate ) ) );
	encoder.Dispatch( groupsX, groupsY );
	encoder.TransitionBuffer( m_Scratch, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
	const std::uint32_t blur[4] = { 1, 0, 0, 0 };
	encoder.SetDrawConstants( 0, std::as_bytes( std::span( blur ) ) );
	encoder.Dispatch( groupsX, groupsY );
	encoder.TransitionTexture( targets.output, ResourceUsage::kStorageWrite, targets.outputUsage );
	return {};
}

void AmbientOcclusion::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( ResourceId id : m_Pending )
		(void)m_Device.Release( id, token );
	m_Pending.clear();
	m_LastToken = token;
}

} // namespace render::pass::ao
