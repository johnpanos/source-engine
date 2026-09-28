//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app map check (RFC 0002, R08 domain logic; legacy Check for
//			Problems). A clean map with the special cases each check must accept
//			(wildcard and special targets, allowlisted keys, visgroup-hidden
//			objects, brush entities) reports nothing. Sensitivity: every problem
//			code has a fixture seeded so that exactly that code fires, and a
//			near-miss fixture (the corrected or boundary case) on which it does
//			not. Fixable codes: the fix removes the problem, the edit validates,
//			and a second fix is NothingToDo (idempotent); unfixable codes refuse.
//			FixAll over every fixable seed leaves no fixable problem. Order is
//			by code, then object id.
//
//=============================================================================//

#include "hammer/app/map_check.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include "fakes/fake_entity_catalog.h"
#include "fakes/fake_material_info.h"

#include <cmath>
#include <functional>
#include <string>

using namespace hammer;
using namespace hammer::app;
using mapgeometry::Vec3d;
using scene::ObjectId;
using Code = MapProblem::Code;
using C = hammertest::FakeEntityCatalog;

namespace
{

scene::FaceTexture Texture( const char *material = "DEV/DEV_MEASUREGENERIC01B" )
{
	scene::FaceTexture tex;
	tex.material = material;
	return tex;
}

scene::Solid Box( Vec3d mins, Vec3d maxs )
{
	return scene::MakeBoxSolid( { mins, maxs }, Texture() );
}

// Low-level insertion (fixtures may break invariants an edit would refuse).
ObjectId Insert( scene::MapDocument &doc, scene::Solid solid )
{
	solid.id = doc.AllocateId();
	if ( solid.vmfId == 0 )
	{
		solid.vmfId = doc.AllocateVmfId();
	}
	for ( scene::Side &side : solid.sides )
	{
		if ( side.vmfId == 0 )
		{
			side.vmfId = doc.AllocateVmfId();
		}
	}
	const ObjectId id = solid.id;
	doc.Put( std::move( solid ) );
	return id;
}

ObjectId Insert( scene::MapDocument &doc, scene::Entity entity )
{
	entity.id = doc.AllocateId();
	if ( entity.vmfId == 0 )
	{
		entity.vmfId = doc.AllocateVmfId();
	}
	const ObjectId id = entity.id;
	doc.Put( std::move( entity ) );
	return id;
}

ObjectId Insert( scene::MapDocument &doc, scene::Group group )
{
	group.id = doc.AllocateId();
	if ( group.vmfId == 0 )
	{
		group.vmfId = doc.AllocateVmfId();
	}
	const ObjectId id = group.id;
	doc.Put( std::move( group ) );
	return id;
}

scene::Entity Point( const char *classname, const char *name, Vec3d origin )
{
	scene::Entity e;
	e.classname = classname;
	if ( name && *name )
	{
		e.SetKey( "targetname", name );
	}
	e.SetOrigin( origin );
	return e;
}

std::size_t CountCode( const std::vector<MapProblem> &problems, Code code )
{
	std::size_t n = 0;
	for ( const MapProblem &p : problems )
	{
		n += p.code == code ? 1 : 0;
	}
	return n;
}

std::string Codes( const std::vector<MapProblem> &problems )
{
	std::string out;
	for ( const MapProblem &p : problems )
	{
		out += std::string( out.empty() ? "" : " " ) + MapProblemCodeName( p.code );
	}
	return out.empty() ? "none" : out;
}

} // namespace

