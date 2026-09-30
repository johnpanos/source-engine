//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.bounce: the projected lights' one bounce into the
//			probe volume's layout; see public/render/pass/bounce/bounce.h.
//
//=============================================================================//

#include "render/pass/bounce/bounce.h"

#include "render/shaderlib/core_artifacts.h"

#include <cmath>
#include <cstring>

namespace render::pass::bounce
{

using namespace render::device;

namespace
{

constexpr const char *kSource = "render/pass/bounce/bounce.comp";

struct Params
{
	projected_light::LightGpu projector;
	float fromClip[16];
	std::uint32_t sizes[4];
	float frustum[4];
};
static_assert( sizeof( Params ) == 112 + 64 + 32, "bounce.comp's BounceParams" );

enum Binding : std::uint32_t
{
	kParams = 0,
	kProbeAtlas = 1,
	kLinear = 2,
	kProbeGrids = 3,
	kPoint = 4,
	kMapAlbedo = 5,
	kMapDepth = 6,
	kCookies = 7,
	kPatches = 8,
	kAtlas = 9,
	kBindingCount = 10
};

} // namespace

foundation::Expected<std::unique_ptr<ProjectorBounce>, BounceStatus> ProjectorBounce::Create(
    IRenderDevice2 &device )
{
	return CreateWithProgram( device, {} );
}

foundation::Expected<std::unique_ptr<ProjectorBounce>, BounceStatus>
ProjectorBounce::CreateWithProgram( IRenderDevice2 &device, std::span<const std::uint32_t> spirv )
{
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( BounceStatus::kDevice );
	std::unique_ptr<ProjectorBounce> pass( new ProjectorBounce( device ) );
	const ShaderStageSet compute{ ShaderStage::kCompute };
	const BindingDesc bindings[kBindingCount] = {
	    { kParams, BindingKind::kUniformBuffer, 1, compute },
	    { kProbeAtlas, BindingKind::kSampledTexture, 1, compute },
	    { kLinear, BindingKind::kSampler, 1, compute },
	    { kProbeGrids, BindingKind::kSampledTexture, 1, compute },
	    { kPoint, BindingKind::kSampler, 1, compute },
	    { kMapAlbedo, BindingKind::kSampledTexture, 1, compute },
	    { kMapDepth, BindingKind::kSampledTexture, 1, compute },
	    { kCookies, BindingKind::kSampledTexture, 1, compute },
	    { kPatches, BindingKind::kStorageBuffer, 1, compute },
	    { kAtlas, BindingKind::kStorageTexture, 1, compute } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( BounceStatus::kDevice );
	pass->m_Layout = layout.Value();
	SamplerDesc linear;
	linear.address = AddressMode::kClampToEdge;
	SamplerDesc point = linear;
	point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
	auto linearSampler = device.CreateSampler( linear );
	auto pointSampler = device.CreateSampler( point );
	if ( !linearSampler || !pointSampler )
		return foundation::MakeUnexpected( BounceStatus::kDevice );
	pass->m_Linear = linearSampler.Value();
	pass->m_Point = pointSampler.Value();
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kSource }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, pass->m_Layout };
	recipe.debugName = kSource;
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !spirv.empty() &&
	     !artifacts.ReplaceSpirv( kSource, spirv, device.Facts().artifactFormat ) )
		return foundation::MakeUnexpected( BounceStatus::kDevice );
	auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected( BounceStatus::kDevice );
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = 16;
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( BounceStatus::kDevice );
	pass->m_Gather = pipeline.Value();
	return pass;
}

ProjectorBounce::~ProjectorBounce()
{
	(void)m_Device.WaitIdle();
	Collect( m_LastToken );
	for ( ResourceId id : { ResourceId( m_Gather ), ResourceId( m_Linear ), ResourceId( m_Point ),
	          ResourceId( m_Layout ) } )
	{
		if ( id.value != 0 )
			(void)m_Device.Release( id, m_LastToken );
	}
}

