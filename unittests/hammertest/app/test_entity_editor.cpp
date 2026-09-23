//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance suite for the shared entity editor (RFC 0002, Q-EDITOR).
//			Pins the selection + multi-selection property aggregation + multi-
//			entity mutation policy that BOTH sibling shells (hammer.adapters.gtk
//			and hammer.adapters.mfc) route through, so the two toolkits share one
//			entity editor logic. Headless: no MFC/tier0/PCH/GPU.
//
//			Contract: unittests/hammertest/contracts/app.entity_editor.v1.md
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_document.h"
#include "hammer/app/entity_selection.h"
#include "hammer/app/property_value.h"
#include "testing/conformance_result.h"

#include <cstddef>
#include <cstdio>
#include <initializer_list>
#include <string>
#include <vector>

using hammer::app::EditorDocument;
using hammer::app::EntitySelection;
using hammer::app::PropertyValue;

namespace
{
int g_failures = 0;
int g_checks = 0;
void Check( bool ok, const char *label )
{
	++g_checks;
	if ( !ok )
	{
		std::printf( "FAIL: %s\n", label );
		++g_failures;
	}
}

// A document with five top-level entities exercising every aggregation case:
//   0 world     classname=worldspawn
//   1 entity    classname=info_player_start  targetname=spawn1
//   2 entity    classname=info_player_start  targetname=spawn2
//   3 entity    classname=light              (no targetname)
//   4 entity    classname=trigger            targetname=""   (empty, not unset)
const char *const kDoc =
    "world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
    "entity\n{\n\t\"classname\" \"info_player_start\"\n\t\"targetname\" \"spawn1\"\n}\n"
    "entity\n{\n\t\"classname\" \"info_player_start\"\n\t\"targetname\" \"spawn2\"\n}\n"
    "entity\n{\n\t\"classname\" \"light\"\n}\n"
    "entity\n{\n\t\"classname\" \"trigger\"\n\t\"targetname\" \"\"\n}\n";

EntitySelection Sel( std::initializer_list<std::size_t> indices )
{
	EntitySelection sel;
	for ( std::size_t i : indices )
	{
		sel.Add( i );
	}
	return sel;
}

const std::string *TargetName( const EditorDocument &doc, std::size_t index )
{
	return doc.Content().children[index].Find( "targetname" );
}

void TestSelectionValueType()
{
	EntitySelection sel;
	Check( sel.Empty() && sel.Size() == 0, "fresh selection is empty" );
	Check( sel.Add( 3 ), "Add(3) is a new insertion" );
	Check( !sel.Add( 3 ), "Add(3) again is not new" );
	Check( sel.Contains( 3 ) && !sel.Contains( 4 ), "membership tracked" );
	Check( sel.Toggle( 4 ) && sel.Contains( 4 ), "Toggle(4) selects" );
	Check( !sel.Toggle( 4 ) && !sel.Contains( 4 ), "Toggle(4) again clears" );
	Check( sel.Remove( 3 ) && !sel.Remove( 3 ), "Remove is idempotent-aware" );

	// Order-independence + dedup: two click orders produce an equal, sorted set.
	Check(
	    Sel( { 2, 0, 1, 2 } ) == Sel( { 0, 1, 2 } ), "selection is order-independent and deduped" );
	const EntitySelection sorted = Sel( { 5, 1, 3 } );
	const std::vector<std::size_t> &idx = sorted.Indices();
	Check( idx.size() == 3 && idx[0] == 1 && idx[1] == 3 && idx[2] == 5,
	    "indices are sorted ascending" );
}

void TestEnumeration( EditorDocument &doc )
{
	Check( doc.EntityCount() == 5, "EntityCount counts top-level blocks" );
	Check( doc.EntityName( 0 ) == "world" && doc.EntityName( 1 ) == "entity",
	    "EntityName is the block keyword" );
	Check( doc.EntityName( 99 ).empty(), "EntityName out of range is empty" );
	Check( doc.EntityClassName( 0 ) == "worldspawn", "EntityClassName(world)" );
	Check( doc.EntityClassName( 3 ) == "light", "EntityClassName(light)" );
	Check( doc.EntityClassName( 99 ).empty(), "EntityClassName out of range is empty" );
}

void TestAggregation( const EditorDocument &doc )
{
	// Agreement across a multi-selection -> Single.
	const PropertyValue cls = doc.AggregateProperty( Sel( { 1, 2 } ), "classname" );
	Check( cls.IsSingle() && cls.Value() == "info_player_start", "agree -> Single(value)" );

	// Disagreement -> Mixed (distinct from empty and unset).
	Check( doc.AggregateProperty( Sel( { 1, 2 } ), "targetname" ).IsMixed(), "disagree -> Mixed" );

	// Empty selection -> Unset.
	Check( doc.AggregateProperty( Sel( {} ), "classname" ).IsUnset(), "empty selection -> Unset" );

	// Nobody defines the key -> Unset.
	Check(
	    doc.AggregateProperty( Sel( { 3 } ), "targetname" ).IsUnset(), "no contributor -> Unset" );

	// Presence-only policy: a non-contributor (light) does not force Mixed.
	const PropertyValue partial = doc.AggregateProperty( Sel( { 1, 3 } ), "targetname" );
	Check( partial.IsSingle() && partial.Value() == "spawn1",
	    "non-contributor is skipped, not mixed" );

	// empty-vs-unset: entity 4 has targetname="" -> Single(""), NOT Unset.
	const PropertyValue empty = doc.AggregateProperty( Sel( { 4 } ), "targetname" );
	Check( empty.IsSingle() && empty.Value().empty(), "empty value -> Single(\"\"), not Unset" );

	// An empty string genuinely disagrees with a non-empty value -> Mixed.
	Check( doc.AggregateProperty( Sel( { 1, 4 } ), "targetname" ).IsMixed(),
	    "\"\" vs value -> Mixed" );

	// Stale/out-of-range index is silently ignored in aggregation.
	const PropertyValue withStale = doc.AggregateProperty( Sel( { 1, 99 } ), "classname" );
	Check( withStale.IsSingle() && withStale.Value() == "info_player_start",
	    "out-of-range index ignored" );
}

void TestMultiEntityEditIsOneUndoUnit()
{
	EditorDocument doc;
	std::string error;
	Check( doc.LoadFromText( kDoc, error ), "load doc for edit" );
	Check( !doc.IsModified(), "loaded document is unmodified" );

	// Edit a two-entity selection in one call.
	const EntitySelection two = Sel( { 1, 2 } );
	Check( doc.SetPropertyOnSelection( two, "targetname", "relay" ),
	    "multi-entity set changes content" );
	Check( doc.IsModified() && doc.CanUndo(), "edit marks modified and enables undo" );
	Check( doc.AggregateProperty( two, "targetname" ).Value() == "relay",
	    "both entities now agree on new value" );

	// A single Undo must revert BOTH entities (one history unit for the group edit).
	Check( doc.Undo(), "one undo" );
	Check( TargetName( doc, 1 ) != nullptr && *TargetName( doc, 1 ) == "spawn1",
	    "undo restored entity 1" );
	Check( TargetName( doc, 2 ) != nullptr && *TargetName( doc, 2 ) == "spawn2",
	    "undo restored entity 2" );
	Check( !doc.IsModified(), "undo to saved position clears modified" );
	Check( !doc.CanUndo(), "only one unit was recorded" );

	// Redo restores both.
	Check( doc.Redo(), "redo" );
	Check( *TargetName( doc, 1 ) == "relay" && *TargetName( doc, 2 ) == "relay",
	    "redo reapplied to both" );

	// No-op: setting the same value again records nothing.
	Check(
	    !doc.SetPropertyOnSelection( two, "targetname", "relay" ), "same-value edit is a no-op" );
	Check( !doc.CanRedo(), "no-op added no redoable state" );
}

void TestEditEdgeCases()
{
	EditorDocument doc;
	std::string error;
	doc.LoadFromText( kDoc, error );

	// Adds the key where absent (entity 3 = light has no targetname).
	Check( doc.SetPropertyOnSelection( Sel( { 3 } ), "targetname", "sun" ), "adds absent key" );
	Check( doc.EntityClassName( 3 ) == "light", "unrelated keys untouched" );
	Check( *TargetName( doc, 3 ) == "sun", "absent key added with value" );

	// Empty selection is a no-op.
	Check( !doc.SetPropertyOnSelection( Sel( {} ), "targetname", "x" ),
	    "empty selection edit is a no-op" );

	// A selection containing ONLY out-of-range indices is a no-op.
	Check( !doc.SetPropertyOnSelection( Sel( { 99, 123 } ), "targetname", "x" ),
	    "all-out-of-range selection is a no-op" );

	// A mixed valid/invalid selection edits only the valid entity.
	Check( doc.SetPropertyOnSelection( Sel( { 0, 99 } ), "mapversion", "7" ),
	    "valid+invalid edits valid only" );
	Check( *doc.Content().children[0].Find( "mapversion" ) == "7", "valid index edited" );
}

} // namespace

int main()
{
	EditorDocument doc;
	std::string error;
	if ( !doc.LoadFromText( kDoc, error ) )
	{
		std::printf( "FAIL: could not load fixture: %s\n", error.c_str() );
		return 1;
	}

	TestSelectionValueType();
	TestEnumeration( doc );
	TestAggregation( doc );
	TestMultiEntityEditIsOneUndoUnit();
	TestEditEdgeCases();

	if ( g_failures != 0 )
	{
		std::printf( "app.entity_editor: %d FAILURE(S)\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "app.entity_editor: selection + multi-select aggregation + atomic multi-entity "
	             "edit verified (shared by both shells)\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
