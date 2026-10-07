//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shadow-mask suite (RFC 0016 Source 2 lighting
//			defaults): a static light's baked shadow mask (LSMK) and the moving
//			casters in its reach, on the world PBR point.
//
//			One lightmapped world quad (a black page, so only the runtime
//			light shows) under a point light whose shadow tile's atlas is
//			cleared to depth 0 (wherever the tile is read, the light is
//			shadowed), and a mask page holding the light's id at visibility 1
//			in every texel. The light's record selects the cases:
//			- the mask alone (no mover): lit;
//			- no mask: the tile, dark;
//			- a mover in reach with every point reading the tile: dark;
//			- a mover sphere far from every pixel's path to the light: lit,
//			  as the mask alone;
//			- a mover sphere on some paths: each pixel dark where its path to
//			  the light passes within the sphere grown by the light's source
//			  radius, else lit (an analytic oracle per pixel, a band at the
//			  boundary left out).
//			The seeded program that ignores the movers' spheres (every point
//			of a light with a mover reads the tile) must fail the far case.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/light_set.h"
#include "render/material/surface_program.h"
#include "render/math/matrix.h"
#include "render/pass/lights/clusters.h"
#include "render/shadow_tile.h"
#include "spv/shadow_mask_defects_spv.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace device;
using math::float3;

constexpr std::uint32_t kSize = 64;
constexpr float kHalf = 100.0f; // the quad spans -kHalf..kHalf at z = 0
constexpr float kLightHeight = 150.0f;
constexpr float kSourceRadius = 2.0f;
constexpr std::uint32_t kMaskId = 1;

std::vector<std::byte> HalfPage( float value )
{
	std::vector<std::byte> out;
	for ( int i = 0; i < 16; ++i )
		for ( int k = 0; k < 4; ++k )
		{
			const std::uint16_t half = k == 3 ? 0x3c00 : ( value == 0.0f ? 0 : 0x3c00 );
			const auto *bytes = reinterpret_cast<const std::byte *>( &half );
			out.insert( out.end(), bytes, bytes + 2 );
		}
	return out;
}

std::vector<std::byte> MaskPage()
{
	// Channel 0: the light's id with visibility 255; the others empty.
	std::vector<std::byte> out;
	for ( int i = 0; i < 16; ++i )
		for ( int k = 0; k < 4; ++k )
		{
			const std::uint16_t value = k == 0 ? std::uint16_t( ( kMaskId << 8 ) | 255u ) : 0;
			const auto *bytes = reinterpret_cast<const std::byte *>( &value );
			out.insert( out.end(), bytes, bytes + 2 );
		}
	return out;
}

std::vector<std::byte> WorldQuad()
{
	std::vector<std::byte> out;
	const float corners[4][2] = {
	    { -kHalf, -kHalf }, { kHalf, -kHalf }, { kHalf, kHalf }, { -kHalf, kHalf } };
	for ( int corner : { 0, 1, 2, 0, 2, 3 } )
	{
		material::SurfaceWorldVertex vertex;
		vertex.position[0] = corners[corner][0];
		vertex.position[1] = corners[corner][1];
		vertex.uv[0] = vertex.lightmapUv[0] = corners[corner][0] > 0 ? 1.0f : 0.0f;
		vertex.uv[1] = vertex.lightmapUv[1] = corners[corner][1] > 0 ? 1.0f : 0.0f;
		vertex.normal[2] = 1.0f;
		vertex.tangentS[0] = 1.0f;
		vertex.tangentT[1] = 1.0f;
		const auto *bytes = reinterpret_cast<const std::byte *>( &vertex );
		out.insert( out.end(), bytes, bytes + sizeof( vertex ) );
	}
	return out;
}

struct Case
{
	int maskId = int( kMaskId );
	bool mover = false;
	std::uint32_t bits = material::kSurfaceMoversEverywhere;
	float sphere[4] = {};
};

