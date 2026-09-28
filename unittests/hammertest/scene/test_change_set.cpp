//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.scene staged edits and change sets (RFC 0002 "Edit
//			transactions and history"): copy-on-write staging that never touches
//			the base, reads through staged state, no-op changes dropped, lossless
//			forward/backward application, id allocation that never collides with
//			the base, and settings changes. Negative checks: foreign change sets
//			are refused without partial mutation; removing a dead object fails.
//
//=============================================================================//

#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"
#include <algorithm>
#include "testing/checks.h"

using namespace hammer::scene;
using mapgeometry::Vec3d;

namespace
{

Solid BoxSolid( const Vec3d &a, const Vec3d &b )
{
	FaceTexture tex;
	tex.material = "TOOLS/TOOLSNODRAW";
	return MakeBoxSolid( { a, b }, tex );
}

} // namespace

int main()
{
	testing::Checks checks;

	MapDocument doc( 3 );
	ObjectId boxId;
	ObjectId lightId;
	{
		DocumentEdit edit( doc );
		boxId = edit.Add( BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) ) );
		Entity light;
		light.classname = "light";
		light.SetOrigin( Vec3d( 32, 32, 32 ) );
		lightId = edit.Add( light );
		checks.That( !doc.Contains( boxId ), "staging never touches the base" );
		checks.That( edit.FindSolid( boxId ) && edit.FindEntity( lightId ), "reads see staged objects" );
		checks.Equal( edit.SolidIds().size(), std::size_t( 1 ), "staged ids by kind" );
		const Solid *staged = edit.FindSolid( boxId );
		bool allIds = staged->vmfId != 0;
		for ( const Side &s : staged->sides )
			allIds = allIds && s.vmfId != 0;
		checks.That( allIds, "Add allocates persistent ids for the solid and its sides" );
		const ChangeSet changes = CommitEdit( doc, edit );
		checks.Equal( changes.Created().size(), std::size_t( 2 ), "two creations recorded" );
		checks.That( doc.FindSolid( boxId ) && doc.FindEntity( lightId ), "commit installs them" );
		checks.That( doc.Validate().empty(), "committed document validates" );
	}

	const MapDocument afterCreate = doc;

	// Modify + remove, then undo/redo by applying the change set both ways.
	ChangeSet edit2;
	{
		DocumentEdit edit( doc );
		Entity *light = edit.MutableEntity( lightId );
		light->SetKey( "_light", "255 255 255 200" );
		checks.That( doc.FindEntity( lightId )->Key( "_light" ) == nullptr, "copy-on-write leaves base intact" );
		checks.That( edit.Remove( boxId ), "remove a live object" );
		checks.That( !edit.FindSolid( boxId ) && !edit.KindOf( boxId ), "removed object is hidden" );
		checks.That( !edit.Remove( boxId ), "removing twice fails (negative)" );
		checks.That( edit.MutableSolid( boxId ) == nullptr, "cannot mutate a removed object" );
		checks.That( edit.MutableSolid( lightId ) == nullptr, "cannot mutate as the wrong kind" );
		edit.MutableSettings().SetWorldKey( "skyname", "sky_wasteland02" );
		checks.That( doc.Settings().WorldKey( "skyname" ) == nullptr, "settings staged separately" );
		edit2 = CommitEdit( doc, edit );
	}
	checks.Equal( edit2.Modified().size(), std::size_t( 1 ), "one modification" );
	checks.Equal( edit2.Removed().size(), std::size_t( 1 ), "one removal" );
	checks.That( edit2.settingsBefore && edit2.settingsAfter, "settings recorded" );
	const MapDocument afterEdit = doc;

	checks.That( Apply( doc, edit2, ApplyDirection::Backward ), "undo applies" );
	checks.That( SameContent( doc, afterCreate ), "undo restores the exact prior content" );
	checks.That( Apply( doc, edit2, ApplyDirection::Forward ), "redo applies" );
	checks.That( SameContent( doc, afterEdit ), "redo restores the exact later content" );

	// An edit that returns to the base records nothing.
	{
		DocumentEdit edit( doc );
		Entity *light = edit.MutableEntity( lightId );
		const std::string old = *light->Key( "_light" );
		light->SetKey( "_light", "0 0 0 0" );
		light->SetKey( "_light", old );
		edit.MutableSettings(); // touched, unchanged
		checks.That( edit.Finish().Empty(), "a round-trip edit is empty" );
	}

	// Adding then removing within one edit records nothing; ids still advance.
	{
		const std::uint32_t before = doc.NextLocalId();
		DocumentEdit edit( doc );
		const ObjectId temp = edit.Add( BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) ) );
		edit.Remove( temp );
		const ChangeSet c = CommitEdit( doc, edit );
		checks.That( c.Empty(), "created-and-removed is empty" );
		checks.That( doc.NextLocalId() > before, "allocated ids are never reissued" );
		checks.That( doc.AllocateId() != temp, "the temp id stays retired" );
	}

	// Put restores an object under its id (the path clipboard/undo helpers use).
	{
		DocumentEdit edit( doc );
		Solid restored = *afterCreate.FindSolid( boxId );
		edit.Put( restored );
		const ChangeSet c = CommitEdit( doc, edit );
		checks.Equal( c.Created().size(), std::size_t( 1 ), "put of a removed id is a creation" );
		checks.That( doc.FindSolid( boxId ) && *doc.FindSolid( boxId ) == restored, "put restores the value" );
	}

	// Negative: a change set from another document is refused atomically.
	{
		MapDocument other( 4 );
		DocumentEdit edit( other );
		edit.Add( BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) ) );
		const ChangeSet foreign = edit.Finish();
		const MapDocument snapshot = doc;
		checks.That( !Apply( doc, foreign, ApplyDirection::Forward ), "foreign change set refused" );
		checks.That( SameContent( doc, snapshot ), "refusal leaves the document unchanged" );
	}

	// Kind listing merges base and staged ids in id order.
	{
		DocumentEdit edit( doc );
		const ObjectId g = edit.Add( Group{} );
		const std::vector<ObjectId> all = edit.AllIds();
		checks.That( std::is_sorted( all.begin(), all.end() ), "ids come in id order" );
		checks.That( all.back() == g, "new ids sort last" );
		checks.Equal( edit.GroupIds().size(), std::size_t( 1 ), "groups listed" );
	}

	// ValidateEdit: the commit safety net.
	{
		DocumentEdit good( doc );
		good.Add( BoxSolid( Vec3d( 200, 0, 0 ), Vec3d( 264, 64, 64 ) ) );
		checks.That( ValidateEdit( good ).empty(), "a valid edit passes" );

		DocumentEdit open( doc );
		Solid s = BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) );
		s.sides.pop_back();
		s.sides.pop_back();
		open.Add( s );
		checks.That( !ValidateEdit( open ).empty(), "an open four-sided solid is rejected" );

		DocumentEdit dupe( doc );
		Solid d = BoxSolid( Vec3d( 300, 0, 0 ), Vec3d( 364, 64, 64 ) );
		d.sides[0].vmfId = doc.FindSolid( boxId )->sides[0].vmfId;
		dupe.Add( d );
		checks.That( !ValidateEdit( dupe ).empty(), "a side id clash with an untouched solid is rejected" );

		DocumentEdit orphan( doc );
		Entity door;
		door.classname = "func_door";
		const ObjectId doorId = orphan.Add( door );
		Solid owned = BoxSolid( Vec3d( 0, 0, 100 ), Vec3d( 8, 8, 108 ) );
		owned.owner = doorId;
		orphan.Add( owned );
		checks.That( ValidateEdit( orphan ).empty(), "a brush entity with its solid is valid" );
		orphan.Remove( doorId );
		checks.That( !ValidateEdit( orphan ).empty(), "a solid left owned by a removed entity is rejected" );

		DocumentEdit cycle( doc );
		const ObjectId g1 = cycle.Add( Group{} );
		Group g2v;
		g2v.group = g1;
		const ObjectId g2 = cycle.Add( g2v );
		cycle.MutableGroup( g1 )->group = g2;
		checks.That( !ValidateEdit( cycle ).empty(), "a group cycle is rejected" );
	}

	return checks.Report();
}
