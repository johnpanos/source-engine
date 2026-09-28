//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/adapters/render/viewport_renderer.h.
//
//=============================================================================//

#include "viewport_renderer.h"

#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"

#include <chrono>
#include <span>
#include <thread>
#include <utility>

namespace hammer::render_adapter
{

namespace
{

using namespace ::render;
using ::render::pass::lines::LineList;
using ::render::pass::lines::LinesTargets;
using ::render::pass::lines::LinesView;
using ::render::pass::lines::MeshBatch;
using ::render::pass::lines::Space;
using ::render::pass::lines::Topology;

constexpr device::ClearColor kClear3D = { 0.13f, 0.14f, 0.17f, 1.0f };
constexpr device::ClearColor kClear2D = { 0.03f, 0.03f, 0.04f, 1.0f };

} // namespace

foundation::Expected<std::unique_ptr<ViewportRenderer>, ViewportStatus> ViewportRenderer::Create(
    device::IRenderDevice2 &device )
{
	std::unique_ptr<ViewportRenderer> renderer( new ViewportRenderer( device ) );
	auto lines = pass::lines::LinesRenderer::Create(
	    device, device::Format::kRGBA8Unorm, device::Format::kD32Float );
	if ( !lines )
	{
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	renderer->m_Lines = std::move( lines ).Value();
	return renderer;
}

ViewportRenderer::~ViewportRenderer()
{
	// Our frames complete before their resources and the pass go.
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 10 );
	while ( !m_Pending.empty() && std::chrono::steady_clock::now() < deadline )
	{
		(void)m_Device.Poll();
		for ( auto it = m_Pending.begin(); it != m_Pending.end(); )
		{
			if ( m_Device.IsComplete( it->second.token ) )
			{
				Release( it->second );
				it = m_Pending.erase( it );
			}
			else
			{
				++it;
			}
		}
		std::this_thread::yield();
	}
	if ( m_LastToken.NamesSubmission() )
	{
		while ( !m_Device.IsComplete( m_LastToken ) && std::chrono::steady_clock::now() < deadline )
		{
			(void)m_Device.Poll();
			std::this_thread::yield();
		}
	}
	m_Lines.reset();
}

void ViewportRenderer::Release( const Pending &pending )
{
	(void)m_Device.Release( pending.color, pending.token );
	(void)m_Device.Release( pending.readback, pending.token );
}

foundation::Expected<void, ViewportStatus> ViewportRenderer::SetScene(
    const viewport::RenderSnapshot &snapshot, std::uint64_t key )
{
	if ( m_HaveScene && key == m_Stats.key )
	{
		return {};
	}
	const SceneGeometry geometry = BuildSceneGeometry( snapshot );
	auto stage = [&]( const char *name, const std::vector<LineVertex> &vertices,
	                 std::optional<resources::MeshEntry> &slot ) -> bool
	{
		if ( vertices.empty() )
		{
			if ( slot )
			{
				(void)m_Meshes.Evict( name );
			}
			slot.reset();
			return true;
		}
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const LineVertex>( vertices ) );
		data.vertexStride = sizeof( LineVertex );
		auto entry = m_Meshes.Stage( name, data );
		if ( !entry )
		{
			return false;
		}
		slot = entry.Value();
		return true;
	};
	if ( !stage( "hammer-faces", geometry.faces, m_Faces ) ||
	     !stage( "hammer-edges", geometry.edges, m_Edges ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	// The uploads run in their own submission; later frames on the queue see
	// them, and replaced buffers retire behind its completion.
	auto encoder = m_Device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !encoder )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	m_Meshes.RecordUploads( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = m_Device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !token )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	m_Meshes.Retire( token.Value() );
	m_LastToken = token.Value();
	m_Bounds = snapshot.bounds;
	m_HaveScene = true;
	m_Stats.key = key;
	m_Stats.triangles = geometry.triangles;
	m_Stats.faceVertices = static_cast<std::uint32_t>( geometry.faces.size() );
	m_Stats.edgeVertices = static_cast<std::uint32_t>( geometry.edges.size() );
	++m_Stats.stagings;
	return {};
}

foundation::Expected<ViewportRenderer::Ticket, ViewportStatus> ViewportRenderer::Render(
    const ViewRequest &request )
{
	const bool threeD = request.kind == viewport::ViewKind::Camera3D;
	if ( request.pixelWidth == 0 || request.pixelHeight == 0 ||
	     ( threeD ? !request.camera3D : !request.camera2D ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kInvalidView );
	}
	const LinesView view =
	    threeD ? ViewFor( *request.camera3D, m_Bounds ) : ViewFor( *request.camera2D );
	if ( view.width == 0 || view.height == 0 )
	{
		return foundation::MakeUnexpected( ViewportStatus::kInvalidView );
	}

	device::TextureDesc colorDesc;
	colorDesc.format = device::Format::kRGBA8Unorm;
	colorDesc.width = request.pixelWidth;
	colorDesc.height = request.pixelHeight;
	colorDesc.usages = {
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
	colorDesc.debugName = "hammer-viewport";
	device::TextureDesc depthDesc = colorDesc;
	depthDesc.format = device::Format::kD32Float;
	depthDesc.usages = { device::ResourceUsage::kDepthWrite };
	depthDesc.debugName = "hammer-viewport-depth";
	device::BufferDesc readbackDesc;
	readbackDesc.size = std::uint64_t( request.pixelWidth ) * request.pixelHeight * 4;
	readbackDesc.usages = { device::ResourceUsage::kCopyDestination };
	readbackDesc.memory = device::MemoryKind::kReadback;
	readbackDesc.debugName = "hammer-viewport-readback";
	auto color = m_Device.CreateTexture( colorDesc );
	auto readback = m_Device.CreateBuffer( readbackDesc );
	if ( !color || !readback )
	{
		if ( color )
			(void)m_Device.Release( color.Value(), device::CompletionToken() );
		if ( readback )
			(void)m_Device.Release( readback.Value(), device::CompletionToken() );
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	Pending pending{ {}, color.Value(), readback.Value(), request.pixelWidth, request.pixelHeight };

	graph::GraphBuilder builder;
	const graph::ResourceRef colorRef = builder.ImportTexture( "hammer-viewport", color.Value(),
	    colorDesc, device::ResourceUsage::kUndefined, device::ResourceUsage::kCopySource );
	const graph::ResourceRef depthRef = builder.CreateTexture( "hammer-viewport-depth", depthDesc );
	const graph::ResourceRef readbackRef = builder.ImportBuffer( "hammer-viewport-readback",
	    readback.Value(), readbackDesc, device::ResourceUsage::kUndefined,
	    device::ResourceUsage::kCopyDestination );

	LinesTargets targets;
	targets.color = colorRef;
	targets.depth = depthRef;
	targets.width = request.pixelWidth;
	targets.height = request.pixelHeight;
	targets.clear = threeD ? kClear3D : kClear2D;

	// 2D: the grid first, under everything.
	bool cleared = false;
	if ( !threeD && !request.grid.empty() )
	{
		LineList grid;
		AppendGrid( request.grid, view.width, view.height, grid );
		if ( !m_Lines->AddPasses( builder, grid, {}, view, targets ) )
		{
			Release( pending );
			return foundation::MakeUnexpected( ViewportStatus::kPass );
		}
		cleared = true;
	}

	LineList overlay;
	AppendOverlay( request.overlay, overlay );
	std::vector<MeshBatch> batches;
	if ( threeD && m_Faces )
	{
		batches.push_back( { *m_Faces, Topology::kFilled, { Space::kWorld, true } } );
	}
	if ( m_Edges )
	{
		batches.push_back( { *m_Edges, Topology::kLines, { Space::kWorld, threeD } } );
	}
	LinesTargets scene = targets;
	scene.clearColor = !cleared;
	scene.clearDepth = !cleared;
	if ( !m_Lines->AddPasses( builder, overlay, batches, view, scene ) )
	{
		Release( pending );
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	const std::uint32_t width = request.pixelWidth;
	const std::uint32_t height = request.pixelHeight;
	builder.AddPass( "hammer-viewport-readback", graph::PassKind::kCopy )
	    .Read( colorRef, device::ResourceUsage::kCopySource )
	    .Write( readbackRef, device::ResourceUsage::kCopyDestination )
	    .SideEffect()
	    .Execute(
	        [colorRef, readbackRef, width, height]( graph::RecordContext &context )
	        {
		        context.Encoder().CopyTextureToBuffer( context.Texture( colorRef ),
		            context.Buffer( readbackRef ), { 0, 0, 0, width, height } );
	        } );
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
	{
		Release( pending );
		return foundation::MakeUnexpected( ViewportStatus::kGraph );
	}
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), m_Device );
	if ( !executed )
	{
		Release( pending );
		return foundation::MakeUnexpected( ViewportStatus::kGraph );
	}
	pending.token = executed.Value().token;
	m_Lines->Collect( pending.token );
	m_LastToken = pending.token;
	const Ticket ticket = m_NextTicket++;
	m_Pending.emplace( ticket, pending );
	return ticket;
}

foundation::Expected<std::optional<ViewPixels>, ViewportStatus> ViewportRenderer::Take(
    Ticket ticket )
{
	auto found = m_Pending.find( ticket );
	if ( found == m_Pending.end() )
	{
		return foundation::MakeUnexpected( ViewportStatus::kUnknownTicket );
	}
	(void)m_Device.Poll();
	if ( !m_Device.IsComplete( found->second.token ) )
	{
		return std::optional<ViewPixels>();
	}
	const Pending pending = found->second;
	m_Pending.erase( found );
	ViewPixels pixels;
	pixels.width = pending.width;
	pixels.height = pending.height;
	pixels.rgba.resize( std::size_t( pending.width ) * pending.height * 4 );
	const bool read =
	    m_Device
	        .ReadBuffer( pending.readback, 0,
	            std::as_writable_bytes( std::span<std::uint8_t>( pixels.rgba ) ) )
	        .HasValue();
	Release( pending );
	if ( !read )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	return std::optional<ViewPixels>( std::move( pixels ) );
}

foundation::Expected<ViewPixels, ViewportStatus> ViewportRenderer::RenderAndWait(
    const ViewRequest &request )
{
	auto ticket = Render( request );
	if ( !ticket )
	{
		return foundation::MakeUnexpected( ticket.Error() );
	}
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	for ( ;; )
	{
		auto taken = Take( ticket.Value() );
		if ( !taken )
		{
			return foundation::MakeUnexpected( taken.Error() );
		}
		if ( taken.Value() )
		{
			return std::move( *taken.Value() );
		}
		if ( std::chrono::steady_clock::now() > deadline )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		std::this_thread::yield();
	}
}

} // namespace hammer::render_adapter
