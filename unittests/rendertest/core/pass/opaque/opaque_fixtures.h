//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.opaque fixtures (RFC 0016 K5, K4 families): a cube
//			mesh of the unlit family's vertex, a mesh resolver, three flat
//			materials drawn by the unlit family over a white texture, the two
//			scenes, and a frame that draws a scene into its own color and depth
//			targets and reads the color back. Scene C draws a lightmapped cube
//			with its lightmap page as a draw group, and a pbr cube that reads
//			the frame's split-sum table and the view's model lighting.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H
#define RENDERTEST_CORE_PASS_OPAQUE_FIXTURES_H

#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/material/lightmapped_family.h"
#include "render/material/material_programs.h"
#include "render/material/pbr_family.h"
#include "render/material/unlit_family.h"
#include "render/material/vertexlit_family.h"
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
constexpr std::uint64_t kViewGroupMaterial = 11; // reads a view group the frame lacks

// Scene C: the lightmapped and pbr families with their groups.
constexpr std::uint64_t kLightmappedCubeMesh = 3;
constexpr std::uint64_t kPbrCubeMesh = 4;
constexpr std::uint64_t kLightmappedMaterial = 20;
constexpr std::uint64_t kPbrMaterial = 21;
constexpr std::uint64_t kLightmapPage = 40; // the lightmapped cube's draw group
// The one frame group of the fixtures' surface program: the LDR lightmap
// scale, output scale 1, no fog, the eye at the origin and the split-sum table.
constexpr std::uint64_t kFrameGroup = 41;
// The draw group of a draw that reads no page and no model lighting.
constexpr std::uint64_t kNeutralDraw = 44;
constexpr std::uint64_t kPbrLighting = 42;           // the pbr cube's draw group
constexpr std::uint64_t kVertexLitCubeMesh = 5;
constexpr std::uint64_t kVertexLitMaterial = 22;
constexpr std::uint64_t kVertexLitLighting = 43; // the vertexlit cube's draw group
// The page's texel (sRGB) and the view's ambient cube (every face).
constexpr std::uint8_t kPageTexel[4] = { 128, 64, 32, 255 };
constexpr float kAmbient[3] = { 0.5f, 0.25f, 0.125f };
// The vertexlit cube's ambient cube: its +z face (the face toward the
// camera) only.
constexpr float kVertexLitAmbient[3] = { 0.125f, 0.5f, 0.25f };

// A unit cube around the origin: 8 corners, 12 triangles, in the surface
// program's flat vertex (uv 0, white).
inline resources::MeshData CubeData(
    std::vector<material::SurfaceFlatVertex> &vertices, std::vector<std::uint16_t> &indices )
{
	vertices.clear();
	for ( int corner = 0; corner < 8; ++corner )
	{
		material::SurfaceFlatVertex vertex;
		vertex.position[0] = ( corner & 1 ) ? 0.5f : -0.5f;
		vertex.position[1] = ( corner & 2 ) ? 0.5f : -0.5f;
		vertex.position[2] = ( corner & 4 ) ? 0.5f : -0.5f;
		vertices.push_back( vertex );
	}
	indices = { 0, 1, 3, 0, 3, 2, 4, 6, 7, 4, 7, 5, 0, 4, 5, 0, 5, 1, 2, 3, 7, 2, 7, 6, 0, 2, 6, 0,
	    6, 4, 1, 5, 7, 1, 7, 3 };
	resources::MeshData data;
	data.vertices = std::as_bytes( std::span<const material::SurfaceFlatVertex>( vertices ) );
	data.vertexStride = sizeof( material::SurfaceFlatVertex );
	data.indices = std::as_bytes( std::span<const std::uint16_t>( indices ) );
	data.indexFormat = device::IndexFormat::kUint16;
	return data;
}

// The cube for the lightmapped family: every lightmap coordinate at the
// page's center.
inline resources::MeshData LightmappedCubeData(
    std::vector<material::SurfaceFlatVertex> &vertices, std::vector<std::uint16_t> &indices )
{
	resources::MeshData data = CubeData( vertices, indices );
	for ( material::SurfaceFlatVertex &vertex : vertices )
		vertex.lightmapUv[0] = vertex.lightmapUv[1] = 0.5f;
	return data;
}

