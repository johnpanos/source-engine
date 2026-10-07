//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's static vertex light suite (RFC 0016 R91 "baked-colour
//			static props"): a static prop's baked per-vertex lighting (the
//			static-prop color lump, VertexLitGeneric's STATIC_LIGHT) drawn by
//			render.pass.world through the shared PBR mesh point with
//			SurfaceVariant::staticVertexLight, as the game's captured dynamic meshes
//			hand it over (the color mesh in the vertex color).
//
//			The fixture is one white quad under an orthographic view with no
//			lights, its vertices' color the baked light c. The legacy rule is
//			GammaToLinear( c * cOverbright ), cOverbright 2, so the diffuse
//			part of a pixel, L(c) - L(0), is proportional to ( 2c )^2.2:
//			relational, so the BRDF's constant factors cancel. The term must
//			change the image (a draw without it ignores the baked light), and
//			the oracle rejects a linear decode of the same values.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/world/world_pass.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
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
constexpr int kBaseHandle = 1;
constexpr std::uint32_t kX = 16, kY = 16;
// The baked light bytes of the fixture's vertices.
constexpr unsigned kLevels[] = { 0, 24, 64, 128 };

class FixtureTextures final : public IWorldTextures
{
public:
	TextureId base;
	TextureId Import( int handle, bool ) override
	{
		return handle == kBaseHandle ? base : TextureId{};
	}
	SamplerDesc Sampler( int ) override { return {}; }
};

WorldMaterial Material()
{
	WorldMaterial material;
	material.name = "static-light-fixture";
	material.shader = "VertexLitGeneric";
	material.mesh = true;
	material.variables = { { "$basetexture", "static_light/white" } };
	material.textures = { { "$basetexture", kBaseHandle } };
	return material;
}

WorldData World()
{
	WorldData world;
	auto stage = std::make_shared<WorldStage>();
	stage->lightmap.width = stage->lightmap.height = 1;
	stage->lightmap.flat.resize( 8 );
	world.stage = std::move( stage );
	world.materials.push_back( Material() );
	return world;
}

// The legacy decode, and the defect the oracle must reject.
double Decode( unsigned level, bool linear )
{
	const double value = 2.0 * level / 255.0;
	return linear ? value : std::pow( value, 2.2 );
}

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( std::optional<std::string> why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	FixtureTextures fixture;
	TextureDesc baseDesc;
	baseDesc.format = Format::kRGBA8Srgb;
	baseDesc.width = baseDesc.height = 1;
	baseDesc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	const std::array<std::byte, 4> white = {
	    std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 }, std::byte{ 255 } };
	auto base = textures.Stage( "static_light/white", baseDesc, white );
	if ( !base )
		return "the white base fixture could not be staged";
	fixture.base = base.Value().texture;

	std::vector<std::unique_ptr<WorldPass>> passes;
	std::uint64_t frame = 0;
	// The pixel at the quad's centre, its vertices' color `level` (gray),
	// with or without the static vertex light term.
	auto render = [&]( unsigned level, bool staticLight,
	                  std::array<float, 3> &pixel ) -> std::optional<std::string>
	{
		auto pass = std::make_unique<WorldPass>();
		pass->SetWorld( World() );
		WorldView view;
		for ( int i = 0; i < 4; ++i )
			view.toClip[i * 5] = 1.0f;
		view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
		view.hostFrame = ++frame;
		WorldView::DynamicDraw draw;
		draw.material = Material();
		draw.staticVertexLight = staticLight;
		for ( const auto &xy : { std::pair{ -0.5f, -0.5f }, std::pair{ 0.5f, -0.5f },
		          std::pair{ 0.5f, 0.5f }, std::pair{ -0.5f, 0.5f } } )
		{
			WorldVertex out;
			out.position[0] = xy.first;
			out.position[1] = xy.second;
			out.position[2] = 0.5f;
			out.uv[0] = xy.first + 0.5f;
			out.uv[1] = 0.5f - xy.second;
			for ( int c = 0; c < 3; ++c )
				out.color[c] = std::uint8_t( level );
			out.color[3] = 255;
			draw.vertices.push_back( out );
		}
		draw.indices = { 0, 1, 2, 0, 2, 3 };
		view.dynamicDraws.push_back( std::move( draw ) );
		const std::uint32_t tag = pass->QueueView( std::move( view ) );
		if ( !tag )
			return "the fixture view queued nothing: " + pass->Stats().lastRefusal;
		const std::uint64_t thisFrame = frame;
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.depth = depth;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.textures = &fixture;
			target.frame = thisFrame;
			target.eye[2] = 1.0e4f;
			pass->Record( tag, encoder, target );
			return std::nullopt;
		};
		CanvasImage image;
		const ClearColor black{ 0, 0, 0, 1 };
		if ( std::optional<std::string> why =
		         canvas->Render( textures, groups, {}, black, &image, post ) )
			return why;
		const WorldStats stats = pass->Stats();
		if ( stats.viewsFailed != 0 )
			return "the fixture view failed: " + stats.lastFailure;
		if ( stats.dynamicDrawsDrawn != 1 )
			return std::string( "the fixture quad was not drawn by the core" );
		const float *at = image.At( kX, kY );
		pixel = { at[0], at[1], at[2] };
		passes.push_back( std::move( pass ) );
		return std::nullopt;
	};

	std::array<std::array<float, 3>, std::size( kLevels )> lit{}, plain{};
	for ( std::size_t i = 0; i < std::size( kLevels ); ++i )
	{
		if ( auto why = render( kLevels[i], true, lit[i] ) )
			return why;
		if ( auto why = render( kLevels[i], false, plain[i] ) )
			return why;
	}
	// The diffuse part relative to the brightest level, against an oracle.
	const auto matches = [&]( bool linear )
	{
		const std::size_t top = std::size( kLevels ) - 1;
		for ( std::size_t i = 1; i < top; ++i )
			for ( int c = 0; c < 3; ++c )
			{
				const double measured =
				    ( lit[i][c] - lit[0][c] ) / std::max( 1e-6, double( lit[top][c] - lit[0][c] ) );
				const double expected =
				    Decode( kLevels[i], linear ) / Decode( kLevels[top], linear );
				if ( !std::isfinite( measured ) ||
				     std::abs( measured - expected ) > 0.02 * expected + 2e-3 )
					return false;
			}
		return true;
	};
	results.That( lit.back()[0] - lit[0][0] > 0.05f, "static-light.baked-light-lights",
	    "the brightest level adds " + std::to_string( lit.back()[0] - lit[0][0] ) );
	results.That( matches( false ), "static-light.gamma-to-linear-of-twice-the-color" );
	results.That( !matches( true ), "static-light.oracle-rejects-a-linear-decode" );
	bool ignored = true;
	for ( std::size_t i = 1; i < std::size( kLevels ); ++i )
		ignored = ignored && std::abs( plain[i][0] - plain[0][0] ) < 1e-4f;
	results.That( ignored, "static-light.without-the-term-the-color-is-not-light" );
	results.That( std::abs( lit.back()[0] - plain.back()[0] ) > 0.05f,
	    "static-light.the-term-changes-the-image" );

	(void)device->WaitIdle();
	for ( auto &pass : passes )
		pass->ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "static-light.validation-silent" );
	return std::nullopt;
}

} // namespace

int RunStaticLightSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "static-light", {}, RunChecks );
}

} // namespace render::lab
