//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.lights on the device (RFC 0016 K7); see cluster_pass.h.
//
//=============================================================================//

#include "render/pass/lights/cluster_pass.h"

#include "render/shaderlib/core_artifacts.h"
#include "render/graph/executor.h"

#include <algorithm>
#include <utility>

namespace render::pass::lights
{

namespace
{

using namespace render::device;

// cluster_assign.comp's bindings, all in the draw group.
enum Binding : std::uint32_t
{
	kParams = 0,
	kLights,
	kLightSetIndex,
	kGrid,
	kFroxels,
	kIndices,
	kTree,
	kAreas,
	kBindingCount
};

constexpr std::uint64_t kTreeBytes = 64 * sizeof( math::float4 ) + 1024 * sizeof( std::uint32_t );

template <typename T> std::span<const std::byte> Bytes( const std::vector<T> &values )
{
	return std::as_bytes( std::span<const T>( values ) );
}

} // namespace

ClusterDispatchData PrepareClusterDispatch( const ClusterGrid &grid,
    std::span<const light_set::RuntimeLight> lights, std::span<const area_light::AreaLight> areas )
{
	ClusterDispatchData data;
	ClusterLightTable table = PackClusterLights( grid, lights );
	data.admission = table.stats;
	data.lightCount = static_cast<std::uint32_t>( table.lights.size() );
	data.lights = std::move( table.lights );
	data.lightSetIndex = std::move( table.lightSetIndex );
	if ( data.lights.empty() )
	{
		data.lights.resize( 1 );
		data.lightSetIndex.resize( 1 );
	}
	data.grid = PackClusterGrid( grid );
	data.froxelCount = grid.FroxelCount();
	data.indexCapacity = grid.limits.maxLightIndices;
	data.params.grid[0] = grid.tilesX;
	data.params.grid[1] = grid.tilesY;
	data.params.grid[2] = grid.slices;
	data.params.grid[3] = data.lightCount;
	data.params.limits[0] = grid.limits.maxLightsPerFroxel;
	data.params.limits[1] = grid.limits.maxLightIndices;
	data.params.limits[2] = static_cast<std::uint32_t>( areas.size() );
	for ( const auto &area : areas )
	{
		const auto transform = [&]( const float *p, float w )
		{
			math::float4 result{};
			for ( int r = 0; r < 3; ++r )
			{
				const auto &row = grid.view.rows[r];
				( &result.x )[r] =
				    static_cast<float>( double( row.x ) * p[0] + double( row.y ) * p[1] +
				                        double( row.z ) * p[2] + double( row.w ) * w );
			}
			return result;
		};
		ClusterAreaGpu record{ transform( area.rect.center, 1 ), transform( area.rect.halfU, 0 ),
		    transform( area.rect.halfV, 0 ) };
		record.centerReach.w = area.reach;
		data.areas.push_back( record );
	}
	if ( data.areas.empty() )
		data.areas.resize( 1 );
	return data;
}

ClusterDispatchData PrepareSurfaceClusterDispatch( const ClusterGrid &grid,
    std::span<const light_set::RuntimeLight> lights, std::span<const area_light::AreaLight> areas )
{
	auto data = PrepareClusterDispatch( grid, lights, areas );
	data.params.limits[0] = std::max( data.lightCount, 1u );
	data.indexCapacity = std::max( data.froxelCount * data.lightCount, 1u );
	data.params.limits[1] = data.indexCapacity;
	return data;
}

ClusterKernel::~ClusterKernel()
{
	// Collect has released every recorded dispatch's bind group behind its
	// token; the pipeline and layout follow the last one.
	Collect( m_LastToken );
	if ( m_BuildPipeline.IsValid() )
		(void)m_Device.Release( m_BuildPipeline, m_LastToken );
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

foundation::Expected<std::unique_ptr<ClusterKernel>, ClusterKernelStatus> ClusterKernel::Create(
    IRenderDevice2 &device, std::span<const std::uint32_t> code )
{
	if ( !device.Facts().capabilities.Has( Capability::kCompute ) )
		return foundation::MakeUnexpected( ClusterKernelStatus::kNoCompute );
	std::unique_ptr<ClusterKernel> kernel( new ClusterKernel( device ) );

	BindingDesc bindings[kBindingCount];
	for ( std::uint32_t b = 0; b < kBindingCount; ++b )
	{
		const BindingKind kind =
		    b == kParams ? BindingKind::kUniformBuffer : BindingKind::kStorageBuffer;
		bindings[b] = { b, kind, 1, { ShaderStage::kCompute } };
	}
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	kernel->m_Layout = layout.Value();

	// The kernel in the device's artifact format (RFC 0016 K10); a suite's
	// seeded kernel replaces the core one, on a SPIR-V device only.
	constexpr const char *kKernel = "render/pass/lights/cluster_assign.comp";
	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !code.empty() && !artifacts.ReplaceSpirv( kKernel, code, device.Facts().artifactFormat ) )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kKernel }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, kernel->m_Layout };
	recipe.debugName = "render.pass.lights.assign";
	auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	auto pipeline = device.CreatePipeline( resolved.Value().Desc() );
	if ( !pipeline )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	kernel->m_Pipeline = pipeline.Value();
	recipe = shaderlib::CoreRecipe(
	    { "render/pass/lights/cluster_build.comp" }, PipelineKind::kCompute );
	recipe.layouts = { {}, {}, {}, kernel->m_Layout };
	recipe.debugName = "render.pass.lights.morton-bvh";
	resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	pipeline = device.CreatePipeline( resolved.Value().Desc() );
	if ( !pipeline )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	kernel->m_BuildPipeline = pipeline.Value();

	return kernel;
}

