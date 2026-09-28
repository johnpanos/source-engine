//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.opaque fixtures (RFC 0016 K5, K4 families): a cube
//			mesh of the unlit family's vertex, a mesh resolver, three flat
//			materials drawn by the unlit family over a white texture, the two
//			scenes, and a frame that draws a scene into its own color and depth
//			targets and reads the color back.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H
#define RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H

#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/material/material_programs.h"
#include "render/material/unlit_family.h"
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
constexpr std::uint64_t kPositionOnlyMesh = 2; // stride 12: no family program reads it
constexpr std::uint64_t kMissingMesh = 99;
constexpr std::uint32_t kSize = 64;

constexpr std::uint64_t kUntexturedMaterial = 8; // names a texture never staged
constexpr std::uint64_t kDrawGroupMaterial = 10; // reads a draw group of layout A
constexpr std::uint64_t kOtherLayoutGroup = 5;   // a draw group of layout B

// A unit cube around the origin: 8 corners, 12 triangles, in the unlit
// family's vertex (uv 0, white).
inline resources::MeshData CubeData(
    std::vector<material::UnlitVertex> &vertices, std::vector<std::uint16_t> &indices )
{
	vertices.clear();
	for ( int corner = 0; corner < 8; ++corner )
	{
		material::UnlitVertex vertex;
		vertex.position[0] = ( corner & 1 ) ? 0.5f : -0.5f;
		vertex.position[1] = ( corner & 2 ) ? 0.5f : -0.5f;
		vertex.position[2] = ( corner & 4 ) ? 0.5f : -0.5f;
		vertices.push_back( vertex );
	}
	indices = { 0, 1, 3, 0, 3, 2, 4, 6, 7, 4, 7, 5, 0, 4, 5, 0, 5, 1, 2, 3, 7, 2, 7, 6, 0, 2, 6, 0,
	    6, 4, 1, 5, 7, 1, 7, 3 };
	resources::MeshData data;
	data.vertices = std::as_bytes( std::span<const material::UnlitVertex>( vertices ) );
	data.vertexStride = sizeof( material::UnlitVertex );
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

// Materials 1 red, 2 blue, 3 green: the unlit family's $color over a white
// texture. Material 8 names a texture that is never staged; material 10
// reads a draw group of layout A, and draw group 5 has layout B; anything
// else is unknown.
struct Materials
{
	explicit Materials( device::IRenderDevice2 &device )
	    : device( device ), textures( device ), programs( device, textures ),
	      drawGroups( device, textures )
	{
	}
	~Materials()
	{
		for ( device::BindGroupLayoutId layout : layouts )
			(void)device.Release( layout, device::CompletionToken() );
	}
	device::IRenderDevice2 &device;
	std::unique_ptr<material::UnlitFamily> family;
	resources::TextureCache textures;
	material::MaterialPrograms programs;
	material::DrawGroups drawGroups;
	std::vector<device::BindGroupLayoutId> layouts;
};

inline material::UnlitClaim FlatClaim( float r, float g, float b )
{
	material::UnlitClaim claim;
	claim.claimed = true;
	claim.constants.color[0] = r;
	claim.constants.color[1] = g;
	claim.constants.color[2] = b;
	return claim;
}

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
		changes.Add( result->Reserve(), Cube( { 3.0f, 3.0f, -10.0f }, 1.0f, kUntexturedMaterial ) );
		changes.Add(
		    result->Reserve(), Cube( { 3.0f, -3.0f, -10.0f }, 1.0f, 1, kPositionOnlyMesh ) );
		scene::MeshInstanceDesc noGroup = Cube( { 0.0f, 0.0f, -20.0f }, 1.0f, kDrawGroupMaterial );
		scene::MeshInstanceDesc wrongGroup = noGroup;
		wrongGroup.world = math::Translation( { 1.0f, 0.0f, -20.0f } );
		wrongGroup.drawGroup = kOtherLayoutGroup;
		changes.Add( result->Reserve(), noGroup );
		changes.Add( result->Reserve(), wrongGroup );
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
inline FrameResult DrawScene( device::IRenderDevice2 &device, const Materials &materials,
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
	auto stats = AddOpaquePasses( builder, snapshot, list, view,
	    { meshes, materials.programs, &materials.drawGroups }, targets );
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

// Submits one encoder of `record`'s uploads and waits for it.
template <typename Record>
bool Upload( device::IRenderDevice2 &device, Record record, device::CompletionToken &token )
{
	auto encoder = device.BeginEncoder( device::QueueKind::kGraphics );
	if ( !encoder )
		return false;
	record( encoder.Value() );
	device::CommandEncoder encoders[] = { std::move( encoder ).Value() };
	auto submitted = device.Submit( device::QueueKind::kGraphics, encoders, {} );
	if ( !submitted || !Wait( device, submitted.Value() ) )
		return false;
	token = submitted.Value();
	return true;
}

// Stages the cube and waits for its upload.
inline std::optional<Meshes> StageCube(
    device::IRenderDevice2 &device, resources::MeshCache &cache )
{
	std::vector<material::UnlitVertex> vertices;
	std::vector<std::uint16_t> indices;
	auto entry = cache.Stage( "cube", CubeData( vertices, indices ) );
	resources::MeshData positions = CubeData( vertices, indices );
	std::vector<float> packed;
	for ( const material::UnlitVertex &vertex : vertices )
		packed.insert( packed.end(), vertex.position, vertex.position + 3 );
	positions.vertices = std::as_bytes( std::span<const float>( packed ) );
	positions.vertexStride = 12;
	auto positionOnly = cache.Stage( "positions", positions );
	device::CompletionToken token;
	if ( !entry || !positionOnly ||
	     !Upload(
	         device,
	         [&]( device::CommandEncoder &encoder )
	         {
		         cache.RecordUploads( encoder );
	         },
	         token ) )
		return std::nullopt;
	cache.Retire( token );
	Meshes meshes;
	meshes.entries[kCubeMesh] = entry.Value();
	meshes.entries[kPositionOnlyMesh] = positionOnly.Value();
	return meshes;
}

// The unlit family for `color`, the white texture and materials 1-3 and 8,
// uploaded.
inline std::unique_ptr<Materials> StageMaterials(
    device::IRenderDevice2 &device, device::Format color, device::Format depth )
{
	auto materials = std::make_unique<Materials>( device );
	auto family = material::UnlitFamily::Create( device, color, depth );
	if ( !family )
		return nullptr;
	materials->family = std::move( family ).Value();
	device::TextureDesc white;
	white.format = device::Format::kRGBA8Srgb;
	white.width = white.height = 1;
	white.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
	const std::byte texel[4] = {
	    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
	if ( !materials->textures.Stage( "white", white, texel ) )
		return nullptr;
	const material::UnlitClaim claims[] = {
	    FlatClaim( 1, 0, 0 ), FlatClaim( 0, 0, 1 ), FlatClaim( 0, 1, 0 ) };
	for ( std::uint64_t id = 1; id <= 3; ++id )
	{
		auto request = materials->family->Request( claims[id - 1], "white" );
		if ( !request || !materials->programs.Set( id, request.Value() ) )
			return nullptr;
	}
	auto untextured = materials->family->Request( FlatClaim( 1, 1, 1 ), "never-staged" );
	if ( !untextured || !materials->programs.Set( kUntexturedMaterial, untextured.Value() ) )
		return nullptr;
	// Two draw-group layouts, A and B, of one texture binding each.
	static const device::BindingDesc drawBindings[] = {
	    { 0, device::BindingKind::kSampledTexture, 1, { device::ShaderStage::kFragment } },
	    { 1, device::BindingKind::kSampler, 1, { device::ShaderStage::kFragment } } };
	for ( int i = 0; i < 2; ++i )
	{
		auto layout =
		    device.CreateBindGroupLayout( { device::BindGroupRole::kDraw, drawBindings } );
		if ( !layout )
			return nullptr;
		materials->layouts.push_back( layout.Value() );
	}
	auto grouped = materials->family->Request( FlatClaim( 1, 1, 1 ), "white" );
	if ( !grouped )
		return nullptr;
	grouped.Value().drawLayout = materials->layouts[0];
	material::GroupRequest other;
	other.layout = materials->layouts[1];
	other.textures.push_back( { 0, "white", 1, {} } );
	if ( !materials->programs.Set( kDrawGroupMaterial, grouped.Value() ) ||
	     !materials->drawGroups.Set( kOtherLayoutGroup, other ) )
		return nullptr;
	device::CompletionToken token;
	if ( !Upload(
	         device,
	         [&]( device::CommandEncoder &encoder )
	         {
		         materials->textures.RecordUploads( encoder );
		         materials->programs.RecordUploads( encoder );
		         materials->drawGroups.RecordUploads( encoder );
	         },
	         token ) )
		return nullptr;
	materials->textures.Retire( token );
	materials->programs.Retire( token );
	materials->drawGroups.Retire( token );
	return materials;
}

} // namespace rendertest::opaque

#endif // RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H
