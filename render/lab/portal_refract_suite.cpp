//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite portal-refract (RFC 0016 K8 portal views):
//			PortalRefract's color stages on the core, judged against
//			portal_refract_vs20 and portal_refract_ps2x restated here on a
//			64 x 64 canvas, the world pass keeping D3D9's pixel centers:
//			- stage 0 warps the scene color snapshot (a red ramp, linear in
//			  x) outward around the opening along the surface's projected
//			  tangent frame and darkens it in a ring; outside the oval the
//			  frame is untouched;
//			- stage 2 draws the flame rim: uniform noise through a gray sRGB
//			  ramp, times $portalcolorscale, alpha blended over the frame
//			  with the source alpha clamped to 1, as the legacy frame buffer
//			  did;
//			- stage 2 without its textures is refused by name.
//
//			Seeded (--sensitivity): an unwarped stage 0 must fail
//			portal-refract.stage0.opening-is-warped.
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

constexpr std::uint32_t kSize = 64;
constexpr float kExtent = 64.0f; // the quad's half size in world units
constexpr int kNoiseHandle = 1;
constexpr int kRampHandle = 2;
constexpr int kStaticHandle = 3;
constexpr unsigned kStatic[] = { 200, 100, 50, 180 };
constexpr float kOpen = 0.6f;
constexpr float kColorScale = 2.0f;
constexpr unsigned kNoiseByte = 128;
constexpr float kGray[] = { .25f, .25f, .25f, 1.0f };

class PortalTextures final : public IWorldTextures
{
public:
	TextureId noise, ramp, statics;
	TextureId Import( int handle, bool ) override
	{
		return handle == kNoiseHandle    ? noise
		       : handle == kRampHandle   ? ramp
		       : handle == kStaticHandle ? statics
		                                 : TextureId();
	}
	SamplerDesc Sampler( int ) override
	{
		SamplerDesc desc;
		desc.address = AddressMode::kClampToEdge;
		return desc;
	}
};

double Smoothstep( double e0, double e1, double x )
{
	const double t = std::clamp( ( x - e0 ) / ( e1 - e0 ), 0.0, 1.0 );
	return t * t * ( 3.0 - 2.0 * t );
}

double Linearstep( double e0, double e1, double x )
{
	return std::clamp( ( x - e0 ) / ( e1 - e0 ), 0.0, 1.0 );
}

double Decode( double v )
{
	return v <= .04045 ? v / 12.92 : std::pow( ( v + .055 ) / 1.055, 2.4 );
}

// The ramp's filtered value at c (gray bytes 0..255 over 256 texels, sRGB
// decoded per texel before filtering, clamped to the edge).
double Ramp( double c )
{
	const double x = c * 256.0 - 0.5;
	const int i = int( std::floor( x ) );
	const double f = x - i;
	auto texel = []( int t )
	{
		return Decode( std::clamp( t, 0, 255 ) / 255.0 );
	};
	return texel( i ) * ( 1.0 - f ) + texel( i + 1 ) * f;
}

struct Opening
{
	double u, v, radius, open, openSquared;
	double stretchX, stretchY;
};

// portal_refract_vs20's vUv0 and the pixel stage's opening terms at the
// pixel (x, y): D3D9's pixel centers put the quad's uv at x / size.
Opening At( std::uint32_t x, std::uint32_t y )
{
	Opening o;
	const double baseU = double( x ) / kSize, baseV = double( y ) / kSize;
	o.u = baseU * 1.075 - 0.0375;
	o.v = baseV * 1.075 - 0.0375;
	o.stretchX = o.u * 2.0 - 1.0;
	o.stretchY = o.v * 2.0 - 1.0;
	o.radius = std::sqrt( o.stretchX * o.stretchX + o.stretchY * o.stretchY );
	o.open = Smoothstep( 0.0, 1.0, kOpen );
	o.openSquared = o.open * o.open;
	return o;
}

// Stage 0's red at (x, y), over the ramp whose red is the pixel's x / size.
// The projected tangent is (0.5 / extent, 0) per world unit; the binormal
// moves only v, which the ramp ignores. The snapshot is read bilinearly at
// texel centers, so the ramp's value at u is u - 0.5 / size.
double Stage0Red( std::uint32_t x, std::uint32_t y, bool warped )
{
	const Opening o = At( x, y );
	if ( o.radius > 1.0 )
		return double( x ) / kSize;
	double tangentX = o.radius > 0.0 ? -o.stretchX / o.radius : 0.0;
	tangentX *= o.openSquared * ( 1.0 - std::pow( std::clamp( o.radius, 0.0, 1.0 ), 64.0 ) );
	tangentX *= Smoothstep( o.open * 1.5, o.open, o.radius ) * 32.0;
	const double unwarped = ( x + 0.5 ) / kSize;
	const double u = unwarped + ( warped ? tangentX * 0.5 / kExtent : 0.0 );
	double ring =
	    Linearstep( o.openSquared - 0.01, std::clamp( o.open * 2.0, 0.0, 1.0 ), o.radius );
	ring = std::abs( ring * 2.0 - 1.0 ) * 0.15 + 0.85;
	const double sampled = std::clamp( u - 0.5 / kSize, 0.0, 1.0 - 1.0 / kSize );
	return sampled * ring;
}

