//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite panel (RFC 0016 K11; render.world-panel.v1 and
//			render.pass.panels): a UI panel in the world drawn by the core as
//			an emissive surface, judged by oracles that share no code with
//			the pass or its shaders:
//			- resolution: world_panel::PixelsPerUnit against a per-pixel ray
//			  cast of the canvas onto the panel (the Jacobian of its units by
//			  pixel, inverted), and ChooseResolution's ladder, hold and
//			  clamps;
//			- sharpness: a hard edge in the list, seen close, spans at most
//			  1.6 pixels (10 to 90 percent) at the chosen resolution; the
//			  same list at the legacy panel's one texel per unit must span
//			  more than 2 (the control, which the suite requires to fail);
//			- emission: a flat image value v shows as SrgbToLinear(v) times
//			  the emission scale; with the emission term off the panel is
//			  black (it is emission, not a lit base); seen from behind it is
//			  not drawn;
//			- one frame, one image: each frame's view shows that frame's
//			  list even when the next frame's is submitted first, two views
//			  of a frame share one raster, a second list for a frame is
//			  refused, and a view of a frame with no list fails by name;
//			- mips: a one-texel checker minified 8 times reads its linear
//			  mean (0.5), not its gamma mean (0.214);
//			- the light it casts: world_panel::TileRadiance (the area
//			  lights' radiance) against the GPU image's tile means, per
//			  flicker state, and a dirty lower half casting less light from
//			  there (with its clean control).
//
//			Seeded (--sensitivity): gamma-mips (the mip chain averaged in
//			gamma) must fail panel.mips; no-scatter (the coatings scatter no
//			light) panel.coating.scatter; coating-ignored (the emission
//			passes the coatings untouched) the coated tiles' match.
//
//			RENDER_LAB_IMAGES=<dir> writes each judged frame there as a PFM.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/material/material_programs.h"
#include "render/math/matrix.h"
#include "render/pass/panels/panels.h"
#include "render/shaderlib/debug_view.h"
#include "render/world_panel.h"
#include "spv/panels_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
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
using pass::panels::Panel;
using pass::panels::PanelPass;
using pass::panels::PanelTarget;
using pass::panels::PanelView;
using world_panel::Quad;

// The panel: 400 x 800 units (the chamber sign's aspect), 100 x 200 world
// units, upright in the plane y = 0 and facing -y.
constexpr float kUnitsWide = 400.0f;
constexpr float kUnitsTall = 800.0f;
constexpr world_panel::Placement kPlacement = {
    { -50.0f, 0.0f, 100.0f }, { 100.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -200.0f } };
constexpr std::uint32_t kSize = 256;
constexpr float kFov = 1.0471976f; // 60 degrees

// A lab texture: its gamma RGBA8 texels (the CPU copy the integrator reads)
// and its sampler.
struct LabTexture
{
	std::string name;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> texels;
	SamplerDesc sampler;
};

class LabTextures final : public pass::panels::IPanelTextures
{
public:
	explicit LabTextures( resources::TextureCache &cache ) : m_Cache( cache ) {}

	int Add( LabTexture texture )
	{
		TextureDesc desc;
		desc.format = Format::kRGBA8Unorm;
		desc.width = texture.width;
		desc.height = texture.height;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		if ( !m_Cache.Stage( texture.name, desc, std::as_bytes( std::span( texture.texels ) ) ) )
			return -1;
		m_Textures.push_back( std::move( texture ) );
		return int( m_Textures.size() ) - 1;
	}

	TextureId Import( int key ) override
	{
		if ( key < 0 || key >= int( m_Textures.size() ) )
			return {};
		const resources::TextureEntry *entry = m_Cache.Find( m_Textures[key].name );
		return entry ? entry->texture : TextureId();
	}
	SamplerDesc Sampler( int key ) override
	{
		return key >= 0 && key < int( m_Textures.size() ) ? m_Textures[key].sampler : SamplerDesc();
	}

	// The CPU twin of the GPU's sampling (bilinear or nearest, wrapping or
	// clamped), in gamma values: the integrator's sampler.
	bool Sample( int key, float s, float t, float, float rgba[4] ) const
	{
		if ( key < 0 || key >= int( m_Textures.size() ) )
			return false;
		const LabTexture &tex = m_Textures[key];
		const bool clamp = tex.sampler.address == AddressMode::kClampToEdge;
		auto texel = [&]( int x, int y, int k )
		{
			if ( clamp )
			{
				x = std::clamp( x, 0, int( tex.width ) - 1 );
				y = std::clamp( y, 0, int( tex.height ) - 1 );
			}
			else
			{
				x = ( ( x % int( tex.width ) ) + int( tex.width ) ) % int( tex.width );
				y = ( ( y % int( tex.height ) ) + int( tex.height ) ) % int( tex.height );
			}
			return tex.texels[( std::size_t( y ) * tex.width + x ) * 4 + k] / 255.0f;
		};
		const float fx = s * tex.width - 0.5f;
		const float fy = t * tex.height - 0.5f;
		if ( tex.sampler.magFilter == Filter::kNearest )
		{
			const int x = int( std::floor( s * tex.width ) );
			const int y = int( std::floor( t * tex.height ) );
			for ( int k = 0; k < 4; ++k )
				rgba[k] = texel( x, y, k );
			return true;
		}
		const int x0 = int( std::floor( fx ) );
		const int y0 = int( std::floor( fy ) );
		const float ax = fx - x0;
		const float ay = fy - y0;
		for ( int k = 0; k < 4; ++k )
		{
			rgba[k] =
			    ( texel( x0, y0, k ) * ( 1 - ax ) + texel( x0 + 1, y0, k ) * ax ) * ( 1 - ay ) +
			    ( texel( x0, y0 + 1, k ) * ( 1 - ax ) + texel( x0 + 1, y0 + 1, k ) * ax ) * ay;
		}
		return true;
	}

private:
	resources::TextureCache &m_Cache;
	std::vector<LabTexture> m_Textures;
};

Quad MakeQuad( float x0, float y0, float x1, float y1, std::uint32_t texture, std::uint8_t r,
    std::uint8_t g, std::uint8_t b, std::uint8_t a, std::uint8_t blend = world_panel::kBlendAlpha )
{
	Quad q;
	q.x0 = x0;
	q.y0 = y0;
	q.x1 = x1;
	q.y1 = y1;
	q.s0 = q.t0 = 0.0f;
	q.s1 = q.t1 = 1.0f;
	q.color[0] = r;
	q.color[1] = g;
	q.color[2] = b;
	q.color[3] = a;
	q.texture = texture;
	q.blend = blend;
	q.layer = world_panel::kLayerEmissive;
	q.coverage = -1.0f;
	return q;
}