// The cube in the pbr family's vertex: four corners per face with the
// face's normal and a tangent along it.
inline resources::MeshData PbrCubeData(
    std::vector<material::SurfaceModelVertex> &vertices, std::vector<std::uint16_t> &indices )
{
	vertices.clear();
	indices.clear();
	for ( int axis = 0; axis < 3; ++axis )
	{
		for ( float sign : { -1.0f, 1.0f } )
		{
			const int u = ( axis + 1 ) % 3;
			const int v = ( axis + 2 ) % 3;
			const std::uint16_t first = std::uint16_t( vertices.size() );
			for ( int corner = 0; corner < 4; ++corner )
			{
				material::SurfaceModelVertex vertex;
				vertex.position[axis] = 0.5f * sign;
				vertex.position[u] = ( corner & 1 ) ? 0.5f : -0.5f;
				vertex.position[v] = ( corner & 2 ) ? 0.5f : -0.5f;
				vertex.normal[axis] = sign;
				vertex.tangent[u] = 1.0f;
				vertex.tangent[3] = 1.0f;
				vertices.push_back( vertex );
			}
			for ( std::uint16_t index : { 0, 1, 3, 0, 3, 2 } )
				indices.push_back( std::uint16_t( first + index ) );
		}
	}
	resources::MeshData data;
	data.vertices = std::as_bytes( std::span<const material::SurfaceModelVertex>( vertices ) );
	data.vertexStride = sizeof( material::SurfaceModelVertex );
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
// reads a draw group of layout A, and draw group 5 has layout B; material 11
// reads a view group, which the frames do not supply; anything else is
// unknown.
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
	// The one surface program every family below borrows (declared first:
	// it outlives them).
	std::unique_ptr<material::SurfaceProgram> surface;
	std::unique_ptr<material::UnlitFamily> family;
	std::unique_ptr<material::LightmappedFamily> lightmapped;
	std::unique_ptr<material::PbrFamily> pbr;
	std::unique_ptr<material::VertexLitFamily> vertexlit;
	resources::TextureCache textures;
	material::MaterialPrograms programs;
	material::DrawGroups drawGroups;
	std::vector<device::BindGroupLayoutId> layouts;
};

