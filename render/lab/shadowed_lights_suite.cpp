//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite shadowed-lights (RFC 0016 K11 "Model assembly",
//			the direct-visibility term, render.shadows.v1): the surface
//			program's clustered runtime lights shadowed through the view's
//			atlas. Two box casters over the receiver plane; two spots with
//			atlas tiles (render.pass.shadows plans the atlas and its depth
//			pass draws the casters) and a point light without one. The
//			oracle shares no code with the shader or the depth pass: each
//			light's RuntimeLightOracle times its visibility, a ray test from
//			the receiver point to the light against the boxes, in float.
//			- Where every point within the filter's reach of a pixel's point
//			  (2.5 atlas texels, seen along the light) has the same
//			  visibility, the pixel is within 0.5 percent + 3e-4 of the
//			  oracle, widened by how far the oracle moves within 0.1 units
//			  (the clustered-light suite's band, fixed before the first run).
//			  Elsewhere (a shadow's edge) it lies between the edge lights
//			  fully shadowed and fully lit, with the same band.
//			- Each spot shadows at least 50 judged pixels by more than four
//			  bands, and the point light none.
//			- Lights with no tile are the frame without an atlas, bitwise.
//
//			Seeded programs (--sensitivity): the visibility ignored, each
//			light reading the next light's tile, and the depth compare
//			reversed.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_compute.h"
#include "lab_receiver.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/graph/compiled_graph.h"
#include "render/graph/executor.h"
#include "render/graph/graph_builder.h"
#include "render/light_set.h"
#include "render/material/pbr_family.h"
#include "render/math/matrix.h"
#include "render/pass/lights/clusters.h"
#include "render/pass/shadows/atlas.h"
#include "render/pass/shadows/shadow_passes.h"
#include "render/pass/shadows/shadow_plan.h"
#include "render/pass/shadows/shadow_views.h"
#include "render/resources/mesh_cache.h"
#include "spv/shadowed_light_defects_spv.h"
#include "spv/shadow_cube_probe_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using math::float3;
namespace shadows = render::pass::shadows;

constexpr std::uint32_t kSize = 128;
constexpr float kReceiver = 700.0f;
constexpr int kSampleStep = 2;
constexpr std::uint32_t kTileSize = 16;
constexpr std::uint32_t kAtlasSize = 1024;
constexpr std::uint32_t kGuard = 2;
// The receiver depth bias is in the tile's clip depth, whose world size grows
// with distance squared over the near plane: at a near plane of 1 it was about
// 32 units at the receiver's 400 (the first run lit points that far behind a
// caster's face). A near plane of 16, well short of every caster, makes it
// about 2 units. A bias in world units is render.shadows.v1's open item.
constexpr float kShadowNear = 16.0f;
constexpr float kDepthBias = 2e-4f;

constexpr float kRelative = 0.005f;
constexpr float kAbsolute = 3e-4f;
constexpr float kConditioning = 0.1f; // world units of position uncertainty
constexpr float kFilterTexels = 2.5f;
constexpr int kShadowedMinimum = 50;

struct Box
{
	float3 center;
	float3 half;
};

const Box kCasters[] = {
    { { 0, 0, 60 }, { 60, 30, 12 } }, { { -150, 110, 45 }, { 25, 25, 45 } } };

// Whether the segment from p to q passes through a box (slab test).
bool Blocked( float3 p, float3 q )
{
	const float3 d{ q.x - p.x, q.y - p.y, q.z - p.z };
	for ( const Box &box : kCasters )
	{
		float enter = 0.0f;
		float exit = 1.0f;
		bool miss = false;
		const float origin[3] = { p.x, p.y, p.z };
		const float direction[3] = { d.x, d.y, d.z };
		const float lo[3] = {
		    box.center.x - box.half.x, box.center.y - box.half.y, box.center.z - box.half.z };
		const float hi[3] = {
		    box.center.x + box.half.x, box.center.y + box.half.y, box.center.z + box.half.z };
		for ( int k = 0; k < 3 && !miss; ++k )
		{
			if ( std::fabs( direction[k] ) < 1e-12f )
			{
				miss = origin[k] < lo[k] || origin[k] > hi[k];
				continue;
			}
			float a = ( lo[k] - origin[k] ) / direction[k];
			float b = ( hi[k] - origin[k] ) / direction[k];
			if ( a > b )
				std::swap( a, b );
			enter = std::max( enter, a );
			exit = std::min( exit, b );
			miss = enter > exit;
		}
		if ( !miss && exit > 1e-4f && enter < 1.0f - 1e-4f )
			return true;
	}
	return false;
}

