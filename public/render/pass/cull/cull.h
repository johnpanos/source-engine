//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.cull (RFC 0016 GPU-driven submission S4): frustum and
//			view-mask culling of scene instances on the GPU.
//
//			The kernel (render/pass/cull/cull.comp) decides, per instance,
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

#ifndef RENDER_PASS_CULL_CULL_H
#define RENDER_PASS_CULL_CULL_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/device/encoder.h"
#include "render/graph/graph_builder.h"
#include "render/scene/snapshot.h"
#include "render/scene/view.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace render::pass::cull
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
// becomes indexed indirect draw commands, one per kept instance, in
// instance order. compact.comp writes the draw count as the output's first
// 32-bit word and the commands from kCommandsOffset, so one buffer feeds
// CommandEncoder::DrawIndexedIndirectCount as both records and count:
//   DrawIndexedIndirectCount( out, kCommandsOffset, out, 0, count, 20 ).
// Each command is { indexCount, 1, firstIndex, vertexOffset, instance }:
// the instance index arrives as the draw's first instance, so a shader
// finds its per-instance data at gl_InstanceIndex.

struct DrawTemplate // 16 bytes: what an instance draws
{
	std::uint32_t indexCount = 0;
	std::uint32_t firstIndex = 0;
	std::int32_t vertexOffset = 0;
	std::uint32_t reserved = 0;
};
static_assert( sizeof( DrawTemplate ) == 16 );

constexpr std::uint64_t kCommandsOffset = 16;
constexpr std::uint64_t CommandBufferBytes( std::uint32_t count )
{
	return kCommandsOffset + std::uint64_t( count ) * sizeof( device::DrawIndexedIndirectCommand );
}

// The oracle: the commands compact.comp writes for `mask` (MaskWords(count)
// words) and the instances' templates.
std::vector<device::DrawIndexedIndirectCommand> CompactReference(
    std::span<const std::uint32_t> mask, std::span<const DrawTemplate> templates,
    std::uint32_t count );

struct CompactBuffers
{
	device::BufferId visibility; // the cull pass's mask (at least one word), storage read
	device::BufferId templates;  // DrawTemplate[max(count, 1)], storage read
	device::BufferId view;       // the cull pass's CullView (its count), storage read
	device::BufferId commands;   // CommandBufferBytes(count) bytes, storage write
	std::uint32_t count = 0;
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
	graph::ResourceRef commands; // a later draw pass reads it as kIndirect
	std::uint32_t count = 0;
	bool asyncCompute = false;
};

void AddCompactPass(
    graph::GraphBuilder &builder, CompactKernel &kernel, const CompactPassResources &resources );

} // namespace render::pass::cull

#endif // RENDER_PASS_CULL_CULL_H