inline material::UnlitClaim FlatClaim( float r, float g, float b )
{
	material::UnlitClaim claim;
	claim.claimed = true;
	claim.constants.tint[0] = r;
	claim.constants.tint[1] = g;
	claim.constants.tint[2] = b;
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
	desc.drawGroup = kNeutralDraw;
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
		noGroup.drawGroup = 0;
		scene::MeshInstanceDesc wrongGroup = noGroup;
		wrongGroup.world = math::Translation( { 1.0f, 0.0f, -20.0f } );
		wrongGroup.drawGroup = kOtherLayoutGroup;
		changes.Add( result->Reserve(), noGroup );
		changes.Add( result->Reserve(), wrongGroup );
		changes.Add( result->Reserve(), Cube( { -1.0f, 0.0f, -20.0f }, 1.0f, kViewGroupMaterial ) );
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

// pbrLighting false leaves the pbr cube without its draw group.
inline std::unique_ptr<scene::IRenderScene> SceneC( bool pbrLighting = true )
{
	auto result = scene::CreateRenderScene();
	scene::ChangeSet changes;
	scene::MeshInstanceDesc lit =
	    Cube( { -1.5f, 0.0f, -6.0f }, 1.5f, kLightmappedMaterial, kLightmappedCubeMesh );
	lit.drawGroup = kLightmapPage;
	changes.Add( result->Reserve(), lit );
	scene::MeshInstanceDesc pbr = Cube( { 1.5f, 0.0f, -6.0f }, 1.5f, kPbrMaterial, kPbrCubeMesh );
	pbr.drawGroup = pbrLighting ? kPbrLighting : 0;
	changes.Add( result->Reserve(), pbr );
	scene::MeshInstanceDesc vertexlit =
	    Cube( { 0.0f, 1.6f, -6.0f }, 1.0f, kVertexLitMaterial, kVertexLitCubeMesh );
	vertexlit.drawGroup = kVertexLitLighting;
	changes.Add( result->Reserve(), vertexlit );
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
    const Meshes &meshes, const scene::SceneSnapshot &snapshot,
    const material::DrawGroup *frame = nullptr, const material::DrawGroup *viewGroup = nullptr,
    std::span<const material::DrawGroup *const> frames = {} )
{
	FrameResult result;
	if ( !frame )
		frame = materials.drawGroups.Group( kFrameGroup );
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
	    { meshes, materials.programs, &materials.drawGroups, frame, viewGroup, frames }, targets );
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
	std::vector<material::SurfaceFlatVertex> vertices;
	std::vector<std::uint16_t> indices;
	auto entry = cache.Stage( "cube", CubeData( vertices, indices ) );
	resources::MeshData positions = CubeData( vertices, indices );
	std::vector<float> packed;
	for ( const material::SurfaceFlatVertex &vertex : vertices )
		packed.insert( packed.end(), vertex.position, vertex.position + 3 );
	positions.vertices = std::as_bytes( std::span<const float>( packed ) );
	positions.vertexStride = 12;
	auto positionOnly = cache.Stage( "positions", positions );
	std::vector<material::SurfaceFlatVertex> lightmappedVertices;
	std::vector<std::uint16_t> lightmappedIndices;
	auto lightmapped = cache.Stage(
	    "lightmapped-cube", LightmappedCubeData( lightmappedVertices, lightmappedIndices ) );
	std::vector<material::SurfaceModelVertex> pbrVertices;
	std::vector<std::uint16_t> pbrIndices;
	auto pbr = cache.Stage( "pbr-cube", PbrCubeData( pbrVertices, pbrIndices ) );
	device::CompletionToken token;
	if ( !entry || !positionOnly || !lightmapped || !pbr ||
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
	meshes.entries[kLightmappedCubeMesh] = lightmapped.Value();
	meshes.entries[kPbrCubeMesh] = pbr.Value();
	meshes.entries[kVertexLitCubeMesh] = pbr.Value(); // the model vertex serves both
	return meshes;
}

// Scene C's families, textures, programs and groups, and the frame group: a
// lightmapped material over the white texture whose draw group holds the
// page, a pbr
// dielectric (white base, MRAO rough and unoccluded) whose frame group holds
// the split-sum table and whose draw group holds a uniform ambient cube, and
// a vertexlit material over the white texture whose draw group holds its
// lighting (an ambient cube lit on +z only, no lights).
inline bool StageSceneCMaterials( Materials &materials )
{
	materials.lightmapped = std::make_unique<material::LightmappedFamily>( *materials.surface );
	materials.pbr = std::make_unique<material::PbrFamily>( *materials.surface );
	materials.vertexlit = std::make_unique<material::VertexLitFamily>( *materials.surface );
	auto stage = [&]( const char *name, device::Format format, std::uint32_t size,
	                 std::span<const std::byte> pixels )
	{
		device::TextureDesc desc;
		desc.format = format;
		desc.width = desc.height = size;
		desc.usages = { device::ResourceUsage::kCopyDestination, device::ResourceUsage::kSampled };
		return materials.textures.Stage( name, desc, pixels ).HasValue();
	};
	const std::byte mrao[4] = {
	    std::byte( 0 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
	const material::PbrSplitSumTable table = material::SplitSumTable();
	if ( !stage(
	         "page", device::Format::kRGBA8Srgb, 1, std::as_bytes( std::span( kPageTexel ) ) ) ||
	     !stage( "mrao", device::Format::kRGBA8Unorm, 1, mrao ) ||
	     !stage( "splitsum", table.format, table.width,
	         std::as_bytes( std::span<const float>( table.texels ) ) ) )
		return false;

	material::LightmappedClaim litClaim;
	litClaim.claimed = true;
	auto lit = materials.lightmapped->Request( litClaim, "white" );
	if ( !lit || !materials.programs.Set( kLightmappedMaterial, lit.Value() ) ||
	     !materials.drawGroups.Set(
	         kLightmapPage, materials.lightmapped->LightmapGroup( "page" ) ) )
		return false;

	material::PbrClaim pbrClaim;
	pbrClaim.claimed = true;
	material::SurfaceTextures pbrTextures;
	pbrTextures.base = "white";
	pbrTextures.mrao = "mrao";
	auto program = materials.pbr->Request( pbrClaim, pbrTextures );
	const float eye[3] = {};
	float cube[6][3];
	for ( auto &face : cube )
		std::copy( kAmbient, kAmbient + 3, face );
	const material::ModelLighting lighting = material::PackSourceModelLighting( eye, cube, {} );
	if ( !program || !materials.programs.Set( kPbrMaterial, program.Value() ) ||
	     !materials.drawGroups.Set(
	         kFrameGroup, materials.pbr->FrameGroup( material::SurfaceFrame(), "splitsum" ) ) ||
	     !materials.drawGroups.Set( kPbrLighting, materials.pbr->LightingGroup( lighting ) ) )
		return false;

	material::VertexLitClaim vertexLitClaim;
	vertexLitClaim.claimed = true;
	auto vertexLitProgram = materials.vertexlit->Request( vertexLitClaim, "white" );
	float faces[6][3] = {};
	std::copy( kVertexLitAmbient, kVertexLitAmbient + 3, faces[4] );
	const material::ModelLighting vertexLitLighting =
	    material::PackSourceModelLighting( eye, faces, {} );
	return vertexLitProgram &&
	       materials.programs.Set( kVertexLitMaterial, vertexLitProgram.Value() ) &&
	       materials.drawGroups.Set(
	           kVertexLitLighting, materials.vertexlit->LightingGroup( vertexLitLighting ) );
}

// The unlit family for `color`, the white texture and materials 1-3 and 8,
// and scene C's materials, uploaded.
inline std::unique_ptr<Materials> StageMaterials(
    device::IRenderDevice2 &device, device::Format color, device::Format depth )
{
	auto materials = std::make_unique<Materials>( device );
	auto surface = material::SurfaceProgram::Create( device, color, depth );
	if ( !surface )
		return nullptr;
	materials->surface = std::move( surface ).Value();
	materials->family = std::make_unique<material::UnlitFamily>( *materials->surface );
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
	auto viewed = materials->family->Request( FlatClaim( 1, 1, 1 ), "white" );
	if ( !viewed )
		return nullptr;
	viewed.Value().viewLayout = materials->layouts[0];
	if ( !materials->programs.Set( kViewGroupMaterial, viewed.Value() ) ||
	     !materials->programs.Set( kDrawGroupMaterial, grouped.Value() ) ||
	     !materials->drawGroups.Set( kOtherLayoutGroup, other ) )
		return nullptr;
	if ( !StageSceneCMaterials( *materials ) ||
	     !materials->drawGroups.Set( kNeutralDraw, materials->family->DrawGroup() ) )
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
