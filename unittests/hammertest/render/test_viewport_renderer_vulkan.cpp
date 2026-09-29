//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.viewport (RFC 0002 hammer.adapters.render;
//			RFC 0016 "Editor viewports"; linux-native-vulkan-gpu): the
//			editor's ViewportRenderer on render.device.vulkan with real
//			pixels, judged by relations rather than goldens (RFC 0016 decision
//			"oracles"):
//
//			R1 3D: the pixel under each box's top-face center (projected by
//			   the view's own Camera3D) has that face's color as the geometry
//			   builds it, within one level (the unlit family decodes vertex
//			   colors as gamma 2.2 and the renderer re-encodes the display
//			   colors for it): the selected box the shaded selection fill, the
//			   other its own fill;
//			R2 2D: the pixel on a box edge (projected by the Camera2D) has the
//			   edge color (selection orange for the selected box, the plain
//			   edge color otherwise); a grid line shows where no geometry is;
//			   the tool overlay's pending box lands where its corners project;
//			R3 a restage after the selection moves changes the old selected
//			   box's edges to the plain color; the same inputs give the same
//			   frame;
//			R4 textured: with a material source whose base texture is red on
//			   its left half and blue on its right, the unselected box's top
//			   face shows red where the side's texture axes put u in the left
//			   half and blue where they put it in the right, each the texel
//			   times the face's shading (sRGB-correct, within two levels); a
//			   source with no texture leaves the R1 colors;
//			R5 exported frames (where the device exports images): an external
//			   frame's memory, mapped through its description, equals the
//			   read-back frame of the same view; a returned lease's image is
//			   drawn into again (no new image), an unreturned one is not, and
//			   a resize replaces the free images;
//			R6 mipmaps: a one-texel black and white checker seen from afar
//			   shows the linear-light mid gray (188) times the shading at
//			   every sampled point of the face, within four levels, where
//			   mip 0 alone would alias to either extreme; seen from close,
//			   the same face shows the checker (over 96 levels of contrast,
//			   the control);
//			R7 blending: a $translucent pane (blue, alpha 128) over an
//			   opaque wall (red) shows their blend in linear light, where an
//			   opaque pane of the same texture shows only the pane; an
//			   $alphatest grate shows the wall through its transparent half
//			   and itself on its opaque half;
//			R8 per-chunk restaging: after one solid of a three-chunk scene
//			   moves, the restaged renderer (one chunk rebuilt) draws the 3D
//			   and top views byte-identical to a fresh renderer of the moved
//			   scene, and the frame differs from the one before the move;
//			the Khronos validation layer reports no message.
//
//=============================================================================//

#include "hammer/adapters/render/viewport_renderer.h"
#include "render/device/vulkan/provider.h"
#include "testing/checks.h"
#include "viewport_fixture.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>

#include <linux/dma-buf.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstdio>
#include <cstdlib>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::BuildSceneGeometry;
using hammer::render_adapter::IMaterialTextures;
using hammer::render_adapter::MaterialImage;
using hammer::render_adapter::SceneGeometry;
using hammer::render_adapter::ViewportRenderer;
using hammer::render_adapter::ViewPixels;
using hammer::render_adapter::ViewRequest;
using hammer::viewport::ViewKind;

struct Rgb
{
	int r = 0;
	int g = 0;
	int b = 0;
	friend bool operator==( const Rgb &, const Rgb & ) = default;
};

Rgb At( const ViewPixels &pixels, int x, int y )
{
	if ( x < 0 || y < 0 || x >= int( pixels.width ) || y >= int( pixels.height ) ||
	     pixels.rgba.size() != std::size_t( pixels.width ) * pixels.height * 4 )
		return { -1, -1, -1 };
	const std::size_t i = ( std::size_t( y ) * pixels.width + std::size_t( x ) ) * 4;
	return { pixels.rgba[i], pixels.rgba[i + 1], pixels.rgba[i + 2] };
}

