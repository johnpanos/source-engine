//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless workflow oracle for the thin MFC sibling (RFC 0002,
//			hammer.adapters.mfc). Mirrors test_shell_workflow.cpp (the GTK sibling
//			oracle) but is LINKED AGAINST MFC and run under Wine, so it proves two
//			things a GUI cannot on a headless host: (1) MFC and the reusable,
//			engine-free editor libraries link and coexist in one binary, and
//			(2) the delegated New -> Set Key -> Save -> reopen -> Undo/Redo workflow
//			the MFC shell's command handlers invoke actually executes correctly.
//
//			It is a console MFC program (afx.h + CString touch-point), not a GUI,
//			so it needs no display. The GUI shell (hammer_mfc_shell.cpp) is
//			separately proven to compile+link. Build/run:
//			unittests/hammertest/adapters/build_mfc_shell.sh (which builds and runs
//			this under Wine).
//
//=============================================================================//

#include <afx.h> // links MFC; proves MFC coexists with the reusable libraries

#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/app/editor_document.h"
#include "hammer/formats/keyvalues.h"

#include <cstdio>
#include <filesystem>
#include <random>
#include <string>

using hammer::app::EditorDocument;
using hammer::app::SaveStatus;

namespace
{
int g_failures = 0;
void Check( bool ok, const char *label )
{
	if ( !ok )
	{
		std::printf( "FAIL: %s\n", label );
		++g_failures;
	}
}

const std::string *FirstBlockValue( const EditorDocument &doc, const std::string &key )
{
	const auto &children = doc.Content().children;
	return children.empty() ? nullptr : children.front().Find( key );
}
} // namespace

int main()
{
	// Touch MFC so the link is real, not merely present: build the temp-dir name
	// with a CString, exactly the kind of glue the MFC shell uses.
	CString stamp;
	stamp.Format( _T( "hammer_mfc_wf_%u" ), static_cast<unsigned>( std::random_device{}() ) );
	CStringA stampA( stamp );

	namespace fs = std::filesystem;
	std::error_code ec;
	fs::path dir = fs::temp_directory_path( ec ) / static_cast<const char *>( stampA );
	fs::create_directories( dir, ec );
	if ( ec )
	{
		std::printf( "FAIL: temp dir\n" );
		return 1;
	}
	const std::string path = ( dir / "map.vmf" ).string();

	hammer::adapters::platform::DiskFileStore store;

	// "New" then "Set classname" (what CHammerFrame::OnNew / OnApplyEdit do).
	EditorDocument doc;
	std::string error;
	Check( doc.LoadFromText( "world\n{\n\t\"classname\" \"worldspawn\"\n}\n", error ),
		   "New: seed document loads" );
	Check( !doc.IsModified(), "New: fresh document is unmodified" );
	Check( doc.SetFirstBlockKey( "classname", "func_detail" ), "Set Key commits" );
	Check( doc.IsModified(), "Set Key marks modified" );
	Check( doc.CanUndo(), "Set Key enables undo" );

	// "Save" (OnSave) through the real disk store.
	Check( doc.Save( store, path ) == SaveStatus::kOk, "Save returns kOk" );
	Check( store.Exists( path ), "Save persisted a file on disk" );
	Check( !doc.IsModified(), "Save clears modified" );

	// "Open" (OnOpen) into a fresh document reads it back.
	EditorDocument reopened;
	Check( reopened.Load( store, path, error ), "Open loads the saved file" );
	const std::string *value = FirstBlockValue( reopened, "classname" );
	Check( value != nullptr && *value == "func_detail", "reopened content matches saved" );

	// "Undo" / "Redo" (OnUndo / OnRedo).
	Check( doc.Undo(), "Undo succeeds" );
	const std::string *undone = FirstBlockValue( doc, "classname" );
	Check( undone != nullptr && *undone == "worldspawn", "Undo restores original value" );
	Check( doc.Redo(), "Redo succeeds" );
	const std::string *redone = FirstBlockValue( doc, "classname" );
	Check( redone != nullptr && *redone == "func_detail", "Redo restores edited value" );

	fs::remove_all( dir, ec );

	if ( g_failures != 0 )
	{
		std::printf( "adapters.mfc_workflow: %d FAILURE(S)\n", g_failures );
		return 1;
	}
	std::printf( "adapters.mfc_workflow: MFC-linked composition (EditorDocument + "
				 "DiskFileStore) round-trips New/SetKey/Save/Open/Undo/Redo\n" );
	return 0;
}
