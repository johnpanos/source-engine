//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app func_instance operations and the instance name rule
//			(RFC 0002, R08 domain logic): fixup styles, global '@' and '!'
//			names, replaceNN parameters (order, case, every occurrence);
//			CollapseInstance substitutes parameters, fixes every name-typed
//			key and connection target (catalog-typed or conventional),
//			rotates then translates the content (solids, entities, overlay
//			bases), gives fresh ids with overlay sides remapped, puts the
//			content in the instance's group, removes the instance, keeps
//			nested instances as entities, and picks the fallback fixup names.
//			Every result passes ValidateEdit. Negative checks: non-instances,
//			unknown fixup styles, malformed origin/angles and instances that
//			own solids stage nothing.
//
//=============================================================================//

#include "hammer/app/instance_fixup.h"
#include "hammer/app/ops/decal_ops.h"
#include "hammer/app/ops/instance_ops.h"
#include "hammer/app/ops/prefab_ops.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "fakes/fake_entity_catalog.h"

#include <cmath>
#include <set>

using namespace hammer;
using namespace hammer::app;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

bool Near( const Vec3d &a, const Vec3d &b, double tolerance = 1e-6 )
{
	return std::fabs( a.x - b.x ) <= tolerance && std::fabs( a.y - b.y ) <= tolerance &&
	       std::fabs( a.z - b.z ) <= tolerance;
}

const scene::Entity *ByClass( const scene::DocumentReader &doc, std::string_view classname )
{
	const std::vector<ObjectId> ids = scene::FindEntitiesByClass( doc, classname );
	return ids.size() == 1 ? doc.FindEntity( ids[0] ) : nullptr;
}

bool UniqueSideIds( const scene::DocumentReader &doc )
{
	std::set<std::uint32_t> sides;
	for ( ObjectId id : doc.SolidIds() )
	{
		for ( const scene::Side &side : doc.FindSolid( id )->sides )
		{
			if ( !sides.insert( side.vmfId ).second )
			{
				return false;
			}
		}
	}
	return true;
}

} // namespace

