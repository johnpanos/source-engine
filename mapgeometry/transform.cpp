//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/mapgeometry/transform.h.
//
//=============================================================================//

#include "mapgeometry/transform.h"

#include "mapgeometry/vec3.h"

#include <cmath>

namespace mapgeometry
{

namespace
{

constexpr double kPi = 3.14159265358979323846;

double Radians( double degrees )
{
	return degrees * kPi / 180.0;
}

double Degrees( double radians )
{
	return radians * 180.0 / kPi;
}

// Two unit vectors spanning the plane orthogonal to 'n'.
void PlaneBasis( const Vec3d &n, Vec3d &a, Vec3d &b )
{
	const Vec3d helper = std::fabs( n.z ) < 0.9 ? Vec3d( 0, 0, 1 ) : Vec3d( 1, 0, 0 );
	a = Normalize( Cross( helper, n ) );
	b = Cross( n, a );
}

} // namespace

Mat3 Mat3::Scale( const Vec3d &s )
{
	Mat3 r;
	r.m[0][0] = s.x;
	r.m[1][1] = s.y;
	r.m[2][2] = s.z;
	return r;
}

Mat3 Mat3::AxisRotation( int axis, double degrees )
{
	// Exact quarter turns avoid sin/cos rounding so grid-aligned rotations stay
	// on the grid.
	double c = 0.0;
	double s = 0.0;
	const double turns = degrees / 90.0;
	if ( turns == std::floor( turns ) )
	{
		const int q = ( static_cast<int>( std::fmod( turns, 4.0 ) ) + 4 ) % 4;
		const double cs[4] = { 1, 0, -1, 0 };
		const double sn[4] = { 0, 1, 0, -1 };
		c = cs[q];
		s = sn[q];
	}
	else
	{
		c = std::cos( Radians( degrees ) );
		s = std::sin( Radians( degrees ) );
	}

	Mat3 r;
	const int i = ( axis + 1 ) % 3;
	const int j = ( axis + 2 ) % 3;
	r.m[i][i] = c;
	r.m[i][j] = -s;
	r.m[j][i] = s;
	r.m[j][j] = c;
	return r;
}

Mat3 Mat3::Mirror( int axis )
{
	Mat3 r;
	r.m[axis][axis] = -1.0;
	return r;
}

Mat3 Multiply( const Mat3 &a, const Mat3 &b )
{
	Mat3 out;
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			out.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j];
		}
	}
	return out;
}

Mat3 Transpose( const Mat3 &a )
{
	Mat3 out;
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			out.m[i][j] = a.m[j][i];
		}
	}
	return out;
}

double Determinant( const Mat3 &a )
{
	const auto &m = a.m;
	return m[0][0] * ( m[1][1] * m[2][2] - m[1][2] * m[2][1] ) -
	       m[0][1] * ( m[1][0] * m[2][2] - m[1][2] * m[2][0] ) +
	       m[0][2] * ( m[1][0] * m[2][1] - m[1][1] * m[2][0] );
}

Vec3d Apply( const Mat3 &a, const Vec3d &v )
{
	return Vec3d( a.m[0][0] * v.x + a.m[0][1] * v.y + a.m[0][2] * v.z,
	    a.m[1][0] * v.x + a.m[1][1] * v.y + a.m[1][2] * v.z,
	    a.m[2][0] * v.x + a.m[2][1] * v.y + a.m[2][2] * v.z );
}

Mat3 AngleMatrix( double pitch, double yaw, double roll )
{
	const double sp = std::sin( Radians( pitch ) );
	const double cp = std::cos( Radians( pitch ) );
	const double sy = std::sin( Radians( yaw ) );
	const double cy = std::cos( Radians( yaw ) );
	const double sr = std::sin( Radians( roll ) );
	const double cr = std::cos( Radians( roll ) );

	Mat3 r;
	r.m[0][0] = cp * cy;
	r.m[0][1] = sr * sp * cy - cr * sy;
	r.m[0][2] = cr * sp * cy + sr * sy;
	r.m[1][0] = cp * sy;
	r.m[1][1] = sr * sp * sy + cr * cy;
	r.m[1][2] = cr * sp * sy - sr * cy;
	r.m[2][0] = -sp;
	r.m[2][1] = sr * cp;
	r.m[2][2] = cr * cp;
	return r;
}

