//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app decal and overlay placement (RFC 0002, R08 domain
//			logic): an infodecal carries "texture" at the point; an
//			info_overlay gets the legacy basis from its first face (normal,
//			initial U by the normal's major axis, V = N x U, U = V x N; origin
//			projected onto the face; texture-coordinate flip by the axes'
//			signs), centered uv corners, the face side ids and the full key
//			set. The basis is orthonormal and in-plane for every box face and
//			a sloped plane. SetOverlayFaces keeps the basis unless the first
//			face changes; SetOverlaySize rewrites the corners and keeps their
//			flip bits; TransformedOverlay rotates the basis, or moves corners
//			under a scale. Negative checks: unknown faces, no faces, too many
//			faces, empty material, bad sizes, non-overlays and non-finite
//			points stage nothing.
//
//=============================================================================//

#include "hammer/app/ops/decal_ops.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <cmath>
#include <limits>

using namespace hammer;
using namespace hammer::app;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

bool Near( const Vec3d &a, const Vec3d &b, double tolerance = 1e-9 )
{
	return std::fabs( a.x - b.x ) <= tolerance && std::fabs( a.y - b.y ) <= tolerance &&
	       std::fabs( a.z - b.z ) <= tolerance;
}

Vec3d KeyVec( const scene::Entity &e, const char *key )
{
	return mapgeometry::ParseVec3( *e.Key( key ) ).value_or( Vec3d( 1e9, 1e9, 1e9 ) );
}

bool Orthonormal( const OverlayBasis &b, const Vec3d &normal )
{
	using mapgeometry::Dot;
	using mapgeometry::Length;
	return std::fabs( Length( b.u ) - 1 ) < 1e-12 && std::fabs( Length( b.v ) - 1 ) < 1e-12 &&
	       std::fabs( Length( b.normal ) - 1 ) < 1e-12 && std::fabs( Dot( b.u, b.v ) ) < 1e-12 &&
	       std::fabs( Dot( b.u, normal ) ) < 1e-12 && std::fabs( Dot( b.v, normal ) ) < 1e-12 &&
	       Near( b.normal, mapgeometry::Normalize( normal ) );
}