void Store( const math::float4x4 &m, float out[16] )
{
	for ( int r = 0; r < 4; ++r )
	{
		out[r * 4 + 0] = m.rows[r].x;
		out[r * 4 + 1] = m.rows[r].y;
		out[r * 4 + 2] = m.rows[r].z;
		out[r * 4 + 3] = m.rows[r].w;
	}
}

// A perspective camera at `eye` looking at `target`, z up.
struct Camera
{
	float3 eye;
	float toClip[16] = {};
};

Camera Perspective(
    float3 eye, float3 target, std::uint32_t width = kSize, std::uint32_t height = kSize )
{
	Camera camera;
	camera.eye = eye;
	const math::float4x4 view = math::LookAt( eye, target, { 0.0f, 0.0f, 1.0f } );
	const math::float4x4 projection =
	    math::Perspective( kFov, float( width ) / float( height ), 1.0f, 10000.0f );
	Store( math::Multiply( projection, view ), camera.toClip );
	return camera;
}

// An orthographic camera that maps the panel exactly onto the canvas (its
// top-left corner to the canvas's), from in front.
Camera PanelFill( const world_panel::Placement &p )
{
	Camera camera;
	const float rr = p.right[0] * p.right[0] + p.right[1] * p.right[1] + p.right[2] * p.right[2];
	const float dd = p.down[0] * p.down[0] + p.down[1] * p.down[1] + p.down[2] * p.down[2];
	float ro = 0.0f, dO = 0.0f;
	for ( int k = 0; k < 3; ++k )
	{
		ro += p.right[k] * p.origin[k];
		dO += p.down[k] * p.origin[k];
	}
	// clip x = 2u - 1, clip y = 1 - 2v (y up), z = 0.5, w = 1.
	for ( int k = 0; k < 3; ++k )
	{
		camera.toClip[0 * 4 + k] = 2.0f * p.right[k] / rr;
		camera.toClip[1 * 4 + k] = -2.0f * p.down[k] / dd;
	}
	camera.toClip[0 * 4 + 3] = -2.0f * ro / rr - 1.0f;
	camera.toClip[1 * 4 + 3] = 2.0f * dO / dd + 1.0f;
	camera.toClip[2 * 4 + 3] = 0.5f;
	camera.toClip[3 * 4 + 3] = 1.0f;
	float normal[3];
	world_panel::Placement copy = p;
	normal[0] = copy.down[1] * copy.right[2] - copy.down[2] * copy.right[1];
	normal[1] = copy.down[2] * copy.right[0] - copy.down[0] * copy.right[2];
	normal[2] = copy.down[0] * copy.right[1] - copy.down[1] * copy.right[0];
	camera.eye = { p.origin[0] + normal[0], p.origin[1] + normal[1], p.origin[2] + normal[2] };
	return camera;
}

float Luma( const float *rgba )
{
	return 0.2126f * rgba[0] + 0.7152f * rgba[1] + 0.0722f * rgba[2];
}

// The 10-to-90 percent width, in pixels, of the step along row `y` between
// x0 and x1 (a falling or rising edge), from the plateaus at either end.
float EdgeWidth( const CanvasImage &image, std::uint32_t y, std::uint32_t x0, std::uint32_t x1 )
{
	std::vector<float> v;
	for ( std::uint32_t x = x0; x <= x1; ++x )
		v.push_back( Luma( image.At( x, y ) ) );
	const float lo = v.front(), hi = v.back();
	if ( std::fabs( hi - lo ) < 1e-3f )
		return 1e9f;
	auto crossing = [&]( float fraction )
	{
		const float level = lo + ( hi - lo ) * fraction;
		for ( std::size_t i = 1; i < v.size(); ++i )
		{
			const float a = v[i - 1] - level, b = v[i] - level;
			if ( ( a <= 0.0f && b > 0.0f ) || ( a >= 0.0f && b < 0.0f ) )
				return float( i - 1 ) + a / ( a - b );
		}
		return 1e9f;
	};
	return std::fabs( crossing( 0.9f ) - crossing( 0.1f ) );
}

// The oracle for PixelsPerUnit: every canvas pixel's ray against the
// panel's plane; where it hits, the panel units of the pixel's neighbours
// (half a pixel each way) give the Jacobian of units by pixel, whose inverse's
// columns are the screen pixels per unit along each panel axis.
float OraclePixelsPerUnit( const Camera &camera, std::uint32_t width, std::uint32_t height )
{
	const auto inverse = [&]
	{
		math::float4x4 m;
		for ( int r = 0; r < 4; ++r )
			m.rows[r] = { camera.toClip[r * 4 + 0], camera.toClip[r * 4 + 1],
			    camera.toClip[r * 4 + 2], camera.toClip[r * 4 + 3] };
		return math::Inverse( m );
	}();
	if ( !inverse )
		return -1.0f;
	const float rr = kPlacement.right[0] * kPlacement.right[0] +
	                 kPlacement.right[1] * kPlacement.right[1] +
	                 kPlacement.right[2] * kPlacement.right[2];
	const float dd = kPlacement.down[0] * kPlacement.down[0] +
	                 kPlacement.down[1] * kPlacement.down[1] +
	                 kPlacement.down[2] * kPlacement.down[2];
	// Panel units at a (fractional) pixel; false off the panel.
	auto unitsAt = [&]( float px, float py, float &u, float &v )
	{
		const float x = px / width * 2.0f - 1.0f;
		const float y = 1.0f - py / height * 2.0f;
		const float3 nearP = math::TransformPoint( *inverse, { x, y, 0.0f } );
		const float3 farP = math::TransformPoint( *inverse, { x, y, 1.0f } );
		const float dir[3] = { farP.x - nearP.x, farP.y - nearP.y, farP.z - nearP.z };
		// The plane y = 0.
		if ( std::fabs( dir[1] ) < 1e-9f )
			return false;
		const float t = -nearP.y / dir[1];
		if ( t < 0.0f || t > 1.0f )
			return false;
		const float hit[3] = { nearP.x + dir[0] * t, nearP.y + dir[1] * t, nearP.z + dir[2] * t };
		float ru = 0.0f, rv = 0.0f;
		for ( int k = 0; k < 3; ++k )
		{
			ru += ( hit[k] - kPlacement.origin[k] ) * kPlacement.right[k];
			rv += ( hit[k] - kPlacement.origin[k] ) * kPlacement.down[k];
		}
		u = ru / rr * kUnitsWide;
		v = rv / dd * kUnitsTall;
		return u >= 0.0f && u <= kUnitsWide && v >= 0.0f && v <= kUnitsTall;
	};
	float best = 0.0f;
	for ( std::uint32_t py = 0; py < height; ++py )
	{
		for ( std::uint32_t px = 0; px < width; ++px )
		{
			float u0, v0, u1, v1, u2, v2, u3, v3;
			const float cx = px + 0.5f, cy = py + 0.5f;
			if ( !unitsAt( cx - 0.5f, cy, u0, v0 ) || !unitsAt( cx + 0.5f, cy, u1, v1 ) ||
			     !unitsAt( cx, cy - 0.5f, u2, v2 ) || !unitsAt( cx, cy + 0.5f, u3, v3 ) )
				continue;
			// d(units)/d(pixel): columns per pixel axis.
			const float a = u1 - u0, b = u2 - u3 == 0.0f ? u3 - u2 : u3 - u2;
			const float c = v1 - v0, d = v3 - v2;
			const float det = a * d - b * c;
			if ( std::fabs( det ) < 1e-12f )
				continue;
			// Inverse: d(pixel)/d(units) = 1/det [ d -b; -c a ]; its columns are
			// the pixels per unit of u and of v.
			const float perU = std::sqrt( d * d + c * c ) / std::fabs( det );
			const float perV = std::sqrt( b * b + a * a ) / std::fabs( det );
			best = std::max( best, std::max( perU, perV ) );
		}
	}
	return best;
}

