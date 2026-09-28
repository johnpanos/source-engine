//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.scene structural queries (RFC 0002, R08 domain logic): group
//			and brush-entity membership, object expansion to leaves, containers
//			and top-level objects, bounds of solids/point entities/brush
//			entities/groups, Source target-name matching and side lookup. The
//			same queries run on a committed document and a staged edit.
//			Negative checks: unknown ids, empty patterns, originless entities.
//
//=============================================================================//

#include "hammer/scene/change_set.h"
#include "hammer/scene/map_queries.h"
#include "testing/checks.h"

using namespace hammer::scene;
using mapgeometry::Vec3d;

int main()
{
	testing::Checks checks;

	MapDocument doc;
	FaceTexture tex;
	tex.material = "TOOLS/TOOLSTRIGGER";
	ObjectId outer, inner, loose, grouped, door, doorSolid, lamp, bare;
	{
		DocumentEdit edit( doc );
		outer = edit.Add( Group{} );
		Group innerGroup;
		innerGroup.group = outer;
		inner = edit.Add( innerGroup );
		loose = edit.Add( MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ) }, tex ) );
		Solid g = MakeBoxSolid( { Vec3d( 100, 0, 0 ), Vec3d( 116, 16, 16 ) }, tex );
		g.group = inner;
		grouped = edit.Add( g );
		Entity d;
		d.classname = "func_door";
		d.SetKey( "targetname", "Door_01" );
		d.group = outer;
		door = edit.Add( d );
		Solid ds = MakeBoxSolid( { Vec3d( 0, 100, 0 ), Vec3d( 8, 164, 128 ) }, tex );
		ds.owner = door;
		ds.group = inner; // ignored: the entity carries group membership
		doorSolid = edit.Add( ds );
		Entity l;
		l.classname = "light";
		l.SetKey( "targetname", "lamp_a" );
		l.SetOrigin( Vec3d( 50, 50, 50 ) );
		lamp = edit.Add( l );
		Entity n;
		n.classname = "logic_auto";
		bare = edit.Add( n );

		// Queries run on the staged edit too.
		checks.Equal( GroupMembers( edit, outer ).size(), std::size_t( 2 ),
		    "staged: outer has inner + door" );
		CommitEdit( doc, edit );
	}

	checks.That( GroupMembers( doc, outer ) == std::vector<ObjectId>{ inner, door },
	    "outer's direct members" );
	checks.That( GroupMembers( doc, inner ) == std::vector<ObjectId>{ grouped },
	    "brush-entity solids stay with the entity" );
	checks.That( EntitySolids( doc, door ) == std::vector<ObjectId>{ doorSolid }, "entity solids" );

	const std::vector<ObjectId> all = ExpandObjects( doc, { outer } );
	checks.That(
	    all == std::vector<ObjectId>{ outer, inner, grouped, door, doorSolid }, "expand a group" );
	checks.That(
	    ExpandToLeaves( doc, { outer } ) == std::vector<ObjectId>{ grouped, door, doorSolid },
	    "leaves drop groups" );
	checks.That(
	    ExpandObjects( doc, { door, door, doorSolid } ) == std::vector<ObjectId>{ door, doorSolid },
	    "expansion deduplicates" );
	ObjectId unknown;
	unknown.value = 12345;
	checks.That( ExpandObjects( doc, { unknown } ).empty(), "unknown ids are skipped (negative)" );

	checks.That( ContainerOf( doc, doorSolid ) == door, "a brush entity contains its solid" );
	checks.That( ContainerOf( doc, grouped ) == inner, "a group contains its member" );
	checks.That( TopLevelOf( doc, doorSolid ) == outer, "top level climbs entity then groups" );
	checks.That( TopLevelOf( doc, loose ) == loose, "a loose solid is top level" );

	const std::optional<Box> lampBox = ObjectBounds( doc, lamp );
	checks.That(
	    lampBox && lampBox->mins == Vec3d( 42, 42, 42 ) && lampBox->maxs == Vec3d( 58, 58, 58 ),
	    "point entity bounds use the half size" );
	const std::optional<Box> doorBox = ObjectBounds( doc, door );
	checks.That(
	    doorBox && doorBox->mins == Vec3d( 0, 100, 0 ) && doorBox->maxs == Vec3d( 8, 164, 128 ),
	    "brush entity bounds are its solids" );
	const std::optional<Box> groupBox = ObjectBounds( doc, outer );
	checks.That(
	    groupBox && groupBox->mins == Vec3d( 0, 0, 0 ) && groupBox->maxs == Vec3d( 116, 164, 128 ),
	    "group bounds cover nested members" );
	checks.That(
	    !ObjectBounds( doc, bare ), "an originless point entity has no bounds (negative)" );
	checks.That( !ObjectBounds( doc, unknown ), "unknown id has no bounds" );

	checks.That( NameMatches( "door_01", "Door_01" ), "names match case-insensitively" );
	checks.That(
	    NameMatches( "door*", "Door_01" ) && !NameMatches( "door*", "lamp" ), "trailing wildcard" );
	checks.That( !NameMatches( "door", "door_01" ), "no implicit prefix match" );
	checks.That(
	    FindEntitiesByName( doc, "lamp*" ) == std::vector<ObjectId>{ lamp }, "find by name" );
	checks.That( FindEntitiesByName( doc, "" ).empty(), "empty pattern finds nothing (negative)" );
	checks.That(
	    FindEntitiesByClass( doc, "func_*" ) == std::vector<ObjectId>{ door }, "find by class" );

	const std::uint32_t sideId = doc.FindSolid( grouped )->sides[2].vmfId;
	const std::optional<FaceRef> face = FindSideById( doc, sideId );
	checks.That( face && face->solid == grouped && face->side == sideId, "side lookup" );
	checks.That( !FindSideById( doc, 999999 ), "missing side (negative)" );

	// Visibility: own flag, visgroup flag and every container.
	checks.That( IsVisible( doc, doorSolid ) && IsVisible( doc, loose ), "visible by default" );
	{
		DocumentEdit edit( doc );
		edit.MutableGroup( outer )->hidden = true;
		checks.That( !IsVisible( edit, doorSolid ) && !IsVisible( edit, grouped ),
		    "a hidden group hides nested members" );
		checks.That( IsVisible( edit, loose ), "objects outside stay visible" );
		edit.MutableGroup( outer )->hidden = false;
		edit.MutableSolid( loose )->editor.visgroupShown = false;
		checks.That( !IsVisible( edit, loose ), "a hidden visgroup hides the object" );
		edit.MutableEntity( door )->hidden = true;
		checks.That( !IsVisible( edit, doorSolid ), "a hidden brush entity hides its solids" );
	}
	checks.That( !IsVisible( doc, unknown ), "unknown ids are not visible (negative)" );

	return checks.Report();
}
