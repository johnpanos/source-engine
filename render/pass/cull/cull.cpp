//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.cull (RFC 0016 GPU-driven submission S4); see cull.h.
//
//=============================================================================//

#include "render/pass/cull/cull.h"

#include "render/graph/executor.h"

#include "render/shaderlib/core_artifacts.h"

namespace render::pass::cull
{

namespace
{

using namespace render::device;

// cull.comp's bindings, all in the draw group.
enum Binding : std::uint32_t
{
	kInstances = 0,
	kView,
	kVisibility,
	kBindingCount
};

static_assert( static_cast<int>( math::FrustumPlane::kCount ) == 6 );

} // namespace

std::vector<CullInstance> PackInstances( const scene::SceneSnapshot &snapshot )
{
	std::vector<CullInstance> out( snapshot.instances.size() );
	for ( std::size_t i = 0; i < out.size(); ++i )
	{
		const scene::MeshInstance &instance = snapshot.instances[i];
		const math::Aabb &bounds = instance.worldBounds;
		out[i].min[0] = bounds.min.x;
		out[i].min[1] = bounds.min.y;
		out[i].min[2] = bounds.min.z;
		out[i].max[0] = bounds.max.x;
		out[i].max[1] = bounds.max.y;
		out[i].max[2] = bounds.max.z;
		out[i].viewMask = instance.desc.viewMask;
	}
	return out;
}

CullView PackView( const scene::SceneView &view, std::uint32_t count )
{
	CullView out;
	for ( int p = 0; p < 6; ++p )
	{
		const math::Plane &plane = view.frustum.planes[p];
		out.planes[p][0] = plane.normal.x;
		out.planes[p][1] = plane.normal.y;
		out.planes[p][2] = plane.normal.z;
		out.planes[p][3] = plane.d;
	}
	out.viewBit = view.desc.viewBit;
	out.count = count;
	return out;
}

CullKernel::CullKernel( IRenderDevice2 &device ) : m_Device( device )
{
}

CullKernel::~CullKernel()
{
	// Collect has released every recorded dispatch's bind group behind its
	// token; the pipeline and layout follow the last one.
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

foundation::Expected<std::unique_ptr<CullKernel>, CullStatus> CullKernel::Create(
    IRenderDevice2 &device, std::span<const std::uint32_t> code )
{
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( CullStatus::kNoCompute );
	std::unique_ptr<CullKernel> kernel( new CullKernel( device ) );

	BindingDesc bindings[kBindingCount];
	for ( std::uint32_t b = 0; b < kBindingCount; ++b )
		bindings[b] = { b, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( CullStatus::kDevice );
	kernel->m_Layout = layout.Value();

	// The kernel in the device's artifact format; a suite's seeded kernel
	// replaces the core one, on a SPIR-V device only.
	constexpr const char *kKernel = "render/pass/cull/cull.comp";
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !code.empty() && !artifacts.ReplaceSpirv( kKernel, code, device.Facts().artifactFormat ) )
		return foundation::MakeUnexpected( CullStatus::kDevice );
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kKernel }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, kernel->m_Layout };
	recipe.debugName = "render.pass.cull";
	auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected( CullStatus::kDevice );
	auto pipeline = device.CreatePipeline( resolved.Value().Desc() );
	if ( !pipeline )
		return foundation::MakeUnexpected( CullStatus::kDevice );
	kernel->m_Pipeline = pipeline.Value();
	return kernel;
}

foundation::Expected<void, CullStatus> CullKernel::Record(
    CommandEncoder &encoder, const CullBuffers &buffers )
{
	if ( buffers.count == 0 )
		return {};
	const BindGroupEntry entries[kBindingCount] = {
	    { kInstances, buffers.instances, 0, std::uint64_t( buffers.count ) * sizeof( CullInstance ),
	        {}, {} },
	    { kView, buffers.view, 0, sizeof( CullView ), {}, {} },
	    { kVisibility, buffers.visibility, 0,
	        std::uint64_t( MaskWords( buffers.count ) ) * sizeof( std::uint32_t ), {}, {} },
	};
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( CullStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch( ( MaskWords( buffers.count ) + 63 ) / 64 );
	return {};
}

void CullKernel::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

void AddCullPass(
    graph::GraphBuilder &builder, CullKernel &kernel, const CullPassResources &resources )
{
	graph::PassBuilder pass = builder.AddPass( "cull", graph::PassKind::kCompute );
	if ( resources.asyncCompute )
		pass.OnQueue( graph::Queue::kAsyncCompute );
	pass.Read( resources.instances, ResourceUsage::kStorageRead );
	pass.Read( resources.view, ResourceUsage::kStorageRead );
	pass.Write( resources.visibility, ResourceUsage::kStorageWrite );
	pass.Execute(
	    [&kernel, resources]( graph::RecordContext &context )
	    {
		    CullBuffers buffers;
		    buffers.instances = context.Buffer( resources.instances );
		    buffers.view = context.Buffer( resources.view );
		    buffers.visibility = context.Buffer( resources.visibility );
		    buffers.count = resources.count;
		    (void)kernel.Record( context.Encoder(), buffers );
	    } );
}

} // namespace render::pass::cull
