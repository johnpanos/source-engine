//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/adapters/render/viewport_renderer.h.
//
//=============================================================================//

#include "viewport_renderer.h"

#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/scene/draw_list.h"

#include <array>
#include <chrono>
#include <cmath>
#include <set>
#include <span>
#include <string>
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

constexpr device::Format kColorFormat = device::Format::kRGBA8Srgb;
constexpr device::Format kDepthFormat = device::Format::kD32Float;
constexpr std::uint64_t kUntextured = 1; // the untextured batch's material and mesh id
constexpr const char *kWhite = "hammer-white";

// The clear colors as the editor shows them, in linear light for the sRGB target.
float Linear( float display )
{
	return display <= 0.04045f ? display / 12.92f : std::pow( ( display + 0.055f ) / 1.055f, 2.4f );
}

device::ClearColor Clear( float r, float g, float b )
{
	return { Linear( r ), Linear( g ), Linear( b ), 1.0f };
}

// The unlit family reads vertex colors as gamma 2.2 (Source's convention); the
// editor's shading colors are display (sRGB) values. This table re-encodes a
// display byte so the family's decode gives its sRGB linear value, and the
// sRGB target shows the byte again (within a level of quantization).
const std::array<std::uint8_t, 256> &Gamma22FromDisplay()
{
	static const std::array<std::uint8_t, 256> table = []
	{
		std::array<std::uint8_t, 256> out{};
		for ( int i = 0; i < 256; ++i )
		{
			const double linear = Linear( float( i ) / 255.0f );
			out[i] = std::uint8_t( std::lround( std::pow( linear, 1.0 / 2.2 ) * 255.0 ) );
		}
		return out;
	}();
	return table;
}

std::string TextureName( const std::string &material )
{
	return "hammer-texture:" + material;
}

::render::math::Aabb BoundsOf( const std::vector<UnlitVertex> &vertices )
{
	::render::math::Aabb bounds;
	for ( const UnlitVertex &v : vertices )
	{
		const ::render::math::float3 p = { v.position[0], v.position[1], v.position[2] };
		if ( bounds.IsEmpty() )
		{
			bounds.min = bounds.max = p;
			continue;
		}
		bounds.min = { std::min( bounds.min.x, p.x ), std::min( bounds.min.y, p.y ),
		    std::min( bounds.min.z, p.z ) };
		bounds.max = { std::max( bounds.max.x, p.x ), std::max( bounds.max.y, p.y ),
		    std::max( bounds.max.z, p.z ) };
	}
	return bounds;
}

} // namespace

ViewportRenderer::ViewportRenderer( device::IRenderDevice2 &device, IMaterialTextures *textures )
    : m_Device( device ), m_Source( textures ), m_MeshCache( device ), m_Textures( device ),
      m_Programs( device, m_Textures ), m_Scene( ::render::scene::CreateRenderScene() )
{
}

