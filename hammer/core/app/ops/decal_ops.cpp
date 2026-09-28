//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/decal_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/decal_ops.h"

#include "mapgeometry/vec3.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <sstream>

namespace hammer::app::ops
{

using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

const char *const kCornerKeys[4] = { "uv0", "uv1", "uv2", "uv3" };

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	return a.size() == b.size() && std::equal( a.begin(), a.end(), b.begin(),
	                                   []( char x, char y )
	                                   {
		                                   return std::tolower( static_cast<unsigned char>( x ) ) ==
		                                          std::tolower( static_cast<unsigned char>( y ) );
	                                   } );
}

bool Finite( const Vec3d &v )
{
	return std::isfinite( v.x ) && std::isfinite( v.y ) && std::isfinite( v.z );
}

bool IsOverlay( const scene::Entity &e )
{
	return EqualsNoCase( e.classname, "info_overlay" );
}

const scene::Side *FindFace( const scene::DocumentReader &doc, const scene::FaceRef &face )
{
	const scene::Solid *solid = doc.FindSolid( face.solid );
	return solid ? solid->FindSide( face.side ) : nullptr;
}

// Checks the face list and removes duplicates (first kept). Empty on success,
// otherwise the refusal.
std::string CheckFaces( const scene::DocumentReader &doc, const std::vector<scene::FaceRef> &faces,
    std::vector<scene::FaceRef> &unique )
{
	if ( faces.empty() )
	{
		return "an overlay needs at least one face";
	}
	for ( const scene::FaceRef &face : faces )
	{
		if ( !FindFace( doc, face ) )
		{
			return "unknown face";
		}
		if ( std::find( unique.begin(), unique.end(), face ) == unique.end() )
		{
			unique.push_back( face );
		}
	}
	if ( unique.size() > kMaxOverlayFaces )
	{
		return "an overlay may name at most 64 faces";
	}
	return {};
}

std::string SidesValue( const std::vector<scene::FaceRef> &faces )
{
	std::string out;
	for ( std::size_t i = 0; i < faces.size(); ++i )
	{
		out += ( i ? " " : "" ) + std::to_string( faces[i].side );
	}
	return out;
}

std::optional<std::uint32_t> FirstSide( const scene::Entity &overlay )
{
	const std::string *sides = overlay.Key( "sides" );
	if ( !sides )
	{
		return std::nullopt;
	}
	std::istringstream in( *sides );
	unsigned long value = 0;
	if ( in >> value )
	{
		return static_cast<std::uint32_t>( value );
	}
	return std::nullopt;
}

bool SizeValid( double width, double height )
{
	return std::isfinite( width ) && std::isfinite( height ) && width > 0.0 && height > 0.0;
}

void WriteBasis( scene::Entity &e, const OverlayBasis &basis )
{
	e.SetKey( "StartU", scene::FormatNumber( basis.startU ) );
	e.SetKey( "EndU", scene::FormatNumber( basis.endU ) );
	e.SetKey( "StartV", scene::FormatNumber( basis.startV ) );
	e.SetKey( "EndV", scene::FormatNumber( basis.endV ) );
	e.SetKey( "BasisOrigin", scene::FormatVec3( basis.origin ) );
	e.SetKey( "BasisU", scene::FormatVec3( basis.u ) );
	e.SetKey( "BasisV", scene::FormatVec3( basis.v ) );
	e.SetKey( "BasisNormal", scene::FormatVec3( basis.normal ) );
}

// The legacy handle corners of a centered rectangle; 'third' keeps each
// corner's third component.
std::array<Vec3d, 4> Corners( double width, double height, const std::array<double, 4> &third )
{
	const double w = width / 2.0;
	const double h = height / 2.0;
	return { Vec3d( -w, -h, third[0] ), Vec3d( -w, h, third[1] ), Vec3d( w, h, third[2] ),
		Vec3d( w, -h, third[3] ) };
}

void WriteCorners( scene::Entity &e, const std::array<Vec3d, 4> &corners )
{
	for ( int i = 0; i < 4; ++i )
	{
		e.SetKey( kCornerKeys[i], scene::FormatVec3( corners[i] ) );
	}
}

std::optional<Vec3d> KeyVec3( const scene::Entity &e, std::string_view key )
{
	const std::string *value = e.Key( key );
	if ( !value )
	{
		return std::nullopt;
	}
	std::optional<Vec3d> v = mapgeometry::ParseVec3( *value );
	return v && Finite( *v ) ? v : std::nullopt;
}

