//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal stencil recursion, clip planes and viewmodel depth range
//          through the core world pass, independent of the engine.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"
#include "render/pass/world/world_pass.h"

#include <atomic>

namespace render::lab
{
namespace
{
using namespace device;
using namespace pass::world;
constexpr unsigned kSize = 64;
class NoTextures final : public IWorldTextures
{
public:
	TextureId Import( int, bool ) override { return {}; }
	SamplerDesc Sampler( int ) override { return {}; }
};

WorldData Fixture()
{
	WorldData world;
	for ( const char *color : { "[1 0 0]", "[0 1 0]", "[0 0 1]", "[0 0 0]" } )
	{
		WorldMaterial material;
		material.name = color;
		material.shader = world.materials.size() == 3 ? "WriteZ" : "UnlitGeneric";
		if ( world.materials.size() != 3 )
			material.variables.emplace_back( "$color", color );
		world.materials.push_back( std::move( material ) );
	}
	for ( const char *open : { "1", "0.5" } )
	{
		WorldMaterial material;
		material.name = std::string( "portal-aperture-" ) + open;
		material.shader = "PortalRefract";
		// PortalRefract's InitParams sets both flags even for its stencil stage.
		material.variables = { { "$stage", "1" }, { "$portalopenamount", open }, { "$model", "1" },
		    { "$translucent", "1" } };
		world.materials.push_back( std::move( material ) );
	}
	auto quad = [&]( unsigned mat, float left, float right, float z )
	{
		const unsigned base = world.vertices.size(), first = world.indices.size();
		for ( auto xy : { std::pair{ left, -1.0f }, std::pair{ right, -1.0f },
		          std::pair{ right, 1.0f }, std::pair{ left, 1.0f } } )
		{
			WorldVertex v{};
			v.position[0] = xy.first;
			v.position[1] = xy.second;
			v.position[2] = z;
			v.uv[0] = (xy.first-left)/(right-left); v.uv[1] = (xy.second+1)*0.5f;
			v.normal[2] = v.tangentS[0] = 1.0f;
			for ( auto &c : v.color )
				c = 255;
			world.vertices.push_back( v );
		}
		world.indices.insert(
		    world.indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 } );
		world.surfaces.push_back( { mat, 0, first, 6 } );
	};
	quad( 0, -1, 1, 0.3f );    // parent room
	quad( 3, -1, 0, 0.2f );    // visible opening
	quad( 3, -1, 1, 1.0f );    // depth reset, stencil constrained
	quad( 1, -1, 1, 0.5f );    // child room behind parent wall
	quad( 3, -0.5f, 1, 0.1f ); // nested opening intersects first opening
	quad( 2, -1, 1, 0.8f );    // grandchild room / foreground weapon
	quad( 4, -1, 0, 0.2f );
	quad( 5, -1, 0, 0.2f );
	return world;
}
WorldView View( unsigned surface )
{
	WorldView view;
	view.toClip[0] = view.toClip[5] = view.toClip[10] = view.toClip[15] = 1;
	view.viewport = { 0, 0, kSize, kSize, 0, 1 };
	view.surfaces = { surface };
	return view;
}
bool Color( const CanvasImage &image, unsigned x, unsigned y, unsigned channel )
{
	const float *p = image.At( x, y );
	return p[channel] > 0.95f && p[( channel + 1 ) % 3] < 0.01f && p[( channel + 2 ) % 3] < 0.01f;
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
	TextureDesc depthDesc;
	depthDesc.width = depthDesc.height = kSize;
	depthDesc.format = Format::kD32FloatS8;
	depthDesc.usages = { ResourceUsage::kDepthWrite };
	auto depth = device->CreateTexture( depthDesc );
	if ( !depth )
		return "stencil attachment unavailable";
	bool initialized = false;
	NoTextures imports;
	WorldPass pass;
	const WorldData fixture = Fixture();
	pass.SetWorld( fixture );
	std::vector<unsigned> tags;
	for ( unsigned i = 0; i < 8; ++i )
	{
		WorldView view = View( i );
		if ( i >= 6 )
		{
			// Exercise the same dynamic snapshot acceptance as the game frontend.
			WorldView::DynamicDraw draw;
			draw.material = fixture.materials[fixture.surfaces[i].material];
			draw.vertices.assign(
			    fixture.vertices.begin() + i * 4, fixture.vertices.begin() + ( i + 1 ) * 4 );
			draw.indices = { 0, 1, 2, 0, 2, 3 };
			view.surfaces.clear();
			view.dynamicDraws.push_back( std::move( draw ) );
		}
		tags.push_back( pass.QueueView( std::move( view ) ) );
	}
	results.That( tags[6] != 0 && tags[7] != 0, "view-state.game-portal-material-flags-accepted" );
	for ( const char *stage : { "0", "2" } )
	{
		WorldView view = View( 6 );
		WorldView::DynamicDraw draw;
		draw.material = fixture.materials[4];
		draw.material.variables.front().second = stage;
		draw.vertices.assign( fixture.vertices.begin() + 24, fixture.vertices.begin() + 28 );
		draw.indices = { 0, 1, 2, 0, 2, 3 };
		view.surfaces.clear();
		view.dynamicDraws.push_back( std::move( draw ) );
		results.That( pass.QueueView( std::move( view ) ) == 0,
		    std::string( "view-state.aperture-does-not-claim-stage-" ) + stage );
	}
	std::uint64_t frame = 0;
	auto render = [&]( int control, CanvasImage &image )
	{
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId ) -> std::optional<std::string>
		{
			if ( !initialized )
			{
				encoder.TransitionTexture(
				    depth.Value(), ResourceUsage::kUndefined, ResourceUsage::kDepthWrite );
				initialized = true;
			}
			RenderingDesc clear;
			clear.width = clear.height = kSize;
			clear.depth = DepthAttachment{ depth.Value(), LoadOp::kClear, StoreOp::kStore, 1 };
			encoder.BeginRendering( clear );
			encoder.EndRendering();
			WorldTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.depth = depth.Value();
			target.depthFormat = depthDesc.format;
			target.width = target.height = kSize;
			target.textures = &imports;
			target.frame = ++frame;
			pass.Record( tags[0], encoder, target );
			if ( control == 3 || control == 4 )
			{
				target.overrideDepthRange = control == 3;
				target.maxDepth = 0.1f;
				pass.Record( tags[5], encoder, target );
				return std::nullopt;
			}
			auto &stencil = target.drawState.stencil;
			stencil.enabled = true;
			stencil.pass = StencilOp::kReplace;
			stencil.reference = 1;
			pass.Record( tags[control == 6 ? 6 : control == 7 ? 7 : 1], encoder, target );
			stencil.compare = CompareOp::kEqual;
			stencil.pass = StencilOp::kKeep;
			stencil.enabled = control != 1;
			target.drawState.overrideDepth = true;
			target.drawState.depthCompare = CompareOp::kAlways;
			pass.Record( tags[2], encoder, target );
			target.drawState.overrideDepth = false;
			stencil.enabled = control != 1;
			if ( control != 2 && control != 5 && control != 6 && control != 7 )
				target.clipPlanes[0][1] = 1;
			pass.Record( tags[3], encoder, target );
			if ( control == 5 )
			{
				stencil.reference = 3;
				stencil.readMask = 1;
				stencil.pass = StencilOp::kReplace;
				pass.Record( tags[4], encoder, target );
				stencil.readMask = 255;
				stencil.pass = StencilOp::kKeep;
				target.drawState.overrideDepth = true;
				target.drawState.depthCompare = CompareOp::kAlways;
				pass.Record( tags[2], encoder, target );
				target.drawState.overrideDepth = false;
				pass.Record( tags[5], encoder, target );
			}
			return std::nullopt;
		};
		return canvas->Render( textures, groups, {}, { 0, 0, 0, 1 }, &image, post );
	};
	CanvasImage normal, replay, noMask, noClip, weapon, noRange, nested, aperture, halfOpen;
	for ( auto sample : { std::pair{ 0, &normal }, { 1, &noMask }, { 2, &noClip }, { 5, &nested },
	          { 3, &weapon }, { 4, &noRange }, { 0, &replay }, { 6, &aperture }, { 7, &halfOpen } } )
		if ( auto why = render( sample.first, *sample.second ) )
			return why;
	results.That( Color( normal, 8, 16, 1 ) && Color( normal, 48, 16, 0 ),
	    "view-state.portal-keeps-parent-outside-opening" );
	results.That( !Color( normal, 8, 48, 1 ) && Color( noClip, 8, 48, 1 ),
	    "view-state.exit-plane-clips-and-negative-control-detected" );
	results.That( Color( noMask, 48, 16, 1 ) && !Color( normal, 48, 16, 1 ),
	    "view-state.missing-stencil-negative-control" );
	results.That(
	    Color( nested, 4, 32, 1 ) && Color( nested, 24, 32, 2 ) && Color( nested, 48, 32, 0 ),
	    "view-state.recursive-mask-intersection" );
	results.That( Color( weapon, 48, 32, 2 ) && Color( noRange, 48, 32, 0 ),
	    "view-state.viewmodel-depth-range-and-negative-control" );
	results.That( Color( replay, 8, 16, 1 ) && Color( replay, 48, 16, 0 ),
	    "view-state.capture-replay-restores-slot-state" );
	results.That( Color(aperture, 16, 32, 1) && Color(aperture, 2, 2, 0) && Color(aperture, 48, 32, 0),
	    "view-state.portal-aperture-is-an-ellipse-not-a-quad" );
	results.That( Color(halfOpen, 16, 32, 1) && Color(halfOpen, 22, 32, 0) && Color(aperture, 22, 32, 1),
	    "view-state.portal-opening-amount-and-negative-control" );
	results.That(
	    pass.Stats().viewsFailed == 0, "view-state.no-lost-slots", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	(void)device->Release( depth.Value(), {} );
	messages = counter.load();
	results.That( messages == 0, "view-state.validation-silent" );
	return std::nullopt;
}
}
int RunViewStateSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "view-state", std::span<const Seeded>(), RunChecks );
}
}
