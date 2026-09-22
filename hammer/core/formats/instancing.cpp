//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats func_instance expansion. See
//			public/hammer/formats/instancing.h.
//
//=============================================================================//

#include "hammer/formats/instancing.h"

#include "hammer/geometry/brush.h" // geometry::Vec3d

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

// Row-major 3x3 rotation matrix.
struct Mat3
{
	double m[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
};

// Source QAngle -> rotation matrix (pitch about Y, yaw about Z, roll about X),
// degrees. Matches AngleMatrix so instance placement agrees with the engine.
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

// Applies the placement (rotate then translate) to a world point.
Vec3d Place( const Mat3 &r, const Vec3d &origin, const Vec3d &p )
{
	const Vec3d rot = Rotate( r, p );
	return Vec3d( rot.x + origin.x, rot.y + origin.y, rot.z + origin.z );
}

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

// Parses a VMF side plane value "(x y z) (x y z) (x y z)".
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

// Sets (or leaves) the first pair with 'key' to 'value'.
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

// Returns a copy of 'solid' with every side plane transformed by (r, origin).
// Non-plane data (materials, texture axes) is preserved verbatim; rotating texture
// axes is a declared later increment.
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

// Returns a copy of 'entity' with its "origin" transformed. Entity angle
// composition is a declared later increment.
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

const std::string *FindPair( const KeyValueNode &node, const std::string &key )
{
	return node.Find( key );
}

// Recursively flattens 'doc' under the cumulative placement (r, origin), pushing
// world solids and non-instance entities into the pools. 'identity' skips the
// transform copy at the top level for speed/fidelity.
struct Flattener
{
	ports::IFileStore &store;
	int maxDepth = 16;
	std::vector<KeyValueNode> solids;
	std::vector<KeyValueNode> entities;
	int expanded = 0;
	bool ok = true;
	std::string error;

	void Flatten(
	    const KeyValueNode &doc, const Mat3 &r, const Vec3d &origin, bool identity, int depth )
	{
		if ( !ok )
		{
			return;
		}
		if ( depth > maxDepth )
		{
			ok = false;
			error = "func_instance recursion exceeded maxDepth (cyclic or too deep)";
			return;
		}

		for ( const KeyValueNode &block : doc.children )
		{
			if ( !ok )
			{
				return;
			}
			if ( block.name == "world" )
			{
				for ( const KeyValueNode &child : block.children )
				{
					if ( child.name == "solid" )
					{
						solids.push_back( identity ? child : TransformSolid( child, r, origin ) );
					}
				}
			}
			else if ( block.name == "entity" )
			{
				const std::string *cls = FindPair( block, "classname" );
				if ( cls != nullptr && *cls == "func_instance" )
				{
					ExpandInstance( block, r, origin, depth );
				}
				else
				{
					entities.push_back( identity ? block : TransformEntity( block, r, origin ) );
				}
			}
		}
	}

	void ExpandInstance( const KeyValueNode &inst, const Mat3 &r, const Vec3d &origin, int depth )
	{
		const std::string *file = FindPair( inst, "file" );
		if ( file == nullptr || file->empty() )
		{
			ok = false;
			error = "func_instance has no 'file'";
			return;
		}

		Vec3d instOrigin;
		if ( const std::string *o = FindPair( inst, "origin" ) )
		{
			if ( !ParseVec3( *o, instOrigin ) )
			{
				ok = false;
				error = "func_instance 'origin' is not a vector: " + *o;
				return;
			}
		}
		double pitch = 0.0;
		double yaw = 0.0;
		double roll = 0.0;
		if ( const std::string *a = FindPair( inst, "angles" ) )
		{
			Vec3d ang;
			if ( !ParseVec3( *a, ang ) )
			{
				ok = false;
				error = "func_instance 'angles' is not a vector: " + *a;
				return;
			}
			pitch = ang.x;
			yaw = ang.y;
			roll = ang.z;
		}

		std::string text;
		if ( !store.Read( *file, text ) )
		{
			ok = false;
			error = "cannot read func_instance file: " + *file;
			return;
		}
		ParseResult pr = ParseKeyValues( text );
		if ( !pr.ok )
		{
			ok = false;
			error = "func_instance file failed to parse (" + *file + "): " + pr.error;
			return;
		}

		// Compose: a child point p is first placed by the instance's own
		// (angles, origin), then by the enclosing placement (r, origin):
		//   outer( Ri*p + instOrigin ) = (r*Ri)*p + (r*instOrigin + origin).
		const Mat3 ri = AngleMatrix( pitch, yaw, roll );
		const Mat3 composed = Multiply( r, ri );
		const Vec3d composedOrigin = Place( r, origin, instOrigin );

		++expanded;
		Flatten( pr.root, composed, composedOrigin, false, depth + 1 );
	}
};

} // namespace

InstanceExpandResult ExpandInstances(
    const KeyValueNode &root, ports::IFileStore &store, int maxDepth )
{
	InstanceExpandResult result;

	Flattener flat{ store, maxDepth, {}, {}, 0, true, {} };
	flat.Flatten( root, Mat3(), Vec3d(), true, 0 );
	if ( !flat.ok )
	{
		result.ok = false;
		result.error = flat.error;
		return result;
	}

	// Rebuild the flattened document: copy the root's non-world/non-instance
	// top-level blocks (versioninfo, etc.), then one world with all merged solids,
	// then all collected entities.
	KeyValueNode out;
	out.name = root.name;

	KeyValueNode worldOut;
	worldOut.name = "world";
	bool haveWorld = false;

	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name == "world" )
		{
			if ( !haveWorld )
			{
				// Keep the root world's pairs and any non-solid children (groups).
				worldOut = block;
				worldOut.children.clear();
				for ( const KeyValueNode &child : block.children )
				{
					if ( child.name != "solid" )
					{
						worldOut.children.push_back( child );
					}
				}
				haveWorld = true;
			}
		}
		else if ( block.name == "entity" )
		{
			// Instances are dropped (expanded); non-instance entities are added
			// from the collected pool below, so skip here.
		}
		else
		{
			out.children.push_back( block ); // versioninfo, cameras, cordon, visgroups, ...
		}
	}

	for ( const KeyValueNode &solid : flat.solids )
	{
		worldOut.children.push_back( solid );
	}
	out.children.push_back( worldOut );

	for ( const KeyValueNode &entity : flat.entities )
	{
		KeyValueNode e = entity;
		e.name = "entity";
		out.children.push_back( e );
	}

	result.ok = true;
	result.document = std::move( out );
	result.expanded = flat.expanded;
	return result;
}

} // namespace hammer::formats
