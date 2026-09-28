//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.lines (RFC 0016); see lines.h.
//
//=============================================================================//

#include "render/pass/lines/lines.h"

#include "spv/lines_spv.h"
#include "render/graph/executor.h"

#include <cmath>
#include <cstring>
#include <optional>
#include <utility>

namespace render::pass::lines
{

namespace
{

using namespace render::device;

constexpr double kPi = 3.14159265358979323846;

// The draw-constant block both stages declare (80 bytes, D16).
struct Constants
{
	float toClip[4][4] = {}; // row-major, column vectors
	float params[4] = {};    // x: clip-space depth bias times w
};
static_assert( sizeof( Constants ) == 80 );

LineVertex Vertex( const math::float3 &p, Rgba8 color )
{
	LineVertex v;
	v.position[0] = p.x;
	v.position[1] = p.y;
	v.position[2] = p.z;
	v.color = color.Packed();
	return v;
}

void Store( const math::float4x4 &m, float out[4][4] )
{
	for ( int r = 0; r < 4; ++r )
	{
		out[r][0] = m.rows[r].x;
		out[r][1] = m.rows[r].y;
		out[r][2] = m.rows[r].z;
		out[r][3] = m.rows[r].w;
	}
}

// One draw of the render pass: a vertex range of the frame buffer or a mesh.
struct Draw
{
	PipelineId pipeline;
	bool screen = false;
	bool biased = false;
	// The frame's list buffer when mesh is empty.
	std::optional<resources::MeshEntry> mesh;
	std::uint32_t firstVertex = 0;
	std::uint32_t vertexCount = 0;
};

struct Frame
{
	Constants world;
	Constants screen;
	std::vector<LineVertex> vertices;
	std::vector<Draw> draws;
};

} // namespace

// --- LineList -------------------------------------------------------------

std::size_t LineList::Index( Style style, Topology topology )
{
	return ( style.space == Space::kScreen ? 4 : 0 ) + ( style.depthTest ? 2 : 0 ) +
	       ( topology == Topology::kFilled ? 1 : 0 );
}

std::vector<LineVertex> &LineList::Batch( Style style, Topology topology )
{
	return m_Batches[Index( style, topology )];
}

void LineList::Line( Style style, const math::float3 &a, const math::float3 &b, Rgba8 color )
{
	std::vector<LineVertex> &batch = Batch( style, Topology::kLines );
	batch.push_back( Vertex( a, color ) );
	batch.push_back( Vertex( b, color ) );
}

void LineList::Box( Style style, const math::float3 &mins, const math::float3 &maxs, Rgba8 color )
{
	const math::float3 p[8] = { { mins.x, mins.y, mins.z }, { maxs.x, mins.y, mins.z },
	    { maxs.x, maxs.y, mins.z }, { mins.x, maxs.y, mins.z }, { mins.x, mins.y, maxs.z },
	    { maxs.x, mins.y, maxs.z }, { maxs.x, maxs.y, maxs.z }, { mins.x, maxs.y, maxs.z } };
	static const int kEdges[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 },
	    { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
	for ( const auto &edge : kEdges )
	{
		Line( style, p[edge[0]], p[edge[1]], color );
	}
}

void LineList::Polygon( Style style, std::span<const math::float3> points, Rgba8 color )
{
	if ( points.size() < 2 )
	{
		return;
	}
	for ( std::size_t i = 0; i < points.size(); ++i )
	{
		Line( style, points[i], points[( i + 1 ) % points.size()], color );
	}
}

void LineList::Triangle( Style style, const math::float3 &a, const math::float3 &b,
    const math::float3 &c, Rgba8 ca, Rgba8 cb, Rgba8 cc )
{
	std::vector<LineVertex> &batch = Batch( style, Topology::kFilled );
	batch.push_back( Vertex( a, ca ) );
	batch.push_back( Vertex( b, cb ) );
	batch.push_back( Vertex( c, cc ) );
}

void LineList::Quad( Style style, float x0, float y0, float x1, float y1, Rgba8 color )
{
	const math::float3 a{ x0, y0, 0.0f };
	const math::float3 b{ x1, y0, 0.0f };
	const math::float3 c{ x1, y1, 0.0f };
	const math::float3 d{ x0, y1, 0.0f };
	Triangle( style, a, b, c, color, color, color );
	Triangle( style, a, c, d, color, color, color );
}

void LineList::Disc( Style style, float x, float y, float radius, Rgba8 color, int segments )
{
	segments = segments < 3 ? 3 : segments;
	const math::float3 center{ x, y, 0.0f };
	for ( int i = 0; i < segments; ++i )
	{
		const double a0 = 2.0 * kPi * i / segments;
		const double a1 = 2.0 * kPi * ( i + 1 ) / segments;
		const math::float3 p0{
		    x + float( radius * std::cos( a0 ) ), y + float( radius * std::sin( a0 ) ), 0.0f };
		const math::float3 p1{
		    x + float( radius * std::cos( a1 ) ), y + float( radius * std::sin( a1 ) ), 0.0f };
		Triangle( style, center, p0, p1, color, color, color );
	}
}

std::span<const LineVertex> LineList::Vertices( Style style, Topology topology ) const
{
	return m_Batches[Index( style, topology )];
}

std::size_t LineList::VertexCount() const
{
	std::size_t count = 0;
	for ( const std::vector<LineVertex> &batch : m_Batches )
	{
		count += batch.size();
	}
	return count;
}

void LineList::Clear()
{
	for ( std::vector<LineVertex> &batch : m_Batches )
	{
		batch.clear();
	}
}

// --- LinesRenderer --------------------------------------------------------

foundation::Expected<std::unique_ptr<LinesRenderer>, LinesStatus> LinesRenderer::Create(
    IRenderDevice2 &device, Format colorFormat, Format depthFormat )
{
	if ( colorFormat == Format::kUnknown )
	{
		return foundation::MakeUnexpected( LinesStatus::kInvalidTargets );
	}
	std::unique_ptr<LinesRenderer> renderer( new LinesRenderer( device ) );
	renderer->m_ColorFormat = colorFormat;
	renderer->m_DepthFormat = depthFormat;
	return renderer;
}

LinesRenderer::~LinesRenderer()
{
	for ( const auto &[key, pipeline] : m_Pipelines )
	{
		(void)m_Device.Release( pipeline, m_LastToken );
	}
}

foundation::Expected<PipelineId, LinesStatus> LinesRenderer::PipelineFor(
    Topology topology, bool depthTest )
{
	const int key = ( topology == Topology::kFilled ? 2 : 0 ) + ( depthTest ? 1 : 0 );
	if ( auto found = m_Pipelines.find( key ); found != m_Pipelines.end() )
	{
		return found->second;
	}
	const ShaderArtifactView stages[] = {
	    { ShaderStage::kVertex, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kLinesVertex ) ), "main", {}, sizeof( Constants ) },
	    { ShaderStage::kFragment, ArtifactFormat::kSpirv,
	        std::as_bytes( std::span( spirv::kLinesFragment ) ), "main", {}, 0 } };
	const VertexAttribute attributes[] = {
	    { 0, VertexFormat::kFloat3, 0, 0 }, { 1, VertexFormat::kUnorm8x4, 12, 0 } };
	const VertexBufferLayout buffers[] = { { sizeof( LineVertex ), false } };
	const Format colors[] = { m_ColorFormat };
	const BlendMode blends[] = { BlendMode::kAlpha };
	PipelineDesc desc;
	desc.kind = PipelineKind::kGraphics;
	desc.stages = stages;
	desc.drawConstantBytes = sizeof( Constants );
	desc.vertex = { attributes, buffers };
	desc.topology = topology == Topology::kFilled ? PrimitiveTopology::kTriangleList
	                                              : PrimitiveTopology::kLineList;
	desc.raster.cull = CullMode::kNone;
	desc.depthStencil = { depthTest, depthTest, CompareOp::kLessEqual };
	desc.colorFormats = colors;
	desc.blends = blends;
	desc.depthFormat = m_DepthFormat;
	desc.debugName = "render.pass.lines";
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
	{
		return foundation::MakeUnexpected( LinesStatus::kDevice );
	}
	m_Pipelines.emplace( key, pipeline.Value() );
	return pipeline.Value();
}

