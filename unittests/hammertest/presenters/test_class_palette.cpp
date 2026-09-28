//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters class palette (RFC 0002, R08 domain logic) over
//			the fake catalog and a real EditSession: the category table,
//			category counts under filters, kind filter, search ranking (prefix,
//			substring, description; catalog order within a rank), category
//			filter, the bounded most-recent list and the active class, and map
//			counts that follow edits and undo. Negative checks: unknown classes
//			refused without changing state, searches with no match, destruction
//			after the session.
//
//=============================================================================//

#include "hammer/presenters/class_palette.h"

#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/ops/create_ops.h"
#include "testing/checks.h"

#include "fakes/fake_entity_catalog.h"

#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using C = hammertest::FakeEntityCatalog;

namespace
{

std::vector<std::string> Names( const ClassPalette &palette )
{
	std::vector<std::string> names;
	for ( const ClassEntry &entry : palette.Entries() )
	{
		names.push_back( entry.name );
	}
	return names;
}

std::size_t CountOf( const ClassPalette &palette, const std::string &category )
{
	for ( const ClassCategory &c : palette.Categories() )
	{
		if ( c.name == category )
		{
			return c.count;
		}
	}
	return 999;
}

} // namespace

int main()
{
	testing::Checks checks;

	checks.That(
	    CategoryOf( "info_player_start" ) == "Info" && CategoryOf( "func_door" ) == "Func" &&
	        CategoryOf( "light" ) == "Lights" && CategoryOf( "light_spot" ) == "Lights" &&
	        CategoryOf( "prop_physics" ) == "Props" && CategoryOf( "trigger_once" ) == "Triggers" &&
	        CategoryOf( "logic_relay" ) == "Logic" && CategoryOf( "env_sprite" ) == "Environment" &&
	        CategoryOf( "npc_turret" ) == "NPCs" && CategoryOf( "point_camera" ) == "Point" &&
	        CategoryOf( "weapon_portalgun" ) == "Weapons" && CategoryOf( "item_cube" ) == "Items" &&
	        CategoryOf( "filter_activator_name" ) == "Filters" && CategoryOf( "ai_goal" ) == "AI" &&
	        CategoryOf( "path_track" ) == "Paths" && CategoryOf( "game_text" ) == "Game" &&
	        CategoryOf( "worldspawn" ) == "Other" && CategoryOf( "INFO_target" ) == "Info",
	    "category table" );
	checks.That( CategoryOf( "infox" ) == "Other" && CategoryOf( "" ) == "Other",
	    "prefix needs the underscore" );

	C catalog;
	catalog.AddPoint( "info_player_start" ).Description( "Player spawn point" );
	catalog.AddPoint( "info_target" ).Description( "A named spot to aim at" );
	catalog.AddPoint( "light" ).Description( "An invisible omnidirectional light" );
	catalog.AddPoint( "light_spot" ).Description( "A cone of light" );
	catalog.AddSolid( "func_door" ).Description( "A sliding door" );
	catalog.AddSolid( "trigger_once" ).Description( "Fires once when touched" );
	catalog.AddKind( ports::EntityClassKind::Other, "filter_activator_name" )
	    .Description( "Filters by the activator's name" );
	catalog.AddPoint( "prop_spotlight" ).Description( "Spot" );

	scene::MapDocument doc;
	auto session = std::make_unique<app::EditSession>( doc );
	app::EditorSettings settings;
	auto palette = std::make_unique<ClassPalette>( catalog, settings, session.get(), 3 );

	checks.Equal( palette->Entries().size(), std::size_t( 8 ), "every class" );
	checks.That(
	    Names( *palette ).front() == "filter_activator_name", "catalog (case-insensitive) order" );
	{
		const auto &cats = palette->Categories();
		checks.That( cats.size() == 6 && cats[0].name == "Info" && cats[0].count == 2 &&
		                 cats[1].name == "Func" && cats[2].name == "Lights" &&
		                 cats[3].name == "Props" && cats[4].name == "Triggers" &&
		                 cats[5].name == "Filters",
		    "used categories in table order with counts" );
	}

	// Kind filter.
	palette->SetKindFilter( ClassKindFilter::Solid );
	checks.That( Names( *palette ) == std::vector<std::string>{ "func_door", "trigger_once" },
	    "solid classes" );
	palette->SetKindFilter( ClassKindFilter::Point );
	checks.That( palette->Entries().size() == 6 && CountOf( *palette, "Filters" ) == 1 &&
	                 CountOf( *palette, "Func" ) == 0,
	    "point filter keeps point-like classes; counts follow" );
	palette->SetKindFilter( ClassKindFilter::All );

	// Search ranking.
	const std::uint64_t rev = palette->Revision();
	palette->SetSearch( "SPOT" );
	checks.That( palette->Revision() > rev, "revision moves" );
	checks.That( Names( *palette ) ==
	                 std::vector<std::string>{ "light_spot", "prop_spotlight", "info_target" },
	    "substring in name first (catalog order), then description" );
	palette->SetSearch( " light " );
	checks.That(
	    Names( *palette ) == std::vector<std::string>{ "light", "light_spot", "prop_spotlight" },
	    "prefix, then substring, then description; trimmed" );
	checks.That( palette->Entries()[0].rank == 0 && palette->Entries()[2].rank == 1, "ranks" );
	palette->SetSearch( "door" );
	checks.That( Names( *palette ) == std::vector<std::string>{ "func_door" }, "substring" );
	palette->SetSearch( "zzz" );
	checks.That(
	    palette->Entries().empty() && CountOf( *palette, "Info" ) == 0, "no match (negative)" );
	palette->SetSearch( "" );

	// Category filter.
	palette->SetCategory( std::string( "Lights" ) );
	checks.That(
	    Names( *palette ) == std::vector<std::string>{ "light", "light_spot" }, "category filter" );
	checks.Equal( CountOf( *palette, "Info" ), std::size_t( 2 ),
	    "category counts ignore the category filter" );
	palette->SetCategory( std::nullopt );

	// Active and recent.
	checks.That( palette->SetActive( "LIGHT" ).HasValue() && palette->Active() == "light" &&
	                 settings.entityClass == "light",
	    "catalog spelling, written to the settings owner" );
	settings.entityClass = "info_target";
	checks.Equal(
	    palette->Active(), std::string( "info_target" ), "reads the settings owner live" );
	checks.That( palette->SetActive( "func_door" ).HasValue() &&
	                 palette->SetActive( "info_target" ).HasValue() &&
	                 palette->SetActive( "light" ).HasValue(),
	    "more uses" );
	checks.That(
	    palette->Recent() == std::vector<std::string>{ "light", "info_target", "func_door" },
	    "most recent first, no duplicates, bounded to 3" );
	const std::vector<std::string> recent = palette->Recent();
	checks.That( !palette->SetActive( "npc_unknown" ) && palette->Active() == "light" &&
	                 palette->Recent() == recent,
	    "unknown class refused, nothing changes (negative)" );

	// Map counts follow the document.
	auto placed = session->Execute( "Place light",
	    [&]( scene::DocumentEdit &edit ) -> app::EditResult
	    {
		    scene::Entity e;
		    e.classname = "light";
		    edit.Add( e );
		    return {};
	    } );
	checks.That( placed.HasValue(), "place" );
	auto countLight = [&]()
	{
		for ( const ClassEntry &entry : palette->Entries() )
		{
			if ( entry.name == "light" )
			{
				return entry.countInMap;
			}
		}
		return std::size_t( 999 );
	};
	checks.Equal( countLight(), std::size_t( 1 ), "count in map" );
	checks.That( session->Undo().HasValue() && countLight() == 0, "undo recounts" );

	ClassPalette standalone( catalog, settings );
	checks.That(
	    standalone.Entries().size() == 8 && standalone.Entries()[0].countInMap == 0, "no session" );

	session.reset();
	palette.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
