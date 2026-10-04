//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Source soft-particle depth fade, through the ordered core draw
//          packet, against the independently evaluated depth-copy contract.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"
#include "render/pass/world/world_pass.h"
#include "spv/softparticle_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>

namespace render::lab
{
namespace
{
using namespace device;
using namespace pass::world;
constexpr unsigned kSize = 64;
class DepthTextures final : public IWorldTextures
{
public:
	TextureId depth;
	TextureId Import( int handle, bool srgb ) override
	{
		return handle == 1 && !srgb ? depth : TextureId();
	}
	SamplerDesc Sampler( int ) override { return {}; }
};

WorldView View( bool enabled, float scale = 50.0f, float range = 192.0f )
{
	WorldView view;
	// Homogeneous clip Z = 60, w = 128, independently of screen x/y.
	view.toClip[0] = view.toClip[5] = view.toClip[15] = 128;
	view.toClip[10] = 1;
	view.viewport = { 0, 0, kSize, kSize, 0, 1 };
	view.depthAlphaHandle = 1;
	view.depthAlphaRange = range;
	WorldView::DynamicDraw draw;
	draw.material.name = "particle/particle_noisesphere";
	draw.material.shader = "UnlitGeneric";
	draw.material.variables = { { "$translucent", "1" }, { "$vertexcolor", "1" },
	    { "$vertexalpha", "1" }, { "$depthblend", enabled ? "1" : "0" },
	    { "$depthblendscale", std::to_string( scale ) }, { "$alpha", "0.8" } };
	for ( auto xy : { std::pair{ -1.0f, -1.0f }, std::pair{ 1.0f, -1.0f }, std::pair{ 1.0f, 1.0f },
	          std::pair{ -1.0f, 1.0f } } )
	{
		WorldVertex vertex{};
		vertex.position[0] = xy.first;
		vertex.position[1] = xy.second;
		vertex.position[2] = 60;
		vertex.normal[2] = vertex.tangentS[0] = 1;
		vertex.color[0] = vertex.color[1] = vertex.color[2] = 255;
		vertex.color[3] = 128;
		draw.vertices.push_back( vertex );
	}
	draw.indices = { 0, 1, 2, 0, 2, 3 };
	view.dynamicDraws.push_back( std::move( draw ) );
	return view;
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	WorldPass pass;
	pass.SetSurfaceFragmentModule( module );
	pass.SetWorld( WorldData() );
	for ( float scale : { 0.0f, -1.0f, std::numeric_limits<float>::infinity() } )
	{
		results.That( pass.QueueView( View( true, scale ) ) == 0,
		    "softparticle.claim.reject-invalid-scale-" + std::to_string( scale ) );
	}
	for ( float range : { 0.0f, -1.0f, std::numeric_limits<float>::quiet_NaN() } )
	{
		results.That( pass.QueueView( View( true, 50, range ) ) == 0,
		    "softparticle.claim.reject-invalid-range-" + std::to_string( range ) );
	}
	WorldView missing = View( true );
	missing.depthAlphaHandle = 0;
	results.That( pass.QueueView( std::move( missing ) ) == 0,
	    "softparticle.claim.reject-missing-depth-by-name", pass.Stats().lastRefusal );

	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	TextureDesc desc;
	desc.width = 32;
	desc.height = 48; // the game resamples its depth copy independently of the viewport
	desc.format = Format::kRGBA8Unorm;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	std::vector<std::byte> texels( desc.width * desc.height * 4 );
	// Four physically different gaps, including the encoded range's far fade.
	const unsigned sceneBytes[] = { 80, 106, 160, 244 };
	for ( unsigned y = 0; y < desc.height; ++y )
		for ( unsigned x = 0; x < desc.width; ++x )
			texels[( y * desc.width + x ) * 4 + 3] =
			    std::byte( sceneBytes[( y >= desc.height / 2 ) * 2 + ( x >= desc.width / 2 )] );
	auto staged = textures.Stage( "softparticle/depth-alpha", desc, texels );
	if ( !staged )
		return "could not stage depth-alpha fixture";
	DepthTextures imports;
	imports.depth = staged.Value().texture;
	unsigned frame = 0;
	for ( unsigned viewportSize : { kSize, kSize / 2 } )
		for ( bool enabled : { false, true } )
			for ( float scale : { 25.0f, 50.0f, 100.0f } )
			{
				WorldView view = View( enabled, scale );
				view.viewport.width = view.viewport.height = viewportSize;
				const auto tag = pass.QueueView( std::move( view ) );
				if ( !tag )
					return pass.Stats().lastRefusal;
				CanvasImage image;
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
					target.textures = &imports;
					target.frame = ++frame;
					pass.Record( tag, encoder, target );
					return std::nullopt;
				};
				if ( auto why = canvas->Render( textures, groups, {}, {}, &image, post ) )
					return why;
				for ( unsigned quadrant = 0; quadrant < 4; ++quadrant )
				{
					const unsigned sourceQuadrant = viewportSize == kSize ? quadrant : 0;
					const double scene = sceneBytes[sourceQuadrant] / 255.0;
					const double t = std::clamp( ( scene - 0.75 ) * 4.0, 0.0, 1.0 );
					const double farFade = t * t * ( 3.0 - 2.0 * t );
					const double gap = std::abs( scene * 192.0 - 60.0 );
					const double feather =
					    enabled ? std::clamp( std::max( gap / scale, farFade ), 0.0, 1.0 ) : 1.0;
					const double expected = 0.8 * ( 128.0 / 255.0 ) * feather;
					const float *pixel =
					    image.At( quadrant % 2 * viewportSize / 2 + viewportSize / 4,
					        quadrant / 2 * viewportSize / 2 + viewportSize / 4 );
					bool close = true;
					for ( unsigned channel = 0; channel < 3; ++channel )
						close &= std::isfinite( pixel[channel] ) &&
						         std::abs( pixel[channel] - expected ) < 0.002;
					results.That( close,
					    std::string( "softparticle.fade." ) +
					        ( enabled ? "enabled-" : "neutral-" ) + std::to_string( scale ) +
					        "-viewport-" + std::to_string( viewportSize ) + "-quadrant-" +
					        std::to_string( quadrant ),
					    "actual " + std::to_string( pixel[0] ) + " expected " +
					        std::to_string( expected ) );
				}
			}
	results.That( pass.Stats().viewsFailed == 0 && pass.Stats().dynamicDrawsDrawn == 12,
	    "softparticle.all-claimed-draws-recorded", pass.Stats().lastFailure );
	// A valid handle that loses its import is a failed claim, not neutral white.
	imports.depth = {};
	const auto lost = pass.QueueView( View( true ) );
	CanvasPost missingPost = [&]( CommandEncoder &encoder, TextureId color,
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
		pass.Record( lost, encoder, target );
		return std::nullopt;
	};
	if ( auto why = canvas->Render( textures, groups, {}, {}, nullptr, missingPost ) )
		return why;
	results.That( pass.Stats().viewsFailed == 1 &&
	                  pass.Stats().lastFailure.find( "$depthblend" ) != std::string::npos,
	    "softparticle.lost-depth-import-fails-by-name", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "softparticle.validation-silent" );
	return std::nullopt;
}
const Seeded kSeeded[] = { { "fade-ignored", spirv::kSoftParticleFadeIgnored, "softparticle.fade" },
    { "range-ignored", spirv::kSoftParticleRangeIgnored, "softparticle.fade" },
    { "texture-extent", spirv::kSoftParticleTextureExtent, "softparticle.fade" },
    { "viewport-extent", spirv::kSoftParticleViewportExtent, "softparticle.fade" } };
}
int RunSoftParticleSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "softparticle", kSeeded, RunChecks );
}
}