// Whether the pixel at 'p' or one of its 8 neighbors has 'color' (lines
// exactly on a pixel boundary may land on either side).
bool Near( const ViewPixels &pixels, double x, double y, Rgb color )
{
	for ( int dy = -1; dy <= 1; ++dy )
		for ( int dx = -1; dx <= 1; ++dx )
			if ( At( pixels, int( std::floor( x ) ) + dx, int( std::floor( y ) ) + dy ) == color )
				return true;
	return false;
}

bool Within( Rgb a, Rgb b, int levels )
{
	return std::abs( a.r - b.r ) <= levels && std::abs( a.g - b.g ) <= levels &&
	       std::abs( a.b - b.b ) <= levels;
}

std::optional<Rgb> TopFaceColor( const SceneGeometry &g, double z, double x0, double x1 )
{
	for ( const auto &batch : g.faces )
	{
		const auto &faces = batch.vertices;
		for ( std::size_t i = 0; i + 2 < faces.size(); i += 3 )
		{
			bool match = true;
			for ( std::size_t k = 0; k < 3; ++k )
			{
				const float *p = faces[i + k].position;
				match =
				    match && std::fabs( p[2] - z ) < 1e-3 && p[0] >= x0 - 1e-3 && p[0] <= x1 + 1e-3;
			}
			if ( match )
				return Rgb{ faces[i].color[0], faces[i].color[1], faces[i].color[2] };
		}
	}
	return std::nullopt;
}

double ToLinear( double display )
{
	return display <= 0.04045 ? display / 12.92 : std::pow( ( display + 0.055 ) / 1.055, 2.4 );
}

int ToDisplay( double linear )
{
	const double v =
	    linear <= 0.0031308 ? linear * 12.92 : 1.055 * std::pow( linear, 1.0 / 2.4 ) - 0.055;
	return int( std::lround( std::clamp( v, 0.0, 1.0 ) * 255.0 ) );
}

// texel x shade in linear light, as the unlit family draws a textured face.
Rgb Modulated( Rgb texel, Rgb shade )
{
	auto channel = []( int t, int s )
	{
		return ToDisplay( ToLinear( t / 255.0 ) * ToLinear( s / 255.0 ) );
	};
	return {
	    channel( texel.r, shade.r ), channel( texel.g, shade.g ), channel( texel.b, shade.b ) };
}

constexpr Rgb kRed{ 220, 40, 30 };
constexpr Rgb kBlue{ 30, 60, 220 };
constexpr std::uint32_t kTextureWidth = 1024;

// Red on the left half of a 1024x2 texture, blue on the right, for the
// fixture's material (or nothing at all when 'none').
class HalfTextures final : public IMaterialTextures
{
public:
	explicit HalfTextures( bool none ) : m_None( none ) {}
	std::optional<MaterialImage> BaseTexture( const std::string & ) override
	{
		if ( m_None )
			return std::nullopt;
		MaterialImage image;
		image.width = kTextureWidth;
		image.height = 2;
		for ( std::uint32_t y = 0; y < image.height; ++y )
		{
			for ( std::uint32_t x = 0; x < image.width; ++x )
			{
				const Rgb c = x < kTextureWidth / 2 ? kRed : kBlue;
				image.rgba.insert( image.rgba.end(),
				    { std::uint8_t( c.r ), std::uint8_t( c.g ), std::uint8_t( c.b ), 255 } );
			}
		}
		return image;
	}

private:
	bool m_None = false;
};

// The fraction of the texture width at x on the unselected box's top face.
double UFraction( const hammer::viewport::RenderSnapshot &snapshot, double x, double y )
{
	for ( const auto &solid : snapshot.solids )
	{
		for ( const auto &face : solid.faces )
		{
			if ( face.normal.z > 0.9 && face.vertices[0].z > 63 && face.vertices[0].z < 65 )
			{
				const auto &a = face.uAxis;
				const double u =
				    ( x * a.axis.x + y * a.axis.y + 64 * a.axis.z ) / a.scale + a.shift;
				const double f = u / kTextureWidth;
				return f - std::floor( f );
			}
		}
	}
	return -1.0;
}

