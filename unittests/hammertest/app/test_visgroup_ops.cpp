//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app visgroup operations (RFC 0002, R08 domain logic): the
//			visgroup tree (fresh ids past dangling references, create under a
//			parent, rename, delete with children moving up and membership
//			removed, reparent with cycles refused), membership (a brush solid
//			stands for its entity, a group for itself, "move to visgroup"),
//			show/hide over a visgroup's subtree and the objects its members
//			contain, the several-visgroups rule (shown only while every
//			visgroup is shown), the presenter state, and orphan restoration.
//			Every result passes ValidateEdit. Negative checks: unknown ids,
//			empty names, cycles, no-op requests.
//
//=============================================================================//

#include "hammer/app/ops/visgroup_ops.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <algorithm>

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

bool HasVisgroup( const scene::DocumentReader &doc, ObjectId id, int visgroup )
{
	const scene::EditorInfo *info = nullptr;
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		info = &s->editor;
	}
	else if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		info = &e->editor;
	}
	else if ( const scene::Group *g = doc.FindGroup( id ) )
	{
		info = &g->editor;
	}
	return info && std::find( info->visgroupIds.begin(), info->visgroupIds.end(), visgroup ) !=
	                   info->visgroupIds.end();
}

bool Shown( const scene::DocumentReader &doc, ObjectId id )
{
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		return s->editor.visgroupShown;
	}
	if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		return e->editor.visgroupShown;
	}
	return doc.FindGroup( id ) && doc.FindGroup( id )->editor.visgroupShown;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	auto box = [&]( double x, std::vector<int> visgroups = {} )
	{
		scene::Solid s = scene::MakeBoxSolid( { Vec3d( x, 0, 0 ), Vec3d( x + 16, 16, 16 ) }, tex );
		s.editor.visgroupIds = std::move( visgroups );
		return s;
	};

	// Tree: 1 walls { 2 inner }, 3 props.
	scene::MapDocument doc;
	ObjectId a, b, c, group, g1, door, d1, d2, light;
	{
		scene::DocumentEdit edit( doc );
		scene::Visgroup inner;
		inner.id = 2;
		inner.name = "inner";
		scene::Visgroup walls;
		walls.id = 1;
		walls.name = "walls";
		walls.children.push_back( inner );
		scene::Visgroup props;
		props.id = 3;
		props.name = "props";
		edit.MutableSettings().visgroups = { walls, props };
		a = edit.Add( box( 0, { 1, 3 } ) );
		b = edit.Add( box( 100, { 3 } ) );
		c = edit.Add( box( 200, { 2 } ) );
		scene::Group gr;
		gr.editor.visgroupIds = { 1 };
		group = edit.Add( gr );
		scene::Solid gs = box( 300 );
		gs.group = group;
		g1 = edit.Add( gs );
		scene::Entity d;
		d.classname = "func_door";
		door = edit.Add( d );
		scene::Solid s1 = box( 400 );
		s1.owner = door;
		d1 = edit.Add( s1 );
		scene::Solid s2 = box( 500 );
		s2.owner = door;
		d2 = edit.Add( s2 );
		scene::Entity l;
		l.classname = "light";
		l.SetOrigin( Vec3d( 0, 0, 64 ) );
		light = edit.Add( l );
		scene::CommitEdit( doc, edit );
	}

	// Ids and the tree.
	checks.Equal( NextVisgroupId( doc ), 4, "next id is past the tree" );
	{
		scene::DocumentEdit edit( doc );
		edit.MutableSolid( b )->editor.visgroupIds.push_back( 9 );
		checks.Equal( NextVisgroupId( edit ), 10, "next id is past a dangling reference too" );
	}
	{
		scene::DocumentEdit edit( doc );
		int id = 0;
		checks.That(
		    CreateVisgroup( edit, "trim", 1, id ).HasValue() && id == 4, "create under a parent" );
		const scene::VisgroupLookup found = scene::FindVisgroup( edit.Settings().visgroups, 4 );
		checks.That( found.visgroup && found.parentId == 1 && found.visgroup->name == "trim",
		    "created in place" );
		int top = 0;
		checks.That( CreateVisgroup( edit, "trim", 0, top ).HasValue() && top == 5 &&
		                 edit.Settings().visgroups.back().id == 5,
		    "duplicate names allowed; top level appended" );
		int bad = 0;
		checks.That(
		    CreateVisgroup( edit, "", 0, bad ).Error().code == app::EditErrorCode::Rejected,
		    "an empty name is refused (negative)" );
		checks.That(
		    CreateVisgroup( edit, "x", 77, bad ).Error().code == app::EditErrorCode::Rejected,
		    "an unknown parent is refused (negative)" );
		checks.That(
		    RenameVisgroup( edit, 3, "decor" ).HasValue() &&
		        scene::FindVisgroup( edit.Settings().visgroups, 3 ).visgroup->name == "decor",
		    "rename" );
		checks.That( RenameVisgroup( edit, 3, "decor" ).Error().code == app::EditErrorCode::Nothing,
		    "same name is nothing to do" );
		checks.That( RenameVisgroup( edit, 42, "q" ).Error().code == app::EditErrorCode::Rejected,
		    "renaming an unknown visgroup (negative)" );
		checks.That( RenameVisgroup( edit, 3, "" ).Error().code == app::EditErrorCode::Rejected,
		    "renaming to empty (negative)" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after tree edits" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That( MoveVisgroup( edit, 1, 2 ).Error().code == app::EditErrorCode::Rejected,
		    "moving into a descendant is a cycle (negative)" );
		checks.That( MoveVisgroup( edit, 1, 1 ).Error().code == app::EditErrorCode::Rejected,
		    "moving into itself (negative)" );
		checks.That( MoveVisgroup( edit, 1, 99 ).Error().code == app::EditErrorCode::Rejected,
		    "an unknown new parent (negative)" );
		checks.That( MoveVisgroup( edit, 2, 1 ).Error().code == app::EditErrorCode::Nothing,
		    "already under that parent" );
		checks.That( MoveVisgroup( edit, 2, 0 ).HasValue(), "move to top level" );
		checks.That(
		    edit.Settings().visgroups.size() == 3 && edit.Settings().visgroups.back().id == 2 &&
		        scene::FindVisgroup( edit.Settings().visgroups, 1 ).visgroup->children.empty(),
		    "reparented and appended" );
		checks.That( MoveVisgroup( edit, 1, 3 ).HasValue() &&
		                 scene::FindVisgroup( edit.Settings().visgroups, 1 ).parentId == 3,
		    "move under another" );
		checks.That(
		    edit.Base().Settings().visgroups.size() == 2, "the base document is untouched" );
	}

	// Membership.
	{
		scene::DocumentEdit edit( doc );
		checks.That( AddToVisgroup( edit, { d1, g1, light }, 3 ).HasValue(), "add" );
		checks.That( HasVisgroup( edit, door, 3 ) && !HasVisgroup( edit, d1, 3 ),
		    "a brush solid stands for its entity" );
		checks.That( HasVisgroup( edit, g1, 3 ) && HasVisgroup( edit, light, 3 ),
		    "group members can belong" );
		checks.That(
		    AddToVisgroup( edit, { light }, 3 ).Error().code == app::EditErrorCode::Nothing,
		    "already a member" );
		checks.That(
		    AddToVisgroup( edit, { light }, 42 ).Error().code == app::EditErrorCode::Rejected,
		    "an unknown visgroup (negative)" );
		checks.That(
		    AddToVisgroup( edit, { ObjectId() }, 3 ).Error().code == app::EditErrorCode::Nothing,
		    "no live objects" );
		checks.That( AddToVisgroup( edit, { a }, 2, true ).HasValue(), "move to visgroup" );
		const scene::Solid *sa = edit.FindSolid( a );
		checks.That( sa->editor.visgroupIds == std::vector<int>{ 2 }, "other memberships dropped" );
		checks.That( RemoveFromVisgroup( edit, { light, door }, 3 ).HasValue() &&
		                 !HasVisgroup( edit, light, 3 ) && !HasVisgroup( edit, door, 3 ),
		    "remove" );
		checks.That(
		    RemoveFromVisgroup( edit, { light }, 3 ).Error().code == app::EditErrorCode::Nothing,
		    "not a member" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after membership edits" );
		const std::vector<ObjectId> props = VisgroupMembers( edit, 3 );
		checks.That( props == std::vector<ObjectId>{ b, g1 }, "members of a visgroup" );
		checks.That( VisgroupMembers( edit, 1, false ) == std::vector<ObjectId>{ group } &&
		                 VisgroupMembers( edit, 1, true ) == std::vector<ObjectId>{ a, c, group },
		    "direct and recursive members" );
	}

	// Show and hide.
	{
		scene::DocumentEdit edit( doc );
		checks.That( VisgroupVisibility( edit, 1 ) == VisgroupState::Shown, "initially shown" );
		checks.That( SetVisgroupVisible( edit, 1, false ).HasValue(), "hide walls" );
		checks.That(
		    !Shown( edit, a ) && !Shown( edit, c ), "members and child-visgroup members hidden" );
		checks.That(
		    !Shown( edit, group ) && !Shown( edit, g1 ), "a group member's contents hidden too" );
		checks.That(
		    !scene::IsVisible( edit, g1 ) && !scene::IsVisible( edit, c ), "IsVisible agrees" );
		checks.That(
		    Shown( edit, b ) && Shown( edit, light ) && Shown( edit, d1 ), "others untouched" );
		checks.That( VisgroupVisibility( edit, 1 ) == VisgroupState::Hidden &&
		                 VisgroupVisibility( edit, 2 ) == VisgroupState::Hidden,
		    "presenter: hidden" );
		checks.That( VisgroupVisibility( edit, 3 ) == VisgroupState::Mixed,
		    "presenter: mixed (a hidden, b shown)" );
		checks.That(
		    SetVisgroupVisible( edit, 1, false ).Error().code == app::EditErrorCode::Nothing,
		    "already hidden" );
		checks.That(
		    SetVisgroupVisible( edit, 1, true ).HasValue() && Shown( edit, a ) && Shown( edit, g1 ),
		    "show again: props (b shown) does not suppress a" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after show/hide" );
	}
	{
		// a is in walls and props; b only in props.
		scene::DocumentEdit edit( doc );
		checks.That( SetVisgroupVisible( edit, 1, false ).HasValue() &&
		                 SetVisgroupVisible( edit, 3, false ).HasValue() && !Shown( edit, a ) &&
		                 !Shown( edit, b ) && !Shown( edit, c ),
		    "hide walls and props" );
		checks.That( SetVisgroupVisible( edit, 1, true ).HasValue(), "show walls" );
		checks.That( !Shown( edit, a ), "a stays hidden: props is still hidden" );
		checks.That( Shown( edit, c ) && Shown( edit, g1 ), "walls' other objects shown" );
		checks.That( VisgroupVisibility( edit, 1 ) == VisgroupState::Mixed, "walls is mixed" );
		checks.That(
		    SetVisgroupVisible( edit, 3, true ).HasValue() && Shown( edit, a ) && Shown( edit, b ),
		    "showing props shows both" );
		checks.That(
		    SetVisgroupVisible( edit, 3, true ).Error().code == app::EditErrorCode::Nothing,
		    "already shown" );
		checks.That(
		    SetVisgroupVisible( edit, 99, true ).Error().code == app::EditErrorCode::Rejected,
		    "an unknown visgroup (negative)" );
		// A visgroup whose members are all shared has no state of its own.
		checks.That( RemoveFromVisgroup( edit, { b }, 3 ).HasValue(), "props now holds only a" );
		checks.That( SetVisgroupVisible( edit, 3, false ).HasValue() &&
		                 SetVisgroupVisible( edit, 1, false ).HasValue() &&
		                 SetVisgroupVisible( edit, 1, true ).HasValue(),
		    "hide props and walls, show walls" );
		checks.That( Shown( edit, a ), "a props visgroup with no other member does not suppress" );
		int empty = 0;
		checks.That( CreateVisgroup( edit, "empty", 0, empty ).HasValue() &&
		                 VisgroupVisibility( edit, empty ) == VisgroupState::Empty &&
		                 VisgroupVisibility( edit, 1234 ) == VisgroupState::Empty,
		    "presenter: empty and unknown" );
		checks.That(
		    SetVisgroupVisible( edit, empty, false ).Error().code == app::EditErrorCode::Nothing,
		    "an empty visgroup has nothing to hide" );
	}

	// Delete and orphans.
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    SetVisgroupVisible( edit, 2, false ).HasValue() && !Shown( edit, c ), "hide inner" );
		checks.That( DeleteVisgroup( edit, 2 ).HasValue(), "delete inner" );
		checks.That( !scene::FindVisgroup( edit.Settings().visgroups, 2 ).visgroup &&
		                 !HasVisgroup( edit, c, 2 ),
		    "the visgroup and its memberships are gone" );
		checks.That( Shown( edit, c ), "an orphaned hidden object is shown" );
		checks.That( DeleteVisgroup( edit, 2 ).Error().code == app::EditErrorCode::Rejected,
		    "deleting an unknown visgroup (negative)" );
		int child = 0;
		int grandchild = 0;
		checks.That( CreateVisgroup( edit, "c1", 3, child ).HasValue() &&
		                 CreateVisgroup( edit, "c2", 3, grandchild ).HasValue(),
		    "two children of props" );
		checks.That( DeleteVisgroup( edit, 3 ).HasValue(), "delete props" );
		const std::vector<scene::Visgroup> &tree = edit.Settings().visgroups;
		checks.That( tree.size() == 3 && tree[1].id == child && tree[2].id == grandchild,
		    "children move to the parent, at its place, in order" );
		checks.That( !HasVisgroup( edit, a, 3 ) && HasVisgroup( edit, a, 1 ),
		    "only that membership removed" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after deletes" );
	}
	{
		// A group hidden through its visgroup: its member is shown again when
		// the group leaves the visgroup.
		scene::DocumentEdit edit( doc );
		checks.That( SetVisgroupVisible( edit, 1, false ).HasValue(), "hide walls" );
		checks.That(
		    RemoveFromVisgroup( edit, { group }, 1 ).HasValue(), "the group leaves walls" );
		checks.That(
		    Shown( edit, group ) && Shown( edit, g1 ), "the group and its member are shown again" );
		checks.That( !Shown( edit, a ), "a is still covered by walls and props: stays hidden" );
		checks.That( CoveredByVisgroup( edit, a ) && !CoveredByVisgroup( edit, g1 ), "coverage" );
	}

	return checks.Report();
}
