//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite visibility (RFC 0016 K8 sprites cohort;
//			render.pass.visibility over device clause D43): the client's pixel
//			visibility proxies counted on the core, against an occluder the
//			world pass draws into the canvas's depth first. Every edge sits on
//			a pixel boundary after the views' half-pixel shift, so the counts
//			are exact:
//			- an unoccluded proxy: visible and possible are its rectangle's
//			  pixels;
//			- behind a full-screen occluder: none visible, the same possible;
//			- behind an occluder over the left half: only its right part;
//			- in front of the occluder: all visible;
//			- counts are pending until the frame's token is reported and read
//			  once; a recapture draws nothing more; an empty viewport is
//			  refused.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/visibility/visibility.h"
#include "render/pass/world/world_pass.h"

#include <atomic>
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
using pass::visibility::Counts;
using pass::visibility::Query;
using pass::visibility::VisibilityCounter;
using pass::visibility::VisibilityTarget;

constexpr std::uint32_t kSize = 32;

class NoTextures final : public IWorldTextures
{
public:
	TextureId Import( int, bool ) override { return TextureId(); }
	SamplerDesc Sampler( int ) override { return {}; }
};

// Clip x whose shifted edge lands on pixel boundary k; clip y for row k.
float ClipX( float k )
{
	return 2.0f * ( k - 0.5f ) / float( kSize ) - 1.0f;
}
float ClipY( float k )
{
	return 1.0f - 2.0f * ( k - 0.5f ) / float( kSize );
}

// The proxy over pixel columns [x0, x1) and rows [y0, y1) at depth z: the
// apex at its center and the four base corners.
Query Proxy( float x0, float y0, float x1, float y1, float z )
{
	Query query;
	const float cx = ( ClipX( x0 ) + ClipX( x1 ) ) * 0.5f,
	            cy = ( ClipY( y0 ) + ClipY( y1 ) ) * 0.5f;
	const float points[5][2] = { { cx, cy }, { ClipX( x0 ), ClipY( y0 ) },
	    { ClipX( x1 ), ClipY( y0 ) }, { ClipX( x1 ), ClipY( y1 ) }, { ClipX( x0 ), ClipY( y1 ) } };
	for ( int i = 0; i < 5; ++i )
	{
		query.points[i][0] = points[i][0];
		query.points[i][1] = points[i][1];
		query.points[i][2] = z;
		query.points[i][3] = 1.0f;
	}
	query.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
	return query;
}

