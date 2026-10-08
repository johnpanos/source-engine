//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite blob-shadow (RFC 0016 K8: the render-to-texture
//			blob shadows RFC 0016 keeps for legacy renderables): ShadowBuild
//			and Shadow on the decal-modulate point, judged against the legacy
//			shaders restated here:
//			- ShadowBuild (shadowbuildtexture_ps2x) adds white with the base
//			  alpha times the vertex and $alpha coverage (ONE, ONE): two
//			  overlapping casters of 0.3 leave 0.6 in alpha; without a base
//			  texture the coverage is 1;
//			- Shadow (shadow_vs20, shadow_ps2x) averages five bilinear samples
//			  of the page (the base and one texel along each diagonal), less
//			  the vertex alpha, and multiplies white lerped toward $color
//			  (gamma to linear) into the frame: inside a covered region, at an
//			  edge where the jittered samples straddle it, and faded by the
//			  vertex alpha;
//			- Shadow without its page is refused by name.
//
//			Seeded (--sensitivity): one sample instead of five must fail
//			blob-shadow.edge-is-the-five-sample-mean.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/world/world_pass.h"
#include "spv/particles_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using namespace render::pass::world;

constexpr std::uint32_t kSize = 32;
constexpr int kPageHandle = 1;
constexpr int kCasterHandle = 2;
// The page: 8 x 8, alpha 255 in columns 0..3, 0 in 4..7.
constexpr std::uint32_t kPage = 8;
constexpr float kBackground[] = { .8f, .6f, .4f, 1.0f };
// $color as the client's ColorModulate sets it (gamma space).
constexpr float kShadowColor[] = { .5f, .25f, .75f };

class ShadowTextures final : public IWorldTextures
{
public:
	TextureId page, caster;
	TextureId Import( int handle, bool ) override
	{
		return handle == kPageHandle ? page : handle == kCasterHandle ? caster : TextureId();
	}
	SamplerDesc Sampler( int ) override
	{
		SamplerDesc desc;
		desc.address = AddressMode::kClampToEdge;
		return desc;
	}
};

double PageAlpha( int x )
{
	return std::clamp( x, 0, int( kPage ) - 1 ) < 4 ? 1.0 : 0.0;
}

// A bilinear sample of the page's alpha at u (the page is uniform in v).
double SamplePage( double u )
{
	const double x = u * kPage - 0.5;
	const int x0 = int( std::floor( x ) );
	const double f = x - x0;
	return PageAlpha( x0 ) * ( 1.0 - f ) + PageAlpha( x0 + 1 ) * f;
}

// shadow_ps2x at the canvas pixel x: five samples, less the vertex alpha.
// The world pass keeps D3D9's pixel centers (its views' half-pixel shift),
// so the pixel x reads u = x / size, as the legacy frame did.
double Coverage( std::uint32_t x, double vertexAlpha )
{
	const double u = double( x ) / kSize, j = 1.0 / kPage;
	const double mean =
	    ( SamplePage( u ) + 2.0 * SamplePage( u + j ) + 2.0 * SamplePage( u - j ) ) /
	    5.0; // the diagonals' u offsets are +j and -j twice
	return std::clamp( mean - vertexAlpha, 0.0, 1.0 );
}

double Expected( std::uint32_t x, int c, double vertexAlpha )
{
	const double tint = std::pow( kShadowColor[c], 2.2 );
	return kBackground[c] * ( 1.0 + Coverage( x, vertexAlpha ) * ( tint - 1.0 ) );
}

// A full-canvas quad in clip space, uv 0..1 left to right, top to bottom.
WorldView::DynamicDraw Quad(
    WorldMaterial material, std::uint8_t alpha, float x0 = -1, float x1 = 1 )
{
	WorldView::DynamicDraw draw;
	draw.material = std::move( material );
	const float corners[4][2] = { { x0, -1 }, { x1, -1 }, { x1, 1 }, { x0, 1 } };
	for ( const auto &corner : corners )
	{
		WorldVertex v{};
		v.position[0] = corner[0];
		v.position[1] = corner[1];
		v.position[2] = 0.5f;
		v.uv[0] = ( corner[0] + 1.0f ) * 0.5f;
		v.uv[1] = ( 1.0f - corner[1] ) * 0.5f;
		v.color[3] = alpha;
		draw.vertices.push_back( v );
	}
	draw.indices = { 0, 1, 2, 0, 2, 3 };
	return draw;
}

WorldView View()
{
	WorldView view;
	view.toClip[0] = view.toClip[5] = view.toClip[10] = view.toClip[15] = 1;
	view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
	view.drawsWorldGeometry = false;
	return view;
}

WorldMaterial ShadowMaterial( bool page = true )
{
	WorldMaterial material;
	material.name = "decals/rendershadow";
	material.shader = "Shadow";
	char color[64];
	std::snprintf(
	    color, sizeof( color ), "[%g %g %g]", kShadowColor[0], kShadowColor[1], kShadowColor[2] );
	material.variables = { { "$color", color }, { "$decal", "1" } };
	if ( page )
	{
		material.variables.emplace_back( "$basetexture", "_rt_Shadows" );
		material.textures.emplace_back( "$basetexture", kPageHandle );
	}
	return material;
}

