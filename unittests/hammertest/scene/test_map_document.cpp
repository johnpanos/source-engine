//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.scene map document conformance (RFC 0002, R08 domain logic):
//			object values (side planes from authored points, entity key and
//			connection accessors), document identity (serial-stamped ids never
//			reissued, persistent VMF id floor), lookup by kind, and the
//			invariant check. Negative checks: foreign ids, cross-kind id reuse,
//			malformed keys and connections, and each seeded invariant violation.
//
//=============================================================================//

#include "hammer/scene/map_document.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

using namespace hammer::scene;
using mapgeometry::Vec3d;

namespace
{

Solid MakeBox( MapDocument &doc, const Vec3d &a, const Vec3d &b )
{
	FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	Solid s = MakeBoxSolid( { a, b }, tex );
	s.id = doc.AllocateId();
	s.vmfId = doc.AllocateVmfId();
	for ( Side &side : s.sides )
		side.vmfId = doc.AllocateVmfId();
	return s;
}

} // namespace

int main()
{
	testing::Checks checks;

	// Side planes come from the authored points with the legacy winding.
	{
		Side side;
		side.points = { Vec3d( 0, 0, 64 ), Vec3d( 0, 64, 64 ), Vec3d( 64, 64, 64 ) };
		const mapgeometry::Plane p = side.Plane();
		checks.That( p.normal == Vec3d( 0, 0, 1 ) && p.dist == 64,
		    "legacy (p0-p1)x(p2-p1) winding faces up" );
		Side degenerate;
		degenerate.points = { Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ), Vec3d( 2, 0, 0 ) };
		checks.That( degenerate.Plane().normal == Vec3d(), "collinear points give a zero normal" );
	}

	// Entity keys: ordered, typed accessors through the one list.
	{
		Entity e;
		e.classname = "light";
		checks.That( !e.Origin(), "no origin key -> nothing" );
		e.SetOrigin( Vec3d( 1, -2.5, 0 ) );
		checks.That(
		    e.Key( "origin" ) && *e.Key( "origin" ) == "1 -2.5 0", "origin formats compactly" );
		checks.That( e.Origin() == Vec3d( 1, -2.5, 0 ), "origin parses back" );
		checks.That( !e.SetKey( "origin", "1 -2.5 0" ), "setting an equal value is a no-op" );
		checks.That( e.SetKey( "targetname", "lamp" ) && e.Name() == "lamp", "targetname" );
		checks.Equal( e.keys.size(), std::size_t( 2 ), "keys appended in order" );
		checks.That( e.keys[0].key == "origin" && e.keys[1].key == "targetname", "key order kept" );
		e.SetKey( "angles", "0 90 zero" );
		checks.That( !e.Angles(), "malformed angles are rejected" );
		e.SetKey( "angles", "0 90 0 4" );
		checks.That( !e.Angles(), "four numbers are not angles" );
		e.SetAngles( Vec3d( -0.0, 90, 0 ) );
		checks.That( *e.Key( "angles" ) == "0 90 0", "negative zero formats as 0" );
		checks.That(
		    e.RemoveKey( "angles" ) && !e.RemoveKey( "angles" ), "remove reports presence" );
	}

	// Connections: both separators, and malformed values.
	{
		const std::optional<Connection> c = ParseConnection( "OnTrigger", "door,Open,,0.5,-1" );
		checks.That( c && c->target == "door" && c->input == "Open" && c->parameter.empty() &&
		                 c->delay == 0.5 && c->timesToFire == -1 && c->separator == ',',
		    "comma connection parses" );
		checks.That(
		    c && FormatConnectionValue( *c ) == "door,Open,,0.5,-1", "comma connection formats" );
		const std::optional<Connection> esc = ParseConnection( "OnPressed", "relay\x1bTrigger\x1b"
		                                                                    "a,b\x1b"
		                                                                    "0\x1b"
		                                                                    "1" );
		checks.That( esc && esc->parameter == "a,b" && esc->separator == '\x1b',
		    "ESC separator keeps commas" );
		checks.That( esc && FormatConnectionValue( *esc ) == "relay\x1bTrigger\x1b"
		                                                     "a,b\x1b"
		                                                     "0\x1b"
		                                                     "1",
		    "ESC connection formats" );
		checks.That(
		    !ParseConnection( "OnTrigger", "door,Open,0.5,-1" ), "four fields are rejected" );
		checks.That( !ParseConnection( "OnTrigger", "door,Open,,soon,-1" ),
		    "non-numeric delay is rejected" );
		checks.That(
		    !ParseConnection( "OnTrigger", "door,Open,,0,1.5" ), "fractional count is rejected" );
	}

	// Identity: serial-stamped, never reissued, VMF floor.
	{
		MapDocument doc( 7 );
		checks.That(
		    doc.Settings().WorldKey( "mapversion" ) != nullptr, "a new world has a mapversion" );
		const ObjectId a = doc.AllocateId();
		const ObjectId b = doc.AllocateId();
		checks.That( a != b && DocumentSerialOf( a ) == 7 && DocumentSerialOf( b ) == 7,
		    "ids carry the serial" );
		Solid s = MakeBox( doc, Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
		const ObjectId sid = s.id;
		checks.That( doc.Put( s ), "put a solid" );
		checks.That(
		    doc.KindOf( sid ) == ObjectKind::Solid && doc.FindSolid( sid ), "lookup by kind" );
		checks.That( !doc.FindEntity( sid ), "a solid is not an entity" );
		checks.That( doc.Erase( sid ) && !doc.Contains( sid ) && !doc.Erase( sid ), "erase once" );
		// Re-putting an old id (undo) must push the counter past it.
		Solid late;
		late.id.value = ( std::uint64_t( 7 ) << 32 ) | 500;
		late.sides = s.sides;
		for ( Side &side : late.sides )
			side.vmfId += 1000;
		checks.That( doc.Put( late ), "put a restored id" );
		checks.That(
		    ( doc.AllocateId().value & 0xffffffffu ) == 501, "restored ids are never reissued" );
		checks.That(
		    doc.AllocateVmfId() > late.sides.back().vmfId, "VMF id floor follows put sides" );
		doc.NoteVmfId( 9000 );
		checks.Equal( doc.AllocateVmfId(), std::uint32_t( 9001 ), "NoteVmfId raises the floor" );

		// Negative: a foreign serial and a cross-kind id collision are refused.
		MapDocument other( 8 );
		Solid foreign = MakeBox( other, Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) );
		checks.That( !doc.Put( foreign ), "a foreign id is refused" );
		Entity clash;
		clash.id = late.id;
		checks.That( !doc.Put( clash ), "an id cannot be two kinds" );
		Solid invalid;
		checks.That( !doc.Put( invalid ), "the invalid id is refused" );
	}

	// Invariants.
	{
		MapDocument doc;
		Solid s = MakeBox( doc, Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
		doc.Put( s );
		Entity e;
		e.id = doc.AllocateId();
		e.classname = "func_detail";
		doc.Put( e );
		Group g;
		g.id = doc.AllocateId();
		doc.Put( g );
		checks.That( doc.Validate().empty(), "a consistent document validates" );
		checks.Equal( doc.ObjectCount(), std::size_t( 3 ), "object count" );
		checks.Equal( doc.SolidIds().size() + doc.EntityIds().size() + doc.GroupIds().size(),
		    std::size_t( 3 ), "ids by kind" );

		MapDocument bad = doc;
		Solid orphan = s;
		orphan.owner = g.id; // a group is not an owner
		bad.Put( orphan );
		checks.Equal( bad.Validate().size(), std::size_t( 1 ), "owner must be an entity" );

		bad = doc;
		Entity lost = e;
		lost.group = s.id;
		bad.Put( lost );
		checks.Equal( bad.Validate().size(), std::size_t( 1 ), "group ref must be a group" );

		bad = doc;
		Group g2;
		g2.id = bad.AllocateId();
		g2.group = g.id;
		bad.Put( g2 );
		Group loop = g;
		loop.group = g2.id;
		bad.Put( loop );
		checks.That( bad.Validate().size() >= 2, "group cycles are reported" );

		bad = doc;
		Solid dup = MakeBox( bad, Vec3d( 100, 0, 0 ), Vec3d( 164, 64, 64 ) );
		dup.sides[0].vmfId = s.sides[0].vmfId;
		bad.Put( dup );
		checks.Equal( bad.Validate().size(), std::size_t( 1 ), "side ids must be unique" );

		bad = doc;
		Solid thin = s;
		thin.sides.resize( 3 );
		bad.Put( thin );
		checks.Equal( bad.Validate().size(), std::size_t( 1 ), "a solid needs four sides" );

		checks.That(
		    SameContent( doc, doc ) && !SameContent( doc, bad ), "SameContent compares content" );
	}

	// Settings and the visgroup tree.
	{
		DocumentSettings s;
		checks.That( s.SetWorldKey( "skyname", "sky_day01_01" ) &&
		                 !s.SetWorldKey( "skyname", "sky_day01_01" ),
		    "world keys change once" );
		Visgroup inner{ 3, "inner", std::nullopt, {} };
		Visgroup outer{ 1, "outer", Rgb{ 1, 2, 3 }, { inner } };
		s.visgroups = { outer, Visgroup{ 2, "other", std::nullopt, {} } };
		const VisgroupLookup found = FindVisgroup( s.visgroups, 3 );
		checks.That( found.visgroup && found.visgroup->name == "inner" && found.parentId == 1,
		    "nested lookup" );
		checks.That( FindVisgroup( s.visgroups, 2 ).parentId == 0, "top-level parent is 0" );
		checks.That( !FindVisgroup( s.visgroups, 9 ).visgroup, "missing visgroup" );
	}

	return checks.Report();
}
