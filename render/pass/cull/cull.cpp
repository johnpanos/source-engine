//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.cull (RFC 0016 GPU-driven submission S4); see cull.h.
//
//=============================================================================//

#include "render/pass/cull/cull.h"

#include "render/graph/executor.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <utility>

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

// compact.comp's bindings, all in the draw group.
enum CompactBinding : std::uint32_t
{
	kCompactVisibility = 0,
	kCompactTemplates,
	kCompactView,
	kCompactCommands,
	kCompactBindingCount
};

// A compute kernel of `bindingCount` storage buffers in the draw group, from
// the core artifact `source` or a suite's replacement SPIR-V.
foundation::Expected<std::pair<BindGroupLayoutId, PipelineId>, CullStatus> CreateKernel(
    IRenderDevice2 &device, const char *source, std::uint32_t bindingCount,
    const char *debugName, std::span<const std::uint32_t> code )
{
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( CullStatus::kNoCompute );
	BindingDesc bindings[kCompactBindingCount];
	for ( std::uint32_t b = 0; b < bindingCount; ++b )
		bindings[b] = { b, BindingKind::kStorageBuffer, 1, { ShaderStage::kCompute } };
	auto layout = device.CreateBindGroupLayout(
	    { BindGroupRole::kDraw, std::span<const BindingDesc>( bindings, bindingCount ) } );
	if ( !layout )
		return foundation::MakeUnexpected( CullStatus::kDevice );
	auto fail = [&]
	{
		(void)device.Release( layout.Value(), CompletionToken() );
		return foundation::MakeUnexpected( CullStatus::kDevice );
	};
	// The kernel in the device's artifact format; a suite's seeded kernel
	// replaces the core one, on a SPIR-V device only.
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !code.empty() && !artifacts.ReplaceSpirv( source, code, device.Facts().artifactFormat ) )
		return fail();
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { source }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, layout.Value() };
	recipe.debugName = debugName;
	auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return fail();
	auto pipeline = device.CreatePipeline( resolved.Value().Desc() );
	if ( !pipeline )
		return fail();
	return std::make_pair( layout.Value(), pipeline.Value() );
}

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
	auto made = CreateKernel(
	    device, "render/pass/cull/cull.comp", kBindingCount, "render.pass.cull", code );
	if ( !made )
		return foundation::MakeUnexpected( made.Error() );
	std::unique_ptr<CullKernel> kernel( new CullKernel( device ) );
	kernel->m_Layout = made.Value().first;
	kernel->m_Pipeline = made.Value().second;
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

namespace render::pass::cull
{

std::vector<device::DrawIndexedIndirectCommand> CompactReference(
    std::span<const std::uint32_t> mask, std::span<const DrawTemplate> templates,
    std::uint32_t count )
{
	std::vector<DrawIndexedIndirectCommand> out;
	for ( std::uint32_t i = 0; i < count && i / 32 < mask.size() && i < templates.size(); ++i )
	{
		if ( ( mask[i / 32] >> ( i % 32 ) ) & 1u )
		{
			const DrawTemplate &t = templates[i];
			out.push_back( { t.indexCount, 1, t.firstIndex, t.vertexOffset, i } );
		}
	}
	return out;
}

CompactKernel::CompactKernel( IRenderDevice2 &device ) : m_Device( device )
{
}

CompactKernel::~CompactKernel()
{
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

foundation::Expected<std::unique_ptr<CompactKernel>, CullStatus> CompactKernel::Create(
    IRenderDevice2 &device, std::span<const std::uint32_t> code )
{
	auto made = CreateKernel( device, "render/pass/cull/compact.comp", kCompactBindingCount,
	    "render.pass.cull.compact", code );
	if ( !made )
		return foundation::MakeUnexpected( made.Error() );
	std::unique_ptr<CompactKernel> kernel( new CompactKernel( device ) );
	kernel->m_Layout = made.Value().first;
	kernel->m_Pipeline = made.Value().second;
	return kernel;
}

foundation::Expected<void, CullStatus> CompactKernel::Record(
    CommandEncoder &encoder, const CompactBuffers &buffers )
{
	// Zero instances still write a zero draw count.
	const std::uint32_t words = std::max( MaskWords( buffers.count ), 1u );
	const std::uint32_t templates = std::max( buffers.count, 1u );
	const BindGroupEntry entries[kCompactBindingCount] = {
	    { kCompactVisibility, buffers.visibility, 0, std::uint64_t( words ) * 4, {}, {} },
	    { kCompactTemplates, buffers.templates, 0,
	        std::uint64_t( templates ) * sizeof( DrawTemplate ), {}, {} },
	    { kCompactView, buffers.view, 0, sizeof( CullView ), {}, {} },
	    { kCompactCommands, buffers.commands, 0, CommandBufferBytes( buffers.count ), {}, {} },
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
	encoder.Dispatch( 1 );
	return {};
}

void CompactKernel::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	m_LastToken = token;
}

void AddCompactPass(
    graph::GraphBuilder &builder, CompactKernel &kernel, const CompactPassResources &resources )
{
	graph::PassBuilder pass = builder.AddPass( "cull-compact", graph::PassKind::kCompute );
	if ( resources.asyncCompute )
		pass.OnQueue( graph::Queue::kAsyncCompute );
	pass.Read( resources.visibility, ResourceUsage::kStorageRead );
	pass.Read( resources.templates, ResourceUsage::kStorageRead );
	pass.Read( resources.view, ResourceUsage::kStorageRead );
	pass.Write( resources.commands, ResourceUsage::kStorageWrite );
	pass.Execute(
	    [&kernel, resources]( graph::RecordContext &context )
	    {
		    CompactBuffers buffers;
		    buffers.visibility = context.Buffer( resources.visibility );
		    buffers.templates = context.Buffer( resources.templates );
		    buffers.view = context.Buffer( resources.view );
		    buffers.commands = context.Buffer( resources.commands );
		    buffers.count = resources.count;
		    (void)kernel.Record( context.Encoder(), buffers );
	    } );
}

} // namespace render::pass::cull
