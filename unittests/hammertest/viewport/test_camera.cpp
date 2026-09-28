//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.viewport cameras (RFC 0002, R08 domain logic; contract
//			viewport.camera.v1): the 2D view axis mapping (AxesOf), world <->
//			screen round trips, zoom about
//			the cursor, panning and framing; the 3D camera's Source angle
//			convention, pixel rays, projection, fly/look/orbit and framing, and
//			the GTK shell's existing camera expectations re-expressed in this
//			convention. Negative checks: rejected input, degenerate viewports,
//			behind-camera points, and seeded wrong operations the oracles must
//			detect.
//
//=============================================================================//

#include "hammer/viewport/camera.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <cmath>
#include <limits>
#include <numbers>

using namespace hammer::viewport;
using hammer::scene::Box;
using mapgeometry::Vec3d;

namespace
{

constexpr double kEps = 1.0e-9;
const double kNaN = std::numeric_limits<double>::quiet_NaN();

bool Near( const Vec3d &a, const Vec3d &b, double eps = 1.0e-9 )
{
	return mapgeometry::NearlyEqual( a, b, eps );
}

// Oracle: the world point under (px, py) stays under it after 'op'.
template <typename Op> bool KeepsPointUnderCursor( Camera2D camera, double px, double py, Op op )
{
	const PlanePoint before = camera.ScreenToPlane( px, py );
	op( camera );
	const ScreenPoint after = camera.PlaneToScreen( before );
	return std::fabs( after.x - px ) < 1.0e-6 && std::fabs( after.y - py ) < 1.0e-6;
}

// Oracle: every corner of 'box' projects inside the viewport.
bool BoxOnScreen( const Camera3D &camera, const Box &box )
{
	for ( int i = 0; i < 8; ++i )
	{
		const Vec3d corner( ( i & 1 ) ? box.maxs.x : box.mins.x,
		    ( i & 2 ) ? box.maxs.y : box.mins.y, ( i & 4 ) ? box.maxs.z : box.mins.z );
		const std::optional<ScreenPoint> p = camera.WorldToScreen( corner );
		if ( !p || p->x < -1.0e-6 || p->y < -1.0e-6 || p->x > camera.Width() + 1.0e-6 ||
		     p->y > camera.Height() + 1.0e-6 )
		{
			return false;
		}
	}
	return true;
}

void AxisMapping( testing::Checks &checks )
{
	// Top = X/Y, Front = X/Z, Side = Y/Z; u right, v up.
	const ViewAxes top = AxesOf( ViewKind::Top );
	const ViewAxes front = AxesOf( ViewKind::Front );
	const ViewAxes side = AxesOf( ViewKind::Side );
	const ViewAxes cam = AxesOf( ViewKind::Camera3D );
	checks.That( top.u == 0 && top.v == 1 && top.depth == 2, "Top = X/Y, free Z" );
	checks.That( front.u == 0 && front.v == 2 && front.depth == 1, "Front = X/Z, free Y" );
	checks.That( side.u == 1 && side.v == 2 && side.depth == 0, "Side = Y/Z, free X" );
	checks.That( cam.u == 0 && cam.v == 1 && cam.depth == 2, "Camera3D reports Top's mapping" );
	checks.That( ViewDirection( ViewKind::Top ) == Vec3d( 0, 0, -1 ), "Top looks down -Z" );
	checks.That( ViewDirection( ViewKind::Front ) == Vec3d( 0, 1, 0 ), "Front looks along +Y" );
	checks.That( ViewDirection( ViewKind::Side ) == Vec3d( -1, 0, 0 ), "Side looks along -X" );
}

void Camera2DBasics( testing::Checks &checks )
{
	Camera2D camera;
	checks.That( camera.Kind() == ViewKind::Top, "default kind is Top" );
	checks.Near( camera.Zoom(), Camera2D::kDefaultZoom, 0.0, "default zoom" );
	checks.That( !camera.HasArea(), "default viewport is empty" );

	checks.That( !camera.SetKind( ViewKind::Camera3D ), "Camera3D is not a 2D kind (negative)" );
	checks.That( camera.Kind() == ViewKind::Top, "rejected kind leaves the camera unchanged" );
	checks.That(
	    camera.SetKind( ViewKind::Front ) && camera.Kind() == ViewKind::Front, "set Front" );

	checks.That( !camera.SetZoom( 0.0 ), "zero zoom rejected (negative)" );
	checks.That( !camera.SetZoom( -2.0 ), "negative zoom rejected (negative)" );
	checks.That( !camera.SetZoom( kNaN ), "NaN zoom rejected (negative)" );
	checks.Near( camera.Zoom(), Camera2D::kDefaultZoom, 0.0, "rejected zooms leave the zoom" );
	checks.That( camera.SetZoom( 1000.0 ), "large zoom accepted" );
	checks.Near( camera.Zoom(), Camera2D::kMaxZoom, 0.0, "zoom clamps to the maximum" );
	camera.SetZoom( 1.0e-9 );
	checks.Near( camera.Zoom(), Camera2D::kMinZoom, 0.0, "zoom clamps to the minimum" );
	checks.That( !camera.SetCenter( { kNaN, 0.0 } ), "NaN centre rejected (negative)" );

	camera.SetViewport( -5, 40 );
	checks.That( camera.Width() == 0 && camera.Height() == 40 && !camera.HasArea(),
	    "negative viewport width stored as zero" );
}

void Camera2DProjection( testing::Checks &checks )
{
	Camera2D front;
	front.SetKind( ViewKind::Front );
	front.SetViewport( 200, 100 );
	front.SetZoom( 1.0 );
	const ScreenPoint p = front.WorldToScreen( Vec3d( 10, 5, 20 ) );
	checks.Near( p.x, 110.0, kEps, "Front: X maps to screen right" );
	checks.Near( p.y, 30.0, kEps, "Front: Z maps to screen up (y down)" );
	checks.Near( front.DepthOf( Vec3d( 10, 5, 20 ) ), 5.0, 0.0, "Front depth is Y" );

	const Vec3d samples[] = {
	    Vec3d( 0, 0, 0 ), Vec3d( 13.5, -7.25, 1024 ), Vec3d( -16384, 16384, -3 ) };
	for ( ViewKind kind : { ViewKind::Top, ViewKind::Front, ViewKind::Side } )
	{
		Camera2D camera;
		camera.SetKind( kind );
		camera.SetViewport( 640, 480 );
		camera.SetZoom( 0.37 );
		camera.SetCenter( { 100.0, -250.0 } );
		for ( const Vec3d &world : samples )
		{
			const ScreenPoint s = camera.WorldToScreen( world );
			const Vec3d back = camera.ScreenToWorld( s.x, s.y, camera.DepthOf( world ) );
			checks.That( Near( back, world, 1.0e-9 ), "world -> screen -> world round trip" );
		}
		const ScreenPoint centre = camera.PlaneToScreen( camera.Center() );
		checks.That( centre.x == 320.0 && centre.y == 240.0, "centre maps to the viewport centre" );
		const ScreenPoint low = camera.PlaneToScreen( { 0.0, 0.0 } );
		const ScreenPoint high = camera.PlaneToScreen( { 0.0, 10.0 } );
		checks.That( high.y < low.y, "larger v is higher on screen" );
	}

	Camera2D top;
	top.SetViewport( 800, 600 );
	top.SetZoom( 2.0 );
	top.SetCenter( { 10.0, 20.0 } );
	const PlaneRect rect = top.VisibleRect();
	checks.That(
	    rect.min.u == -190.0 && rect.max.u == 210.0 && rect.min.v == -130.0 && rect.max.v == 170.0,
	    "visible rectangle" );
}

void Camera2DNavigation( testing::Checks &checks )
{
	Camera2D camera;
	camera.SetViewport( 800, 600 );
	camera.SetZoom( 0.5 );
	camera.SetCenter( { 32.0, -64.0 } );

	checks.That( KeepsPointUnderCursor( camera, 37.0, 81.0,
	                 []( Camera2D &c )
	                 {
		                 c.ZoomAt( 37.0, 81.0, 2.0 );
	                 } ),
	    "ZoomAt keeps the world point under the cursor" );
	checks.That( KeepsPointUnderCursor( camera, 700.0, 10.0,
	                 []( Camera2D &c )
	                 {
		                 c.ZoomAt( 700.0, 10.0, 0.125 );
	                 } ),
	    "zooming out keeps the point too" );
	{
		Camera2D nearMax = camera;
		nearMax.SetZoom( 200.0 );
		checks.That( KeepsPointUnderCursor( nearMax, 123.0, 456.0,
		                 []( Camera2D &c )
		                 {
			                 c.ZoomAt( 123.0, 456.0, 4.0 );
		                 } ),
		    "a clamped zoom still keeps the point fixed" );
	}
	{
		Camera2D copy = camera;
		copy.ZoomAt( 1.0, 1.0, 2.0 );
		checks.Near( copy.Zoom(), 1.0, kEps, "ZoomAt multiplies the zoom" );
		Camera2D rejected = camera;
		checks.That( !rejected.ZoomAt( 1.0, 1.0, 0.0 ) && !rejected.ZoomAt( 1.0, 1.0, -2.0 ) &&
		                 !rejected.ZoomAt( kNaN, 1.0, 2.0 ),
		    "invalid zoom factors and pixels rejected (negative)" );
		checks.That( rejected.Zoom() == camera.Zoom() && rejected.Center() == camera.Center(),
		    "rejected ZoomAt changes nothing" );
	}
	// Seeded mutant: zooming without re-anchoring must fail the oracle.
	checks.That( !KeepsPointUnderCursor( camera, 37.0, 81.0,
	                 []( Camera2D &c )
	                 {
		                 c.SetZoom( c.Zoom() * 2.0 );
	                 } ),
	    "oracle detects an un-anchored zoom (seeded)" );

	{
		Camera2D panned = camera;
		const Vec3d world( 100, 50, 0 );
		const ScreenPoint before = panned.WorldToScreen( world );
		checks.That( panned.PanPixels( 10.0, -4.0 ), "pan accepted" );
		const ScreenPoint after = panned.WorldToScreen( world );
		checks.Near( after.x - before.x, 10.0, 1.0e-9, "content follows a right drag" );
		checks.Near( after.y - before.y, -4.0, 1.0e-9, "content follows an upward drag" );
		checks.That( !panned.PanPixels( kNaN, 0.0 ), "NaN pan rejected (negative)" );
	}

	// GTK shell (Renderer::PixelToWorld / DragBy / ZoomAtPixel) formulas.
	{
		const double panU = camera.Center().u;
		const double panV = camera.Center().v;
		const double ppu = camera.Zoom();
		const PlanePoint p = camera.ScreenToPlane( 250.0, 120.0 );
		checks.Near( p.u, panU + ( 250.0 - 400.0 ) / ppu, kEps, "GTK PixelToWorld u" );
		checks.Near( p.v, panV - ( 120.0 - 300.0 ) / ppu, kEps, "GTK PixelToWorld v" );
		Camera2D dragged = camera;
		dragged.PanPixels( 12.0, 7.0 );
		checks.Near( dragged.Center().u, panU - 12.0 / ppu, kEps, "GTK DragBy panU" );
		checks.Near( dragged.Center().v, panV + 7.0 / ppu, kEps, "GTK DragBy panV" );
	}
}

void Camera2DFrame( testing::Checks &checks )
{
	Camera2D camera;
	camera.SetViewport( 800, 600 );
	const Box box{ Vec3d( 0, 0, 0 ), Vec3d( 512, 256, 64 ) };
	checks.That( camera.Frame( box, 20.0 ), "frame accepted" );
	checks.Near( camera.Zoom(), 760.0 / 512.0, kEps, "frame picks the limiting axis zoom" );
	checks.That( camera.Center() == PlanePoint{ 256.0, 128.0 }, "frame centres the box" );
	const ScreenPoint lo = camera.WorldToScreen( box.mins );
	const ScreenPoint hi = camera.WorldToScreen( box.maxs );
	checks.Near( lo.x, 20.0, 1.0e-9, "limiting axis touches the margin" );
	checks.That( hi.y >= 20.0 && lo.y <= 580.0, "other axis fits inside the margin" );

	Camera2D point = camera;
	const double zoomBefore = point.Zoom();
	checks.That( point.Frame( { Vec3d( 5, 6, 7 ), Vec3d( 5, 6, 7 ) }, 0.0 ), "point box frames" );
	checks.That( point.Zoom() == zoomBefore && point.Center() == PlanePoint{ 5.0, 6.0 },
	    "a point box keeps the zoom and centres" );

	Camera2D bad = camera;
	checks.That( !bad.Frame( { Vec3d( 1, 0, 0 ), Vec3d( 0, 1, 1 ) }, 0.0 ),
	    "inverted box rejected (negative)" );
	checks.That( !bad.Frame( box, 400.0 ), "margin larger than the viewport rejected (negative)" );
	checks.That( !bad.Frame( box, -1.0 ), "negative margin rejected (negative)" );
	Camera2D empty;
	checks.That( !empty.Frame( box, 0.0 ), "frame without a viewport rejected (negative)" );
	checks.That( bad.Zoom() == camera.Zoom() && bad.Center() == camera.Center(),
	    "rejected frames change nothing" );
}

void Camera3DAngles( testing::Checks &checks )
{
	Camera3D camera;
	checks.That( Near( camera.Forward(), Vec3d( 1, 0, 0 ) ), "yaw 0 looks along +X" );
	checks.That( Near( camera.Right(), Vec3d( 0, -1, 0 ) ), "right of +X is -Y" );
	checks.That( Near( camera.Up(), Vec3d( 0, 0, 1 ) ), "up is +Z" );
	camera.SetAngles( 90.0, 0.0 );
	checks.That( Near( camera.Forward(), Vec3d( 0, 1, 0 ) ), "yaw 90 looks along +Y" );
	camera.SetAngles( 0.0, 45.0 );
	checks.That( camera.Forward().z < 0.0, "positive pitch looks down (Source)" );

	camera.SetAngles( 270.0, 120.0 );
	checks.Near( camera.Yaw(), -90.0, kEps, "yaw normalized to (-180, 180]" );
	checks.Near( camera.Pitch(), Camera3D::kMaxPitch, 0.0, "pitch clamps at +89" );
	camera.SetAngles( -180.0, -500.0 );
	checks.Near( camera.Yaw(), 180.0, kEps, "-180 normalizes to 180" );
	checks.Near( camera.Pitch(), -Camera3D::kMaxPitch, 0.0, "pitch clamps at -89" );
	checks.That( !camera.SetAngles( kNaN, 0.0 ), "NaN angle rejected (negative)" );
	checks.That( !camera.SetFov( kNaN ), "NaN fov rejected (negative)" );
	camera.SetFov( 500.0 );
	checks.Near( camera.Fov(), Camera3D::kMaxFov, 0.0, "fov clamps" );

	camera.SetAngles( 33.0, -21.0 );
	const Vec3d f = camera.Forward();
	const Vec3d r = camera.Right();
	const Vec3d u = camera.Up();
	checks.That( std::fabs( mapgeometry::Dot( f, r ) ) < kEps &&
	                 std::fabs( mapgeometry::Dot( f, u ) ) < kEps &&
	                 std::fabs( mapgeometry::Dot( r, u ) ) < kEps &&
	                 std::fabs( mapgeometry::Length( u ) - 1.0 ) < kEps,
	    "basis is orthonormal" );
	checks.That( Near( mapgeometry::Cross( f, r ), -u ) || Near( mapgeometry::Cross( r, f ), u ),
	    "up = right x forward" );
}

void Camera3DProjection( testing::Checks &checks )
{
	Camera3D camera;
	camera.SetPosition( Vec3d( 10, 20, 30 ) );
	camera.SetAngles( 40.0, 15.0 );
	checks.That( !camera.RayThroughPixel( 1.0, 1.0 ), "no ray without a viewport (negative)" );
	checks.That( !camera.WorldToScreen( Vec3d( 100, 100, 0 ) ),
	    "no projection without a viewport (negative)" );
	camera.SetViewport( 800, 600 );

	const std::optional<Ray> centre = camera.RayThroughPixel( 400.0, 300.0 );
	checks.That( centre && Near( centre->direction, camera.Forward() ), "centre ray is Forward" );
	checks.That( centre && centre->origin == camera.Position(), "ray starts at the eye" );
	checks.That( !camera.RayThroughPixel( kNaN, 0.0 ), "NaN pixel rejected (negative)" );

	const std::optional<Ray> corner = camera.RayThroughPixel( 123.0, 45.0 );
	checks.That( corner && std::fabs( mapgeometry::Length( corner->direction ) - 1.0 ) < kEps,
	    "ray direction is unit length" );
	const Vec3d along = corner->origin + corner->direction * 250.0;
	const std::optional<ScreenPoint> back = camera.WorldToScreen( along );
	checks.That(
	    back && std::fabs( back->x - 123.0 ) < 1.0e-6 && std::fabs( back->y - 45.0 ) < 1.0e-6,
	    "pixel -> ray -> point -> pixel round trip" );
	const std::optional<ScreenPoint> ahead =
	    camera.WorldToScreen( camera.Position() + camera.Forward() * 64.0 );
	checks.That(
	    ahead && std::fabs( ahead->x - 400.0 ) < 1.0e-6 && std::fabs( ahead->y - 300.0 ) < 1.0e-6,
	    "a point straight ahead projects to the centre" );
	checks.That( !camera.WorldToScreen( camera.Position() - camera.Forward() * 64.0 ),
	    "a point behind the camera does not project (negative)" );
	checks.That(
	    !camera.WorldToScreen( camera.Position() ), "the eye itself does not project (negative)" );
	const std::optional<ScreenPoint> right =
	    camera.WorldToScreen( camera.Position() + camera.Forward() * 64.0 + camera.Right() * 5.0 );
	const std::optional<ScreenPoint> up =
	    camera.WorldToScreen( camera.Position() + camera.Forward() * 64.0 + camera.Up() * 5.0 );
	checks.That( right && right->x > 400.0, "Right projects to screen right" );
	checks.That( up && up->y < 300.0, "Up projects to screen up" );
}

void Camera3DNavigation( testing::Checks &checks )
{
	Camera3D camera;
	camera.SetViewport( 800, 600 );
	camera.SetPosition( Vec3d( 0, 0, 100 ) );
	camera.SetAngles( 30.0, 20.0 );

	Camera3D flown = camera;
	flown.Fly( 100.0, 0.0, 0.0 );
	checks.That( Near( flown.Position(), camera.Position() + camera.Forward() * 100.0 ),
	    "Fly(forward) moves along Forward" );
	flown = camera;
	flown.Fly( 0.0, 0.0, 25.0 );
	checks.That( Near( flown.Position(), camera.Position() + Vec3d( 0, 0, 25 ) ),
	    "Fly(up) rises along world +Z while pitched" );
	flown = camera;
	flown.Fly( 0.0, 50.0, 0.0 );
	checks.That(
	    Near( flown.Position(), camera.Position() + camera.Right() * 50.0 ), "Fly(right) strafes" );
	checks.That( !flown.Fly( kNaN, 0.0, 0.0 ), "NaN fly rejected (negative)" );

	Camera3D looked = camera;
	looked.Look( 15.0, -5.0 );
	checks.That( looked.Position() == camera.Position(), "Look keeps the eye fixed" );
	checks.Near( looked.Yaw(), 45.0, kEps, "Look turns the yaw" );
	checks.Near( looked.Pitch(), 15.0, kEps, "Look turns the pitch" );
	looked.Look( 0.0, 1000.0 );
	checks.Near( looked.Pitch(), Camera3D::kMaxPitch, 0.0, "Look clamps the pitch" );

	const Vec3d pivot( 300, -200, 50 );
	const double distance = mapgeometry::Length( camera.Position() - pivot );
	Camera3D orbit = camera;
	for ( int i = 0; i < 12; ++i )
	{
		orbit.Orbit( pivot, 37.0, 11.0 );
		checks.Near( mapgeometry::Length( orbit.Position() - pivot ), distance, 1.0e-8,
		    "Orbit keeps the distance to the pivot" );
	}
	checks.That(
	    Near( mapgeometry::Normalize( pivot - orbit.Position() ), orbit.Forward(), 1.0e-9 ),
	    "after Orbit the camera looks at the pivot" );
	checks.Near( orbit.Pitch(), Camera3D::kMaxPitch, 0.0, "Orbit clamps the pitch" );
	Camera3D atPivot = camera;
	atPivot.Orbit( camera.Position(), 10.0, 0.0 );
	checks.That(
	    atPivot.Position() == camera.Position() && std::fabs( atPivot.Yaw() - 40.0 ) < kEps,
	    "Orbit about the eye only turns" );
	checks.That( !atPivot.Orbit( Vec3d( kNaN, 0, 0 ), 1.0, 1.0 ), "NaN pivot rejected (negative)" );

	Camera3D aimed = camera;
	checks.That( !aimed.LookAt( camera.Position() ), "LookAt the eye rejected (negative)" );
	checks.That( aimed.LookAt( Vec3d( 0, 100, 100 ) ) && std::fabs( aimed.Yaw() - 90.0 ) < kEps &&
	                 std::fabs( aimed.Pitch() ) < kEps,
	    "LookAt a point along +Y" );
	aimed.LookAt( Vec3d( 0, 0, 0 ) );
	checks.Near( aimed.Pitch(), Camera3D::kMaxPitch, 0.0, "LookAt straight down clamps" );

	// Framing.
	const Box box{ Vec3d( -64, 0, 0 ), Vec3d( 512, 256, 128 ) };
	Camera3D framed = camera;
	checks.That( framed.Frame( box ), "3D frame accepted" );
	checks.That( BoxOnScreen( framed, box ), "every corner of the framed box is on screen" );
	checks.That( Near( framed.Forward(), camera.Forward() ), "Frame keeps the angles" );
	checks.That( !BoxOnScreen( camera, box ), "oracle rejects the unframed view (seeded)" );
	Camera3D badFrame = camera;
	checks.That( !badFrame.Frame( { Vec3d( 1, 1, 1 ), Vec3d( 0, 0, 0 ) } ),
	    "inverted 3D box rejected (negative)" );
	checks.That( badFrame.Position() == camera.Position(), "rejected frame leaves the eye" );
	Camera3D pointFrame;
	checks.That( pointFrame.Frame( { Vec3d( 5, 5, 5 ), Vec3d( 5, 5, 5 ) } ) &&
	                 mapgeometry::Length( pointFrame.Position() - Vec3d( 5, 5, 5 ) ) > 0.0,
	    "a point box frames at a positive distance (degenerate viewport ok)" );
}

// The GTK shell's orbit camera (hammer/gtk/tests/test_camera_nav.cpp),
// expressed as an eye plus Source angles: eye = target + distance * dir with
// dir = (cos p cos y, cos p sin y, sin p), looking at the target.
void GtkCompatibility( testing::Checks &checks )
{
	const auto fromGtk = []( double yawDeg, double pitchDeg, double distance, const Vec3d &target )
	{
		const double y = yawDeg * std::numbers::pi / 180.0;
		const double p = pitchDeg * std::numbers::pi / 180.0;
		Camera3D camera;
		camera.SetViewport( 800, 600 );
		camera.SetPosition( target + Vec3d( std::cos( p ) * std::cos( y ),
		                                 std::cos( p ) * std::sin( y ), std::sin( p ) ) *
		                                 distance );
		camera.LookAt( target );
		return camera;
	};

	Camera3D camera = fromGtk( 0.0, 0.0, 1000.0, Vec3d() );
	checks.Near(
	    camera.Yaw(), 180.0, kEps, "GTK yaw 0 = Source yaw 180 (looking back at the target)" );
	const std::optional<Ray> centre = camera.RayThroughPixel( 400.0, 300.0 );
	checks.That(
	    centre && Near( centre->origin, Vec3d( 1000, 0, 0 ) ), "GTK: ray origin is the eye" );
	checks.That(
	    centre && Near( centre->direction, Vec3d( -1, 0, 0 ) ), "GTK: centre ray along -X" );
	const std::optional<Ray> off = camera.RayThroughPixel( 600.0, 300.0 );
	checks.That( off && std::fabs( off->direction.y - centre->direction.y ) > 1.0e-3,
	    "GTK: off-centre ray differs in Y" );

	Camera3D flown = camera;
	flown.Fly( 100.0, 0.0, 0.0 );
	checks.That( Near( flown.Position(), Vec3d( 900, 0, 0 ) ),
	    "GTK: FlyMove(forward) moves the eye -100 in X" );
	flown.Fly( 0.0, 50.0, 0.0 );
	checks.That(
	    std::fabs( flown.Position().x - 900.0 ) < kEps && std::fabs( flown.Position().y ) > 1.0,
	    "GTK: FlyMove(right) moves laterally" );
	flown.Fly( 0.0, 0.0, 25.0 );
	checks.Near( flown.Position().z, 25.0, kEps, "GTK: FlyMove(up) rises along world Z" );

	// GTK pitch > 0 puts the eye above the target, looking down: Source pitch > 0.
	Camera3D pitched = fromGtk( 45.0, 30.0, 700.0, Vec3d( 100, -50, 20 ) );
	checks.Near( pitched.Pitch(), 30.0, 1.0e-9, "GTK pitch 30 = Source pitch 30 (down)" );
	checks.Near( pitched.Yaw(), -135.0, 1.0e-9, "GTK yaw 45 = Source yaw -135" );

	// GTK PixelToRay: dir = f + ndcX*aspect*tanHalf*r + ndcY*tanHalf*u, 60 deg fov,
	// r = f x Z, u = r x f.
	const Vec3d f = pitched.Forward();
	const Vec3d r = mapgeometry::Normalize( mapgeometry::Cross( f, Vec3d( 0, 0, 1 ) ) );
	const Vec3d u = mapgeometry::Cross( r, f );
	const double tanHalf = std::tan( 30.0 * std::numbers::pi / 180.0 );
	const double ndcX = 2.0 * 610.0 / 800.0 - 1.0;
	const double ndcY = 1.0 - 2.0 * 170.0 / 600.0;
	const Vec3d gtk = mapgeometry::Normalize(
	    f + r * ( ndcX * 800.0 / 600.0 * tanHalf ) + u * ( ndcY * tanHalf ) );
	const std::optional<Ray> ray = pitched.RayThroughPixel( 610.0, 170.0 );
	checks.That( ray && Near( ray->direction, gtk, 1.0e-12 ), "GTK PixelToRay formula matches" );

	Camera3D looked = pitched;
	looked.Look( 15.0, -5.0 );
	checks.That( looked.Position() == pitched.Position(), "GTK: FlyLook keeps the eye fixed" );
}

} // namespace

int main()
{
	testing::Checks checks;
	AxisMapping( checks );
	Camera2DBasics( checks );
	Camera2DProjection( checks );
	Camera2DNavigation( checks );
	Camera2DFrame( checks );
	Camera3DAngles( checks );
	Camera3DProjection( checks );
	Camera3DNavigation( checks );
	GtkCompatibility( checks );
	return checks.Report();
}
