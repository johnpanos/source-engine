//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app entity search and replace (RFC 0002, R08 domain
//			logic): class filters (substring, whole, wildcard; always
//			case-insensitive), key and value filters (a key alone asks for the
//			key), connection targets and parameters searched without a key,
//			case sensitivity, whole-value and glob matching, visibility and
//			scope; ReplaceKeyValues rewrites every occurrence (Contains) or
//			whole values (Whole, Wildcard) and counts changed values. Negative
//			checks: an empty pattern, no match and an unchanged replacement
//			stage nothing and count zero.
//
//=============================================================================//

#include "hammer/app/ops/find_replace_ops.h"
#include "testing/checks.h"

using namespace hammer;
using namespace hammer::app;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using scene::ObjectId;

int main()
{
	testing::Checks checks;

	// Text matching.
	checks.That( TextMatches( "Door_A", "or_a", TextMatch::Contains, false ) &&
	                 !TextMatches( "Door_A", "or_a", TextMatch::Contains, true ),
	    "contains, with and without case" );
	checks.That( TextMatches( "door", "DOOR", TextMatch::Whole, false ) &&
	                 !TextMatches( "door2", "door", TextMatch::Whole, false ),
	    "whole value" );
	checks.That( TextMatches( "axxbyyc", "a*b*c", TextMatch::Wildcard, false ) &&
	                 TextMatches( "abc", "a?c", TextMatch::Wildcard, false ) &&
	                 !TextMatches( "abcd", "a?c", TextMatch::Wildcard, false ) &&
	                 TextMatches( "", "*", TextMatch::Wildcard, false ) &&
	                 TextMatches( "aXb", "a*B", TextMatch::Wildcard, false ) &&
	                 !TextMatches( "aXb", "a*B", TextMatch::Wildcard, true ),
	    "globs anchor at both ends and backtrack" );

	scene::MapDocument doc;
	ObjectId lampA, lampB, crate, trigger, door, hidden;
	{
		scene::DocumentEdit edit( doc );
		auto make = [&]( const char *classname, const char *name )
		{
			scene::Entity e;
			e.classname = classname;
			if ( name )
			{
				e.SetKey( "targetname", name );
			}
			e.SetOrigin( Vec3d( 0, 0, 0 ) );
			return e;
		};
		scene::Entity a = make( "light", "lamp_a" );
		a.SetKey( "_light", "255 255 255 200" );
		lampA = edit.Add( a );
		lampB = edit.Add( make( "light_spot", "lamp_b" ) );
		scene::Entity c = make( "prop_static", nullptr );
		c.SetKey( "model", "models/props/crate.mdl" );
		crate = edit.Add( c );
		scene::Entity t = make( "trigger_once", nullptr );
		t.connections.push_back(
		    *scene::ParseConnection( "OnTrigger", "lamp_a,TurnOn,lamp_a_x,0,-1" ) );
		trigger = edit.Add( t );
		door = edit.Add( make( "info_target", "Door_A_A" ) );
		scene::Entity h = make( "light", "lamp_hidden" );
		h.hidden = true;
		hidden = edit.Add( h );
		scene::CommitEdit( doc, edit );
	}

	auto find = [&]( EntityQuery q )
	{
		return FindEntities( doc, q );
	};
	EntityQuery q;
	q.classPattern = "LIGHT";
	checks.Equal( find( q ).size(), std::size_t( 3 ), "class substring, any case" );
	q.classMatch = TextMatch::Whole;
	checks.Equal( find( q ).size(), std::size_t( 2 ), "class whole" );
	q.classPattern = "prop_*";
	q.classMatch = TextMatch::Wildcard;
	checks.That( find( q ) == std::vector<ObjectId>{ crate }, "class wildcard" );

	q = EntityQuery{};
	q.key = "MODEL";
	checks.That( find( q ) == std::vector<ObjectId>{ crate }, "a key alone asks for the key" );
	q.valuePattern = "crate";
	checks.That( find( q ) == std::vector<ObjectId>{ crate }, "key and value" );
	q.valuePattern = "lamp";
	checks.That( find( q ).empty(), "value under another key does not match" );

	q = EntityQuery{};
	q.valuePattern = "lamp_a";
	checks.That( find( q ) == std::vector<ObjectId>{ lampA, trigger },
	    "no key: key values and connection targets" );
	q.valuePattern = "LAMP_A";
	q.caseSensitive = true;
	checks.That( find( q ).empty(), "case-sensitive values" );
	q = EntityQuery{};
	q.valuePattern = "lamp_?";
	q.valueMatch = TextMatch::Wildcard;
	checks.That( find( q ) == std::vector<ObjectId>{ lampA, lampB, trigger }, "wildcard values" );
	q.visibleOnly = false;
	q.valuePattern = "lamp_*";
	checks.Equal( find( q ).size(), std::size_t( 4 ), "hidden entities are found" );
	q.visibleOnly = true;
	checks.That( find( q ) == std::vector<ObjectId>{ lampA, lampB, trigger }, "visible only" );
	q.visibleOnly = false;
	q.within = std::vector<ObjectId>{ lampB, hidden, ObjectId() };
	checks.That( find( q ) == std::vector<ObjectId>{ lampB, hidden }, "within a scope" );
	q = EntityQuery{};
	q.valuePattern = "lamp";
	q.valueMatch = TextMatch::Whole;
	checks.That( find( q ).empty(), "whole values do not match substrings" );
	checks.Equal(
	    find( EntityQuery{} ).size(), std::size_t( 6 ), "an empty query finds every entity" );

	// Replace every occurrence, case-insensitively.
	{
		scene::DocumentEdit edit( doc );
		EntityQuery r;
		r.valuePattern = "_a";
		int count = -1;
		checks.That( ReplaceKeyValues( edit, r, "_x", count ).HasValue(), "replace substring" );
		// lamp_a (targetname), the trigger's target and parameter, Door_A_A.
		checks.Equal( count, 4, "count of changed values" );
		checks.That( edit.FindEntity( door )->Name() == "Door_x_x", "every occurrence replaced" );
		checks.That( edit.FindEntity( lampA )->Name() == "lamp_x" &&
		                 *edit.FindEntity( lampA )->Key( "_light" ) == "255 255 255 200",
		    "only matching values change" );
		const scene::Connection &c = edit.FindEntity( trigger )->connections[0];
		checks.That( c.target == "lamp_x" && c.parameter == "lamp_x_x" && c.input == "TurnOn",
		    "connection target and parameter replaced, input untouched" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after replace" );
		checks.Equal( edit.Finish().Modified().size(), std::size_t( 3 ), "three entities changed" );
	}
	{
		scene::DocumentEdit edit( doc );
		EntityQuery r;
		r.key = "model";
		r.valuePattern = "models/props/*.mdl";
		r.valueMatch = TextMatch::Wildcard;
		int count = 0;
		checks.That( ReplaceKeyValues( edit, r, "models/props/barrel.mdl", count ).HasValue() &&
		                 count == 1 &&
		                 *edit.FindEntity( crate )->Key( "model" ) == "models/props/barrel.mdl",
		    "wildcard replaces the whole value of the key" );
		EntityQuery w;
		w.key = "targetname";
		w.valuePattern = "LAMP_B";
		w.valueMatch = TextMatch::Whole;
		checks.That( ReplaceKeyValues( edit, w, "spot", count ).HasValue() && count == 1 &&
		                 edit.FindEntity( lampB )->Name() == "spot",
		    "whole replaces the value" );
		EntityQuery scoped;
		scoped.valuePattern = "lamp";
		scoped.within = std::vector<ObjectId>{ hidden };
		checks.That( ReplaceKeyValues( edit, scoped, "bulb", count ).HasValue() && count == 1 &&
		                 edit.FindEntity( hidden )->Name() == "bulb_hidden" &&
		                 edit.FindEntity( lampA )->Name() == "lamp_a",
		    "replacement honors the scope" );
	}

	// Refusals stage nothing.
	{
		scene::DocumentEdit edit( doc );
		int count = 7;
		EntityQuery empty;
		checks.That(
		    ReplaceKeyValues( edit, empty, "x", count ).Error().code == EditErrorCode::Rejected &&
		        count == 0,
		    "an empty pattern (negative)" );
		EntityQuery none;
		none.valuePattern = "no such text";
		count = 7;
		checks.That(
		    ReplaceKeyValues( edit, none, "x", count ).Error().code == EditErrorCode::Nothing &&
		        count == 0,
		    "no match (negative)" );
		EntityQuery same;
		same.valuePattern = "lamp_a";
		same.valueMatch = TextMatch::Whole;
		checks.That( ReplaceKeyValues( edit, same, "LAMP_A", count ).HasValue() && count == 2,
		    "a case-only change counts" );
		scene::DocumentEdit edit2( doc );
		checks.That( ReplaceKeyValues( edit2, same, "lamp_a", count ).Error().code ==
		                     EditErrorCode::Nothing &&
		                 count == 0,
		    "an unchanged replacement is nothing (negative)" );
		checks.That( edit2.Finish().Empty(), "refusals staged nothing" );
	}

	return checks.Report();
}