light_set::RuntimeLight Light( float3 at, float3 direction, float innerDegrees,
    float outerDegrees, float color, float radius, light_set::LightFalloff falloff,
    float minLight = 0.0f )
{
	light_set::RuntimeLight light;
	light.id = 1;
	light.kind = light_set::LightKind::Dynamic;
	light.shape =
	    outerDegrees > 0.0f ? light_set::LightShape::Spot : light_set::LightShape::Point;
	std::memcpy( light.position, &at, sizeof( light.position ) );
	light.color[0] = light.color[1] = light.color[2] = color;
	light.radius = radius;
	light.falloff = falloff;
	light.minLight = minLight;
	light.sourceRadius = light_set::kInverseSquareSourceRadius;
	if ( outerDegrees > 0.0f )
	{
		const float3 unit = math::Normalize( direction );
		std::memcpy( light.direction, &unit, sizeof( light.direction ) );
		light.innerCos = std::cos( innerDegrees * 3.14159265f / 180.0f );
		light.outerCos = std::cos( outerDegrees * 3.14159265f / 180.0f );
	}
	return light;
}

// A light of the scene and its shadow view (spots only).
struct SceneLight
{
	light_set::RuntimeLight light;
	std::optional<shadows::ShadowView> shadow;
	int tile = -1; // its record in the view's tile list
	float tanHalfAngle = 0.0f;
};

std::vector<SceneLight> SceneLights()
{
	using light_set::LightFalloff;
	std::vector<SceneLight> lights = {
	    { Light( { 40, -30, 320 }, { -0.05f, 0.04f, -1.0f }, 28, 40, 4.0f, 1000,
	          LightFalloff::InverseSquare ),
	        {}, 0, 0 },
	    { Light( { -320, 250, 260 }, { 260, -210, -260 }, 25, 35, 0.8f, 900, LightFalloff::Legacy,
	          0.02f ),
	        {}, 1, 0 },
	    { Light( { 250, -200, 150 }, {}, 0, 0, 0.6f, 500, LightFalloff::Legacy, 0.02f ), {}, -1,
	        0 } };
	for ( SceneLight &scene : lights )
	{
		if ( scene.tile < 0 )
			continue;
		const light_set::RuntimeLight &l = scene.light;
		auto view = shadows::BuildSpotShadowView(
		    { { l.position[0], l.position[1], l.position[2] },
		        { l.direction[0], l.direction[1], l.direction[2] }, l.outerCos, kShadowNear,
		        l.radius } );
		if ( view )
			scene.shadow = view.Value();
		scene.tanHalfAngle = std::tan( std::acos( l.outerCos ) );
	}
	return lights;
}

// A light's visibility at p over the filter's reach: 1 lit, 0 shadowed, or
// empty on a shadow's edge. `texels` is the tile's viewport size.
std::optional<float> Visibility( const SceneLight &scene, float3 p, std::uint32_t texels )
{
	const float3 at{ scene.light.position[0], scene.light.position[1], scene.light.position[2] };
	const bool centre = Blocked( p, at );
	if ( scene.tile < 0 )
		return 1.0f;
	const float3 toLight{ at.x - p.x, at.y - p.y, at.z - p.z };
	const float distance = std::sqrt( math::Dot( toLight, toLight ) );
	const float texel = 2.0f * distance * scene.tanHalfAngle / float( texels );
	const float reach = kFilterTexels * texel / std::max( toLight.z / distance, 0.25f );
	for ( int k = 0; k < 8; ++k )
	{
		const float angle = float( k ) * 3.14159265f / 4.0f;
		const float3 q{ p.x + reach * std::cos( angle ), p.y + reach * std::sin( angle ), 0.0f };
		if ( Blocked( q, at ) != centre )
			return std::nullopt;
	}
	return centre ? 0.0f : 1.0f;
}

struct Lab
{
	std::unique_ptr<LabClusterLists> clusters;
	IRenderDevice2 &device;
	resources::TextureCache textures;
	resources::MeshCache meshes;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	std::unique_ptr<material::PbrFamily> family;
	std::unique_ptr<shadows::ShadowDepthRenderer> depth;
	std::map<std::string, std::uint64_t> materialGroups;
	std::uint64_t drawGroup = 0;
	std::uint64_t nextGroup = 1;
	std::vector<std::byte> receiver;
	TextureId atlas;
	TextureDesc atlasDesc;
	resources::MeshEntry cube;
	std::vector<SceneLight> lights;
	std::vector<ShadowTileGpu> tiles;
	std::uint32_t tileTexels = 0; // the spots' viewport size

	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), meshes( d ), groups( d, textures )
	{
	}
	~Lab()
	{
		if ( atlas.IsValid() )
			(void)device.Release( atlas, CompletionToken() );
	}
};

