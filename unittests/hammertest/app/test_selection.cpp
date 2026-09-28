//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app selection policy conformance (RFC 0002 "Selection and
//			property editing"): pick resolution per granularity (groups, objects,
//			solids), Replace/Add/Toggle/Remove with the primary rule, face
//			selection, pruning of dead ids, select-all (hidden excluded) and
//			invert. Negative checks: invalid ids, unknown hits, removing absent
//			members, and faces of deleted sides.
//
//=============================================================================//

#include "hammer/app/selection.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <algorithm>

using namespace hammer::app;
using namespace hammer::scene;
using mapgeometry::Vec3d;

int main()
{
	testing::Checks checks;

	MapDocument doc;
	FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	ObjectId group, grouped, door, doorSolid, loose, hiddenSolid;
	{
		DocumentEdit edit( doc );
		group = edit.Add( Group{} );
		Solid g = MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ) }, tex );
		g.group = group;
		grouped = edit.Add( g );
		Entity d;
		d.classname = "func_door";
		d.group = group;
		door = edit.Add( d );
		Solid ds = MakeBoxSolid( { Vec3d( 32, 0, 0 ), Vec3d( 40, 16, 16 ) }, tex );
		ds.owner = door;
		doorSolid = edit.Add( ds );
		loose = edit.Add( MakeBoxSolid( { Vec3d( 100, 0, 0 ), Vec3d( 116, 16, 16 ) }, tex ) );
		Solid h = MakeBoxSolid( { Vec3d( 200, 0, 0 ), Vec3d( 216, 16, 16 ) }, tex );
		h.hidden = true;
		hiddenSolid = edit.Add( h );
		CommitEdit( doc, edit );
	}

	// Pick resolution.
	checks.That( ResolvePick( doc, doorSolid, SelectionGranularity::Solids ) == doorSolid, "solids: the solid" );
	checks.That( ResolvePick( doc, doorSolid, SelectionGranularity::Objects ) == door, "objects: its brush entity" );
	checks.That( ResolvePick( doc, doorSolid, SelectionGranularity::Groups ) == group, "groups: the outer group" );
	checks.That( ResolvePick( doc, grouped, SelectionGranularity::Objects ) == grouped, "objects ignore groups" );
	checks.That( ResolvePick( doc, loose, SelectionGranularity::Groups ) == loose, "loose solid is itself" );
	ObjectId unknown;
	unknown.value = 999;
	checks.That( !ResolvePick( doc, unknown, SelectionGranularity::Groups ).IsValid(), "unknown hit selects nothing" );

	// Combine and the primary rule.
	Selection s = CombineObjects( {}, { loose, grouped }, SelectMode::Replace );
	checks.That( s.objects.size() == 2 && s.primary == grouped, "replace; primary is the last added" );
	s = CombineObjects( s, { door }, SelectMode::Add );
	checks.That( s.objects.size() == 3 && s.primary == door && s.Contains( door ), "add" );
	s = CombineObjects( s, { door }, SelectMode::Toggle );
	checks.That( !s.Contains( door ) && s.primary.IsValid() && s.Contains( s.primary ),
	    "toggling the primary off falls back to a member" );
	s = CombineObjects( s, { door }, SelectMode::Toggle );
	checks.That( s.Contains( door ) && s.primary == door, "toggle on becomes primary" );
	s = CombineObjects( s, { hiddenSolid }, SelectMode::Remove );
	checks.Equal( s.objects.size(), std::size_t( 3 ), "removing an absent id changes nothing (negative)" );
	s = CombineObjects( s, { ObjectId() }, SelectMode::Add );
	checks.Equal( s.objects.size(), std::size_t( 3 ), "the invalid id is ignored (negative)" );
	checks.That( std::is_sorted( s.objects.begin(), s.objects.end() ), "objects stay sorted" );
	s = CombineObjects( s, {}, SelectMode::Replace );
	checks.That( s.Empty() && !s.primary.IsValid(), "replace with nothing clears" );

	// Faces.
	const std::uint32_t side0 = doc.FindSolid( loose )->sides[0].vmfId;
	const std::uint32_t side1 = doc.FindSolid( loose )->sides[1].vmfId;
	Selection f = CombineFaces( {}, { { loose, side0 }, { loose, side1 } }, SelectMode::Replace );
	checks.That( f.faces.size() == 2 && f.ContainsFace( { loose, side1 } ), "face selection" );
	f = CombineFaces( f, { { loose, side0 } }, SelectMode::Toggle );
	checks.That( f.faces.size() == 1 && !f.ContainsFace( { loose, side0 } ), "face toggle" );

	// Prune after deletions.
	{
		Selection mixed = CombineObjects( f, { loose, door }, SelectMode::Add );
		DocumentEdit edit( doc );
		edit.Remove( loose );
		const Selection pruned = Prune( edit, mixed );
		checks.That( !pruned.Contains( loose ) && pruned.Contains( door ), "prune drops dead objects" );
		checks.That( pruned.faces.empty(), "prune drops faces of dead solids" );
		checks.That( pruned.primary == door, "prune keeps a live primary" );

		DocumentEdit edit2( doc );
		Solid *l = edit2.MutableSolid( loose );
		l->sides.erase( l->sides.begin() + 1 ); // side1 no longer exists
		checks.That( Prune( edit2, f ).faces.empty(), "prune drops faces of removed sides (negative)" );
	}

	// Select all / invert.
	const Selection all = SelectAll( doc, SelectionGranularity::Groups );
	checks.That( all.objects == std::vector<ObjectId>{ group, loose }, "select all by groups skips hidden" );
	checks.Equal( SelectAll( doc, SelectionGranularity::Groups, true ).objects.size(), std::size_t( 3 ),
	    "select all can include hidden" );
	const Selection solids = SelectAll( doc, SelectionGranularity::Solids );
	checks.Equal( solids.objects.size(), std::size_t( 4 ), "select all solids and entities" );
	const Selection inverted =
	    InvertSelection( doc, CombineObjects( {}, { loose }, SelectMode::Replace ), SelectionGranularity::Groups );
	checks.That( inverted.objects == std::vector<ObjectId>{ group }, "invert" );

	return checks.Report();
}
