//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app CSG operations (RFC 0002, R08 domain logic): clipping by
//			a plane (keep front, back or both; untouched solids; ids, owners and
//			side ids preserved), carving (pieces cover target minus carver
//			exactly by volume, carver faces texture the cuts, contained targets
//			vanish, carvers survive), and hollowing (six walls for a box inside
//			a new group, walls outward for negative thickness, brush entities
//			keep ownership). Negative checks: degenerate planes, no overlap,
//			too-thin solids refuse the whole selection, zero thickness.
//
//=============================================================================//

#include "hammer/app/ops/csg_ops.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <cmath>

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;

namespace
{

// Volume of a convex solid by summing pyramids from its centroid.
double Volume( const scene::Solid &s )
{
	const mapgeometry::BrushSolid g = scene::BuildGeometry( s );
	const std::vector<Vec3d> verts = mapgeometry::SolidVertices( g );
	Vec3d c;
	for ( const Vec3d &v : verts )
		c += v;
	c = c / static_cast<double>( verts.size() );
	double vol = 0;
	for ( const mapgeometry::BrushFace &f : g.faces )
		for ( std::size_t i = 1; i + 1 < f.vertices.size(); ++i )
			vol += std::fabs( mapgeometry::Dot( f.vertices[0] - c,
			           mapgeometry::Cross( f.vertices[i] - c, f.vertices[i + 1] - c ) ) ) /
			       6.0;
	return vol;
}

double TotalVolume( const scene::DocumentReader &doc, const std::vector<scene::ObjectId> &ids )
{
	double v = 0;
	for ( scene::ObjectId id : ids )
		if ( const scene::Solid *s = doc.FindSolid( id ) )
			v += Volume( *s );
	return v;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::FaceTexture cap;
	cap.material = "TOOLS/TOOLSNODRAW";

	scene::MapDocument doc;
	scene::ObjectId box, far, door, doorSolid;
	{
		scene::DocumentEdit edit( doc );
		box = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 128, 64, 64 ) }, tex ) );
		far = edit.Add( scene::MakeBoxSolid( { Vec3d( 500, 0, 0 ), Vec3d( 564, 64, 64 ) }, tex ) );
		scene::Entity d;
		d.classname = "func_door";
		door = edit.Add( d );
		scene::Solid ds = scene::MakeBoxSolid( { Vec3d( 0, 200, 0 ), Vec3d( 64, 264, 64 ) }, tex );
		ds.owner = door;
		doorSolid = edit.Add( ds );
		scene::CommitEdit( doc, edit );
	}
	const std::uint32_t boxTopSide = doc.FindSolid( box )->sides[0].vmfId;
	const mapgeometry::Plane x64 = mapgeometry::PlaneThrough( Vec3d( 64, 0, 0 ), Vec3d( 1, 0, 0 ) );

	// Clip keep back: the box becomes 0..64, same id, top side id kept.
	{
		scene::DocumentEdit edit( doc );
		checks.That( ClipSolids( edit, { box, far }, x64, ClipKeep::Back, cap ).HasValue(),
		    "clip keep back" );
		const std::optional<scene::Box> b = scene::SolidBounds( *edit.FindSolid( box ) );
		checks.That( b && b->maxs == Vec3d( 64, 64, 64 ), "back piece bounds" );
		checks.That( edit.FindSolid( box )->FindSide( boxTopSide ) != nullptr,
		    "untouched sides keep their ids" );
		checks.That(
		    *edit.FindSolid( far ) == *doc.FindSolid( far ), "a solid off the plane is untouched" );
		int caps = 0;
		for ( const scene::Side &s : edit.FindSolid( box )->sides )
			caps += s.texture.material == cap.material;
		checks.Equal( caps, 1, "one cap face with the cap material" );
		checks.That( scene::ValidateEdit( edit ).empty(), "clipped result validates" );
	}
	// Clip keep both: two pieces, the second new with fresh side ids.
	{
		scene::DocumentEdit edit( doc );
		std::vector<scene::ObjectId> created;
		checks.That(
		    ClipSolids( edit, { box, door }, x64, ClipKeep::Both, cap, &created ).HasValue(),
		    "clip keep both" );
		checks.Equal(
		    created.size(), std::size_t( 1 ), "one new piece (the door solid is not crossed)" );
		checks.That(
		    !created.empty() && scene::SolidBounds( *edit.FindSolid( created[0] ) )->mins.x == 64,
		    "the front piece" );
		checks.Near( TotalVolume( edit, { box, created.empty() ? box : created[0] } ),
		    128.0 * 64 * 64, 1e-6, "pieces keep the volume" );
		checks.That( scene::ValidateEdit( edit ).empty(), "new pieces get fresh side ids" );
	}
	// Clip keep front, and negative inputs.
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    ClipSolids( edit, { box }, x64, ClipKeep::Front, cap ).HasValue(), "clip keep front" );
		checks.That( scene::SolidBounds( *edit.FindSolid( box ) )->mins.x == 64,
		    "front piece keeps the id" );
		mapgeometry::Plane none;
		checks.That( !ClipSolids( edit, { box }, none, ClipKeep::Back, cap ),
		    "a zero normal refuses (negative)" );
		const mapgeometry::Plane miss =
		    mapgeometry::PlaneThrough( Vec3d( 1000, 0, 0 ), Vec3d( 1, 0, 0 ) );
		checks.That( ClipSolids( edit, { box }, miss, ClipKeep::Back, cap ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "a plane crossing nothing is nothing (negative)" );
	}

	// Carve: a 32-cube through the middle of the box's top.
	{
		scene::DocumentEdit edit( doc );
		scene::FaceTexture carverTex;
		carverTex.material = "METAL/METALWALL001";
		const scene::ObjectId carver = edit.Add(
		    scene::MakeBoxSolid( { Vec3d( 48, 16, 32 ), Vec3d( 80, 48, 96 ) }, carverTex ) );
		std::vector<scene::ObjectId> created;
		const double before = Volume( *edit.FindSolid( box ) );
		checks.That(
		    Carve( edit, { carver }, { box, far, carver }, &created ).HasValue(), "carve" );
		std::vector<scene::ObjectId> pieces = created;
		pieces.push_back( box );
		checks.Near( TotalVolume( edit, pieces ), before - 32.0 * 32 * 32, 1e-6,
		    "carved pieces = box minus carver" );
		checks.That( edit.FindSolid( carver ) && *edit.FindSolid( far ) == *doc.FindSolid( far ),
		    "the carver and non-overlapping solids survive" );
		bool carverFaces = false;
		for ( scene::ObjectId p : pieces )
			for ( const scene::Side &s : edit.FindSolid( p )->sides )
				carverFaces = carverFaces || s.texture.material == carverTex.material;
		checks.That( carverFaces, "cut faces take the carver's texture" );
		checks.That( scene::ValidateEdit( edit ).empty(), "carved result validates" );
		// A target completely inside the carver vanishes.
		const scene::ObjectId inside =
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 56, 24, 70 ), Vec3d( 64, 32, 80 ) }, tex ) );
		checks.That( Carve( edit, { carver }, { inside } ).HasValue() && !edit.FindSolid( inside ),
		    "a contained target is carved away" );
		checks.That( Carve( edit, { carver }, { far } ).Error().code == app::EditErrorCode::Nothing,
		    "no overlap is nothing (negative)" );
		checks.That( !Carve( edit, {}, { far } ), "no carvers (negative)" );
	}

	// Hollow.
	{
		scene::DocumentEdit edit( doc );
		std::vector<scene::ObjectId> groups;
		checks.That( Hollow( edit, { box }, 16, &groups ).HasValue(), "hollow a box" );
		checks.That(
		    !edit.FindSolid( box ) && groups.size() == 1, "the box is replaced by a group" );
		const std::vector<scene::ObjectId> walls = groups.empty()
		                                               ? std::vector<scene::ObjectId>{}
		                                               : scene::GroupMembers( edit, groups[0] );
		checks.Equal( walls.size(), std::size_t( 6 ), "six walls" );
		checks.Near( TotalVolume( edit, walls ), 128.0 * 64 * 64 - 96.0 * 32 * 32, 1e-6,
		    "walls = shell volume" );
		checks.That( scene::ValidateEdit( edit ).empty(), "hollowed result validates" );
	}
	{
		scene::DocumentEdit edit( doc );
		std::vector<scene::ObjectId> groups;
		checks.That( Hollow( edit, { far }, -8, &groups ).HasValue(), "hollow outward" );
		const std::optional<scene::Box> b =
		    scene::ObjectBounds( edit, groups.empty() ? far : groups[0] );
		checks.That( b && b->mins == Vec3d( 492, -8, -8 ) && b->maxs == Vec3d( 572, 72, 72 ),
		    "outward walls surround the old solid" );
		checks.That( Hollow( edit, { doorSolid }, 8 ).HasValue(), "hollow a brush entity solid" );
		const std::vector<scene::ObjectId> doorSolids = scene::EntitySolids( edit, door );
		checks.Equal( doorSolids.size(), std::size_t( 6 ), "walls stay owned by the entity" );
		checks.That( scene::ValidateEdit( edit ).empty(), "entity walls validate" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That( !Hollow( edit, { box, far }, 40 ) && edit.Finish().Empty(),
		    "a too-thin solid refuses everything (negative)" );
		checks.That( !Hollow( edit, { box }, 0 ), "zero thickness (negative)" );
	}

	return checks.Report();
}