// Stage 2's color at (x, y) over the gray clear (alpha blend, alpha
// clamped to 1); a negative value marks a discarded pixel.
double Stage2Gray( std::uint32_t x, std::uint32_t y, double *alphaOut )
{
	const Opening o = At( x, y );
	const double cutout = o.radius <= o.openSquared ? 1.0 : 0.0;
	const double outer =
	    ( 1.0 - Linearstep( o.openSquared, o.openSquared + 0.075, o.radius ) ) * ( 1.0 - cutout );
	const double inner = Linearstep( o.openSquared - 0.3, o.openSquared, o.radius ) * cutout;
	const double fadeIn = std::max( std::clamp( o.open * 2.5, 0.0, 1.0 ), 0.0 );
	double mask = ( inner + outer ) * fadeIn;
	const double noise = kNoiseByte / 255.0;
	const double activeWithNoise = Smoothstep( 0.0, noise, 1.0 );
	const double border = 1.0 - Smoothstep( mask - 0.875, mask + 0.875, noise );
	mask *= border;
	const double transparency =
	    std::clamp( mask + cutout * ( 1.0 - activeWithNoise ), 0.0, 1.0 ) * 1.5;
	*alphaOut = transparency;
	if ( transparency <= 1.0 / 255.0 )
		return -1.0;
	const double shift = std::pow( std::abs( o.v ), 1.5 ) * 0.8 + 0.2;
	const double flame = Ramp( std::sqrt( border ) * shift * transparency ) * kColorScale;
	const double a = std::min( transparency, 1.0 );
	return kGray[0] * ( 1.0 - a ) + flame * a;
}

WorldView View()
{
	WorldView view;
	view.toClip[0] = view.toClip[5] = 1.0f / kExtent;
	view.toClip[10] = view.toClip[15] = 1.0f;
	std::copy_n( view.toClip, 16, view.motionToClip );
	view.viewport = { 0, 0, float( kSize ), float( kSize ), 0, 1 };
	view.drawsWorldGeometry = false;
	return view;
}

WorldView::DynamicDraw Quad(
    WorldMaterial material, float z, std::uint8_t leftRed = 255, std::uint8_t rightRed = 255 )
{
	WorldView::DynamicDraw draw;
	draw.material = std::move( material );
	const float corners[4][2] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
	for ( const auto &corner : corners )
	{
		WorldVertex v{};
		v.position[0] = corner[0] * kExtent;
		v.position[1] = corner[1] * kExtent;
		v.position[2] = z;
		v.uv[0] = ( corner[0] + 1.0f ) * 0.5f;
		v.uv[1] = ( 1.0f - corner[1] ) * 0.5f;
		v.color[0] = corner[0] < 0 ? leftRed : rightRed;
		v.color[1] = v.color[2] = 0;
		draw.vertices.push_back( v );
	}
	draw.indices = { 0, 1, 2, 0, 2, 3 };
	return draw;
}

