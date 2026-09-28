//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/create_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/create_ops.h"

#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"

#include <cmath>
#include <cstdlib>

namespace hammer::app::ops
{

using mapgeometry::Vec3d;

namespace
{

constexpr double kPi = 3.14159265358979323846;

double Radians( double degrees )
{
	return degrees * kPi / 180.0;
}

// Maps local coordinates (x, y across, z along the primitive's axis) to world.
Vec3d ToWorld( int axis, double a, double b, double h )
{
	switch ( axis )
	{
	case 0:
		return Vec3d( h, a, b );
	case 1:
		return Vec3d( b, h, a );
	default:
		return Vec3d( a, b, h );
	}
}

// Legacy polyMake: 'n' points on the ellipse inscribed in [x1,x2]x[y1,y2],
// starting at +Y and advancing clockwise (sin for x, cos for y), rounded.
std::vector<std::pair<double, double>> PolyMake( double x1, double y1, double x2, double y2, int n )
{
	std::vector<std::pair<double, double>> out;
	const double xrad = ( x2 - x1 ) / 2.0;
	const double yrad = ( y2 - y1 ) / 2.0;
	const double cx = x1 + xrad;
	const double cy = y1 + yrad;
	for ( int i = 0; i < n; ++i )
	{
		const double angle = 360.0 * i / n;
		out.emplace_back( std::round( cx + std::sin( Radians( angle ) ) * xrad ),
		    std::round( cy + std::cos( Radians( angle ) ) * yrad ) );
	}
	return out;
}

// A solid from the convex hull of 'points', textured per face.
std::optional<scene::Solid> HullSolid(
    const std::vector<Vec3d> &points, const scene::FaceTexture &texture )
{
	const std::optional<std::vector<mapgeometry::Plane>> planes =
	    mapgeometry::ConvexHullPlanes( points );
	if ( !planes )
	{
		return std::nullopt;
	}
	scene::Solid solid;
	for ( const mapgeometry::Plane &p : *planes )
	{
		scene::Side side;
		side.points = scene::PointsFromPlane( p );
		side.texture = scene::WorldAlignedTexture( texture, p.normal );
		solid.sides.push_back( std::move( side ) );
	}
	return scene::NormalizeSides( solid );
}

// Local extents of 'box' for a primitive along 'axis': (a0,a1) and (b0,b1)
// across, (h0,h1) along.
struct Local
{
	double a0, a1, b0, b1, h0, h1;
};

Local LocalOf( const scene::Box &box, int axis )
{
	const int ia = axis == 2 ? 0 : ( axis == 0 ? 1 : 2 );
	const int ib = axis == 2 ? 1 : ( axis == 0 ? 2 : 0 );
	return { mapgeometry::Component( box.mins, ia ), mapgeometry::Component( box.maxs, ia ),
	    mapgeometry::Component( box.mins, ib ), mapgeometry::Component( box.maxs, ib ),
	    mapgeometry::Component( box.mins, axis ), mapgeometry::Component( box.maxs, axis ) };
}

} // namespace

std::optional<scene::Solid> MakePrimitive(
    const PrimitiveSpec &spec, const scene::Box &box, const scene::FaceTexture &texture )
{
	const Vec3d size = box.Size();
	if ( size.x <= 0.0 || size.y <= 0.0 || size.z <= 0.0 || spec.axis < 0 || spec.axis > 2 )
	{
		return std::nullopt;
	}
	const bool needsSides = spec.kind == PrimitiveKind::Cylinder ||
	                        spec.kind == PrimitiveKind::Spike || spec.kind == PrimitiveKind::Sphere;
	if ( needsSides && ( spec.sides < 3 || spec.sides > 32 ) )
	{
		return std::nullopt;
	}
	const Local l = LocalOf( box, spec.axis );
	std::vector<Vec3d> points;
	switch ( spec.kind )
	{
	case PrimitiveKind::Block:
		return scene::MakeBoxSolid( box, texture );
	case PrimitiveKind::Wedge:
		// Full height along -B, a sloped top down to the +B bottom edge.
		for ( double a : { l.a0, l.a1 } )
		{
			points.push_back( ToWorld( spec.axis, a, l.b0, l.h0 ) );
			points.push_back( ToWorld( spec.axis, a, l.b1, l.h0 ) );
			points.push_back( ToWorld( spec.axis, a, l.b0, l.h1 ) );
		}
		break;
	case PrimitiveKind::Cylinder:
		for ( const auto &[a, b] : PolyMake( l.a0, l.b0, l.a1, l.b1, spec.sides ) )
		{
			points.push_back( ToWorld( spec.axis, a, b, l.h0 ) );
			points.push_back( ToWorld( spec.axis, a, b, l.h1 ) );
		}
		break;
	case PrimitiveKind::Spike:
		for ( const auto &[a, b] : PolyMake( l.a0, l.b0, l.a1, l.b1, spec.sides ) )
		{
			points.push_back( ToWorld( spec.axis, a, b, l.h0 ) );
		}
		points.push_back( ToWorld( spec.axis, std::round( ( l.a0 + l.a1 ) / 2 ),
		    std::round( ( l.b0 + l.b1 ) / 2 ), l.h1 ) );
		break;
	case PrimitiveKind::Sphere:
	{
		// Legacy slices: rings at constant angular steps from pole to pole.
		const double ca = ( l.a0 + l.a1 ) / 2;
		const double cb = ( l.b0 + l.b1 ) / 2;
		const double ch = ( l.h0 + l.h1 ) / 2;
		const double ra = ( l.a1 - l.a0 ) / 2;
		const double rb = ( l.b1 - l.b0 ) / 2;
		const double rh = ( l.h1 - l.h0 ) / 2;
		const double step = 180.0 / spec.sides;
		points.push_back( ToWorld( spec.axis, std::round( ca ), std::round( cb ), l.h1 ) );
		points.push_back( ToWorld( spec.axis, std::round( ca ), std::round( cb ), l.h0 ) );
		for ( int ring = 1; ring < spec.sides; ++ring )
		{
			const double angle = step * ring;
			const double s = std::sin( Radians( angle ) );
			const double h = std::round( ch + rh * std::cos( Radians( angle ) ) );
			for ( const auto &[a, b] :
			    PolyMake( ca - ra * s, cb - rb * s, ca + ra * s, cb + rb * s, spec.sides ) )
			{
				points.push_back( ToWorld( spec.axis, a, b, h ) );
			}
		}
		break;
	}
	}
	return HullSolid( points, texture );
}

EditResult CreatePrimitive( scene::DocumentEdit &edit, const PrimitiveSpec &spec,
    const scene::Box &box, const scene::FaceTexture &texture, scene::ObjectId &created )
{
	if ( texture.material.empty() )
	{
		return Reject( "no material" );
	}
	std::optional<scene::Solid> solid = MakePrimitive( spec, box, texture );
	if ( !solid )
	{
		return Reject( "the primitive does not fit the box (degenerate box or side count)" );
	}
	created = edit.Add( std::move( *solid ) );
	return {};
}

EditResult CreateArch( scene::DocumentEdit &edit, const ArchSpec &spec, const scene::Box &box,
    const scene::FaceTexture &texture, scene::ObjectId &created,
    std::vector<scene::ObjectId> *segments )
{
	const Vec3d size = box.Size();
	if ( size.x <= 0.0 || size.y <= 0.0 || spec.sides < 3 || spec.sides > 128 || spec.arc <= 0.0 ||
	     spec.arc > 360.0 || spec.wallWidth <= 0.0 )
	{
		return Reject( "invalid arch parameters" );
	}
	if ( texture.material.empty() )
	{
		return Reject( "no material" );
	}
	const double zMin = box.mins.z;
	const double zMax = size.z < 1.0 ? box.mins.z + 1.0 : box.maxs.z;
	const Vec3d center = box.Center();
	const double outerX = size.x / 2;
	const double outerY = size.y / 2;
	// Legacy: a wall of half the box or more collapses the inside to the center.
	const bool solidCore = spec.wallWidth * 2 + 8 >= size.x || spec.wallWidth * 2 + 8 >= size.y;
	const double innerX = solidCore ? 0.0 : outerX - spec.wallWidth;
	const double innerY = solidCore ? 0.0 : outerY - spec.wallWidth;

	auto arcPoint = [&]( double rx, double ry, int i )
	{
		const double angle = spec.startAngle + spec.arc * i / spec.sides;
		return std::pair<double, double>(
		    std::round( center.x + std::cos( Radians( angle ) ) * rx ),
		    std::round( center.y + std::sin( Radians( angle ) ) * ry ) );
	};

	std::vector<scene::Solid> built;
	for ( int i = 0; i < spec.sides; ++i )
	{
		const double lift = spec.addHeight * i;
		std::vector<Vec3d> points;
		for ( int k : { i, i + 1 } )
		{
			const auto [ox, oy] = arcPoint( outerX, outerY, k );
			const auto [ix, iy] = arcPoint( innerX, innerY, k );
			for ( double z : { zMin + lift, zMax + lift } )
			{
				points.push_back( Vec3d( ox, oy, z ) );
				points.push_back( Vec3d( ix, iy, z ) );
			}
		}
		std::optional<scene::Solid> segment = HullSolid( points, texture );
		if ( !segment )
		{
			return Reject( "arch segment " + std::to_string( i ) +
			               " is degenerate; use more space or fewer sides" );
		}
		built.push_back( std::move( *segment ) );
	}
	created = edit.Add( scene::Group{} );
	for ( scene::Solid &s : built )
	{
		s.group = created;
		const scene::ObjectId id = edit.Add( std::move( s ) );
		if ( segments )
		{
			segments->push_back( id );
		}
	}
	return {};
}

void ApplyClassDefaults( scene::Entity &entity, const ports::EntityClassInfo &info )
{
	for ( const ports::KeyDefinition &key : info.keys )
	{
		if ( entity.Key( key.key ) )
		{
			continue;
		}
		if ( key.type == ports::KeyType::Flags )
		{
			long long flags = 0;
			for ( const ports::KeyChoice &c : key.choices )
			{
				if ( c.defaultOn )
				{
					flags += std::atoll( c.value.c_str() );
				}
			}
			entity.SetKey( key.key, std::to_string( flags ) );
		}
		else if ( !key.defaultValue.empty() )
		{
			entity.SetKey( key.key, key.defaultValue );
		}
	}
}

EditResult PlaceEntity( scene::DocumentEdit &edit, const std::string &classname,
    const Vec3d &origin, const ports::IEntityCatalog *catalog, scene::ObjectId &created )
{
	if ( classname.empty() )
	{
		return Reject( "no entity class" );
	}
	scene::Entity entity;
	entity.classname = classname;
	if ( catalog )
	{
		const ports::EntityClassInfo *info = catalog->Find( classname );
		if ( !info )
		{
			return Reject( "unknown entity class '" + classname + "'" );
		}
		if ( info->kind == ports::EntityClassKind::Solid )
		{
			return Reject( "'" + classname + "' is a brush entity; tie solids to it instead" );
		}
		entity.classname = info->name; // the catalog's spelling
		ApplyClassDefaults( entity, *info );
	}
	entity.SetOrigin( origin );
	created = edit.Add( std::move( entity ) );
	return {};
}

} // namespace hammer::app::ops
