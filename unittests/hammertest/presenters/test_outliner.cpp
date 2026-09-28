//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters outliner (RFC 0002, R08 domain logic) over a real
//			EditSession: tree structure (world, nested groups, brush entities
//			with their solids, loose solids), labels, child counts, collapsed
//			display rows, selection through containers, visibility through
//			containers and the hide toggle as one undo step, filters that keep
//			ancestors, case-insensitive matching, expansion surviving rebuilds.
//			Negative checks: unknown ids refused, the world row selects nothing,
//			a filter with no match leaves only the root, expansion cleared on
//			document replacement, destruction after the session.
//
//=============================================================================//

#include "hammer/presenters/outliner.h"

#include "hammer/app/edit_session.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using app::SelectMode;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

std::vector<ObjectId> Ids( const Outliner &outliner, const std::vector<std::size_t> &rows )
{
	std::vector<ObjectId> ids;
	for ( std::size_t i : rows )
	{
		ids.push_back( outliner.Rows()[i].id );
	}
	return ids;
}

std::vector<ObjectId> AllIds( const Outliner &outliner )
{
	std::vector<ObjectId> ids;
	for ( const OutlinerRow &row : outliner.Rows() )
	{
		ids.push_back( row.id );
	}
	return ids;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "dev/dev_measuregeneric01b";
	auto box = [&]( double x )
	{
		return scene::MakeBoxSolid( { Vec3d( x, 0, 0 ), Vec3d( x + 16, 16, 16 ) }, tex );
	};
	scene::MapDocument doc;
	ObjectId g1, g2, s2, lamp, door, d1, d2, s1, player;
	{
		scene::DocumentEdit edit( doc );
		g1 = edit.Add( scene::Group{} );
		scene::Group inner;
		inner.group = g1;
		g2 = edit.Add( inner );
		scene::Solid b2 = box( 0 );
		b2.group = g2;
		s2 = edit.Add( b2 );
		scene::Entity l;
		l.classname = "light";
		l.SetKey( "targetname", "Lamp_Hall" );
		l.group = g1;
		lamp = edit.Add( l );
		scene::Entity d;
		d.classname = "func_door";
		door = edit.Add( d );
		scene::Solid b = box( 32 );
		b.owner = door;
		d1 = edit.Add( b );
		b = box( 64 );
		b.owner = door;
		d2 = edit.Add( b );
		s1 = edit.Add( box( 96 ) );
		scene::Entity p;
		p.classname = "info_player_start";
		player = edit.Add( p );
		scene::CommitEdit( doc, edit );
	}
	const std::uint32_t g1Vmf = doc.FindGroup( g1 )->vmfId;
	const std::uint32_t s1Vmf = doc.FindSolid( s1 )->vmfId;

	auto session = std::make_unique<app::EditSession>( doc );
	auto outliner = std::make_unique<Outliner>( *session );

	// Structure.
	const std::vector<ObjectId> expected = {
	    ObjectId{}, g1, g2, s2, lamp, door, d1, d2, player, s1 };
	checks.That( AllIds( *outliner ) == expected,
	    "pre-order: groups, entities, solids; brush solids under entity" );
	{
		const auto &rows = outliner->Rows();
		checks.That( rows.size() == 10 && rows[0].kind == OutlinerKind::World &&
		                 rows[0].label == "world" && rows[0].childCount == 4,
		    "world root with four children" );
		checks.That( rows.size() == 10 && rows[1].label == "group " + std::to_string( g1Vmf ) &&
		                 rows[1].childCount == 2 && rows[2].depth == 2 && rows[3].depth == 3 &&
		                 rows[3].kind == OutlinerKind::Solid,
		    "nested groups with depths" );
		checks.That( rows.size() == 10 && rows[4].label == "Lamp_Hall" &&
		                 rows[4].classname == "light" && rows[5].label == "func_door" &&
		                 rows[5].childCount == 2 &&
		                 rows[9].label == "solid " + std::to_string( s1Vmf ),
		    "labels: targetname, classname, solid vmf id" );
		checks.That( rows.size() == 10 && rows[3].parent == std::optional<std::size_t>( 2 ) &&
		                 !rows[0].parent,
		    "parent rows" );
	}
	checks.That( Ids( *outliner, outliner->DisplayRows() ) ==
	                 std::vector<ObjectId>{ ObjectId{}, g1, door, player, s1 },
	    "containers start collapsed" );
	outliner->SetExpanded( g1, true );
	checks.That( Ids( *outliner, outliner->DisplayRows() ) ==
	                 std::vector<ObjectId>{ ObjectId{}, g1, g2, lamp, door, player, s1 },
	    "expanding shows children, nested collapsed stays collapsed" );

	// Selection through containers.
	const std::uint64_t rev = outliner->Revision();
	checks.That( outliner->Select( { ObjectId{}, g1 }, SelectMode::Replace ).HasValue(),
	    "select group row" );
	checks.That( session->CurrentSelection().objects == std::vector<ObjectId>{ g1 },
	    "the world row selects nothing" );
	checks.That( outliner->Revision() > rev, "revision moves" );
	{
		const auto &rows = outliner->Rows();
		checks.That( rows[1].selected && rows[2].selected && rows[3].selected && rows[4].selected &&
		                 !rows[5].selected && !rows[0].selected,
		    "members of a selected container are selected" );
	}
	checks.That( outliner->Select( { d1 }, SelectMode::Add ).HasValue() &&
	                 outliner->Rows()[*outliner->FindRow( d1 )].selected &&
	                 !outliner->Rows()[*outliner->FindRow( d2 )].selected,
	    "a solid selected alone" );

	// Visibility.
	const std::size_t position = session->History().Position();
	checks.That( outliner->ToggleVisibility( g1 ).HasValue(), "hide group" );
	checks.That(
	    session->History().Position() == position + 1 &&
	        session->History().UndoEntry()->label == "Hide group " + std::to_string( g1Vmf ),
	    "one labeled step" );
	checks.That( !outliner->Rows()[1].visible && !outliner->Rows()[3].visible &&
	                 !outliner->Rows()[4].visible && outliner->Rows()[5].visible,
	    "hidden through containers" );
	checks.That( outliner->IsExpanded( g1 ) && !outliner->IsExpanded( door ),
	    "expansion survives the rebuild" );
	checks.That( outliner->ToggleVisibility( g1 ).HasValue() &&
	                 session->History().UndoEntry()->label.rfind( "Show ", 0 ) == 0 &&
	                 outliner->Rows()[4].visible,
	    "show again" );
	checks.That( session->Undo().HasValue() && !outliner->Rows()[4].visible, "undo re-hides" );
	checks.That( !outliner->ToggleVisibility( ObjectId{} ) &&
	                 !outliner->ToggleVisibility( ObjectId{ 12345 } ),
	    "unknown ids refused (negative)" );

	// Filters keep ancestors.
	outliner->SetFilter( "lamp" );
	checks.That( AllIds( *outliner ) == std::vector<ObjectId>{ ObjectId{}, g1, lamp },
	    "match keeps ancestors" );
	checks.That( !outliner->Rows()[1].matches && outliner->Rows()[2].matches &&
	                 outliner->Rows()[1].childCount == 1,
	    "ancestor kept but not matching; counts after filter" );
	outliner->SetFilter( "FUNC_DOOR" );
	checks.That( AllIds( *outliner ) == std::vector<ObjectId>{ ObjectId{}, door },
	    "case-insensitive classname match; non-matching children dropped" );
	outliner->SetFilter( "solid" );
	checks.That(
	    AllIds( *outliner ) == std::vector<ObjectId>{ ObjectId{}, g1, g2, s2, door, d1, d2, s1 },
	    "every solid with its containers" );
	checks.That( outliner->DisplayRows().size() == outliner->Rows().size(),
	    "a filter displays every kept row" );
	outliner->SetFilter( "nothing-matches" );
	checks.That( outliner->Rows().size() == 1 && outliner->Rows()[0].childCount == 0,
	    "no match leaves the root (negative)" );
	outliner->SetFilter( "" );
	checks.Equal( outliner->Rows().size(), std::size_t( 10 ), "cleared filter restores the tree" );
	checks.That( outliner->IsExpanded( g1 ), "expansion survives filtering" );

	// Replacement clears expansion.
	checks.That( session->Replace( doc ).HasValue(), "replace" );
	checks.That( !outliner->IsExpanded( g1 ) && outliner->Rows().size() == 10,
	    "expansion cleared on replace" );

	session.reset();
	outliner.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