WorldMaterial BuildMaterial( bool caster )
{
	WorldMaterial material;
	material.name = "engine/shadowbuild";
	material.shader = "ShadowBuild";
	material.variables = { { "$nocull", "1" }, { "$model", "1" } };
	if ( caster )
	{
		material.variables.emplace_back( "$basetexture", "lab/caster" );
		material.textures.emplace_back( "$basetexture", kCasterHandle );
	}
	return material;
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	ShadowTextures imports;
	{
		TextureDesc desc;
		desc.format = Format::kRGBA8Unorm;
		desc.width = desc.height = kPage;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		std::vector<std::byte> texels( kPage * kPage * 4, std::byte( 0 ) );
		for ( std::uint32_t y = 0; y < kPage; ++y )
			for ( std::uint32_t x = 0; x < kPage; ++x )
				texels[( y * kPage + x ) * 4 + 3] = std::byte( x < 4 ? 255 : 0 );
		auto staged = textures.Stage( "blob-shadow/page", desc, texels );
		if ( !staged )
			return std::string( "the shadow page did not stage" );
		imports.page = staged.Value().texture;
		desc.width = desc.height = 1;
		desc.format = Format::kRGBA8Srgb;
		const std::byte caster[4] = {
		    std::byte( 0 ), std::byte( 0 ), std::byte( 0 ), std::byte( 153 ) }; // alpha 0.6
		auto casterStaged = textures.Stage( "blob-shadow/caster", desc, caster );
		if ( !casterStaged )
			return std::string( "the caster texture did not stage" );
		imports.caster = casterStaged.Value().texture;
	}
	WorldPass pass;
	pass.SetSurfaceFragmentModule( module );
	pass.SetWorld( WorldData() );
	unsigned frame = 0;
	auto render = [&]( WorldView view, const float *clear,
	                  CanvasImage &image ) -> std::optional<std::string>
	{
		const auto tag = pass.QueueView( std::move( view ) );
		if ( !tag )
			return pass.Stats().lastRefusal;
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.depth = depth;
			target.colorFormat = kCanvasColor;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.textures = &imports;
			target.frame = ++frame;
			pass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render(
		    textures, groups, {}, { clear[0], clear[1], clear[2], clear[3] }, &image, post );
	};

	// ShadowBuild: two casters of 0.6 x 0.5 vertex alpha add 0.6 to the
	// cleared page's alpha; without a base the coverage is the vertex alpha.
	{
		const float cleared[] = { 0, 0, 0, 0 };
		WorldView view = View();
		view.dynamicDraws.push_back( Quad( BuildMaterial( true ), 128 ) );
		view.dynamicDraws.push_back( Quad( BuildMaterial( true ), 128 ) );
		view.dynamicDraws.push_back( Quad( BuildMaterial( false ), 51, 0.0f, 1.0f ) );
		CanvasImage image;
		if ( auto why = render( std::move( view ), cleared, image ) )
			return "blob-shadow.build: " + *why;
		const double once = 0.6 * ( 128.0 / 255.0 );
		const float *left = image.At( 4, 16 );
		const float *right = image.At( 28, 16 );
		results.That( std::abs( left[3] - 2.0 * once ) < .01 && left[0] > 0.99f,
		    "blob-shadow.build-adds-the-casters-coverage",
		    std::to_string( left[3] ) + " / " + std::to_string( 2.0 * once ) );
		results.That( std::abs( right[3] - ( 2.0 * once + 51.0 / 255.0 ) ) < .01,
		    "blob-shadow.build-without-a-base-adds-the-vertex-alpha", std::to_string( right[3] ) );
	}

	// Shadow: the page over the background, at full strength and faded.
	for ( const auto [alpha, suffix] : { std::pair{ 0, "" }, std::pair{ 64, ".faded" } } )
	{
		WorldView view = View();
		view.dynamicDraws.push_back( Quad( ShadowMaterial(), std::uint8_t( alpha ) ) );
		CanvasImage image;
		if ( auto why = render( std::move( view ), kBackground, image ) )
			return "blob-shadow.decal: " + *why;
		const double vertexAlpha = alpha / 255.0;
		auto judge = [&]( std::uint32_t x, const std::string &name )
		{
			const float *pixel = image.At( x, 16 );
			bool close = true;
			std::string detail;
			for ( int c = 0; c < 3; ++c )
			{
				const double expected = Expected( x, c, vertexAlpha );
				close &= std::abs( pixel[c] - expected ) < .004;
				detail += std::to_string( pixel[c] ) + "/" + std::to_string( expected ) + " ";
			}
			results.That( close, name + suffix, detail );
		};
		judge( 4, "blob-shadow.covered-region-takes-the-shadow-color" );
		judge( 13, "blob-shadow.edge-is-the-five-sample-mean" );
		judge( 26, "blob-shadow.uncovered-region-is-unchanged" );
	}
	// The edge case discriminates: one sample would be fully covered.
	results.That( std::abs( Coverage( 13, 0.0 ) - 1.0 ) > .2,
	    "blob-shadow.edge-fixture-discriminates", std::to_string( Coverage( 13, 0.0 ) ) );

	{
		WorldView view = View();
		view.dynamicDraws.push_back( Quad( ShadowMaterial( false ), 0 ) );
		const auto tag = pass.QueueView( std::move( view ) );
		results.That( !tag && pass.Stats().lastRefusal.find( "shadow page" ) != std::string::npos,
		    "blob-shadow.refuse-without-its-page", pass.Stats().lastRefusal );
	}
	results.That(
	    pass.Stats().viewsFailed == 0, "blob-shadow.views-complete", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "blob-shadow.validation-silent" );
	return std::nullopt;
}

const Seeded kSeeded[] = {
    { "one-sample", spirv::kBlobShadowOneSample, "blob-shadow.edge-is-the-five-sample-mean" },
};

} // namespace

int RunBlobShadowSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "blob-shadow", kSeeded, RunChecks );
}

} // namespace render::lab
