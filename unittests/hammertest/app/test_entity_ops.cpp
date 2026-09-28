//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app entity operations (RFC 0002, R08 domain logic): keys on
//			many entities as one edit (no-op detection), removal and renaming,
//			class changes with kind checks and defaults, spawnflag bits,
//			connections (add, replace, remove by predicate), renaming with
//			reference updates (catalog target_destination keys, or the
//			conventional keys without a catalog) and worldspawn keys.
//			Negative checks: reserved keys, kind mismatches, non-bit flags,
//			incomplete connections, rename collisions.
//
//=============================================================================//

#include "hammer/app/ops/entity_ops.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "fakes/fake_entity_catalog.h"

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using scene::ObjectId;

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::MapDocument doc;
	ObjectId lamp, lamp2, button, door, doorSolid, trigger;
	{
		scene::DocumentEdit edit( doc );
		scene::Entity l;
		l.classname = "light";
		l.SetKey( "targetname", "lamp" );
		lamp = edit.Add( l );
		l.SetKey( "targetname", "lamp2" );
		lamp2 = edit.Add( l );
		scene::Entity b;
		b.classname = "func_button";
		b.connections.push_back( *scene::ParseConnection( "OnPressed", "lamp,TurnOn,,0,-1" ) );
		b.connections.push_back( *scene::ParseConnection( "OnPressed", "door,Open,,1,1" ) );
		button = edit.Add( b );
		scene::Entity d;
		d.classname = "func_door";
		d.SetKey( "targetname", "door" );
		door = edit.Add( d );
		scene::Solid ds = scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 8, 64, 128 ) }, tex );
		ds.owner = door;
		doorSolid = edit.Add( ds );
		scene::Entity t;
		t.classname = "logic_relay";
		t.SetKey( "target", "lamp" );
		t.SetKey( "message", "lamp" );
		trigger = edit.Add( t );
		scene::CommitEdit( doc, edit );
	}

	hammertest::FakeEntityCatalog catalog;
	catalog
	    .AddPoint( "light",
	        { hammertest::FakeEntityCatalog::Key( "_light", "color255", "255 255 255 200" ) } )
	    .AddPoint(
	        "light_spot", { hammertest::FakeEntityCatalog::Key( "_cone", "integer", "45" ) } )
	    .AddSolid( "func_door" )
	    .AddSolid( "func_door_rotating" )
	    .AddPoint(
	        "logic_relay", { hammertest::FakeEntityCatalog::Key( "target", "target_destination" ),
	                           hammertest::FakeEntityCatalog::Key( "message", "string" ) } );

	// Keys.
	{
		scene::DocumentEdit edit( doc );
		checks.That( SetKey( edit, { lamp, lamp2 }, "_light", "255 0 0 100" ).HasValue(),
		    "set on two entities" );
		checks.That( *edit.FindEntity( lamp2 )->Key( "_light" ) == "255 0 0 100", "both set" );
		checks.That( SetKey( edit, { lamp, lamp2 }, "_light", "255 0 0 100" ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "an unchanged value is nothing" );
		checks.That( SetKey( edit, { doorSolid }, "speed", "200" ).HasValue() &&
		                 edit.FindEntity( door )->Key( "speed" ),
		    "a brush solid stands for its entity" );
		checks.That(
		    !SetKey( edit, { lamp }, "classname", "x" ) && !SetKey( edit, { lamp }, "id", "3" ),
		    "reserved keys refused (negative)" );
		checks.That(
		    RenameKey( edit, { lamp, lamp2 }, "_light", "_lighthdr" ).HasValue(), "rename a key" );
		checks.That( edit.FindEntity( lamp )->Key( "_lighthdr" ) &&
		                 !edit.FindEntity( lamp )->Key( "_light" ),
		    "renamed" );
		checks.That( !RenameKey( edit, { lamp }, "_lighthdr", "targetname" ),
		    "rename onto an existing key (negative)" );
		checks.That( RemoveKey( edit, { lamp, lamp2 }, "_lighthdr" ).HasValue() &&
		                 !edit.FindEntity( lamp2 )->Key( "_lighthdr" ),
		    "remove" );
		checks.That(
		    RemoveKey( edit, { lamp }, "nothing" ).Error().code == app::EditErrorCode::Nothing,
		    "remove absent" );
		checks.That( SetKey( edit, {}, "a", "b" ).Error().code == app::EditErrorCode::Nothing,
		    "no entities (negative)" );
	}

	// Class changes.
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    SetClass( edit, { lamp }, "light_spot", &catalog ).HasValue(), "point to point" );
		checks.That( edit.FindEntity( lamp )->classname == "light_spot" &&
		                 *edit.FindEntity( lamp )->Key( "_cone" ) == "45",
		    "new class defaults added" );
		checks.That( !SetClass( edit, { lamp }, "func_door", &catalog ),
		    "point to solid refused (negative)" );
		checks.That(
		    !SetClass( edit, { door }, "light", &catalog ), "solid to point refused (negative)" );
		checks.That( SetClass( edit, { door }, "func_door_rotating", &catalog ).HasValue(),
		    "solid to solid" );
		checks.That( !SetClass( edit, { lamp }, "nope", &catalog ), "unknown class (negative)" );
	}

	// Spawnflags.
	{
		scene::DocumentEdit edit( doc );
		checks.That( SetSpawnFlag( edit, { lamp }, 4, true ).HasValue() &&
		                 *edit.FindEntity( lamp )->Key( "spawnflags" ) == "4",
		    "set a flag" );
		checks.That( SetSpawnFlag( edit, { lamp }, 1, true ).HasValue() &&
		                 *edit.FindEntity( lamp )->Key( "spawnflags" ) == "5",
		    "set another" );
		checks.That( SetSpawnFlag( edit, { lamp }, 4, false ).HasValue() &&
		                 *edit.FindEntity( lamp )->Key( "spawnflags" ) == "1",
		    "clear one" );
		checks.That( !SetSpawnFlag( edit, { lamp }, 3, true ), "not a single bit (negative)" );
	}

	// Connections.
	{
		scene::DocumentEdit edit( doc );
		scene::Connection c = *scene::ParseConnection( "OnPressed", "lamp2,TurnOff,,0,-1" );
		checks.That( AddConnection( edit, { button }, c ).HasValue() &&
		                 edit.FindEntity( button )->connections.size() == 3,
		    "add" );
		c.delay = 2;
		checks.That( ReplaceConnection( edit, button, 2, c ).HasValue() &&
		                 edit.FindEntity( button )->connections[2].delay == 2,
		    "replace" );
		checks.That(
		    ReplaceConnection( edit, button, 2, c ).Error().code == app::EditErrorCode::Nothing,
		    "unchanged replace" );
		checks.That( !ReplaceConnection( edit, button, 9, c ), "bad index (negative)" );
		checks.That( RemoveConnections( edit, { button },
		                 []( const scene::Connection &x )
		                 {
			                 return x.target == "door";
		                 } ).HasValue() &&
		                 edit.FindEntity( button )->connections.size() == 2,
		    "remove by predicate" );
		const std::size_t count = edit.FindEntity( button )->connections.size();
		checks.That(
		    AddConnection( edit, { button }, edit.FindEntity( button )->connections[0] ).HasValue(),
		    "add a duplicate" );
		checks.That( RemoveConnectionAt( edit, button, 0 ).HasValue() &&
		                 edit.FindEntity( button )->connections.size() == count,
		    "remove by index removes exactly one of the duplicates" );
		checks.That( !RemoveConnectionAt( edit, button, 99 ), "remove a missing index (negative)" );
		scene::Connection bad;
		bad.output = "OnPressed";
		checks.That( !AddConnection( edit, { button }, bad ), "incomplete connection (negative)" );
		c.delay = -1;
		checks.That( !AddConnection( edit, { button }, c ), "negative delay (negative)" );
	}

	// Rename with reference updates.
	{
		scene::DocumentEdit edit( doc );
		checks.That( RenameEntity( edit, lamp, "hall_lamp", true, &catalog ).HasValue(),
		    "rename with catalog" );
		checks.That( edit.FindEntity( lamp )->Name() == "hall_lamp", "renamed" );
		checks.That( edit.FindEntity( button )->connections[0].target == "hall_lamp",
		    "connection target updated" );
		checks.That( *edit.FindEntity( trigger )->Key( "target" ) == "hall_lamp",
		    "target_destination key updated" );
		checks.That( *edit.FindEntity( trigger )->Key( "message" ) == "lamp",
		    "a plain string key is left alone" );
		checks.That(
		    edit.FindEntity( button )->connections[1].target == "door", "other targets untouched" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That( RenameEntity( edit, lamp, "hall_lamp", true, nullptr ).HasValue(),
		    "rename without catalog" );
		checks.That( *edit.FindEntity( trigger )->Key( "target" ) == "hall_lamp",
		    "conventional key updated" );
		checks.That( RenameEntity( edit, lamp2, "x", false, nullptr ).HasValue() &&
		                 edit.FindEntity( button )->connections[0].target == "hall_lamp",
		    "no reference update when not asked" );
		checks.That( RenameEntity( edit, lamp, "hall_lamp", true, nullptr ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "same name (negative)" );
	}

	// World keys.
	{
		scene::DocumentEdit edit( doc );
		checks.That( SetWorldKey( edit, "skyname", "sky_day01_01" ).HasValue() &&
		                 *edit.Settings().WorldKey( "skyname" ) == "sky_day01_01",
		    "world key" );
		checks.That( SetWorldKey( edit, "skyname", "sky_day01_01" ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "unchanged" );
		checks.That( !SetWorldKey( edit, "classname", "x" ), "reserved world key (negative)" );
	}

	return checks.Report();
}
