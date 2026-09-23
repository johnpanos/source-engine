//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::app::EditorDocument (RFC 0002). Runs the
//			reference headless workflow end-to-end: open a VMF, edit, undo, redo,
//			save, reopen, and compare -- integrating the codec, history, and save
//			seams. Also pins no-op-edit neutrality and the save-position/modified
//			interaction.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_document.h"

#include "fake_file_store.h"

#include "hammer/formats/keyvalues.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <string>

using hammer::app::EditorDocument;
using hammer::app::SaveStatus;
using hammer::formats::CompareKeyValues;
using hammertest::InMemoryFileStore;

namespace
{

int g_failures = 0;
int g_checks = 0;

void Check( bool condition, const char *expression, int line )
{
	++g_checks;
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_editor_document.cpp", line, expression );
		++g_failures;
	}
}

const char *kVmf = "world\n{\n\t\"id\" \"1\"\n\t\"classname\" \"worldspawn\"\n}\n";

std::string FirstBlockValue( const EditorDocument &doc, const std::string &key )
{
	if ( doc.Content().children.empty() )
	{
		return "<none>";
	}
	const std::string *value = doc.Content().children[0].Find( key );
	return value ? *value : "<none>";
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

// open -> edit -> undo -> redo -> save -> reopen -> compare
void TestReferenceWorkflow()
{
	EditorDocument doc;
	std::string error;
	CHECK( doc.LoadFromText( kVmf, error ) );
	CHECK( !doc.IsModified() );
	CHECK( FirstBlockValue( doc, "classname" ) == "worldspawn" );

	// Edit one property -> one history unit, modified.
	CHECK( doc.SetFirstBlockKey( "classname", "func_detail" ) );
	CHECK( doc.IsModified() );
	CHECK( FirstBlockValue( doc, "classname" ) == "func_detail" );

	// Undo restores the recorded snapshot and clears modified (back to saved pos).
	CHECK( doc.Undo() );
	CHECK( FirstBlockValue( doc, "classname" ) == "worldspawn" );
	CHECK( !doc.IsModified() );

	// Redo reapplies.
	CHECK( doc.Redo() );
	CHECK( FirstBlockValue( doc, "classname" ) == "func_detail" );

	// Save transactionally, then reopen from the store and compare semantically.
	InMemoryFileStore store;
	CHECK( doc.Save( store, "map.vmf" ) == SaveStatus::kOk );
	CHECK( !doc.IsModified() );

	EditorDocument reopened;
	CHECK( reopened.Load( store, "map.vmf", error ) );
	CHECK( CompareKeyValues( doc.Content(), reopened.Content() ).equal );
	CHECK( FirstBlockValue( reopened, "classname" ) == "func_detail" );
}

// A no-op edit adds no history unit and does not mark the document modified.
void TestNoOpEditIsNeutral()
{
	EditorDocument doc;
	std::string error;
	CHECK( doc.LoadFromText( kVmf, error ) );

	CHECK( !doc.SetFirstBlockKey( "classname", "worldspawn" ) ); // same value -> no-op
	CHECK( !doc.IsModified() );
	CHECK( !doc.CanUndo() );
}

// After saving, undo makes the document modified again (position != saved).
void TestSavePositionInteraction()
{
	EditorDocument doc;
	std::string error;
	doc.LoadFromText( kVmf, error );
	doc.SetFirstBlockKey( "classname", "trigger_once" );

	InMemoryFileStore store;
	CHECK( doc.Save( store, "map.vmf" ) == SaveStatus::kOk );
	CHECK( !doc.IsModified() );

	CHECK( doc.Undo() );
	CHECK( doc.IsModified() ); // back before the saved edit
	CHECK( doc.Redo() );
	CHECK( !doc.IsModified() ); // back at the saved position
}

int main()
{
	TestReferenceWorkflow();
	TestNoOpEditIsNeutral();
	TestSavePositionInteraction();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.app EditorDocument: %d check(s) FAILED\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "hammer.app EditorDocument: reference workflow passed\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