void WriteImage( const std::string &name, const CanvasImage &image )
{
	const char *dir = std::getenv( "RENDER_LAB_IMAGES" );
	if ( !dir || !*dir )
		return;
	std::filesystem::create_directories( dir );
	(void)WritePfm( std::filesystem::path( dir ) / ( name + ".gpu.pfm" ), image.width, image.height,
	    image.rgba );
}

// The suite's world: the device, a canvas, the textures and one pass.
struct Lab
{
	std::unique_ptr<IRenderDevice2> device;
	std::atomic<std::uint64_t> messages{ 0 };
	std::unique_ptr<Canvas> canvas;
	std::unique_ptr<Canvas> fill; // 128 x 256: the panel's aspect
	std::unique_ptr<resources::TextureCache> cache;
	std::unique_ptr<material::GroupResidency> groups;
	std::unique_ptr<LabTextures> textures;
	std::unique_ptr<PanelPass> pass;
	std::uint64_t frame = 1;
	int board = -1, dirt = -1, checker = -1;
};

// Renders the views (each a camera, a host frame and the panels it draws)
// into one canvas frame; `after` runs before the read back.
std::optional<std::string> Render( Lab &lab, Canvas &canvas,
    const std::vector<std::pair<Camera, PanelView>> &views, CanvasImage &out )
{
	const std::uint64_t frame = lab.frame++;
	CanvasPost post = [&]( CommandEncoder &encoder, TextureId color,
	                      TextureId depth ) -> std::optional<std::string>
	{
		for ( const auto &[camera, queued] : views )
		{
			PanelView view = queued;
			std::copy( camera.toClip, camera.toClip + 16, view.toClip );
			view.viewport = {
			    0.0f, 0.0f, float( canvas.Width() ), float( canvas.Height() ), 0.0f, 1.0f };
			const std::uint32_t tag = lab.pass->QueueView( view );
			if ( !tag )
				return std::string( "a view queued nothing" );
			PanelTarget target;
			target.device = lab.device.get();
			target.color = color;
			target.colorFormat = kCanvasColor;
			target.depth = depth;
			target.depthFormat = kCanvasDepth;
			target.width = canvas.Width();
			target.height = canvas.Height();
			target.textures = lab.textures.get();
			target.frame = frame;
			target.terms.eye[0] = camera.eye.x;
			target.terms.eye[1] = camera.eye.y;
			target.terms.eye[2] = camera.eye.z;
			lab.pass->Record( tag, encoder, target );
		}
		return std::nullopt;
	};
	return canvas.Render( *lab.cache, *lab.groups, {}, { 0.0f, 0.0f, 0.0f, 0.0f }, &out, post );
}

Panel MakePanel( std::vector<Quad> quads, float texelsPerUnit, float emissionScale = 1.0f )
{
	Panel panel;
	panel.id = 7;
	panel.placement = kPlacement;
	panel.unitsWide = kUnitsWide;
	panel.unitsTall = kUnitsTall;
	panel.quads = std::move( quads );
	world_panel::Resolution none = { 0, 0, 0.0f };
	panel.resolution = world_panel::ChooseResolution( kUnitsWide, kUnitsTall, texelsPerUnit, none );
	panel.emissionScale = emissionScale;
	return panel;
}

// The lab's lists name textures by their LabTextures keys, in order.
void NameTextures( const Lab &lab, Panel &panel )
{
	panel.textures = { lab.board, lab.dirt, lab.checker };
}

// Linear mean of a canvas region.
void Mean( const CanvasImage &image, std::uint32_t x0, std::uint32_t y0, std::uint32_t x1,
    std::uint32_t y1, float out[3] )
{
	double sum[3] = {};
	for ( std::uint32_t y = y0; y < y1; ++y )
		for ( std::uint32_t x = x0; x < x1; ++x )
			for ( int k = 0; k < 3; ++k )
				sum[k] += image.At( x, y )[k];
	const double n = double( x1 - x0 ) * double( y1 - y0 );
	for ( int k = 0; k < 3; ++k )
		out[k] = float( sum[k] / n );
}

std::string Rgb( const float c[3] )
{
	char text[96];
	std::snprintf( text, sizeof( text ), "(%.4f %.4f %.4f)", c[0], c[1], c[2] );
	return text;
}

