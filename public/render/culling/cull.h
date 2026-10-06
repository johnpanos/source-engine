//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.culling (RFC 0016 GPU-driven submission S4): frustum and
//			view-mask culling of scene instances on the GPU.
//
//			The kernel (render/culling/cull.comp) decides, per instance,
//			exactly what render::scene's CPU culler decides before its
//			visibility provider: kept when the view bit is in the instance's
//			mask (or the view's bit is 32 or more) and its world bounds are
//			not empty and not wholly behind any frustum plane. It writes one
//			visibility bit per instance (bit i % 32 of word i / 32). The CPU
//			culler (scene::BuildDrawList) is the oracle; RFC 0003's
//			placement rule decides which one the product runs.
//
//			The records below are the std430 layouts cull.comp reads.
//
//=============================================================================//

#ifndef RENDER_CULLING_CULL_H
#define RENDER_CULLING_CULL_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/device/encoder.h"
#include "render/graph/graph_builder.h"
#include "render/math/matrix.h"
#include "render/scene/snapshot.h"
#include "render/scene/view.h"

#include <cstdint>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace render::culling
{

struct CullInstance // 32 bytes
{
	float min[3] = {};
	std::uint32_t viewMask = 0;
	float max[3] = {};
	std::uint32_t reserved = 0;
};

struct CullView // 112 bytes
{
	float planes[6][4] = {}; // normal xyz, d: kept while normal . p + d >= 0
	std::uint32_t viewBit = 0;
	std::uint32_t count = 0; // instances
	std::uint32_t reserved[2] = {};
};

static_assert( sizeof( CullInstance ) == 32 && sizeof( CullView ) == 112 );

// The instances' world bounds and masks, in snapshot order.
std::vector<CullInstance> PackInstances( const scene::SceneSnapshot &snapshot );
CullView PackView( const scene::SceneView &view, std::uint32_t count );
// The same from a frustum and view bit (32: every instance's mask matches).
CullView PackView( const math::Frustum &frustum, std::uint32_t viewBit, std::uint32_t count );
// Words of a visibility mask for `count` instances.
constexpr std::uint32_t MaskWords( std::uint32_t count )
{
	return ( count + 31 ) / 32;
}

struct CullBuffers
{
	device::BufferId instances;  // CullInstance[count], storage read
	device::BufferId view;       // one CullView, storage read
	device::BufferId visibility; // MaskWords(count) words, storage write
	std::uint32_t count = 0;
};

enum class CullStatus : std::uint8_t
{
	kNoCompute = 1, // the device lacks Capability::kCompute
	kDevice         // the device refused the layout, pipeline or bind group
};

class CullKernel
{
public:
	// `code` replaces cull.comp's SPIR-V (the suite's seeded variants).
	static foundation::Expected<std::unique_ptr<CullKernel>, CullStatus> Create(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> code = {} );
	~CullKernel();
	CullKernel( const CullKernel & ) = delete;
	CullKernel &operator=( const CullKernel & ) = delete;

	// Records one dispatch; the buffers must be in their usages. The bind
	// group stays live until Collect.
	foundation::Expected<void, CullStatus> Record(
	    device::CommandEncoder &encoder, const CullBuffers &buffers );
	// Releases recorded dispatches' bind groups behind `token`.
	void Collect( device::CompletionToken token );
	// The recorded dispatches' bind groups, for an owner that retires them by
	// frame (the world pass); Collect then has none.
	std::vector<device::BindGroupId> TakeRecorded() { return std::exchange( m_Pending, {} ); }
	// Dispatches a graph pass could not record; the owner fails the frame
	// when it rises.
	std::uint32_t RecordFailures() const { return m_RecordFailures; }

private:
	explicit CullKernel( device::IRenderDevice2 &device );
	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_Layout;
	device::PipelineId m_Pipeline;
	std::vector<device::BindGroupId> m_Pending;
	device::CompletionToken m_LastToken;
	std::uint32_t m_RecordFailures = 0;
};

struct CullPassResources
{
	graph::ResourceRef instances;
	graph::ResourceRef view;
	graph::ResourceRef visibility;
	std::uint32_t count = 0;
	// The pass asks for the async compute queue (RFC 0016 S8); the graph
	// places it on graphics unless compiled with CompileOptions::asyncCompute.
	bool asyncCompute = false;
};

void AddCullPass(
    graph::GraphBuilder &builder, CullKernel &kernel, const CullPassResources &resources );

// Compaction (RFC 0016 GPU-driven submission S3/S4): the visibility mask
// becomes indexed indirect draw commands, one command list per bucket (a
// pipeline and its bindings), in instance order. Instances are ordered by
// bucket: bucket b owns instances [first, first + count) and the command
// slots at the same indices; each instance's DrawTemplate names its bucket.
// compact.comp writes bucket b's draw count as the output's 32-bit word b and
// the commands from CommandsOffset(bucketCount), so one buffer feeds every
// bucket's CommandEncoder::DrawIndexedIndirectCount as records and count:
//   DrawIndexedIndirectCount( out, CommandsOffset( n ) + 20 * first, out, 4 * b,
//                             count, 20 ).
// Each command is { indexCount, 1, firstIndex, vertexOffset, instance }: the
// instance index arrives as the draw's first instance (a device claiming
// Capability::kIndirectFirstInstance honours it). One bucket of all
// instances is the plain list (kCommandsOffset).

struct DrawTemplate // 16 bytes: what an instance draws
{
	std::uint32_t indexCount = 0;
	std::uint32_t firstIndex = 0;
	std::int32_t vertexOffset = 0;
	std::uint32_t bucket = 0; // into the buckets, which order the instances
};
static_assert( sizeof( DrawTemplate ) == 16 );

struct DrawBucket // 8 bytes
{
	std::uint32_t first = 0; // its first instance
	std::uint32_t count = 0;
};
static_assert( sizeof( DrawBucket ) == 8 );

// compact.comp scans the mask in one workgroup's shared memory.
constexpr std::uint32_t kMaxCompactInstances = 3072 * 32;

constexpr std::uint64_t CommandsOffset( std::uint32_t bucketCount )
{
	return ( std::uint64_t( bucketCount ) * 4 + 15 ) / 16 * 16;
}
constexpr std::uint64_t kCommandsOffset = CommandsOffset( 1 );
constexpr std::uint64_t CommandBufferBytes( std::uint32_t count, std::uint32_t bucketCount = 1 )
{
	return CommandsOffset( bucketCount ) +
	       std::uint64_t( count ) * sizeof( device::DrawIndexedIndirectCommand );
}

// The oracle: per bucket, the commands compact.comp writes for `mask`
// (MaskWords(count) words), the templates and the buckets.
std::vector<std::vector<device::DrawIndexedIndirectCommand>> CompactReference(
    std::span<const std::uint32_t> mask, std::span<const DrawTemplate> templates,
    std::span<const DrawBucket> buckets, std::uint32_t count );

struct CompactBuffers
{
	device::BufferId visibility; // the mask (at least one word), storage read
	device::BufferId templates;  // DrawTemplate[max(count, 1)], storage read
	device::BufferId view;       // the cull pass's CullView (its count), storage read
	device::BufferId buckets;    // DrawBucket[max(bucketCount, 1)], storage read
	device::BufferId commands;   // CommandBufferBytes(count, bucketCount) bytes, storage write
	std::uint32_t count = 0;     // at most kMaxCompactInstances
	std::uint32_t bucketCount = 1;
};

class CompactKernel
{
public:
	// `code` replaces compact.comp's SPIR-V (the suite's seeded variants).
	static foundation::Expected<std::unique_ptr<CompactKernel>, CullStatus> Create(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> code = {} );
	~CompactKernel();
	CompactKernel( const CompactKernel & ) = delete;
	CompactKernel &operator=( const CompactKernel & ) = delete;

	foundation::Expected<void, CullStatus> Record(
	    device::CommandEncoder &encoder, const CompactBuffers &buffers );
	void Collect( device::CompletionToken token );
	// The bind groups of recorded dispatches, for an owner that retires them
	// by frame instead of by token (the world pass); Collect then has none.
	std::vector<device::BindGroupId> TakeRecorded() { return std::exchange( m_Pending, {} ); }
	std::uint32_t RecordFailures() const { return m_RecordFailures; }

private:
	explicit CompactKernel( device::IRenderDevice2 &device );
	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_Layout;
	device::PipelineId m_Pipeline;
	std::vector<device::BindGroupId> m_Pending;
	device::CompletionToken m_LastToken;
	std::uint32_t m_RecordFailures = 0;
};

struct CompactPassResources
{
	graph::ResourceRef visibility;
	graph::ResourceRef templates;
	graph::ResourceRef view;
	graph::ResourceRef buckets;
	graph::ResourceRef commands; // a later draw pass reads it as kIndirect
	std::uint32_t count = 0;
	std::uint32_t bucketCount = 1;
	bool asyncCompute = false;
};

void AddCompactPass(
    graph::GraphBuilder &builder, CompactKernel &kernel, const CompactPassResources &resources );

// Occlusion culling (RFC 0016 GPU-driven submission S4): a hierarchical
// depth pyramid built from a depth texture (hiz.comp), and a test of the
// frustum culler's kept instances against it (occlusion.comp). Conventions
// are render.device.v2 D13's: clip depth 0 near to 1 far, clip Y up, row 0
// at the top. An instance is occluded when all eight corners of its world
// box lie in front of the camera and past the near plane, and its nearest
// projected depth is farther than the pyramid's farthest depth over its
// screen rectangle, read at the first level where the rectangle spans at
// most 2x2 texels. The test is conservative: what it removes is hidden by
// the depth the pyramid was built from.

constexpr std::uint32_t kMaxPyramidLevels = 16;

struct OcclusionView // 144 bytes, std430
{
	float viewProjection[4][4] = {}; // rows; clip = M * (x, y, z, 1)
	std::uint32_t width = 0;         // level 0, the depth texture's size
	std::uint32_t height = 0;
	std::uint32_t levels = 0;
	std::uint32_t count = 0;                           // instances
	std::uint32_t levelOffset[kMaxPyramidLevels] = {}; // in floats
};
static_assert( sizeof( OcclusionView ) == 144 );

// The view for a depth texture of width x height (all levels down to 1x1,
// at most kMaxPyramidLevels) and `count` instances.
OcclusionView MakeOcclusionView( const math::float4x4 &viewProjection, std::uint32_t width,
    std::uint32_t height, std::uint32_t count );
// The pyramid's size in floats.
std::uint64_t PyramidFloats( const OcclusionView &view );
// Oracles: the pyramid hiz.comp builds from `depth` (row-major, width x
// height), and the mask occlusion.comp writes.
std::vector<float> DepthPyramidReference( std::span<const float> depth, const OcclusionView &view );
std::vector<std::uint32_t> OcclusionReference( std::span<const CullInstance> instances,
    std::span<const std::uint32_t> frustumMask, std::span<const float> pyramid,
    const OcclusionView &view );

struct HiZBuffers
{
	device::TextureId depth;   // sampled, the size the view names
	device::SamplerId sampler; // a point sampler
	device::BufferId view;     // one OcclusionView, storage read
	device::BufferId pyramid;  // PyramidFloats floats, storage write
};

struct OcclusionBuffers
{
	device::BufferId instances;  // CullInstance[count]
	device::BufferId view;       // one OcclusionView
	device::BufferId pyramid;    // storage read
	device::BufferId frustum;    // the cull pass's mask
	device::BufferId visibility; // MaskWords(count) words, storage write
	std::uint32_t count = 0;
};

class OcclusionKernels
{
public:
	// `occlusionCode` replaces occlusion.comp's SPIR-V (seeded variants).
	static foundation::Expected<std::unique_ptr<OcclusionKernels>, CullStatus> Create(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> occlusionCode = {} );
	~OcclusionKernels();
	OcclusionKernels( const OcclusionKernels & ) = delete;
	OcclusionKernels &operator=( const OcclusionKernels & ) = delete;

	// One dispatch per pyramid level, in order (each reads the last).
	foundation::Expected<void, CullStatus> RecordPyramid(
	    device::CommandEncoder &encoder, const HiZBuffers &buffers, const OcclusionView &view );
	foundation::Expected<void, CullStatus> RecordOcclusion(
	    device::CommandEncoder &encoder, const OcclusionBuffers &buffers );
	void Collect( device::CompletionToken token );
	std::uint32_t RecordFailures() const { return m_RecordFailures; }

private:
	explicit OcclusionKernels( device::IRenderDevice2 &device );
	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_PyramidLayout;
	device::PipelineId m_PyramidPipeline;
	device::BindGroupLayoutId m_OcclusionLayout;
	device::PipelineId m_OcclusionPipeline;
	std::vector<device::BindGroupId> m_Pending;
	device::CompletionToken m_LastToken;
	std::uint32_t m_RecordFailures = 0;
};

struct OcclusionPassResources
{
	graph::ResourceRef depth; // read as kSampled
	device::SamplerId sampler;
	graph::ResourceRef instances;
	graph::ResourceRef view;    // OcclusionView
	graph::ResourceRef pyramid; // written then read by the pass
	graph::ResourceRef frustum;
	graph::ResourceRef visibility;
	OcclusionView occlusionView; // the levels and count the dispatches use
	bool asyncCompute = false;
};

// One compute pass: the pyramid, then the occlusion test.
void AddOcclusionPass( graph::GraphBuilder &builder, OcclusionKernels &kernels,
    const OcclusionPassResources &resources );

} // namespace render::culling

#endif // RENDER_CULLING_CULL_H