int main()
{
	testing::Checks checks;

	C catalog;
	catalog.AddPoint( "info_player_start", { C::Key( "angles", "angle" ) } );
	catalog.AddPoint( "info_player_teamspawn", { C::Key( "angles", "angle" ) } );
	catalog.AddPoint( "light",
	    { C::Key( "targetname", "target_source" ), C::Key( "_light", "color255" ) },
	    { C::Io( "TurnOn" ), C::Io( "TurnOff" ) } );
	catalog.AddPoint( "logic_relay", { C::Key( "targetname", "target_source" ) },
	    { C::Io( "Trigger" ) }, { C::Io( "OnTrigger" ) } );
	catalog.AddSolid( "func_door",
	    { C::Key( "targetname", "target_source" ), C::Key( "speed", "float" ) },
	    { C::Io( "Open" ) }, { C::Io( "OnOpen" ) } );
	catalog.AddSolid( "trigger_once",
	    { C::Key( "targetname", "target_source" ), C::Key( "target", "target_destination" ),
	        C::Key( "filtername", "filterclass" ) },
	    { C::Io( "Enable" ) }, { C::Io( "OnTrigger" ), C::Io( "OnStartTouch" ) } );

	hammertest::FakeMaterialInfo materials;
	materials.Add( "dev/dev_measuregeneric01b", 128, 128 );

	// The clean map.
	scene::MapDocument clean;
	ObjectId worldBox, lamp, trigger, triggerSolid, door, doorSolid, groupBox, group, hiddenBox;
	{
		scene::Visgroup v;
		v.id = 1;
		v.name = "details";
		clean.MutableSettings().visgroups = { v };
		Insert( clean, Point( "info_player_start", "", Vec3d( 0, 0, 8 ) ) );
		worldBox = Insert( clean, Box( Vec3d( -512, -512, -64 ), Vec3d( 512, 512, 0 ) ) );
		scene::Entity l = Point( "light", "lamp", Vec3d( 0, 0, 128 ) );
		l.SetKey( "_light", "255 255 255 200" );
		l.SetAngles( Vec3d( 0, 90, 0 ) ); // allowlisted, not in the schema
		lamp = Insert( clean, l );
		Insert( clean, Point( "logic_relay", "relay1", Vec3d( 16, 0, 16 ) ) );
		scene::Entity t;
		t.classname = "trigger_once";
		t.SetKey( "target", "relay*" ); // a wildcard names relay1
		t.connections.push_back( *scene::ParseConnection( "OnTrigger", "lamp,TurnOff,,0,-1" ) );
		t.connections.push_back( *scene::ParseConnection( "ontrigger", "RELAY*,Trigger,,0.5,1" ) );
		t.connections.push_back(
		    *scene::ParseConnection( "OnStartTouch", "!activator,Anything,,0,-1" ) );
		t.connections.push_back(
		    *scene::ParseConnection( "OnStartTouch", "!speechtarget,Speak,,0,-1" ) );
		trigger = Insert( clean, t );
		scene::Solid ts = Box( Vec3d( 64, 64, 0 ), Vec3d( 128, 128, 64 ) );
		ts.owner = trigger;
		triggerSolid = Insert( clean, ts );
		scene::Entity d = Point( "func_door", "door", Vec3d( 0, 0, 0 ) );
		d.RemoveKey( "origin" );
		d.SetKey( "speed", "100" );
		door = Insert( clean, d );
		scene::Solid ds = Box( Vec3d( 200, 0, 0 ), Vec3d( 216, 64, 128 ) );
		ds.owner = door;
		doorSolid = Insert( clean, ds );
		group = Insert( clean, scene::Group{} );
		scene::Solid gs = Box( Vec3d( 300, 0, 0 ), Vec3d( 316, 16, 16 ) );
		gs.group = group;
		groupBox = Insert( clean, gs );
		scene::Solid hs = Box( Vec3d( 400, 0, 0 ), Vec3d( 416, 16, 16 ) );
		hs.editor.visgroupIds = { 1 };
		hs.editor.visgroupShown = false; // hidden by a real visgroup
		hiddenBox = Insert( clean, hs );
		checks.That( clean.Validate().empty(), "the clean fixture is a valid document" );
	}
	{
		const std::vector<MapProblem> problems = CheckMap( clean, &catalog, &materials );
		checks.That( problems.empty(), "a clean map reports nothing (" + Codes( problems ) + ")" );
		checks.That(
		    CheckMap( clean, nullptr, nullptr ).empty(), "nothing without the ports either" );
	}

	struct Case
	{
		Code code;
		const char *what;
		std::function<ObjectId( scene::MapDocument & )>
		    seed; // returns the object the problem names
		std::function<void( scene::MapDocument & )> nearMiss;
	};
	const std::vector<Case> cases = {
	    { Code::NoPlayerStart, "no player start",
	        [&]( scene::MapDocument &d )
	        {
		        d.Erase( d.EntityIds().front() );
		        return ObjectId();
	        },
	        [&]( scene::MapDocument &d )
	        {
		        d.Erase( d.EntityIds().front() );
		        Insert( d, Point( "info_player_teamspawn", "", Vec3d( 0, 0, 8 ) ) );
	        } },
	    { Code::EmptyClassname, "empty classname",
	        [&]( scene::MapDocument &d )
	        {
		        return Insert( d, Point( "", "", Vec3d( 0, 0, 0 ) ) );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        Insert( d, Point( "logic_relay", "", Vec3d( 0, 0, 0 ) ) );
	        } },
	    { Code::UnknownClass, "unknown class",
	        [&]( scene::MapDocument &d )
	        {
		        return Insert( d, Point( "prop_mystery", "", Vec3d( 0, 0, 0 ) ) );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        Insert( d, Point( "LOGIC_RELAY", "", Vec3d( 0, 0, 0 ) ) );
	        } },
	    { Code::EmptyBrushEntity, "brush entity without solids",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e;
		        e.classname = "func_door";
		        return Insert( d, e );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e;
		        e.classname = "func_door";
		        const ObjectId id = Insert( d, e );
		        scene::Solid s = Box( Vec3d( 0, 300, 0 ), Vec3d( 16, 316, 16 ) );
		        s.owner = id;
		        Insert( d, s );
	        } },
	    { Code::PointEntityWithSolids, "point entity owning solids",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = Box( Vec3d( 0, 300, 0 ), Vec3d( 16, 316, 16 ) );
		        s.owner = lamp;
		        Insert( d, s );
		        return lamp;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = Box( Vec3d( 0, 300, 0 ), Vec3d( 16, 316, 16 ) );
		        s.owner = door;
		        Insert( d, s );
	        } },
	    { Code::MissingTarget, "key naming no entity",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( trigger );
		        e.SetKey( "target", "nobody" );
		        d.Put( e );
		        return trigger;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( trigger );
		        e.SetKey( "target", "!player" );
		        d.Put( e );
	        } },
	    { Code::ConnectionMissingTarget, "output to nobody",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( trigger );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnTrigger", "ghost,Trigger,,0,-1" ) );
		        d.Put( e );
		        return trigger;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( trigger );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnTrigger", "!pvsplayer,Trigger,,0,-1" ) );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnTrigger", "!picker,Trigger,,0,-1" ) );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnTrigger", "!self,Enable,,0,-1" ) );
		        d.Put( e );
	        } },
	    { Code::UnknownOutput, "undeclared output",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( trigger );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnExplode", "lamp,TurnOn,,0,-1" ) );
		        d.Put( e );
		        return trigger;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( trigger );
		        e.connections.push_back(
		            *scene::ParseConnection( "ONSTARTTOUCH", "lamp,TurnOn,,0,-1" ) );
		        d.Put( e );
	        } },
	    { Code::UnknownInput, "undeclared input",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( trigger );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnTrigger", "lamp,Explode,,0,-1" ) );
		        d.Put( e );
		        return trigger;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        // Inputs match case-insensitively, and a special target is not judged.
		        scene::Entity e = *d.FindEntity( trigger );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnTrigger", "lamp,turnon,,0,-1" ) );
		        e.connections.push_back(
		            *scene::ParseConnection( "OnTrigger", "!caller,Explode,,0,-1" ) );
		        d.Put( e );
	        } },
	    { Code::UnusedKeyvalues, "keys outside the schema",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( lamp );
		        e.SetKey( "bogus", "1" );
		        d.Put( e );
		        return lamp;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( lamp );
		        e.SetKey( "spawnflags", "1" );
		        e.SetKey( "ANGLE", "90" );
		        d.Put( e );
	        } },
	    { Code::DuplicateKeys, "a key set twice",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( lamp );
		        e.keys.push_back( kvtext::KeyValue{ "_LIGHT", "1 1 1 1" } );
		        d.Put( e );
		        return lamp;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = *d.FindEntity( lamp );
		        e.SetKey( "_light", "1 1 1 1" );
		        d.Put( e );
	        } },
	    { Code::InvalidSolid, "an open solid",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		        s.sides.erase( s.sides.begin(), s.sides.begin() + 2 ); // top and bottom
		        return Insert( d, s );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        Insert( d, Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) ) );
	        } },
	    { Code::DuplicatePlanes, "a redundant side",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		        scene::Side extra = s.sides[0];
		        extra.vmfId = 0;
		        const Vec3d up = extra.Plane().normal * 8.0;
		        for ( Vec3d &p : extra.points )
		        {
			        p = p + up;
		        }
		        s.sides.push_back( extra );
		        return Insert( d, s );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        Insert( d, Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) ) );
	        } },
	    { Code::MissingMaterial, "a missing material",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		        s.sides[2].texture.material = "missing/texture";
		        return Insert( d, s );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		        s.sides[2].texture.material = "DEV\\Dev_MeasureGeneric01b";
		        Insert( d, s );
	        } },
	    { Code::DuplicateSideId, "a reused side id",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		        s.sides[3].vmfId = d.FindSolid( worldBox )->sides[1].vmfId;
		        return Insert( d, s );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        Insert( d, Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) ) );
	        } },
	    { Code::DuplicateObjectId, "a reused entity id",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Entity e = Point( "logic_relay", "", Vec3d( 0, 0, 0 ) );
		        e.vmfId = d.FindEntity( lamp )->vmfId;
		        return Insert( d, e );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        // The same number on objects of different kinds is not a clash.
		        scene::Entity e = Point( "logic_relay", "", Vec3d( 0, 0, 0 ) );
		        e.vmfId = d.FindSolid( worldBox )->vmfId;
		        Insert( d, e );
	        } },
	    { Code::UndefinedVisgroup, "a dangling visgroup id",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = *d.FindSolid( groupBox );
		        s.editor.visgroupIds = { 1, 77 };
		        d.Put( s );
		        return groupBox;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = *d.FindSolid( groupBox );
		        s.editor.visgroupIds = { 1 };
		        d.Put( s );
	        } },
	    { Code::HiddenWithoutVisgroup, "hidden with no visgroup",
	        [&]( scene::MapDocument &d )
	        {
		        scene::Solid s = *d.FindSolid( hiddenBox );
		        s.editor.visgroupIds.clear();
		        d.Put( s );
		        return hiddenBox;
	        },
	        [&]( scene::MapDocument &d )
	        {
		        // Hidden inside a group that is in a visgroup: covered.
		        scene::Group g = *d.FindGroup( group );
		        g.editor.visgroupIds = { 1 };
		        d.Put( g );
		        scene::Solid s = *d.FindSolid( groupBox );
		        s.editor.visgroupShown = false;
		        d.Put( s );
	        } },
	    { Code::EmptyGroup, "an empty nested group",
	        [&]( scene::MapDocument &d )
	        {
		        const ObjectId outer = Insert( d, scene::Group{} );
		        scene::Group inner;
		        inner.group = outer;
		        return Insert( d, inner );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        const ObjectId outer = Insert( d, scene::Group{} );
		        scene::Group inner;
		        inner.group = outer;
		        const ObjectId innerId = Insert( d, inner );
		        scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		        s.group = innerId;
		        Insert( d, s );
	        } },
	    { Code::OutsideMapBounds, "beyond the coordinate limit",
	        [&]( scene::MapDocument &d )
	        {
		        return Insert( d, Box( Vec3d( 16380, 0, 0 ), Vec3d( 16400, 16, 16 ) ) );
	        },
	        [&]( scene::MapDocument &d )
	        {
		        Insert( d, Box( Vec3d( 16368, 0, 0 ), Vec3d( 16384, 16, 16 ) ) );
		        Insert( d, Point( "logic_relay", "", Vec3d( -16384, 0, 0 ) ) );
	        } },
	};

	std::size_t fixableSeen = 0;
	for ( const Case &c : cases )
	{
		const std::string name = std::string( MapProblemCodeName( c.code ) ) + " (" + c.what + ")";
		scene::MapDocument bad = clean;
		const ObjectId subject = c.seed( bad );
		const std::vector<MapProblem> problems = CheckMap( bad, &catalog, &materials );
		checks.That( problems.size() == 1 && problems[0].code == c.code,
		    name + ": fires exactly once (" + Codes( problems ) + ")" );
		if ( problems.size() != 1 )
		{
			continue;
		}
		const MapProblem &p = problems[0];
		checks.That(
		    subject.IsValid() ? p.objects == std::vector<ObjectId>{ subject } : p.objects.empty(),
		    name + ": names its object" );
		checks.That( !p.message.empty(), name + ": has a message" );

		scene::MapDocument near = clean;
		c.nearMiss( near );
		const std::vector<MapProblem> nearProblems = CheckMap( near, &catalog, &materials );
		checks.That( CountCode( nearProblems, c.code ) == 0 && nearProblems.empty(),
		    name + ": the near miss does not fire (" + Codes( nearProblems ) + ")" );

		scene::DocumentEdit edit( bad );
		const EditResult fixed = FixProblem( edit, p, &catalog );
		if ( p.fixable )
		{
			++fixableSeen;
			checks.That( fixed.HasValue(), name + ": the fix applies" );
			checks.That( CheckMap( edit, &catalog, &materials ).empty(), name + ": fixed" );
			checks.That( scene::ValidateEdit( edit ).empty(), name + ": the fixed edit validates" );
			const EditResult again = FixProblem( edit, p, &catalog );
			checks.That( !again && again.Error().code == EditErrorCode::Nothing,
			    name + ": the fix is idempotent" );
		}
		else
		{
			checks.That( !fixed && fixed.Error().code == EditErrorCode::Rejected,
			    name + ": no fix (refused)" );
			checks.That( edit.Finish().Empty(), name + ": the refusal staged nothing" );
		}
	}
	checks.Equal( cases.size(), std::size_t( 20 ), "every code has a case" );
	checks.Equal( fixableSeen, std::size_t( 14 ), "fixable codes" );

	// Specific fix results.
	{
		scene::MapDocument bad = clean;
		scene::Entity e = *bad.FindEntity( lamp );
		e.keys.push_back( kvtext::KeyValue{ "_LIGHT", "1 1 1 1" } );
		bad.Put( e );
		scene::DocumentEdit edit( bad );
		checks.That( FixAll( edit, &catalog, &materials ).HasValue(), "fix all" );
		checks.That( *edit.FindEntity( lamp )->Key( "_light" ) == "255 255 255 200" &&
		                 edit.FindEntity( lamp )->keys.size() == e.keys.size() - 1,
		    "duplicate keys: the first occurrence is kept" );
	}
	{
		scene::MapDocument bad = clean;
		scene::Solid s = *bad.FindSolid( hiddenBox );
		s.editor.visgroupIds.clear();
		bad.Put( s );
		scene::DocumentEdit edit( bad );
		checks.That( FixAll( edit, &catalog, &materials ).HasValue(), "fix all" );
		const scene::Solid *fixed = edit.FindSolid( hiddenBox );
		const scene::Visgroup *v =
		    fixed->editor.visgroupIds.size() == 1
		        ? scene::FindVisgroup( edit.Settings().visgroups, fixed->editor.visgroupIds[0] )
		              .visgroup
		        : nullptr;
		checks.That( v && v->name == kCheckHiddenVisgroupName && !fixed->editor.visgroupShown,
		    "hidden-without-visgroup: filed in the check's visgroup, still hidden" );
	}
	{
		scene::MapDocument bad = clean;
		scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		scene::Side extra = s.sides[0];
		extra.vmfId = 0;
		s.sides.push_back( extra ); // an exact duplicate plane
		const ObjectId id = Insert( bad, s );
		const std::vector<MapProblem> problems = CheckMap( bad, &catalog, &materials );
		checks.That( CountCode( problems, Code::DuplicatePlanes ) == 1,
		    "a coincident duplicate side is reported" );
		scene::DocumentEdit edit( bad );
		checks.That( FixAll( edit, &catalog, &materials ).HasValue() &&
		                 edit.FindSolid( id )->sides.size() == 6,
		    "the duplicate side is dropped" );
		checks.That( scene::SolidBounds( *edit.FindSolid( id ) ) == scene::SolidBounds( s ),
		    "the shape is kept" );
	}
	{
		// An input into a target of unknown class cannot be judged.
		scene::MapDocument bad = clean;
		Insert( bad, Point( "info_custom", "custom", Vec3d( 0, 0, 0 ) ) );
		scene::Entity e = *bad.FindEntity( trigger );
		e.connections.push_back( *scene::ParseConnection( "OnTrigger", "custom,Explode,,0,-1" ) );
		bad.Put( e );
		const std::vector<MapProblem> problems = CheckMap( bad, &catalog, &materials );
		checks.That( problems.size() == 1 && problems[0].code == Code::UnknownClass,
		    "only the unknown class is reported (" + Codes( problems ) + ")" );
		checks.That(
		    CheckMap( bad, nullptr, &materials ).empty(), "without a catalog nothing is judged" );
	}
	{
		// A dangling visgroup id does not count as a covering visgroup.
		scene::MapDocument bad = clean;
		scene::Solid s = *bad.FindSolid( hiddenBox );
		s.editor.visgroupIds = { 77 };
		bad.Put( s );
		const std::vector<MapProblem> problems = CheckMap( bad, &catalog, &materials );
		checks.That( problems.size() == 2 && CountCode( problems, Code::UndefinedVisgroup ) == 1 &&
		                 CountCode( problems, Code::HiddenWithoutVisgroup ) == 1,
		    "hidden under a dangling visgroup only (" + Codes( problems ) + ")" );
		scene::DocumentEdit edit( bad );
		checks.That( FixAll( edit, &catalog, &materials ).HasValue() &&
		                 CheckMap( edit, &catalog, &materials ).empty(),
		    "both fixed" );
	}
	{
		// A side that bounds no face, with a direction no other side has.
		scene::MapDocument bad = clean;
		scene::Solid s = Box( Vec3d( 0, 600, 0 ), Vec3d( 16, 616, 16 ) );
		scene::Side extra = s.sides[0];
		extra.vmfId = 0;
		const double r = 1.0 / std::sqrt( 2.0 );
		extra.points = scene::PointsFromPlane( mapgeometry::Plane{ Vec3d( r, r, 0 ), 640.0 * r } );
		s.sides.push_back( extra );
		const ObjectId id = Insert( bad, s );
		const std::vector<MapProblem> problems = CheckMap( bad, &catalog, &materials );
		checks.That( problems.size() == 1 && problems[0].code == Code::DuplicatePlanes &&
		                 problems[0].message.find( "bounds no face" ) != std::string::npos,
		    "an outside side is reported (" + Codes( problems ) + ")" );
		scene::DocumentEdit edit( bad );
		checks.That( FixAll( edit, &catalog, &materials ).HasValue() &&
		                 edit.FindSolid( id )->sides.size() == 6,
		    "the outside side is dropped" );
	}
	{
		// The only solid of a brush entity is invalid: the fix removes both.
		scene::MapDocument bad = clean;
		scene::Solid s = *bad.FindSolid( doorSolid );
		s.sides.erase( s.sides.begin(), s.sides.begin() + 2 );
		bad.Put( s );
		const std::vector<MapProblem> problems = CheckMap( bad, &catalog, &materials );
		checks.That( problems.size() == 1 && problems[0].code == Code::InvalidSolid,
		    "invalid brush-entity solid" );
		scene::DocumentEdit edit( bad );
		checks.That( FixProblem( edit, problems[0], &catalog ).HasValue() &&
		                 !edit.FindSolid( doorSolid ) && !edit.FindEntity( door ),
		    "the emptied brush entity goes with it" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid" );
	}

	// Everything fixable at once, and the order.
	{
		scene::MapDocument bad = clean;
		for ( const Case &c : cases )
		{
			if ( c.code != Code::NoPlayerStart && c.code != Code::EmptyClassname &&
			     c.code != Code::UnknownClass && c.code != Code::PointEntityWithSolids &&
			     c.code != Code::MissingMaterial && c.code != Code::OutsideMapBounds )
			{
				c.seed( bad );
			}
		}
		const std::vector<MapProblem> before = CheckMap( bad, &catalog, &materials );
		checks.That( before.size() >= 14, "every fixable seed reported (" + Codes( before ) + ")" );
		bool ordered = true;
		for ( std::size_t i = 1; i < before.size(); ++i )
		{
			const MapProblem &a = before[i - 1];
			const MapProblem &b = before[i];
			ordered = ordered &&
			          ( a.code < b.code ||
			              ( a.code == b.code && ( a.objects.empty() || b.objects.empty() ||
			                                        a.objects.front() <= b.objects.front() ) ) );
		}
		checks.That( ordered, "ordered by code, then object id" );
		scene::DocumentEdit edit( bad );
		checks.That( FixAll( edit, &catalog, &materials ).HasValue(), "fix all" );
		const std::vector<MapProblem> after = CheckMap( edit, &catalog, &materials );
		bool anyFixable = false;
		for ( const MapProblem &p : after )
		{
			anyFixable = anyFixable || p.fixable;
		}
		checks.That( !anyFixable, "no fixable problem remains (" + Codes( after ) + ")" );
		checks.That( scene::ValidateEdit( edit ).empty(), "the fixed edit validates" );
		const EditResult again = FixAll( edit, &catalog, &materials );
		checks.That(
		    !again && again.Error().code == EditErrorCode::Nothing, "fix all is idempotent" );
		checks.That( CheckMap( bad, &catalog, &materials ).size() == before.size(),
		    "the base is untouched" );
	}

	// Deterministic: a second scan is identical.
	{
		scene::MapDocument bad = clean;
		for ( const Case &c : cases )
		{
			c.seed( bad );
		}
		const std::vector<MapProblem> a = CheckMap( bad, &catalog, &materials );
		const std::vector<MapProblem> b = CheckMap( bad, &catalog, &materials );
		bool same = a.size() == b.size();
		for ( std::size_t i = 0; same && i < a.size(); ++i )
		{
			same = a[i].code == b[i].code && a[i].message == b[i].message &&
			       a[i].objects == b[i].objects;
		}
		checks.That( same && a.front().code == Code::NoPlayerStart,
		    "deterministic, map-wide problems first" );
		const MapProblem unfixable{
		    Code::OutsideMapBounds, MapProblem::Severity::Error, "x", { worldBox }, false };
		scene::DocumentEdit edit( bad );
		checks.That(
		    FixProblem( edit, unfixable, &catalog ).Error().code == EditErrorCode::Rejected,
		    "an unfixable code is refused (negative)" );
		const MapProblem stale{
		    Code::EmptyGroup, MapProblem::Severity::Warning, "x", { ObjectId() }, true };
		checks.That( FixProblem( edit, stale, &catalog ).Error().code == EditErrorCode::Nothing,
		    "a stale problem is nothing to do (negative)" );
	}
	(void)triggerSolid;

	return checks.Report();
}
