//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Expanded Source sprite quads through the core, with independent
//          texture/color/alpha, blend, fog and depth expectations.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"
#include "render/pass/world/world_pass.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace render::lab
{
namespace
{
using namespace device;
using namespace pass::world;
constexpr unsigned kSize = 32;
constexpr unsigned kTexel[] = { 128, 192, 64, 160 };
constexpr unsigned kColor[] = { 128, 64, 192, 128 };
constexpr float kBackground[] = { .05f, .08f, .12f, .7f };
class SpriteTextures final : public IWorldTextures
{
public:
	TextureId linear, srgb;
	TextureId Import( int handle, bool decoded ) override
	{
		return handle == 1 ? ( decoded ? srgb : linear ) : TextureId();
	}
	SamplerDesc Sampler( int ) override { return {}; }
};
WorldView View( int mode, bool srgb, bool occluded )
{
	WorldView view;
	view.toClip[0] = view.toClip[5] = view.toClip[10] = view.toClip[15] = 1;
	view.viewport = { 0, 0, kSize, kSize, 0, 1 };
	WorldView::DynamicDraw sprite;
	sprite.material.name = "sprites/glow1";
	sprite.material.shader = "Sprite_DX9";
	sprite.material.variables = { { "$basetexture", "sprite/base" },
	    { "$spriterendermode", std::to_string( mode ) }, { "$nosrgb", srgb ? "0" : "1" },
	    { "$ignorevertexcolors", "0" }, { "$hdrcolorscale", "1" }, { "$spriteorientation", "2" },
	    { "$spriteorigin", "[1 2 3]" } };
	sprite.material.textures.emplace_back( "$basetexture", 1 );
	for ( const auto xy : { std::pair{ -1.f, -1.f }, std::pair{ 1.f, -1.f }, std::pair{ 1.f, 1.f },
	          std::pair{ -1.f, 1.f } } )
	{
		WorldVertex v{};
		v.position[0] = xy.first;
		v.position[1] = xy.second;
		v.position[2] = .7f;
		v.normal[2] = v.tangentS[0] = 1;
		std::copy_n( kColor, 4, v.color );
		sprite.vertices.push_back( v );
	}
	sprite.indices = { 0, 1, 2, 0, 2, 3 };
	if ( occluded )
	{
		auto wall = sprite;
		wall.material = {};
		wall.material.shader = "UnlitGeneric";
		wall.material.name = "sprite/opaque-occluder";
		wall.material.variables = { { "$color", "[0 0 0]" } };
		for ( auto &v : wall.vertices )
			v.position[2] = .1f;
		view.dynamicDraws.push_back( std::move( wall ) );
	}
	view.dynamicDraws.push_back( std::move( sprite ) );
	return view;
}
double Decode( double value )
{
	return value <= .04045 ? value / 12.92 : std::pow( ( value + .055 ) / 1.055, 2.4 );
}
bool Expected( const CanvasImage &image, int mode, bool srgb, bool occluded, bool fog )
{
	const bool glow = mode == 3 || mode == 9;
	const bool additive = glow || mode == 5;
	const bool vertex = mode != 0;
	const double alpha = ( kTexel[3] / 255.0 ) * ( vertex ? kColor[3] / 255.0 : 1.0 );
	for ( unsigned y = 8; y < 24; ++y )
		for ( unsigned x = 8; x < 24; ++x )
		{
			const float *pixel = image.At( x, y );
			for ( unsigned c = 0; c < 3; ++c )
			{
				const double base = srgb ? Decode( kTexel[c] / 255.0 ) : kTexel[c] / 255.0;
				const double color = !vertex ? 1.0
				                     : srgb  ? std::pow( kColor[c] / 255.0, 2.2 )
				                             : kColor[c] / 255.0;
				double source = base * color;
				if ( fog )
					source = source * .75 + ( additive ? 0.0 : .2 ) * .25;
				const double back = occluded ? 0.0 : kBackground[c];
				const double expected = occluded && !glow ? back
				                        : mode == 0       ? source
				                        : additive        ? back + source * alpha
				                                          : back * ( 1.0 - alpha ) + source * alpha;
				if ( !std::isfinite( pixel[c] ) || std::abs( pixel[c] - expected ) > .002 )
					return false;
			}
			if ( !occluded && std::abs( pixel[3] - kBackground[3] ) > .002 )
				return false;
		}
	return true;
}
std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
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
	SpriteTextures imports;
	for ( bool srgb : { false, true } )
	{
		TextureDesc desc;
		desc.width = desc.height = 1;
		desc.format = srgb ? Format::kRGBA8Srgb : Format::kRGBA8Unorm;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		std::byte texel[4];
		for ( unsigned c = 0; c < 4; ++c )
			texel[c] = std::byte( kTexel[c] );
		auto staged = textures.Stage( srgb ? "sprite/srgb" : "sprite/linear", desc, texel );
		if ( !staged )
			return "sprite fixture texture staging failed";
		( srgb ? imports.srgb : imports.linear ) = staged.Value().texture;
	}
	WorldPass pass;
	pass.SetWorld( WorldData() );
	unsigned frame = 0;
	auto render = [&]( WorldView view, bool fog, CanvasImage &image ) -> std::optional<std::string>
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
			if ( fog )
			{
				target.fogType = 0;
				std::fill_n( target.fogColor, 3, .2f );
				target.fogParams[3] = .5f / .7f;
			}
			pass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {},
		    { kBackground[0], kBackground[1], kBackground[2], kBackground[3] }, &image, post );
	};
	for ( int mode : { 0, 1, 2, 3, 4, 5, 9 } )
		for ( bool srgb : { false, true } )
			for ( bool covered : { false, true } )
			{
				CanvasImage image;
				if ( auto why = render( View( mode, srgb, covered ), false, image ) )
					return why;
				results.That( Expected( image, mode, srgb, covered, false ),
				    "sprite.color-alpha-depth." + std::to_string( mode ) +
				        ( srgb ? ".srgb" : ".linear" ) + ( covered ? ".covered" : ".visible" ) );
			}
	for ( int mode : { 2, 9 } )
	{
		CanvasImage image;
		if ( auto why = render( View( mode, false, false ), true, image ) )
			return why;
		results.That(
		    Expected( image, mode, false, false, true ), "sprite.fog." + std::to_string( mode ) );
	}
	for ( int mode : { -1, 6, 7, 8, 10 } )
	{
		const auto tag = pass.QueueView( View( mode, false, false ) );
		const auto &why = pass.Stats().lastRefusal;
		results.That(
		    !tag && why.find( "$spriterendermode " + std::to_string( mode ) ) != std::string::npos,
		    "sprite.refuse-mode." + std::to_string( mode ), why );
	}
	for ( int fault = 0; fault < 3; ++fault )
	{
		auto bad = View( fault == 0 ? 5 : 9, fault == 1, true );
		if ( fault == 2 )
			for ( auto &v : bad.dynamicDraws.back().vertices )
				v.color[3] = 255;
		CanvasImage image;
		if ( auto why = render( std::move( bad ), false, image ) )
			return why;
		results.That( !Expected( image, 9, false, true, false ),
		    "sprite.oracle.reject-wrong-depth-decoding-alpha." + std::to_string( fault ) );
	}
	results.That(
	    pass.Stats().viewsFailed == 0, "sprite.claimed-views-complete", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "sprite.validation-silent" );
	return std::nullopt;
}
} // namespace
int RunSpriteSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "sprite", {}, RunChecks );
}
} // namespace render::lab
