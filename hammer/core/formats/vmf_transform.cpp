//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the shared VMF geometry transform. See
//			public/hammer/formats/vmf_transform.h.
//
//=============================================================================//

#include "hammer/formats/vmf_transform.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace hammer::formats
{

namespace
{

using geometry::Vec3d;

constexpr double kPi = 3.14159265358979323846;

bool ParseDoubles( const std::string &text, std::vector<double> &out )
{
	out.clear();
	std::istringstream in( text );
	double value = 0.0;
	while ( in >> value )
	{
		out.push_back( value );
	}
	if ( in.eof() )
	{
		return true;
	}
	in.clear();
	std::string leftover;
	in >> leftover;
	return leftover.empty();
}

std::string FormatDouble( double v )
{
	char buf[64];
	std::snprintf( buf, sizeof( buf ), "%.10g", v );
	return std::string( buf );
}

std::string FormatVec3( const Vec3d &v )
{
	return FormatDouble( v.x ) + " " + FormatDouble( v.y ) + " " + FormatDouble( v.z );
}

bool ParseVec3( const std::string &text, Vec3d &out )
{
	std::string cleaned = text;
	for ( char &ch : cleaned )
	{
		if ( ch == '[' || ch == ']' || ch == '(' || ch == ')' )
		{
			ch = ' ';
		}
	}
	std::vector<double> vals;
	if ( !ParseDoubles( cleaned, vals ) || vals.size() != 3 )
	{
		return false;
	}
	out = Vec3d( vals[0], vals[1], vals[2] );
	return true;
}

std::optional<std::array<Vec3d, 3>> ParsePlanePoints( const std::string &value )
{
	std::string cleaned = value;
	for ( char &ch : cleaned )
	{
		if ( ch == '(' || ch == ')' )
		{
			ch = ' ';
		}
	}
	std::vector<double> vals;
	if ( !ParseDoubles( cleaned, vals ) || vals.size() != 9 )
	{
		return std::nullopt;
	}
	std::array<Vec3d, 3> pts;
	for ( int i = 0; i < 3; ++i )
	{
		pts[static_cast<std::size_t>( i )] = Vec3d( vals[static_cast<std::size_t>( i ) * 3 + 0],
		    vals[static_cast<std::size_t>( i ) * 3 + 1],
		    vals[static_cast<std::size_t>( i ) * 3 + 2] );
	}
	return pts;
}

std::string FormatPlanePoints( const std::array<Vec3d, 3> &pts )
{
	return "(" + FormatVec3( pts[0] ) + ") (" + FormatVec3( pts[1] ) + ") (" +
	       FormatVec3( pts[2] ) + ")";
}

void SetPair( KeyValueNode &node, const std::string &key, const std::string &value )
{
	for ( KeyValue &kv : node.pairs )
	{
		if ( kv.key == key )
		{
			kv.value = value;
			return;
		}
	}
}

} // namespace

Mat3 AngleMatrix( double pitch, double yaw, double roll )
{
	const double sp = std::sin( pitch * kPi / 180.0 );
	const double cp = std::cos( pitch * kPi / 180.0 );
	const double sy = std::sin( yaw * kPi / 180.0 );
	const double cy = std::cos( yaw * kPi / 180.0 );
	const double sr = std::sin( roll * kPi / 180.0 );
	const double cr = std::cos( roll * kPi / 180.0 );

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

Vec3d Rotate( const Mat3 &r, const Vec3d &v )
{
	return Vec3d( r.m[0][0] * v.x + r.m[0][1] * v.y + r.m[0][2] * v.z,
	    r.m[1][0] * v.x + r.m[1][1] * v.y + r.m[1][2] * v.z,
	    r.m[2][0] * v.x + r.m[2][1] * v.y + r.m[2][2] * v.z );
}

Vec3d Place( const Mat3 &r, const Vec3d &origin, const Vec3d &p )
{
	const Vec3d rot = Rotate( r, p );
	return Vec3d( rot.x + origin.x, rot.y + origin.y, rot.z + origin.z );
}

KeyValueNode TransformSolid( const KeyValueNode &solid, const Mat3 &r, const Vec3d &origin )
{
	KeyValueNode out = solid;
	for ( KeyValueNode &side : out.children )
	{
		if ( side.name != "side" )
		{
			continue;
		}
		for ( KeyValue &kv : side.pairs )
		{
			if ( kv.key != "plane" )
			{
				continue;
			}
			if ( auto pts = ParsePlanePoints( kv.value ) )
			{
				std::array<Vec3d, 3> moved = { Place( r, origin, ( *pts )[0] ),
				    Place( r, origin, ( *pts )[1] ), Place( r, origin, ( *pts )[2] ) };
				kv.value = FormatPlanePoints( moved );
			}
		}
	}
	return out;
}

KeyValueNode TransformEntity( const KeyValueNode &entity, const Mat3 &r, const Vec3d &origin )
{
	KeyValueNode out = entity;
	if ( const std::string *o = out.Find( "origin" ) )
	{
		Vec3d p;
		if ( ParseVec3( *o, p ) )
		{
			SetPair( out, "origin", FormatVec3( Place( r, origin, p ) ) );
		}
	}
	return out;
}

} // namespace hammer::formats
