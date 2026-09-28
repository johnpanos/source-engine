//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app structural operations (RFC 0002, R08 domain logic):
//			delete cleanup (groups take members, entities take solids, emptied
//			brush entities and groups go, authored empty groups stay), grouping
//			(nesting in a shared parent, brush solids standing for their entity,
//			self-grouping refused), ungrouping, tie to a new or existing brush
//			entity with class defaults, move to world, and quick hide/unhide.
//			Every result passes ValidateEdit. Negative checks: empty selections,
//			point classes for brush entities, unknown classes.
//
//=============================================================================//

#include "hammer/app/ops/structure_ops.h"
#include "hammer/scene/map_queries.h"
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
	auto box = [&]( double x ) { return scene::MakeBoxSolid( { Vec3d( x, 0, 0 ), Vec3d( x + 16, 16, 16 ) }, tex ); };

	scene::MapDocument doc;
	ObjectId a, b, c, group, grouped, door, doorSolid1, doorSolid2, light, emptyGroup;
	{
		scene::DocumentEdit edit( doc );
		a = edit.Add( box( 0 ) );
		b = edit.Add( box( 100 ) );
		c = edit.Add( box( 200 ) );
		group = edit.Add( scene::Group{} );
		scene::Solid g = box( 300 );
		g.group = group;
		grouped = edit.Add( g );
		scene::Entity d;
		d.classname = "func_door";
		d.group = group;
		door = edit.Add( d );
		scene::Solid d1 = box( 400 );
		d1.owner = door;
		doorSolid1 = edit.Add( d1 );
		scene::Solid d2 = box( 500 );
		d2.owner = door;
		doorSolid2 = edit.Add( d2 );
		scene::Entity l;
		l.classname = "light";
		l.SetOrigin( Vec3d( 0, 0, 64 ) );
		light = edit.Add( l );
		emptyGroup = edit.Add( scene::Group{} );
		scene::CommitEdit( doc, edit );
	}

	hammertest::FakeEntityCatalog catalog;
	catalog.AddSolid( "func_detail" ).AddSolid( "func_brush", { hammertest::FakeEntityCatalog::Key( "rendermode", "choices", "0" ) } ).AddPoint( "light" );

	// Delete.
	{
		scene::DocumentEdit edit( doc );
		checks.That( DeleteObjects( edit, { group } ).HasValue(), "delete a group" );
		checks.That( !edit.FindGroup( group ) && !edit.FindSolid( grouped ) && !edit.FindEntity( door ) &&
		                 !edit.FindSolid( doorSolid1 ),
		    "the group takes its members and the entity its solids" );
		checks.That( edit.FindGroup( emptyGroup ) != nullptr, "an authored empty group stays" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after delete" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That( DeleteObjects( edit, { doorSolid1 } ).HasValue() && edit.FindEntity( door ),
		    "deleting one of two solids keeps the entity" );
		checks.That( DeleteObjects( edit, { doorSolid2 } ).HasValue() && !edit.FindEntity( door ),
		    "deleting the last solid deletes the brush entity" );
		checks.That( DeleteObjects( edit, { grouped } ).HasValue() && !edit.FindGroup( group ),
		    "a group emptied by the delete goes too" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after cascades" );
		checks.That( DeleteObjects( edit, {} ).Error().code == app::EditErrorCode::Nothing, "nothing to delete (negative)" );
	}

	// Group / ungroup.
	{
		scene::DocumentEdit edit( doc );
		ObjectId g;
		checks.That( GroupObjects( edit, { a, b, doorSolid1 }, g ).HasValue(), "group" );
		checks.That( scene::GroupMembers( edit, g ) == std::vector<ObjectId>{ a, b, door },
		    "a brush solid stands for its entity" );
		checks.That( edit.FindGroup( g )->group == ObjectId(), "mixed parents: top level" );
		checks.That( !edit.FindGroup( group ) || !scene::GroupMembers( edit, group ).empty(),
		    "the old group keeps its other member" );
		ObjectId inner;
		checks.That( GroupObjects( edit, { a, b }, inner ).HasValue() && edit.FindGroup( inner )->group == g,
		    "a shared parent nests the new group" );
		checks.That( UngroupObjects( edit, { inner } ).HasValue() && edit.FindSolid( a )->group == g &&
		                 !edit.FindGroup( inner ),
		    "ungroup moves members up" );
		checks.That( UngroupObjects( edit, { a } ).Error().code == app::EditErrorCode::Nothing,
		    "ungrouping a non-group (negative)" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after grouping" );
	}

	// Tie to entity and move to world.
	{
		scene::DocumentEdit edit( doc );
		ObjectId e;
		checks.That( TieToEntity( edit, { a, b }, "func_brush", &catalog, ObjectId(), e ).HasValue(), "tie to a new entity" );
		checks.That( scene::EntitySolids( edit, e ) == std::vector<ObjectId>{ a, b }, "solids owned" );
		checks.That( edit.FindEntity( e )->Key( "rendermode" ) && *edit.FindEntity( e )->Key( "rendermode" ) == "0",
		    "class defaults applied" );
		ObjectId same;
		checks.That( TieToEntity( edit, { doorSolid1, doorSolid2 }, "", nullptr, e, same ).HasValue() && same == e,
		    "tie to an existing entity" );
		checks.That( !edit.FindEntity( door ), "the emptied door is deleted" );
		checks.That( MoveToWorld( edit, { e } ).HasValue() && !edit.FindEntity( e ) && !edit.FindSolid( a )->owner.IsValid(),
		    "move to world dissolves the entity" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after tie/move" );
		ObjectId bad;
		checks.That( !TieToEntity( edit, { c }, "light", &catalog, ObjectId(), bad ), "a point class (negative)" );
		checks.That( !TieToEntity( edit, { c }, "nope", &catalog, ObjectId(), bad ), "an unknown class (negative)" );
		checks.That( MoveToWorld( edit, { c } ).Error().code == app::EditErrorCode::Nothing,
		    "a world solid is already in the world (negative)" );
	}

	// Hide.
	{
		scene::DocumentEdit edit( doc );
		checks.That( SetHidden( edit, { a, light }, true ).HasValue(), "hide" );
		checks.That( !scene::IsVisible( edit, a ) && !scene::IsVisible( edit, light ), "hidden" );
		checks.That( SetHidden( edit, { a }, true ).Error().code == app::EditErrorCode::Nothing, "already hidden" );
		checks.That( UnhideAll( edit ).HasValue() && scene::IsVisible( edit, a ), "unhide all" );
		checks.That( HideUnselected( edit, { doorSolid1 } ).HasValue(), "hide unselected" );
		checks.That( scene::IsVisible( edit, doorSolid1 ), "the kept solid stays visible" );
		checks.That( !scene::IsVisible( edit, doorSolid2 ), "its sibling solid is hidden" );
		checks.That( !scene::IsVisible( edit, grouped ) && !scene::IsVisible( edit, a ), "everything else hidden" );
		checks.That( !edit.FindGroup( group )->hidden, "the group holding the kept solid is not hidden itself" );
	}

	return checks.Report();
}