// Plans the atlas and draws the casters into the spots' tiles; the atlas is
// left in kSampled.
std::optional<std::string> DrawAtlas( Lab &lab )
{
	std::vector<float> corners;
	for ( int corner = 0; corner < 8; ++corner )
	{
		corners.push_back( ( corner & 1 ) ? 0.5f : -0.5f );
		corners.push_back( ( corner & 2 ) ? 0.5f : -0.5f );
		corners.push_back( ( corner & 4 ) ? 0.5f : -0.5f );
	}
	const std::uint16_t indices[] = { 0, 1, 3, 0, 3, 2, 4, 6, 7, 4, 7, 5, 0, 4, 5, 0, 5, 1, 2, 3, 7,
	    2, 7, 6, 0, 2, 6, 0, 6, 4, 1, 5, 7, 1, 7, 3 };
	resources::MeshData mesh;
	mesh.vertices = std::as_bytes( std::span<const float>( corners ) );
	mesh.vertexStride = 12;
	mesh.indices = std::as_bytes( std::span( indices ) );
	mesh.indexFormat = IndexFormat::kUint16;
	auto cube = lab.meshes.Stage( "sl/cube", mesh );
	if ( !cube )
		return std::string( "the caster mesh was refused" );
	lab.cube = cube.Value();

	shadows::ShadowAtlasLimits limits;
	limits.atlasSize = kAtlasSize;
	limits.minTileSize = 128;
	limits.maxTileSize = 512;
	limits.casterBudget = 8;
	limits.guardTexels = kGuard;
	std::vector<shadows::ShadowRequest> requests;
	for ( const SceneLight &scene : lab.lights )
	{
		if ( scene.tile >= 0 )
			requests.push_back( { std::uint64_t( scene.tile + 1 ), 2.0f - float( scene.tile ),
			    1.0f } );
	}
	auto plan = shadows::PlanShadowAtlas( limits, requests );
	if ( !plan )
		return std::string( "the atlas plan was refused" );
	std::vector<shadows::ShadowCaster> casters;
	for ( const Box &box : kCasters )
		casters.push_back( { lab.cube, math::Multiply( math::Translation( box.center ),
		                                   math::Scale( { 2 * box.half.x, 2 * box.half.y,
		                                       2 * box.half.z } ) ) } );
	std::vector<shadows::ShadowDepthView> views;
	lab.tiles.assign( requests.size(), {} );
	for ( const SceneLight &scene : lab.lights )
	{
		if ( scene.tile < 0 )
			continue;
		const shadows::ShadowAllocation &allocation = plan.Value().allocations[scene.tile];
		if ( !allocation.HasTile() || !scene.shadow )
			return std::string( "a spot has no tile" );
		views.push_back( { scene.shadow->viewProjection, allocation.tile, casters } );
		lab.tiles[scene.tile] = shadows::PackShadowTile(
		    shadows::MakeTileProjection(
		        scene.shadow->viewProjection, allocation.tile, kAtlasSize, kGuard ),
		    kAtlasSize, kDepthBias );
		lab.tileTexels = shadows::TileViewport( allocation.tile, kGuard ).size;
	}

	lab.atlasDesc.format = Format::kD32Float;
	lab.atlasDesc.width = lab.atlasDesc.height = kAtlasSize;
	lab.atlasDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
	lab.atlasDesc.debugName = "render_lab.shadowed-lights.atlas";
	auto atlas = lab.device.CreateTexture( lab.atlasDesc );
	if ( !atlas )
		return std::string( "the atlas was refused" );
	lab.atlas = atlas.Value();
	lab.atlasDesc.debugName = {};

	// The mesh upload first; then the depth pass as a graph.
	auto encoded = lab.device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return std::string( "no encoder" );
	lab.meshes.RecordUploads( encoded.Value() );
	CommandEncoder encoders[] = { std::move( encoded ).Value() };
	auto uploaded = lab.device.Submit( QueueKind::kGraphics, encoders, {} );
	if ( !uploaded )
		return std::string( "the mesh upload was refused" );
	(void)lab.device.WaitIdle();
	lab.meshes.Retire( uploaded.Value() );

	graph::GraphBuilder builder;
	const graph::ResourceRef atlasRef = builder.ImportTexture(
	    "atlas", lab.atlas, lab.atlasDesc, ResourceUsage::kUndefined, ResourceUsage::kSampled );
	auto stats = lab.depth->AddPasses( builder, { atlasRef, kAtlasSize, kGuard }, views );
	if ( !stats || stats.Value().views != views.size() )
		return std::string( "the depth passes were refused" );
	auto compiled = graph::CompileGraph( std::move( builder ) );
	if ( !compiled )
		return std::string( "the depth graph did not compile" );
	graph::SerialGraphExecutor executor;
	auto executed = executor.Execute( compiled.Value(), lab.device );
	if ( !executed )
		return std::string( "the depth graph did not execute" );
	(void)lab.device.WaitIdle();
	lab.depth->Collect( executed.Value().token );
	return std::nullopt;
}