foundation::Expected<LinesStats, LinesStatus> LinesRenderer::AddPasses(
    graph::GraphBuilder &builder, const LineList &list, std::span<const MeshBatch> batches,
    const LinesView &view, const LinesTargets &targets )
{
	const bool hasDepth = m_DepthFormat != Format::kUnknown;
	if ( !targets.color.IsValid() || targets.width == 0 || targets.height == 0 || view.width == 0 ||
	     view.height == 0 || hasDepth != targets.depth.IsValid() )
	{
		return foundation::MakeUnexpected( LinesStatus::kInvalidTargets );
	}
	for ( const MeshBatch &batch : batches )
	{
		if ( batch.mesh.vertexStride != sizeof( LineVertex ) || !batch.mesh.vertices.IsValid() )
		{
			return foundation::MakeUnexpected( LinesStatus::kBadMesh );
		}
		if ( batch.style.depthTest && !hasDepth )
		{
			return foundation::MakeUnexpected( LinesStatus::kNeedsDepth );
		}
	}
	for ( bool filled : { false, true } )
	{
		const Topology topology = filled ? Topology::kFilled : Topology::kLines;
		for ( Space space : { Space::kWorld, Space::kScreen } )
		{
			if ( !list.Vertices( { space, true }, topology ).empty() && !hasDepth )
			{
				return foundation::MakeUnexpected( LinesStatus::kNeedsDepth );
			}
		}
	}

	auto frame = std::make_shared<Frame>();
	Store( view.worldToClip, frame->world.toClip );
	Store( math::PixelToClip( float( view.width ), float( view.height ) ), frame->screen.toClip );
	LinesStats stats;
	auto addDraw = [&]( Topology topology, Style style, std::optional<resources::MeshEntry> mesh,
	                   std::uint32_t first,
	                   std::uint32_t count ) -> foundation::Expected<void, LinesStatus>
	{
		auto pipeline = PipelineFor( topology, style.depthTest );
		if ( !pipeline )
		{
			return foundation::MakeUnexpected( pipeline.Error() );
		}
		Draw draw;
		draw.pipeline = pipeline.Value();
		draw.screen = style.space == Space::kScreen;
		draw.biased = style.depthTest && topology == Topology::kLines;
		draw.mesh = std::move( mesh );
		draw.firstVertex = first;
		draw.vertexCount = count;
		frame->draws.push_back( std::move( draw ) );
		return {};
	};
	for ( const MeshBatch &batch : batches )
	{
		const std::uint32_t count =
		    batch.mesh.indices.IsValid() ? batch.mesh.indexCount : batch.mesh.vertexCount;
		if ( count == 0 )
		{
			continue;
		}
		if ( auto added = addDraw( batch.topology, batch.style, batch.mesh, 0, count ); !added )
		{
			return foundation::MakeUnexpected( added.Error() );
		}
		++stats.meshBatches;
	}
	struct Slot
	{
		Style style;
		Topology topology;
	};
	static const Slot kOrder[] = { { { Space::kWorld, true }, Topology::kFilled },
	    { { Space::kWorld, true }, Topology::kLines },
	    { { Space::kWorld, false }, Topology::kFilled },
	    { { Space::kWorld, false }, Topology::kLines },
	    { { Space::kScreen, false }, Topology::kLines },
	    { { Space::kScreen, true }, Topology::kLines },
	    { { Space::kScreen, false }, Topology::kFilled },
	    { { Space::kScreen, true }, Topology::kFilled } };
	for ( const Slot &slot : kOrder )
	{
		std::span<const LineVertex> vertices = list.Vertices( slot.style, slot.topology );
		if ( vertices.empty() )
		{
			continue;
		}
		const auto first = static_cast<std::uint32_t>( frame->vertices.size() );
		frame->vertices.insert( frame->vertices.end(), vertices.begin(), vertices.end() );
		if ( auto added = addDraw( slot.topology, slot.style, std::nullopt, first,
		         static_cast<std::uint32_t>( vertices.size() ) );
		    !added )
		{
			return foundation::MakeUnexpected( added.Error() );
		}
	}
	stats.listVertices = static_cast<std::uint32_t>( frame->vertices.size() );
	stats.draws = static_cast<std::uint32_t>( frame->draws.size() );
	frame->world.params[0] = view.depthBias;

	// The list's vertices, uploaded into a transient buffer.
	graph::ResourceRef listBuffer;
	if ( !frame->vertices.empty() )
	{
		BufferDesc desc;
		desc.size = frame->vertices.size() * sizeof( LineVertex );
		listBuffer = builder.CreateBuffer( "lines-vertices", desc );
		builder.AddPass( "lines-upload", graph::PassKind::kCopy )
		    .Write( listBuffer, ResourceUsage::kCopyDestination )
		    .Execute(
		        [frame, listBuffer]( graph::RecordContext &context )
		        {
			        context.Encoder().WriteBuffer( context.Buffer( listBuffer ), 0,
			            std::as_bytes( std::span<const LineVertex>( frame->vertices ) ) );
		        } );
	}

	// Resident meshes, imported once each in their residency usages.
	std::map<std::uint64_t, graph::ResourceRef> vertexRefs;
	std::map<std::uint64_t, graph::ResourceRef> indexRefs;
	auto import = [&]( BufferId buffer, ResourceUsage usage,
	                  std::map<std::uint64_t, graph::ResourceRef> &refs )
	{
		if ( refs.count( buffer.value ) == 0 )
		{
			BufferDesc desc;
			desc.usages = { usage };
			refs[buffer.value] = builder.ImportBuffer( "lines-mesh", buffer, desc, usage, usage );
		}
	};
	for ( const Draw &draw : frame->draws )
	{
		if ( !draw.mesh )
		{
			continue;
		}
		import( draw.mesh->vertices, ResourceUsage::kVertex, vertexRefs );
		if ( draw.mesh->indices.IsValid() )
		{
			import( draw.mesh->indices, ResourceUsage::kIndex, indexRefs );
		}
	}

	graph::PassBuilder pass = builder.AddPass( "lines", graph::PassKind::kRender );
	pass.Write( targets.color, ResourceUsage::kColorAttachment );
	if ( targets.depth.IsValid() )
	{
		pass.Write( targets.depth, ResourceUsage::kDepthWrite );
	}
	if ( listBuffer.IsValid() )
	{
		pass.Read( listBuffer, ResourceUsage::kVertex );
	}
	for ( const auto &[buffer, ref] : vertexRefs )
	{
		pass.Read( ref, ResourceUsage::kVertex );
	}
	for ( const auto &[buffer, ref] : indexRefs )
	{
		pass.Read( ref, ResourceUsage::kIndex );
	}
	pass.Execute(
	    [frame, listBuffer, targets]( graph::RecordContext &context )
	    {
		    CommandEncoder &encoder = context.Encoder();
		    ColorAttachment color;
		    color.texture = context.Texture( targets.color );
		    color.load = targets.clearColor ? LoadOp::kClear : LoadOp::kLoad;
		    color.clear = targets.clear;
		    const ColorAttachment colorList[] = { color };
		    RenderingDesc rendering;
		    rendering.colors = colorList;
		    if ( targets.depth.IsValid() )
		    {
			    DepthAttachment depth;
			    depth.texture = context.Texture( targets.depth );
			    depth.load = targets.clearDepth ? LoadOp::kClear : LoadOp::kLoad;
			    rendering.depth = depth;
		    }
		    rendering.width = targets.width;
		    rendering.height = targets.height;
		    encoder.BeginRendering( rendering );
		    encoder.SetViewport(
		        { 0.0f, 0.0f, float( targets.width ), float( targets.height ), 0.0f, 1.0f } );
		    for ( const Draw &draw : frame->draws )
		    {
			    encoder.SetPipeline( draw.pipeline );
			    Constants constants = draw.screen ? frame->screen : frame->world;
			    if ( !draw.biased )
			    {
				    constants.params[0] = 0.0f;
			    }
			    encoder.SetDrawConstants(
			        0, std::as_bytes( std::span<const Constants>( &constants, 1 ) ) );
			    if ( draw.mesh )
			    {
				    encoder.SetVertexBuffer( 0, draw.mesh->vertices );
				    if ( draw.mesh->indices.IsValid() )
				    {
					    encoder.SetIndexBuffer( draw.mesh->indices, 0, draw.mesh->indexFormat );
					    encoder.DrawIndexed( draw.vertexCount );
				    }
				    else
				    {
					    encoder.Draw( draw.vertexCount );
				    }
			    }
			    else
			    {
				    encoder.SetVertexBuffer( 0, context.Buffer( listBuffer ) );
				    encoder.Draw( draw.vertexCount, 1, draw.firstVertex );
			    }
		    }
		    encoder.EndRendering();
	    } );
	return stats;
}

} // namespace render::pass::lines
