//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.geometry (RFC 0002 hammer.adapters.render;
//			RFC 0016 "Editor viewports"; contract
//			render_adapter.viewport-geometry.v1): the pure half of the editor's
//			viewports on the render core, with no device.
//
//			G1 a snapshot's solids become two triangles per quad face and one
//			   edge per face side; point entities become shaded marker boxes;
//			   with no texture sizes everything is one untextured batch;
//			G2 selection is data: the selected solid's faces carry the
//			   selection fill and its edges the selection edge color, others
//			   their own;
//			G3 the fixed two-light shading: an upward face is brighter than a
//			   downward one, and a face's color equals the shading rule
//			   computed here independently;
//			G4 every overlay kind maps to its items (world items untested in
//			   every view, screen rects as outlines, handles filled, labels
//			   dropped);
//			G5 grid lines become full-width or full-height screen lines on
//			   pixel centers;
//			G6 ViewFor(Camera2D) projects like Camera2D::WorldToScreen for
//			   every 2D kind, and ViewFor(Camera3D) like Camera3D::
//			   WorldToScreen, with depth in [0, 1] growing with distance;
//			G7 each projection check rejects a seeded wrong view;
//			G8 the textured preview: faces whose material has a texture size
//			   go into that material's batch with the shading alone as color
//			   (the selection tint over the selected solid) and uv from the
//			   side's texture axes over the size, restated here; markers stay
//			   untextured;
//			G9 a material without a size (or a zero size) stays in the
//			   untextured batch, as with no sizes at all;
//			G10 BuildMipChain: an 8x4 image has 4 levels, 8x4 down to 1x1;
//			   a black and white 2x2 averages in linear light to the byte
//			   restated here (188, where averaging the encoded bytes gives
//			   128) and its alpha to the rounded mean; an odd axis's last
//			   box takes three texels; an image whose bytes do not match
//			   its size has no chain;
//			G11 a solid's fill comes from its id: without its neighbor (a
//			   chunk restaged alone) it keeps its color; ChunkOf is the id
//			   over 64.
//
//=============================================================================//

#include "hammer/adapters/render/material_textures.h"
#include "hammer/adapters/render/scene_geometry.h"
#include "testing/checks.h"
#include "viewport_fixture.h"

#include <cmath>
#include <random>

namespace
{

using namespace hammertest::viewport_render;
using hammer::render_adapter::AppendGrid;
using hammer::render_adapter::AppendOverlay;
using hammer::render_adapter::BuildMipChain;
using hammer::render_adapter::BuildSceneGeometry;
using hammer::render_adapter::ChunkOf;
using hammer::render_adapter::MaterialImage;
using hammer::render_adapter::LineVertex;
using hammer::render_adapter::SceneGeometry;
using hammer::render_adapter::TextureSize;
using hammer::render_adapter::UnlitVertex;
using hammer::render_adapter::ViewFor;
using hammer::viewport::ViewKind;
namespace lines = render::pass::lines;

struct Rgb
{
	int r = 0;
	int g = 0;
	int b = 0;
	friend bool operator==( const Rgb &, const Rgb & ) = default;
};

Rgb ColorOf( const LineVertex &v )
{
	return { int( v.color & 0xFF ), int( ( v.color >> 8 ) & 0xFF ), int( ( v.color >> 16 ) & 0xFF ) };
}

Rgb ColorOf( const UnlitVertex &v )
{
	return { v.color[0], v.color[1], v.color[2] };
}

// The batch of 'material' ("" is the untextured one), or an empty list.
const std::vector<UnlitVertex> &Batch( const SceneGeometry &g, const std::string &material )
{
	static const std::vector<UnlitVertex> none;
	for ( const auto &batch : g.faces )
	{
		if ( batch.material == material )
			return batch.vertices;
	}
	return none;
}

// The shading rule, restated independently of the adapter.
Rgb Shaded( double r, double g, double b, Vec3d n )
{
	auto unit = []( Vec3d v )
	{
		const double l = std::sqrt( v.x * v.x + v.y * v.y + v.z * v.z );
		return Vec3d( v.x / l, v.y / l, v.z / l );
	};
	const Vec3d l1 = unit( Vec3d( 0.4, 0.6, 0.8 ) );
	const Vec3d l2 = unit( Vec3d( -0.5, -0.3, 0.4 ) );
	auto dot = []( Vec3d a, Vec3d b )
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	};
	double d = 0.35 + 0.55 * std::max( dot( n, l1 ), 0.0 ) + 0.25 * std::max( dot( n, l2 ), 0.0 );
	d = std::min( std::max( d, 0.0 ), 1.0 );
	auto byte = []( double v )
	{
		return int( std::lround( std::min( std::max( v, 0.0 ), 1.0 ) * 255.0 ) );
	};
	return { byte( r * d ), byte( g * d ), byte( b * d ) };
}

