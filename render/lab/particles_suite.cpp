//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lab.particles (RFC 0016 K8 particles): SpriteCard cards
//          and warp particles through the core's dynamic draws, against
//          analytic expectations: the corners render.sprite-card.v1 builds,
//          alpha, additive, $addself and $mod2x blending, vertex color and
//          alpha, the frame blend, depth feathering, and the Refract point's
//          warp scaled and tinted by the vertex color.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"
#include "render/pass/world/world_pass.h"
#include "render/sprite_card.h"
#include "spv/particles_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace render::lab
{
namespace
{
using namespace device;
using namespace pass::world;
constexpr unsigned kSize = 64;
// The card's two frames (sRGB bytes, then alpha), its record color (r, g, b,
// a) and the background the canvas clears to.
constexpr unsigned kFrame0[] = { 200, 100, 50, 255 };
constexpr unsigned kFrame1[] = { 40, 180, 220, 128 };
constexpr unsigned kRecord[] = { 128, 64, 192, 160 };
constexpr float kBackground[] = { .2f, .3f, .4f, .7f };
constexpr float kBlend = .25f;
constexpr float kOverbright = 2.0f;
constexpr int kBaseHandle = 1;
constexpr int kNormalHandle = 2;
constexpr int kDepthHandle = 3;
// The depth copy's byte: the opaque scene at 80/255 of the 192-unit range,
// just behind the card's clip z of 60.
constexpr unsigned kSceneDepthByte = 80;
constexpr float kDepthRange = 192.0f;
constexpr float kDepthScale = 4.0f;

class ParticleTextures final : public IWorldTextures
{
public:
	TextureId baseLinear, baseSrgb, normal, depth;
	TextureId Import( int handle, bool decoded ) override
	{
		if ( handle == kBaseHandle )
			return decoded ? baseSrgb : baseLinear;
		if ( handle == kNormalHandle )
			return normal;
		if ( handle == kDepthHandle && !decoded )
			return depth;
		return TextureId();
	}
	SamplerDesc Sampler( int ) override { return {}; }
};

double Decode( double value )
{
	return value <= .04045 ? value / 12.92 : std::pow( ( value + .055 ) / 1.055, 2.4 );
}

double Encode( double value )
{
	return value <= .0031308 ? value * 12.92 : 1.055 * std::pow( value, 1.0 / 2.4 ) - .055;
}

// World to clip: clip x, y = 128 x, 128 y; z = z; w = 128 (row-major, D3D9
// conventions, as the softparticle suite's view).
void SetClip( WorldView &view )
{
	view.toClip[0] = view.toClip[5] = view.toClip[15] = 128;
	view.toClip[10] = 1;
	view.viewport = { 0, 0, kSize, kSize, 0, 1 };
}

// One screen-aligned card of radius .5 at (0, 0, 60): the D3D view is the
// identity (eye at the origin looking down +z), so the card covers the
// view's middle half. Each frame's sheet rectangle is a point at its texel.
WorldView::DynamicDraw Card( std::vector<std::pair<std::string, std::string>> variables )
{
	WorldView::DynamicDraw draw;
	draw.material.name = "particle/lab_card";
	draw.material.shader = "Spritecard";
	draw.material.variables = std::move( variables );
	draw.material.variables.emplace_back( "$basetexture", "particle/lab_card_frames" );
	draw.material.textures.emplace_back( "$basetexture", kBaseHandle );
	constexpr float kCorners[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	for ( const auto &corner : kCorners )
	{
		sprite_card::Record record;
		record.position[2] = 60.0f;
		record.bgra[0] = std::uint8_t( kRecord[2] );
		record.bgra[1] = std::uint8_t( kRecord[1] );
		record.bgra[2] = std::uint8_t( kRecord[0] );
		record.bgra[3] = std::uint8_t( kRecord[3] );
		const float frame0[4] = { .25f, .5f, .25f, .5f };
		const float frame1[4] = { .75f, .5f, .75f, .5f };
		std::copy_n( frame0, 4, record.texCoords[0] );
		std::copy_n( frame1, 4, record.texCoords[1] );
		record.texCoords[2][0] = kBlend;
		record.texCoords[2][2] = .5f; // radius
		record.texCoords[3][0] = corner[0];
		record.texCoords[3][1] = corner[1];
		draw.cards.push_back( record );
	}
	draw.indices = { 0, 1, 2, 0, 2, 3 };
	return draw;
}

WorldView CardView( std::vector<std::pair<std::string, std::string>> variables, bool depth = false )
{
	WorldView view;
	SetClip( view );
	if ( depth )
	{
		view.depthAlphaHandle = kDepthHandle;
		view.depthAlphaRange = kDepthRange;
	}
	view.dynamicDraws.push_back( Card( std::move( variables ) ) );
	return view;
}

enum class Blend
{
	kAlpha,
	kAdditive,
	kAddSelf,
	kMod2x
};

// spritecard_ps2x's color for the card, in the canvas's linear target.
bool CardExpected(
    const CanvasImage &image, Blend blend, float addSelf, double feather, std::string &detail )
{
	const double frameAlpha = kFrame0[3] / 255.0 * ( 1.0 - kBlend ) + kFrame1[3] / 255.0 * kBlend;
	const double vertexAlpha = kRecord[3] / 255.0 * feather;
	const double alpha = frameAlpha * vertexAlpha;
	const float *pixel = image.At( kSize / 2, kSize / 2 );
	bool close = true;
	for ( unsigned c = 0; c < 3; ++c )
	{
		const double frame =
		    Decode( kFrame0[c] / 255.0 ) * ( 1.0 - kBlend ) + Decode( kFrame1[c] / 255.0 ) * kBlend;
		const double vertex = std::pow( kRecord[c] / 255.0, 2.2 );
		const double back = kBackground[c];
		double expected = 0.0;
		switch ( blend )
		{
		case Blend::kAlpha:
			expected = back * ( 1.0 - alpha ) + frame * kOverbright * vertex * alpha;
			break;
		case Blend::kAdditive:
			expected = back + frame * kOverbright * vertex * alpha;
			break;
		case Blend::kAddSelf:
		{
			double rgb = frame * kOverbright * alpha;
			rgb += kOverbright * addSelf * vertexAlpha * rgb;
			expected = back * ( 1.0 - alpha ) + rgb * vertex;
			break;
		}
		case Blend::kMod2x:
		{
			// A dimensionless factor toward the blend's neutral .5, in the
			// texture's stored encoding; the blend is 2 src dst.
			const double factor = std::clamp(
			    .5 + ( .5 + ( Encode( frame ) - .5 ) * vertex - .5 ) * alpha, 0.0, 1.0 );
			expected = 2.0 * factor * back;
			break;
		}
		}
		close &= std::isfinite( pixel[c] ) && std::abs( pixel[c] - expected ) < .004;
		detail += std::to_string( pixel[c] ) + "/" + std::to_string( expected ) + " ";
	}
	return close;
}

// The corners render.sprite-card.v1 builds, against geometry worked out by
// hand: a screen-aligned card spans its radius along the view's axes, a
// z-aligned card faces the eye around world z, a ground card lies in the
// model's xy plane, a spline card's middle is the Catmull-Rom point between
// its middle controls with its width across the curve, and size limits clamp
// and fade the radius.
void CheckCorners( Results &results )
{
	using namespace sprite_card;
	auto record = []( float cx, float cy, float radius, float yaw = 0.0f )
	{
		Record r;
		r.position[0] = 10.0f;
		r.position[1] = 0.0f;
		r.position[2] = 100.0f;
		r.texCoords[2][2] = radius;
		r.texCoords[2][3] = yaw;
		r.texCoords[3][0] = cx;
		r.texCoords[3][1] = cy;
		r.bgra[3] = 255;
		return r;
	};
	auto near = []( const float *a, std::initializer_list<float> b )
	{
		bool same = true;
		int i = 0;
		for ( float v : b )
			same &= std::abs( a[i++] - v ) < 1e-4f;
		return same;
	};
	// The identity view: the eye at the origin, view x, y, z on world x, y,
	// z. Corner (1, 1) of a screen card goes to -x (spritecard_vs20's
	// -x1 cos yaw) and +y.
	Frame screen;
	Prepare( screen );
	const Corner c11 = Expand( screen, record( 1, 1, 2 ) );
	results.That( near( c11.position, { 8, 2, 100 } ), "particles.corners.screen-aligned",
	    std::to_string( c11.position[0] ) + " " + std::to_string( c11.position[1] ) );
	// A view turned 90 degrees about y: view x is world -z. The same corner
	// moves along the turned axis, not world x (negative control: the
	// identity frame's answer).
	Frame turned;
	const float turnedView[16] = { 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1 };
	std::copy_n( turnedView, 16, turned.view );
	Prepare( turned );
	const Corner t11 = Expand( turned, record( 1, 1, 2 ) );
	results.That( near( t11.position, { 10, 2, 102 } ) && !near( t11.position, { 8, 2, 100 } ),
	    "particles.corners.screen-aligned-follows-view",
	    std::to_string( t11.position[0] ) + " " + std::to_string( t11.position[2] ) );
	Frame zAligned;
	zAligned.orientation = 1;
	Prepare( zAligned );
	const Corner z10 = Expand( zAligned, record( 1, 0, 2 ) );
	// To the eye: (-10, 0, -100) from the card; its right in the ground plane
	// is (0, 10)/10, so x1 = 1 moves +y by the radius, y1 = -1 moves -z.
	results.That( near( z10.position, { 10, 2, 98 } ), "particles.corners.z-aligned",
	    std::to_string( z10.position[1] ) + " " + std::to_string( z10.position[2] ) );
	Frame ground;
	ground.orientation = 2;
	Prepare( ground );
	const Corner g10 = Expand( ground, record( 1, 0, 2 ) );
	results.That( near( g10.position, { 8, 2, 100 } ), "particles.corners.ground-parallel",
	    std::to_string( g10.position[0] ) + " " + std::to_string( g10.position[1] ) );
	// $maxsize .01 at distance ~100.5 clamps the radius to ~1.005.
	Frame clamped;
	clamped.sizes.maxSize = .01f;
	Prepare( clamped );
	const Corner m11 = Expand( clamped, record( 1, 1, 2 ) );
	const float l = std::sqrt( 10.0f * 10.0f + 100.0f * 100.0f );
	results.That( near( m11.position, { 10 - .01f * l, .01f * l, 100 } ),
	    "particles.corners.max-size-clamps", std::to_string( m11.position[1] ) );
	// $startfadesize .015 / $endfadesize .025: the radius 2 is past the start
	// (1.507) and short of the end (2.512), so the color fades linearly.
	Frame fading;
	fading.sizes.startFadeSize = .015f;
	fading.sizes.endFadeSize = .025f;
	Prepare( fading );
	const Corner f11 = Expand( fading, record( 1, 1, 2 ) );
	const float tint = 1.0f - ( 2.0f - .015f * l ) / ( .025f * l - .015f * l );
	results.That( std::abs( f11.color[3] - tint ) < 1e-4f, "particles.corners.size-fade",
	    std::to_string( f11.color[3] ) + " expected " + std::to_string( tint ) );
	// A spline card along x: controls at x = 0, 10, 20, 30 with width 4. At
	// t = .5 the curve is at x = 15; side 1 offsets half the width across the
	// curve (the cross of the eye direction and the tangent).
	Frame spline;
	spline.kind = Kind::kSpline;
	const float splineView[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 100, 1 };
	std::copy_n( splineView, 16, spline.view ); // the eye at z = -100
	Prepare( spline );
	Record s;
	s.position[0] = .5f;
	s.position[1] = 0.0f;
	s.position[2] = 1.0f;
	for ( int k = 0; k < 4; ++k )
	{
		s.texCoords[k][0] = 10.0f * k;
		s.texCoords[k][3] = 4.0f;
	}
	const Corner sc = Expand( spline, s );
	results.That( near( sc.position, { 15, 2, 0 } ) && near( sc.uv, { 0, 0 } ),
	    "particles.corners.spline-midpoint-and-width",
	    std::to_string( sc.position[0] ) + " " + std::to_string( sc.position[1] ) );
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	CheckCorners( results );

	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, kSize, kSize, canvas ) )
		return why;
	LabSceneColorCapture sceneColor( *device );
	ParticleTextures imports;
	for ( bool srgb : { false, true } )
	{
		TextureDesc desc;
		desc.width = 2;
		desc.height = 1;
		desc.format = srgb ? Format::kRGBA8Srgb : Format::kRGBA8Unorm;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		std::byte texels[8];
		for ( unsigned c = 0; c < 4; ++c )
		{
			texels[c] = std::byte( kFrame0[c] );
			texels[4 + c] = std::byte( kFrame1[c] );
		}
		auto staged = textures.Stage(
		    srgb ? "particles/frames-srgb" : "particles/frames-linear", desc, texels );
		if ( !staged )
			return "particle frame texture staging failed";
		( srgb ? imports.baseSrgb : imports.baseLinear ) = staged.Value().texture;
	}
	{
		TextureDesc desc;
		desc.width = desc.height = 1;
		desc.format = Format::kRGBA8Unorm;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		// A normal tilted fully toward +x, at full alpha: the warp moves the
		// lookup right by $refractamount.
		const std::byte normal[4] = {
		    std::byte( 255 ), std::byte( 128 ), std::byte( 255 ), std::byte( 255 ) };
		auto staged = textures.Stage( "particles/warp-normal", desc, normal );
		if ( !staged )
			return "particle warp normal staging failed";
		imports.normal = staged.Value().texture;
		std::vector<std::byte> depth( 16 * 16 * 4, std::byte( 0 ) );
		for ( std::size_t i = 3; i < depth.size(); i += 4 )
			depth[i] = std::byte( kSceneDepthByte );
		desc.width = desc.height = 16;
		auto depthStaged = textures.Stage( "particles/depth-alpha", desc, depth );
		if ( !depthStaged )
			return "particle depth-alpha staging failed";
		imports.depth = depthStaged.Value().texture;
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
			target.colorCopySource = true;
			target.sceneColorCapture = &sceneColor;
			target.frame = ++frame;
			pass.Record( tag, encoder, target );
			return std::nullopt;
		};
		return canvas->Render(
		    textures, groups, {}, { clear[0], clear[1], clear[2], clear[3] }, &image, post );
	};

	struct BlendCase
	{
		const char *name;
		Blend blend;
		std::vector<std::pair<std::string, std::string>> variables;
		float addSelf;
	};
	const std::string overbright = std::to_string( kOverbright );
	const BlendCase cases[] = {
	    { "particles.frame-blend.vertex-color.alpha", Blend::kAlpha,
	        { { "$translucent", "1" }, { "$overbrightfactor", overbright } }, 0.0f },
	    { "particles.frame-blend.vertex-color.additive", Blend::kAdditive,
	        { { "$additive", "1" }, { "$overbrightfactor", overbright } }, 0.0f },
	    { "particles.addself", Blend::kAddSelf,
	        { { "$translucent", "1" }, { "$overbrightfactor", overbright }, { "$addself", "1.5" } },
	        1.5f },
	    { "particles.mod2x", Blend::kMod2x, { { "$mod2x", "1" } }, 0.0f },
	};
	for ( const BlendCase &c : cases )
	{
		CanvasImage image;
		if ( auto why = render( CardView( c.variables ), kBackground, image ) )
			return c.name + std::string( ": " ) + *why;
		std::string detail;
		results.That( CardExpected( image, c.blend, c.addSelf, 1.0, detail ), c.name, detail );
		// Outside the card the background is untouched.
		const float *outside = image.At( 2, 2 );
		results.That( std::abs( outside[0] - kBackground[0] ) < .002f,
		    c.name + std::string( ".outside-unchanged" ), std::to_string( outside[0] ) );
	}

	// $depthblend: the vertex alpha feathered by the gap to the opaque scene
	// over $depthblendscale.
	{
		CanvasImage image;
		if ( auto why =
		         render( CardView( { { "$translucent", "1" }, { "$overbrightfactor", overbright },
		                               { "$depthblend", "1" },
		                               { "$depthblendscale", std::to_string( kDepthScale ) } },
		                     true ),
		             kBackground, image ) )
			return "particles.depth-feather: " + *why;
		const double scene = kSceneDepthByte / 255.0;
		const double feather =
		    std::clamp( std::abs( scene * kDepthRange - 60.0 ) / kDepthScale, 0.0, 1.0 );
		std::string detail;
		results.That( CardExpected( image, Blend::kAlpha, 0.0f, feather, detail ),
		    "particles.depth-feather", detail );
		results.That(
		    feather < .1, "particles.depth-feather.fixture-near-scene", std::to_string( feather ) );
	}

	// Claims and refusals by name.
	{
		const auto dual = pass.QueueView( CardView( { { "$dualsequence", "1" } } ) );
		results.That(
		    !dual && pass.Stats().lastRefusal.find( "$dualsequence" ) != std::string::npos,
		    "particles.refuse.dualsequence", pass.Stats().lastRefusal );
		const auto lum = pass.QueueView( CardView( { { "$maxlumframeblend1", "1" } } ) );
		results.That(
		    !lum && pass.Stats().lastRefusal.find( "$maxlumframeblend" ) != std::string::npos,
		    "particles.refuse.maxlumframeblend", pass.Stats().lastRefusal );
		WorldView notCard = CardView( {} );
		notCard.dynamicDraws.front().material.shader = "UnlitGeneric";
		const auto wrong = pass.QueueView( std::move( notCard ) );
		results.That(
		    !wrong && pass.Stats().lastRefusal.find( "not a SpriteCard" ) != std::string::npos,
		    "particles.refuse.cards-for-another-shader", pass.Stats().lastRefusal );
		const auto depthMissing = pass.QueueView( CardView( { { "$depthblend", "1" } } ) );
		results.That(
		    !depthMissing && pass.Stats().lastRefusal.find( "$depthblend" ) != std::string::npos,
		    "particles.refuse.depthblend-without-depth", pass.Stats().lastRefusal );
	}

	// Warp particles: the Refract point over a scene whose left half is red.
	// The normal moves the lookup right by $refractamount times the vertex
	// alpha; the vertex alpha also weighs the warped, vertex-tinted scene
	// against the unwarped one.
	{
		const float black[] = { 0, 0, 0, 1 };
		auto warpView = [&]( unsigned alphaByte, unsigned redByte )
		{
			WorldView view;
			SetClip( view );
			WorldView::DynamicDraw red;
			red.material.name = "particles/lab_red";
			red.material.shader = "UnlitGeneric";
			red.material.variables = { { "$vertexcolor", "1" } };
			const float quad[4][2] = { { -1, -1 }, { 0, -1 }, { 0, 1 }, { -1, 1 } };
			for ( const auto &xy : quad )
			{
				WorldVertex v{};
				v.position[0] = xy[0];
				v.position[1] = xy[1];
				v.position[2] = 100.0f;
				v.color[0] = 255;
				v.color[1] = v.color[2] = 0;
				v.color[3] = 255;
				red.vertices.push_back( v );
			}
			red.indices = { 0, 1, 2, 0, 2, 3 };
			view.dynamicDraws.push_back( std::move( red ) );
			WorldView::DynamicDraw warp;
			warp.material.name = "particle/lab_warp";
			warp.material.shader = "Refract_DX90";
			warp.material.mesh = true;
			warp.material.variables = { { "$normalmap", "particles/warp-normal" },
			    { "$refractamount", ".25" }, { "$vertexcolormodulate", "1" },
			    { "$vertexcolor", "1" }, { "$vertexalpha", "1" }, { "$translucent", "1" } };
			warp.material.textures.emplace_back( "$normalmap", kNormalHandle );
			const float full[4][2] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
			for ( const auto &xy : full )
			{
				WorldVertex v{};
				v.position[0] = xy[0];
				v.position[1] = xy[1];
				v.position[2] = 50.0f;
				v.normal[2] = -1.0f;
				v.tangentS[0] = 1.0f;
				v.tangentT[1] = 1.0f;
				v.color[0] = std::uint8_t( redByte );
				v.color[1] = v.color[2] = 255;
				v.color[3] = std::uint8_t( alphaByte );
				warp.vertices.push_back( v );
			}
			warp.indices = { 0, 1, 2, 0, 2, 3 };
			view.dynamicDraws.push_back( std::move( warp ) );
			return view;
		};
		// The pixel at view x .4: red in the scene, black .25 to its right.
		const unsigned px = unsigned( .4f * kSize ) + 0, py = kSize / 2;
		struct WarpCase
		{
			const char *name;
			unsigned alpha, red;
			double expected;
		};
		// Alpha .6: the lookup lands at .55, black; the result is .4 of red.
		// Alpha .2: it lands at .45, still red, tinted by the vertex red
		// (128: .2195 linear) with weight .2.
		const double tintRed = std::pow( 128.0 / 255.0, 2.2 );
		const WarpCase warpCases[] = {
		    { "particles.warp.vertex-alpha-scales-warp", 153, 255, 1.0 - 153.0 / 255.0 },
		    { "particles.warp.vertex-alpha-weak-warp-stays", 51, 255, 1.0 },
		    { "particles.warp.vertex-color-tints", 51, 128,
		        ( 1.0 - 51.0 / 255.0 ) + 51.0 / 255.0 * tintRed },
		};
		for ( const WarpCase &c : warpCases )
		{
			CanvasImage image;
			if ( auto why = render( warpView( c.alpha, c.red ), black, image ) )
				return c.name + std::string( ": " ) + *why;
			const float value = image.At( px, py )[0];
			results.That( std::abs( value - c.expected ) < .01, c.name,
			    std::to_string( value ) + " expected " + std::to_string( c.expected ) );
		}
	}

	results.That( pass.Stats().viewsFailed == 0, "particles.claimed-views-complete",
	    pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "particles.validation-silent" );
	return std::nullopt;
}

const Seeded kSeeded[] = {
    { "frame-blend-ignored", spirv::kCardFrameBlendIgnored, "particles.frame-blend" },
    { "vertex-color-ignored", spirv::kCardVertexColorIgnored,
        "particles.frame-blend.vertex-color" },
    { "addself-ignored", spirv::kCardAddSelfIgnored, "particles.addself" },
    { "mod2x-neutral-zero", spirv::kCardMod2xNeutralZero, "particles.mod2x" },
    { "depth-feather-ignored", spirv::kCardDepthBlendIgnored, "particles.depth-feather" },
    { "warp-vertex-alpha-ignored", spirv::kWarpVertexAlphaIgnored, "particles.warp" },
};

} // namespace

int RunParticlesSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "particles", kSeeded, RunChecks );
}

} // namespace render::lab
