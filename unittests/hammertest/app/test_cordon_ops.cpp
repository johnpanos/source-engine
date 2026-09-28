//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app cordon operations (RFC 0002, R08 domain logic): adding,
//			removing and renaming cordons, box edits, per-cordon and document
//			enables, the Single/List form rule (the form becomes List when the
//			content no longer fits the older single 'cordon' block, and the
//			Single form's one 'active' key moves both flags), and the query
//			used by builds and tools (active boxes of active cordons when
//			enabled; inclusive intersection; no active box = no restriction).
//			Negative checks: bad indices, degenerate and non-finite boxes,
//			removing a cordon's last box, no-op requests.
//
//=============================================================================//

#include "hammer/app/ops/cordon_ops.h"
#include "testing/checks.h"

#include <limits>

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using Form = scene::DocumentSettings::CordonForm;

int main()
{
	testing::Checks checks;

	const scene::Box unit{ Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) };
	const scene::Box far{ Vec3d( 1000, 0, 0 ), Vec3d( 1064, 64, 64 ) };

	// A new map: no cordons, form None.
	scene::MapDocument empty;
	{
		scene::DocumentEdit edit( empty );
		std::size_t index = 99;
		checks.That(
		    AddCordon( edit, "arena", unit, &index ).HasValue() && index == 0, "add a cordon" );
		const scene::DocumentSettings &s = edit.Settings();
		checks.That( s.cordons.size() == 1 && s.cordons[0].name == "arena" && s.cordons[0].active &&
		                 s.cordons[0].boxes.size() == 1,
		    "one active cordon with one box" );
		checks.That( s.cordonForm == Form::List, "a map without cordons gets the List form" );
		checks.That( ActiveCordonBoxes( edit ).empty() && InsideCordons( edit, far ),
		    "disabled cordons restrict nothing" );
		checks.That(
		    SetCordonsEnabled( edit, true ).HasValue() && edit.Settings().cordonsActive, "enable" );
		checks.That( SetCordonsEnabled( edit, true ).Error().code == app::EditErrorCode::Nothing,
		    "already enabled" );
		checks.That(
		    ActiveCordonBoxes( edit ) == std::vector<scene::Box>{ unit }, "the active box" );
		checks.That( !InsideCordons( edit, far ), "outside" );
		checks.That( InsideCordons( edit, scene::Box{ Vec3d( 64, 0, 0 ), Vec3d( 80, 8, 8 ) } ),
		    "touching an edge counts as inside" );
		checks.That( InsideCordons( edit, scene::Box{ Vec3d( -8, -8, -8 ), Vec3d( 8, 8, 8 ) } ),
		    "straddling is inside" );

		checks.That(
		    AddCordonBox( edit, 0, far ).HasValue() && InsideCordons( edit, far ), "a second box" );
		checks.That(
		    SetCordonBox( edit, 0, 1, scene::Box{ Vec3d( 2000, 0, 0 ), Vec3d( 2064, 8, 8 ) } )
		            .HasValue() &&
		        !InsideCordons( edit, far ),
		    "moving a box" );
		checks.That( SetCordonBox( edit, 0, 0, unit ).Error().code == app::EditErrorCode::Nothing,
		    "unchanged box" );
		checks.That( SetCordonBox( edit, 0, 5, unit ).Error().code == app::EditErrorCode::Rejected,
		    "bad box index (negative)" );
		checks.That( SetCordonBox( edit, 3, 0, unit ).Error().code == app::EditErrorCode::Rejected,
		    "bad cordon index (negative)" );
		checks.That( RemoveCordonBox( edit, 0, 1 ).HasValue() &&
		                 edit.Settings().cordons[0].boxes.size() == 1,
		    "remove a box" );
		checks.That( RemoveCordonBox( edit, 0, 0 ).Error().code == app::EditErrorCode::Rejected,
		    "the last box stays (negative)" );
		checks.That( RemoveCordonBox( edit, 0, 7 ).Error().code == app::EditErrorCode::Rejected,
		    "bad box (negative)" );

		std::size_t second = 0;
		checks.That(
		    AddCordon( edit, "", far, &second ).HasValue() && second == 1, "a second cordon" );
		checks.That( SetCordonActive( edit, 0, false ).HasValue(), "deactivate the first" );
		checks.That( ActiveCordonBoxes( edit ) == std::vector<scene::Box>{ far } &&
		                 !InsideCordons( edit, unit ),
		    "only active cordons restrict" );
		checks.That( SetCordonActive( edit, 0, false ).Error().code == app::EditErrorCode::Nothing,
		    "already inactive" );
		checks.That( SetCordonActive( edit, 9, true ).Error().code == app::EditErrorCode::Rejected,
		    "bad index (negative)" );
		checks.That( SetCordonActive( edit, 1, false ).HasValue() &&
		                 ActiveCordonBoxes( edit ).empty() && InsideCordons( edit, unit ),
		    "no active cordon: no restriction" );
		checks.That(
		    RenameCordon( edit, 1, "b" ).HasValue() && edit.Settings().cordons[1].name == "b",
		    "rename" );
		checks.That(
		    RenameCordon( edit, 1, "b" ).Error().code == app::EditErrorCode::Nothing, "same name" );
		checks.That( RenameCordon( edit, 2, "c" ).Error().code == app::EditErrorCode::Rejected,
		    "rename bad index (negative)" );
		checks.That( RemoveCordon( edit, 0 ).HasValue() && edit.Settings().cordons.size() == 1 &&
		                 edit.Settings().cordons[0].name == "b",
		    "remove a cordon" );
		checks.That( RemoveCordon( edit, 1 ).Error().code == app::EditErrorCode::Rejected,
		    "remove bad index (negative)" );

		// Degenerate boxes.
		const double nan = std::numeric_limits<double>::quiet_NaN();
		const double inf = std::numeric_limits<double>::infinity();
		checks.That( !AddCordon( edit, "flat", scene::Box{ Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 0 ) } ),
		    "zero extent (negative)" );
		checks.That(
		    !AddCordon( edit, "inv", scene::Box{ Vec3d( 0, 0, 0 ), Vec3d( -64, 64, 64 ) } ),
		    "inverted (negative)" );
		checks.That(
		    !AddCordon( edit, "nan", scene::Box{ Vec3d( nan, 0, 0 ), Vec3d( 64, 64, 64 ) } ),
		    "NaN (negative)" );
		checks.That(
		    !AddCordon( edit, "inf", scene::Box{ Vec3d( 0, 0, 0 ), Vec3d( inf, 64, 64 ) } ),
		    "infinite (negative)" );
		checks.That( !AddCordonBox( edit, 0, scene::Box{ Vec3d( 0, 0, 0 ), Vec3d( 0, 1, 1 ) } ),
		    "degenerate added box (negative)" );
		checks.That( !SetCordonBox( edit, 0, 0, scene::Box{ Vec3d( 5, 0, 0 ), Vec3d( 1, 1, 1 ) } ),
		    "degenerate set box (negative)" );
		checks.That( edit.Settings().cordons.size() == 1, "refusals added nothing" );
	}
	checks.That( empty.Settings().cordons.empty(), "the base document is untouched" );

	// A map loaded with the older single 'cordon' block.
	scene::MapDocument single;
	{
		scene::DocumentEdit edit( single );
		scene::DocumentSettings &s = edit.MutableSettings();
		scene::Cordon c;
		c.active = false;
		c.boxes.push_back( scene::CordonBox{ unit.mins, unit.maxs } );
		s.cordons = { c };
		s.cordonsActive = false;
		s.cordonForm = Form::Single;
		scene::CommitEdit( single, edit );
	}
	{
		scene::DocumentEdit edit( single );
		checks.That( SetCordonsEnabled( edit, true ).HasValue(), "toggle the single cordon on" );
		checks.That( edit.Settings().cordonsActive && edit.Settings().cordons[0].active &&
		                 edit.Settings().cordonForm == Form::Single,
		    "both flags move together; the Single form is kept" );
		checks.That(
		    ActiveCordonBoxes( edit ) == std::vector<scene::Box>{ unit }, "single box active" );
		checks.That( SetCordonActive( edit, 0, false ).HasValue() &&
		                 !edit.Settings().cordonsActive &&
		                 edit.Settings().cordonForm == Form::Single,
		    "the cordon flag is the document flag in the Single form" );
		checks.That( SetCordonBox( edit, 0, 0, far ).HasValue() &&
		                 edit.Settings().cordonForm == Form::Single,
		    "moving the box keeps the Single form" );
	}
	{
		scene::DocumentEdit edit( single );
		checks.That(
		    RenameCordon( edit, 0, "named" ).HasValue() && edit.Settings().cordonForm == Form::List,
		    "a name does not fit the Single form" );
	}
	{
		scene::DocumentEdit edit( single );
		checks.That(
		    AddCordonBox( edit, 0, far ).HasValue() && edit.Settings().cordonForm == Form::List,
		    "a second box does not fit the Single form" );
		checks.That(
		    RemoveCordonBox( edit, 0, 1 ).HasValue() && edit.Settings().cordonForm == Form::List,
		    "the form does not switch back" );
	}
	{
		scene::DocumentEdit edit( single );
		checks.That(
		    AddCordon( edit, "", far ).HasValue() && edit.Settings().cordonForm == Form::List,
		    "a second cordon does not fit the Single form" );
	}
	{
		scene::DocumentEdit edit( single );
		checks.That( RemoveCordon( edit, 0 ).HasValue() &&
		                 edit.Settings().cordonForm == Form::List &&
		                 edit.Settings().cordons.empty(),
		    "removing the only cordon keeps an (empty) list" );
	}

	checks.That(
	    InsideCordons( std::vector<scene::Box>{}, far ), "an empty box list restricts nothing" );
	checks.That( ValidCordonBox( unit ) && !ValidCordonBox( scene::Box{} ), "box validity" );

	return checks.Report();
}