// The color of the first face triangle of a batch whose vertices all have
// z == 'z' and x within [x0, x1].
std::optional<Rgb> FaceColor(
    const std::vector<UnlitVertex> &faces, double z, double x0, double x1 )
{
	for ( std::size_t i = 0; i + 2 < faces.size(); i += 3 )
	{
		bool match = true;
		for ( std::size_t k = 0; k < 3; ++k )
		{
			const float *p = faces[i + k].position;
			match = match && std::fabs( p[2] - z ) < 1e-3 && p[0] >= x0 - 1e-3 && p[0] <= x1 + 1e-3;
		}
		if ( match )
			return ColorOf( faces[i] );
	}
	return std::nullopt;
}

std::optional<Rgb> FaceColor( const SceneGeometry &g, double z, double x0, double x1 )
{
	return FaceColor( Batch( g, "" ), z, x0, x1 );
}

// Whether every vertex of 'faces' has the uv of some face of the snapshot
// that holds its position: texels = dot(p, axis) / scale + shift, over the size.
bool UvsFollowTheAxes( const std::vector<UnlitVertex> &faces,
    const hammer::viewport::RenderSnapshot &snapshot, TextureSize size )
{
	for ( const UnlitVertex &v : faces )
	{
		const Vec3d p( v.position[0], v.position[1], v.position[2] );
		bool matched = false;
		for ( const auto &solid : snapshot.solids )
		{
			for ( const auto &face : solid.faces )
			{
				bool holds = false;
				for ( const Vec3d &q : face.vertices )
					holds =
					    holds || ( std::fabs( q.x - p.x ) < 1e-3 && std::fabs( q.y - p.y ) < 1e-3 &&
					                 std::fabs( q.z - p.z ) < 1e-3 );
				if ( !holds )
					continue;
				auto texel = [&]( const hammer::scene::TextureAxis &axis )
				{
					return ( p.x * axis.axis.x + p.y * axis.axis.y + p.z * axis.axis.z ) /
					           axis.scale +
					       axis.shift;
				};
				matched = matched ||
				          ( std::fabs( texel( face.uAxis ) / size.width - v.uv[0] ) < 1e-4 &&
				              std::fabs( texel( face.vAxis ) / size.height - v.uv[1] ) < 1e-4 );
			}
		}
		if ( !matched )
			return false;
	}
	return !faces.empty();
}

bool EdgeColorsIn( const SceneGeometry &g, double x0, double x1, Rgb color )
{
	bool any = false;
	for ( std::size_t i = 0; i + 1 < g.edges.size(); i += 2 )
	{
		const float *a = g.edges[i].position;
		const float *b = g.edges[i + 1].position;
		if ( a[0] >= x0 - 1e-3 && a[0] <= x1 + 1e-3 && b[0] >= x0 - 1e-3 && b[0] <= x1 + 1e-3 &&
		     a[2] < 200 && b[2] < 200 )
		{
			any = true;
			if ( !( ColorOf( g.edges[i] ) == color ) )
				return false;
		}
	}
	return any;
}

// The sRGB byte of the mean of 'bytes' in linear light, restated.
int LinearMean( std::initializer_list<int> bytes )
{
	double sum = 0.0;
	for ( int b : bytes )
	{
		const double c = b / 255.0;
		sum += c <= 0.04045 ? c / 12.92 : std::pow( ( c + 0.055 ) / 1.055, 2.4 );
	}
	const double l = sum / double( bytes.size() );
	const double c = l <= 0.0031308 ? l * 12.92 : 1.055 * std::pow( l, 1.0 / 2.4 ) - 0.055;
	return int( std::lround( c * 255.0 ) );
}

MaterialImage Image( std::uint32_t width, std::uint32_t height, std::vector<std::uint8_t> rgba )
{
	MaterialImage image;
	image.width = width;
	image.height = height;
	image.rgba = std::move( rgba );
	return image;
}

// Pixel of a world point through a pass view.
std::optional<hammer::viewport::ScreenPoint> Project( const lines::LinesView &view, Vec3d p )
{
	const render::math::float4 c = render::math::Transform(
	    view.worldToClip, { float( p.x ), float( p.y ), float( p.z ), 1.0f } );
	if ( c.w <= 0.0f )
		return std::nullopt;
	return hammer::viewport::ScreenPoint{ ( c.x / c.w + 1.0 ) * 0.5 * view.width,
	    ( 1.0 - c.y / c.w ) * 0.5 * view.height };
}