// An opaque black occluder over clip x in [left, right], at depth z.
WorldView Occluder( float left, float right, float z )
{
	WorldView view;
	view.toClip[0] = view.toClip[5] = view.toClip[10] = view.toClip[15] = 1;
	view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
	view.drawsWorldGeometry = false;
	WorldView::DynamicDraw wall;
	wall.material.shader = "UnlitGeneric";
	wall.material.name = "visibility/occluder";
	wall.material.variables = { { "$color", "[0 0 0]" } };
	for ( const auto &[x, y] : { std::pair{ left, -1.f }, std::pair{ right, -1.f },
	          std::pair{ right, 1.f }, std::pair{ left, 1.f } } )
	{
		WorldVertex v{};
		v.position[0] = x;
		v.position[1] = y;
		v.position[2] = z;
		v.normal[2] = v.tangentS[0] = 1;
		wall.vertices.push_back( v );
	}
	wall.indices = { 0, 1, 2, 0, 2, 3 };
	view.dynamicDraws.push_back( std::move( wall ) );
	return view;
}

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	if ( !device->Facts().capabilities.Has( Capability::kOcclusionQueries ) )
		return std::string( "the lab device counts no occlusion samples (clause D43)" );
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	NoTextures imports;
	WorldPass world;
	world.SetWorld( WorldData() );
	VisibilityCounter visibility;
	std::uint64_t frame = 0;

	// One frame: the occluder (if any), then the proxies, then a later token.
	auto run = [&]( std::optional<WorldView> occluder,
	               const std::vector<std::uint32_t> &tags ) -> std::optional<std::string>
	{
		std::uint32_t worldTag = 0;
		if ( occluder )
		{
			worldTag = world.QueueView( std::move( *occluder ) );
			if ( !worldTag )
				return "the occluder was refused: " + world.Stats().lastRefusal;
		}
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
		                      TextureId depth ) -> std::optional<std::string>
		{
			++frame;
			if ( worldTag )
			{
				WorldTarget target;
				target.device = device.get();
				target.color = color;
				target.depth = depth;
				target.colorFormat = kCanvasColor;
				target.depthFormat = kCanvasDepth;
				target.width = target.height = kSize;
				target.textures = &imports;
				target.frame = frame;
				world.Record( worldTag, encoder, target );
			}
			VisibilityTarget target;
			target.device = device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.depth = depth;
			target.depthFormat = kCanvasDepth;
			target.width = target.height = kSize;
			target.frame = frame;
			for ( std::uint32_t tag : tags )
				visibility.Record( tag, encoder, target );
			return std::nullopt;
		};
		if ( auto why = canvas->Render( textures, groups, {}, { 0, 0, 0, 1 }, nullptr, post ) )
			return why;
		auto encoded = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoded )
			return std::string( "a marker encoder was refused" );
		CommandEncoder marker = std::move( encoded ).Value();
		auto token = device->Submit( QueueKind::kGraphics, { &marker, 1 }, {} );
		if ( !token || !device->WaitIdle() )
			return std::string( "the marker submission failed" );
		visibility.FrameSubmitted( *device, token.Value() );
		return std::nullopt;
	};
	auto expect = [&]( std::uint32_t tag, std::int64_t visible, std::int64_t possible,
	                  const std::string &name )
	{
		const std::optional<Counts> counts = visibility.Result( tag );
		results.That( counts && counts->visible == visible && counts->possible == possible, name,
		    counts ? std::to_string( counts->visible ) + " of " + std::to_string( counts->possible )
		           : std::string( "pending" ) );
	};

	// The proxy over columns [4, 20) and rows [6, 18): 192 pixels.
	const std::int64_t area = 16 * 12;
	{
		const std::uint32_t open = visibility.Queue( Proxy( 4, 6, 20, 18, 0.5f ) );
		results.That(
		    open != 0 && !visibility.Result( open ), "visibility.pending-until-complete" );
		if ( auto why = run( std::nullopt, { open } ) )
			return why;
		expect( open, area, area, "visibility.unoccluded" );
		results.That( !visibility.Result( open ), "visibility.result-read-once" );
		const std::uint64_t recorded = visibility.Stats().recorded;
		if ( auto why = run( std::nullopt, { open } ) )
			return why;
		results.That(
		    visibility.Stats().recorded == recorded, "visibility.recorded-again-draws-nothing" );
	}
	{
		const std::uint32_t hidden = visibility.Queue( Proxy( 4, 6, 20, 18, 0.5f ) );
		if ( auto why = run( Occluder( -1.0f, 1.0f, 0.2f ), { hidden } ) )
			return why;
		expect( hidden, 0, area, "visibility.behind-full-occluder" );
	}
	{
		// The occluder's right edge at pixel boundary 16: columns 16..19 show.
		const std::uint32_t half = visibility.Queue( Proxy( 4, 6, 20, 18, 0.5f ) );
		if ( auto why = run( Occluder( -1.0f, ClipX( 16 ), 0.2f ), { half } ) )
			return why;
		expect( half, 4 * 12, area, "visibility.behind-half-occluder" );
	}
	{
		const std::uint32_t front = visibility.Queue( Proxy( 4, 6, 20, 18, 0.1f ) );
		if ( auto why = run( Occluder( -1.0f, 1.0f, 0.2f ), { front } ) )
			return why;
		expect( front, area, area, "visibility.in-front-of-occluder" );
	}
	{
		Query empty = Proxy( 4, 6, 20, 18, 0.5f );
		empty.viewport.width = 0;
		results.That( visibility.Queue( empty ) == 0, "visibility.refuse-empty-viewport" );
	}
	results.That( visibility.Stats().failed == 0 && world.Stats().viewsFailed == 0,
	    "visibility.no-failures", visibility.Stats().lastFailure + world.Stats().lastFailure );
	(void)device->WaitIdle();
	visibility.ReleaseDevice( *device );
	world.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "visibility.validation-silent" );
	return std::nullopt;
}

} // namespace

int RunVisibilitySuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "visibility", {}, RunChecks );
}

} // namespace render::lab
