//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/transform_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/transform_ops.h"

#include "hammer/app/ops/texture_ops.h"
#include "hammer/scene/map_queries.h"
#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"

#include <cmath>
#include <sstream>

namespace hammer::app::ops
{

using mapgeometry::Affine;
using mapgeometry::Vec3d;

namespace
{

double Tidy( double v )
{
	const double r = std::round( v );
	if ( std::fabs( v - r ) < 1.0e-6 )
	{
		return r == 0.0 ? 0.0 : r;
	}
	return v;
}

Vec3d Tidy( const Vec3d &v )
{
	return Vec3d( Tidy( v.x ), Tidy( v.y ), Tidy( v.z ) );
}

std::vector<double> ParseNumbers( const std::string &text )
{
	std::vector<double> out;
	std::string cleaned = text;
	for ( char &c : cleaned )
	{
		if ( c == '[' || c == ']' || c == '(' || c == ')' )
		{
			c = ' ';
		}
	}
	std::istringstream in( cleaned );
	double v = 0.0;
	while ( in >> v )
	{
		out.push_back( v );
	}
	return out;
}

std::string JoinNumbers( const std::vector<double> &values )
{
	std::string out;
	for ( std::size_t i = 0; i < values.size(); ++i )
	{
		out += ( i ? " " : "" ) + scene::FormatNumber( values[i] );
	}
	return out;
}

kvtext::KeyValueNode *FindChild( kvtext::KeyValueNode &node, const char *name )
{
	for ( kvtext::KeyValueNode &child : node.children )
	{
		if ( child.name == name )
		{
			return &child;
		}
	}
	return nullptr;
}

// Transforms the vector rows of a dispinfo block: the start position is a point;
// normals and offsets are vectors; a normal's length change moves into the
// matching distance so the displaced surface follows the map exactly.
void TransformDispInfo( kvtext::KeyValueNode &disp, const Affine &xf )
{
	for ( kvtext::KeyValue &kv : disp.pairs )
	{
		if ( kv.key == "startposition" )
		{
			const std::vector<double> v = ParseNumbers( kv.value );
			if ( v.size() == 3 )
			{
				const Vec3d p = Tidy( xf.Point( Vec3d( v[0], v[1], v[2] ) ) );
				kv.value = "[" + scene::FormatVec3( p ) + "]";
			}
		}
	}
	kvtext::KeyValueNode *normals = FindChild( disp, "normals" );
	kvtext::KeyValueNode *distances = FindChild( disp, "distances" );
	if ( normals )
	{
		for ( kvtext::KeyValue &row : normals->pairs )
		{
			std::vector<double> n = ParseNumbers( row.value );
			std::vector<double> *d = nullptr;
			std::vector<double> dist;
			kvtext::KeyValue *distRow = nullptr;
			if ( distances )
			{
				for ( kvtext::KeyValue &r : distances->pairs )
				{
					if ( r.key == row.key )
					{
						distRow = &r;
						dist = ParseNumbers( r.value );
						d = &dist;
					}
				}
			}
			for ( std::size_t i = 0; i + 2 < n.size(); i += 3 )
			{
				const Vec3d v = xf.Direction( Vec3d( n[i], n[i + 1], n[i + 2] ) );
				const double len = mapgeometry::Length( v );
				const Vec3d unit = len > 1.0e-12 ? v / len : Vec3d();
				n[i] = Tidy( unit.x );
				n[i + 1] = Tidy( unit.y );
				n[i + 2] = Tidy( unit.z );
				if ( d && i / 3 < d->size() && len > 1.0e-12 )
				{
					( *d )[i / 3] *= len;
				}
			}
			row.value = JoinNumbers( n );
			if ( distRow )
			{
				distRow->value = JoinNumbers( dist );
			}
		}
	}
	for ( const char *name : { "offsets", "offset_normals" } )
	{
		kvtext::KeyValueNode *rows = FindChild( disp, name );
		if ( !rows )
		{
			continue;
		}
		const bool unit = std::string( name ) == "offset_normals";
		for ( kvtext::KeyValue &row : rows->pairs )
		{
			std::vector<double> n = ParseNumbers( row.value );
			for ( std::size_t i = 0; i + 2 < n.size(); i += 3 )
			{
				Vec3d v = xf.Direction( Vec3d( n[i], n[i + 1], n[i + 2] ) );
				if ( unit )
				{
					v = mapgeometry::Normalize( v );
				}
				n[i] = Tidy( v.x );
				n[i + 1] = Tidy( v.y );
				n[i + 2] = Tidy( v.z );
			}
			row.value = JoinNumbers( n );
		}
	}
}

// Rotation part of 'xf' applied to an orientation: forward and up are mapped,
// re-orthonormalized, and left completes a right-handed frame.
std::optional<mapgeometry::EulerAngles> ComposeAngles(
    const mapgeometry::Affine &xf, double pitch, double yaw, double roll )
{
	const mapgeometry::Mat3 r = mapgeometry::AngleMatrix( pitch, yaw, roll );
	const Vec3d forward( r.m[0][0], r.m[1][0], r.m[2][0] );
	const Vec3d up( r.m[0][2], r.m[1][2], r.m[2][2] );
	const Vec3d f = mapgeometry::Normalize( xf.Direction( forward ) );
	Vec3d u = xf.Direction( up );
	u = mapgeometry::Normalize( u - f * mapgeometry::Dot( f, u ) );
	if ( mapgeometry::Length( f ) < 0.5 || mapgeometry::Length( u ) < 0.5 )
	{
		return std::nullopt;
	}
	const Vec3d l = mapgeometry::Cross( u, f );
	mapgeometry::Mat3 out;
	for ( int i = 0; i < 3; ++i )
	{
		out.m[i][0] = mapgeometry::Component( f, i );
		out.m[i][1] = mapgeometry::Component( l, i );
		out.m[i][2] = mapgeometry::Component( u, i );
	}
	mapgeometry::EulerAngles a = mapgeometry::MatrixToAngles( out );
	a.pitch = Tidy( a.pitch );
	a.yaw = Tidy( a.yaw );
	a.roll = Tidy( a.roll );
	if ( a.yaw < 0.0 )
	{
		a.yaw = Tidy( a.yaw + 360.0 );
	}
	return a;
}

} // namespace

std::optional<scene::Solid> TransformedSolid(
    const scene::Solid &solid, const Affine &xf, const TransformOptions &options )
{
	if ( std::fabs( mapgeometry::Determinant( xf.linear ) ) < 1.0e-9 )
	{
		return std::nullopt;
	}
	const bool mirrors = xf.Mirrors();
	scene::Solid out = solid;
	for ( scene::Side &side : out.sides )
	{
		for ( Vec3d &p : side.points )
		{
			p = Tidy( xf.Point( p ) );
		}
		if ( mirrors )
		{
			std::swap( side.points[0], side.points[2] );
		}
		if ( options.textureLock )
		{
			side.texture = LockTexture( side.texture, xf );
		}
		if ( side.dispinfo )
		{
			TransformDispInfo( *side.dispinfo, xf );
		}
	}
	// The result must still bound a closed solid with outward planes.
	std::vector<mapgeometry::Plane> planes;
	for ( const scene::Side &side : out.sides )
	{
		planes.push_back( side.Plane() );
	}
	if ( !mapgeometry::IsClosedSolid( planes ) )
	{
		return std::nullopt;
	}
	const mapgeometry::BrushSolid geometry = scene::BuildGeometry( out );
	if ( geometry.faces.size() != scene::BuildGeometry( solid ).faces.size() )
	{
		return std::nullopt;
	}
	const std::vector<Vec3d> vertices = mapgeometry::SolidVertices( geometry );
	Vec3d centroid;
	for ( const Vec3d &v : vertices )
	{
		centroid += v;
	}
	centroid = centroid / static_cast<double>( vertices.size() );
	if ( !mapgeometry::InsideAll( planes, centroid ) )
	{
		return std::nullopt; // inward planes: the winding rule was violated
	}
	return out;
}

scene::Entity TransformedEntity( const scene::Entity &entity, const Affine &xf )
{
	scene::Entity out = entity;
	if ( const std::optional<Vec3d> origin = entity.Origin() )
	{
		out.SetOrigin( Tidy( xf.Point( *origin ) ) );
	}
	if ( xf.IsTranslation() )
	{
		return out;
	}
	if ( const std::optional<Vec3d> angles = entity.Angles() )
	{
		if ( const auto a = ComposeAngles( xf, angles->x, angles->y, angles->z ) )
		{
			out.SetAngles( Vec3d( a->pitch, a->yaw, a->roll ) );
		}
	}
	else if ( const std::string *angle = entity.Key( "angle" ) )
	{
		const std::vector<double> v = ParseNumbers( *angle );
		// -1 and -2 are the legacy "up" and "down" markers.
		if ( v.size() == 1 && v[0] != -1.0 && v[0] != -2.0 )
		{
			if ( const auto a = ComposeAngles( xf, 0.0, v[0], 0.0 ) )
			{
				out.SetKey( "angle", scene::FormatNumber( a->yaw ) );
			}
		}
	}
	return out;
}

EditResult TransformObjects( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const Affine &xf, const TransformOptions &options )
{
	const std::vector<scene::ObjectId> leaves = scene::ExpandToLeaves( edit, ids );
	if ( leaves.empty() )
	{
		return NothingToDo( "nothing selected to transform" );
	}
	// Validate the complete selection before any write.
	std::vector<std::pair<scene::ObjectId, scene::Solid>> solids;
	for ( scene::ObjectId id : leaves )
	{
		if ( const scene::Solid *s = edit.FindSolid( id ) )
		{
			std::optional<scene::Solid> moved = TransformedSolid( *s, xf, options );
			if ( !moved )
			{
				return Reject( "the transform would make a solid degenerate" );
			}
			solids.emplace_back( id, std::move( *moved ) );
		}
	}
	for ( auto &[id, solid] : solids )
	{
		*edit.MutableSolid( id ) = std::move( solid );
	}
	for ( scene::ObjectId id : leaves )
	{
		if ( const scene::Entity *e = edit.FindEntity( id ) )
		{
			scene::Entity moved = TransformedEntity( *e, xf );
			*edit.MutableEntity( id ) = std::move( moved );
		}
	}
	return {};
}

EditResult Translate( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const Vec3d &delta, const TransformOptions &options )
{
	if ( delta == Vec3d() )
	{
		return NothingToDo( "zero translation" );
	}
	return TransformObjects( edit, ids, Affine::Translation( delta ), options );
}

EditResult Rotate( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, int axis,
    double degrees, const Vec3d &pivot, const TransformOptions &options )
{
	if ( axis < 0 || axis > 2 )
	{
		return Reject( "rotation axis must be 0, 1 or 2" );
	}
	if ( std::fmod( degrees, 360.0 ) == 0.0 )
	{
		return NothingToDo( "zero rotation" );
	}
	return TransformObjects(
	    edit, ids, Affine::About( mapgeometry::Mat3::AxisRotation( axis, degrees ), pivot ), options );
}

EditResult ScaleToBox( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const scene::Box &from, const scene::Box &to, const TransformOptions &options )
{
	Vec3d scale;
	Vec3d offset;
	for ( int axis = 0; axis < 3; ++axis )
	{
		const double f0 = mapgeometry::Component( from.mins, axis );
		const double f1 = mapgeometry::Component( from.maxs, axis );
		const double t0 = mapgeometry::Component( to.mins, axis );
		const double t1 = mapgeometry::Component( to.maxs, axis );
		double s = 1.0;
		if ( f1 - f0 > 0.0 )
		{
			if ( t1 - t0 <= 0.0 )
			{
				return Reject( "the target box is flat or inverted" );
			}
			s = ( t1 - t0 ) / ( f1 - f0 );
		}
		mapgeometry::SetComponent( scale, axis, s );
		mapgeometry::SetComponent( offset, axis, t0 - s * f0 );
	}
	Affine xf;
	xf.linear = mapgeometry::Mat3::Scale( scale );
	xf.translation = offset;
	if ( xf.IsTranslation() && offset == Vec3d() )
	{
		return NothingToDo( "the boxes are equal" );
	}
	return TransformObjects( edit, ids, xf, options );
}

EditResult Mirror( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids, int axis,
    const Vec3d &pivot, const TransformOptions &options )
{
	if ( axis < 0 || axis > 2 )
	{
		return Reject( "mirror axis must be 0, 1 or 2" );
	}
	return TransformObjects( edit, ids, Affine::About( mapgeometry::Mat3::Mirror( axis ), pivot ), options );
}

EditResult SnapToGrid( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    double grid, const TransformOptions &options )
{
	if ( grid <= 0.0 )
	{
		return Reject( "grid size must be positive" );
	}
	const std::optional<scene::Box> bounds = scene::ObjectsBounds( edit, ids );
	if ( !bounds )
	{
		return NothingToDo( "nothing selected to snap" );
	}
	Vec3d delta;
	for ( int axis = 0; axis < 3; ++axis )
	{
		const double v = mapgeometry::Component( bounds->mins, axis );
		mapgeometry::SetComponent( delta, axis, std::round( v / grid ) * grid - v );
	}
	return Translate( edit, ids, Tidy( delta ), options );
}

EditResult AlignObjects( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    int axis, bool toMaximum, const TransformOptions &options )
{
	if ( axis < 0 || axis > 2 )
	{
		return Reject( "align axis must be 0, 1 or 2" );
	}
	const std::optional<scene::Box> all = scene::ObjectsBounds( edit, ids );
	if ( !all || ids.size() < 2 )
	{
		return NothingToDo( "align needs two or more objects" );
	}
	const double target = mapgeometry::Component( toMaximum ? all->maxs : all->mins, axis );
	bool moved = false;
	for ( scene::ObjectId id : ids )
	{
		const std::optional<scene::Box> b = scene::ObjectBounds( edit, id );
		if ( !b )
		{
			continue;
		}
		const double at = mapgeometry::Component( toMaximum ? b->maxs : b->mins, axis );
		if ( at == target )
		{
			continue;
		}
		Vec3d delta;
		mapgeometry::SetComponent( delta, axis, target - at );
		if ( EditResult r = TransformObjects( edit, { id }, Affine::Translation( delta ), options ); !r )
		{
			return r;
		}
		moved = true;
	}
	if ( !moved )
	{
		return NothingToDo( "the objects are already aligned" );
	}
	return {};
}

} // namespace hammer::app::ops