bool Agrees2D( const lines::LinesView &view, const hammer::viewport::Camera2D &camera )
{
	std::mt19937 rng( 7 );
	std::uniform_real_distribution<double> coord( -2000.0, 2000.0 );
	for ( int i = 0; i < 200; ++i )
	{
		const Vec3d p( coord( rng ), coord( rng ), coord( rng ) );
		const auto got = Project( view, p );
		const hammer::viewport::ScreenPoint want = camera.WorldToScreen( p );
		if ( !got || std::fabs( got->x - want.x ) > 1e-2 || std::fabs( got->y - want.y ) > 1e-2 )
			return false;
	}
	return true;
}

bool Agrees3D( const lines::LinesView &view, const hammer::viewport::Camera3D &camera )
{
	std::mt19937 rng( 11 );
	std::uniform_real_distribution<double> coord( -500.0, 500.0 );
	int compared = 0;
	for ( int i = 0; i < 400; ++i )
	{
		const Vec3d p( coord( rng ), coord( rng ), coord( rng ) );
		const auto want = camera.WorldToScreen( p );
		if ( !want )
			continue;
		const auto got = Project( view, p );
		if ( !got || std::fabs( got->x - want->x ) > 1e-2 || std::fabs( got->y - want->y ) > 1e-2 )
			return false;
		++compared;
	}
	return compared > 100;
}

double Depth( const lines::LinesView &view, Vec3d p )
{
	const render::math::float4 c = render::math::Transform(
	    view.worldToClip, { float( p.x ), float( p.y ), float( p.z ), 1.0f } );
	return c.z / c.w;
}

} // namespace