int MajorAxis( const Vec3d &v )
{
	int axis = 0;
	if ( std::fabs( v.y ) > std::fabs( v.x ) )
	{
		axis = 1;
	}
	if ( std::fabs( v.z ) > std::fabs( mapgeometry::Component( v, axis ) ) )
	{
		axis = 2;
	}
	return axis;
}

} // namespace

std::optional<OverlayBasis> OverlayBasisFor( const mapgeometry::Plane &facePlane, const Vec3d &point )
{
	const double length = mapgeometry::Length( facePlane.normal );
	if ( !Finite( facePlane.normal ) || !std::isfinite( facePlane.dist ) || length <= 1e-12 ||
	     !Finite( point ) )
	{
		return std::nullopt;
	}
	OverlayBasis basis;
	basis.normal = facePlane.normal / length;
	const double dist = facePlane.dist / length;
	const int major = MajorAxis( basis.normal );
	const Vec3d initialU = major == 0 ? Vec3d( 0, 1, 0 ) : Vec3d( 1, 0, 0 );
	basis.v = mapgeometry::Normalize( mapgeometry::Cross( basis.normal, initialU ) );
	basis.u = mapgeometry::Normalize( mapgeometry::Cross( basis.v, basis.normal ) );
	basis.origin = point - basis.normal * ( mapgeometry::Dot( basis.normal, point ) - dist );

	const bool uPositive = mapgeometry::Component( basis.u, MajorAxis( basis.u ) ) >= 0.0;
	const bool vPositive = mapgeometry::Component( basis.v, MajorAxis( basis.v ) ) >= 0.0;
	if ( uPositive != vPositive )
	{
		basis.startU = 1.0;
		basis.endU = 0.0;
		basis.startV = 0.0;
		basis.endV = 1.0;
	}
	return basis;
}

EditResult PlaceDecal( scene::DocumentEdit &edit, const scene::FaceRef &face, const Vec3d &point,
    const std::string &material, ObjectId &created )
{
	if ( !FindFace( edit, face ) )
	{
		return Reject( "unknown face" );
	}
	if ( material.empty() )
	{
		return Reject( "a decal needs a material" );
	}
	if ( !Finite( point ) )
	{
		return Reject( "the decal position must be finite" );
	}
	scene::Entity decal;
	decal.classname = "infodecal";
	decal.SetKey( "texture", material );
	decal.SetOrigin( point );
	created = edit.Add( std::move( decal ) );
	return {};
}

EditResult PlaceOverlay( scene::DocumentEdit &edit, const std::vector<scene::FaceRef> &faces,
    const Vec3d &point, const std::string &material, double width, double height,
    ObjectId &created )
{
	std::vector<scene::FaceRef> unique;
	if ( std::string refusal = CheckFaces( edit, faces, unique ); !refusal.empty() )
	{
		return Reject( std::move( refusal ) );
	}
	if ( material.empty() )
	{
		return Reject( "an overlay needs a material" );
	}
	if ( !SizeValid( width, height ) )
	{
		return Reject( "the overlay size must be positive" );
	}
	if ( !Finite( point ) )
	{
		return Reject( "the overlay position must be finite" );
	}
	const std::optional<OverlayBasis> basis =
	    OverlayBasisFor( FindFace( edit, unique.front() )->Plane(), point );
	if ( !basis )
	{
		return Reject( "the first face is degenerate" );
	}
	scene::Entity overlay;
	overlay.classname = "info_overlay";
	overlay.SetKey( "material", material );
	overlay.SetKey( "sides", SidesValue( unique ) );
	overlay.SetKey( "RenderOrder", "0" );
	WriteBasis( overlay, *basis );
	WriteCorners( overlay, Corners( width, height, { 0, 0, 0, 0 } ) );
	overlay.SetOrigin( point );
	created = edit.Add( std::move( overlay ) );
	return {};
}