std::optional<std::string> Prepare( Lab &lab, std::span<const std::uint32_t> module )
{
	if ( std::optional<std::string> why = Canvas::Create( lab.device, kSize, kSize, lab.canvas ) )
		return why;
	auto family = material::CreateSurfaceFamily<material::PbrFamily>(
	    lab.device, kCanvasColor, kCanvasDepth, 1, module );
	if ( !family )
		return std::string( "the surface program was refused" );
	lab.family = std::move( family ).Value();
	auto depth = shadows::ShadowDepthRenderer::Create( lab.device );
	if ( !depth )
		return std::string( "the shadow depth renderer was refused" );
	lab.depth = std::move( depth ).Value();
	bool staged = StageConstant(
	    lab.textures, "sl/base", Format::kRGBA8Srgb, ByteTexel( 255, 255, 255, 255 ) );
	for ( const ReceiverMaterial &m : kReceiverMaterials )
		staged = staged && StageConstant( lab.textures, std::string( "sl/mrao-" ) + m.name,
		                       Format::kRGBA8Unorm, ByteTexel( m.metal, m.roughness, 255, 255 ) );
	const material::PbrSplitSumTable table = material::SplitSumTable();
	TextureDesc desc;
	desc.format = table.format;
	desc.width = table.width;
	desc.height = table.height;
	staged = staged &&
	         lab.textures.Stage( "sl/splitsum", desc, std::as_bytes( std::span( table.texels ) ) )
	             .HasValue();
	if ( !staged )
		return std::string( "a fixture texture was refused" );
	for ( const ReceiverMaterial &m : kReceiverMaterials )
	{
		material::PbrClaim claim;
		claim.claimed = true;
		material::SurfaceTextures textures;
		textures.base = "sl/base";
		textures.mrao = std::string( "sl/mrao-" ) + m.name;
		auto request = lab.family->Request( claim, textures );
		if ( !request )
			return std::string( "the pbr point was refused" );
		const std::uint64_t id = lab.nextGroup++;
		if ( !lab.groups.Set( id, request.Value().material ) )
			return std::string( "a material group was refused" );
		lab.materialGroups[m.name] = id;
	}
	lab.drawGroup = lab.nextGroup++;
	if ( !lab.groups.Set( lab.drawGroup, lab.family->LightingGroup( material::ModelLighting() ) ) )
		return std::string( "the draw group was refused" );
	lab.receiver = ReceiverMesh( kReceiver );
	lab.lights = SceneLights();
	return DrawAtlas( lab );
}

struct Frame
{
	const ReceiverMaterial *material = &kReceiverMaterials[0];
	const ReceiverView *view = nullptr;
	bool shadowed = true; // the spots index their tiles
	bool atlas = true;    // the view group binds the atlas and tiles
	shaderlib::DebugSpecialization debug = {};
};

std::optional<material::GroupRequest> ViewGroup( Lab &lab, const Frame &frame )
{
	pass::lights::ClusterLimits limits = pass::lights::DesktopClusterLimits();
	limits.tileSizePixels = kTileSize;
	pass::lights::ClusterViewDesc desc;
	desc.view = frame.view->view;
	desc.projection = frame.view->projection;
	desc.widthPixels = desc.heightPixels = kSize;
	desc.nearZ = frame.view->nearZ;
	desc.farZ = frame.view->farZ;
	auto grid = pass::lights::CreateClusterGrid( desc, limits );
	if ( !grid )
		return std::nullopt;
	std::vector<light_set::RuntimeLight> lights;
	for ( const SceneLight &scene : lab.lights )
		lights.push_back( scene.light );
	lab.clusters = LabClusterLists::Create( lab.device, grid.Value(), lights );
	if ( !lab.clusters )
		return std::nullopt;
	material::SurfaceViewGpu view;
	view.grid[0] = grid.Value().tilesX;
	view.grid[1] = grid.Value().tilesY;
	view.grid[2] = grid.Value().slices;
	view.grid[3] = kTileSize;
	view.slices[0] = grid.Value().sliceScale;
	view.slices[1] = grid.Value().sliceBias;
	view.slices[2] = grid.Value().nearZ;
	const math::float4 &z = frame.view->view.rows[2];
	view.viewDistance[0] = -z.x;
	view.viewDistance[1] = -z.y;
	view.viewDistance[2] = -z.z;
	view.viewDistance[3] = -z.w;
	std::vector<material::SurfaceLightGpu> records;
	for ( const SceneLight &scene : lab.lights )
		records.push_back(
		    material::PackSurfaceLight( scene.light, frame.shadowed ? scene.tile : -1 ) );
	material::SurfaceShadows shadowing;
	if ( frame.atlas )
		shadowing = { lab.atlas, lab.atlasDesc, lab.tiles };
	auto request = lab.family->Program().ViewGroup( view, {}, {}, records, shadowing );
	lab.clusters->Bind( request );
	return request;
}