foundation::Expected<std::unique_ptr<ViewportRenderer>, ViewportStatus> ViewportRenderer::Create(
    device::IRenderDevice2 &device, IMaterialTextures *textures )
{
	std::unique_ptr<ViewportRenderer> renderer( new ViewportRenderer( device, textures ) );
	auto lines = pass::lines::LinesRenderer::Create( device, kColorFormat, kDepthFormat );
	auto unlit = material::UnlitFamily::Create( device, kColorFormat, kDepthFormat );
	if ( !lines || !unlit )
	{
		return foundation::MakeUnexpected( ViewportStatus::kPass );
	}
	renderer->m_Lines = std::move( lines ).Value();
	renderer->m_Unlit = std::move( unlit ).Value();
	device::TextureDesc white;
	white.format = kColorFormat;
	white.width = white.height = 1;
	white.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	const std::byte texel[4] = {
	    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
	if ( !renderer->m_Textures.Stage( kWhite, white, texel ) ||
	     !renderer->AddProgram( kUntextured, kWhite ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
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

// The unlit family's program for a material id: its base texture (a
// TextureCache name) modulated by the vertex colors (the editor's shading).
foundation::Expected<std::uint64_t, ViewportStatus> ViewportRenderer::AddProgram(
    std::uint64_t id, const std::string &texture )
{
	material::UnlitClaim claim;
	claim.claimed = true;
	claim.constants.flags[0] = 1.0f; // $vertexcolor
	auto request = m_Unlit->Request( claim, texture );
	if ( !request || !m_Programs.Set( id, request.Value() ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	return id;
}

// Fetches the base texture of every material the snapshot names for the first
// time (misses are kept too).
foundation::Expected<void, ViewportStatus> ViewportRenderer::ResolveMaterials(
    const viewport::RenderSnapshot &snapshot )
{
	if ( !m_Source )
	{
		return {};
	}
	std::set<std::string> named;
	for ( const viewport::SolidDraw &solid : snapshot.solids )
	{
		for ( const viewport::FaceDraw &face : solid.faces )
		{
			if ( !face.material.empty() && m_Materials.count( face.material ) == 0 )
			{
				named.insert( face.material );
			}
		}
	}
	for ( const std::string &name : named )
	{
		Material material;
		std::optional<MaterialImage> image = m_Source->BaseTexture( name );
		if ( image && image->width > 0 && image->height > 0 &&
		     image->rgba.size() == std::size_t( image->width ) * image->height * 4 )
		{
			device::TextureDesc desc;
			desc.format = kColorFormat;
			desc.width = image->width;
			desc.height = image->height;
			desc.usages = {
			    device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
			const std::string texture = TextureName( name );
			if ( !m_Textures.Stage( texture, desc, std::as_bytes( std::span( image->rgba ) ) ) )
			{
				return foundation::MakeUnexpected( ViewportStatus::kDevice );
			}
			material.id = m_NextMaterial++;
			if ( auto added = AddProgram( material.id, texture ); !added )
			{
				return foundation::MakeUnexpected( added.Error() );
			}
			material.size = TextureSize{ image->width, image->height };
			++m_Stats.textures;
		}
		else
		{
			++m_Stats.missingTextures;
		}
		m_Materials.emplace( name, material );
	}
	return {};
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
	if ( auto resolved = ResolveMaterials( snapshot ); !resolved )
	{
		return resolved;
	}
	const SceneGeometry geometry = BuildSceneGeometry( snapshot,
	    [this]( const std::string &material ) -> std::optional<TextureSize>
	    {
		    auto found = m_Materials.find( material );
		    return found == m_Materials.end() ? std::nullopt : found->second.size;
	    } );

	// One mesh and one scene instance per batch, keyed by its material id.
	::render::scene::ChangeSet changes;
	for ( ::render::scene::InstanceId instance : m_Instances )
	{
		changes.Remove( instance );
	}
	m_Instances.clear();
	std::map<std::uint64_t, ::render::resources::MeshEntry> meshes;
	std::uint32_t faceVertices = 0;
	std::uint32_t textured = 0;
	for ( const FaceBatch &batch : geometry.faces )
	{
		const std::uint64_t id =
		    batch.material.empty() ? kUntextured : m_Materials.at( batch.material ).id;
		std::vector<UnlitVertex> vertices = batch.vertices;
		const std::array<std::uint8_t, 256> &encode = Gamma22FromDisplay();
		for ( UnlitVertex &vertex : vertices )
		{
			for ( int c = 0; c < 3; ++c )
				vertex.color[c] = encode[vertex.color[c]];
		}
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const UnlitVertex>( vertices ) );
		data.vertexStride = sizeof( UnlitVertex );
		auto entry = m_MeshCache.Stage( "hammer-faces:" + std::to_string( id ), data );
		if ( !entry )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		meshes[id] = entry.Value();
		::render::scene::MeshInstanceDesc desc;
		desc.mesh = id;
		desc.material = id;
		desc.localBounds = BoundsOf( batch.vertices );
		const ::render::scene::InstanceId instance = m_Scene->Reserve();
		changes.Add( instance, desc );
		m_Instances.push_back( instance );
		faceVertices += static_cast<std::uint32_t>( batch.vertices.size() );
		textured += batch.material.empty() ? 0 : 1;
	}
	for ( const auto &[id, entry] : m_FaceMeshes.entries )
	{
		if ( meshes.count( id ) == 0 )
		{
			(void)m_MeshCache.Evict( "hammer-faces:" + std::to_string( id ) );
		}
	}
	m_FaceMeshes.entries = std::move( meshes );
	if ( !m_Scene->Commit( changes ) )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	if ( geometry.edges.empty() )
	{
		if ( m_Edges )
		{
			(void)m_MeshCache.Evict( "hammer-edges" );
		}
		m_Edges.reset();
	}
	else
	{
		resources::MeshData data;
		data.vertices = std::as_bytes( std::span<const LineVertex>( geometry.edges ) );
		data.vertexStride = sizeof( LineVertex );
		auto entry = m_MeshCache.Stage( "hammer-edges", data );
		if ( !entry )
		{
			return foundation::MakeUnexpected( ViewportStatus::kDevice );
		}
		m_Edges = entry.Value();
	}

	// The uploads run in their own submission; later frames on the queue see
	// them, and replaced buffers and groups retire behind its completion.
	auto encoder = m_Device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !encoder )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	m_MeshCache.RecordUploads( encoder.Value() );
	m_Textures.RecordUploads( encoder.Value() );
	m_Programs.RecordUploads( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = m_Device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !token )
	{
		return foundation::MakeUnexpected( ViewportStatus::kDevice );
	}
	m_MeshCache.Retire( token.Value() );
	m_Textures.Retire( token.Value() );
	m_Programs.Retire( token.Value() );
	m_LastToken = token.Value();
	m_Bounds = snapshot.bounds;
	m_HaveScene = true;
	m_Stats.key = key;
	m_Stats.triangles = geometry.triangles;
	m_Stats.faceVertices = faceVertices;
	m_Stats.edgeVertices = static_cast<std::uint32_t>( geometry.edges.size() );
	m_Stats.batches = static_cast<std::uint32_t>( geometry.faces.size() );
	m_Stats.texturedBatches = textured;
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
	colorDesc.format = kColorFormat;
	colorDesc.width = request.pixelWidth;
	colorDesc.height = request.pixelHeight;
	colorDesc.usages = {
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
	colorDesc.debugName = "hammer-viewport";
	device::TextureDesc depthDesc = colorDesc;
	depthDesc.format = kDepthFormat;
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
	targets.clear = threeD ? Clear( 0.13f, 0.14f, 0.17f ) : Clear( 0.03f, 0.03f, 0.04f );

	// First pass (clearing): the solids through the families in the camera
	// view, the grid in 2D views.
	m_LastView = ViewStats();
	if ( threeD )
	{
		::render::scene::ViewDesc viewDesc;
		viewDesc.projection = view.worldToClip; // the view matrix stays identity
		const ::render::scene::SceneView sceneView = ::render::scene::MakeView( viewDesc );
		const std::shared_ptr<const ::render::scene::SceneSnapshot> snapshot = m_Scene->Snapshot();
		const ::render::scene::DrawList list =
		    ::render::scene::BuildDrawList( *snapshot, sceneView );
		pass::opaque::OpaqueTargets opaque{
		    colorRef, depthRef, request.pixelWidth, request.pixelHeight, targets.clear };
		auto drawn = pass::opaque::AddOpaquePasses(
		    builder, *snapshot, list, sceneView, { m_FaceMeshes, m_Programs }, opaque );
		if ( !drawn )
		{
			Release( pending );
			return foundation::MakeUnexpected( ViewportStatus::kPass );
		}
		m_LastView = { drawn.Value().drawn, drawn.Value().unresolved };
	}
	else
	{
		LineList grid;
		AppendGrid( request.grid, view.width, view.height, grid );
		if ( !m_Lines->AddPasses( builder, grid, {}, view, targets ) )
		{
			Release( pending );
			return foundation::MakeUnexpected( ViewportStatus::kPass );
		}
	}

	// Then the edges (depth-tested and biased in the camera view) and the
	// tool overlay.
	LineList overlay;
	AppendOverlay( request.overlay, overlay );
	std::vector<MeshBatch> batches;
	if ( m_Edges )
	{
		batches.push_back( { *m_Edges, Topology::kLines, { Space::kWorld, threeD } } );
	}
	LinesTargets scene = targets;
	scene.clearColor = false;
	scene.clearDepth = false;
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