EulerAngles MatrixToAngles( const Mat3 &r )
{
	const Vec3d forward( r.m[0][0], r.m[1][0], r.m[2][0] );
	const Vec3d left( r.m[0][1], r.m[1][1], r.m[2][1] );
	const double up2 = r.m[2][2];

	EulerAngles out;
	const double xyDist = std::sqrt( forward.x * forward.x + forward.y * forward.y );
	if ( xyDist > 0.001 )
	{
		out.yaw = Degrees( std::atan2( forward.y, forward.x ) );
		out.pitch = Degrees( std::atan2( -forward.z, xyDist ) );
		out.roll = Degrees( std::atan2( left.z, up2 ) );
	}
	else
	{
		out.yaw = Degrees( std::atan2( -left.x, left.y ) );
		out.pitch = Degrees( std::atan2( -forward.z, xyDist ) );
		out.roll = 0.0;
	}
	return out;
}

Affine Affine::Translation( const Vec3d &delta )
{
	Affine a;
	a.translation = delta;
	return a;
}

Affine Affine::About( const Mat3 &linear, const Vec3d &pivot )
{
	Affine a;
	a.linear = linear;
	a.translation = pivot - Apply( linear, pivot );
	return a;
}

Vec3d Affine::Point( const Vec3d &p ) const
{
	return Apply( linear, p ) + translation;
}

Vec3d Affine::Direction( const Vec3d &d ) const
{
	return Apply( linear, d );
}

bool Affine::Mirrors() const
{
	return Determinant( linear ) < 0.0;
}

bool Affine::IsTranslation() const
{
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			if ( linear.m[i][j] != ( i == j ? 1.0 : 0.0 ) )
			{
				return false;
			}
		}
	}
	return true;
}

Affine Compose( const Affine &a, const Affine &b )
{
	Affine out;
	out.linear = Multiply( a.linear, b.linear );
	out.translation = Apply( a.linear, b.translation ) + a.translation;
	return out;
}

std::optional<Affine> Inverse( const Affine &a )
{
	const double det = Determinant( a.linear );
	if ( std::fabs( det ) < 1.0e-12 )
	{
		return std::nullopt;
	}
	const auto &m = a.linear.m;
	Mat3 inv;
	inv.m[0][0] = ( m[1][1] * m[2][2] - m[1][2] * m[2][1] ) / det;
	inv.m[0][1] = ( m[0][2] * m[2][1] - m[0][1] * m[2][2] ) / det;
	inv.m[0][2] = ( m[0][1] * m[1][2] - m[0][2] * m[1][1] ) / det;
	inv.m[1][0] = ( m[1][2] * m[2][0] - m[1][0] * m[2][2] ) / det;
	inv.m[1][1] = ( m[0][0] * m[2][2] - m[0][2] * m[2][0] ) / det;
	inv.m[1][2] = ( m[0][2] * m[1][0] - m[0][0] * m[1][2] ) / det;
	inv.m[2][0] = ( m[1][0] * m[2][1] - m[1][1] * m[2][0] ) / det;
	inv.m[2][1] = ( m[0][1] * m[2][0] - m[0][0] * m[2][1] ) / det;
	inv.m[2][2] = ( m[0][0] * m[1][1] - m[0][1] * m[1][0] ) / det;

	Affine out;
	out.linear = inv;
	out.translation = -Apply( inv, a.translation );
	return out;
}

std::optional<Plane> TransformPlane( const Affine &xf, const Plane &plane )
{
	if ( std::fabs( Determinant( xf.linear ) ) < 1.0e-12 )
	{
		return std::nullopt;
	}
	const Vec3d n = Normalize( plane.normal );
	Vec3d a;
	Vec3d b;
	PlaneBasis( n, a, b );
	const Vec3d p0 = n * plane.dist;
	const Vec3d q0 = xf.Point( p0 );
	const Vec3d q1 = xf.Point( p0 + a );
	const Vec3d q2 = xf.Point( p0 + b );
	Vec3d normal = Normalize( Cross( q1 - q0, q2 - q0 ) );
	// Orient by an outside point: p0 + n is outside the original plane.
	const Vec3d outside = xf.Point( p0 + n );
	if ( Dot( normal, outside - q0 ) < 0.0 )
	{
		normal = -normal;
	}
	Plane out;
	out.normal = normal;
	out.dist = Dot( normal, q0 );
	return out;
}

} // namespace mapgeometry