std::optional<std::string> Render( Lab &lab, const Frame &frame, CanvasImage &image )
{
	material::SurfaceFrame terms;
	terms.eye[0] = frame.view->eye.x;
	terms.eye[1] = frame.view->eye.y;
	terms.eye[2] = frame.view->eye.z;
	const std::uint64_t frameGroup = lab.nextGroup++;
	const std::uint64_t viewGroup = lab.nextGroup++;
	const std::optional<material::GroupRequest> view = ViewGroup( lab, frame );
	if ( !view )
		return std::string( "the cluster lists were not built" );
	if ( !lab.groups.Set( frameGroup, lab.family->FrameGroup( terms, "sl/splitsum" ) ) ||
	     !lab.groups.Set( viewGroup, *view ) )
		return std::string( "a frame or view group was refused" );
	if ( std::optional<std::string> why =
	         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
		return why;
	material::PbrClaim claim;
	claim.claimed = true;
	material::SurfaceVariant variant = claim.Variant();
	variant.terms |= material::kSurfaceClustered;
	auto pipeline = lab.family->Program().Pipeline( variant, frame.debug );
	if ( !pipeline )
		return std::string( "no pipeline" );
	auto group = [&]( std::uint64_t id ) -> BindGroupId
	{
		const material::ResidentGroup *resident = lab.groups.Group( id );
		return resident ? resident->group : BindGroupId();
	};
	CanvasDraw draw;
	draw.pipeline = pipeline.Value();
	draw.groups[std::size_t( BindGroupRole::kFrame )] = group( frameGroup );
	draw.groups[std::size_t( BindGroupRole::kView )] = group( viewGroup );
	draw.groups[std::size_t( BindGroupRole::kMaterial )] =
	    group( lab.materialGroups.at( frame.material->name ) );
	draw.groups[std::size_t( BindGroupRole::kDraw )] = group( lab.drawGroup );
	draw.vertices = lab.canvas->Vertices( lab.receiver );
	draw.vertexCount = 6;
	material::FamilyDrawConstants constants;
	std::memcpy( constants.toClip, &frame.view->toClip, sizeof( constants.toClip ) );
	constants.world[0] = constants.world[5] = constants.world[10] = constants.world[15] = 1.0f;
	const auto *bytes = reinterpret_cast<const std::byte *>( &constants );
	draw.constants.assign( bytes, bytes + sizeof( constants ) );
	const CanvasDraw draws[] = { draw };
	std::optional<std::string> why =
	    lab.canvas->Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image );
	lab.groups.Remove( frameGroup );
	lab.groups.Remove( viewGroup );
	return why;
}

bool SameImage( const CanvasImage &a, const CanvasImage &b )
{
	return a.rgba.size() == b.rgba.size() &&
	       std::memcmp( a.rgba.data(), b.rgba.data(), a.rgba.size() * sizeof( float ) ) == 0;
}

std::string Format4( const char *format, double a, double b = 0, double c = 0, double d = 0 )
{
	char text[200];
	std::snprintf( text, sizeof( text ), format, a, b, c, d );
	return text;
}

// The oracle's bounds at p: each light's term times its visibility, edge
// lights at 0 for the lower and 1 for the upper bound.
struct Bounds
{
	float lo = 0.0f;
	float hi = 0.0f;
	bool edge = false;
};

Bounds Oracle( const Lab &lab, const ReceiverMaterial &material, float3 p, float3 v,
    std::vector<float> *shadowedBy = nullptr )
{
	Bounds bounds;
	for ( std::size_t i = 0; i < lab.lights.size(); ++i )
	{
		const float term = RuntimeLightOracle( lab.lights[i].light, material, p, v );
		const std::optional<float> visible = Visibility( lab.lights[i], p, lab.tileTexels );
		if ( !visible )
		{
			bounds.edge = true;
			bounds.hi += term;
			continue;
		}
		bounds.lo += term * *visible;
		bounds.hi += term * *visible;
		if ( shadowedBy && *visible == 0.0f )
			( *shadowedBy )[i] = term;
	}
	return bounds;
}

