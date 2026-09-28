//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.scene document index conformance (RFC 0002, R08 domain
//			logic): on a generated document with brush entities, nested groups,
//			names and duplicate ids, every index answer equals the scanning
//			query it replaces (EntitySolids, GroupMembers, FindEntitiesByName)
//			for every object and pattern, plus VMF-id and side-id lookups.
//			Negative checks: empty patterns, unknown ids, a stale index after an
//			edit is detectably stale (the index is a snapshot).
//
//=============================================================================//

#include "hammer/scene/change_set.h"
#include "hammer/scene/document_index.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <random>

using namespace hammer::scene;
using mapgeometry::Vec3d;

int main()
{
	testing::Checks checks;

	MapDocument doc;
	FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	std::mt19937 rng( 20260928u );
	{
		DocumentEdit edit( doc );
		std::vector<ObjectId> groups{ ObjectId() };
		std::vector<ObjectId> brushes;
		for ( int i = 0; i < 12; ++i )
		{
			Group g;
			g.group = groups[rng() % groups.size()];
			groups.push_back( edit.Add( g ) );
		}
		const char *names[] = { "door", "Door_A", "door_b", "lamp", "LAMP", "relay" };
		for ( int i = 0; i < 40; ++i )
		{
			Entity e;
			e.classname = i % 3 ? "func_door" : "light";
			if ( i % 4 )
			{
				e.SetKey( "targetname", names[rng() % 6] );
			}
			e.group = groups[rng() % groups.size()];
			const ObjectId id = edit.Add( e );
			if ( e.classname == "func_door" )
			{
				brushes.push_back( id );
			}
		}
		for ( int i = 0; i < 120; ++i )
		{
			Solid s = MakeBoxSolid( { Vec3d( i * 32.0, 0, 0 ), Vec3d( i * 32.0 + 16, 16, 16 ) }, tex );
			if ( i % 2 )
			{
				s.owner = brushes[rng() % brushes.size()];
			}
			else
			{
				s.group = groups[rng() % groups.size()];
			}
			edit.Add( s );
		}
		// A duplicate VMF id for the id lookup.
		Entity twin;
		twin.classname = "info_target";
		twin.vmfId = edit.FindEntity( edit.EntityIds().front() )->vmfId;
		edit.Add( twin );
		CommitEdit( doc, edit );
	}

	const DocumentIndex index( doc );
	bool solidsAgree = true;
	for ( ObjectId e : doc.EntityIds() )
		solidsAgree = solidsAgree && index.EntitySolids( e ) == EntitySolids( doc, e );
	checks.That( solidsAgree, "entity solids agree with the scan for every entity" );
	bool membersAgree = index.GroupMembers( ObjectId() ) == GroupMembers( doc, ObjectId() );
	for ( ObjectId g : doc.GroupIds() )
		membersAgree = membersAgree && index.GroupMembers( g ) == GroupMembers( doc, g );
	checks.That( membersAgree, "group members agree with the scan for every group and the top level" );
	bool namesAgree = true;
	for ( const char *p : { "door", "DOOR*", "door_*", "lamp", "l*", "relay", "nothing", "*", "Door_A" } )
	{
		namesAgree = namesAgree && index.EntitiesNamed( p ) == FindEntitiesByName( doc, p ) &&
		             index.AnyEntityNamed( p ) == !FindEntitiesByName( doc, p ).empty();
	}
	checks.That( namesAgree, "name lookups agree with the scan for every pattern" );
	checks.That( index.EntitiesNamed( "" ).empty() && !index.AnyEntityNamed( "" ), "empty pattern (negative)" );

	const ObjectId first = doc.EntityIds().front();
	checks.Equal( index.WithVmfId( ObjectKind::Entity, doc.FindEntity( first )->vmfId ).size(), std::size_t( 2 ),
	    "duplicate VMF ids are both listed" );
	checks.That( index.WithVmfId( ObjectKind::Solid, 999999 ).empty(), "unknown VMF id (negative)" );
	const Solid &someSolid = doc.Solids().begin()->second;
	checks.That( index.SolidsWithSide( someSolid.sides[2].vmfId ) == std::vector<ObjectId>{ someSolid.id },
	    "side id lookup" );
	ObjectId unknown;
	unknown.value = 77;
	checks.That( index.EntitySolids( unknown ).empty() && index.GroupMembers( unknown ).empty(), "unknown ids (negative)" );

	// The index is a snapshot: after an edit it no longer matches, and a
	// rebuilt one does.
	{
		DocumentEdit edit( doc );
		const ObjectId door = doc.EntityIds()[1];
		Solid extra = MakeBoxSolid( { Vec3d( 0, 500, 0 ), Vec3d( 16, 516, 16 ) }, tex );
		extra.owner = door;
		edit.Add( extra );
		checks.That( index.EntitySolids( door ) != EntitySolids( edit, door ), "a stale snapshot differs (negative)" );
		checks.That( DocumentIndex( edit ).EntitySolids( door ) == EntitySolids( edit, door ), "a rebuilt index agrees" );
	}

	return checks.Report();
}