WorldMaterial Portal( int stage, bool textures = true )
{
	WorldMaterial material;
	material.name = "models/portals/lab_portal";
	material.shader = "PortalRefract";
	material.variables = { { "$stage", std::to_string( stage ) },
	    { "$portalopenamount", std::to_string( kOpen ) }, { "$portalstatic", "0" },
	    { "$portalcolorscale", std::to_string( kColorScale ) }, { "$time", "10" },
	    { "$model", "1" }, { "$nocull", "1" } };
	if ( textures )
	{
		material.variables.emplace_back( "$portalmasktexture", "lab/portal-noise" );
		material.variables.emplace_back( "$portalcolortexture", "lab/portal-ramp" );
		material.textures.emplace_back( "$portalmasktexture", kNoiseHandle );
		material.textures.emplace_back( "$portalcolortexture", kRampHandle );
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
	LabSceneColorCapture sceneColor( *device );
	PortalTextures imports;
	{
		TextureDesc desc;
		desc.format = Format::kRGBA8Unorm;
		desc.width = desc.height = 1;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		const std::byte noise[4] = { std::byte( kNoiseByte ), std::byte( kNoiseByte ),
		    std::byte( kNoiseByte ), std::byte( 255 ) };
		auto staged = textures.Stage( "portal-refract/noise", desc, noise );
		if ( !staged )
			return std::string( "the noise texture did not stage" );
		imports.noise = staged.Value().texture;
		desc.width = 256;
		desc.format = Format::kRGBA8Srgb;
		std::vector<std::byte> ramp( 256 * 4 );
		for ( unsigned i = 0; i < 256; ++i )
			for ( unsigned c = 0; c < 4; ++c )
				ramp[i * 4 + c] = std::byte( c == 3 ? 255 : i );
		auto rampStaged = textures.Stage( "portal-refract/ramp", desc, ramp );
		if ( !rampStaged )
			return std::string( "the ramp texture did not stage" );
		imports.ramp = rampStaged.Value().texture;
		desc.width = 1;
		const std::byte statics[4] = { std::byte( kStatic[0] ), std::byte( kStatic[1] ),
		    std::byte( kStatic[2] ), std::byte( kStatic[3] ) };
		auto staticStaged = textures.Stage( "portal-refract/static", desc, statics );
		if ( !staticStaged )
			return std::string( "the static texture did not stage" );
		imports.statics = staticStaged.Value().texture;
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

	// Stage 0 over a red ramp, linear in x (vertex colors decoded per vertex).
	{
		WorldMaterial ramp;
		ramp.name = "lab/red-ramp";
		ramp.shader = "UnlitGeneric";
		ramp.variables = { { "$vertexcolor", "1" }, { "$nofog", "1" } };
		WorldView view = View();
		view.dynamicDraws.push_back( Quad( ramp, 0.5f, 0, 255 ) );
		view.dynamicDraws.push_back( Quad( Portal( 0 ), 0.4f ) );
		CanvasImage image;
		const float black[] = { 0, 0, 0, 1 };
		if ( auto why = render( std::move( view ), black, image ) )
			return "portal-refract.stage0: " + *why;
		auto judge = [&]( std::uint32_t x, std::uint32_t y, const std::string &name )
		{
			const double expected = Stage0Red( x, y, true );
			const float red = image.At( x, y )[0];
			results.That( std::abs( red - expected ) < .006, name,
			    std::to_string( red ) + "/" + std::to_string( expected ) + " (unwarped " +
			        std::to_string( Stage0Red( x, y, false ) ) + ")" );
		};
		judge( 20, 32, "portal-refract.stage0.opening-is-warped" );
		judge( 44, 32, "portal-refract.stage0.opening-is-warped.right" );
		judge( 32, 12, "portal-refract.stage0.ring" );
		judge( 2, 2, "portal-refract.stage0.outside-the-oval-unchanged" );
		results.That( std::abs( Stage0Red( 20, 32, true ) - Stage0Red( 20, 32, false ) ) > .05,
		    "portal-refract.stage0.fixture-discriminates" );
	}

	// Stage 2: the flame over the gray clear.
	{
		WorldView view = View();
		view.dynamicDraws.push_back( Quad( Portal( 2 ), 0.4f ) );
		CanvasImage image;
		if ( auto why = render( std::move( view ), kGray, image ) )
			return "portal-refract.stage2: " + *why;
		int judged = 0;
		bool close = true;
		std::string detail;
		for ( std::uint32_t y = 4; y < kSize; y += 6 )
			for ( std::uint32_t x = 4; x < kSize; x += 6 )
			{
				double alpha = 0.0;
				double expected = Stage2Gray( x, y, &alpha );
				if ( expected < 0.0 )
					expected = kGray[0];
				else
					++judged;
				const float gray = image.At( x, y )[1];
				if ( std::abs( gray - expected ) > .01 && close )
				{
					close = false;
					detail = "(" + std::to_string( x ) + "," + std::to_string( y ) + ") " +
					         std::to_string( gray ) + "/" + std::to_string( expected );
				}
			}
		results.That( close, "portal-refract.stage2.flame-matches-the-oracle", detail );
		results.That(
		    judged > 4, "portal-refract.stage2.fixture-draws-flame", std::to_string( judged ) );
	}

	// Portal (portal_ps2x) without the alternate view: $basetexture (the gray
	// sRGB ramp) at the pixel, with $staticamount 0.5 of the static texture
	// (the uniform noise, read linear), opaque; a $renderfixz material is
	// refused by name.
	{
		WorldMaterial surface;
		surface.name = "models/portals/lab_portal_surface";
		surface.shader = "Portal";
		surface.variables = { { "$basetexture", "lab/portal-ramp" }, { "$staticamount", "0.5" },
		    { "$staticblendtexture", "lab/portal-noise" } };
		surface.textures = { { "$basetexture", kRampHandle }, { "$staticblendtexture", kNoiseHandle } };
		WorldView view = View();
		view.dynamicDraws.push_back( Quad( surface, 0.4f ) );
		CanvasImage image;
		if ( auto why = render( std::move( view ), kGray, image ) )
			return "portal-refract.portal-surface: " + *why;
		bool close = true;
		std::string detail;
		for ( std::uint32_t x : { 8u, 24u, 40u, 56u } )
		{
			const double expected = Ramp( ( x + 0.5 ) / kSize ) * 0.5 + kNoiseByte / 255.0 * 0.5;
			const float got = image.At( x, 32 )[1];
			if ( std::abs( got - expected ) > .006 && close )
			{
				close = false;
				detail = std::to_string( x ) + ": " + std::to_string( got ) + "/" +
				         std::to_string( expected );
			}
		}
		results.That( close, "portal-refract.portal-surface-mixes-the-frame-and-static", detail );
		WorldMaterial fixz = surface;
		fixz.variables.push_back( { "$renderfixz", "1" } );
		WorldView refused = View();
		refused.dynamicDraws.push_back( Quad( fixz, 0.4f ) );
		const auto tag = pass.QueueView( std::move( refused ) );
		results.That( !tag && pass.Stats().lastRefusal.find( "$renderfixz" ) != std::string::npos,
		    "portal-refract.refuse-portal-renderfixz", pass.Stats().lastRefusal );
	}

	// PortalStaticOverlay's ghost: the static texture times the vertex alpha
	// and $staticamount, premultiplied over the clear; the portal faces away
	// from the viewer, so no distance fade applies.
	{
		WorldMaterial ghost;
		ghost.name = "models/portals/lab_ghost";
		ghost.shader = "PortalStaticOverlay";
		ghost.variables = { { "$ghostoverlay", "2" }, { "$staticamount", "0.3" },
		    { "$staticblendtexture", "lab/static" }, { "$additive", "1" }, { "$nocull", "1" } };
		ghost.textures = { { "$staticblendtexture", kStaticHandle } };
		WorldView view = View();
		auto quad = Quad( ghost, 0.4f );
		for ( auto &v : quad.vertices )
			v.color[3] = 128;
		view.dynamicDraws.push_back( std::move( quad ) );
		CanvasImage image;
		if ( auto why = render( std::move( view ), kGray, image ) )
			return "portal-refract.ghost: " + *why;
		const double va = 128.0 / 255.0, alpha = kStatic[3] / 255.0 * va;
		const float *pixel = image.At( 32, 32 );
		bool close = true;
		std::string detail;
		for ( int c = 0; c < 3; ++c )
		{
			const double expected =
			    Decode( kStatic[c] / 255.0 ) * va * 0.3 + kGray[c] * ( 1.0 - alpha );
			close &= std::abs( pixel[c] - expected ) < .004;
			detail += std::to_string( pixel[c] ) + "/" + std::to_string( expected ) + " ";
		}
		results.That( close, "portal-refract.ghost-overlay-is-the-premultiplied-static", detail );
		// Facing the viewer closer than 120 units the ghost fades out entirely.
		WorldView facing = View();
		auto near = Quad( ghost, 0.4f );
		for ( auto &v : near.vertices )
		{
			v.color[3] = 128;
			v.normal[2] = -1.0f;
		}
		facing.dynamicDraws.push_back( std::move( near ) );
		CanvasImage faded;
		if ( auto why = render( std::move( facing ), kGray, faded ) )
			return "portal-refract.ghost-facing: " + *why;
		results.That( std::abs( faded.At( 32, 32 )[0] - kGray[0] ) < .002,
		    "portal-refract.ghost-facing-near-is-faded-out",
		    std::to_string( faded.At( 32, 32 )[0] ) );
		WorldMaterial plain = ghost;
		plain.variables = { { "$staticamount", "0.3" } };
		WorldView refused = View();
		refused.dynamicDraws.push_back( Quad( plain, 0.4f ) );
		const auto tag = pass.QueueView( std::move( refused ) );
		results.That( !tag && pass.Stats().lastRefusal.find( "$ghostoverlay" ) != std::string::npos,
		    "portal-refract.refuse-overlay-without-ghost", pass.Stats().lastRefusal );
	}

	{
		WorldView view = View();
		view.dynamicDraws.push_back( Quad( Portal( 2, false ), 0.4f ) );
		const auto tag = pass.QueueView( std::move( view ) );
		results.That(
		    !tag && pass.Stats().lastRefusal.find( "$portalmasktexture" ) != std::string::npos,
		    "portal-refract.refuse-stage2-without-textures", pass.Stats().lastRefusal );
	}
	results.That(
	    pass.Stats().viewsFailed == 0, "portal-refract.views-complete", pass.Stats().lastFailure );
	(void)device->WaitIdle();
	pass.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "portal-refract.validation-silent" );
	return std::nullopt;
}

const Seeded kSeeded[] = {
    { "unwarped", spirv::kPortalRefractUnwarped, "portal-refract.stage0.opening-is-warped" },
};

} // namespace

int RunPortalRefractSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "portal-refract", kSeeded, RunChecks );
}

} // namespace render::lab
