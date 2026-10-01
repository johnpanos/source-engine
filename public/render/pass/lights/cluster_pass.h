//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.lights on the device (RFC 0016 K7, render.lights.v1):
//			cluster_assign.comp as a render.graph compute pass.
//
// GPU Morton sort, 32-way BVH and one 32-thread group per froxel.
// Lists retain ascending light-set order. Capacity overflow is counted; which
// froxels exhaust the global index budget depends on scheduling.
// RecordView owns its buffers until Collect receives the LAST CONSUMER token,
// not merely the assignment submission token. No CPU assignment or readback.
//
//=============================================================================//

#ifndef RENDER_PASS_LIGHTS_CLUSTER_PASS_H
#define RENDER_PASS_LIGHTS_CLUSTER_PASS_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/graph/graph_builder.h"
#include "render/pass/lights/clusters.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace render::pass::lights
{

// std140 parameters (binding 0).
struct ClusterParamsGpu
{
	std::uint32_t grid[4] = {};   // tilesX, tilesY, slices, packed light count
	std::uint32_t limits[4] = {}; // maxLightsPerFroxel, maxLightIndices, 0, 0
};

// The index list's header (binding 5), before the indices.
struct ClusterIndexHeader
{
	// Indices the froxels asked for after their per-froxel limit; the list
	// holds min( requested, maxLightIndices ) of them.
	std::uint32_t requested = 0;
	std::uint32_t froxelsOverflowed = 0;
	std::uint32_t assignmentsDropped = 0;
	// CPU surface lists may append area masks: uint offset after this header + 1.
	// The compute point/spot assignment leaves this zero.
	std::uint32_t reserved = 0;
};

static_assert( sizeof( ClusterParamsGpu ) == 32 && sizeof( ClusterIndexHeader ) == 16 &&
               sizeof( ClusterLightGpu ) == 32 && sizeof( FroxelRange ) == 8 );

// One view's dispatch inputs. Each list holds at least one record, so every
// binding has a size; lightCount says how many are real.
struct ClusterAreaGpu
{
	math::float4 centerReach;
	math::float4 halfU;
	math::float4 halfV;
};

struct ClusterDispatchData
{
	ClusterParamsGpu params;
	ClusterStats admission;
	std::vector<ClusterLightGpu> lights;
	std::vector<std::uint32_t> lightSetIndex;
	std::vector<math::float4> grid;
	std::vector<ClusterAreaGpu> areas;
	std::uint32_t froxelCount = 0;
	std::uint32_t lightCount = 0;
	std::uint32_t indexCapacity = 0;

	std::uint64_t FroxelBytes() const
	{
		return std::uint64_t( froxelCount ) * sizeof( FroxelRange );
	}
	std::uint64_t IndexBytes() const
	{
		return sizeof( ClusterIndexHeader ) +
		       std::uint64_t( indexCapacity ) * sizeof( std::uint32_t ) +
		       ( params.limits[2] ? std::uint64_t( froxelCount ) * 8 : 0 );
	}
};

ClusterDispatchData PrepareClusterDispatch( const ClusterGrid &grid,
    std::span<const light_set::RuntimeLight> lights,
    std::span<const area_light::AreaLight> areas = {} );

// Surface consumers reserve for every admitted light in every froxel, so neither
// per-froxel nor global index capacity can silently reduce the rendered image.
ClusterDispatchData PrepareSurfaceClusterDispatch( const ClusterGrid &grid,
    std::span<const light_set::RuntimeLight> lights,
    std::span<const area_light::AreaLight> areas = {} );

// The dispatch's buffers: params as a uniform buffer (kUniform); lights,
// light-set indices and grid as read-only storage (kStorageRead); froxels and
// indices as written storage (kStorageWrite).
struct ClusterBuffers
{
	device::BufferId params;
	device::BufferId lights;
	device::BufferId lightSetIndex;
	device::BufferId grid;
	device::BufferId froxels;
	device::BufferId indices;
	device::BufferId tree;
	device::BufferId areas;
	std::uint32_t areaCount = 0;
	std::uint32_t froxelCount = 0;
	std::uint32_t lightRecords = 1; // records bound (at least 1)
	std::uint32_t gridRecords = 0;
	std::uint32_t indexCapacity = 0;
};

enum class ClusterKernelStatus : std::uint8_t
{
	kNoCompute = 1, // the device lacks Capability::kCompute
	kCapacity,
	kDevice // the device refused the layout, pipeline or bind group
};

class ClusterKernel
{
public:
	// `code` is the kernel's SPIR-V; the default is cluster_assign.comp. The
	// suite passes its seeded defective variants here.
	static foundation::Expected<std::unique_ptr<ClusterKernel>, ClusterKernelStatus> Create(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> code = {} );
	~ClusterKernel();
	ClusterKernel( const ClusterKernel & ) = delete;
	ClusterKernel &operator=( const ClusterKernel & ) = delete;

	// Records one dispatch. The buffers must already be in their usages; the
	// index header must be zero. The bind group stays live until Collect.
	foundation::Expected<void, ClusterKernelStatus> Record(
	    device::CommandEncoder &encoder, const ClusterBuffers &buffers );
	// Allocates/uploads inputs and records assignment on the consumer encoder.
	// Returned outputs are in kStorageRead. Collect retires the owned buffers.
	foundation::Expected<ClusterBuffers, ClusterKernelStatus> RecordView(
	    device::CommandEncoder &encoder, const ClusterDispatchData &data );
	// Releases the bind groups of recorded dispatches behind `token`.
	void Collect( device::CompletionToken token );
	// Dispatches a graph pass could not record; the owner checks this after
	// execution and fails the frame when it rose.
	std::uint32_t RecordFailures() const { return m_RecordFailures; }

private:
	explicit ClusterKernel( device::IRenderDevice2 &device ) : m_Device( device ) {}
	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_Layout;
	device::PipelineId m_Pipeline;
	device::PipelineId m_BuildPipeline;
	std::vector<device::BufferId> m_Buffers;
	std::vector<device::BindGroupId> m_Pending;
	device::CompletionToken m_LastToken;
	std::uint32_t m_RecordFailures = 0;
};

// Graph resources of one view's assignment.
struct ClusterPassResources
{
	graph::ResourceRef params;
	graph::ResourceRef lights;
	graph::ResourceRef lightSetIndex;
	graph::ResourceRef grid;
	graph::ResourceRef froxels;
	graph::ResourceRef indices;
	graph::ResourceRef tree;
	graph::ResourceRef areas;
};

// Transient buffers sized for `data`, declared on `builder`.
ClusterPassResources CreateClusterResources(
    graph::GraphBuilder &builder, const ClusterDispatchData &data );

// A copy pass writing the inputs and zeroing the index header. `data` is
// kept alive by the pass until the graph is destroyed.
void AddClusterUploadPass( graph::GraphBuilder &builder,
    std::shared_ptr<const ClusterDispatchData> data, const ClusterPassResources &resources );

// The compute pass. The kernel must outlive the graph's execution; a
// dispatch it cannot record counts in RecordFailures.
void AddClusterAssignPass( graph::GraphBuilder &builder, ClusterKernel &kernel,
    const ClusterDispatchData &data, const ClusterPassResources &resources );

} // namespace render::pass::lights

#endif // RENDER_PASS_LIGHTS_CLUSTER_PASS_H
