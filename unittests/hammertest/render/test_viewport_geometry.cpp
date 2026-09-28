//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render.geometry (RFC 0002 hammer.adapters.render;
//			RFC 0016 "Editor viewports"; contract
//			render_adapter.viewport-geometry.v1): the pure half of the editor's
//			viewports on the render core, with no device.
//
//			G1 a snapshot's solids become two triangles per quad face and one
//			   edge per face side; point entities become shaded marker boxes;
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
//			G7 each projection check rejects a seeded wrong view.
//
//=============================================================================//

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
using hammer::render_adapter::BuildSceneGeometry;
using hammer::render_adapter::LineVertex;
using hammer::render_adapter::SceneGeometry;
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

// The color of the first face triangle whose vertices all have z == 'z' and
// x within [x0, x1].
std::optional<Rgb> FaceColor( const SceneGeometry &g, double z, double x0, double x1 )
{
	for ( std::size_t i = 0; i + 2 < g.faces.size(); i += 3 )
	{
		bool match = true;
		for ( std::size_t k = 0; k < 3; ++k )
		{
			const float *p = g.faces[i + k].position;
			match = match && std::fabs( p[2] - z ) < 1e-3 && p[0] >= x0 - 1e-3 && p[0] <= x1 + 1e-3;
		}
		if ( match )
			return ColorOf( g.faces[i] );
	}
	return std::nullopt;
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
	checks.Equal( g.faces.size(), std::size_t( 24 * 3 + 36 ), "G1.solid-triangles-and-a-marker-box" );
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