std::optional<std::string> ShadowedChecks( Lab &lab, Results &results )
{
	const ReceiverView overhead = MakeReceiverView( { 0, -60, 420 }, { -40, 20, 0 }, kSize );
	const ReceiverView oblique = MakeReceiverView( { 250, -380, 220 }, { -60, 40, 0 }, kSize );
	const std::pair<const char *, const ReceiverView *> views[] = {
	    { "overhead", &overhead }, { "oblique", &oblique } };
	const ReceiverMaterial *materials[] = { &kReceiverMaterials[0], &kReceiverMaterials[2] };

	std::vector<int> shadowedPixels( lab.lights.size(), 0 );
	for ( const ReceiverMaterial *material : materials )
	{
		for ( const auto &[viewName, view] : views )
		{
			CanvasImage image;
			if ( std::optional<std::string> why =
			         Render( lab, { material, view, true, true }, image ) )
				return why;
			int pixels = 0, edges = 0, bad = 0;
			float worst = 0.0f;
			for ( std::uint32_t y = 0; y < kSize; y += kSampleStep )
			{
				for ( std::uint32_t x = 0; x < kSize; x += kSampleStep )
				{
					const std::optional<float3> p = ReceiverHit( *view, x, y, kReceiver );
					if ( !p )
						continue;
					std::vector<float> shadowedBy( lab.lights.size(), 0.0f );
					const Bounds expected = Oracle( lab, *material, *p, ToEye( *view, *p ),
					    &shadowedBy );
					float moved = 0.0f;
					for ( const float3 offset :
					    { float3{ kConditioning, 0, 0 }, float3{ -kConditioning, 0, 0 },
					        float3{ 0, kConditioning, 0 }, float3{ 0, -kConditioning, 0 } } )
					{
						const float3 q{ p->x + offset.x, p->y + offset.y, 0.0f };
						const Bounds near = Oracle( lab, *material, q, ToEye( *view, q ) );
						moved = std::max( { moved, std::fabs( near.lo - expected.lo ),
						    std::fabs( near.hi - expected.hi ) } );
					}
					const float band = kRelative * expected.hi + kAbsolute + moved;
					const float value = image.At( x, y )[0];
					++pixels;
					edges += expected.edge ? 1 : 0;
					const float error =
					    std::max( { expected.lo - value, value - expected.hi, 0.0f } );
					worst = std::max( worst, error / std::max( expected.hi, 1e-3f ) );
					bad += error > band ? 1 : 0;
					if ( !expected.edge )
					{
						for ( std::size_t i = 0; i < lab.lights.size(); ++i )
							shadowedPixels[i] += shadowedBy[i] > 4.0f * band ? 1 : 0;
					}
				}
			}
			results.That( pixels > 1000 && bad == 0,
			    std::string( "shadowed." ) + material->name + "." + viewName,
			    Format4( "%.0f of %.0f outside the tolerance (%.0f on edges), worst relative %.3g",
			        bad, pixels, edges, worst ) );
		}
	}
	for ( std::size_t i = 0; i < lab.lights.size(); ++i )
	{
		const bool spot = lab.lights[i].tile >= 0;
		results.That( spot ? shadowedPixels[i] >= kShadowedMinimum : shadowedPixels[i] == 0,
		    "coverage.light-" + std::to_string( i ) + ( spot ? "-shadows" : "-casts-no-shadow" ),
		    Format4( "%.0f judged pixels shadowed", shadowedPixels[i] ) );
	}

	// Lights with no tile are the frame without an atlas, bitwise; the atlas
	// changes it.
	CanvasImage untiled, withoutAtlas, shadowed;
	for ( auto [frame, image] :
	    { std::pair{ Frame{ &kReceiverMaterials[0], &overhead, false, true }, &untiled },
	        std::pair{ Frame{ &kReceiverMaterials[0], &overhead, false, false }, &withoutAtlas },
	        std::pair{ Frame{ &kReceiverMaterials[0], &overhead, true, true }, &shadowed } } )
	{
		if ( std::optional<std::string> why = Render( lab, frame, *image ) )
			return why;
	}
	results.That( SameImage( untiled, withoutAtlas ), "neutral.untiled-lights-ignore-the-atlas" );
	results.That( !SameImage( untiled, shadowed ), "neutral.a-shadowed-frame-differs" );
	Frame visibilityOff{ &kReceiverMaterials[0], &overhead, true, true };
	visibilityOff.debug.termsOff = shaderlib::kDebugTermShadowVisibility;
	CanvasImage unshadowed;
	if ( std::optional<std::string> why = Render( lab, visibilityOff, unshadowed ) )
		return why;
	results.That(
	    SameImage( untiled, unshadowed ), "debug.shadow-visibility-off-is-untiled-bitwise" );
	return std::nullopt;
}

