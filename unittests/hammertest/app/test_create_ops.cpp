//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app creation operations (RFC 0002, R08 domain logic): stock
//			primitives (block, wedge, cylinder, spike, sphere) with outward
//			sides, exact bounds, whole-unit vertices and per-axis orientation;
//			grouped arches; and point entities with catalog defaults (keys and
//			default-on spawnflags). Negative checks: degenerate boxes, side
//			counts out of range, bad arch parameters, unknown or brush classes,
//			empty class names and materials.
//
//=============================================================================//

#include "hammer/app/ops/create_ops.h"
#include "hammer/scene/map_queries.h"
#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include "fakes/fake_entity_catalog.h"

#include <cmath>

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;

namespace
{

bool WholeUnits( const scene::Solid &s )
{
	for ( const mapgeometry::BrushFace &f : scene::BuildGeometry( s ).faces )
		for ( const Vec3d &v : f.vertices )
			if ( v.x != std::round( v.x ) || v.y != std::round( v.y ) || v.z != std::round( v.z ) )
				return false;
	return true;
}

bool Outward( const scene::Solid &s )
{
	const std::optional<scene::Box> b = scene::SolidBounds( s );
	if ( !b )
		return false;
	const mapgeometry::BrushSolid g = scene::BuildGeometry( s );
	Vec3d c;
	const std::vector<Vec3d> verts = mapgeometry::SolidVertices( g );
	for ( const Vec3d &v : verts )
		c += v;
	c = c / static_cast<double>( verts.size() );
	for ( const scene::Side &side : s.sides )
		if ( mapgeometry::PlaneDistance( side.Plane(), c ) >= 0 )
			return false;
	return g.faces.size() == s.sides.size();
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	const scene::Box box{ Vec3d( 0, 0, 0 ), Vec3d( 128, 128, 64 ) };

	struct Case
	{
		PrimitiveKind kind;
		int sides;
		std::size_t faces;
		const char *what;
	};
	const Case cases[] = {
	    { PrimitiveKind::Block, 0, 6, "block has 6 faces" },
	    { PrimitiveKind::Wedge, 0, 5, "wedge has 5 faces" },
	    { PrimitiveKind::Cylinder, 8, 10, "8-sided cylinder has 10 faces" },
	    { PrimitiveKind::Spike, 6, 7, "6-sided spike has 7 faces" },
	    // Rounding to whole units can split a legacy quad into two hull faces.
	    { PrimitiveKind::Sphere, 8, 0, "8-sided sphere has 64 to 128 faces" },
	};
	for ( const Case &c : cases )
	{
		PrimitiveSpec spec;
		spec.kind = c.kind;
		spec.sides = c.sides ? c.sides : 8;
		const std::optional<scene::Solid> s = MakePrimitive( spec, box, tex );
		checks.That( s && ( c.faces ? s->sides.size() == c.faces
		                            : s->sides.size() >= 64 && s->sides.size() <= 128 ),
		    c.what );
		if ( !s )
			continue;
		checks.That( Outward( *s ), "primitive sides face outward" );
		checks.That( WholeUnits( *s ), "primitive vertices are whole units" );
		const std::optional<scene::Box> b = scene::SolidBounds( *s );
		checks.That( b && box.Encloses( *b ), "primitive stays inside its box" );
		checks.That( b && b->mins.z == 0 && b->maxs.z == 64, "primitive spans the box height" );
	}

	// Orientation along another axis.
	{
		PrimitiveSpec spec;
		spec.kind = PrimitiveKind::Cylinder;
		spec.axis = 0;
		const std::optional<scene::Solid> s = MakePrimitive( spec, box, tex );
		const std::optional<scene::Box> b = s ? scene::SolidBounds( *s ) : std::nullopt;
		checks.That( b && b->mins.x == 0 && b->maxs.x == 128, "an X cylinder spans X" );
		bool capX = false;
		if ( s )
			for ( const scene::Side &side : s->sides )
				capX = capX || side.Plane().normal == Vec3d( 1, 0, 0 );
		checks.That( capX, "an X cylinder has caps facing X" );
	}

	// Negative primitive inputs.
	{
		PrimitiveSpec spec;
		spec.kind = PrimitiveKind::Cylinder;
		checks.That( !MakePrimitive( spec, { Vec3d( 0, 0, 0 ), Vec3d( 0, 64, 64 ) }, tex ),
		    "flat box (negative)" );
		spec.sides = 2;
		checks.That( !MakePrimitive( spec, box, tex ), "two sides (negative)" );
		spec.sides = 33;
		checks.That( !MakePrimitive( spec, box, tex ), "33 sides (negative)" );
		spec.sides = 8;
		spec.axis = 3;
		checks.That( !MakePrimitive( spec, box, tex ), "bad axis (negative)" );
	}

	scene::MapDocument doc;
	{
		scene::DocumentEdit edit( doc );
		scene::ObjectId id;
		PrimitiveSpec spec;
		checks.That( CreatePrimitive( edit, spec, box, tex, id ).HasValue() && edit.FindSolid( id ),
		    "create a block" );
		scene::FaceTexture none;
		checks.That( !CreatePrimitive( edit, spec, box, none, id ), "no material (negative)" );
		checks.That( scene::ValidateEdit( edit ).empty(), "created block validates" );
	}

	// Arches.
	{
		scene::DocumentEdit edit( doc );
		ArchSpec arch;
		arch.sides = 8;
		arch.arc = 180;
		arch.wallWidth = 16;
		std::vector<scene::ObjectId> segments;
		scene::ObjectId group;
		checks.That( CreateArch( edit, arch, { Vec3d( -128, -128, 0 ), Vec3d( 128, 128, 32 ) }, tex,
		                 group, &segments )
		                 .HasValue(),
		    "create an arch" );
		checks.That(
		    edit.FindGroup( group ) && segments.size() == 8, "one grouped solid per segment" );
		checks.That(
		    scene::GroupMembers( edit, group ) == segments, "segments are the group's members" );
		checks.That( scene::ValidateEdit( edit ).empty(), "arch validates" );
		bool allOut = true;
		for ( scene::ObjectId s : segments )
			allOut = allOut && Outward( *edit.FindSolid( s ) );
		checks.That( allOut, "arch segments face outward" );
		ArchSpec bad = arch;
		bad.arc = 0;
		checks.That(
		    !CreateArch( edit, bad, { Vec3d( -128, -128, 0 ), Vec3d( 128, 128, 32 ) }, tex, group ),
		    "zero arc (negative)" );
		arch.addHeight = 8;
		segments.clear();
		checks.That( CreateArch( edit, arch, { Vec3d( -128, -128, 0 ), Vec3d( 128, 128, 32 ) }, tex,
		                 group, &segments )
		                 .HasValue(),
		    "spiral arch" );
		checks.That( scene::SolidBounds( *edit.FindSolid( segments.back() ) )->mins.z == 56,
		    "each segment rises" );
	}

	// Entities with catalog defaults.
	{
		hammertest::FakeEntityCatalog catalog;
		auto flags = hammertest::FakeEntityCatalog::Key( "spawnflags", "flags" );
		flags.choices = {
		    { "1", "Start off", true }, { "2", "Toggle", false }, { "4", "Once", true } };
		catalog
		    .AddPoint( "light",
		        { hammertest::FakeEntityCatalog::Key( "_light", "color255", "255 255 255 200" ),
		            hammertest::FakeEntityCatalog::Key( "targetname", "target_source" ), flags } )
		    .AddSolid( "func_door" );
		scene::DocumentEdit edit( doc );
		scene::ObjectId id;
		checks.That( PlaceEntity( edit, "LIGHT", Vec3d( 1, 2, 3 ), &catalog, id ).HasValue(),
		    "place with catalog" );
		const scene::Entity *e = edit.FindEntity( id );
		checks.That( e && e->classname == "light", "catalog spelling of the class" );
		checks.That( e && e->Key( "_light" ) && *e->Key( "_light" ) == "255 255 255 200",
		    "default key applied" );
		checks.That( e && !e->Key( "targetname" ), "keys without defaults are not added" );
		checks.That( e && e->Key( "spawnflags" ) && *e->Key( "spawnflags" ) == "5",
		    "default-on flags summed" );
		checks.That( e && e->Origin() == Vec3d( 1, 2, 3 ), "origin" );
		checks.That(
		    !PlaceEntity( edit, "nope", Vec3d(), &catalog, id ), "unknown class (negative)" );
		checks.That(
		    !PlaceEntity( edit, "func_door", Vec3d(), &catalog, id ), "brush class (negative)" );
		checks.That( !PlaceEntity( edit, "", Vec3d(), nullptr, id ), "empty class (negative)" );
		checks.That( PlaceEntity( edit, "info_anything", Vec3d(), nullptr, id ).HasValue(),
		    "no catalog: any class" );
	}

	return checks.Report();
}