foundation::Expected<void, BounceStatus> ProjectorBounce::Record(
    CommandEncoder &encoder, const BounceInputs &inputs )
{
	if ( !inputs.probeAtlas.IsValid() || !inputs.probeGrids.IsValid() ||
	     !inputs.output.IsValid() || !inputs.cookies.IsValid() || inputs.gridCount == 0 )
		return foundation::MakeUnexpected( BounceStatus::kInvalidInputs );
	auto keep = [&]( ResourceId id )
	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		m_Pending.push_back( id );
	};
	auto makeBuffer = [&]( std::uint64_t size, std::initializer_list<ResourceUsage> usages )
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = UsageSet( usages );
		auto made = m_Device.CreateBuffer( desc );
		return made ? made.Value() : BufferId();
	};
	auto step = [&]( std::uint32_t mode )
	{
		const std::uint32_t values[4] = { mode, 0, 0, 0 };
		encoder.SetDrawConstants( 0, std::as_bytes( std::span( values ) ) );
	};
	encoder.TransitionTexture( inputs.output, inputs.outputUsage, ResourceUsage::kStorageWrite );
	encoder.SetPipeline( m_Gather );
	bool first = true;
	for ( const ReflectiveShadowMap &map : inputs.maps )
	{
		if ( !map.albedo.IsValid() || !map.depth.IsValid() || map.size == 0 )
			return foundation::MakeUnexpected( BounceStatus::kInvalidInputs );
		const auto fromClip = math::Inverse( map.viewProjection );
		if ( !fromClip )
			return foundation::MakeUnexpected( BounceStatus::kInvalidInputs );
		const BufferId patches = makeBuffer( std::uint64_t( map.size ) * map.size * 48,
		    { ResourceUsage::kStorageRead, ResourceUsage::kStorageWrite } );
		if ( !patches.IsValid() )
			return foundation::MakeUnexpected( BounceStatus::kDevice );
		keep( patches );
		for ( std::uint32_t grid = 0; grid < inputs.gridCount; ++grid )
		{
			Params params{};
			params.projector = map.light;
			for ( int r = 0; r < 4; ++r )
			{
				params.fromClip[r * 4 + 0] = fromClip->rows[r].x;
				params.fromClip[r * 4 + 1] = fromClip->rows[r].y;
				params.fromClip[r * 4 + 2] = fromClip->rows[r].z;
				params.fromClip[r * 4 + 3] = fromClip->rows[r].w;
			}
			params.sizes[0] = map.size;
			params.sizes[1] = grid;
			params.sizes[2] = inputs.maxProbesPerGrid;
			params.frustum[0] = map.light.frustum[0];
			params.frustum[1] = map.light.frustum[1];
			const BufferId constants = makeBuffer(
			    sizeof( Params ), { ResourceUsage::kCopyDestination, ResourceUsage::kUniform } );
			if ( !constants.IsValid() )
				return foundation::MakeUnexpected( BounceStatus::kDevice );
			keep( constants );
			const BindGroupEntry entries[kBindingCount] = {
			    { kParams, constants, 0, sizeof( Params ), {}, {} },
			    { kProbeAtlas, {}, 0, 0, inputs.probeAtlas, {} },
			    { kLinear, {}, 0, 0, {}, m_Linear },
			    { kProbeGrids, {}, 0, 0, inputs.probeGrids, {} },
			    { kPoint, {}, 0, 0, {}, m_Point },
			    { kMapAlbedo, {}, 0, 0, map.albedo, {} },
			    { kMapDepth, {}, 0, 0, map.depth, {} },
			    { kCookies, {}, 0, 0, inputs.cookies, {} },
			    { kPatches, patches, 0, 0, {}, {} },
			    { kAtlas, {}, 0, 0, inputs.output, {} } };
			auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
			if ( !group )
				return foundation::MakeUnexpected( BounceStatus::kDevice );
			keep( group.Value() );
			encoder.TransitionBuffer(
			    constants, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( constants, 0, std::as_bytes( std::span( &params, 1 ) ) );
			encoder.TransitionBuffer(
			    constants, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
			encoder.SetPipeline( m_Gather );
			encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
			if ( first )
			{
				step( 0 );
				encoder.Dispatch( ( inputs.probeAtlasDesc.width * inputs.probeAtlasDesc.height + 63 ) / 64,
				    1 );
				encoder.TransitionTexture(
				    inputs.output, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
				first = false;
			}
			if ( grid == 0 )
			{
				encoder.TransitionBuffer(
				    patches, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
				step( 1 );
				encoder.Dispatch( ( map.size * map.size + 63 ) / 64, 1 );
				encoder.TransitionBuffer(
				    patches, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
			}
			step( 2 );
			encoder.Dispatch( ( inputs.maxProbesPerGrid + 63 ) / 64, 64 );
			encoder.TransitionTexture(
			    inputs.output, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
		}
	}
	if ( first )
	{
		// No projector: the atlas is cleared (no bounce), with the pass's
		// own bind group.
		return foundation::MakeUnexpected( BounceStatus::kInvalidInputs );
	}
	encoder.TransitionTexture( inputs.output, ResourceUsage::kStorageWrite, inputs.outputUsage );
	return {};
}

void ProjectorBounce::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( ResourceId id : m_Pending )
		(void)m_Device.Release( id, token );
	m_Pending.clear();
	m_LastToken = token;
}

} // namespace render::pass::bounce