foundation::Expected<void, ClusterKernelStatus> ClusterKernel::Record(
    CommandEncoder &encoder, const ClusterBuffers &buffers )
{
	if ( buffers.lightRecords > 1024 || buffers.areaCount > 64 )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	if ( buffers.froxelCount == 0 )
		return {};
	const std::uint32_t lights = std::max( buffers.lightRecords, 1u );
	const BindGroupEntry entries[kBindingCount] = {
	    { kParams, buffers.params, 0, sizeof( ClusterParamsGpu ), {}, {} },
	    { kLights, buffers.lights, 0, std::uint64_t( lights ) * sizeof( ClusterLightGpu ), {}, {} },
	    { kLightSetIndex, buffers.lightSetIndex, 0,
	        std::uint64_t( lights ) * sizeof( std::uint32_t ), {}, {} },
	    { kGrid, buffers.grid, 0, std::uint64_t( buffers.gridRecords ) * sizeof( math::float4 ), {},
	        {} },
	    { kFroxels, buffers.froxels, 0,
	        std::uint64_t( buffers.froxelCount ) * sizeof( FroxelRange ), {}, {} },
	    { kIndices, buffers.indices, 0,
	        sizeof( ClusterIndexHeader ) +
	            std::uint64_t( std::max( buffers.indexCapacity, 1u ) ) * sizeof( std::uint32_t ) +
	            ( buffers.areaCount ? std::uint64_t( buffers.froxelCount ) * 8 : 0 ),
	        {}, {} },
	    { kTree, buffers.tree, 0, kTreeBytes, {}, {} },
	    { kAreas, buffers.areas, 0,
	        std::uint64_t( std::max( buffers.areaCount, 1u ) ) * sizeof( ClusterAreaGpu ), {}, {} },
	};
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.BeginLabel( "cluster Morton sort and BVH" );
	encoder.SetPipeline( m_BuildPipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch( 1 );
	encoder.EndLabel();
	encoder.TransitionBuffer(
	    buffers.tree, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
	encoder.BeginLabel( "cluster BVH assignment" );
	encoder.SetPipeline( m_Pipeline );
	encoder.Dispatch( buffers.froxelCount );
	encoder.EndLabel();
	return {};
}

foundation::Expected<ClusterBuffers, ClusterKernelStatus> ClusterKernel::RecordView(
    CommandEncoder &encoder, const ClusterDispatchData &data )
{
	ClusterBuffers out;
	if ( data.admission.lightsOverCapacity || data.lightCount > 1024 || data.params.limits[2] > 64 )
		return foundation::MakeUnexpected( ClusterKernelStatus::kCapacity );
	const auto start = m_Buffers.size();
	const auto buffer =
	    [&]( std::uint64_t size, ResourceUsage usage, std::span<const std::byte> bytes = {} )
	{
		BufferDesc desc;
		desc.size = std::max<std::uint64_t>( size, 4 );
		desc.usages = { ResourceUsage::kCopySource, ResourceUsage::kCopyDestination,
		    ResourceUsage::kStorageRead, ResourceUsage::kStorageWrite, ResourceUsage::kUniform };
		desc.debugName = "cluster view";
		auto made = m_Device.CreateBuffer( desc );
		if ( !made )
			return BufferId();
		m_Buffers.push_back( made.Value() );
		encoder.TransitionBuffer(
		    made.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		if ( !bytes.empty() )
			encoder.WriteBuffer( made.Value(), 0, bytes );
		encoder.TransitionBuffer( made.Value(), ResourceUsage::kCopyDestination, usage );
		return made.Value();
	};
	out.params = buffer( sizeof( data.params ), ResourceUsage::kUniform,
	    std::as_bytes( std::span( &data.params, 1 ) ) );
	out.lights =
	    buffer( Bytes( data.lights ).size(), ResourceUsage::kStorageRead, Bytes( data.lights ) );
	out.lightSetIndex = buffer( Bytes( data.lightSetIndex ).size(), ResourceUsage::kStorageRead,
	    Bytes( data.lightSetIndex ) );
	out.grid = buffer( Bytes( data.grid ).size(), ResourceUsage::kStorageRead, Bytes( data.grid ) );
	out.tree = buffer( kTreeBytes, ResourceUsage::kStorageWrite );
	out.areas =
	    buffer( Bytes( data.areas ).size(), ResourceUsage::kStorageRead, Bytes( data.areas ) );
	out.froxels = buffer( data.FroxelBytes(), ResourceUsage::kStorageWrite );
	ClusterIndexHeader zero;
	zero.reserved = data.params.limits[2] ? data.indexCapacity + 1 : 0;
	out.indices = buffer(
	    data.IndexBytes(), ResourceUsage::kStorageWrite, std::as_bytes( std::span( &zero, 1 ) ) );
	if ( m_Buffers.size() != start + 8 )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	out.areaCount = data.params.limits[2];
	out.froxelCount = data.froxelCount;
	out.lightRecords = data.lightCount;
	out.gridRecords = static_cast<std::uint32_t>( data.grid.size() );
	out.indexCapacity = data.indexCapacity;
	auto result = Record( encoder, out );
	if ( !result )
		return foundation::MakeUnexpected( result.Error() );
	encoder.TransitionBuffer(
	    out.froxels, ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead );
	encoder.TransitionBuffer(
	    out.indices, ResourceUsage::kStorageWrite, ResourceUsage::kStorageRead );
	return out;
}

void ClusterKernel::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	for ( BufferId buffer : m_Buffers )
		(void)m_Device.Release( buffer, token );
	m_Buffers.clear();
	m_LastToken = token;
}

ClusterPassResources CreateClusterResources(
    graph::GraphBuilder &builder, const ClusterDispatchData &data )
{
	const auto buffer = [&]( const char *name, std::uint64_t size )
	{
		BufferDesc desc;
		desc.size = std::max<std::uint64_t>( size, 4 );
		return builder.CreateBuffer( name, desc );
	};
	ClusterPassResources resources;
	resources.params = buffer( "cluster-params", sizeof( ClusterParamsGpu ) );
	resources.lights = buffer( "cluster-lights", data.lights.size() * sizeof( ClusterLightGpu ) );
	resources.lightSetIndex =
	    buffer( "cluster-light-set-index", data.lightSetIndex.size() * sizeof( std::uint32_t ) );
	resources.grid = buffer( "cluster-grid", data.grid.size() * sizeof( math::float4 ) );
	resources.froxels = buffer( "cluster-froxels", data.FroxelBytes() );
	resources.indices = buffer( "cluster-indices", data.IndexBytes() );
	resources.tree = buffer( "cluster-tree", kTreeBytes );
	resources.areas = buffer( "cluster-areas", data.areas.size() * sizeof( ClusterAreaGpu ) );
	return resources;
}

void AddClusterUploadPass( graph::GraphBuilder &builder,
    std::shared_ptr<const ClusterDispatchData> data, const ClusterPassResources &resources )
{
	builder.AddPass( "cluster-upload", graph::PassKind::kCopy )
	    .Write( resources.params, ResourceUsage::kCopyDestination )
	    .Write( resources.lights, ResourceUsage::kCopyDestination )
	    .Write( resources.lightSetIndex, ResourceUsage::kCopyDestination )
	    .Write( resources.grid, ResourceUsage::kCopyDestination )
	    .Write( resources.areas, ResourceUsage::kCopyDestination )
	    .Write( resources.indices, ResourceUsage::kCopyDestination )
	    .Execute(
	        [data = std::move( data ), resources]( graph::RecordContext &context )
	        {
		        CommandEncoder &encoder = context.Encoder();
		        encoder.WriteBuffer( context.Buffer( resources.params ), 0,
		            std::as_bytes( std::span( &data->params, 1 ) ) );
		        encoder.WriteBuffer( context.Buffer( resources.lights ), 0, Bytes( data->lights ) );
		        encoder.WriteBuffer(
		            context.Buffer( resources.lightSetIndex ), 0, Bytes( data->lightSetIndex ) );
		        encoder.WriteBuffer( context.Buffer( resources.grid ), 0, Bytes( data->grid ) );
		        ClusterIndexHeader zero;
		        zero.reserved = data->params.limits[2] ? data->indexCapacity + 1 : 0;
		        encoder.WriteBuffer( context.Buffer( resources.areas ), 0, Bytes( data->areas ) );
		        encoder.WriteBuffer( context.Buffer( resources.indices ), 0,
		            std::as_bytes( std::span( &zero, 1 ) ) );
	        } );
}

void AddClusterAssignPass( graph::GraphBuilder &builder, ClusterKernel &kernel,
    const ClusterDispatchData &data, const ClusterPassResources &resources )
{
	ClusterBuffers counts;
	counts.areaCount = data.params.limits[2];
	counts.froxelCount = data.froxelCount;
	counts.lightRecords = static_cast<std::uint32_t>( data.lights.size() );
	counts.gridRecords = static_cast<std::uint32_t>( data.grid.size() );
	counts.indexCapacity = data.indexCapacity;
	builder.AddPass( "cluster-assign", graph::PassKind::kCompute )
	    .Read( resources.params, ResourceUsage::kUniform )
	    .Read( resources.lights, ResourceUsage::kStorageRead )
	    .Read( resources.lightSetIndex, ResourceUsage::kStorageRead )
	    .Read( resources.grid, ResourceUsage::kStorageRead )
	    .Read( resources.areas, ResourceUsage::kStorageRead )
	    .Write( resources.tree, ResourceUsage::kStorageWrite )
	    .Write( resources.froxels, ResourceUsage::kStorageWrite )
	    .Write( resources.indices, ResourceUsage::kStorageWrite )
	    .Execute(
	        [&kernel, resources, counts]( graph::RecordContext &context )
	        {
		        ClusterBuffers buffers = counts;
		        buffers.params = context.Buffer( resources.params );
		        buffers.lights = context.Buffer( resources.lights );
		        buffers.lightSetIndex = context.Buffer( resources.lightSetIndex );
		        buffers.grid = context.Buffer( resources.grid );
		        buffers.froxels = context.Buffer( resources.froxels );
		        buffers.indices = context.Buffer( resources.indices );
		        buffers.tree = context.Buffer( resources.tree );
		        buffers.areas = context.Buffer( resources.areas );
		        (void)kernel.Record( context.Encoder(), buffers );
	        } );
}

} // namespace render::pass::lights
