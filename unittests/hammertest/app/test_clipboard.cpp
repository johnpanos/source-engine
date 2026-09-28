//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app clipboard (RFC 0002, R08 domain logic): Copy expands
//			like ExpandObjects and detaches the fragment (references outside it
//			dropped: a brush solid alone pastes as a world solid), Paste gives
//			new runtime ids and fresh VMF ids to everything, remaps owners,
//			groups and overlay side lists, matches or creates visgroups by name,
//			Paste Special repeats with cumulative offset and rotation (texture
//			lock), wraps copies in groups, and fixes pasted names (suffix,
//			prefix, uniqueness) with references inside the pasted set updated
//			and outside ones untouched; Duplicate. Every result passes
//			ValidateEdit and commits. Negative checks: empty clipboard, copy
//			counts, non-finite offsets and angles leave nothing staged.
//
//=============================================================================//

#include "hammer/app/clipboard.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <cmath>
#include <limits>
#include <set>
#include <sstream>

using namespace hammer;
using namespace hammer::app;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

bool Near( const Vec3d &a, const Vec3d &b, double tolerance = 1e-6 )
{
	return std::fabs( a.x - b.x ) <= tolerance && std::fabs( a.y - b.y ) <= tolerance &&
	       std::fabs( a.z - b.z ) <= tolerance;
}

// Every VMF id (solids, sides, entities, groups) of 'doc' is unique.
bool UniqueVmfIds( const scene::DocumentReader &doc )
{
	std::set<std::uint32_t> solids;
	std::set<std::uint32_t> sides;
	std::set<std::uint32_t> entities;
	std::set<std::uint32_t> groups;
	for ( ObjectId id : doc.SolidIds() )
	{
		const scene::Solid *s = doc.FindSolid( id );
		if ( !solids.insert( s->vmfId ).second )
		{
			return false;
		}
		for ( const scene::Side &side : s->sides )
		{
			if ( !sides.insert( side.vmfId ).second )
			{
				return false;
			}
		}
	}
	for ( ObjectId id : doc.EntityIds() )
	{
		if ( !entities.insert( doc.FindEntity( id )->vmfId ).second )
		{
			return false;
		}
	}
	for ( ObjectId id : doc.GroupIds() )
	{
		if ( !groups.insert( doc.FindGroup( id )->vmfId ).second )
		{
			return false;
		}
	}
	return true;
}