struct Lab
{
	IRenderDevice2 &device;
	resources::TextureCache textures;
	material::GroupResidency groups;
	std::unique_ptr<Canvas> canvas;
	std::unique_ptr<material::SurfaceProgram> program;
	TextureId atlas;
	TextureDesc atlasDesc;
	std::vector<ShadowTileGpu> tiles;
	math::float4x4 view, projection, toClip, fromClip;
	std::uint64_t nextGroup = 1;
	explicit Lab( IRenderDevice2 &d ) : device( d ), textures( d ), groups( d, textures ) {}
	~Lab()
	{
		(void)device.WaitIdle();
		if ( atlas.IsValid() )
			(void)device.Release( atlas, {} );
	}
};

light_set::RuntimeLight Light()
{
	light_set::RuntimeLight light;
	light.position[2] = kLightHeight;
	light.color[0] = light.color[1] = light.color[2] = 4.0f;
	light.radius = 1000.0f;
	light.falloff = light_set::LightFalloff::InverseSquare;
	light.sourceRadius = kSourceRadius;
	return light;
}

std::optional<std::string> Prepare( Lab &lab, std::span<const std::uint32_t> module )
{
	if ( auto why = Canvas::Create( lab.device, kSize, kSize, lab.canvas ) )
		return why;
	auto program =
	    material::SurfaceProgram::Create( lab.device, kCanvasColor, kCanvasDepth, 1, module );
	if ( !program )
		return std::string( "the surface program was refused" );
	lab.program = std::move( program ).Value();
	TextureDesc page;
	page.format = Format::kRGBA16Float;
	page.width = page.height = 4;
	page.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	TextureDesc mask = page;
	mask.format = Format::kRGBA16Unorm;
	bool staged = lab.textures.Stage( "sm/page", page, HalfPage( 0.0f ) ).HasValue() &&
	              lab.textures.Stage( "sm/gradient", page, HalfPage( 0.0f ) ).HasValue() &&
	              lab.textures.Stage( "sm/mask", mask, MaskPage() ).HasValue() &&
	              StageConstant( lab.textures, "sm/base", Format::kRGBA8Srgb,
	                  ByteTexel( 255, 255, 255, 255 ) );
	const material::PbrSplitSumTable table = material::SplitSumTable();
	TextureDesc tableDesc;
	tableDesc.format = table.format;
	tableDesc.width = table.width;
	tableDesc.height = table.height;
	staged =
	    staged &&
	    lab.textures.Stage( "sm/splitsum", tableDesc, std::as_bytes( std::span( table.texels ) ) )
	        .HasValue();
	if ( !staged )
		return std::string( "a fixture texture was refused" );
	// The light's tile: looking down from the light over the whole quad, its
	// atlas cleared to depth 0 (every receiver behind a blocker).
	lab.atlasDesc.format = Format::kD32Float;
	lab.atlasDesc.width = lab.atlasDesc.height = 256;
	lab.atlasDesc.usages = { ResourceUsage::kDepthWrite, ResourceUsage::kSampled };
	auto atlas = lab.device.CreateTexture( lab.atlasDesc );
	if ( !atlas )
		return std::string( "the shadow atlas was refused" );
	lab.atlas = atlas.Value();
	ShadowTileGpu tile;
	const auto lightView =
	    math::Multiply( math::Perspective( 150.0f * 3.14159265f / 180.0f, 1.0f, 1.0f, 1000.0f ),
	        math::LookAt( { 0, 0, kLightHeight }, { 0, 0, 0 }, { 0, 1, 0 } ) );
	std::memcpy( tile.viewProjection, &lightView, sizeof( tile.viewProjection ) );
	tile.transform[0] = 0.5f;
	tile.transform[1] = 0.5f;
	tile.transform[2] = 0.5f;
	tile.transform[3] = 0.5f;
	tile.bounds[2] = tile.bounds[3] = 1.0f;
	tile.params[0] = 0.0005f;
	tile.params[1] = float( lab.atlasDesc.width );
	tile.params[2] = 1.0f;
	tile.params[3] = 1000.0f;
	lab.tiles = { tile };
	CanvasPost clear = [&]( CommandEncoder &encoder, TextureId,
	                       TextureId ) -> std::optional<std::string>
	{
		encoder.TransitionTexture(
		    lab.atlas, ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
		RenderingDesc desc;
		desc.depth = DepthAttachment{ lab.atlas, LoadOp::kClear, StoreOp::kStore, 0.0f };
		desc.width = desc.height = lab.atlasDesc.width;
		encoder.BeginRendering( desc );
		encoder.EndRendering();
		encoder.TransitionTexture( lab.atlas, ResourceUsage::kDepthWrite, ResourceUsage::kSampled );
		return std::nullopt;
	};
	if ( auto why =
	         lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr, clear ) )
		return why;
	lab.view = math::LookAt( { 0, 0, 300 }, { 0, 0, 0 }, { 0, 1, 0 } );
	lab.projection = math::Perspective( 50.0f * 3.14159265f / 180.0f, 1.0f, 1.0f, 2000.0f );
	lab.toClip = math::Multiply( lab.projection, lab.view );
	lab.fromClip = *math::Inverse( lab.toClip );
	return std::nullopt;
}

