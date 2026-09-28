//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.opaque (RFC 0016 K5): draws a scene view's draw list
//			as graph passes.
//
//			AddOpaquePass resolves each draw's mesh (render.resources) and
//			color, writes the view's view-projection and one record per drawn
//			instance (world matrix, color) into transient buffers in a copy
//			pass, then draws the list in its order into the color and depth
//			targets with a depth test. A draw whose mesh or material does not
//			resolve is not drawn; it is counted in OpaqueStats::unresolved,
//			which the frame's owner checks (nothing is dropped silently).
//
//			The color stands in for material families until K4: families will
//			supply the pipelines, the material bind group and the raster state
//			(this pass culls nothing).
//
//			Mesh buffers are imported in their residency usages (vertex,
//			index), so the graph sees every access. Bind groups name buffers
//			that exist only during execution, so they are created while
//			recording and released behind the token given to Collect.
//
//=============================================================================//

#ifndef RENDER_PASS_OPAQUE_OPAQUE_H
#define RENDER_PASS_OPAQUE_OPAQUE_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/graph/graph_builder.h"
#include "render/resources/mesh_cache.h"
#include "render/scene/draw_list.h"

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace render::pass::opaque
{

struct InstanceRecord // std430, 80 bytes
{
	float world[4][4] = {}; // row-major, column vectors
	float color[4] = {};
};
static_assert( sizeof( InstanceRecord ) == 80 );

class IMeshResolver
{
public:
	virtual ~IMeshResolver() = default;
	// nullptr when the id names no resident mesh.
	virtual const resources::MeshEntry *Mesh( std::uint64_t mesh ) const = 0;
};

class IMaterialColors
{
public:
	virtual ~IMaterialColors() = default;
	// False when the id names no material.
	virtual bool Color( std::uint64_t material, float out[4] ) const = 0;
};

struct OpaqueTargets
{
	graph::ResourceRef color; // written as kColorAttachment
	graph::ResourceRef depth; // written as kDepthWrite
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	device::ClearColor clear;
};

struct OpaqueStats
{
	std::uint32_t drawn = 0;
	std::uint32_t unresolved = 0;
};

enum class OpaqueStatus : std::uint8_t
{
	kDevice = 1, // a layout or pipeline was refused
	kInvalidTargets
};

class OpaqueRenderer
{
public:
	static foundation::Expected<std::unique_ptr<OpaqueRenderer>, OpaqueStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat );
	~OpaqueRenderer();
	OpaqueRenderer( const OpaqueRenderer & ) = delete;
	OpaqueRenderer &operator=( const OpaqueRenderer & ) = delete;

	// Adds the upload and draw passes. The snapshot, list and resolvers are
	// read here; the renderer must outlive the graph's execution.
	foundation::Expected<OpaqueStats, OpaqueStatus> AddPasses( graph::GraphBuilder &builder,
	    const scene::SceneSnapshot &snapshot, const scene::DrawList &list,
	    const scene::SceneView &view, const IMeshResolver &meshes, const IMaterialColors &colors,
	    const OpaqueTargets &targets );
	// Bind groups recorded so far are released behind `token`.
	void Collect( device::CompletionToken token );
	// Draw passes that could not create their bind groups while recording.
	std::uint32_t RecordFailures() const;

private:
	explicit OpaqueRenderer( device::IRenderDevice2 &device ) : m_Device( device ) {}
	foundation::Expected<device::PipelineId, OpaqueStatus> PipelineFor( std::uint32_t stride );

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	device::Format m_DepthFormat = device::Format::kUnknown;
	device::BindGroupLayoutId m_ViewLayout;
	device::BindGroupLayoutId m_DrawLayout;
	std::map<std::uint32_t, device::PipelineId> m_Pipelines; // by vertex stride
	mutable std::mutex m_PendingLock;                        // recording may run on a pool worker
	std::vector<device::BindGroupId> m_Pending;
	std::uint32_t m_RecordFailures = 0;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::opaque

#endif // RENDER_PASS_OPAQUE_OPAQUE_H