// The side of 'solid' whose outward normal is 'normal'.
std::uint32_t SideFacing( const scene::Solid &solid, const Vec3d &normal )
{
	for ( const scene::Side &side : solid.sides )
	{
		if ( Near( side.Plane().normal, normal, 1e-6 ) )
		{
			return side.vmfId;
		}
	}
	return 0;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::MapDocument doc;
	ObjectId box;
	std::vector<ObjectId> many;
	{
		scene::DocumentEdit edit( doc );
		box = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 16 ) }, tex ) );
		for ( int i = 0; i < 11; ++i )
		{
			many.push_back( edit.Add( scene::MakeBoxSolid(
			    { Vec3d( 200 + 32 * i, 0, 0 ), Vec3d( 216 + 32 * i, 16, 16 ) }, tex ) ) );
		}
		scene::Entity light;
		light.classname = "light";
		light.SetOrigin( Vec3d( 0, 0, 64 ) );
		edit.Add( light );
		scene::CommitEdit( doc, edit );
	}
	const scene::Solid &solid = *doc.FindSolid( box );
	const scene::FaceRef top{ box, SideFacing( solid, Vec3d( 0, 0, 1 ) ) };
	const scene::FaceRef bottom{ box, SideFacing( solid, Vec3d( 0, 0, -1 ) ) };
	const scene::FaceRef east{ box, SideFacing( solid, Vec3d( 1, 0, 0 ) ) };
	const scene::FaceRef south{ box, SideFacing( solid, Vec3d( 0, -1, 0 ) ) };
	checks.That( top.side && bottom.side && east.side && south.side, "fixture faces" );

	// The legacy basis rule.
	for ( const scene::Side &side : solid.sides )
	{
		const std::optional<OverlayBasis> b = OverlayBasisFor( side.Plane(), Vec3d( 10, 20, 30 ) );
		checks.That( b && Orthonormal( *b, side.Plane().normal ),
		    "orthonormal, in-plane basis on every box face" );
	}
	{
		const Vec3d n = mapgeometry::Normalize( Vec3d( 1, 2, 3 ) );
		const std::optional<OverlayBasis> b = OverlayBasisFor( { n, 5.0 }, Vec3d( 7, 7, 7 ) );
		checks.That( b && Orthonormal( *b, n ), "orthonormal basis on a sloped face" );
		checks.That( b && std::fabs( mapgeometry::Dot( b->origin, n ) - 5.0 ) < 1e-9,
		    "the basis origin is projected onto the face plane" );
		checks.That( !OverlayBasisFor( { Vec3d(), 0.0 }, Vec3d() ),
		    "a zero normal has no basis (negative)" );
	}
	{
		const std::optional<OverlayBasis> up =
		    OverlayBasisFor( { Vec3d( 0, 0, 1 ), 16 }, Vec3d( 8, 8, 20 ) );
		checks.That( up && up->u == Vec3d( 1, 0, 0 ) && up->v == Vec3d( 0, 1, 0 ) &&
		                 up->origin == Vec3d( 8, 8, 16 ),
		    "floor: U = +X (major axis Z), V = +Y" );
		checks.That( up && up->startU == 0 && up->endU == 1 && up->startV == 1 && up->endV == 0,
		    "same-sign axes keep the default texture coordinates" );
		const std::optional<OverlayBasis> wall =
		    OverlayBasisFor( { Vec3d( 1, 0, 0 ), 64 }, Vec3d( 64, 8, 8 ) );
		checks.That( wall && wall->u == Vec3d( 0, 1, 0 ) && wall->v == Vec3d( 0, 0, 1 ),
		    "X-facing wall: U = +Y (major axis X), V = +Z" );
		const std::optional<OverlayBasis> down =
		    OverlayBasisFor( { Vec3d( 0, 0, -1 ), 0 }, Vec3d() );
		checks.That( down && down->v == Vec3d( 0, -1, 0 ) && down->startU == 1 && down->endU == 0 &&
		                 down->startV == 0 && down->endV == 1,
		    "ceiling: V = -Y, opposite signs flip the texture coordinates" );
	}

	// Decal.
	{
		scene::DocumentEdit edit( doc );
		ObjectId decal;
		checks.That( PlaceDecal( edit, top, Vec3d( 8, 8, 16 ), "decals/lambda", decal ).HasValue(),
		    "place a decal" );
		const scene::Entity *e = edit.FindEntity( decal );
		checks.That( e && e->classname == "infodecal" && *e->Key( "texture" ) == "decals/lambda" &&
		                 *e->Origin() == Vec3d( 8, 8, 16 ),
		    "infodecal with texture at the point" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after the decal" );
	}

	// Overlay.
	ObjectId overlay;
	scene::MapDocument withOverlay = doc;
	{
		scene::DocumentEdit edit( withOverlay );
		checks.That( PlaceOverlay( edit, { top, top, east }, Vec3d( 32, 32, 16 ), "decals/x", 32,
		                 16, overlay )
		                 .HasValue(),
		    "place an overlay" );
		const scene::Entity *e = edit.FindEntity( overlay );
		checks.That( e && e->classname == "info_overlay", "info_overlay" );
		checks.Equal( *e->Key( "sides" ),
		    std::to_string( top.side ) + " " + std::to_string( east.side ),
		    "sides = the face ids, duplicates collapsed" );
		checks.Equal( *e->Key( "material" ), std::string( "decals/x" ), "material" );
		checks.Equal( *e->Key( "RenderOrder" ), std::string( "0" ), "render order" );
		checks.That( KeyVec( *e, "BasisOrigin" ) == Vec3d( 32, 32, 16 ) &&
		                 KeyVec( *e, "BasisU" ) == Vec3d( 1, 0, 0 ) &&
		                 KeyVec( *e, "BasisV" ) == Vec3d( 0, 1, 0 ) &&
		                 KeyVec( *e, "BasisNormal" ) == Vec3d( 0, 0, 1 ),
		    "basis from the first face" );
		checks.That( *e->Key( "StartU" ) == "0" && *e->Key( "EndU" ) == "1" &&
		                 *e->Key( "StartV" ) == "1" && *e->Key( "EndV" ) == "0",
		    "texture coordinates" );
		checks.That( *e->Key( "uv0" ) == "-16 -8 0" && *e->Key( "uv1" ) == "-16 8 0" &&
		                 *e->Key( "uv2" ) == "16 8 0" && *e->Key( "uv3" ) == "16 -8 0",
		    "centered corners in legacy handle order" );
		checks.That( e->Origin() && *e->Origin() == Vec3d( 32, 32, 16 ), "origin at the point" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after the overlay" );
		scene::CommitEdit( withOverlay, edit );
	}

	// Face list and size edits.
	{
		scene::DocumentEdit edit( withOverlay );
		checks.That(
		    SetOverlayFaces( edit, overlay, { top, east, south } ).HasValue(), "add a face" );
		const scene::Entity *e = edit.FindEntity( overlay );
		checks.That( KeyVec( *e, "BasisNormal" ) == Vec3d( 0, 0, 1 ) &&
		                 *e->Key( "sides" ) == std::to_string( top.side ) + " " +
		                                           std::to_string( east.side ) + " " +
		                                           std::to_string( south.side ),
		    "same first face: sides change, basis kept" );
		checks.That( SetOverlayFaces( edit, overlay, { top, east, south } ).Error().code ==
		                 EditErrorCode::Nothing,
		    "unchanged faces are nothing" );
		checks.That( SetOverlayFaces( edit, overlay, { east } ).HasValue(), "new first face" );
		e = edit.FindEntity( overlay );
		checks.That( KeyVec( *e, "BasisNormal" ) == Vec3d( 1, 0, 0 ) &&
		                 KeyVec( *e, "BasisU" ) == Vec3d( 0, 1, 0 ) &&
		                 KeyVec( *e, "BasisOrigin" ) == Vec3d( 64, 32, 16 ) &&
		                 *e->Key( "uv0" ) == "-16 -8 0",
		    "a new first face rebuilds the basis about the origin and keeps the corners" );
		e = nullptr;
		edit.MutableEntity( overlay )->SetKey( "uv0", "-16 -8 1" );
		checks.That( SetOverlaySize( edit, overlay, 64, 8 ).HasValue(), "resize" );
		e = edit.FindEntity( overlay );
		checks.That( *e->Key( "uv0" ) == "-32 -4 1" && *e->Key( "uv2" ) == "32 4 0",
		    "resized corners keep their flip bits" );
		checks.That( SetOverlaySize( edit, overlay, 64, 8 ).Error().code == EditErrorCode::Nothing,
		    "same size is nothing" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after edits" );
	}

	// TransformedOverlay.
	{
		const scene::Entity &e = *withOverlay.FindEntity( overlay );
		const scene::Entity turned = TransformedOverlay(
		    e, mapgeometry::Affine::About( mapgeometry::AngleMatrix( 0, 90, 0 ), Vec3d() ) );
		checks.That( *turned.Key( "BasisU" ) == "0 1 0" && *turned.Key( "BasisV" ) == "-1 0 0" &&
		                 *turned.Key( "BasisOrigin" ) == "-32 32 16" &&
		                 *turned.Key( "uv0" ) == "-16 -8 0",
		    "a rotation turns the basis and keeps the corners" );
		mapgeometry::Affine scale;
		scale.linear = mapgeometry::Mat3::Scale( Vec3d( 2, 1, 1 ) );
		const scene::Entity scaled = TransformedOverlay( e, scale );
		checks.That( *scaled.Key( "BasisU" ) == "1 0 0" && *scaled.Key( "uv0" ) == "-32 -8 0" &&
		                 *scaled.Key( "BasisOrigin" ) == "64 32 16",
		    "a scale keeps the axes and moves the corners" );
		const scene::Entity moved =
		    TransformedOverlay( e, mapgeometry::Affine::Translation( Vec3d( 1, 2, 3 ) ) );
		checks.That( *moved.Key( "BasisOrigin" ) == "33 34 19" && *moved.Key( "BasisU" ) == "1 0 0",
		    "a translation moves only the origin" );
		const scene::Entity &light = *withOverlay.FindEntity( withOverlay.EntityIds()[0] );
		checks.That( TransformedOverlay( light, scale ) == light, "other entities are unchanged" );
	}

	// Refusals stage nothing.
	{
		scene::DocumentEdit edit( withOverlay );
		ObjectId out;
		const scene::FaceRef unknown{ box, 99999 };
		const double nan = std::numeric_limits<double>::quiet_NaN();
		checks.That(
		    PlaceDecal( edit, unknown, Vec3d(), "d", out ).Error().code == EditErrorCode::Rejected,
		    "decal on an unknown face (negative)" );
		checks.That(
		    PlaceDecal( edit, top, Vec3d(), "", out ).Error().code == EditErrorCode::Rejected,
		    "decal without material (negative)" );
		checks.That( PlaceDecal( edit, top, Vec3d( nan, 0, 0 ), "d", out ).Error().code ==
		                 EditErrorCode::Rejected,
		    "decal at NaN (negative)" );
		checks.That( PlaceOverlay( edit, {}, Vec3d(), "d", 1, 1, out ).Error().code ==
		                 EditErrorCode::Rejected,
		    "overlay without faces (negative)" );
		checks.That( PlaceOverlay( edit, { top, unknown }, Vec3d(), "d", 1, 1, out ).Error().code ==
		                 EditErrorCode::Rejected,
		    "overlay on an unknown face (negative)" );
		checks.That( PlaceOverlay( edit, { top }, Vec3d(), "", 1, 1, out ).Error().code ==
		                 EditErrorCode::Rejected,
		    "overlay without material (negative)" );
		checks.That( PlaceOverlay( edit, { top }, Vec3d(), "d", 0, 1, out ).Error().code ==
		                 EditErrorCode::Rejected,
		    "zero width (negative)" );
		checks.That( PlaceOverlay( edit, { top }, Vec3d(), "d", 1, -2, out ).Error().code ==
		                 EditErrorCode::Rejected,
		    "negative height (negative)" );
		checks.That( PlaceOverlay( edit, { top }, Vec3d(), "d", nan, 1, out ).Error().code ==
		                 EditErrorCode::Rejected,
		    "NaN width (negative)" );
		checks.That(
		    PlaceOverlay( edit, { top }, Vec3d( 0, nan, 0 ), "d", 1, 1, out ).Error().code ==
		        EditErrorCode::Rejected,
		    "overlay at NaN (negative)" );
		std::vector<scene::FaceRef> tooMany;
		for ( ObjectId id : many )
		{
			for ( const scene::Side &side : withOverlay.FindSolid( id )->sides )
			{
				tooMany.push_back( { id, side.vmfId } );
			}
		}
		checks.That( tooMany.size() == 66 &&
		                 PlaceOverlay( edit, tooMany, Vec3d(), "d", 1, 1, out ).Error().code ==
		                     EditErrorCode::Rejected,
		    "more than 64 faces (negative)" );
		tooMany.resize( kMaxOverlayFaces );
		ObjectId big;
		checks.That( PlaceOverlay( edit, tooMany, Vec3d( 200, 0, 16 ), "d", 1, 1, big ).HasValue(),
		    "exactly 64 faces are accepted" );
		const ObjectId lightId = withOverlay.EntityIds()[0];
		checks.That(
		    SetOverlayFaces( edit, lightId, { top } ).Error().code == EditErrorCode::Rejected,
		    "faces of a non-overlay (negative)" );
		checks.That( SetOverlaySize( edit, lightId, 1, 1 ).Error().code == EditErrorCode::Rejected,
		    "size of a non-overlay (negative)" );
		checks.That( SetOverlayFaces( edit, overlay, {} ).Error().code == EditErrorCode::Rejected,
		    "an empty face list (negative)" );
		checks.That(
		    SetOverlayFaces( edit, overlay, { unknown } ).Error().code == EditErrorCode::Rejected,
		    "an unknown face in the list (negative)" );
		checks.That( SetOverlaySize( edit, overlay, 0, 1 ).Error().code == EditErrorCode::Rejected,
		    "a zero size (negative)" );
		const scene::ChangeSet staged = edit.Finish();
		checks.That(
		    staged.objects.size() == 1 && staged.objects[0].id == big, "refusals staged nothing" );
	}

	return checks.Report();
}