std::optional<std::string> Render( Lab &lab, const Case &c, CanvasImage &image )
{
	material::SurfaceVariant variant;
	variant.layout = material::SurfaceVertexLayout::kWorld;
	variant.terms =
	    material::kSurfacePbr | material::kSurfaceBakedLightmap | material::kSurfaceClustered;
	variant.viewFeatures = 0;
	material::SurfaceConstants constants;
	material::SurfaceTextures textures;
	textures.base = "sm/base";
	auto request = lab.program->Request( variant, constants, textures );
	if ( !request )
		return std::string( "the world pbr point was refused" );
	// The view's clustered light.
	pass::lights::ClusterLimits limits = pass::lights::DesktopClusterLimits();
	pass::lights::ClusterViewDesc desc;
	desc.view = lab.view;
	desc.projection = lab.projection;
	desc.widthPixels = desc.heightPixels = kSize;
	desc.nearZ = 1.0f;
	desc.farZ = 2000.0f;
	auto grid = pass::lights::CreateClusterGrid( desc, limits );
	if ( !grid )
		return std::string( "the light grid does not build" );
	const light_set::RuntimeLight light = Light();
	auto clusters = LabClusterLists::Create( lab.device, grid.Value(), std::span( &light, 1 ) );
	if ( !clusters )
		return std::string( "the light was not assigned" );
	material::SurfaceViewGpu view;
	view.grid[0] = grid.Value().tilesX;
	view.grid[1] = grid.Value().tilesY;
	view.grid[2] = grid.Value().slices;
	view.grid[3] = limits.tileSizePixels;
	view.slices[0] = grid.Value().sliceScale;
	view.slices[1] = grid.Value().sliceBias;
	view.slices[2] = grid.Value().nearZ;
	const math::float4 &z = lab.view.rows[2];
	view.viewDistance[0] = -z.x;
	view.viewDistance[1] = -z.y;
	view.viewDistance[2] = -z.z;
	view.viewDistance[3] = -z.w;
	std::copy( c.sphere, c.sphere + 4, view.movers[0] );
	const material::SurfaceLightGpu record = material::PackSurfaceLight(
	    light, 0, RuntimeShadowLayout::kSingle, false, c.maskId, c.mover, c.bits );
	material::SurfaceShadows shadows{ lab.atlas, lab.atlasDesc, lab.tiles };
	auto viewRequest = lab.program->ViewGroup( view, {}, {}, std::span( &record, 1 ), shadows );
	clusters->Bind( viewRequest );
	material::SurfaceFrame frame;
	frame.eye[2] = 300.0f;
	const std::uint64_t materialGroup = lab.nextGroup++;
	const std::uint64_t frameGroup = lab.nextGroup++;
	const std::uint64_t drawGroup = lab.nextGroup++;
	const std::uint64_t viewGroup = lab.nextGroup++;
	if ( !lab.groups.Set( materialGroup, request.Value().material ) ||
	     !lab.groups.Set( viewGroup, viewRequest ) ||
	     !lab.groups.Set( frameGroup, lab.program->FrameGroup( frame, "sm/splitsum" ) ) ||
	     !lab.groups.Set( drawGroup,
	         lab.program->DrawGroup( "sm/page", {}, {}, "sm/gradient", {}, "sm/mask" ) ) )
		return std::string( "a group was refused" );
	if ( auto why = lab.canvas->Render( lab.textures, lab.groups, {}, { 0, 0, 0, 1 }, nullptr ) )
		return why;
	auto group = [&]( std::uint64_t id ) -> BindGroupId
	{
		const material::ResidentGroup *resident = lab.groups.Group( id );
		return resident ? resident->group : BindGroupId();
	};
	CanvasDraw draw;
	draw.pipeline = request.Value().pipeline;
	draw.groups[std::size_t( BindGroupRole::kFrame )] = group( frameGroup );
	draw.groups[std::size_t( BindGroupRole::kView )] = group( viewGroup );
	draw.groups[std::size_t( BindGroupRole::kMaterial )] = group( materialGroup );
	draw.groups[std::size_t( BindGroupRole::kDraw )] = group( drawGroup );
	const std::vector<std::byte> quad = WorldQuad();
	draw.vertices = lab.canvas->Vertices( quad );
	draw.vertexCount = 6;
	material::FamilyDrawConstants drawConstants;
	std::memcpy( drawConstants.toClip, &lab.toClip, sizeof( drawConstants.toClip ) );
	drawConstants.world[0] = drawConstants.world[5] = drawConstants.world[10] =
	    drawConstants.world[15] = 1.0f;
	const auto *bytes = reinterpret_cast<const std::byte *>( &drawConstants );
	draw.constants.assign( bytes, bytes + request.Value().drawConstantBytes );
	const CanvasDraw draws[] = { draw };
	auto why = lab.canvas->Render( lab.textures, lab.groups, draws, { 0, 0, 0, 1 }, &image );
	for ( std::uint64_t id : { materialGroup, frameGroup, drawGroup, viewGroup } )
		lab.groups.Remove( id );
	return why;
}