std::optional<std::string> SoftFilterChecks( Lab &lab, Results &results )
{
	// Hardware comparison sampling must preserve the complete soft filter,
	// with the existing 0.5 percent + 3e-4 image band, including
	// penumbra pixels the independent hard-shadow ray oracle
	// above deliberately leaves as a bounded edge. The reference compiles
	// four integer-addressed depth comparisons on the same immutable inputs;
	// no product selects this oracle.
	Lab reference( lab.device );
	if ( std::optional<std::string> why =
	         Prepare( reference, spirv::kSurfaceShadowReferenceCompare ) )
		return why;
	const ReceiverView overhead = MakeReceiverView( { 0, -60, 420 }, { -40, 20, 0 }, kSize );
	const ReceiverView oblique = MakeReceiverView( { 250, -380, 220 }, { -60, 40, 0 }, kSize );
	for ( const float radius : { 4.0f, 32.0f } )
	{
		for ( Lab *fixture : { &lab, &reference } )
		{
			for ( SceneLight &light : fixture->lights )
			{
				light.light.sourceRadius = radius;
				if ( light.tile >= 0 )
				{
					fixture->tiles[light.tile].params[2] = kShadowNear;
					fixture->tiles[light.tile].params[3] = light.light.radius;
				}
			}
		}
		for ( const ReceiverMaterial *material :
		    { &kReceiverMaterials[0], &kReceiverMaterials[2] } )
		{
			for ( const ReceiverView *view : { &overhead, &oblique } )
			{
				CanvasImage actual, expected;
				const Frame frame{ material, view, true, true };
				for ( const auto &[fixture, image] :
				    { std::pair{ &lab, &actual }, std::pair{ &reference, &expected } } )
				{
					if ( std::optional<std::string> why = Render( *fixture, frame, *image ) )
						return why;
				}
				std::size_t bad = 0;
				float worst = 0;
				for ( std::size_t i = 0; i < std::min( actual.rgba.size(), expected.rgba.size() );
				    ++i )
				{
					const float error = std::fabs( actual.rgba[i] - expected.rgba[i] );
					worst = std::max( worst, error );
					bad += !std::isfinite( actual.rgba[i] ) || !std::isfinite( expected.rgba[i] ) ||
					       error > kAbsolute + kRelative * std::fabs( expected.rgba[i] );
				}
				results.That( actual.rgba.size() == kSize * kSize * 4 &&
				                  actual.rgba.size() == expected.rgba.size() && bad == 0,
				    std::string( "soft-filter." ) + material->name + "." +
				        ( view == &overhead ? "overhead" : "oblique" ) + "." +
				        std::to_string( int( radius ) ),
				    Format4( "%.0f channels differ; max error %.9g", bad, worst ) );
			}
		}
	}
	return std::nullopt;
}