// Uniform or patterned textures by material, with surface parameters.
class SurfaceTextures final : public IMaterialTextures
{
public:
	std::optional<MaterialImage> BaseTexture( const std::string &material ) override
	{
		MaterialImage image;
		image.width = image.height = 64;
		for ( std::uint32_t y = 0; y < 64; ++y )
		{
			for ( std::uint32_t x = 0; x < 64; ++x )
			{
				Rgb c = kRed;
				int alpha = 255;
				if ( material == "CHECKER" )
					c = ( x + y ) % 2 ? Rgb{ 255, 255, 255 } : Rgb{ 0, 0, 0 };
				else if ( material == "GLASS" || material == "PANE" )
				{
					c = kBlue;
					alpha = 128;
				}
				else if ( material == "GRATE" )
				{
					c = kGreen;
					alpha = x < 32 ? 0 : 255;
				}
				image.rgba.insert(
				    image.rgba.end(), { std::uint8_t( c.r ), std::uint8_t( c.g ),
				                          std::uint8_t( c.b ), std::uint8_t( alpha ) } );
			}
		}
		image.surface.translucent = material == "GLASS";
		image.surface.alphaTest = material == "GRATE";
		return image;
	}
	static constexpr Rgb kGreen{ 40, 200, 60 };
};

// A one-face solid: a 128 x 128 quad over the right box's top at height z,
// u = x / 2 - 32 texels (the left half of a 64-texel texture over x 64..128).
hammer::viewport::SolidDraw Pane( std::uint64_t id, const std::string &material, double z )
{
	hammer::viewport::SolidDraw solid;
	solid.id = ObjectId{ id };
	hammer::viewport::FaceDraw face;
	face.material = material;
	face.vertices = {
	    Vec3d( 64, -64, z ), Vec3d( 192, -64, z ), Vec3d( 192, 64, z ), Vec3d( 64, 64, z ) };
	face.normal = Vec3d( 0, 0, 1 );
	face.uAxis = { Vec3d( 1, 0, 0 ), -32.0, 2.0 };
	face.vAxis = { Vec3d( 0, -1, 0 ), 0.0, 2.0 };
	solid.faces.push_back( face );
	solid.bounds = { Vec3d( 64, -64, z ), Vec3d( 192, 64, z ) };
	return solid;
}

// The linear-light blend of 'over' with alpha 'a' onto 'under', displayed.
Rgb Blend( Rgb over, Rgb under, double a )
{
	auto channel = [a]( int o, int u )
	{
		return ToDisplay( a * ToLinear( o / 255.0 ) + ( 1.0 - a ) * ToLinear( u / 255.0 ) );
	};
	return { channel( over.r, under.r ), channel( over.g, under.g ), channel( over.b, under.b ) };
}

std::optional<ViewPixels> Frame(
    foundation::Expected<std::unique_ptr<ViewportRenderer>, hammer::render_adapter::ViewportStatus>
        &renderer,
    const hammer::viewport::RenderSnapshot &scene, std::uint64_t key, const ViewRequest &request )
{
	if ( !renderer || !renderer.Value()->SetScene( scene, key ) )
		return std::nullopt;
	auto frame = renderer.Value()->RenderAndWait( request );
	if ( !frame )
		return std::nullopt;
	return std::move( frame ).Value();
}

constexpr Rgb kSelectedEdge{ 255, 148, 38 };
constexpr Rgb kEdge{ 128, 133, 148 };
constexpr Rgb kPending{ 255, 224, 77 };

} // namespace

