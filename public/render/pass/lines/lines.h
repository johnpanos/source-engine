//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.lines (RFC 0016 decision "lines and overlays"): the
//			shared pass for lines, outlines and flat-colored primitives over a
//			portable item list. Its first users are the Hammer editor's
//			viewports (wireframes, the grid, tool overlays and, until material
//			families carry lighting, its shaded preview); the game's debug
//			overlay is the next.
//
//			Items (LineList) are lines, boxes, closed polygons, filled
//			triangles with per-vertex colors, and filled screen quads and
//			discs for handles. Each has a Style: world space (through the
//			view's world-to-clip matrix) or screen space (logical pixels of
//			the target, origin at the top-left, y down), and depth-tested or
//			not. Colors are 8-bit RGBA, straight alpha, alpha-blended.
//
//			Resident geometry an owner keeps in render.resources (MeshBatch,
//			vertices in the LineVertex layout) draws with the list, so a large
//			static line or triangle set is uploaded once rather than per frame.
//
//			Draw order: the mesh batches in the order given, then the list's
//			world depth-tested filled, world depth-tested lines, world
//			untested filled, world untested lines, screen lines and screen
//			filled (so handles land on top). Depth-tested lines are pulled
//			toward the eye by the view's depthBias, a clip-space z offset, so
//			edges drawn on their own faces win the depth test. Under an
//			orthographic view it is a depth offset; under a perspective view
//			with far >> near it moves a line by about bias / near of its
//			distance, so near and far edges keep the same relative margin.
//
//			AddPasses adds one upload pass (the list's vertices into a
//			transient buffer) and one render pass. Nothing is culled and
//			nothing is dropped: invalid targets or a mesh in another layout
//			fail before any pass is added.
//
//=============================================================================//

#ifndef RENDER_PASS_LINES_LINES_H
#define RENDER_PASS_LINES_LINES_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/graph/graph_builder.h"
#include "render/math/matrix.h"
#include "render/resources/mesh_cache.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <span>
#include <vector>

namespace render::pass::lines
{

struct Rgba8
{
	std::uint8_t r = 255;
	std::uint8_t g = 255;
	std::uint8_t b = 255;
	std::uint8_t a = 255;

	// The vertex encoding: r in the low byte.
	constexpr std::uint32_t Packed() const
	{
		return std::uint32_t( r ) | ( std::uint32_t( g ) << 8 ) | ( std::uint32_t( b ) << 16 ) |
		       ( std::uint32_t( a ) << 24 );
	}
	friend constexpr bool operator==( const Rgba8 &, const Rgba8 & ) = default;
};

struct LineVertex // 16 bytes: float3 position, RGBA8 color
{
	float position[3] = {};
	std::uint32_t color = 0;
};
static_assert( sizeof( LineVertex ) == 16 );

enum class Space : std::uint8_t
{
	kWorld,
	kScreen, // logical pixels of the target: origin top-left, y down
};

struct Style
{
	Space space = Space::kWorld;
	bool depthTest = false;
};

enum class Topology : std::uint8_t
{
	kLines,
	kFilled, // triangle lists
};

class LineList
{
public:
	void Line( Style style, const math::float3 &a, const math::float3 &b, Rgba8 color );
	// The twelve edges of an axis-aligned box.
	void Box( Style style, const math::float3 &mins, const math::float3 &maxs, Rgba8 color );
	// A closed outline through the points (fewer than two: nothing).
	void Polygon( Style style, std::span<const math::float3> points, Rgba8 color );
	void Triangle( Style style, const math::float3 &a, const math::float3 &b, const math::float3 &c,
	    Rgba8 ca, Rgba8 cb, Rgba8 cc );
	// A filled rectangle between two corners, at z 0 of its space.
	void Quad( Style style, float x0, float y0, float x1, float y1, Rgba8 color );
	// A filled disc in its space's x/y plane at z 0 (segments at least 3).
	void Disc( Style style, float x, float y, float radius, Rgba8 color, int segments = 12 );

	std::span<const LineVertex> Vertices( Style style, Topology topology ) const;
	std::size_t VertexCount() const;
	bool Empty() const { return VertexCount() == 0; }
	void Clear();

private:
	static std::size_t Index( Style style, Topology topology );
	std::vector<LineVertex> &Batch( Style style, Topology topology );
	std::array<std::vector<LineVertex>, 8> m_Batches;
};

// Vertices (and optional indices) an owner keeps resident, in the LineVertex
// layout, drawn before the list.
struct MeshBatch
{
	resources::MeshEntry mesh;
	Topology topology = Topology::kLines;
	Style style;
};

struct LinesView
{
	math::float4x4 worldToClip;
	std::uint32_t width = 0; // the target's logical pixels, for screen space
	std::uint32_t height = 0;
	float depthBias = 0.0f; // clip-space z offset toward the eye for depth-tested lines
};

struct LinesTargets
{
	graph::ResourceRef color; // written as kColorAttachment
	graph::ResourceRef depth; // required when the renderer has a depth format
	std::uint32_t width = 0;  // framebuffer pixels
	std::uint32_t height = 0;
	bool clearColor = true;
	device::ClearColor clear;
	bool clearDepth = true;
};

struct LinesStats
{
	std::uint32_t draws = 0;
	std::uint32_t listVertices = 0;
	std::uint32_t meshBatches = 0;
};

enum class LinesStatus : std::uint8_t
{
	kDevice = 1,     // a pipeline was refused
	kInvalidTargets, // missing color, missing depth with a depth format, or a zero size
	kNeedsDepth,     // depth-tested items or batches without a depth format
	kBadMesh,        // a batch mesh not in the LineVertex layout
};

class LinesRenderer
{
public:
	// depthFormat kUnknown: no depth target and no depth-tested items.
	static foundation::Expected<std::unique_ptr<LinesRenderer>, LinesStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat );
	~LinesRenderer();

	LinesRenderer( const LinesRenderer & ) = delete;
	LinesRenderer &operator=( const LinesRenderer & ) = delete;

	// Adds the upload and draw passes. The list and batches are copied or
	// read here; the renderer must outlive the graph's execution.
	foundation::Expected<LinesStats, LinesStatus> AddPasses( graph::GraphBuilder &builder,
	    const LineList &list, std::span<const MeshBatch> batches, const LinesView &view,
	    const LinesTargets &targets );

	// Pipelines are released behind the last token given here.
	void Collect( device::CompletionToken token ) { m_LastToken = token; }

private:
	explicit LinesRenderer( device::IRenderDevice2 &device ) : m_Device( device ) {}
	foundation::Expected<device::PipelineId, LinesStatus> PipelineFor(
	    Topology topology, bool depthTest );

	device::IRenderDevice2 &m_Device;
	device::Format m_ColorFormat = device::Format::kUnknown;
	device::Format m_DepthFormat = device::Format::kUnknown;
	std::map<int, device::PipelineId> m_Pipelines; // by topology * 2 + depth test
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::lines

#endif // RENDER_PASS_LINES_LINES_H