std::optional<std::string> CubeChecks( Lab &lab, Results &results )
{
	Lab fixture( lab.device ); // owns the private atlas until every dispatch completes
	light_set::RuntimeLight light;
	light.shape = light_set::LightShape::Point;
	light.radius = 256.0f;
	shadows::ShadowPlanInput input;
	input.lights = { &light, 1 };
	input.atlasSize = 2048;
	shadows::ShadowPlan plan;
	if ( auto why = shadows::PlanShadows( input, plan ) )
		return why;
	if ( plan.tiles.size() != 6 || plan.lightTiles[0] != 0 ||
	     plan.lightLayouts[0] != RuntimeShadowLayout::kWorldCube )
		return std::string( "the cube planner did not publish all six world-axis faces" );

	// Distinct, constant depths per face make a wrong face observable, without
	// a seam between two rasterizations obscuring which lookup was chosen.
	std::vector<float> depth( input.atlasSize * input.atlasSize, 1.0f );
	for ( std::size_t face = 0; face < plan.views.size(); ++face )
	{
		const shadows::ShadowTile &tile = plan.views[face].tile;
		for ( std::uint32_t y = tile.y; y < tile.y + tile.size; ++y )
			for ( std::uint32_t x = tile.x; x < tile.x + tile.size; ++x )
				depth[y * input.atlasSize + x] = face % 2 ? 1.0f : 0.2f;
	}
	fixture.atlasDesc.format = Format::kD32Float;
	fixture.atlasDesc.width = fixture.atlasDesc.height = input.atlasSize;
	fixture.atlasDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	auto atlas = lab.device.CreateTexture( fixture.atlasDesc );
	if ( !atlas )
		return std::string( "the cube oracle atlas was refused" );
	fixture.atlas = atlas.Value();
	BufferDesc upload;
	upload.size = depth.size() * sizeof( float );
	upload.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	auto staging = lab.device.CreateBuffer( upload );
	auto encoded = lab.device.BeginEncoder( QueueKind::kGraphics );
	if ( !staging || !encoded )
	{
		if ( staging )
			(void)lab.device.Release( staging.Value(), {} );
		return std::string( "the cube oracle upload was refused" );
	}
	CommandEncoder &e = encoded.Value();
	e.TransitionBuffer(
	    staging.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.WriteBuffer( staging.Value(), 0, std::as_bytes( std::span( depth ) ) );
	e.TransitionBuffer(
	    staging.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	e.TransitionTexture(
	    fixture.atlas, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	e.CopyBufferToTexture(
	    staging.Value(), fixture.atlas, { 0, 0, 0, input.atlasSize, input.atlasSize } );
	e.TransitionTexture( fixture.atlas, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	auto token = lab.device.Submit( QueueKind::kGraphics, { &e, 1 }, {} );
	(void)lab.device.WaitIdle();
	(void)lab.device.Release( staging.Value(), token ? token.Value() : CompletionToken() );
	if ( !token )
		return std::string( "the cube oracle upload did not submit" );

	std::vector<math::float4> points;
	for ( float radius : { 0.0f, 4.0f, 32.0f } )
	{
		for ( int z = -4; z <= 4; ++z )
			for ( int y = -4; y <= 4; ++y )
				for ( int x = -4; x <= 4; ++x )
					points.push_back(
					    { float( x * 16 ), float( y * 16 ), float( z * 16 ), radius } );
	}
	// Neighbouring directions on either side of every face boundary.
	for ( int sign : { -1, 1 } )
		for ( float epsilon : { -0.01f, 0.0f, 0.01f } )
		{
			points.push_back( { float( sign ) * 64, 64 + epsilon, 0, 4 } );
			points.push_back( { 0, float( sign ) * 64, 64 + epsilon, 4 } );
			points.push_back( { 64 + epsilon, 0, float( sign ) * 64, 4 } );
		}
	std::vector<std::byte> cases(
	    plan.tiles.size() * sizeof( ShadowTileGpu ) + points.size() * sizeof( math::float4 ) );
	std::memcpy( cases.data(), plan.tiles.data(), plan.tiles.size() * sizeof( ShadowTileGpu ) );
	std::memcpy( cases.data() + plan.tiles.size() * sizeof( ShadowTileGpu ), points.data(),
	    points.size() * sizeof( math::float4 ) );
	SamplerDesc samplers[2];
	samplers[0].minFilter = samplers[0].magFilter = samplers[0].mipFilter = Filter::kNearest;
	samplers[0].address = samplers[1].address = AddressMode::kClampToEdge;
	samplers[1].mipFilter = Filter::kNearest;
	samplers[1].comparison = CompareOp::kLessEqual;
	for ( bool seeded : { false, true } )
	{
		CheckKernel kernel( lab.device );
		if ( auto why = kernel.Create(
		         seeded ? std::span<const std::uint32_t>( spirv::kShadowCubeProbeNext )
		                : std::span<const std::uint32_t>( spirv::kShadowCubeProbe ),
		         1, 2, "shadow cube lookup", samplers ) )
			return why;
		std::vector<std::byte> output;
		const TextureId textures[] = { fixture.atlas };
		if ( auto why = kernel.Run( fixture.textures, textures, std::uint32_t( points.size() ),
		         cases, points.size() * sizeof( math::float2 ), output ) )
			return why;
		std::size_t bad = 0, lit = 0, shadowed = 0;
		for ( std::size_t i = 0; i < points.size(); ++i )
		{
			math::float2 value;
			std::memcpy( &value, output.data() + i * sizeof( value ), sizeof( value ) );
			bad += !std::isfinite( value.x ) || !std::isfinite( value.y ) || value.x != value.y;
			lit += value.y == 1.0f;
			shadowed += value.y == 0.0f;
		}
		results.That( seeded ? bad > 0 : bad == 0 && lit > 100 && shadowed > 100,
		    seeded ? "cube.next-face-rejected" : "cube.matches-matrix-search",
		    Format4( "%.0f directions, %.0f mismatches, %.0f lit, %.0f shadowed", points.size(),
		        bad, lit, shadowed ) );
	}
	return std::nullopt;
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		if ( std::optional<std::string> why = Prepare( lab, module ) )
			return why;
		if ( std::optional<std::string> why = ShadowedChecks( lab, results ) )
			return why;
		if ( std::optional<std::string> why = SoftFilterChecks( lab, results ) )
			return why;
		if ( std::optional<std::string> why = CubeChecks( lab, results ) )
			return why;
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kShadowedSeeded[] = { { "shadow-ignored", spirv::kSurfaceShadowIgnored, "shadowed." },
    { "tile-next", spirv::kSurfaceShadowTileNext, "shadowed." },
    { "depth-reversed", spirv::kSurfaceShadowDepthReversed, "shadowed." },
    { "unstable-gather", spirv::kSurfaceShadowUnstableGather, "soft-filter." } };

} // namespace

int RunShadowedLightsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "shadowed-lights", kShadowedSeeded, RunOnce );
}

} // namespace render::lab
