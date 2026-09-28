//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.opaque fixtures (RFC 0016 K5): a cube mesh, mesh and
//			color resolvers, the two scenes, and a frame that draws a scene
//			into its own color and depth targets and reads the color back.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H
#define RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H

#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/pass/opaque/opaque.h"
#include "render/scene/scene.h"

#include <chrono>
#include <cstring>
#include <map>
#include <optional>
#include <thread>
#include <vector>

namespace rendertest::opaque
{

using namespace render;
using namespace render::pass::opaque;

constexpr std::uint64_t kCubeMesh = 1;
constexpr std::uint64_t kMissingMesh = 99;
constexpr std::uint32_t kSize = 64;

// A unit cube around the origin: 8 corners, 12 triangles.
inline resources::MeshData CubeData(
    std::vector<float> &vertices, std::vector<std::uint16_t> &indices )
{
	vertices.clear();
	for ( int corner = 0; corner < 8; ++corner )
	{
		vertices.push_back( ( corner & 1 ) ? 0.5f : -0.5f );
		vertices.push_back( ( corner & 2 ) ? 0.5f : -0.5f );
		vertices.push_back( ( corner & 4 ) ? 0.5f : -0.5f );
	}
	indices = { 0, 1, 3, 0, 3, 2, 4, 6, 7, 4, 7, 5, 0, 4, 5, 0, 5, 1, 2, 3, 7, 2, 7, 6, 0, 2, 6, 0,
	    6, 4, 1, 5, 7, 1, 7, 3 };
	resources::MeshData data;
	data.vertices = std::as_bytes( std::span<const float>( vertices ) );
	data.vertexStride = 12;
	data.indices = std::as_bytes( std::span<const std::uint16_t>( indices ) );
	data.indexFormat = device::IndexFormat::kUint16;
	return data;
}

class Meshes final : public IMeshResolver
{
public:
	std::map<std::uint64_t, resources::MeshEntry> entries;
	const resources::MeshEntry *Mesh( std::uint64_t mesh ) const override
	{
		auto found = entries.find( mesh );
		return found == entries.end() ? nullptr : &found->second;
	}
};

// Materials 1 red, 2 blue, 3 green; anything else is unknown.
class Colors final : public IMaterialColors
{
public:
	bool Color( std::uint64_t material, float out[4] ) const override
	{
		static const float table[4][4] = { {}, { 1, 0, 0, 1 }, { 0, 0, 1, 1 }, { 0, 1, 0, 1 } };
		if ( material == 0 || material > 3 )
			return false;
		std::memcpy( out, table[material], sizeof( table[material] ) );
		return true;
	}
};

inline scene::MeshInstanceDesc Cube(
    math::float3 center, float size, std::uint64_t material, std::uint64_t mesh = kCubeMesh )
{
	scene::MeshInstanceDesc desc;
	desc.mesh = mesh;
	desc.material = material;
	desc.world = math::Multiply( math::Translation( center ), math::Scale( { size, size, size } ) );
	desc.localBounds = { { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } };
	return desc;
}

// Scene A: a blue cube left of center, and a smaller red cube in front of
// it. Draws sort by material, so the far blue cube draws after the near red
// one: only the depth test keeps the red cube visible.
// Scene B: a green cube right of center. The camera sits at the origin
// looking down -Z (render.math: right-handed, clip depth 0..1, Y up).
inline std::unique_ptr<scene::IRenderScene> SceneA( bool withMissing = false )
{
	auto result = scene::CreateRenderScene();
	scene::ChangeSet changes;
	changes.Add( result->Reserve(), Cube( { -2.0f, 0.0f, -10.0f }, 3.0f, 2 ) );
	changes.Add( result->Reserve(), Cube( { -2.0f, 0.0f, -6.0f }, 0.8f, 1 ) );
	if ( withMissing )
	{
		changes.Add( result->Reserve(), Cube( { 0.0f, 3.0f, -10.0f }, 1.0f, 1, kMissingMesh ) );
		changes.Add( result->Reserve(), Cube( { 0.0f, -3.0f, -10.0f }, 1.0f, 7 ) );
	}
	(void)result->Commit( changes );
	return result;
}

inline std::unique_ptr<scene::IRenderScene> SceneB()
{
	auto result = scene::CreateRenderScene();
	scene::ChangeSet changes;
	changes.Add( result->Reserve(), Cube( { 2.5f, 0.0f, -8.0f }, 2.0f, 3 ) );
	(void)result->Commit( changes );
	return result;
}

inline scene::SceneView View()
{
	scene::ViewDesc desc;
	desc.projection = math::Perspective( 1.0f, 1.0f, 0.5f, 100.0f );
	return scene::MakeView( desc );
}

// The pixel a world point projects to (row 0 at the top).
inline std::pair<int, int> Pixel( const scene::SceneView &view, math::float3 point )
{
	const math::float4 clip =
	    math::Transform( view.viewProjection, { point.x, point.y, point.z, 1.0f } );
	const float x = clip.x / clip.w;
	const float y = clip.y / clip.w;
	return { int( ( x * 0.5f + 0.5f ) * kSize ), int( ( 0.5f - y * 0.5f ) * kSize ) };
}

inline bool Wait( device::IRenderDevice2 &device, device::CompletionToken token )
{
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
	while ( !device.IsComplete( token ) )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		(void)device.Poll();
		std::this_thread::yield();
	}
	(void)device.Poll();
	return true;
}