// The world point a pixel centre sees on the quad (z = 0), if it does.
std::optional<float3> Hit( const Lab &lab, std::uint32_t px, std::uint32_t py )
{
	const float nx = ( float( px ) + 0.5f ) / float( kSize ) * 2.0f - 1.0f;
	const float ny = 1.0f - ( float( py ) + 0.5f ) / float( kSize ) * 2.0f;
	const math::float4 a = math::Transform( lab.fromClip, { nx, ny, 0.0f, 1.0f } );
	const math::float4 b = math::Transform( lab.fromClip, { nx, ny, 1.0f, 1.0f } );
	const float3 p0{ a.x / a.w, a.y / a.w, a.z / a.w };
	const float3 p1{ b.x / b.w, b.y / b.w, b.z / b.w };
	if ( ( p0.z > 0.0f ) == ( p1.z > 0.0f ) )
		return std::nullopt;
	const float t = p0.z / ( p0.z - p1.z );
	const float3 p{ p0.x + ( p1.x - p0.x ) * t, p0.y + ( p1.y - p0.y ) * t, 0.0f };
	if ( std::fabs( p.x ) > kHalf * 0.95f || std::fabs( p.y ) > kHalf * 0.95f )
		return std::nullopt;
	return p;
}

// The distance from `centre` to the segment from `p` to the light.
float PathDistance( float3 p, const float centre[3] )
{
	const float3 toLight{ -p.x, -p.y, kLightHeight - p.z };
	const float3 toMover{ centre[0] - p.x, centre[1] - p.y, centre[2] - p.z };
	const float lengthSquared =
	    toLight.x * toLight.x + toLight.y * toLight.y + toLight.z * toLight.z;
	const float along = std::clamp(
	    ( toMover.x * toLight.x + toMover.y * toLight.y + toMover.z * toLight.z ) / lengthSquared,
	    0.0f, 1.0f );
	const float3 off{ toMover.x - toLight.x * along, toMover.y - toLight.y * along,
	    toMover.z - toLight.z * along };
	return std::sqrt( off.x * off.x + off.y * off.y + off.z * off.z );
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		Lab lab( *device );
		if ( auto why = Prepare( lab, module ) )
			return why;
		Case maskOnly;
		Case noMask;
		noMask.maskId = -1;
		Case everywhere;
		everywhere.mover = true;
		Case far = everywhere;
		far.bits = 1u;
		far.sphere[0] = far.sphere[1] = 600.0f;
		far.sphere[2] = 400.0f;
		far.sphere[3] = 20.0f;
		Case near = far;
		near.sphere[0] = 0.0f;
		near.sphere[1] = 0.0f;
		near.sphere[2] = 75.0f;
		near.sphere[3] = 18.0f;
		CanvasImage maskImage, noMaskImage, everywhereImage, farImage, nearImage;
		for ( auto [c, image] : { std::pair{ &maskOnly, &maskImage },
		          std::pair{ &noMask, &noMaskImage }, std::pair{ &everywhere, &everywhereImage },
		          std::pair{ &far, &farImage }, std::pair{ &near, &nearImage } } )
			if ( auto why = Render( lab, *c, *image ) )
				return why;
		std::uint32_t covered = 0, maskLit = 0, noMaskDark = 0, everywhereDark = 0, farSame = 0,
		              nearRight = 0, nearDark = 0, nearLit = 0, nearJudged = 0;
		for ( std::uint32_t py = 0; py < kSize; ++py )
			for ( std::uint32_t px = 0; px < kSize; ++px )
			{
				const auto hit = Hit( lab, px, py );
				if ( !hit )
					continue;
				++covered;
				const float lit = maskImage.At( px, py )[1];
				const auto dark = [&]( const CanvasImage &image )
				{
					return image.At( px, py )[1] <= 0.02f * lit;
				};
				maskLit += lit > 1e-3f;
				noMaskDark += dark( noMaskImage );
				everywhereDark += dark( everywhereImage );
				farSame += std::fabs( farImage.At( px, py )[1] - lit ) <= 1e-3f * lit + 1e-6f;
				// The sphere grown by the source radius, a band left out at
				// its boundary (the shader's rounding).
				const float reach = near.sphere[3] + kSourceRadius;
				const float distance = PathDistance( *hit, near.sphere );
				if ( std::fabs( distance - reach ) < 1.0f )
					continue;
				++nearJudged;
				const bool shadowed = distance < reach;
				const bool got = dark( nearImage );
				const bool same =
				    std::fabs( nearImage.At( px, py )[1] - lit ) <= 1e-3f * lit + 1e-6f;
				nearRight += shadowed ? got : same;
				nearDark += shadowed;
				nearLit += !shadowed;
			}
		const auto all = [&]( std::uint32_t n )
		{
			return covered > 0 && n == covered;
		};
		const std::string of = " of " + std::to_string( covered );
		results.That( covered > kSize * kSize / 4, "shadow-mask.fixture-covers-the-view",
		    std::to_string( covered ) + " pixels" );
		results.That(
		    all( maskLit ), "shadow-mask.mask-alone-lit", std::to_string( maskLit ) + of );
		results.That(
		    all( noMaskDark ), "shadow-mask.tile-shadows", std::to_string( noMaskDark ) + of );
		results.That( all( everywhereDark ), "shadow-mask.mover-everywhere-reads-the-tile",
		    std::to_string( everywhereDark ) + of );
		results.That( all( farSame ), "shadow-mask.mover-far-is-the-mask-alone",
		    std::to_string( farSame ) + of );
		results.That( nearDark > 8 && nearLit > 8 && nearRight == nearJudged,
		    "shadow-mask.mover-near-shadows-only-its-paths",
		    std::to_string( nearRight ) + " of " + std::to_string( nearJudged ) + " (" +
		        std::to_string( nearDark ) + " on a path, " + std::to_string( nearLit ) +
		        " clear)" );
	}
	(void)device->WaitIdle();
	messages = counter.load();
	results.That( messages == 0, "shadow-mask.validation-silent" );
	return std::nullopt;
}

const Seeded kShadowMaskSeeded[] = {
    { "movers-ignored", spirv::kSurfaceMoversIgnored, "shadow-mask.mover-far" } };

} // namespace

int RunShadowMaskSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "shadow-mask", kShadowMaskSeeded, RunChecks );
}

} // namespace render::lab