int main()
{
	testing::Checks checks;
	namespace vulkan = render::device::vulkan;
	const bool layer = vulkan::ValidationLayerAvailable();
	std::atomic<std::uint64_t> messages{ 0 };
	vulkan::VulkanAdapterOptions options;
	options.validation = layer;
	options.validationCounter = &messages;
	if ( const char *adapter = std::getenv( "RENDER_VK_ADAPTER" ) )
		options.adapterIndex = std::atoi( adapter );
	{
		auto created = vulkan::Create( options );
		if ( !checks.That( created.HasValue(), "device.a-vulkan-device-is-created" ) )
			return checks.Report();
		std::unique_ptr<render::device::IRenderDevice2> device = std::move( created ).Value();
		Document d;
		Build( d );
		auto made = ViewportRenderer::Create( *device );
		if ( !checks.That( made.HasValue(), "setup.renderer" ) )
			return checks.Report();
		ViewportRenderer &renderer = *made.Value();
		const hammer::viewport::RenderSnapshot selected = Snapshot( d );
		const SceneGeometry geometry = BuildSceneGeometry( selected );
		checks.That( renderer.SetScene( selected, 1 ).HasValue(), "setup.scene" );

		// R1.
		const hammer::viewport::Camera3D eye = EyeCamera();
		ViewRequest request3D;
		request3D.kind = ViewKind::Camera3D;
		request3D.camera3D = &eye;
		request3D.pixelWidth = 256;
		request3D.pixelHeight = 192;
		auto frame3D = renderer.RenderAndWait( request3D );
		checks.That( frame3D.HasValue(), "R1.the-3d-view-renders" );
		if ( frame3D )
		{
			const auto leftCenter = eye.WorldToScreen( Vec3d( -64, 0, 128 ) );
			const auto rightCenter = eye.WorldToScreen( Vec3d( 128, 0, 64 ) );
			const auto leftColor = TopFaceColor( geometry, 128, -128, 0 );
			const auto rightColor = TopFaceColor( geometry, 64, 64, 192 );
			checks.That(
			    leftCenter && leftColor &&
			        Within( At( frame3D.Value(), int( leftCenter->x ), int( leftCenter->y ) ),
			            *leftColor, 1 ),
			    "R1.the-selected-top-face-shows-its-selection-fill" );
			checks.That(
			    rightCenter && rightColor &&
			        Within( At( frame3D.Value(), int( rightCenter->x ), int( rightCenter->y ) ),
			            *rightColor, 1 ),
			    "R1.the-other-top-face-shows-its-own-fill" );
		}

		// R2.
		const hammer::viewport::Camera2D top = TopCamera();
		ViewRequest request2D;
		request2D.kind = ViewKind::Top;
		request2D.camera2D = &top;
		request2D.grid = { { hammer::viewport::GridLineKind::Axis,
		    hammer::viewport::GridLineOrientation::Horizontal, 0.0, 170.0 } };
		request2D.overlay.Box(
		    Vec3d( 240, 40, 0 ), Vec3d( 256, 56, 16 ), hammer::tools::OverlayRole::Pending );
		request2D.pixelWidth = 256;
		request2D.pixelHeight = 192;
		auto frame2D = renderer.RenderAndWait( request2D );
		checks.That( frame2D.HasValue(), "R2.the-2d-view-renders" );
		if ( frame2D )
		{
			const hammer::viewport::ScreenPoint leftEdge = top.WorldToScreen( Vec3d( -128, 0, 0 ) );
			const hammer::viewport::ScreenPoint rightEdge = top.WorldToScreen( Vec3d( 192, 0, 0 ) );
			const hammer::viewport::ScreenPoint pending = top.WorldToScreen( Vec3d( 240, 48, 0 ) );
			checks.That( Near( frame2D.Value(), leftEdge.x, leftEdge.y, kSelectedEdge ),
			    "R2.the-selected-box-edge-is-orange" );
			checks.That( Near( frame2D.Value(), rightEdge.x, rightEdge.y, kEdge ),
			    "R2.the-other-box-edge-is-plain" );
			checks.That( Near( frame2D.Value(), 20, 170.5, Rgb{ 115, 51, 51 } ),
			    "R2.a-grid-line-shows-where-no-geometry-is" );
			checks.That( Near( frame2D.Value(), pending.x, pending.y, kPending ),
			    "R2.the-pending-box-lands-at-its-projected-corner-edge" );

			// R3.
			auto again = renderer.RenderAndWait( request2D );
			checks.That( again && again.Value().rgba == frame2D.Value().rgba,
			    "R3.the-same-inputs-give-the-same-frame" );
			checks.That( renderer.SetScene( Snapshot( d, false ), 2 ).HasValue(), "R3.restaged" );
			auto moved = renderer.RenderAndWait( request2D );
			checks.That( moved && Near( moved.Value(), leftEdge.x, leftEdge.y, kEdge ) &&
			                 !Near( moved.Value(), leftEdge.x, leftEdge.y, kSelectedEdge ),
			    "R3.a-restage-follows-the-selection" );
		}
		made.Value().reset();

		// R4.
		{
			const hammer::viewport::RenderSnapshot plain = Snapshot( d, false );
			HalfTextures half( false );
			HalfTextures none( true );
			auto textured = ViewportRenderer::Create( *device, &half );
			auto untextured = ViewportRenderer::Create( *device, &none );
			checks.That( textured && untextured && textured.Value()->SetScene( plain, 1 ) &&
			                 untextured.Value()->SetScene( plain, 1 ) &&
			                 textured.Value()->Scene().texturedBatches == 1,
			    "R4.textured-scenes-are-staged" );
			// Points on the top face at y = 0 whose u lies well inside each half.
			std::optional<double> redX;
			std::optional<double> blueX;
			for ( double x = 70; x <= 186; x += 2 )
			{
				const double f = UFraction( plain, x, 0 );
				if ( !redX && f > 0.1 && f < 0.4 )
					redX = x;
				if ( !blueX && f > 0.6 && f < 0.9 )
					blueX = x;
			}
			checks.That( redX && blueX, "R4.the-face-spans-both-halves" );
			auto frame =
			    textured ? textured.Value()->RenderAndWait( request3D )
			             : foundation::Expected<ViewPixels, hammer::render_adapter::ViewportStatus>(
			                   foundation::MakeUnexpected(
			                       hammer::render_adapter::ViewportStatus::kDevice ) );
			const auto shade =
			    TopFaceColor( BuildSceneGeometry( plain,
			                      []( const std::string & )
			                      {
				                      return std::optional<hammer::render_adapter::TextureSize>(
				                          { kTextureWidth, 2 } );
			                      } ),
			        64, 64, 192 );
			if ( frame && redX && blueX && shade )
			{
				const auto red = eye.WorldToScreen( Vec3d( *redX, 0, 64 ) );
				const auto blue = eye.WorldToScreen( Vec3d( *blueX, 0, 64 ) );
				const Rgb gotRed = red ? At( frame.Value(), int( red->x ), int( red->y ) ) : Rgb{};
				const Rgb gotBlue =
				    blue ? At( frame.Value(), int( blue->x ), int( blue->y ) ) : Rgb{};
				checks.That( Within( gotRed, Modulated( kRed, *shade ), 2 ),
				    "R4.the-left-half-of-u-is-the-red-texel-times-the-shading" );
				checks.That( Within( gotBlue, Modulated( kBlue, *shade ), 2 ),
				    "R4.the-right-half-of-u-is-the-blue-texel-times-the-shading" );
			}
			else
			{
				checks.That( false, "R4.the-textured-frame-renders" );
			}
			auto flat =
			    untextured
			        ? untextured.Value()->RenderAndWait( request3D )
			        : foundation::Expected<ViewPixels, hammer::render_adapter::ViewportStatus>(
			              foundation::MakeUnexpected(
			                  hammer::render_adapter::ViewportStatus::kDevice ) );
			const auto rightCenter = eye.WorldToScreen( Vec3d( 128, 0, 64 ) );
			const auto rightColor = TopFaceColor( BuildSceneGeometry( plain ), 64, 64, 192 );
			checks.That(
			    flat && rightCenter && rightColor &&
			        Within( At( flat.Value(), int( rightCenter->x ), int( rightCenter->y ) ),
			            *rightColor, 1 ),
			    "R4.a-source-without-the-texture-leaves-the-flat-colors" );
		}
		// R6.
		{
			const hammer::viewport::RenderSnapshot plain = Snapshot( d, false );
			hammer::viewport::RenderSnapshot checker = plain;
			for ( auto &solid : checker.solids )
				for ( auto &face : solid.faces )
					face.material = "CHECKER";
			SurfaceTextures textures;
			auto renderer = ViewportRenderer::Create( *device, &textures );
			const auto shade = TopFaceColor(
			    BuildSceneGeometry( checker,
			        []( const std::string & )
			        {
				        return std::optional<hammer::render_adapter::TextureSize>( { 64, 64 } );
			        } ),
			    64, 64, 192 );
			const Rgb mid = shade ? Modulated( { 188, 188, 188 }, *shade ) : Rgb{};
			const auto far = Frame( renderer, checker, 1, request3D );
			int sampled = 0;
			int worst = 0;
			if ( far )
			{
				for ( double x = 80; x <= 176; x += 16 )
					for ( double y = -48; y <= 48; y += 16 )
						if ( const auto p = eye.WorldToScreen( Vec3d( x, y, 64 ) ) )
						{
							const Rgb got = At( *far, int( p->x ), int( p->y ) );
							worst = std::max( { worst, std::abs( got.r - mid.r ),
							    std::abs( got.g - mid.g ), std::abs( got.b - mid.b ) } );
							++sampled;
						}
			}
			checks.That( shade && sampled == 49 && worst <= 4,
			    "R6.a-far-checker-averages-to-the-linear-mid-gray" );
			std::printf(
			    "R6: far checker, worst deviation %d levels over %d points\n", worst, sampled );
			hammer::viewport::Camera3D close;
			close.SetViewport( 256, 192 );
			close.SetPosition( Vec3d( 128, -4, 70 ) ); // several pixels per texel
			close.LookAt( Vec3d( 128, 0, 64 ) );
			ViewRequest near = request3D;
			near.camera3D = &close;
			auto nearFrame =
			    renderer ? renderer.Value()->RenderAndWait( near )
			             : foundation::Expected<ViewPixels, hammer::render_adapter::ViewportStatus>(
			                   foundation::MakeUnexpected(
			                       hammer::render_adapter::ViewportStatus::kDevice ) );
			int darkest = 255;
			int brightest = 0;
			if ( nearFrame )
			{
				for ( int y = 64; y < 128; ++y )
					for ( int x = 96; x < 160; ++x )
					{
						const Rgb got = At( nearFrame.Value(), x, y );
						darkest = std::min( darkest, got.g );
						brightest = std::max( brightest, got.g );
					}
			}
			std::printf( "R6: close checker, green from %d to %d (shade %d)\n", darkest, brightest,
			    shade ? shade->g : -1 );
			// Bilinear between one-texel cells never reaches pure black at a
			// few pixels per texel, so the control is the contrast.
			checks.That( nearFrame && shade && brightest - darkest >= 96,
			    "R6.the-close-checker-shows-both-extremes" );
		}

		// R7.
		{
			const hammer::viewport::RenderSnapshot plain = Snapshot( d, false );
			SurfaceTextures textures;
			auto renderer = ViewportRenderer::Create( *device, &textures );
			const auto shade = TopFaceColor(
			    BuildSceneGeometry( plain,
			        []( const std::string & )
			        {
				        return std::optional<hammer::render_adapter::TextureSize>( { 64, 64 } );
			        } ),
			    64, 64, 192 );
			auto with = [&]( const std::string &material )
			{
				hammer::viewport::RenderSnapshot scene = plain;
				scene.solids.push_back( Pane( 500, material, 68 ) );
				return scene;
			};
			const Rgb wall = shade ? Modulated( kRed, *shade ) : Rgb{};
			const Rgb pane = shade ? Modulated( kBlue, *shade ) : Rgb{};
			const Rgb grate = shade ? Modulated( SurfaceTextures::kGreen, *shade ) : Rgb{};
			const Rgb blend = Blend( pane, wall, 128.0 / 255.0 );
			const auto glassAt = eye.WorldToScreen( Vec3d( 100, -20, 68 ) );
			const auto holeAt = eye.WorldToScreen( Vec3d( 90, -20, 68 ) );
			const auto solidAt = eye.WorldToScreen( Vec3d( 160, -20, 68 ) );
			const auto glass = Frame( renderer, with( "GLASS" ), 1, request3D );
			const auto opaque = Frame( renderer, with( "PANE" ), 2, request3D );
			const auto grated = Frame( renderer, with( "GRATE" ), 3, request3D );
			const Rgb gotGlass =
			    glass && glassAt ? At( *glass, int( glassAt->x ), int( glassAt->y ) ) : Rgb{};
			const Rgb gotOpaque =
			    opaque && glassAt ? At( *opaque, int( glassAt->x ), int( glassAt->y ) ) : Rgb{};
			std::printf( "R7: glass %d %d %d (want %d %d %d), opaque %d %d %d (want %d %d %d)\n",
			    gotGlass.r, gotGlass.g, gotGlass.b, blend.r, blend.g, blend.b, gotOpaque.r,
			    gotOpaque.g, gotOpaque.b, pane.r, pane.g, pane.b );
			checks.That( shade && !Within( blend, pane, 10 ) && !Within( blend, wall, 10 ),
			    "R7.the-blend-differs-from-either-layer" );
			checks.That( shade && Within( gotGlass, blend, 3 ),
			    "R7.a-translucent-pane-blends-over-the-wall" );
			checks.That(
			    shade && Within( gotOpaque, pane, 2 ), "R7.an-opaque-pane-shows-only-itself" );
			checks.That(
			    shade && grated && holeAt && solidAt &&
			        Within( At( *grated, int( holeAt->x ), int( holeAt->y ) ), wall, 2 ) &&
			        Within( At( *grated, int( solidAt->x ), int( solidAt->y ) ), grate, 2 ),
			    "R7.an-alpha-tested-grate-shows-the-wall-through-its-hole" );
		}

		// R8.
		{
			const hammer::viewport::RenderSnapshot spread = Spread( d, 70 );
			const hammer::viewport::RenderSnapshot moved =
			    Moved( spread, ObjectId{ 128 }, Vec3d( 16, 0, 0 ) );
			auto restaged = ViewportRenderer::Create( *device );
			auto fresh = ViewportRenderer::Create( *device );
			const auto before3D = Frame( restaged, spread, 1, request3D );
			const auto before2D = Frame( restaged, spread, 1, request2D );
			const auto after3D = Frame( restaged, moved, 2, request3D );
			const auto after2D = Frame( restaged, moved, 2, request2D );
			const bool partial = restaged && restaged.Value()->Scene().stagedChunks == 1 &&
			                     restaged.Value()->Scene().chunks == 3;
			const auto fresh3D = Frame( fresh, moved, 1, request3D );
			const auto fresh2D = Frame( fresh, moved, 1, request2D );
			checks.That( partial && fresh && fresh.Value()->Scene().stagedChunks == 3,
			    "R8.the-edit-restages-one-chunk-of-three" );
			checks.That( after3D && fresh3D && after3D->rgba == fresh3D->rgba && after2D &&
			                 fresh2D && after2D->rgba == fresh2D->rgba,
			    "R8.the-restaged-frames-equal-a-fresh-renderers" );
			checks.That( before3D && before2D && after3D && after2D &&
			                 before3D->rgba != after3D->rgba && before2D->rgba != after2D->rgba,
			    "R8.the-move-changes-both-frames" );
		}

		// R5.
		if ( device->ExternalImages() )
		{
			auto exporting = ViewportRenderer::Create( *device );
			ViewportRenderer *r = exporting ? exporting.Value().get() : nullptr;
			const bool staged = r && r->SetScene( Snapshot( d ), 1 ).HasValue() && r->CanExport();
			ViewRequest external = request3D;
			external.external = true;
			auto readback = staged ? r->RenderAndWait( request3D )
			                       : foundation::Expected<ViewPixels,
			                             hammer::render_adapter::ViewportStatus>(
			                             foundation::MakeUnexpected(
			                                 hammer::render_adapter::ViewportStatus::kDevice ) );
			auto first = staged ? r->RenderAndWait( external ) : readback;
			bool equal = readback && first && first.Value().external && first.Value().rgba.empty();
			if ( equal )
			{
				const hammer::render_adapter::ExternalFrame &frame = *first.Value().external;
				const std::size_t rows = request3D.pixelHeight;
				const std::size_t row = std::size_t( request3D.pixelWidth ) * 4;
				const std::size_t length = frame.offset + frame.stride * ( rows - 1 ) + row;
				void *mapped = ::mmap( nullptr, length, PROT_READ, MAP_SHARED, int( frame.handle ), 0 );
				equal = mapped != MAP_FAILED;
				if ( equal )
				{
					dma_buf_sync sync{ DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
					(void)::ioctl( int( frame.handle ), DMA_BUF_IOCTL_SYNC, &sync );
					const auto *base = static_cast<const std::uint8_t *>( mapped ) + frame.offset;
					for ( std::size_t y = 0; equal && y < rows; ++y )
						equal = std::memcmp( base + y * frame.stride,
						            readback.Value().rgba.data() + y * row, row ) == 0;
					sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
					(void)::ioctl( int( frame.handle ), DMA_BUF_IOCTL_SYNC, &sync );
					::munmap( mapped, length );
				}
			}
			checks.That( equal, "R5.an-external-frame-equals-the-read-back-frame" );
			if ( first && first.Value().external )
			{
				auto second = r->RenderAndWait( external ); // the first is still leased
				checks.That( second && second.Value().external && r->ExternalImageCount() == 2,
				    "R5.a-leased-image-is-not-drawn-into" );
				r->ReturnFrame( first.Value().external->lease );
				if ( second && second.Value().external )
					r->ReturnFrame( second.Value().external->lease );
				auto third = r->RenderAndWait( external );
				checks.That( third && third.Value().external && r->ExternalImageCount() == 2,
				    "R5.a-returned-image-is-drawn-into-again" );
				if ( third && third.Value().external )
					r->ReturnFrame( third.Value().external->lease );
				ViewRequest resized = external;
				resized.pixelWidth = 128;
				resized.pixelHeight = 64;
				hammer::viewport::Camera3D small = EyeCamera( 128, 64 );
				resized.camera3D = &small;
				auto fourth = r->RenderAndWait( resized );
				checks.That( fourth && fourth.Value().external && r->ExternalImageCount() == 1,
				    "R5.a-resize-replaces-the-free-images" );
			}
			exporting.Value().reset();
		}
		else
		{
			std::printf( "SKIP R5: the device does not export images\n" );
		}
		(void)device->WaitIdle();
	}
	if ( layer )
		checks.Equal( messages.load(), std::uint64_t( 0 ), "validation.no-messages" );
	else
		std::printf( "SKIP validation: the Khronos validation layer is not installed\n" );
	return checks.Report();
}
