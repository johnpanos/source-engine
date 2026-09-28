//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.lights on the device (RFC 0016 K7); see cluster_pass.h.
//
//=============================================================================//

#include "render/pass/lights/cluster_pass.h"

#include "spv/cluster_assign_spv.h"
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
	kBindingCount
};

constexpr std::uint32_t kGroupSize = 64;

template <typename T> std::span<const std::byte> Bytes( const std::vector<T> &values )
{
	return std::as_bytes( std::span<const T>( values ) );
}

} // namespace

ClusterDispatchData PrepareClusterDispatch(
    const ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights )
{
	ClusterDispatchData data;
	ClusterLightTable table = PackClusterLights( grid, lights );
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
	return data;
}

ClusterKernel::~ClusterKernel()
{
	// Collect has released every recorded dispatch's bind group behind its
	// token; the pipeline and layout follow the last one.
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
	if ( code.empty() )
		code = spirv::kClusterAssignCompute;
	std::unique_ptr<ClusterKernel> kernel( new ClusterKernel( device ) );

	BindingDesc bindings[kBindingCount];
	ReflectedBinding reflected[kBindingCount];
	for ( std::uint32_t b = 0; b < kBindingCount; ++b )
	{
		const BindingKind kind =
		    b == kParams ? BindingKind::kUniformBuffer : BindingKind::kStorageBuffer;
		bindings[b] = { b, kind, 1, { ShaderStage::kCompute } };
		reflected[b] = { static_cast<std::uint32_t>( BindGroupRole::kDraw ), b, kind };
	}
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	kernel->m_Layout = layout.Value();

	const BindGroupLayoutId layouts[kMaxBindGroups] = { {}, {}, {}, kernel->m_Layout };
	const ShaderArtifactView stage[] = { { ShaderStage::kCompute, ArtifactFormat::kSpirv,
	    std::as_bytes( code ), "main", reflected } };
	PipelineDesc desc;
	desc.kind = PipelineKind::kCompute;
	desc.stages = stage;
	desc.layouts = layouts;
	desc.debugName = "render.pass.lights.assign";
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	kernel->m_Pipeline = pipeline.Value();
	return kernel;
}

foundation::Expected<void, ClusterKernelStatus> ClusterKernel::Record(
    CommandEncoder &encoder, const ClusterBuffers &buffers )
{
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
	            std::uint64_t( std::max( buffers.indexCapacity, 1u ) ) * sizeof( std::uint32_t ),
	        {}, {} },
	};
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
	{
		++m_RecordFailures;
		return foundation::MakeUnexpected( ClusterKernelStatus::kDevice );
	}
	m_Pending.push_back( group.Value() );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch( ( buffers.froxelCount + kGroupSize - 1 ) / kGroupSize );
	return {};
}

void ClusterKernel::Collect( CompletionToken token )
{
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
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
		        const ClusterIndexHeader zero;
		        encoder.WriteBuffer( context.Buffer( resources.indices ), 0,
		            std::as_bytes( std::span( &zero, 1 ) ) );
	        } );
}

void AddClusterAssignPass( graph::GraphBuilder &builder, ClusterKernel &kernel,
    const ClusterDispatchData &data, const ClusterPassResources &resources )
{
	ClusterBuffers counts;
	counts.froxelCount = data.froxelCount;
	counts.lightRecords = static_cast<std::uint32_t>( data.lights.size() );
	counts.gridRecords = static_cast<std::uint32_t>( data.grid.size() );
	counts.indexCapacity = data.indexCapacity;
	builder.AddPass( "cluster-assign", graph::PassKind::kCompute )
	    .Read( resources.params, ResourceUsage::kUniform )
	    .Read( resources.lights, ResourceUsage::kStorageRead )
	    .Read( resources.lightSetIndex, ResourceUsage::kStorageRead )
	    .Read( resources.grid, ResourceUsage::kStorageRead )
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
		        (void)kernel.Record( context.Encoder(), buffers );
	        } );
}

} // namespace render::pass::lights