EditResult SetOverlayFaces(
    scene::DocumentEdit &edit, ObjectId overlay, const std::vector<scene::FaceRef> &faces )
{
	const scene::Entity *current = edit.FindEntity( overlay );
	if ( !current || !IsOverlay( *current ) )
	{
		return Reject( "not an overlay" );
	}
	std::vector<scene::FaceRef> unique;
	if ( std::string refusal = CheckFaces( edit, faces, unique ); !refusal.empty() )
	{
		return Reject( std::move( refusal ) );
	}
	scene::Entity next = *current;
	next.SetKey( "sides", SidesValue( unique ) );
	if ( FirstSide( *current ) != unique.front().side )
	{
		std::optional<Vec3d> origin = current->Origin();
		if ( !origin )
		{
			origin = KeyVec3( *current, "BasisOrigin" );
		}
		if ( !origin )
		{
			return Reject( "the overlay has no origin to rebuild its basis about" );
		}
		const std::optional<OverlayBasis> basis =
		    OverlayBasisFor( FindFace( edit, unique.front() )->Plane(), *origin );
		if ( !basis )
		{
			return Reject( "the first face is degenerate" );
		}
		WriteBasis( next, *basis );
	}
	if ( next == *current )
	{
		return NothingToDo( "the overlay already uses these faces" );
	}
	*edit.MutableEntity( overlay ) = std::move( next );
	return {};
}

EditResult SetOverlaySize( scene::DocumentEdit &edit, ObjectId overlay, double width, double height )
{
	const scene::Entity *current = edit.FindEntity( overlay );
	if ( !current || !IsOverlay( *current ) )
	{
		return Reject( "not an overlay" );
	}
	if ( !SizeValid( width, height ) )
	{
		return Reject( "the overlay size must be positive" );
	}
	std::array<double, 4> third = { 0, 0, 0, 0 };
	for ( int i = 0; i < 4; ++i )
	{
		if ( const std::optional<Vec3d> corner = KeyVec3( *current, kCornerKeys[i] ) )
		{
			third[i] = corner->z;
		}
	}
	scene::Entity next = *current;
	WriteCorners( next, Corners( width, height, third ) );
	if ( next == *current )
	{
		return NothingToDo( "the overlay already has this size" );
	}
	*edit.MutableEntity( overlay ) = std::move( next );
	return {};
}

scene::Entity TransformedOverlay( const scene::Entity &overlay, const mapgeometry::Affine &xf )
{
	scene::Entity out = overlay;
	if ( !IsOverlay( overlay ) )
	{
		return out;
	}
	const std::optional<Vec3d> origin = KeyVec3( overlay, "BasisOrigin" );
	const std::optional<Vec3d> u0 = KeyVec3( overlay, "BasisU" );
	const std::optional<Vec3d> v0 = KeyVec3( overlay, "BasisV" );
	const std::optional<Vec3d> n0 = KeyVec3( overlay, "BasisNormal" );
	if ( !origin || !u0 || !v0 || !n0 || mapgeometry::Length( *u0 ) <= 1e-12 ||
	     mapgeometry::Length( *v0 ) <= 1e-12 )
	{
		return out;
	}
	out.SetKey( "BasisOrigin", scene::FormatVec3( xf.Point( *origin ) ) );
	if ( xf.IsTranslation() )
	{
		return out;
	}
	const Vec3d u = mapgeometry::Normalize( *u0 );
	const Vec3d v = mapgeometry::Normalize( *v0 );
	const Vec3d tu = xf.Direction( u );
	const Vec3d tv = xf.Direction( v );
	const Vec3d tn = xf.Direction( *n0 );
	auto unit = []( const Vec3d &a )
	{
		return std::fabs( mapgeometry::Length( a ) - 1.0 ) <= 1e-4;
	};
	auto perpendicular = []( const Vec3d &a, const Vec3d &b )
	{
		return std::fabs( mapgeometry::Dot( a, b ) ) <= 0.0025;
	};
	if ( unit( tu ) && unit( tv ) && unit( tn ) && perpendicular( tu, tv ) &&
	     perpendicular( tu, tn ) && perpendicular( tv, tn ) )
	{
		out.SetKey( "BasisU", scene::FormatVec3( tu ) );
		out.SetKey( "BasisV", scene::FormatVec3( tv ) );
		out.SetKey( "BasisNormal", scene::FormatVec3( tn ) );
		return out;
	}
	// Scale or shear: keep the (normalized) axes and move the corners.
	out.SetKey( "BasisU", scene::FormatVec3( u ) );
	out.SetKey( "BasisV", scene::FormatVec3( v ) );
	for ( int i = 0; i < 4; ++i )
	{
		const std::optional<Vec3d> uv = KeyVec3( overlay, kCornerKeys[i] );
		if ( !uv )
		{
			continue;
		}
		const Vec3d moved = xf.Direction( u * uv->x + v * uv->y );
		out.SetKey( kCornerKeys[i],
		    scene::FormatVec3( Vec3d( mapgeometry::Dot( u, moved ), mapgeometry::Dot( v, moved ), uv->z ) ) );
	}
	return out;
}

} // namespace hammer::app::ops