struct FrameResult
{
	bool ok = false;
	OpaqueStats stats;
	std::vector<std::uint8_t> rgba; // kSize x kSize, row 0 at the top
	std::uint32_t passes = 0;
};

// Draws `snapshot` into fresh targets and reads the color back.
inline FrameResult DrawScene( device::IRenderDevice2 &device, OpaqueRenderer &renderer,
    const Meshes &meshes, const scene::SceneSnapshot &snapshot )
{
	FrameResult result;
	const scene::SceneView view = View();
	const scene::DrawList list = scene::BuildDrawList( snapshot, view );
	device::TextureDesc colorDesc;
	colorDesc.format = device::Format::kRGBA8Unorm;
	colorDesc.width = colorDesc.height = kSize;
	colorDesc.usages = {
	    device::ResourceUsage::kColorAttachment, device::ResourceUsage::kCopySource };
	device::TextureDesc depthDesc = colorDesc;
	depthDesc.format = device::Format::kD32Float;
	depthDesc.usages = { device::ResourceUsage::kDepthWrite };
	device::BufferDesc readbackDesc;
	readbackDesc.size = kSize * kSize * 4;
	readbackDesc.usages = { device::ResourceUsage::kCopyDestination };
	readbackDesc.memory = device::MemoryKind::kReadback;
	auto color = device.CreateTexture( colorDesc );
	auto readback = device.CreateBuffer( readbackDesc );
	if ( !color || !readback )
		return result;

	graph::GraphBuilder builder;
	const graph::ResourceRef colorRef = builder.ImportTexture( "color", color.Value(), colorDesc,
	    device::ResourceUsage::kUndefined, device::ResourceUsage::kCopySource );
	const graph::ResourceRef depthRef = builder.CreateTexture( "depth", depthDesc );
	const graph::ResourceRef readbackRef = builder.ImportBuffer( "readback", readback.Value(),
	    readbackDesc, device::ResourceUsage::kUndefined, device::ResourceUsage::kCopyDestination );
	OpaqueTargets targets{ colorRef, depthRef, kSize, kSize, { 0.0f, 0.0f, 0.0f, 1.0f } };
	auto stats = renderer.AddPasses( builder, snapshot, list, view, meshes, Colors(), targets );
	if ( stats )
	{
		result.stats = stats.Value();
		builder.AddPass( "readback", graph::PassKind::kCopy )
		    .Read( colorRef, device::ResourceUsage::kCopySource )
		    .Write( readbackRef, device::ResourceUsage::kCopyDestination )
		    .SideEffect()
		    .Execute(
		        [colorRef, readbackRef]( graph::RecordContext &context )
		        {
			        context.Encoder().CopyTextureToBuffer( context.Texture( colorRef ),
			            context.Buffer( readbackRef ), { 0, 0, 0, kSize, kSize } );
		        } );
		auto compiled = graph::CompileGraph( std::move( builder ) );
		if ( compiled )
		{
			graph::SerialGraphExecutor executor;
			auto executed = executor.Execute( compiled.Value(), device );
			if ( executed && Wait( device, executed.Value().token ) )
			{
				renderer.Collect( executed.Value().token );
				result.passes = executed.Value().passes;
				result.rgba.resize( kSize * kSize * 4 );
				result.ok =
				    device
				        .ReadBuffer( readback.Value(), 0,
				            std::as_writable_bytes( std::span<std::uint8_t>( result.rgba ) ) )
				        .HasValue();
			}
		}
	}
	(void)device.Release( color.Value(), device::CompletionToken() );
	(void)device.Release( readback.Value(), device::CompletionToken() );
	(void)device.Poll();
	return result;
}

// Stages the cube and waits for its upload.
inline std::optional<Meshes> StageCube(
    device::IRenderDevice2 &device, resources::MeshCache &cache )
{
	std::vector<float> vertices;
	std::vector<std::uint16_t> indices;
	auto entry = cache.Stage( "cube", CubeData( vertices, indices ) );
	auto encoder = device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !entry || !encoder )
		return std::nullopt;
	cache.RecordUploads( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto token = device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !token || !Wait( device, token.Value() ) )
		return std::nullopt;
	cache.Retire( token.Value() );
	Meshes meshes;
	meshes.entries[kCubeMesh] = entry.Value();
	return meshes;
}

} // namespace rendertest::opaque

#endif // RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H
