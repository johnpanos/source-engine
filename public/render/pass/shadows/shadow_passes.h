//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.shadows on the device (RFC 0016 K7, render.shadows.v1):
//			the caster depth pass that fills the shadow atlas, and the
//			receiver helper that reads it.
//
//			ShadowDepthRenderer::AddPasses draws each shadow view's casters
//			into its atlas tile: one copy pass uploads a clip matrix per
//			(view, caster) pair, composed in double on the CPU, then one
//			depth-only render pass clears the atlas to the far plane and
//			draws each view with its viewport set to the tile's viewport (the
//			tile less its guard band), depth test less, no culling.
//
//			The receiver helper is render/shaders/common/shadow_sample.glsl
//			with ShadowTileGpu (render/shadow_tile.h), its record: families
//			that receive shadows include it. ShadowReceiverRenderer draws receivers lit by one shadowed
//			light (a spot, or the sun through its cascades) and writes the
//			light at each pixel; it is the pass the pixel oracles run and
//			the shadow mask a legacy family can read.
//
//			The device port has no comparison sampler and no depth bias
//			state, so the compare runs in the shader with a point sampler and
//			the bias is the receiver's (ShadowTileGpu::params).
//
//=============================================================================//

#ifndef RENDER_PASS_SHADOWS_SHADOW_PASSES_H
#define RENDER_PASS_SHADOWS_SHADOW_PASSES_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/graph/graph_builder.h"
#include "render/math/matrix.h"
#include "render/pass/shadows/atlas.h"
#include "render/resources/mesh_cache.h"
#include "render/shadow_tile.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace render::pass::shadows
{

// The record's one definition is render.contracts' (render/shadow_tile.h).
using render::ShadowTileGpu;

ShadowTileGpu PackShadowTile(
    const ShadowTileProjection &projection, std::uint32_t atlasSize, float depthBias );

enum class ShadowPassStatus : std::uint8_t
{
	kDevice = 1,    // a layout, sampler or pipeline was refused
	kInvalidTarget, // no atlas, a zero size, or a tile outside the atlas
	kInvalidLight,  // a tile count outside 1 to 4
};

struct ShadowCaster
{
	resources::MeshEntry mesh; // positions (float3) at offset 0 of each vertex
	math::float4x4 world;
};

struct ShadowDepthView
{
	math::float4x4 viewProjection;
	ShadowTile tile;
	std::span<const ShadowCaster> casters; // read by AddPasses
};

struct ShadowAtlasTarget
{
	graph::ResourceRef atlas; // written as kDepthWrite
	std::uint32_t atlasSize = 0;
	std::uint32_t guardTexels = 0;
};

struct ShadowDepthStats
{
	std::uint32_t views = 0;
	std::uint32_t draws = 0;
};

class ShadowDepthRenderer
{
public:
	static foundation::Expected<std::unique_ptr<ShadowDepthRenderer>, ShadowPassStatus> Create(
	    device::IRenderDevice2 &device, device::Format depthFormat = device::Format::kD32Float );
	~ShadowDepthRenderer();
	ShadowDepthRenderer( const ShadowDepthRenderer & ) = delete;
	ShadowDepthRenderer &operator=( const ShadowDepthRenderer & ) = delete;

	// Adds the upload and depth passes. The renderer must outlive the graph's
	// execution.
	foundation::Expected<ShadowDepthStats, ShadowPassStatus> AddPasses(
	    graph::GraphBuilder &builder, const ShadowAtlasTarget &target,
	    std::span<const ShadowDepthView> views );
	void Collect( device::CompletionToken token );
	std::uint32_t RecordFailures() const;

private:
	explicit ShadowDepthRenderer( device::IRenderDevice2 &device ) : m_Device( device ) {}
	foundation::Expected<device::PipelineId, ShadowPassStatus> PipelineFor( std::uint32_t stride );

	device::IRenderDevice2 &m_Device;
	device::Format m_DepthFormat = device::Format::kUnknown;
	device::BindGroupLayoutId m_DrawLayout;
	std::map<std::uint32_t, device::PipelineId> m_Pipelines; // by vertex stride
	mutable std::mutex m_PendingLock;
	std::vector<device::BindGroupId> m_Pending;
	std::uint32_t m_RecordFailures = 0;
	device::CompletionToken m_LastToken;
};

// The shadowed light a receiver pass applies.
struct ShadowReceiverLight
{
	enum class Kind : std::uint8_t
	{
		kSpot,
		kSun
	};
	Kind kind = Kind::kSpot;
	math::float3 position;       // spot
	math::float3 axis;           // spot, unit
	float outerCos = 1.0f;       // spot
	float range = 0.0f;          // spot
	std::uint32_t tileCount = 1; // 1 for a spot; the cascades for the sun
	std::array<ShadowTileGpu, 4> tiles;
	// The sun's cascades: each one's far view distance (Cascade::splitFar).
	std::array<float, 4> splitFar = {};
	float ambient = 0.2f;
};

struct ShadowReceiverView
{
	math::float4x4 view;           // world to view (cascade selection)
	math::float4x4 viewProjection; // world to clip
};

struct ShadowReceiver
{
	resources::MeshEntry mesh;
	math::float4x4 world;
};

struct ShadowReceiverTargets
{
	graph::ResourceRef color; // written as kColorAttachment
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	device::ClearColor clear;
};

// The receiver pass's view record (std140), 640 bytes.
struct ShadowReceiverViewGpu
{
	float viewProjection[4][4] = {};
	float view[4][4] = {};
	float lightPositionKind[4] = {};
	float lightAxisCos[4] = {};
	float lightRange[4] = {}; // spot range, tile count, ambient, 0
	float cascadeSplits[4] = {};
	ShadowTileGpu tiles[4];
};
static_assert( sizeof( ShadowReceiverViewGpu ) == 640 );

class ShadowReceiverRenderer
{
public:
	// `fragmentCode` replaces shadow_receiver.frag; the suite passes its seeded
	// defective variants here.
	static foundation::Expected<std::unique_ptr<ShadowReceiverRenderer>, ShadowPassStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat,
	    std::span<const std::uint32_t> fragmentCode = {} );
	~ShadowReceiverRenderer();
	ShadowReceiverRenderer( const ShadowReceiverRenderer & ) = delete;
	ShadowReceiverRenderer &operator=( const ShadowReceiverRenderer & ) = delete;

	// Adds the upload and receiver passes; the receiver pass reads `atlas` as
	// kSampled. The renderer must outlive the graph's execution.
	foundation::Expected<std::uint32_t, ShadowPassStatus> AddPasses( graph::GraphBuilder &builder,
	    graph::ResourceRef atlas, const ShadowReceiverView &view, const ShadowReceiverLight &light,
	    std::span<const ShadowReceiver> receivers, const ShadowReceiverTargets &targets );
	void Collect( device::CompletionToken token );
	std::uint32_t RecordFailures() const;

private:
	explicit ShadowReceiverRenderer( device::IRenderDevice2 &device ) : m_Device( device ) {}
	foundation::Expected<device::PipelineId, ShadowPassStatus> PipelineFor( std::uint32_t stride );

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	std::vector<std::uint32_t> m_Fragment;
	device::SamplerId m_Sampler;
	device::BindGroupLayoutId m_FrameLayout;
	device::BindGroupLayoutId m_ViewLayout;
	device::BindGroupLayoutId m_DrawLayout;
	std::map<std::uint32_t, device::PipelineId> m_Pipelines;
	mutable std::mutex m_PendingLock;
	std::vector<device::BindGroupId> m_Pending;
	std::uint32_t m_RecordFailures = 0;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::shadows

#endif // RENDER_PASS_SHADOWS_SHADOW_PASSES_H