std::vector<ObjectId> Created( const scene::DocumentEdit &edit )
{
	return edit.Finish().Created();
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	auto box = [&]( Vec3d mins, Vec3d maxs )
	{
		return scene::MakeBoxSolid( { mins, maxs }, tex );
	};

	scene::MapDocument doc;
	ObjectId group, gs, lamp, trigger, outside, door, d1, d2, overlay;
	std::uint32_t d1Side0 = 0, d1Side1 = 0;
	{
		scene::DocumentEdit edit( doc );
		scene::Visgroup walls;
		walls.id = 1;
		walls.name = "walls";
		walls.color = scene::Rgb{ 10, 20, 30 };
		edit.MutableSettings().visgroups = { walls };
		group = edit.Add( scene::Group{} );
		scene::Solid s = box( Vec3d( 0, 0, 0 ), Vec3d( 64, 16, 16 ) );
		s.group = group;
		s.editor.visgroupIds = { 1, 42 }; // 42 is not defined: not carried
		gs = edit.Add( s );
		scene::Entity l;
		l.classname = "light";
		l.group = group;
		l.SetKey( "targetname", "lamp" );
		l.SetOrigin( Vec3d( 32, 8, 32 ) );
		l.SetAngles( Vec3d( 0, 0, 0 ) );
		lamp = edit.Add( l );
		scene::Entity t;
		t.classname = "trigger_once";
		t.group = group;
		t.SetOrigin( Vec3d( 8, 8, 8 ) );
		t.SetKey( "target", "LAMP" );
		t.SetKey( "target2", "outside" );
		t.connections.push_back( *scene::ParseConnection( "OnTrigger", "lamp,TurnOff,,0,-1" ) );
		t.connections.push_back( *scene::ParseConnection( "OnTrigger", "outside,Kill,lamp,0,-1" ) );
		trigger = edit.Add( t );
		scene::Entity o;
		o.classname = "info_target";
		o.SetKey( "targetname", "outside" );
		o.SetOrigin( Vec3d( 500, 0, 0 ) );
		outside = edit.Add( o );
		scene::Entity d;
		d.classname = "func_door";
		d.SetKey( "targetname", "door" );
		door = edit.Add( d );
		scene::Solid s1 = box( Vec3d( 200, 0, 0 ), Vec3d( 216, 16, 64 ) );
		s1.owner = door;
		d1 = edit.Add( s1 );
		scene::Solid s2 = box( Vec3d( 216, 0, 0 ), Vec3d( 232, 16, 64 ) );
		s2.owner = door;
		d2 = edit.Add( s2 );
		d1Side0 = edit.FindSolid( d1 )->sides[0].vmfId;
		d1Side1 = edit.FindSolid( d1 )->sides[1].vmfId;
		scene::Entity ov;
		ov.classname = "info_overlay";
		ov.SetOrigin( Vec3d( 208, 0, 32 ) );
		ov.SetKey( "sides", std::to_string( d1Side0 ) + " " + std::to_string( d1Side1 ) + " 999" );
		overlay = edit.Add( ov );
		checks.That( scene::ValidateEdit( edit ).empty(), "fixture valid" );
		scene::CommitEdit( doc, edit );
	}

	// Copy.
	const MapFragment groupFragment = Copy( doc, { group } );
	checks.Equal( groupFragment.Count(), std::size_t( 4 ), "a group brings its members" );
	checks.That( groupFragment.visgroups.size() == 1 && groupFragment.visgroups[0].name == "walls",
	    "the named visgroup is carried" );
	checks.That( std::get<scene::Solid>( groupFragment.objects[1] ).editor.visgroupIds ==
	                 std::vector<int>{ 1 },
	    "an undefined visgroup reference is not carried" );
	checks.That( groupFragment.bounds.has_value(), "fragment bounds" );
	const MapFragment soloFragment = Copy( doc, { d1 } );
	checks.That( soloFragment.Count() == 1 &&
	                 !std::get<scene::Solid>( soloFragment.objects[0] ).owner.IsValid(),
	    "a brush solid alone is copied as a world solid" );
	checks.That( Copy( doc, { door } ).Count() == 3, "a brush entity brings its solids" );
	checks.That( Copy( doc, { ObjectId() } ).Empty(), "nothing to copy" );

	// Paste into the same document.
	{
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		PasteOptions options;
		options.offset = Vec3d( 0, 256, 0 );
		checks.That( Paste( edit, groupFragment, options, &created ).HasValue(), "paste" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after paste" );
		checks.That( UniqueVmfIds( edit ), "fresh VMF ids" );
		checks.That( created.size() == 1 && edit.FindGroup( created[0] ) && created[0] != group,
		    "the pasted top-level group is reported" );
		const std::vector<ObjectId> members = scene::GroupMembers( edit, created[0] );
		checks.Equal(
		    members.size(), std::size_t( 3 ), "the pasted group holds the pasted members" );
		checks.Equal( Created( edit ).size(), std::size_t( 4 ), "four objects created" );
		const scene::Solid *ps = edit.FindSolid( members[0] );
		checks.That( ps && ps->editor.visgroupIds == std::vector<int>{ 1 },
		    "same visgroup in the same document" );
		checks.That(
		    ps && Near( scene::SolidBounds( *ps )->mins, Vec3d( 0, 256, 0 ) ), "offset applied" );
		const scene::Entity *pl = edit.FindEntity( members[1] );
		checks.That( pl && pl->Name() == "lamp", "names kept by default" );
		checks.That( edit.Settings().visgroups.size() == 1, "no visgroup created" );
		scene::MapDocument committed = doc;
		scene::CommitEdit( committed, edit );
		checks.That( committed.Validate().empty(), "commits cleanly" );
	}
	{
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		checks.That(
		    Paste( edit, soloFragment, PasteOptions{}, &created ).HasValue() && created.size() == 1,
		    "paste a solid" );
		const scene::Solid *s = edit.FindSolid( created[0] );
		checks.That( s && !s->owner.IsValid() && !s->group.IsValid(),
		    "a lone brush solid pastes into the world" );
		checks.That( scene::ValidateEdit( edit ).empty() && UniqueVmfIds( edit ), "valid" );
	}

	// Overlay side lists.
	{
		const MapFragment fragment = Copy( doc, { door, overlay } );
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		checks.That(
		    Paste( edit, fragment, PasteOptions{}, &created ).HasValue() && created.size() == 2,
		    "paste door + overlay" );
		const scene::Entity *ov = edit.FindEntity( created[1] );
		const std::vector<ObjectId> solids = scene::EntitySolids( edit, created[0] );
		checks.That( solids.size() == 2, "the door's solids come along" );
		const scene::Solid *n1 = solids.empty() ? nullptr : edit.FindSolid( solids[0] );
		const std::string expected = n1 ? std::to_string( n1->sides[0].vmfId ) + " " +
		                                      std::to_string( n1->sides[1].vmfId ) + " 999"
		                                : std::string();
		checks.That(
		    ov && *ov->Key( "sides" ) == expected, "overlay sides remapped; uncopied ids kept" );
		const std::string original =
		    std::to_string( d1Side0 ) + " " + std::to_string( d1Side1 ) + " 999";
		checks.That( *edit.FindEntity( overlay )->Key( "sides" ) == original,
		    "the original overlay is untouched" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid" );
	}

	// Paste into another document: visgroups by name.
	{
		scene::MapDocument other( 7 );
		scene::DocumentEdit edit( other );
		checks.That( Paste( edit, groupFragment, PasteOptions{} ).HasValue(),
		    "paste into another document" );
		const std::vector<scene::Visgroup> &tree = edit.Settings().visgroups;
		checks.That( tree.size() == 1 && tree[0].name == "walls" && tree[0].id == 1 &&
		                 tree[0].color == std::optional<scene::Rgb>( scene::Rgb{ 10, 20, 30 } ),
		    "a missing visgroup is created with its name and color" );
		checks.That( scene::ValidateEdit( edit ).empty() && edit.SolidIds().size() == 1 &&
		                 edit.EntityIds().size() == 2,
		    "valid, all objects pasted" );
		for ( ObjectId id : edit.AllIds() )
		{
			checks.That( scene::DocumentSerialOf( id ) == 7, "ids belong to the target document" );
		}
	}
	{
		scene::MapDocument other( 8 );
		{
			scene::DocumentEdit setup( other );
			scene::Visgroup a;
			a.id = 1;
			a.name = "other";
			scene::Visgroup b;
			b.id = 7;
			b.name = "walls";
			setup.MutableSettings().visgroups = { a, b };
			scene::CommitEdit( other, setup );
		}
		scene::DocumentEdit edit( other );
		std::vector<ObjectId> created;
		checks.That( Paste( edit, groupFragment, PasteOptions{}, &created ).HasValue(), "paste" );
		const scene::Solid *s = edit.FindSolid( scene::GroupMembers( edit, created[0] )[0] );
		checks.That( s && s->editor.visgroupIds == std::vector<int>{ 7 },
		    "matched by name, not by a foreign id" );
		checks.That( edit.Settings().visgroups.size() == 2, "nothing created" );
	}

	// Paste Special: copies, offsets, rotation, grouping.
	{
		scene::DocumentEdit edit( doc );
		const MapFragment fragment = Copy( doc, { gs } );
		PasteOptions options;
		options.copies = 3;
		options.offset = Vec3d( 0, 100, 0 );
		std::vector<ObjectId> created;
		checks.That( Paste( edit, fragment, options, &created ).HasValue() && created.size() == 3,
		    "three copies" );
		for ( int k = 0; k < 3 && created.size() == 3; ++k )
		{
			const std::optional<scene::Box> b = scene::SolidBounds( *edit.FindSolid( created[k] ) );
			checks.That(
			    b && Near( b->mins, Vec3d( 0, 100.0 * ( k + 1 ), 0 ) ), "cumulative offset" );
		}
		checks.That( !edit.FindSolid( created[0] )->group.IsValid(), "not grouped: top level" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid" );
	}
	{
		scene::DocumentEdit edit( doc );
		const MapFragment fragment = Copy( doc, { gs, lamp } );
		PasteOptions options;
		options.copies = 2;
		options.rotation = Vec3d( 0, 90, 0 );
		options.group = true;
		std::vector<ObjectId> created;
		checks.That( Paste( edit, fragment, options, &created ).HasValue() && created.size() == 2,
		    "rotated grouped copies" );
		checks.That( edit.FindGroup( created[0] ) && edit.FindGroup( created[1] ),
		    "each copy in its own group" );
		const std::vector<ObjectId> first = scene::GroupMembers( edit, created[0] );
		const std::vector<ObjectId> second = scene::GroupMembers( edit, created[1] );
		checks.That( first.size() == 2 && second.size() == 2, "each group holds one copy" );
		if ( first.size() == 2 && second.size() == 2 )
		{
			// The fragment (solid 0..64 x 0..16 plus the lamp's +/-8 box) is
			// centered at (32, 8, 20); a yaw of 90 turns the long X side to Y.
			const scene::Box b1 = *scene::SolidBounds( *edit.FindSolid( first[0] ) );
			checks.That( Near( b1.Size(), Vec3d( 16, 64, 16 ) ), "yaw 90 about the center" );
			checks.That( Near( b1.Center(), Vec3d( 32, 8, 8 ) ), "the center is fixed (x, y)" );
			const scene::Box b2 = *scene::SolidBounds( *edit.FindSolid( second[0] ) );
			checks.That( Near( b2.Size(), Vec3d( 64, 16, 16 ) ), "the second copy turns 180" );
			const scene::Entity *l1 = edit.FindEntity( first[1] );
			checks.That( l1 && l1->Angles() && Near( *l1->Angles(), Vec3d( 0, 90, 0 ) ),
			    "entity angles turn too" );
		}
		checks.That( scene::ValidateEdit( edit ).empty(), "valid" );
	}

	// Name fixes.
	{
		scene::DocumentEdit edit( doc );
		PasteOptions options;
		options.nameFix = PasteOptions::NameFix::Suffix;
		options.nameText = "_copy";
		options.copies = 2;
		std::vector<ObjectId> created;
		checks.That(
		    Paste( edit, groupFragment, options, &created ).HasValue() && created.size() == 2,
		    "paste with a suffix" );
		for ( int k = 0; k < 2 && created.size() == 2; ++k )
		{
			const std::vector<ObjectId> members = scene::GroupMembers( edit, created[k] );
			const scene::Entity *pl = edit.FindEntity( members[1] );
			const scene::Entity *pt = edit.FindEntity( members[2] );
			const std::string name = k == 0 ? "lamp_copy" : "lamp_copy1";
			checks.That( pl && pl->Name() == name, "suffix, then made unique across copies" );
			checks.That(
			    pt && *pt->Key( "target" ) == name, "a key naming it follows (case-insensitive)" );
			checks.That( pt && pt->connections[0].target == name, "a connection target follows" );
			checks.That(
			    pt && pt->connections[1].parameter == name, "a connection parameter follows" );
			checks.That(
			    pt && pt->connections[1].target == "outside" && *pt->Key( "target2" ) == "outside",
			    "references outside the pasted set are untouched" );
		}
		checks.That( edit.FindEntity( lamp )->Name() == "lamp" &&
		                 *edit.FindEntity( trigger )->Key( "target" ) == "LAMP",
		    "the originals are untouched" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid" );
	}
	{
		scene::DocumentEdit edit( doc );
		PasteOptions options;
		options.nameFix = PasteOptions::NameFix::Suffix;
		std::vector<ObjectId> created;
		checks.That( Paste( edit, groupFragment, options, &created ).HasValue(),
		    "paste with uniqueness only" );
		checks.That(
		    edit.FindEntity( scene::GroupMembers( edit, created[0] )[1] )->Name() == "lamp1",
		    "a taken name gets a number" );
	}
	{
		scene::DocumentEdit edit( doc );
		PasteOptions options;
		options.nameFix = PasteOptions::NameFix::Prefix;
		options.nameText = "p_";
		std::vector<ObjectId> created;
		checks.That(
		    Paste( edit, groupFragment, options, &created ).HasValue(), "paste with a prefix" );
		checks.That(
		    edit.FindEntity( scene::GroupMembers( edit, created[0] )[1] )->Name() == "p_lamp",
		    "prefixed" );
		checks.That(
		    Paste( edit, groupFragment, options, &created ).HasValue() &&
		        edit.FindEntity( scene::GroupMembers( edit, created[1] )[1] )->Name() == "p_lamp1",
		    "a second paste avoids the first's name" );
	}
	{
		PasteOptions options;
		options.nameFix = PasteOptions::NameFix::Suffix;
		checks.Equal( FixedPasteName( "door9", options, { "door9" } ), std::string( "door10" ),
		    "trailing number incremented" );
		checks.Equal( FixedPasteName( "door", options, { "door", "door1" } ),
		    std::string( "door2" ), "until free" );
		checks.Equal(
		    FixedPasteName( "Door", options, {} ), std::string( "Door" ), "a free name is kept" );
		options.nameFix = PasteOptions::NameFix::Keep;
		checks.Equal(
		    FixedPasteName( "door", options, { "door" } ), std::string( "door" ), "Keep keeps" );
	}

	// Duplicate.
	{
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		checks.That( Duplicate( edit, { door }, Vec3d( 0, 0, 128 ), &created ).HasValue() &&
		                 created.size() == 1,
		    "duplicate a brush entity" );
		checks.That( scene::EntitySolids( edit, created[0] ).size() == 2, "with its solids" );
		checks.That( edit.FindEntity( created[0] )->Name() == "door", "names kept" );
		checks.That( scene::ValidateEdit( edit ).empty() && UniqueVmfIds( edit ), "valid" );
		checks.That(
		    Duplicate( edit, {}, Vec3d(), &created ).Error().code == app::EditErrorCode::Nothing,
		    "nothing to duplicate (negative)" );
	}

	// Refusals stage nothing.
	{
		scene::DocumentEdit edit( doc );
		checks.That( Paste( edit, MapFragment{}, PasteOptions{} ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "an empty clipboard (negative)" );
		PasteOptions zero;
		zero.copies = 0;
		checks.That(
		    Paste( edit, groupFragment, zero ).Error().code == app::EditErrorCode::Rejected,
		    "zero copies (negative)" );
		PasteOptions many;
		many.copies = 5000;
		checks.That(
		    Paste( edit, groupFragment, many ).Error().code == app::EditErrorCode::Rejected,
		    "too many copies (negative)" );
		PasteOptions nan;
		nan.offset = Vec3d( std::numeric_limits<double>::quiet_NaN(), 0, 0 );
		checks.That( Paste( edit, groupFragment, nan ).Error().code == app::EditErrorCode::Rejected,
		    "NaN offset (negative)" );
		PasteOptions badAngles;
		badAngles.rotation = Vec3d( 0, std::numeric_limits<double>::infinity(), 0 );
		checks.That(
		    Paste( edit, groupFragment, badAngles ).Error().code == app::EditErrorCode::Rejected,
		    "infinite angles (negative)" );
		checks.That( edit.Finish().Empty(), "refusals staged nothing" );
	}

	return checks.Report();
}