// The board of the light and flicker cases: a gradient lit board (the
// sign's board is near white), dark icons, and grime whose alpha rises toward
// the bottom when `dirty`: a coating, or with `emissiveGrime` painted into
// the lit image as the legacy sign paints it (the control).
std::vector<Quad> BoardList(
    const Lab &lab, int brightness, bool dirty, int dirtAlpha = 255, bool emissiveGrime = false )
{
	std::vector<Quad> quads;
	const auto b = std::uint8_t( brightness );
	quads.push_back(
	    MakeQuad( 0, 0, kUnitsWide, kUnitsTall, 0, b, b, b, 255, world_panel::kBlendOpaque ) );
	quads.push_back( MakeQuad( 80, 580, 140, 640, world_panel::kWhite, 0, 0, 0, 255 ) );
	quads.push_back( MakeQuad( 150, 580, 210, 640, world_panel::kWhite, 0, 0, 0, 134 ) );
	if ( dirty )
	{
		Quad grime =
		    MakeQuad( 0, 0, kUnitsWide, kUnitsTall, 1, 255, 255, 255, std::uint8_t( dirtAlpha ) );
		grime.layer = emissiveGrime ? world_panel::kLayerEmissive : world_panel::kLayerCoating;
		quads.push_back( grime );
	}
	(void)lab;
	return quads;
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	Lab lab;
	if ( std::optional<std::string> why = CreateLabDevice( validate, lab.messages, lab.device ) )
		return why;
	if ( std::optional<std::string> why = Canvas::Create( *lab.device, kSize, kSize, lab.canvas ) )
		return why;
	if ( std::optional<std::string> why = Canvas::Create( *lab.device, 128, 256, lab.fill ) )
		return why;
	lab.cache = std::make_unique<resources::TextureCache>( *lab.device );
	lab.groups = std::make_unique<material::GroupResidency>( *lab.device, *lab.cache );
	lab.textures = std::make_unique<LabTextures>( *lab.cache );
	lab.pass = std::make_unique<PanelPass>( module );

	// The textures: a board (a smooth gradient near white), a dirt layer
	// (grey, alpha rising from 0 at the top to 0.8 at the bottom, with a
	// low-frequency pattern), and a 2 x 1 black and white checker (nearest,
	// wrapping).
	{
		LabTexture board{ "lab/panel_board", 64, 128, {}, {} };
		board.sampler.address = AddressMode::kClampToEdge;
		for ( std::uint32_t y = 0; y < board.height; ++y )
			for ( std::uint32_t x = 0; x < board.width; ++x )
				for ( int k = 0; k < 4; ++k )
					board.texels.push_back(
					    std::uint8_t( k == 3 ? 255
					                         : 200 + ( k == 2 ? 40 : 30 ) * ( y + x ) /
					                                     ( board.width + board.height ) ) );
		lab.board = lab.textures->Add( std::move( board ) );
		LabTexture dirt{ "lab/panel_dirt", 64, 128, {}, {} };
		dirt.sampler.address = AddressMode::kClampToEdge;
		for ( std::uint32_t y = 0; y < dirt.height; ++y )
		{
			for ( std::uint32_t x = 0; x < dirt.width; ++x )
			{
				const float fy = float( y ) / ( dirt.height - 1 );
				const float wave = 0.5f + 0.5f * std::sin( x * 0.3f ) * std::cos( y * 0.17f );
				const float alpha = std::clamp( 0.8f * fy * ( 0.7f + 0.3f * wave ), 0.0f, 1.0f );
				dirt.texels.push_back( 90 );
				dirt.texels.push_back( 84 );
				dirt.texels.push_back( 70 );
				dirt.texels.push_back( std::uint8_t( std::lround( alpha * 255.0f ) ) );
			}
		}
		lab.dirt = lab.textures->Add( std::move( dirt ) );
		LabTexture checker{ "lab/panel_checker", 2, 1, { 0, 0, 0, 255, 255, 255, 255, 255 }, {} };
		checker.sampler.minFilter = checker.sampler.magFilter = checker.sampler.mipFilter =
		    Filter::kNearest;
		lab.checker = lab.textures->Add( std::move( checker ) );
		if ( lab.board < 0 || lab.dirt < 0 || lab.checker < 0 )
			return std::string( "the lab's textures were refused" );
	}
	const world_panel::Resolution none = { 0, 0, 0.0f };

	// --- Resolution policy against the ray-cast oracle ---------------------
	struct Placed
	{
		const char *name;
		Camera camera;
	};
	const Placed placements[] = {
	    { "head-on-far", Perspective( { 0.0f, -900.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } ) },
	    { "head-on-mid", Perspective( { 0.0f, -250.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } ) },
	    { "head-on-close", Perspective( { 10.0f, -30.0f, 20.0f }, { 10.0f, 0.0f, 20.0f } ) },
	    { "oblique", Perspective( { -300.0f, -120.0f, 40.0f }, { 0.0f, 0.0f, 0.0f } ) },
	    { "partly-off-screen", Perspective( { 60.0f, -80.0f, 90.0f }, { 60.0f, 0.0f, 90.0f } ) },
	};
	const float viewport[4] = { 0.0f, 0.0f, float( kSize ), float( kSize ) };
	for ( const Placed &placed : placements )
	{
		const float got = world_panel::PixelsPerUnit(
		    kPlacement, kUnitsWide, kUnitsTall, placed.camera.toClip, viewport );
		const float want = OraclePixelsPerUnit( placed.camera, kSize, kSize );
		const float ratio = want > 0.0f ? got / want : 0.0f;
		results.That( ratio >= 0.93f && ratio <= 1.07f,
		    std::string( "panel.policy.pixels-per-unit." ) + placed.name,
		    "got " + std::to_string( got ) + ", ray-cast oracle " + std::to_string( want ) );
		const world_panel::Resolution chosen =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, got, none );
		const float density = chosen.texelsPerUnit;
		const bool clampedHigh = chosen.width == world_panel::kMaxDimension ||
		                         chosen.height == world_panel::kMaxDimension;
		const bool clampedLow = density <= world_panel::kMinTexelsPerUnit * 1.001f;
		const bool onLadder =
		    density >= want * 0.99f && density < want * world_panel::kDensityStep * 1.001f;
		results.That( onLadder || clampedHigh || ( clampedLow && want <= density ),
		    std::string( "panel.policy.texel-per-pixel." ) + placed.name,
		    "density " + std::to_string( density ) + " for " + std::to_string( want ) +
		        " pixels per unit" );
	}
	{
		const world_panel::Resolution a =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, 1.0f, none );
		const world_panel::Resolution held =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, 0.75f, a );
		const world_panel::Resolution dropped =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, 0.6f, a );
		const world_panel::Resolution grown =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, 1.05f, a );
		const world_panel::Resolution off =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, 0.0f, a );
		const world_panel::Resolution huge =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, 100.0f, none );
		results.That( a.texelsPerUnit == 1.0f && a.width == 400 && a.height == 800,
		    "panel.policy.ladder",
		    "one pixel per unit chose " + std::to_string( a.texelsPerUnit ) );
		results.That(
		    held == a, "panel.policy.hold", "0.75 pixels per unit dropped 1 texel per unit" );
		results.That( dropped.texelsPerUnit < a.texelsPerUnit && dropped.texelsPerUnit >= 0.6f,
		    "panel.policy.shrink",
		    "0.6 pixels per unit kept " + std::to_string( dropped.texelsPerUnit ) );
		results.That(
		    grown.texelsPerUnit >= 1.05f && grown.texelsPerUnit < 1.05f * world_panel::kDensityStep,
		    "panel.policy.grow",
		    "1.05 pixels per unit chose " + std::to_string( grown.texelsPerUnit ) );
		results.That(
		    off == a, "panel.policy.off-screen-keeps", "off screen changed the resolution" );
		results.That( huge.height == world_panel::kMaxDimension && huge.width == 2048,
		    "panel.policy.max-dimension",
		    std::to_string( huge.width ) + " x " + std::to_string( huge.height ) );
	}

	// --- Sharpness: a hard edge seen close -----------------------------------
	// Close enough for about 3 screen pixels per panel unit at the center.
	const Camera close = Perspective( { 0.0f, -18.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } );
	const float closeNeed =
	    world_panel::PixelsPerUnit( kPlacement, kUnitsWide, kUnitsTall, close.toClip, viewport );
	auto edgeList = [&]()
	{
		std::vector<Quad> quads;
		quads.push_back( MakeQuad( 0, 0, 200.37f, kUnitsTall, world_panel::kWhite, 255, 255, 255,
		    255, world_panel::kBlendOpaque ) );
		quads.push_back( MakeQuad( 200.37f, 0, kUnitsWide, kUnitsTall, world_panel::kWhite, 0, 0, 0,
		    255, world_panel::kBlendOpaque ) );
		return quads;
	};
	auto edgeAt = [&]( float density, float &width ) -> std::optional<std::string>
	{
		Panel panel = MakePanel( edgeList(), density );
		NameTextures( lab, panel );
		const std::uint64_t host = lab.frame;
		if ( !lab.pass->Submit( host, panel ) )
			return "the edge list was refused: " + lab.pass->Stats().lastFailure;
		PanelView view;
		view.hostFrame = host;
		view.panels = { panel.id };
		CanvasImage image;
		if ( std::optional<std::string> why =
		         Render( lab, *lab.canvas, { { close, view } }, image ) )
			return why;
		WriteImage( "panel-edge-" + std::to_string( density ), image );
		width = EdgeWidth( image, kSize / 2, kSize / 2 - 24, kSize / 2 + 24 );
		return std::nullopt;
	};
	{
		const world_panel::Resolution chosen =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, closeNeed, none );
		float sharp = 0.0f, legacy = 0.0f;
		if ( std::optional<std::string> why = edgeAt( chosen.texelsPerUnit, sharp ) )
			return why;
		if ( std::optional<std::string> why = edgeAt( 1.0f, legacy ) )
			return why;
		results.That( closeNeed > 2.0f, "panel.sharp.fixture-magnifies",
		    std::to_string( closeNeed ) + " pixels per unit" );
		results.That( sharp <= 1.6f, "panel.sharp.edge",
		    "the edge spans " + std::to_string( sharp ) + " pixels at " +
		        std::to_string( chosen.texelsPerUnit ) + " texels per unit" );
		// The control: the legacy panel's one texel per unit, magnified.
		results.That( legacy > 2.0f, "panel.control.fixed-resolution.rejected",
		    "at one texel per unit the edge spans " + std::to_string( legacy ) +
		        " pixels, which the sharpness check would pass" );
	}

	// --- Emission: the value, the term, the back ----------------------------
	const Camera mid = Perspective( { 0.0f, -250.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } );
	auto flat = [&]( std::uint8_t value, float scale, const frame::DebugControls &debug,
	                const Camera &camera, float out[3] ) -> std::optional<std::string>
	{
		std::vector<Quad> quads = { MakeQuad( 0, 0, kUnitsWide, kUnitsTall, world_panel::kWhite,
		    value, value, value, 255, world_panel::kBlendOpaque ) };
		Panel panel = MakePanel( std::move( quads ), 1.0f, scale );
		NameTextures( lab, panel );
		const std::uint64_t host = lab.frame;
		if ( !lab.pass->Submit( host, panel ) )
			return "the flat list was refused: " + lab.pass->Stats().lastFailure;
		PanelView view;
		view.hostFrame = host;
		view.panels = { panel.id };
		view.debug = debug;
		CanvasImage image;
		if ( std::optional<std::string> why =
		         Render( lab, *lab.canvas, { { camera, view } }, image ) )
			return why;
		Mean( image, kSize / 2 - 8, kSize / 2 - 8, kSize / 2 + 8, kSize / 2 + 8, out );
		return std::nullopt;
	};
	{
		float one[3], two[3], off[3], back[3];
		const frame::DebugControls neutral;
		frame::DebugControls noEmission;
		noEmission.termsOff = shaderlib::kDebugTermEmission;
		if ( std::optional<std::string> why = flat( 160, 1.0f, neutral, mid, one ) )
			return why;
		if ( std::optional<std::string> why = flat( 160, 2.0f, neutral, mid, two ) )
			return why;
		if ( std::optional<std::string> why = flat( 160, 1.0f, noEmission, mid, off ) )
			return why;
		const std::uint64_t failuresBefore = lab.pass->Failures();
		if ( std::optional<std::string> why = flat( 160, 1.0f, neutral,
		         Perspective( { 0.0f, 250.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } ), back ) )
			return why;
		const float want = world_panel::SrgbToLinear( 160.0f / 255.0f );
		results.That( std::fabs( one[1] - want ) <= 0.01f * want + 2e-3f, "panel.emission.value",
		    "image value 160 shows " + Rgb( one ) + ", emission " + std::to_string( want ) +
		        " (last failure: " + lab.pass->Stats().lastFailure + ")" );
		results.That( std::fabs( two[1] - 2.0f * one[1] ) <= 0.01f * two[1] + 2e-3f,
		    "panel.emission.scale", "scale 2 shows " + Rgb( two ) + " against " + Rgb( one ) );
		results.That( off[0] < 2e-3f && off[1] < 2e-3f && off[2] < 2e-3f, "panel.emission.term",
		    "with the emission term off the panel shows " + Rgb( off ) );
		results.That( back[0] == 0.0f && back[1] == 0.0f && back[2] == 0.0f &&
		                  lab.pass->Failures() == failuresBefore,
		    "panel.emission.back-not-drawn", "seen from behind: " + Rgb( back ) );
	}

	// --- One frame, one image -------------------------------------------------
	{
		// The next frame's list submitted before this frame's view records.
		const std::uint64_t host = lab.frame;
		Panel first = MakePanel( BoardList( lab, 140, false ), 0.5f );
		Panel second = MakePanel( BoardList( lab, 35, false ), 0.5f );
		NameTextures( lab, first );
		NameTextures( lab, second );
		const bool submitted =
		    lab.pass->Submit( host, first ) && lab.pass->Submit( host + 1, second );
		const bool refused = !lab.pass->Submit( host, second );
		results.That( refused, "panel.frame.one-list", "a second list for a frame was accepted" );
		PanelView view;
		view.hostFrame = host;
		view.panels = { first.id };
		const std::uint64_t rasterizedBefore = lab.pass->Stats().rasterized;
		CanvasImage image;
		if ( !submitted )
			return "the frame lists were refused: " + lab.pass->Stats().lastFailure;
		// Two views of the frame: one raster.
		if ( std::optional<std::string> why =
		         Render( lab, *lab.canvas, { { mid, view }, { mid, view } }, image ) )
			return why;
		float shown[3];
		Mean( image, kSize / 2 - 8, kSize / 2 - 40, kSize / 2 + 8, kSize / 2 - 24, shown );
		// The board (near 215 at the center) times the list's brightness,
		// in gamma: 140 shows near 0.18, the next frame's 35 near 0.013.
		const float want140 = world_panel::SrgbToLinear( 215.0f / 255.0f * 140.0f / 255.0f );
		results.That( std::fabs( shown[1] - want140 ) < 0.2f * want140, "panel.frame.own-list",
		    "frame " + std::to_string( host ) + " shows " + Rgb( shown ) +
		        " (its list is at 140, the next frame's at 35)" );
		results.That( lab.pass->Stats().rasterized == rasterizedBefore + 1,
		    "panel.frame.one-raster",
		    std::to_string( lab.pass->Stats().rasterized - rasterizedBefore ) +
		        " rasters for two views of one frame" );
		// A frame with no list fails by name.
		PanelView missing;
		missing.hostFrame = host + 100;
		missing.panels = { first.id };
		const std::uint64_t failuresBefore = lab.pass->Failures();
		CanvasImage ignored;
		if ( std::optional<std::string> why =
		         Render( lab, *lab.canvas, { { mid, missing } }, ignored ) )
			return why;
		results.That( lab.pass->Failures() == failuresBefore + 1 &&
		                  lab.pass->Stats().lastFailure.find( "no list" ) != std::string::npos,
		    "panel.frame.missing-fails", "last failure: " + lab.pass->Stats().lastFailure );
	}

	// --- Mips: a one-texel checker minified eight times -----------------------
	{
		const float density = 4.0f;
		const world_panel::Resolution r =
		    world_panel::ChooseResolution( kUnitsWide, kUnitsTall, density, none );
		// The checker's texels across the image: one per image texel.
		Quad q = MakeQuad(
		    0, 0, kUnitsWide, kUnitsTall, 2, 255, 255, 255, 255, world_panel::kBlendOpaque );
		q.s1 = float( r.width ) / 2.0f;
		q.t1 = 1.0f;
		Panel panel = MakePanel( { q }, density );
		NameTextures( lab, panel );
		const std::uint64_t host = lab.frame;
		if ( !lab.pass->Submit( host, panel ) )
			return "the checker list was refused: " + lab.pass->Stats().lastFailure;
		// 0.5 pixels per unit: 8 image texels per pixel across.
		const Camera far = Perspective( { 0.0f, -110.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } );
		const float perUnit =
		    world_panel::PixelsPerUnit( kPlacement, kUnitsWide, kUnitsTall, far.toClip, viewport );
		PanelView view;
		view.hostFrame = host;
		view.panels = { panel.id };
		CanvasImage image;
		if ( std::optional<std::string> why = Render( lab, *lab.canvas, { { far, view } }, image ) )
			return why;
		WriteImage( "panel-mips", image );
		float shown[3];
		Mean( image, kSize / 2 - 4, kSize / 2 - 4, kSize / 2 + 4, kSize / 2 + 4, shown );
		results.That( perUnit < 0.6f, "panel.mips.fixture-minifies",
		    std::to_string( perUnit ) + " pixels per unit at " + std::to_string( r.texelsPerUnit ) +
		        " texels per unit" );
		results.That( std::fabs( shown[1] - 0.5f ) <= 0.05f, "panel.mips.linear-mean",
		    "the minified checker shows " + Rgb( shown ) +
		        " (linear mean 0.5; a gamma-space chain gives 0.214)" );
	}

	// --- The light it casts: tiles against the GPU image ----------------------
	const Camera fillCamera = PanelFill( kPlacement );
	int across = 0, down = 0;
	world_panel::Tiles( kUnitsWide, kUnitsTall, across, down );
	results.That( across == 2 && down == 4, "panel.light.grid",
	    std::to_string( across ) + " x " + std::to_string( down ) + " tiles for a 1:2 panel" );
	auto tiles = [&]( const Panel &panel, std::vector<float> &cpu, std::vector<float> &gpu,
	                 const std::string &image ) -> std::optional<std::string>
	{
		const std::uint64_t host = lab.frame;
		if ( !lab.pass->Submit( host, panel ) )
			return "a light list was refused: " + lab.pass->Stats().lastFailure;
		PanelView view;
		view.hostFrame = host;
		view.panels = { panel.id };
		CanvasImage shown;
		if ( std::optional<std::string> why =
		         Render( lab, *lab.fill, { { fillCamera, view } }, shown ) )
			return why;
		WriteImage( image, shown );
		const world_panel::DrawListView list = { panel.unitsWide, panel.unitsTall,
		    panel.quads.data(), std::uint32_t( panel.quads.size() ), panel.textures.data(),
		    std::uint32_t( panel.textures.size() ) };
		std::vector<float> out( std::size_t( across * down ) * 3 );
		world_panel::TileRadiance(
		    list, panel.emissionScale, across, down, 16,
		    [&]( int key, float s, float t, float footprint, float rgba[4] )
		    {
			    return lab.textures->Sample( key, s, t, footprint, rgba );
		    },
		    reinterpret_cast<float ( * )[3]>( out.data() ) );
		cpu = out;
		gpu.assign( out.size(), 0.0f );
		const std::uint32_t tw = shown.width / across, th = shown.height / down;
		for ( int ty = 0; ty < down; ++ty )
			for ( int tx = 0; tx < across; ++tx )
				Mean( shown, tx * tw, ty * th, ( tx + 1 ) * tw, ( ty + 1 ) * th,
				    &gpu[( ty * across + tx ) * 3] );
		return std::nullopt;
	};
	auto agree =
	    [&]( const std::vector<float> &cpu, const std::vector<float> &gpu, std::string &detail )
	{
		bool ok = true;
		for ( std::size_t i = 0; i < cpu.size(); ++i )
		{
			if ( std::fabs( cpu[i] - gpu[i] ) > 0.02f * gpu[i] + 3e-3f )
			{
				ok = false;
				detail = "tile " + std::to_string( i / 3 ) + " channel " + std::to_string( i % 3 ) +
				         ": integrator " + std::to_string( cpu[i] ) + ", image " +
				         std::to_string( gpu[i] );
			}
		}
		return ok;
	};
	{
		// The flicker states: brightness 140, 35 and 70 with the overlay alpha
		// the sign gives each (255, 63, 127), dirty.
		const int states[3][2] = { { 140, 255 }, { 35, 63 }, { 70, 127 } };
		for ( const auto &state : states )
		{
			Panel panel = MakePanel( BoardList( lab, state[0], true, state[1] ), 1.0f, 1.0f );
			NameTextures( lab, panel );
			std::vector<float> cpu, gpu;
			if ( std::optional<std::string> why =
			         tiles( panel, cpu, gpu, "panel-light-" + std::to_string( state[0] ) ) )
				return why;
			std::string detail;
			results.That( agree( cpu, gpu, detail ),
			    "panel.light.tiles-match-image." + std::to_string( state[0] ), detail );
		}
		// Dirt: the lower tiles, under the dirtier part, cast less light; the
		// clean board's tiles differ only by its gradient.
		Panel dirty = MakePanel( BoardList( lab, 140, true ), 1.0f );
		Panel clean = MakePanel( BoardList( lab, 140, false ), 1.0f );
		NameTextures( lab, dirty );
		NameTextures( lab, clean );
		std::vector<float> dirtyCpu, dirtyGpu, cleanCpu, cleanGpu;
		if ( std::optional<std::string> why =
		         tiles( dirty, dirtyCpu, dirtyGpu, "panel-light-dirty" ) )
			return why;
		if ( std::optional<std::string> why =
		         tiles( clean, cleanCpu, cleanGpu, "panel-light-clean" ) )
			return why;
		auto rowMean = [&]( const std::vector<float> &v, int row )
		{
			float sum = 0.0f;
			for ( int tx = 0; tx < across; ++tx )
				sum += v[( row * across + tx ) * 3 + 1];
			return sum / across;
		};
		const float dirtyRatio = rowMean( dirtyCpu, down - 1 ) / rowMean( dirtyCpu, 0 );
		const float cleanRatio = rowMean( cleanCpu, down - 1 ) / rowMean( cleanCpu, 0 );
		const float dirtyTotal = rowMean( dirtyCpu, 0 ) + rowMean( dirtyCpu, down - 1 );
		const float cleanTotal = rowMean( cleanCpu, 0 ) + rowMean( cleanCpu, down - 1 );
		results.That( dirtyRatio < 0.8f * cleanRatio, "panel.light.dirt-darkens-where-dirty",
		    "bottom / top light: dirty " + std::to_string( dirtyRatio ) + ", clean " +
		        std::to_string( cleanRatio ) );
		results.That( dirtyTotal < cleanTotal, "panel.light.dirt-dims",
		    "dirty " + std::to_string( dirtyTotal ) + ", clean " + std::to_string( cleanTotal ) );
		results.That( std::fabs( cleanRatio - 1.0f ) < 0.15f, "panel.control.clean-is-even",
		    "the clean board's bottom / top light is " + std::to_string( cleanRatio ) );

		// Grime never adds light: in the flicker's dim state (35) the dirty
		// board casts no more than the clean one, tile by tile. Painted into
		// the lit image (the legacy sign's grey grime, the control), it adds.
		auto casts = [&]( bool dirt, bool emissiveGrime, std::vector<float> &cpu,
		                 std::vector<float> &gpu,
		                 const std::string &name ) -> std::optional<std::string>
		{
			Panel panel = MakePanel( BoardList( lab, 35, dirt, 127, emissiveGrime ), 1.0f );
			NameTextures( lab, panel );
			return tiles( panel, cpu, gpu, name );
		};
		std::vector<float> dimClean, dimCleanGpu, dimCoated, dimCoatedGpu, dimPainted,
		    dimPaintedGpu;
		if ( std::optional<std::string> why =
		         casts( false, false, dimClean, dimCleanGpu, "panel-dim-clean" ) )
			return why;
		if ( std::optional<std::string> why =
		         casts( true, false, dimCoated, dimCoatedGpu, "panel-dim-coated" ) )
			return why;
		if ( std::optional<std::string> why =
		         casts( true, true, dimPainted, dimPaintedGpu, "panel-dim-painted" ) )
			return why;
		auto never = [&]( const std::vector<float> &dirty, std::string &detail )
		{
			bool ok = true;
			for ( std::size_t i = 0; i < dirty.size(); ++i )
			{
				if ( dirty[i] > dimClean[i] * 1.001f + 1e-5f )
				{
					ok = false;
					detail = "tile " + std::to_string( i / 3 ) + ": dirty " +
					         std::to_string( dirty[i] ) + ", clean " +
					         std::to_string( dimClean[i] );
				}
			}
			return ok;
		};
		std::string coatedDetail, paintedDetail;
		results.That(
		    never( dimCoated, coatedDetail ), "panel.coating.never-adds-light", coatedDetail );
		results.That( !never( dimPainted, paintedDetail ), "panel.control.emissive-grime.rejected",
		    "grime painted into the lit image adds no light in the dim state" );
		std::string agreeDetail;
		results.That( agree( dimCoated, dimCoatedGpu, agreeDetail ),
		    "panel.light.tiles-match-image.coated", agreeDetail );
	}

	// --- The coating's scatter: grime over a dark print catches the light ------
	{
		// A lit board (brightness 255) with a dark vertical stroke 20 units
		// wide at x 190..210, under a uniform grime (alpha 0.5, mid grey), and
		// the same without grime. The stroke's center reads the grime's
		// scatter of the board around it, on the GPU as world_panel::EmissionAt
		// defines it; without grime it stays dark.
		auto strokeList = [&]( bool grime )
		{
			std::vector<Quad> quads;
			quads.push_back( MakeQuad( 0, 0, kUnitsWide, kUnitsTall, world_panel::kWhite, 255, 255,
			    255, 255, world_panel::kBlendOpaque ) );
			quads.push_back( MakeQuad( 190, 0, 210, kUnitsTall, world_panel::kWhite, 0, 0, 0, 255,
			    world_panel::kBlendOpaque ) );
			if ( grime )
			{
				Quad coat = MakeQuad(
				    0, 0, kUnitsWide, kUnitsTall, world_panel::kWhite, 128, 128, 128, 128 );
				coat.layer = world_panel::kLayerCoating;
				quads.push_back( coat );
			}
			return quads;
		};
		float strokeCenter[2][3], expected[3];
		for ( int grime = 0; grime < 2; ++grime )
		{
			Panel panel = MakePanel( strokeList( grime != 0 ), 1.0f );
			NameTextures( lab, panel );
			const std::uint64_t host = lab.frame;
			if ( !lab.pass->Submit( host, panel ) )
				return "the stroke list was refused: " + lab.pass->Stats().lastFailure;
			PanelView view;
			view.hostFrame = host;
			view.panels = { panel.id };
			CanvasImage shown;
			if ( std::optional<std::string> why =
			         Render( lab, *lab.fill, { { fillCamera, view } }, shown ) )
				return why;
			WriteImage( grime ? "panel-stroke-grime" : "panel-stroke-clean", shown );
			// x 200 of 400 units is canvas column 64 of 128; the middle row.
			Mean( shown, 63, shown.height / 2 - 4, 65, shown.height / 2 + 4, strokeCenter[grime] );
			if ( grime )
			{
				const world_panel::DrawListView list = { panel.unitsWide, panel.unitsTall,
				    panel.quads.data(), std::uint32_t( panel.quads.size() ), panel.textures.data(),
				    std::uint32_t( panel.textures.size() ) };
				const float step[2] = { 1.0f, 1.0f };
				auto sampler = [&]( int key, float u, float v, float footprint, float rgba[4] )
				{
					return lab.textures->Sample( key, u, v, footprint, rgba );
				};
				const world_panel::ScatterGrid field = world_panel::ScatterField( list, sampler );
				world_panel::EmissionAt(
				    list, field, 200.0f, kUnitsTall / 2, step, sampler, expected );
			}
		}
		results.That( strokeCenter[0][1] < 0.01f, "panel.control.clean-stroke-dark",
		    "the stroke without grime shows " + Rgb( strokeCenter[0] ) );
		results.That( strokeCenter[1][1] > 0.02f, "panel.coating.scatter-lights-dark-print",
		    "grime over the dark stroke shows " + Rgb( strokeCenter[1] ) );
		results.That( std::fabs( strokeCenter[1][1] - expected[1] ) <= 0.03f * expected[1] + 0.002f,
		    "panel.coating.scatter-matches-contract",
		    "the GPU shows " + Rgb( strokeCenter[1] ) + ", world_panel::EmissionAt " +
		        Rgb( expected ) );
	}

	// --- The coating's albedo: lit by the scene, not emitted -------------------
	{
		// A dark board (brightness 0) under the grime, with the emission term
		// off: what shows is the grime reflecting a white ambient cube, and the
		// clean top reflects nothing (the face has no albedo of its own).
		Panel panel = MakePanel( BoardList( lab, 0, true ), 1.0f );
		NameTextures( lab, panel );
		for ( auto &face : panel.ambientCube )
			face[0] = face[1] = face[2] = 1.0f;
		const std::uint64_t host = lab.frame;
		if ( !lab.pass->Submit( host, panel ) )
			return "the albedo list was refused: " + lab.pass->Stats().lastFailure;
		PanelView view;
		view.hostFrame = host;
		view.panels = { panel.id };
		view.debug.termsOff = shaderlib::kDebugTermEmission;
		CanvasImage shown;
		if ( std::optional<std::string> why =
		         Render( lab, *lab.fill, { { fillCamera, view } }, shown ) )
			return why;
		WriteImage( "panel-albedo", shown );
		float top[3], bottom[3];
		Mean( shown, 0, 0, shown.width, shown.height / 8, top );
		Mean( shown, 0, shown.height * 7 / 8, shown.width, shown.height, bottom );
		// The clean face reflects only its specular (a dielectric's F0 of 0.04
		// under the white cube); the grime adds its diffuse albedo.
		results.That( top[1] < 0.06f && bottom[1] - top[1] > 0.02f, "panel.coating.albedo-reflects",
		    "emission off, white ambient: grime " + Rgb( bottom ) + ", clean top " + Rgb( top ) );
	}

	// Release before the device goes.
	(void)lab.device->WaitIdle();
	lab.pass->ReleaseDevice( *lab.device );
	messages = lab.messages.load();
	return std::nullopt;
}

} // namespace

int RunPanelSuite( int argc, char **argv )
{
	static const Seeded kSeeded[] = {
	    { "gamma-mips", spirv::kPanelMipsGamma, "panel.mips" },
	    { "no-scatter", spirv::kPanelMipsNoScatter, "panel.coating.scatter" },
	    { "coating-ignored", spirv::kPanelMipsCoatingIgnored,
	        "panel.light.tiles-match-image.coated" },
	};
	return RunSeededSuite( argc, argv, "panel", kSeeded, RunChecks );
}

} // namespace render::lab
