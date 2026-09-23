//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the entity editor's atomic multi-entity edit
//			(RFC 0002/0005). The shared contract is that setting a property across
//			a multi-entity selection is ONE undo unit: a single Undo restores every
//			touched entity. A "helpful" shell that instead committed one history
//			unit per entity would need N undos to revert N entities -- the classic
//			bug where multi-select edits are not atomic. The oracle must detect it.
//
//			We drive the REAL EditorDocument two ways: correctly (one grouped call)
//			and in the broken per-entity fashion (a call per entity). The invariant
//			predicate -- "a single Undo restores all touched entities" -- must hold
//			for the correct path and FAIL for the broken one.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_document.h"
#include "hammer/app/entity_selection.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::app::EditorDocument;
using hammer::app::EntitySelection;

namespace
{
const char *const kDoc =
    "entity\n{\n\t\"classname\" \"info_player_start\"\n\t\"targetname\" \"spawn1\"\n}\n"
    "entity\n{\n\t\"classname\" \"info_player_start\"\n\t\"targetname\" \"spawn2\"\n}\n";

EntitySelection One( std::size_t i )
{
	EntitySelection sel;
	sel.Add( i );
	return sel;
}

std::string TargetName( const EditorDocument &doc, std::size_t index )
{
	const std::string *value = doc.Content().children[index].Find( "targetname" );
	return value != nullptr ? *value : std::string();
}

// The shared invariant, evaluated against a document that has just had entities 0
// and 1 edited to "relay": one Undo must return BOTH to their original values.
bool SingleUndoRestoresBoth( EditorDocument &doc )
{
	if ( !doc.Undo() )
	{
		return false;
	}
	return TargetName( doc, 0 ) == "spawn1" && TargetName( doc, 1 ) == "spawn2";
}

// Correct: one grouped edit -> one history unit.
bool RealPathConforms()
{
	EditorDocument doc;
	std::string error;
	doc.LoadFromText( kDoc, error );

	EntitySelection both;
	both.Add( 0 );
	both.Add( 1 );
	doc.SetPropertyOnSelection( both, "targetname", "relay" );
	return SingleUndoRestoresBoth( doc );
}

// Broken: edit each entity in its own call -> two history units. A single Undo
// reverts only the last-edited entity, so the invariant must NOT hold.
bool BrokenPathConforms()
{
	EditorDocument doc;
	std::string error;
	doc.LoadFromText( kDoc, error );

	doc.SetPropertyOnSelection( One( 0 ), "targetname", "relay" ); // unit 1
	doc.SetPropertyOnSelection( One( 1 ), "targetname", "relay" ); // unit 2 (the bug)
	return SingleUndoRestoresBoth( doc );
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;

	++checks;
	if ( !RealPathConforms() )
	{
		std::printf( "FAIL: real grouped edit was not atomic under one Undo\n" );
		++failures;
	}
	++checks;
	if ( BrokenPathConforms() )
	{
		std::printf( "FAIL: invariant did NOT detect the non-atomic per-entity editor\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "app.entity_editor.sensitivity: %d FAILURE(S)\n", failures );
		return testing::ReportConformance( checks, failures );
	}
	std::printf( "app.entity_editor.sensitivity: atomic-multi-entity-undo invariant is "
	             "effective (real passes, non-atomic editor caught)\n" );
	return testing::ReportConformance( checks, failures );
}