int main()
{
	testing::Checks checks;

	// --- The rule on strings --------------------------------------------------
	checks.That( ParseFixupStyle( "" ) == InstanceFixupStyle::Prefix &&
	                 ParseFixupStyle( "0" ) == InstanceFixupStyle::Prefix &&
	                 ParseFixupStyle( "1" ) == InstanceFixupStyle::Postfix &&
	                 ParseFixupStyle( "2" ) == InstanceFixupStyle::None,
	    "fixup styles" );
	checks.That(
	    !ParseFixupStyle( "3" ) && !ParseFixupStyle( "prefix" ), "unknown styles (negative)" );
	checks.Equal( FixupInstanceName( "door", "room1", InstanceFixupStyle::Prefix ),
	    std::string( "room1-door" ), "prefix" );
	checks.Equal( FixupInstanceName( "door", "room1", InstanceFixupStyle::Postfix ),
	    std::string( "door-room1" ), "postfix" );
	checks.Equal( FixupInstanceName( "door", "room1", InstanceFixupStyle::None ),
	    std::string( "door" ), "none" );
	checks.That(
	    FixupInstanceName( "@exit", "r", InstanceFixupStyle::Prefix ) == "@exit" &&
	        FixupInstanceName( "!activator", "r", InstanceFixupStyle::Prefix ) == "!activator" &&
	        FixupInstanceName( "", "r", InstanceFixupStyle::Prefix ).empty(),
	    "global and empty names are kept" );
	const std::vector<kvtext::KeyValue> instanceKeys = { { "targetname", "room1" },
	    { "replace01", "$speed 50" }, { "REPLACE02", "$Color 255 0 0" }, { "replace03", "novalue" },
	    { "replace04", " $x 1" }, { "notreplace", "$y 2" } };
	const std::vector<InstanceParameter> params = InstanceParameters( instanceKeys );
	checks.That(
	    params == std::vector<InstanceParameter>{ { "$speed", "50" }, { "$Color", "255 0 0" } },
	    "replace keys in order; no-space and leading-space values skipped" );
	checks.Equal( SubstituteInstanceParameters( "$SPEED/$speed $color", params ),
	    std::string( "50/50 255 0 0" ), "case-insensitive, every occurrence" );
	checks.Equal( SubstituteInstanceParameters( "$speedy", params ), std::string( "50y" ),
	    "plain substring substitution (legacy)" );
	checks.Equal( SubstituteInstanceParameters( "$a", { { "$a", "$b" }, { "$b", "c" } } ),
	    std::string( "c" ), "parameters apply in order" );

	// --- The instance content --------------------------------------------------
	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::MapDocument contentDoc( 9 );
	std::uint32_t contentTop = 0;
	{
		scene::DocumentEdit edit( contentDoc );
		const ObjectId floor =
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 16, 32, 8 ) }, tex ) );
		for ( const scene::Side &side : edit.FindSolid( floor )->sides )
		{
			contentTop = side.Plane().normal.z > 0.5 ? side.vmfId : contentTop;
		}
		scene::Entity relay;
		relay.classname = "logic_relay";
		relay.SetKey( "targetname", "relay" );
		relay.SetKey( "mykey", "door" );
		relay.SetKey( "note", "door" );
		relay.SetOrigin( Vec3d( 10, 0, 0 ) );
		relay.connections.push_back(
		    *scene::ParseConnection( "OnTrigger", "door,Open,$speed,0,-1" ) );
		relay.connections.push_back( *scene::ParseConnection( "OnTrigger", "@exit,Kill,,0,-1" ) );
		relay.connections.push_back(
		    *scene::ParseConnection( "OnTrigger", "!activator,Use,,0,-1" ) );
		edit.Add( relay );
		scene::Entity door;
		door.classname = "func_door";
		door.SetKey( "targetname", "door" );
		door.SetKey( "speed", "$speed" );
		door.SetKey( "rendercolor", "$color" );
		door.SetKey( "parentname", "outside_thing" );
		door.SetKey( "angles", "0 0 0" );
		const ObjectId doorId = edit.Add( door );
		scene::Solid leaf = scene::MakeBoxSolid( { Vec3d( 0, 40, 0 ), Vec3d( 8, 48, 64 ) }, tex );
		leaf.owner = doorId;
		edit.Add( leaf );
		scene::Entity nested;
		nested.classname = "func_instance";
		nested.SetKey( "targetname", "sub" );
		nested.SetKey( "file", "sub.vmf" );
		nested.SetKey( "replace01", "$x $speed" );
		nested.SetOrigin( Vec3d( 0, 0, 0 ) );
		edit.Add( nested );
		ObjectId overlay;
		checks.That( PlaceOverlay( edit, { scene::FaceRef{ floor, contentTop } }, Vec3d( 8, 16, 8 ),
		                 "decals/x", 8, 8, overlay )
		                 .HasValue(),
		    "content overlay" );
		checks.That( scene::ValidateEdit( edit ).empty(), "content fixture valid" );
		scene::CommitEdit( contentDoc, edit );
	}
	const MapFragment content = *FragmentFromDocument( contentDoc );

	// --- The map ---------------------------------------------------------------
	scene::MapDocument doc;
	ObjectId instance, group, plain, owner;
	{
		scene::DocumentEdit edit( doc );
		group = edit.Add( scene::Group{} );
		scene::Entity inst;
		inst.classname = "func_instance";
		inst.group = group;
		inst.SetKey( "targetname", "room1" );
		inst.SetKey( "file", "instances\\room" );
		inst.SetKey( "fixup_style", "0" );
		inst.SetKey( "replace01", "$speed 50" );
		inst.SetKey( "replace02", "$color 255 0 0" );
		inst.SetOrigin( Vec3d( 100, 0, 0 ) );
		inst.SetAngles( Vec3d( 0, 90, 0 ) );
		instance = edit.Add( inst );
		scene::Entity unnamed;
		unnamed.classname = "FUNC_INSTANCE";
		unnamed.SetKey( "file", "a.b/c" );
		plain = edit.Add( unnamed );
		scene::Entity taken;
		taken.classname = "info_target";
		taken.SetKey( "targetname", "InstanceAuto1-x" );
		edit.Add( taken );
		scene::Entity withSolid;
		withSolid.classname = "func_instance";
		owner = edit.Add( withSolid );
		scene::Solid bad = scene::MakeBoxSolid( { Vec3d( 500, 0, 0 ), Vec3d( 516, 16, 16 ) }, tex );
		bad.owner = owner;
		edit.Add( bad );
		scene::CommitEdit( doc, edit );
	}
	checks.Equal( FindInstances( doc ).size(), std::size_t( 3 ), "FindInstances (any class case)" );
	checks.Equal( InstanceFile( *doc.FindEntity( instance ) ), std::string( "instances/room.vmf" ),
	    "file: slashes fixed, extension added" );
	checks.Equal( InstanceFile( *doc.FindEntity( plain ) ), std::string( "a.b/c.vmf" ),
	    "a dot in a directory is not an extension" );
	{
		scene::Entity e;
		e.classname = "func_instance";
		e.SetKey( "file", "x.vmf" );
		scene::Entity notInstance;
		notInstance.classname = "info_target";
		notInstance.SetKey( "file", "x.vmf" );
		checks.That( InstanceFile( e ) == "x.vmf" && InstanceFile( notInstance ).empty() &&
		                 InstanceFile( *doc.FindEntity( owner ) ).empty(),
		    "file kept; non-instances and missing files are empty" );
	}

	// Collapse, prefix style, no catalog.
	{
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		checks.That( CollapseInstance( edit, instance, content, &created ).HasValue(), "collapse" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after collapse" );
		checks.That( UniqueSideIds( edit ), "fresh side ids" );
		checks.That( !edit.FindEntity( instance ), "the instance is removed" );
		checks.Equal( created.size(), std::size_t( 5 ), "top-level content objects reported" );
		bool grouped = !created.empty();
		for ( ObjectId id : created )
		{
			grouped = grouped && scene::ContainerOf( edit, id ) == group;
		}
		checks.That( grouped, "content joins the instance's group" );

		const scene::Entity *relay = ByClass( edit, "logic_relay" );
		checks.That( relay && relay->Name() == "room1-relay", "targetname prefixed" );
		checks.That(
		    relay && relay->connections[0].target == "room1-door", "connection target prefixed" );
		checks.That(
		    relay && relay->connections[0].parameter == "50", "parameter substituted in I/O" );
		checks.That( relay && relay->connections[1].target == "@exit" &&
		                 relay->connections[2].target == "!activator",
		    "global targets are kept" );
		checks.That( relay && *relay->Key( "mykey" ) == "door" && *relay->Key( "note" ) == "door",
		    "without a catalog only conventional keys are names" );
		checks.That( relay && Near( *relay->Origin(), Vec3d( 100, 10, 0 ) ),
		    "origin rotated by yaw 90 then translated" );
		const scene::Entity *door = ByClass( edit, "func_door" );
		checks.That( door && door->Name() == "room1-door" && *door->Key( "speed" ) == "50" &&
		                 *door->Key( "rendercolor" ) == "255 0 0",
		    "parameters substituted in keys" );
		checks.That( door && *door->Key( "parentname" ) == "room1-outside_thing",
		    "names not defined in the instance are fixed too (compiler rule)" );
		checks.That( door && door->Angles() && Near( *door->Angles(), Vec3d( 0, 90, 0 ) ),
		    "entity angles composed with the instance's" );
		const std::vector<ObjectId> doorIds = scene::FindEntitiesByClass( edit, "func_door" );
		const std::vector<ObjectId> leaves =
		    doorIds.empty() ? std::vector<ObjectId>() : scene::EntitySolids( edit, doorIds[0] );
		const std::optional<scene::Box> leafBox =
		    leaves.size() == 1 ? scene::SolidBounds( *edit.FindSolid( leaves[0] ) ) : std::nullopt;
		checks.That( leafBox && Near( leafBox->mins, Vec3d( 52, 0, 0 ) ) &&
		                 Near( leafBox->maxs, Vec3d( 60, 8, 64 ) ),
		    "brush entity solids transformed with their entity" );
		const std::vector<ObjectId> instances = FindInstances( edit );
		const scene::Entity *sub = nullptr;
		for ( ObjectId id : instances )
		{
			sub = edit.FindEntity( id )->Name() == "room1-sub" ? edit.FindEntity( id ) : sub;
		}
		checks.That( sub && *sub->Key( "file" ) == "sub.vmf" && *sub->Key( "replace01" ) == "$x 50",
		    "a nested instance is kept as an entity with its keys fixed" );
		const scene::Entity *ov = ByClass( edit, "info_overlay" );
		const std::optional<scene::FaceRef> face =
		    ov ? scene::FindSideById(
		             edit, static_cast<std::uint32_t>( std::stoul( *ov->Key( "sides" ) ) ) )
		       : std::nullopt;
		checks.That(
		    face && edit.FindSolid( face->solid )->FindSide( face->side )->Plane().normal.z > 0.5,
		    "overlay sides name the merged top face" );
		checks.That( ov && *ov->Key( "BasisOrigin" ) == "84 8 8" && *ov->Key( "BasisU" ) == "0 1 0",
		    "overlay basis transformed" );
		std::vector<ObjectId> floors;
		for ( ObjectId id : created )
		{
			if ( edit.FindSolid( id ) )
			{
				floors.push_back( id );
			}
		}
		const std::optional<scene::Box> floorBox =
		    floors.size() == 1 ? scene::SolidBounds( *edit.FindSolid( floors[0] ) ) : std::nullopt;
		checks.That( floorBox && Near( floorBox->mins, Vec3d( 68, 0, 0 ) ) &&
		                 Near( floorBox->maxs, Vec3d( 100, 16, 8 ) ),
		    "world solid rotated then translated" );
		scene::MapDocument committed = doc;
		scene::CommitEdit( committed, edit );
		checks.That( committed.Validate().empty(), "commits cleanly" );
	}

	// Postfix, none, the catalog, and fallback fixup names.
	{
		scene::DocumentEdit edit( doc );
		edit.MutableEntity( instance )->SetKey( "fixup_style", "1" );
		checks.That( CollapseInstance( edit, instance, content ).HasValue(), "collapse postfix" );
		const scene::Entity *relay = ByClass( edit, "logic_relay" );
		checks.That(
		    relay && relay->Name() == "relay-room1" && relay->connections[0].target == "door-room1",
		    "postfix names" );
	}
	{
		scene::DocumentEdit edit( doc );
		edit.MutableEntity( instance )->SetKey( "fixup_style", "2" );
		checks.That(
		    CollapseInstance( edit, instance, content ).HasValue(), "collapse without fixup" );
		const scene::Entity *relay = ByClass( edit, "logic_relay" );
		checks.That( relay && relay->Name() == "relay" && relay->connections[0].parameter == "50",
		    "no fixup, parameters still substituted" );
	}
	{
		hammertest::FakeEntityCatalog catalog;
		using C = hammertest::FakeEntityCatalog;
		catalog.AddPoint( "logic_relay",
		    { C::Key( "targetname", "target_source" ), C::Key( "mykey", "target_destination" ),
		        C::Key( "note", "string" ) } );
		scene::DocumentEdit edit( doc );
		checks.That( CollapseInstance( edit, instance, content, nullptr, &catalog ).HasValue(),
		    "collapse with a catalog" );
		const scene::Entity *relay = ByClass( edit, "logic_relay" );
		checks.That(
		    relay && *relay->Key( "mykey" ) == "room1-door" && *relay->Key( "note" ) == "door",
		    "catalog-typed name keys are fixed, others not" );
		const scene::Entity *door = ByClass( edit, "func_door" );
		checks.That(
		    door && door->Name() == "room1-door", "unknown classes use the conventional keys" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    CollapseInstance( edit, plain, content ).HasValue(), "collapse an unnamed instance" );
		const scene::Entity *relay = ByClass( edit, "logic_relay" );
		checks.That( relay && relay->Name() == "InstanceAuto2-relay",
		    "automatic fixup name skips one already in use" );
		checks.That( relay && Near( *relay->Origin(), Vec3d( 10, 0, 0 ) ),
		    "no origin or angles: identity placement" );
	}
	{
		scene::DocumentEdit edit( doc );
		scene::Entity *e = edit.MutableEntity( plain );
		e->SetKey( "name", "legacyname" );
		checks.That( CollapseInstance( edit, plain, content ).HasValue() &&
		                 ByClass( edit, "logic_relay" )->Name() == "legacyname-relay",
		    "the \"name\" key is the second fixup source" );
	}
	{
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		checks.That( CollapseInstance( edit, plain, MapFragment{}, &created ).HasValue() &&
		                 !edit.FindEntity( plain ) && created.empty() &&
		                 scene::ValidateEdit( edit ).empty(),
		    "empty content only removes the instance" );
	}

	// Refusals stage nothing.
	{
		scene::DocumentEdit probe( doc );
		const ObjectId target = scene::FindEntitiesByClass( probe, "info_target" ).front();
		scene::DocumentEdit edit( doc );
		checks.That(
		    CollapseInstance( edit, target, content ).Error().code == EditErrorCode::Rejected,
		    "not an instance (negative)" );
		checks.That(
		    CollapseInstance( edit, ObjectId(), content ).Error().code == EditErrorCode::Rejected,
		    "unknown id (negative)" );
		checks.That(
		    CollapseInstance( edit, owner, content ).Error().code == EditErrorCode::Rejected,
		    "an instance owning solids (negative)" );
		scene::MapDocument broken = doc;
		{
			scene::DocumentEdit setup( broken );
			setup.MutableEntity( instance )->SetKey( "fixup_style", "7" );
			setup.MutableEntity( plain )->SetKey( "origin", "1 2" );
			scene::CommitEdit( broken, setup );
		}
		scene::DocumentEdit edit2( broken );
		checks.That(
		    CollapseInstance( edit2, instance, content ).Error().code == EditErrorCode::Rejected,
		    "unknown fixup_style (negative)" );
		checks.That(
		    CollapseInstance( edit2, plain, content ).Error().code == EditErrorCode::Rejected,
		    "malformed origin (negative)" );
		{
			scene::DocumentEdit setup( broken );
			setup.MutableEntity( plain )->SetKey( "origin", "0 0 0" );
			setup.MutableEntity( plain )->SetKey( "angles", "x y z" );
			scene::CommitEdit( broken, setup );
		}
		scene::DocumentEdit edit3( broken );
		checks.That(
		    CollapseInstance( edit3, plain, content ).Error().code == EditErrorCode::Rejected,
		    "malformed angles (negative)" );
		checks.That( edit.Finish().Empty() && edit2.Finish().Empty() && edit3.Finish().Empty(),
		    "refusals staged nothing" );
	}

	return checks.Report();
}
