//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Viewport projection for the Hammer editor (RFC 0002,
//			hammer.viewport). See public/hammer/viewport/camera.h.
//
//=============================================================================//

#include "hammer/viewport/camera.h"

#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace hammer::viewport
{

using mapgeometry::Vec3d;

namespace
{

constexpr double kDegToRad = std::numbers::pi / 180.0;
constexpr double kRadToDeg = 180.0 / std::numbers::pi;

bool Finite( double value )
{
	return std::isfinite( value );
}

bool Finite( const Vec3d &v )
{
	return std::isfinite( v.x ) && std::isfinite( v.y ) && std::isfinite( v.z );
}

bool ValidBox( const scene::Box &box )
{
	return Finite( box.mins ) && Finite( box.maxs ) && box.mins.x <= box.maxs.x &&
	       box.mins.y <= box.maxs.y && box.mins.z <= box.maxs.z;
}

double NormalizeYaw( double yaw )
{
	double wrapped = std::fmod( yaw, 360.0 );
	if ( wrapped > 180.0 )
	{
		wrapped -= 360.0;
	}
	else if ( wrapped <= -180.0 )
	{
		wrapped += 360.0;
	}
	return wrapped;
}

double ClampPitch( double pitch )
{
	return std::clamp( pitch, -Camera3D::kMaxPitch, Camera3D::kMaxPitch );
}

} // namespace

ViewAxes AxesOf( ViewKind kind )
{
	switch ( kind )
	{
	case ViewKind::Front:
		return { 0, 2, 1 };
	case ViewKind::Side:
		return { 1, 2, 0 };
	case ViewKind::Top:
	case ViewKind::Camera3D:
	default:
		return { 0, 1, 2 };
	}
}

Vec3d ViewDirection( ViewKind kind )
{
	const ViewAxes axes = AxesOf( kind );
	Vec3d u;
	Vec3d v;
	mapgeometry::SetComponent( u, axes.u, 1.0 );
	mapgeometry::SetComponent( v, axes.v, 1.0 );
	return -mapgeometry::Cross( u, v );
}

// --- Camera2D ---------------------------------------------------------------

bool Camera2D::SetKind( ViewKind kind )
{
	if ( kind != ViewKind::Top && kind != ViewKind::Front && kind != ViewKind::Side )
	{
		return false;
	}
	m_kind = kind;
	return true;
}

bool Camera2D::SetCenter( PlanePoint center )
{
	if ( !Finite( center.u ) || !Finite( center.v ) )
	{
		return false;
	}
	m_center = center;
	return true;
}

bool Camera2D::SetZoom( double pixelsPerUnit )
{
	if ( !Finite( pixelsPerUnit ) || pixelsPerUnit <= 0.0 )
	{
		return false;
	}
	m_zoom = std::clamp( pixelsPerUnit, kMinZoom, kMaxZoom );
	return true;
}

void Camera2D::SetViewport( int width, int height )
{
	m_width = std::max( width, 0 );
	m_height = std::max( height, 0 );
}

PlanePoint Camera2D::ToPlane( const Vec3d &world ) const
{
	const ViewAxes axes = Axes();
	return { mapgeometry::Component( world, axes.u ), mapgeometry::Component( world, axes.v ) };
}

double Camera2D::DepthOf( const Vec3d &world ) const
{
	return mapgeometry::Component( world, Axes().depth );
}

Vec3d Camera2D::PlaneToWorld( PlanePoint plane, double depth ) const
{
	const ViewAxes axes = Axes();
	Vec3d world;
	mapgeometry::SetComponent( world, axes.u, plane.u );
	mapgeometry::SetComponent( world, axes.v, plane.v );
	mapgeometry::SetComponent( world, axes.depth, depth );
	return world;
}

ScreenPoint Camera2D::PlaneToScreen( PlanePoint plane ) const
{
	return { ( plane.u - m_center.u ) * m_zoom + m_width * 0.5,
	    m_height * 0.5 - ( plane.v - m_center.v ) * m_zoom };
}

ScreenPoint Camera2D::WorldToScreen( const Vec3d &world ) const
{
	return PlaneToScreen( ToPlane( world ) );
}

PlanePoint Camera2D::ScreenToPlane( double px, double py ) const
{
	return { m_center.u + ( px - m_width * 0.5 ) / m_zoom,
	    m_center.v - ( py - m_height * 0.5 ) / m_zoom };
}

Vec3d Camera2D::ScreenToWorld( double px, double py, double depth ) const
{
	return PlaneToWorld( ScreenToPlane( px, py ), depth );
}

PlaneRect Camera2D::VisibleRect() const
{
	const double halfU = m_width * 0.5 / m_zoom;
	const double halfV = m_height * 0.5 / m_zoom;
	return {
	    { m_center.u - halfU, m_center.v - halfV }, { m_center.u + halfU, m_center.v + halfV } };
}

bool Camera2D::PanPixels( double dx, double dy )
{
	if ( !Finite( dx ) || !Finite( dy ) )
	{
		return false;
	}
	// Screen y is down and v is up, so a downward drag raises the centre's v.
	m_center.u -= dx / m_zoom;
	m_center.v += dy / m_zoom;
	return true;
}

bool Camera2D::ZoomAt( double px, double py, double factor )
{
	if ( !Finite( px ) || !Finite( py ) || !Finite( factor ) || factor <= 0.0 )
	{
		return false;
	}
	const PlanePoint anchor = ScreenToPlane( px, py );
	m_zoom = std::clamp( m_zoom * factor, kMinZoom, kMaxZoom );
	m_center.u = anchor.u - ( px - m_width * 0.5 ) / m_zoom;
	m_center.v = anchor.v + ( py - m_height * 0.5 ) / m_zoom;
	return true;
}

bool Camera2D::Frame( const scene::Box &box, double marginPixels )
{
	if ( !ValidBox( box ) || !Finite( marginPixels ) || marginPixels < 0.0 )
	{
		return false;
	}
	const double usableW = m_width - 2.0 * marginPixels;
	const double usableH = m_height - 2.0 * marginPixels;
	if ( usableW <= 0.0 || usableH <= 0.0 )
	{
		return false;
	}
	const PlanePoint lo = ToPlane( box.mins );
	const PlanePoint hi = ToPlane( box.maxs );
	const double extentU = hi.u - lo.u;
	const double extentV = hi.v - lo.v;
	double zoom = m_zoom;
	if ( extentU > 0.0 || extentV > 0.0 )
	{
		zoom = kMaxZoom;
		if ( extentU > 0.0 )
		{
			zoom = std::min( zoom, usableW / extentU );
		}
		if ( extentV > 0.0 )
		{
			zoom = std::min( zoom, usableH / extentV );
		}
	}
	m_zoom = std::clamp( zoom, kMinZoom, kMaxZoom );
	m_center = { ( lo.u + hi.u ) * 0.5, ( lo.v + hi.v ) * 0.5 };
	return true;
}

// --- Camera3D ---------------------------------------------------------------

bool Camera3D::SetPosition( const Vec3d &position )
{
	if ( !Finite( position ) )
	{
		return false;
	}
	m_position = position;
	return true;
}

bool Camera3D::SetAngles( double yawDegrees, double pitchDegrees )
{
	if ( !Finite( yawDegrees ) || !Finite( pitchDegrees ) )
	{
		return false;
	}
	m_yaw = NormalizeYaw( yawDegrees );
	m_pitch = ClampPitch( pitchDegrees );
	return true;
}

bool Camera3D::SetFov( double degrees )
{
	if ( !Finite( degrees ) )
	{
		return false;
	}
	m_fov = std::clamp( degrees, kMinFov, kMaxFov );
	return true;
}

void Camera3D::SetViewport( int width, int height )
{
	m_width = std::max( width, 0 );
	m_height = std::max( height, 0 );
}

Vec3d Camera3D::Forward() const
{
	const double yaw = m_yaw * kDegToRad;
	const double pitch = m_pitch * kDegToRad;
	return Vec3d( std::cos( pitch ) * std::cos( yaw ), std::cos( pitch ) * std::sin( yaw ),
	    -std::sin( pitch ) );
}

Vec3d Camera3D::Right() const
{
	const double yaw = m_yaw * kDegToRad;
	return Vec3d( std::sin( yaw ), -std::cos( yaw ), 0.0 );
}

Vec3d Camera3D::Up() const
{
	return mapgeometry::Cross( Right(), Forward() );
}

std::optional<Ray> Camera3D::RayThroughPixel( double px, double py ) const
{
	if ( !HasArea() || !Finite( px ) || !Finite( py ) )
	{
		return std::nullopt;
	}
	const double tanHalf = std::tan( m_fov * kDegToRad * 0.5 );
	const double aspect = static_cast<double>( m_width ) / static_cast<double>( m_height );
	const double ndcX = 2.0 * px / m_width - 1.0;
	const double ndcY = 1.0 - 2.0 * py / m_height;
	const Vec3d direction =
	    Forward() + Right() * ( ndcX * aspect * tanHalf ) + Up() * ( ndcY * tanHalf );
	return Ray{ m_position, mapgeometry::Normalize( direction ) };
}

std::optional<ScreenPoint> Camera3D::WorldToScreen( const Vec3d &world ) const
{
	if ( !HasArea() || !Finite( world ) )
	{
		return std::nullopt;
	}
	const Vec3d offset = world - m_position;
	const double depth = mapgeometry::Dot( offset, Forward() );
	if ( depth <= kNearDepth )
	{
		return std::nullopt;
	}
	const double tanHalf = std::tan( m_fov * kDegToRad * 0.5 );
	const double aspect = static_cast<double>( m_width ) / static_cast<double>( m_height );
	const double ndcX = mapgeometry::Dot( offset, Right() ) / ( depth * aspect * tanHalf );
	const double ndcY = mapgeometry::Dot( offset, Up() ) / ( depth * tanHalf );
	return ScreenPoint{ ( ndcX + 1.0 ) * 0.5 * m_width, ( 1.0 - ndcY ) * 0.5 * m_height };
}

bool Camera3D::Fly( double forward, double right, double up )
{
	if ( !Finite( forward ) || !Finite( right ) || !Finite( up ) )
	{
		return false;
	}
	m_position += Forward() * forward + Right() * right + Vec3d( 0.0, 0.0, up );
	return true;
}

bool Camera3D::Look( double dyawDegrees, double dpitchDegrees )
{
	return SetAngles( m_yaw + dyawDegrees, m_pitch + dpitchDegrees );
}

bool Camera3D::Orbit( const Vec3d &pivot, double dyawDegrees, double dpitchDegrees )
{
	if ( !Finite( pivot ) || !Finite( dyawDegrees ) || !Finite( dpitchDegrees ) )
	{
		return false;
	}
	const double distance = mapgeometry::Length( m_position - pivot );
	SetAngles( m_yaw + dyawDegrees, m_pitch + dpitchDegrees );
	if ( distance > 0.0 )
	{
		m_position = pivot - Forward() * distance;
	}
	return true;
}

bool Camera3D::LookAt( const Vec3d &target )
{
	if ( !Finite( target ) )
	{
		return false;
	}
	const Vec3d offset = target - m_position;
	const double length = mapgeometry::Length( offset );
	if ( length <= 0.0 )
	{
		return false;
	}
	const double yaw = std::atan2( offset.y, offset.x ) * kRadToDeg;
	const double pitch = -std::asin( std::clamp( offset.z / length, -1.0, 1.0 ) ) * kRadToDeg;
	return SetAngles( yaw, pitch );
}

bool Camera3D::Frame( const scene::Box &box )
{
	if ( !ValidBox( box ) )
	{
		return false;
	}
	const Vec3d center = box.Center();
	const double radius = std::max( mapgeometry::Length( box.Size() ) * 0.5, 1.0 );
	const double halfV = m_fov * kDegToRad * 0.5;
	const double aspect =
	    HasArea() ? static_cast<double>( m_width ) / static_cast<double>( m_height ) : 1.0;
	const double halfH = std::atan( aspect * std::tan( halfV ) );
	const double half = std::min( halfV, halfH );
	const double distance = radius / std::sin( half );
	m_position = center - Forward() * distance;
	return true;
}

} // namespace hammer::viewport