int main()
{
	testing::Checks checks;
	Document d;
	Build( d );
	const SceneGeometry g = BuildSceneGeometry( Snapshot( d ) );

	// G1.
	checks.Equal( g.triangles, 24u, "G1.two-triangles-per-quad-face" );
	checks.That( g.faces.size() == 1 && g.faces[0].material.empty(),
	    "G1.no-sizes-means-one-untextured-batch" );
	checks.Equal(
	    Batch( g, "" ).size(), std::size_t( 24 * 3 + 36 ), "G1.solid-triangles-and-a-marker-box" );
	checks.Equal(
	    g.edges.size(), std::size_t( 12 * 4 * 2 + 24 ), "G1.one-edge-per-face-side-and-marker-edges" );

	// G2, G3.
	const auto leftTop = FaceColor( g, 128, -128, 0 );
	const auto rightTop = FaceColor( g, 64, 64, 192 );
	const auto rightBottom = FaceColor( g, 0, 64, 192 );
	checks.That( leftTop && *leftTop == Shaded( 1.00, 0.62, 0.28, Vec3d( 0, 0, 1 ) ),
	    "G2.the-selected-solid-carries-the-shaded-selection-fill" );
	checks.That( rightTop && !( *rightTop == *leftTop ), "G2.an-unselected-solid-keeps-its-fill" );
	checks.That( EdgeColorsIn( g, -128, 0, Rgb{ 255, 148, 38 } ),
	    "G2.the-selected-solid-has-selection-edges" );
	checks.That( EdgeColorsIn( g, 64, 192, Rgb{ 128, 133, 148 } ),
	    "G2.a-world-solid-has-the-plain-edge-color" );
	checks.That( rightTop && rightBottom &&
	                 rightTop->r + rightTop->g + rightTop->b >
	                     rightBottom->r + rightBottom->g + rightBottom->b,
	    "G3.an-upward-face-is-brighter-than-a-downward-one" );

	// G8, G9.
	{
		const std::string material = "DEV/DEV_MEASUREGENERIC01B";
		const TextureSize size{ 64, 32 };
		const auto snapshot = Snapshot( d );
		const SceneGeometry textured = BuildSceneGeometry( snapshot,
		    [&]( const std::string &name ) -> std::optional<TextureSize>
		    {
			    return name == material ? std::optional<TextureSize>( size ) : std::nullopt;
		    } );
		checks.That( textured.faces.size() == 2 && textured.faces[0].material.empty() &&
		                 textured.faces[1].material == material,
		    "G8.textured-faces-get-their-material-batch" );
		checks.Equal( Batch( textured, material ).size(), std::size_t( 24 * 3 ),
		    "G8.every-solid-face-is-textured" );
		checks.Equal(
		    Batch( textured, "" ).size(), std::size_t( 36 ), "G8.the-marker-box-stays-untextured" );
		const auto texturedTop = FaceColor( Batch( textured, material ), 64, 64, 192 );
		const auto selectedTop = FaceColor( Batch( textured, material ), 128, -128, 0 );
		checks.That( texturedTop && *texturedTop == Shaded( 1, 1, 1, Vec3d( 0, 0, 1 ) ),
		    "G8.a-textured-face-is-colored-by-the-shading-alone" );
		checks.That( selectedTop && *selectedTop == Shaded( 1.00, 0.62, 0.28, Vec3d( 0, 0, 1 ) ),
		    "G8.the-selection-tints-a-textured-face" );
		checks.That( UvsFollowTheAxes( Batch( textured, material ), snapshot, size ),
		    "G8.uv-follows-the-texture-axes-over-the-size" );
		checks.That(
		    !UvsFollowTheAxes( Batch( textured, material ), snapshot, TextureSize{ 32, 32 } ),
		    "G8.a-wrong-size-is-rejected" );
		const SceneGeometry missing = BuildSceneGeometry( snapshot,
		    []( const std::string & ) -> std::optional<TextureSize>
		    {
			    return std::nullopt;
		    } );
		const SceneGeometry zero = BuildSceneGeometry( snapshot,
		    []( const std::string & ) -> std::optional<TextureSize>
		    {
			    return TextureSize{ 0, 16 };
		    } );
		checks.That( missing.faces.size() == 1 && zero.faces.size() == 1 &&
		                 Batch( missing, "" ).size() == Batch( g, "" ).size() &&
		                 Batch( zero, "" ).size() == Batch( g, "" ).size(),
		    "G9.a-material-without-a-size-stays-untextured" );
	}

	// G10.
	{
		const auto chain =
		    BuildMipChain( Image( 8, 4, std::vector<std::uint8_t>( 8 * 4 * 4, 77 ) ) );
		bool sizes = chain.size() == 4;
		const std::uint32_t want[4][2] = { { 8, 4 }, { 4, 2 }, { 2, 1 }, { 1, 1 } };
		for ( std::size_t m = 0; sizes && m < 4; ++m )
			sizes = chain[m].width == want[m][0] && chain[m].height == want[m][1] &&
			        chain[m].rgba.size() == std::size_t( want[m][0] ) * want[m][1] * 4;
		checks.That( sizes, "G10.an-8x4-image-has-four-levels-down-to-1x1" );
		checks.That( sizes && chain[3].rgba == std::vector<std::uint8_t>{ 77, 77, 77, 77 },
		    "G10.a-uniform-image-stays-uniform" );
		// Black, white / white, black; alpha 255, 255 / 0, 0.
		const auto checker = BuildMipChain(
		    Image( 2, 2, { 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0 } ) );
		const int mid = LinearMean( { 0, 255, 255, 0 } );
		checks.That( checker.size() == 2 && checker[1].rgba[0] == mid &&
		                 checker[1].rgba[1] == mid && checker[1].rgba[2] == mid && mid == 188,
		    "G10.a-black-and-white-box-averages-in-linear-light" );
		checks.That( checker.size() == 2 && checker[1].rgba[0] != 128,
		    "G10.the-encoded-byte-average-is-rejected" );
		checks.That(
		    checker.size() == 2 && checker[1].rgba[3] == 128, "G10.alpha-averages-as-stored" );
		// 3x1: 255, 0, 0 in red; the 1x1 level takes all three.
		const auto odd =
		    BuildMipChain( Image( 3, 1, { 255, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255 } ) );
		checks.That( odd.size() == 2 && odd[1].width == 1 && odd[1].height == 1 &&
		                 odd[1].rgba[0] == LinearMean( { 255, 0, 0 } ) && odd[1].rgba[0] == 156,
		    "G10.an-odd-axis-last-box-takes-three-texels" );
		checks.That( BuildMipChain( Image( 2, 2, std::vector<std::uint8_t>( 15, 0 ) ) ).empty() &&
		                 BuildMipChain( Image( 0, 0, {} ) ).empty(),
		    "G10.an-inconsistent-image-has-no-chain" );
	}

	// G11.
	{
		auto snapshot = Snapshot( d, false );
		const auto full = FaceColor( BuildSceneGeometry( snapshot ), 64, 64, 192 );
		std::erase_if( snapshot.solids,
		    [&]( const hammer::viewport::SolidDraw &solid )
		    {
			    return solid.id == d.left;
		    } );
		const auto alone = FaceColor( BuildSceneGeometry( snapshot ), 64, 64, 192 );
		checks.That( full && alone && *full == *alone, "G11.a-solid-keeps-its-fill-alone" );
		checks.That( ChunkOf( ObjectId{ 63 } ) == 0 && ChunkOf( ObjectId{ 64 } ) == 1 &&
		                 ChunkOf( ObjectId{ 200 } ) == 3,
		    "G11.a-chunk-is-the-id-over-64" );
	}

	// G4.
	{
		hammer::tools::OverlayList overlay;
		overlay.Box( Vec3d( 0, 0, 0 ), Vec3d( 1, 1, 1 ), hammer::tools::OverlayRole::Pending );
		overlay.Line( Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ), hammer::tools::OverlayRole::Clip );
		overlay.Polygon( { Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ), Vec3d( 1, 1, 0 ), Vec3d( 0, 1, 0 ) },
		    hammer::tools::OverlayRole::Hover );
		overlay.Rect( { 1, 1 }, { 5, 5 }, hammer::tools::OverlayRole::Selection );
		overlay.Handle( { 10, 10 }, 3, hammer::tools::HandleShape::Square,
		    hammer::tools::OverlayRole::Handle );
		overlay.Handle( { 20, 20 }, 3, hammer::tools::HandleShape::Circle,
		    hammer::tools::OverlayRole::HandleHot );
		overlay.Label( { 5, 5 }, "size", hammer::tools::OverlayRole::Handle );
		lines::LineList list;
		AppendOverlay( overlay, list );
		checks.Equal( list.Vertices( { lines::Space::kWorld, false }, lines::Topology::kLines ).size(),
		    std::size_t( 24 + 2 + 8 ), "G4.world-items-are-untested-lines" );
		checks.That( list.Vertices( { lines::Space::kWorld, true }, lines::Topology::kLines ).empty(),
		    "G4.no-world-item-is-depth-tested" );
		checks.Equal( list.Vertices( { lines::Space::kScreen, false }, lines::Topology::kLines ).size(),
		    std::size_t( 8 ), "G4.a-screen-rect-is-an-outline" );
		checks.Equal( list.Vertices( { lines::Space::kScreen, false }, lines::Topology::kFilled ).size(),
		    std::size_t( 6 + 12 * 3 ), "G4.handles-are-filled-and-labels-dropped" );
	}

	// G5.
	{
		const hammer::viewport::GridLine grid[] = {
		    { hammer::viewport::GridLineKind::Minor, hammer::viewport::GridLineOrientation::Vertical,
		        0.0, 10.2 },
		    { hammer::viewport::GridLineKind::Axis, hammer::viewport::GridLineOrientation::Horizontal,
		        0.0, 20.9 } };
		lines::LineList list;
		AppendGrid( grid, 256, 192, list );
		const auto v = list.Vertices( { lines::Space::kScreen, false }, lines::Topology::kLines );
		checks.That( v.size() == 4 && v[0].position[0] == 10.5f && v[0].position[1] == 0.0f &&
		                 v[1].position[1] == 192.0f && v[2].position[1] == 20.5f &&
		                 v[3].position[0] == 256.0f,
		    "G5.grid-lines-span-the-view-on-pixel-centers" );
	}

	// G6, G7.
	bool all2D = true;
	for ( ViewKind kind : { ViewKind::Top, ViewKind::Front, ViewKind::Side } )
	{
		hammer::viewport::Camera2D camera = TopCamera();
		camera.SetKind( kind );
		all2D = all2D && Agrees2D( ViewFor( camera ), camera );
	}
	checks.That( all2D, "G6.the-2d-view-projects-like-its-camera" );
	const hammer::viewport::Camera3D eye = EyeCamera();
	const lines::LinesView view3D = ViewFor( eye, Snapshot( d ).bounds );
	checks.That( Agrees3D( view3D, eye ), "G6.the-3d-view-projects-like-its-camera" );
	const double nearDepth = Depth( view3D, Vec3d( 32, -500, 280 ) );
	const double farDepth = Depth( view3D, Vec3d( 32, 200, 0 ) );
	checks.That( nearDepth >= 0.0 && nearDepth < farDepth && farDepth <= 1.0,
	    "G6.depth-grows-with-distance-within-0-1" );
	{
		hammer::viewport::Camera2D top = TopCamera();
		hammer::viewport::Camera2D front = TopCamera();
		front.SetKind( ViewKind::Front );
		checks.That( !Agrees2D( ViewFor( front ), top ), "G7.a-wrong-2d-view-is-rejected" );
		hammer::viewport::Camera3D turned = eye;
		turned.Look( 5.0, 0.0 );
		checks.That( !Agrees3D( ViewFor( turned, Snapshot( d ).bounds ), eye ),
		    "G7.a-wrong-3d-view-is-rejected" );
	}
	return checks.Report();
}
